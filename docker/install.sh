#!/bin/bash
USERNAME=admin
PASSWORD=students
DOMEN=my.domen.ru
ORG=IoT
BUCKET=IoT
INFLUXDB_TOKEN="kFhczFje8dRm2SXK1V9Ds7xpcJTr6wVUS881KQoUQWE-QAfcg-S-6j1FvFiSvWW0wTPlmWHCvXf_JU1hRx5rZg=="

LOC_IP=$(ip route get 1 | awk '{print $7}' | head -1)
PUB_IP=$(wget -qO- ifconfig.me)
ALL_IP=0.0.0.0
DOCKER_VERSION=$(docker --version | awk '{print $3}' | cut -d '.' -f1)
if [ -z "$DOCKER_VERSION" ] || [ "$(echo $DOCKER_VERSION | cut -d '.' -f1)" -lt "24" ]; then
    echo "Установка или обновление Docker..."

    # Удаляем старые версии Docker
    for pkg in docker.io docker-doc docker-compose podman-docker containerd runc; do
        sudo apt-get remove -y $pkg
    done

    # Устанавливаем необходимые пакеты
    sudo apt-get update
    sudo apt-get -y install ca-certificates wget curl gnupg catimg

    # Устанавливаем ключи и добавляем репозиторий Docker
    sudo install -m 0755 -d /etc/apt/keyrings
    curl -fsSL https://download.docker.com/linux/ubuntu/gpg | sudo gpg --dearmor -o /etc/apt/keyrings/docker.gpg
    sudo chmod a+r /etc/apt/keyrings/docker.gpg
    echo "deb [arch="$(dpkg --print-architecture)" signed-by=/etc/apt/keyrings/docker.gpg] https://download.docker.com/linux/ubuntu \
    "$(. /etc/os-release && echo "$VERSION_CODENAME")" stable" | sudo tee /etc/apt/sources.list.d/docker.list > /dev/null

    # Обновляем пакеты и устанавливаем Docker
    sudo apt-get update
    sudo apt-get -y install git docker-ce docker-ce-cli containerd.io docker-buildx-plugin docker-compose-plugin

    echo "Docker успешно установлен или обновлен."
else
    echo "У вас уже установлена актуальная версия Docker."
fi
clear
apt -y install jq curl wget catimg
clear
mkdir -m 777 -p ~/mosquitto/config && \
mkdir -m 777 -p ~/mosquitto/data && \
mkdir -m 777 -p ~/mosquitto/log && \
mkdir -m 777 -p ~/influxdb2/data && \
mkdir -m 777 -p ~/influxdb2/conf && \
mkdir -m 777 -p ~/telegraf/conf && \
mkdir -m 777 -p ~/grafana/data && \
mkdir -m 777 -p ~/grafana/conf && \
mkdir -m 777 -p ~/grafana/log && \
mkdir -m 777 -p ~/node-red/data && \
mkdir -m 777 -p ~/wireguard/config

cat > ~/grafana/conf/grafana.ini <<EOF
[server]
http_addr = $ALL_IP
protocol = http
http_port = 3000
#domain = $DOMEN
#root_url = http://$DOMEN:3000/grafana/
#serve_from_sub_path = true

[log]
level = debug

[security]
admin_user = $USERNAME
admin_password = $PASSWORD

#[database]
#type = influxdb
#url = $LOC_IP
#basic_auth = false
#token = kFhczFje8dRm2SXK1V9Ds7xpcJTr6wVUS881KQoUQWE-QAfcg-S-6j1FvFiSvWW0wTPlmWHCvXf_JU1hRx5rZg==

#[influxdb]
#organization = $ORG
#bucket = $BUCKET

EOF

cat > ~/mosquitto/config/mosquitto.conf <<EOF
listener 1883 $ALL
allow_anonymous false
listener 8081 $ALL
protocol websockets
persistence true
persistence_location /mosquitto/data/
password_file /mosquitto/config/password.txt
log_dest file /mosquitto/log/mosquitto.log
EOF

touch ~/mosquitto/log/mosquitto.log

cat > ~/mosquitto/config/password.txt <<EOF
$USERNAME:\$7\$101\$6DNQoUH7oXD1MS76\$SlOUlbz5SG3sxdHOr4mJwE9KzZmxeXQmh2IOdapsI7xYASYjajCVUA5ECD+SXGWfeQFPBvYoLkWMH/WpEIZjDg==
EOF

