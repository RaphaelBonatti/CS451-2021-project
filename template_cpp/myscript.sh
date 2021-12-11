#!/bin/bash

while getopts b:i:v: flag
do
    case "${flag}" in
        b) build=${OPTARG};;
        i) id=${OPTARG};;
        v) valgrind=${OPTARG};;
    esac
done

if [ $build -gt 0 ]; then
./build.sh
fi

if [ $valgrind -gt 0 ]; then
valgrind --leak-check=full ./bin/da_proc --id $id --hosts ../example/hosts --output ../example/output/$id.output ../example/configs/lcausal-broadcast.config;
else
./bin/da_proc --id $id --hosts ../example/hosts --output ../example/output/$id.output ../example/configs/lcausal-broadcast.config;
fi