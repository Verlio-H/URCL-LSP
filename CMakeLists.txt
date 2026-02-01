cmake_minimum_required(VERSION 3.14)

project(urcl-lsp LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(FETCHCONTENT_QUIET FALSE)
include(FetchContent)

FetchContent_Declare(
  lsp
  GIT_REPOSITORY https://github.com/leon-bckl/lsp-framework.git
  GIT_TAG        master
)


FetchContent_MakeAvailable(lsp)

file(GLOB_RECURSE URCL_LSP_SOURCES CONFIGURE_DEPENDS
  src/*.cpp
  src/urcl/*.cpp
)

add_executable(urcl-lsp ${URCL_LSP_SOURCES})
target_include_directories(urcl-lsp PUBLIC ${lsp_SOURCE_DIR})
target_include_directories(urcl-lsp PUBLIC ${lsp_BINARY_DIR}/generated)
target_compile_options(urcl-lsp PRIVATE
    -Wall
    -Wextra
    -pedantic
    -O3
)

target_link_libraries(urcl-lsp PUBLIC lsp)
get_target_property(APP_INCLUDE_DIRS urcl-lsp INCLUDE_DIRECTORIES)

install(TARGETS urcl-lsp DESTINATION bin)
