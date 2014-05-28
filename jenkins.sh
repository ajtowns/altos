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
# NOTE: the build process may fail on multi-cpu systems. If it fails try setting cpus=1
# cpus=$(nproc)
cpus=1

echo "=== starting altos build at $(date) ==="
env
echo "======================================="
set -x

./autogen.sh $prefix $android
make -j $cpus clean
$time make -j $cpus all fat
