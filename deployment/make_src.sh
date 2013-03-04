#!/bin/sh
mkdir package
cp -r ../src package
cp ../README package
cp ../FindIrrlicht.cmake package
cp ../CMakeLists.txt package
mkdir package/deployment
cp irrlamb irrlamb.desktop irrlamb.xpm license.txt changelog.txt package/deployment
cp -r ../working/ package

rm -rf irrlamb-$1-src
mv package irrlamb-$1-src
tar -czf irrlamb-$1-src.tar.gz irrlamb-$1-src
