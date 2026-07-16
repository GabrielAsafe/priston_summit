#!/usr/bin/env bash
set -euo pipefail

FILE="SrcServer/src/Server/OnSever.cpp"
STAMP="$(date +%Y%m%d_%H%M%S)"
BACKUP="${FILE}.bak_easy_remaining_v2_${STAMP}"

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
# 1. Mp -> Mana
###############################################################################

replacements = [
    (
        "lpPlayInfo->smCharInfo.Mana[0] = lpTransPlayData->Mp[0];",
        "lpPlayInfo->smCharInfo.Mana[0] = lpTransPlayData->Mana[0];",
    ),
    (
        "lpPlayInfo->smCharInfo.Mana[1] = lpTransPlayData->Mp[1];",
        "lpPlayInfo->smCharInfo.Mana[1] = lpTransPlayData->Mana[1];",
    ),
]

for old, new in replacements:
    count = text.count(old)

    if count == 1:
        text = text.replace(old, new, 1)
        changes.append(f"{old} -> {new}")
    elif count == 0 and new in text:
        print(f"Já corrigido: {new}")
    else:
        print(f"Erro: ocorrência inesperada ({count}) para:")
        print(f"  {old}")
        sys.exit(1)

###############################################################################
# 2. Remover apenas as duas comparações CoinPrice
###############################################################################

coin_old_1 = """lpMyShopItem->Price == lpPlayInfo->sMyShopList[cnt].Price &&
                                                lpMyShopItem->Temp[0] == lpPlayInfo->sMyShopList[cnt].CoinPrice"""

coin_new_1 = """lpMyShopItem->Price == lpPlayInfo->sMyShopList[cnt].Price"""

coin_old_2 = """lpMyShopItem->Price == lpPlayInfo2->sMyShopList[cnt].Price &&
                                lpMyShopItem->Temp[0] == lpPlayInfo2->sMyShopList[cnt].CoinPrice"""

coin_new_2 = """lpMyShopItem->Price == lpPlayInfo2->sMyShopList[cnt].Price"""

for old, new, label in [
    (coin_old_1, coin_new_1, "CoinPrice lpPlayInfo"),
    (coin_old_2, coin_new_2, "CoinPrice lpPlayInfo2"),
]:
    count = text.count(old)

    if count == 1:
        text = text.replace(old, new, 1)
        changes.append(label)
    elif count == 0 and new in text:
        print(f"Possivelmente já corrigido: {label}")
    else:
        print(f"Erro: ocorrência inesperada ({count}) para {label}")
        sys.exit(1)

###############################################################################
# 3. Mover apenas o accountId da rotina de receção de pacotes
###############################################################################

anchor_before = "dwTransCode = IPData[1];"
account_decl = "char accountId[32] = { 0 };"
account_use = (
    "strcpy_s(accountId, sizeof(accountId), "
    "lpPlayInfo->getAccount());"
)
insert_anchor = "EnterCriticalSection(&cSerSection);"

before_pos = text.find(anchor_before)

if before_pos < 0:
    print("Erro: anchor dwTransCode não encontrado.")
    sys.exit(1)

decl_pos = text.find(account_decl, before_pos)

if decl_pos < 0:
    print("Erro: accountId alvo não encontrado após dwTransCode.")
    sys.exit(1)

use_pos = text.find(account_use, decl_pos)

if use_pos < 0:
    print("Erro: uso esperado de accountId não encontrado.")
    sys.exit(1)

# Garantir que estamos a mexer numa região curta e específica.
if use_pos - decl_pos > 500:
    print("Erro: accountId e strcpy_s demasiado afastados.")
    sys.exit(1)

# Procurar EnterCriticalSection imediatamente antes desta região.
search_start = max(0, before_pos - 3000)
enter_pos = text.rfind(insert_anchor, search_start, before_pos)

if enter_pos < 0:
    print("Erro: EnterCriticalSection alvo não encontrado.")
    sys.exit(1)

# Verificar se accountId já está antes do primeiro goto desta região.
first_goto = text.find("goto quit;", enter_pos, decl_pos)

if first_goto < 0:
    print("Aviso: nenhum goto quit encontrado antes de accountId.")
else:
    # Remover declaração antiga, incluindo quebra de linha.
    line_start = text.rfind("\n", 0, decl_pos) + 1
    line_end = text.find("\n", decl_pos)

    if line_end < 0:
        line_end = len(text)
    else:
        line_end += 1

    declaration_line = text[line_start:line_end]
    indent = declaration_line[:len(declaration_line) -
                              len(declaration_line.lstrip())]

    text = text[:line_start] + text[line_end:]

    # A remoção ocorreu depois de enter_pos, logo enter_pos não muda.
    insert_line_start = text.rfind("\n", 0, enter_pos) + 1

    new_decl = f"{indent}char accountId[32] = {{ 0 }};\n\n"

    text = (
        text[:insert_line_start]
        + new_decl
        + text[insert_line_start:]
    )

    changes.append("accountId alvo movido antes dos goto quit")

###############################################################################
# 4. Verificações finais
###############################################################################

for forbidden in [
    "lpTransPlayData->Mp[0]",
    "lpTransPlayData->Mp[1]",
    "lpPlayInfo->sMyShopList[cnt].CoinPrice",
    "lpPlayInfo2->sMyShopList[cnt].CoinPrice",
]:
    if forbidden in text:
        print(f"Erro: referência ainda presente: {forbidden}")
        sys.exit(1)

# Confirmar a ordem no bloco específico.
region_start = text.find("EnterCriticalSection(&cSerSection);", search_start)
region_end = text.find(account_use, region_start)

region = text[region_start:region_end]

account_relative = region.find(account_decl)
goto_relative = region.find("goto quit;")

if account_relative < 0:
    print("Erro: accountId desapareceu da região.")
    sys.exit(1)

if goto_relative >= 0 and account_relative > goto_relative:
    print("Erro: accountId continua depois de goto quit.")
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
echo "=== Região accountId ==="
sed -n '24068,24122p' "$FILE"

echo
echo "=== Região Mana ==="
sed -n '25422,25433p' "$FILE"

echo
echo "=== Referências CoinPrice restantes ==="
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
