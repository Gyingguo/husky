class SPCAReceiver:
    @staticmethod
    def register(receiver_map):
        receiver_map["SPCA#spca_train_py"] = SPCAReceiver.train_receiver

    @staticmethod
    def train_receiver(reply):
        res = ""
        # eat dummy int64 represents the string length
        dummy = reply.load_int64()

        param_v = reply.load_str()
        res += param_v
        return res
