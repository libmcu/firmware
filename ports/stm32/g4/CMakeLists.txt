# SPDX-License-Identifier: Apache-2.0

AUX_SOURCE_DIRECTORY(${CMAKE_CURRENT_LIST_DIR} PORT_SRCS)

set(SDK_ROOT ${CMAKE_SOURCE_DIR}/external/STM32CubeG4)
set(LD_SCRIPT ${CMAKE_CURRENT_LIST_DIR}/STM32G473CEUx_FLASH.ld)
set(ST_SRCS
	${SDK_ROOT}/Drivers/CMSIS/Device/ST/STM32G4xx/Source/Templates/gcc/startup_stm32g473xx.s
	${SDK_ROOT}/Drivers/CMSIS/Device/ST/STM32G4xx/Source/Templates/system_stm32g4xx.c

	${SDK_ROOT}/Drivers/STM32G4xx_HAL_Driver/Src/stm32g4xx_hal_pcd.c
	${SDK_ROOT}/Drivers/STM32G4xx_HAL_Driver/Src/stm32g4xx_hal_pcd_ex.c
	${SDK_ROOT}/Drivers/STM32G4xx_HAL_Driver/Src/stm32g4xx_ll_usb.c
	${SDK_ROOT}/Drivers/STM32G4xx_HAL_Driver/Src/stm32g4xx_hal.c
	${SDK_ROOT}/Drivers/STM32G4xx_HAL_Driver/Src/stm32g4xx_hal_rcc.c
	${SDK_ROOT}/Drivers/STM32G4xx_HAL_Driver/Src/stm32g4xx_hal_rcc_ex.c
	${SDK_ROOT}/Drivers/STM32G4xx_HAL_Driver/Src/stm32g4xx_hal_flash.c
	${SDK_ROOT}/Drivers/STM32G4xx_HAL_Driver/Src/stm32g4xx_hal_flash_ex.c
	${SDK_ROOT}/Drivers/STM32G4xx_HAL_Driver/Src/stm32g4xx_hal_flash_ramfunc.c
	${SDK_ROOT}/Drivers/STM32G4xx_HAL_Driver/Src/stm32g4xx_hal_gpio.c
	${SDK_ROOT}/Drivers/STM32G4xx_HAL_Driver/Src/stm32g4xx_hal_exti.c
	${SDK_ROOT}/Drivers/STM32G4xx_HAL_Driver/Src/stm32g4xx_hal_dma.c
	${SDK_ROOT}/Drivers/STM32G4xx_HAL_Driver/Src/stm32g4xx_hal_dma_ex.c
	${SDK_ROOT}/Drivers/STM32G4xx_HAL_Driver/Src/stm32g4xx_hal_pwr.c
	${SDK_ROOT}/Drivers/STM32G4xx_HAL_Driver/Src/stm32g4xx_hal_pwr_ex.c
	${SDK_ROOT}/Drivers/STM32G4xx_HAL_Driver/Src/stm32g4xx_hal_cortex.c
	${SDK_ROOT}/Drivers/STM32G4xx_HAL_Driver/Src/stm32g4xx_hal_adc.c
	${SDK_ROOT}/Drivers/STM32G4xx_HAL_Driver/Src/stm32g4xx_hal_adc_ex.c
	${SDK_ROOT}/Drivers/STM32G4xx_HAL_Driver/Src/stm32g4xx_ll_adc.c
	${SDK_ROOT}/Drivers/STM32G4xx_HAL_Driver/Src/stm32g4xx_hal_fdcan.c
	${SDK_ROOT}/Drivers/STM32G4xx_HAL_Driver/Src/stm32g4xx_hal_i2c.c
	${SDK_ROOT}/Drivers/STM32G4xx_HAL_Driver/Src/stm32g4xx_hal_i2c_ex.c
	${SDK_ROOT}/Drivers/STM32G4xx_HAL_Driver/Src/stm32g4xx_hal_qspi.c
	${SDK_ROOT}/Drivers/STM32G4xx_HAL_Driver/Src/stm32g4xx_hal_tim.c
	${SDK_ROOT}/Drivers/STM32G4xx_HAL_Driver/Src/stm32g4xx_hal_tim_ex.c
	${SDK_ROOT}/Drivers/STM32G4xx_HAL_Driver/Src/stm32g4xx_hal_uart.c
	${SDK_ROOT}/Drivers/STM32G4xx_HAL_Driver/Src/stm32g4xx_hal_uart_ex.c

	${SDK_ROOT}/Middlewares/Third_Party/FreeRTOS/Source/croutine.c
	${SDK_ROOT}/Middlewares/Third_Party/FreeRTOS/Source/event_groups.c
	${SDK_ROOT}/Middlewares/Third_Party/FreeRTOS/Source/list.c
	${SDK_ROOT}/Middlewares/Third_Party/FreeRTOS/Source/queue.c
	${SDK_ROOT}/Middlewares/Third_Party/FreeRTOS/Source/stream_buffer.c
	${SDK_ROOT}/Middlewares/Third_Party/FreeRTOS/Source/tasks.c
	${SDK_ROOT}/Middlewares/Third_Party/FreeRTOS/Source/timers.c
	${SDK_ROOT}/Middlewares/Third_Party/FreeRTOS/Source/portable/MemMang/heap_4.c
	${SDK_ROOT}/Middlewares/Third_Party/FreeRTOS/Source/portable/GCC/ARM_CM4F/port.c
	${SDK_ROOT}/Middlewares/ST/STM32_USB_Device_Library/Core/Src/usbd_core.c
	${SDK_ROOT}/Middlewares/ST/STM32_USB_Device_Library/Core/Src/usbd_ctlreq.c
	${SDK_ROOT}/Middlewares/ST/STM32_USB_Device_Library/Core/Src/usbd_ioreq.c
	${SDK_ROOT}/Middlewares/ST/STM32_USB_Device_Library/Class/CDC/Src/usbd_cdc.c

	${PORT_SRCS}

	${CMAKE_SOURCE_DIR}/external/libmcu/ports/stubs/semaphore.c
)
set(ST_INCS
	${SDK_ROOT}/Drivers/STM32G4xx_HAL_Driver/Inc
	${SDK_ROOT}/Drivers/STM32G4xx_HAL_Driver/Inc/Legacy
	${SDK_ROOT}/Drivers/CMSIS/Device/ST/STM32G4xx/Include
	${SDK_ROOT}/Drivers/CMSIS/Include
	${SDK_ROOT}/Middlewares/Third_Party/FreeRTOS/Source/include
	${SDK_ROOT}/Middlewares/Third_Party/FreeRTOS/Source/portable/GCC/ARM_CM4F
	${SDK_ROOT}/Middlewares/ST/STM32_USB_Device_Library/Core/Inc
	${SDK_ROOT}/Middlewares/ST/STM32_USB_Device_Library/Class/CDC/Inc

	${CMAKE_CURRENT_LIST_DIR}
)
set(ST_DEFS
	USE_HAL_DRIVER
	STM32G473xx
	STM32_THREAD_SAFE_STRATEGY=4
)

target_include_directories(libmcu PUBLIC
	${CMAKE_SOURCE_DIR}/external/libmcu/modules/common/include/libmcu/posix)

add_library(stm32 OBJECT ${ST_SRCS})
target_include_directories(stm32 PRIVATE ${APP_INCS} PUBLIC ${ST_INCS})
target_compile_definitions(stm32 PUBLIC ${ST_DEFS})
target_link_libraries(stm32 libmcu)

target_link_libraries(${PROJECT_NAME} PRIVATE stm32)
target_link_options(${PROJECT_NAME} PRIVATE -T${LD_SCRIPT})
