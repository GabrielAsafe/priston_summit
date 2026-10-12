#!/usr/bin/env python3
"""Prepare a temporary, page-based cooked-model gallery for maps 0/1.

Uses verified binary staging directly (read-only) and never mutates the
canonical map, server, assets, or Git index. This is a source-level showroom,
not a runtime debug switch or a new log facility. Restoring the source requires
rebuilding the client; generated DLLs and caches are never touched.
"""
from __future__ import annotations
import argparse
import hashlib
import json
import os
from pathlib import Path, PurePosixPath
import shutil
import sys

SRC_REL = Path('SourceCode/SrcGame/src/platform/windows/application/Winmain.cpp')
HDR_REL = Path('SourceCode/SrcGame/src/platform/windows/application/SummitCookedGallery287.inl')
MARK = '// SUMMIT_COOKED_GALLERY_287'
INCLUDE = '#include "SummitCookedGallery287.inl" ' + MARK
BEFORE = 'void smPlayD3D(int x, int y, int z, int ax, int ay, int az)\n{'
ANCHOR_WORLD = '\tDrawPat3D_Alpha();'
ANCHOR_TEXT = '\tDrawPat2D(x, y, z, ax, ay, az);'
RENDER = '\tSummitCookedGallery287Draw(x, y, z, ax, ay, az); ' + MARK
OVERLAY = '\tSummitCookedGallery287Labels(); ' + MARK
# Only the two audited Winmain snapshots are accepted. Unknown user edits
# require re-audit, not a blind injection into current gameplay code.
ALLOWED_WINMAIN_SHA256 = {
    'de12fa015a940883e4067ad750ef0e63e3e8abc47ee66f7ec365b25d81af01b8',
    '8d2c1182eea5adb333228440aff01f09b992e98060302b435f019e42e51bf935',
}

def digest(p: Path) -> str:
    h = hashlib.sha256()
    with p.open('rb') as f:
        for chunk in iter(lambda: f.read(1024 * 1024), b''):
            h.update(chunk)
    return h.hexdigest()

def safe_dir(root: Path, p: Path) -> Path:
    if not p.is_absolute():
        p = root / p
    p = p.absolute()
    for q in (p, *p.parents):
        if q.is_symlink():
            raise ValueError('symlink path rejected: ' + str(q))
    return p.resolve(strict=True)

def literal(s: str) -> str:
    return '"' + s.replace('\\', '\\\\').replace('"', '\\"') + '"'

def validate_manifest(stage: Path) -> tuple[dict, list[dict]]:
    manifest = json.loads((stage / 'manifest.json').read_text(encoding='utf-8'))
    rows = manifest.get('artifacts')
    if not manifest.get('complete') or not isinstance(rows, list) or len(rows) != 134:
        raise ValueError('expected complete 134-artifact manifest')
    paths = set()
    out = []
    for item in rows:
        p = item.get('cache')
        if not isinstance(p, str) or '\\' in p:
            raise ValueError('unsafe artifact name')
        parts = PurePosixPath(p)
        if parts.is_absolute() or not p or '..' in parts.parts or '.' in parts.parts:
            raise ValueError('unsafe artifact path')
        if parts.suffix.lower() not in ('.smd', '.smb'):
            raise ValueError('unknown model type')
        if p.casefold() in paths:
            raise ValueError('duplicate case-insensitive model name')
        paths.add(p.casefold())
        target = stage.joinpath(*parts.parts)
        if not target.is_file() or target.is_symlink():
            raise ValueError('missing/untrusted model: ' + p)
        if digest(target) != item.get('sha256'):
            raise ValueError('staged model hash differs: ' + p)
        out.append({'cache': p, 'source': item['source'], 'sha256': item['sha256']})
    if not (sum(x['cache'].lower().endswith('.smd') for x in out) > 0 and sum(x['cache'].lower().endswith('.smb') for x in out) > 0):
        raise ValueError('no mixed mesh/bone catalog')
    out.sort(key=lambda row: (row['source'].casefold(), row['cache'].casefold()))
    return manifest, out

