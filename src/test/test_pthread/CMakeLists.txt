cmake_minimum_required (VERSION 2.8)

project (chat)

	list (APPEND SRC 
			chat.c
			task.c
	     )

	list (APPEND LIBS
			pthread
	     )


add_executable (${PROJECT_NAME} ${SRC})
target_link_libraries (${PROJECT_NAME} ${LIBS})

set (EXECUTABLE_OUTPUT_PATH ${PROJECT_BINARY_DIR}/bin)
set (LIBRARY_OUTPUT_PATH ${PROJECT_BINARY_DIR}/lib)


	install (TARGETS ${PROJECT_NAME} 
			RUNTIME DESTINATION bin)
