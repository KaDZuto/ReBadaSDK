# ReBadaSDK

Linux-hosted среда разработки нативных программ для Samsung Bada.

Цель проекта — собирать Bada-приложения без Windows-IDE: через GNU/Linux, cross-toolchain `arm-bada-eabi` и небольшие утилиты для генерации htb, подписи и установки.

## Что внутри

- `sdk/init.sh` — активация SDK.
- `sdk/bin/` — обёртки для компилятора, `genhtb`, подписи, broker и удобные команды: `bada-build`, `bada-clean`, `bada-verify`, `bada-new`, `badb`.
- `sdk/scripts/` — Python-утилиты `genhtb.py`, `signing.py`, `broker.py`.
- `sdk/examples/hello-from-2026/` — готовый пример приложения "Hello From 2026".
- `DEVELOPMENT.md` — инструкции по разработке.

## Требования

Нужен локально извлечённый Samsung Bada SDK (`bada_SDK_2.0.6`) и Wine/Proton для запуска bundled Windows cross-toolchain. Дополнительно: Python 2, OpenSSL, make.

## Подготовка

```sh
cd /home/alexius/Projects/ReBadaSDK
# предполагается, что Windows SDK извлечён в ex/
source sdk/init.sh
${CROSS_COMPILE}gcc --version
```

## Сборка примера

```sh
source sdk/init.sh
cd sdk/examples/hello-from-2026
make
```

Результат:

- `2026hllo/Bin/hello2026.exe`
- `2026hllo/Info/hello2026.htb`
- `2026hllo/signature.xml`

Быстрее:

```sh
source sdk/init.sh
bada-build sdk/examples/hello-from-2026
bada-verify sdk/examples/hello-from-2026
```

## Новый проект

```sh
source sdk/init.sh
bada-new myapp myappid
cd sdk/examples/myapp
bada-build .
```

## Установка на устройство

Подключите телефон через USB, затем:

```sh
badb devices
badb install myappid myapp.exe
```

Логи обмена:

```sh
badb logcat
```

## Загрузка файлов

```sh
badb push ./file.txt /dest/path
```

## Лицензия

В репозиторий не включаются proprietary-файлы Samsung. Для полноценной работы нужен собственный экземпляр Bada SDK.
