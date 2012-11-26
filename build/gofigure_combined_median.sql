select load_extension('/home/s062372/lib/libsqlitefunctions.so');
select
  operation,
  implementation,
  case when start_time between 1353616265959869788 and 1353617242303705709 then 'sequential' else 'randomized' end as 'usage pattern',
  count,
  median(duration/1e6) as 'avg dur (ms)',
  '',
  median(duration/1e6/count) as 'avg dur (ms)',
  '',
  sum(count)/(sum(duration)/1e9) as 'ops_per_sec',
  count(*) as 'rows'
from results
where
  start_time between 1353604497330383784 and 1353607611263695115
  or
  start_time between 1353616265959869788 and 1353617242303705709
group by
  operation,
  implementation,
  count
order by
  operation,
  `usage pattern` desc,
  implementation,
  count
;

