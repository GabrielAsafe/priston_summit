#!/usr/bin/env python3
"""
summit_encoding_audit.py

Auditoria somente-leitura para localizar corrupção de encoding e fontes
potenciais de recuperação em árvores conhecidas do Priston Tale.

Princípios:
- NÃO altera arquivos.
- NÃO varre o HDD inteiro.
- Só percorre roots explicitamente passados.
- Trabalha em bytes sempre que possível.
- "latin-1 decodifica" NÃO é usado como evidência de encoding correto.
- U+FFFD (EF BF BD) é tratado como perda já gravada no arquivo.
- CP949 é testado explicitamente por ser relevante ao source coreano original.

Exemplo:
  python3 summit_encoding_audit.py \
    --target "/home/gabriel/Área de Trabalho/summit/SourceCode/SrcGame/src" \
    --target "/home/gabriel/Área de Trabalho/summit/SourceCode/SrcServer/src" \
    --source "pstale=/home/gabriel/armazem/priston/pristontale-1-master/src" \
    --scope "Language/Korean" \
    --out "/home/gabriel/Área de Trabalho/summit/audits/encoding_audit"

Também pode auditar um arquivo/relpath específico:
  python3 summit_encoding_audit.py ... \
    --only "Language/Korean/k_ServerMsg.h"
"""

from __future__ import annotations

import argparse
import csv
import hashlib
import re
from dataclasses import dataclass, asdict
from pathlib import Path
from typing import Iterable

FFFD = b"\xef\xbf\xbd"
TEXT_EXTS = {
    ".c", ".cc", ".cpp", ".cxx", ".h", ".hh", ".hpp",
    ".txt", ".ini", ".cfg", ".lua", ".rc", ".def", ".dat"
}

@dataclass
class FileInfo:
    root_kind: str
    root_name: str
    root_path: str
    relpath: str
    fullpath: str
    size: int
    sha256: str
    fffd_count: int
    utf8_valid: bool
    cp949_valid: bool
    ascii_only: bool
    nul_count: int
    likely_text: bool

def sha256_bytes(data: bytes) -> str:
    return hashlib.sha256(data).hexdigest()

def inspect_file(path: Path, root: Path, kind: str, name: str) -> FileInfo:
    data = path.read_bytes()

    try:
        data.decode("utf-8")
        utf8_valid = True
    except UnicodeDecodeError:
        utf8_valid = False

    try:
        data.decode("cp949")
        cp949_valid = True
    except UnicodeDecodeError:
        cp949_valid = False

    ascii_only = all(b < 128 for b in data)
    nul_count = data.count(b"\x00")
    likely_text = path.suffix.lower() in TEXT_EXTS and nul_count == 0

    return FileInfo(
        root_kind=kind,
        root_name=name,
        root_path=str(root),
        relpath=str(path.relative_to(root)),
        fullpath=str(path),
        size=len(data),
        sha256=sha256_bytes(data),
        fffd_count=data.count(FFFD),
        utf8_valid=utf8_valid,
        cp949_valid=cp949_valid,
        ascii_only=ascii_only,
        nul_count=nul_count,
        likely_text=likely_text,
    )

def iter_files(root: Path, scopes: list[str], only: list[str]) -> Iterable[Path]:
    # Restrição forte: se --only existe, não há rglob.
    if only:
        for rel in only:
            p = root / rel
            if p.is_file():
                yield p
        return

    bases = [root / s for s in scopes] if scopes else [root]

    for base in bases:
        if base.is_file():
            yield base
            continue
        if not base.is_dir():
            continue
        for p in base.rglob("*"):
            if p.is_file() and (p.suffix.lower() in TEXT_EXTS):
                yield p

def parse_source(value: str) -> tuple[str, Path]:
    if "=" not in value:
        raise argparse.ArgumentTypeError(
            "--source deve ser NOME=/caminho, ex.: pstale=/home/.../pristontale-1-master/src"
        )
    name, path = value.split("=", 1)
    return name.strip(), Path(path).expanduser()

def structural_signature(data: bytes) -> str:
    """
    Assinatura aproximada da estrutura C/C++:
    - decodifica preservando bytes desconhecidos como replacement apenas em memória;
    - remove conteúdo de strings;
    - normaliza whitespace;
    Útil para indicar parentesco estrutural, NÃO para decidir recuperação sozinho.
    """
    text = data.decode("utf-8", errors="replace")
    text = re.sub(r'"(?:\\.|[^"\\])*"', '"STRING"', text)
    text = re.sub(r"'(?:\\.|[^'\\])*'", "'CHAR'", text)
    text = re.sub(r"\s+", " ", text).strip()
    return hashlib.sha256(text.encode("utf-8")).hexdigest()

