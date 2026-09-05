#!/usr/bin/env bash
# ==============================================================================
# Helper script to launch the 5 Tactical Tracks UDP Generator
# ==============================================================================
set -e

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"

# Run with Python 3, forwarding any custom CLI flags (e.g. --interval 0.5 --speed-factor 2.0)
exec python3 "$SCRIPT_DIR/track_sender.py" "$@"
