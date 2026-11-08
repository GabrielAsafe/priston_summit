# Summit V2 — Análise cruzada das provas de modelos e referências (01/11/2026)

## Origem, âmbito e limites

- `SUMMIT_PROVAS_MODELOS_V2_FEDORA(1).zip`: 4 relatórios de provas; integridade ZIP verificada.
- `SUMMIT_REFERENCIAS_INI_INX_20261101T160136Z.zip`: 3 relatórios de referências; integridade ZIP verificada.
- Cruzamento com `SUMMIT_RETOMADA_VISUAIS_ASSETS_V2_20261101(1).zip` (snapshot parcial de 10:02 UTC).
- Verificação feita nesta sessão sobre os **relatórios e os assets previamente incluídos no ZIP de Retomada**, não sobre o filesystem Fedora atual. Os ZIPs novos **não incluem os ficheiros binários SMD/INX/INI** nem executáveis do jogo; portanto não permitem certificar renderização, animação ou compilação completa.

## Verificações independentes

- 22 linhas de provas do Dragão; 22/22 pares com SHA256 **SMD e INX** iguais aos bytes que constam no ZIP de Retomada anterior.
- 0 inconsistências declaradas no relatório de provas do Fedora; não se estende a prova à renderização Wine.
- Três evidências já presentes na Retomada são byte-a-byte iguais aos novos ficheiros: resultados=Sim; matriz=Sim; catálogo candidato=Sim.
- O relatório de referências contém 2107 INX, 1979 SMD e 5 INI pesquisados pelo verificador do Fedora (números provenientes do relatório, não contados de novo no projeto).
- 10 modelos têm uma ligação interna reportada; 1 modelo não tem ligação no conjunto pesquisado.
- Foram encontradas **zero** fontes INI para os 11 modelos-alvo (`ini_sources=[]` em todos). O campo `inx_links[].ini` tem forma de caminho lógico/nome inferido; não é um INI comprovado em disco.
- Todos os dez `model_reference` do INX apontam textualmente para nomes `.ASE`, com basename correspondente aos dez `.smd` pesquisados; é correspondência de referência interna, **não validação de skeleton, animações, formato da malha nem resolução runtime**.

## Mapeamento encontrado — 10 candidatos Futebol

| Modelo físico SMD | INX com referência interna | Referência de movimento |
|---|---|---|
| `CtmbA11.smd` | `Ca011.inx` | `char\tmABCD\M1Bip.in` |
| `CtmbB11.smd` | `Cb011.inx` | `char\tmABCD\M1Bip.in` |
| `CtmbC11.smd` | `Cc011.inx` | `char\tmABCD\M4Bip.in` |
| `CmmbA11.smd` | `Cma011.inx` | `char\tmABCD\M1Bip.in` |
| `CmmbD11.smd` | `Cmd011.inx` | `char\tmABCD\M3Bip.in` |
| `CmmbE11.smd` | `Cme011.inx` | `char\tmABCD\M7Bip.in` |
| `CtfbD11.smd` | `Cd011.inx` | `char\tmABCD\M2Bip.in` |
| `CtfbE11.smd` | `Ce011.inx` | `char\tmABCD\M6bip.in` |
| `CmfbB11.smd` | `Cmb011.inx` | `char\tmABCD\M2Bip.in` |
| `CmfbC11.smd` | `Cmc011.inx` | `char\tmABCD\M5Bip.in` |

**Importante:** o relatório antigo classificava estes dez candidatos como `INX_AUSENTE` por procurar INX de *mesmo basename*. O relatório de referência posterior encontra INX com **outros nomes**. As duas observações tornam-se compatíveis depois de corrigido esse critério de procura. O ficheiro CSV anexo contém os caminhos, referências `.ASE`, texturas e hashes de cada correspondência.

## Pendência: Uniforme

