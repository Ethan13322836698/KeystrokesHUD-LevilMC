# Keystrokes HUD / Windows Client

A native **LeviLamina Windows x64 client mod** showing W/A/S/D and SPACE on Minecraft's own UI overlay. Key presses change the button background and border. **F8** toggles visibility. Does not send network packets, simulate input, or change game movement.

## Requirements
- Windows 10/11 x64, LeviLauncher with a Minecraft Bedrock instance and **client-side LeviLamina 26.60.7–26.60.x** (reference version 26.60.8).
- For compilation: Visual Studio C++ Desktop workload + Windows SDK, LLVM/clang-cl, xmake and Git.
- **Only the source code is included. This ZIP is not a prebuilt DLL or directly installable mod.**

## Build
Open PowerShell in the project folder:

```powershell
xmake repo -u
xmake f -y -p windows -a x64 -m release --target_type=client
xmake -w -y
```

If successful, xmake's `@levibuildscript/modpacker` creates `bin/keystrokes-hud/` containing a compiled DLL and a substituted `manifest.json`.

## Install
1. Open **LeviLauncher** on Windows and select the Minecraft instance already configured with **LeviLamina client**.
2. Find the **instance/game installation directory** (not the vanilla game directory). Copy `bin/keystrokes-hud/` into its `mods/` folder. Target: `<instance>/mods/keystrokes-hud/keystrokes-hud.dll` and `manifest.json`.
3. Start the modded instance and enter a world. Use W/A/S/D/SPACE. Press F8 to toggle HUD.
4. If the client reports DLL or loader errors, compare the versions of Minecraft / LeviLamina / build package. Do not use a server-side DLL.

## Layout customization
At the top of `src/mod/KeystrokesMod.cpp`, adjust constants `kLeft`, `kTop`, `kUnit`, `kGap`, `kSpaceHeight`. Background/border colors are defined in `renderOverlay`. Recompile after editing.

The code monitors the **literal W/A/S/D/Space keys** rather than resolving user-remapped movement controls. If Minecraft uses another movement binding, this overlay still shows literal keyboard presses.

## How it works
- `KeyInputEvent`: observes key press/release events without cancelling them.
- `AfterUIRenderEvent`: draws five rectangles and uses Minecraft's `drawDebugText` font helper; flushes text once per event.
- No DirectX injection or external overlay program.

## Publishing with lip
Update `id` in `lip.toml` (`YOUR_GITHUB_NAME` -> your GitHub account), push the project, and publish Git tag `v0.1.0` as a GitHub Release. The included workflow builds the Windows client ZIP. The release archive is only for a **client** variant. The initial source ZIP here is not a GitHub release package.

## Limitations / validation
- Prepared from public LeviLamina 26.60 API docs and the official xmake template as of Nov 2026.
- **Not compiled or runtime-tested in this environment**, which lacks the Windows toolchain, game runtime and downloaded LeviLamina SDK. Native Bedrock API compatibility and render ordering must be checked with a real Windows client.
- In some screens or versions `AfterUIRenderEvent` may fire during menu render, so the HUD may appear on menus. This version has no screen-scene filter.

## Upstream docs
- https://levimc.org/tutorials/develop/first-levilamina-mod/
- https://levimc.org/tutorials/develop/publish-a-package/
- https://levimc.org/LeviLamina/api_reference/input/
- https://levimc.org/faq/develop/levilamina/api-ui-network/
