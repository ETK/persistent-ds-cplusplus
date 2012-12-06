#!/bin/bash

begin_seq=$(<begin_compare_rollbacks_sequential)
end_seq=$(<end_compare_rollbacks_sequential)
begin_rnd=$(<begin_compare_rollbacks)
end_rnd=$(<end_compare_rollbacks)

sqlite3 -header -csv -separator ';' sqlite.db \
"select load_extension('${HOME}/lib/libsqlitefunctions.so');
select
  operation,
  implementation,
  case when start_time between ${begin_seq} and ${end_seq} then 'sequential' else 'randomized' end as 'usage pattern',
  count,
  avg(duration/1e6) as 'avg dur (ms)',
  1.96*stdev(duration/1e6) as '1.96*stdev of avg dur (ms)',
  avg(duration/1e6/count) as 'avg dur (ms)',
  1.96*stdev(duration/1e6/count) as '1.96*stdev of avg dur (ms)',
  sum(count)/(sum(duration)/1e9) as 'ops_per_sec',
  count(*) as 'rows'
from results
where
  start_time between ${begin_rnd} and ${end_rnd}
  or
  start_time between ${begin_seq} and ${end_seq}
group by
  operation,
  implementation,
  count
order by
  operation,
  \`usage pattern\` desc,
  case
    when implementation like 'node%' then 1
    when implementation like 'black%' then 2
    when implementation like 'elim%' then 3
    else 4
  end,
  count
;" | tr '.' ',' > 'compare_rollbacks_combined.csv'
