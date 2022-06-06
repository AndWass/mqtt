#!/bin/bash

find ./include -iname *.hpp | xargs clang-format -i
find ./examples -iname *.cpp | xargs clang-format -i
