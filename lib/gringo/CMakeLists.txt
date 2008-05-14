file(GLOB LIBGRINGO_SRC src/*.cpp)
SET(LIBGRINGO_SRC ${LIBGRINGO_SRC} ${GrinGo_BINARY_DIR}/lib/gringo/lparselexer.cpp ${GrinGo_BINARY_DIR}/lib/gringo/lparseparser_impl.cpp)
#SET_SOURCE_FILES_PROPERTIES(lparselexer.cpp GENERATED)
#SET_SOURCE_FILES_PROPERTIES(lparseparser_impl.cpp GENERATED)

include_directories(include ${GrinGo_BINARY_DIR}/lib/gringo ..)

ADD_EXECUTABLE(lemon src/lemon.c)

ADD_CUSTOM_COMMAND(
	OUTPUT ${GrinGo_BINARY_DIR}/lib/gringo/lparselexer.cpp
	COMMAND re2c -o ${GrinGo_BINARY_DIR}/lib/gringo/lparselexer.cpp ${GrinGo_SOURCE_DIR}/lib/gringo/src/lparselexer.re
	MAIN_DEPENDENCY ${GrinGo_SOURCE_DIR}/lib/gringo/src/lparselexer.re
	DEPENDS ${GrinGo_BINARY_DIR}/lib/gringo/lparseparser_impl.h
)

ADD_CUSTOM_COMMAND(
	OUTPUT ${GrinGo_BINARY_DIR}/lib/gringo/lparseparser_impl.cpp ${GrinGo_BINARY_DIR}/lib/gringo/lparseparser_impl.h
	COMMAND cp ${GrinGo_SOURCE_DIR}/lib/gringo/src/lempar.c ${GrinGo_BINARY_DIR}/lib/gringo/lempar.c
	COMMAND lemon -q ${GrinGo_SOURCE_DIR}/lib/gringo/src/lparseparser_impl.y
	COMMAND rm ${GrinGo_BINARY_DIR}/lib/gringo/lempar.c
	COMMAND mv ${GrinGo_SOURCE_DIR}/lib/gringo/src/lparseparser_impl.c ${GrinGo_BINARY_DIR}/lib/gringo/lparseparser_impl.cpp
	COMMAND mv ${GrinGo_SOURCE_DIR}/lib/gringo/src/lparseparser_impl.h ${GrinGo_BINARY_DIR}/lib/gringo/lparseparser_impl.h
	COMMAND touch ${GrinGo_BINARY_DIR}/lib/gringo/lparseparser_impl.h ${GrinGo_BINARY_DIR}/lib/gringo/lparseparser_impl.cpp
	MAIN_DEPENDENCY ${GrinGo_SOURCE_DIR}/lib/gringo/src/lparseparser_impl.y
	DEPENDS lemon ${GrinGo_SOURCE_DIR}/lib/gringo/src/lempar.c
)

add_library(gringo-lib STATIC ${LIBGRINGO_SRC})
target_link_libraries (gringo-lib clasp-lib)
set_target_properties(gringo-lib PROPERTIES OUTPUT_NAME gringo)
