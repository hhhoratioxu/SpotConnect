#!/bin/bash
set -euo pipefail
umask 077
hori_package=$(cd "$(dirname "$0")/.." && pwd)
if [[ ! -x "$hori_package/bin/HoriSpotConnect" ]]; then
  "$hori_package/scripts/build-macos.sh"
  "$hori_package/scripts/package-macos.sh"
  exec "$hori_package/dist/HoriSpotConnect-0.1.0-preview.2-macos-arm64/scripts/install.sh" "$@"
fi
if [[ $(uname -s) != Darwin || $(uname -m) != arm64 ]]; then echo "macOS ARM64 required" >&2; exit 2; fi
hori_agent=true
if [[ ${1:-} == --no-launchagent ]]; then hori_agent=false
elif [[ $# != 0 ]]; then echo "usage: install.sh [--no-launchagent]" >&2; exit 2; fi
(cd "$hori_package" && shasum -a 256 -c SHA256SUMS)
for hori_binary in HoriSpotConnect hori-discover hori-audio-probe hori-raop-probe; do
  codesign --verify --strict "$hori_package/bin/$hori_binary"
done
hori_home=${HORISPOT_HOME:-"$HOME/Library/Application Support/HoriSpotConnect"}
hori_dest="$hori_home/versions/0.1.0-preview.2"
# Stop only this owned installation before replacing an executable that may run.
hori_existing_agent="${HORISPOT_AGENT_DIR:-$HOME/Library/LaunchAgents}/com.hhhoratioxu.HoriSpotConnect.plist"
if [[ -f "$hori_existing_agent" ]]; then
  launchctl bootout "gui/$(id -u)" "$hori_existing_agent" 2>/dev/null || true
fi
if [[ -f "$hori_home/state/bridge.pid" ]]; then
  IFS= read -r hori_old_pid < "$hori_home/state/bridge.pid" || true
  if [[ ${hori_old_pid:-} =~ ^[0-9]+$ ]]; then
    hori_old_command=$(ps -p "$hori_old_pid" -o comm= 2>/dev/null || true)
    if [[ "$hori_old_command" == "$hori_home/current/bin/HoriSpotConnect" || "$hori_old_command" == "$hori_home"/versions/*/bin/HoriSpotConnect ]]; then
      kill -TERM "$hori_old_pid" 2>/dev/null || true
      for hori_attempt in {1..50}; do
        if ! kill -0 "$hori_old_pid" 2>/dev/null; then break; fi
        sleep 0.1
      done
      if kill -0 "$hori_old_pid" 2>/dev/null; then
        echo "Previous HoriSpotConnect is still stopping; retry after it exits." >&2; exit 2
      fi
    fi
  fi
fi
mkdir -p "$hori_dest" "$hori_home/state" "$hori_home/logs"
chmod 700 "$hori_home" "$hori_home/state" "$hori_home/logs"
for hori_item in bin scripts config licenses docs README.md CHANGELOG.md LICENSE SOURCE-MANIFEST.json SHA256SUMS; do
  cp -R "$hori_package/$hori_item" "$hori_dest/"
done
ln -sfn "$hori_dest" "$hori_home/current"
if [[ ! -f "$hori_home/state/config.xml" ]]; then
  cp "$hori_package/config/config.example.xml" "$hori_home/state/config.xml"
fi
if [[ ! -f "$hori_home/state/quality.mode" ]]; then printf 'AUTO\n' > "$hori_home/state/quality.mode"; fi
chmod 600 "$hori_home/state/config.xml" "$hori_home/state/quality.mode"
cat > "$hori_home/start.command" <<'SH'
#!/bin/bash
hori_launcher_home=$(cd "$(dirname "$0")" && pwd)
export HORISPOT_HOME="$hori_launcher_home"
exec "$hori_launcher_home/current/scripts/start.sh" "$@"
SH
cat > "$hori_home/uninstall.command" <<'SH'
#!/bin/bash
hori_launcher_home=$(cd "$(dirname "$0")" && pwd)
export HORISPOT_HOME="$hori_launcher_home"
exec "$hori_launcher_home/current/scripts/uninstall.sh" "$@"
SH
chmod +x "$hori_home/start.command" "$hori_home/uninstall.command"
if $hori_agent; then "$hori_dest/scripts/enable-login.sh"; fi
echo "Installed Preview at: $hori_home"
echo "Private Spotify Client ID / Secret, if available, belong only in state/config.xml."
echo "Start: $hori_home/start.command"
echo "Uninstall: $hori_home/uninstall.command"
