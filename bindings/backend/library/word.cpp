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

#include "bindings/backend/library/word.hpp"

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
#include "lib/word.hpp"


namespace husky {

thread_local std::map<std::string, Husky::WordModel> PyHuskyWord::local_wordmodels;

void PyHuskyWord::init_py_handlers() {
    PythonConnector::add_handler("Word#load_phlist_py", to_words_handler);
}

void PyHuskyWord::init_cpp_handlers() {
#ifdef WITH_HDFS
    WorkerDriver::add_handler("Word#load_hdfs_py", load_hdfs_handler);
#endif
    WorkerDriver::add_handler("Word#wordcount_py", wordcount_handler);
    WorkerDriver::add_handler("Word#wordcount_topk_py", wordcount_topk_handler);
    WorkerDriver::add_handler("Word#wordcount_print_py", wordcount_print_handler);
    WorkerDriver::add_handler("Word#del_py", del_handler);
}

void PyHuskyWord::init_daemon_handlers() {
    ThreadConnector::add_handler("Word#wordcount_topk", daemon_wordcount_topk_handler);
    ThreadConnector::add_handler("Word#wordcount_print", daemon_wordcount_print_handler);
}

void PyHuskyWord::del_handler(const Operation & op, PythonSocket & python_socket, ITCWorker & daemon_socket) {
    const std::string & name = op.get_param("list_name");
    local_wordmodels.erase(name);
}
#ifdef WITH_HDFS
void PyHuskyWord::load_hdfs_handler(const Operation & op, PythonSocket & python_socket, ITCWorker & daemon_socket) {
    const std::string & name = op.get_param("list_name");
    const std::string & url = op.get_param("url");
    // create word_model
    local_wordmodels[name] = WordModel(name);
    local_wordmodels[name].load_hdfs(url);
}
#endif

void PyHuskyWord::to_words_handler(PythonSocket & python_socket, ITCWorker & daemon_socket) {
    // Receive words from Python, and store them in C++
    auto & worker = Context::get_worker<BaseWorker>();
    std::string name = zmq_recv_string(python_socket.pipe_from_python);
    int num = std::stoi(zmq_recv_string(python_socket.pipe_from_python));
    Name list_name = Name(name);
    auto & word_list = worker.create_list<Husky::Word>(name);
    for (int i = 0; i < num; ++i) {
        Word w;
        w.word = zmq_recv_string(python_socket.pipe_from_python);
        worker.add_object(word_list, w);
    }
    // create word_model
    local_wordmodels[name] = WordModel(name);
    local_wordmodels[name].use_list(name);
}

void PyHuskyWord::wordcount_handler(const Operation & op, PythonSocket & python_socket, ITCWorker & daemon_wocket) {
    const std::string & name = op.get_param("list_name");
    // caculate wordcount
    local_wordmodels[name].wordcount();
}
void PyHuskyWord::wordcount_print_handler(const Operation & op, PythonSocket & python_socket, ITCWorker & daemon_socket) {
    const std::string & name = op.get_param("list_name");
    auto & worker = Context::get_worker<BaseWorker>();
    auto & wordcount_list = worker.create_list<Husky::WCObject>(name+"_wc");
    // return all the wordcount
    BinStream result;
    int k = worker.list_size(wordcount_list);
    result << k;
    worker.list_execute(wordcount_list, [&](WCObject & w){
        result << w.word << w.num_occur;
    });
    daemon_socket.sendmore("Word#wordcount_print");
    daemon_socket.send(std::move(result));
}
void PyHuskyWord::wordcount_topk_handler(const Operation & op, PythonSocket & python_socket, ITCWorker & daemon_socket) {
    const std::string & name = op.get_param("list_name");
    const int k = stoi(op.get_param("k"));
    auto & worker = Context::get_worker<BaseWorker>();
    // word_model topk function
    auto topk = local_wordmodels[name].topk(k);
    if (worker.id == 0) {
        BinStream result;
        result << static_cast<int>(topk.size());
        for (const auto & elem : topk) {
            result << elem.second << elem.first;
        }
        daemon_socket.sendmore("Word#wordcount_topk");
        daemon_socket.send(std::move(result));
    }
}

void PyHuskyWord::daemon_wordcount_topk_handler(ITCDaemon & to_worker, BinStream & buffer) {
    BinStream recv = to_worker.recv_binstream();
    int flag = 1;  // 1 means sent by cpp
    buffer << flag << recv.to_string();
}
void PyHuskyWord::daemon_wordcount_print_handler(ITCDaemon & to_worker, BinStream & buffer) {
    BinStream recv = to_worker.recv_binstream();
    int flag = 1;  // 1 means sent by cpp
    buffer << flag << recv.to_string();
}

}  // namespace husky
