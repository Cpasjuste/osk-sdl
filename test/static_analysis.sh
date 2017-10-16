#!/bin/bash
set -e

files=$(find . -name "*.cpp")
out=$(mktemp)
cppcheck_enable="style,warning,performance,unusedFunction"
cppcheck_opts="--template=gcc --std=c++14 --language=c++ --enable=${cppcheck_enable} "

function oops(){
  echo "*************************************************"
  echo "Found an issue with the source code! Please check"
  echo "the output from cppcheck and resolve all warnings"
  echo "and errors!"
  echo "*************************************************"
  rm $out
  exit 1
}

cppcheck $cppcheck_opts ${files} 2>&1 |tee ${out}

# Check output for any errors or warnings
regex=$(echo $cppcheck_enable,error | sed 's/,/|/g')
grep -q -E "$regex" $out && oops

rm $out
exit 0
