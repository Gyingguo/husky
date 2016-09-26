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

#include "bindings/backend/library/graph.hpp"

#include "bindings/backend/pythonconnector.hpp"
#include "bindings/backend/threadconnector.hpp"
#include "bindings/backend/workerdriver.hpp"
#include "bindings/operation.hpp"
#include "bindings/itc.hpp"
#include "core/baseworker.hpp"
#include "core/context.hpp"
#include "core/workerdevapi.hpp"
#include "core/zmq_helpers.hpp"
#include "lib/pagerank.hpp"


namespace husky {

void PyHuskyGraph::init_py_handlers() {
    PythonConnector::add_handler("Graph#load_edgelist_phlist_py", load_edgelist_phlist_handler);
}

void PyHuskyGraph::init_cpp_handlers() {
    WorkerDriver::add_handler("Graph#pagerank_py", pagerank_handler);
    WorkerDriver::add_handler("Graph#pagerank_topk_py", pagerank_topk_handler);
}

void PyHuskyGraph::init_daemon_handlers() {
    ThreadConnector::add_handler("Graph#topk_cpp", daemon_topk_handler);
}

void PyHuskyGraph::load_edgelist_phlist_handler(PythonSocket & python_socket, ITCWorker & daemon_socket) {
    auto & worker = Context::get_worker<BaseWorker>();
    auto & buffermanager = worker.get_buffermanager();
    auto & mailbox = Context::get_mailbox();
    std::string name = zmq_recv_string(python_socket.pipe_from_python);
    Name list_name = Name(name);
    auto & obj_list = worker.create_list<Graph::PRVertex>(name);
    int num = std::stoi(zmq_recv_string(python_socket.pipe_from_python));
    auto & graph_list = worker.create_list<Graph::PRVertex>(name, false);
    debug_msg("frome_edgelist: list_name: " + name);
    std::vector<BinStream>* send_buffer_ptr = &buffermanager.get_buffer(list_name, COMM_PUSH);
    for (int i = 0; i < num; ++i) {
        int src = std::stoi(zmq_recv_string(python_socket.pipe_from_python));
        int dst = std::stoi(zmq_recv_string(python_socket.pipe_from_python));
        int dst_worker_id = WorkerDevApi::hash_lookup<Graph::PRVertex>(worker, src, worker.get_hashring());
        (*send_buffer_ptr)[dst_worker_id] << src << dst;
    }
    while (mailbox.poll(worker.id)) std::this_thread::yield();
    mailbox.recv_complete(worker.id);

    WorkerDevApi::flush_buffer(worker, obj_list, COMM_PUSH);
    mailbox.send_complete(worker.id);
    while (mailbox.poll(worker.id)) std::this_thread::yield();

    // invoke libraray function
    Graph::from_edgelist(graph_list);
}
void PyHuskyGraph::pagerank_handler(const Operation & op, PythonSocket & python_socket, ITCWorker & daemon_socket) {
    const std::string & name = op.get_param("list_name");
    const int num_iter = stoi(op.get_param("iter"));
    auto & worker = Context::get_worker<BaseWorker>();
    auto & graph_list = worker.create_list<Graph::PRVertex>(name, false);
    debug_msg("pagerank:list_name: " + name);
    // pagerank
    worker.list_execute(graph_list, Graph::exec_pagerank(graph_list), num_iter);
    // Graph::print_pr(graph_list);
    // Graph::print_neighbors(graph_list);
}
void PyHuskyGraph::pagerank_topk_handler(const Operation & op, PythonSocket & python_socket, ITCWorker & daemon_socket) {
    const std::string & name = op.get_param("list_name");
    const int k = stoi(op.get_param("k"));
    auto & worker = Context::get_worker<BaseWorker>();
    auto & graph_list = worker.create_list<Graph::PRVertex>(name, false);
    debug_msg("pagerank-topk:list_name: " + name + " k: "+std::to_string(k));
    // topk
    worker.list_execute(graph_list, [](Graph::PRVertex & v){ get_messages<float>(v); });  // consume messages
    std::vector<std::pair<float, int>> topk = Graph::topk(graph_list, k);
    if (worker.id == 0) {
        BinStream result;
        result << static_cast<int>(topk.size());
        int rank = 1;
        for (const auto elem : topk) {
            debug_msg("rank: "+std::to_string(rank++)+"\t pr: "+std::to_string(elem.first)+"\t id: "+std::to_string(elem.second));
            result << elem.second << elem.first;
        }
        daemon_socket.sendmore("Graph#topk_cpp");
        daemon_socket.send(std::move(result));
    }
    // Graph::print_pr(graph_list);
    // Graph::print_neighbors(graph_list);
}
void PyHuskyGraph::daemon_topk_handler(ITCDaemon & to_worker, BinStream & buffer) {
    BinStream recv = to_worker.recv_binstream();
    int flag = 1;  // 1 means sent by cpp
    buffer << flag << recv.to_string();
}

}  // namespace husky
