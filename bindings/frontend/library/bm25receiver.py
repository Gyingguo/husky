class BM25Receiver:
    @staticmethod
    def register(receiver_map):
        receiver_map["BM25#topk_py"] = BM25Receiver.topk_receiver
        receiver_map["BM25#get_score_py"] = BM25Receiver.get_score_receiver

    @staticmethod
    def topk_receiver(reply):
        ret = []
        dummy = reply.load_int64()
        k = reply.load_int32()
        for i in xrange(k):
            rk = reply.load_int32()
            id = reply.load_int32()
            pr = reply.load_double()
            ret.append((rk,id,pr))
        return ret

    @staticmethod
    def get_score_receiver(reply):
        ret = []
        dummy = reply.load_int64()
        id = reply.load_int32()
        pr = reply.load_double()
        ret.append((id,pr))
        return ret
         
