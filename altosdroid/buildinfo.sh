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
branch=''
commitnum=''
commithash=''
builddate=$(date "+%Y-%m-%d")
buildtime=$(date "+%H:%M")
buildtz=$(date "+%z")


describe=$(git describe --match "$version" --long --always 2>/dev/null || echo '')
if [ -n "$describe" ]; then
   branch=$(git branch | sed -ne 's/^\* //p')
   commitdetails=$(echo $describe | sed -e "s/^$version-//")
   commitnum=$(echo $commitdetails | cut -s -d- -f1)
   commithash=$(echo $commitdetails | cut -d- -f2)
fi


echo "Version $describe, built on $builddate $buildtime $buildtz"

sed -e "s/@VERSION@/$version/" \
    -e "s/@DESCRIBE@/$describe/" \
    -e "s/@BRANCH@/$branch/" \
    -e "s/@COMMITNUM@/$commitnum/" \
    -e "s/@COMMITHASH@/$commithash/" \
    -e "s/@BUILDDATE@/$builddate/" \
    -e "s/@BUILDTIME@/$buildtime/" \
    -e "s/@BUILDTZ@/$buildtz/" \
 $infile > $outfile
