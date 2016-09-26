from bindings.frontend.huskylist import HuskyList
from bindings.frontend.huskylist import PyHuskyList
from bindings.frontend.operation import Operation, OperationParam
from bindings.frontend import scheduler

class Graph(HuskyList):
    def __init__(self):
        super(Graph, self).__init__()
        self.list_name += "PRVertex"
        self._loaded = False
        self._computed = False

    def load_edgelist_phlist(self, edgelist):
        assert self._loaded == False, "The graph was already loaded"
        assert isinstance(edgelist, PyHuskyList)
        param = {OperationParam.list_str : self.list_name}
        op = Operation("Graph#load_edgelist_phlist_py", param, [edgelist.pending_op])
        scheduler.compute(op)
        self._loaded = True

    # def load_adjlist_hdfs(self, url):
    #     assert type(url) is str
    #     assert self._loaded == False, "The graph was already loaded"
    #     param = {OperationParam.list_str : self.list_name,
    #             "url" : url,
    #             "Type" : "cpp"}
    #     op = Operation("Graph#load_adjlist_hdfs_py", param, []);
    #     scheduler.compute(op)
    #     self._loaded = True

    def compute(self, iter):
        assert self._loaded == True, "The graph is not loaded"
        param = {"iter" : str(iter), 
                 OperationParam.list_str : self.list_name,
                 "Type" : "cpp"}
        op = Operation("Graph#pagerank_py", param, [])
        scheduler.compute(op)
        self._computed = True

    def topk(self, k):
        assert self._computed == True, "You haven't computed Pagerank"
        param = {"k" : str(k), 
                 OperationParam.list_str : self.list_name,
                 "Type" : "cpp"}
        op = Operation("Graph#pagerank_topk_py", param, [])
        topk_list = scheduler.compute_collect(op)
        return topk_list

