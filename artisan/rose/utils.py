from .rose_meta import meta

def scopify(ast):
    pass

def query_loops(root, for_loop=True, while_loop=True, do_loop=True):
    loop_type = []
    if for_loop:
        loop_type.append('ForLoop')
    if while_loop:
        loop_type.append('WhileLoop')
    if do_loop:
        loop_type.append('DoLoop')

    loop_query = root.query('s:Scope', where='s.entity in loop_type', env={'loop_type': loop_type})
    loops = []
    for l in loop_query:
        loops.append(l.s)
    return loops
    

def instrument_block(block, code, entry=False, exit=False):    
    if type(block).__name__ != 'PySgNode' or not block.is_entity('Block'):
        raise RuntimeError("invalid node: not a Block!") 

    if entry:
        block.instrument(pos='begin', code=code)

    if exit: 
        nstmts = block.stmts_count()
        # checks if we need to instrument end of the block
        end_stmt=False

        ret=block.query("ret:Return")
        for r in ret:
            r.ret.instrument(pos='before', code=code)
            if not end_stmt and r.ret.parent().uid == block.uid and r.ret.index() == nstmts-1:
                end_stmt=True

        calls = block.query("call:Call")
        for c in calls:
            if c.call.name() == 'exit':
                stmt = c.call.stmt()        
                stmt.instrument(pos='before', code=code)
                if not end_stmt and stmt.parent().uid == block.uid and stmt.index() == nstmts-1:
                    end_stmt=True


        if not end_stmt:
            block.instrument(pos='end', code=code)

        

         


    
    

