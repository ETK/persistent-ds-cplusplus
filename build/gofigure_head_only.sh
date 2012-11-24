date +%s%N > begin_log10
{ while kill -0 $(pidof msc); do sleep 0.25; kill -USR1 $(pidof msc); sleep 0.25; done } &
for c in 1000 2327  5415  12599 29317 68219 158740 369375  859506  2000000
do
  echo "Count: ${c}"
  for i in -p -l
  do
    echo "Implementation ${i}"
    for k in 1 2 3 4 5 6 7 8 9 10
    do
      echo "Run #${k}"
      ./msc $i -s -c $c -h --randomize-operations
    done
  done
done
date +%s%N > end_log10
