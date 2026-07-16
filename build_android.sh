#!/usr/bin/env bash
# Build libkos_nitro.a for the Android ABIs and place them in jniLibs.
set -euo pipefail

cd "$(dirname "$0")"

CRATE=kos-nitro
LIB=libkos_nitro.a
PROFILE=release
JNILIBS="android/src/main/jniLibs"

# Locate an NDK if not already exported.
if [[ -z "${ANDROID_NDK_HOME:-}" ]]; then
  SDK="${ANDROID_HOME:-$HOME/Library/Android/sdk}"
  ANDROID_NDK_HOME="$(ls -d "$SDK"/ndk/* 2>/dev/null | sort -V | tail -1)"
  export ANDROID_NDK_HOME
fi
echo ">> Using NDK: $ANDROID_NDK_HOME"

# ABI -> rust target triple
ABIS=("arm64-v8a" "armeabi-v7a" "x86_64" "x86")
TRIPLES=("aarch64-linux-android" "armv7-linux-androideabi" "x86_64-linux-android" "i686-linux-android")

echo ">> Ensuring rust targets are installed"
rustup target add "${TRIPLES[@]}"

echo ">> Cross-compiling for: ${ABIS[*]}"
NDK_ARGS=()
for abi in "${ABIS[@]}"; do NDK_ARGS+=("-t" "$abi"); done
cargo ndk "${NDK_ARGS[@]}" build --"$PROFILE" -p "$CRATE"

for i in "${!ABIS[@]}"; do
  abi="${ABIS[$i]}"; triple="${TRIPLES[$i]}"
  src="target/$triple/$PROFILE/$LIB"
  dst="$JNILIBS/$abi/$LIB"
  mkdir -p "$JNILIBS/$abi"
  cp "$src" "$dst"
  echo ">> $abi <- $src"
done

echo ">> Done. jniLibs:"
find "$JNILIBS" -name "$LIB"
