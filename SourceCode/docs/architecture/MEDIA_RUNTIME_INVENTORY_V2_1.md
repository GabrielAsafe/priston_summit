# Media runtime inventory (V2 boundary)

This inventory is deliberately descriptive. It establishes ownership boundaries before any codec, DSP, DirectShow, or voice-IPC rewrite.

| Subsystem | Current legacy owner | Native/resource ownership | Existing V2 boundary | Next safe slice |
|---|---|---|---|---|
| Audio playback | `SrcGame/src/systems/audio/playback/legacy/Wave.cpp`, `Dxwav.cpp` | DirectSound/audio buffers and playback state | `presentation::IAudioService` covers selected one-shot/entity sounds | expand adapter coverage only when a concrete caller migrates |
| Audio effects | `SrcGame/src/systems/audio/effects/legacy/effectsnd.cpp` | effect catalog + playback calls | partial through `LegacyAudioAdapter` | separate effect catalog IDs from playback device calls |
| Voice | `SrcGame/src/systems/audio/voice/legacy_client/Process.cpp` + `audio/voice/ipc/memmap.*` | external `VoiceChat` window/process + shared-memory IPC | none before this inventory | first migrate lifecycle/resource ownership, then control messages |
| Video | `SrcGame/src/systems/media/video/ParkDXPlayer.cpp` | COM/DirectShow graph and interfaces | none before this inventory | encapsulate graph lifecycle without changing rendering/codec behavior |

## Invariants

- Do not move codec/DSP implementation into GREEN services merely to rename files.
- Native handles, COM interfaces, DirectSound buffers, process handles, HWNDs, and shared-memory maps remain adapter/runtime concerns.
- GREEN may own intent and lifecycle state, but not Win32/DirectShow types.
- Voice control-plane migration must preserve the current external-process/shared-memory protocol until explicitly versioned.
- Existing `IAudioService` remains the narrow playback command boundary; this patch does not broaden its semantics.

## Known lifecycle risks discovered during inventory

- Voice `CProcess::Clear()` currently dereferences `m_cMemMap` before checking it for null and does not reset every owned handle/state after cleanup.
- Video `CParkPlayer::LoadPlayerFile()` overwrites `HRESULT` repeatedly and reports success unconditionally; graph/interface cleanup is manual in `StopPlayer()`.

Those findings are intentionally not silently fixed in this inventory patch. They define the next implementation slices so behavior changes remain reviewable.
