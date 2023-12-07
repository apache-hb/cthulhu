sudo apt-get install -y build-essential python3-dev automake cmake git flex bison libglib2.0-dev libpixman-1-dev python3-setuptools
sudo apt-get install -y lld llvm llvm-dev clang
sudo apt-get install -y gcc-$(gcc --version|head -n1|sed 's/\..*//'|sed 's/.* //')-plugin-dev libstdc++-$(gcc --version|head -n1|sed 's/\..*//'|sed 's/.* //')-dev

git clone https://github.com/AFLplusplus/AFLplusplus aflpp --depth 1
cd aflpp
make source-only NO_NYX=1 STATIC=1 NO_PYTHON=1 
make install
# disable NYX because we dont need it and it has a heap of deps
# build only static because its smaller
# disable python because we dont need it
