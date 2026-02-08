cmake_minimum_required(VERSION 3.16)
project(SysPulse VERSION 2.0)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)

# Find Qt5
find_package(Qt5 COMPONENTS Widgets Charts REQUIRED)

# Find Threads for terminal mode
find_package(Threads REQUIRED)

# Source files
set(SOURCES
    main.cpp
    mainwindow.cpp
    mainwindow.h
    terminalmonitor.cpp
    terminalmonitor.h
)

# Create executable
add_executable(${PROJECT_NAME} ${SOURCES})

# Link libraries
target_link_libraries(${PROJECT_NAME} 
    Qt5::Widgets 
    Qt5::Charts
    Threads::Threads
)

# Install target
install(TARGETS ${PROJECT_NAME} DESTINATION bin)

# Set output directory
set_target_properties(${PROJECT_NAME} PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
)