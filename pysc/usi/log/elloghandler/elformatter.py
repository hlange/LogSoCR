import traceback
import logging
import sys
from datetime import datetime
import uuid
import getpass
try:
    import json
except ImportError:
    import simplejson as json


class FormatterBase(logging.Formatter):
 
    def get_extra_fields(self, record):
        # The list contains all the attributes listed in
        # http://docs.python.org/library/logging.html#logrecord-attributes
        #and severity because it would be redudant
        skip_list = (
            'args', 'asctime', 'created', 'exc_info', 'exc_text', 'filename',
            'funcName', 'id', 'levelname', 'levelno', 'lineno', 'module',
            'msecs', 'msecs', 'message', 'msg', 'name', 'pathname', 'process',
            'processName', 'relativeCreated', 'thread', 'threadName', 'extra',
            'auth_token', 'password','severity')

        if sys.version_info < (3, 0):
            easy_types = (basestring, bool, dict, float, int, long, list, type(None))
        else:
            easy_types = (str, bool, dict, float, int, list, type(None))

        fields = {}

        for key, value in record.__dict__.items():
            if key not in skip_list:
                if isinstance(value, easy_types):
                    fields[key] = value
                #else:
                    #fields[key] = repr(value)

        return fields

    def get_debug_fields(self, record):
        fields = {
            'stack_trace': self.format_exception(record.exc_info),
            'lineno': record.lineno,
            'process': record.process,
            'thread_name': record.threadName,
        }

        # funcName was added in 2.5
        if not getattr(record, 'funcName', None):
            fields['funcName'] = record.funcName

        # processName was added in 2.6
        if not getattr(record, 'processName', None):
            fields['processName'] = record.processName

        return fields


    @classmethod
    def format_timestamp(cls, time):
        tstamp = datetime.utcfromtimestamp(time)
        return tstamp.strftime("%Y-%m-%dT%H:%M:%S") + ".%03d" % (tstamp.microsecond / 1000) + "Z"

    @classmethod
    def format_exception(cls, exc_info):
        return ''.join(traceback.format_exception(*exc_info)) if exc_info else ''


class Formatter(FormatterBase):
    logindex= ""
    user = getpass.getuser()
    log_id = uuid.uuid1()
    def __init__(self,index):
            # run the regular Handler __init__
            Formatter.logindex = index
    def format(self, record):
        # Add extra fields
        
        extra = self.get_extra_fields(record)
        # If exception, add debug info
        if record.exc_info:
            extra.update(self.get_debug_fields(record))
        extra.update({'levelname': record.levelname,
                'logger': record.name,
                '@timestamp': self.format_timestamp(record.created),
                'message_text': record.getMessage(),
                'user': Formatter.user,
                'logid': Formatter.log_id,
                'filename':record.filename
                     })
        eslog = {
            "_index": Formatter.logindex,
            "_type": "logs",
            "_source": extra      
        }
        

        return eslog