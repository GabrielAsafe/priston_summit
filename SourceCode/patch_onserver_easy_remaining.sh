#!/usr/bin/env bash
set -euo pipefail

FILE="SrcServer/src/Server/OnSever.cpp"
STAMP="$(date +%Y%m%d_%H%M%S)"
BACKUP="${FILE}.bak_easy_remaining_${STAMP}"

if [ ! -f "$FILE" ]; then
    echo "Erro: ficheiro não encontrado: $FILE"
    exit 1
fi

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
# 1. Corrigir smTRANS_PLAYDATA::Mp -> Mana
###############################################################################

mp_patterns = [
    (
        "lpPlayInfo->smCharInfo.Mana[0] = lpTransPlayData->Mp[0];",
        "lpPlayInfo->smCharInfo.Mana[0] = lpTransPlayData->Mana[0];",
    ),
    (
        "lpPlayInfo->smCharInfo.Mana[1] = lpTransPlayData->Mp[1];",
        "lpPlayInfo->smCharInfo.Mana[1] = lpTransPlayData->Mana[1];",
    ),
]

for old, new in mp_patterns:
    count = text.count(old)

    if count == 1:
        text = text.replace(old, new, 1)
        changes.append(f"Mp -> Mana: {old.strip()}")
    elif count == 0:
        if new in text:
            print(f"Já corrigido: {new.strip()}")
        else:
            print(f"Aviso: expressão não encontrada: {old.strip()}")
    else:
        print(
            f"Erro: expressão encontrada {count} vezes; "
            f"patch recusado: {old.strip()}"
        )
        sys.exit(1)

###############################################################################
# 2. Remover CoinPrice das duas condições
#
# Mantém a comparação por:
#   dwCode
#   dwHead
#   dwChkSum
#   Price
#
# O custo em coin continua a ser validado diretamente por Temp[0] noutras
# condições desta função.
###############################################################################

coin_patterns = [
    re.compile(
        r"""
        (?P<indent>[ \t]*)
        lpMyShopItem->Price
        \s*==\s*
        lpPlayInfo->sMyShopList\[cnt\]\.Price
        \s*&&\s*
        lpMyShopItem->Temp\[0\]
        \s*==\s*
        lpPlayInfo->sMyShopList\[cnt\]\.CoinPrice
        """,
        re.VERBOSE,
    ),
    re.compile(
        r"""
        (?P<indent>[ \t]*)
        lpMyShopItem->Price
        \s*==\s*
        lpPlayInfo2->sMyShopList\[cnt\]\.Price
        \s*&&\s*
        lpMyShopItem->Temp\[0\]
        \s*==\s*
        lpPlayInfo2->sMyShopList\[cnt\]\.CoinPrice
        """,
        re.VERBOSE,
    ),
]

coin_replacements = [
    "lpMyShopItem->Price == lpPlayInfo->sMyShopList[cnt].Price",
    "lpMyShopItem->Price == lpPlayInfo2->sMyShopList[cnt].Price",
]

for pattern, replacement in zip(coin_patterns, coin_replacements):
    matches = list(pattern.finditer(text))

    if len(matches) == 1:
        text = pattern.sub(replacement, text, count=1)
        changes.append(f"CoinPrice removido da condição: {replacement}")
    elif len(matches) == 0:
        if replacement in text:
            print(f"Já corrigido: {replacement}")
        else:
            print(f"Aviso: condição CoinPrice não encontrada: {replacement}")
    else:
        print(
            f"Erro: condição CoinPrice encontrada {len(matches)} vezes; "
            "patch recusado."
        )
        sys.exit(1)

###############################################################################
# 3. Mover accountId antes dos primeiros goto quit
#
# Atual:
#   declarações
#   EnterCriticalSection(...)
#   ...
#   goto quit
#   ...
#   char accountId[32] = { 0 };
#
# Novo:
#   declarações
#   char accountId[32] = { 0 };
#   EnterCriticalSection(...)
###############################################################################

account_decl_re = re.compile(
    r"^[ \t]*char[ \t]+accountId"
    r"[ \t]*\[[ \t]*32[ \t]*\]"
    r"[ \t]*=[ \t]*\{[ \t]*0[ \t]*\}"
    r"[ \t]*;[ \t]*$",
    re.MULTILINE,
)

account_matches = list(account_decl_re.finditer(text))

if len(account_matches) == 1:
    account_match = account_matches[0]
    declaration = account_match.group(0)

    # Remover a declaração atual.
    text_without_decl = (
        text[:account_match.start()]
        + text[account_match.end():]
    )

    # Inserir antes de EnterCriticalSection da mesma função.
    search_start = max(0, account_match.start() - 5000)
    preceding = text_without_decl[search_start:account_match.start()]

    enter_positions = [
        match.start()
        for match in re.finditer(
            r"^[ \t]*EnterCriticalSection\(&cSerSection\);[ \t]*$",
            preceding,
            re.MULTILINE,
        )
    ]

    if not enter_positions:
        print(
            "Erro: EnterCriticalSection(&cSerSection) não encontrado "
            "antes de accountId."
        )
        sys.exit(1)

    insert_at = search_start + enter_positions[-1]

    # Preservar a indentação observada na declaração.
    indent_match = re.match(r"^[ \t]*", declaration)
    indent = indent_match.group(0)

    inserted = (
        f"{indent}char accountId[32] = {{ 0 }};\n\n"
    )

    text = (
        text_without_decl[:insert_at]
        + inserted
        + text_without_decl[insert_at:]
    )

    changes.append(
        "accountId movido para antes dos goto quit"
    )

elif len(account_matches) == 0:
    # Verificar se já foi movido.
    account_pos = text.find("char accountId[32] = { 0 };")
    first_goto = text.find("goto quit;")

    if account_pos >= 0 and first_goto >= 0 and account_pos < first_goto:
        print("accountId já está antes do primeiro goto quit.")
    else:
        print("Aviso: declaração accountId não encontrada.")
else:
    print(
        f"Erro: encontradas {len(account_matches)} declarações "
        "de accountId; patch recusado."
    )
    sys.exit(1)

###############################################################################
# 4. Verificações antes de gravar
###############################################################################

remaining = []

for expression in [
    "lpTransPlayData->Mp[0]",
    "lpTransPlayData->Mp[1]",
    ".CoinPrice",
]:
    if expression in text:
        remaining.append(expression)

if remaining:
    print()
    print("Erro: referências incompatíveis ainda presentes:")
    for expression in remaining:
        print(f"  {expression}")
    sys.exit(1)

if text == original:
    print()
    print("Nenhuma alteração necessária.")
    sys.exit(0)

path.write_text(text, encoding="latin-1")

print("Alterações aplicadas:")
for change in changes:
    print(f"  - {change}")
PY

echo
echo "Verificação das regiões alteradas:"
echo

echo "=== accountId ==="
grep -n -A6 -B6 \
'char accountId\[32\]' \
"$FILE" |
head -40 || true

echo
echo "=== Mana ==="
grep -n -A3 -B3 \
'lpTransPlayData->Mana' \
"$FILE" |
head -40 || true

echo
echo "=== CoinPrice restante ==="
if grep -n '\.CoinPrice' "$FILE"; then
    echo
    echo "ERRO: CoinPrice ainda está ativo."
    echo "A restaurar backup..."
    cp -a "$BACKUP" "$FILE"
    exit 1
else
    echo "Nenhuma referência ativa a CoinPrice."
fi

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
