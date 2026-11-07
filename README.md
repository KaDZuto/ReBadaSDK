# ReBadaSDK

Сборка и установка нативных приложений Samsung Bada 1.x с Linux-хоста.
Кросс-тулчейн `arm-bada-eabi` работает под Wine/Proton, пакет собирается
локально: `htb` → подпись → `AppPkgInstall` по диагностическому каналу.

Проверено на GT-S5250 (Wave 525, Bada 1.1): пример `sdk/examples/privet`
(белый экран, «Привет с Линукса») собирается, устанавливается и запускается.

## Состав

- `sdk/init.sh` — окружение (`SDK_ROOT`, `PATH`, `CROSS_COMPILE`).
- `sdk/bin/` — компилятор, `bada-genhtb`, `bada-signing`, `bada-broker`,
  `bada-build`, `bada-clean`, `bada-verify`, `bada-new`, `badb`.
- `sdk/scripts/` — `genhtb.py`, `signing.py`, `broker.py`.
- `sdk/examples/hello-from-2026` — минимальный пример.
- `sdk/examples/privet` — пример с собственным экраном.
- `docs/REVERSE_ENGINEERING.md` — разбор формата пакета и подписи,
  история ошибок установки.

## Требования

- Локально извлечённый Bada SDK 2.0.6 (`ex/`), см. `.gitignore`.
- Wine или Proton: тулчейн поставляется как Windows `.exe`.
- Python 2 (`broker`, `genhtb`, `signing`), Python 3 + `cryptography`
  (только для ручной проверки подписи), `openssl`, `make`.

## Сборка

```sh
source sdk/init.sh
cd sdk/examples/privet
make
```

Результат:

- `prvlnx2026/Bin/privet.exe`
- `prvlnx2026/Info/privet.htb`
- `prvlnx2026/Info/appSign.sys`
- `prvlnx2026/Info/signerCert.sys`
- `prvlnx2026/Signature.xml`

Новый проект:

```sh
source sdk/init.sh
bada-new myapp myappid
cd sdk/examples/myapp
make
```

## Формат пакета

Устройство принимает только формат production-пакета:

- `Package` — RSA PKCS#1 v1.5 над 20-байтным SHA1(`exe || manifest`),
  **без** ASN.1 DigestInfo.
- `AppSignature` — RSA PKCS#1 v1.5 над SHA1(`htb`).
- `Info/appSign.sys` — base64 от `AppSignature` (172 байта).
- `Info/signerCert.sys` — DER сертификата разработчика.
- `Signature.xml` — в корне каталога приложения (имя с заглавной `S`).
- `FileList` — `/bin/<app>.exe`, `/info/manifest.xml` (строчными).

`signing.py` собирает всё это за один вызов; встроенные test-сертификаты
SDK (`badaDev.cer`, `badaCA.cer`) прошивка принимает.

## Установка

Телефон подключён по USB, включён режим Kies/диагностики:

```sh
badb devices
badb install prvlnx2026 privet.exe
```

`bada-broker` сам находит пару `/dev/ttyACM*`, заливает каталог приложения,
выполняет `AppPkgInstall` и запускает процесс. Альтернативы:

```sh
badb deploy <appdir> <app.exe>   # appRun без AppPkgInstall
badb donors                      # список установленных приложений
```

## Ограничения

- Внутренний `/Osp` около 20 МБ; крупные игры не помещаются.
- `/Osp/Mount/Mmc` в режиме Kies не виден, установка на microSD требует
  другого режима.
- Пакеты Bada 2.0 (`APIVersion 2.0`) на Bada 1.1 не рассчитаны.

## Лицензия

Proprietary-файлы Samsung в репозиторий не входят. Для работы нужен
собственный экземпляр Bada SDK.
