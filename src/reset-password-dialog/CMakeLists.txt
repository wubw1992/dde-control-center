cmake_minimum_required(VERSION 3.7)

set(VERSION 4.0)

set(BIN_NAME reset-password-dialog)
set(DEEPIN_PW_CHECK libdeepin_pw_check.so)

#set(CMAKE_VERBOSE_MAKEFILE ON)
set(CMAKE_CXX_STANDARD 14)
set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_CXX_FLAGS "-g -Wall")

# 增加安全编译参数
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fstack-protector-all")
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fstack-protector-all")
set(CMAKE_EXE_LINKER_FLAGS  "-z relro -z now -z noexecstack -pie")

if (${CMAKE_SYSTEM_PROCESSOR} MATCHES "mips64")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O3 -ftree-vectorize -march=loongson3a -mhard-float -mno-micromips -mno-mips16 -flax-vector-conversions -mloongson-ext2 -mloongson-mmi")
endif()

if (DEFINED ENABLE_MIEEE)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -mieee")
endif()


if(NOT DEFINED DISABLE_SYS_UPDATE)

set(SRCS
        main.cpp
        utils.h
        resetpassworddialog.h
        resetpassworddialog.cpp
        securitykeywidget.h
        securitykeywidget.cpp
        unionidwidget.h
        unionidwidget.cpp
        passwordwidget.h
        passwordwidget.cpp
        resetpasswordworker.h
        resetpasswordworker.cpp
)

set(DCC_SRCS
    ../frame/widgets/securitylevelitem.cpp
    ../frame/window/modules/accounts/pwqualitymanager.cpp
    ../frame/window/utils.h
    ../frame/window/dconfigwatcher.cpp
)

set(QRC resetpassworddialog.qrc)
# Find the library
find_package(PkgConfig REQUIRED)
find_package(Qt5 COMPONENTS
    Widgets
    Concurrent
    DBus
    Svg
    Network
REQUIRED)
find_package(DtkWidget REQUIRED)
find_package(DtkGui REQUIRED)

pkg_check_modules(DFrameworkDBus REQUIRED dframeworkdbus)

add_executable(${BIN_NAME} ${SRCS} ${DCC_SRCS} ${QRC})
target_include_directories(${BIN_NAME} PUBLIC
    ${DtkWidget_INCLUDE_DIRS}
    ${DFrameworkDBus_INCLUDE_DIRS}
    ${Qt5Gui_PRIVATE_INCLUDE_DIRS}
    ${PROJECT_BINARY_DIR}
    ../../include
    ../frame/window/modules/accounts/
    ../frame/
)

target_link_libraries(${BIN_NAME} PRIVATE
    dccwidgets
    ${DFrameworkDBus_LIBRARIES}
    ${DtkWidget_LIBRARIES}
    ${Qt5Widgets_LIBRARIES}
    ${Qt5Network_LIBRARIES}
    ${Qt5DBus_LIBRARIES}
    ${Qt5Concurrent_LIBRARIES}
    ${Qt5Svg_LIBRARIES}
    ${DEEPIN_PW_CHECK}
    crypt
    ${LIBS}
)

# bin
install(TARGETS ${BIN_NAME} DESTINATION lib/dde-control-center/)

endif()
