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

# Get Dobby
if [ ! -d "$DIR/deps/Dobby" ]; then
    echo "==> Fetching Dobby..."
    git clone --depth 1 https://github.com/jmpews/Dobby.git "$DIR/deps/Dobby"
fi

# Build Dobby
echo "==> Building Dobby..."
cd "$DIR/deps/Dobby"
mkdir -p build && cd build
cmake .. \
    -DCMAKE_TOOLCHAIN_FILE="$NDK/build/cmake/android.toolchain.cmake" \
    -DANDROID_ABI="$ABI" \
    -DANDROID_PLATFORM="$API" \
    -DDOBBY_GENERATE_SHARED=ON \
    -DDOBBY_GENERATE_OBJECT=OFF \
    -DCMAKE_BUILD_TYPE=Release
cmake --build . --target dobby -j$(nproc)

# Build JSHook
echo "==> Building JSHook..."
cd "$DIR"
mkdir -p build && cd build
cmake .. \
    -DCMAKE_TOOLCHAIN_FILE="$NDK/build/cmake/android.toolchain.cmake" \
    -DANDROID_ABI="$ABI" \
    -DANDROID_PLATFORM="$API" \
    -DCMAKE_BUILD_TYPE=Release
cmake --build . -j$(nproc)

echo "==> Build complete!"
ls -lh "$DIR/build/libjshook.so"
