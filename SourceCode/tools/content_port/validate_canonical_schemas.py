#!/usr/bin/env python3
from __future__ import annotations
import argparse, json
from pathlib import Path

try:
    import jsonschema
except ImportError as exc:
    raise SystemExit('jsonschema is required for canonical schema validation') from exc


def load(path: Path):
    return json.loads(path.read_text(encoding='utf-8'))


def main():
    ap=argparse.ArgumentParser(description='Validate Summit canonical JSON documents against published schemas')
    ap.add_argument('--canonical-text',type=Path,required=True)
    ap.add_argument('--canonical-spawns',type=Path,required=True)
    ap.add_argument('--canonical-maps',type=Path,required=True)
    a=ap.parse_args()
    schema_root=Path(__file__).resolve().parent/'schemas'
    text_schema=load(schema_root/'canonical_text_resource.schema.json')
    spawn_schema=load(schema_root/'stage_spawn.schema.json')
    map_schema=load(schema_root/'map_catalog.schema.json')
    localization_schema=load(schema_root/'localization_catalog.schema.json')
    family_schemas={
        'monster_definition': load(schema_root/'monster_definition.schema.json'),
        'npc_definition': load(schema_root/'npc_definition.schema.json'),
        'open_item_definition': load(schema_root/'item_definition.schema.json'),
    }
    gameplay_schema=load(schema_root/'gameplay_config.schema.json')
    text_idx=load(a.canonical_text/'index.json')['documents']
    spawn_idx=load(a.canonical_spawns/'index.json')['documents']
    TextValidator=jsonschema.validators.validator_for(text_schema); TextValidator.check_schema(text_schema); text_validator=TextValidator(text_schema)
    SpawnValidator=jsonschema.validators.validator_for(spawn_schema); SpawnValidator.check_schema(spawn_schema); spawn_validator=SpawnValidator(spawn_schema)
    MapValidator=jsonschema.validators.validator_for(map_schema); MapValidator.check_schema(map_schema); map_validator=MapValidator(map_schema)
    LocalizationValidator=jsonschema.validators.validator_for(localization_schema); LocalizationValidator.check_schema(localization_schema); localization_validator=LocalizationValidator(localization_schema)
    family_validators={}
    for kind,schema in family_schemas.items():
        V=jsonschema.validators.validator_for(schema); V.check_schema(schema); family_validators[kind]=V(schema)
    GV=jsonschema.validators.validator_for(gameplay_schema); GV.check_schema(gameplay_schema); gameplay_validator=GV(gameplay_schema)
    family_counts={k:0 for k in family_validators}; gameplay_count=0
    for e in text_idx:
        doc=load(a.canonical_text/e['json_path']); text_validator.validate(doc)
        kind=doc.get('kind')
        if kind in family_validators:
            family_validators[kind].validate(doc); family_counts[kind]+=1
        if kind=='ini_config' and doc.get('selection_reason')=='immutable_gameplay_config':
            gameplay_validator.validate(doc); gameplay_count+=1
    for e in spawn_idx:
        spawn_validator.validate(load(a.canonical_spawns/e['json_path']))
    for profile in ('client','server'):
        map_validator.validate(load(a.canonical_maps/f'{profile}.maps.json'))
    loc_index=load(a.canonical_text/'localization'/'index.json')
    for entry in loc_index['catalogs']:
        localization_validator.validate(load(a.canonical_text/entry['path']))
    print(json.dumps({'status':'PASS','canonical_text':len(text_idx),'stage_spawns':len(spawn_idx),'map_catalogs':2,'localization_catalogs':len(loc_index['catalogs']),'definition_families':family_counts,'gameplay_configs':gameplay_count}))
if __name__=='__main__':main()
