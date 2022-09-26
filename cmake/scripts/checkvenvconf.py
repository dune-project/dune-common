# SPDX-FileCopyrightInfo: Copyright © DUNE Project contributors, see file LICENSE.md in module root
# SPDX-License-Identifier: LicenseRef-GPL-2.0-only-with-DUNE-exception

import sys, os
try:
    from dune.__main__ import run
except ImportError:
    # calling before dune.common was installed - that is not an error since
    # build dir have to be okay
    sys.exit(0)

ret = run()
sys.exit(ret)
