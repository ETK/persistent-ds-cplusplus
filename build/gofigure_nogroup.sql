select load_extension('/home/s062372/lib/libsqlitefunctions.so');
select
  operation,
  implementation,
  count,
  duration/1e3 as 'dur (µs)',
  duration/1e3/count as 'duration (µs) per op',
  count/(duration/1e6) as 'ops per ms'
from results
where
  start_time between 1353509928113432817 and 1353516054899158595
order by
  operation,
  implementation,
  count,
  start_time
;

