#!/usr/bin/env bash
# Build libkos_nitro.a for iOS device + simulator and assemble the xcframework.
set -euo pipefail

cd "$(dirname "$0")"

CRATE=kos-nitro
LIB=libkos_nitro.a
PROFILE=release
TARGET_DIR="target"
OUT="ios/KosNitro.xcframework"

DEVICE_TARGET="aarch64-apple-ios"
SIM_TARGETS=("aarch64-apple-ios-sim" "x86_64-apple-ios")

echo ">> Ensuring rust targets are installed"
rustup target add "$DEVICE_TARGET" "${SIM_TARGETS[@]}"

echo ">> Building device ($DEVICE_TARGET)"
cargo build -p "$CRATE" --"$PROFILE" --target "$DEVICE_TARGET"

for t in "${SIM_TARGETS[@]}"; do
  echo ">> Building simulator ($t)"
  cargo build -p "$CRATE" --"$PROFILE" --target "$t"
done

BUILD="build/ios"
rm -rf "$BUILD"
mkdir -p "$BUILD/device" "$BUILD/sim"

cp "$TARGET_DIR/$DEVICE_TARGET/$PROFILE/$LIB" "$BUILD/device/$LIB"

echo ">> Creating fat simulator lib"
lipo -create \
  "$TARGET_DIR/aarch64-apple-ios-sim/$PROFILE/$LIB" \
  "$TARGET_DIR/x86_64-apple-ios/$PROFILE/$LIB" \
  -output "$BUILD/sim/$LIB"

echo ">> Assembling xcframework"
rm -rf "$OUT"
xcodebuild -create-xcframework \
  -library "$BUILD/device/$LIB" \
  -library "$BUILD/sim/$LIB" \
  -output "$OUT"

echo ">> Done: $OUT"
lipo -info "$BUILD/device/$LIB" "$BUILD/sim/$LIB"
