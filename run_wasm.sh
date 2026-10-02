#!/bin/bash

cd ~/Documents/code/personal/LooseChange-olcCodeJam2026
./build_wasm.sh
cd build-wasm
emrun LooseChange.html
