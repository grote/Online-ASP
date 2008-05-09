#!/bin/bash

# very simple script to check the grounder against lparse/smodels

n=1
echo "clasp says: "
echo -n "compute{" > solution.lp
gringo2 -l $* | clasp -n $n | tail -n 4 | head -n 1 | tr '\n' '}' | sed -e 's/ /,/g' | sed -e 's/,}/}./g'  1>> solution.lp
#lparse $* | clasp -n $n | tail -n 4 | head -n 1 | tr '\n' '}' | sed -e 's/) /),/g' | sed -e 's/,}/}./g'  1>> solution.lp
echo "smodels says: "
lparse --true-negation $* solution.lp | smodels
