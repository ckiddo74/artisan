import os.path as osp
from itertools import takewhile

def __allnamesequal(name):
    return all(n==name[0] for n in name[1:])

def __common_path(paths, sep='/'):
    bydirectorylevels = zip(*[p.split(sep) for p in paths])
    res = sep.join(x[0] for x in takewhile(__allnamesequal, bydirectorylevels))
    if res == "":
        res = sep
    return res    

def rootdir(files):
    # compute rootdir if needed
    rootdir = ""
    for source in files:
        if rootdir == "":
            rootdir = osp.abspath(osp.dirname(source))
        else:
            rootdir = __common_path([rootdir,  osp.abspath(osp.dirname(source))])
    return rootdir


def translate_paths(old_rootdir, new_rootdir, files):
    new_rootdir = new_rootdir.rstrip("/")
    old_rootdir = old_rootdir.rstrip("/")
    abs_files = [ osp.abspath(f) for f in files ]
    return ([ new_rootdir + f[len(old_rootdir):] for f in abs_files ], new_rootdir)