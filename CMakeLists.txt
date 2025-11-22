cmake_minimum_required(VERSION 3.28)
project(CMakeSFMLProject LANGUAGES C CXX)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
option(BUILD_SHARED_LIBS "Build shared libraries" OFF)

include(FetchContent)
FetchContent_Declare(SFML
    GIT_REPOSITORY https://github.com/SFML/SFML.git
    GIT_TAG 2.6.x
    GIT_SHALLOW ON
    EXCLUDE_FROM_ALL
    SYSTEM)
FetchContent_MakeAvailable(SFML)

add_library(libcli STATIC src/libcli.c)
target_include_directories(libcli PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)
set_target_properties(libcli PROPERTIES C_STANDARD 11 C_STANDARD_REQUIRED ON)

add_library(libcliwrap STATIC src/libcliwrap.cpp)
target_include_directories(libcliwrap PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_link_libraries(libcliwrap PUBLIC libcli)
target_compile_features(libcliwrap PUBLIC cxx_std_17)

file(GLOB UI_COMMON_SRC
    src/ui/common/*.cpp
)

file(GLOB UI_PAGES_SRC
    src/ui/pages/*.cpp
)

file(GLOB UI_WIDGETS_SRC
    src/ui/widgets/*.cpp
)

add_executable(main
    src/main.cpp
    ${UI_COMMON_SRC}
    ${UI_PAGES_SRC}
    ${UI_WIDGETS_SRC}
)

target_include_directories(main PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_link_libraries(main PRIVATE sfml-graphics libcliwrap)
target_compile_features(main PRIVATE cxx_std_17)

add_custom_command(TARGET main POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:main> ${CMAKE_SOURCE_DIR}/$<TARGET_FILE_NAME:main>
    COMMENT "Copying built executable to source root"
)

if(WIN32)
    add_custom_command(
        TARGET main
        COMMENT "Copy OpenAL DLL"
        PRE_BUILD COMMAND ${CMAKE_COMMAND} -E copy 
        ${SFML_SOURCE_DIR}/extlibs/bin/$<IF:$<EQUAL:${CMAKE_SIZEOF_VOID_P},8>,x64,x86>/openal32.dll 
        $<TARGET_FILE_DIR:main>
        VERBATIM)
endif()