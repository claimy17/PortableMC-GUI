#!/bin/bash
set -e

APP_DIR="$HOME/.local/share/PortableMC"
DESKTOP_FILE="$HOME/.local/share/applications/portablemc-gui.desktop"

echo "Installing PortableMC-GUI..."
mkdir -p "$APP_DIR" "$HOME/.local/applications"

cp PortableMC-GUI-Linux "$APP_DIR/PortableMC-GUI"
chmod +x "$APP_DIR/PortableMC-GUI"

if [ -f "icon.png" ]; then
    cp icon.png "$APP_DIR/icon.png"
fi

cat > "$DESKTOP_FILE" <<EOF
[Desktop Entry]
Type=Application
Name=PortableMC
Comment=Lightweight Native C Launcher for PortableMC
Exec=$APP_DIR/PortableMC-GUI
Path=$APP_DIR
Icon=$APP_DIR/icon.png
Terminal=false
Categories=Game;
EOF

chmod +x "$DESKTOP_FILE"
echo "Installation done! You can find PortableMC-GUI on applications menù."
