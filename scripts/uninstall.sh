#!/bin/bash
set -euo pipefail
hori_home=${HORISPOT_HOME:-"$HOME/Library/Application Support/HoriSpotConnect"}
hori_agent_dir=${HORISPOT_AGENT_DIR:-"$HOME/Library/LaunchAgents"}
hori_plist="$hori_agent_dir/com.hhhoratioxu.HoriSpotConnect.plist"
launchctl bootout "gui/$(id -u)" "$hori_plist" 2>/dev/null || true
hori_stamp=$(date +%Y%m%d-%H%M%S)
mkdir -p "$HOME/.Trash"
if [[ -f "$hori_plist" ]]; then mv "$hori_plist" "$HOME/.Trash/HoriSpotConnect-$hori_stamp.plist"; fi
if [[ -d "$hori_home" ]]; then mv "$hori_home" "$HOME/.Trash/HoriSpotConnect-$hori_stamp"; fi
echo "Uninstalled. Application and private state were moved to Trash for recovery."
echo "Project source and other applications were left in place."
