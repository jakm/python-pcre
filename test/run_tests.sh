#!/bin/bash
export PYTHONPATH="$PWD/../src"
export LD_LIBRARY_PATH="/usr/local/lib"

for i in test_*.py; do
echo -en "\nTEST: $i "
python $i
echo
done
