cmake_minimum_required(VERSION 3.20)
project(SpellChecker LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

#static library spell_checker containing the implementation of the spell checker in src/spell_checker.cpp.
add_library(spell_checker STATIC
    src/spell_checker.cpp
)

#include directory include to spell_checker target so that it can find the header files and use them by other targets that link to spell_checker
target_include_directories(spell_checker PUBLIC include) 

target_compile_options(spell_checker PRIVATE
    $<$<CXX_COMPILER_ID:MSVC>:/W4 /permissive->
    $<$<NOT:$<CXX_COMPILER_ID:MSVC>>:-Wall -Wextra -Wpedantic>
)

# executable for the spell_checker library will be named spell_checker_app and will be built from src/main.cpp. 

add_executable(spell_checker_app src/main.cpp)
#It will link to the spell_checker library so that it can use its functionality.
target_link_libraries(spell_checker_app PRIVATE spell_checker)

enable_testing()
add_executable(spell_checker_tests tests/test_spell_checker.cpp)
target_link_libraries(spell_checker_tests PRIVATE spell_checker)
add_test(NAME spell_checker_tests COMMAND spell_checker_tests)
