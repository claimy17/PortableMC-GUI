# PortableMC Lightweight GUI
<img width="441" height="453" alt="preview (2)" src="https://github.com/user-attachments/assets/a74a69bd-420b-46e2-9c45-60d397e7fe4d" /> <img width="441" height="453" alt="preview" src="https://github.com/user-attachments/assets/9cdfaddf-7127-40af-849a-f56b10c497a1" />

A fast, lightweight, native Win32 graphical user interface written in pure C for **[PortableMC](https://github.com)**. Built with zero external runtime dependencies and an Apple-inspired aesthetic featuring vertical gradient backgrounds, rounded controls, custom modal dialogs, and instant Light/Dark theme switching.

---

## Features

- **Automatic CLI Detection**: Automatically detects and uses either `portablemc.exe` (official release name) or `portablemc-cli.exe` located in the same directory—no manual renaming required.
- **Apple-Inspired UI & Themes**:
  - Smooth vertical gradient backgrounds (`GradientFill`), custom owner-drawn rounded buttons, rounded input fields with dynamic placeholders, and custom dropdown menus.
  - Instant switching between **Light Theme** and **Dark Theme** (including native Windows immersive dark title bar and dark scrollbar theming).
- **Live Status & Integrated Progress Bar**:
  - **Status Indicator**: Color-coded status dot next to the title (`Green` = Ready, `Orange` = Running/Downloading, `Red` = Error/Cancelled).
  - **Interactive Progress Bar**: The main **LAUNCH MINECRAFT** button transforms into a real-time green progress bar during downloads (`DOWNLOADING... X% (Cancel)`) and switches to `LAUNCHING... (Cancel)` when starting the game.
- **Safe Cancellation & Custom Modal Dialog**:
  - Click the progress bar at any time to immediately terminate the active process.
  - If cancelled mid-download, a custom rounded Apple-style modal dialog notifies you without risking accidental file deletion and provides a one-click **Open Folder** button to inspect or clean up partial files manually.
- **Dual Authentication Modes**:
  - **Username (Offline)**: Launch directly with a custom offline nickname (`--username`).
  - **Microsoft Account**: Integrated browser/device-code authentication (`auth login`) and authenticated launch (`-a`).
- **Complete Version & Mod Loader Support (`0.0.11a` to `26.4`)**:
  - Supports **Vanilla**, **Fabric**, **Forge**, **NeoForge**, and **Quilt**.
  - Quick toggle filters for **Snapshot** and **Beta / Alpha / Classic** versions.
- **Built-in Mod Importer & Live Console**:
  - One-click `.jar` mod importer (`+ Mod`) that automatically creates and copies mods into your selected game directory's `mods` folder.
  - Silent background execution (`CREATE_NO_WINDOW`) with clean, ANSI-stripped real-time output inside the GUI.

---

## Requirements

- **Operating System**: Windows 10 or Windows 11
- **Backend CLI**: Place either `portablemc.exe` or `portablemc-cli.exe` in the **exact same folder** as `PortableMC-GUI.exe`.
- **Compiler** *(only if building from source)*: MinGW-w64 (`gcc` and `windres`).

---

## Building from Source

1. Place `launcher.c`, `app.rc`, and `icon.ico` in the same directory.
2. Compile the resource file and build `PortableMC-GUI.exe` using GCC:

```bat
windres app.rc -O coff -o app.res
gcc launcher.c app.res -o PortableMC-GUI.exe -mwindows -lcomdlg32 -lole32 -luuid -lshell32 -ldwmapi -lmsimg32 -luxtheme -lcomctl32
```

### Linked Windows Libraries
- `-mwindows`: Compiles as a native Win32 GUI application without spawning an external console window.
- `-lcomdlg32`: Native file picker dialog for importing `.jar` mods.
- `-lole32 -luuid -lshell32`: Folder browser dialog, shell execution, and system path resolution.
- `-ldwmapi -luxtheme`: Native Dark Mode title bar and themed scrollbars.
- `-lmsimg32`: Hardware-accelerated `GradientFill` rendering.
- `-lcomctl32`: Common Controls support for input cue banners (placeholders).

---

## Usage

1. Place `PortableMC-GUI.exe` alongside `portablemc.exe` (or `portablemc-cli.exe`) in the same folder.
2. Open `PortableMC-GUI.exe`.
3. Select **Username** (for offline play) or **Microsoft** (click **Login** first to authenticate your account).
4. Pick your **Mod Loader** (`Vanilla`, `fabric`, `forge`, `neoforge`, `quilt`) and **Version** (toggle **Snapshot** or **Beta** to expand the version list).
5. Verify or change the **Game folder** (defaults to `%AppData%\.minecraft`).
6. Click **LAUNCH MINECRAFT**. To abort at any time, click the progress bar again.

---

## Windows Security / Smart App Control Notice

Because open-source binaries compiled with GCC do not include a commercial code-signing certificate, Windows Defender SmartScreen or Windows 11 Smart App Control may display a warning on first launch:
- Click **More info > Run anyway** if SmartScreen appears, or
- Add the folder to **Windows Security > Virus & threat protection > Exclusions**, or
- Compile the executable directly from `launcher.c` on your own machine.

---

## License

This project is open-source and free to use and modify. `PortableMC` is developed by its respective authors.
