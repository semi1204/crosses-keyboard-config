/* SPDX-License-Identifier: MIT */

#define DT_DRV_COMPAT zmk_input_processor_motion_gate

#include <stdlib.h>

#include <zephyr/device.h>
#include <zephyr/devicetree.h>
#include <zephyr/dt-bindings/input/input-event-codes.h>
#include <zephyr/kernel.h>

#include <drivers/input_processor.h>
#include <zmk/keymap.h>

struct motion_gate_config {
    const struct device *target;
    int32_t threshold;
    int32_t window_ms;
};

struct motion_gate_data {
    int64_t burst_start;
    int64_t last_event;
    int32_t net_x;
    int32_t net_y;
    bool open;
};

static bool layer_active(uint32_t layer) {
    return layer < ZMK_KEYMAP_LAYERS_LEN &&
           zmk_keymap_layer_active(zmk_keymap_layer_index_to_id(layer));
}

static int motion_gate_handle_event(const struct device *dev, struct input_event *event,
                                    uint32_t param1, uint32_t param2,
                                    struct zmk_input_processor_state *state) {
    const struct motion_gate_config *config = dev->config;
    struct motion_gate_data *data = dev->data;

    if (event->type != INPUT_EV_REL ||
        (event->code != INPUT_REL_X && event->code != INPUT_REL_Y)) {
        return ZMK_INPUT_PROC_CONTINUE;
    }

    const int64_t now = k_uptime_get();
    if (now - data->last_event > config->window_ms) {
        data->open = false;
    }
    data->last_event = now;

    if (!data->open && layer_active(param1)) {
        data->open = true;
    }

    if (!data->open) {
        if (now - data->burst_start > config->window_ms) {
            data->burst_start = now;
            data->net_x = 0;
            data->net_y = 0;
        }
        if (event->code == INPUT_REL_X) {
            data->net_x += event->value;
        } else {
            data->net_y += event->value;
        }
        if (abs(data->net_x) + abs(data->net_y) < config->threshold) {
            return ZMK_INPUT_PROC_CONTINUE;
        }
        data->open = true;
    }

    int ret = zmk_input_processor_handle_event(config->target, event, param1, param2, state);
    return ret < 0 ? ret : ZMK_INPUT_PROC_CONTINUE;
}

static const struct zmk_input_processor_driver_api motion_gate_driver_api = {
    .handle_event = motion_gate_handle_event,
};

#define MOTION_GATE_INST(n)                                                                        \
    static const struct motion_gate_config motion_gate_config_##n = {                             \
        .target = DEVICE_DT_GET(DT_INST_PHANDLE(n, target)),                                       \
        .threshold = DT_INST_PROP(n, threshold),                                                   \
        .window_ms = DT_INST_PROP(n, window_ms),                                                   \
    };                                                                                             \
    static struct motion_gate_data motion_gate_data_##n;                                          \
    DEVICE_DT_INST_DEFINE(n, NULL, NULL, &motion_gate_data_##n, &motion_gate_config_##n,           \
                          POST_KERNEL, CONFIG_KERNEL_INIT_PRIORITY_DEFAULT,                        \
                          &motion_gate_driver_api);

DT_INST_FOREACH_STATUS_OKAY(MOTION_GATE_INST)