def main() -> int:
    ap = argparse.ArgumentParser()
    ap.add_argument("--target", action="append", required=True,
                    help="Root alvo. Pode repetir.")
    ap.add_argument("--source", action="append", default=[],
                    help="Fonte conhecida no formato NOME=/caminho")
    ap.add_argument("--scope", action="append", default=[],
                    help="Subárvore relativa a auditar, ex. Language/Korean")
    ap.add_argument("--only", action="append", default=[],
                    help="Relpath exato. Evita qualquer varredura.")
    ap.add_argument("--out", required=True, help="Diretório de saída")
    args = ap.parse_args()

    out = Path(args.out).expanduser()
    out.mkdir(parents=True, exist_ok=True)

    targets = [Path(p).expanduser() for p in args.target]
    sources = [parse_source(s) for s in args.source]

    infos: list[FileInfo] = []
    by_rel: dict[tuple[str, str], FileInfo] = {}

    for idx, root in enumerate(targets, 1):
        root = root.resolve()
        name = f"target{idx}"
        for p in iter_files(root, args.scope, args.only):
            info = inspect_file(p, root, "target", name)
            infos.append(info)
            by_rel[(name, info.relpath)] = info

    source_infos: dict[str, dict[str, FileInfo]] = {}
    for name, root in sources:
        root = root.resolve()
        source_infos[name] = {}
        for p in iter_files(root, args.scope, args.only):
            info = inspect_file(p, root, "source", name)
            infos.append(info)
            source_infos[name][info.relpath] = info

    # TSV completo.
    tsv = out / "files.tsv"
    fields = list(asdict(infos[0]).keys()) if infos else [
        "root_kind","root_name","root_path","relpath","fullpath","size",
        "sha256","fffd_count","utf8_valid","cp949_valid","ascii_only",
        "nul_count","likely_text"
    ]
    with tsv.open("w", encoding="utf-8", newline="") as f:
        w = csv.DictWriter(f, fields, delimiter="\t")
        w.writeheader()
        for x in infos:
            w.writerow(asdict(x))

    # Candidatos: target com FFFD e fonte equivalente sem FFFD.
    candidates = []
    for t in [x for x in infos if x.root_kind == "target" and x.fffd_count]:
        for sname, mapping in source_infos.items():
            s = mapping.get(t.relpath)
            if not s:
                continue

            tdata = Path(t.fullpath).read_bytes()
            sdata = Path(s.fullpath).read_bytes()

            candidates.append({
                "target": t.fullpath,
                "target_relpath": t.relpath,
                "target_fffd": t.fffd_count,
                "source_name": sname,
                "source": s.fullpath,
                "source_fffd": s.fffd_count,
                "source_utf8_valid": s.utf8_valid,
                "source_cp949_valid": s.cp949_valid,
                "same_sha256": t.sha256 == s.sha256,
                "same_structure_signature":
                    structural_signature(tdata) == structural_signature(sdata),
                "target_size": t.size,
                "source_size": s.size,
            })

    cand_tsv = out / "recovery_candidates.tsv"
    cfields = [
        "target","target_relpath","target_fffd","source_name","source",
        "source_fffd","source_utf8_valid","source_cp949_valid",
        "same_sha256","same_structure_signature","target_size","source_size"
    ]
    with cand_tsv.open("w", encoding="utf-8", newline="") as f:
        w = csv.DictWriter(f, cfields, delimiter="\t")
        w.writeheader()
        for c in candidates:
            w.writerow(c)

    # Relatório Markdown.
    report = out / "REPORT.md"
    target_files = [x for x in infos if x.root_kind == "target"]
    source_files = [x for x in infos if x.root_kind == "source"]

    clean_source_candidates = [
        c for c in candidates
        if c["target_fffd"] > 0
        and c["source_fffd"] == 0
        and c["source_cp949_valid"]
    ]

    with report.open("w", encoding="utf-8") as f:
        f.write("# Encoding Audit Report\n\n")
        f.write("Auditoria somente-leitura. Nenhum arquivo foi convertido ou modificado.\n\n")

        f.write("## Resumo\n\n")
        f.write(f"- Arquivos alvo analisados: {len(target_files)}\n")
        f.write(f"- Arquivos fonte analisados: {len(source_files)}\n")
        f.write(f"- Alvos contendo U+FFFD: {sum(x.fffd_count > 0 for x in target_files)}\n")
        f.write(f"- Total de U+FFFD nos alvos: {sum(x.fffd_count for x in target_files)}\n")
        f.write(f"- Candidatos com fonte equivalente limpa e CP949 válido: {len(clean_source_candidates)}\n\n")

        f.write("## Regra de interpretação\n\n")
        f.write("- `U+FFFD > 0`: a perda já está gravada no arquivo alvo.\n")
        f.write("- `CP949 válido + U+FFFD=0` numa fonte antiga: forte candidato, não prova sozinho equivalência semântica.\n")
        f.write("- `latin-1` não é usado como prova porque aceita praticamente qualquer byte.\n")
        f.write("- Nenhuma substituição automática deve ser feita a partir deste relatório.\n\n")

        f.write("## Melhores candidatos de recuperação\n\n")
        if not clean_source_candidates:
            f.write("Nenhum candidato encontrado com os critérios atuais.\n")
        else:
            f.write("| Target | FFFD | Fonte | CP949 | Estrutura igual |\n")
            f.write("|---|---:|---|:---:|:---:|\n")
            for c in sorted(clean_source_candidates,
                            key=lambda x: (-x["target_fffd"], x["target_relpath"])):
                f.write(
                    f"| `{c['target_relpath']}` | {c['target_fffd']} | "
                    f"`{c['source_name']}` | sim | "
                    f"{'sim' if c['same_structure_signature'] else 'não'} |\n"
                )

        f.write("\n## Próxima ação recomendada\n\n")
        f.write(
            "Selecionar um único candidato, comparar estrutura/identificadores/índices, "
            "extrair a fonte em memória com CP949 e criar um bugfix BEFORE/AFTER. "
            "Não substituir arquivos inteiros sem provar compatibilidade.\n"
        )

    print(f"OK: {report}")
    print(f"OK: {tsv}")
    print(f"OK: {cand_tsv}")
    return 0

if __name__ == "__main__":
    raise SystemExit(main())
