# SPDX-FileCopyrightInfo: Copyright © DUNE Project contributors, see file LICENSE.md in module root
# SPDX-License-Identifier: LicenseRef-GPL-2.0-only-with-DUNE-exception

import sys
import logging

logger = logging.getLogger(__name__)

if "dune.citations" in sys.modules:
    logger.warning('`dune.citations` was imported before, ignoring `dune.citationsfile` and not writing file!')
else:
    import dune.citations as citations
    from pathlib import Path

    # set '_bibFile' to a file name to get the output printed into that file instead of stdout
    citations._bibFile = Path(sys.argv[0]).resolve().name + ".bib"

    # use show function from citations
    show = citations.show
