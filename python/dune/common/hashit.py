# SPDX-FileCopyrightInfo: Copyright © DUNE Project contributors, see file LICENSE.md in module root
# SPDX-License-Identifier: LicenseRef-GPL-2.0-only-with-DUNE-exception

import hashlib

def hashIt(typeName):
    if hasattr(typeName, '__iter__'):
        return hashlib.md5("".join(t for t in typeName).encode('utf-8')).hexdigest()
    else:
        return hashlib.md5(typeName.encode('utf-8')).hexdigest()
