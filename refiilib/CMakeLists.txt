project("refiilib")

add_compile_options(
    -fno-strict-aliasing
)

if (WIN32)
    add_compile_options(/fp:strict)
else()
    add_compile_options(-ffp-model=strict)
endif()

set(REFII_PPC_RECOMPILED_SOURCES 
    "${CMAKE_CURRENT_SOURCE_DIR}/ppc/ppc_config.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/ppc/ppc_context.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/ppc/ppc_func_mapping.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/ppc/ppc_recomp_shared.h"
)
set(REFII_CONFIG_FILE="${CMAKE_CURRENT_SOURCE_DIR}/config/config.toml")

target_compile_definitions(XenonRecomp PRIVATE 
    XENON_RECOMP_CONFIG_FILE_PATH=\"${CMAKE_CURRENT_SOURCE_DIR}/config/config.toml\"
    XENON_RECOMP_HEADER_FILE_PATH=\"${REFII_TOOLS_ROOT}/XenonRecomp/XenonUtils/ppc_context.h\")

 foreach(i RANGE 0 240)
    list(APPEND REFII_PPC_RECOMPILED_SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/ppc/ppc_recomp.${i}.cpp")
 endforeach()

add_custom_command(
    OUTPUT 
        ${REFII_PPC_RECOMPILED_SOURCES}
    COMMAND 
        $<TARGET_FILE:XenonRecomp>
    DEPENDS 
        "${CMAKE_CURRENT_SOURCE_DIR}/private/default.xex"
        "${CMAKE_CURRENT_SOURCE_DIR}/config/config.toml"
)

add_library(refiilib 
    ${REFII_PPC_RECOMPILED_SOURCES}
)

target_include_directories(refiilib 
    PUBLIC 
        ${CMAKE_CURRENT_SOURCE_DIR}
)

target_precompile_headers(refiilib
    PUBLIC
        "ppc/ppc_recomp_shared.h"
)


