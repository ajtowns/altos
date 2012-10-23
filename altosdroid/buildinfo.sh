#!/bin/sh
#

describe=$(git describe --always 2>/dev/null || echo '')
if [ -n "$describe" ]; then
   version=$(echo $describe | cut -d- -f1)
   commitnum=$(echo $describe | cut -d- -f2)
   commithash=$(echo $describe | cut -d- -f3)
else
   . ../src/Version
   version=$VERSION
   commitnum=''
   commithash=''
fi

builddate=$(date "+%Y-%m-%d")
buildtime=$(date "+%H:%M")


infile=src/org/altusmetrum/AltosDroid/BuildInfo.java.in
outfile=src/org/altusmetrum/AltosDroid/BuildInfo.java

echo "Version $describe, built on $builddate, $buildtime"

sed -e "s/@DESCRIBE@/$describe/" \
    -e "s/@VERSION@/$version/" \
    -e "s/@COMMITNUM@/$commitnum/" \
    -e "s/@COMMITHASH@/$commithash/" \
    -e "s/@BUILDDATE@/$builddate/" \
    -e "s/@BUILDTIME@/$buildtime/" \
 $infile > $outfile
