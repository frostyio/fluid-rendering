cmake_minimum_required(VERSION 3.15)
set(PROJECT_NAME final)
project(${PROJECT_NAME} LANGUAGES CXX VERSION 0.1.0)

set(SRC_DIR ${CMAKE_SOURCE_DIR}/src)
set(INCLUDE_DIR ${CMAKE_SOURCE_DIR}/include)
set(LIB_DIR ${CMAKE_SOURCE_DIR}/libs)

set(GLFW_BUILD_DOCS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON) # for editor

include_directories(${INCLUDE_DIR})

file(GLOB_RECURSE SOURCES "${SRC_DIR}/*.cpp")
add_executable(${PROJECT_NAME}
	${SOURCES}
)

set_target_properties(${PROJECT_NAME} PROPERTIES
				CXX_STANDARD 17
				CXX_STANDARD_REQUIRED ON
				CXX_EXTENSIONS OFF
)

target_compile_definitions(${PROJECT_NAME} PRIVATE PROJECT_NAME="${PROJECT_NAME}")

# glfw
target_link_libraries(${PROJECT_NAME} PUBLIC glfw)
target_include_directories(${PROJECT_NAME} PUBLIC libs/glfw/include)
add_subdirectory(libs/glfw)

# opengl
find_package(OpenGL REQUIRED)
target_link_libraries(${PROJECT_NAME} PUBLIC opengl32) # assumes opengl32 can be found on your system

# glew
set(GLEW_DIR "${LIB_DIR}/glew-2.1.0")
target_link_libraries(${PROJECT_NAME} PUBLIC ${GLEW_DIR}/lib/Release/x64/glew32.lib)
target_include_directories(${PROJECT_NAME} PUBLIC ${GLEW_DIR}/include)
add_custom_command(
				TARGET ${PROJECT_NAME} POST_BUILD
				COMMAND ${CMAKE_COMMAND} -E copy
												${GLEW_DIR}/bin/Release/x64/glew32.dll
												$<TARGET_FILE_DIR:${PROJECT_NAME}>
)

# cyCodeBase
set(CYCODEBASE_DIR "${LIB_DIR}/cyCodeBase" CACHE PATH "Path to cyCodeBase")
target_include_directories(${PROJECT_NAME} PUBLIC ${CYCODEBASE_DIR})


# lodepng
set(LODEPNG_DIR "${LIB_DIR}/lodepng" CACHE PATH "Path to lodepng")
target_include_directories(${PROJECT_NAME} PRIVATE ${LODEPNG_DIR})
target_sources(${PROJECT_NAME} PRIVATE ${LODEPNG_DIR}/lodepng.cpp)

# alembic
find_package(Alembic REQUIRED)
find_package(Imath REQUIRED)

target_include_directories(${PROJECT_NAME} PRIVATE ${ALEMBIC_INCLUDE_DIRS})
target_link_libraries(${PROJECT_NAME} PRIVATE Alembic::Alembic Imath::Imath)

# copy assets
file(COPY ${CMAKE_SOURCE_DIR}/assets DESTINATION ${CMAKE_BINARY_DIR})
