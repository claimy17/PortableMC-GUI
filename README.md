# PortableMC Lightweight GUI   <img width="64" height="35" alt="icon" src="https://github.com/user-attachments/assets/5e835cd6-a222-437b-b25e-24a15a216fbd" />

<img width="521" height="453" alt="Screenshot 2026-11-14 113345" src="https://github.com/user-attachments/assets/dad24598-8f1e-4dbe-801f-d8ed5c964cce" />
<img width="521" height="453" alt="Screenshot 2026-11-14 113352" src="https://github.com/user-attachments/assets/2d5b7978-8003-440a-8e1c-326bc288c0c1" />

A fast, lightweight, native graphical user interface built in pure **C / Objective-C** for **[PortableMC](https://github.com)** across **Windows**, **Linux**, and **macOS**, featuring zero heavy runtime dependencies and native system toolkits (Win32, GTK3, Cocoa).

---

## Features

- **Cross-Platform Native Binaries**:
  - **Windows (`Win32 C`)**: 3D cube startup animation, gradient backgrounds, `PortableMC-Setup` installer, and a ~260 KB standalone portable `.exe`.
  - **Linux (`GTK3 C`)**: Native binary for Ubuntu and Debian desktops with a `.desktop` menu installer.
  - **macOS (`Cocoa Objective-C`)**: Universal Binary (`Intel x86_64` + `Apple Silicon`) packaged as a `.app` bundle.
- **Automatic CLI Detection**: Detects `portablemc` in the same directory or system `PATH`.
- **Live Status & Controls**: Color-coded status indicator, safe download cancellation, and an **Open Folder** modal.
- **Dual Authentication**: Offline username launcher or Microsoft browser device-code login (`auth login --no-browser`).
- **Version & Mod Loader Support**: Supports Vanilla, Fabric, Forge, NeoForge, and Quilt (`0.0.11a` to `26.4`), with Snapshot/Beta/Alpha filters.
- **Built-in Importers**: One-click `.jar` mod, skin (`.png`), and cape importers.

---

## Installation & Usage

Download packages from the **[Releases](../../releases)** page:

- **Windows**: Use **`PortableMC-Setup`** for an all-in-one install or place **`PortableMC-GUI.exe`** alongside `portablemc.exe`.
- **Linux**: Extract the archive and run `./PortableMC-GUI-Linux` (or `./install_linux.sh`) with `portablemc` installed.
- **macOS**: Unzip **`PortableMC-GUI-macOS-Universal.zip`** and right-click > **Open** the `.app` on first launch.

---

## Building from Source

- **Windows (MinGW-w64):**
  ```bat
  windres app.rc -O coff -o app.res
  gcc launcher.c app.res -o PortableMC-GUI.exe -mwindows -lcomdlg32 -lole32 -luuid -lshell32 -ldwmapi -lmsimg32 -luxtheme -lcomctl32
  ```
- **Linux (GCC + GTK3):**
  ```bash
  gcc launcher_linux.c -o PortableMC-GUI-Linux \$(pkg-config --cflags --libs gtk+-3.0)
  ```
- **macOS (Clang + Cocoa):**
  ```bash
  clang -framework Cocoa -fobjc-arc -arch x86_64 -arch arm64 -O2 launcher_mac.m -o PortableMC-GUI
  ```
*(For full source commands, refer to the original repository documentation).*

---

## Security Notice

Unsigned open-source binaries may trigger warnings:
- **Windows:** Click **More info > Run anyway** in SmartScreen.
- **macOS:** Right-click **`PortableMC-GUI.app`** and select **Open**.

---

## License

MIT License. `PortableMC` is developed by its respective authors.
