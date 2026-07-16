#!/bin/bash
set -e

BUILD_HOME="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
ROOT_HOME="$(cd "$BUILD_HOME/../.." && pwd)"
NITRO_ANDROID_JNI_DIR="$ROOT_HOME/android/src/main/jniLibs"

PROFILE="mobile"
ANDROID_MIN_API="27"

ANDROID_ARCHS=("aarch64-linux-android" "armv7-linux-androideabi" "i686-linux-android" "x86_64-linux-android")
ANDROID_JNI=("arm64-v8a" "armeabi-v7a" "x86" "x86_64")
ANDROID_LLVM_ARCHS=("aarch64-linux-android" "armv7a-linux-androideabi" "i686-linux-android" "x86_64-linux-android")

if [[ -z "$ANDROID_NDK_HOME" ]]; then
    echo "ERROR: ANDROID_NDK_HOME is not set. Please set it to your Android NDK root."
    exit 1
fi

HOST_OS="$(uname -s | tr '[:upper:]' '[:lower:]')"
HOST_ARCH="$(uname -m)"
if [[ "$HOST_ARCH" == "arm64" ]]; then
    HOST_ARCH="aarch64"
fi
PREBUILT_DIR="$ANDROID_NDK_HOME/toolchains/llvm/prebuilt/$HOST_OS-$HOST_ARCH"

if [[ ! -d "$PREBUILT_DIR" ]]; then
    echo "ERROR: Could not find NDK toolchain at $PREBUILT_DIR"
    exit 1
fi

echo "Building kos-nitro Android static libraries..."

export AR="$PREBUILT_DIR/bin/llvm-ar"
export CARGO_TARGET_AARCH64_LINUX_ANDROID_LINKER="$PREBUILT_DIR/bin/aarch64-linux-android${ANDROID_MIN_API}-clang"
export CARGO_TARGET_ARMV7_LINUX_ANDROIDEABI_LINKER="$PREBUILT_DIR/bin/armv7a-linux-androideabi${ANDROID_MIN_API}-clang"
export CARGO_TARGET_I686_LINUX_ANDROID_LINKER="$PREBUILT_DIR/bin/i686-linux-android${ANDROID_MIN_API}-clang"
export CARGO_TARGET_X86_64_LINUX_ANDROID_LINKER="$PREBUILT_DIR/bin/x86_64-linux-android${ANDROID_MIN_API}-clang"

for i in "${!ANDROID_ARCHS[@]}"; do
    target="${ANDROID_ARCHS[$i]}"
    rustup target add "$target"
    echo "Building $target..."
    PROTOC="${PROTOC:-/opt/homebrew/bin/protoc}" \
    PATH="${PROTOC%/*}:$PATH" \
    cargo build --manifest-path "$BUILD_HOME/Cargo.toml" --target "$target" --profile "$PROFILE" -p kos-nitro

    jni_dir="${ANDROID_JNI[$i]}"
    mkdir -p "$NITRO_ANDROID_JNI_DIR/$jni_dir"
    cp -f "$ROOT_HOME/target/$target/$PROFILE/libkos_nitro.a" "$NITRO_ANDROID_JNI_DIR/$jni_dir/libkos_nitro.a"
done

echo "Android static libraries copied to: $NITRO_ANDROID_JNI_DIR"
