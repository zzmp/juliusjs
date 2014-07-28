#!/bin/sh

# (c) 2014 Zachary Pomerantz, @zzmp

###
# This script will remake the Julius SRE
# It should only be used after `emscript.sh` has been run
#
# $1 (-j4)- `make` arguments
###
MK_ARG=${1:-'-j4'};

# Build julius.js
pushd src/emscripted
# - build intermediary targets
# -- update Web Audio adin_mic library
pushd libsent
cp -f ../../include/libsent/src/adin/adin_mic_webaudio.c src/adin/.
popd
pushd libjulius
cp -f ../../include/libjulius/src/m_adin.c src/.
popd
# -- update app.h routines for (evented) multithreading
pushd julius
cp -f ../../include/julius/app.h .
cp -f ../../include/julius/main.c .
cp -f ../../include/julius/recogloop.c .
popd
# -- update libjulius for (evented) multithreading
pushd libjulius
cp -f ../../include/libjulius/src/recogmain.c src/.
cp -f ../../include/libjulius/src/adin-cut.c src/.
popd

# -- emscript
emmake make -j4
mv julius/julius julius/julius.bc

popd

# - build javascript package
pushd js
emcc -O3 ../src/emscripted/julius/julius.bc -L../src/include/zlib -lz -o recognizer.js --preload-file voxforge -s INVOKE_RUN=0 -s NO_EXIT_RUNTIME=1 -s ALLOW_MEMORY_GROWTH=1 -s BUILD_AS_WORKER=1 -s EXPORTED_FUNCTIONS="['_main', '_main_event_recognition_stream_loop', '_end_event_recognition_stream_loop', '_event_recognize_stream', '_get_rate', '_fill_buffer']" 
popd
