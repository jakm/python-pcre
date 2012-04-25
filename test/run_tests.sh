#!/bin/bash

workdir=$(dirname $(which $0))
cd $workdir

export PYTHONPATH="$workdir/../src"
export LD_LIBRARY_PATH="/usr/local/lib"

for i in test_*.py; do
echo -en "\nTEST: $i\n"
python $i
echo
done
