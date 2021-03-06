#!/bin/bash
version=`grep 'GAME_VERSION=".*"' -o ../CMakeLists.txt | sed -r "s/GAME_VERSION=\"(.*)\"/\1/"`
base=irrlamb-${version}
pkg=${base}-src.tar.gz

tar --transform "s,^,${base}/," -czvf ${pkg} -C ../ src/ tools/ working/ deployment/ cmake/ CMakeLists.txt README CHANGELOG LICENSE --exclude=$pkg --exclude=move.{sh,bat} --exclude=*.swp --exclude=*.nsi --exclude=make_src.sh

echo -e "\nMade ${pkg}"
