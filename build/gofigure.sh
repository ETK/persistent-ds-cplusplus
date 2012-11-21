date +%s%N > begin_log10
{ while kill -0 $(pidof msc); do sleep 0.5; kill -USR1 $(pidof msc); done } &
for k in 1 2 3 4 5 6 7 8 9 10
do
  echo "Run #${k}"
  for c in 1000 2327  5415  12599 29317 68219 158740  369375  859506  2000000
  do
    for i in -p -l
    do
      ./msc $i -s -c $c --randomize-operations
    done
  done
done
date +%s%N > end_log10
