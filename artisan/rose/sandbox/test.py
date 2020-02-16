#!/usr/bin/env artisan
from artisan.core import *
from artisan.rose import *


log.level = 2

ast = model(args=cli(), ws=Workspace('test'))

# ['Block', 'File', 'FnDef', 'ForLoop', 'Node', 'Project', 'Scope', 'SrcNode', 'Stmt']

print(ast.project.tree())

project = ast.project
r = project.query("g:Global => f:FnDef", where="f.name == 'main'")

if r:
    glb = r[0].g
    glb.instrument(pos='begin', code='#define __ARTISAN__INIT__\n#include <artisan.hpp>')
    r[0].f.body().instrument(pos='end', code='Artisan::report("test");')
    
    loops = project.query("l:ForLoop")
    for loop in loops:
        loop.l.body().instrument(pos='begin', code='Artisan::Timer __timer__("test", Artisan::op_add);')


    ast.commit()
    ast.print()







