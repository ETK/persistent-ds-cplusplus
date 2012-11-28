for c in 1000 2327  5415  12599 29317 68219 158740 369375  859506  2000000
do
#  echo "Count: ${c}"
  for i in -l -p
  do
#    echo "Implementation ${i}"
    ./msc $i -c $c --randomize-operations
  done
done

