#!/bin/sh
#
# Author: Mike Beattie <mike@ethernal.org>
#
# Script to parse result from git describe, and push values into
# BuildInfo.java for use within altosdroid (to display the current
# version and build information, primarily).
#

infile=src/org/altusmetrum/AltosDroid/BuildInfo.java.in
outfile=src/org/altusmetrum/AltosDroid/BuildInfo.java

. ../src/Version
version=$VERSION
commitnum=''
commithash=''
builddate=$(date "+%Y-%m-%d")
buildtime=$(date "+%H:%M")


describe=$(git describe --match "$version" --long --always 2>/dev/null || echo '')
if [ -n "$describe" ]; then
   commitdetails=$(echo $describe | sed -e "s/^$version-//")
   commitnum=$(echo $commitdetails | cut -d- -f1)
   commithash=$(echo $commitdetails | cut -d- -f2)
fi


echo "Version $describe, built on $builddate, $buildtime"

sed -e "s/@DESCRIBE@/$describe/" \
    -e "s/@VERSION@/$version/" \
    -e "s/@COMMITNUM@/$commitnum/" \
    -e "s/@COMMITHASH@/$commithash/" \
    -e "s/@BUILDDATE@/$builddate/" \
    -e "s/@BUILDTIME@/$buildtime/" \
 $infile > $outfile
