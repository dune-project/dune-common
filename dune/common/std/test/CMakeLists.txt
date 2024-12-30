# SPDX-FileCopyrightInfo: Copyright © DUNE Project contributors, see file LICENSE.md in module root
# SPDX-License-Identifier: LicenseRef-GPL-2.0-only-with-DUNE-exception

# Link all test targets in this directory against Dune::Common
link_libraries(Dune::Common)

dune_add_test(SOURCES accessorstest.cc
              LABELS quick)

dune_add_test(SOURCES algorithmtest.cc
              LABELS quick)

dune_add_test(SOURCES comparetest.cc
              LABELS quick)

dune_add_test(SOURCES extentstest.cc
              LABELS quick)

dune_add_test(SOURCES iteratortest.cc
              LABELS quick)

dune_add_test(SOURCES mappingtest.cc
              LABELS quick)

dune_add_test(SOURCES mdspantest.cc
              LABELS quick)

dune_add_test(SOURCES mdarraytest.cc
              LABELS quick)

dune_add_test(SOURCES spantest.cc
              LABELS quick)