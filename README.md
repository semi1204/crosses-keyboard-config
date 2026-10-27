# crosses-keyboard-config

Crosses BT 54 (gggw) firmware with the keymap ported from
[semi1204/sofle-keyboard-config](https://github.com/semi1204/sofle-keyboard-config) (`codex/ctrl-rightclick-thumb`).

- Base: Sofle layout without the middle column. Thumbs: L Cmd / Space / Fn1-Esc, R B-Ctrl / Space-Fn2 / RCmd.
- X and `,`: tap = key, hold 350 ms = Option (Windows profile: Alt).
- Sofle combos, macros and Mac/Windows profile modifiers are kept.
- Moving the right trackball enables the AUTO MOUSE layer (S/D/F = right/middle/left click) for 500 ms.
- Trackball speed: pointer x1.75, scroll x0.4 on top of the stock /4.

Flash `crosses_54_right` (USB / central) and `crosses_54_left` from the Actions artifacts.
After flashing, reset ZMK Studio settings once so the saved Studio keymap does not override this one.
