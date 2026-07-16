#!/bin/bash
set -e

BUILD_HOME="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
ROOT_HOME="$(cd "$BUILD_HOME/../.." && pwd)"
NITRO_IOS_DIR="$ROOT_HOME/ios"

PROFILE="mobile"
IOS_TARGETS=("x86_64-apple-ios" "aarch64-apple-ios-sim" "aarch64-apple-ios")

echo "Building kos-nitro iOS static libraries..."

for target in "${IOS_TARGETS[@]}"; do
    rustup target add "$target"
done

for target in "${IOS_TARGETS[@]}"; do
    echo "Building $target..."
    PROTOC="${PROTOC:-/opt/homebrew/bin/protoc}" \
    PATH="${PROTOC%/*}:$PATH" \
    cargo build --manifest-path "$BUILD_HOME/Cargo.toml" --target "$target" --profile "$PROFILE" -p kos-nitro
done

SIM_FAT_DIR="$BUILD_HOME/sim_fat"
SIM_FAT="$SIM_FAT_DIR/libkos_nitro.a"
mkdir -p "$SIM_FAT_DIR"

lipo -create \
    "$ROOT_HOME/target/aarch64-apple-ios-sim/$PROFILE/libkos_nitro.a" \
    "$ROOT_HOME/target/x86_64-apple-ios/$PROFILE/libkos_nitro.a" \
    -output "$SIM_FAT"

mkdir -p "$NITRO_IOS_DIR"
rm -rf "$NITRO_IOS_DIR/KosNitro.xcframework"

xcodebuild -create-xcframework \
    -library "$ROOT_HOME/target/aarch64-apple-ios/$PROFILE/libkos_nitro.a" \
    -library "$SIM_FAT" \
    -output "$NITRO_IOS_DIR/KosNitro.xcframework"

rm -rf "$SIM_FAT_DIR"

echo "KosNitro.xcframework created at: $NITRO_IOS_DIR/KosNitro.xcframework"
