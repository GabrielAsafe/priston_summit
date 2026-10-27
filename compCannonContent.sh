cd "$HOME/Área de Trabalho/summit" && \
python3 SourceCode/tools/content_port/consolidate_canonical_content.py \
  --repo-root "$PWD" && \
python3 SourceCode/tools/content_port/validate_runtime_pack_resolution.py \
  --repo-root "$PWD" && \
echo "CONTENT_CANONICO_COMPILADO_E_DISTRIBUIDO_COM_SUCESSO"
