import json, re, os, shutil, pathlib, subprocess, textwrap, difflib
ROOT=pathlib.Path('/mnt/data/audit_game')
SRC=ROOT/'src'; CONTENT=ROOT/'content'/'Content'; OUT=ROOT/'out'; PATCH=OUT/'patched'
if PATCH.exists(): shutil.rmtree(PATCH)
shutil.copytree(SRC,PATCH/'src')
shutil.copytree(CONTENT,PATCH/'Content')

# 1) Correct clear non-PT contamination in pt-BR catalog.
pt_fixes={
'item.2_wd109.name':'Adaga de Platina','item.2_wn109.name':'Fantasma Tatanka','item.bc133.name':'Redução de Recarga 4','item.bi200.name':'Bolsa da Sorte','item.uwd121_enigma_dagger.name':'Adaga Enigma','item.uwn121_kahsehtha_phantom.name':'Fantasma Kahsehtha',
'item.wd101.name':'Adaga Normal','item.wd102.name':'Adaga de Ferro','item.wd103.name':'Faca de Caça','item.wd104.name':'Cutelo','item.wd105.name':'Hanger','item.wd106.name':'Adaga de Aço','item.wd107.name':'Faca do Abate','item.wd108.name':'Adaga Sagrada','item.wd109.name':'Falcione','item.wd110.name':'Adaga Vine','item.wd111.name':'Adaga do Abismo','item.wd112.name':'Espada Dupla Alada','item.wd113.name':'Adaga Sol','item.wd114.name':'Flamberge','item.wd115.name':'Adaga Fortaleza','item.wd116.name':'Espada Dupla de Lava','item.wd117.name':'Adaga Arcana','item.wd118.name':'Adaga Pedra Lunar','item.wd119.name':'Adaga Caótica','item.wd120.name':'Adaga Saqueadora','item.wd122.name':'Espada Dupla do Dragão','item.wd123.name':'Adaga Devilian','item.wd124.name':'Adaga Brilho Sombrio','item.wd125.name':'Adaga de Cristal','item.wd126.name':'Adaga Mitológica','item.wd127.name':'Adaga Mortal','item.wd128.name':'Adaga Flamejante','item.wd129.name':'Adaga Verus','item.wd151.name':'Adaga Saqueadora (5h)','item.wd152.name':'Espada Dupla do Dragão (5h)','item.wd153.name':'Adaga de Suporte',
}
# Phantom family: preserve proper name, localize type.
phantoms={'wn101':'Escudo','wn102':'Creak','wn103':'Pin','wn104':'Terte','wn105':'Okute','wn106':'Wankala','wn107':'Paha','wn108':'Piskin','wn109':'Tatanka','wn110':'Sosho','wn111':'Yumni','wn112':'Siha','wn113':'Wi','wn114':'Roan','wn115':'Canku','wn116':'Owa','wn117':'Nkeez','wn118':'Pejula','wn119':'Luta','wn120':'Natanh','wn122':'Niyaha','wn123':'Acala','wn124':'Wakinyan','wn125':'Wakan','wn126':'Mitológico','wn127':'Mortal','wn128':'Flamejante','wn129':'Verus'}
for code,name in phantoms.items(): pt_fixes[f'item.{code}.name']=f'Fantasma {name}'
pt_fixes.update({'item.wn151.name':'Fantasma Natanh (5h)','item.wn152.name':'Fantasma Niyaha (5h)','item.wn153.name':'Fantasma de Suporte (5h)',
'npc.babelquest.name':'Colecionador Hobgoblin','npc.chocolate.name':'Mercador de Chocolate','npc.coward.name':'Covarde','npc.enquete.name':'Assistente de Pesquisa','npc.eto.name':'Investigador Eto','npc.kamiyu.name':'Investigadora Kamiyu','npc.karastears.name':'Descendente de Kara','npc.lake_guard.path_b82b34c224.name':'Guarda do Lago','npc.magicalgreen.name':'Pesquisador de Magia','npc.navisko_warehouse.chat.1':'Este trabalho também é difícil... Passe por aqui de vez em quando!','npc.navisko_warehouse.name':'Mif, Guardião do Armazém','npc.newter_o_s_u00e1bio.name':'Newter, o Sábio','npc.priston.name':'Namorada de Longbo','npc.pumpkin.name':'Mercador de Abóboras','npc.resercher.name':'Pesquisador de Magia','npc.ricarden_civilian2.chat.1':'Ultimamente muitos guerreiros compram armas e armaduras no mercado negro. Sempre que ouço isso fico frustrado... Acabo me intrometendo!','npc.ricarden_civilian2.name':'Galdin, o Intrometido','npc.ricarden_civilian3.chat.1':'Ah! Dizem que os monstros estão cada vez mais fortes. Precisamos encontrar uma nova forma de caçar... Como será que podemos fazer isso?','npc.ricarden_civilian3.name':'Jacon, o Preocupado','npc.sellquest.name':'Buma','text.mapletree.chat.1':'Até num lugar tão remoto existe a magia de Igolanos!','text.mapletree.chat.2':'Não podemos deixar a magia de Igolanos assim. Precisamos detê-la de algum jeito.'})
for base in [PATCH/'Content', PATCH/'src'/'Content']:
 p=base/'canonical/text/localization/pt-BR.json'
 d=json.load(open(p,encoding='utf8'))
 for e in d['entries']:
  if e['key'] in pt_fixes:
   e['text']=pt_fixes[e['key']]; e['quality']='translated'
 p.write_text(json.dumps(d,ensure_ascii=False,indent=2)+'\n',encoding='utf8')

