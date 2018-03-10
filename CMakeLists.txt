SET(PROJECT_NAME DSO)

PROJECT(${PROJECT_NAME})
CMAKE_MINIMUM_REQUIRED(VERSION 2.6)
#set(CMAKE_VERBOSE_MAKEFILE ON)


set(BUILD_TYPE Release)
#set(BUILD_TYPE RelWithDebInfo)

set(EXECUTABLE_OUTPUT_PATH bin)
set(LIBRARY_OUTPUT_PATH lib)
set(CMAKE_MODULE_PATH ${PROJECT_SOURCE_DIR}/cmake D:/Projects/opencv/build D:/Projects/suitesparse/built/cmake)

# required libraries
set(EIGEN3_INCLUDE_DIR D:/Projects/eigen-eigen-f562a193118d)
set(BOOST_ROOT D:/Projects/boost_1_63_0)
set(OpenCV_DIR D:/Projects/opencv/build)
set(CHOLMOD_INCLUDE_DIR D:/Projects/suitesparse/built/include/suitesparse)
set(CSPARSE_INCLUDE_DIR D:/Projects/suitesparse/built/include/suitesparse)
find_package(SuiteParse REQUIRED)
find_package(Eigen3 REQUIRED)
find_package(Boost COMPONENTS system thread)

# optional libraries
find_package(LibZip QUIET)
find_package(Pangolin 0.2 QUIET)
find_package(OpenCV QUIET)

# flags
add_definitions("-DENABLE_SSE")
set(CMAKE_CXX_FLAGS
        "${SSE_FLAGS} /bigobj"# -O3 -g -std=c++0x -march=native"
        #   "${SSE_FLAGS} -O3 -g -std=c++0x -fno-omit-frame-pointer"
        )

if (MSVC)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /EHsc")
endif (MSVC)

set(dso_HEADER_FILES
        ${PROJECT_SOURCE_DIR}/src/util/FrameShell.h
        ${PROJECT_SOURCE_DIR}/src/util/globalCalib.h
        ${PROJECT_SOURCE_DIR}/src/util/ImageAndExposure.h
        ${PROJECT_SOURCE_DIR}/src/util/MinimalImage.h
        ${PROJECT_SOURCE_DIR}/src/util/IndexThreadReduce.h
        ${PROJECT_SOURCE_DIR}/src/util/NumType.h
        ${PROJECT_SOURCE_DIR}/src/util/settings.h
        ${PROJECT_SOURCE_DIR}/src/util/Undistort.h
        ${PROJECT_SOURCE_DIR}/src/util/DatasetReader.h
        ${PROJECT_SOURCE_DIR}/src/util/DatasetReader_Image.h
        ${PROJECT_SOURCE_DIR}/src/util/DatasetReader_Video.h
        )
# Sources files
set(dso_SOURCE_FILES
        ${PROJECT_SOURCE_DIR}/src/FullSystem/FullSystem.cpp
        ${PROJECT_SOURCE_DIR}/src/FullSystem/FullSystemOptimize.cpp
        ${PROJECT_SOURCE_DIR}/src/FullSystem/FullSystemOptPoint.cpp
        ${PROJECT_SOURCE_DIR}/src/FullSystem/FullSystemDebugStuff.cpp
        ${PROJECT_SOURCE_DIR}/src/FullSystem/FullSystemMarginalize.cpp
        ${PROJECT_SOURCE_DIR}/src/FullSystem/Residuals.cpp
        ${PROJECT_SOURCE_DIR}/src/FullSystem/CoarseTracker.cpp
        ${PROJECT_SOURCE_DIR}/src/FullSystem/CoarseInitializer.cpp
        ${PROJECT_SOURCE_DIR}/src/FullSystem/ImmaturePoint.cpp
        ${PROJECT_SOURCE_DIR}/src/FullSystem/HessianBlocks.cpp
        ${PROJECT_SOURCE_DIR}/src/FullSystem/PixelSelector2.cpp
        ${PROJECT_SOURCE_DIR}/src/OptimizationBackend/EnergyFunctional.cpp
        ${PROJECT_SOURCE_DIR}/src/OptimizationBackend/AccumulatedTopHessian.cpp
        ${PROJECT_SOURCE_DIR}/src/OptimizationBackend/AccumulatedSCHessian.cpp
        ${PROJECT_SOURCE_DIR}/src/OptimizationBackend/EnergyFunctionalStructs.cpp
        ${PROJECT_SOURCE_DIR}/src/util/settings.cpp
        ${PROJECT_SOURCE_DIR}/src/util/Undistort.cpp
        ${PROJECT_SOURCE_DIR}/src/util/globalCalib.cpp
        )


