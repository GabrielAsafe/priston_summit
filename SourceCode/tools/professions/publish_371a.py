#!/usr/bin/env python3
"""SUMMIT 371A: publish canonical Miner/Lumberjack resources & placed visual nodes.
Only works on the ACTUAL summit/ SourceCode and Servidor database. Never reads OLD.
Game Server MUST be stopped before 'apply'. No physical DAT writes here.
"""
import argparse
import hashlib
import json
import os
from pathlib import Path
import re
import sqlite3
import sys
from datetime import datetime, timezone

HERE = Path(__file__).resolve()
KINDS = {'Miner': (0,4), 'Lumberjack': (2,3)}
NODE_HEADER = 'SourceCode/Shared/gameplay/gathering/v2/legacy/VisualNodeCombatConfig.generated.h'
RESOURCE_KEYS = {'raw_ore_crystalline','raw_ore_energetic','raw_ore_deep','raw_ore_arcane','raw_ore_runic','raw_ore_primordial', 'common_logs','resistant_logs','crystalline_wood','arcane_wood'}

def sha(b): return hashlib.sha256(b).hexdigest()
def fail(message): raise ValueError(message)
def load(path): return json.loads(path.read_text(encoding='utf-8'))
def paths(root):
    base=root/'SourceCode/Content/canonical/professions_v2'
    return (base/'ecosystem_366/professions_ecosystem_366.json',base/'spawn_placements_371.json',root/NODE_HEADER,root/'Servidor/Database/summit.sqlite3')

def build_plan(root):
    ep, pp, hp, dbp=paths(root)
    if not ep.is_file() or not pp.is_file() or not hp.is_file(): fail('SourceCode incompleta: 366A/367A, placements 371A ou header visual em falta')
    eco, placement = load(ep), load(pp)
    if eco.get('schema')!='summit.professions-ecosystem/v1' or placement.get('schema')!='summit.professions-visual-placements/v1': fail('schema JSON inesperado')
    families={}
    resources={r['key'] for r in eco['resources']}
    for group,prof,base in [('vein_families','Miner',3711001),('tree_families','Lumberjack',3712001)]:
        expected=6 if prof=='Miner' else 4
        if len(eco.get(group,[])) != expected: fail(f'{group}: quantidade esperada {expected}')
        for i,f in enumerate(eco[group]):
            if f['family_id'] in families or f['profession']!=prof: fail('identidade de familia duplicada/invalida')
            if prof=='Miner': primary=f['raw_resource_key']; quantities=f['harvest']['raw_resource_count_draft']
            else: primary=f['harvest']['drops_draft'][0]['key']; quantities=f['harvest']['drops_draft'][0]['amount']
            if primary not in RESOURCE_KEYS or primary not in resources: fail('recurso primario desconhecido: '+primary)
            if (not isinstance(quantities,list) or len(quantities)!=2 or
                not all(isinstance(n,int) and 1<=n<=25 for n in quantities) or quantities[0]>quantities[1]): fail('quantidade invalida '+primary)
            level=f['minimum_profession_level']
            if not isinstance(level,int) or level<0 or level>50: fail('nivel invalido')
            family={'family_id':f['family_id'],'primary':primary,'resource_id':base+i,'min_level':level,'min':quantities[0],'max':quantities[1], 'profession':prof,'kind':KINDS[prof][0],'prof_id':KINDS[prof][1], 'template':f['visual']['monster_template'],'model':f['visual']['model_path']}
            if not family['model'].startswith('char/monster/') or '..' in family['model'] or not family['model'].endswith('.ini'):fail('caminho de modelo inseguro')
            families[f['family_id']]=family
    nodes=[]; seen=set()
    for n in placement['nodes']:
        nid=n.get('node_id'); fid=n.get('family_id')
        if type(nid)!=int or nid<920000 or nid>9999999 or nid in seen: fail('node_id duplicado/reservado/invalido')
        seen.add(nid)
        if fid not in families: fail('familia de node desconhecida: '+str(fid))
        if type(n.get('enabled'))!=bool: fail('enabled deve ser boolean')
        if n['enabled']:
            for field in ('map_id','x','y','z','interaction_radius','respawn_ms'):
                if type(n.get(field))!=int: fail(f'node {nid}: {field} tem de ser inteiro')
            if not 0<=n['map_id']<59 or not 0<n['interaction_radius']<=2048 or not 3000<=n['respawn_ms']<=3600000: fail('mapa/radius/respawn fora dos limites')
            if abs(n['x'])>4000000 or abs(n['y'])>4000000 or abs(n['z'])>4000000: fail('coordenadas fora dos limites')
            if not (n['x'] and n['z']): fail('coordenadas nao podem ser zero')
            nodes.append({**n,**families[fid]})
    if len(families)!=10: fail('10 familias exigidas')
    return families,nodes,hp,dbp

