#!/bin/bash
./build.sh
make
git clone https://github.com/AFLplusplus/AFLplusplus.git
pushd AFLplusplus
make distrib
popd
