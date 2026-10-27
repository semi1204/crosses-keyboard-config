/* SPDX-License-Identifier: MIT */

#define DT_DRV_COMPAT zmk_input_processor_scroll_keys

#include <stdlib.h>

#include <zephyr/device.h>
#include <zephyr/devicetree.h>
#include <zephyr/dt-bindings/input/input-event-codes.h>
#include <zephyr/kernel.h>

#include <drivers/input_processor.h>
#include <zmk/behavior.h>
#include <zmk/keymap.h>
#include <zmk/virtual_key_position.h>

struct scroll_keys_config {
    uint8_t index;
    uint16_t code;
    int16_t threshold;
    struct zmk_behavior_binding bindings[2];
};

struct scroll_keys_data {
    int16_t accumulated;
};

static int tap_binding(const struct scroll_keys_config *config, uint8_t i, uint8_t listener_index) {
    struct zmk_behavior_binding_event event = {
        .position = ZMK_VIRTUAL_KEY_POSITION_BEHAVIOR_INPUT_PROCESSOR(listener_index, config->index),
        .timestamp = k_uptime_get(),
#if IS_ENABLED(CONFIG_ZMK_SPLIT)
        .source = ZMK_POSITION_STATE_CHANGE_SOURCE_LOCAL,
#endif
    };

    int ret = zmk_behavior_invoke_binding(&config->bindings[i], event, true);
    if (ret < 0) {
        return ret;
    }
    return zmk_behavior_invoke_binding(&config->bindings[i], event, false);
}

static int scroll_keys_handle_event(const struct device *dev, struct input_event *event,
                                    uint32_t param1, uint32_t param2,
                                    struct zmk_input_processor_state *state) {
    const struct scroll_keys_config *config = dev->config;
    struct scroll_keys_data *data = dev->data;

    if (event->type != INPUT_EV_REL) {
        return ZMK_INPUT_PROC_CONTINUE;
    }
    if (event->code != config->code) {
        return ZMK_INPUT_PROC_STOP;
    }

    data->accumulated += event->value;
    while (abs(data->accumulated) >= config->threshold) {
        const uint8_t i = data->accumulated > 0 ? 0 : 1;
        data->accumulated += i == 0 ? -config->threshold : config->threshold;

        int ret = tap_binding(config, i, state->input_device_index);
        if (ret < 0) {
            return ret;
        }
    }

    return ZMK_INPUT_PROC_STOP;
}

static const struct zmk_input_processor_driver_api scroll_keys_driver_api = {
    .handle_event = scroll_keys_handle_event,
};

#define SCROLL_KEYS_INST(n)                                                                        \
    BUILD_ASSERT(DT_INST_PROP_LEN(n, bindings) == 2, "scroll-keys needs exactly two bindings");    \
    static const struct scroll_keys_config scroll_keys_config_##n = {                             \
        .index = n,                                                                                \
        .code = DT_INST_PROP(n, code),                                                             \
        .threshold = DT_INST_PROP(n, threshold),                                                   \
        .bindings = {ZMK_KEYMAP_EXTRACT_BINDING(0, DT_DRV_INST(n)),                                \
                     ZMK_KEYMAP_EXTRACT_BINDING(1, DT_DRV_INST(n))},                               \
    };                                                                                             \
    static struct scroll_keys_data scroll_keys_data_##n;                                          \
    DEVICE_DT_INST_DEFINE(n, NULL, NULL, &scroll_keys_data_##n, &scroll_keys_config_##n,           \
                          POST_KERNEL, CONFIG_KERNEL_INIT_PRIORITY_DEFAULT,                        \
                          &scroll_keys_driver_api);

DT_INST_FOREACH_STATUS_OKAY(SCROLL_KEYS_INST)
