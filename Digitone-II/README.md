# Digitone II

A VST3 plug-in that emulates the Elektron Digitone II and runs its real operating system. It is not made or supported by Elektron, and it does not include any Elektron firmware.

## Install

On a Mac, copy `plugin/mac/Digitone II.vst3` to `~/Library/Audio/Plug-Ins/VST3/`. If you downloaded it with a browser, macOS will block it until you run `xattr -dr com.apple.quarantine ~/Library/Audio/Plug-Ins/VST3/"Digitone II.vst3"`. On Windows, copy the folder `plugin/windows/Digitone II.vst3` to `C:\Program Files\Common Files\VST3\`. Then rescan plug-ins in your DAW.

## Firmware

You need Digitone II OS 1.11 (other versions don't work yet). Download it from the Elektron support page for your machine, unzip it and put `Digitone_II_OS1.11.syx` in:

- Mac: `~/Library/Application Support/Elektron Emu/Digitone II/Firmware/`
- Windows: `%APPDATA%\Elektron Emu\Digitone II\Firmware\`

The plug-in creates the folder and shows "Open folder" and "Choose firmware..." until the file is there. The first start boots the OS from the .syx and then builds the DSP for your computer: a few minutes, once. The build needs a C compiler. On a Mac it uses Apple's command line tools (the plug-in asks for them and offers to install them if they are missing). On Windows it downloads a small open source compiler (llvm-mingw, about 190 MB) the first time, so it needs an internet connection then. Your machine is saved in `~/Library/Application Support/Elektron Emu/Digitone II` (Windows: `%APPDATA%\Elektron Emu\Digitone II`). It is kept when you close the DAW, even without saving the project. Only one instance at a time, like the real machine.

## Build

On an Apple Silicon Mac with the Xcode command line tools, cmake, ninja and python3:

    cd source
    vst/scripts/build-mac.sh ~/build/dn

The DSP is not part of the build: the plug-in translates and compiles it on the user's machine (`source/vst/NativeBuild.cpp`, `source/core/dsp/dn_gensrc.inc`). The script downloads Unicorn 2.1.4 and DPF at a fixed commit, applies the patches in `patches/` and `vst/patches/`, and builds `~/build/dn/mac/vst-build/bin/Digitone II.vst3`. `vst/scripts/build-windows.sh ~/build/dn` cross-compiles the Windows version with mingw-w64. `dtcore/` is a copy of the engine files shared with the Digitakt II plug-in. Code comments are in Italian.

## Keyboard controls

These controls are in the source on `dev/digitone2-keyboard`; rebuild the plug-in to use them. The checked-in `plugin/mac` and `plugin/windows` bundles predate this change.

Give the plug-in window keyboard focus. The layout follows Syntakt's trigger rows, Option/Alt FUNC, Tab TRK, navigation and transport, with Model:Cycles-style feedback for held controls. Every mapped button and encoder push stays down until its physical key is released; holding a key does not generate repeated presses. Multiple keys can be held together, including FUNC + TRK or FUNC + a trigger. The panel outlines every held control, with a different colour for mouse-latched modifiers.

| Computer key | Digitone II control |
| --- | --- |
| `1`–`8` | TRIG 1–8 |
| `Q W E R T Y U I` | TRIG 9–16 |
| Left or right Option/Alt | FUNC (hold) |
| `Tab` | TRK (hold; combine with a trigger to select a track) |
| `Space` / `.` / `J` | PLAY / STOP / RECORD |
| Arrow keys | UP / DOWN / LEFT / RIGHT |
| `Enter` or keypad Enter | YES |
| `Backspace` | NO |
| `P` / `S` / `K` | PTN / SONG / keyboard mode |
| `9` | PAGE |
| `F1` / `F2` / `F3` / `F4` | TRIG / SYN / FLTR / AMP parameter page |
| `F5` / `F6` | MOD / FX parameter page (LFO / FX positions on Syntakt) |
| `F7` / `F8` | TEMPO / SETTINGS |
| `F9` / `F10` | PRESET/KIT / VOICE SETUP |
| `F11` / `F12` | ARPEGGIATOR / NOTE EDIT |
| `+` or `=` / `-` | TRANSPOSE+ / TRANSPOSE− |
| `Z X C V B N M ,` | Push encoders A–H |
| `0` | Push LEVEL/DATA |
| `Escape` | Release all held and latched controls |
| `Cmd+S` / `Ctrl+S` | Save the emulated machine immediately |

`R` remains TRIG 12, so RECORD uses `J`. To copy, clear or paste, hold Option/Alt and press `J`, `Space` or `.` respectively. Turn encoders and Main Volume by dragging or scrolling the mouse as before; Shift speeds up encoder scrolling. On keyboards with media-function keys, you may need `Fn` for F1–F12. Other Ctrl/Cmd combinations are left to the host, including Ctrl+number desktop switching.

Hold Option/Alt and press `F9` for PERFORM KIT, or `F10` to toggle UNISON. The standalone keys open PRESET/KIT and VOICE SETUP respectively.

Keyboard, mouse and mouse-latched presses are tracked independently: a mouse release or latch timeout cannot release a button still held on the keyboard. Leaving the plug-in window, closing its UI, or pressing Escape releases all inputs to prevent stuck buttons. Keyboard controls activate once the engine is running.

The bindings live in `source/core/layout/digitone2_panel.json` and are compiled by `source/vst/scripts/gen-layout.py`. Run the firmware-free input regression tests with the DPF checkout used for the build:

```sh
DPF_ROOT=/path/to/DPF bash source/vst/tests/run-input-tests.sh
```

The Mac build script also applies `source/vst/patches/pugl-mac-modifiers.patch` inside DPF's `dgl/src/pugl-upstream` submodule. The pinned Pugl version reports modifier releases as presses; the patch reads the native left/right modifier flags so releasing Option releases FUNC, including when both Option keys have been held. Apply that patch manually if configuring a Mac build without `build-mac.sh`.

## Known limits

If the compiler is not available it falls back to a slow interpreter (about 0.5x real time): offline rendering works (in Ableton, Export Audio or Freeze) but live playback stays silent and shows a warning. macOS builds are Apple Silicon only and a few +Drive error messages are still in Italian.
