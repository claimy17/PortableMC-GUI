# PortableMC Lightweight GUI   <img width="64" height="35" alt="icon" src="https://github.com/user-attachments/assets/5e835cd6-a222-437b-b25e-24a15a216fbd" />

<img width="521" height="453" alt="Screenshot 2026-11-14 113345" src="https://github.com/user-attachments/assets/dad24598-8f1e-4dbe-801f-d8ed5c964cce" />
<img width="521" height="453" alt="Screenshot 2026-11-14 113352" src="https://github.com/user-attachments/assets/2d5b7978-8003-440a-8e1c-326bc288c0c1" />

A fast, lightweight, native Win32 graphical user interface written in pure C for **[PortableMC](https://github.com)**. Built with zero external runtime dependencies and an Apple-inspired aesthetic featuring vertical gradient backgrounds, a 3D startup animation, custom modal dialogs, and instant Light/Dark theme switching.

---

## Features

- **All-in-One Installer (`PortableMC-Setup`) or Portable Mode**:
  - Choose between the single-click native C installer (which bundles both `PortableMC-GUI.exe` and `portablemc.exe` and creates Desktop & Start Menu shortcuts) or the standalone ~260 KB `PortableMC-GUI.exe`.
- **Automatic CLI Detection**: Automatically detects and uses either `portablemc.exe` (official release name) or `portablemc-cli.exe` located in the same directory—no manual renaming required.
- **Apple-Inspired UI, 3D Intro & Themes**:
  - Smooth 3D isometric cube assembly animation on startup, vertical gradient backgrounds (`GradientFill`), custom rounded controls, and custom modal dialogs.
  - Instant switching between **Light Theme** and **Dark Theme** (including native Windows immersive dark title bar and dark scrollbar theming).
- **Live Status & Integrated Progress Bar**:
  - **Status Indicator**: Color-coded status dot next to the title (`Green` = Ready, `Orange` = Running/Downloading, `Red` = Error/Cancelled).
  - **Interactive Progress Bar**: The main **LAUNCH MINECRAFT** button transforms into a real-time green progress bar during downloads (`DOWNLOADING... X% (Cancel)`) and switches to `LAUNCHING... (Cancel)` when starting the game.
- **Safe Cancellation & Custom Modal Dialog**:
  - Click the progress bar at any time to immediately terminate the active process.
  - If cancelled mid-download, a custom rounded Apple-style modal dialog notifies you without risking accidental file deletion and provides a one-click **Open Folder** button to inspect or clean up partial files manually.
- **Dual Authentication Modes**:
  - **Username (Offline)**: Launch directly with a custom offline nickname (`--username`).
  - **Microsoft Account**: One-click browser device-code authentication (`auth login --no-browser` with automatic redirect to `://microsoft.com`) and authenticated launch (`-a`).
- **Complete Version & Mod Loader Support (`0.0.11a` to `26.4`)**:
  - Supports **Vanilla**, **Fabric**, **Forge**, **NeoForge**, and **Quilt**.
  - Quick toggle filters for **Snapshot** and **Beta / Alpha / Classic** versions.
- **Built-in Mod, Skin & Cape Importers**:
  - **`+ Mod`**: One-click `.jar` mod importer that copies mods directly into your selected game directory's `mods` folder.
  - **`+ Skin` & `+ Cape`**: In **Microsoft** mode, opens your official Minecraft profile page to manage skins and capes. In **Offline** mode (on Fabric, Forge, NeoForge, or Quilt), imports `.png` textures directly into `CustomSkinLoader\LocalSkin` for your username.

---

## Installation & Usage

Go to the **[Releases](../../releases)** page and choose one of the two options:

### Option 1: All-in-One Installer *(Recommended)*
1. Download **`PortableMC-Setup`** (`PortableMC-Setup-v1.2.0.exe`).
2. Run the setup and click **Install PortableMC** (it automatically installs both `PortableMC-GUI.exe` and `portablemc.exe` into `%LocalAppData%\Programs\PortableMC` and creates **Desktop** and **Start Menu** shortcuts).
3. Click **Launch PortableMC** (or open it from your Desktop / Start Menu).
*(To uninstall later, run the setup again and click **Uninstall**).*

### Option 2: Portable / Standalone
1. Download **`PortableMC-GUI.exe`** from **[Releases](../../releases)**.
2. Place `PortableMC-GUI.exe` in the **exact same folder** as `portablemc.exe` (or `portablemc-cli.exe`).
3. Double-click `PortableMC-GUI.exe` to start.

---

## Building from Source

- **Requirements**: Windows 10 or Windows 11, and MinGW-w64 (`gcc` and `windres`).

### 1. Build the Launcher (`PortableMC-GUI.exe`)
```bat
windres app.rc -O coff -o app.res
gcc launcher.c app.res -o PortableMC-GUI.exe -mwindows -lcomdlg32 -lole32 -luuid -lshell32 -ldwmapi -lmsimg32 -luxtheme -lcomctl32
```

### 2. Build the All-in-One Setup (`PortableMC-Setup.exe`)
Place `setup_pmc.c`, `PortableMC-GUI.exe`, `portablemc.exe` (or `portablemc-cli.exe`), and `icon.ico` in the same folder, then run `build_setup.bat` (or compile manually via `windres` and `gcc`).

---

## Windows Security / Smart App Control Notice

Because open-source binaries compiled with GCC do not include a commercial code-signing certificate, Windows Defender SmartScreen or Windows 11 Smart App Control may display a warning on first launch:
- Click **More info > Run anyway** if SmartScreen appears, or
- Compile the executables directly from `launcher.c` and `setup_pmc.c` on your own machine.

---

## License

This project is open-source under the MIT License. `PortableMC` is developed by its respective authors.

