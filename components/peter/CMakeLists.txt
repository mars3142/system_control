if (DEFINED ENV{IDF_PATH})
idf_component_register(SRCS
        PushButton.cpp
        INCLUDE_DIRS "include"
)
return()
endif()

cmake_minimum_required(VERSION 3.30)
project(peter)

add_library(${PROJECT_NAME} STATIC
        PushButton.cpp
)

include_directories(include)

target_include_directories(${PROJECT_NAME} PUBLIC include)
