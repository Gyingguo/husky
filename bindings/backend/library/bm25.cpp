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
//

#include "bindings/backend/library/bm25.hpp"

#include <map>
#include <string>
#include <utility>
#include <vector>

#include "boost/tokenizer.hpp"

#include "bindings/backend/pythonconnector.hpp"
#include "bindings/backend/threadconnector.hpp"
#include "bindings/backend/workerdriver.hpp"
#include "bindings/itc.hpp"
#include "bindings/operation.hpp"
#include "core/baseworker.hpp"
#include "core/context.hpp"
#include "core/workerdevapi.hpp"
#include "core/zmq_helpers.hpp"
#include "io/inputformat/lineinputformat.hpp"
#include "lib/topk.hpp"


namespace husky {

thread_local std::map<std::string, ML::BM25> PyHuskyBM25::local_BM25_model;

void PyHuskyBM25::init_py_handlers() {
    PythonConnector::add_handler("BM25#load_pyhlist_py", BM25_load_pyhlist_handler);
}
void PyHuskyBM25::init_cpp_handlers() {
    WorkerDriver::add_handler("BM25#compute_py", BM25_handler);
    WorkerDriver::add_handler("BM25#topk_py", BM25_topk_handler);
    WorkerDriver::add_handler("BM25#get_score_py", BM25_get_score_handler);
}

void PyHuskyBM25::init_daemon_handlers() {
    ThreadConnector::add_handler("BM25#topk", daemon_BM25_topk_handler);
    ThreadConnector::add_handler("BM25#get_score", daemon_BM25_get_score_handler);
}

void PyHuskyBM25::BM25_load_pyhlist_handler(PythonSocket & python_socket,
                     ITCWorker & daemon_socket) {
    std::string name = zmq_recv_string(python_socket.pipe_from_python);
    int num = std::stoi(zmq_recv_string(python_socket.pipe_from_python));
    auto & worker = Context::get_worker<BaseWorker>();
    auto & doc_list = worker.create_list<Husky::ML::DocObject>("doc_list_" + name);
    ML::BM25 model(name);
    local_BM25_model.insert(std::pair<std::string, ML::BM25>(name, model));
    for (int i = 0; i < num; i++) {
        Husky::ML::DocObject doc;
        doc.key = std::stoi(zmq_recv_string(python_socket.pipe_from_python));
        doc.content = zmq_recv_string(python_socket.pipe_from_python);
        worker.add_object(doc_list, doc);
    }
    local_BM25_model.at(name).set_doc_list(doc_list);
}

void PyHuskyBM25::BM25_handler(const Operation & op,
        PythonSocket & python_socket,
        ITCWorker & daemon_socket) {
    const std::string & name = op.get_param("list_name");
    const std::string queries = op.get_param("query");
    double TFS_ = std::stod(op.get_param("TFS"));
    double FLN_ = std::stod(op.get_param("FLN"));
    local_BM25_model.at(name).set_TFS(TFS_);
    local_BM25_model.at(name).set_FLN(FLN_);
    local_BM25_model.at(name).cal(queries);
}
void PyHuskyBM25::BM25_topk_handler(const Operation & op,
        PythonSocket & python_socket,
        ITCWorker & daemon_socket) {
    const std::string & name = op.get_param("list_name");
    const int k = stoi(op.get_param("k"));
    auto & worker = Context::get_worker<BaseWorker>();
    std::vector<std::pair<double, int>> topk = local_BM25_model.at(name).topx(k);
    if (worker.id == 0) {
        BinStream result;
        int rank = 1;
        result << static_cast<int>(topk.size());
        for (const auto & elem : topk) {
            result << rank << elem.second << elem.first;
            rank++;
        }
        daemon_socket.sendmore("BM25#topk");
        daemon_socket.send(std::move(result));
    }
}
void PyHuskyBM25::BM25_get_score_handler(const Operation & op,
        PythonSocket & python_socket,
        ITCWorker & daemon_socket) {
    const std::string & name = op.get_param("list_name");
    auto & worker = Context::get_worker<BaseWorker>();
    int key = stoi(op.get_param("id"));
    double score = local_BM25_model.at(name).get_score(key);
    if (worker.id == 0) {
        BinStream result;
        result << key << score;
        daemon_socket.sendmore("BM25#get_score");
        daemon_socket.send(std::move(result));
    }
}
void PyHuskyBM25::daemon_BM25_topk_handler(ITCDaemon & to_worker,
        BinStream & buffer) {
    BinStream recv = to_worker.recv_binstream();
    int flag = 1;
    buffer << flag << recv.to_string();
}
void PyHuskyBM25::daemon_BM25_get_score_handler(ITCDaemon & to_worker,
        BinStream & buffer) {
    BinStream recv = to_worker.recv_binstream();
    int flag = 1;
    buffer << flag << recv.to_string();
}
}  // namespace husky
