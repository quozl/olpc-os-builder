#!/bin/bash -x
# get the debian rootfs into the cache

# the following sources os-builder-root/lib/shlib.sh (OOB__shlib is in env)
. $OOB__shlib

missing=
for x in debootstrap make gcc zip; do
   which $x >/dev/null || missing="$x, $missing"
done
if [ ! -z "$missing" ]; then
    missing=${missing:: -2}
    echo -e "\nMissing packages, please install $missing." >&2
    exit 1
fi

debian_release=$(read_config debian debian_release)
mkdir -p $cachedir/rootfs
if [ ! -f $cachedir/rootfs/root/debian_cache ];then
  mkdir -p $cachedir/rootfs/root
  debootstrap --arch i386 $debian_release $cachedir/rootfs ftp://ftp.us.debian.org/debian 
  echo "This file may be deleted. It was used during automated build" > \
		$cachedir/rootfs/root/debian_cache
fi
