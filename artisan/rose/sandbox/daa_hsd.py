#!/usr/bin/env artisan
from artisan.core import *
from artisan.rose import *

import subprocess 
import os
import json

log.level = 2

ast = model(args=cli(), ws=Workspace('daa_hsd'))
project = ast.project

# Instrument code with artisan include header and init
global_scope = project.query("g:Global")
global_scope[0].g.instrument(pos='begin', code='#define __ARTISAN__INIT__\n#include <artisan.hpp>')

# Find all loops, instrument each with  a timer   
## TODO: all loop types?                                  
loop_table = project.query("l:ForLoop")
## TODO: indicate nesting level / parent loop  somehow                   
for row in loop_table:
    loop = row.l
    loop.body().instrument(pos='begin', code='Artisan::Timer __timer__("%s", Artisan::op_add);' % str(loop.uid))

# Instrument main function with a timer                 
main_func = project.query("f:FnDef", where="f.name == 'main'")[0].f
main_func.body().instrument(pos='begin', code='Artisan::Timer __timer__("main", Artisan::op_add);')
ast.commit()
# need to add extra block {} for timer                   ## TODO: make sure return is outside of block 
project = ast.project
main_func = project.query("f:FnDef", where="f.name == 'main'")[0].f
main_func.body().instrument(pos='replace', code='{\n %s \n}\n' % main_func.body().unparse()) 

# Artisan::report() at end of main function              ## TODO: check all possible exits
main_func.body().instrument(pos='end', code='Artisan::report("file.json");')
ast.commit()  

# build and run instrumented code, load reported results 
ast.export_to("temp")
wd = os.getcwd()
os.chdir("./temp")
subprocess.call("make")
subprocess.call("./test")
os.chdir(wd)
with open('./temp/file.json') as json_file:
    times = json.load(json_file)

# calculate the time of each loop as a percentage of the main function
main_time = times["main"][0]
for t in times:
    print(t, times[t][0], (times[t][0]/main_time)*100)

# identify hotspots based on some threshold 
