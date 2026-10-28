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
- Space/Command uses balanced hold-tap at 200 ms and is always Space within 150 ms of typing. X can be held for Control, and T or comma for right Option, all at 350 ms.
- Fn1 numpad on the right three columns: 789 / 456 / 123 / 0, with Delete bottom right (F9/F10 stay on Fn2).
- BLE profile 1 selects the Windows modifier variant; default/USB uses Mac modifiers.
- Sofle combos, macros and window-management layers are retained.
- Right trackball movement enables AUTO MOUSE for 1000 ms; S/D/F become middle/right/left click; thumbs keep Option/Command for modified clicks.
- On the Fn2 layer, the left Space/Command thumb is Shift.
- On AUTO MOUSE, tapping S or Y is middle click; holding S or Y turns the right ball into a scroll wheel. U is also left click.
- On the Fn2 layer, the left ball zooms with Cmd +/- (Alt +/- on the Windows profile) via the local `modules/scroll-keys` input processor.
- Pointer multiplier: 1.75. Left scrolling uses a 1/8 multiplier after the peripheral's 1/4 scaler.
- On Manage layers, Z+X enters the right central's bootloader; C+V restarts the right central.

The initial four-layer device keymap is preserved separately in the `codex/crosses-initial-keymap-20261027` branch. It has not been applied to the working keyboard.

## Firmware checksums

The application images actually deployed for the verified baseline have these SHA-256 hashes:

```text
d9b915746aa900d84058b54c02de8e0dadd6c0c72fab99ff5afbdd837c27b905  crosses_54_right.uf2
dda755c72c0449ac8c2eee41ce06be9f2e9e2d460daf566fa7ef2e1a86464c38  crosses_54_left.uf2
```

The current deployed images for both halves (commit `1640292`, [Actions run 40638763816](https://github.com/semi1204/crosses-keyboard-config/actions/runs/40638763816)):

```text
7f27dd3d17d2966fd5ac1494f7e0c2779bfd362404593df35a2aed830ca86994  crosses_54_right.uf2
ba0d8a9d5fde1a3ce54a5014a57e7c8c71f6b92f67cdd55eecc0ad0d908e79c6  crosses_54_left.uf2
```

Build through GitHub Actions and use the file matching each half. Bluetooth bonds and saved Studio overrides live in device storage; this repository contains firmware sources and the compiled keymap.
