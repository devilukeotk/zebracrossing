cmake_minimum_required(VERSION 2.8)

project( ZebraDetector )

find_package( OpenCV REQUIRED )

add_executable( ZebraDetector src/ZebraDetector.cpp )

target_link_libraries( ZebraDetector ${OpenCV_LIBS} )
