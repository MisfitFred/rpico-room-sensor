# Fügt main.c zum app-Target hinzu und linkt die Komponenten.
target_sources(app PRIVATE
	src/main.c
	src/app_start.c
)

target_include_directories(app PRIVATE
	${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_link_libraries(app PRIVATE sensor_reader)
