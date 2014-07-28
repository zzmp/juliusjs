#!/bin/sh

# (c) 2014 Zachary Pomerantz, @zzmp

###
# This script will emscript the Julius SRE
#
# $1 (-j4)- `make` arguments
###
MK_ARG=${1:-'-j4'};

# Check dependencies
which cvs ||         { echo 'Missing cvs. Install cvs';
                       echo 'See http://savannah.nongnu.org/projects/cvs';
                       exit 1; }
which git ||         { echo 'Missing git. Install git';
                       exit 1; }
which autoconf ||    { echo 'Missing autoconf. Install autoconf';
                       exit 1; }
which emconfigure || { echo 'Missing emconfigure';
                       echo 'Add emconfigure (from emscripten) to your path.';
                       exit 1; }
which emcc ||        { echo 'Missing emcc';
                       echo 'Add emcc (from emscripten) to your path.';
                       exit 1; }

mkdir -p src
mkdir -p bin
mkdir -p js
pushd src

# Grab the sourcecode (version 4.3.1)
cvs -z3 -d:pserver:anonymous@cvs.sourceforge.jp:/cvsroot/julius co -r release_4_3_1 julius4
# Make a clean copy from which to emscript
cp -r julius4 emscripted

# Build local executables
# - this will be used for generating custom grammars
#   and producing binary inputs to reduce network usage
pushd julius4
./configure --disable-pthread
make $MK_ARG
find . -type f -perm +111 -not -regex '.*[sh|in]$' -not -regex '.*config.*' -exec cp -f {} ../../bin \;
pushd ../../bin
# this may need to be customized to your system, dependent on what clang emits
ls *.dSYM | sed 's/\(.*\)\(\.dSYM\)/mv \1\2 \1/' | sh
popd
popd

# Build julius.js
pushd emscripted
# - build intermediary targets
# -- add Web Audio adin_mic library
pushd libsent
cp ../../include/libsent/configure.in .
# -- autoconf configure.in (can't get this to work, so just cp configure)
cp ../../include/libsent/configure .
cp ../../include/libsent/src/adin/adin_mic_webaudio.c src/adin/.
popd
pushd libjulius
cp ../../include/libjulius/src/m_adin.c src/.
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

# -- increase optimization for codesize
sed s/-O2/-Os/g < configure > tmp && mv tmp configure
chmod 751 configure
for subd in $(find . -type d -maxdepth 1); do
  pushd "$subd"
  if [ -e configure ]; then
    sed s/-O2/-O3/g < configure > tmp && mv tmp configure
    chmod 751 configure
  fi
  popd
done

# -- remove implicit declarations per C99 errors
pushd julius
grep -Ev 'j_process_remove' module.c > tmp && mv tmp module.c
grep -Ev 'j_process_lm_remove' module.c > tmp && mv tmp module.c
popd

# -- emscript
emconfigure ./configure --disable-pthread --with-mictype=webaudio
emmake make -j4
mv julius/julius julius/julius.bc

popd

# - build zlib intermediary targets
mkdir -p include
pushd include
# -- zlib
curl http://zlib.net/zlib-1.2.8.tar.gz | tar zx
mv zlib-1.2.8 zlib
pushd zlib
emconfigure ./configure
emmake make
popd
popd

popd

# - build javascript package
pushd js

# -- grab a recent voxforge LM
mkdir -p voxforge
pushd voxforge
curl http://www.repository.voxforge1.org/downloads/Main/Tags/Releases/0_1_1-build726/Julius_AcousticModels_16kHz-16bit_MFCC_O_D_\(0_1_1-build726\).tgz | tar zx
popd

emcc -O3 ../src/emscripted/julius/julius.bc -L../src/include/zlib -lz -o recognizer.js --preload-file voxforge -s INVOKE_RUN=0 -s NO_EXIT_RUNTIME=1 -s ALLOW_MEMORY_GROWTH=1 -s BUILD_AS_WORKER=1 -s EXPORTED_FUNCTIONS="['_main', '_main_event_recognition_stream_loop', '_end_event_recognition_stream_loop', '_event_recognize_stream', '_get_rate', '_fill_buffer']"

popd
