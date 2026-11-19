/* SPDX-License-Identifier: MIT */

/*
 * Rolling from a Space/Cmd thumb into a letter and pressing Cmd+letter quickly look the same
 * until something is released. For the few keys where a wrong guess is costly (close, quit,
 * hide, minimize) wait for that release:
 *   letter released first -> the letter is tapped while the hold is down (Cmd+letter)
 *   thumb released first  -> it was typing: the thumb's tap (Space), then the letter
 * A guarded key pressed quick-ms or later after the thumb went down is a plain key press.
 * One guarded hold per keymap: the state below is shared.
 */

#include <zephyr/device.h>
#include <zephyr/devicetree.h>
#include <zephyr/kernel.h>

#include <drivers/behavior.h>
#include <zmk/behavior.h>
#include <zmk/event_manager.h>
#include <zmk/events/keycode_state_changed.h>

#define MAX_WAITING 4

struct waiting_key {
    uint32_t position;
    uint32_t keycode;
    bool typed; /* already sent as typing; only its key release is still to come */
};

static bool hold_down;
static int64_t hold_since;
static int32_t hold_quick_ms;

static struct waiting_key waiting[MAX_WAITING];
static uint8_t waiting_len;

static void tap_keycode(uint32_t keycode) {
    const int64_t now = k_uptime_get();
    raise_zmk_keycode_state_changed_from_encoded(keycode, true, now);
    raise_zmk_keycode_state_changed_from_encoded(keycode, false, now);
}

/* ---- the hold ---- */

#define DT_DRV_COMPAT zmk_behavior_roll_guard_hold

struct guard_hold_config {
    struct zmk_behavior_binding bindings[2];
    int32_t quick_ms;
};

static int on_hold_pressed(struct zmk_behavior_binding *binding,
                           struct zmk_behavior_binding_event event) {
    const struct device *dev = zmk_behavior_get_binding(binding->behavior_dev);
    const struct guard_hold_config *config = dev->config;

    hold_down = true;
    hold_since = event.timestamp; /* hold-tap passes the time its key went down */
    hold_quick_ms = config->quick_ms;
    return zmk_behavior_invoke_binding(&config->bindings[0], event, true);
}

static int on_hold_released(struct zmk_behavior_binding *binding,
                            struct zmk_behavior_binding_event event) {
    const struct device *dev = zmk_behavior_get_binding(binding->behavior_dev);
    const struct guard_hold_config *config = dev->config;

    hold_down = false;
    const int ret = zmk_behavior_invoke_binding(&config->bindings[0], event, false);

    bool tapped = false;
    for (int i = 0; i < waiting_len; i++) {
        if (waiting[i].typed) {
            continue;
        }
        if (!tapped) {
            event.timestamp = k_uptime_get();
            zmk_behavior_invoke_binding(&config->bindings[1], event, true);
            zmk_behavior_invoke_binding(&config->bindings[1], event, false);
            tapped = true;
        }
        tap_keycode(waiting[i].keycode);
        waiting[i].typed = true;
    }
    return ret;
}

static const struct behavior_driver_api guard_hold_driver_api = {
    .locality = BEHAVIOR_LOCALITY_CENTRAL,
    .binding_pressed = on_hold_pressed,
    .binding_released = on_hold_released,
#if IS_ENABLED(CONFIG_ZMK_BEHAVIOR_METADATA)
    .get_parameter_metadata = zmk_behavior_get_empty_param_metadata,
#endif
};

#define GUARD_BINDING(node, idx)                                                                   \
    {                                                                                              \
        .behavior_dev = DEVICE_DT_NAME(DT_INST_PHANDLE_BY_IDX(node, bindings, idx)),               \
        .param1 = COND_CODE_0(DT_INST_PHA_HAS_CELL_AT_IDX(node, bindings, idx, param1), (0),       \
                              (DT_INST_PHA_BY_IDX(node, bindings, idx, param1))),                 \
        .param2 = COND_CODE_0(DT_INST_PHA_HAS_CELL_AT_IDX(node, bindings, idx, param2), (0),       \
                              (DT_INST_PHA_BY_IDX(node, bindings, idx, param2))),                 \
    }

#define GUARD_HOLD_INST(n)                                                                         \
    static const struct guard_hold_config guard_hold_config_##n = {                               \
        .bindings = {GUARD_BINDING(n, 0), GUARD_BINDING(n, 1)},                                    \
        .quick_ms = DT_INST_PROP(n, quick_ms),                                                     \
    };                                                                                             \
    BEHAVIOR_DT_INST_DEFINE(n, NULL, NULL, NULL, &guard_hold_config_##n, POST_KERNEL,              \
                            CONFIG_KERNEL_INIT_PRIORITY_DEFAULT, &guard_hold_driver_api);

DT_INST_FOREACH_STATUS_OKAY(GUARD_HOLD_INST)

/* ---- the guarded keys ---- */

#undef DT_DRV_COMPAT
#define DT_DRV_COMPAT zmk_behavior_roll_guard

static int on_key_pressed(struct zmk_behavior_binding *binding,
                          struct zmk_behavior_binding_event event) {
    if (hold_down && event.timestamp - hold_since < hold_quick_ms && waiting_len < MAX_WAITING) {
        waiting[waiting_len++] = (struct waiting_key){
            .position = event.position,
            .keycode = binding->param1,
        };
        return ZMK_BEHAVIOR_OPAQUE;
    }
    return raise_zmk_keycode_state_changed_from_encoded(binding->param1, true, event.timestamp);
}

static int on_key_released(struct zmk_behavior_binding *binding,
                           struct zmk_behavior_binding_event event) {
    for (int i = 0; i < waiting_len; i++) {
        if (waiting[i].position != event.position) {
            continue;
        }
        const struct waiting_key key = waiting[i];
        waiting_len--;
        for (int j = i; j < waiting_len; j++) { /* keep press order for the others */
            waiting[j] = waiting[j + 1];
        }
        if (!key.typed) {
            tap_keycode(key.keycode); /* the hold is still down: this is the shortcut */
        }
        return ZMK_BEHAVIOR_OPAQUE;
    }
    return raise_zmk_keycode_state_changed_from_encoded(binding->param1, false, event.timestamp);
}

#if IS_ENABLED(CONFIG_ZMK_BEHAVIOR_METADATA)

static const struct behavior_parameter_value_metadata guard_param_values[] = {
    {
        .display_name = "Key",
        .type = BEHAVIOR_PARAMETER_VALUE_TYPE_HID_USAGE,
    },
};

static const struct behavior_parameter_metadata_set guard_param_metadata_set[] = {{
    .param1_values = guard_param_values,
    .param1_values_len = ARRAY_SIZE(guard_param_values),
}};

static const struct behavior_parameter_metadata guard_metadata = {
    .sets_len = ARRAY_SIZE(guard_param_metadata_set),
    .sets = guard_param_metadata_set,
};

#endif

static const struct behavior_driver_api guard_driver_api = {
    .locality = BEHAVIOR_LOCALITY_CENTRAL,
    .binding_pressed = on_key_pressed,
    .binding_released = on_key_released,
#if IS_ENABLED(CONFIG_ZMK_BEHAVIOR_METADATA)
    .parameter_metadata = &guard_metadata,
#endif
};

#define GUARD_INST(n)                                                                              \
    BEHAVIOR_DT_INST_DEFINE(n, NULL, NULL, NULL, NULL, POST_KERNEL,                                \
                            CONFIG_KERNEL_INIT_PRIORITY_DEFAULT, &guard_driver_api);

DT_INST_FOREACH_STATUS_OKAY(GUARD_INST)
