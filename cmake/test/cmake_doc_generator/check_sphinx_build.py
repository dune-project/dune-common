#!/usr/bin/env python3
# SPDX-FileCopyrightInfo: Copyright © DUNE Project contributors, see file LICENSE.md in module root
# SPDX-License-Identifier: LicenseRef-GPL-2.0-only-with-DUNE-exception

from __future__ import annotations

import importlib.util
import shutil
import subprocess
import sys
from pathlib import Path


SKIP_RETURN_CODE = 77


def main() -> int:
    if importlib.util.find_spec("sphinx") is None:
        return SKIP_RETURN_CODE

    test_dir = Path(__file__).resolve().parent
    build_dir = test_dir / "_sphinx_fixture"
    fixture = test_dir / "FixtureModernDocs.cmake"
    scripts_dir = test_dir.parents[1] / "scripts"
    extract_script = scripts_dir / "extract_cmake_data.py"
    command_script = scripts_dir / "generate_cmake_command_pages.py"

    if build_dir.exists():
        shutil.rmtree(build_dir)
    build_dir.mkdir(parents=True)

    subprocess.run(
        [sys.executable, str(extract_script), "--module", str(fixture), "--builddir", str(build_dir)],
        check=True,
    )
    subprocess.run(
        [sys.executable, str(command_script), "--builddir", str(build_dir)],
        check=True,
    )

    conf_py = build_dir / "conf.py"
    conf_py.write_text(
        "\n".join([
            "import sys",
            f"sys.path.append({str(scripts_dir)!r})",
            "extensions = ['sphinx_cmake_dune', 'sphinx_moderncmakedomain']",
            "master_doc = 'index'",
            "project = 'dune-cmake-doc-fixture'",
            "exclude_patterns = ['_html', '_doctrees']",
            "html_theme = 'classic'",
            "",
        ])
    )

    index_rst = build_dir / "index.rst"
    index_rst.write_text(
        "\n".join([
            "Fixture Docs",
            "============",
            "",
            ".. toctree::",
            "   :maxdepth: 1",
            "   :glob:",
            "",
            "   commands/*",
            "   modules/*",
            "   variables/*",
            "",
        ])
    )

    html_dir = build_dir / "_html"
    doctree_dir = build_dir / "_doctrees"

    subprocess.run(
        [
            sys.executable,
            "-m",
            "sphinx",
            "-W",
            "-b",
            "html",
            "-d",
            str(doctree_dir),
            str(build_dir),
            str(html_dir),
        ],
        check=True,
    )

    expected = [
        html_dir / "index.html",
        html_dir / "modules" / "FixtureModernDocs.html",
        html_dir / "commands" / "dune_fixture_public.html",
        html_dir / "variables" / "DUNE_FIXTURE_VARIABLE.html",
    ]
    for path in expected:
        if not path.exists():
            raise AssertionError(f"Expected generated HTML file {path} to exist")

    internal_html = html_dir / "commands" / "dune_fixture_helper.html"
    if internal_html.exists():
        raise AssertionError(f"Did not expect internal command page {internal_html} to exist")

    module_html = (html_dir / "modules" / "FixtureModernDocs.html").read_text()
    if "dune_fixture_helper" not in module_html:
        raise AssertionError("Expected helper command reference to appear in rendered module page")
    if "https://cmake.org/cmake/help/latest/command/target_compile_definitions.html#command:target_compile_definitions" not in module_html:
        raise AssertionError("Expected upstream CMake command shortcut to render as an external link")

    return 0


if __name__ == "__main__":
    raise SystemExit(main())
