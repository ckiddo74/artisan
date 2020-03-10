import sys
import os.path as osp
import os
import shlex
import pathlib

from pygments import highlight
from pygments.lexers.c_cpp import CppLexer
from pygments.formatters import Terminal256Formatter

from artisan.core import *
import artisan.core.path as artcore_path


from build.lib.artrose import frontend as rose_frontend, \
                              destroy_project as rose_destroy_project,  \
                              unparse_prj as rose_unparse_prj, \
                              set_verbose_level    

def model(args, workdir="", std="", ws=None, sid=None):
    return Model(args=args, workdir=workdir, std=std, ws=ws, sid=sid)

def cli():    
   cwd = os.getcwd()
   # make sure all source files have an absolute path
   lst_args = [ os.path.join(cwd, arg) if (arg[0] not in ['-', '/']) else arg for arg in sys.argv[1:]]

   # add quotes
   lst_args = [ '"' + os.path.join(cwd, arg)  + '"' if (arg[0] not in ['-', '"']) else arg for arg in sys.argv[1:]]

   log.debug(2, "cli args: %s" % lst_args, "cli")

   lst_args.extend(['-I/artisan/artisan/rose/repo/cpp'])

   args = ' '.join(lst_args)
   return args

class Model:
    stds = { 
       'C'   :   {'rose-args': ['-rose:c99_only'], 'language': 'C', 'std': 'C99' },
       'C89'   :   {'rose-args': ['-rose:c89_only'], 'language': 'C', 'std': 'C89' },
       'C99'   :   {'rose-args': ['-rose:c99_only'], 'language': 'C', 'std': 'C99' },
       'C11'   :   {'rose-args': ['-rose:c11_only'], 'language': 'C', 'std': 'C11' },
       'C++' :   {'rose-args': ['-rose:cxx11_only'], 'language': 'C++', 'std': 'C++11' },
       'C++89' :   {'rose-args': ['-rose:cxx_only'], 'language': 'C++', 'std': 'C++89' },
       'C++11' :   {'rose-args': ['-rose:cxx11_only'], 'language': 'C++', 'std': 'C++11' },
    }    


    def __init__ (self, args, workdir="", std="", ws=None, sid=None):     
        self.project = None

        # self.args <- { 'sources': [...], 'flags': [...], 'workdir': "...", 'language': '...', 'std': '...' }

        self.args = Model.__process_args(args=args, workdir=workdir, std=std)
        self.ws = ws
        self.sid = sid

        set_verbose_level(log.level)

        edg_path = osp.dirname(osp.realpath(__file__)) + "/../../build"
        os.environ['ROSE_SYS_HEADERS'] = edg_path
        
        # forces EDG to use a new /usr/include path
        os.environ['USR_INCLUDE'] =  edg_path + "/include/edg/usr/include"

        if self.sid != None and ws == None:
            raise RuntimeError("sid specified without a workspace!")

        # process workspace and sid
        if ws != None:
            version = None; step = None;  
            if sid:
                (version, step) = ws.parse_sid(sid)
            if not version:
                version = "default"    
             
            self.sid = ws.to_sid(version, step) 

            sid_status = ws.verify_sid(self.sid)
            
            if sid_status == ws.SID_OK:
                print("B1")
                # we should load the snapshot                                              
                if (step != None): 
                   #ws.rollback(self.sid)
                   raise RuntimeError("cannot specify step number!")

                new_path = ws.path(self.sid)
                (self.args["sources"], self.args["workdir"]) = artcore_path.translate_paths(self.args["workdir"], new_path , self.args["sources"])   
                self.__run_frontend()

            elif (sid_status == ws.SID_STEP_OUT_OF_RANGE):
                raise RuntimeError("invalid step specified (out of range!)")          
            elif (sid_status == ws.SID_VERSION_FOLDER_NOT_FOUND):
               ws.import_from(self.args["workdir"], self.sid)
               new_path = ws.path(self.sid)
               (self.args["sources"], self.args["workdir"]) = artcore_path.translate_paths(self.args["workdir"], new_path , self.args["sources"])
               self.__run_frontend()
               # In principle, it is not required to sync
               self.commit("initial code", sync=False) 
        else:  
               self.__run_frontend()
        
        self._update_workspace_sid()  

    def print(self):
        if self.project is None:
            raise RuntimeError("project is None!")
        print(highlight(self.project.unparse(), CppLexer(), Terminal256Formatter()))
        
        
    @classmethod
    def __process_args(cls, args, workdir, std):
        ret_args = { 'sources': [], 'flags': [], 'workdir': "", 'language': '', 'std': '' }

        # parse args
        for arg in shlex.split(args):
            if arg[0] == '-':
                ret_args["flags"].append(arg)
            else:
                ret_args["sources"].append(arg)

        if len(ret_args["sources"]) == 0:
            raise RuntimeError("no sources have been specified to build the model!")                 

        if std == "":
            # determine if we have C++ or C sources. 
            if all([pathlib.Path(a).suffix == '.c' for a in ret_args['sources']]):
                std = "C"
            else:
                std = "C++"

        # check if the language standard exists in our list
        std = std.upper()
        if std not in Model.stds:
            raise RuntimeError("standard [%s] does not exist!" % std)
        ret_args['std'] = Model.stds[std]['std']
        ret_args['language'] = Model.stds[std]['language']
        ret_args['flags'].extend(Model.stds[std]['rose-args'])        

        # set workdir 
        if workdir != "" and workdir[0] == '/':
            # set supplied workdir if it is absolute
            ret_args['workdir'] = workdir
        else:
            sources_wd = artcore_path.rootdir(ret_args["sources"])
            if workdir == "":
                ret_args['workdir'] = sources_wd
            else:
                # user supplied relative path
                ret_args['workdir'] = osp.join(sources_wd, workdir)

        if ret_args["workdir"] == "":
            raise RuntimeError("workdir must be specified!")
        elif ret_args["workdir"] == "/":
            raise RuntimeError("workdir cannot be root ('/') directory!")    
        
        return ret_args

    def _update_workspace_sid(self):
        if self.ws == None: return
        (version, _) = Workspace.parse_sid(self.sid)
        num_steps = self.ws.num_steps(Workspace.to_sid(version))
        # always point to the top
        self.sid = Workspace.to_sid (version=version, step=num_steps)
                  
    def __run_frontend(self):
        curr_dir = os.getcwd()
        os.chdir(self.args["workdir"])
        # removes all -compiler:X in flags, and replaces -artisan:Y to -Y
        flags = [ flag.replace("-artisan:", "-") for flag in self.args["flags"] if not any(x in flag for x in ['-compiler:'] )]
     
        self.project = rose_frontend(self.args["sources"] + flags)
        os.chdir(curr_dir)

    def __del__(self):        
        if self.project != None:
            set_verbose_level(log.level)       
            rose_destroy_project(self.project) 
             
    def query(self, match, where="", env=None):        
        table = self.project.query(match=match, where=where, env=env)
        return table

    def commit(self, desc="", sync=True, pretty_print=True):
        if self.ws == None:
            raise RuntimeError("cannot commit: model requires a workspace!")
        set_verbose_level(log.level)            
        rose_unparse_prj(self.project, pretty_print)
        self.ws.commit(sid=self.ws.sid2sid(self.sid, incl_step = False), description=desc)
        self._update_workspace_sid()  
        if sync:
           self.sync()    
           
    def undo(self, n=1, sync=True):
        if self.ws == None:
            raise RuntimeError("cannot commit: model requires a workspace!")   
        
        (version, _) = Workspace.parse_sid(self.sid)
        num_steps = self.ws.num_steps(Workspace.to_sid(version))             
        step = num_steps - n
        if step <= 0:
            raise RuntimeError("Invalid number of steps: %d" % n)
        roll_sid = Workspace.to_sid(version, step) 
        self.ws.rollback(sid=roll_sid)
        if sync:
            self.sync()
        else:
            self._update_workspace_sid() 
        
    
    def sync(self):
       if not self.ws:
           raise RuntimeError("cannot sync without workspace!") 

       self.__run_frontend() 
       self._update_workspace_sid()
                 

    def export_to(self, target_dir, sid="", overwrite=True):
        if self.ws == None:
            raise RuntimeError("cannot export: model requires a workspace!")  
        if sid == "":
           exp_sid = self.sid
        else:
            (w_version, w_step) = Workspace.parse_sid(self.sid)
            (s_version, s_step) = Workspace.parse_sid(sid)
            if (s_version != None) and (s_version != ""):
                raise RuntimeError("cannot specify source version, only step number!")

            s_version = w_version
            if s_step == None:
                s_step = w_step

            exp_sid = Workspace.to_sid(s_version, s_step)  
        log.debug(1, "exporting to path '%s' from '%s'" % (target_dir, exp_sid), "cli") 
        self.ws.export_to(target_dir, exp_sid, overwrite)    
    
    def execute(self, compiler=None, app_name=None, app_args=None, extra_flags=None, silence=True):

        if not compiler:
            if self.language['language'] == 'C':
                compiler = "gcc"
            else:
                compiler = "g++"

        if not app_name:
            app_name = "app"
        if not app_args:
            app_args = ""    
        
        flags = self.args["flags"]  
        if extra_flags:            
            flags.extend(extra_flags)
        # remove ROSE and edg flags, and replace -compiler:param to -param
        flags = [ flag.replace("-compiler:", "-") for flag in flags if not any(x in flag for x in ['-rose:', '-edg:', '-artisan:'] )]
        
        cmd = "cd %s && %s -o %s %s %s" % (self.args["workdir"], compiler, app_name, " ".join(self.args["sources"]), " ".join(flags))
        
        # remove this msg
        if silence:
            ret=os.system(cmd + " > /dev/null")
        else:
           print("[i] building: %s" % cmd)
           ret=os.system(cmd)

        if ret != 0:
            raise RuntimeError("cannot run build cmd: %s!" % cmd)

        cmd = "cd %s && ./%s %s" % (self.args["workdir"], app_name, app_args)
        if silence:
            ret = os.system(cmd + " > /dev/null")            
        else:
           print("[i] running: %s" % cmd)
           ret= os.system(cmd)        

        if ret != 0:
            raise RuntimeError("cannot run cmd: %s!" % cmd)



        
            



        

