# Summit Tale - Bugfix History

Cada correção deve ser aplicada isoladamente.

## Regra

Um bugfix por vez.

Fluxo obrigatório:

1. Identificar exatamente o problema.
2. Identificar todos os ficheiros que serão alterados.
3. Guardar uma cópia BEFORE de cada ficheiro.
4. Calcular SHA-256 BEFORE.
5. Aplicar somente o fix atual.
6. Guardar uma cópia AFTER de cada ficheiro.
7. Calcular SHA-256 AFTER.
8. Gerar o patch Git do fix.
9. Documentar:
   - problema;
   - causa;
   - alteração realizada;
   - ficheiros alterados;
   - hashes before/after;
   - build realizado;
   - teste realizado;
   - resultado.
10. Testar.
11. Somente se aprovado, criar o commit.
12. Só então iniciar o próximo fix.

## Estrutura

bugfixes/NNN_nome_do_fix/
    README.md
    hashes_before.sha256
    hashes_after.sha256
    change.patch
    before/
    after/

## Regras adicionais

- Não misturar fixes diferentes.
- Não alterar items/drop/merchant enquanto essa área estiver congelada.
- Não apagar instrumentação sem um fix específico para cleanup.
- Cliente: usar cliente.sh para build + deploy.
- Servidor: usar server.sh para build + deploy.
- Configurações necessárias ao runtime devem fazer parte do processo de deploy.
- Servidor e cliente testados devem corresponder exatamente ao estado documentado no commit.
