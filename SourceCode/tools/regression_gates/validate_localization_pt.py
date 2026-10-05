#!/usr/bin/env python3
from __future__ import annotations

from pathlib import Path
import json
import re
import sys

root = Path(__file__).resolve().parents[2]
canonical = root / 'Content/canonical'
errors: list[str] = []
warnings: list[str] = []
json_count = 0

for p in canonical.rglob('*.json'):
    json_count += 1
    try:
        json.loads(p.read_text(encoding='utf-8'))
    except UnicodeDecodeError as exc:
        errors.append(f'{p.relative_to(root)}: invalid UTF-8: {exc}')
    except json.JSONDecodeError as exc:
        errors.append(f'{p.relative_to(root)}: invalid JSON: {exc}')

loc = canonical / 'text/localization'
override_path = canonical / 'text/localization_overrides/pt-BR.json'

def load(path: Path) -> dict:
    return json.loads(path.read_text(encoding='utf-8'))

en_doc = load(loc / 'en.json')
pt_doc = load(loc / 'pt-BR.json')
bindings = load(loc / 'bindings.json')
override_doc = load(override_path)
runtime_messages_path = canonical / 'text/runtime_messages.json'
runtime_messages_doc = load(runtime_messages_path) if runtime_messages_path.exists() else None

if pt_doc.get('locale') != 'pt-BR':
    errors.append('pt-BR.json locale is not pt-BR')
if pt_doc.get('fallback_locale') is not None:
    errors.append('pt-BR.json must not fall back to English; fallback_locale must be null')
if override_doc.get('locale') != 'pt-BR' or override_doc.get('fallback_locale') is not None:
    errors.append('canonical pt-BR override must be authoritative and have fallback_locale=null')


def keyed(doc: dict, name: str) -> dict[str, dict]:
    out: dict[str, dict] = {}
    for row in doc.get('entries', []):
        k = row.get('key')
        text = row.get('text')
        if not isinstance(k, str) or not k:
            errors.append(f'{name}: empty/invalid key')
            continue
        if k in out:
            errors.append(f'{name}: duplicate key {k}')
        if not isinstance(text, str) or not text.strip():
            errors.append(f'{name}: empty text {k}')
        out[k] = row
    return out

en = keyed(en_doc, 'en')
pt = keyed(pt_doc, 'pt-BR')
override = keyed(override_doc, 'pt-BR override')

for key in sorted(set(en) - set(pt)):
    errors.append(f'pt-BR missing runtime key: {key}')
for key in sorted(set(pt) - set(en)):
    errors.append(f'pt-BR key has no English baseline: {key}')
for key in sorted(set(en) - set(override)):
    errors.append(f'canonical pt-BR override missing runtime key: {key}')
for key in sorted(set(override) - set(en)):
    errors.append(f'canonical pt-BR override has no English baseline: {key}')

# The generated runtime catalog must be byte-for-byte equivalent to the maintained
# canonical override for text/quality. This prevents regeneration from silently
# restoring legacy English, Korean, Japanese, or mojibake J_* values.
for key in sorted(set(pt) & set(override)):
    if pt[key].get('text') != override[key].get('text'):
        errors.append(f'generated pt-BR differs from canonical override: {key}')
    if pt[key].get('quality') != 'translated':
        errors.append(f'pt-BR entry is not authoritative translated content: {key} ({pt[key].get("quality")!r})')

# Every bound runtime localization key must exist in PT too, and a definition name
# must agree with its linked legacy localization resource name when both exist.
for b in bindings.get('bindings', []):
    name_key = b.get('name_key')
    if name_key and name_key in en and name_key not in pt:
        errors.append(f'binding missing pt-BR key: {name_key}')
    elif name_key and name_key not in en:
        warnings.append(f'dangling binding without EN/PT catalog row: {name_key}')
    for key in b.get('chat_keys') or []:
        if key and key in en and key not in pt:
            errors.append(f'binding missing pt-BR key: {key}')
        elif key and key not in en:
            warnings.append(f'dangling binding without EN/PT catalog row: {key}')

    linked = b.get('linked_resource_id')
    linked_name = f'{linked}.name' if linked else None
    if name_key in pt and linked_name in pt:
        if pt[name_key].get('text') != pt[linked_name].get('text'):
            errors.append(
                f'incoherent entity name: {name_key}={pt[name_key].get("text")!r} '
                f'but {linked_name}={pt[linked_name].get("text")!r}'
            )

