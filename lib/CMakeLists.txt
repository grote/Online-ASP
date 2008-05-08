file(GLOB LIBGRINGO_SRC src/*.cpp)
SET(LIBGRINGO_SRC ${LIBGRINGO_SRC} ${GrinGo_BINARY_DIR}/lib/lparselexer.cpp ${GrinGo_BINARY_DIR}/lib/lparseparser_impl.cpp)
#SET_SOURCE_FILES_PROPERTIES(lparselexer.cpp GENERATED)
#SET_SOURCE_FILES_PROPERTIES(lparseparser_impl.cpp GENERATED)

include_directories(include ${GrinGo_BINARY_DIR}/lib)

ADD_CUSTOM_TARGET(LparseLexer echo "Creating lparselexer.cpp")
ADD_CUSTOM_TARGET(LparseParser echo "Creating lparseparser.cpp")

ADD_EXECUTABLE(lemon src/lemon.c)

ADD_CUSTOM_COMMAND(
	SOURCE ${GrinGo_SOURCE_DIR}/lib/src/lparselexer.re
	COMMAND re2c -o ${GrinGo_BINARY_DIR}/lib/lparselexer.cpp ${GrinGo_SOURCE_DIR}/lib/src/lparselexer.re
	TARGET LparseLexer
	DEPENDS ${GrinGo_BINARY_DIR}/lib/lparseparser_impl.h
	OUTPUTS ${GrinGo_BINARY_DIR}/lib/lparselexer.cpp
)

ADD_CUSTOM_COMMAND(
	SOURCE ${GrinGo_SOURCE_DIR}/lib/src/lparselexer.re
	COMMAND cp ${GrinGo_SOURCE_DIR}/lib/src/lempar.c ${GrinGo_BINARY_DIR}/lib/lempar.c
	COMMAND lemon -q ${GrinGo_SOURCE_DIR}/lib/src/lparseparser_impl.y
	COMMAND rm ${GrinGo_BINARY_DIR}/lib/lempar.c
	COMMAND mv ${GrinGo_SOURCE_DIR}/lib/src/lparseparser_impl.c ${GrinGo_BINARY_DIR}/lib/lparseparser_impl.cpp
	COMMAND mv ${GrinGo_SOURCE_DIR}/lib/src/lparseparser_impl.h ${GrinGo_BINARY_DIR}/lib/lparseparser_impl.h
	COMMAND touch ${GrinGo_BINARY_DIR}/lib/lparseparser_impl.h ${GrinGo_BINARY_DIR}/lib/lparseparser_impl.cpp
	TARGET LparseParser
	DEPENDS lemon ${GrinGo_SOURCE_DIR}/lib/src/lempar.c
	MAIN_DEPENDENCY ${GrinGo_SOURCE_DIR}/lib/src/lparseparser_impl.y
	OUTPUTS ${GrinGo_BINARY_DIR}/lib/lparseparser_impl.cpp ${GrinGo_BINARY_DIR}/lib/lparseparser_impl.h
)

add_library(gringo-lib STATIC ${LIBGRINGO_SRC})
set_target_properties(gringo-lib PROPERTIES OUTPUT_NAME gringo)

