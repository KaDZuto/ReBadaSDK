#!/bin/sh
set -eu
# Компиляция J2ME MIDlet с использованием stub-классов J2ME (без WTK).
# Для реальной сборки под телефон замените J2ME_CP на официальные MIDP/CLDC jar
# (например, из Sun WTK 2.5.2) и используйте preverify.
HERE=$(cd "$(dirname "$0")" && pwd)
ROOT=$(cd "$HERE/.." && pwd)
CP="${J2ME_CP:-$ROOT/stubs}"
OUT="$HERE/build/classes"
rm -rf "$OUT"; mkdir -p "$OUT"
# compile stubs + midlet
find "$ROOT/stubs" "$HERE" -name '*.java' > /tmp/j2me-sources.txt 2>/dev/null
javac -source 1.6 -target 1.6 -d "$OUT" @/tmp/j2me-sources.txt 2>/dev/null || javac -d "$OUT" @/tmp/j2me-sources.txt
echo "Скомпилировано в $OUT"
# Упаковка JAR/MANIFEST
mkdir -p "$HERE/build"
cd "$OUT"
printf "Manifest-Version: 1.0\nMIDlet-Name: HelloMIDlet\nMIDlet-Version: 1.0.0\nMIDlet-Vendor: ReBadaSDK\nMIDlet-1: HelloMIDlet, , HelloMIDlet\nMicroEdition-Profile: MIDP-2.0\nMicroEdition-Configuration: CLDC-1.1\n" > "$HERE/build/MANIFEST.MF"
jar cfm "$HERE/build/HelloMIDlet.jar" "$HERE/build/MANIFEST.MF" .
echo "JAR: $HERE/build/HelloMIDlet.jar"

# Генерация JAD-дескриптора с корректными обязательными полями.
# Размер JAR должен точно совпадать с файлом, иначе телефон отвергает приложение
# как имеющее неверный формат.
JAR_SIZE=$(stat -c %s "$HERE/build/HelloMIDlet.jar")
cat > "$HERE/build/HelloMIDlet.jad" <<JAD
MIDlet-Name: HelloMIDlet
MIDlet-Version: 1.0.0
MIDlet-Vendor: ReBadaSDK
MIDlet-1: HelloMIDlet, , HelloMIDlet
MicroEdition-Profile: MIDP-2.0
MicroEdition-Configuration: CLDC-1.1
MIDlet-Jar-URL: HelloMIDlet.jar
MIDlet-Jar-Size: $JAR_SIZE
JAD
echo "JAD: $HERE/build/HelloMIDlet.jad"