# 2) Canonical coinshop catalog. Real rows were not supplied, so don't invent inventory.
coin={"schema":"summit.coinshop-catalog/v1","schema_version":1,"notes":["Catálogo de itens da CoinShop. A DB fornecida não contém dados de ItemList/ItemList_T; preencher a partir da instância real antes do deploy.","currency=coin usa UserInfo.UserCoin; currency=time usa UserInfo_T.UserMinutos."],"items":[]}
for base in [PATCH/'Content',PATCH/'src'/'Content']:
 p=base/'canonical/commerce/coinshop.json'; p.parent.mkdir(parents=True,exist_ok=True); p.write_text(json.dumps(coin,ensure_ascii=False,indent=2)+'\n',encoding='utf8')

# 3) Migration removes only static catalog tables. Player balances remain transactional DB state.
mig='''-- Summit SQLite migration 046: move CoinShop static item catalog to canonical JSON.\n-- UserInfo/UserInfo_T intentionally remain in DB because they are mutable player balances.\nDROP TABLE IF EXISTS "CoinShopDB__ItemList";\nDROP TABLE IF EXISTS "CoinShopDB__ItemList_T";\n'''
for dbroot in [PATCH/'src'/'Database/migrations']:
 (dbroot/'046_coinshop_catalog_to_json.sql').write_text(mig,encoding='utf8')

