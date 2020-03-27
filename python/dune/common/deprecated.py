import logging

class DeprecatedObject(object):
    def __init__(self, real, message):
        self.real = real
        self.logger = logging.getLogger(real.__module__)
        self.message = message

    def __call__(self, *args, **kwargs):
        object.__getattribute__(self, "logger").warning(object.__getattribute__(self, "message"))
        return object.__getattribute__(self, "real")(*args, **kwargs)

    def __getattribute__(self, name):
        object.__getattribute__(self, "logger").warning(object.__getattribute__(self, "message"))
        return getattr(object.__getattribute__(self, "real"), name)

    def __repr__(self):
        object.__getattribute__(self, "logger").warning(object.__getattribute__(self, "message"))
        return repr(object.__getattribute__(self, "real"))
