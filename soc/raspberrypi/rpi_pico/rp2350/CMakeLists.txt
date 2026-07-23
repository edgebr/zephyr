# Copyright (c) 2021 Yonatan Schachter
# SPDX-License-Identifier: Apache-2.0

zephyr_library()

zephyr_library_sources(soc.c)

if(CONFIG_PM)
  zephyr_library_sources(power.c)
endif()

if(CONFIG_PM_S2RAM)
  zephyr_library_sources(pm_s2ram_skip.c)
  zephyr_library_sources(rp2350_powman.c)
  # For <cortex_m/exception.h>, used to restore core CPU state on S2RAM resume.
  zephyr_library_include_directories(${ZEPHYR_BASE}/arch/arm/include)
endif()

zephyr_include_directories(.)

set(SOC_LINKER_SCRIPT ${CMAKE_CURRENT_SOURCE_DIR}/linker.ld CACHE INTERNAL "")
