# SPDX-FileCopyrightInfo: Copyright © DUNE Project contributors, see file LICENSE.md in module root
# SPDX-License-Identifier: LicenseRef-GPL-2.0-only-with-DUNE-exception

import warnings

def deprecated(msg,name=None):
    '''This is a decorator which can be used to mark functions
    as deprecated. It will result in a warning being emitted
    when the function is used.'''
    def deprecated_decorator(func):
        def new_func(*args, **kwargs):
            if name is None:
                funcName = func.__name__
            else:
                funcName = name
            if msg:
                print("Call to deprecated function/property `{}`.".format(funcName),msg)
            else:
                print("Call to deprecated function/property `{}`.".format(funcName))
            warnings.warn("Call to deprecated function {}.".format(funcName),
                          category=DeprecationWarning)
            # note: DepracationWarning is ignored by default (could use FutureWarning)
            # also the warning will be only shown once
            return func(*args, **kwargs)
        new_func.__name__ = func.__name__
        new_func.__doc__ = func.__doc__
        new_func.__dict__.update(func.__dict__)
        return new_func
    return deprecated_decorator
