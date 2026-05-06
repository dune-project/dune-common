# SPDX-FileCopyrightInfo: Copyright © DUNE Project contributors, see file LICENSE.md in module root
# SPDX-License-Identifier: LicenseRef-GPL-2.0-only-with-DUNE-exception

import sys
import dune.citations as citations
from pathlib import Path

# set 'bibFile' to a file name to get the output printed into that file instead of stdout
citations.bibFile = Path(sys.argv[0]).resolve().name + ".bib"
