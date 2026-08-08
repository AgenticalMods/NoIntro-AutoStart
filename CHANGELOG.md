# Changelog

## v1.1.0 — 2026-08-07

**Compatibility release for ModLoader v1.17.3, and the mod now updates itself.** No change to what it does in game.

v1.17.3 changed the plugin interface version it accepts to **60**, and it accepts that one value only. v1.0.1 reports 52, so v1.17.3 rejects it at load and the auto-start does nothing — the game asks for the spacebar again. This release is the same plugin rebuilt against the current SDK.

A new `AgenticalMods-NoIntroAutoStart.json` ships beside the DLL. The ModLoader reads it on launch — before plugins load — and replaces the DLL when a newer build is published. The manifest it points at declares which loader versions each build works with, so an incompatible build is skipped rather than installed. To opt out, delete the `.json`; the plugin keeps working and stops updating itself.

> ### ⚠ This one is a manual install, and it is the last one
> Two reasons, both one-off:
> - **The plugin was renamed** `AM-NoIntroAutoStart.dll` → `AgenticalMods-NoIntroAutoStart.dll`, matching the rest of the AgenticalMods plugins. **Delete `AM-NoIntroAutoStart.dll` and `config\AM-NoIntroAutoStart.ini`** from `StarRupture\Binaries\Win64\ModLoader\Plugins\` first. Leaving the old DLL there loads both copies, and the old `.ini` is orphaned — the loader pairs config to the DLL name.
> - **Auto-update cannot deliver itself.** It only works once the `.json` is present, so **copy both files** from `install\ModLoader-Plugins\`. Copying only the DLL leaves the mod working but never updating.

### Changed

- Rebuilt against Plugin SDK interface 60 (was 52).
- Plugin renamed to `AgenticalMods-NoIntroAutoStart`; its config file is now `config\AgenticalMods-NoIntroAutoStart.ini`.
- Reported version string is now `1.1.0`.

### Added

- `AgenticalMods-NoIntroAutoStart.json` — the auto-update sidecar.

### Compatibility

- Requires ModLoader **v1.17.3**. The loader pins a single interface version rather than a range, so v1.16.0–v1.17.2 reject this build — update the loader.
- Plugin DLL only. The intro-skip paks are unchanged and do not need reinstalling — they need no mod loader at all.
- Game build target: Early Access `0.2.8.121391-S`.
