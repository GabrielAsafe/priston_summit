#!/usr/bin/env bash
set -Eeuo pipefail

ROOT="/home/gabriel/Área de Trabalho/summit/SourceCode"
FILE="$ROOT/SrcServer/src/systems/gameplay/quests/server/cQuestHandler.cpp"

die() {
    echo "[ERRO] $*" >&2
    exit 1
}

[[ -f "$FILE" ]] || die "arquivo não encontrado: $FILE"

STAMP="$(date +%Y%m%d_%H%M%S)"
BACKUP="${FILE}.before_r25_skiptier_diag_${STAMP}"

cp -a "$FILE" "$BACKUP"

python3 - "$FILE" <<'PY'
from pathlib import Path
import sys

p = Path(sys.argv[1])
s = p.read_text(encoding="utf-8", errors="strict")

marker = "R25_SKIPTIER_DIAG_V1"
if marker in s:
    print("[INFO] instrumentação já aplicada")
    raise SystemExit(0)

if "#include <cstdio>" not in s:
    anchor = '#include <cstring>\n'
    if anchor not in s:
        raise SystemExit("[ERRO] include <cstring> esperado não encontrado")
    s = s.replace(anchor, anchor + '#include <cstdio>\n', 1)

global_anchor = "cQUESTHANDLER cQuestHandler;\n"
if global_anchor not in s:
    raise SystemExit("[ERRO] âncora cQuestHandler não encontrada")

helper = r'''
// R25_SKIPTIER_DIAG_V1
static void R25SkipTierDiag(const char* stage, int questId, const char* player, const char* detail)
{
	FILE* f = std::fopen("LOGS\\r25_skiptier_diag.log", "a");
	if(!f)
		return;

	std::fprintf(
		f,
		"[R25_SKIPTIER] stage=%s quest=%d player=%s detail=%s\n",
		stage ? stage : "<null>",
		questId,
		player ? player : "<null>",
		detail ? detail : "<null>"
	);
	std::fclose(f);
}
'''

s = s.replace(global_anchor, global_anchor + helper, 1)

sig = "bool cQUESTHANDLER::MarkSingleQuestCompletedForTesting(rsPLAYINFO* lpPlayInfo, int QuestID)"
start = s.find(sig)
if start < 0:
    raise SystemExit("[ERRO] função MarkSingleQuestCompletedForTesting não encontrada")

brace = s.find("{", start)
if brace < 0:
    raise SystemExit("[ERRO] abertura da função não encontrada")

depth = 0
end = None
for i in range(brace, len(s)):
    ch = s[i]
    if ch == "{":
        depth += 1
    elif ch == "}":
        depth -= 1
        if depth == 0:
            end = i + 1
            break

if end is None:
    raise SystemExit("[ERRO] fechamento da função não encontrado")

