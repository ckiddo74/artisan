from .rose_meta import meta

# right now only works with loops
def scopify(root):
   # find our target elements
   scopes = root.query("s:Scope")
   for _scope in scopes:
       scope = _scope.s
       if hasattr(scope, 'body'):
            body = scope.body()
            if body and body.entity != 'Block':
                body.instrument(pos="before", code="{")
                body.instrument(pos="replace", code="{@ _.unparse() @}")           
                body.instrument(pos="after", code="}")
   '''
   BUG: cannot instrument if statement bodies
   conditionals = ast.query("if:IfStmt")
   for ifstmt in conditionals:
        tbody = ifstmt.body()
        ebody = ifstmt.body_else()
        cond = ifstmt.cond().unparse(format=3)

        if tbody:
            if tbody.entity == "Block":
                tbody_str = "{@ tbody.unparse() @}" 
            else:
                tbody_str = "{ {@ tbody.unparse() @} }"
        else:
            tbody_str = "{ }"
            
        if ebody:
            if ebody.entity == "Block":
                ebody_str = "else {@ ebody.unparse() @}"
            else:
                ebody_str = "else { {@ ebody.unparse() @} }"
        elif incl_missing_else:
            ebody_str = "else { }"
        else:
            ebody_str = ""            

        ifstmt.instrument(where="around", code = "if (%s) %s%s" % (cond, tbody_str, ebody_str), env={'tbody': tbody, 'ebody': ebody})
    '''


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

        

         


    
    

