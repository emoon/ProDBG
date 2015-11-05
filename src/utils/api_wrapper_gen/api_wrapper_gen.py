#-----------------------------------------------------------------
# pycparser: func_defs.py
#
# Using pycparser for printing out all the calls of some function
# in a C file.
#
# Copyright (C) 2008-2015, Eli Bendersky
# License: BSD
#-----------------------------------------------------------------
from __future__ import print_function
import sys

# This is not required if you've installed pycparser into
# your site-packages/ with setup.py
sys.path.extend(['.', '..'])

from pycparser import c_parser, c_ast, parse_file

def _explain_type(decl):
    """ Recursively explains a type decl node
    """
    typ = type(decl)

    if typ == c_ast.TypeDecl:
        quals = ' '.join(decl.quals) + ' ' if decl.quals else ''
        return quals + str(_explain_type(decl.type))
    elif typ == c_ast.Typename or typ == c_ast.Decl:
        return _explain_type(decl.type)
    elif typ == c_ast.IdentifierType:
        return ' '.join(decl.names)
    elif typ == c_ast.PtrDecl:
        quals = ' '.join(decl.quals) + ' ' if decl.quals else ''
        return quals + 'pointer to ' + _explain_type(decl.type)
    elif typ == c_ast.ArrayDecl:
        arr = 'array'
        if decl.dim: arr += '[%s]' % decl.dim.value

        return arr + " of " + _explain_type(decl.type)

    elif typ == c_ast.FuncDecl:
        if decl.args:
            params = [str(_explain_type(param)) for param in decl.args.params]
            args = ', '.join(params)
        else:
            args = ''

        return ('function(%s) returning ' % (args) +
                _explain_type(decl.type))

# A visitor with some state information (the funcname it's
# looking for)
#
class FuncCallVisitor(c_ast.NodeVisitor):


    def _explain_decl_node(self, decl_node):
        """ Receives a c_ast.Decl note and returns its explanation in
            English.
        """
        storage = ' '.join(decl_node.storage) + ' ' if decl_node.storage else ''

        return (decl_node.name + " is a " + storage + _explain_type(decl_node.type))

    def visit_Struct(self, n):
        print("struct name " + n.name + "\n")
        if n.decls:
            #s += '\n'
            #s += self._make_indent()
            #s += '{\n'
            for decl in n.decls:
                print(decl.name)
                for c_name, c in decl.children():
                    m = self._explain_decl_node(decl)
                    print(m)

    # def visit_FuncCall(self, node):
     #    if node.name.name == self.funcname:
      #       print('%s called at %s' % (self.funcname, node.name.coord))

if __name__ == "__main__":

    ast = parse_file('/Users/danielcollin/code/ProDBG/api/include/pd_ui.h', use_cpp=True,
            cpp_path='gcc',
            cpp_args=['-E', r'-Ifake_libc_include'])

    v = FuncCallVisitor()
    v.visit(ast)

