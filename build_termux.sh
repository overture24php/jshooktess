#!/data/data/com.termux/files/usr/bin/bash
# Build JSHook for Cocos2d-js in Termux
# Requires: pkg install git cmake ninja ndk-multilib

set -e

DIR="$(cd "$(dirname "$0")" && pwd)"
ABI="${1:-arm64-v8a}"
NDK="${ANDROID_NDK_HOME:-$HOME/android-ndk}"
API=21

echo "==> JSHook build for $ABI"

# Setup NDK if not exists
if [ ! -d "$NDK" ]; then
    echo "Android NDK not found at $NDK"
    echo "Set ANDROID_NDK_HOME or install via: pkg install ndk-multilib"
    exit 1
fi

# Get Dobby (prebuilt)
DOBBY_REPO="https://github.com/3equals3/DobbyHook"
if [ ! -d "$DIR/deps/Dobby" ]; then
    echo "==> Downloading prebuilt Dobby..."
    mkdir -p "$DIR/deps/Dobby/include" "$DIR/deps/Dobby/lib"

    # Map ABI name
    case "$ABI" in
        arm64-v8a) DOBBY_ABI="arm64-v8a" ;;
        armeabi-v7a) DOBBY_ABI="armeabi-v7a" ;;
        *) echo "Unknown ABI: $ABI"; exit 1 ;;
    esac

    curl -sL -o "$DIR/deps/Dobby/include/dobby.h" \
        "$DOBBY_REPO/raw/main/Dobby.h"
    curl -sL -o "$DIR/deps/Dobby/lib/libdobby.a" \
        "$DOBBY_REPO/raw/main/$DOBBY_ABI/libdobby.a"
fi

# Build JSHook
echo "==> Building JSHook..."
cd "$DIR"
mkdir -p build && cd build
cmake .. \
    -DCMAKE_TOOLCHAIN_FILE="$NDK/build/cmake/android.toolchain.cmake" \
    -DANDROID_ABI="$ABI" \
    -DANDROID_PLATFORM="$API" \
    -DCMAKE_BUILD_TYPE=Release \
    -DDOBBY_USE_PREBUILT=ON
cmake --build . -j$(nproc)

echo "==> Build complete!"
ls -lh "$DIR/build/libjshook.so"