def header(rows: list[dict], relative_stage: str) -> str:
    # Path anchored to working directory cliente; only root/sibling staging supported.
    prefix = '..\\bugfixes\\' + relative_stage + '\\'
    cpp_rows = []
    for row in rows:
        relative = row['cache'].replace('/', '\\')
        kind = 'true' if relative.lower().endswith('.smb') else 'false'
        cpp_rows.append('    {' + literal(prefix + relative) + ', ' + literal(row['cache']) + ', ' + kind + '},')
    return '''// AUTO-GENERATED SUMMIT GALLERY 287; temporary preview build ONLY.
// Staged SMD/SMB binaries are never modified. No world spawns or canonical changes.
// Bone-only SMB entries are READBACK checks, not visible 3D meshes.
namespace {
struct SummitGalleryRow287 { const char* path; const char* title; bool bone; };
static const SummitGalleryRow287 kSummitGallery287[] = {
''' + '\n'.join(cpp_rows) + '''
};
static const int kSummitGalleryCount287 = sizeof(kSummitGallery287) / sizeof(kSummitGallery287[0]);
static const int kSummitGalleryPageSize287 = 8;
static int gSummitGalleryPage287 = 0;
static int gSummitGalleryLoadedPage287 = -1;
static smPAT3D* gSummitGalleryPatterns287[kSummitGalleryPageSize287] = {};
static int gSummitGalleryState287[kSummitGalleryPageSize287] = {};
static int gSummitGalleryLoadCursor287 = 0;
static bool gSummitGalleryWasNext287 = false;
static bool gSummitGalleryWasPrev287 = false;
static bool gSummitGalleryInMap287 = false;
static void SummitCookedGallery287Unload() {
    for (int i = 0; i < kSummitGalleryPageSize287; ++i) {
        delete gSummitGalleryPatterns287[i];
        gSummitGalleryPatterns287[i] = 0;
        gSummitGalleryState287[i] = 0;
    }
    gSummitGalleryLoadCursor287 = 0;
    gSummitGalleryLoadedPage287 = -1;
}
static void SummitCookedGallery287Draw(int cx, int cy, int cz, int ax, int ay, int az) {
    gSummitGalleryInMap287 = lpCurPlayer && lpCurPlayer->OnStageField >= 0 &&
        lpCurPlayer->OnStageField < 2 && StageField[lpCurPlayer->OnStageField] &&
        (StageField[lpCurPlayer->OnStageField]->FieldCode == 0 ||
         StageField[lpCurPlayer->OnStageField]->FieldCode == 1);
    if (!gSummitGalleryInMap287) {
        if (gSummitGalleryLoadedPage287 >= 0) SummitCookedGallery287Unload();
        return;
    }
    const bool modifier = (GetAsyncKeyState(VK_CONTROL) & 0x8000) &&
                          (GetAsyncKeyState(VK_SHIFT) & 0x8000);
    const bool next = modifier && (GetAsyncKeyState(VK_NEXT) & 0x8000);
    const bool prev = modifier && (GetAsyncKeyState(VK_PRIOR) & 0x8000);
    const int pageCount = (kSummitGalleryCount287 + kSummitGalleryPageSize287 - 1) / kSummitGalleryPageSize287;
    if (next && !gSummitGalleryWasNext287 && gSummitGalleryPage287 + 1 < pageCount) ++gSummitGalleryPage287;
    if (prev && !gSummitGalleryWasPrev287 && gSummitGalleryPage287 > 0) --gSummitGalleryPage287;
    gSummitGalleryWasNext287 = next;
    gSummitGalleryWasPrev287 = prev;
    if (gSummitGalleryLoadedPage287 != gSummitGalleryPage287) {
        SummitCookedGallery287Unload();
        gSummitGalleryLoadedPage287 = gSummitGalleryPage287;
    }
    // At most ONE binary load per frame to avoid a 134-file stall.
    if (gSummitGalleryLoadCursor287 < kSummitGalleryPageSize287) {
        const int slot = gSummitGalleryLoadCursor287++;
        const int index = gSummitGalleryPage287 * kSummitGalleryPageSize287 + slot;
        if (index < kSummitGalleryCount287) {
            const SummitGalleryRow287 &entry = kSummitGallery287[index];
            // Read cooked binaries DIRECTLY. Never call smASE_Read/ASE conversion.
            smPAT3D* obj = entry.bone
                ? smReadModel_Bip(const_cast<char*>(entry.path))
                : smReadModel(const_cast<char*>(entry.path), 0);
            if (!obj) gSummitGalleryState287[slot] = -1;
            else if (entry.bone) {
                gSummitGalleryState287[slot] = 2; // bone readback; no visible mesh
                delete obj;
            } else if (obj->nObj3d <= 0) {
                gSummitGalleryState287[slot] = 3; // structurally present, no geometry
                delete obj;
            } else if (obj->Bound > 4096) {
                gSummitGalleryState287[slot] = 4; // giant sky/scene mesh: do not occlude gameplay
                delete obj;
            } else {
                gSummitGalleryPatterns287[slot] = obj;
                gSummitGalleryState287[slot] = 1;
            }
        }
    }
    POINT3D camera = { cx, cy, cz };
    POINT3D angle = { ax, ay, az };
    POINT3D orientation = { 0, 0, 0 };
    // Preserve D3D9 state so the temporary viewer cannot leak basic depth/blend
    // switches into the normal effects or HUD passes.
    DWORD savedZ = TRUE, savedZWrite = TRUE, savedAlpha = FALSE;
    DWORD savedSrc = D3DBLEND_ONE, savedDest = D3DBLEND_ZERO;
    d3ddev->GetRenderState(D3DRS_ZENABLE, &savedZ);
    d3ddev->GetRenderState(D3DRS_ZWRITEENABLE, &savedZWrite);
    d3ddev->GetRenderState(D3DRS_ALPHABLENDENABLE, &savedAlpha);
    d3ddev->GetRenderState(D3DRS_SRCBLEND, &savedSrc);
    d3ddev->GetRenderState(D3DRS_DESTBLEND, &savedDest);
    // Display four models per row around the player, not server-side NPCs.
    // Player can rotate to observe all stands; positions follow character.
    for (int i = 0; i < kSummitGalleryPageSize287; ++i) {
        smPAT3D* obj = gSummitGalleryPatterns287[i];
        if (!obj) continue;
        POINT3D pos = {
            lpCurPlayer->pX + ((i % 4) * 220 - 330) * fONE,
            lpCurPlayer->pY,
            lpCurPlayer->pZ + ((i / 4) * 240 + 300) * fONE
        };
        if (smGameStage[lpCurPlayer->OnStageField]) {
            const int ground = smGameStage[lpCurPlayer->OnStageField]->GetHeight(pos.x, pos.z);
            if (ground > CLIP_OUT) pos.y = ground;
        }
        obj->Frame = 0;
        obj->SetPosi(&pos, &orientation);
        smRender.Init();
        obj->RenderD3D(&camera, &angle);
    }
    d3ddev->SetRenderState(D3DRS_ZENABLE, savedZ);
    d3ddev->SetRenderState(D3DRS_ZWRITEENABLE, savedZWrite);
    d3ddev->SetRenderState(D3DRS_ALPHABLENDENABLE, savedAlpha);
    d3ddev->SetRenderState(D3DRS_SRCBLEND, savedSrc);
    d3ddev->SetRenderState(D3DRS_DESTBLEND, savedDest);
    smRender.Color_R = 0;
    smRender.Color_G = 0;
    smRender.Color_B = 0;
    smRender.Color_A = 0;
}
static void SummitCookedGallery287Labels() {
    if (!gSummitGalleryInMap287) return;
    char text[270];
    const int pages = (kSummitGalleryCount287 + kSummitGalleryPageSize287 - 1) / kSummitGalleryPageSize287;
    sprintf_s(text, sizeof(text), "COOKED GALLERY 287 page %d/%d | Ctrl+Shift+PgUp/PgDn", gSummitGalleryPage287 + 1, pages);
    dsTextLineOut(10, 265, text, lstrlen(text));
    for (int slot = 0; slot < kSummitGalleryPageSize287; ++slot) {
        const int index = gSummitGalleryPage287 * kSummitGalleryPageSize287 + slot;
        if (index >= kSummitGalleryCount287) break;
        const char* state = "loading";
        switch (gSummitGalleryState287[slot]) {
            case -1: state = "READ_FAIL"; break;
            case 1: state = "MESH"; break;
            case 2: state = "SMB_READ_OK"; break;
            case 3: state = "EMPTY_MESH"; break;
            case 4: state = "TOO_LARGE"; break;
        }
        sprintf_s(text, sizeof(text), "%03d [%s] %.170s", index + 1, state, kSummitGallery287[index].title);
        dsTextLineOut(10, 281 + 15 * slot, text, lstrlen(text));
    }
}
} // anonymous namespace
'''

