#!/usr/bin/env bash
set -euo pipefail

ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
MODE="static"
case "${1:-}" in
    "") ;;
    --portable) MODE="portable" ;;
    --native-build) MODE="native" ;;
    -h|--help)
        cat <<'EOF'
Usage: tools/run_frozen_legacy_preflight.sh [--portable|--native-build]

  default          Fast architecture/build-manifest preflight.
  --portable       Also run portable domain/UI/integration suites.
  --native-build   Run portable gates, then full MinGW i686 client/server compile+link.

Native build does not replace in-game runtime smoke/soak certification.
EOF
        exit 0
        ;;
    *) echo "Unknown option: $1" >&2; exit 2 ;;
esac

run() {
    echo
    echo ">>> $*"
    "$@"
}

cd "$ROOT"
echo "Frozen Legacy preflight mode: $MODE"

# Definition-of-Frozen-Legacy source/architecture gates.
run python3 tools/architecture/validate_zero_legacy_debt.py
run python3 tools/validate_green_transport_boundary_v1.py
run python3 tools/validate_client_gameplay_socket_freeze_v1.py
run python3 tools/validate_red_feature_ingress_freeze_v1.py
run python3 tools/validate_direct_socket_send_budget_v1.py
run python3 tools/validate_onserver_packet_router_freeze_v1.py
run python3 tools/validate_presentation_renderer_boundary_v1.py
run python3 tools/validate_server_shared_gameplay_manifest.py
run python3 tools/validate_character_repository_boundary_v1.py
run python3 tools/validate_companion_status_cleanup_v2.py
run python3 tools/validate_companion_class_skill_profile_v1.py
run bash -n build_client_mingw_i686_multicore.sh build_server_mingw_i686_j8.sh \
    link_client_mingw_i686.sh link_server_mingw_i686.sh
run bash tools/verify_mingw_build_fix_v2.sh

if [[ "$MODE" == "native" ]]; then
    required=(i686-w64-mingw32-g++ i686-w64-mingw32-windres)
    missing=()
    for tool in "${required[@]}"; do
        command -v "$tool" >/dev/null 2>&1 || missing+=("$tool")
    done
    if ((${#missing[@]})); then
        printf 'NATIVE_BUILD_BLOCKED: missing toolchain:' >&2
        printf ' %s' "${missing[@]}" >&2
        printf '\n' >&2
        exit 2
    fi
fi

if [[ "$MODE" == "portable" || "$MODE" == "native" ]]; then
    run bash tools/run_asset_load_v2_portable_tests.sh
    run bash tools/run_companion_v1_1_portable_tests.sh
    run bash tools/run_covenant_ui_tests.sh
    run bash tools/run_covenant_v0_1_portable_tests.sh
    run bash tools/run_covenant_v1_complete_tests.sh
    run bash tools/run_gathering_mining_v1_tests.sh
    run bash tools/run_takeoff_final_validation.sh
fi

if [[ "$MODE" == "native" ]]; then
    run bash build_client_mingw_i686_multicore.sh
    run bash link_client_mingw_i686.sh
    run bash build_server_mingw_i686_j8.sh
    run bash link_server_mingw_i686.sh
    echo
    echo "NATIVE_BUILD: PASS"
    echo "RUNTIME_CERTIFICATION: still requires in-game smoke/soak."
else
    echo
    echo "FROZEN_LEGACY_${MODE^^}_PREFLIGHT: PASS"
    echo "NATIVE_BUILD/RUNTIME_CERTIFICATION: not claimed by this mode."
fi