cat > ~/node-red/data/settings.js <<EOF
module.exports = {
    flowFile: 'flows.json',
    flowFilePretty: true,
    adminAuth: {
    	type: "credentials",
    	users: [{
    		username: "$USERNAME",
    		password: "\$2b\$08\$8KNl4/HWaKxJLYiF0wBCFe596FyS8ibyxWF4hwcWJIVSPIeAaEGOu",
        permissions: "*"
    		}]
    	},
    uiPort: process.env.PORT || 1880,
    diagnostics: {
        enabled: true,
        ui: true,
    },
    runtimeState: {
        enabled: false,
        ui: false,
    },
    logging: {
        console: {
            level: "info",
            metrics: false,
            audit: false
        }
    },
    exportGlobalContextKeys: false,
    externalModules: {
    },
    editorTheme: {
        palette: {
        },
        projects: {
            enabled: false,
            workflow: {
                mode: "manual"
            }
        },
        codeEditor: {
            lib: "monaco",
            options: {
            }
        },
        markdownEditor: {
            mermaid: {
                enabled: true
            }
        },
    },
    functionExternalModules: true,
    functionTimeout: 0,
    functionGlobalContext: {
    },
    debugMaxLength: 1000,
    mqttReconnectTime: 15000,
    serialReconnectTime: 15000,
}
EOF

cat > ~/telegraf/conf/telegraf.conf <<EOF
[agent]
 interval = "3s"
 round_interval = true
 metric_batch_size = 1000
 metric_buffer_limit = 10000
 collection_jitter = "0s"
 flush_interval = "3s"
 flush_jitter = "0s"
 precision = ""
 hostname = ""
 omit_hostname = false

[[outputs.influxdb_v2]]
  urls = ["http://$LOC_IP:8086"]
  token = "$INFLUXDB_TOKEN"
  organization = "$ORG"
  bucket = "$BUCKET"

[[inputs.mqtt_consumer]]
  servers = ["tcp://$LOC_IP:1883"]
  topics = ["#"]
  username = "$USERNAME"
  password = "$PASSWORD"
  data_format = "value"
  data_type = "float"

[[inputs.docker]]
  endpoint = "unix:///var/run/docker.sock"

EOF

cat > ~/docker-compose.yml <<EOF
services:
  influxdb:
    container_name: influxdb
    image: influxdb:latest
    environment:
      - TZ=Europe/Moscow
      - DOCKER_INFLUXDB_INIT_MODE=setup
      - DOCKER_INFLUXDB_INIT_USERNAME=$USERNAME
      - DOCKER_INFLUXDB_INIT_PASSWORD=$PASSWORD
      - DOCKER_INFLUXDB_INIT_ORG=$ORG
      - DOCKER_INFLUXDB_INIT_BUCKET=$BUCKET
      - DOCKER_INFLUXDB_INIT_ADMIN_TOKEN=$INFLUXDB_TOKEN
    ports:
      - "8086:8086"
    volumes:
      - ~/influxdb2/data:/var/lib/influxdb2
      - ~/influxdb2/conf:/etc/influxdb2/
    networks:
      - influxdb-net
    restart: always
  wireguard:
    container_name: wireguard
    image: lscr.io/linuxserver/wireguard:latest
    cap_add:
      - NET_ADMIN
      - SYS_MODULE
    environment:
      - PUID=1000
      - PGID=1000
      - TZ=Europe/Moscow
      - SERVERURL=$PUB_IP
      - SERVERPORT=1871 #optional
      - PEERS=5 #optional
      - PEERDNS=1.1.1.1 #optional
      - INTERNAL_SUBNET=10.13.13.0 #optional
      - ALLOWEDIPS=0.0.0.0/0 #optional
      - LOG_CONFS=false #optional
    volumes:
      - ~/wireguard/config:/config
      - /lib/modules:/lib/modules
    ports:
      - 51820:51820/udp
    sysctls:
      - net.ipv4.conf.all.src_valid_mark=1
    restart: always
  telegraf:
    container_name: telegraf
    image: telegraf:latest
    environment:
      - TZ=Europe/Moscow
    volumes:
      - ~/telegraf/conf/:/etc/telegraf/
      - /var/run/docker.sock:/var/run/docker.sock
    networks:
      - telegraf-net
    restart: always
  grafana:
    container_name: grafana
    image: grafana/grafana-oss:latest
    environment:
      - TZ=Europe/Moscow
    ports:
      - "3000:3000"
    volumes:
      - ~/grafana/data:/var/lib/grafana/
      - ~/grafana/log:/var/log/grafana/
      - ~/grafana/conf:/etc/grafana/
    links:
      - influxdb
    networks:
      - grafana-net
    restart: always
  mosquitto:
    container_name: mosquitto
    image: eclipse-mosquitto:latest
    environment:
      - TZ=Europe/Moscow
    volumes:
      - ~/mosquitto/config:/mosquitto/config
      - ~/mosquitto/data:/mosquitto/data
      - ~/mosquitto/log:/mosquitto/log
    ports:
      - 1883:1883
      - 8081:8081
    networks:
      - mosquitto-net
    restart: always
  node-red:
    container_name: node-red
    image: nodered/node-red:latest
    environment:
      - TZ=Europe/Moscow
    ports:
      - "1880:1880"
    volumes:
      - ~/node-red/data:/data
    networks:
      - node-red-net
    restart: always
