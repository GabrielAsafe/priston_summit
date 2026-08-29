SUMMIT — QUARENTENA REVERSÍVEL
================================

Objetivo
--------
Mover candidatos a recurso legado para fora das árvores de runtime sem apagar nada.
Cliente e servidor possuem destinos de quarentena separados.

Diretório de trabalho esperado
------------------------------
cd "/home/gabriel/Área de Trabalho/summit/audits"

Arquivos do pacote
------------------
- summit_quarantine.py
- quarantine_client_candidates.csv
- quarantine_server_candidates.csv

Destinos
--------
Cliente:
  /home/gabriel/Área de Trabalho/summit/audits/quarantine/cliente/<RUN_ID>/...

Servidor:
  /home/gabriel/Área de Trabalho/summit/audits/quarantine/servidor/<RUN_ID>/...

Manifestos:
  /home/gabriel/Área de Trabalho/summit/audits/quarantine/manifests/<RUN_ID>.csv
  /home/gabriel/Área de Trabalho/summit/audits/quarantine/manifests/<RUN_ID>.json

Segurança
---------
- O script nunca usa rm.
- plan é somente simulação.
- apply move os arquivos preservando a estrutura relativa.
- restore restaura a partir do manifesto.
- Caminhos absolutos e caminhos contendo '..' são recusados.
- Restore não sobrescreve um arquivo/diretório que já reapareceu no local original.
- O plano inicial do servidor está vazio de propósito: nenhuma movimentação do servidor foi autorizada pela auditoria de assets desta rodada.

Primeiro uso
------------
1) Ver o plano sem mover nada:

  cd "/home/gabriel/Área de Trabalho/summit/audits"
  python3 summit_quarantine.py plan --run-id field_legacy_20260825

2) Conferir a lista:

  column -s, -t < quarantine_client_candidates.csv | less -S

3) Aplicar a quarentena:

  python3 summit_quarantine.py apply --run-id field_legacy_20260825

4) Ver status:

  python3 summit_quarantine.py status

5) Se houver regressão, restaurar:

  python3 summit_quarantine.py restore --run-id field_legacy_20260825

Plano inicial do cliente
------------------------
Inclui:
- Field/old_forest
- Field/old_dungeon
- mapas/títulos associados sem SetName() ativo
- Field/forever-fall/3ani-08.ASE
- Field/cave/Dcave_anii14.ASE
- Field/Ricarten/world_ani06.ASE

Não inclui automaticamente:
- arquivos do servidor;
- Field/ani/test.ASE e Field/ani/test-ani.ASE;
- famílias char/temp ainda sem teste direcionado;
- arquivos simplesmente classificados como UNREFERENCED_STATIC.

Fluxo de validação recomendado
------------------------------
Depois do apply:
1. Iniciar o cliente.
2. Testar login e seleção de personagem.
3. Entrar em mapas forest/dungeon/Ricarten/cave/forever-fall relevantes.
4. Observar texturas ausentes, falhas de stage/model e logs.
5. Se houver regressão, executar restore imediatamente.
6. Se o teste passar, manter os arquivos em quarentena por um ciclo de validação antes de exclusão definitiva.