def patched_source(source: str) -> str:
    if MARK in source:
        raise ValueError('gallery already inserted; restore before re-preparing')
    if source.count(BEFORE) != 1 or source.count(ANCHOR_WORLD) != 1 or source.count(ANCHOR_TEXT) != 1:
        raise ValueError('Winmain.cpp layout differs; no changes made')
    source = source.replace(BEFORE, INCLUDE + '\n\n' + BEFORE, 1)
    source = source.replace(ANCHOR_WORLD, ANCHOR_WORLD + '\n' + RENDER, 1)
    source = source.replace(ANCHOR_TEXT, ANCHOR_TEXT + '\n' + OVERLAY, 1)
    return source

def checklist(rows: list[dict]) -> str:
    lines = ['# SUMMIT — Galeria cooked 287 (mapas 0/1)', '',
             'O índice é o número mostrado no ecrã. MESH é apenas leitura/desenho; requer inspeção visual.',
             'SMB_READ_OK valida a abertura, não certifica uma animação ligada à malha.',
             'EMPTY_MESH, TOO_LARGE e READ_FAIL requerem revisão; não marcar como aprovação visual.',
             '', '| Nº | Página | Tipo | Fonte ASE | Cache | Avaliação visual |',
             '|---:|---:|---|---|---|---|']
    for index, row in enumerate(rows):
        typ = 'SMB (esqueleto)' if row['cache'].lower().endswith('.smb') else 'SMD (malha)'
        lines.append(f'| {index+1} | {index//8+1} | {typ} | `{row["source"]}` | `{row["cache"]}` | [ ] OK / [ ] FALHA / [ ] N/A |')
    lines += ['', '## Limites', '',
              '- Não são 134 spawns nem 134 modelos visíveis. Esqueletos isolados não são malhas.',
              '- Os modelos continuam nos respetivos caminhos cooked de staging; nenhum NPC ou mapa é alterado.',
              '- A galeria é temporária; restaurar source e recompilar o cliente depois da inspeção.',
              '- Uma malha carregada não comprova animação/efeito funcional em gameplay.', '']
    return '\n'.join(lines)

