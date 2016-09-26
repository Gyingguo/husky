class TFIDFReceiver:
    @staticmethod
    def register(receiver_map):
        receiver_map["TFIDF#TFIDF_get_TFIDF_value_py"] = TFIDFReceiver.calc_receiver

    @staticmethod
    def calc_receiver(reply):
        res = []
        # eat dummy int64 represents the string length
        dummy = reply.load_int64()
        value = reply.load_double()
        if value == -1:
            res.append("Cannot find the given word in the given document")
        elif value == -2:
            res.append("Cannot find the given document")
        else:
            res.append(value)
        return res

