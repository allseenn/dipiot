#!/bin/bash
# Данный скрипт женит bsec и mqtt через именнованный канал /tmp/bsec
# Если поместить скрипт в автозагрузку, то процесс считывания данных с датчика
# и отправка полученных значений по mqtt будет стартовать при запуске системы
# В качестве автозагрузки использую /etc/rc.local и правильную его настройку в systemd
set -x
mkfifo /tmp/bsec
/usr/local/sbin/bsec > /tmp/bsec &
sleep 2
cat /tmp/bsec | /usr/local/sbin/mqtt &

