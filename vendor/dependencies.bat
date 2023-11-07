set LIB_PATH=%CD%/lib
set SRC_PATH=%CD%/src
set CMAKE_OPTIONS=-DCMAKE_DEBUG_POSTFIX=_debug -DCMAKE_FIND_USE_SYSTEM_ENVIRONMENT_PATH=FALSE -DCMAKE_FIND_USE_CMAKE_ENVIRONMENT_PATH=FALSE -DCMAKE_POLICY_DEFAULT_CMP0144=NEW

IF NOT EXIST %SRC_PATH% mkdir %SRC_PATH%
git -C %SRC_PATH%/blosc pull || git clone https://github.com/Blosc/c-blosc.git %SRC_PATH%/blosc
git -C %SRC_PATH%/eigen pull || git clone https://gitlab.com/libeigen/eigen.git %SRC_PATH%/eigen
git -C %SRC_PATH%/flann pull || git clone https://github.com/3Descape/flann.git -b lz4 %SRC_PATH%/flann
git -C %SRC_PATH%/glad pull || git clone https://github.com/Dav1dde/glad.git %SRC_PATH%/glad
git -C %SRC_PATH%/glm pull || git clone https://github.com/g-truc/glm.git %SRC_PATH%/glm
git -C %SRC_PATH%/hdf5 pull || git clone https://github.com/HDFGroup/hdf5_plugins.git %SRC_PATH%/hdf5
git -C %SRC_PATH%/glfw pull || git clone https://github.com/glfw/glfw.git %SRC_PATH%/glfw
git -C %SRC_PATH%/imgui pull || git clone https://github.com/ocornut/imgui.git -b docking %SRC_PATH%/imgui
git -C %SRC_PATH%/lz4 pull || git clone https://github.com/lz4/lz4.git %SRC_PATH%/lz4
git -C %SRC_PATH%/nrrd pull || git clone https://git.code.sf.net/p/nrrd-cpp/code %SRC_PATH%/nrrd
git -C %SRC_PATH%/nfd pull || git clone https://github.com/btzy/nativefiledialog-extended.git %SRC_PATH%/nfd
git -C %SRC_PATH%/onetbb pull || git clone https://github.com/oneapi-src/oneTBB.git %SRC_PATH%/onetbb
git -C %SRC_PATH%/openvdb pull || git clone https://github.com/AcademySoftwareFoundation/openvdb.git %SRC_PATH%/openvdb
git -C %SRC_PATH%/pcl pull || git clone https://github.com/3Descape/pcl.git %SRC_PATH%/pcl
git -C %SRC_PATH%/tinyply pull || git clone https://github.com/ddiakopoulos/tinyply.git %SRC_PATH%/tinyply
git -C %SRC_PATH%/vtk pull || git clone https://github.com/Kitware/VTK.git %SRC_PATH%/vtk
git -C %SRC_PATH%/zlib pull || git clone https://github.com/madler/zlib.git %SRC_PATH%/zlib

cmake -B ./build/tbb -S %SRC_PATH%/onetbb %CMAKE_OPTIONS% -DCMAKE_INSTALL_PREFIX=%LIB_PATH%/onetbb -DTBB_TEST=OFF
cmake --build ./build/tbb --config Debug -j
cmake --install ./build/tbb --config Debug
cmake --build ./build/tbb --config Release -j
cmake --install ./build/tbb --config Release

cmake -B ./build/blosc -S %SRC_PATH%/blosc %CMAKE_OPTIONS% -DCMAKE_INSTALL_PREFIX=%LIB_PATH%/blosc -DBUILD_BENCHMARKS=OFF -DBUILD_FUZZERS=OFF -DBUILD_TESTS=OFF
cmake --build ./build/blosc --config Debug -j
cmake --install ./build/blosc --config Debug
cmake --build ./build/blosc --config Release -j
cmake --install ./build/blosc --config Release

cmake -B ./build/zlib -S %SRC_PATH%/zlib %CMAKE_OPTIONS% -DCMAKE_INSTALL_PREFIX=%LIB_PATH%/zlib
cmake --build ./build/zlib --config Debug -j
cmake --install ./build/zlib --config Debug
cmake --build ./build/zlib --config Release -j
cmake --install ./build/zlib --config Release

cmake -B ./build/openvdb -S %SRC_PATH%/openvdb %CMAKE_OPTIONS% --fresh -DCMAKE_INSTALL_PREFIX=%LIB_PATH%/openvdb -DUSE_EXPLICIT_INSTANTIATION=OFF -DTBB_ROOT=%LIB_PATH%/onetbb -DBLOSC_ROOT=%LIB_PATH%/blosc -DBLOSC_DEBUG_SUFFIX="_debug" -DZLIB_ROOT=%LIB_PATH%/zlib -DOPENVDB_BUILD_BINARIES=OFF
cmake --build ./build/openvdb --config Debug -j
cmake --install ./build/openvdb --config Debug
cmake --build ./build/openvdb --config Release -j
cmake --install ./build/openvdb --config Release

cmake -B ./build/glm -S %SRC_PATH%/glm %CMAKE_OPTIONS% -DCMAKE_INSTALL_PREFIX=%LIB_PATH%/glm -DBUILD_TESTING=OFF
cmake --build ./build/glm --config Debug -j
cmake --install ./build/glm --config Debug
cmake --build ./build/glm --config Release -j
cmake --install ./build/glm --config Release