def prepare(root: Path, stage: Path, client: Path, server: Path, exe: Path, backup: Path) -> None:
    import verify_native_model_stage_267 as verifier
    from cook_native_models_259 import no_symlink_ancestors
    for p in [root, stage, client, server, exe, backup]:
        no_symlink_ancestors(p)
    if stage != root / 'bugfixes' / 'SUMMIT_NATIVE_MODELS_INCREMENTAL_263':
        raise ValueError('gallery requires standard trusted sibling staging path')
    if client != root / 'cliente' or not client.is_dir():
        raise ValueError('expected client in project root')
    if not (root / 'SourceCode').is_dir():
        raise ValueError('missing SourceCode')
    # Fail closed on current binary identity before compiling the temporary viewer.
    verified = verifier.verify_staging(client, server, stage, allow_partial=False, client_exe=exe)
    if verified['verified'] != 134 or verified['missing'] or not verified['complete']:
        raise ValueError('full native gate 267 required before gallery preparation')
    _, rows = validate_manifest(stage)
    target = root / SRC_REL
    hdr = root / HDR_REL
    if target.is_symlink() or hdr.exists() or hdr.is_symlink() or backup.exists() or backup.is_symlink():
        raise ValueError('output occupied or symlink')
    if target.is_symlink() or not target.is_file():
        raise ValueError('Winmain.cpp missing or symlink')
    original = target.read_bytes()
    if hashlib.sha256(original).hexdigest() not in ALLOWED_WINMAIN_SHA256:
        raise ValueError('Winmain.cpp SHA-256 differs from audited versions; no code edits')
    new = patched_source(original.decode('utf-8'))
    # Read Git index, but do not write to it; protect staged Winmain changes.
    import subprocess
    for git_root, tracked in ((root, SRC_REL), (root / 'SourceCode', SRC_REL.relative_to('SourceCode'))):
        if (git_root / '.git').exists():
            git = subprocess.run(['git', 'diff', '--cached', '--name-only', '--', tracked.as_posix()],
                                 cwd=git_root, capture_output=True, text=True)
            if git.returncode or git.stdout.strip():
                raise ValueError('Winmain has staged changes or Git preflight failed')
    backup.mkdir(parents=True)
    (backup / 'Winmain.cpp.original').write_bytes(original)
    (backup / 'manifest_sha256.txt').write_text(digest(stage / 'manifest.json') + '\n')
    hdr.write_text(header(rows, stage.name), encoding='utf-8')
    (backup / 'CHECKLIST_134.md').write_text(checklist(rows), encoding='utf-8')
    target.write_text(new, encoding='utf-8')
    receipt = {'format': 'SUMMIT_GALLERY_287', 'before': hashlib.sha256(original).hexdigest(),
               'after': digest(target), 'header': digest(hdr), 'staging_manifest': digest(stage / 'manifest.json'),
               'rows': len(rows)}
    (backup / 'receipt.json').write_text(json.dumps(receipt, indent=2) + '\n')
    print('GALLERY287=PREPARED entries=134 smd=' + str(sum(r['cache'].endswith('.smd') for r in rows)) +
          ' smb=' + str(sum(r['cache'].endswith('.smb') for r in rows)) + ' pages=17 client_writes=0')
    print('SOURCE_BACKUP=' + str(backup))

