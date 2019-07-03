import os, distutils.dir_util, shutil
from functools import partial
import os.path as osp 
from git import Repo, Actor
import uuid
import re

class Workspace:

    SID_OK=0
    SID_VERSION_FOLDER_NOT_FOUND=10
    SID_STEP_OUT_OF_RANGE=11

    SID_FORMAT_VAL_EXPECTED=20
    SID_FORMAT_VAL_OPTIONAL=21
    SID_FORMAT_VAL_NOT_EXPECTED=22

    def __init__(self, name, create=True, auto_destroy=False, path='.'):
        self.name = name
        # remove workspace folder only if it is a valid workspace folder
        self.folder = osp.abspath(path + ("" if path.endswith('/') else "/") + name)
        self.auto_destroy = auto_destroy

        if create:
            if osp.exists(self.folder):
                if not self.is_workspace(self.folder):
                   raise TypeError("File or folder '%s' is not a valid workspace! Will not remove it!" % self.folder)   

                distutils.dir_util.remove_tree(self.folder)
            self.create_workspace()      

    def create_workspace(self):
        os.mkdir(self.folder)
        Workspace.__touch(osp.join(self.folder,  '.workspace'))

    def __del__(self):
        if (self.folder != "" and self.auto_destroy):
            distutils.dir_util.remove_tree(self.folder) 

    @staticmethod
    def is_workspace(path):
        return osp.isdir(path) and  osp.isfile(osp.join(path, ".workspace"))

    # [version][:[-]step]]
    @staticmethod
    def parse_sid(sid):
        m = re.search('^([a-zA-Z_][a-zA-Z_0-9]*)?(:(-?\d+))?$', sid)
        if m:
            version = m.group(1)
            step = m.group(3)            
            return (version, step)
        raise ValueError("invalid sid [%s]: wrong format!" % sid)  

    @staticmethod
    def sid2sid(sid, incl_version=True, incl_step=True):
        (version, step) = Workspace.parse_sid(sid)
        return Workspace.to_sid(version if incl_version else None, step if incl_step else None)

    @staticmethod
    def to_sid(version=None, step = None):
        return "%s%s" % (version if version != None else "", 
                         ":" + str(step) if step != None else "")

    def assert_sid(self, sid, incl_version=SID_FORMAT_VAL_OPTIONAL, incl_step=SID_FORMAT_VAL_OPTIONAL, verify=True):
        # check if SID is valid
        (version, step) = Workspace.parse_sid(sid) 
        if incl_version !=  Workspace.SID_FORMAT_VAL_OPTIONAL:
            if incl_version == Workspace.SID_FORMAT_VAL_EXPECTED and version == None:
               raise ValueError("invalid sid [%s]: version must be specified!" % sid)
            elif incl_version == Workspace.SID_FORMAT_VAL_NOT_EXPECTED and version != None:
               raise ValueError("invalid sid [%s]: version must *not* be specified!" % sid)
        if incl_step !=  Workspace.SID_FORMAT_VAL_OPTIONAL:
            if incl_step == Workspace.SID_FORMAT_VAL_EXPECTED and step == None:
               raise ValueError("invalid sid [%s]: step must be specified!" % sid)
            elif incl_step == Workspace.SID_FORMAT_VAL_NOT_EXPECTED and step != None:
               raise ValueError("invalid sid [%s]: step must *not* be specified!" % sid)
        if verify:
            ret = self.verify_sid(sid)
            if ret == Workspace.SID_VERSION_FOLDER_NOT_FOUND:
                raise ValueError("invalid sid [%s]: version folder [%s] not found!" % (sid, version)) 
            elif ret == Workspace.SID_STEP_OUT_OF_RANGE:
                raise ValueError("invalid sid [%s]: step out of range! (# of steps=%d)" % (sid, self.num_steps(sid))) 


    def verify_sid(self, sid):
        (version, step) = Workspace.parse_sid(sid) 

        if version != None:
            version_path = osp.join(self.folder, version)
            if not osp.isdir(version_path):
                return Workspace.SID_VERSION_FOLDER_NOT_FOUND

        if step != None:
            ref_step = self.ref_step(sid, verify=False)
            if ref_step == 0:
               return Workspace.SID_STEP_OUT_OF_RANGE

        return Workspace.SID_OK    

    def path(self, sid, verify = True):
        sid = self.sid2sid(sid, incl_step=False)
        self.assert_sid(sid, incl_version=Workspace.SID_FORMAT_VAL_OPTIONAL, incl_step=Workspace.SID_FORMAT_VAL_OPTIONAL, verify=verify)
        (version, _) = Workspace.parse_sid(sid)
        if version == None:
            return self.folder
        else:    
            return osp.join (self.folder, version)

    def num_steps(self, sid):
        s = Workspace.sid2sid(sid, incl_step=False)
        self.assert_sid(s, incl_version=Workspace.SID_FORMAT_VAL_EXPECTED,  
                           incl_step=Workspace.SID_FORMAT_VAL_NOT_EXPECTED,
                           verify=True)
        p = self.path(s)
        repo = Repo(p)
        return len(repo.tags)

    def ref_step(self, sid, verify=True):
        # get the number of steps - this will also verify if 
        # version is valid
        n = self.num_steps(sid)

        # verify the step is included 
        self.assert_sid(sid, incl_step=Workspace.SID_FORMAT_VAL_EXPECTED, verify=verify)
        
        (_, step) = Workspace.parse_sid(sid)    
        
        if n == 0:
            return 0
        reverse = step[0] == "-"
        nstep = int(step[1:]) if reverse else int(step)

        if nstep == 0:
            return 0

        ret = (n - nstep + 1) if reverse else nstep  
            
        if 0 < ret <= n:
            return ret
        else:        
            return 0


    def create(self, sid, verify_if_exists=True):
        self.assert_sid(sid, incl_version=Workspace.SID_FORMAT_VAL_EXPECTED, incl_step=Workspace.SID_FORMAT_VAL_NOT_EXPECTED, verify=False)

        (version, _) = Workspace.parse_sid(sid)
       
        if version != None:
            version_path = osp.join(self.folder, version)
            if osp.isdir(version_path):
                  if verify_if_exists:
                     raise FileExistsError("cannot create sid[%s]: version dir [%s] already exists!" % (sid, version_path))   
            else:      
               os.mkdir(version_path)
            r = Repo.init(version_path, bare=False)
            r.git.config("user.name", "artisan")
            r.git.config("user.email", "artisan@artisan")

    def commit(self, sid, description):
        self.assert_sid(sid, incl_version=Workspace.SID_FORMAT_VAL_EXPECTED, 
                             incl_step=Workspace.SID_FORMAT_VAL_NOT_EXPECTED, verify=False)
        p = self.path(sid, verify=True)
        repo = Repo(p)
        repo.git.add(A=True)

        # add them to the index
        index = repo.index
        #for (path, stage), entry in index.entries.items():  
        #    print("path: (%s), stage: (%s), entry; (%s)" % (path, stage, entry))   

        commit_id = index.commit(description)
        
        repo.create_tag("s%d" % (len(repo.tags)+1), ref=commit_id)
    

    def rollback(self, sid):
        self.assert_sid(sid, incl_step=Workspace.SID_FORMAT_VAL_EXPECTED, verify=False)
        
        p = self.path(sid, verify=True)
        repo = Repo(p)

        n = self.num_steps(sid)
        s = self.ref_step(sid)

        # hard reset to selected tag
        repo.git.reset('--hard', 's%d' % s)
        
        # remove tags tag
        for i in range(s+1, n+1):
           #print("deleting tag s%d" % i)
           repo.delete_tag("s%d" % i)

    def squash(self, sid, description=None):
        self.assert_sid(sid, incl_version=Workspace.SID_FORMAT_VAL_EXPECTED,  
                        incl_step=Workspace.SID_FORMAT_VAL_NOT_EXPECTED)
        
        p = self.path(sid, verify=True)
        repo = Repo(p)

        n = self.num_steps(sid)
        if n == 0:
            raise RuntimeError("cannot squash [%s]: no commits available!" % sid)
        
        repo.git.reset("--soft", "s1")
        if description == None:
            repo.git.commit("--all", "--amend", "--no-edit")
        else:
            repo.git.commit("--all", "--amend", "-m", "%s" % description)

        # remove all tags
        for i in range(1, n+1):
           repo.delete_tag("s%d" % i)
        
        commit_id = repo.commit("master")
        # create tag
        repo.create_tag("s1", ref=commit_id)        

    def copy(self, sid_src, sid_dst, overwrite = False):
        self.assert_sid(sid_src, incl_version =Workspace.SID_FORMAT_VAL_EXPECTED,  
                                 incl_step=Workspace.SID_FORMAT_VAL_OPTIONAL, 
                                 verify=True)   

        self.assert_sid(sid_dst, incl_version =Workspace.SID_FORMAT_VAL_EXPECTED,  
                                 incl_step=Workspace.SID_FORMAT_VAL_NOT_EXPECTED, 
                                 verify=False)                                    

        (version_src, step_src) = Workspace.parse_sid(sid_src)                
        (version_dst, _) = Workspace.parse_sid(sid_dst)                
           
        # first, check if dst stage exists, otherwise create it
        if (version_src == version_dst):
            raise ValueError("cannot copy to the same location[%s]!" % sid_src)        
        if (version_dst == ""):
            raise ValueError("must specify a valid dst destination!")              
        version_src_path = self.path(self.to_sid(version_src), verify=False)  
        version_dst_path = self.path(self.to_sid(version_dst), verify=False)          

        if not self.verify_sid(version_dst) == Workspace.SID_VERSION_FOLDER_NOT_FOUND:
            if overwrite:
               distutils.dir_util.remove_tree(version_dst_path)
            else:
                raise RuntimeError("version [%s] already exists. Use overwrite=True to remove it before copying!" % version_dst)   
        distutils.dir_util.copy_tree(version_src_path, version_dst_path)

        if  step_src != None:
            new_sid_dst = self.to_sid(version_dst, step_src)
            self.rollback(new_sid_dst)
            #self.squash(self.sid2sid(new_sid_dst, incl_step=False), description="copied from %s" % sid_src)


    def changes(self, sid):
        self.assert_sid(sid, incl_version = Workspace.SID_FORMAT_VAL_EXPECTED,  
                             incl_step= Workspace.SID_FORMAT_VAL_NOT_EXPECTED, 
                             verify=False)  

        p = self.path(sid, verify=True)
        repo = Repo(p)   
        index = repo.index
        i = []
        for (path, stage), entry in index.entries.items(): 
            e = entry[3]
            i.append(e)
        return { 'untracked': repo.untracked_files, 'index': i}    
    '''
    def set_step(self, sid):
        self.assert_sid(sid, incl_version = Workspace.SID_FORMAT_VAL_EXPECTED,  
                             incl_step= Workspace.SID_FORMAT_VAL_OPTIONAL, 
                             verify=True)   
                             
        p = self.path(sid, verify=False)
        (version, step) = Workspace.parse_sid(sid)            

        if step == None:
            n = self.num_steps(sid)
            sid = self.to_sid(stage, version, str(n))

        repo = Repo(p)            
        s = self.ref_step(sid)     
        repo.git.checkout("s%d" % s)
    '''

    def remove(self, sid):
        self.assert_sid(sid, incl_version = Workspace.SID_FORMAT_VAL_EXPECTED,  
                             incl_step= Workspace.SID_FORMAT_VAL_NOT_EXPECTED, 
                             verify=True)             

        p = self.path(sid)
        distutils.dir_util.remove_tree(p)           

    @staticmethod
    def __ign_files(workspace, dir, fns):
        ignore_lst = ['.git']

        if workspace == dir:
            return fns
        elif osp.dirname(workspace) == dir:
            return ignore_lst + [osp.basename(workspace)]                
        else:     
            return ignore_lst 

    def import_from(self, source_dir, sid,  overwrite = False):
        self.assert_sid(sid, incl_version = Workspace.SID_FORMAT_VAL_EXPECTED,  
                              incl_step= Workspace.SID_FORMAT_VAL_NOT_EXPECTED, 
                              verify=False)
        if not osp.isdir(source_dir):
            raise FileNotFoundError("cannot find source dir [%s]!" % source_dir)
        
        dst_dir = self.path(sid, verify=False)
        
        if osp.isdir(dst_dir):
            if overwrite:
                distutils.dir_util.remove_tree(dst_dir)
            else:
                raise RuntimeError("version [%s] already exists. Use overwrite=True to remove it before importing!" % dst_dir)   
        #distutils.dir_util.copy_tree(source_dir, dst_dir)
        shutil.copytree(source_dir, dst_dir, ignore=partial(Workspace.__ign_files, self.folder))        
        self.create(sid, verify_if_exists=False)            
         
    def export_to(self, target_dir, src_sid, overwrite=True):
        self.assert_sid(src_sid, incl_version = Workspace.SID_FORMAT_VAL_EXPECTED,  
                                 incl_step= Workspace.SID_FORMAT_VAL_OPTIONAL, 
                                 verify=False)

        src_dir = self.path(src_sid, verify=True)               
        (_, step) = Workspace.parse_sid(src_sid)                 

        if osp.isdir(target_dir):
            if overwrite:
                distutils.dir_util.remove_tree(target_dir)
            else:
                raise RuntimeError("target directory [%s] already exists. Use overwrite=True to remove it before exporting!" % target_dir)               

        if step != None:
            # prepare a temporary version so that we can rollback
            tmp_sid = Workspace.to_sid(version="__TMP__%s" % str(uuid.uuid4().int))
            self.copy(src_sid, tmp_sid, overwrite=True)
            src_dir = self.path(tmp_sid, verify=True)
            
        distutils.dir_util.copy_tree(src_dir, target_dir)

        if step != None:
            self.remove(tmp_sid)

        # remove .git/ 
        distutils.dir_util.remove_tree(osp.join(target_dir, ".git"))    
        

    @staticmethod
    def __touch(path):
        with open(path, 'a'):
            os.utime(path, None)    





	      
	
