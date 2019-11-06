#!/usr/bin/env bash

for file in $(find . -name CMakeCache.txt); do 
  rm -vrf ${file}
done

for file in $(find . -name CMakeFiles); do 
  rm -vrf ${file}
done

for file in $(find . -name cmake_install.cmake); do
  rm -vrf ${file}
done
