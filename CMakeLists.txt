cmake_minimum_required(VERSION 3.22)
project(ggsample01 LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if(MSVC)
    add_compile_options("/utf-8")
endif()

set(SOURCES
    main.cpp
    ggsample01.cpp
    GgApp.cpp
    Menu.cpp
    Config.cpp
    gg.cpp
)

set(HEADERS
    GgApp.h
    Menu.h
    Config.h
    parseconfig.h
    resource.h
    gg.h
)

set(SHADER_FILES
    simple.vert
    simple.frag
    simple_es3.vert
    simple_es3.frag
)

set(RESOURCE_FILES
    "ggsample01_config.json"
    "logo.obj"
    "logo.mtl"
    "Mplus1-Regular.ttf"
)

# 外部ライブラリの自動ダウンロード設定
set(FETCHCONTENT_BASE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/libs")
set(LIBS_DIR "${CMAKE_CURRENT_SOURCE_DIR}/libs")
set(IMGUI_DIR "${LIBS_DIR}/ImGui")
set(INCLUDE_DIR "${LIBS_DIR}/include")
set(GL_DIR "${INCLUDE_DIR}/GL")
set(KHR_DIR "${INCLUDE_DIR}/KHR")

file(MAKE_DIRECTORY "${LIBS_DIR}")
file(MAKE_DIRECTORY "${INCLUDE_DIR}")
file(MAKE_DIRECTORY "${GL_DIR}")
file(MAKE_DIRECTORY "${KHR_DIR}")

# 1. Dear ImGui & NFD
if(NOT EXISTS "${IMGUI_DIR}/imgui.cpp")
    if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/../calib/libs/ImGui/imgui.cpp")
        message(STATUS "Copying cached ImGui from calib/libs/ImGui...")
        file(COPY "${CMAKE_CURRENT_SOURCE_DIR}/../calib/libs/ImGui" DESTINATION "${LIBS_DIR}")
    else()
        message(STATUS "Downloading Dear ImGui...")
        file(DOWNLOAD "https://github.com/ocornut/imgui/archive/refs/tags/v1.92.8.zip" "${LIBS_DIR}/imgui.zip" SHOW_PROGRESS)
        execute_process(COMMAND ${CMAKE_COMMAND} -E tar -xf imgui.zip WORKING_DIRECTORY "${LIBS_DIR}")
        file(RENAME "${LIBS_DIR}/imgui-1.92.8" "${IMGUI_DIR}")
        file(REMOVE "${LIBS_DIR}/imgui.zip")

        message(STATUS "Downloading Native File Dialog Extended...")
        file(DOWNLOAD "https://github.com/btzy/nativefiledialog-extended/archive/refs/tags/v1.3.0.zip" "${LIBS_DIR}/nfd.zip" SHOW_PROGRESS)
        execute_process(COMMAND ${CMAKE_COMMAND} -E tar -xf nfd.zip WORKING_DIRECTORY "${LIBS_DIR}")
        set(NFD_SRC "${LIBS_DIR}/nativefiledialog-extended-1.3.0/src")
        file(COPY "${NFD_SRC}/include/nfd.h" DESTINATION "${IMGUI_DIR}")
        file(COPY "${NFD_SRC}/include/nfd.hpp" DESTINATION "${IMGUI_DIR}")
        file(COPY "${NFD_SRC}/nfd_win.cpp" DESTINATION "${IMGUI_DIR}")
        file(COPY "${NFD_SRC}/nfd_cocoa.m" DESTINATION "${IMGUI_DIR}")
        file(COPY "${NFD_SRC}/nfd_gtk.cpp" DESTINATION "${IMGUI_DIR}")
        file(REMOVE_RECURSE "${LIBS_DIR}/nativefiledialog-extended-1.3.0")
        file(REMOVE "${LIBS_DIR}/nfd.zip")
    endif()
endif()

# 2. picojson
if(NOT EXISTS "${INCLUDE_DIR}/picojson.h")
    if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/../calib/libs/include/picojson.h")
        file(COPY "${CMAKE_CURRENT_SOURCE_DIR}/../calib/libs/include/picojson.h" DESTINATION "${INCLUDE_DIR}")
    else()
        file(DOWNLOAD "https://raw.githubusercontent.com/kazuho/picojson/master/picojson.h" "${INCLUDE_DIR}/picojson.h")
    endif()
endif()

# 3. OpenGL & KHR headers
if(NOT EXISTS "${GL_DIR}/glcorearb.h")
    message(STATUS "Downloading OpenGL & KHR headers...")
    set(UA_HEADER "User-Agent: Mozilla/5.0 (Windows NT 10.0; Win64; x64) AppleWebKit/537.36")
    file(DOWNLOAD "https://registry.khronos.org/OpenGL/api/GL/glcorearb.h" "${GL_DIR}/glcorearb.h" HTTPHEADER "${UA_HEADER}")
    file(DOWNLOAD "https://registry.khronos.org/OpenGL/api/GL/glext.h" "${GL_DIR}/glext.h" HTTPHEADER "${UA_HEADER}")
    file(DOWNLOAD "https://registry.khronos.org/OpenGL/api/GL/wgl.h" "${GL_DIR}/wgl.h" HTTPHEADER "${UA_HEADER}")
    file(DOWNLOAD "https://registry.khronos.org/OpenGL/api/GL/wglext.h" "${GL_DIR}/wglext.h" HTTPHEADER "${UA_HEADER}")
    file(DOWNLOAD "https://registry.khronos.org/OpenGL/api/GL/glxext.h" "${GL_DIR}/glxext.h" HTTPHEADER "${UA_HEADER}")
    file(DOWNLOAD "https://registry.khronos.org/EGL/api/KHR/khrplatform.h" "${KHR_DIR}/khrplatform.h" HTTPHEADER "${UA_HEADER}")
endif()

# 4. GLFW
include(FetchContent)
FetchContent_Declare(
    glfw
    GIT_REPOSITORY https://github.com/glfw/glfw.git
    GIT_TAG 3.4
)
set(GLFW_BUILD_DOCS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
set(GLFW_INSTALL OFF CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(glfw)

set(IMGUI_SOURCES
    "${IMGUI_DIR}/imgui.cpp"
    "${IMGUI_DIR}/imgui_draw.cpp"
    "${IMGUI_DIR}/imgui_widgets.cpp"
    "${IMGUI_DIR}/imgui_tables.cpp"
    "${IMGUI_DIR}/backends/imgui_impl_glfw.cpp"
    "${IMGUI_DIR}/backends/imgui_impl_opengl3.cpp"
)

if(WIN32)
    list(APPEND IMGUI_SOURCES "${IMGUI_DIR}/nfd_win.cpp")
elseif(APPLE)
    list(APPEND IMGUI_SOURCES "${IMGUI_DIR}/nfd_cocoa.m")
else()
    list(APPEND IMGUI_SOURCES "${IMGUI_DIR}/nfd_gtk.cpp")
endif()

source_group("Source Files" FILES ${SOURCES})
source_group("Header Files" FILES ${HEADERS})
source_group("Shader Files" FILES ${SHADER_FILES})
source_group("Resource Files" FILES ${RESOURCE_FILES})
source_group("ImGui" FILES ${IMGUI_SOURCES})

if(APPLE)
    set(ICON_FILE "gg.icns")
    add_executable(${PROJECT_NAME} MACOSX_BUNDLE ${SOURCES} ${HEADERS} ${SHADER_FILES} ${RESOURCE_FILES} ${IMGUI_SOURCES} ${ICON_FILE})
    find_library(OPENGL_LIBRARY OpenGL)

    target_include_directories(${PROJECT_NAME} PRIVATE
        "${INCLUDE_DIR}"
        "${IMGUI_DIR}"
        "${IMGUI_DIR}/backends"
    )

    target_link_libraries(${PROJECT_NAME} PRIVATE
        glfw
        ${OPENGL_LIBRARY}
        "-framework Cocoa"
        "-framework IOKit"
        "-framework CoreVideo"
        "-framework AppKit"
    )

    set_target_properties(${PROJECT_NAME} PROPERTIES
        MACOSX_BUNDLE TRUE
        MACOSX_BUNDLE_INFO_PLIST "${CMAKE_CURRENT_SOURCE_DIR}/Info.plist"
        MACOSX_BUNDLE_GUI_IDENTIFIER "com.yourcompany.${PROJECT_NAME}"
        MACOSX_BUNDLE_BUNDLE_NAME "${PROJECT_NAME}"
        MACOSX_BUNDLE_BUNDLE_VERSION "1.0"
        MACOSX_BUNDLE_SHORT_VERSION_STRING "1.0"
        MACOSX_BUNDLE_COPYRIGHT "Copyright (c) 2018-2026 Kohe Tokoi. All rights reserved."
        XCODE_ATTRIBUTE_PRODUCT_BUNDLE_IDENTIFIER "com.yourcompany.${PROJECT_NAME}"
        XCODE_GENERATE_SCHEME ON
    )
    set_source_files_properties(${SHADER_FILES} ${RESOURCE_FILES} ${ICON_FILE} PROPERTIES MACOSX_PACKAGE_LOCATION Resources)
    set_property(DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}" PROPERTY XCODE_GENERATE_SCHEME ON)

elseif(WIN32)
    add_executable(${PROJECT_NAME} ${SOURCES} ${HEADERS} ${SHADER_FILES} ${RESOURCE_FILES} ${IMGUI_SOURCES})
    find_package(OpenGL REQUIRED)

    target_include_directories(${PROJECT_NAME} PRIVATE
        "${INCLUDE_DIR}"
        "${IMGUI_DIR}"
        "${IMGUI_DIR}/backends"
        "${glfw_SOURCE_DIR}/include"
    )

    target_link_libraries(${PROJECT_NAME} PRIVATE
        glfw
        OpenGL::GL
    )

    foreach(RESOURCE_FILE IN LISTS SHADER_FILES RESOURCE_FILES)
        add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E make_directory "$<TARGET_FILE_DIR:${PROJECT_NAME}>"
            COMMAND ${CMAKE_COMMAND} -E copy_if_different
                "${CMAKE_CURRENT_SOURCE_DIR}/${RESOURCE_FILE}"
                "$<TARGET_FILE_DIR:${PROJECT_NAME}>"
            COMMAND ${CMAKE_COMMAND} -E copy_if_different
                "${CMAKE_CURRENT_SOURCE_DIR}/${RESOURCE_FILE}"
                "${CMAKE_CURRENT_BINARY_DIR}"
        )
    endforeach()

    set_source_files_properties(${SHADER_FILES} ${RESOURCE_FILES} PROPERTIES
        HEADER_FILE_ONLY TRUE
        VS_COPY_TO_OUT_DIR PreserveNewest
    )

    set_property(DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}" PROPERTY VS_STARTUP_PROJECT ${PROJECT_NAME})
    set_target_properties(${PROJECT_NAME} PROPERTIES VS_DEBUGGER_WORKING_DIRECTORY "$<TARGET_FILE_DIR:${PROJECT_NAME}>")

else()
    add_executable(${PROJECT_NAME} ${SOURCES} ${HEADERS} ${SHADER_FILES} ${RESOURCE_FILES} ${IMGUI_SOURCES})
    find_package(OpenGL REQUIRED)
    find_package(glfw3 CONFIG REQUIRED)
    find_package(PkgConfig REQUIRED)
    pkg_check_modules(GTK3 REQUIRED gtk+-3.0)

    target_include_directories(${PROJECT_NAME} PRIVATE
        "${INCLUDE_DIR}"
        "${IMGUI_DIR}"
        "${IMGUI_DIR}/backends"
        ${GTK3_INCLUDE_DIRS}
    )

    target_link_libraries(${PROJECT_NAME} PRIVATE
        glfw
        OpenGL::GL
        ${GTK3_LIBRARIES}
        m
        pthread
        dl
    )

    foreach(RESOURCE_FILE IN LISTS SHADER_FILES RESOURCE_FILES)
        add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E make_directory "$<TARGET_FILE_DIR:${PROJECT_NAME}>"
            COMMAND ${CMAKE_COMMAND} -E copy_if_different
                "${CMAKE_CURRENT_SOURCE_DIR}/${RESOURCE_FILE}"
                "$<TARGET_FILE_DIR:${PROJECT_NAME}>"
        )
    endforeach()
endif()
