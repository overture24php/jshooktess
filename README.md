# JSHook - Cocos2d-js XXTEA Decryption Hook

Dobby-based native library untuk intercept decrypt `.jsc`/`.json` files di game Cocos Creator 2.x.

## Cara Kerja

1. Inject `libjshook.so` via jshook injector (mirip libTool)
2. Hook `xxtea_decrypt` + `jsb_set_xxtea_key` di `libcocos2djs.so`
3. Semua file yang didecrypt di-dump ke `/sdcard/jshook_dump/`
4. File modding: taruh file di `/sdcard/jshook_replace/`

## Output

| Path | Deskripsi |
|------|-----------|
| `/sdcard/jshook_dump/0000.js` | Decrypted JS files |
| `/sdcard/jshook_dump/0001.json` | Decrypted JSON assets |
| `/sdcard/jshook_dump/xxtea_key.txt` | XXTEA key (jika tertangkap) |
| `/sdcard/jshook_dump/init.log` | Log hook initialization |
| `/sdcard/jshook_dump/files.log` | File access log |

## Build

### Via GitHub Actions (recommended)

Push ke GitHub, Actions otomatis build. Download artifact.

### Via Termux

```bash
pkg install git cmake ninja ndk-multilib
export ANDROID_NDK_HOME=$HOME/android-ndk
./build_termux.sh arm64-v8a
```

## Inject

Gunakan jshook injector untuk inject `libjshook.so` ke proses game.
