cmake_minimum_required(VERSION 3.18)

project(chess VERSION 0.1.0 LANGUAGES C)

find_package(cglm CONFIG REQUIRED)
find_package(SDL2 CONFIG REQUIRED)
find_package(SDL2_image CONFIG REQUIRED)
find_package(SDL2_ttf CONFIG REQUIRED)
find_package(glad CONFIG REQUIRED)
find_package(SDL2_mixer CONFIG REQUIRED)

set(CMAKE_VERBOSE_MAKEFILE ON)

# Trova tutti i file .c e .h nelle directory dei sorgenti e degli include
file(GLOB_RECURSE SOURCES ${SOURCE_DIR}/*.c ${INCLUDE_DIR}/*.h)

# Aggiungi l'eseguibile
add_executable(chess ${SOURCES})

# Includi le directory degli header
target_include_directories(chess PRIVATE ${INCLUDE_DIR})

target_link_libraries(chess PRIVATE
    cglm::cglm
    SDL2::SDL2 
    SDL2_image::SDL2_image 
    SDL2_ttf::SDL2_ttf
    glad::glad
    SDL2_mixer::SDL2_mixer
)