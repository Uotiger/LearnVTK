# DeployQt.cmake - Qt 应用程序部署模块 (修复参数问题)
# 用法：deploy_qt_for_target(TARGET_NAME [QUIET])

# 查找 Qt 部署工具 (Windows)
if(WIN32)
    # 查找 Debug 部署工具 (windeployqt.debug.bat)
    find_program(WINDEPLOYQT_DEBUG
            NAMES windeployqt.debug.bat
            PATHS
            "$ENV{VCPKG_ROOT}/installed/*/tools/qt6/bin"
            DOC "Qt Windows deployment tool for Debug"
    )

    # 查找 Release 部署工具 (windeployqt.exe)
    find_program(WINDEPLOYQT_RELEASE
            NAMES windeployqt.exe
            PATHS
            "$ENV{VCPKG_ROOT}/installed/*/tools/qt6/bin"
            DOC "Qt Windows deployment tool for Release"
    )

    # 设置工具查找状态
    set(WINDEPLOYQT_FOUND FALSE)
    if(WINDEPLOYQT_DEBUG AND WINDEPLOYQT_RELEASE)
        set(WINDEPLOYQT_FOUND TRUE)
    endif()
endif()

# 定义部署函数
function(deploy_qt_for_target TARGET)
    # 解析可选参数
    set(options QUIET)
    cmake_parse_arguments(ARG "${options}" "" "" ${ARGN})

    # 仅支持 Windows 部署
    if(NOT WIN32)
        if(NOT ARG_QUIET)
            message(STATUS "Qt deployment skipped (only supported on Windows)")
        endif()
        return()
    endif()

    # 检查部署工具
    if(NOT WINDEPLOYQT_FOUND)
        if(NOT ARG_QUIET)
            if(NOT WINDEPLOYQT_DEBUG)
                message(WARNING "windeployqt.debug.bat not found!")
            endif()
            if(NOT WINDEPLOYQT_RELEASE)
                message(WARNING "windeployqt.exe not found!")
            endif()
            message(WARNING "Qt deployment tools missing. Deployment skipped.")
        endif()
        return()
    endif()

    # 获取目标输出目录
    get_target_property(TARGET_OUTPUT_DIR ${TARGET} RUNTIME_OUTPUT_DIRECTORY)
    if(NOT TARGET_OUTPUT_DIR)
        # 回退到全局输出目录
        if(CMAKE_BUILD_TYPE STREQUAL "Debug")
            set(TARGET_OUTPUT_DIR ${CMAKE_RUNTIME_OUTPUT_DIRECTORY_DEBUG})
        else()
            set(TARGET_OUTPUT_DIR ${CMAKE_RUNTIME_OUTPUT_DIRECTORY_RELEASE})
        endif()
    endif()

    if(NOT TARGET_OUTPUT_DIR)
        # 最终回退到二进制目录
        set(TARGET_OUTPUT_DIR ${CMAKE_CURRENT_BINARY_DIR})
    endif()

    # 确保目录存在
    file(MAKE_DIRECTORY ${TARGET_OUTPUT_DIR})

    # 创建自定义部署目标
    set(DEPLOY_TARGET ${TARGET}_deploy_qt)

    add_custom_target(${DEPLOY_TARGET} ALL
            COMMENT "Deploying Qt dependencies for ${TARGET}"
            VERBATIM
    )
    #简单版本
#    if(CMAKE_BUILD_TYPE STREQUAL "Debug")
#        add_custom_command(TARGET ${DEPLOY_TARGET} POST_BUILD
#                COMMAND ${WINDEPLOYQT_DEBUG} ${TARGET_OUTPUT_DIR}/${TARGET}.exe)
#    else()
#        add_custom_command(TARGET ${DEPLOY_TARGET} POST_BUILD
#                COMMAND ${WINDEPLOYQT_RELEASE} ${TARGET_OUTPUT_DIR}/${TARGET}.exe)
#    endif()

        # 添加部署命令 - 简化参数并修复顺序
    add_custom_command(TARGET ${DEPLOY_TARGET} POST_BUILD
            COMMAND ${WINDEPLOYQT_DEBUG} ${TARGET_OUTPUT_DIR}/${TARGET}.exe
            COMMAND ${CMAKE_COMMAND} -E echo "Running windeployqt to deploy Qt dependencies"

            # 日志输出
            COMMAND ${CMAKE_COMMAND} -E echo "=========================================="
            COMMAND ${CMAKE_COMMAND} -E echo "Starting Qt deployment for $<CONFIG> build"
            COMMAND ${CMAKE_COMMAND} -E echo "Using tool: $<$<CONFIG:Debug>:${WINDEPLOYQT_DEBUG}>$<$<CONFIG:Release>:${WINDEPLOYQT_RELEASE}>"
            COMMAND ${CMAKE_COMMAND} -E echo "Target executable: $<TARGET_FILE:${TARGET}>"
            COMMAND ${CMAKE_COMMAND} -E echo "Working directory: ${TARGET_OUTPUT_DIR}"
            COMMAND ${CMAKE_COMMAND} -E echo "=========================================="

            # 实际部署命令 - 简化参数
            COMMAND
            "$<$<CONFIG:Debug>:${WINDEPLOYQT_DEBUG}>$<$<CONFIG:Release>:${WINDEPLOYQT_RELEASE}>"
            "$<TARGET_FILE:${TARGET}>"

            # 检查部署结果 - 使用正确的目录列表命令
            COMMAND ${CMAKE_COMMAND} -E echo "=========================================="
            COMMAND ${CMAKE_COMMAND} -E echo "Checking deployment results..."
            COMMAND ${CMAKE_COMMAND} -E echo "Deployed files in ${TARGET_OUTPUT_DIR}:"
            COMMAND ${CMAKE_COMMAND} -E chdir "${TARGET_OUTPUT_DIR}" cmd /c dir /B
            COMMAND ${CMAKE_COMMAND} -E echo "=========================================="

            COMMENT "Running Qt deployment for $<CONFIG> build"
            VERBATIM
            WORKING_DIRECTORY "${TARGET_OUTPUT_DIR}"
    )

    # 添加依赖关系
    add_dependencies(${DEPLOY_TARGET} ${TARGET})

    # 输出状态信息
    if(NOT ARG_QUIET)
        message(STATUS "Added Qt deployment for target: ${TARGET}")
        message(STATUS "  Debug tool: ${WINDEPLOYQT_DEBUG}")
        message(STATUS "  Release tool: ${WINDEPLOYQT_RELEASE}")
        message(STATUS "  Output directory: ${TARGET_OUTPUT_DIR}")
    endif()
endfunction()

# 自动包含 Qt 包（可选）
if(NOT TARGET Qt6::Core AND WIN32)
    find_package(Qt6 COMPONENTS Core QUIET)
endif()

# 提供设置自定义路径的函数
function(set_qt_deploy_paths DEBUG_TOOL RELEASE_TOOL)
    if(EXISTS "${DEBUG_TOOL}")
        set(WINDEPLOYQT_DEBUG "${DEBUG_TOOL}" CACHE FILEPATH "Custom debug deploy tool" FORCE)
    endif()
    if(EXISTS "${RELEASE_TOOL}")
        set(WINDEPLOYQT_RELEASE "${RELEASE_TOOL}" CACHE FILEPATH "Custom release deploy tool" FORCE)
    endif()
    if(WINDEPLOYQT_DEBUG AND WINDEPLOYQT_RELEASE)
        set(WINDEPLOYQT_FOUND TRUE CACHE BOOL "Qt deploy tools found" FORCE)
    endif()
endfunction()