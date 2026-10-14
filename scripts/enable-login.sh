#!/bin/bash
set -euo pipefail
umask 077
hori_home=${HORISPOT_HOME:-"$HOME/Library/Application Support/HoriSpotConnect"}
hori_agent_dir=${HORISPOT_AGENT_DIR:-"$HOME/Library/LaunchAgents"}
hori_plist="$hori_agent_dir/com.hhhoratioxu.HoriSpotConnect.plist"
mkdir -p "$hori_agent_dir" "$hori_home/logs"
escape_xml() { printf '%s' "$1" | sed 's/\&/\&amp;/g; s/</\&lt;/g; s/>/\&gt;/g; s/"/\&quot;/g'; }
cat > "$hori_plist" <<XML
<?xml version="1.0" encoding="UTF-8"?>
<!DOCTYPE plist PUBLIC "-//Apple//DTD PLIST 1.0//EN" "http://www.apple.com/DTDs/PropertyList-1.0.dtd">
<plist version="1.0"><dict>
<key>Label</key><string>com.hhhoratioxu.HoriSpotConnect</string>
<key>ProgramArguments</key><array><string>$(escape_xml "$hori_home/current/scripts/start.sh")</string></array>
<key>EnvironmentVariables</key><dict><key>HORISPOT_HOME</key><string>$(escape_xml "$hori_home")</string></dict>
<key>RunAtLoad</key><true/>
<key>KeepAlive</key><dict><key>Crashed</key><true/></dict>
<key>ThrottleInterval</key><integer>30</integer>
<key>ProcessType</key><string>Background</string>
<key>StandardOutPath</key><string>$(escape_xml "$hori_home/logs/stdout.log")</string>
<key>StandardErrorPath</key><string>$(escape_xml "$hori_home/logs/stderr.log")</string>
</dict></plist>
XML
chmod 600 "$hori_plist"
plutil -lint "$hori_plist"
launchctl bootout "gui/$(id -u)" "$hori_plist" 2>/dev/null || true
launchctl bootstrap "gui/$(id -u)" "$hori_plist"
echo "Login LaunchAgent enabled: $hori_plist"
