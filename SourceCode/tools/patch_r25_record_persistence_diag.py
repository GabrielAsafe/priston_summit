#!/usr/bin/env python3

from pathlib import Path
import re
import shutil
import sys

ROOT = Path.cwd()

PACKET = ROOT / "SrcServer/src/systems/security/network/packet_filter/PacketFilter.cpp"
SERVER = ROOT / "Shared/server/legacy_runtime/OnSever_legacy_server.inl"
RECORD = ROOT / "Shared/persistence/character_record/record_legacy_server.inl"

MARK_PACKET = "R25_RECORD_DIAG_PACKET"
MARK_HANDLER = "R25_RECORD_DIAG_HANDLER"
MARK_RECV = "R25_RECORD_DIAG_RECV"

for p in (PACKET, SERVER, RECORD):
    if not p.is_file():
        raise SystemExit(f"ERRO: arquivo não encontrado: {p}")


def read(path):
    return path.read_text(
        encoding="utf-8",
        errors="surrogateescape",
    )


def write(path, data):
    path.write_text(
        data,
        encoding="utf-8",
        errors="surrogateescape",
    )


def backup_once(path):
    backup = Path(str(path) + ".before_record_diag")

    if not backup.exists():
        shutil.copy2(path, backup)
        print(f"[BACKUP] {backup}")
    else:
        print(f"[BACKUP] já existe: {backup}")


for p in (PACKET, SERVER, RECORD):
    backup_once(p)


# ============================================================
# 0. Validar que o bound fix já está aplicado.
# ============================================================

packet_data = read(PACKET)
record_data = read(RECORD)

old_bound_packet = (
    "TransRecord->TransSize + "
    "(TransRecord->Count * TRANS_RECORD_LEN) >= "
    "sizeof(TRANS_RECORD_DATA)"
)

new_bound_packet = (
    "TransRecord->TransSize + "
    "(TransRecord->Count * TRANS_RECORD_LEN) > "
    "sizeof(TRANS_RECORD_DATA)"
)

old_bound_recv = (
    "lpTransRecord->Count * TRANS_RECORD_LEN + "
    "lpTransRecord->TransSize >= sizeof(TransRecordData)"
)

new_bound_recv = (
    "lpTransRecord->Count * TRANS_RECORD_LEN + "
    "lpTransRecord->TransSize > sizeof(TransRecordData)"
)

if old_bound_packet in packet_data:
    raise SystemExit(
        "ERRO: PacketFilter ainda contém >=. "
        "O bound fix esperado não está aplicado."
    )

if new_bound_packet not in packet_data:
    raise SystemExit(
        "ERRO: não encontrei o bound corrigido no PacketFilter."
    )

if old_bound_recv in record_data:
    raise SystemExit(
        "ERRO: RecvRecordDataFromClient ainda contém >=."
    )

if new_bound_recv not in record_data:
    raise SystemExit(
        "ERRO: não encontrei o bound corrigido "
        "em RecvRecordDataFromClient."
    )

print("[CHECK] bound fix > confirmado nos dois pontos")


# ============================================================
# 1. PacketFilter.cpp
# ============================================================

packet_data = read(PACKET)

if MARK_PACKET not in packet_data:

    if "#include <cstdio>" not in packet_data:
        anchor = "#include <cstring>\n"

        if anchor not in packet_data:
            raise SystemExit(
                "ERRO: não encontrei include <cstring> "
                "em PacketFilter.cpp"
            )

        packet_data = packet_data.replace(
            anchor,
            anchor + "#include <cstdio>\n",
            1,
        )

    anchor = (
        "\t\tTRANS_RECORD_DATAS* TransRecord = "
        "(TRANS_RECORD_DATAS*)m_Packet;\n"
    )

    if anchor not in packet_data:
        raise SystemExit(
            "ERRO: não encontrei anchor TRANS_RECORD_DATAS "
            "no PacketFilter."
        )

    probe = r'''
		// R25_RECORD_DIAG_PACKET
		{
			FILE *r25fp = fopen("LOGS\\r25_record_persistence_diag.log", "a");
			if(r25fp)
			{
				fprintf(
					r25fp,
					"[R25_RECORD][FILTER] len=%u count=%d total=%d "
					"transSize=%d end=%d struct=%u dataCapacity=%u\n",
					(unsigned)m_Length,
					TransRecord->Count,
					TransRecord->Total,
					TransRecord->TransSize,
					TransRecord->Count * TRANS_RECORD_LEN +
						TransRecord->TransSize,
					(unsigned)sizeof(TRANS_RECORD_DATA),
					(unsigned)sizeof(TransRecord->szData)
				);
				fclose(r25fp);
			}
		}
'''

    packet_data = packet_data.replace(
        anchor,
        anchor + probe,
        1,
    )

    write(PACKET, packet_data)
    print("[PATCH] PacketFilter diagnostic instalado")

