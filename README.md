# Курсовая (дипломная) работа по курсу

**Программирование на языке С (IOT)**

Группа: 5471

Ростислав Ромашин


Настройка Nod-Red



## BME680
Датчик газа, давления, температуры и влажности с низким энергопотреблением
BME680 — это цифровой 4-в-1 датчик, измеряющий газ, влажность, давление и температуру на основе проверенных принципов измерения. Модуль датчика помещен в крайне компактный корпус с металлической крышкой LGA, имеющий размеры всего 3,0 × 3,0 мм² и максимальную высоту 1,00 мм (0,93 ± 0,07 мм). Его небольшие размеры и низкое энергопотребление позволяют интегрировать его в устройства, работающие от батареи или в устройства с частотной связью, такие как мобильные телефоны или носимые устройства.


### Программное обеспечение BSEC
Датчик BME680 предназначен для использования вместе с решением Bosch Software Environmental Cluster (BSEC) и API датчика BME6xy, чтобы раскрыть его полный потенциал. Программное обеспечение BSEC включает интеллектуальные алгоритмы, которые позволяют использовать такие сценарии, как мониторинг качества воздуха в помещении с использованием BME680.

Программное обеспечение BSEC от Bosch Sensortec доступно в виде закрытого двоичного файла, который будет предоставлен через Лицензионное Соглашение на Программное Обеспечение (SLA) на сайте Bosch Sensortec (https://www.bosch-sensortec.com/bst/products/all_products/BSEC). API датчика охватывает базовую коммуникацию с датчиком и функции компенсации данных, и доступен как открытый исходный код на Github (https://github.com/BoschSensortec/BME68x-Sensor-API).

Ключевые особенности системы аппаратного и программного обеспечения:

Расчет температуры окружающего воздуха вне устройства (например, телефона)
Расчет относительной влажности окружающего воздуха вне устройства
Расчет индекса качества воздуха (IAQ) вне устройства
Кроме того, программные алгоритмы обрабатывают компенсацию влажности, базовую линию, а также коррекцию долгосрочного дрейфа сигнала газового датчика.


### Список официальной литературы:

1. [BME680 – Datasheet 1.9 02-2024](https://github.com/allseenn/dipiot/blob/main/docs/bst-bme680-ds001.pdf) - 53 страниц
2. [BME680 – Datasheet 1.3 07-2019](https://github.com/allseenn/dipiot/blob/main/docs/CZ_BME680_BOSCH_0001.pdf) - 54 страниц
3. [BSEC Integration Guideline](https://github.com/allseenn/dipiot/blob/main/docs/BST-BME680-Integration-Guide-AN008-47.pdf) - 56 страниц
4. [BSEC Binary Size Information](https://github.com/allseenn/dipiot/blob/main/docs/BSEC2%20Binary%20Size%20Information.pdf) -3 страницы
5. [BME68x - Shipment packaging details](https://github.com/allseenn/dipiot/blob/main/docs/bst-bme68x-sp000.pdf) - 9 страниц
6. [BME680 – Application Note](https://github.com/allseenn/dipiot/blob/main/docs/bst-bme680-an014.pdf) - 13 страниц
7. [BME680 Integrated Environmental Unit](https://github.com/allseenn/dipiot/blob/main/docs/bst-bme680-fl000.pdf) - 2 страниц
8. [BME680: Handling, Soldering and Mounting Instructions](https://github.com/allseenn/dipiot/blob/main/docs/bst-bme680-hs000.pdf) - 16 страниц
9. [BME680 Shuttle board 3.0 flyer](https://github.com/allseenn/dipiot/blob/main/docs/bst-bme680-sf000.pdf) - 2 страницы
10. [Release Notes 2.5.0.2](https://github.com/allseenn/dipiot/blob/main/docs/Release%20Notes_BSEC_2.5.0.2_Website_Release_11012024.pdf)

Итого, около 200 страниц технической информации.

К сожалению компания BOSH удалила свой [старый](https://github.com/BoschSensortec/BME680_driver/releases) репозиторий с открытым драйвером датчика BME680.
Но, нашел старый открытый драйвер 3.5.9 на стороннем репозитории https://github.com/wintersteiger/BME680_driver. 
Драйвер содержит открытие API. Для того чтобы начать получать или записывать информацию на датчик необходимо самостоятельно написать основную программу для конкретной платформы и операционной системы, которая с помощью апи будет обращатеся через драйвер к датчику.
Т.к. готовых примеров не нашел, то за основу взял решение для Raspberry Pi, но для более старого датчика BME280  https://www.waveshare.com/w/upload/b/ba/BME280-Environmental-Sensor-Demo-Code.7z
Код от 280 частично совместим и позволяет отображать только температуру. Но, подсмотрев основую логику организации взаимодействия удалось получать информацию с другими параметрами: давление, влажность, сопротивление воздуха.

