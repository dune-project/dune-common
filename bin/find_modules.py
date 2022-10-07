# SPDX-FileCopyrightInfo: Copyright © DUNE Project contributors, see file LICENSE.md in module root
# SPDX-License-Identifier: LicenseRef-GPL-2.0-only-with-DUNE-exception

try:
    from dune.common.module import resolve_dependencies, resolve_order, select_modules

except ImportError:
    import os
    here = os.path.dirname(os.path.abspath(__file__))
    mods = os.path.join(os.path.dirname(here), "python", "dune", "common")
    if os.path.exists(os.path.join(mods, "module.py")):
        import sys
        sys.path.append(mods)
        from module import resolve_dependencies, resolve_order, select_modules
    else:
        raise

print("Found Modules:")
print("--------------")
modules, _ = select_modules()
for description in modules.values():
    print(repr(description))
    print()

print()
print("Resolved Dependencies:")
print("----------------------")
deps = resolve_dependencies(modules)
for mod_name, mod_deps in deps.items():
    print(mod_name + ": " + " ".join(mod_deps))

print()
print("Build Order:")
print("------------")
print(" ".join(resolve_order(deps)))
