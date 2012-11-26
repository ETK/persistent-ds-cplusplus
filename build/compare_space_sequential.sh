for c in 1000 2327  5415  12599 29317 68219 158740
do
  echo "Count: ${c}"
  for i in -r -o -l -p
  do
    echo "Implementation ${i}"
    ./msc $i -c $c -h
  done
done

