cmake_minimum_required(VERSION 3.6)
project("serialportscanner")

add_library(serial_port SHARED serial_port.c)

find_library(log-lib log)

target_link_libraries(serial_port ${log-lib})
