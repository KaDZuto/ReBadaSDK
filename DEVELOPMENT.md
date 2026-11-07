# ReBadaSDK — руководство разработчика

## Critical setup
Always source init before build:
```sh
source /home/alexius/Projects/ReBadaSDK/sdk/init.sh
```

## Build hello example
```sh
source /home/alexius/Projects/ReBadaSDK/sdk/init.sh
cd /home/alexius/Projects/ReBadaSDK/sdk/examples/hello-from-2026
make
```

## Expected outputs
- `2026hllo/Bin/hello2026.exe`
- `2026hllo/Info/hello2026.htb`
- `2026hllo/signature.xml`

## Common agent tasks
- Add source files to example directory and update Makefile wildcard if needed (`cxx_sources` auto wildcard).
- Change app id/version in `2026hllo/Info/manifest.xml`.
- Adjust screen profile by switching `WaveHVGA` paths in Makefile and app id.
- Do not invoke Windows `.exe` directly; use wrappers in `sdk/bin` or source init.
- Wine may emit harmless fixme logs.
- Cleaning: `make clean`.

## Build verification
```sh
test -f /home/alexius/Projects/ReBadaSDK/sdk/examples/hello-from-2026/2026hllo/Bin/hello2026.exe
```