else:
    print("[SKIP] PacketFilter diagnostic já instalado")


# ============================================================
# 2. RecvRecordDataFromClient()
# ============================================================

record_data = read(RECORD)

if MARK_RECV not in record_data:

    start_token = (
        "int rsRECORD_DBASE::RecvRecordDataFromClient("
        "TRANS_RECORD_DATAS *lpTransRecord, "
        "rsPLAYINFO *lpPlayInfo)"
    )

    end_token = (
        "int rsRecordMemoryBuffToFile("
        "rsPLAYINFO *lpPlayInfo"
    )

    start = record_data.find(start_token)

    if start < 0:
        raise SystemExit(
            "ERRO: não encontrei início de "
            "RecvRecordDataFromClient."
        )

    end = record_data.find(end_token, start)

    if end < 0:
        raise SystemExit(
            "ERRO: não encontrei fim de "
            "RecvRecordDataFromClient."
        )

    before = record_data[:start]
    func = record_data[start:end]
    after = record_data[end:]

    anchor = (
        "\tchar *lpRecordMemBuff = "
        "lpPlayInfo->lpRecordDataBuff;\n"
    )

    if anchor not in func:
        raise SystemExit(
            "ERRO: não encontrei lpRecordMemBuff "
            "dentro de RecvRecordDataFromClient."
        )

    diag = r'''
	// R25_RECORD_DIAG_RECV
	#define R25_REC_DIAG(stage_, ret_) \
		do { \
			FILE *r25fp = fopen("LOGS\\r25_record_persistence_diag.log", "a"); \
			if(r25fp) { \
				fprintf( \
					r25fp, \
					"[R25_RECORD][RECV] stage=%s srcLine=%d ret=%d " \
					"account='%s' playName='%s' " \
					"count=%d total=%d transSize=%d end=%d struct=%u " \
					"lastPart=%d assembledSize=%d assembledCode=0x%08X " \
					"assembledName='%s'\n", \
					(stage_), \
					__LINE__, \
					(int)(ret_), \
					(lpPlayInfo && lpPlayInfo->getAccount()) ? \
						lpPlayInfo->getAccount() : "", \
					(lpPlayInfo && lpPlayInfo->getName()) ? \
						lpPlayInfo->getName() : "", \
					lpTransRecord ? lpTransRecord->Count : -999, \
					lpTransRecord ? lpTransRecord->Total : -999, \
					lpTransRecord ? lpTransRecord->TransSize : -999, \
					lpTransRecord ? \
						(lpTransRecord->Count * TRANS_RECORD_LEN + \
						 lpTransRecord->TransSize) : -999, \
					(unsigned)sizeof(TRANS_RECORD_DATA), \
					TransLastPartCount, \
					TransRecordData.size, \
					(unsigned)TransRecordData.code, \
					TransRecordData.smCharInfo.szName \
				); \
				fclose(r25fp); \
			} \
		} while(0)

	#define R25_REC_RETURN(v_) \
		do { \
			int r25ret = (v_); \
			R25_REC_DIAG("return", r25ret); \
			return r25ret; \
		} while(0)

	R25_REC_DIAG("enter", 0);
'''

    func = func.replace(
        anchor,
        anchor + diag,
        1,
    )

    # Toda rejeição negativa passa a registrar o código
    # e a linha C++ exata que retornou.
    func, neg_count = re.subn(
        r"\breturn\s+(-[0-9]+)\s*;",
        r"R25_REC_RETURN(\1);",
        func,
    )

    if neg_count < 1:
        raise SystemExit(
            "ERRO: nenhum retorno negativo encontrado "
            "em RecvRecordDataFromClient."
        )

    # O TRUE final significa registro completo aceito.
    true_matches = len(
        re.findall(r"\breturn\s+TRUE\s*;", func)
    )

    if true_matches != 1:
        raise SystemExit(
            "ERRO: esperava exatamente 1 'return TRUE;' "
            f"na função; encontrei {true_matches}"
        )

    func = re.sub(
        r"\breturn\s+TRUE\s*;",
        (
            'R25_REC_DIAG("complete", TRUE);\n'
            "\t\treturn TRUE;"
        ),
        func,
        count=1,
    )

    # O FALSE final significa: fragmento aceito,
    # mas registro ainda incompleto.
    false_matches = len(
        re.findall(r"\breturn\s+FALSE\s*;", func)
    )

    if false_matches != 1:
        raise SystemExit(
            "ERRO: esperava exatamente 1 'return FALSE;' "
            f"na função; encontrei {false_matches}"
        )

    func = re.sub(
        r"\breturn\s+FALSE\s*;",
        (
            'R25_REC_DIAG("fragment_wait", FALSE);\n'
            "\treturn FALSE;"
        ),
        func,
        count=1,
    )

    # Não deixar macros vazarem para o resto do .inl.
    func = func.rstrip() + r'''

#undef R25_REC_RETURN
#undef R25_REC_DIAG

'''

    record_data = before + func + after
    write(RECORD, record_data)

    print(
        f"[PATCH] RecvRecordDataFromClient diagnostic instalado "
        f"({neg_count} retornos negativos instrumentados)"
    )

