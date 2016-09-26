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

#include "bindings/backend/library/spca.hpp"

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

namespace husky {
typedef ML::SparseRowObject SparseRowObject;
typedef Eigen::SparseMatrix<double> SpMat;

thread_local std::map<std::string, ML::SPCAModel> PyHuskyPCA::local_SPCA_models;

void PyHuskyPCA::init_py_handlers() {
    PythonConnector::add_handler("SPCA#spca_load_pyhlist_py", spca_load_pyhlist_handler);
}

void PyHuskyPCA::init_cpp_handlers() {
    WorkerDriver::add_handler("SPCA#spca_init_py", spca_init_handler);
    WorkerDriver::add_handler("SPCA#spca_load_hdfs_py", spca_load_hdfs_handler);
    WorkerDriver::add_handler("SPCA#spca_train_py", spca_train_handler);
    WorkerDriver::add_handler("SPCA#del_py", del_handler);
}

void PyHuskyPCA::init_daemon_handlers() {
    ThreadConnector::add_handler("SPCA#spca_train", daemon_train_handler);
}

void PyHuskyPCA::del_handler(const Operation & op,
        PythonSocket & python_socket,
        ITCWorker & daemon_socket) {
    const std::string & name = op.get_param("list_name");
    local_SPCA_models.erase(name);
}

void PyHuskyPCA::spca_load_pyhlist_handler(PythonSocket & python_socket,
        ITCWorker & daemon_socket) {
    auto & worker = Context::get_worker<BaseWorker>();
    std::string name = zmq_recv_string(python_socket.pipe_from_python);
    auto & load_list = worker.create_list<SparseRowObject>(name);
    long n_samples = std::stol(zmq_recv_string(python_socket.pipe_from_python));
    int n_features = std::stoi(zmq_recv_string(python_socket.pipe_from_python));
    long local_row_cnt = 0;
    Aggregator<long> num_samples_agg(0, [](long & a, const long & b) {
        a += b;
    });
    for (int i = 0; i < n_samples; i++) {
        SparseRowObject Y_row;
        std::string key = std::to_string(worker.id) + "_" + std::to_string(i);
        Y_row.key_init(key);
        SpMat sparse_row(1, n_features);
        int row_fea_num = std::stoi(zmq_recv_string(python_socket.pipe_from_python));
        for (int j = 0; j < row_fea_num; j++) {
            int fea_index = std::stoi(zmq_recv_string(python_socket.pipe_from_python));
            double fea_val = std::stod(zmq_recv_string(python_socket.pipe_from_python));
            sparse_row.coeffRef(0, fea_index-1) = fea_val;
        }
        Y_row.row_init(sparse_row);
        worker.add_object(load_list, Y_row);
        local_row_cnt++;
    }
    worker.globalize_list(load_list);
    num_samples_agg.update(local_row_cnt);
    worker.list_execute(load_list, [&](SparseRowObject & m) {});
    n_samples = num_samples_agg.get_value();
    local_SPCA_models[name].set_n_samples(n_samples);
    local_SPCA_models[name].set_n_features(n_features);
    local_SPCA_models[name].set_loaded();
}

void PyHuskyPCA::spca_init_handler(const Operation & op,
        PythonSocket & python_socket,
        ITCWorker & daemon_socket) {
    auto & worker = Context::get_worker<BaseWorker>();
    // Get Parameters sent from python
    const std::string & name = op.get_param("list_name");
    const int n_components = std::stoi(op.get_param("n_components"));
    // Create SPCA model and store at local
    assert(n_components > 0);

    local_SPCA_models[name] = ML::SPCAModel(name, n_components);
    log_msg("create SPCA Model: " + name);
}

void PyHuskyPCA::spca_load_hdfs_handler(const Operation & op,
        PythonSocket & python_socket,
        ITCWorker & daemon_socket) {
    auto & worker = Context::get_worker<BaseWorker>();
    // Get Parameters sent from python
    const std::string & url = op.get_param("url");
    const std::string & name = op.get_param("list_name");

    local_SPCA_models[name].load_info_libsvm(url);

    long n_samples = local_SPCA_models[name].get_n_samples();
    int n_features = local_SPCA_models[name].get_n_features();
    assert(n_samples > 0);
    assert(n_features > 0);
}

void PyHuskyPCA::spca_train_handler(const Operation & op,
        PythonSocket & python_socket,
        ITCWorker & daemon_socket) {
    auto & worker = Context::get_worker<BaseWorker>();
    const std::string & name = op.get_param("list_name");
    const int max_iteration = std::stoi(op.get_param("max_iteration"));

    local_SPCA_models[name].train(max_iteration);

    // Send Back the parameter to pyHusky
    if (worker.id == 0) {
        BinStream result;
        std::string final_C = local_SPCA_models[name].get_C();
        result << final_C;
        daemon_socket.sendmore("SPCA#spca_train");
        daemon_socket.send(std::move(result));
    }
}

void PyHuskyPCA::daemon_train_handler(ITCDaemon & to_worker, BinStream & buffer) {
    BinStream recv = to_worker.recv_binstream();
    int flag = 1;  // 1 means sent by cpp
    buffer << flag << recv.to_string();
}

}  // end of namespace husky
