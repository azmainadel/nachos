#! /usr/bin/bash

make
cd threads/
./nachos -rs $1
