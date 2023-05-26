cmake_minimum_required(VERSION 3.15)

project(python_haptics)

set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY_DEBUG ${CMAKE_BINARY_DIR}/../bin/debug)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY_DEBUG ${CMAKE_BINARY_DIR}/../bin/debug)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_DEBUG ${CMAKE_BINARY_DIR}/../bin/debug)

set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY_RELEASE ${CMAKE_BINARY_DIR}/../bin/release)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY_RELEASE ${CMAKE_BINARY_DIR}/../bin/release)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_RELEASE ${CMAKE_BINARY_DIR}/../bin/release)

set(SOURCE_FILES
		python_haptics.cpp
)
	
include_directories(
	../lib/boost/include/boost-1_71
	../lib/LeapSDK/4.0.0/LeapSDK/include
	../lib/ultrahaptics/Headers
	../lib/freeglut/include
	C:/Users/sam/Anaconda3/include
	../lib/installed/x64-windows/include
)

link_directories(
	../lib/boost/lib
	../lib/ultrahaptics/Libraries/CPlusPlus
	../lib/LeapSDK/4.0.0/LeapSDK/lib/x64
	../lib/freeglut/lib/x64
	C:/Users/sam/Anaconda3/libs
	../lib/installed/x64-windows/lib
)

add_library(python_haptics SHARED python_haptics.cpp ExampleConnection.cpp)
set_target_properties(python_haptics PROPERTIES SUFFIX ".pyd")
target_link_libraries(python_haptics LeapC Ultrahaptics)


add_executable(main main.cpp)
#target_link_libraries(leap Leap)

add_executable(leap leapmotion_sample.cpp)
target_link_libraries(leap Leap)

add_executable(ultrahaptics AmplitudeModulation_Focus.cpp)
target_link_libraries(ultrahaptics Ultrahaptics)

add_executable(uh2 TimePointStreaming_Forcefield.cpp)
target_link_libraries(uh2 Leap Ultrahaptics)

add_executable(leapc ExampleConnection.cpp PollingSample.cpp tracker.cpp)
target_link_libraries(leapc LeapC)

add_executable(track track.cpp user_data.cpp tracking_logger.cpp tracking_emitter.cpp tracking_window.cpp ExampleConnection.cpp utils.cpp)
target_link_libraries(track freeglut LeapC Ultrahaptics avro jansson lzma snappy zlib glew32)

# from: https://stackoverflow.com/questions/10671916/how-to-copy-dll-files-into-the-same-folder-as-the-executable-using-cmake

add_custom_command(TARGET python_haptics POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
        "${PROJECT_SOURCE_DIR}/../lib/boost/lib/"
        $<TARGET_FILE_DIR:python_haptics>)

add_custom_command(TARGET python_haptics POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
        "${PROJECT_SOURCE_DIR}/../lib/LeapSDK/4.0.0/LeapSDK/lib/x64"
        $<TARGET_FILE_DIR:python_haptics>)

add_custom_command(TARGET python_haptics POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
        "${PROJECT_SOURCE_DIR}/../lib/ultrahaptics/Libraries/CPlusPlus"
        $<TARGET_FILE_DIR:python_haptics>)

add_custom_command(TARGET python_haptics POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
        "${PROJECT_SOURCE_DIR}/../lib/freeglut/bin/x64"
        $<TARGET_FILE_DIR:python_haptics>)

# -----------------------------------------------------------------------------
# Boost setup
# -----------------------------------------------------------------------------
set(BOOST_ROOT $ENV{HAPTICS_ROOT}/boost)
set(Boost_USE_STATIC_LIBS ON)
set(Boost_USE_MULTITHREADED ON)
set(Boost_USE_STATIC_RUNTIME OFF)
find_package(Boost 1.71.0 REQUIRED python37)
if(Boost_FOUND)
    include_directories(SYSTEM ${Boost_INCLUDE_DIRS})
    #target_link_libraries(pext ${Boost_LIBRARIES})
endif()

#target_link_libraries(pext 
#    ${Boost_LIBRARIES}
	#python37.lib
#)