include_directories(
        ${PROJECT_SOURCE_DIR}/src
        ${PROJECT_SOURCE_DIR}/thirdparty/Sophus
        #${PROJECT_SOURCE_DIR}/thirdparty/sse2neon
        ${EIGEN3_INCLUDE_DIR}
        D:/Projects/glew-2.0.0/include
)


# decide if we have pangolin
set(Pangolin_FOUND TRUE)
if (Pangolin_FOUND)
    message("--- found PANGOLIN, compiling dso_pangolin library.")
    include_directories("D:/Projects/Pangolin/Built/include")
    set(dso_pangolin_SOURCE_FILES
            ${PROJECT_SOURCE_DIR}/src/IOWrapper/Pangolin/KeyFrameDisplay.cpp
            ${PROJECT_SOURCE_DIR}/src/IOWrapper/Pangolin/PangolinDSOViewer.cpp)
    set(HAS_PANGOLIN 1)
else ()
    message("--- could not find PANGOLIN, not compiling dso_pangolin library.")
    message("    this means there will be no 3D display / GUI available for dso_dataset.")
    set(dso_pangolin_SOURCE_FILES)
    set(HAS_PANGOLIN 0)
endif ()

# decide if we have openCV
if (OpenCV_FOUND)
    message("--- found OpenCV, compiling dso_opencv library.")
    include_directories(${OpenCV_INCLUDE_DIRS})
    set(dso_opencv_SOURCE_FILES
            ${PROJECT_SOURCE_DIR}/src/IOWrapper/OpenCV/ImageDisplay_OpenCV.cpp
            ${PROJECT_SOURCE_DIR}/src/IOWrapper/OpenCV/ImageRW_OpenCV.cpp)
    set(HAS_OPENCV 1)
else ()
    message("--- could not find OpenCV, not compiling dso_opencv library.")
    message("    this means there will be no image display, and image read / load functionality.")
    set(dso_opencv_SOURCE_FILES
            ${PROJECT_SOURCE_DIR}/src/IOWrapper/ImageDisplay_dummy.cpp
            ${PROJECT_SOURCE_DIR}/src/IOWrapper/ImageRW_dummy.cpp)
    set(HAS_OPENCV 0)
endif ()

# decide if we have ziplib.
if (LIBZIP_LIBRARY)
    message("--- found ziplib (${LIBZIP_VERSION}), compiling with zip capability.")
    add_definitions(-DHAS_ZIPLIB=1)
    include_directories(${LIBZIP_INCLUDE_DIR_ZIP} ${LIBZIP_INCLUDE_DIR_ZIPCONF})
else ()
    message("--- not found ziplib (${LIBZIP_LIBRARY}), compiling without zip capability.")
    set(LIBZIP_LIBRARY "")
endif ()


# compile main library.
include_directories(${CSPARSE_INCLUDE_DIR} ${CHOLMOD_INCLUDE_DIR})
add_library(dso ${dso_HEADER_FILES} ${dso_SOURCE_FILES} ${dso_opencv_SOURCE_FILES} ${dso_pangolin_SOURCE_FILES})
if (MSVC)
    set_property(TARGET dso APPEND PROPERTY LINK_FLAGS /bigobj)
endif ()

#set_property( TARGET dso APPEND_STRING PROPERTY COMPILE_FLAGS -Wall )


# build main executable (only if we have both OpenCV and Pangolin)
if (OpenCV_FOUND AND Pangolin_FOUND)
    message("--- compiling dso_dataset.")
    include_directories(${Boost_INCLUDE_DIRS})
    LINK_DIRECTORIES(${Boost_LIBRARY_DIRS})
    add_executable(dso_dataset ${PROJECT_SOURCE_DIR}/src/main_dso_pangolin.cpp ${PROJECT_SOURCE_DIR}/src/FullSystem/dirent.c ${PROJECT_SOURCE_DIR}/src/FullSystem/PixelSelector2.cpp)
    target_link_libraries(dso_dataset dso ${Boost_LIBRARIES} ${SuiteSparse_LIBRARIES} ${LIBZIP_LIBRARY} ${Pangolin_LIBRARIES} ${OpenCV_LIBS})

    if (MSVC)
        set_property(TARGET dso_dataset APPEND PROPERTY LINK_FLAGS /NODEFAULTLIB:libcmt.lib)
        set_property(TARGET dso_dataset APPEND PROPERTY LINK_FLAGS /bigobj)
    endif ()

else ()
    message("--- not building dso_dataset, since either don't have openCV or Pangolin.")
endif ()

