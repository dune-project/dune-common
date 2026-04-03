#!/usr/bin/env python3
# SPDX-FileCopyrightInfo: Copyright © DUNE Project contributors, see file LICENSE.md in module root
# SPDX-License-Identifier: LicenseRef-GPL-2.0-only-with-DUNE-exception

from __future__ import annotations

import argparse
from pathlib import Path


def get_args():
    parser = argparse.ArgumentParser()
    parser.add_argument("-b", "--builddir", required=True,
                        help="Build directory containing extracted module rst files")
    return parser.parse_args()


def iter_object_blocks(lines, directive):
    current = None
    body = []
    for line in lines:
        if line.startswith(f".. cmake:{directive}:: "):
            if current is not None:
                yield current, body
            current = line.split(":: ", 1)[1].strip()
            body = []
        elif current is not None:
            body.append(line)
    if current is not None:
        yield current, body


def is_internal(body):
    return any(line.strip() == ".. dune:internal::" for line in body)


def module_title(module_rst):
    for line in module_rst.read_text().splitlines():
        if line.strip():
            return line.strip()
    return module_rst.stem


def write_object_page(output_dir, objname, objkind, module_name, module_doc_title):
    object_path = output_dir / f"{objname}.rst"
    object_path.write_text(
        "\n".join([
            objname,
            "=" * len(objname),
            "",
            f"This {objkind} is documented in :doc:`../modules/{module_name}`.",
            "",
            f"See :cmake:{objkind}:`{objname}` in the module page {module_doc_title}.",
            "",
        ])
    )


def main():
    args = get_args()
    builddir = Path(args.builddir)
    modules_dir = builddir / "modules"
    commands_dir = builddir / "commands"
    variables_dir = builddir / "variables"
    commands_dir.mkdir(parents=True, exist_ok=True)
    variables_dir.mkdir(parents=True, exist_ok=True)

    generated = []
    if modules_dir.exists():
        for module_rst in sorted(modules_dir.glob("*.rst")):
            module_name = module_rst.stem
            title = module_title(module_rst)
            lines = module_rst.read_text().splitlines()
            for command, body in iter_object_blocks(lines, "command"):
                if is_internal(body):
                    continue
                write_object_page(commands_dir, command, "command", module_name, title)
                generated.append(command)
            for variable, _body in iter_object_blocks(lines, "variable"):
                write_object_page(variables_dir, variable, "variable", module_name, title)

    stamp = builddir / "commands" / ".dune-public-commands.stamp"
    stamp.write_text("\n".join(sorted(generated)) + ("\n" if generated else ""))


if __name__ == "__main__":
    main()
