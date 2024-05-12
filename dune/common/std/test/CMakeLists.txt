# SPDX-FileCopyrightInfo: Copyright © DUNE Project contributors, see file LICENSE.md in module root
# SPDX-License-Identifier: LicenseRef-GPL-2.0-only-with-DUNE-exception

dune_add_test(SOURCES accessorstest.cc
              LABELS quick
              LINK_LIBRARIES Dune::Common)

dune_add_test(SOURCES extentstest.cc
              LABELS quick
              LINK_LIBRARIES Dune::Common)

dune_add_test(SOURCES mappingtest.cc
              LABELS quick
              LINK_LIBRARIES Dune::Common)

dune_add_test(SOURCES mdspantest.cc
              LABELS quick
              LINK_LIBRARIES Dune::Common)

dune_add_test(SOURCES mdarraytest.cc
              LABELS quick
              LINK_LIBRARIES Dune::Common)

dune_add_test(SOURCES spantest.cc
              LABELS quick
              LINK_LIBRARIES Dune::Common)