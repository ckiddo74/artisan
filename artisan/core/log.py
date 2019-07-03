import sys, logging, os.path as osp
from colors import *

class Log:
    def __init__ (self):
        self._log = logging.getLogger("artisan")   
        self._log.setLevel(logging.DEBUG)     
        sh = logging.StreamHandler()
        sh.setFormatter(logging.Formatter("%(message)s"))
        self._log.addHandler(sh)
        self._log.propagate = False
        
        self.level = 0
        self._tag = ''

    def debug(self, level, msg, tag=''):
        if (level <= self._level) and (self._tag == '' or (tag.startswith(self._tag))):
           caller = sys._getframe().f_back.f_code 
           info0 = color("[dbg:%d%s]" % (level, '' if tag == '' else (':' + tag)), fg="blue")
           info1 = color(" <%s:%s():%d> " % (osp.basename(caller.co_filename), caller.co_name, sys._getframe().f_back.f_lineno), fg="lightblue", style="italic")
           self._log.debug("%s%s%s" % (info0, info1, color(msg, fg='yellow')))

    @property
    def level(self):
       return (self._level, self._tag)

    @level.setter
    def level(self, level):
        if (type(level) is tuple):
            self._level = level[0]
            self._tag = level[1]
        else:
           self._level = level
        self._log.setLevel(self._level)
        
        if (self._level < 2):
            sys.tracebacklimit = 0
            logging.basicConfig(level=logging.WARNING)
        else:
            sys.tracebacklimit = 1000 # default
            logging.basicConfig(level=logging.DEBUG)

log = Log()











    
