# Minimum CMake version required
cmake_minimum_required(VERSION 3.10)

# Project name and version
project(ImageDewarping VERSION 1.0)

# Find OpenCV package
find_package(OpenCV REQUIRED)

# Add the executable target (replace 'dewarp_image.cpp' with your source file name if different)
add_executable(dewarp_image dewarp_image.cpp)

# Link OpenCV libraries to the executable target
target_link_libraries(dewarp_image ${OpenCV_LIBS})

# Set the C++ standard (C++17 in this case)
set_target_properties(dewarp_image PROPERTIES
    CXX_STANDARD 17
    CXX_STANDARD_REQUIRED YES
    CXX_EXTENSIONS NO
)

# Specify the installation directory for the executable (optional)
install(TARGETS dewarp_image DESTINATION bin)

# Print status messages (optional)
message(STATUS "OpenCV version: ${OpenCV_VERSION}")
message(STATUS "Executable will be created at: ${CMAKE_BINARY_DIR}/dewarp_image")
