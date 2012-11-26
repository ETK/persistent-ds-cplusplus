#!/bin/bash

begin_seq=$(<begin_log10_sequential_head_only)
end_seq=$(<end_log10_sequential_head_only)
begin_rnd=$(<begin_log10_head_only)
end_rnd=$(<end_log10_head_only)

sqlite3 -header -csv -separator ';' sqlite.db \
"select load_extension('${HOME}/lib/libsqlitefunctions.so');
select
  implementation,
  case when start_time between ${begin_seq} and ${end_seq} then 'sequential' else 'randomized' end as 'usage pattern',
  case
    when count <    500 then    250
    when count <   1300 then    584
    when count <   3100 then   1354
    when count <   7300 then   3150
    when count <  17000 then   7330
    when count <  39600 then  17055
    when count <  92000 then  39685
    when count < 214000 then  92344
    when count < 499600 then 214877
    else                     500000
  end,
  sum(duration/1e6) as 'sum dur (ms)',
--  1.96*stdev(duration/1e6) as '1.96*stdev of avg dur (ms)',
  sum(duration/1e6)/sum(count) as 'sum dur (ms)/sum count',
--  1.96*stdev(duration/1e6/count) as '1.96*stdev of avg dur (ms)',
--  sum(count)/(sum(duration)/1e9) as 'ops_per_sec',
  count(*) as 'rows'
from results
where
  start_time between ${begin_rnd} and ${end_rnd}
  or
  start_time between ${begin_seq} and ${end_seq}
group by
  implementation,
  case when start_time between ${begin_seq} and ${end_seq} then 'sequential' else 'randomized' end,
  case
    when count <    500 then    250
    when count <   1300 then    584
    when count <   3100 then   1354
    when count <   7300 then   3150
    when count <  17000 then   7330
    when count <  39600 then  17055
    when count <  92000 then  39685
    when count < 214000 then  92344
    when count < 499600 then 214877
    else                     500000
  end
order by
  \`usage pattern\` desc,
  implementation,
  count
;" | tr '.' ',' > 'overall.csv'

