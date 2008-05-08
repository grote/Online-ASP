file(GLOB GRINGO_SRC *.cpp)

include_directories (${GrinGo_SOURCE_DIR}/lib/include)
link_directories (${GrinGo_BINARY_DIR}/lib/src)

add_executable (gringo-app ${GRINGO_SRC})
target_link_libraries (gringo-app gringo-lib)
set_target_properties(gringo-app PROPERTIES OUTPUT_NAME gringo)

