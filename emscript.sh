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

# Build julius.dSYM
# this will be used to produce binary inputs for julius.js to reduce network usage
cvs -z3 -d:pserver:anonymous@cvs.sourceforge.jp:/cvsroot/julius co julius4
# Make a clean copy from which to emscript
cp -r julius4 emscripted

# build local executables
pushd julius4
./configure --disable-pthread
make $MK_ARG
cp ./**/*.dSYM ../../bin
popd

# Build julius.js intermediary targets
pushd emscripted
# Add Web Audio adin_mic library
pushd libsent
cp ../../include/libsent/configure.in .
# autoconf configure.in (can't get this to work, so just cp configure)
cp ../../include/libsent/configure .
cp ../../include/libsent/src/adin/adin_mic_webaudio.c src/adin/.
popd
pushd libjulius
cp ../../include/libjulius/src/m_adin.c src/.
popd

# increase optimization level
sed s/-O2/-O3/g < configure > tmp && mv tmp configure
chmod 751 configure
for subd in $(find . -type d -maxdepth 1); do
  pushd "$subd"
  if [ -e configure ]; then
    sed s/-O2/-O3/g < configure > tmp && mv tmp configure
    chmod 751 configure
  fi
  popd
done

# remove implicit declarations per C99 errors
pushd julius
grep -Ev 'j_process_remove' module.c > tmp && mv tmp module.c
grep -Ev 'j_process_lm_remove' module.c > tmp && mv tmp module.c
popd

# emscript
emconfigure ./configure --disable-pthread --with-mictype=webaudio
emmake make -j4
mv julius/julius julius/julius.bc

popd

# Build zlib intermediary targets
mkdir -p include
pushd include
# zlib
curl http://zlib.net/zlib-1.2.8.tar.gz | tar zx
mv zlib-1.2.8 zlib
pushd zlib
emconfigure ./configure
emmake make
popd
popd

popd

# Build javascript package
pushd js

# Grab a recent voxforge LM
mkdir -p voxforge
pushd voxforge
curl http://www.repository.voxforge1.org/downloads/Main/Tags/Releases/0_1_1-build726/Julius_AcousticModels_16kHz-16bit_MFCC_O_D_\(0_1_1-build726\).tgz | tar zx
popd

emcc -O3 ../src/emscripted/julius/julius.bc -L../src/include/zlib -lz -o julius.html --preload-file voxforge -s INVOKE_RUN=0 -s NO_EXIT_RUNTIME=1 -s ALLOW_MEMORY_GROWTH=1 -s EXPORTED_FUNCTIONS="['_fill_buffer']"

popd

