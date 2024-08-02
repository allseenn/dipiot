#!/bin/bash
mkfifo /tmp/bsec
/usr/local/sbin/bsec > /tmp/bsec &
cat /tmp/bsec > /usr/local/sbin/mqtt &

