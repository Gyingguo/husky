import json
import cPickle
from bindings.backend.python.globalvar import GlobalVar, GlobalSocket, OperationParam

def register_all():
    SPCAModel.register()

class SPCAModel:
    @staticmethod
    def register():
        GlobalVar.name_to_prefunc["SPCA#spca_load_pyhlist_py"] = SPCAModel.prefunc
        GlobalVar.name_to_func["SPCA#spca_load_pyhlist_py"] = SPCAModel.func
        GlobalVar.name_to_postfunc["SPCA#spca_load_pyhlist_end_py"] = SPCAModel.end_postfunc

        GlobalVar.name_to_type["SPCA#spca_load_pyhlist_py"] = GlobalVar.librarytype
        GlobalVar.name_to_type["SPCA#spca_load_pyhlist_end_py"] = GlobalVar.librarytype

    # For those functions that need to load from PyHuskyList
    @staticmethod
    def prefunc(op):
        GlobalVar.spca_line_list = op.op_param[OperationParam.list_str]
        GlobalVar.spca_max_idx = op.op_param['max_idx']
        GlobalVar.spca_line_store = []
    @staticmethod
    def func(op, data):
        for I in data:
            assert type(I) is list
            (GlobalVar.spca_line_store).append(I)
    @staticmethod
    def end_postfunc(op):
        GlobalSocket.pipe_to_cpp.send("SPCA#spca_load_pyhlist_py")
        GlobalSocket.pipe_to_cpp.send(GlobalVar.spca_line_list)
        # send out the len of datatset
        GlobalSocket.pipe_to_cpp.send(str(len(GlobalVar.spca_line_store)))
        GlobalSocket.pipe_to_cpp.send(str(GlobalVar.spca_max_idx))
        for I in GlobalVar.spca_line_store:
            GlobalSocket.pipe_to_cpp.send(str(len(I)))
            for i in I:
                GlobalSocket.pipe_to_cpp.send(str(i[0]))
                GlobalSocket.pipe_to_cpp.send(str(i[1]))
        GlobalSocket.spca_line_store = []
