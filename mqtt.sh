#!/bin/bash

while true; do
    # Чтение данных из именованного конвейера
    read line < /tmp/bsec_output
    echo "MQTT получил: $line"
    # Обработка данных для MQTT-брокера
    # ...
done

