#!/usr/bin/env bash
set -u

OUT="audit_server_mingw_risks"
rm -rf "$OUT"
mkdir -p "$OUT"

echo "=== SummitTale Server MinGW Risk Audit ===" | tee "$OUT/summary.txt"
date | tee -a "$OUT/summary.txt"

echo
echo "===== 1. INCLUDE SkillSub QUEBRADO ====="
grep -RIn 'Skills/SkillSub.h' SrcServer/src \
  | tee "$OUT/01_bad_skillsub_include.txt"

echo
echo "===== 2. BUGS /0 EM STRINGS ====="
grep -RIn "'/0'" SrcServer/src Shared Shared2 modules 2>/dev/null \
  | tee "$OUT/02_bad_slash_zero.txt"

echo
echo "===== 3. DIRECTX / DSOUND / D3D ====="
grep -RInE '#include[[:space:]]*[<"].*(d3d|d3dx|ddraw|dsound|dinput).*' SrcServer/src 2>/dev/null \
  | tee "$OUT/03_directx_includes.txt"

grep -RInE '\b(IDirect|D3D|D3DX|DDS|DSOUND|DirectSound|Direct3D|DirectDraw)\b' SrcServer/src 2>/dev/null \
  | tee "$OUT/03_directx_usage.txt" >/dev/null

echo
echo "===== 4. SAL MACROS / __in / __out ====="
grep -RInE '\b__(in|out|inout|deref|in_bcount|out_bcount|in_ecount|out_ecount)' SrcServer/src Dependency/DirectXSDK 2>/dev/null \
  | tee "$OUT/04_sal_macros.txt" >/dev/null

echo
echo "===== 5. C++11 USAGE ====="
grep -RInE '\bstd::function\b|\bnullptr\b|\bauto[[:space:]]+[^;=]+=' SrcServer/src modules 2>/dev/null \
  | tee "$OUT/05_cpp11_usage.txt"

echo
echo "===== 6. ENCODING / CARACTERES ESTRANHOS ====="
python3 <<'PY'
from pathlib import Path

roots = [Path("SrcServer/src"), Path("Shared"), Path("Shared2"), Path("modules")]
out = Path("audit_server_mingw_risks/06_encoding_suspects.txt")
bad = []

for root in roots:
    if not root.exists():
        continue
    for p in root.rglob("*"):
        if p.suffix.lower() not in [".cpp", ".c", ".h", ".hpp", ".cc", ".cxx"]:
            continue
        try:
            b = p.read_bytes()
        except Exception:
            continue

        try:
            text = b.decode("utf-8")
        except UnicodeDecodeError as e:
            bad.append(f"{p}: UTF8_DECODE_ERROR byte={e.start}")
            continue

        for i, ch in enumerate(text):
            o = ord(ch)
            if o > 127 and ch not in "\n\r\t":
                line = text.count("\n", 0, i) + 1
                bad.append(f"{p}:{line}: non-ascii U+{o:04X} {repr(ch)}")
                break

out.write_text("\n".join(bad) + ("\n" if bad else ""))
print(out.read_text(), end="")
PY

echo
echo "===== 7. PACKET / tCnt ====="
grep -RInE '\btCnt\b|class[[:space:]]+CPacket|CPacket::' SrcServer/src/TJBOY/Ygy Shared Shared2 modules 2>/dev/null \
  | tee "$OUT/07_packet_tcnt.txt"

echo
echo "===== 8. GLOBAIS/STUBS MINGW DO CLIENTE ====="
find SrcGame/src \
  -type f \( -iname "*mingw*.cpp" -o -iname "*stub*.cpp" -o -iname "*alias*.cpp" \) \
  | sort \
  | tee "$OUT/08_client_mingw_stub_files.txt"

grep -RInE 'TEMP_MINGW|MINGW|stub|alias|missing_globals' SrcGame/src 2>/dev/null \
  | tee "$OUT/08_client_mingw_markers.txt" >/dev/null

echo
echo "===== 9. ENTRYPOINT / WinMain / main ====="
grep -RInE '(^|[[:space:]])(int[[:space:]]+main|WinMain[[:space:]]*\(|ServerWinMain)' SrcServer/src 2>/dev/null \
  | tee "$OUT/09_entrypoints.txt"

echo
echo "===== 10. OBJETOS ATUAIS DO SERVIDOR ====="
find build_mingw_server_i686 -type f -name "*.o" 2>/dev/null \
  | sort \
  | tee "$OUT/10_current_server_objects.txt" >/dev/null

echo
echo "===== 11. ERROS ATUAIS DE BUILD ====="
grep -Ei "error:|fatal:" build_server_i686_compile_errors.log 2>/dev/null \
  | tee "$OUT/11_current_compile_errors.txt" >/dev/null

echo
echo "===== 12. RANK DE ERROS POR ARQUIVO ====="
grep -Ei "error:|fatal:" build_server_i686_compile_errors.log 2>/dev/null \
  | sed -E 's/^([^:]+):.*/\1/' \
  | sort | uniq -c | sort -nr \
  | tee "$OUT/12_error_file_rank.txt"

echo
echo "===== RESUMO ====="
{
  echo "Bad SkillSub includes: $(wc -l < "$OUT/01_bad_skillsub_include.txt" 2>/dev/null || echo 0)"
  echo "Bad '/0': $(wc -l < "$OUT/02_bad_slash_zero.txt" 2>/dev/null || echo 0)"
  echo "DirectX includes: $(wc -l < "$OUT/03_directx_includes.txt" 2>/dev/null || echo 0)"
  echo "SAL macro refs: $(wc -l < "$OUT/04_sal_macros.txt" 2>/dev/null || echo 0)"
  echo "C++11 refs: $(wc -l < "$OUT/05_cpp11_usage.txt" 2>/dev/null || echo 0)"
  echo "Encoding suspects: $(wc -l < "$OUT/06_encoding_suspects.txt" 2>/dev/null || echo 0)"
  echo "Packet/tCnt refs: $(wc -l < "$OUT/07_packet_tcnt.txt" 2>/dev/null || echo 0)"
  echo "Client MinGW stub files: $(wc -l < "$OUT/08_client_mingw_stub_files.txt" 2>/dev/null || echo 0)"
  echo "Entrypoints: $(wc -l < "$OUT/09_entrypoints.txt" 2>/dev/null || echo 0)"
  echo "Current server objects: $(wc -l < "$OUT/10_current_server_objects.txt" 2>/dev/null || echo 0)"
  echo "Current compile errors: $(wc -l < "$OUT/11_current_compile_errors.txt" 2>/dev/null || echo 0)"
} | tee -a "$OUT/summary.txt"

echo
echo "Arquivos gerados em: $OUT"
echo
cat "$OUT/summary.txt"
