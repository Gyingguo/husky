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

#include "bindings/backend/library/tfidf.hpp"

#include <algorithm>
#include <map>
#include <string>
#include <vector>

#include "boost/tokenizer.hpp"

#include "bindings/backend/pythonconnector.hpp"
#include "bindings/backend/threadconnector.hpp"
#include "bindings/backend/workerdriver.hpp"
#include "bindings/itc.hpp"
#include "bindings/operation.hpp"
#include "core/baseobject.hpp"
#include "core/baseworker.hpp"
#include "core/context.hpp"
#include "core/objlist.hpp"
#include "core/workerdevapi.hpp"
#include "core/zmq_helpers.hpp"
#include "lib/dcaggregator.hpp"

namespace husky {

thread_local std::map<std::string, Husky::ML::Tfidf> PyHuskyTFIDF::local_tfidfs;
void PyHuskyTFIDF::init_py_handlers() {
    PythonConnector::add_handler("TFIDF#TFIDF_load_phlist_py", TFIDF_to_documents_handler);
}

void PyHuskyTFIDF::init_cpp_handlers() {
#ifdef WITH_MONGODB
    WorkerDriver::add_handler("TFIDF#TFIDF_load_mongodb_py", TFIDF_load_mongodb_handler);
#endif
    WorkerDriver::add_handler("TFIDF#TFIDF_calc_py", TFIDF_calc_handler);
    WorkerDriver::add_handler("TFIDF#TFIDF_get_TFIDF_value_py", TFIDF_get_TFIDF_value_handler);
    WorkerDriver::add_handler("TFIDF#TFIDF_del_py", TFIDF_del_handler);
}

void PyHuskyTFIDF::init_daemon_handlers() {
    ThreadConnector::add_handler("TFIDF#TFIDF_get_TFIDF_value", daemon_get_TFIDF_value_handler);
}

void PyHuskyTFIDF::TFIDF_del_handler(const Operation & op,
        PythonSocket & python_socket,
        ITCWorker & daemon_socket) {
    const std::string & name = op.get_param("list_name");
    local_tfidfs.erase(name);
}

#ifdef WITH_MONGODB
void PyHuskyTFIDF::TFIDF_load_mongodb_handler(const Operation & op,
        PythonSocket & python_socket,
        ITCWorker & daemon_socket) {
    const std::string & name = op.get_param("list_name");
    const std::string & server = op.get_param("server");
    const std::string & db = op.get_param("db");
    const std::string & collection = op.get_param("collection");
    const std::string & user = op.get_param("user");
    const std::string & pwd = op.get_param("pwd");
    local_tfidfs[name] = ML::Tfidf(name);
    local_tfidfs[name].load_mongodb(server, db, collection, user, pwd);
}
#endif

void PyHuskyTFIDF::TFIDF_to_documents_handler(PythonSocket & python_socket, ITCWorker & daemon_socket) {
    auto & worker = Context::get_worker<BaseWorker>();
    std::string name = zmq_recv_string(python_socket.pipe_from_python);
    int num = std::stoi(zmq_recv_string(python_socket.pipe_from_python));
    Name list_name = Name(name);
    auto & document_list = worker.create_list<Husky::ML::Document>(name);
    for (int i = 0; i < num; i++) {
        Husky::ML::Document doc;
        doc.title = zmq_recv_string(python_socket.pipe_from_python);
        doc.content = zmq_recv_string(python_socket.pipe_from_python);
        worker.add_object(document_list, doc);
    }
    local_tfidfs[name] = ML::Tfidf(name);
    local_tfidfs[name].use_list(name);
}

void PyHuskyTFIDF::TFIDF_calc_handler(const Operation & op,
        PythonSocket & python_socket,
        ITCWorker & daemon_socket) {
    // Please make sure that every document in the document_list does not have a vector naming words
    // or the vector does not contain anything useful.
    const std::string & name = op.get_param("list_name");
    local_tfidfs[name].calc();
}

int BinarySearch(std::vector<std::string> words, std::string value) {
    int low = 0;
    int high = words.size() - 1;
    int mid = 0;
    if (high < 0)
        return -1;
    while ( low <= high ) {
        mid = (low + high)/2;
        if (words[mid] > value)
            high = mid - 1;
        else if (words[mid] < value)
           low = mid + 1;
        else
            return mid;
    }
    return -1;
}

void PyHuskyTFIDF::TFIDF_get_TFIDF_value_handler(const Operation & op,
        PythonSocket & python_socket,
        ITCWorker & daemon_socket) {
    Husky::BaseWorker& worker = Husky::Context::get_worker<Husky::BaseWorker>();
    const std::string & name = op.get_param("list_name");
    auto & document_list = worker.create_list<Husky::ML::Document>(name);
    const std::string & document = op.get_param("document");
    const std::string & originalWord = op.get_param("word");
    std::string word;
    word.assign(originalWord);
    std::transform(word.begin(), word.end(), word.begin(), ::tolower);
    Husky::Aggregator<int> givenDoc(0, [](int& a, const int& b) { a += b; });
    worker.list_execute(document_list, [&](Husky::ML::Document& doc) {
        if (doc.title.compare(document) == 0 && doc.words.size() > 0){
            givenDoc.update(1);
            BinStream result;
            int index = BinarySearch(doc.words, word);
            if (index != -1) {
                result << doc.tf_idf.at(index);
            } else {
                 result << static_cast<double>(-1);
            }
            daemon_socket.sendmore("TFIDF#TFIDF_get_TFIDF_value");
            daemon_socket.send(std::move(result));
        }
    });
    if (worker.id == 0 && givenDoc.get_value() == 0) {
        BinStream result;
        result << static_cast<double>(-2);
        daemon_socket.sendmore("TFIDF#TFIDF_get_TFIDF_value");
        daemon_socket.send(std::move(result));
    }
}
void PyHuskyTFIDF::daemon_get_TFIDF_value_handler(ITCDaemon & to_worker, BinStream & buffer) {
    BinStream recv = to_worker.recv_binstream();
    int flag = 1;  // 1 means sent by cpp
    buffer << flag << recv.to_string();
}
}  // namespace husky