new_func = r'''bool cQUESTHANDLER::MarkSingleQuestCompletedForTesting(rsPLAYINFO* lpPlayInfo, int QuestID)
{
	if (!lpPlayInfo || QuestID <= 0)
	{
		R25SkipTierDiag("invalid_args", QuestID, lpPlayInfo ? lpPlayInfo->getName() : "<null>", "");
		return false;
	}

	char playerName[32] = {};
	wsprintf(playerName, "%s", lpPlayInfo->getName());
	R25SkipTierDiag("enter", QuestID, playerName, "");

	std::shared_ptr<SQLConnection> db = SQLConnection::GetConnection(DATABASEID_QuestDB);
	if (!db)
	{
		R25SkipTierDiag("get_connection_fail", QuestID, playerName, "DATABASEID_QuestDB returned null");
		return false;
	}

	if (!db->Open())
	{
		R25SkipTierDiag("open_fail", QuestID, playerName, db->GetLastError());
		return false;
	}

	bool ok = false;
	int questType = 0;
	int questKind = 9;
	int questDefinitionCount = 0;
	char diag[256] = {};

	if (!db->Prepare("SELECT COALESCE(MAX(QuestType),0), COUNT(*) FROM SingleQuest WHERE QuestID=?"))
	{
		R25SkipTierDiag("select_prepare_fail", QuestID, playerName, db->GetLastError());
		goto done;
	}
	if (!db->BindInputParameter(&QuestID, 1, PARAMTYPE_Integer))
	{
		R25SkipTierDiag("select_bind_fail", QuestID, playerName, db->GetLastError());
		goto done;
	}
	if (!db->Execute())
	{
		R25SkipTierDiag("select_execute_fail", QuestID, playerName, db->GetLastError());
		goto done;
	}
	if (!db->GetData(&questType, 4, 1, PARAMTYPE_Integer))
	{
		R25SkipTierDiag("select_get_questtype_fail", QuestID, playerName, db->GetLastError());
		goto done;
	}
	if (!db->GetData(&questDefinitionCount, 4, 2, PARAMTYPE_Integer))
	{
		R25SkipTierDiag("select_get_count_fail", QuestID, playerName, db->GetLastError());
		goto done;
	}

	if (questDefinitionCount <= 0)
		questType = 0;

	std::snprintf(
		diag,
		sizeof(diag),
		"definitionCount=%d questType=%d",
		questDefinitionCount,
		questType
	);
	R25SkipTierDiag("select_ok", QuestID, playerName, diag);

	if (!db->Prepare("DELETE FROM MonsterKilled WHERE QuestID=? AND PlayerName=?"))
	{
		R25SkipTierDiag("monster_delete_prepare_fail", QuestID, playerName, db->GetLastError());
		goto done;
	}
	if (!db->BindInputParameter(&QuestID, 1, PARAMTYPE_Integer) ||
		!db->BindInputParameter(playerName, 2, PARAMTYPE_String))
	{
		R25SkipTierDiag("monster_delete_bind_fail", QuestID, playerName, db->GetLastError());
		goto done;
	}
	if (!db->Execute(false))
	{
		R25SkipTierDiag("monster_delete_execute_fail", QuestID, playerName, db->GetLastError());
		goto done;
	}
	R25SkipTierDiag("monster_delete_ok", QuestID, playerName, "");

	if (!db->Prepare("DELETE FROM Progress WHERE QuestID=? AND PlayerName=?"))
	{
		R25SkipTierDiag("progress_delete_prepare_fail", QuestID, playerName, db->GetLastError());
		goto done;
	}
	if (!db->BindInputParameter(&QuestID, 1, PARAMTYPE_Integer) ||
		!db->BindInputParameter(playerName, 2, PARAMTYPE_String))
	{
		R25SkipTierDiag("progress_delete_bind_fail", QuestID, playerName, db->GetLastError());
		goto done;
	}
	if (!db->Execute(false))
	{
		R25SkipTierDiag("progress_delete_execute_fail", QuestID, playerName, db->GetLastError());
		goto done;
	}
	R25SkipTierDiag("progress_delete_ok", QuestID, playerName, "");

	if (!db->Prepare("INSERT OR REPLACE INTO Complete (QuestID, QuestKind, QuestType, PlayerName, EndDate) VALUES (?1, ?2, ?3, ?4, datetime('now','localtime'))"))
	{
		R25SkipTierDiag("complete_prepare_fail", QuestID, playerName, db->GetLastError());
		goto done;
	}
	if (!db->BindInputParameter(&QuestID, 1, PARAMTYPE_Integer) ||
		!db->BindInputParameter(&questKind, 2, PARAMTYPE_Integer) ||
		!db->BindInputParameter(&questType, 3, PARAMTYPE_Integer) ||
		!db->BindInputParameter(playerName, 4, PARAMTYPE_String))
	{
		R25SkipTierDiag("complete_bind_fail", QuestID, playerName, db->GetLastError());
		goto done;
	}

	ok = db->Execute(false) ? true : false;
	R25SkipTierDiag(
		ok ? "complete_execute_ok" : "complete_execute_fail",
		QuestID,
		playerName,
		ok ? "" : db->GetLastError()
	);

done:
	R25SkipTierDiag(ok ? "return_true" : "return_false", QuestID, playerName, db->GetLastError());
	db->Close();
	return ok;
}'''

s = s[:start] + new_func + s[end:]
p.write_text(s, encoding="utf-8")
print("[OK] instrumentação aplicada")
PY

echo
echo "Backup:"
echo "  $BACKUP"
echo
echo "Marcadores:"
grep -n "R25_SKIPTIER_DIAG_V1\|MarkSingleQuestCompletedForTesting\|R25SkipTierDiag" "$FILE" | head -40

echo
echo "Diff resumido:"
diff -u "$BACKUP" "$FILE" | sed -n '1,260p' || true

echo
echo "============================================================"
echo "PATCH SKIPTIER DIAG: PASS"
echo "============================================================"
echo
echo "Este script NÃO compilou e NÃO iniciou o servidor."
