#!/bin/bash

while true; do
    # Чтение данных из именованного конвейера
    read line < /tmp/bsec_output
    echo "WEB получил: $line"
    # Обработка данных для веб-сервера
    # ...
done

