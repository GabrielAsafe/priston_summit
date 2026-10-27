# Gathering no Content canónico — piloto (mapa 58)

- `nodes.experimental.json` contém exatamente os nós `910001` e `910002` validados em jogo. O servidor atual **não** carrega JSON: lê SQLite `ServerDB__GatheringNodeDefinition` e `ServerDB__GatheringResourceDefinition` no arranque.
- `resources.reference.json` documenta as referências à tabela de recursos. Não redefine nem altera as 9 definições da base de dados.
- `visuals.experimental.json` e `assets/*_preview.*` são **ícones de conceito ilustrativos**. Não são .ASE/.SMD, não criam NPCs/objetos 3D, nem entram automaticamente na UI atual. A imagem infográfica usada como origem é uma ilustração, não uma captura verdadeira do jogo.
- Para importar dados do JSON canónico, usar `instalar_nodos_experimentais.py` do pacote anterior em dry-run; nós já aplicados deverão mostrar `ALREADY_PRESENT=2`, `PENDING=0`.
- O local da StarArena 58 é apenas laboratório; não alterar `center/start_points` nem mover o jogador para estas coordenadas.
- Os testes confirmaram persistência do material `iron_ore` de 2 para 4, mas `ServerDB__ProfessionProgress` continua 0. Não declarar XP funcional ainda.
- O pipeline `compCannonContent.sh` inclui a pasta canónica no manifesto `CANONICAL_SOURCE.json`; **não compila nem publica os nós em packs atualmente**. A sincronização com a DB permanece explícita.
