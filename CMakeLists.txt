cmake_minimum_required(VERSION 3.16)
project(DevU-GameEngine LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

find_package(SDL3 REQUIRED CONFIG)

# --------------------------------------------------------------------------
# Por enquanto a engine ainda não existe como biblioteca separada: tudo está
# centralizado dentro do próprio jogo (engine/src e engine/include ainda
# não têm .cpp). Quando isso mudar, basta descomentar o bloco abaixo e
# linkar "devu-engine" no lugar de só incluir o diretório de headers.
#
add_library(devu-engine STATIC
    engine/src/core/Window.cpp
    engine/src/core/Renderer.cpp
    engine/src/core/Application.cpp
)
target_include_directories(devu-engine PUBLIC engine/include)
target_link_libraries(devu-engine PUBLIC SDL3::SDL3)
# --------------------------------------------------------------------------

add_executable(the-hunter
    games/the-hunter/src/main.cpp
)


target_link_libraries(the-hunter PRIVATE
    devu-engine
)

set_target_properties(the-hunter PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin
)
