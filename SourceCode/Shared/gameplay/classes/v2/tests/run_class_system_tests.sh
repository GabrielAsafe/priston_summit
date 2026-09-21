#!/usr/bin/env bash
set -euo pipefail

ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/../../../../.." && pwd)"
CXX="${CXX:-g++}"
BUILD_DIR="${TMPDIR:-/tmp}/priston_class_system_v2_tests"
mkdir -p "$BUILD_DIR"

COMMON=(-std=gnu++14 -Wall -Wextra -Werror -pedantic -I"$ROOT/Shared" -I"$ROOT/SrcServer/src")
CLASS_IMPL=(
    "$ROOT/Shared/gameplay/classes/v2/PlayableClassRegistry.cpp"
    "$ROOT/Shared/gameplay/classes/v2/content/DefaultClassContent.cpp"
    "$ROOT/Shared/gameplay/classes/v2/content/LegacyClassCatalog.cpp"
    "$ROOT/Shared/gameplay/classes/v2/content/ReservedClassCatalog.cpp"
    "$ROOT/Shared/gameplay/classes/v2/services/ClassService.cpp"
)

build_class_test() {
    local name="$1"
    "$CXX" "${COMMON[@]}" "${CLASS_IMPL[@]}" \
        "$ROOT/Shared/gameplay/classes/v2/tests/${name}.cpp" \
        -o "$BUILD_DIR/$name"
    "$BUILD_DIR/$name"
    echo "$name: PASS"
}

build_class_test playable_class_registry_smoke
build_class_test legacy_class_quest_adapter_smoke
build_class_test legacy_class_commerce_adapter_smoke
build_class_test legacy_class_onboarding_adapter_smoke

"$CXX" "${COMMON[@]}" \
    "$ROOT/Shared/gameplay/classes/v2/tests/legacy_class_event_kit_adapter_smoke.cpp" \
    -o "$BUILD_DIR/legacy_class_event_kit_adapter_smoke"
"$BUILD_DIR/legacy_class_event_kit_adapter_smoke"
echo "legacy_class_event_kit_adapter_smoke: PASS"

python3 "$ROOT/Shared/gameplay/classes/v2/tests/class_architecture_guard.py"
echo "Class system V2 integration suite: PASS"
