#!/bin/bash
cmake -S . -B build -G "Unix Makefiles" -DCMAKE_EXPORT_COMPILE_COMMANDS=ON
