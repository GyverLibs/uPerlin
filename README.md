[![latest](https://img.shields.io/github/v/release/GyverLibs/uPerlin.svg?color=brightgreen)](https://github.com/GyverLibs/uPerlin/releases/latest/download/uPerlin.zip)
[![PIO](https://badges.registry.platformio.org/packages/gyverlibs/library/uPerlin.svg)](https://registry.platformio.org/libraries/gyverlibs/uPerlin)
[![Foo](https://img.shields.io/badge/Website-AlexGyver.ru-blue.svg?style=flat-square)](https://alexgyver.ru/)
[![Foo](https://img.shields.io/badge/%E2%82%BD%24%E2%82%AC%20%D0%9F%D0%BE%D0%B4%D0%B4%D0%B5%D1%80%D0%B6%D0%B0%D1%82%D1%8C-%D0%B0%D0%B2%D1%82%D0%BE%D1%80%D0%B0-orange.svg?style=flat-square)](https://alexgyver.ru/support_alex/)
[![Foo](https://img.shields.io/badge/README-ENGLISH-blueviolet.svg?style=flat-square)](https://github-com.translate.goog/GyverLibs/uPerlin?_x_tr_sl=ru&_x_tr_tl=en)  

[![Foo](https://img.shields.io/badge/ПОДПИСАТЬСЯ-НА%20ОБНОВЛЕНИЯ-brightgreen.svg?style=social&logo=telegram&color=blue)](https://t.me/GyverLibs)

# uPerlin
Лёгкая целочисленная реализация шума Перлина. База взята из FastLED и немного улучшена

### Совместимость
Совместима со всеми Arduino платформами (используются Arduino-функции)

## Содержание
- [Использование](#usage)
- [Версии](#versions)
- [Установка](#install)
- [Баги и обратная связь](#feedback)

<a id="usage"></a>

## Использование
### Описание функций
```cpp
// 8-bit output, integer coordinates
uint8_t uperlin8(uint16_t x);
uint8_t uperlin8(uint16_t x, uint16_t y);
uint8_t uperlin8(uint16_t x, uint16_t y, uint16_t z);

// 16-bit output, integer coordinates
uint16_t uperlin16(uint32_t x);
uint16_t uperlin16(uint32_t x, uint32_t y);
uint16_t uperlin16(uint32_t x, uint32_t y, uint32_t z);

// 8-bit output, Q8.8 coordinates
int8_t uperlin8Raw(uint16_t x);
int8_t uperlin8Raw(uint16_t x, uint16_t y);
int8_t uperlin8Raw(uint16_t x, uint16_t y, uint16_t z);

// 16-bit output, Q16.16 coordinates
int16_t uperlin16Raw(uint32_t x);
int16_t uperlin16Raw(uint32_t x, uint32_t y);
int16_t uperlin16Raw(uint32_t x, uint32_t y, uint32_t z);
```

<a id="versions"></a>

## Версии
- v1.0

<a id="install"></a>

## Установка
- Библиотеку можно найти по названию **uPerlin** и установить через менеджер библиотек в:
    - Arduino IDE
    - Arduino IDE v2
    - PlatformIO
- [Скачать библиотеку](https://github.com/GyverLibs/uPerlin/archive/refs/heads/main.zip) .zip архивом для ручной установки:
    - Распаковать и положить в *C:\Program Files (x86)\Arduino\libraries* (Windows x64)
    - Распаковать и положить в *C:\Program Files\Arduino\libraries* (Windows x32)
    - Распаковать и положить в *Документы/Arduino/libraries/*
    - (Arduino IDE) автоматическая установка из .zip: *Скетч/Подключить библиотеку/Добавить .ZIP библиотеку…* и указать скачанный архив
- Читай более подробную инструкцию по установке библиотек [здесь](https://alexgyver.ru/arduino-first/#%D0%A3%D1%81%D1%82%D0%B0%D0%BD%D0%BE%D0%B2%D0%BA%D0%B0_%D0%B1%D0%B8%D0%B1%D0%BB%D0%B8%D0%BE%D1%82%D0%B5%D0%BA)
### Обновление
- Рекомендую всегда обновлять библиотеку: в новых версиях исправляются ошибки и баги, а также проводится оптимизация и добавляются новые фичи
- Через менеджер библиотек IDE: найти библиотеку как при установке и нажать "Обновить"
- Вручную: **удалить папку со старой версией**, а затем положить на её место новую. "Замену" делать нельзя: иногда в новых версиях удаляются файлы, которые останутся при замене и могут привести к ошибкам!

<a id="feedback"></a>

## Баги и обратная связь
При нахождении багов создавайте **Issue**, а лучше сразу пишите на почту [alex@alexgyver.ru](mailto:alex@alexgyver.ru)  
Библиотека открыта для доработки и ваших **Pull Request**'ов!

При сообщении о багах или некорректной работе библиотеки нужно обязательно указывать:
- Версия библиотеки
- Какой используется МК
- Версия SDK (для ESP)
- Версия Arduino IDE
- Корректно ли работают ли встроенные примеры, в которых используются функции и конструкции, приводящие к багу в вашем коде
- Какой код загружался, какая работа от него ожидалась и как он работает в реальности
- В идеале приложить минимальный код, в котором наблюдается баг. Не полотно из тысячи строк, а минимальный код