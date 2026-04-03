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


def iter_command_blocks(lines):
    current = None
    body = []
    for line in lines:
        if line.startswith(".. cmake:command:: "):
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


def write_command_page(commands_dir, command, module_name, module_doc_title):
    command_path = commands_dir / f"{command}.rst"
    command_path.write_text(
        "\n".join([
            command,
            "=" * len(command),
            "",
            f"This command is documented in :doc:`../modules/{module_name}`.",
            "",
            f"See :cmake:command:`{command}` in the module page {module_doc_title}.",
            "",
        ])
    )


def main():
    args = get_args()
    builddir = Path(args.builddir)
    modules_dir = builddir / "modules"
    commands_dir = builddir / "commands"
    commands_dir.mkdir(parents=True, exist_ok=True)

    generated = []
    if modules_dir.exists():
        for module_rst in sorted(modules_dir.glob("*.rst")):
            module_name = module_rst.stem
            title = module_title(module_rst)
            lines = module_rst.read_text().splitlines()
            for command, body in iter_command_blocks(lines):
                if is_internal(body):
                    continue
                write_command_page(commands_dir, command, module_name, title)
                generated.append(command)

    stamp = builddir / "commands" / ".dune-public-commands.stamp"
    stamp.write_text("\n".join(sorted(generated)) + ("\n" if generated else ""))


if __name__ == "__main__":
    main()
