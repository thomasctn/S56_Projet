cd build/
rm -rf *
cmake ..
make -j $(nproc) 2>log.txt
cd ..
