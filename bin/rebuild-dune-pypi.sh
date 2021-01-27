#!/bin/bash

#
# This script builds a Python package index on gitlab.dune-project.org. Such an
# index is necessary as a drop-in replacement for PyPI in continuous integration,
# when runners operate with restricted network access.
#
# This script needs only to be run once by a core developer when something fundamental
# changed:
# * An additional Dune module contributes dependencies
# * A Dune module introduced new upstream dependencies
# * A new Python version came out and should be supported
#
# Running this script requires the following prerequisites to be met:
# * pyenv needs to be set up according to https://realpython.com/intro-to-pyenv/. Pyenv is
#   a tool that manages co-installation of multiple Python versions on one system
# * Make sure that all required versions of Python are installed with pyenv. These
#   are defined in the ALL_PYTHON_VERSIONS variable below.
# * Go to Gitlab Profile/Setting/Access Tokens and create a personal API access token with
#   at least the `write_registry` scope.
# * Export your token with `export TWINE_PASSWORD=<token>`
#

# This script exits upon errors
set -e

# Define the list of Python versions that we care about
ALL_PYTHON_VERSIONS="3.6.12 3.7.9 3.8.7 3.9.1"

# We authenticate with a personal Gitlab API token. You are expected to
# have set TWINE_PASSWORD to your API token when calling this script.
export TWINE_USERNAME=__token__

# Make sure that TWINE_PASSWORD was set
if [ -z "$TWINE_PASSWORD" ]
then
  echo "TWINE_PASSWORD was not set!"
  exit 1
fi

# Create a temporary directory as workspace for this script
TMPDIR=$(mktemp -d)
pushd $TMPDIR

# Clone the Dune modules that we gather Python dependencies from.
rm -rf dune
mkdir dune
cd dune
git clone https://gitlab.dune-project.org/quality/dune-testtools.git
git clone https://gitlab.dune-project.org/core/dune-common.git
git clone https://gitlab.dune-project.org/core/dune-geometry.git
git clone https://gitlab.dune-project.org/core/dune-grid.git
git clone https://gitlab.dune-project.org/core/dune-istl.git
git clone https://gitlab.dune-project.org/core/dune-localfunctions.git
cd ..

# Iterate over a range of Python versions to make sure that the index works
# for this range of Python versions. The script assumes that these versions
# are installed using pyenv, e.g. according to https://realpython.com/intro-to-pyenv/
for PYTHON_VERSION in $ALL_PYTHON_VERSIONS
do
  # Switch to the requested Python version using pyenv
  echo "Now running with python version $PYTHON_VERSION"
  pyenv virtualenv -f $PYTHON_VERSION pypi-mirror-$PYTHON_VERSION
  pyenv local pypi-mirror-$PYTHON_VERSION
  python -m pip install git+git://github.com/BraulioV/pip2pi.git

  # Collect the wheels of all dependencies for the Dune modules
  pip2pi packages ./dune/dune-testtools/python
  pip2pi packages ./dune/dune-common
  pip2pi packages ./dune/dune-geometry
  pip2pi packages ./dune/dune-grid
  pip2pi packages ./dune/dune-istl
  pip2pi packages ./dune/dune-localfunctions
done

# Upload the packages to the index
python -m pip install twine
for filename in packages/*.whl
do
  # NB: The 133 here is the Gitlab project ID of dune-common.
  python -m twine upload --skip-existing --repository-url https://gitlab.dune-project.org/api/v4/projects/133/packages/pypi $filename
done

# Clean up the temporary directory
popd
rm -rf $TMPDIR