else:
    print("[SKIP] RecvRecordDataFromClient diagnostic já instalado")


# ============================================================
# 3. Handler OPCODE_RECORDDATA
# ============================================================

server_data = read(SERVER)

if MARK_HANDLER not in server_data:

    pattern = re.compile(
        r"(?P<indent>[ \t]*)int val = "
        r"lpPlayInfo->lpRecordBase->RecvRecordDataFromClient"
        r"\(\(TRANS_RECORD_DATAS \*\)SockInfo->Buff, lpPlayInfo\);"
    )

    match = pattern.search(server_data)

    if not match:
        raise SystemExit(
            "ERRO: não encontrei a chamada do handler para "
            "RecvRecordDataFromClient."
        )

    indent = match.group("indent")
    original = match.group(0)

    handler_probe = original + "\n" + indent + r'''// R25_RECORD_DIAG_HANDLER
			{
				TRANS_RECORD_DATAS *r25rec =
					(TRANS_RECORD_DATAS *)SockInfo->Buff;

				FILE *r25fp =
					fopen("LOGS\\r25_record_persistence_diag.log", "a");

				if(r25fp)
				{
					fprintf(
						r25fp,
						"[R25_RECORD][HANDLER] val=%d "
						"account='%s' playName='%s' "
						"count=%d total=%d transSize=%d end=%d struct=%u "
						"assembledSize=%d assembledCode=0x%08X "
						"assembledName='%s'\n",
						val,
						lpPlayInfo->getAccount() ?
							lpPlayInfo->getAccount() : "",
						lpPlayInfo->getName() ?
							lpPlayInfo->getName() : "",
						r25rec->Count,
						r25rec->Total,
						r25rec->TransSize,
						r25rec->Count * TRANS_RECORD_LEN +
							r25rec->TransSize,
						(unsigned)sizeof(TRANS_RECORD_DATA),
						lpPlayInfo->lpRecordBase ?
							lpPlayInfo->lpRecordBase->TransRecordData.size : -1,
						lpPlayInfo->lpRecordBase ?
							(unsigned)lpPlayInfo->lpRecordBase->TransRecordData.code : 0,
						lpPlayInfo->lpRecordBase ?
							lpPlayInfo->lpRecordBase->TransRecordData.smCharInfo.szName : ""
					);

					fclose(r25fp);
				}
			}'''

    server_data = (
        server_data[:match.start()]
        + handler_probe
        + server_data[match.end():]
    )

    write(SERVER, server_data)
    print("[PATCH] OPCODE_RECORDDATA handler diagnostic instalado")

else:
    print("[SKIP] handler diagnostic já instalado")


# ============================================================
# 4. Verificação final
# ============================================================

packet_final = read(PACKET)
record_final = read(RECORD)
server_final = read(SERVER)

checks = [
    (
        MARK_PACKET in packet_final,
        "PacketFilter marker",
    ),
    (
        MARK_RECV in record_final,
        "RecvRecordDataFromClient marker",
    ),
    (
        MARK_HANDLER in server_final,
        "handler marker",
    ),
    (
        new_bound_packet in packet_final,
        "PacketFilter bound >",
    ),
    (
        new_bound_recv in record_final,
        "RecvRecordDataFromClient bound >",
    ),
]

failed = False

print()
print("===== VERIFICACAO =====")

for ok, label in checks:
    print(f"{'OK' if ok else 'FAIL'}  {label}")
    if not ok:
        failed = True

if failed:
    raise SystemExit("ERRO: verificação final falhou")

print()
print("PATCH R25 RECORD DIAG: OK")
