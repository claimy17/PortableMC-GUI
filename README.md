# PortableMC Lightweight GUI   <img width="64" height="35" alt="icon" src="https://github.com/user-attachments/assets/5e835cd6-a222-437b-b25e-24a15a216fbd" />

<img width="521" height="493" alt="Screenshot 2026-11-15 165022" src="https://github.com/user-attachments/assets/a045dae6-9c2d-41ff-8e63-30d8022cadd3" />
<img width="521" height="493" alt="Screenshot 2026-11-15 165028" src="https://github.com/user-attachments/assets/fda09a56-ccb1-4993-9689-d5416a77437e" />


A fast, lightweight, native graphical user interface built in pure **C / Objective-C** for **[PortableMC](https://github.com)** across **Windows**, **Linux**, and **macOS**, featuring zero heavy runtime dependencies, native system toolkits (Win32, GTK3, Cocoa), and built-in Modrinth integration.

---

## Features

- **Cross-Platform Native Binaries**:
  - **Windows (`Win32 C`)**: 3D isometric cube startup animation, gradient backgrounds, `PortableMC-Setup.exe` all-in-one installer, and a ~300 KB standalone portable `.exe`.
  - **Linux (`GTK3 + Cairo C`)**: Native executable bundled with `install.sh` and `uninstall.sh` scripts for desktop menu integration.
  - **macOS 11+ (`Cocoa Objective-C`)**: Universal Binary (`Intel x86_64` + `Apple Silicon`) packaged as `PortableMC-GUI.app`.
- **1-Click "Boost FPS" Optimization Pack**: Automatically downloads and installs **Sodium**, **Lithium**, **FerriteCore**, **EntityCulling**, and **Fabric API** compatible with your selected Minecraft version and mod loader.
- **Built-in Modrinth Catalog (`Search Mods`)**: Search and install thousands of mods directly inside the launcher without opening a web browser.
- **Auto-Close Mode (`0 MB RAM` in-game)**: Automatically closes the launcher the moment Minecraft starts, freeing up 100% of background memory for maximum FPS.
- **Isolated Version Instances (`Isolated`)**: Keep mods, worlds, skins, and configurations separated per Minecraft version inside `instances/<version>/` to prevent mod conflicts.
- **Automatic Offline Skin & Cape Setup**: Importing a `.png` skin or cape automatically checks for `CustomSkinLoader` and downloads the right version from Modrinth if missing.
- **Persistent Configuration**: Automatically saves your theme, username, mod loader, game directory, and preferences across sessions.
- **Dual Authentication & Full Version List**: Offline username or Microsoft device-code login (`https://microsoft.com`), supporting Vanilla, Fabric, Forge, NeoForge, and Quilt (`rd-132211` up to `26.3` / `26.4-rc1`).

---

## Installation & Usage

Download packages from the **[Releases](../../releases)** page:

- **Windows**: Run **`PortableMC-Setup.exe`** *(Recommended)* for an all-in-one install, or place **`PortableMC-GUI.exe`** alongside `portablemc.exe`.
- **Linux**: Extract **`PortableMC-GUI-Linux.tar.gz`** and run `./install.sh` to install the app, desktop shortcut, and `portablemc` backend automatically (use `./uninstall.sh` to remove).
- **macOS 11+**: Extract **`PortableMC-GUI-macOS.zip`**, move `PortableMC-GUI.app` to **Applications**, and right-click > **Open** on first launch *(requires `portablemc` CLI via `pip3 install portablemc` or `brew install portablemc`)*.

---

## Building from Source

- **Windows (MinGW-w64):**
  ```bat
  windres app.rc -O coff -o app.res
  gcc launcher.c modrinth.c app.res -o PortableMC-GUI.exe -mwindows -lcomctl32 -lshell32 -lgdi32 -lmsimg32 -ldwmapi -luxtheme -lole32 -Os -s
  ```
- **Linux (GCC + GTK3):**
  ```bash
  gcc launcher_linux.c modrinth.c -o PortableMC-GUI \$(pkg-config --cflags --libs gtk+-3.0) -lm -Os -s
  ```
- **macOS 11+ (Clang + Cocoa Universal):**
  ```bash
  clang -fobjc-arc -arch arm64 -arch x86_64 -mmacosx-version-min=11.0 launcher_mac.m modrinth.c -framework Cocoa -o PortableMC-GUI -Os
  ```

---

## Security Notice

Unsigned open-source binaries may trigger OS warnings:
- **Windows:** Click **More info > Run anyway** in SmartScreen (or install the self-signed certificate included in the setup).
- **macOS:** Right-click **`PortableMC-GUI.app`** and select **Open**.

---

## License

MIT License. `PortableMC` is developed by its respective authors.
