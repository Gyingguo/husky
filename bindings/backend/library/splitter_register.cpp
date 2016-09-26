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

#include "bindings/backend/library/splitter_register.hpp"

namespace husky {

void splitter_register() {
    // Functional
    OperationSplitter::add_splitter("Functional#reduce_by_key_py", OperationSplitter::simple_split);
    OperationSplitter::add_splitter("Functional#group_by_key_py", OperationSplitter::simple_split);
    OperationSplitter::add_splitter("Functional#distinct_py", OperationSplitter::simple_split);
    OperationSplitter::add_splitter("Functional#parallelize_py", OperationSplitter::load);
    OperationSplitter::add_splitter("Functional#load_py", OperationSplitter::load);
    OperationSplitter::add_splitter("Functional#uncache_py", OperationSplitter::load);
    OperationSplitter::add_splitter("Functional#load_cache_py", OperationSplitter::load);
    OperationSplitter::add_splitter("Functional#count_py", OperationSplitter::simple_split);
    OperationSplitter::add_splitter("Functional#collect_py", OperationSplitter::simple_split);
    OperationSplitter::add_splitter("Functional#reduce_py", OperationSplitter::simple_split);
    OperationSplitter::add_splitter("Functional#difference_py", OperationSplitter::difference);

    // Graph
    OperationSplitter::add_splitter("Graph#load_edgelist_phlist_py", OperationSplitter::simple_split);
    OperationSplitter::add_splitter("Graph#pagerank_py", OperationSplitter::load);
    OperationSplitter::add_splitter("Graph#pagerank_topk_py", OperationSplitter::load);

    // Word
    OperationSplitter::add_splitter("Word#load_phlist_py", OperationSplitter::simple_split);
    OperationSplitter::add_splitter("Word#load_hdfs_py", OperationSplitter::load);
    OperationSplitter::add_splitter("Word#wordcount_py", OperationSplitter::load);
    OperationSplitter::add_splitter("Word#wordcount_topk_py", OperationSplitter::load);
    OperationSplitter::add_splitter("Word#wordcount_print_py", OperationSplitter::load);
    OperationSplitter::add_splitter("Word#del_py", OperationSplitter::load);

    // Linear Regression
    OperationSplitter::add_splitter("LinearRegressionModel#LinearR_init_py", OperationSplitter::load);
    OperationSplitter::add_splitter("LinearRegressionModel#LinearR_load_hdfs_py", OperationSplitter::load);
    OperationSplitter::add_splitter("LinearRegressionModel#LinearR_load_pyhlist_py", OperationSplitter::simple_split);
    OperationSplitter::add_splitter("LinearRegressionModel#LinearR_train_py", OperationSplitter::load);

    // SPCA
    OperationSplitter::add_splitter("SPCA#spca_init_py", OperationSplitter::load);
    OperationSplitter::add_splitter("SPCA#spca_load_hdfs_py", OperationSplitter::load);
    OperationSplitter::add_splitter("SPCA#spca_load_pyhlist_py", OperationSplitter::simple_split);
    OperationSplitter::add_splitter("SPCA#spca_train_py", OperationSplitter::load);
    OperationSplitter::add_splitter("SPCA#del_py", OperationSplitter::load);
   
    // BM25
    OperationSplitter::add_splitter("BM25#load_pyhlist_py", OperationSplitter::simple_split);
    OperationSplitter::add_splitter("BM25#compute_py", OperationSplitter::load);
    OperationSplitter::add_splitter("BM25#topk_py", OperationSplitter::load);
    OperationSplitter::add_splitter("BM25#get_score_py", OperationSplitter::load);

    // TF-IDF
    OperationSplitter::add_splitter("TFIDF#TFIDF_load_mongodb_py", OperationSplitter::load);
    OperationSplitter::add_splitter("TFIDF#TFIDF_load_phlist_py", OperationSplitter::simple_split);
    OperationSplitter::add_splitter("TFIDF#TFIDF_calc_py", OperationSplitter::load);
    OperationSplitter::add_splitter("TFIDF#TFIDF_get_TFIDF_value_py", OperationSplitter::load);
    OperationSplitter::add_splitter("TFIDF#TFIDF_del_py", OperationSplitter::load);
}
}  // namespace husky
