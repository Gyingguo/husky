// Copyright 2016 Husky Team
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
// http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#include "bindings/backend/library/gradient_descent.hpp"

#include <map>
#include <string>
#include <utility>
#include <vector>

#include "bindings/backend/pythonconnector.hpp"
#include "bindings/backend/threadconnector.hpp"
#include "bindings/backend/workerdriver.hpp"
#include "bindings/itc.hpp"
#include "bindings/operation.hpp"
#include "core/baseworker.hpp"
#include "core/context.hpp"
#include "core/utils.hpp"
#include "core/workerdevapi.hpp"
#include "core/zmq_helpers.hpp"
#include "lib/dataloader.hpp"
#include "lib/gradient_descent.hpp"

namespace husky {

typedef std::vector<double> vec_double;
typedef ML::FeatureLabel FeatureLabel;

thread_local std::map<std::string, ML::SGD_LinearRegression<FeatureLabel>> local_SGD_LinearR_model;

void PyHuskyML::init_py_handlers() {
    PythonConnector::add_handler("LinearRegressionModel#LinearR_load_pyhlist_py", LinearR_load_pyhlist_handler);
}

void PyHuskyML::init_cpp_handlers() {
    WorkerDriver::add_handler("LinearRegressionModel#LinearR_init_py", LinearR_init_handler);
    WorkerDriver::add_handler("LinearRegressionModel#LinearR_load_hdfs_py", LinearR_load_hdfs_handler);
    WorkerDriver::add_handler("LinearRegressionModel#LinearR_train_py", LinearR_train_handler);
}

void PyHuskyML::init_daemon_handlers() {
    ThreadConnector::add_handler("LinearRegressionModel#LinearR_train", daemon_train_handler);
}

void PyHuskyML::LinearR_load_pyhlist_handler(PythonSocket & python_socket, ITCWorker & daemon_socket) {
    auto & worker = Context::get_worker<BaseWorker>();
    std::string name = zmq_recv_string(python_socket.pipe_from_python);
    auto & load_list = worker.create_list<FeatureLabel>(name);
    int n_sample = std::stoi(zmq_recv_string(python_socket.pipe_from_python));

    Aggregator<int> n_feature_agg(0, [](int & a, const int & b){a = b;});

    for (int i = 0; i < n_sample; i++) {
        auto key = std::make_pair(worker.id, i);
        std::vector<double> X;
        int n_feature = std::stoi(zmq_recv_string(python_socket.pipe_from_python));
        for (int j = 0; j < n_feature; j++) {
            double X_elem = std::stod(zmq_recv_string(python_socket.pipe_from_python));
            X.push_back(X_elem);
        }
        n_feature_agg.update(n_feature);
        double y = std::stod(zmq_recv_string(python_socket.pipe_from_python));
        FeatureLabel this_obj(X, y, key);
        worker.add_object(load_list, this_obj);
    }
    worker.globalize_list(load_list);
    worker.list_execute(load_list, [&](FeatureLabel& this_obj){});

    int n_feature = n_feature_agg.get_value();
    assert(n_feature > 0);
    local_SGD_LinearR_model[name].set_n_feature(n_feature);
}

void PyHuskyML::LinearR_init_handler(const Operation & op,
        PythonSocket & python_socket,
        ITCWorker & daemon_socket) {
    auto & worker = Husky::Context::get_worker<Husky::BaseWorker>();
    // Get Parameters sent from python
    const std::string & name = op.get_param("list_name");
    // Create LR model and store at local
    ML::SGD_LinearRegression<> SGD_MODEL(-1);
    local_SGD_LinearR_model.insert(
            std::pair<std::string, ML::SGD_LinearRegression<>>(
                name, SGD_MODEL));
    log_msg("create SGD Linear Regeression Model: " + name);
}

void PyHuskyML::LinearR_load_hdfs_handler(const Operation & op,
        PythonSocket & python_socket,
        ITCWorker & daemon_socket) {
    auto & worker = Husky::Context::get_worker<Husky::BaseWorker>();
    // Get Parameters sent from python
    const std::string & url = op.get_param("url");
    const std::string & name = op.get_param("list_name");
    auto & load_list = worker.create_list<FeatureLabel>(name);

    Husky::ML::Data_Loader<> data_loader;
    data_loader.load_info(url, load_list, ML::LAST_ONE);
    worker.globalize_list(load_list);
    int n_feature = data_loader.get_n_feature();

    assert(n_feature > 0);
    local_SGD_LinearR_model[name].set_n_feature(n_feature);
}

void PyHuskyML::LinearR_train_handler(const Operation & op,
        PythonSocket & python_socket,
        ITCWorker & daemon_socket) {
    auto & worker = Context::get_worker<BaseWorker>();
    const std::string & name = op.get_param("list_name");
    auto & train_list = worker.create_list<FeatureLabel>(name);
    const int n_iter = stoi(op.get_param("n_iter"));
    const double alpha = stod(op.get_param("alpha"));

    int n_feature = local_SGD_LinearR_model[name].get_n_feature();
    ML::LinearScaler<> linscaler(n_feature);
    linscaler.fit_transform(train_list);
    local_SGD_LinearR_model[name].train(train_list, n_iter, alpha);

    vec_double max_X = linscaler.get_max_X();
    double max_y = linscaler.get_max_y();

    // Send Back the parameter to pyHusky
    if (worker.id == 0) {
        int n_param = local_SGD_LinearR_model[name].get_n_param();
        BinStream result;
        result << n_param;
        assert(n_param > 0);
        auto params_vec = local_SGD_LinearR_model[name].get_param();
        // With Linear Scaling
        int n = params_vec.size();
        for (int i = 0; i < n-1; i++) {
            result << params_vec[i] / max_X[i] * max_y;
        }
        // Last term is intercept term
        result << params_vec[n-1] * max_y;
        daemon_socket.sendmore("LinearRegressionModel#LinearR_train");
        daemon_socket.send(std::move(result));
    }
}

void PyHuskyML::daemon_train_handler(ITCDaemon & to_worker, BinStream & buffer) {
    BinStream recv = to_worker.recv_binstream();
    int flag = 1;  // 1 means sent by cpp
    buffer << flag << recv.to_string();
}

}  // End of namespace husky
