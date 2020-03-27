from __future__ import absolute_import, division, print_function, unicode_literals

class CompileError(Exception):
    '''raise this when there's a problem compiling an extension module'''
    def __init__(self, error):
        Exception.__init__(self,error)
class ConfigurationError(Exception):
    '''raise this when there's a problem with the configuration of dune-py'''
    def __init__(self, error):
        Exception.__init__(self,error)
