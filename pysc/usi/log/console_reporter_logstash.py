from __future__ import print_function
import usi
import json
import colorlog
import logging
import logstash
from datetime import datetime, date, time

#elasticsearch
es = Elasticsearch()
COLORLOG_FORMAT = '@%(time)s ns /%(delta_count)s (%(blue)s%(message_type)s%(white)s): %(log_color)s%(levelname)s%(reset)s: %(white)s%(message)s %(parameters)s'
STD_FORMAT = '@%(time)s ns /%(delta_count)s (%(message_type)s): %(levelname)s: %(message)s %(parameters)s'
FILE_FORMAT = '%(asctime)s @%(time)s ns /%(delta_count)s (%(message_type)s): %(levelname)s: %(message)s %(parameters)s'

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
fh = logging.FileHandler(datetime.now().strftime('log_%d_%m_%Y.log'))
fh.setFormatter(
    logging.Formatter(
        FILE_FORMAT,
    )
)

#logstash handler
lh = logstash.TCPLogstashHandler('localhost', 5959, version=1)
#lh = logstash.LogstashHandler('localhost', 5959, version=1)



#memory handler
mh = logging.handlers.MemoryHandler(50000,logging.ERROR, lh)

#set level of the handler
lh.setLevel(logging.DEBUG)
fh.setLevel(logging.DEBUG)
ch.setLevel(logging.WARNING)

#add handler to logger
logger.addHandler(mh)
logger.addHandler(ch)
logger.addHandler(fh)



#at the end of the simulation
@usi.on("end_of_evaluation")
def save_db(phase):
    mh.flush()
    
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
    
    for value in kwargs:
        if isinstance(kwargs[value], int):
            parameters += "{0}={1:#x} ".format(value, kwargs[value])
        else:
            parameters += "{0}={1} ".format(value, kwargs[value])
                  
    extra={'message_type': message_type,
           #'message_text': message_text, 
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
           'parameters': parameters
          }
    
#extra.update(kwargs) is not possible, without looking at every parameter. Some kwargs use name as key, which is used for something else.
    
    
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


   

