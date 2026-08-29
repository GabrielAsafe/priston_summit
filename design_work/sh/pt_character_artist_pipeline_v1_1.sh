#!/usr/bin/env bash
set -euo pipefail

# Summit Tale / Priston Tale
# Character Artist Pipeline V1.1
#
# Proven flow:
#   Artist Master .blend
#     -> conservative game-normalized COPY
#     -> optional TEST-ONLY forced ambiguity resolver
#     -> strict validator
#     -> free-mesh SMD writer
#     -> independent V3 reimport
#     -> manifest
#
# This script NEVER installs anything into the game client.
# Runtime installation remains a separate, SHA-restricted step.

ROOT="$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")/.." && pwd)"

BLENDER_CMD=(flatpak run --filesystem=home org.blender.Blender)

NORMALIZER="$ROOT/pt_rigid_skin_normalize_copy_v1.py"
FORCE_RESOLVER="$ROOT/pt_rigid_skin_force_resolve_test_v1.py"
VALIDATOR="$ROOT/pt_free_mesh_validate_v1_4.py"
WRITER="$ROOT/pt_blender_to_smd_free_mesh_v2_3.py"
IMPORTER="$ROOT/history_py/pt_smd_blender_import_v3_static_armature.py"
BRIDGE="$ROOT/summit_workspace_asset_bridge_v1.py"

SMD_SOURCE="$ROOT/MfbC01.smd"
SKELETON="$ROOT/m5.smb"
MODEL_INFO="$ROOT/M5Bip.inx"
CLIENT_ROOT_DEFAULT="/home/gabriel/Área de Trabalho/summit/Summit Tale Client Full/cliente"
CLIENT_ROOT="${PT_CLIENT_ROOT:-$CLIENT_ROOT_DEFAULT}"

PRISTINE_SHA256="9ec6be4cc148125e56156b5a35da0f2939c2e0b52f17103c9253bfe8bb87120b"

die() {
  printf 'ERROR: %s\n' "$*" >&2
  exit 1
}

usage() {
  cat <<'EOF'
Usage:
  ./sh/pt_character_artist_pipeline_v1_1.sh doctor

  ./sh/pt_character_artist_pipeline_v1_1.sh run \
    --artist blender/<artist_master>.blend \
    [--name <build_name>] \
    [--force-test] \
    [--overwrite] \
    [--roundtrip-workspace <workspace>] \
    [--roundtrip-target <client-relative-path>]

Behavior:
  Default:
    - creates a separate game-normalized .blend;
    - auto-resolves only defensible rigid-skin cases;
    - if ambiguous vertices remain, STOPS with REVIEW_REQUIRED;
    - does not export an SMD until the copy is artist-reviewed.

  --force-test:
    - TEST ONLY;
    - resolves remaining PT_REVIEW_REQUIRED vertices using the tested
      nearest-rigid-seed heuristic;
    - continues through Validate -> Export -> independent Reimport.

  --overwrite:
    - permits replacing this pipeline's generated files only.
    - never overwrites the artist master.

Outputs:
  blender/<name>_game_normalized.blend
  blender/<name>_game_normalized_forced_test.blend   (only --force-test)
  blender/<name>_reimport.blend
  pt_build/C01_topology/<name>.smd
  logs/<name>_*.log/json
  logs/<name>_pipeline_manifest.json

Optional round-trip publication:
  --roundtrip-workspace and --roundtrip-target must be supplied together.
  After pipeline_check=PASS, the validated SMD is published into the already
  registered workspace slot via summit_workspace_asset_bridge_v1.py.
  The client is still NOT modified.

The pipeline does NOT modify the runtime client.
EOF
}

need_file() {
  [[ -f "$1" ]] || die "Required file missing: $1"
}

sha256_file() {
  sha256sum -- "$1" | awk '{print $1}'
}

file_size() {
  stat -c %s -- "$1"
}

check_pristine() {
  need_file "$SMD_SOURCE"
  local h
  h="$(sha256_file "$SMD_SOURCE")"
  [[ "$h" == "$PRISTINE_SHA256" ]] || \
    die "Root MfbC01.smd is not pristine. SHA256=$h"
}

