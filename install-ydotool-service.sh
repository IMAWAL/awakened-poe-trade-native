#!/usr/bin/env bash
set -euo pipefail

MODE="${1:---install}"
if [[ "$MODE" != "--install" && "$MODE" != "--print-unit" ]]; then
  echo "usage: $0 [--install|--print-unit]" >&2
  exit 1
fi
TARGET_USER="${SUDO_USER:-$(id -un)}"
TARGET_UID="$(id -u "$TARGET_USER")"
TARGET_GID="$(id -g "$TARGET_USER")"
DAEMON="$(command -v ydotoold || true)"
MODPROBE="$(command -v modprobe || true)"
if [[ -z "$DAEMON" ]]; then
  echo "error: install your distribution's ydotool package (including ydotoold) first." >&2
  exit 1
fi
if [[ "$TARGET_UID" == 0 ]]; then
  echo "error: run this script as your desktop user, with sudo available." >&2
  exit 1
fi
if [[ ! "$DAEMON" =~ ^/[A-Za-z0-9_./+-]+$ || ! "$MODPROBE" =~ ^/[A-Za-z0-9_./+-]+$ ]]; then
  echo "error: ydotoold and modprobe must be installed at standard absolute paths." >&2
  exit 1
fi

print_unit() {
  cat <<UNIT
[Unit]
Description=Awakened PoE Trade input daemon
After=systemd-udevd.service

[Service]
Type=simple
ExecStartPre=$MODPROBE uinput
ExecStart=$DAEMON --socket-path=/run/awakened-poe-trade-ydotoold/socket --socket-own=$TARGET_UID:$TARGET_GID --socket-perm=0600
RuntimeDirectory=awakened-poe-trade-ydotoold
RuntimeDirectoryMode=0755
UMask=0077
Restart=on-failure
RestartSec=2

[Install]
WantedBy=multi-user.target
UNIT
}
if [[ "$MODE" == "--print-unit" ]]; then
  print_unit
  exit 0
fi
if systemctl is-active --quiet ydotool.service || systemctl is-active --quiet ydotoold.service; then
  echo "error: a distribution ydotool service is already active; use its socket instead." >&2
  exit 1
fi

UNIT_FILE="$(mktemp)"
trap 'rm -f -- "$UNIT_FILE"' EXIT
print_unit > "$UNIT_FILE"
echo "Installing a system input daemon. Its socket is accessible only to $TARGET_USER."
SUDO=()
if [[ "$EUID" != 0 ]]; then SUDO=(sudo); fi
"${SUDO[@]}" install -m 0644 "$UNIT_FILE" /etc/systemd/system/awakened-poe-trade-ydotoold.service
"${SUDO[@]}" systemctl daemon-reload
"${SUDO[@]}" systemctl enable --now awakened-poe-trade-ydotoold.service
for attempt in {1..30}; do
  if [[ -S /run/awakened-poe-trade-ydotoold/socket ]]; then
    echo 'Input daemon is ready. Restart Awakened PoE Trade Native.'
    exit 0
  fi
  sleep 0.1
done
"${SUDO[@]}" systemctl status --no-pager awakened-poe-trade-ydotoold.service
exit 1
