#!/bin/bash

for c in 1000 2327  5415  12599 29317 68219 158740
do
  echo "Count: ${c}"
  for i in -e -n
  do
    echo "Implementation ${i}"
    for r in 1 2 3 4 5 6 7 8 9 10
    do
      echo "Run #${r}"
      ./msc $i -c $c --seed $(date +%N)
    done
  done
done

