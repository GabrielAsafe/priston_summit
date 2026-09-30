#!/usr/bin/env python3
"""Verify that selected C/C++ files changed only in comments/whitespace.

The comparison tokenizes the base Git blob and working-tree file while dropping
comments and whitespace. Strings, character literals, identifiers, operators,
preprocessor tokens and punctuation are preserved. Identical token streams are
strong evidence that a documentation-only annotation pass did not modify C/C++
semantics or preprocessor structure.
"""
from __future__ import annotations

import argparse
import subprocess
import sys
from pathlib import Path


def git_blob(root: Path, base: str, rel: str) -> str:
    proc = subprocess.run(
        ["git", "show", f"{base}:{rel}"],
        cwd=root,
        stdout=subprocess.PIPE,
        stderr=subprocess.PIPE,
    )
    if proc.returncode:
        message = proc.stderr.decode("utf-8", errors="replace").strip()
        raise RuntimeError(message or f"unable to read {base}:{rel}")
    return proc.stdout.decode("utf-8", errors="surrogateescape")


def tokens(text: str) -> list[str]:
    result: list[str] = []
    i = 0
    length = len(text)
    two_char = {
        "::", "->", "++", "--", "==", "!=", "<=", ">=", "&&", "||", "+=", "-=",
        "*=", "/=", "%=", "&=", "|=", "^=", "<<", ">>", "##", ".*", "->*",
    }
    three_char = {"<<=", ">>=", "...", "->*"}
    while i < length:
        c = text[i]
        n = text[i + 1] if i + 1 < length else ""
        if c.isspace():
            i += 1
            continue
        if c == "/" and n == "/":
            i += 2
            while i < length and text[i] != "\n":
                i += 1
            continue
        if c == "/" and n == "*":
            i += 2
            while i + 1 < length and not (text[i] == "*" and text[i + 1] == "/"):
                i += 1
            i = min(length, i + 2)
            continue
        if c in {'"', "'"}:
            quote = c
            start = i
            i += 1
            while i < length:
                if text[i] == "\\":
                    i += 2
                    continue
                if text[i] == quote:
                    i += 1
                    break
                i += 1
            result.append(text[start:i])
            continue
        if c.isalpha() or c == "_":
            start = i
            i += 1
            while i < length and (text[i].isalnum() or text[i] == "_"):
                i += 1
            result.append(text[start:i])
            continue
        if c.isdigit():
            start = i
            i += 1
            while i < length and (text[i].isalnum() or text[i] in "._'"):
                i += 1
            result.append(text[start:i])
            continue
        if text[i : i + 3] in three_char:
            result.append(text[i : i + 3])
            i += 3
            continue
        if text[i : i + 2] in two_char:
            result.append(text[i : i + 2])
            i += 2
            continue
        result.append(c)
        i += 1
    return result


def first_difference(left: list[str], right: list[str]) -> tuple[int, str, str] | None:
    count = min(len(left), len(right))
    for index in range(count):
        if left[index] != right[index]:
            return index, left[index], right[index]
    if len(left) != len(right):
        return count, "<end>" if count == len(left) else left[count], "<end>" if count == len(right) else right[count]
    return None


def main() -> int:
    parser = argparse.ArgumentParser()
    parser.add_argument("--base", default="HEAD", help="Git revision used as the semantic baseline")
    parser.add_argument("files", nargs="+", help="tracked C/C++ files to verify")
    args = parser.parse_args()

    root = Path(subprocess.check_output(["git", "rev-parse", "--show-toplevel"], text=True).strip())
    failed = False
    for value in args.files:
        path = (root / value).resolve() if not Path(value).is_absolute() else Path(value).resolve()
        try:
            rel = path.relative_to(root).as_posix()
        except ValueError:
            print(f"FAIL: outside repository: {path}")
            failed = True
            continue
        if not path.exists():
            print(f"FAIL: working-tree file missing: {rel}")
            failed = True
            continue
        try:
            before = git_blob(root, args.base, rel)
        except RuntimeError as exc:
            print(f"FAIL: {rel}: {exc}")
            failed = True
            continue
        after = path.read_text(errors="surrogateescape")
        before_tokens = tokens(before)
        after_tokens = tokens(after)
        diff = first_difference(before_tokens, after_tokens)
        if diff:
            index, old, new = diff
            print(f"FAIL: {rel}: non-comment token changed at token {index}: {old!r} -> {new!r}")
            failed = True
        else:
            print(f"PASS: {rel}: code token stream unchanged ({len(after_tokens)} tokens)")
    if failed:
        return 1
    print("comment-only semantic token guard: PASS")
    return 0


if __name__ == "__main__":
    sys.exit(main())
