/*
 * Copyright (c) 2017-2018, ARM Limited and Contributors. All rights reserved.
 * Copyright (c) 2018,2020 The Linux Foundation. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef QCOM_UART_CONSOLE_H
#define QCOM_UART_CONSOLE_H

#include <drivers/console.h>

#ifndef __ASSEMBLER__

int qcom_console_uart_register(console_t *console, uintptr_t uart_base_addr);

#endif /* __ASSEMBLER__ */

#endif /* QCOM_UART_CONSOLE_H */
