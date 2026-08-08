# Building the plugin from source

The auto-start plugin (`src\`) is a C++ DLL built against the **AlienX StarRupture Plugin SDK**. The intro-skip paks are prebuilt data and are not compiled.

## Prerequisites

- **Visual Studio 2022 or newer** with the **Desktop development with C++** workload (MSVC toolset + Windows 10/11 SDK).
  - The project targets **PlatformToolset `v145`**. If you have a different toolset (e.g. `v143` from VS2022), retarget the project (right-click → *Retarget*) or edit `<PlatformToolset>` in the `.vcxproj`.
- The **StarRupture Plugin SDK**, cloned with its Game-SDK submodule:
  ```
  git clone https://github.com/AlienXAXS/StarRupture-Plugin-SDK
  cd StarRupture-Plugin-SDK
  "Update SDK.bat"          REM fetches the StarRupture Game-SDK submodule (class layouts)
  ```

## Where the SDK must live

The project expects the SDK as a **sibling of this repo**:

```
<parent>\
  NoIntro-AutoStart\          (this repo)
  StarRupture-Plugin-SDK\     (the cloned SDK)
```

If the SDK is elsewhere, pass its path when building:
`msbuild ... /p:SdkDir=<full path to StarRupture-Plugin-SDK>\`

## Build

From a *Developer Command Prompt* (or *Developer PowerShell*):

```
msbuild src\AgenticalMods-NoIntroAutoStart.vcxproj /p:Configuration="Client Release" /p:Platform=x64
```

The DLL is written to `src\bin\Client Release\AgenticalMods-NoIntroAutoStart.dll`. Copy it to the loader's
`...\Binaries\Win64\ModLoader\Plugins\` folder (see [`README.md`](README.md)).

## What the plugin does

On the first `WorldBeginPlay` for `Map_MainMenu`, once the startup widget exists, it invokes
`CommonUserSubsystem.TryToLoginForOnlinePlay(0)` and the widget's `SwitchToFullMenu()` **by name** — advancing
past `PRESS (SPACE) TO START` to the full menu. It fires once, touches nothing else, and stands down silently on any
failure (the game then boots normally). All game calls are by name, so a game patch degrades it to a no-op rather
than a crash.
