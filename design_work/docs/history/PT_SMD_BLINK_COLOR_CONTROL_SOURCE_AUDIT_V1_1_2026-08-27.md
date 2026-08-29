# PT SMD Blink Color Control Source Audit V1.1

Hotfix for V1.

The V1 scanner failed during startup because the regex for numeric BlinkStep /
WindMeshBottom candidates contained an extra escaped backslash before `)`.

Incorrect:

```python
r"(?:BlinkStep|WindMeshBottom)\s*(?:=|,|\\))\s*(-?[0-9]+)"
```

Correct:

```python
r"(?:BlinkStep|WindMeshBottom)\s*(?:=|,|\))\s*(-?[0-9]+)"
```

No audit semantics were weakened or changed.

Run:

```bash
python3 pt_smd_blink_color_control_source_audit_v1_1.py \
  --source-root "/home/gabriel/Área de Trabalho/summit" \
  --report logs/pt_smd_blink_color_control_source_audit_v1_1.json \
  > logs/pt_smd_blink_color_control_source_audit_v1_1.txt

cat logs/pt_smd_blink_color_control_source_audit_v1_1.txt
```

Read-only:
- source_modified=False
- client_modified=False
