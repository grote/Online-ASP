
MACRO(RE2C NAME SRC DST VAR)
	IF(NOT EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/generated/${NAME}.cpp")
		# this macro requires re2c
		IF(NOT RE2C_FOUND)
			FIND_PACKAGE(RE2C REQUIRED)
		ENDIF(NOT RE2C_FOUND)
		ADD_CUSTOM_COMMAND(
			OUTPUT ${DST}/${NAME}.cpp
			COMMAND ${RE2C_EXECUTABLE} -o ${DST}/${NAME}.cpp ${SRC}/${NAME}.re
			MAIN_DEPENDENCY ${SRC}/${NAME}.re
		)
		#SET_SOURCE_FILES_PROPERTIES("${DST}/${NAME}.cpp" GENERATED)
		SET(${VAR} ${${VAR}} "${DST}/${NAME}.cpp")
	ELSE(NOT EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/generated/${NAME}.cpp")
		SET(${VAR} ${${VAR}} "${CMAKE_CURRENT_SOURCE_DIR}/generated/${NAME}.cpp")
	ENDIF(NOT EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/generated/${NAME}.cpp")
ENDMACRO(RE2C SRC DEST VAR)

MACRO(LEMON NAME SRC DST VAR)
	IF(NOT EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/generated/${NAME}.cpp" OR NOT EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/generated/${NAME}.h")
		ADD_CUSTOM_COMMAND(
		        OUTPUT ${DST}/lempar.c
			COMMAND cmake -E copy ${CMAKE_SOURCE_DIR}/lib/gringo/src/lempar.c ${DST}/lempar.c
			MAIN_DEPENDENCY ${CMAKE_SOURCE_DIR}/lib/gringo/src/lempar.c
		)
		ADD_CUSTOM_COMMAND(
			OUTPUT ${DST}/${NAME}.cpp ${DST}/${NAME}.h
			COMMAND ${EXECUTABLE_OUTPUT_PATH}/lemon -q ${SRC}/${NAME}.y
			COMMAND cmake -E copy ${SRC}/${NAME}.c ${DST}/${NAME}.cpp
			COMMAND cmake -E copy ${SRC}/${NAME}.h ${DST}/${NAME}.h
			COMMAND cmake -E remove ${SRC}/${NAME}.c
			COMMAND cmake -E remove ${SRC}/${NAME}.h
			COMMAND cmake -E touch ${DST}/${NAME}.h ${DST}/${NAME}.cpp
			MAIN_DEPENDENCY ${SRC}/${NAME}.y
			DEPENDS lemon ${DST}/lempar.c
		)
		# add the .cpp file to the sources
		#SET_SOURCE_FILES_PROPERTIES("${DST}/${NAME}.cpp" GENERATED)

		SET(${VAR} ${${VAR}} "${DST}/${NAME}.cpp")
	ELSE(NOT EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/generated/${NAME}.cpp" OR NOT EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/generated/${NAME}.h")
		SET(${VAR} ${${VAR}} "${CMAKE_CURRENT_SOURCE_DIR}/generated/${NAME}.cpp")
	ENDIF(NOT EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/generated/${NAME}.cpp" OR NOT EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/generated/${NAME}.h")
ENDMACRO(LEMON NAME SRC DST VAR)

