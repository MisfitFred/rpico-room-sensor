# Baut sensor_reader als statische Library.
add_library(sensor_reader STATIC src/sensor_reader.c)
target_include_directories(sensor_reader PUBLIC include)
target_link_libraries(sensor_reader PRIVATE zephyr_interface)
add_dependencies(sensor_reader offsets_h)
