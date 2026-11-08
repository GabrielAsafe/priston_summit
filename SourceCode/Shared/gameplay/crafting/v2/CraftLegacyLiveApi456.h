#pragma once
#include "gameplay/crafting/v2/CraftLegacyLiveWire456.h"
// Bridge implemented once inside record_legacy_server.inl, used by the
// authenticated chat ingress, packet ingress and validated-save endpoint.
class rsPLAYINFO;
struct TRANS_RECORD_DATA;
namespace summit456l {
enum class BeginResult {Queued,Rejected,Busy,Replay,Unsupported};
BeginResult Begin(rsPLAYINFO* player,const char* recipe,const char* request);
void ReceiveReply(rsPLAYINFO* sender,const void* bytes,int length);
void ValidatedSave(rsPLAYINFO* player,const TRANS_RECORD_DATA* record);
}
