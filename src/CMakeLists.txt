add_compile_options(
	"-DDEBUG"
  "$<$<CONFIG:DEBUG>:-O0;-g3;-ggdb>"
)

include (FindPkgConfig)

pkg_check_modules (SSL "openssl")
pkg_check_modules (MYSQL "mysqlclient")

add_executable (dating
	main.c
	server.c
	logger.c
	common.c
	debug.c
	command.c
	db.c
	storage.c
	)

target_include_directories (dating PRIVATE
	${SSL_INCLUDE_DIRS}
	${MYSQL_INCLUDE_DIRS}
	)

target_link_libraries (dating PRIVATE
	${SSL_LIBRARIES}
	${MYSQL_LIBRARIES}
	)

set (EXECUTABLE_OUTPUT_PATH ..)
