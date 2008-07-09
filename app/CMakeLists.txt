if(WITH_CLASP)
	file(GLOB GRINGO_SRC *.cpp program_opts/*.cpp)
	include_directories (${GrinGo_SOURCE_DIR}/lib/gringo/include ../lib ${GrinGo_BINARY_DIR}/lib/gringo/config)
else(WITH_CLASP)
	file(GLOB GRINGO_SRC main.cpp)
	include_directories (${GrinGo_SOURCE_DIR}/lib/gringo/include ${GrinGo_BINARY_DIR}/lib/gringo/config)
endif(WITH_CLASP)

add_executable (gringo-app ${GRINGO_SRC})

if(WITH_CLASP)
	target_link_libraries (gringo-app clasp-lib gringo-lib)
else(WITH_CLASP)
	target_link_libraries (gringo-app gringo-lib)
endif(WITH_CLASP)

set_target_properties(gringo-app PROPERTIES OUTPUT_NAME gringo)
