#!/bin/sh

# prerequisites
#
# 1. add assembly comxd
# 2. add Toolchain
# 3. set search path for umk
#
PLATFORM=windows
OUTPUT_DIR=/d/BUILD-OUT/comxd

if [ ! "$(uname -a | grep MINGW)" = "" ]; then
	OUTPUT_EXT=.exe
else
	OUTPUT_EXT=
fi

BUILD_METHODS="MSVS22x64 MSVS22"

if [ ! -d $OUTPUT_DIR/dist ]; then
	mkdir $OUTPUT_DIR/dist
fi

for bm in ${BUILD_METHODS}; do
	ARCH=$(echo $bm | grep -Eo x64)
	umk comxd comxd $bm -abru +GUI,STATIC,ENABLE_ICU $OUTPUT_DIR
	if [ -x ${OUTPUT_DIR}/comxd${OUTPUT_EXT} ]; then
		cp ${OUTPUT_DIR}/comxd${OUTPUT_EXT} ${OUTPUT_DIR}/dist/comxd${ARCH}_icu${OUTPUT_EXT}
	fi
	umk comxd comxd $bm -abru +GUI,STATIC,DISABLE_ICU $OUTPUT_DIR
	if [ -x ${OUTPUT_DIR}/comxd${OUTPUT_EXT} ]; then
		cp ${OUTPUT_DIR}/comxd${OUTPUT_EXT} ${OUTPUT_DIR}/dist/comxd${ARCH}${OUTPUT_EXT}
	fi
done
