cmake_minimum_required(VERSION 2.8)

project( RotateVideo )

find_package( OpenCV REQUIRED )

add_executable( RotateVideo src/RotateVideo.cpp )

target_link_libraries( RotateVideo ${OpenCV_LIBS} )