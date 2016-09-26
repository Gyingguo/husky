from bindings.frontend.huskylist import HuskyList
from bindings.frontend.huskylist import PyHuskyList
from bindings.frontend.operation import Operation, OperationParam
from bindings.frontend import scheduler

class SPCA(HuskyList):
    def __init__(self, n_components=-1):
        assert type(n_components) is int

        super(SPCA, self).__init__();
        self.list_name += "SPCA"
        self._loaded = False
        self._trained = False

        param = {"n_components" : str(n_components),
                OperationParam.list_str : self.list_name,
                "Type" : "cpp"}
        op = Operation("SPCA#spca_init_py", param, [])
        scheduler.compute(op)

    def load_hdfs(self, url):
        """Load data into list from hdfs

        Parameter:
        -----------
        url: string
             path to source file (hdfs)

        """
        assert type(url) is str
        assert self._loaded == False

        param = {"url" : url,
                OperationParam.list_str : self.list_name,
                "Type" : "cpp"}
        op = Operation("SPCA#spca_load_hdfs_py", param, []);
        scheduler.compute(op)
        self._loaded = True

    def load_pyhlist(self, load_list, max_idx):
        """Load data into list from PyHuskyList

        Parameters:
        ----------
        load_list: list of lists (PyHuskyList), 
                   the sublist of load_list consists of tuples 
                   PyHuskyList which is loaded using env.load() and then 
                   mapped using spca_line_parse
        
        max_idx: int
                 the maximum index of all first elements of the tuples
                 i.e. the number of features in the datasets 
        """
        assert self._loaded == False

        if isinstance(load_list, PyHuskyList):
            param = {OperationParam.list_str : self.list_name,
                    "max_idx" : str(max_idx)}
            self.pending_op = Operation("SPCA#spca_load_pyhlist_py",
                    param,
                    [load_list.pending_op])
            scheduler.compute(self.pending_op)
            self._loaded = True
        else:
            return NotImplemented

    def train(self, max_iteration=100, reconErr_limit=0.1):
        """Train the datasets according to your configuration (n_components, max_iteration, reconErr_limit)

        Parameters:
        -----------
        max_iteration: int
                       maximum number of iterations before terminating

        reconErr_limit: double
                        reconErr = (Yi - Ym) * CM * C' - (Yi - Ym)
                        the error limit which you can barely accept
        """
        assert self._loaded is True
        assert type(max_iteration) is int
        assert type(reconErr_limit) is float

        param = {"max_iteration" : str(max_iteration),
                "reconErr_limit" : str(reconErr_limit),
                OperationParam.list_str : self.list_name,
                "Type" : "cpp"}
        op = Operation("SPCA#spca_train_py", param, [])
        final_C = scheduler.compute_collect(op)
        self.result_ = final_C
        self._loaded = False
        self._trained = True

    def get_C(self):
        """Compute the transform matrix

        Return:
        -------
        C: string
           you can tranform it into matrix of size(n_features, n_components) 
           for you own use  
        """
        assert self._trained is True
        return self.result_

    def __del__(self):
        param = {OperationParam.list_str : self.list_name,
                "Type" : "cpp"}
        op = Operation("SPCA#del_py", param, [])
        return scheduler.compute_collect(op)
