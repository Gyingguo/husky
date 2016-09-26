from bindings.backend.python.globalvar import GlobalVar, GlobalSocket, OperationParam

def register_all():
    # register word 
    ToWords.register()

class ToWords:
    @staticmethod
    def register():
        GlobalVar.name_to_func["Word#load_phlist_py"] = ToWords.func
        GlobalVar.name_to_prefunc["Word#load_phlist_py"] = ToWords.prefunc

        GlobalVar.name_to_postfunc["Word#load_phlist_end_py"] = ToWords.end_postfunc

        GlobalVar.name_to_type["Word#load_phlist_py"] = GlobalVar.librarytype
        GlobalVar.name_to_type["Word#load_phlist_end_py"] = GlobalVar.librarytype 

    @staticmethod
    def func(op, data):
        for x in data:
            GlobalVar.word_store.append(str(x))
    @staticmethod
    def prefunc(op):
        GlobalVar.word_list = op.op_param[OperationParam.list_str]
        GlobalVar.word_store = []
    @staticmethod
    def end_postfunc(op):
        GlobalSocket.send("Word#load_phlist_py")
        GlobalSocket.send(GlobalVar.word_list)
        GlobalSocket.send(str(len(GlobalVar.word_store)))
        for x in GlobalVar.word_store:
            GlobalSocket.send(x)
        GlobalVar.word_store = []

