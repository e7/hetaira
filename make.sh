#! /bin/bash


BUILD_DIR="build"
if [[ ! -d ${BUILD_DIR} ]] ; then
    mkdir ${BUILD_DIR}
fi

cd build;cmake ..;make;cd ..
