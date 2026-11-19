/* SPDX-License-Identifier: MIT */

/*
 * A thumb that taps a key and holds a modifier wants two rules: right after typing it should
 * nearly always type, after a pause it should be the modifier at once. ZMK's
 * require-prior-idle-ms can only make it always type, so a shortcut pressed right after typing
 * (Cmd+V) came out as Space and V. This behavior picks one of two hold-taps when the key goes
 * down:
 *   bindings[0]  no key pressed for prior-idle-ms or more (a pause)
 *   bindings[1]  a key pressed less than prior-idle-ms ago (typing)
 * The release goes to the hold-tap the press went to. Both get this key's two parameters.
 * "A key pressed" is any non-modifier keycode press, the same as require-prior-idle-ms.
 */

#define DT_DRV_COMPAT zmk_behavior_typing_switch

#include <zephyr/device.h>
#include <zephyr/kernel.h>

#include <drivers/behavior.h>
#include <zmk/behavior.h>
#include <zmk/event_manager.h>
#include <zmk/events/keycode_state_changed.h>
#include <zmk/keys.h>

#if DT_HAS_COMPAT_STATUS_OKAY(DT_DRV_COMPAT)

#define MAX_HELD 4

struct typing_switch_config {
    const char *bindings[2];
    int32_t prior_idle_ms;
};

struct held_key {
    uint32_t position;
    const char *behavior_dev;
};

/* Same start value as ZMK's hold-tap: far in the past without overflowing. */
static int64_t last_key_press = INT32_MIN;

static struct held_key held[MAX_HELD];
static uint8_t held_len;

static int typing_switch_keycode_listener(const zmk_event_t *eh) {
    const struct zmk_keycode_state_changed *ev = as_zmk_keycode_state_changed(eh);
    if (ev != NULL && ev->state && !is_mod(ev->usage_page, ev->keycode) &&
        ev->timestamp > last_key_press) {
        last_key_press = ev->timestamp;
    }
    return ZMK_EV_EVENT_BUBBLE;
}

ZMK_LISTENER(behavior_typing_switch, typing_switch_keycode_listener);
ZMK_SUBSCRIPTION(behavior_typing_switch, zmk_keycode_state_changed);

static int on_switch_pressed(struct zmk_behavior_binding *binding,
                             struct zmk_behavior_binding_event event) {
    const struct device *dev = zmk_behavior_get_binding(binding->behavior_dev);
    const struct typing_switch_config *config = dev->config;

    const bool typing = event.timestamp - last_key_press < config->prior_idle_ms;
    const struct zmk_behavior_binding chosen = {
        .behavior_dev = config->bindings[typing ? 1 : 0],
        .param1 = binding->param1,
        .param2 = binding->param2,
    };

    if (held_len < MAX_HELD) {
        held[held_len++] = (struct held_key){
            .position = event.position,
            .behavior_dev = chosen.behavior_dev,
        };
    }
    return zmk_behavior_invoke_binding(&chosen, event, true);
}

static int on_switch_released(struct zmk_behavior_binding *binding,
                              struct zmk_behavior_binding_event event) {
    const struct device *dev = zmk_behavior_get_binding(binding->behavior_dev);
    const struct typing_switch_config *config = dev->config;

    const char *behavior_dev = config->bindings[0];
    for (int i = 0; i < held_len; i++) {
        if (held[i].position != event.position) {
            continue;
        }
        behavior_dev = held[i].behavior_dev;
        held_len--;
        for (int j = i; j < held_len; j++) {
            held[j] = held[j + 1];
        }
        break;
    }

    const struct zmk_behavior_binding chosen = {
        .behavior_dev = behavior_dev,
        .param1 = binding->param1,
        .param2 = binding->param2,
    };
    return zmk_behavior_invoke_binding(&chosen, event, false);
}

#if IS_ENABLED(CONFIG_ZMK_BEHAVIOR_METADATA)
/* Both hold-taps take the same parameters; describe them with the first one's. */
static int typing_switch_parameter_metadata(const struct device *dev,
                                            struct behavior_parameter_metadata *param_metadata) {
    const struct typing_switch_config *config = dev->config;
    return behavior_get_parameter_metadata(zmk_behavior_get_binding(config->bindings[0]),
                                           param_metadata);
}
#endif

static const struct behavior_driver_api typing_switch_driver_api = {
    .locality = BEHAVIOR_LOCALITY_CENTRAL,
    .binding_pressed = on_switch_pressed,
    .binding_released = on_switch_released,
#if IS_ENABLED(CONFIG_ZMK_BEHAVIOR_METADATA)
    .get_parameter_metadata = typing_switch_parameter_metadata,
#endif
};

#define TYPING_SWITCH_INST(n)                                                                      \
    static const struct typing_switch_config typing_switch_config_##n = {                         \
        .bindings = {DEVICE_DT_NAME(DT_INST_PHANDLE_BY_IDX(n, bindings, 0)),                       \
                     DEVICE_DT_NAME(DT_INST_PHANDLE_BY_IDX(n, bindings, 1))},                      \
        .prior_idle_ms = DT_INST_PROP(n, prior_idle_ms),                                           \
    };                                                                                             \
    BEHAVIOR_DT_INST_DEFINE(n, NULL, NULL, NULL, &typing_switch_config_##n, POST_KERNEL,           \
                            CONFIG_KERNEL_INIT_PRIORITY_DEFAULT, &typing_switch_driver_api);

DT_INST_FOREACH_STATUS_OKAY(TYPING_SWITCH_INST)

#endif /* DT_HAS_COMPAT_STATUS_OKAY(DT_DRV_COMPAT) */
