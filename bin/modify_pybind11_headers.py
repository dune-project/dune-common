#!/usr/bin/env python3
# SPDX-FileCopyrightInfo: Copyright © DUNE Project contributors, see file LICENSE.md in module root
# SPDX-License-Identifier: LicenseRef-GPL-2.0-only-with-DUNE-exception

"""Wrap vendored pybind11 headers with DUNE_USE_SYSTEM_PYBIND11 guards."""

from __future__ import annotations

import argparse
from pathlib import Path
import re

PRAGMA_RE = re.compile(r"^#if DUNE_USE_SYSTEM_PYBIND11\s*$", re.MULTILINE)


def transform_header(path: Path, root: Path) -> tuple[bool, str, str | None]:
    text = path.read_text(encoding="utf-8")

    if "#ifdef DUNE_USE_SYSTEM_PYBIND11" in text and CLOSE_GUARD in text:
        return False, "already_wrapped", None

    match = PRAGMA_RE.search(text)
    if not match:
        return False, "missing_if_dune_use_system_pybind11", None

    rel = path.relative_to(root).as_posix()
    replacement = (
        "#ifdef DUNE_USE_SYSTEM_PYBIND11"
    )
    updated = PRAGMA_RE.sub(replacement, text, count=1)

    return True, "updated", updated


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument(
        "--root",
        type=Path,
        default=Path("dune/python/pybind11"),
        help="Root directory containing pybind11 headers",
    )
    parser.add_argument(
        "--write",
        action="store_true",
        help="Write changes to disk (default is dry run)",
    )
    args = parser.parse_args()

    root = args.root.resolve()
    headers = sorted(root.rglob("*.h"))

    if not headers:
        print(f"No .h files found under {root}")
        return 1

    updated_count = 0
    skipped_count = 0

    for header in headers:
        changed, reason, updated_content = transform_header(header, root)
        if changed:
            updated_count += 1
            print(f"UPDATE {header}")
            if args.write:
                header.write_text(updated_content, encoding="utf-8")
        else:
            skipped_count += 1
            print(f"SKIP   {header} ({reason})")

    mode = "write" if args.write else "dry-run"
    print(f"\nMode: {mode}")
    print(f"Updated: {updated_count}")
    print(f"Skipped: {skipped_count}")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
