#!/bin/bash
set -e

# Set default installation path if not provided
INSTALL_PATH="${1:-/opt/MovieTag}"

# Ensure the installation directory exists
mkdir -p "$INSTALL_PATH"

# Copy all files and directories except install.sh to the installation directory
echo "Installing MovieTag to $INSTALL_PATH..."
find . -mindepth 1 -not -name 'install.sh' -exec cp -r {} "$INSTALL_PATH/" \;

# Generate the .desktop file in the appropriate directory
echo "Generating .desktop file..."

DESKTOP_FILE="$INSTALL_PATH/MovieTag.desktop"
cat <<EOL > "$DESKTOP_FILE"
#!/usr/bin/env xdg-open
[Desktop Entry]
Version=1.0
Name=MovieTag
Comment=Movie Tagging Application
Path=$INSTALL_PATH
Exec=$INSTALL_PATH/start.sh
Icon=$INSTALL_PATH/icons/MovieTag.png
Terminal=false
Type=Application
Categories=Utility;Application;
EOL

# Make the .desktop file executable
chmod +x "$DESKTOP_FILE"

# Create necessary directories if they don't exist
mkdir -p "$HOME/.local/share/applications"

# Copy the .desktop file to the applications folder
cp "$DESKTOP_FILE" "$HOME/.local/share/applications/"

echo "MovieTag installation completed."
