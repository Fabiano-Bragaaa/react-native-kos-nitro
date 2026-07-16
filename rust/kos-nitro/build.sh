#!/bin/bash
set -e

BUILD_HOME="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"

if [[ "$1" == "ios" || -z "$1" ]]; then
    echo "Building iOS..."
    "$BUILD_HOME/build_ios.sh"
fi

if [[ "$1" == "android" || -z "$1" ]]; then
    echo "Building Android..."
    "$BUILD_HOME/build_android.sh"
fi

echo "Done."
