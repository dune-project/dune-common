# -*-sh-*-

###############################################
###
### Configuration
###

# name of the "control" files
CONTROL="dune.module"

###############################################


#
# read paramters from a $CONTROL file
#
# paramters:
# $1 file to read
#
parse_control() {
  # check file existence
  if ! test -f "$1"; then
    echo "ERROR: could not read file $1"
    exit 1
  fi
  # read parameters from control file
  local name=$(echo $(grep Module: "$1" | cut -d ':' -f2))
  if test "x$name" == "x"; then
    echo "ERROR: $CONTROL files $1 does not contain a Module entry"
    exit 1
  fi
  local deps=$(echo $(grep Depends: "$1" | cut -d ':' -f2))
  local sugs=$(echo $(grep Suggests: "$1" | cut -d ':' -f2))
  local path=$(dirname "$1")
  # create and check variable name from module name
  fix_and_assign module $name
  if ! check_modname "$module"; then
    echo "ERROR: $CONTROL files $1 contains an invalid Module entry"
    exit 1
  fi
  # avoid multiple definition of the same module
  if test "x$(eval echo \$HAVE_$module)" != "x"; then
    echo "ERROR: multiple definition of module $name"
    echo "previous defined in:"
    echo "  $(eval echo \$PATH_$module)/$CONTROL"
    echo "redefined in:"
    echo "  $path/$CONTROL"
    exit 1
  fi
  # set status variables
  export HAVE_${module}=yes
  export PATH_${module}="$path"
  export NAME_${module}="$name"
  fix_and_assign DEPS_${module} "$deps"
  fix_and_assign SUGS_${module} "$sugs"
}

#
# search for modules in each directory in DUNE_CONTROL_PATH
#
find_modules_in_path() {
  # foreach dir in $@
  while read dir; do
    find_modules $dir
  done <<EOF
  $(echo ${DUNE_CONTROL_PATH:-.} | sed -e 's/:/\n/g') 
EOF
}

#
# search a directory recursively for $CONTROL files
#
# paramters:
# $1 directory to search for modules
#
find_modules() {
  if test -d "$1"; then
    local dir=$(cd "$1" && pwd)
    while read m; do
      if test "x$m" != "x"; then
        export module=""
        parse_control "$m"
        export MODULES="$MODULES $module"
      fi
    done <<EOF
 $(find "$dir" -name $CONTROL )
EOF
  else
    echo "ERROR: could not find directory $1"
    false
  fi
}

#
# sort $MODULES according to the dependencies
#
sort_modules() {
  for m in "$@"; do
    if test "x$(eval echo \$HAVE_$m)" != "x"; then
      _sort_module $m
    else
      echo "ERROR: could not find module $dep"
      exit 1
    fi
  done
  if test x"$ONLY" != x; then
	SORTEDMODULES=$ONLY
  fi
  export MODULES=$SORTEDMODULES
}

#
# recursive part of sort_modules
# evaluate dependencies of one module
#
# paramters:
# $1 name of the modules
#
_sort_module() {
  local module=$1
  shift 1
  if ! check_modname $module; then echo "ERROR: invalid module name $module"; exit 1; fi
  if test "x$(eval echo \$SORT_DONE_${command}_${module})" != "xyes"; then
    # resolve dependencies
    for dep in $(eval "echo \$DEPS_$module"); do
      if ! check_modname $dep; then echo "ERROR: invalid module name $dep"; exit 1; fi
      if test "x$(eval echo \$HAVE_$dep)" != "x"; then
        _sort_module $dep
      else
        echo "ERROR: could not find module $dep"
        exit 1
      fi
    done
    # resolve suggestions
    for dep in $(eval "echo \$SUGS_$module"); do
      if ! check_modname $dep; then echo "ERROR: invalid module name $dep"; exit 1; fi
      if test "x$(eval echo \$HAVE_$dep)" != "x"; then
        _sort_module $dep
      fi
    done
    # insert this module into the list
    export SORT_DONE_${command}_${module}=yes
    export SORTEDMODULES="$SORTEDMODULES $module"
  fi
}

#
# load the $CONTROL file, skip all control variables
# and run a command
#
# parameters:
# $1 command to execute
# $2 full path of the $CONTROL file
#
eval_control() {
  local command=$1
  local file=$2
  shift 2
  if test -f "$file"; then
    # open subshell
    (
      set -e
      # load functions defined in $file
      # if $command is not defined in $file,
      # then the default implementation will be executed
      eval "$(grep -v -e '^[[:alnum:]]\+:' $file)"
      # execute $command
      $command
    ) || false
  else
    echo "ERROR: could not find $file"
    exit 1
  fi
}

#
# fix a value such that it is suitable for a variable name and assign it
#
# parameters:
# $1 name of variable
# $2 value
#
fix_and_assign() {
  local name=$1
  shift 1
  if ! check_modname $name; then
    echo ERROR: error in assignment. $name is not a valid variabel name.
  fi
  export $name="$(echo "$@" | sed -e 's/-/_/g')"
}

#
# make sure the module name fits the naming convention
#
# parameters:
# $1 module name
#
check_modname() {
  if sh -c "$ID=huhu" > /dev/null 2>&1; then
    return 1
  fi
#  if ! echo "$1" | grep -q '^[a-zA-Z0-9_]\+$'; then
#    return 1
#  fi
  return 0
}
