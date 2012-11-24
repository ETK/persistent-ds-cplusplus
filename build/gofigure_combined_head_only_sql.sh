#!/bin/bash

begin_seq=$(<begin_log10_sequential_head_only)
end_seq=$(<end_log10_sequential_head_only)
begin_rnd=$(<begin_log10_head_only)
end_rnd=$(<end_log10_head_only)

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
  implementation,
  count
;" | tr '.' ',' > 'gofigure_combined_head_only.csv'

