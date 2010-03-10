#! /bin/sh

JDKS_PREFIX="${JDKS_PREFIX:-$HOME/if2kv2}"
export JDKS_PREFIX

mkdir -p "${JDKS_PREFIX}"

DSTROOT="$JDKS_PREFIX"
export DSTROOT

#cd ../../if2k/platform/macosx/ && xcodebuild -target all -configuration Release install "DSTROOT=$DSTROOT"

cp ../../if2k/platform/macosx/build/Release/If2k.dmg "$JDKS_PREFIX"
