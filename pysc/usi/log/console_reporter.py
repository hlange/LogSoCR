from __future__ import print_function
import json
import colorlog
import logging
import logging.handlers
from elloghandler.handler import ElLogHandler
from socr_streamhandler.handler import SoCR_StreamHandler
from socr_filehandler.handler import SoCR_FileHandler
from elasticsearch import Elasticsearch
from elasticsearch.helpers import bulk
from datetime import datetime, date, time


def logger_conf(loglevel,index=None):
    #Formatter
    COLORLOG_FORMAT = '@%(time)s ns /%(delta_count)s (%(blue)s%(filename)s%(white)s): %(log_color)s%(levelname)s%(reset)s: %(white)s%(message)s'
    STD_FORMAT = '@%(time)s ns /%(delta_count)s (%(filename)s): %(levelname)s: %(message)s '
    FILE_FORMAT = '%(asctime)s @%(time)s ns /%(delta_count)s (%(filename)s): %(levelname)s: %(message)s'
    #Logger initialization
    log_root = logging.getLogger()
    logger.setLevel(logging.DEBUG)

    #ElasticSearch Handler
    if index is not None:
        eh =  ElLogHandler(index)
    else:
        eh =  ElLogHandler()


    #console handler
    ch = SoCR_StreamHandler()
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
    fh = SoCR_FileHandler(datetime.now().strftime('log_%H:%M_%d_%m_%Y.log'))
    fh.setFormatter(
        logging.Formatter(
            FILE_FORMAT,
        )
    )
    
    #setting the priorities of the handler
    
    #Elasticsearch
    if loglevel[0] == '0':
        eh.setLevel(logging.INFO)
    elif loglevel[0] == '1':
        eh.setLevel(logging.WARNING)
    elif loglevel[0] == '2':
        eh.setLevel(logging.ERROR)
    elif loglevel[0] == '3':
        eh.setLevel(logging.CRITICAL)

    if loglevel[0] == 'x':
        pass
    else:
        log_root.addHandler(eh)
    
    #File
    if loglevel[2] == '0':
        fh.setLevel(logging.INFO)
    elif loglevel[2] == '1':
        fh.setLevel(logging.WARNING)
    elif loglevel[2] == '2':
        fh.setLevel(logging.ERROR)
    elif loglevel[2] == '3':
        fh.setLevel(logging.CRITICAL)

    if loglevel[2] == 'x':
        pass
    else:
        log_root.addHandler(fh)
        
    #Terminal
    if loglevel[4] == '0':
        ch.setLevel(logging.INFO)
    elif loglevel[4] == '1':
        ch.setLevel(logging.WARNING)
    elif loglevel[4] == '2':
        ch.setLevel(logging.ERROR)
    elif loglevel[4] == '3':
        ch.setLevel(logging.CRITICAL)

    if loglevel[4] == 'x':
        pass
    else:
        log_root.addHandler(ch)
logger = logging.getLogger(__name__)         
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
    extra={ 
           'message_type':message_type,
           'severity': severity, 
           'file_name': file_name, 
           'line_number': line_number, 
           'time': time,
           'delta_count': delta_count,
           'process_name': process_name, 
           'verbosity': verbosity, 
           'what': what, 
           'actions': actions, 
           'phase': phase}
          
    
    #Collection log information
    
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


   

