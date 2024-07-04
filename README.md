# Курсовая (дипломная) работа по курсу

**Программирование на языке С (IOT)**

Группа: 5471

Ростислав Ромашин

Список официальной литературы:

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