# 4) Auditors: drops, localization, hardcoded strings.
tools=PATCH/'src'/'tools'; tools.mkdir(exist_ok=True)
(tools/'audit_drops.py').write_text(r'''#!/usr/bin/env python3
import argparse,json,re,pathlib
ap=argparse.ArgumentParser();ap.add_argument('--drops',required=True);ap.add_argument('--items',required=True);a=ap.parse_args()
known=set()
for p in pathlib.Path(a.items).glob('*.items.json'):
 d=json.load(open(p,encoding='utf8'));known.update(x.upper() for x in d.get('codes',[]))
rows=[]
for p in pathlib.Path(a.drops).rglob('*.inf'):
 raw=p.read_bytes().decode('latin1','replace')
 for n,line in enumerate(raw.splitlines(),1):
  if line.lstrip().startswith('//') or '*아이템' not in line: continue
  toks=re.findall(r'\b[A-Za-z][A-Za-z0-9_]{2,15}\b',line)
  # first token after probability may be item codes; exclude Korean directive artifacts by known-code check/report
  for t in toks:
   if any(c.isdigit() for c in t) and t.upper() not in known: rows.append((str(p),n,t))
print(json.dumps({'known_item_codes':len(known),'missing_references':[{'file':p,'line':n,'code':c} for p,n,c in rows]},ensure_ascii=False,indent=2))
raise SystemExit(1 if rows else 0)
''',encoding='utf8')
(tools/'audit_localization.py').write_text(r'''#!/usr/bin/env python3
import argparse,json,re,pathlib
ap=argparse.ArgumentParser();ap.add_argument('catalog');a=ap.parse_args();d=json.load(open(a.catalog,encoding='utf8'));loc=d.get('locale','');bad=[]
foreign=re.compile(r'[\uac00-\ud7af\u3040-\u30ff\u4e00-\u9fff]')
moji=re.compile(r'(Ã.|Â.|#U[0-9A-Fa-f]{4}|\ufffd)')
for e in d.get('entries',[]):
 t=e.get('text','')
 reasons=[]
 if loc.startswith(('pt','es')) and foreign.search(t): reasons.append('foreign-script')
 if moji.search(t): reasons.append('encoding/mojibake')
 if reasons: bad.append({'key':e.get('key'),'text':t,'reasons':reasons})
print(json.dumps({'locale':loc,'issues':bad,'count':len(bad)},ensure_ascii=False,indent=2));raise SystemExit(1 if bad else 0)
''',encoding='utf8')
(tools/'audit_hardcoded_text.py').write_text(r'''#!/usr/bin/env python3
import argparse,json,re,pathlib
ap=argparse.ArgumentParser();ap.add_argument('--source',required=True);ap.add_argument('--catalog',required=True);a=ap.parse_args()
d=json.load(open(a.catalog,encoding='utf8'));known={e.get('text','') for e in d.get('entries',[])}
pat=re.compile(r'(?<![A-Za-z0-9_])"((?:\\.|[^"\\]){4,})"')
skip=('systems/localization/locales/','systems/localization/legacy_catalog/','legacy_oracle/','before_')
out=[]
for p in pathlib.Path(a.source).rglob('*'):
 if p.suffix.lower() not in {'.cpp','.h','.inl','.c'} or any(s in str(p).replace('\\','/') for s in skip): continue
 try: txt=p.read_text(encoding='utf8',errors='ignore')
 except: continue
 for n,line in enumerate(txt.splitlines(),1):
  for m in pat.finditer(line):
   s=m.group(1)
   if not re.search(r'[A-Za-zÀ-ÿ]{3}',s) or s in known: continue
   if re.fullmatch(r'[A-Za-z0-9_./\\:%+\-*?=<>\[\](){} ,]+',s) and (' ' not in s or len(s.split())<2): continue
   out.append({'file':str(p),'line':n,'text':s[:240]})
print(json.dumps({'candidates':out,'count':len(out)},ensure_ascii=False,indent=2))
''',encoding='utf8')