doctor() {
  local failed=0

  printf 'PT_CHARACTER_ARTIST_PIPELINE_V1_1_DOCTOR\n'
  printf 'ROOT=%s\n' "$ROOT"

  for f in \
    "$NORMALIZER" \
    "$FORCE_RESOLVER" \
    "$VALIDATOR" \
    "$WRITER" \
    "$IMPORTER" \
    "$SMD_SOURCE" \
    "$SKELETON" \
    "$MODEL_INFO"
  do
    if [[ -f "$f" ]]; then
      printf 'FILE_OK=%s\n' "$f"
    else
      printf 'FILE_MISSING=%s\n' "$f"
      failed=1
    fi
  done

  if command -v flatpak >/dev/null 2>&1; then
    printf 'FLATPAK=OK\n'
  else
    printf 'FLATPAK=MISSING\n'
    failed=1
  fi

  if [[ -d "$CLIENT_ROOT" ]]; then
    printf 'CLIENT_ROOT=OK:%s\n' "$CLIENT_ROOT"
  else
    printf 'CLIENT_ROOT=MISSING:%s\n' "$CLIENT_ROOT"
    failed=1
  fi

  if [[ -f "$SMD_SOURCE" ]]; then
    printf 'PRISTINE_SIZE=%s\n' "$(file_size "$SMD_SOURCE")"
    printf 'PRISTINE_SHA256=%s\n' "$(sha256_file "$SMD_SOURCE")"
    if [[ "$(sha256_file "$SMD_SOURCE")" != "$PRISTINE_SHA256" ]]; then
      printf 'PRISTINE_CHECK=FAIL\n'
      failed=1
    else
      printf 'PRISTINE_CHECK=PASS\n'
    fi
  fi

  if (( failed )); then
    printf 'pipeline_doctor_check=FAIL\n'
    exit 1
  fi

  printf 'pipeline_doctor_check=PASS\n'
}

run_blender_stage() {
  local log="$1"
  shift

  mkdir -p "$(dirname "$log")"

  set +e
  "${BLENDER_CMD[@]}" "$@" 2>&1 | tee "$log"
  local rc=${PIPESTATUS[0]}
  set -e

  (( rc == 0 )) || die "Blender stage failed (exit=$rc). Log: $log"
}

json_value() {
  local file="$1"
  local expr="$2"
  python3 - "$file" "$expr" <<'PY'
import json, sys
path, expr = sys.argv[1], sys.argv[2]
with open(path, "r", encoding="utf-8") as f:
    data = json.load(f)
obj = data
for part in expr.split("."):
    if part:
        obj = obj[part]
if isinstance(obj, bool):
    print("true" if obj else "false")
elif obj is None:
    print("null")
else:
    print(obj)
PY
}

