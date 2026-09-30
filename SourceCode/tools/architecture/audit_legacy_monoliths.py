#!/usr/bin/env python3
"""Heuristic inventory of large legacy source files and function bodies.

This tool is intentionally dependency-free. It is an audit aid, not a C++ parser.
It strips comments/strings for brace matching and reports likely function bodies.
Use the output to prioritize documentation/annotation work; do not use it to
rewrite or move legacy code automatically.
"""
from __future__ import annotations

import argparse
import re
from pathlib import Path

SOURCE_GLOBS = ("*.cpp", "*.c", "*.inl", "*.h", "*.hpp")
CONTROL = re.compile(r"^\s*(?:if|for|while|switch|catch|else\s+if|else|do|try)\b")
FUNCTION_TAIL = re.compile(
    r"([~A-Za-z_][\w:~<>]*)\s*\([^()]*\)\s*"
    r"(?:const\b|noexcept\b|override\b|final\b|->[^=]+|:\s*[^{}]+)?\s*$"
)


def clean_lines(text: str) -> list[str]:
    out: list[str] = []
    i = 0
    state = "code"
    line = ""
    while i < len(text):
        c = text[i]
        n = text[i + 1] if i + 1 < len(text) else ""
        if state == "code":
            if c == "/" and n == "/":
                state = "line_comment"
                i += 2
                continue
            if c == "/" and n == "*":
                state = "block_comment"
                i += 2
                continue
            if c == '"':
                state = "string"
                line += " "
                i += 1
                continue
            if c == "'":
                state = "char"
                line += " "
                i += 1
                continue
            line += c
            if c == "\n":
                out.append(line[:-1])
                line = ""
            i += 1
            continue
        if state == "line_comment":
            if c == "\n":
                out.append(line)
                line = ""
                state = "code"
            i += 1
            continue
        if state == "block_comment":
            if c == "\n":
                out.append(line)
                line = ""
            elif c == "*" and n == "/":
                state = "code"
                i += 2
                continue
            i += 1
            continue
        quote = '"' if state == "string" else "'"
        if c == "\\":
            i += 2
            continue
        if c == quote:
            state = "code"
            i += 1
            continue
        if c == "\n":
            out.append(line)
            line = ""
            state = "code"
        i += 1
    if line:
        out.append(line)
    return out


def signature_before(lines: list[str], line_index: int, column: int) -> str:
    pieces = [lines[line_index][:column]]
    for j in range(line_index - 1, max(-1, line_index - 25), -1):
        value = lines[j]
        if re.match(r"^\s*#", value):
            break
        if ";" in value or "{" in value or "}" in value:
            pos = max(value.rfind(";"), value.rfind("{"), value.rfind("}"))
            tail = value[pos + 1 :]
            if tail.strip():
                pieces.append(tail)
            break
        pieces.append(value)
    return " ".join(reversed(pieces)).strip()


def probable_functions(path: Path) -> list[tuple[int, int, int, str]]:
    lines = clean_lines(path.read_text(errors="ignore"))
    stack: list[tuple[int, int, tuple[str, str] | None]] = []
    found: list[tuple[int, int, int, str]] = []
    for li, line in enumerate(lines):
        for column, char in enumerate(line):
            if char == "{":
                signature = signature_before(lines, li, column)
                candidate = None
                if "(" in signature and ")" in signature and not CONTROL.match(signature):
                    match = FUNCTION_TAIL.search(signature)
                    if match and match.group(1) not in {"if", "for", "while", "switch", "catch", "sizeof"}:
                        candidate = (signature, match.group(1))
                stack.append((li, column, candidate))
            elif char == "}" and stack:
                start_line, _start_column, candidate = stack.pop()
                if candidate and not any(entry[2] is not None for entry in stack):
                    found.append((li - start_line + 1, start_line + 1, li + 1, candidate[1]))
    return found


def main() -> int:
    parser = argparse.ArgumentParser()
    parser.add_argument("--root", default=str(Path(__file__).resolve().parents[2]))
    parser.add_argument("--top-files", type=int, default=30)
    parser.add_argument("--top-functions", type=int, default=30)
    parser.add_argument("--min-file-lines", type=int, default=1000)
    args = parser.parse_args()
    root = Path(args.root).resolve()

    paths: set[Path] = set()
    for pattern in SOURCE_GLOBS:
        paths.update(root.rglob(pattern))
    paths = {p for p in paths if ".git" not in p.parts}

    file_rows = []
    function_rows = []
    for path in sorted(paths):
        text = path.read_text(errors="ignore")
        line_count = text.count("\n") + (0 if not text or text.endswith("\n") else 1)
        rel = path.relative_to(root).as_posix()
        if line_count >= args.min_file_lines:
            file_rows.append((line_count, rel))
        if path.suffix.lower() in {".cpp", ".c", ".inl", ".h", ".hpp"}:
            for size, start, end, name in probable_functions(path):
                function_rows.append((size, rel, start, end, name))

    file_rows.sort(reverse=True)
    function_rows.sort(reverse=True)

    print("# Large legacy/source files")
    for lines, rel in file_rows[: args.top_files]:
        print(f"{lines:6d}  {rel}")
    print("\n# Largest probable function bodies (heuristic)")
    for size, rel, start, end, name in function_rows[: args.top_functions]:
        print(f"{size:6d}  {rel}:{start}-{end}  {name}")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
