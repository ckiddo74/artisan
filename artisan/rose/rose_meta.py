from build.lib.artrose import meta_entities, meta_info
import pprint
from colors import *

class Meta:
    pp = pprint.PrettyPrinter(indent=2)
    entities = meta_entities()    

    @staticmethod
    def info(entity):
        return meta_info(entity)

    @staticmethod
    def pp_info(entity):
        Meta.pp.pprint(Meta.info(entity)) 

    @staticmethod
    def ranks():
        ranks = {  }
        index = {  }
        entities = list(Meta.entities)
        while len(entities) > 0:
           lent = list(entities) 
           for e in lent:
               info = Meta.info(e)
               if len(info['entities']) > 1:
                   parent = info['entities'][1:][0]
                   if parent in index:
                       index[e] = { }
                       index[parent].update({e:index[e]})
                       entities.remove(e)
               else:
                   ranks[e] = { }
                   index[e] = ranks[e]
                   entities.remove(e)
        return ranks

                
    @staticmethod
    def print_ranks(ranks=None, level = 0):
        if ranks == None:
           ranks = Meta.ranks()

        for e in ranks:
            print(color(('' if level == 0 else '+')  + '-'*(level*3) + (' ' if level != 0 else ''), fg='grey') + red(e, style='bold') + ": " + color(Meta.info(e)['description'], fg='lightgrey', style='italic'))
            Meta.print_ranks(ranks[e], level+1)


    @staticmethod
    def help(entity, quit=True):
        if type(entity) != str:
            entity = entity.entity

        all_entities = Meta.entities        
        info = Meta.info(entity)
        
        #Meta.pp_info(entity)
        
        frame = color('='*(len(entity)+6), fg='grey', style='bold')
        print("\n%s\n%s\n%s" % (frame, color("HELP: %s" % entity, fg='white',  style='bold'), frame))

        docs = ''
        unique_attrs = set()
        for e in info['entities']:
            e_info = Meta.info(e)
            print(' - %s: %s' % ((red(e, style='bold') if e == entity else yellow(e, style='bold')), \
                                 color(e_info['description'], fg='red') if e == entity else color(e_info['description'], fg='yellow')))
            e_attrs = e_info['attributes']              
            lst_attrs = list(e_attrs.keys())
            lst_attrs.sort()          
            for e_attr in lst_attrs:
               if e_attr not in unique_attrs:
                   unique_attrs.add(e_attr)
               else:
                   continue
               e_doc = e_attrs[e_attr] 
               is_method = e_doc['is_method']
               params = ''
               for p in e_doc['params']:
                  param = "%s=%s" % (p[0], '\"\"' if p[1] == "" else p[1]) if type(p) == tuple else p
                  params += "%s%s" % (("" if params == "" else ", "), param)
                   
               print("      %s: %s" % (color(e_attr+"(" + params + ")", fg='cyan', style='bold') if is_method else color(e_attr, fg='blue'), e_doc['description']))
            print()
        if (quit):
            exit(0)

        


meta = Meta


