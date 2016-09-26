# -*- coding: utf-8 -*-
from bindings.backend.python.globalvar import GlobalVar, GlobalSocket, OperationParam

def register_all():
    # register documents 
    ToDocuments.register()

class ToDocuments:
    @staticmethod
    def register():
        GlobalVar.name_to_func["TFIDF#TFIDF_load_phlist_py"] = ToDocuments.func
        GlobalVar.name_to_prefunc["TFIDF#TFIDF_load_phlist_py"] = ToDocuments.prefunc

        GlobalVar.name_to_postfunc["TFIDF#TFIDF_load_phlist_end_py"] = ToDocuments.end_postfunc

        GlobalVar.name_to_type["TFIDF#TFIDF_load_phlist_py"] = GlobalVar.librarytype
        GlobalVar.name_to_type["TFIDF#TFIDF_load_phlist_end_py"] = GlobalVar.librarytype 

    @staticmethod
    def func(op, data):
        for x in data:
            GlobalVar.document_store.append(x)
    @staticmethod
    def prefunc(op):
        GlobalVar.document_list = op.op_param[OperationParam.list_str]
        GlobalVar.document_store = []
    @staticmethod
    def end_postfunc(op):
        GlobalSocket.send("TFIDF#TFIDF_load_phlist_py")
        GlobalSocket.send(GlobalVar.document_list)
        GlobalSocket.send(str(len(GlobalVar.document_store)))
        for x in GlobalVar.document_store:
            GlobalSocket.send(str(x[0].encode('utf-8')))
            GlobalSocket.send(str(x[1].encode('utf-8')))
        GlobalVar.document_store = []

