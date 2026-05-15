#
# Copyright (c) 2026 Nordic Semiconductor
#
# SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
#

#zephyr_syscall_header(${ZEPHYR_BASE}../nrf/include/zephyr/saadct.h)

zephyr_library()

zephyr_sources_ifdef(CONFIG_SAADCT saadct_nrfx.c)
