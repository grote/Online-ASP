ADD_EXECUTABLE(lemon lemon.c)
if (MSVC)
	SET_TARGET_PROPERTIES(lemon PROPERTIES PREFIX "../")
endif()

