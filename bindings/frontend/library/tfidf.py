from bindings.frontend.huskylist import HuskyList
from bindings.frontend.huskylist import PyHuskyList
from bindings.frontend.operation import Operation, OperationParam
from bindings.frontend import scheduler

class TFIDF(HuskyList):
    def __init__(self):
        super(TFIDF, self).__init__()
        self.list_name += "TFIDF"

    def load_phlist(self, doclist):
        assert isinstance(doclist, PyHuskyList)
        param = {OperationParam.list_str : self.list_name}
        op = Operation("TFIDF#TFIDF_load_phlist_py", param, [doclist.pending_op])
        scheduler.compute(op)
        self._loaded = True

    def load_mongodb(self, server, db, collection, user, pwd):
        param = {"server" : server,
                 "db" : db,
                 "collection" : collection,
                 "user" : user,
                 "pwd" : pwd,
                 OperationParam.list_str : self.list_name,
                 "Type" : "cpp"}
        op = Operation("TFIDF#TFIDF_load_mongodb_py", param, [])
        self._loaded = True
        scheduler.compute(op)

    def calc(self):
        assert self._loaded is True
        param = {OperationParam.list_str : self.list_name,
                 "Type" : "cpp"}
        op = Operation("TFIDF#TFIDF_calc_py", param, [])
        scheduler.compute(op)

    def get_TFIDF_value(self, document, word):
        param = {"document" : document,
                 "word" : word,
                 OperationParam.list_str : self.list_name,
                 "Type" : "cpp"}
        op = Operation("TFIDF#TFIDF_get_TFIDF_value_py", param, [])
        value = scheduler.compute_collect(op)
        return value

    def __del__(self):
        param = {OperationParam.list_str : self.list_name,
                "Type" : "cpp"}
        op = Operation("TFIDF#TFIDF_del_py", param, []) 
        return scheduler.compute_collect(op)
