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

# Native certification must fail fast before the large static/portable suite.
# The dedicated checker covers every command and source/dependency root required
# by the compile/link scripts, including GNU parallel.
if [[ "$MODE" == "native" ]]; then
    run bash tools/check_native_build_prerequisites_v1.sh
fi

# Definition-of-Frozen-Legacy source/architecture gates.
run python3 tools/architecture/validate_zero_legacy_debt.py
run python3 tools/validate_green_transport_boundary_v1.py
run python3 tools/validate_client_gameplay_socket_freeze_v1.py
run python3 tools/validate_red_feature_ingress_freeze_v1.py
run python3 tools/validate_red_ingress_retired_v2.py
run python3 tools/validate_direct_socket_send_budget_v1.py
run python3 tools/validate_red_direct_socket_ownership_v3.py
run python3 tools/validate_hunter_profession_activation_v1.py
run python3 tools/validate_player_experience_ownership_v1.py
run python3 tools/validate_profession_inventory_reward_authority_v2.py
run python3 tools/validate_quest_codex_exposure_v1.py
run python3 tools/validate_quest_closure_readiness_v1.py
run python3 tools/validate_mixing_kernel_ownership_v1.py
run python3 tools/validate_aging_kernel_ownership_v1.py
run python3 tools/validate_crafting_kernel_ownership_v1.py
run python3 tools/validate_onserver_packet_router_freeze_v1.py
run python3 tools/validate_presentation_renderer_boundary_v1.py
run python3 tools/validate_presentation_animation_boundary_v1.py
run python3 tools/validate_presentation_audio_boundary_v1.py
run python3 tools/validate_presentation_texture_boundary_v1.py
run python3 tools/validate_presentation_visual_effect_boundary_v1.py

# Standalone runtime pack deployment is meaningful only from a full Summit
# repository where cliente/ and Servidor/ are siblings of SourceCode/. Source-only
# snapshots keep the static resolver/source-pack checks but skip deployed hashes.
SUMMIT_ROOT="$(cd "$ROOT/.." && pwd)"
if [[ -d "$SUMMIT_ROOT/cliente" && -d "$SUMMIT_ROOT/Servidor" ]]; then
    run python3 tools/content_port/validate_runtime_pack_resolution.py --repo-root "$SUMMIT_ROOT"
else
    run python3 tools/content_port/validate_runtime_pack_resolution.py --repo-root "$ROOT"
    echo "PORTABLE_NOTE: deployed runtime pack hash checks skipped; cliente/ and Servidor/ are not present beside SourceCode/."
fi
run python3 tools/validate_server_shared_gameplay_manifest.py
run python3 tools/validate_character_repository_boundary_v1.py
run python3 tools/validate_companion_status_cleanup_v2.py
run python3 tools/validate_server_status_effect_mutation_ownership_v1.py
run python3 tools/validate_legacy_monolith_authority_residual_v1.py
run python3 tools/validate_companion_class_skill_profile_v1.py
run python3 tools/validate_companion_skill_resources_v2.py
run python3 tools/validate_companion_damage_boundary_v1.py
run python3 tools/validate_companion_life_boundary_v1.py
run python3 tools/validate_companion_kill_life_authority_v1.py

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
run python3 tools/validate_monster_npc_authority_v1.py
run python3 tools/validate_monster_follower_release_v2.py
run python3 tools/validate_character_poison_life_authority_v2.py
run python3 tools/validate_character_monster_regen_life_authority_v2.py
run python3 tools/validate_character_main_targeting_authority_v2.py
run python3 tools/validate_sendtransattack_attack_rating_v2.py
run python3 tools/validate_character_companion_reflect_life_authority_v2.py
run python3 tools/validate_character_main_packet_sender_v2.py
run python3 tools/validate_character_main_heal_life_authority_v2.py
run python3 tools/validate_onserver_healing_life_authority_v1.py
run python3 tools/validate_onserver_holy_reflection_life_authority_v1.py
run python3 tools/validate_onserver_around_monster_life_authority_v1.py
run python3 tools/validate_life_adapter_entity_resolution_v1.py
run python3 tools/validate_t5_arena_life_authority_v1.py
run python3 tools/validate_onserver_lethal_cleanup_life_authority_v1.py
run python3 tools/validate_onserver_virtual_life_restore_authority_v1.py
run python3 tools/validate_castle_life_authority_v1.py
run python3 tools/validate_hardcore_nonlethal_life_authority_v1.py
run python3 tools/validate_autochar_life_clamp_authority_v1.py
run python3 tools/validate_easy_castle_life_authority_v1.py
run python3 tools/validate_server_life_mutation_ownership_v1.py
run python3 tools/validate_server_resource_mutation_ownership_v1.py
run python3 tools/validate_sendtransattack_packet_sender_v2.py
run python3 tools/validate_character_socket_retired_v2.py
run python3 tools/validate_character_transport_retired_v3.py
run python3 tools/validate_playmain_transport_retired_v3.py
run python3 tools/validate_news_packet_sender_v2.py
run python3 tools/validate_character_mingw_compile_fix_v2.py
run python3 tools/validate_skill_info_packet_sender_v2.py
run python3 tools/validate_skill_transport_retired_v3.py
run python3 tools/validate_event_spawn_packet_sender_v2.py
run python3 tools/validate_event_game_packet_sender_v2.py
run python3 tools/validate_client_event_transport_retired_v3.py
run python3 tools/validate_dllserver_packet_sender_v2.py
run python3 tools/validate_integrity_packet_sender_v2.py
run python3 tools/validate_client_integrity_transport_retired_v3.py
run python3 tools/validate_character_record_player_packet_sender_v2.py
run python3 tools/validate_character_record_transport_retired_v3.py
run python3 tools/validate_server_damage_player_packet_sender_v2.py
run python3 tools/validate_server_damage_socket_retired_v2.py
run python3 tools/validate_client_damage_transport_retired_v3.py
run python3 tools/validate_client_party_transport_retired_v3.py
run python3 tools/validate_character_main_simple_lethal_v2.py
run python3 tools/validate_character_actor_damage_life_authority_v3.py
run python3 tools/validate_server_actor_lifecycle_authority_v2.py

