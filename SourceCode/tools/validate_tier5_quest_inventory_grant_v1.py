#!/usr/bin/env python3
from pathlib import Path
import sys
ROOT=Path(__file__).resolve().parents[1]
P=(ROOT/'SrcServer/src/systems/gameplay/quests/server/cQuestHandler.cpp').read_text(errors='replace')
def fail(m): print(f'tier5 quest inventory grant v1: FAIL: {m}',file=sys.stderr); raise SystemExit(1)
def main():
 section=P.split('bool cQUESTHANDLER::HandleTier5MonsterKill',1)
 if len(section)!=2: fail('HandleTier5MonsterKill missing')
 body=section[1].split('void cQUESTHANDLER::GetReward',1)[0]
 for bad in ('rsPutItemChecked(',):
  if bad in body: fail(f'Tier5 kill handler retains direct grant authority: {bad}')
 for tok in ('GrantTier5QuestItem(lpKiller, sinQT134)','GrantTier5QuestItem(player, sinQT135)','GrantTier5QuestItem(lpKiller, sinQT135)'):
  if tok not in body: fail(f'migrated Tier5 grant missing: {tok}')
 helper=P.split('bool GrantTier5QuestItem',1)
 if len(helper)!=2: fail('grant helper missing')
 helper=helper[1].split('bool cQUESTHANDLER::HandleTier5MonsterKill',1)[0]
 for tok in ('ServerLegacyInventoryRuntime().Grant(','gameplay::inventory::ItemGrant(','gameplay::PlayerId(','gameplay::ItemId('):
  if tok not in helper: fail(f'grant helper does not route through Inventory: {tok}')
 print('tier5 quest inventory grant v1: PASS (Tier5 quest item delivery routes through Inventory grant command)')
if __name__=='__main__': main()
