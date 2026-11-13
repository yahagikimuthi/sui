cmake_minimum_required(VERSION 3.30)

project(sui CXX)

set(CMAKE_CXX_STANDARD 26)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF) # GNU拡張を無効化（標準準拠）
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

find_package(Vulkan REQUIRED)
find_package(glfw3 REQUIRED)
find_program(GLSLC_EXECUTABLE glslc)
find_package(glm REQUIRED)

add_library(sui INTERFACE)
add_library(sui::sui ALIAS sui)

target_include_directories(sui INTERFACE
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
)

include(FetchContent)

FetchContent_Declare(
    hgn
    GIT_REPOSITORY https://github.com/yahagikimuthi/hgn.git
    GIT_TAG main
)
FetchContent_MakeAvailable(hgn)

target_link_libraries(sui INTERFACE
    Vulkan::Vulkan
    glfw
    glm::glm
    hgn::hgn
)


if(PROJECT_IS_TOP_LEVEL)
    add_library(project_warnings INTERFACE)
    target_compile_options(project_warnings INTERFACE
        -Wall
        -Werror
        -Wextra
        -Wshadow
        -Wconversion
        -Wsign-conversion
        -Wpedantic
        -Wunreachable-code
        -Wimplicit-fallthrough
        -Wnoexcept
        -fcf-protection
        -mtune=native
        -march=native
        -flto
        -fcontracts
        $<$<CONFIG:Release>:-O2>
        $<$<CONFIG:Debug>:-O0>
        $<$<CONFIG:Debug>:-D_GLIBCXX_DEBUG>
        $<$<CONFIG:Debug>:-D_GLIBCXX_ASSERTIONS>
        $<$<CONFIG:Debug>:-D_FORTIFY_SOURCE=2>
        $<$<CONFIG:Debug>:-fstack-protector-strong>
        $<$<CONFIG:Debug>:-fstack-clash-protection>
        $<$<CONFIG:Debug>:-fsanitize=address,undefined,float-divide-by-zero,float-cast-overflow>
        $<$<CONFIG:Debug>:-fno-omit-frame-pointer>
    )
        target_link_options(project_warnings INTERFACE
        -fcontracts
        $<$<CONFIG:Debug>:-fsanitize=address,undefined,float-divide-by-zero,float-cast-overflow>
    )

    file(GLOB_RECURSE MAIN_SOURCES CONFIGURE_DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp")

    add_executable(main.exe)
    target_sources(main.exe PRIVATE ${MAIN_SOURCES})
    target_link_libraries(main.exe PRIVATE
        main
        project_warnings
    )
    target_include_directories(main.exe PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
endif()