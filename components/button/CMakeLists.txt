set(SRC_FILES "button_gpio.c" "iot_button.c")

idf_component_register(SRCS ${SRC_FILES}
                        INCLUDE_DIRS include)
