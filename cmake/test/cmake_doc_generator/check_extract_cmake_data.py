#!/usr/bin/env python3
# SPDX-FileCopyrightInfo: Copyright © DUNE Project contributors, see file LICENSE.md in module root
# SPDX-License-Identifier: LicenseRef-GPL-2.0-only-with-DUNE-exception

from __future__ import annotations

import shutil
import subprocess
import sys
from pathlib import Path


def main() -> int:
    test_dir = Path(__file__).resolve().parent
    build_dir = test_dir / "_build"
    fixture = test_dir / "FixtureModernDocs.cmake"
    script = test_dir.parents[1] / "scripts" / "extract_cmake_data.py"
    command_script = test_dir.parents[1] / "scripts" / "generate_cmake_command_pages.py"

    if build_dir.exists():
        shutil.rmtree(build_dir)
    build_dir.mkdir(parents=True)

    subprocess.run(
        [sys.executable, str(script), "--module", str(fixture), "--builddir", str(build_dir)],
        check=True,
    )
    subprocess.run(
        [sys.executable, str(command_script), "--builddir", str(build_dir)],
        check=True,
    )

    module_rst = build_dir / "modules" / "FixtureModernDocs.rst"
    if not module_rst.exists():
        raise AssertionError(f"Expected extracted module file {module_rst} to exist")

    content = module_rst.read_text()

    expected_fragments = [
        "FixtureModernDocs",
        ".. cmake:command:: dune_fixture_public",
        ".. cmake:command:: dune_fixture_helper",
        ".. cmake:variable:: DUNE_FIXTURE_VARIABLE",
        ".. versionchanged:: 2.9",
        ":cmake:command:`dune_fixture_helper()`",
        ".. dune:internal::",
    ]
    for fragment in expected_fragments:
        if fragment not in content:
            raise AssertionError(f"Missing expected fragment in extracted module docs: {fragment!r}")

    public_command_rst = build_dir / "commands" / "dune_fixture_public.rst"
    internal_command_rst = build_dir / "commands" / "dune_fixture_helper.rst"
    variable_rst = build_dir / "variables" / "DUNE_FIXTURE_VARIABLE.rst"
    stamp = build_dir / "commands" / ".dune-public-commands.stamp"

    if not public_command_rst.exists():
        raise AssertionError(f"Expected public command page {public_command_rst} to exist")
    if internal_command_rst.exists():
        raise AssertionError(f"Did not expect internal command page {internal_command_rst} to exist")
    if not variable_rst.exists():
        raise AssertionError(f"Expected variable page {variable_rst} to exist")
    if not stamp.exists():
        raise AssertionError(f"Expected stamp file {stamp} to exist")

    public_content = public_command_rst.read_text()
    if "../modules/FixtureModernDocs" not in public_content:
        raise AssertionError("Expected generated public command page to link to the module page")
    variable_content = variable_rst.read_text()
    if ":cmake:variable:`DUNE_FIXTURE_VARIABLE`" not in variable_content:
        raise AssertionError("Expected generated variable page to link back through the cmake variable role")

    return 0


if __name__ == "__main__":
    raise SystemExit(main())
