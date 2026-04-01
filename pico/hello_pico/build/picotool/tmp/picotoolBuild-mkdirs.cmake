# Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
# file Copyright.txt or https://cmake.org/licensing for details.

cmake_minimum_required(VERSION 3.5)

file(MAKE_DIRECTORY
  "/home/pi/access_control_system/pico/hello_pico/build/_deps/picotool-src"
  "/home/pi/access_control_system/pico/hello_pico/build/_deps/picotool-build"
  "/home/pi/access_control_system/pico/hello_pico/build/_deps"
  "/home/pi/access_control_system/pico/hello_pico/build/picotool/tmp"
  "/home/pi/access_control_system/pico/hello_pico/build/picotool/src/picotoolBuild-stamp"
  "/home/pi/access_control_system/pico/hello_pico/build/picotool/src"
  "/home/pi/access_control_system/pico/hello_pico/build/picotool/src/picotoolBuild-stamp"
)

set(configSubDirs )
foreach(subDir IN LISTS configSubDirs)
    file(MAKE_DIRECTORY "/home/pi/access_control_system/pico/hello_pico/build/picotool/src/picotoolBuild-stamp/${subDir}")
endforeach()
if(cfgdir)
  file(MAKE_DIRECTORY "/home/pi/access_control_system/pico/hello_pico/build/picotool/src/picotoolBuild-stamp${cfgdir}") # cfgdir has leading slash
endif()