networks:
  node-red-net:
  influxdb-net:
  mosquitto-net:
  grafana-net:
  telegraf-net:
EOF

docker compose -f docker-compose.yml up -d
clear
echo "ПАРАМЕТРЫ ДЛЯ ЛОКАЛЬНОГО ПОДКЛЮЧЕНИЯ
Grafana http://$LOC_IP:$(docker ps -f name=grafana --format '{{.Ports}}' | cut -d ':' -f 2 | cut -d '-' -f 1)
NodeRed http://$LOC_IP:$(docker ps -f name=node-red --format '{{.Ports}}' | cut -d ':' -f 2 | cut -d '-' -f 1)
Influx2 http://$LOC_IP:$(docker ps -f name=influxdb --format '{{.Ports}}' | cut -d ':' -f 2 | cut -d '-' -f 1)
" > ~/info.txt
echo "...готовлю список подключениий для внутренних адресов..."
sleep 7

echo "ПАРАМЕТРЫ ДЛЯ ПОДКЛЮЧЕНИЯ ИЗВНЕ, ЕСЛИ ПРОБРОШЕНЫ ПОРТЫ
Grafana http://$PUB_IP:$(docker ps -f name=grafana --format '{{.Ports}}' | cut -d ':' -f 2 | cut -d '-' -f 1)
NodeRed http://$PUB_IP:$(docker ps -f name=node-red --format '{{.Ports}}' | cut -d ':' -f 2 | cut -d '-' -f 1)
Influx2 http://$PUB_IP:$(docker ps -f name=influxdb --format '{{.Ports}}' | cut -d ':' -f 2 | cut -d '-' -f 1)
" >> ~/info.txt
echo "..готовлю инфу по внешним адресам..."

sleep 7

curl --user $USERNAME:$PASSWORD "http://$LOC_IP:3000/api/datasources" -X POST -H 'Content-Type: application/json;charset=UTF-8' --data-binary '{
  "name": "InfluxDB",
  "isDefault": true,
  "type": "influxdb",
  "url": "'http://$LOC_IP:8086'",
  "access": "proxy",
  "basicAuth": false,
  "jsonData": {
    "httpMode": "Bearer",
    "version": "Flux",
    "organization": "'"$ORG"'",
    "defaultBucket": "'"$BUCKET"'"
  },
  "secureJsonData": {
    "token": "'"$INFLUXDB_TOKEN"'"
  }
}'

echo "...подготовливаю соединение от графаны до инфлюкса..."
sleep 7

echo "ПАРАМЕТРЫ ДЛЯ АВТОРИЗАЦИИ
username: $USERNAME
password: $PASSWORD
внешний ip: $PUB_IP
локальный ip: $LOC_IP
token influxdb: $INFLUXDB_TOKEN
Версия WireGuard для андроида: https://play.google.com/store/search?q=wireguard
Конфиги настроки WireGuard лежат ~/wireguard/config/wg_confs/wg0.conf" >> ~/info.txt

echo "...сохраняю параметры подключения в файл ~/info.txt..."
sleep 7

cat ~/info.txt
echo "...готовлю код для WireGuard..."
sleep 7

echo "QR-КОД ДЛЯ НАСТРОЙКИ WIREGUARD"
catimg ~/wireguard/config/peer1/peer1.png -w 150

echo "УСТАНОВКА И НАСТРОЙКА ЗАВЕРШЕНЫ!
Все настройки и пароли сохранены в файле ~/info.txt"

