cmake_minimum_required(VERSION 3.16)
project(Double_Trouble C)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)

include(cmake/config.cmake)
include(cmake/directories.cmake)
include(cmake/targets/core.cmake)
include(cmake/targets/test.cmake)
include(cmake/targets/game.cmake)
include(cmake/targets/editor.cmake)
include(cmake/linking.cmake)