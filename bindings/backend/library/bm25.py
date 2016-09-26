import json
import cPickle
from bindings.backend.python.globalvar import GlobalVar, GlobalSocket, OperationParam

def register_all():
    BM25Model.register()

class BM25Model:
    @staticmethod
    def register():
        GlobalVar.name_to_func["BM25#load_pyhlist_py"] = BM25Model.func
        GlobalVar.name_to_prefunc["BM25#load_pyhlist_py"] = BM25Model.prefunc
        GlobalVar.name_to_func["BM25#load_pyhlist_end_py"] = None
        GlobalVar.name_to_postfunc["BM25#load_pyhlist_end_py"] = BM25Model.end_postfunc

        GlobalVar.name_to_type["BM25#load_pyhlist_py"] = GlobalVar.librarytype
        GlobalVar.name_to_type["BM25#load_pyhlist_end_py"] = GlobalVar.librarytype

    @staticmethod
    def func(op, data):
        for x in data:
            GlobalVar.doc_store.append(x)

    @staticmethod
    def prefunc(op):
        GlobalVar.doc_list = op.op_param[OperationParam.list_str]
        GlobalVar.doc_store = []

    @staticmethod
    def end_postfunc(op):
        GlobalSocket.send("BM25#load_pyhlist_py")
        GlobalSocket.send(GlobalVar.doc_list)
        GlobalSocket.send(str(len(GlobalVar.doc_store)))
        for x in GlobalVar.doc_store:
            GlobalSocket.send(str(x[0].encode('utf-8')))
            GlobalSocket.send(str(x[1].encode('utf-8')))
        GlobalVar.doc_store = []
