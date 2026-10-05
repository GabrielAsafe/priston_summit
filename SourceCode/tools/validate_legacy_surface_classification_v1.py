#!/usr/bin/env python3
from pathlib import Path
import sys
ROOT=Path(__file__).resolve().parents[1]
DOC=ROOT/'docs/architecture/LEGACY_SURFACE_RETIREMENT_MATRIX_V1.md'
PREFLIGHT=ROOT/'tools/run_frozen_legacy_preflight.sh'
SURFACES={
 'smCHAR::Main': ROOT/'SrcServer/src/systems/gameplay/character/core/character.cpp',
 'cITEM::ShowItemInfo': ROOT/'SrcGame/src/systems/inventory/legacy/sinItem.cpp',
 'cINVENTORY::RButtonDown': ROOT/'SrcGame/src/systems/inventory/legacy/sinInvenTory.cpp',
 'cSKILL::ShowSkillInfo': ROOT/'Shared/gameplay/skills/sinSkill_legacy_client.inl',
 'PlaySkillAttack': ROOT/'Shared/gameplay/skills/SkillSub_legacy_client.inl',
 'smCHAR::EventSkill': ROOT/'SrcServer/src/systems/gameplay/character/core/character.cpp',
 'RecvCommand': ROOT/'Shared/server/legacy_runtime/OnSever_legacy_server.inl',
 'netplay.cpp': ROOT/'SrcGame/src/systems/network/client/netplay.cpp',
 'sinTrade': ROOT/'SrcGame/src/systems/commerce/legacy/sinTrade.cpp',
 'ClanChipLegacyCommon': ROOT/'Shared/social/clan/ClanChipLegacyCommon.inl',
 'HoEffect / AssaParticle': ROOT/'SrcGame/src/systems/rendering/effects/legacy_ho/core/HoEffect.cpp',
}
REQUIRED_GATES=(
 'validate_legacy_monolith_authority_residual_v1.py',
 'validate_inventory_boundary_v1.py',
 'validate_item_catalog_boundary_v1.py',
 'validate_trade_exact_authority_v3.py',
 'validate_playskillattack_attack_rating_v2.py',
 'validate_playskillattack_animation_authority_v2.py',
 'validate_eventskill_attack_rating_v2.py',
 'validate_eventskill_animation_authority_v2.py',
 'validate_netplay_backend_transport_retired_v3.py',
 'validate_netplay_socket_transport_retired_v3.py',
 'validate_clan_membership_mutation_ownership_v1.py',
 'validate_clan_query_ownership_v2.py',
 'validate_clan_chat_boundary_v1.py',
 'validate_clan_update_notifier_boundary_v1.py',
 'validate_clan_custom_query_boundary_v1.py',
 'validate_clan_service_packet_sender_v1.py',
 'validate_clan_custom_packet_sender_v2.py',
 'validate_presentation_renderer_boundary_v1.py',
 'validate_presentation_animation_boundary_v1.py',
 'validate_presentation_audio_boundary_v1.py',
 'validate_presentation_texture_boundary_v1.py',
 'validate_presentation_visual_effect_boundary_v1.py',
)

def fail(m): print('legacy surface classification v1: FAIL: '+m,file=sys.stderr); raise SystemExit(1)
if not DOC.is_file(): fail('missing retirement matrix')
doc=DOC.read_text(encoding='utf-8')
for token in ('LEGACY_SURFACES_CLASSIFIED=11','CLASSIFIED_HIGH_AUTHORITY_RESIDUAL=ZERO_STATIC','VISUAL_RETIREMENT=BLOCKED_RUNTIME_PARITY'):
 if token not in doc: fail('matrix missing token '+token)
for label,path in SURFACES.items():
 if not path.is_file(): fail(f'missing classified surface file for {label}: {path.relative_to(ROOT)}')
 # Each label (or its primary function/name fragment) must be represented in the matrix.
 needle=label.split(' / ')[0]
 if needle not in doc: fail('matrix missing surface '+label)
# AssaParticle is the second half of the combined effects row.
assa=ROOT/'Shared/rendering/effects/assa/AssaParticleLegacyCommon.inl'
if not assa.is_file() or 'AssaParticle' not in doc: fail('AssaParticle shell is not classified')
# Replacement presentation surface cited for the skill tooltip/page must exist.
if not (ROOT/'SrcGame/src/systems/ui/v2/pages/SkillsPage.cpp').is_file(): fail('missing UI V2 SkillsPage')
pre=PREFLIGHT.read_text(encoding='utf-8')
missing=[x for x in REQUIRED_GATES if x not in pre]
if missing: fail('required authority gates not wired into frozen preflight: '+', '.join(missing))
print(f'legacy surface classification v1: PASS (surfaces={len(SURFACES)}, required_gates={len(REQUIRED_GATES)}, visual_retirement=BLOCKED_RUNTIME_PARITY)')
