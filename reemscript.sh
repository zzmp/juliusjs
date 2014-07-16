###
# This script will emscript the Julius SRE
#
# $1 (-j4)- `make` arguments
###
MK_ARG=${1:-'-j4'};

# Build julius.js intermediary targets
pushd src/emscripted
# Update Web Audio adin_mic library
pushd libsent
cp -f ../../include/libsent/src/adin/adin_mic_webaudio.c src/adin/.
popd
pushd libjulius
cp -f ../../include/libjulius/src/m_adin.c src/.
popd

# emscript
emmake make -j4
mv julius/julius julius/julius.bc

popd

# Build javascript package
pushd js
emcc -O3 ../src/emscripted/julius/julius.bc -L../src/include/zlib -lz -o julius.html --preload-file voxforge -s INVOKE_RUN=0 -s NO_EXIT_RUNTIME=1 -s OUTLINING_LIMIT=100000 -s ALLOW_MEMORY_GROWTH=1 -s EXPORTED_FUNCTIONS="['_main', '_get_rate', '_fill_buffer']"
popd