def restore(root: Path, backup: Path) -> None:
    source = root / SRC_REL
    hdr = root / HDR_REL
    receipt = json.loads((backup / 'receipt.json').read_text())
    original = backup / 'Winmain.cpp.original'
    if source.is_symlink() or hdr.is_symlink() or original.is_symlink():
        raise ValueError('symlink introduced; refusing restore')
    if receipt.get('format') != 'SUMMIT_GALLERY_287' or digest(source) != receipt['after'] or digest(hdr) != receipt['header'] or digest(original) != receipt['before']:
        raise ValueError('source/header changed since prepare; refusing unsafe restore')
    source.write_bytes(original.read_bytes())
    hdr.unlink()
    print('GALLERY287=SOURCE_RESTORED requires_client_rebuild=1')

def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('command', choices=['prepare', 'restore'])
    parser.add_argument('--repo', type=Path, default=Path('.'))
    parser.add_argument('--stage', type=Path, default=Path('bugfixes/SUMMIT_NATIVE_MODELS_INCREMENTAL_263'))
    parser.add_argument('--client', type=Path, default=Path('cliente'))
    parser.add_argument('--server', type=Path, default=Path('Servidor/Server.exe'))
    parser.add_argument('--client-exe', type=Path, default=Path('SourceCode/dist_mingw_client_i686/Game.exe'))
    parser.add_argument('--backup-dir', type=Path, default=Path('bugfixes/SUMMIT_GALLERY_287_SOURCE_BACKUP'))
    args = parser.parse_args()
    try:
        root = args.repo.resolve(strict=True)
        stage = (root / args.stage).resolve(strict=True)
        client = (root / args.client).resolve(strict=True)
        server = (root / args.server).resolve(strict=True)
        exe = (root / args.client_exe).resolve(strict=True)
        backup = (root / args.backup_dir).absolute()
        if backup != root / 'bugfixes' / 'SUMMIT_GALLERY_287_SOURCE_BACKUP':
            raise ValueError('backup outside expected protected path')
        if args.command == 'prepare':
            prepare(root, stage, client, server, exe, backup)
        else:
            restore(root, backup)
        return 0
    except (ValueError, OSError, KeyError, json.JSONDecodeError) as e:
        print('GALLERY287=FAIL_CLOSED ' + str(e))
        return 2

if __name__ == '__main__':
    sys.exit(main())
