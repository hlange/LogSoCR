import logging
import elformatter
import json
import usi
import usi.systemc
from elasticsearch import Elasticsearch
from elasticsearch.helpers import bulk
import threading
from datetime import datetime

class ElLogHandler(logging.Handler): # Inherit from logging.Handler
    eslogs=[]
    es = Elasticsearch()
    logindex= datetime.now().strftime('log_%d_%m_%Y')
    def __init__(self, elindex = None):
        # run the regular Handler __init__
        logging.Handler.__init__(self)
        if elindex is not None:
            ElLogHandler.logindex = elindex
        self.formatter = elformatter.Formatter(ElLogHandler.logindex)
        #create index in elasticsearch
        ElLogHandler.es.indices.create(index=ElLogHandler.logindex,ignore=400)
        ElLogHandler.es.indices.put_settings(index=ElLogHandler.logindex, body={'refresh_interval': '-1', 'index.number_of_replicas': '0'})
    def emit(self, record):
        if "message_type"  in record.__dict__:
            record.filename = record.__dict__["message_type"]
        if "delta_count"  not in record.__dict__:
            record.__dict__["delta_count"] = usi.systemc.delta_count()
        if "time"  not in record.__dict__:
            record.__dict__["time"] = usi.systemc.simulation_time(usi.systemc.NS)
            # record.message is the log message
        ElLogHandler.eslogs.append(self.formatter.format(record))
        if len(ElLogHandler.eslogs)>20000:
            bulk(ElLogHandler.es, ElLogHandler.eslogs)
            ElLogHandler.eslogs = []
                
    @usi.on("end_of_evaluation")
    def save_db(phase):
        bulk(ElLogHandler.es, ElLogHandler.eslogs)
        ElLogHandler.es.indices.put_settings(index=ElLogHandler.logindex, body={'refresh_interval': '1s','index.number_of_replicas': '1'})