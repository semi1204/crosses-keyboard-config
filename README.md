# Crosses BT firmware

ZMK configuration for the tested 54-key split Crosses BT keyboard. The left half is the central (so it also works on its own); the right half forwards its keys and raw trackball movement over Bluetooth. The personal 10-layer keymap is ported from [semi1204/sofle-keyboard-config](https://github.com/semi1204/sofle-keyboard-config).

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
- One-handed mode (left half alone): Space + Fn1/Escape toggles it. The Space thumb taps Space and, held, mirrors the right hand onto the left keys (Half-QWERTY: D→K, Tab→Backspace, Caps→Enter); the Option thumb becomes Shift and D+F gives Command (with Space held, D+F deletes a word like J+K).
- Space/Command uses balanced hold-tap at 200 ms and is always Space within 150 ms of typing. X can be held for Control, and T or comma for right Option, all at 350 ms.
- Fn1 numpad on the right three columns: 789 / 456 / 123 / 0, with Delete bottom right (F9/F10 stay on Fn2).
- BLE profile 1 selects the Windows modifier variant; default/USB uses Mac modifiers.
- Sofle combos, macros and window-management layers are retained.
- Right trackball movement enables AUTO MOUSE for 1000 ms; S/D/F become middle/right/left click; the Command thumb stays Command for modified clicks.
- On the Fn2 layer, the left Space/Command thumb is Shift.
- On AUTO MOUSE, tapping S is middle click; holding S turns the right ball into a scroll wheel. U is also left click.
- On AUTO MOUSE, the left Option thumb taps as left click (quick re-taps give double/triple click); holding it past 150 ms turns the right ball into a scroll wheel (no Option).
- On the Fn2 layer, the left ball zooms with Cmd +/- (Alt +/- on the Windows profile) via the local `modules/scroll-keys` input processor.
- Pointer multiplier: 1.75. Left scrolling uses a 1/32 multiplier.
- On Manage layers, Z+X enters the left central's bootloader; C+V restarts the left central.

The initial four-layer device keymap is preserved separately in the `codex/crosses-initial-keymap-20261027` branch. It has not been applied to the working keyboard.

## Firmware checksums

The application images actually deployed for the verified baseline have these SHA-256 hashes:

```text
d9b915746aa900d84058b54c02de8e0dadd6c0c72fab99ff5afbdd837c27b905  crosses_54_right.uf2
dda755c72c0449ac8c2eee41ce06be9f2e9e2d460daf566fa7ef2e1a86464c38  crosses_54_left.uf2
```

The current deployed images for both halves (commit `6aaf924`, [Actions run 40681071358](https://github.com/semi1204/crosses-keyboard-config/actions/runs/40681071358)). Switching central roles required a settings reset on both halves and re-pairing the hosts:

```text
90996ea3b9d17f2858433e9e4f0fddee31024f02ec842bc906e5c72c99a4d7a1  crosses_54_right.uf2
a8e7871c0b0dfd3bac0f8a956e0b24fbb55782524164dcce6682fe22c2388651  crosses_54_left.uf2
```

Build through GitHub Actions and use the file matching each half. Bluetooth bonds and saved Studio overrides live in device storage; this repository contains firmware sources and the compiled keymap.
