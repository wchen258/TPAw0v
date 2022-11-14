#!/bin/bash

TEST=$1

if [ -z $TEST ]; then
    echo "Specify the test to run"; exit 0
fi

if [ $TEST == "graph" ]; then
    APP=("disparity" "mser" "sift" "tracking" "texture_synthesis" "svm" "multi_ncut")
    for app in ${APP[@]}; do
        python3 lean_cfg.py --graph $app
    done
fi

if [ $TEST == "disparity" ]; then
    python3 lean_cfg.py --graph disparity
fi