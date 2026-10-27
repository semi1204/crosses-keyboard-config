# Crosses BT firmware

ZMK configuration for the tested 54-key split Crosses BT keyboard. The right half is the central; the left half forwards keys and scrolling over Bluetooth. The personal 10-layer keymap is ported from [semi1204/sofle-keyboard-config](https://github.com/semi1204/sofle-keyboard-config).

## Verified Bluetooth configuration

This configuration was reported working over Bluetooth on 2026-10-27. The deployed firmware baseline is commit `24808e8f4962fb7c5fc7e5bae7496c375932c3ce`, built by [Actions run 40496429961](https://github.com/semi1204/crosses-keyboard-config/actions/runs/40496429961).

- Both halves use the 1 Mbps PHY (`CONFIG_BT_CTLR_PHY_2M=n`).
- Internal RC 32 kHz clock, 500 ppm, with calibration enabled.
- Matrix scan: row2col, with the left/right column order in the respective overlays.
- Trackball SPI: SCK P0.17, shared MOSI/MISO P0.20, interrupt P0.6.
- Right trackball: XY swap, with X/Y inversion removed.
- ZMK Studio locking is enabled.
- Normal builds produce only `crosses_54_right.uf2` and `crosses_54_left.uf2`.

These pin assignments describe the tested PCB revision. GPIO numbers above are raw Nordic port/pin numbers.

## Personal keymap

- Sofle layout adapted to 54 keys, with the middle column removed.
- Left thumbs: Option, Space/Command, Fn1/Escape. Right thumbs: B/Control, Space/Fn2, right Command.
- Space/Command uses balanced hold-tap at 200 ms. X can be held for Control and comma for right Option, both at 350 ms.
- BLE profile 1 selects the Windows modifier variant; default/USB uses Mac modifiers.
- Sofle combos, macros and window-management layers are retained.
- Right trackball movement enables AUTO MOUSE for 1000 ms; S/D/F become middle/right/left click; thumbs keep Option/Command for modified clicks.
- On the Fn2 layer, the left Space/Command thumb is Shift.
- Pointer multiplier: 1.75. Left scrolling uses a 1/8 multiplier after the peripheral's 1/4 scaler.
- On Manage layers, Z+X enters the right central's bootloader; C+V restarts the right central.

The initial four-layer device keymap is preserved separately in the `codex/crosses-initial-keymap-20261027` branch. It has not been applied to the working keyboard.

## Firmware checksums

The application images actually deployed for the verified baseline have these SHA-256 hashes:

```text
d9b915746aa900d84058b54c02de8e0dadd6c0c72fab99ff5afbdd837c27b905  crosses_54_right.uf2
dda755c72c0449ac8c2eee41ce06be9f2e9e2d460daf566fa7ef2e1a86464c38  crosses_54_left.uf2
```

The keymap tweaks above were then flashed to the right half only (commit `7c86c69`, [Actions run 40511391902](https://github.com/semi1204/crosses-keyboard-config/actions/runs/40511391902)); the left image is unchanged:

```text
9578d4405d36819c9f3c4c5b7c8a9a045546aeb261ac08787d7f522598753883  crosses_54_right.uf2
```

Build through GitHub Actions and use the file matching each half. Bluetooth bonds and saved Studio overrides live in device storage; this repository contains firmware sources and the compiled keymap.
