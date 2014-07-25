#!/usr/bin/env bash

# wrapper for Google's cpplint.py
# see http://google-styleguide.googlecode.com/svn/trunk/cppguide.xml

# debug
# set -x

cpplint="cpplint"

assert_dependencies()
{
  # Try to execute cpplint
  "$cpplint" --filter= > /dev/null 2>&1 \
    || { echo >&2 "Cannot execute cpplint"; exit 1; }
  
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
  echo "       Wrapper for cpplint"
}

[ $# -eq 1 ] || { print_usage ; exit 0; }

assert_dependencies "$1"

path="$1/src"

# run cpplint on all cpp files 
# Some checks are disabled, since google's lint is very restrictive
# xargs echo -n: remove trailing newline from arguments
# This might break with filenames containing spaces
2>&1 "$cpplint" --filter=-whitespace,-legal,-build/header_guard,\
-build/include_order,-runtime/references,-readability/streams,-build/storage_class,-readability/namespace \
  $(find "$path" -name "*.h" -or -name "*.cpp" -or -name "*.hpp" | xargs echo -n)\
| grep -v ^Done

# return with cpplint's return code 
exit ${PIPESTATUS[0]}

# useful helper to print each line
# Testing/run_cpplint.sh . 2>&1 | grep -v ^Done | grep -v ^Total | \
# grep references | awk '{ print $1; }' | awk -F":" \
# '{ printf "sed -n -e "; printf $2; printf "p " ; print $1 }' \
# > reference_test.sh