run_pipeline() {
  local artist=""
  local name=""
  local force_test=0
  local overwrite=0
  local roundtrip_workspace=""
  local roundtrip_target=""

  while (( $# )); do
    case "$1" in
      --artist)
        (( $# >= 2 )) || die "--artist requires a value"
        artist="$2"
        shift 2
        ;;
      --name)
        (( $# >= 2 )) || die "--name requires a value"
        name="$2"
        shift 2
        ;;
      --force-test)
        force_test=1
        shift
        ;;
      --overwrite)
        overwrite=1
        shift
        ;;
      --roundtrip-workspace)
        (( $# >= 2 )) || die "--roundtrip-workspace requires a value"
        roundtrip_workspace="$2"
        shift 2
        ;;
      --roundtrip-target)
        (( $# >= 2 )) || die "--roundtrip-target requires a value"
        roundtrip_target="$2"
        shift 2
        ;;
      -h|--help)
        usage
        return 0
        ;;
      *)
        die "Unknown argument: $1"
        ;;
    esac
  done

  [[ -n "$artist" ]] || die "--artist is required"

  if [[ -n "$roundtrip_workspace" || -n "$roundtrip_target" ]]; then
    [[ -n "$roundtrip_workspace" && -n "$roundtrip_target" ]] ||       die "--roundtrip-workspace and --roundtrip-target must be used together"
    need_file "$BRIDGE"
    if [[ "$roundtrip_workspace" != /* ]]; then
      roundtrip_workspace="$ROOT/$roundtrip_workspace"
    fi
    roundtrip_workspace="$(realpath -m -- "$roundtrip_workspace")"
    [[ -f "$roundtrip_workspace/workspace_manifest.json" ]] ||       die "round-trip workspace manifest missing: $roundtrip_workspace/workspace_manifest.json"
  fi

  if [[ "$artist" != /* ]]; then
    artist="$ROOT/$artist"
  fi
  artist="$(realpath -m -- "$artist")"
  need_file "$artist"

  check_pristine
  need_file "$NORMALIZER"
  need_file "$FORCE_RESOLVER"
  need_file "$VALIDATOR"
  need_file "$WRITER"
  need_file "$IMPORTER"
  need_file "$SKELETON"
  need_file "$MODEL_INFO"
  [[ -d "$CLIENT_ROOT" ]] || die "Client root missing: $CLIENT_ROOT"

  if [[ -z "$name" ]]; then
    name="$(basename "$artist")"
    name="${name%.blend}"
  fi

  [[ "$name" =~ ^[A-Za-z0-9._-]+$ ]] || \
    die "--name may contain only letters, numbers, dot, underscore, hyphen"

  local normalized="$ROOT/blender/${name}_game_normalized.blend"
  local forced="$ROOT/blender/${name}_game_normalized_forced_test.blend"
  local reimport="$ROOT/blender/${name}_reimport.blend"
  local smd_out="$ROOT/pt_build/C01_topology/${name}.smd"

  local normalize_json="$ROOT/logs/${name}_normalize.json"
  local force_json="$ROOT/logs/${name}_force_resolve_test.json"
  local validate_json="$ROOT/logs/${name}_validation.json"
  local export_json="$ROOT/pt_build/C01_topology/${name}_export_report.json"
  local manifest="$ROOT/logs/${name}_pipeline_manifest.json"

  local normalize_log="$ROOT/logs/${name}_normalize.log"
  local force_log="$ROOT/logs/${name}_force_resolve_test.log"
  local validate_log="$ROOT/logs/${name}_validation.log"
  local export_log="$ROOT/logs/${name}_export.log"
  local reimport_log="$ROOT/logs/${name}_reimport.log"

  mkdir -p "$ROOT/blender" "$ROOT/logs" "$ROOT/pt_build/C01_topology"

  if (( ! overwrite )); then
    for f in \
      "$normalized" "$forced" "$reimport" "$smd_out" \
      "$normalize_json" "$force_json" "$validate_json" "$export_json" "$manifest"
    do
      [[ ! -e "$f" ]] || die "Generated output already exists: $f (use --overwrite intentionally)"
    done
  else
    rm -f -- \
      "$normalized" "$forced" "$reimport" "$smd_out" \
      "$normalize_json" "$force_json" "$validate_json" "$export_json" "$manifest"
  fi

  local artist_sha
  artist_sha="$(sha256_file "$artist")"

  printf '\n=== STAGE 1: NORMALIZE COPY ===\n'
  local normalize_args=(
    --background "$artist"
    --python-exit-code 1
    --python "$NORMALIZER" --
    --object MshC01
    --out "$normalized"
    --report "$normalize_json"
  )
  if (( overwrite )); then
    normalize_args+=(--overwrite)
  fi
  run_blender_stage "$normalize_log" "${normalize_args[@]}"

  need_file "$normalized"
  need_file "$normalize_json"

  local unresolved
  unresolved="$(json_value "$normalize_json" "unresolved")"
  local normalized_status
  normalized_status="$(json_value "$normalize_json" "status")"

  printf 'NORMALIZE_STATUS=%s\n' "$normalized_status"
  printf 'UNRESOLVED=%s\n' "$unresolved"
  printf 'NORMALIZED_BLEND=%s\n' "$normalized"

  local game_blend="$normalized"
  local forced_used=false

  if [[ "$unresolved" != "0" ]]; then
    if (( ! force_test )); then
      cat <<EOF

PIPELINE_STATUS=REVIEW_REQUIRED
The artist master was preserved.
A game-normalized copy was created:
  $normalized

$unresolved vertex/vertices are ambiguous for the game's rigid Physique.
They remain marked in:
  PT_REVIEW_REQUIRED

Reason:
  Blender supports interpolated multi-bone weights.
  This runtime pipeline requires exactly one bone per vertex.

Review the marked vertices in the normalized copy, assign exactly one valid
skeleton bone at weight 1.0, remove PT_REVIEW_REQUIRED, then rerun the pipeline
using that reviewed file as --artist.

For engineering/runtime tests only, rerun with:
  --force-test
EOF
      exit 3
    fi

    printf '\n=== STAGE 2: FORCE AMBIGUOUS SKIN (TEST ONLY) ===\n'
    local force_args=(
      --background "$normalized"
      --python-exit-code 1
      --python "$FORCE_RESOLVER" --
      --object MshC01
      --out "$forced"
      --report "$force_json"
    )
    if (( overwrite )); then
      force_args+=(--overwrite)
    fi
    run_blender_stage "$force_log" "${force_args[@]}"
    need_file "$forced"
    need_file "$force_json"

    local invalid_after
    invalid_after="$(json_value "$force_json" "invalid_after")"
    [[ "$invalid_after" == "0" ]] || \
      die "Force resolver left invalid vertices: $invalid_after"

    game_blend="$forced"
    forced_used=true
  else
    printf '\n=== STAGE 2: FORCE AMBIGUOUS SKIN ===\n'
    printf 'SKIPPED=no ambiguous vertices\n'
  fi

  printf '\n=== STAGE 3: VALIDATE GAME COPY ===\n'
  run_blender_stage "$validate_log" \
    --background "$game_blend" \
    --python-exit-code 1 \
    --python "$VALIDATOR" -- \
    --smd "$SMD_SOURCE" \
    --skeleton "$SKELETON" \
    --object MshC01 \
    --report "$validate_json"

  need_file "$validate_json"
  local validation_check
  validation_check="$(json_value "$validate_json" "validation_check")"
  [[ "$validation_check" == "PASS" ]] || \
    die "Validator report is not PASS: $validation_check"

  printf '\n=== STAGE 4: EXPORT SMD ===\n'
  run_blender_stage "$export_log" \
    --background "$game_blend" \
    --python-exit-code 1 \
    --python "$WRITER" -- \
    --smd "$SMD_SOURCE" \
    --skeleton "$SKELETON" \
    --object MshC01 \
    --out "$smd_out" \
    --report "$export_json"

  need_file "$smd_out"
  need_file "$export_json"

  local semantic_check
  semantic_check="$(json_value "$export_json" "semantic_check")"
  [[ "$semantic_check" == "PASS" ]] || \
    die "Writer semantic report is not PASS: $semantic_check"

  local out_v out_f out_t
  out_v="$(json_value "$export_json" "output_counts.vertices")"
  out_f="$(json_value "$export_json" "output_counts.faces")"
  out_t="$(json_value "$export_json" "output_counts.texlinks")"

  local smd_sha smd_size
  smd_sha="$(sha256_file "$smd_out")"
  smd_size="$(file_size "$smd_out")"

  local report_sha report_size
  report_sha="$(json_value "$export_json" "output_sha256")"
  report_size="$(json_value "$export_json" "output_size")"

  [[ "$smd_sha" == "$report_sha" ]] || \
    die "SMD SHA differs from writer report"
  [[ "$smd_size" == "$report_size" ]] || \
    die "SMD size differs from writer report"

  printf '\n=== STAGE 5: INDEPENDENT V3 REIMPORT ===\n'
  run_blender_stage "$reimport_log" \
    --background \
    --python-exit-code 1 \
    --python "$IMPORTER" -- \
    --smd "$smd_out" \
    --skeleton "$SKELETON" \
    --model-info "$MODEL_INFO" \
    --lod high \
    --client-root "$CLIENT_ROOT" \
    --out "$reimport" \
    --no-pack

  need_file "$reimport"

  grep -Fq 'static_armature_import_check=PASS' "$reimport_log" || \
    die "Independent reimport did not report static_armature_import_check=PASS"

  grep -Fq "OBJECT MshC01: V=$out_v F=$out_f UV=$out_t" "$reimport_log" || \
    die "Independent reimport MshC01 counts do not match writer report"

  python3 - \
    "$manifest" \
    "$artist" \
    "$artist_sha" \
    "$normalized" \
    "$game_blend" \
    "$forced_used" \
    "$unresolved" \
    "$validate_json" \
    "$export_json" \
    "$smd_out" \
    "$smd_size" \
    "$smd_sha" \
    "$reimport" \
    "$out_v" \
    "$out_f" \
    "$out_t" <<'PY'
import json, sys
(
    manifest,
    artist,
    artist_sha,
    normalized,
    game_blend,
    forced_used,
    unresolved,
    validate_json,
    export_json,
    smd_out,
    smd_size,
    smd_sha,
    reimport,
    out_v,
    out_f,
    out_t,
) = sys.argv[1:]

data = {
    "tool": "PT_CHARACTER_ARTIST_PIPELINE_V1_1",
    "pipeline_check": "PASS",
    "artist_master": artist,
    "artist_master_sha256": artist_sha,
    "artist_master_overwritten": False,
    "normalized_copy": normalized,
    "game_copy_used": game_blend,
    "test_force_ambiguous_used": forced_used == "true",
    "normalize_unresolved_before_test_force": int(unresolved),
    "validator_report": validate_json,
    "export_report": export_json,
    "smd": smd_out,
    "smd_size": int(smd_size),
    "smd_sha256": smd_sha,
    "reimport_blend": reimport,
    "output_counts": {
        "vertices": int(out_v),
        "faces": int(out_f),
        "texlinks": int(out_t),
    },
    "runtime_installed": False,
    "note": (
        "Pipeline stops before runtime installation. Runtime install must use "
        "a separate installer restricted to this exact SMD size/SHA."
    ),
}
with open(manifest, "w", encoding="utf-8") as f:
    json.dump(data, f, ensure_ascii=False, indent=2)
PY

  local roundtrip_published=false
  if [[ -n "$roundtrip_workspace" ]]; then
    printf '\n=== STAGE 6: PUBLISH VALIDATED BUILD TO ROUNDTRIP WORKSPACE ===\n'
    python3 "$BRIDGE" publish "$roundtrip_workspace" \
      --pipeline-manifest "$manifest" \
      --target "$roundtrip_target"
    roundtrip_published=true
  fi

  printf '\nPT_CHARACTER_ARTIST_PIPELINE_V1_1\n'
  printf 'artist_master=%s\n' "$artist"
  printf 'artist_master_sha256=%s\n' "$artist_sha"
  printf 'artist_master_overwritten=False\n'
  printf 'normalized_copy=%s\n' "$normalized"
  printf 'game_copy_used=%s\n' "$game_blend"
  printf 'test_force_ambiguous_used=%s\n' "$forced_used"
  printf 'output_counts=%s/%s/%s\n' "$out_v" "$out_f" "$out_t"
  printf 'smd=%s\n' "$smd_out"
  printf 'smd_size=%s\n' "$smd_size"
  printf 'smd_sha256=%s\n' "$smd_sha"
  printf 'reimport=%s\n' "$reimport"
  printf 'manifest=%s\n' "$manifest"
  printf 'roundtrip_published=%s\n' "$roundtrip_published"
  if [[ "$roundtrip_published" == true ]]; then
    printf 'roundtrip_workspace=%s\n' "$roundtrip_workspace"
    printf 'roundtrip_target=%s\n' "$roundtrip_target"
  fi
  printf 'runtime_installed=False\n'
  printf 'pipeline_check=PASS\n'
}

case "${1:-}" in
  doctor)
    shift
    (( $# == 0 )) || die "doctor takes no arguments"
    doctor
    ;;
  run)
    shift
    run_pipeline "$@"
    ;;
  -h|--help|"")
    usage
    ;;
  *)
    die "Unknown command: $1"
    ;;
esac
