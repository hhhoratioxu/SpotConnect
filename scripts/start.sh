#!/bin/bash
set -euo pipefail
umask 077
hori_home=${HORISPOT_HOME:-"$HOME/Library/Application Support/HoriSpotConnect"}
hori_version=$(cd "$(dirname "$0")/.." && pwd)
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
hori_interface=${HORISPOT_INTERFACE:-}
if [[ -z "$hori_interface" ]]; then
  "$hori_version/bin/hori-discover" --seconds 3 --match HomePod > "$hori_home/state/discovery.json"
  hori_interface=$(plutil -extract devices.0.interface raw -o - "$hori_home/state/discovery.json" 2>/dev/null || true)
fi
if [[ -z "$hori_interface" ]]; then
  hori_interface=$(/usr/sbin/networksetup -listallhardwareports | awk '/Hardware Port: Wi-Fi/{getline; print $2; exit}')
fi
if [[ -z "$hori_interface" ]]; then
  echo "No LAN interface found. Set HORISPOT_INTERFACE to the HomePod's interface." >&2; exit 2
fi
echo "Binding only this bridge to LAN interface: $hori_interface" >&2
exec "$hori_version/bin/HoriSpotConnect" -Z -x "$hori_home/state/config.xml" \
  -b "$hori_interface" -o AudioAccessory -N '%s [HoriSpot]' -r 320 -c alac
