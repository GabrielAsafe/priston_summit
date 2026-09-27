#!/usr/bin/env bash
# Compatibility helper only. Compiled content activation is built into the
# client/server binaries; no SUMMIT_* environment variables are required.
echo "Summit compiled content: activation is built into the client/server binaries; no environment variables required."
if [[ "${BASH_SOURCE[0]}" == "$0" ]]; then
  exit 0
fi
return 0
