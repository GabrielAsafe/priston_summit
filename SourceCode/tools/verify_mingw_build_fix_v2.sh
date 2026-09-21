#!/usr/bin/env bash
set -euo pipefail
ROOT="${1:-$(pwd)}"
cd "$ROOT"
fail=0
ok(){ printf '[PASS] %s\n' "$1"; }
bad(){ printf '[FAIL] %s\n' "$1"; fail=1; }
contains(){ grep -Fq "$2" "$1" && ok "$3" || bad "$3 ($1)"; }

contains Shared/formats/item_config/LegacyItemRuntimeExact_client.inl 'LegacyJobDataView.h' 'client exact item runtime directly imports LegacyJobDataView'
contains Shared/formats/item_config/LegacyItemRuntimeExact_server.inl 'LegacyJobDataView.h' 'server exact item runtime directly imports LegacyJobDataView'
contains SrcGame/src/systems/combat/damage/Damage.cpp 'int criticalScalingValue = 0;' 'client Damage declares criticalScalingValue'
contains SrcServer/src/systems/combat/damage/Damage.cpp 'int criticalScalingValue = 0;' 'server Damage declares criticalScalingValue'
contains Shared/gameplay/character/character_legacy_client.inl 'std::uint32_t motionJobMask = 0u;' 'client character uses uint32 motion mask boundary'
contains SrcServer/src/systems/gameplay/character/core/character.cpp 'std::uint32_t motionJobMask = 0u;' 'server character uses uint32 motion mask boundary'
contains SrcGame/src/systems/ui/help/legacy_sin/sinHelp.cpp 'const DWORD changeJobWeapon' 'client sinHelp scopes change-job reward'
contains SrcServer/src/systems/ui/help/legacy_sin/sinHelp.cpp 'const DWORD changeJobWeapon' 'server sinHelp scopes change-job reward'
contains SrcGame/src/systems/ui/hud/legacy_sin/sinInterFace.cpp 'int nextChangeJob = 0;' 'client interface uses int ChangeJob boundary'
contains SrcServer/src/systems/ui/hud/legacy_sin/sinInterFace.cpp 'int nextChangeJob = 0;' 'server interface uses int ChangeJob boundary'
contains SrcGame/src/systems/ui/messages/legacy_sin/sinMessageBox.cpp 'int nextChangeJob = 0;' 'client messagebox uses int ChangeJob boundary'
contains SrcServer/src/systems/ui/messages/legacy_sin/sinMessageBox.cpp 'int nextChangeJob = 0;' 'server messagebox uses int ChangeJob boundary'
contains SrcGame/src/systems/commerce/coin_shop/CoinShop.h 'GetSectionForUI() const' 'CoinShop exposes UI V2 bridge'
contains SrcGame/src/systems/commerce/coin_shop/CoinShop_T.h 'GetSectionForUI() const' 'TimeShop exposes UI V2 bridge'
contains build_client_mingw_i686_multicore.sh 'compile_success.ok' 'client compile emits success marker'
contains build_server_mingw_i686_j8.sh 'compile_success.ok' 'server compile emits success marker'
contains link_client_mingw_i686.sh 'a compilação completa não foi confirmada' 'client link refuses partial compile'
contains link_server_mingw_i686.sh 'a compilação completa não foi confirmada' 'server link refuses partial compile'

if [ "$fail" -ne 0 ]; then
  echo 'BUILD_FIX_V2=FAIL'
  exit 1
fi
echo 'BUILD_FIX_V2=PASS'