# Docs
DOC=PATCH/'src'/'docs'; DOC.mkdir(exist_ok=True)
(DOC/'CONTENT_COMMERCE_EVENTS_AUDIT.md').write_text('''# Auditoria de Content, vendedores, CoinShop, drops e eventos\n\n## Resultado principal\n\n- O Content canónico possui localização `en`, `pt-BR`, `ko` e `zh-TW`; o runtime atual escolhe apenas `pt-BR` para `_LANGUAGE_BRAZIL` e `en` nos demais builds. Não existe catálogo JSON espanhol ativo.\n- `pt-BR.json` continha várias entradas coreanas herdadas. O patch traduz as ocorrências de script coreano encontradas no catálogo PT-BR e inclui um auditor que falha em `foreign-script`/mojibake.\n- Amanda já está em português no catálogo fornecido.\n- A CoinShop carrega catálogo estático de `CoinShopDB.ItemList`/`ItemList_T`. O ZIP de DB contém apenas migrations, sem as linhas reais; por isso o patch cria `Content/canonical/commerce/coinshop.json` vazio, sem inventar itens/preços.\n- Os saldos `UserInfo`/`UserInfo_T` são estado mutável por jogador e devem continuar em DB. Apenas o catálogo estático deve ir para JSON.\n\n## Como associar um item a um vendedor\n\nHá dois modelos diferentes. NPCs normais são definidos por conteúdo/NPC e pela lógica de shop do NPC; CoinShop usa uma lista própria por `section`. Para um item novo aparecer, primeiro ele precisa existir no catálogo de itens (`server.items.json`/definições runtime) e depois ser referenciado pelo catálogo do vendedor. Para CoinShop, a associação proposta é uma entrada `{code, section, price, currency}` em `coinshop.json`.\n\n## Como descobrir itens da CoinShop que não carregam\n\nO código antigo resolve `szItemCode` contra `sItem[].LastCategory` e depois contra `DefaultItems`. Se a segunda busca falhar, `lpDefItem` pode ficar nulo antes do `memcpy`: este é um defeito real do caminho atual. A validação correta deve rejeitar no carregamento qualquer `code` que não exista no perfil de itens e registrar o código inválido, em vez de tentar enviá-lo ao cliente.\n\nAntes de remover `ItemList` da DB de produção, exporte as linhas reais para o JSON. O material recebido não contém essas linhas. Não aplique a migration destrutiva antes dessa exportação.\n\n## Drops\n\nOs 22 `.inf` recebidos são tabelas legadas de drop, não registros presentes nas migrations da DB. Use `tools/audit_drops.py --drops <Drops> --items Content/canonical/item_profiles` para listar referências a códigos que não existem nos perfis canónicos. Isso responde objetivamente se um drop aponta para item inexistente.\n\n## XP, dinheiro e valor por mapa\n\nNão balanceie multiplicadores isoladamente. Crie uma planilha/tabela de progressão com: faixa de nível do mapa, XP/hora alvo, gold/hora alvo, custo médio de consumíveis, custo de aging/mix/sheltons e tempo desejado para atingir o próximo marco. Depois derive multiplicadores por mapa a partir desses alvos. O catálogo de mapas já é canónico; o passo de refactor é acrescentar um bloco `economy` por mapa ou um catálogo `progression/economy.json`, evitando constantes espalhadas no C++.\n\n## Asas por quest\n\nO source ainda contém o sistema legado `WingItem` no inventário. A mudança deve fazer a aquisição depender do estado/recompensa da quest e retirar as asas do catálogo de créditos; não deve apagar a lógica de craft/uso das asas. Primeiro identifique os códigos exatos das asas no catálogo real da CoinShop (ausente no ZIP da DB).\n\n## Eventos por season\n\nRecomendação arquitetural: definições de evento e season em JSON canónico; DB apenas para estado operacional (ativação, progresso, cooldown, auditoria). Assim conteúdo versionável não fica preso à DB e o servidor continua podendo persistir estado. Estrutura sugerida: `events/seasons.json` + `events/<event>.json`, com `season_id`, janela, mapas, spawns, recompensas e feature flags.\n\n## Companions\n\nA decisão “buff em mim ou em outros companions” deve virar política explícita de targeting, não uma exceção por skill. Sugestão de enum: `owner`, `self_companion`, `party_companions`, `owner_and_companions`. O source já possui validadores específicos de companion targeting/group buffs, então a decisão pode ser coberta por testes.\n\n## Mineração, lapidação, sheltons e aging\n\nFluxo recomendado: `mineração -> minério bruto -> lapidação -> pedra/shelton -> aging/mix`. Reaproveite o inventário e os contratos de aging existentes; mineração define aquisição e lapidação define conversão/qualidade. A migration `044_gathering_mining_runtime.sql` mostra que mineração já tem persistência; evite criar uma segunda economia paralela de pedras.\n\n## Hardcoded text\n\n`tools/audit_hardcoded_text.py` gera candidatos fora dos diretórios de localização legada. A migração segura é: criar chave no JSON, compilar `localization.sloc`, trocar o literal pela consulta de localização e manter placeholders (`%s`, `%d`) idênticos. Não é seguro substituir automaticamente todo literal C/C++: paths, SQL, opcodes, formatos e identificadores também são strings e não são UI.\n''',encoding='utf8')
