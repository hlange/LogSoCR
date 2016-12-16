import logging
import json
import usi
import usi.systemc

class SoCR_StreamHandler(logging.StreamHandler): # Inherit from StreamHandler
    def __init__(self, stream=None):
        logging.StreamHandler.__init__(self)
    
    def emit(self, record):
        if "message_type"  in record.__dict__:
            record.filename = record.__dict__["message_type"]
        if "delta_count"  not in record.__dict__:
            record.__dict__["delta_count"] = usi.systemc.delta_count()
        if "time"  not in record.__dict__:
            record.__dict__["time"] = usi.systemc.simulation_time(usi.systemc.NS)

        logging.StreamHandler.emit(self, record)
    