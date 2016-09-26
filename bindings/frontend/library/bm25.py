from bindings.frontend.huskylist import HuskyList
from bindings.frontend.huskylist import PyHuskyList
from bindings.frontend.operation import Operation, OperationParam
from bindings.frontend import scheduler

class BM25(HuskyList):
    def __init__(self):
        super(BM25, self).__init__()
        self.list_name += "BM25"
        self._loaded = False
        self._computed = False
    
    def load_pyhlist(self, doc_list):
        assert self._loaded == False, "The document collection was already loaded"
        assert isinstance(doc_list, PyHuskyList)
        param = {OperationParam.list_str : self.list_name}
        op = Operation("BM25#load_pyhlist_py", param, [doc_list.pending_op])
        scheduler.compute(op)
        self.loaded = True

    def compute(self, query, FLN=0.75, TFS=1.2):
        assert type(query) is str
        assert type(FLN) is float
        assert type(TFS) is float
        param = {"query" : query,
                 "FLN" : str(FLN),
                 "TFS" : str(TFS),
                 OperationParam.list_str : self.list_name,
                 "Type" : "cpp"}
        op = Operation("BM25#compute_py", param, []);
        scheduler.compute(op)
        self._computed = True

    def get_score(self, id):
        assert type(id) is int
        assert self._computed == True, "You haven't computed BM25"
        param = {"id" : str(id),
                 OperationParam.list_str : self.list_name,
                 "Type" : "cpp"}
        op = Operation("BM25#get_score_py", param, []);
        score = scheduler.compute_collect(op)
        return score

    def topk(self, k):
        assert type(k) is int
        assert self._computed == True, "You haven't computed BM25"
        param = {"k" : str(k),
                 OperationParam.list_str : self.list_name,
                 "Type" : "cpp"}
        op = Operation("BM25#topk_py", param, [])
        topk_list = scheduler.compute_collect(op)
        return topk_list
