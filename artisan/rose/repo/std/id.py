#!/usr/bin/env artisan

from artisan.core import *
from artisan.rose import *

log.level = 0
ast = model(args=cli(), ws=Workspace('test', auto_destroy=True))

def id_function(ast):
    fns = ast.query("fn:FnDef", where="fn.in_src()")
    print("*** Found %d function(s):" % len(fns))
    for e in fns:
        print (" => instrumenting: %s" % e.fn.name)
        e.fn.instrument(pos="before", code="/* Function: %s */\n" % e.fn.name)
          
id_function(ast)        
ast.commit(sync=False)
ast.export_to("id-result/")





