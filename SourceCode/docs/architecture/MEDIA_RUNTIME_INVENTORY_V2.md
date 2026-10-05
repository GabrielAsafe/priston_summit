# Media Runtime Inventory V2

This inventory records the remaining native presentation/media ownership boundaries. It is descriptive: it does not move codec, device, process, or IPC ownership into gameplay code.

| Subsystem | Runtime owner | Native responsibility | V2 boundary | Retirement gate |
|---|---|---|---|---|
| Audio playback | `SrcGame/src/systems/audio/playback/legacy/Wave.cpp` | wave/device playback and graph lifecycle | `Shared/presentation/v2/AudioService.h` | native playback parity and device teardown proven |
| Audio effects | `SrcGame/src/systems/audio/effects/legacy/effectsnd.cpp` | effect playback/device calls | `Shared/presentation/v2/AudioService.h` | effect routing parity proven |
| Voice | `SrcGame/src/systems/audio/voice/legacy_client/Process.cpp` + `audio/voice/ipc/memmap.cpp` | external process and shared-memory IPC | presentation/media boundary | reconnect/cleanup and `CProcess::Clear` proven |
| Video | `SrcGame/src/systems/media/video/ParkDXPlayer.cpp` | DirectShow/native graph and playback | presentation/media boundary | load/play/stop parity including `CParkPlayer::LoadPlayerFile` proven |

The machine-readable companion is `Shared/presentation/v2/MediaRuntimeInventory.h`. Legacy native owners remain adapters/shells until runtime parity is demonstrated; existence of the inventory alone does not authorize deletion.
