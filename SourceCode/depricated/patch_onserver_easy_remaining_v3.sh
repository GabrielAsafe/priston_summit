#!/usr/bin/env bash
set -euo pipefail

FILE="SrcServer/src/Server/OnSever.cpp"
STAMP="$(date +%Y%m%d_%H%M%S)"
BACKUP="${FILE}.bak_easy_remaining_v3_${STAMP}"

cp -a "$FILE" "$BACKUP"

echo "Backup criado:"
echo "  $BACKUP"
echo

python3 - <<'PY'
from pathlib import Path
import re
import sys

path = Path("SrcServer/src/Server/OnSever.cpp")
text = path.read_text(encoding="latin-1")
original = text
changes = []

###############################################################################
# 1. smTRANS_PLAYDATA::Mp -> Mana
###############################################################################

for index in (0, 1):
    old = (
        f"lpPlayInfo->smCharInfo.Mana[{index}] = "
        f"lpTransPlayData->Mp[{index}];"
    )
    new = (
        f"lpPlayInfo->smCharInfo.Mana[{index}] = "
        f"lpTransPlayData->Mana[{index}];"
    )

    count = text.count(old)

    if count == 1:
        text = text.replace(old, new, 1)
        changes.append(f"Mp[{index}] -> Mana[{index}]")
    elif count == 0 and new in text:
        print(f"Já corrigido: Mana[{index}]")
    else:
        print(
            f"Erro: número inesperado de ocorrências para Mp[{index}]: "
            f"{count}"
        )
        sys.exit(1)

###############################################################################
# 2. Remover as linhas CoinPrice das condições
#
# Remove:
#     ...Price &&
#     lpMyShopItem->Temp[0] == ...CoinPrice)
#
# e converte a linha Price para terminar em:
#     ...Price)
###############################################################################

coin_targets = [
    "lpPlayInfo->sMyShopList[cnt].CoinPrice",
    "lpPlayInfo2->sMyShopList[cnt].CoinPrice",
]

for target in coin_targets:
    lines = text.splitlines()
    found = False

    for i, line in enumerate(lines):
        if target not in line:
            continue

        found = True

        if i == 0:
            print(f"Erro: CoinPrice na primeira linha: {target}")
            sys.exit(1)

        previous = lines[i - 1]

        if ".Price" not in previous or "&&" not in previous:
            print("Erro: formato inesperado antes de CoinPrice:")
            print(previous)
            print(line)
            sys.exit(1)

        # Remover && final da linha Price.
        previous = re.sub(r"\s*&&\s*$", "", previous)

        # A linha CoinPrice continha o fechamento da condição.
        closing = ")" if line.rstrip().endswith(")") else ""

        lines[i - 1] = previous + closing
        del lines[i]

        text = "\n".join(lines) + "\n"
        changes.append(f"comparação removida: {target}")
        break

    if not found:
        print(f"Já removido ou não encontrado: {target}")

###############################################################################
# 3. Mover o accountId específico antes dos goto quit
###############################################################################

anchor_start = "DWORD dwTime = GetCurrentTime();"
anchor_decl = "char accountId[32] = { 0 };"
anchor_use = (
    "strcpy_s(accountId, sizeof(accountId), "
    "lpPlayInfo->getAccount());"
)

start = text.find(anchor_start)

if start < 0:
    print("Erro: DWORD dwTime = GetCurrentTime() não encontrado.")
    sys.exit(1)

use = text.find(anchor_use, start)

if use < 0:
    print("Erro: strcpy_s do accountId alvo não encontrado.")
    sys.exit(1)

decl = text.find(anchor_decl, start, use)

