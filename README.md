# NoIntro-AutoStart for StarRupture

Skips the intro screens and the **PRESS (SPACE) TO START** requirement during game start. A mod for the PC game StarRupture.

It has two parts:

- **Intro skip (paks)** — removes the startup logo screens (Creepy Jar, Unreal Engine, StarRupture) so you reach the start screen sooner.
- **Auto-start (plugin)** — on reaching the main menu, advances **past** `PRESS (SPACE) TO START` straight to the full menu, logged in, with no keypress.

Built for the July 2026 Early Access build `0.2.8.121391-S`; a later game update may require an update.

## Requirements

Requires the **AlienX StarRupture ModLoader**, **v1.17.3** (a `dwmapi.dll` proxy loader): https://github.com/AlienXAXS/StarRupture-ModLoader

That version floor applies to the **plugin only**. The loader accepts plugins by interface version, and v1.17.3 accepts **exactly one: 60**. This plugin reports 60, so **every loader older than v1.17.3 refuses to load it** and shows *"Update The Mod Loader"* — update the loader to use this build. Because the loader pins a single interface version rather than a range, a newer loader may in turn stop accepting this build; if that happens, watch for a new release here.

The **intro-skip paks** need no mod loader at all — they contain no code and work on their own, on any version.

## Install

**1. Intro-skip paks** → the game's `Content\Paks\` folder:

1. In Steam, right-click StarRupture → **Manage → Browse local files**, then open `StarRupture\Content\Paks\`.
2. Copy the three `pakchunk260722-Windows_P` files (`.pak`, `.utoc`, `.ucas`) from `install\Content-Paks\` into that folder.

**2. Auto-start plugin** → the mod loader's plugins folder:

3. **Upgrading from v1.0.1 or earlier?** The plugin was renamed. In `StarRupture\Binaries\Win64\ModLoader\Plugins\`,
   delete the old `AM-NoIntroAutoStart.dll` **and** `config\AM-NoIntroAutoStart.ini`. If you leave the old DLL in
   place, both copies load and the old config is ignored.
4. With the ModLoader installed, copy **both** files from `install\ModLoader-Plugins\` into
   `StarRupture\Binaries\Win64\ModLoader\Plugins\`:
   - `AgenticalMods-NoIntroAutoStart.dll`
   - `AgenticalMods-NoIntroAutoStart.json`
5. Launch the game once. The loader generates `...\ModLoader\Plugins\config\AgenticalMods-NoIntroAutoStart.ini`. Open it and ensure:
   ```
   [General]
   Enabled=1
   ```
6. Launch again — the game now boots straight to the menu.

> [!IMPORTANT]
> **Copy the `.json` too.** It is a three-line file that tells the ModLoader where to check for new versions of this
> plugin. With it in place, future updates install themselves — see [Automatic updates](#automatic-updates) below.
> The mod works fine without it; you just have to update by hand every time, which on this loader means the plugin
> silently stops working until you notice.

## Automatic updates

`AgenticalMods-NoIntroAutoStart.json` sits beside the DLL and points at a small manifest published with each release. On launch — before plugins are loaded — the ModLoader reads it, checks whether the published version differs from the one you have, and replaces the DLL if so. Nothing is installed silently from anywhere else: the file names one URL, under this repository, and that is the only place it looks.

This matters more than it sounds. The loader accepts exactly one plugin interface version at a time, so when the ModLoader updates itself, every plugin built for the previous interface stops loading — with no error a player would notice, just a game that goes back to asking for the spacebar. With the `.json` in place, the fixed build arrives on its own.

The manifest also declares which loader versions a build is compatible with, so a build that would not load on your loader is skipped rather than installed.

To turn it off, delete `AgenticalMods-NoIntroAutoStart.json`. The plugin keeps working and simply stops updating itself.

The **paks are not covered by this** — they are data files the loader does not manage. They only need replacing if a game update breaks them.

## Uninstall

Delete the three `pakchunk260722-Windows_P` files from `Content\Paks\`, and delete `AgenticalMods-NoIntroAutoStart.dll` and `AgenticalMods-NoIntroAutoStart.json` from the loader's `Plugins\` folder. To keep the DLL but turn it off, set `Enabled=0` in its `.ini`.

## Build from source

The plugin source is in `src\`. See [`BUILD.md`](BUILD.md).

## License & copyright

AgenticalMods' modifications are licensed under the **MIT License** (see [`LICENSE`](LICENSE)). Unofficial, non-commercial fan mod — not affiliated with or endorsed by Creepy Jar. "StarRupture" and its assets are copyright Creepy Jar; all rights in the underlying game content remain those of Creepy Jar. Any Creepy Jar assets included appear only in modified form and are not licensed by AgenticalMods.

## Thanks

AlienX ([AlienXAXS](https://github.com/AlienXAXS)) — for the StarRupture Plugin SDK. Its game-SDK class definitions and mapping file (`.usmap`) made it possible to identify and edit the game asset this mod changes. The plugin auto-update mechanism is his too — it is built into the ModLoader.
