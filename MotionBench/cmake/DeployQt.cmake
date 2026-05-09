# -----------------------------------------------------------------------------
# Copyright (c) 2026 Adam G. Sweeney
# SPDX-License-Identifier: MIT
#
# Contributors:
#   2026 Adam G. Sweeney <agsweeney@gmail.com> - MotionBench original implementation
#
# File: DeployQt.cmake
# Purpose: windeployqt post-build hook for Windows Qt runtime deployment.
#
# Attribution: Portions of this design/implementation are influenced by
# OpENer (Open Source EtherNet/IP Adapter Stack), where applicable.
# -----------------------------------------------------------------------------

function(setup_qt_deploy target_name)
    if(NOT WIN32)
        return()
    endif()

    get_target_property(_target_type ${target_name} TYPE)
    if(NOT _target_type STREQUAL "EXECUTABLE")
        message(STATUS "Skipping Qt deploy for non-executable target ${target_name}")
        return()
    endif()

    # Prefer explicit override for CI/build environments.
    if(DEFINED WINQTDEPLOY_EXECUTABLE)
        set(_windeployqt "${WINQTDEPLOY_EXECUTABLE}")
    else()
        find_program(_windeployqt
            NAMES windeployqt
            HINTS
                "${Qt6_DIR}/../../../bin"
                "$ENV{QTDIR}/bin"
        )
    endif()

    if(NOT _windeployqt)
        message(WARNING "windeployqt (WinQTDeploy) not found; Qt runtime deployment skipped.")
        return()
    endif()

    add_custom_command(TARGET ${target_name} POST_BUILD
        COMMAND "${_windeployqt}"
                --qmldir "${CMAKE_CURRENT_SOURCE_DIR}/qml"
                --verbose 1
                "$<TARGET_FILE:${target_name}>"
        COMMENT "Deploying Qt runtime dependencies beside executable with WinQTDeploy/windeployqt"
        VERBATIM
    )
endfunction()
