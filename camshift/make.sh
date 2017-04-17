#/bin/bash
g++ main.cpp -std=c++11 `pkg-config opencv --libs --cflags opencv` -o  main