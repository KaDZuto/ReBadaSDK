# Реверс пакета Bada и установка своей программы

## Задача

Собрать нативное приложение под Bada 1.1 (GT-S5250, Wave 525) на Linux,
подписать его так, чтобы прошивка приняла пакет через `AppPkgInstall`,
и запустить на устройстве. До этого установка собранного примера
завершалась ошибкой `errType=0112`, а рабочими оставались только
заводские приложения.

## Что дало эталонное приложение

Для разбора взяли установленный `MyExplorer` (`5tv0osr480`). Его каталог
содержит:

```
5tv0osr480/
  Bin/MyExplorer.exe
  Info/manifest.xml
  Info/MyExplorer.htb
  Info/appSign.sys
  Info/signerCert.sys
  Signature.xml
```

Собранный SDK-пример выглядел иначе: `signature.xml` в нижнем регистре,
без `appSign.sys` и `signerCert.sys`. Этого достаточно, чтобы устройство
отклонило пакет.

## htb

Разбор `htb` подтвердил формат из `genhtb.py`:

- заголовок: `"Hash"`, `u32 1`, длина заголовка, AppId, версия в base36,
  Secret (32 hex-символа);
- далее SHA1 каждого блока `.exe` по 4096 байт, подряд.

Для `MyExplorer.exe` размером 1 819 268 байт это 445 записей по 20 байт.
`htb` генерировался правильно, причиной ошибки он не был.

## Подпись

Подпись в пакете — обычный RSA PKCS#1 v1.5, но над «голым» 20-байтным
SHA1, без ASN.1 DigestInfo. Проверка: `signature^e mod n`, снять padding,
сравнить с SHA1.

- `Package` = RSA(`SHA1(exe || manifest)`);
- `AppSignature` = RSA(`SHA1(htb)`);
- `Info/appSign.sys` = base64 от `AppSignature` (172 байта);
- `Info/signerCert.sys` = DER сертификата разработчика;
- `certificateChain` = `[dev, CA]`, корневой test-сертификат уже в прошивке;
- `FileList` = `/bin/<app>.exe`, `/info/manifest.xml` (строчными).

Для `MyExplorer` восстановленный digest совпал с SHA1(`exe || manifest`),
а `AppSignature` — с SHA1(`htb`). Это и стало рабочей схемой.

## Почему падало 0112

Причин было несколько, и все в упаковке:

1. `signing.py` подписывал через `openssl dgst -sha1 -sign`. Это добавляет
   DigestInfo, то есть формат отличается от production-пакета.
2. Не создавались `Info/appSign.sys` и `Info/signerCert.sys`.
3. Файл назывался `signature.xml`, а прошивка ищет `Signature.xml`.
4. `FileList` содержал пути в другом регистре.

`signing.py` переписан: digest считается в Python, подпись — через
`openssl pkeyutl -sign` (ровно raw PKCS#1 без DigestInfo). Скрипт сам
пишет `appSign.sys`, `signerCert.sys` и `Signature.xml`.

Проверка на хосте: восстановление подписи открытым ключом даёт исходный
SHA1 для `Package` и `AppSignature`.

## Канал установки

В `broker.py` поправили две вещи, без которых заливка нестабильна:

- после переподключения телефон может оказаться на другой паре
  `/dev/ttyACM*`; порт теперь определяется автоматически, debug-канал —
  следующий номер;
- `getFile` читает один блок и не делает `_FILE_CLOSE`: на этой прошивке
  закрытие после чтения роняет диагностический канал.

Установка: `badb install <appid> <app.exe>`. Broker заливает каталог,
вызывает `AppPkgInstall`, затем `appRun`.

## Результат

Собран пример `sdk/examples/privet` (AppId `prvlnx2026`): форма с белым
фоном и меткой «Привет с Линукса». Пакет подписан test-цепочкой SDK.
`AppPkgInstall` вернул `errType=0`, процесс `privet.exe` появился в
`PROCESSMGR`, приложение отрисовало экран на устройстве.

## Воспроизведение

```sh
source sdk/init.sh
cd sdk/examples/privet
make
badb install prvlnx2026 privet.exe
```

Ключи и сертификаты берутся из `sdk/usr/share/bada/crypto`
(симлинк на `Tools/sbuild` внутри SDK).

## Что осталось за рамками

- Установка на microSD: в режиме Kies точка монтирования недоступна
  broker-у, нужен другой режим подключения.
- Игры Bada 2.0 (`APIVersion 2.0`) на Bada 1.1 не проверялись; часть
  из них не помещается во внутренние ~20 МБ.