def cpp_string(text):
    # Legacy monster name table uses Windows ANSI; escape non-ASCII as byte sequences.
    result='"'
    for b in text.encode('cp1252'):
        if b in (34,92): result+='\\'+chr(b)
        elif 32<=b<=126: result+=chr(b)
        else: result+=f'\\{b:03o}'
    return result+'"'

def render_header(header, nodes, preview, restore_preview=False):
    text=header.decode('latin-1')
    spec_re=re.compile(r'(static const VisualNodeSpec kVisualNodes\[\] = \{\s*\n)(.*?)(\n\};)',re.S)
    path_re=re.compile(r'(static const char\* const kVisualModelPaths\[\] = \{\s*\n)(.*?)(\n\};)',re.S)
    sm=spec_re.search(text); pm=path_re.search(text)
    if not sm or not pm: fail('header visual inesperado: estrutura desconhecida')
    ids_in_header=[int(x) for x in re.findall(r'\{(\d+)u,',sm.group(2))]
    if any(i not in (910001,910002,910003,910004,920001,920002,920003,920004,920005,920006,920007,920008,920009,920010) for i in ids_in_header):
        fail('header tem nodes customizados fora do dominio 371A; nao reescrever')
    old_specs=[]
    for nid in range(910001,910005):
        matches=[line.strip().rstrip(',') for line in sm.group(2).splitlines() if re.search(r'\{'+str(nid)+r'u,',line)]
        if len(matches)!=1:fail('header visual: base 4 nodes nao encontrada')
        row=matches[0]
        if nid==910001 and restore_preview:
            if "\"raw_ore_crystalline\"" in row: row=row.replace("\"raw_ore_crystalline\"","\"iron_ore\"",1)
            elif "\"iron_ore\"" not in row:fail("node910001 recurso inesperado")
        if nid==910003 and restore_preview:
            if "\"common_logs\"" in row: row=row.replace("\"common_logs\"","\"timber\"",1)
            elif "\"timber\"" not in row:fail("node910003 recurso inesperado")
        if nid==910001 and preview:
            if '"iron_ore"' in row: row=row.replace('"iron_ore"','"raw_ore_crystalline"',1)
            elif '"raw_ore_crystalline"' not in row: fail('node910001 nao usa o recurso esperado')
        if nid==910003 and preview:
            if '"timber"' in row: row=row.replace('"timber"','"common_logs"',1)
            elif '"common_logs"' not in row:fail('node910003 nao usa recurso esperado')
        old_specs.append(row)
    old_paths=[]
    for line in pm.group(2).splitlines():
        stripped=line.strip().rstrip(',')
        if stripped.startswith('"') and stripped.endswith('"'):
            old_paths.append(stripped)
    if len(old_paths)<4:fail('caminhos de modelo existentes incompletos')
    original_paths=old_paths[:4]
    if len(ids_in_header)!=len(old_paths): fail('header visual: numero de modelos nao corresponde aos nodes')
    if not nodes and not preview and not restore_preview and len(ids_in_header)==4: return header
    new_specs=[]; new_paths=[]
    for n in sorted(nodes,key=lambda i:i['node_id']):
        kind='kMining' if n['profession']=='Miner' else 'kWoodcutting'
        new_specs.append('    {'+f'{n["node_id"]}u, {cpp_string(n["primary"])}, summit_professions::{kind}, {n["map_id"]}, {n["x"]}, {n["y"]}, {n["z"]}, {cpp_string(n["template"])}, "", 0u, 0u, {n["respawn_ms"]}u'+'}')
        new_paths.append('    '+cpp_string(n['model']))
    new_spec_body=',\n'.join('    '+row for row in old_specs)+ (',\n'+',\n'.join(new_specs) if new_specs else '')
    new_path_body=',\n'.join('    '+row for row in original_paths)+ (',\n'+',\n'.join(new_paths) if new_paths else '')
    text=spec_re.sub(lambda m: m.group(1)+new_spec_body+m.group(3),text,count=1)
    text=path_re.sub(lambda m: m.group(1)+new_path_body+m.group(3),text,count=1)
    return text.encode('latin-1')

