#!/bin/sh
# jenkins.sh
# This script is used by Jenkins to perform a complete rebuild of Altos


prefix="--prefix=/usr/local"
ANDROID_SDK="${ANDROID_SDK:-$HOME/android-sdk-linux}"
android="--with-android=$ANDROID_SDK"
# use time if we have it
time=`which time`
if [ -n "$time" ]; then
    time="$time -v"
fi

echo "=== starting altos build at $(date) ==="
env
echo "======================================="
set -x

./autogen.sh $prefix $android
make -j $(nproc) clean
$time make -j $(nproc) all fat
