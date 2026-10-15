#!/bin/bash
set -euo pipefail
umask 077
hori_home=${HORISPOT_HOME:-"$HOME/Library/Application Support/HoriSpotConnect"}
hori_version=$(cd "$(dirname "$0")/.." && pwd -P)
hori_mode=${1:-}
if [[ -z "$hori_mode" ]]; then
  if [[ -f "$hori_home/state/quality.mode" ]]; then
    IFS= read -r hori_mode < "$hori_home/state/quality.mode" || true
  else hori_mode=AUTO; fi
fi
case "$hori_mode" in AUTO|LOSSLESS|VORBIS_320) ;; *) echo "quality must be AUTO, LOSSLESS or VORBIS_320" >&2; exit 2;; esac
export HORISPOT_QUALITY="$hori_mode"
if [[ ! -f "$hori_home/state/config.xml" ]]; then echo "Run install.sh first." >&2; exit 2; fi
echo "HoriSpotConnect Preview | quality=$hori_mode | Spotify FLAC access=UNAVAILABLE" >&2
echo "Connect device suffix: [HoriSpot]; HomePod delivery and bit-perfect require acceptance." >&2
if [[ "$hori_mode" == LOSSLESS ]]; then
  exec "$hori_version/bin/HoriSpotConnect" -Z -x "$hori_home/state/config.xml"
fi
hori_interface=${HORISPOT_INTERFACE:-}
if [[ -z "$hori_interface" && -f "$hori_home/state/interface.request" ]]; then
  IFS= read -r hori_interface < "$hori_home/state/interface.request" || true
  rm -- "$hori_home/state/interface.request"
fi
if [[ -z "$hori_interface" ]]; then
  "$hori_version/bin/hori-discover" --seconds 3 --match HomePod > "$hori_home/state/discovery.json"
  hori_interface=$(plutil -extract devices.0.interface raw -o - "$hori_home/state/discovery.json" 2>/dev/null || true)
  if [[ -z "$hori_interface" ]]; then
    echo "HOMEPOD_NOT_DISCOVERED: waiting on the current LAN; Spotify/HomePod playback is unverified." >&2
  fi
fi
if [[ -z "$hori_interface" ]]; then
  hori_interface=$(/usr/sbin/networksetup -listallhardwareports | awk '/Hardware Port: Wi-Fi/{getline; print $2; exit}')
fi
hori_ipv4=$(/usr/sbin/ipconfig getifaddr "$hori_interface" 2>/dev/null || true)
if [[ -z "$hori_interface" || -z "$hori_ipv4" ]]; then
  echo "HORI_LAN_UNAVAILABLE: selected interface has no IPv4 address; retry when the HomePod LAN is connected." >&2; exit 75
fi
hori_pid_file="$hori_home/state/bridge.pid"
hori_restart_agent=false
if [[ -f "$hori_pid_file" ]]; then
  IFS= read -r hori_pid < "$hori_pid_file" || true
  if [[ ${hori_pid:-} =~ ^[0-9]+$ ]]; then
    hori_command=$(ps -p "$hori_pid" -o comm= 2>/dev/null || true)
    if [[ "$hori_command" == "$hori_version/bin/HoriSpotConnect" || "$hori_command" == "$hori_home/current/bin/HoriSpotConnect" || "$hori_command" == "$hori_home"/versions/*/bin/HoriSpotConnect ]]; then
      hori_running_mode=$(cat "$hori_home/state/running.mode" 2>/dev/null || true)
      hori_running_interface=$(cat "$hori_home/state/running.interface" 2>/dev/null || true)
      hori_running_ipv4=$(cat "$hori_home/state/running.ipv4" 2>/dev/null || true)
      if [[ "$hori_running_mode" == "$hori_mode" && "$hori_running_interface" == "$hori_interface" && "$hori_running_ipv4" == "$hori_ipv4" ]]; then
        echo "HoriSpotConnect is already running (PID $hori_pid, $hori_mode, $hori_interface). HomePod discovery/playback still require separate verification." >&2; exit 0
      fi
      # Mode or LAN change: stop only the executable belonging to this installation.
      hori_agent_pid=$(launchctl print "gui/$(id -u)/com.hhhoratioxu.HoriSpotConnect" 2>/dev/null | awk '/^[[:space:]]*pid =/{print $3; exit}' || true)
      if [[ "$hori_agent_pid" == "$hori_pid" ]]; then hori_restart_agent=true; fi
      kill -TERM "$hori_pid"
      for hori_attempt in {1..30}; do
        if ! kill -0 "$hori_pid" 2>/dev/null; then break; fi
        sleep 0.1
      done
      if kill -0 "$hori_pid" 2>/dev/null; then
        echo "Previous bridge is still stopping; retry after it exits." >&2; exit 2
      fi
    fi
  fi
fi
printf '%s\n' "$hori_mode" > "$hori_home/state/quality.mode"
printf '%s\n' "$hori_mode" > "$hori_home/state/running.mode"
printf '%s\n' "$hori_interface" > "$hori_home/state/running.interface"
printf '%s\n' "$hori_ipv4" > "$hori_home/state/running.ipv4"
if $hori_restart_agent; then
  # Pass this invocation's selected interface to the next managed process only.
  # A later network failure goes back to automatic Bonjour/Wi-Fi discovery.
  printf '%s\n' "$hori_interface" > "$hori_home/state/interface.request"
  launchctl kickstart "gui/$(id -u)/com.hhhoratioxu.HoriSpotConnect"
  echo "Restart requested through the existing user LaunchAgent ($hori_mode)." >&2
  exit 0
fi
echo "Binding only this bridge to LAN interface: $hori_interface" >&2
exec "$hori_version/bin/HoriSpotConnect" -Z -x "$hori_home/state/config.xml" \
  -b "$hori_interface" -p "$hori_pid_file" -o AudioAccessory -N '%s [HoriSpot]' -r 320 -c alac
