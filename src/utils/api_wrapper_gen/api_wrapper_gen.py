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

# A visitor with some state information (the funcname it's
# looking for)
#
class FuncCallVisitor(c_ast.NodeVisitor):
        
    def visit_Struct(self, n):
        print("struct name " + n.name + "\n")

    # def visit_Enum(self, n):
    # print("enum " + n.name);

    def visit_FuncCall(self, node):
        print('%s called at %s' % (self.funcname, node.name.coord))


    # def visit_FuncCall(self, node):
     #    if node.name.name == self.funcname:
      #       print('%s called at %s' % (self.funcname, node.name.coord))

if __name__ == "__main__":

    ast = parse_file('/Users/danielcollin/code/ProDBG/api/include/pd_backend.h', use_cpp=True,
            cpp_path='gcc',
            cpp_args=['-E', r'-Ifake_libc_include'])

    v = FuncCallVisitor()
    v.visit(ast)