def verify_db(dbp):
    if not dbp.is_file():fail('SQLite principal nao encontrada: '+str(dbp))
    db=sqlite3.connect(f'file:{dbp}?mode=ro',uri=True)
    try:
        version=db.execute('PRAGMA user_version').fetchone()[0]
        for table in ('ServerDB__GatheringResourceDefinition','ServerDB__GatheringNodeDefinition'):
            if not db.execute("SELECT 1 FROM sqlite_master WHERE type='table' AND name=?",(table,)).fetchone():fail('tabela de runtime em falta: '+table)
        if version<48:fail(f'SQLite version {version}, necessario 48+')
    finally:db.close()
    return version

def publish(root, preview, inspect_only, restore_preview=False):
    fam,nodes,hp,dbp=build_plan(root)
    original=hp.read_bytes()
    staged=render_header(original,nodes,preview,restore_preview)
    if inspect_only:
        print(f'SUMMIT371A_INSPECT_PASS families={len(fam)} active_new_nodes={len(nodes)} preview_map58={int(preview)} changed_header={int(staged!=original)}')
        if dbp.exists():print('SQLITE_VERSION=',verify_db(dbp))
        return
    verify_db(dbp)
    # Save SQLite as a safe online-format backup (including WAL) before write.
    # Do NOT use this backup to undo once the game has been played.
    backupdir=root/'bugfixes/_backups_371a';backupdir.mkdir(parents=True,exist_ok=True)
    timestamp=datetime.now(timezone.utc).strftime('%Y%m%dT%H%M%S_%fZ')
    backup=backupdir/f'summit_antes_371a_{timestamp}.sqlite3'
    with sqlite3.connect(f'file:{dbp}?mode=ro',uri=True) as source, sqlite3.connect(backup) as target:source.backup(target)
    header_backup=backupdir/f'VisualNodeCombatConfig_{timestamp}.h'
    header_backup.write_bytes(original)
    db=sqlite3.connect(dbp,timeout=10)
    try:
        db.execute('PRAGMA busy_timeout=10000')
        db.execute('BEGIN IMMEDIATE')
        for f in fam.values():
            row=(f['resource_id'],f['primary'],f['kind'],f['prof_id'],f['min_level'],f['min'],f['max'],1)
            exists=db.execute('SELECT ResourceId,ResourceKey,Kind,RequiredProfession,RequiredLevel,MinimumYield,MaximumYield,Enabled FROM ServerDB__GatheringResourceDefinition WHERE ResourceId=? OR ResourceKey=?',(f['resource_id'],f['primary'])).fetchall()
            if exists and (len(exists)!=1 or tuple(exists[0])!=row):fail('conflito de recurso existente: '+f['primary'])
            if not exists: db.execute('INSERT INTO ServerDB__GatheringResourceDefinition(ResourceId,ResourceKey,Kind,RequiredProfession,RequiredLevel,MinimumYield,MaximumYield,Enabled) VALUES(?,?,?,?,?,?,?,?)',row)
        if preview or restore_preview:
            for nid,allowed,target in [(910001,1001,3711001),(910003,3001,3712001)]:
                actual=db.execute('SELECT ResourceId,Area FROM ServerDB__GatheringNodeDefinition WHERE NodeId=?',(nid,)).fetchone()
                if not actual or actual[1]!=58 or actual[0] not in (allowed,target): fail('node 58 inesperado: '+str(nid))
                db.execute('UPDATE ServerDB__GatheringNodeDefinition SET ResourceId=? WHERE NodeId=?',(allowed if restore_preview else target,nid))
        # A previously published node may be moved or disabled in JSON.
        # Only manage the ten reserved 371A IDs; never touch other nodes.
        _, placement_path, _, _ = paths(root)
        all_placements=load(placement_path)['nodes']
        for placed in all_placements:
            fid=placed['family_id']; family=fam[fid]; nid=placed['node_id']
            exists=db.execute('SELECT ResourceId FROM ServerDB__GatheringNodeDefinition WHERE NodeId=?',(nid,)).fetchone()
            if exists and exists[0]!=family['resource_id']:fail('node de outro sistema ocupa id reservado: '+str(nid))
            if not placed['enabled']:
                if exists:db.execute('UPDATE ServerDB__GatheringNodeDefinition SET Enabled=0 WHERE NodeId=?',(nid,))
                continue
            row=(nid,family['resource_id'],placed['map_id'],placed['x'],placed['y'],placed['z'],placed['interaction_radius'],placed['respawn_ms'],1)
            if exists:
                db.execute('UPDATE ServerDB__GatheringNodeDefinition SET Area=?,X=?,Y=?,Z=?,InteractionRadius=?,RespawnMilliseconds=?,Enabled=1 WHERE NodeId=?',
                           (placed['map_id'],placed['x'],placed['y'],placed['z'],placed['interaction_radius'],placed['respawn_ms'],nid))
            else:db.execute('INSERT INTO ServerDB__GatheringNodeDefinition(NodeId,ResourceId,Area,X,Y,Z,InteractionRadius,RespawnMilliseconds,Enabled) VALUES(?,?,?,?,?,?,?,?,?)',row)
        db.commit()
    except Exception:
        db.rollback(); raise
    finally:db.close()
    try:
        if staged!=original:
            temp=hp.with_name(hp.name+'.371a-tmp')
            temp.write_bytes(staged)
            os.replace(temp,hp)
    except Exception:
        print('ATENCAO: BD atualizada, mas header NAO. Servidor NAO DEVE ARRANCAR antes de recuperar. Backup:',backup,file=sys.stderr)
        raise
    print(f'SUMMIT371A_APPLIED resources=10 new_visual_nodes={len(nodes)} preview_map58={int(preview)} header_changed={int(staged!=original)}')
    print('DB_BACKUP=',backup)
    print('HEADER_BACKUP=',header_backup)
    print('REBUILD_REQUIRED=',int(staged!=original))

if __name__=='__main__':
    p=argparse.ArgumentParser(description=__doc__)
    p.add_argument('action',choices=('inspect','apply'))
    p.add_argument('--root',type=Path,default=Path('.'))
    p.add_argument('--restore-preview-map58',action='store_true',help='Restaurar iron_ore e timber em 910001/910003, sem apagar recursos SQLite')
    p.add_argument('--preview-map58',action='store_true',help='Opt-in: conservar models/coords e alterar apenas resourceKey dos nodes 910001 e 910003')
    a=p.parse_args()
    try:
        if a.preview_map58 and a.restore_preview_map58:fail('preview e restore sao mutuamente exclusivos')
        publish(a.root.resolve(),a.preview_map58,a.action=='inspect',a.restore_preview_map58)
    except (ValueError,OSError,sqlite3.Error,UnicodeError,KeyError,TypeError) as e:
        print('SUMMIT371A_ERROR:',e,file=sys.stderr);sys.exit(1)