# Runtime-only messages are part of the same language contract. They must have an
# explicit PT value and it must be exactly what ships in the generated/override catalog.
if runtime_messages_doc is not None:
    if runtime_messages_doc.get('schema') != 'summit.runtime-localization-source/v1':
        errors.append('runtime_messages.json has an invalid schema')
    for row in runtime_messages_doc.get('entries', []):
        key = row.get('key') if isinstance(row, dict) else None
        locales = row.get('locales') if isinstance(row, dict) else None
        if not isinstance(key, str) or not key or not isinstance(locales, dict):
            errors.append('runtime_messages.json contains an invalid entry')
            continue
        pt_text = locales.get('pt-BR')
        en_text = locales.get('en')
        if not isinstance(en_text, str) or not en_text.strip():
            errors.append(f'runtime message has no English baseline: {key}')
        if not isinstance(pt_text, str) or not pt_text.strip():
            errors.append(f'runtime message has no explicit pt-BR value: {key}')
            continue
        if key not in pt:
            errors.append(f'runtime message missing from generated pt-BR catalog: {key}')
        elif pt[key].get('text') != pt_text:
            errors.append(f'runtime message differs from generated pt-BR catalog: {key}')
        if key not in override:
            errors.append(f'runtime message missing from canonical pt-BR override: {key}')
        elif override[key].get('text') != pt_text:
            errors.append(f'runtime message differs from canonical pt-BR override: {key}')

# PT must not carry foreign-script residues or common decoding corruption.
foreign_re = re.compile(r'[\u3040-\u30ff\u3400-\u9fff\uac00-\ud7af\u0080-\u009f\ufffd]')
mojibake_markers = ('Ã', 'Â', 'â€', 'ï¿½', '#U00', '#u00', 'ƒ')

def looks_like_cp932_mojibake(value: str) -> bool:
    # Legacy Japanese strings were frequently decoded as CP1252 and then stored
    # as Unicode (for example: ƒCƒxƒ“ƒg). Re-encoding that display text as
    # CP1252 and decoding it as CP932 recovers Japanese kana. Any pt-BR value
    # with that signature is corrupted source text, never valid Portuguese.
    try:
        decoded = value.encode('cp1252').decode('cp932')
    except (UnicodeEncodeError, UnicodeDecodeError):
        return False
    return decoded != value and bool(re.search(r'[\u3040-\u30ff]', decoded))
corrupt_question_re = re.compile(r'[A-Za-zÀ-ÿ]\?[A-Za-zÀ-ÿ]')

# Strong English function words are useful for dialogue detection. Proper nouns and
# game names do not trip this heuristic because it requires a sentence-level score.
english_function_words = {
    'the','and','you','your','yours','are','is','was','were','have','has','had','can',
    'could','would','should','will','with','from','this','that','these','those','they',
    'them','their','there','here','what','when','where','why','how','who','which','please',
    'welcome','into','nowadays','seem','lack','wish','best','luck','way','going','turned',
    'bring','find','need','give','take','talk','thank','sorry','hello','field','team','win',
}
portuguese_function_words = {
    'de','da','do','das','dos','e','o','a','os','as','um','uma','para','por','com','sem',
    'em','no','na','nos','nas','você','voce','seu','sua','seus','suas','eu','ele','ela',
    'eles','elas','nós','que','não','nao','sim','muito','mais','menos','se','quando','onde',
    'como','porque','aqui','ali','este','esta','esse','essa','meu','minha','lhe','ao','à',
}

# Generic English vocabulary is forbidden in PT names. Established proper world/game
# terms (Priston Tale, Bless Castle, SoD, Aging, class names) are intentionally not in
# this set; generic UI/entity nouns must be localized.
# Generic English display vocabulary is forbidden anywhere in pt-BR, not only
# names. Keep this list focused on unambiguous English words so proper game/world
# names and Portuguese loanwords do not become false positives.
banned_display_tokens = {
    'what','talking','skills','level','coins','tune','aging','mix','messenger','helper',
    'healing','captured','disguises','peddler','ribbon','cant','find','dear','prince',
    'gorgeous','survival','die','boss','earring','player','skin','skins','ticket','craft',
    'clear','endgame','package','succubus','incubus','goblin','mummy','princess','sage',
    'hermit','elder','adventurer','cursed','web','summon','tiger','shotgun','octopus',
    'shark','destroyer','hand','rule','wind','beef','thigh','wrench','pencil','pistol',
    'gorgon','imp','ghoul','cyclops','quake','stun','freeze','skull','respec','craft',
    'mirage','nymph','archon','legion','slayer','claymore','soccerfan','ranking','wolverine','clans',
}

banned_name_tokens = {
    'armor','armour','boots','boot','costume','ring','amulet','sword','axe','bow','staff',
    'shield','robe','gauntlet','bracelet','crystal','support','shop','guard','hunter','quest',
    'village','stone','potion','scroll','doom','dark','ancient','great','holy','chaos',
    'phantom','ghost','dragon','demon','vampire','bloody','golden','silver','red','blue',
    'green','black','white','yellow','pink','purple','wing','wings','helmet','hammer',
    'javelin','claw','scythe','spear','box','bag','skill','delay','lucky','wedding','student',
    'army','military','platinum','meteorite','mist','fury','restoration','revolt','fire','ice',
    'lightning','death','resurrection','hair','soul','fireworks','gravity','clean','honor',
    'smile','love','sadness','cute','chest','emerald','collector','beginner','beach','swan',
    'raven','commander','school','combatant','additional','serpent','abyss','guardian','steel',
    'supreme','abomination','specialization','creation','heart','turtle','bone','light','long',
    'wide','fold','scales','rigid','winged','grand','spiky','justice','minotaur','flaming',
    'starter','sacred','egg',
}