# PATCH 77+ transport ownership gates. RED/legacy feature owners may consume
# transport services, but direct socket ownership must remain in AMBER adapters.
run python3 tools/validate_pet_runtime_packet_sender_v2.py
run python3 tools/validate_chat_delivery_packet_sender_v2.py
run python3 tools/validate_backend_channel_sender_v2.py
run python3 tools/validate_client_command_sender_boundary_v1.py
run python3 tools/validate_hellsgate_client_command_sender_boundary_v1.py
run python3 tools/validate_restoration_client_command_sender_boundary_v1.py
run python3 tools/validate_netplay_backend_transport_retired_v3.py
run python3 tools/validate_netplay_socket_transport_retired_v3.py
run python3 tools/validate_area_transport_retired_v3.py
run python3 tools/validate_onserver_socket_transport_retired_v3.py
run python3 tools/validate_login_backend_channel_sender_v2.py
run python3 tools/validate_anticheat_backend_channel_sender_v2.py
run python3 tools/validate_premium_backend_channel_sender_v2.py
run python3 tools/validate_ranking_backend_channel_sender_v2.py
run python3 tools/validate_pvp_score_backend_channel_sender_v2.py
run python3 tools/validate_battle_pass_backend_channel_sender_v2.py
run python3 tools/validate_battle_pass_inventory_authority_v2.py
run python3 tools/validate_class_rank_backend_channel_sender_v2.py
run python3 tools/validate_message_box_backend_channel_sender_v2.py
run python3 tools/validate_interface_transport_retired_v3.py
run python3 tools/validate_inventory_backend_channel_sender_v2.py
run python3 tools/validate_server_inventory_backend_channel_sender_v2.py
run python3 tools/validate_onserver_inventory_authority_v2.py
run python3 tools/validate_inventory_mutation_ownership_v3.py
run python3 tools/validate_inventory_boundary_v1.py
run python3 tools/validate_item_catalog_boundary_v1.py
run python3 tools/validate_wallet_boundary_v1.py
run python3 tools/validate_wallet_mutation_ownership_v2.py
run python3 tools/validate_coin_wallet_mutation_ownership_v1.py
run python3 tools/validate_party_membership_mutation_ownership_v2.py
run python3 tools/validate_clan_membership_mutation_ownership_v1.py
run python3 tools/validate_clan_query_boundary_v1.py
run python3 tools/validate_clan_query_ownership_v2.py
run python3 tools/validate_clan_chat_boundary_v1.py
run python3 tools/validate_clan_update_notifier_boundary_v1.py
run python3 tools/validate_clan_custom_query_boundary_v1.py
run python3 tools/validate_clan_service_packet_sender_v1.py
run python3 tools/validate_clan_custom_packet_sender_v2.py
run python3 tools/validate_coin_shop_client_backend_channel_sender_v2.py
run python3 tools/validate_premium_inventory_grant_boundary_v3.py
run python3 tools/validate_damage_data_backend_channel_sender_v2.py
run python3 tools/validate_shop_backend_channel_sender_v2.py
run python3 tools/validate_character_data_backend_channel_sender_v2.py
run python3 tools/validate_backend_channel_primary_alias_v2.py
run python3 tools/validate_damage_user_extend_backend_channel_sender_v2.py
run python3 tools/validate_cracker_backend_channel_sender_v2.py
run python3 tools/validate_playsub_backend_channel_sender_v2.py
run python3 tools/validate_trade_exact_authority_v3.py
run python3 tools/validate_legacy_surface_classification_v1.py
run python3 tools/validate_preflight_terminal_pass_order_v1.py
run python3 tools/validate_native_preflight_failfast_v1.py
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


if [[ "$MODE" == "portable" || "$MODE" == "native" ]]; then
    # Focused V2 command/kernel smokes introduced by the PATCH 31+ series.
    run bash Shared/gameplay/inventory/v2/tests/run_inventory_snapshot_rollback_tests.sh
    run bash Shared/gameplay/combat/v2/tests/run_combat_buff_service_tests.sh
    run bash Shared/gameplay/combat/v2/tests/run_life_commands_tests.sh
    run bash Shared/gameplay/combat/v2/tests/run_actor_damage_life_policy_tests.sh
    run bash Shared/gameplay/skills/v2/tests/run_skill_animation_commands_tests.sh
    run bash tools/run_monster_npc_v2_portable_tests.sh
    run bash Shared/network/v2/tests/run_backend_channel_sender_tests.sh
    run bash tools/run_commerce_v2_portable_tests.sh
    run bash tools/run_quest_progression_professions_portable_tests.sh
    run bash tools/run_party_v2_portable_tests.sh
    run bash tools/run_clan_v2_portable_tests.sh
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
