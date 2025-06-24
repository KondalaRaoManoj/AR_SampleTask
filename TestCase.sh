#!/bin/bash

docker start ar_project_2_udp_sender_run_7c27a26d14c8 > /dev/null 2>&1
sleep 1 # a small delay to initialize the container
docker exec -i ar_project_2_udp_sender_run_7c27a26d14c8 ./ar_udp_sender <<EOF
1
Hello
2
Test
3
Automated
4
UDP
5
KondalaRao Manoj
6
Gutlapalli
STOP
EOF