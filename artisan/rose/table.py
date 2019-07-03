from prettytable import PrettyTable
from colors import *
from artisan.core.table import NodeTable

class RoseNodeTable(NodeTable):
   def __repr__ (self):
      width=40
      if len(self.nodes) == 0: 
          return ""
      'set header'    
      keys = self.keys
      t = PrettyTable(keys)
      for k in keys:
          t.align[k] = "l"
      for n in self.nodes:
          
          l = [cyan(n[k].entity) + red(" (" + 
                      (lambda x: x.sg_type if x.sg_type == x.sg_type_real else x.sg_type + ":" + x.sg_type_real) (n[k])
                      + ")") + ":\n" + yellow(n[k].unparse()[0:width])  for k in keys if n[k]]
          t.add_row(l)   

      return str(t)
