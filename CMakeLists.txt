cmake_minimum_required(VERSION 3.21)
project(4PT_Repo_GUI)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

IF(APPLE)
    set(MACOS_DIR "${CMAKE_CURRENT_SOURCE_DIR}/build/macos/")
    set(CMAKE_PREFIX_PATH /Users/victorjimenezrugama/Desktop/qt-everywhere-src-6.2.3/cmake /usr/local/Cellar/yaml-cpp/0.6.3_1/lib/cmake)
ELSEIF(WIN32)
    set(WINDOWS_DIR "${CMAKE_CURRENT_SOURCE_DIR}/build/windows/")
    set(CMAKE_PREFIX_PATH C:/Qt/6.2.3/mingw_64/lib/cmake)
    set(REQUIRED_DLLS libgcc_s_seh-1 libstdc++-6 libwinpthread-1 opengl32sw Qt6OpenGL Qt6OpenGLWidgets)
ENDIF()

set(QT_VERSION 6)

set(REQUIRED_LIBS Core Gui Widgets UiTools)
set(REQUIRED_LIBS_QUALIFIED Qt${QT_VERSION}::Core Qt${QT_VERSION}::Gui Qt${QT_VERSION}::Widgets Qt${QT_VERSION}::UiTools  )
set(REQUIRED_YAML_LIB ${CMAKE_SOURCE_DIR}/libs/libyaml-cppd.a)

file(GLOB 4PT_Repo_GUI_SRC
        "src/*.h"
        "src/*.cpp"
        "src/PKGUtils/*.h"
        "src/PKGUtils/*.cpp"
        "src/sandbird/*.h"
        "src/sandbird/*.cpp"
        "src/zip/*.h"
        "src/zip/*.c"
        )

find_package(Qt${QT_VERSION} COMPONENTS ${REQUIRED_LIBS} REQUIRED)

set(RESOURCE ${CMAKE_SOURCE_DIR}/Resources/assets.qrc)
qt6_add_resources(RESOURCE_ADDED ${RESOURCE})
set(RESOURCE_ADDED ${RESOURCE_ADDED})

if(APPLE)
    message("Detected OSX")

    set(CMAKE_PREFIX_PATH /Users/victorjimenezrugama/Desktop/qt-everywhere-src-6.2.3/cmake /usr/local/Cellar/yaml-cpp/0.6.3_1/lib/cmake)

    find_package(yaml-cpp)

    ################################################################################################################
    ################################################################################################################
    ################################################DEFINE DEBUG################################################
    ################################################################################################################
    ################################################################################################################

    add_executable(DEBUG ${4PT_Repo_GUI_SRC} ${RESOURCE_ADDED})

    add_custom_command(TARGET DEBUG
            POST_BUILD
            COMMAND cp -r ${CMAKE_SOURCE_DIR}/Resources/* ${CMAKE_SOURCE_DIR}/cmake-build-debug/)

    target_link_libraries(DEBUG ${REQUIRED_LIBS_QUALIFIED} yaml-cpp)

    ################################################################################################################
    ################################################################################################################
    ################################################DEFINE BUILDOSX################################################
    ################################################################################################################
    ################################################################################################################

    add_executable(BUILD_OSX ${4PT_Repo_GUI_SRC} ${RESOURCE_ADDED})

    set_target_properties(BUILD_OSX
            PROPERTIES
            RUNTIME_OUTPUT_DIRECTORY "${MACOS_DIR}/bin")

    target_link_libraries(BUILD_OSX ${REQUIRED_LIBS_QUALIFIED} yaml-cpp)

    add_custom_command(TARGET BUILD_OSX
            POST_BUILD
            COMMAND cd  ${CMAKE_CURRENT_SOURCE_DIR} && ./genOSXApp.sh '4PT Repository Creator' 'build/macos/bin/BUILD_OSX' 'Resources/icon.png' 1 00
            COMMAND cp -r ${CMAKE_SOURCE_DIR}/cmake-build-debug/qrc_assets.cpp ${MACOS_DIR}4PT\ Repository\ Creator.app/Contents/Resources/
            COMMAND rm -r ${MACOS_DIR}bin
            )


    ################################################################################################################
    ################################################################################################################
    ################################################DEFINE CLEAN################################################
    ################################################################################################################
    ################################################################################################################

    add_custom_target(CLEAN
            COMMAND cd  ${CMAKE_CURRENT_SOURCE_DIR} && rm -rf build && rm -rf cmake-build-debug
            )

ELSEIF (WIN32)
    message("Detected Windows")


    add_executable(${PROJECT_NAME} ${4PT_Repo_GUI_SRC})

    set_target_properties(${PROJECT_NAME}
            PROPERTIES
            RUNTIME_OUTPUT_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/build/windows/bin")

    if(NOT CMAKE_PREFIX_PATH)
        message(WARNING "CMAKE_PREFIX_PATH is not defined, you may need to set it "
                "(-DCMAKE_PREFIX_PATH=\"path/to/Qt/lib/cmake\" or -DCMAKE_PREFIX_PATH=/usr/include/{host}/qt{version}/ on Ubuntu)")
    endif()
    include_directories(${CMAKE_SOURCE_DIR}/include)
    target_link_libraries(${PROJECT_NAME} )

    target_link_libraries(${PROJECT_NAME} ${REQUIRED_LIBS_QUALIFIED} ${REQUIRED_YAML_LIB} ws2_32)

    set(QT_INSTALL_PATH "${CMAKE_PREFIX_PATH}")
    if(NOT EXISTS "${QT_INSTALL_PATH}/bin")
        set(QT_INSTALL_PATH "${QT_INSTALL_PATH}/..")
        if(NOT EXISTS "${QT_INSTALL_PATH}/bin")
            set(QT_INSTALL_PATH "${QT_INSTALL_PATH}/..")
        endif()
    endif()
    if(EXISTS "${QT_INSTALL_PATH}/plugins/platforms/qwindows.dll")
        add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
                COMMAND ${CMAKE_COMMAND} -E make_directory
                "$<TARGET_FILE_DIR:${PROJECT_NAME}>/plugins/platforms/")
        add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
                COMMAND ${CMAKE_COMMAND} -E copy
                "${QT_INSTALL_PATH}/plugins/platforms/qwindows.dll"
                "$<TARGET_FILE_DIR:${PROJECT_NAME}>/plugins/platforms/")
    endif()
    foreach(QT_LIB ${REQUIRED_LIBS})
        add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
                COMMAND ${CMAKE_COMMAND} -E copy
                "${QT_INSTALL_PATH}/bin/Qt${QT_VERSION}${QT_LIB}.dll"
                "$<TARGET_FILE_DIR:${PROJECT_NAME}>")
    endforeach(QT_LIB)
    foreach(DLL ${REQUIRED_DLLS})
        add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
                COMMAND ${CMAKE_COMMAND} -E copy
                "${QT_INSTALL_PATH}/bin/${DLL}.dll"
                "$<TARGET_FILE_DIR:${PROJECT_NAME}>")
    endforeach(DLL)

    file(GLOB RESOURCES
            "Resources/*.*"
            )

    foreach(RESOURCE ${RESOURCES})
    add_custom_command(TARGET ${PROJECT_NAME}
            POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy ${RESOURCE} "$<TARGET_FILE_DIR:${PROJECT_NAME}>")
    endforeach(RESOURCE)
ELSE()
    message("Unsupported System")
    # set stuff for other systems
ENDIF()