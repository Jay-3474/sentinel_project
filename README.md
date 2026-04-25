# sentinel_project
Basic c++ projects

#Vehicle status
# 1. Create a workspace for the library
mkdir -p ~/vsomeip_build && cd ~/vsomeip_build

# 2. Clone the specific version
git clone --branch 3.4.10 https://github.com/COVESA/vsomeip.git
cd vsomeip

# 2. Re-configure with the fix
cmake .. \
    -DCMAKE_INSTALL_PREFIX=../install \
    -DVSOMEIP_BOOST_VERSION=107400 \
    -DENABLE_UNIT_TESTS=OFF \
    -DWERROR=OFF \
    -DCMAKE_CXX_FLAGS="-I$(pwd)/../compat -Wno-error -Wno-stringop-overflow"

# 3. Build and Install
make -j$(nproc)
make install

# 1. Install dependencies
conan install . --build=missing

# 2. Build binaries (server and client will be in /build)
conan build .

# 3. Setup Library Path
export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:/home/jay/vsomeip_build/vsomeip/install/lib

# 4. Run Server in Terminal 1
./build/server

# 5. Run Client in Terminal 2
./build/client
