#!/usr/bin/env artisan

import os
import sys
from artisan.core import *
from artisan.rose import *
import pprint

pp = pprint.PrettyPrinter(indent=2)

log.level = 2
args = cli()

ws = Workspace('test', auto_destroy=True)
ast = model(args, ws=ws)
x = ast.project
'''
print("UID=", x.uid)
print("ENTITY=", x.entity)
print("ENTITIES=", x.entities)
print("TAG=", x.tag)
print("SG_TYPE=", x.sg_type)
print("SG_TYPE_REAL=", x.sg_type_real)
print(x.is_entity("gaga"))
print(x.is_entity("Project"))
print(x.is_entity("Node"))
meta.help('Project')
print("HI!")
'''

print(meta.entities)
meta.print_ranks()

yy = x.query('p:Project=>fn:FnDef')

for y in yy:
    y.fn.instrument(pos='before', code='// %s' % y.fn.name)

ast.commit()
ast.export_to("gabriel")    





