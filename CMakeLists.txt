#pico-zephyr-project
#-------------------------------------------------------------------------------
# Zephyr Example Application
#
# Copyright (c) 2021 Nordic Semiconductor ASA
# SPDX-License-Identifier: Apache-2.0
#-------------------------------------------------------------------------------
# NOTE: Please do not remove the #pico-zephyr-project header, it is used by
# the Raspberry Pi Pico VS Code extension to identify the project type.
#-------------------------------------------------------------------------------

cmake_minimum_required(VERSION 3.20.0)

set(SNIPPET_ROOT ${CMAKE_SOURCE_DIR} CACHE STRING "Extra snippet root")

find_package(Zephyr REQUIRED HINTS $ENV{ZEPHYR_BASE})
project(zephyr-pico-w-starter LANGUAGES C)

add_subdirectory(components/main)
add_subdirectory(components/sensor_reader)