if decl >= 0:
    first_goto = text.find("goto quit;", start, decl)

    if first_goto >= 0:
        # Remover a declaração antiga inteira.
        line_start = text.rfind("\n", 0, decl) + 1
        line_end = text.find("\n", decl)

        if line_end < 0:
            line_end = len(text)
        else:
            line_end += 1

        declaration_line = text[line_start:line_end]
        indent = declaration_line[:len(declaration_line) -
                                  len(declaration_line.lstrip())]

        text = text[:line_start] + text[line_end:]

        # Inserir imediatamente após dwTime.
        start = text.find(anchor_start)
        insert_at = text.find("\n", start)

        if insert_at < 0:
            print("Erro: fim da linha dwTime não encontrado.")
            sys.exit(1)

        insert_at += 1

        text = (
            text[:insert_at]
            + f"{indent}char accountId[32] = {{ 0 }};\n"
            + text[insert_at:]
        )

        changes.append("accountId movido após dwTime")
    else:
        print("accountId já está antes dos goto quit.")
else:
    # Confirmar se já está logo após dwTime.
    region = text[start:use]

    if anchor_decl in region:
        print("accountId já corrigido.")
    else:
        print("Erro: accountId alvo não encontrado.")
        sys.exit(1)

###############################################################################
# 4. Verificações específicas
###############################################################################

for forbidden in [
    "lpTransPlayData->Mp[0]",
    "lpTransPlayData->Mp[1]",
    "lpPlayInfo->sMyShopList[cnt].CoinPrice",
    "lpPlayInfo2->sMyShopList[cnt].CoinPrice",
]:
    if forbidden in text:
        print(f"Erro: referência incompatível ainda presente: {forbidden}")
        sys.exit(1)

start = text.find(anchor_start)
use = text.find(anchor_use, start)
region = text[start:use]

decl_pos = region.find(anchor_decl)
goto_pos = region.find("goto quit;")

if decl_pos < 0:
    print("Erro: accountId não encontrado na região final.")
    sys.exit(1)

if goto_pos >= 0 and decl_pos > goto_pos:
    print("Erro: accountId ainda está depois de goto quit.")
    sys.exit(1)

if text == original:
    print("Nenhuma alteração necessária.")
else:
    path.write_text(text, encoding="latin-1")

    print("Alterações aplicadas:")
    for change in changes:
        print(f"  - {change}")
PY

echo
echo "=== accountId ==="
sed -n '24076,24120p' "$FILE"

echo
echo "=== Mana ==="
grep -n -A2 -B2 \
'lpTransPlayData->Mana' \
"$FILE" || true

echo
echo "=== CoinPrice ativo ==="
grep -n '\.CoinPrice' "$FILE" || true

echo
echo "Compilando OnSever.cpp..."

rm -f /tmp/OnSever.o /tmp/OnSever.err

i686-w64-mingw32-g++ \
    -m32 \
    -std=gnu++14 \
    -fpermissive \
    -w \
    -fmax-errors=400 \
    -DWIN32 \
    -D_WINDOWS \
    -D_M_IX86 \
    -Di386 \
    -D__i386__ \
    -D_SERVER \
    -I. \
    -ISrcServer/src \
    -ISrcServer/src/sinbaram \
    -ISrcServer/src/Skills \
    -ISrcGame/src \
    -IShared \
    -IShared/Lua \
    -IShared2 \
    -Imodules \
    -Imodules/File \
    "-IDependency/DirectXSDK/Microsoft DirectX SDK (June 2010)/Include" \
    -c "$FILE" \
    -o /tmp/OnSever.o \
    2>/tmp/OnSever.err || true

echo

if [ -f /tmp/OnSever.o ]; then
    echo "ONSERVER OK"
else
    echo "OnSever ainda possui erros:"
    echo

    grep -E 'error:|fatal error:' /tmp/OnSever.err \
        | sed -E 's/^.*error: //' \
        | sort -u \
        | head -160

    echo
    echo "Erros com linhas:"

    grep -E \
        'OnSever\.cpp:[0-9]+:|error:|fatal error:|note:' \
        /tmp/OnSever.err \
        | head -240
fi
