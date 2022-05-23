```shell
$ git submodule add https://github.com/FreeRTOS/FreeRTOS-Kernel.git external/FreeRTOS-Kernel
$ cat sources.mk
FREERTOS_ROOT ?= external/FreeRTOS-Kernel
FREERTOS_PORT_ROOT ?= ports/freertos
include $(FREERTOS_PORT_ROOT)/freertos.mk

SRCS += $(FREERTOS_SRCS)
INCS += $(FREERTOS_INCS) $(FREERTOS_PORT_ROOT)
```
