#!/bin/bash
# SPDX-FileCopyrightInfo: Copyright © DUNE Project contributors, see file LICENSE.md in module root
# SPDX-License-Identifier: LicenseRef-GPL-2.0-only-with-DUNE-exception
#
# This script builds a Python package index on gitlab.dune-project.org. Such an
# index is necessary as a drop-in replacement for PyPI in continuous integration,
# when runners operate with restricted network access.
#
# Running this script requires the following prerequisites to be met:
# * Go to Gitlab Profile/Setting/Access Tokens and create a personal API access token with
#   at least the `write_registry` and 'api' scope.
# * Export your token with `export TWINE_PASSWORD=<token>`
#

# This script exits upon errors
set -e

# We authenticate with a personal Gitlab API token. You are expected to
# have set TWINE_PASSWORD to your API token when calling this script.
export TWINE_USERNAME=__token__

# Make sure that TWINE_PASSWORD was set
# export TWINE_PASSWORD=...
if [ -z "$TWINE_PASSWORD" ]
then
  echo "TWINE_PASSWORD was not set!"
  exit 1
fi

# Create a temporary directory as workspace for this script
TMPDIR=$(mktemp -d)
pushd $TMPDIR

python3 -m venv env
source env/bin/activate
python -m pip install pip-download twine

pip-download -d $(pwd)/downloads \
  # dune-common \
  # dune-geometry \
  # dune-grid \
  # dune-istl \
  # dune-localfunctions \
  # dune-alugrid \
  # dune-fem \
  pyparsing \
  mpi4py \
  wheel \
  setuptools \
  jinja2 \
  portalocker \
  fenics-ufl==2019.1.0 \
  matplotlib \
  scipy \
  pip>=21 \
  ninja \
  sortedcontainers


# Upload the packages to the index
for filename in downloads/*
do
  # NB: The 133 here is the Gitlab project ID of dune-common.
  python -m twine upload --verbose --skip-existing --repository-url https://gitlab.dune-project.org/api/v4/projects/133/packages/pypi $filename
done

# Clean up the temporary directory
popd
rm -rf $TMPDIR
