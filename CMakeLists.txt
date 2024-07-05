# CMakeList.txt: проект CMake для CMakequality_scan; включите исходный код и определения,
# укажите здесь логику для конкретного проекта.
#


# Добавьте источник в исполняемый файл этого проекта.
add_executable (CMakequality_scan "CMakequality_scan.cpp" "CMakequality_scan.h")

if (CMAKE_VERSION VERSION_GREATER 3.12)
  set_property(TARGET CMakequality_scan PROPERTY CXX_STANDARD 20)
endif()

# TODO: Добавьте тесты и целевые объекты, если это необходимо.
