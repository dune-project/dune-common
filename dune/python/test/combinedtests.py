try:
    from dune.common.module import get_dune_py_dir
    _ = get_dune_py_dir()
    import test_class_export
except:
    pass
