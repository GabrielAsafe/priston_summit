#!/usr/bin/env python3
from pathlib import Path
root=Path(__file__).resolve().parents[1]
h=(root/'SrcGame/src/systems/audio/voice/legacy_client/Process.h').read_text(errors='replace')
cpp=(root/'SrcGame/src/systems/audio/voice/legacy_client/Process.cpp').read_text(errors='replace')
if 'VoiceSessionLifecycle m_voiceLifecycle' not in h:
    raise SystemExit('voice legacy adapter lacks V2 lifecycle state')
for req in ('m_voiceLifecycle.MarkRemoteReady()', 'm_voiceLifecycle.MarkIpcReady()', 'm_voiceLifecycle.CanUseIpc()', 'm_voiceLifecycle.Close()'):
    if req not in cpp: raise SystemExit(f'voice lifecycle transition missing: {req}')
clear=cpp.split('VOID CProcess::Clear()',1)[1].split('void PacketParsing()',1)[0]
if 'm_cMemMap->Clear();' in clear and clear.index('m_cMemMap->Clear();') < clear.index('if(m_cMemMap != NULL)'):
    raise SystemExit('voice clear still dereferences memory map before null check')
if 'm_cMemMap = NULL;' not in clear or 'm_bIsMemMap = FALSE;' not in clear:
    raise SystemExit('voice clear does not reset IPC ownership')
print('voice session lifecycle validator: PASS')
