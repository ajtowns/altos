#!/bin/sh
# jenkins.sh
# This script is used by Jenkins to perform a complete rebuild of Altos


prefix="--prefix=/usr/local"
ANDROID_SDK="${ANDROID_SDK:-$HOME/android-sdk-linux}"
android="--with-android=$ANDROID_SDK"

echo "=== starting altos build at $(date) ==="
env
echo "======================================="
set -x

./autogen.sh $prefix $android
make -j $(nproc) clean
time make -j $(nproc)
time make -j $(nproc) fat
