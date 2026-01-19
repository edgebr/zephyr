# SPDX-License-Identifier: Apache-2.0

set(flag_for_ram_report ram)
set(flag_for_rom_report rom)
set(flag_for_footprint all)
set(report_depth 99)

if(DEFINED ZEPHYR_WORKSPACE)
  set(workspace_arg "--workspace=${ZEPHYR_WORKSPACE}")
elseif(DEFINED WEST_TOPDIR)
  set(workspace_arg "--workspace=${WEST_TOPDIR}")
endif()

foreach(report ram rom)
  add_custom_target(
    ${report}_report
    ${PYTHON_EXECUTABLE}
    ${ZEPHYR_BASE}/scripts/footprint/size_report
    -k ${ZEPHYR_BINARY_DIR}/${KERNEL_ELF_NAME}
    -z ${ZEPHYR_BASE}
    -o ${CMAKE_BINARY_DIR}
    ${workspace_arg}
    -d ${report_depth}
    --json ${report}.json
    ${flag_for_${report}_report}
    DEPENDS ${logical_target_for_zephyr_elf}
            $<TARGET_PROPERTY:zephyr_property_target,${report}_report_DEPENDENCIES>
    USES_TERMINAL
    WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
    )
  add_custom_target(
    ${report}_plot
    ${PYTHON_EXECUTABLE}
    ${ZEPHYR_BASE}/scripts/footprint/plot.py
    ${report}.json
    DEPENDS ${report}_report
    USES_TERMINAL
    WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
    )
endforeach()

add_custom_target(
  footprint
  DEPENDS ram_report rom_report
  )

if(CONFIG_BUILD_WITH_TFM)
  foreach(report ram rom)
    add_custom_target(
      tfm_${report}_report
      ${PYTHON_EXECUTABLE}
      ${ZEPHYR_BASE}/scripts/footprint/size_report
      -k $<TARGET_PROPERTY:tfm,TFM_S_ELF_FILE>
      -z ${ZEPHYR_BASE}
      -o ${CMAKE_BINARY_DIR}
      ${workspace_arg}
      -d ${report_depth}
      --json tfm_${report}.json
      ${flag_for_${report}_report}
      DEPENDS tfm
      USES_TERMINAL
      WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
      )
    add_custom_target(
      tfm_${report}_plot
      ${PYTHON_EXECUTABLE}
      ${ZEPHYR_BASE}/scripts/footprint/plot.py
      tfm_${report}.json
      DEPENDS tfm_${report}_report
      USES_TERMINAL
      WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
      )
  endforeach()

  add_custom_target(
    tfm_footprint
    DEPENDS tfm_ram_report tfm_rom_report
    )
endif()

if(CONFIG_TFM_BL2)
  foreach(report ram rom)
    add_custom_target(
      bl2_${report}_report
      ${PYTHON_EXECUTABLE}
      ${ZEPHYR_BASE}/scripts/footprint/size_report
      -k $<TARGET_PROPERTY:tfm,BL2_ELF_FILE>
      -z ${ZEPHYR_BASE}
      -o ${CMAKE_BINARY_DIR}
      ${workspace_arg}
      -d ${report_depth}
      --json bl2_${report}.json
      ${flag_for_${report}_report}
      DEPENDS tfm
      USES_TERMINAL
      WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
      )
    add_custom_target(
      bl2_${report}_plot
      ${PYTHON_EXECUTABLE}
      ${ZEPHYR_BASE}/scripts/footprint/plot.py
      bl2_${report}.json
      DEPENDS bl2_${report}_report
      USES_TERMINAL
      WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
      )
  endforeach()

  add_custom_target(
    bl2_footprint
    DEPENDS bl2_ram_report bl2_rom_report
    )
endif()

find_program(PUNCOVER puncover)

if(NOT ${PUNCOVER} STREQUAL PUNCOVER-NOTFOUND)
  set(PUNCOVER_ARGS "")
  zephyr_get(PUNCOVER_PORT)
  zephyr_get(PUNCOVER_HOST)
  if(DEFINED PUNCOVER_PORT)
    list(APPEND PUNCOVER_ARGS "--port=${PUNCOVER_PORT}")
  endif()
  if(DEFINED PUNCOVER_HOST)
    list(APPEND PUNCOVER_ARGS "--host=${PUNCOVER_HOST}")
  endif()
  add_custom_target(
    puncover
    ${PUNCOVER}
    --elf_file       ${ZEPHYR_BINARY_DIR}/${KERNEL_ELF_NAME}
    --gcc-tools-base ${CROSS_COMPILE}
    --src_root       ${ZEPHYR_BASE}
    --build_dir      ${CMAKE_BINARY_DIR}
    ${PUNCOVER_ARGS}
    DEPENDS ${logical_target_for_zephyr_elf}
            $<TARGET_PROPERTY:zephyr_property_target,${report}_DEPENDENCIES>
    WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
    USES_TERMINAL
    )
endif()

find_program(PAHOLE pahole)

if(NOT ${PAHOLE} STREQUAL PAHOLE-NOTFOUND)
  add_custom_target(
    pahole
    ${PAHOLE}
    --anon_include
    --nested_anon_include
    --show_decl_info
    $<$<BOOL:${CMAKE_VERBOSE_MAKEFILE}>:--verbose>
    ${ZEPHYR_BINARY_DIR}/${KERNEL_ELF_NAME}
    DEPENDS ${logical_target_for_zephyr_elf}
            $<TARGET_PROPERTY:zephyr_property_target,${report}_DEPENDENCIES>
    WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
    USES_TERMINAL
    )
endif()

find_program(ZVIEW zview)

if(NOT ${ZVIEW} STREQUAL ZVIEW-NOTFOUND)
  if(CONFIG_INIT_STACKS AND CONFIG_THREAD_MONITOR AND CONFIG_THREAD_STACK_INFO)
    add_custom_target(
      zview
      ${ZVIEW}
      --elf          ${ZEPHYR_BINARY_DIR}/${KERNEL_ELF_NAME}
      --runners_yaml ${ZEPHYR_BINARY_DIR}/runners.yaml
      $<$<BOOL:$ENV{ZVIEW_RUNNER}>:--runner=$ENV{ZVIEW_RUNNER}>
      $<$<BOOL:${CONFIG_SYS_HEAP_RUNTIME_STATS}>:--has_heap_stats>
      $<$<BOOL:${CONFIG_THREAD_RUNTIME_STATS}>:--has_thread_runtime_stats>
      $<$<BOOL:${CONFIG_THREAD_NAME}>:--has_thread_names>
      --thread_name_size ${CONFIG_THREAD_MAX_NAME_LEN}
      DEPENDS ${logical_target_for_zephyr_elf}
        $<TARGET_PROPERTY:zephyr_property_target,${report}_DEPENDENCIES>
      WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
      USES_TERMINAL
      )
  else()
    message(WARNING "There are unmet dependencies for ZView, it will be unable to run.")
  endif()
endif()
