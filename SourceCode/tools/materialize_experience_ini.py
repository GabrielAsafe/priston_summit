#!/usr/bin/env python3
import argparse,json
from pathlib import Path

def main():
    ap=argparse.ArgumentParser(description='Materialize legacy ExpManager.ini from canonical experience JSON.')
    ap.add_argument('canonical_json',type=Path)
    ap.add_argument('output_ini',type=Path)
    a=ap.parse_args()
    d=json.loads(a.canonical_json.read_text(encoding='utf-8'))
    party=d['party']; shared={int(k):int(v) for k,v in party.get('member_share_percent',{}).items()}
    fallback={int(k):v for k,v in party.get('legacy_fallback_7_8',{}).items() if str(k).isdigit()}
    lines=['; generated from summit.experience-balance/v1','[LevelParty]',f"Diferenca = {int(party['maximum_level_difference'])}",'','[PartyRate]']
    for n in range(2,9):
        if n in fallback:
            killer=int(fallback[n]['killer_percent']); member=int(fallback[n]['member_percent'])
        else:
            killer=member=shared[n]
        # New runtime keys support asymmetric party shares.
        lines += [f'{n} Killer={killer}',f'{n} Membro={member}']
        # Retain the legacy common key for older binaries/tools.
        if killer==member: lines.append(f'{n} Membros={member}')
    lines += ['','[ExpConfig]']
    vals=d['level_multiplier']['values']
    for level in sorted((int(k) for k in vals)):
        lines.append(f'Nv[{level}]={int(vals[str(level)])}')
    a.output_ini.parent.mkdir(parents=True,exist_ok=True)
    a.output_ini.write_text('\n'.join(lines)+'\n',encoding='utf-8')
    print(a.output_ini)
if __name__=='__main__': main()
