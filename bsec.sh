#!/bin/bash
PATH=/usr/local/sbin:/usr/local/bin:/usr/bin:/usr/bin/site_perl:/usr/bin/vendor_perl:/usr/bin/core_perl
mkfifo /tmp/bsec
/usr/local/sbin/bsec > /tmp/bsec &
cat /tmp/bsec | /usr/local/sbin/mqtt &

