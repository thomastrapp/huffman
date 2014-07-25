#!/usr/bin/env bash

# wrapper for cppcheck
# see http://cppcheck.sourceforge.net/

# debug
# set -x

cppcheck="/usr/bin/cppcheck"
# red
color_highlight=$(echo -n -e "\033[0;31m")
# reset color
color_end=$(echo -n -e "\033[0m")

assert_dependencies()
{
  # Try to execute cpp check
  "$cppcheck" --help > /dev/null 2>&1 \
    || { echo >&2 "Cannot execute cppcheck"; exit 1; }
 
  # check if a directory was passed
  [ -d "$1" ] \
    || { echo >&2 "Project root not found"; exit 1; } 

  # check if src directory exists
  [ -d "$1/src" ] \
    || { echo >&2 "src directory not found"; exit 1; } 
}

print_usage()
{
  echo "Usage:" "$0" "[path to project root]"
  echo "       Wrapper for cppcheck"
}

[ $# -eq 1 ] || { print_usage ; exit 0; }

assert_dependencies $1

path="$1/src"

# run cppcheck on all cpp files, enable all checks, use $path as base directory 
# for includes 
# xargs echo -n: remove trailing newline from arguments
# This might break with filenames containing spaces
"$cppcheck" -I "$path" --quiet --enable=all --error-exitcode=1 \
  $(find "$path" -name "*.h" -or -name "*.cpp" -or -name "*.hpp" | \
    xargs echo -n)\
    2>&1 | sed "s/\[[^]]*\]/${color_highlight}&${color_end}/"

# return with cppcheck's return code 
exit ${PIPESTATUS[0]}