- `cliente/char/tmABCD/StylishUniform.smd`: `SEM_REFERENCIA_NO_CONJUNTO`; zero `inx_links` e zero `ini_sources`.
- O SMD é candidato real e contém indícios das texturas `stylishuniform1.tga`, `stylishuniform2.tga` e `stylishuniform.bmp`, com marcador de malha `fig_pa_01body`.
- O resultado **não demonstra que o SMD esteja corrompido, nem que um INX não exista fora da pesquisa**. A ausência de fontes INI para todos os onze modelos impede fechar a cadeia de configuração mesmo para Futebol.
- Antes de criar alguma cache INX, localizar o carregamento existente do Uniforme, o INI fonte usado no projeto ou ferramentas oficiais de assets, as animações/skeleton e o papel do Uniforme como malha de traje, corpo ou armadura. Não clonar/renomear INX às cegas.

## Perfis e bindings de classe

- `CA871/872/873/874`, `DA147/247/153/253`: IDs numéricos cliente/servidor coincidentes **no relatório de provas**.
- `CA115 = 0x02105F00` e `CA715 = 0x02115F00`: aparecem apenas no perfil cliente do snapshot analisado; investigar se o servidor atual tem outra forma de registo ou já recebeu uma correção. Não concluir que estão ausentes do servidor em execução.
- O catálogo proposto contém 22 bindings para quatro Dragões, contra 16 no catálogo de origem: novos bindings `shaman`, `guardian` e `scout` (dois por classe).
- O compilador incluído na Retomada original só reconhece oito classes clássicas. Portanto o catálogo proposto com 22 associações não é instalável isoladamente; exige primeiro o mapeamento real de job codes e a integração C++/Content compatível.

## O que a evidência **não** prova

1. Que `CA115` ou `CA715` usam os dez SMD encontrados, ou qual seria o binding para cada classe/género. Textura `_WC` sugere família temática; não prova vínculo ao item.
2. Que o ficheiro `Ca011.ini` ou os outros caminhos aparentes `.ini` existam. Os campos de INI são apenas nomes associados ao INX e os `ini_sources` estão vazios.
3. Que os `M1Bip.in` etc. e as texturas estejam íntegros e resolvidos pelo carregador Wine em tempo de execução.
4. Que o compilador oficial PT Asset Tool não exista no Fedora. `compiler_found=[]` é apenas resultado da procura feita por este verificador.
5. Que equipamento V2, clique direito, alternância da armadura, compras, inventário ou crafting tenham sido corrigidos por estes relatórios; estes ZIPs são apenas provas de leitura.

## Sequência de trabalho preservando a Retomada

1. **Não instalar nenhum destes ZIPs** — são relatórios. Manter código vivo do Fedora como base e preservar a frente concorrente de crafting.
2. Documentar o elo `modelo SMD/ASE ↔ INX de nome diferente` e verificar em `smModelDecode` se os dez INX são selecionados em runtime por INI, registro de personagem ou outra via; recuperar INI fonte real se existir.
3. Resolver a ausência de referência do `StylishUniform.smd`: classificar função do mesh, movimentos e seleção do renderer para `DA147/153/247/253`; não usar fallback para Dragão.
4. Verificar no checkout atual o mapa de IDs `CA115/CA715`, o Content cliente/servidor e os itens já persistidos; jamais renumerar por dedução.
5. Integrar as 3 classes adicionais do Dragão no compilador, Content e runtime (se job codes concretos existirem); não apenas copiar o JSON proposto.
6. Fechar com um único conjunto coerente e teste Wine por classe, equipar/desequipar, mochila, swaps 3/14, crash/relogin, compra duplicada e expiração 7/30 dias.

**Conclusão:** o Futebol avançou de uma busca de pares por nome para dez associações internas concretas, mas o fecho visual ainda depende da cadeia de carregamento e do vínculo aos códigos de item. O Uniforme permanece o bloqueio principal; o Dragão tem pares físicos, mas não render confirmado.
