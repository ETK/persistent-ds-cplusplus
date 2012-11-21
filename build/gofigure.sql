select load_extension('/home/s062372/lib/libsqlitefunctions.so');
select
  operation,
  implementation,
  count,
  avg(duration/1e6) as 'avg dur (ms)',
  stdev(duration/1e6) as 'stdev of avg dur (ms)',
  avg(duration/1e6/count) as 'avg dur (ms)',
  stdev(duration/1e6/count) as 'stdev of avg dur (ms)',
  sum(count)/(sum(duration)/1e9) as 'ops_per_sec',
  count(*) as 'rows'
from results
where
  start_time between 1353509928113432817 and 1353516054899158595
group by
  operation,
  implementation,
  count
;

