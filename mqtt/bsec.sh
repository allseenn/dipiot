#!/bin/bash
set -x
mkfifo /tmp/bsec
/usr/local/sbin/bsec > /tmp/bsec &
sleep 2
cat /tmp/bsec | /usr/local/sbin/mqtt &

