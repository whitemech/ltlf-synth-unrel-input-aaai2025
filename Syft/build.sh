# Clear build
rm -rf build/*
sudo apt install libtool-bin g++ automake cmake flex bison mona libboost-all-dev 
cd ../external/cudd 
autoreconf -i 
./configure --enable-silent-rules --enable-obj --enable-dddmp 
sudo make install 
cd ../../Syft
mkdir build && cd build
cmake .. 
cmake --build .

