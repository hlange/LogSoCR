'''Basic Logger, no function '''


import json
import logging
import logging.handlers
from datetime import datetime, date, time

STD_FORMAT = '@%(time)s ns /%(delta_count)s (%(filename)s): %(levelname)s: %(message)s '
formatter = logging.Formatter('@%(time)s ns /%(delta_count)s (%(filename)s): %(levelname)s: %(message)s %(parameters)s')
logger = logging.getLogger(__name__) 
logger.setLevel(logging.DEBUG)
ch = logging.StreamHandler()
ch.setFormatter(formatter)
ch.setLevel(logging.WARNING)

#add handler to logger
logger.addHandler(ch)
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
       'phase': phase,
       'parameters':parameters}
           
    extra.update(kwargs)
    logger.warning(message_text,extra=extra)
    
 


   

