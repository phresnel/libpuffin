cmake_minimum_required(VERSION 3.5)
set(CMAKE_CXX_STANDARD 14)
project(OneImageLib)


# == Compiler settings. ================================================================================================
# Compiler detection as per: https://stackoverflow.com/a/10055571/76722
macro(print_cxx_flags msg)
        macro(print_var_with_name v)
                message(STATUS "  " ${v} ": " ${${v}})
        endmacro()
        message(STATUS "C++ flags${msg}:")
        print_var_with_name(CMAKE_CXX_FLAGS)
        print_var_with_name(CMAKE_CXX_FLAGS_DEBUG)
        print_var_with_name(CMAKE_CXX_FLAGS_MINSIZEREL)
        print_var_with_name(CMAKE_CXX_FLAGS_RELEASE)
        print_var_with_name(CMAKE_CXX_FLAGS_RELWITHDEBINFO)
endmacro()
macro(replace_or_add output_var match_expr repl_expr)
        if (${output_var} MATCHES ${match_expr})
                string(REGEX REPLACE ${match_expr}
                       ${repl_expr}
                       ${output_var}
                       ${${output_var}})
        else()
                set(${output_var} "${${output_var}} ${repl_expr}")
        endif()
endmacro()

print_cxx_flags(" before any modifications")
if ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "MSVC")
        message(STATUS "Compiler is MSVC.")

        replace_or_add(CMAKE_CXX_FLAGS "/W[0-9]+" "/W4")
        replace_or_add(CMAKE_CXX_FLAGS_RELEASE "/O[x12]" "/O2")
        replace_or_add(CMAKE_CXX_FLAGS_RELWITHDEBINFO "/O[x12]" "/O2")
        # replace_or_add(CMAKE_CXX_FLAGS_RELEASE "/arch:[0-9a-zA-Z-]+" /arch:SSE2)
        # replace_or_add(CMAKE_CXX_FLAGS_RELWITHDEBINFO "/arch:[0-9a-zA-Z-]+" /arch:SSE2)
        add_compile_definitions(_CRT_SECURE_NO_WARNINGS) # because it complained about localtime() (::localtime_s() is not guaranteed to exist in C++)
elseif("${CMAKE_CXX_COMPILER_ID}" STREQUAL "GNU")
        message(STATUS "Compiler is GNU.")
        # see also: https://gcc.gnu.org/onlinedocs/gcc/Optimize-Options.html
        # and: https://github.com/phresnel/tinscape/blob/master/build/Makefile
        add_compile_options(-O3 -Wall -march=native -mfpmath=sse)
else()
        message(WARNING "Compiler unknown: " ${CMAKE_CXX_COMPILER_ID})
        message(WARNING "Compiling in generic mode, which may be suboptimal.")
endif()
print_cxx_flags(" after modifications")



# == Linking. ==================================================================
include_directories(vendor/SDL2-2.0.8)
include_directories(include/)
add_executable(
        puffin

        src/main.cc

        # image/ ===============================================================
        src/image/coords.hh
        src/image/Image.hh
        src/image/ImageFilter.hh
        #src/image/io.cc
        #src/image/io.detail.cc
        #src/image/io.hh
        src/image/Rgb.hh

        # sdlxx/ ===============================================================
        src/sdlxx/Sdl.hh
        src/sdlxx/Renderer.hh

        # include/puffin =======================================================
        include/puffin/exceptions.hh

        # bmp ==================================================================
        include/puffin/bmp/read_bmp.hh
        src/bmp/read_bmp.cc
)



# == Libraries. ================================================================

## -- SDL 2 --------------------------------------------------------------------
if(MSVC)
        set(CMAKE_MODULE_PATH ${PROJECT_SOURCE_DIR}/cmake-modules)
        set(SDL2_STATIC ON)
        find_package(SDL2 REQUIRED)
        include_directories(${SDL2_INCLUDE_DIRS})
        configure_file(${SDL2_DLL} ${CMAKE_BINARY_DIR}/ COPYONLY)
else()
        find_package(SDL2 REQUIRED)
        include_directories(${SDL2_INCLUDE_DIRS})
        # Because it otherwise has trailing whitespace, which CMake forbids.
        string(STRIP ${SDL2_LIBRARIES} SDL2_LIBRARIES)
endif()
target_link_libraries(puffin ${SDL2_LIBRARIES})
target_compile_definitions(puffin PUBLIC SDL_MAIN_HANDLED)

# == Assets. ===================================================================
## -- dev-assets/ --------------------------------------------------------------
set(ASSET_FOLDER dev-assets)
file(MAKE_DIRECTORY ${CMAKE_BINARY_DIR}/${ASSET_FOLDER}/)
configure_file("${ASSET_FOLDER}/puffin.jpg" ${CMAKE_BINARY_DIR}/${ASSET_FOLDER}/ COPYONLY)

## -- dev-assets/bmp/ -------------------------------------------------------
set(ASSET_FOLDER dev-assets/bmp)
file(MAKE_DIRECTORY ${CMAKE_BINARY_DIR}/${ASSET_FOLDER}/)
configure_file("${ASSET_FOLDER}/puffin_1bit.bmp" ${CMAKE_BINARY_DIR}/${ASSET_FOLDER}/ COPYONLY)
configure_file("${ASSET_FOLDER}/puffin_4bit.bmp" ${CMAKE_BINARY_DIR}/${ASSET_FOLDER}/ COPYONLY)
configure_file("${ASSET_FOLDER}/puffin_8bit.bmp" ${CMAKE_BINARY_DIR}/${ASSET_FOLDER}/ COPYONLY)
configure_file("${ASSET_FOLDER}/puffin_24bit.bmp" ${CMAKE_BINARY_DIR}/${ASSET_FOLDER}/ COPYONLY)
configure_file("${ASSET_FOLDER}/red256_green128_blue64_32x16x24bit.bmp" ${CMAKE_BINARY_DIR}/${ASSET_FOLDER}/ COPYONLY)
configure_file("${ASSET_FOLDER}/red256_green128_blue64_32x16x8bit.bmp" ${CMAKE_BINARY_DIR}/${ASSET_FOLDER}/ COPYONLY)
configure_file("${ASSET_FOLDER}/red256_green128_blue64_4x2x24bit.bmp" ${CMAKE_BINARY_DIR}/${ASSET_FOLDER}/ COPYONLY)
configure_file("${ASSET_FOLDER}/red256_green128_blue64_4x2x8bit.bmp" ${CMAKE_BINARY_DIR}/${ASSET_FOLDER}/ COPYONLY)
configure_file("${ASSET_FOLDER}/red256_green128_blue64_4x2x4bit.bmp" ${CMAKE_BINARY_DIR}/${ASSET_FOLDER}/ COPYONLY)
configure_file("${ASSET_FOLDER}/red256_green128_blue64_4x2x1bit.bmp" ${CMAKE_BINARY_DIR}/${ASSET_FOLDER}/ COPYONLY)
