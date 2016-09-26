from bindings.frontend.library.graphreceiver import GraphReceiver
from bindings.frontend.library.wordreceiver import WordReceiver
from bindings.frontend.library.gradient_descent_receiver import GDReceiver
from bindings.frontend.library.spca_receiver import SPCAReceiver
from bindings.frontend.library.tfidf_receiver import TFIDFReceiver
from bindings.frontend.library.bm25receiver import BM25Receiver

def register(receiver_map):
    GraphReceiver.register(receiver_map)
    WordReceiver.register(receiver_map)
    GDReceiver.register(receiver_map)
    SPCAReceiver.register(receiver_map)
    TFIDFReceiver.register(receiver_map)
    BM25Receiver.register(receiver_map)
