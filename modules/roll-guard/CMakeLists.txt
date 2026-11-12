# Keycode events exist on the central half only, like ZMK's own key press behavior.
if(CONFIG_ZMK_BEHAVIOR_ROLL_GUARD AND ((NOT CONFIG_ZMK_SPLIT) OR CONFIG_ZMK_SPLIT_ROLE_CENTRAL))
  zephyr_library()
  zephyr_library_include_directories(${APPLICATION_SOURCE_DIR}/include)
  zephyr_library_sources(src/behavior_roll_guard.c)
endif()
