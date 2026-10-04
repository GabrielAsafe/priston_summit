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
run python3 tools/validate_red_ingress_retired_v2.py
run python3 tools/validate_direct_socket_send_budget_v1.py
run python3 tools/validate_quest_codex_exposure_v1.py
run python3 tools/validate_mixing_kernel_ownership_v1.py
run python3 tools/validate_aging_kernel_ownership_v1.py
run python3 tools/validate_crafting_kernel_ownership_v1.py
run python3 tools/validate_onserver_packet_router_freeze_v1.py
run python3 tools/validate_presentation_renderer_boundary_v1.py
run python3 tools/validate_server_shared_gameplay_manifest.py
run python3 tools/validate_character_repository_boundary_v1.py
run python3 tools/validate_companion_status_cleanup_v2.py
run python3 tools/validate_companion_class_skill_profile_v1.py
run python3 tools/validate_companion_skill_resources_v2.py
run python3 tools/validate_companion_damage_boundary_v1.py
run python3 tools/validate_companion_life_boundary_v1.py

# PATCH 31+ authority-transfer gates. These are static and fast: once an
# authority leaves RED, the preflight must reject any accidental return.
run python3 tools/validate_crafting_inventory_reservation_v2.py
run python3 tools/validate_aging_inventory_reservation_v2.py
run python3 tools/validate_mixing_inventory_reservation_v2.py
run python3 tools/validate_inventory_snapshot_rollback_v2.py
run python3 tools/validate_crafting_transaction_rollback_v2.py
run python3 tools/validate_aging_transaction_rollback_v2.py
run python3 tools/validate_mixing_transaction_rollback_v2.py
run python3 tools/validate_playskillattack_attack_rating_v2.py
run python3 tools/validate_eventskill_attack_rating_v2.py
run python3 tools/validate_playskillattack_animation_authority_v2.py
run python3 tools/validate_eventskill_animation_authority_v2.py
run python3 tools/validate_monster_follower_release_v2.py
run python3 tools/validate_character_poison_life_authority_v2.py
run python3 tools/validate_character_monster_regen_life_authority_v2.py
run python3 tools/validate_character_main_targeting_authority_v2.py
run python3 tools/validate_sendtransattack_attack_rating_v2.py
run python3 tools/validate_character_companion_reflect_life_authority_v2.py
run python3 tools/validate_character_main_packet_sender_v2.py
run python3 tools/validate_character_main_heal_life_authority_v2.py
run python3 tools/validate_sendtransattack_packet_sender_v2.py
run python3 tools/validate_character_socket_retired_v2.py
run python3 tools/validate_news_packet_sender_v2.py
run python3 tools/validate_character_mingw_compile_fix_v2.py
run python3 tools/validate_skill_info_packet_sender_v2.py
run python3 tools/validate_event_spawn_packet_sender_v2.py
run python3 tools/validate_event_game_packet_sender_v2.py
run python3 tools/validate_dllserver_packet_sender_v2.py
run python3 tools/validate_integrity_packet_sender_v2.py
run python3 tools/validate_character_record_player_packet_sender_v2.py
run python3 tools/validate_server_damage_player_packet_sender_v2.py
run python3 tools/validate_server_damage_socket_retired_v2.py
run python3 tools/validate_character_main_simple_lethal_v2.py

# PATCH 77+ transport ownership gates. RED/legacy feature owners may consume
# transport services, but direct socket ownership must remain in AMBER adapters.
run python3 tools/validate_pet_runtime_packet_sender_v2.py
run python3 tools/validate_chat_delivery_packet_sender_v2.py
run python3 tools/validate_backend_channel_sender_v2.py
run python3 tools/validate_login_backend_channel_sender_v2.py
run python3 tools/validate_anticheat_backend_channel_sender_v2.py
run python3 tools/validate_premium_backend_channel_sender_v2.py
run python3 tools/validate_ranking_backend_channel_sender_v2.py
run python3 tools/validate_pvp_score_backend_channel_sender_v2.py
run python3 tools/validate_battle_pass_backend_channel_sender_v2.py
run python3 tools/validate_class_rank_backend_channel_sender_v2.py
run python3 tools/validate_message_box_backend_channel_sender_v2.py
run python3 tools/validate_inventory_backend_channel_sender_v2.py
run python3 tools/validate_server_inventory_backend_channel_sender_v2.py
run python3 tools/validate_coin_shop_client_backend_channel_sender_v2.py
run python3 tools/validate_damage_data_backend_channel_sender_v2.py
run python3 tools/validate_shop_backend_channel_sender_v2.py
run python3 tools/validate_character_data_backend_channel_sender_v2.py
run python3 tools/validate_backend_channel_primary_alias_v2.py
run python3 tools/validate_damage_user_extend_backend_channel_sender_v2.py
run python3 tools/validate_cracker_backend_channel_sender_v2.py
run python3 tools/validate_playsub_backend_channel_sender_v2.py
build_scripts=(
    build_client_mingw_i686_multicore.sh
    build_server_mingw_i686_j8.sh
    link_client_mingw_i686.sh
    link_server_mingw_i686.sh
)
missing_build_scripts=()
for script in "${build_scripts[@]}"; do
    [[ -f "$script" ]] || missing_build_scripts+=("$script")
done
if ((${#missing_build_scripts[@]} == 0)); then
    run bash -n "${build_scripts[@]}"
    if [[ -f tools/verify_mingw_build_fix_v2.sh ]]; then
        run bash tools/verify_mingw_build_fix_v2.sh
    fi
elif [[ "$MODE" == "native" ]]; then
    printf 'NATIVE_BUILD_BLOCKED: source archive is missing build scripts:' >&2
    printf ' %s' "${missing_build_scripts[@]}" >&2
    printf '\n' >&2
    exit 2
else
    printf 'PORTABLE_NOTE: source archive omits native build scripts:'
    printf ' %s' "${missing_build_scripts[@]}"
    printf '\n'
fi

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
    # Focused V2 command/kernel smokes introduced by the PATCH 31+ series.
    run bash Shared/gameplay/inventory/v2/tests/run_inventory_snapshot_rollback_tests.sh
    run bash Shared/gameplay/combat/v2/tests/run_combat_buff_service_tests.sh
    run bash Shared/gameplay/combat/v2/tests/run_life_commands_tests.sh
    run bash Shared/gameplay/skills/v2/tests/run_skill_animation_commands_tests.sh
    run bash Shared/gameplay/monsters/v2/tests/run_monster_commands_tests.sh
    run bash Shared/network/v2/tests/run_backend_channel_sender_tests.sh
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
