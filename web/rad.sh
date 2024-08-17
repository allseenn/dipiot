#!/bin/bash

while true; do
    byte1=$(i2cget -y 1 0x66 0x03)
    byte2=$(i2cget -y 1 0x66 0x04)
    byte3=$(i2cget -y 1 0x66 0x05)
    value=$(( (byte1 << 16) | (byte2 << 8) | byte3 ))
    dinamic=$((value / 10))
    
    byte1=$(i2cget -y 1 0x66 0x06)
    byte2=$(i2cget -y 1 0x66 0x07)
    byte3=$(i2cget -y 1 0x66 0x08)
    value=$(( (byte1 << 16) | (byte2 << 8) | byte3 ))
    static=$((value / 10))
    
    echo "$dinamic $static"
    
    if [ -n "$1" ] && [ "$1" -eq 0 ]; then
        sleep 1
    else
        sleep 1
        break
    fi
done

