import sys,os

def inVEnv():
    # check whether we are in a anaconda environment
    # were the checks based on prefix and base_prefix
    # seem to fail
    if "CONDA_DEFAULT_ENV" in os.environ:
        return 1

    # If sys.real_prefix exists, this is a virtualenv set up with the virtualenv package
    real_prefix = hasattr(sys, 'real_prefix')
    if real_prefix:
        return 1
    # If a virtualenv is set up with pyvenv, we check for equality of base_prefix and prefix
    if hasattr(sys, 'base_prefix'):
        return (sys.prefix != sys.base_prefix)
    # If none of the above conditions triggered, this is probably no virtualenv interpreter
    return 0

if __name__ == "__main__":
    if not inVEnv():
        print("")
    else:
        print(sys.prefix)
