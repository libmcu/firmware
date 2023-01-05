# SPDX-License-Identifier: Apache-2.0

if(PREREQUISITE_ONLY STREQUAL true)
include(${BASEDIR}/projects/arch/cm4f.cmake)
return()
endif()

add_executable(${PROJECT_NAME})
target_include_directories(${PROJECT_NAME} PRIVATE ${APP_INCS})
target_compile_definitions(${PROJECT_NAME} PRIVATE ${APP_DEFS})

add_subdirectory(${CMAKE_SOURCE_DIR}/ports/at32/f403a)

target_link_libraries(${PROJECT_NAME} PRIVATE c nosys m fpl_app)

add_custom_target(${PROJECT_NAME}.bin ALL DEPENDS ${PROJECT_NAME})
add_custom_target(${PROJECT_NAME}.hex ALL DEPENDS ${PROJECT_NAME})
add_custom_target(flash DEPENDS ${PROJECT_NAME}.hex)

add_custom_command(TARGET ${PROJECT_NAME}.hex
	COMMAND ${CMAKE_OBJCOPY} -O ihex $<TARGET_FILE:${PROJECT_NAME}>
			${PROJECT_NAME}.hex
	WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
)

add_custom_command(TARGET ${PROJECT_NAME}.bin 
	COMMAND ${CMAKE_OBJCOPY} -O binary $<TARGET_FILE:${PROJECT_NAME}>
			${PROJECT_NAME}.bin
	WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
)

add_custom_command(TARGET flash
	USES_TERMINAL COMMAND
		dfu-util --device 2e3c:df11 --alt 0
			--download ${PROJECT_NAME}.bin
			--dfuse-address 0x08000000
			--reset
)