# Cross-entity naming glossary. These are the same canonical creature/service
# presented through different legacy definitions and must not drift in pt-BR.
canonical_name_groups = [
    ('monster.4_wolverine_de_bronze.name', 'npc.bronzwolverin.name'),
    ('monster.13_wolverine_de_ouro.name', 'npc.goldenwolverin.name'),
    ('monster.8_wolverine_de_prata.name', 'npc.sillverwolverin.name'),
    ('monster.64_rei_hopi.name', 'npc.puzzleking.name'),
]
for left, right in canonical_name_groups:
    if left in pt and right in pt and pt[left].get('text') != pt[right].get('text'):
        errors.append(
            f'incoherent canonical entity name: {left}={pt[left].get("text")!r} '
            f'but {right}={pt[right].get("text")!r}'
        )

expected_names = {
    'npc.npc_ninefox.name': 'Preia',
    'npc.quest.name': 'Mestre de Missões',
    'npc.rosemari.name': 'Garota do Evento',
    'npc.sod_buma.name': 'Buma',
    'npc.sod_goblin.name': 'Hobgoblin',
    'npc.real_clanmaster.name': 'Mestre dos Clãs',
    'npc.nadia.name': 'Passe de Batalha',
    'npc.salon.name': 'Passe de Batalha',
    'npc.soccerfan.name': 'Torcedor',
    'npc.soccerfan_nego_ney.name': 'Torcedor',
    'npc.star_girl.name': 'Garota da Classificação',
    'npc.navisko_magic.name': '3ª Loja do Maron',
    'npc.nevisko_magic.name': '3ª Loja do Maron',
    'npc.ruiden_magic.name': '2ª Loja do Maron',
    'npc.timeshop.name': 'Loja de Minutos',
    'monster.sod_shadow_phalanx.name': 'Falange Sombria do SoD',
}
for key, expected in expected_names.items():
    if key not in pt:
        errors.append(f'canonical PT name missing: {key}')
    elif pt[key].get('text') != expected:
        errors.append(f'canonical PT name drift: {key}={pt[key].get("text")!r}; expected {expected!r}')

for key, row in pt.items():
    text = row.get('text', '')
    if row.get('quality') != 'translated':
        errors.append(f'pt-BR fallback/source quality is forbidden: {key} ({row.get("quality")!r})')
    if foreign_re.search(text):
        errors.append(f'pt-BR contains foreign/C1/replacement characters: {key}: {text!r}')
    if any(marker in text for marker in mojibake_markers):
        errors.append(f'pt-BR contains mojibake marker: {key}: {text!r}')
    if looks_like_cp932_mojibake(text):
        errors.append(f'pt-BR contains CP932-as-CP1252 mojibake: {key}: {text!r}')
    if corrupt_question_re.search(text):
        errors.append(f'pt-BR contains a corrupted character represented by ?: {key}: {text!r}')
    if '.name' in key and '?' in text:
        errors.append(f'pt-BR name contains ?: {key}: {text!r}')

    words = [w.casefold() for w in re.findall(r"[A-Za-zÀ-ÿ']+", text)]
    display_tokens = sorted({w for w in words if w in banned_display_tokens})
    if display_tokens:
        errors.append(f'pt-BR contains generic English display token(s) {display_tokens}: {key}: {text!r}')
    if '.chat.' in key:
        en_score = sum(w in english_function_words for w in words)
        pt_score = sum(w in portuguese_function_words for w in words)
        if en_score >= 2 and en_score > pt_score:
            errors.append(f'pt-BR dialogue appears to be English: {key}: {text!r}')
    if '.name' in key:
        foreign_tokens = sorted({w for w in words if w in banned_name_tokens})
        if foreign_tokens:
            errors.append(f'pt-BR name contains generic English token(s) {foreign_tokens}: {key}: {text!r}')

print(f'JSON syntax/UTF-8: {json_count} files OK' if not errors else f'JSON scanned: {json_count}')
print(f'Localization keys: en={len(en)} pt-BR={len(pt)} canonical_override={len(override)}')
for w in dict.fromkeys(warnings):
    print('WARN:', w)
if errors:
    print('\n'.join('ERROR: ' + x for x in errors[:300]), file=sys.stderr)
    if len(errors) > 300:
        print(f'ERROR: ... {len(errors) - 300} more errors', file=sys.stderr)
    sys.exit(1)
print('PATCH 193 gate: OK — pt-BR is authoritative, Portuguese-only, coherent across canonical entity names, UTF-8 clean, and has no fallback')
