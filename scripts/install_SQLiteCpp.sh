# shellcheck disable=SC2164
cd ../third_party/SQLiteCpp
cmake -B ./build
cmake --build ./build -j8
cd build
sudo make install
cd ..
rm -r build
cd ..