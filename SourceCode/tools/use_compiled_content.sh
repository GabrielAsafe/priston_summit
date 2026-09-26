#!/usr/bin/env bash
# Source this file from SourceCode/: source tools/use_compiled_content.sh client|server [locale]
if [[ "${BASH_SOURCE[0]}" == "$0" ]]; then
  echo "Use: source tools/use_compiled_content.sh client|server [locale]" >&2
  exit 2
fi
_profile="${1:-server}"
case "$_profile" in
  client|server) ;;
  *) echo "invalid profile: $_profile (expected client or server)" >&2; return 2 ;;
esac
_root="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
_packs="$_root/Content/packs"
export SUMMIT_TEXT_RESOURCE_PACK="$_packs/text_resources.sres"
export SUMMIT_DEFINITION_PACK="$_packs/definitions.scfg"
export SUMMIT_LOCALIZATION_PACK="$_packs/localization.sloc"
export SUMMIT_GAMEPLAY_CONFIG_PACK="$_packs/gameplay_config.sgcf"
export SUMMIT_STAGE_SPAWN_PACK="$_packs/stage_spawns.sspb"
export SUMMIT_MAP_PACK="$_packs/maps_${_profile}.smap"
export SUMMIT_TEXT_RESOURCE_STRICT="${SUMMIT_CONTENT_STRICT:-1}"
export SUMMIT_DEFINITION_STRICT="${SUMMIT_CONTENT_STRICT:-1}"
export SUMMIT_LOCALIZATION_STRICT="${SUMMIT_CONTENT_STRICT:-1}"
export SUMMIT_GAMEPLAY_CONFIG_STRICT="${SUMMIT_CONTENT_STRICT:-1}"
export SUMMIT_STAGE_SPAWN_STRICT="${SUMMIT_CONTENT_STRICT:-1}"
export SUMMIT_MAP_STRICT="${SUMMIT_CONTENT_STRICT:-1}"
if [[ -n "${2:-}" ]]; then export SUMMIT_LOCALE="$2"; fi
printf 'Summit compiled content: profile=%s strict=%s packs=%s\n' "$_profile" "${SUMMIT_CONTENT_STRICT:-1}" "$_packs"
unset _profile _root _packs
