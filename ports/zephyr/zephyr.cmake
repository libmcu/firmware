zephyr_include_directories(${APP_INCS})
zephyr_compile_definitions(${APP_DEFS})
target_sources(app PRIVATE ${APP_SRCS})
