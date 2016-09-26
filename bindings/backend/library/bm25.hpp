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

#include <map>
#include <string>

#include "lib/bm25.hpp"

namespace husky {
class PythonConnector;
class ITCWorker;
class ITCDaemon;
class PythonSocket;
class ThreadConnector;
class BinStream;
class Operation;

class PyHuskyBM25 {
  public:
    static void init_py_handlers();
    static void init_cpp_handlers();
    static void init_daemon_handlers();
  protected:
    static void BM25_load_pyhlist_handler(PythonSocket & python_socket,
            ITCWorker & daemon_socket);
    static void BM25_handler(const Operation & op,
            PythonSocket & python_socket,
            ITCWorker & daemon_socket);
    static void BM25_topk_handler(const Operation & op,
            PythonSocket & python_socket,
            ITCWorker & daemon_socket);
    static void BM25_get_score_handler(const Operation & op,
            PythonSocket & python_socket,
            ITCWorker & daemon_socket);
    static void daemon_BM25_topk_handler(ITCDaemon&, BinStream&);
    static void daemon_BM25_get_score_handler(ITCDaemon&, BinStream&);
    static thread_local std::map<std::string, ML::BM25> local_BM25_model;
};
}  // namespace husky
