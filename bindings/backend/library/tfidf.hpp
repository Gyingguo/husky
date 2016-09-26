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

#pragma once

#include <map>
#include <string>

#include "lib/tfidf.hpp"

namespace husky {

class ITCWorker;
class ITCDaemon;
class PythonSocket;
class BinStream;
class Operation;

class PyHuskyTFIDF {
public:
    static void init_py_handlers();
    static void init_cpp_handlers();
    static void init_daemon_handlers();

protected:
    // python_handlers
    static void TFIDF_to_documents_handler(PythonSocket & python_socket,
            ITCWorker & daemon_socket);

    // cpp handlers
    static void TFIDF_load_mongodb_handler(const Operation & op,
            PythonSocket & python_socket,
            ITCWorker & daemon_socket);
    static void TFIDF_calc_handler(const Operation & op,
            PythonSocket & python_socket,
            ITCWorker & daemon_socket);
    static void TFIDF_get_TFIDF_value_handler(const Operation & op,
            PythonSocket & python_socket,
            ITCWorker & daemon_socket);
    static void TFIDF_del_handler(const Operation & op,
            PythonSocket & python_socket,
            ITCWorker & daemon_socket);

    // daemon handlers
    static void daemon_get_TFIDF_value_handler(ITCDaemon&, BinStream&);

    // A thread_local wordmodel_map
    static thread_local std::map<std::string, ML::Tfidf> local_tfidfs;
};  // class PyHuskyTFIDF

}  // namespace husky
