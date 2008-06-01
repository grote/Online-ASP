file(GLOB LIBGRINGO_SRC src/*.cpp)
SET(LIBGRINGO_SRC 
	${LIBGRINGO_SRC} 
	${GrinGo_BINARY_DIR}/lib/gringo/lparselexer.cpp 
	${GrinGo_BINARY_DIR}/lib/gringo/lparseparser_impl.cpp
	${GrinGo_BINARY_DIR}/lib/gringo/plainlparselexer.cpp 
	${GrinGo_BINARY_DIR}/lib/gringo/lparseconverter_impl.cpp)
#SET_SOURCE_FILES_PROPERTIES(lparselexer.cpp GENERATED)
#SET_SOURCE_FILES_PROPERTIES(lparseparser_impl.cpp GENERATED)

include_directories(include ${GrinGo_BINARY_DIR}/lib/gringo ..)

ADD_EXECUTABLE(lemon src/lemon.c)

MACRO(RE2C SRC DST)
	ADD_CUSTOM_COMMAND(
		OUTPUT ${DST}
		COMMAND re2c -o ${DST} ${SRC}
		MAIN_DEPENDENCY ${SRC}
	)
ENDMACRO(RE2C SRC DEST)

MACRO(LEMON NAME SRC DST)
	ADD_CUSTOM_COMMAND(
		OUTPUT ${DST}/${NAME}.cpp ${DST}/${NAME}.h
		COMMAND cp ${SRC}/lempar.c ${DST}/lempar.c
		COMMAND lemon -q ${SRC}/${NAME}.y
		COMMAND rm ${DST}/lempar.c
		COMMAND mv ${SRC}/${NAME}.c ${DST}/${NAME}.cpp
		COMMAND mv ${SRC}/${NAME}.h ${DST}/${NAME}.h
		COMMAND touch ${DST}/${NAME}.h ${DST}/${NAME}.cpp
		MAIN_DEPENDENCY ${SRC}/${NAME}.y
		DEPENDS lemon ${SRC}/lempar.c
	)
ENDMACRO(LEMON SRC DST LEMPAR)

RE2C(${GrinGo_SOURCE_DIR}/lib/gringo/src/lparselexer.re ${GrinGo_BINARY_DIR}/lib/gringo/lparselexer.cpp)
LEMON(lparseparser_impl ${GrinGo_SOURCE_DIR}/lib/gringo/src ${GrinGo_BINARY_DIR}/lib/gringo)

RE2C(${GrinGo_SOURCE_DIR}/lib/gringo/src/plainlparselexer.re ${GrinGo_BINARY_DIR}/lib/gringo/plainlparselexer.cpp)
LEMON(lparseconverter_impl ${GrinGo_SOURCE_DIR}/lib/gringo/src ${GrinGo_BINARY_DIR}/lib/gringo)

#ADD_CUSTOM_COMMAND(
#	OUTPUT ${GrinGo_BINARY_DIR}/lib/gringo/lparselexer.cpp
#	COMMAND re2c -o ${GrinGo_BINARY_DIR}/lib/gringo/lparselexer.cpp ${GrinGo_SOURCE_DIR}/lib/gringo/src/lparselexer.re
#	MAIN_DEPENDENCY ${GrinGo_SOURCE_DIR}/lib/gringo/src/lparselexer.re
#	DEPENDS ${GrinGo_BINARY_DIR}/lib/gringo/lparseparser_impl.h
#)

#ADD_CUSTOM_COMMAND(
#	OUTPUT ${GrinGo_BINARY_DIR}/lib/gringo/lparseparser_impl.cpp ${GrinGo_BINARY_DIR}/lib/gringo/lparseparser_impl.h
#	COMMAND cp ${GrinGo_SOURCE_DIR}/lib/gringo/src/lempar.c ${GrinGo_BINARY_DIR}/lib/gringo/lempar.c
#	COMMAND lemon -q ${GrinGo_SOURCE_DIR}/lib/gringo/src/lparseparser_impl.y
#	COMMAND rm ${GrinGo_BINARY_DIR}/lib/gringo/lempar.c
#	COMMAND mv ${GrinGo_SOURCE_DIR}/lib/gringo/src/lparseparser_impl.c ${GrinGo_BINARY_DIR}/lib/gringo/lparseparser_impl.cpp
#	COMMAND mv ${GrinGo_SOURCE_DIR}/lib/gringo/src/lparseparser_impl.h ${GrinGo_BINARY_DIR}/lib/gringo/lparseparser_impl.h
#	COMMAND touch ${GrinGo_BINARY_DIR}/lib/gringo/lparseparser_impl.h ${GrinGo_BINARY_DIR}/lib/gringo/lparseparser_impl.cpp
#	MAIN_DEPENDENCY ${GrinGo_SOURCE_DIR}/lib/gringo/src/lparseparser_impl.y
#	DEPENDS lemon ${GrinGo_SOURCE_DIR}/lib/gringo/src/lempar.c
#)

add_library(gringo-lib STATIC ${LIBGRINGO_SRC})
target_link_libraries (gringo-lib clasp-lib)
set_target_properties(gringo-lib PROPERTIES OUTPUT_NAME gringo)
