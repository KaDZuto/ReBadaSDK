# J2ME workspace

Рабочая папка для разработки J2ME MIDlet-ов (например, для установки на старый телефон вместо нативных Bada-программ).

## Состав

- `hello/` — пример MIDlet `HelloMIDlet`.
- `stubs/` — минимальные заглушки `javax.microedition.*` для локальной компиляции без Sun WTK.

## Сборка (заглушки)

```sh
cd /home/alexius/Projects/ReBadaSDK/J2ME/hello
./build.sh
```

Результат: `J2ME/hello/build/HelloMIDlet.jar`.

## Для реального телефона

Для настоящей сборки нужен J2ME SDK (Sun Java Wireless Toolkit 2.5.2 / S40 SDK / и т.п.):

1. Установить `J2ME_CP` на путь к официальным `midpapi20.jar` / `cldcapi11.jar`.
2. Использовать `preverify` от WTK перед упаковкой.
3. Указать `MIDlet-1` и параметры в `META-INF/MANIFEST.MF`.

Сборка с официальным WTK:

```sh
export J2ME_CP=/path/to/wtk/lib/midpapi20.jar:/path/to/wtk/lib/cldcapi11.jar
J2ME_CP="$J2ME_CP" ./build.sh
```

Примечание: stub-версия не preverified и не подходит для запуска на устройстве/эмуляторе.
