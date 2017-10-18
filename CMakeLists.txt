cmake_minimum_required(VERSION 3.8)
project(RWdemo)

set(CMAKE_CXX_STANDARD 11)

set(SOURCE_FILES main.cpp GDALOPENCV.h GDALOPENCV.cpp temp.cpp)
add_executable(RWdemo ${SOURCE_FILES})

find_package(OpenCV REQUIRED)
target_link_libraries(RWdemo ${OpenCV_LIBS})

#find_package(gdal REQUIRED)
#LDFLAGS:  -L/usr/local/opt/gdal2/lib
#CPPFLAGS: -I/usr/local/opt/gdal2/include

# If the package has been found, several variables will
# be set, you can find the full list with descriptions
# in the OpenCVConfig.cmake file.()
# Print some message showing some of them
#message(STATUS "gdal library status:")
#message(STATUS "    version: ${GDAL_VERSION}")
#message(STATUS "    libraries: ${GDAL_LIBRARIES}")
#message(STATUS "    include path: ${GDAL_INCLUDE_DIRS}")
#include_directories(${GDAL_INCLUDE_DIRS})
#link_directories(${GDAL_INCLUDE_DIRS})
#target_link_libraries(RWdemo ${GDAL})


include_directories(/usr/local/Cellar/gdal2/2.2.2/include)
link_directories(/usr/local/Cellar/gdal2/2.2.2/include)
target_link_libraries(RWdemo /usr/local/Cellar/gdal2/2.2.2/lib/libgdal.dylib)