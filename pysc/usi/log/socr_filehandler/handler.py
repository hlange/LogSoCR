import logging
import json
import usi
import usi.systemc

class SoCR_FileHandler(logging.FileHandler): # Inherit from FileHandler

    def emit(self, record):
        if "message_type"   in record.__dict__:
            record.filename = record.__dict__["message_type"]
        if "delta_count"  not in record.__dict__:
            record.__dict__["delta_count"] = usi.systemc.delta_count()
        if "parameters"  not in record.__dict__:
            record.__dict__["parameters"] = ""
        if "time"  not in record.__dict__:
            record.__dict__["time"] = usi.systemc.simulation_time(usi.systemc.NS)
        logging.FileHandler.emit(self, record)
    
