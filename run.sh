#!/bin/bash

build_type=$1

echo "build_type = $build_type"

echo "Setting build options"
time (cmake -DCMAKE_BUILD_TYPE="$build_type" -S . -B build-"$build_type" && echo "Building $build_type" && cmake --build build-"$build_type")

echo "Running"
script -c "time (build-"$build_type"/Cryptolyser_Attacker $2 $3 $4)" run_"$4".log
