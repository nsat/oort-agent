#!/bin/sh

/bin/echo '{"version": 1, "schema_version": 1, "payload_timestamp": 1628760852, "need_hard_reset": false, "int_metrics":['
for j in $(seq 255); do
    /bin/echo "$j, "
done
/bin/echo '256], "float_metrics": [0.2, 0.3, 0.4, 2.86], "int_metric_counts":[4, 5, 6], "float_metric_counts":[10,11,24,32]}'

exit 0
