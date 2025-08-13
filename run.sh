#!/bin/bash
# unofficial bash strict mode
set -euo pipefail
IFS=$'\n\t'

project="Cryptolyser_Attacker"

build_type="release"
build_only=false
job_count="12"

Help()
{
   echo "Builds and runs the project."
   echo
   echo "Syntax: run.sh [-h|b|B|j] [ARGS]"
   echo "Options:"
   echo "    -h     Prints this help page."
   echo "    -b     Sets the build type (debug/release/relwithdebinfo/minsizerel)."
   echo "           DEFAULT: release"
   echo "    -B     Build only, do not run program."
   echo "    -j     Sets the thread job count."
   echo "           DEFAULT: 12"
   echo "ARGS: run program with given arguments"
}

while getopts ":hb:Bj:" option; do
   case $option in
      h) # help
         Help
         exit;;
      b) # build type
         build_type=$OPTARG;;
      B) # build only
         build_only=true;;
      j) # job count
         job_count=$OPTARG;;
     \?) # Invalid option
         echo "Error: Invalid option"
         exit;;
   esac
done
shift $(expr $OPTIND - 1 ) # jump over all parsed arguments

if ! [[ "$build_type" =~ ^(debug|relwithdebinfo|minsizerel)$ ]]
then
    build_type="release"
fi

echo "build_type = $build_type"

build_dir="$(dirname $0)/build_$build_type"
echo "build_dir = $build_dir"

echo "Setting build options"
time (cmake  -DCMAKE_BUILD_TYPE="$build_type" -S . -B "$build_dir" \
    && echo "Building $build_type" && cmake --build "$build_dir" -j "$job_count") &&

if [ "$build_only" = false ]
then
    echo "Running $build_dir/$project" "$@"
    "$build_dir/$project" "$@"
fi
