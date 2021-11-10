from portalocker import Lock as _Lock
from portalocker.constants import LOCK_EX, LOCK_SH

class Lock(_Lock):
    """A lock to prevent data races in situations with multiple processes"""
    def __init__(self, path, flags, *args, **kwargs):
        _Lock.__init__(self, path, *args, flags=flags, timeout=None, **kwargs)
