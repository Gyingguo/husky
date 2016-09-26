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

#include "bindings/backend/library/register.hpp"

// #include "bindings/backend/library/bm25.hpp"
#include "bindings/backend/library/functional.hpp"
// #include "bindings/backend/library/gradient_descent.hpp"
// #include "bindings/backend/library/graph.hpp"
// #include "bindings/backend/library/spca.hpp"
// #include "bindings/backend/library/tfidf.hpp"
// #include "bindings/backend/library/word.hpp"

namespace husky {
void RegisterFunction::register_py_handlers() {
    PyHuskyFunctional::init_py_handlers();
    // PyHuskyGraph::init_py_handlers();
    // PyHuskyWord::init_py_handlers();
    // PyHuskyML::init_py_handlers();
    // PyHuskyPCA::init_py_handlers();
    // PyHuskyTFIDF::init_py_handlers();
    // PyHuskyBM25::init_py_handlers();
}

void RegisterFunction::register_cpp_handlers() {
    // PyHuskyGraph::init_cpp_handlers();
    // PyHuskyWord::init_cpp_handlers();
    // PyHuskyML::init_cpp_handlers();
    // PyHuskyPCA::init_cpp_handlers();
    // PyHuskyTFIDF::init_cpp_handlers();
    // PyHuskyBM25::init_cpp_handlers();
}

void RegisterFunction::register_daemon_handlers() {
    PyHuskyFunctional::init_daemon_handlers();
    // PyHuskyGraph::init_daemon_handlers();
    // PyHuskyWord::init_daemon_handlers();
    // PyHuskyML::init_daemon_handlers();
    // PyHuskyPCA::init_daemon_handlers();
    // PyHuskyTFIDF::init_daemon_handlers();
    // PyHuskyBM25::init_daemon_handlers();
}

}  // namespace husky
