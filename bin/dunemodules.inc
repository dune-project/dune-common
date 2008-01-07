# -*-sh-*-

###############################################
###
### Configuration
###

# name of the "control" files
CONTROL="dune.module"

###############################################
###
### check for environment variables
###

if test -z $DUNE_CONTROL_PATH; then
  DUNE_CONTROL_PATH=.
fi
if test -z $GREP; then
  GREP=grep
fi

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
    echo "ERROR: could not read file $1" > /dev/stderr
    exit 1
  fi
  # read parameters from control file
  local name="$(echo $($GREP Module: "$1" | cut -d ':' -f2))"
  if test "x$name" = "x"; then
    echo "ERROR: $CONTROL files $1 does not contain a Module entry" > /dev/stderr
    exit 1
  fi
  local deps="$(echo $($GREP Depends: "$1" | cut -d ':' -f2))"
  local sugs="$(echo $($GREP Suggests: "$1" | cut -d ':' -f2))"
  local path="$(dirname "$1")"
  # create and check variable name from module name
  export module=$(fix_variable_name $name)
  if ! check_modname "$module"; then
    echo "ERROR: $CONTROL files $1 contains an invalid Module entry" > /dev/stderr
    exit 1
  fi
  # avoid multiple definition of the same module
  if test "x$(eval echo \$HAVE_$module)" != "x"; then
    echo "ERROR: multiple definition of module $name" > /dev/stderr
    echo "previous defined in:" > /dev/stderr
    echo "  $(eval echo \$PATH_$module)/$CONTROL" > /dev/stderr
    echo "redefined in:" > /dev/stderr
    echo "  $path/$CONTROL" > /dev/stderr
    exit 1
  fi
  # set status variables
  export HAVE_${module}=yes
  export PATH_${module}="$path"
  export NAME_${module}="$name"
  export DEPS_${module}="$deps"
  for name in $deps; do
    mod=$(fix_variable_name $name)
    export NAME_${mod}="$name"
  done
  export SUGS_${module}="$sugs"
  for name in $sugs; do
    mod=$(fix_variable_name $name)
    export NAME_${mod}="$name"
  done
}

#
# search for modules in each directory in DUNE_CONTROL_PATH
#
find_modules_in_path() {
  if test -z "$FOUND_MODULES"; then
    # foreach dir in $@
    while read dir; do
      find_modules $dir
    done <<EOF
    $(echo $DUNE_CONTROL_PATH | sed -e 's/:/\n/g') 
EOF
  fi
  export FOUND_MODULES=$MODULES
}

#
# search a directory recursively for $CONTROL files
#
# paramters:
# $1 directory to search for modules
#
find_modules() {
  if test -d "$1"; then
    local dir="$(cd "$1" && pwd)"
    while read m; do
      if test "x$m" != "x"; then
        export module=""
        parse_control "$m"
        export MODULES="$MODULES $module"
      fi
    done <<EOF
 $(find "$dir" -name $CONTROL | $GREP -v 'dune-[-_a-zA-Z]/dune\-[-a-zA-Z_]*\-[0-9]\+.[0-9]\+/')
EOF
  else
    if test -f "$1" && 
       test "$(basename $1)" = "$CONTROL"; then
      export module=""
      parse_control "$1"
      export MODULES="$MODULES $module"
    else
      echo "ERROR: '$1' is neither a directory nor a $CONTROL file" > /dev/stderr
      false
    fi
  fi
}

#
# sort $MODULES according to the dependencies
#
sort_modules() {
  # reset list
  export SORTEDMODULES=""
  # handle each modules passed as parameter
  for m in "$@"; do
    # did we find a module file for this mopdule?
    if test "x$(eval echo \$HAVE_$m)" != "x"; then
      _sort_module $m
    else
      echo "ERROR: could not find module $(eval echo \$NAME_$m)" > /dev/stderr
      exit 1
    fi
  done
  # save result
  export MODULES="$SORTEDMODULES"
  # clean up temporary variables
  for m in $MODULES; do
    export SORT_DONE_${m}=""
  done
}

#
# sort $MODULES's dependencies according to the dependencies
# don't include $MODULE itself
#
sort_dependecies() {
  # reset list
  export SORTEDMODULES=""
  # handle each modules passed as parameter
  for m in "$@"; do
    # did we find a module file for this mopdule?
    if test "x$(eval echo \$HAVE_$m)" != "x"; then
	  if test "x$(eval echo \$DEPS_$m)" != "x" ; then
        for name in $(eval echo \$DEPS_$m); do
          dep=$(fix_variable_name $name)
          _sort_module $dep
        done
      fi
    else
      echo "ERROR: could not find module $(eval echo \$NAME_$m)" > /dev/stderr
      exit 1
    fi
  done
  # save result
  export MODULES="$SORTEDMODULES"
  # clean up temporary variables
  for m in $MODULES; do
    export SORT_DONE_${m}=""
  done
}

#
# recursive part of sort_modules
# evaluate dependencies of one module
#
# paramters:
# $1 name of the modules
#
_sort_module() {
  local module="$1"
  shift 1
  if ! check_modname $module; then 
    echo "ERROR: invalid module name $module" > /dev/stderr
    exit 1
  fi
  if test "x$(eval echo \$SORT_DONE_${module})" != "xyes"; then
    # resolve dependencies
    for name in $(eval "echo \$DEPS_$module"); do
	  dep=$(fix_variable_name $name)
      if ! check_modname $dep; then 
        echo "ERROR: invalid module name $name" > /dev/stderr
        exit 1
      fi
      if test "x$(eval echo \$HAVE_$dep)" != "x"; then
        _sort_module $dep
      else
        # perhaps this module is installed,
        # then it should be handled via pkg-config
        if ! pkg-config $name; then
          echo "ERROR: could not find module $(eval echo \$NAME_$dep)" > /dev/stderr
          echo "       module is also unknown to pkg-config" > /dev/stderr
          echo "       Maybe you need to adjust PKG_CONFIG_PATH!" > /dev/stderr
          echo "       $(eval echo \$NAME_$dep) is required by $(eval echo \$NAME_$module)" > /dev/stderr
          exit 1
        fi
      fi
    done
    # resolve suggestions
    for name in $(eval "echo \$SUGS_$module"); do
	  dep=$(fix_variable_name $name)
      if ! check_modname $dep; then
        echo "ERROR: invalid module name $name" > /dev/stderr
        exit 1
      fi
      if test "x$(eval echo \$HAVE_$dep)" != "x"; then
        _sort_module $dep
      fi
    done
    # insert this module into the list
    export SORT_DONE_${module}=yes
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
  local command="$1"
  local file="$2"
  shift 2
  if test -f "$file"; then
    # open subshell
    (
      set -e
      # load functions defined in $file
      # if $command is not defined in $file,
      # then the default implementation will be executed
      eval "$($GREP -v '^[[:alnum:]]\+:' $file)"
      # execute $command
      $command
    ) || false
  else
    echo "ERROR: could not find $file" > /dev/stderr
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
fix_variable_name() {
  echo -n "$@" | tr '[:punct:]' '__'
}

fix_and_assign() {
  local name="$1"
  if ! check_modname $name; then
    echo "ERROR: error in assignment. $name is not a valid variabel name." > /dev/stderr
  fi
  shift 1
  export $name=$(fix_variable_name $@)
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
#  if ! echo "$1" | $GREP -q '^[a-zA-Z0-9_]\+$'; then
#    return 1
#  fi
  return 0
}
