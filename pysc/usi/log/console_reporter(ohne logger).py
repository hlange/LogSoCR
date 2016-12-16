from __future__ import print_function
import usi
import json
import colorlog
import logging
from datetime import datetime, date, time
from elasticsearch import Elasticsearch
from elasticsearch.helpers import bulk

#elasticsearch
es = Elasticsearch()
global eslogs 
eslogs=[]


#Formatter
COLORLOG_FORMAT = '@%(time)s ns /%(delta_count)s (%(blue)s%(message_type)s%(white)s): %(log_color)s%(levelname)s%(reset)s: %(white)s%(message)s'
STD_FORMAT = '@%(time)s ns /%(delta_count)s (%(message_type)s): %(levelname)s: %(message)s '
FILE_FORMAT = '%(asctime)s @%(time)s ns /%(delta_count)s (%(message_type)s): %(levelname)s: %(message)s'

#Logger initialization
logger = logging.getLogger(__name__)
logger.setLevel(logging.DEBUG)

#console handler
ch = logging.StreamHandler()
ch.setFormatter(colorlog.ColoredFormatter(
    COLORLOG_FORMAT,
    datefmt=None,
    reset=True,
    log_colors={
        'DEBUG':    'cyan',
        'INFO':     'green',
        'WARNING':  'yellow',
        'ERROR':    'red',
        'CRITICAL': 'red,bg_white',
    }))

#file handler
fh = logging.FileHandler(datetime.now().strftime('log_%H:%M_%d_%m_%Y.log'))
fh.setFormatter(
    logging.Formatter(
        FILE_FORMAT,
    )
)


#set level of the handler
fh.setLevel(logging.DEBUG)
ch.setLevel(logging.WARNING)

#add handler to logger
logger.addHandler(ch)
logger.addHandler(fh)

@usi.on("end_of_evaluation")
def save_db(phase):
    global eslogs
    bulk(es, eslogs)
    eslogs = []
    
#function for log messages  
def report(
      message_type=None,
      message_text=None,
      severity=None,
      file_name=None,
      line_number=None,
      time=None,
      delta_count=None,
      process_name=None,
      verbosity=None,
      what=None,
      actions=None,
      phase=None,
      **kwargs):
   
    parameters = ""
    #print(kwargs)
    for value in kwargs:
        if isinstance(kwargs[value], int):
            parameters += "{0}={1:#x} ".format(value, kwargs[value])
        else:
            parameters += "{0}={1} ".format(value, kwargs[value])
            
    now = datetime.utcnow()
    ltime = now.strftime("%Y-%m-%dT%H:%M:%S") + ".%03d" % (now.microsecond /1000) + "Z"        
    extra={'message_type': message_type,
           'message_text': message_text, 
           'severity': severity, 
           'file_name': file_name, 
           'line_number': line_number, 
           'time': time,
           'delta_count': delta_count,
           'process_name': process_name, 
           'verbosity': verbosity, 
           'what': what, 
           'actions': actions, 
           'phase': phase, 
          '@timestamp': ltime}
    #res = es.index(index="logs1", doc_type='document', body=extra)
    logindex=datetime.now().strftime('log_%d_%m_%Y')
    #Collection log information
    global eslogs
    eslog = {
        "_index": logindex,
        "_type": "logs",
        "_source": extra      
        }
    eslogs.append(eslog)
    #sending
    if len(eslogs) > 20000:
        bulk(es, eslogs)
        eslogs = []
    
    extra.update(kwargs)
    
    
    if severity == 0:
        logger.info(message_text, extra=extra)
    elif severity == 1:
        logger.warning(message_text, extra=extra)
    elif severity == 2:
        logger.error(message_text, extra=extra)     
    elif severity == 3:
        logger.critical(message_text, extra=extra)
    elif severity == 4:
         logger.debug(message_text, extra=extra)


   

