#!/usr/bin/env bash
set -euo pipefail

SCRIPT="build_server_mingw_i686_j8.sh"

echo "[1] Backup"
cp -n "$SCRIPT" "$SCRIPT.bak"

echo "[2] Patch C++14"
perl -0pi -e 's/-std=gnu\+\+11/-std=gnu++14/g' "$SCRIPT"

echo "[3] Procurando lua.h"
LUA_H="$(find . -iname lua.h | head -1 || true)"
if [ -n "$LUA_H" ]; then
  LUA_DIR="$(dirname "$LUA_H")"
  LUA_PARENT="$(dirname "$LUA_DIR")"

  grep -q -- "-I$LUA_PARENT" "$SCRIPT" || \
    perl -0pi -e "s#(-ISrcGame/src\n)#$1  -I$LUA_PARENT\n#g" "$SCRIPT"

  grep -q -- "-I$LUA_DIR" "$SCRIPT" || \
    perl -0pi -e "s#(-I$LUA_PARENT\n)#\$1  -I$LUA_DIR\n#g" "$SCRIPT"

  echo "[OK] Lua include: $LUA_PARENT e $LUA_DIR"
else
  echo "[WARN] lua.h não encontrado"
fi

echo "[4] Procurando Joi.hpp"
JOI_H="$(find . -iname Joi.hpp | head -1 || true)"
if [ -n "$JOI_H" ]; then
  JOI_PARENT="$(dirname "$(dirname "$JOI_H")")"
  grep -q -- "-I$JOI_PARENT" "$SCRIPT" || \
    perl -0pi -e "s#(-Imodules\n)#$1  -I$JOI_PARENT\n#g" "$SCRIPT"
  echo "[OK] Joi include: $JOI_PARENT"
else
  echo "[WARN] Joi.hpp não encontrado"
fi

echo "[5] Corrigindo include server/onserver.h"
grep -RIl 'server/onserver.h' SrcServer/src modules 2>/dev/null | while read -r f; do
  cp -n "$f" "$f.bak"
  perl -0pi -e 's#["<]server/onserver\.h[">]#"Server/onserver.h"#gi' "$f"
  echo "[PATCHED] $f"
done

echo "[6] Scan strings quebradas"
grep -RIn 'operator""' SrcServer/src modules 2>/dev/null || true

echo "[7] Mostrar blocos críticos"
sed -n '1340,1385p' SrcServer/src/cQuestHandler.cpp 2>/dev/null || true
sed -n '980,1045p' SrcServer/src/HoBaram/HoNewParticle.cpp 2>/dev/null || true

echo "[8] Patch strings conhecidas"
for f in SrcServer/src/cQuestHandler.cpp SrcServer/src/HoBaram/HoNewParticle.cpp SrcServer/src/smReg.cpp SrcServer/src/record.cpp; do
  [ -f "$f" ] && cp -n "$f" "$f.bak"
done

perl -0pi -e 's#/"#\\"#g; s#/"#\\"#g; s#/r/n#\\r\\n#g' \
  SrcServer/src/smReg.cpp SrcServer/src/record.cpp 2>/dev/null || true

perl -0pi -e '
s#OutputDebugString\("Expecting \\"so\\" after \\"fade\\"."\);#OutputDebugString("Expecting \\"so\\" after \\"fade\\".");#g;
s#OutputDebugString\("Cannot use \\"fade so\\" modifier on \\"initial\\" times."\);#OutputDebugString("Cannot use \\"fade so\\" modifier on \\"initial\\" times.");#g;
s#OutputDebugString\("Expected time specifier: \\"\[fade so\] at\\", \\"initial\\", or \\"\[fade so\] final\\""\);#OutputDebugString("Expected time specifier: \\"\[fade so\] at\\", \\"initial\\", or \\"\[fade so\] final\\"");#g;
s#Unexpected \\"%s\\"#Unexpected \\"%s\\"#g;
' SrcServer/src/HoBaram/HoNewParticle.cpp 2>/dev/null || true

echo "[9] Verificação final"
grep -RIn 'operator""' SrcServer/src/HoBaram/HoNewParticle.cpp SrcServer/src/smReg.cpp SrcServer/src/record.cpp SrcServer/src/cQuestHandler.cpp 2>/dev/null || true

echo
echo "[DONE] Agora roda:"
echo "bash $SCRIPT"
