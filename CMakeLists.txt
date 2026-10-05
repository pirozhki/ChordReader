cmake_minimum_required(VERSION 3.20)
project(ChordReader LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

add_library(chordreader STATIC
    chord.cpp
)

target_include_directories(chordreader PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
)

add_executable(ChordReader example.cpp)
target_link_libraries(ChordReader PRIVATE chordreader)

option(CHORDREADER_BUILD_TESTS "Build ChordReader tests" ON)
if(CHORDREADER_BUILD_TESTS)
    enable_testing()
    add_executable(ChordReaderTests tests/test_chord.cpp)
    target_link_libraries(ChordReaderTests PRIVATE chordreader)
    add_test(NAME ChordReaderTests COMMAND ChordReaderTests)
endif()
