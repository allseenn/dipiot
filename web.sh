#!/bin/bash
while true; do
    line = `cat /tmp/bsec`
    echo "$line"
    echo "$line" > /tmp/bsec
done