cmake -B ./build/glfw -S %SRC_PATH%/glfw %CMAKE_OPTIONS% -DCMAKE_INSTALL_PREFIX=%LIB_PATH%/glfw -DGLFW_BUILD_DOCS=OFF -DGLFW_BUILD_EXAMPLES=OFF -DGLFW_BUILD_TESTS=OFF
cmake --build ./build/glfw --config Debug -j
cmake --install ./build/glfw --config Debug
cmake --build ./build/glfw --config Release -j
cmake --install ./build/glfw --config Release

cmake -B ./build/glad -S ./glad %CMAKE_OPTIONS% -DCMAKE_INSTALL_PREFIX=%LIB_PATH%/glad
cmake --build ./build/glad --config Debug -j
cmake --install ./build/glad --config Debug
cmake --build ./build/glad --config Release -j
cmake --install ./build/glad --config Release

cmake -B ./build/tinyply -S %SRC_PATH%/tinyply %CMAKE_OPTIONS% -DCMAKE_INSTALL_PREFIX=%LIB_PATH%/tinyply
cmake --build ./build/tinyply --config Debug -j
cmake --install ./build/tinyply --config Debug
cmake --build ./build/tinyply --config Release -j
cmake --install ./build/tinyply --config Release

cmake -B ./build/lz4 -S %SRC_PATH%/lz4/build/cmake %CMAKE_OPTIONS% -DCMAKE_INSTALL_PREFIX=%LIB_PATH%/lz4 -DBUILD_STATIC_LIBS=ON -DLZ4_BUILD_CLI=OFF -DLZ4_BUILD_LEGACY_LZ4C=OFF
cmake --build ./build/lz4 --config Debug -j
cmake --install ./build/lz4 --config Debug
cmake --build ./build/lz4 --config Release -j
cmake --install ./build/lz4 --config Release

cmake -B ./build/flann -S %SRC_PATH%/flann %CMAKE_OPTIONS% -DCMAKE_INSTALL_PREFIX=%LIB_PATH%/flann -DLZ4_ROOT=%LIB_PATH%/lz4 -DCMAKE_BUILD_STATIC_LIBS=ON -DBUILD_C_BINDINGS=OFF -DBUILD_DOC=OFF -DBUILD_EXAMPLES=OFF -DBUILD_MATLAB_BINDINGS=OFF -DBUILD_PYTHON_BINDINGS=OFF -DBUILD_TESTS=OFF -DCMAKE_POLICY_DEFAULT_CMP0074=NEW
cmake --build ./build/flann --config Debug -j
cmake --install ./build/flann --config Debug
cmake --build ./build/flann --config Release -j
cmake --install ./build/flann --config Release

cmake -B ./build/pcl -S %SRC_PATH%/pcl %CMAKE_OPTIONS% -DCMAKE_INSTALL_PREFIX=%LIB_PATH%/pcl -DFLANN_ROOT=%LIB_PATH%/flann -DZLIB_ROOT=%LIB_PATH%/zlib -DWITH_CUDA=OFF -DLZ4_ROOT=%LIB_PATH%/lz4 -DBUILD_ml=OFF -DBUILD_segmentation=OFF -DBUILD_stereo=OFF -DBUILD_tools=OFF -DBUILD_tracking=OFF -DWITH_OPENGL=OFF -DWITH_VTK=OFF -DPCL_ONLY_CORE_POINT_TYPES=OFF
cmake --build ./build/pcl --config Debug -j
cmake --install ./build/pcl --config Debug
cmake --build ./build/pcl --config Release -j
cmake --install ./build/pcl --config Release

cmake -B ./build/eigen -S %SRC_PATH%/eigen %CMAKE_OPTIONS% -DCMAKE_INSTALL_PREFIX=%LIB_PATH%/eigen -DEIGEN_BUILD_DOC=OFF -DEIGEN_BUILD_TESTING=OFF -DBUILD_TESTING=OFF
cmake --build ./build/eigen --config Debug -j
cmake --install ./build/eigen --config Debug
cmake --build ./build/eigen --config Release -j
cmake --install ./build/eigen --config Release

cmake -B ./build/nfd -S %SRC_PATH%/nfd %CMAKE_OPTIONS% -DCMAKE_INSTALL_PREFIX=%LIB_PATH%/nfd -DNFD_BUILD_TESTS=OFF
cmake --build ./build/nfd --config Debug -j
cmake --install ./build/nfd --config Debug
cmake --build ./build/nfd --config Release -j
cmake --install ./build/nfd --config Release

cmake -B ./build/imgui -S ./imgui %CMAKE_OPTIONS% -DCMAKE_INSTALL_PREFIX=%LIB_PATH%/imgui -DGLFW3_ROOT=%LIB_PATH%/glfw
cmake --build ./build/imgui --config Debug -j
cmake --install ./build/imgui --config Debug
cmake --build ./build/imgui --config Release -j
cmake --install ./build/imgui --config Release

cd %SRC_PATH%/nrrd
git restore NRRD/nrrd.hxx
git apply ../../nrrd/gzip.diff
cd ../../
cmake -B ./build/nrrd -S ./nrrd %CMAKE_OPTIONS% -DCMAKE_INSTALL_PREFIX=%LIB_PATH%/nrrd
cmake --install ./build/nrrd --config Release