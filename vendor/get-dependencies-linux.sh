#!/usr/bin/env sh

# tukan
if [ ! -f tukan/README.md ]; then
  git clone https://github.com/phresnel/tukan.git tukan
fi

# catch
if [ ! -f catchorg/catch2/catch.hpp ]; then
  mkdir -p catchorg/catch2/
  cd catchorg/catch2
    wget https://github.com/catchorg/Catch2/releases/download/v2.4.2/catch.hpp
  cd -
fi

# cimg
if [ ! -f cimg/CImg/CImg.h ]; then
  mkdir -p cimg/CImg/
  cd cimg/CImg/
    wget https://framagit.org/dtschump/CImg/raw/v.2.4.2/CImg.h
  cd -
fi

# stb
if [ ! -f stb/stb/stb_image.h ]; then
  mkdir -p stb/stb/
  cd stb/stb/
    wget https://raw.githubusercontent.com/nothings/stb/master/stb_image.h
    wget https://raw.githubusercontent.com/nothings/stb/master/stb_image_write.h
    wget https://raw.githubusercontent.com/nothings/stb/master/stb_image_resize.h
    wget https://raw.githubusercontent.com/nothings/stb/master/stb_perlin.h
  cd -
fi

# boost
if [ ! -f boost_1_67_0/bootstrap.sh ]; then
  wget -c https://dl.bintray.com/boostorg/release/1.67.0/source/boost_1_67_0.tar.bz2
  tar --bzip2 -xf boost_1_67_0.tar.bz2
fi;
if [ ! -f boost_1_67_0/b2 ]; then
  cd boost_1_67_0
    ./bootstrap.sh
  cd -
fi
if [ ! -d boost_1_67_0/stage ]; then
  cd boost_1_67_0
    ./b2 --layout=versioned --build-type=complete
  cd -
fi
