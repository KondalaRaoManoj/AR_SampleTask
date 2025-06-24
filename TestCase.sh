#!/bin/bash

docker start udp_sender > /dev/null 2>&1
sleep 1 # a small delay to initialize the container
docker exec -i udp_sender ./ar_udp_sender <<EOF
1
Hello
2
Test
3
Automated
4
UDP
5
KondalaRao Manoj3
6
Gutlapalli
STOP
EOF