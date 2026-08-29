# Summit Semantic Encoding Audit

Auditoria somente-leitura de literals C/C++ potencialmente corrompidos.

## Escopo

- Target `game`: `/home/gabriel/Área de Trabalho/summit/SourceCode/SrcGame/src`
- Target `server`: `/home/gabriel/Área de Trabalho/summit/SourceCode/SrcServer/src`
- Target `shared`: `/home/gabriel/Área de Trabalho/summit/SourceCode/Shared`

Fontes em ordem de prioridade:

1. `pstale`: `/home/gabriel/armazem/priston/pristontale-1-master/src`
2. `fpt_rpt`: `/home/gabriel/armazem/priston/drive-download-20260818T103311Z-1-002/fPT-rPT-Project-main`
3. `priston_main`: `/home/gabriel/armazem/priston/drive-download-20260818T103311Z-1-002/priston-main`
4. `exmachina`: `/home/gabriel/armazem/priston/ExMachina`

## Resumo

- Linhas suspeitas: 17325
- Linhas em lógica (fora de Language): 4658
- Linhas em Language: 12667
- Candidatos HIGH: 5875
- Candidatos MEDIUM: 0
- Candidatos LOW: 65957
- Linhas ainda sem recuperação HIGH/MEDIUM: 12054

## Cobertura por fonte

- `pstale`: 5875 candidatos HIGH/MEDIUM
- `fpt_rpt`: 0 candidatos HIGH/MEDIUM
- `priston_main`: 0 candidatos HIGH/MEDIUM
- `exmachina`: 0 candidatos HIGH/MEDIUM

## Arquivos prioritários de lógica

| Arquivo | Linhas suspeitas | U+FFFD |
|---|---:|---:|
| `server:sinbaram/sinMsg.cpp` | 633 | 7469 |
| `game:sinbaram/sinMsg.cpp` | 632 | 7445 |
| `game:fileread.cpp` | 326 | 2008 |
| `game:TJBOY/isaocheck/isaocheck.cpp` | 208 | 1975 |
| `server:TJBOY/isaocheck/isaocheck.cpp` | 208 | 1975 |
| `game:Server/OnSever.cpp` | 177 | 1685 |
| `game:sinbaram/sinSkill.cpp` | 244 | 1510 |
| `server:fileread.cpp` | 223 | 1440 |
| `game:TJBOY/Ygy/Util.cpp` | 43 | 1296 |
| `server:TJBOY/Ygy/Util.cpp` | 43 | 1296 |
| `server:sinbaram/sinSkill.cpp` | 171 | 1222 |
| `game:sinbaram/sinCraftItem_Server.h` | 127 | 1066 |
| `server:sinbaram/sinCraftItem_Server.h` | 127 | 1066 |
| `game:TJBOY/clanmenu/cE_chip.cpp` | 62 | 989 |
| `server:TJBOY/clanmenu/cE_chip.cpp` | 62 | 989 |
| `game:sinbaram/sinCraftItem_Client.h` | 115 | 907 |
| `server:sinbaram/sinCraftItem_Client.h` | 115 | 907 |
| `server:Server/OnSever.cpp` | 136 | 808 |
| `game:TJBOY/isaocheck/auth.cpp` | 59 | 666 |
| `server:TJBOY/isaocheck/auth.cpp` | 59 | 666 |
| `game:TJBOY/clanmenu/clan_Enti.cpp` | 36 | 595 |
| `server:TJBOY/clanmenu/clan_Enti.cpp` | 36 | 595 |
| `game:TJBOY/clanmenu/cE_user.cpp` | 30 | 526 |
| `server:TJBOY/clanmenu/cE_user.cpp` | 30 | 526 |
| `game:TJBOY/clanmenu/cE_CJoin.cpp` | 18 | 319 |
| `server:TJBOY/clanmenu/cE_CJoin.cpp` | 18 | 319 |
| `game:sinbaram/sinItem.cpp` | 155 | 227 |
| `server:sinbaram/sinInterFace.cpp` | 23 | 197 |
| `game:TJBOY/park/MicVolume.cpp` | 7 | 169 |
| `server:TJBOY/park/MicVolume.cpp` | 7 | 169 |
| `game:TJBOY/clanmenu/cE_isclanmember.cpp` | 10 | 133 |
| `server:TJBOY/clanmenu/cE_isclanmember.cpp` | 10 | 133 |
| `server:record.cpp` | 18 | 101 |
| `server:TJBOY/clanmenu/tjclan.cpp` | 13 | 91 |
| `game:TJBOY/clanmenu/tjclan.cpp` | 11 | 83 |
| `game:sinbaram/sinEffect2.cpp` | 13 | 78 |
| `server:sinbaram/sinEffect2.cpp` | 13 | 78 |
| `game:netplay.cpp` | 14 | 74 |
| `server:netplay.cpp` | 10 | 67 |
| `game:TJBOY/clanmenu/cE_CViewClanInfo.cpp` | 5 | 66 |
| `server:TJBOY/clanmenu/cE_CViewClanInfo.cpp` | 5 | 66 |
| `game:sinbaram/sinSubMain.cpp` | 11 | 61 |
| `server:sinbaram/sinSubMain.cpp` | 11 | 61 |
| `game:TJBOY/Ygy/Packet/PacketStruct.h` | 4 | 54 |
| `server:TJBOY/Ygy/Packet/PacketStruct.h` | 4 | 54 |
| `shared:b_sinCraftItem.h` | 24 | 48 |
| `game:character.cpp` | 6 | 41 |
| `server:character.cpp` | 6 | 41 |
| `server:Server/Svr_Damge.cpp` | 21 | 37 |
| `game:TJBOY/Ygy/Process/Process.cpp` | 2 | 34 |
| `server:TJBOY/Ygy/Process/Process.cpp` | 2 | 34 |
| `game:HoBaram/HoOpening.cpp` | 7 | 32 |
| `game:sinbaram/sinHelp.cpp` | 5 | 32 |
| `server:sinbaram/sinHelp.cpp` | 5 | 32 |
| `server:BattlePass.cpp` | 24 | 28 |
| `server:JOBS.cpp` | 17 | 28 |
| `game:sinbaram/sinTrade.cpp` | 2 | 28 |
| `game:HoBaram/HoSkyOld.cpp` | 3 | 27 |
| `server:HoBaram/HoSky.cpp` | 3 | 27 |
| `game:AreaServer.cpp` | 2 | 27 |
| `server:AreaServer.cpp` | 2 | 27 |
| `game:cQuestWindow.cpp` | 20 | 25 |
| `game:sinbaram/sinInvenTory.cpp` | 14 | 25 |
| `game:sinbaram/AssaUtil.cpp` | 4 | 25 |
| `server:sinbaram/AssaUtil.cpp` | 4 | 25 |
| `game:Server/E_serverlog.h` | 2 | 22 |
| `server:Server/E_serverlog.h` | 2 | 22 |
| `game:cSettingsWindow.cpp` | 16 | 21 |
| `game:TJBOY/clanmenu/cE_Notice.cpp` | 5 | 20 |
| `server:TJBOY/clanmenu/cE_Notice.cpp` | 5 | 20 |
| `game:TJBOY/Ygy/memmap/memmap.cpp` | 4 | 20 |
| `server:TJBOY/Ygy/memmap/memmap.cpp` | 4 | 20 |
| `game:TCore.cpp` | 15 | 19 |
| `game:sinbaram/sinCharStatus.cpp` | 4 | 19 |
| `server:sinbaram/sinCharStatus.cpp` | 4 | 19 |
| `game:cracker.cpp` | 2 | 19 |
| `server:cracker.cpp` | 2 | 19 |
| `game:TJBOY/clanmenu/cE_chatlist.cpp` | 4 | 16 |
| `server:TJBOY/clanmenu/cE_chatlist.cpp` | 4 | 16 |
| `server:sinbaram/sinInvenTory.cpp` | 9 | 15 |
| `game:sinbaram/sinMessageBox.cpp` | 6 | 14 |
| `game:TJBOY/clanmenu/cE_Cmake.cpp` | 2 | 14 |
| `server:TJBOY/clanmenu/cE_Cmake.cpp` | 2 | 14 |
| `server:sinbaram/sinTrade.cpp` | 1 | 14 |
| `game:Winmain.bak2.cpp` | 5 | 13 |
| `game:Winmain.cpp` | 5 | 13 |
| `game:smLib3d/smTexture.cpp` | 3 | 12 |
| `server:smLib3d/smTexture.cpp` | 3 | 12 |
| `game:HoBaram/HoNewParticle.cpp` | 1 | 12 |
| `game:TJBOY/Ygy/Packet/Packet.cpp` | 1 | 12 |
| `server:HoBaram/HoNewParticle.cpp` | 1 | 12 |
| `server:TJBOY/Ygy/Packet/Packet.cpp` | 1 | 12 |
| `server:cHellsGate.cpp` | 9 | 11 |
| `game:cRestaureWindow.cpp` | 3 | 9 |
| `game:playmain.cpp` | 6 | 8 |
| `game:TJBOY/clanmenu/Help.h` | 1 | 8 |
| `server:TJBOY/clanmenu/Help.h` | 1 | 8 |
| `server:sinbaram/sinMessageBox.cpp` | 1 | 8 |
| `game:Ranking.cpp` | 7 | 7 |
| `game:CoinShop.cpp` | 2 | 7 |

## Candidatos de alta confiança

| Target | Linha | Fonte | Linha fonte | Preview fonte |
|---|---:|---|---:|---|
| `game:cracker.cpp` | 26 | `pstale` | 31 | `생각하는` |
| `game:cracker.cpp` | 27 | `pstale` | 32 | `天空之城` |
| `game:cracker.cpp` | 27 | `pstale` | 32 | `스피드핵` |
| `game:Winmain.cpp` | 1039 | `pstale` | 1009 | `굃굍 굊긕긘긞긏` |
| `game:fileread.cpp` | 136 | `pstale` | 133 | `반복` |
| `game:fileread.cpp` | 137 | `pstale` | 134 | `동작` |
| `game:fileread.cpp` | 139 | `pstale` | 136 | `뛰는` |
| `game:fileread.cpp` | 140 | `pstale` | 137 | `*서있기` |
| `game:fileread.cpp` | 141 | `pstale` | 138 | `*정지` |
| `game:fileread.cpp` | 142 | `pstale` | 139 | `*걷는` |
| `game:fileread.cpp` | 144 | `pstale` | 141 | `공격` |
| `game:fileread.cpp` | 145 | `pstale` | 142 | `죽기` |
| `game:fileread.cpp` | 146 | `pstale` | 143 | `죽는` |
| `game:fileread.cpp` | 147 | `pstale` | 144 | `타격` |
| `game:fileread.cpp` | 148 | `pstale` | 145 | `데미지` |
| `game:fileread.cpp` | 149 | `pstale` | 146 | `막기` |
| `game:fileread.cpp` | 151 | `pstale` | 148 | `울기` |
| `game:fileread.cpp` | 152 | `pstale` | 149 | `워프` |
| `game:fileread.cpp` | 154 | `pstale` | 151 | `가끔` |
| `game:fileread.cpp` | 160 | `pstale` | 157 | `다시시작` |
| `game:fileread.cpp` | 161 | `pstale` | 158 | `먹기` |
| `game:fileread.cpp` | 162 | `pstale` | 159 | `기술` |
| `game:fileread.cpp` | 163 | `pstale` | 160 | `만세` |
| `game:fileread.cpp` | 167 | `pstale` | 164 | `표정` |
| `game:fileread.cpp` | 169 | `pstale` | 166 | `아표정` |
| `game:fileread.cpp` | 170 | `pstale` | 167 | `이표정` |
| `game:fileread.cpp` | 171 | `pstale` | 168 | `오표정` |
| `game:fileread.cpp` | 172 | `pstale` | 169 | `눈깜빡표정` |
| `game:fileread.cpp` | 174 | `pstale` | 171 | `무표정(작동않함)` |
| `game:fileread.cpp` | 175 | `pstale` | 172 | `웃는표정` |
| `game:fileread.cpp` | 176 | `pstale` | 173 | `화난표정` |
| `game:fileread.cpp` | 177 | `pstale` | 174 | `슬픈표정` |
| `game:fileread.cpp` | 178 | `pstale` | 175 | `놀란표정` |
| `game:fileread.cpp` | 179 | `pstale` | 176 | `고유표정` |
| `game:fileread.cpp` | 180 | `pstale` | 177 | `특별표정` |
| `game:fileread.cpp` | 1226 | `pstale` | 1230 | `좋음` |
| `game:fileread.cpp` | 1226 | `pstale` | 1230 | `보통` |
| `game:fileread.cpp` | 1226 | `pstale` | 1230 | `나쁨` |
| `game:fileread.cpp` | 1226 | `pstale` | 1230 | `아주나쁨` |
| `game:fileread.cpp` | 3626 | `pstale` | 4310 | `소형` |
| `game:fileread.cpp` | 3626 | `pstale` | 4310 | `중형` |
| `game:fileread.cpp` | 3626 | `pstale` | 4310 | `중대형` |
| `game:fileread.cpp` | 3626 | `pstale` | 4310 | `대형` |
| `game:Server/E_serverlog.h` | 19 | `pstale` | 19 | `%s\\(믹스쳐리셋)%d월%d일.log` |
| `game:Server/E_serverlog.h` | 178 | `pstale` | 179 | `%d시%d분%d초 - ID:( %s ) / Name:( %s ) / IP ( %s ) - 아이템( %s ) ( %d %d <%d> )\r\n` |
| `game:Server/OnSever.cpp` | 142 | `pstale` | 195 | `좀비` |
| `game:Server/OnSever.cpp` | 418 | `pstale` | 468 | ` 오미크론 ` |
| `game:Server/OnSever.cpp` | 419 | `pstale` | 469 | ` 스파이더 트루퍼 ` |
| `game:Server/OnSever.cpp` | 421 | `pstale` | 471 | ` 오메가 ` |
| `game:Server/OnSever.cpp` | 4090 | `pstale` | 4205 | `바곤` |
| `game:Server/OnSever.cpp` | 4090 | `pstale` | 4205 | `스켈레톤아처` |
| `game:Server/OnSever.cpp` | 4090 | `pstale` | 4205 | `크립트` |
| `game:Server/OnSever.cpp` | 4091 | `pstale` | 4206 | `싸이클롭스 나이트` |
| `game:Server/OnSever.cpp` | 4091 | `pstale` | 4206 | `스켈레톤아처` |
| `game:Server/OnSever.cpp` | 4091 | `pstale` | 4206 | `스켈레톤워리어` |
| `game:Server/OnSever.cpp` | 4091 | `pstale` | 4206 | `언홀리 나이트` |
| `game:Server/OnSever.cpp` | 4092 | `pstale` | 4207 | `헤드커터` |
| `game:Server/OnSever.cpp` | 4092 | `pstale` | 4207 | `스켈레톤레인저` |
| `game:Server/OnSever.cpp` | 4092 | `pstale` | 4207 | `이블 메이플` |
| `game:Server/OnSever.cpp` | 4092 | `pstale` | 4207 | `웹` |
| `game:Server/OnSever.cpp` | 4093 | `pstale` | 4208 | `일루젼나이트` |
| `game:Server/OnSever.cpp` | 4093 | `pstale` | 4208 | `파이곤` |
| `game:Server/OnSever.cpp` | 4093 | `pstale` | 4208 | `둠 가드` |
| `game:Server/OnSever.cpp` | 4093 | `pstale` | 4208 | `그리븐` |
| `game:Server/OnSever.cpp` | 4094 | `pstale` | 4209 | `헐크` |
| `game:Server/OnSever.cpp` | 4094 | `pstale` | 4209 | `파이곤` |
| `game:Server/OnSever.cpp` | 4094 | `pstale` | 4209 | `슬러터` |
| `game:Server/OnSever.cpp` | 4094 | `pstale` | 4209 | `쉐도우` |
| `game:Server/OnSever.cpp` | 4094 | `pstale` | 4209 | `베르문 악타룬` |
| `game:Server/OnSever.cpp` | 4096 | `pstale` | 4211 | `아이언 가드` |
| `game:Server/OnSever.cpp` | 4096 | `pstale` | 4211 | `킹호피` |
| `game:Server/OnSever.cpp` | 4096 | `pstale` | 4211 | `인큐버스` |
| `game:Server/OnSever.cpp` | 4096 | `pstale` | 4211 | `하이퍼 머신` |
| `game:Server/OnSever.cpp` | 4096 | `pstale` | 4211 | `둠 로드` |
| `game:Server/OnSever.cpp` | 4102 | `pstale` | 4217 | `바곤` |
| `game:Server/OnSever.cpp` | 4102 | `pstale` | 4217 | `메핏` |
| `game:Server/OnSever.cpp` | 4102 | `pstale` | 4217 | `부마` |
| `game:Server/OnSever.cpp` | 4103 | `pstale` | 4218 | `싸이클롭스 나이트` |
| `game:Server/OnSever.cpp` | 4103 | `pstale` | 4218 | `스켈레톤아처` |
| `game:Server/OnSever.cpp` | 4103 | `pstale` | 4218 | `스켈레톤워리어` |
| `game:Server/OnSever.cpp` | 4103 | `pstale` | 4218 | `언홀리 나이트` |
| `game:Server/OnSever.cpp` | 4104 | `pstale` | 4219 | `스켈레톤나이트` |
| `game:Server/OnSever.cpp` | 4104 | `pstale` | 4219 | `스켈레톤레인저` |
| `game:Server/OnSever.cpp` | 4104 | `pstale` | 4219 | `마이어 키퍼` |
| `game:Server/OnSever.cpp` | 4104 | `pstale` | 4219 | `웹` |
| `game:Server/OnSever.cpp` | 4105 | `pstale` | 4220 | `슬러터` |
| `game:Server/OnSever.cpp` | 4105 | `pstale` | 4220 | `파이곤` |
| `game:Server/OnSever.cpp` | 4105 | `pstale` | 4220 | `다크 나이트` |
| `game:Server/OnSever.cpp` | 4105 | `pstale` | 4220 | `그리븐` |
| `game:Server/OnSever.cpp` | 4106 | `pstale` | 4221 | `헐크` |
| `game:Server/OnSever.cpp` | 4106 | `pstale` | 4221 | `슬리버` |
| `game:Server/OnSever.cpp` | 4106 | `pstale` | 4221 | `아벨리스크-L` |
| `game:Server/OnSever.cpp` | 4106 | `pstale` | 4221 | `쉐도우` |
| `game:Server/OnSever.cpp` | 4106 | `pstale` | 4221 | `베르문 악타룬` |
| `game:Server/OnSever.cpp` | 4108 | `pstale` | 4223 | `아이언 가드` |
| `game:Server/OnSever.cpp` | 4108 | `pstale` | 4223 | `킹호피` |
| `game:Server/OnSever.cpp` | 4108 | `pstale` | 4223 | `아벨리스크 로드` |
| `game:Server/OnSever.cpp` | 4108 | `pstale` | 4223 | `하이퍼 머신` |
| `game:Server/OnSever.cpp` | 4108 | `pstale` | 4223 | `둠 로드` |
| `game:Server/OnSever.cpp` | 4114 | `pstale` | 4229 | `메핏` |
| `game:Server/OnSever.cpp` | 4114 | `pstale` | 4229 | `좀비` |
| `game:Server/OnSever.cpp` | 4114 | `pstale` | 4229 | `홉고블린 칩` |
| `game:Server/OnSever.cpp` | 4115 | `pstale` | 4230 | `구울` |
| `game:Server/OnSever.cpp` | 4115 | `pstale` | 4230 | `스켈레톤아처` |
| `game:Server/OnSever.cpp` | 4115 | `pstale` | 4230 | `싸이클롭스` |
| `game:Server/OnSever.cpp` | 4116 | `pstale` | 4231 | `마이티고블린` |
| `game:Server/OnSever.cpp` | 4116 | `pstale` | 4231 | `스켈레톤아처` |
| `game:Server/OnSever.cpp` | 4116 | `pstale` | 4231 | `아머드 비틀` |
| `game:Server/OnSever.cpp` | 4116 | `pstale` | 4231 | `언홀리 나이트` |
| `game:Server/OnSever.cpp` | 4117 | `pstale` | 4232 | `타이탄` |
| `game:Server/OnSever.cpp` | 4117 | `pstale` | 4232 | `스켈레톤레인저` |
| `game:Server/OnSever.cpp` | 4117 | `pstale` | 4232 | `머디 고렘` |
| `game:Server/OnSever.cpp` | 4117 | `pstale` | 4232 | `웹` |
| `game:Server/OnSever.cpp` | 4118 | `pstale` | 4233 | `나이트메어` |
| `game:Server/OnSever.cpp` | 4118 | `pstale` | 4233 | `파이곤` |
| `game:Server/OnSever.cpp` | 4118 | `pstale` | 4233 | `비블` |
| `game:Server/OnSever.cpp` | 4118 | `pstale` | 4233 | `그리븐` |
| `game:Server/OnSever.cpp` | 4119 | `pstale` | 4234 | `헐크` |
| `game:Server/OnSever.cpp` | 4119 | `pstale` | 4234 | `슬리버` |
| `game:Server/OnSever.cpp` | 4119 | `pstale` | 4234 | `아벨리스크-L` |
| `game:Server/OnSever.cpp` | 4119 | `pstale` | 4234 | `쉐도우` |
| `game:Server/OnSever.cpp` | 4119 | `pstale` | 4234 | `베르문 악타룬` |
| `game:Server/OnSever.cpp` | 4120 | `pstale` | 4235 | `오미크론` |
| `game:Server/OnSever.cpp` | 4120 | `pstale` | 4235 | `스티지언` |
| `game:Server/OnSever.cpp` | 4120 | `pstale` | 4235 | `헐크` |
| `game:Server/OnSever.cpp` | 4120 | `pstale` | 4235 | `둠 가드` |
| `game:Server/OnSever.cpp` | 4121 | `pstale` | 4236 | `아이언 가드` |
| `game:Server/OnSever.cpp` | 4121 | `pstale` | 4236 | `체인 고렘` |
| `game:Server/OnSever.cpp` | 4121 | `pstale` | 4236 | `위치` |
| `game:Server/OnSever.cpp` | 4121 | `pstale` | 4236 | `하이퍼 머신` |
| `game:Server/OnSever.cpp` | 4121 | `pstale` | 4236 | `둠 로드` |
| `game:Server/OnSever.cpp` | 24958 | `pstale` | 33252 | `> 개인상점 메세지 ( %d )` |
| `game:Server/OnSever.cpp` | 25110 | `pstale` | 33398 | `바벨 생명 재생 ( 주변:%d명 ) ( 재생력:%d )` |
| `game:Server/OnSever.cpp` | 27008 | `pstale` | 35255 | `필연적 아이템 과의 만남( %d )` |
| `game:sinbaram/sinMsg.cpp` | 122 | `pstale` | 127 | `몇명의 용병을` |
| `game:sinbaram/sinMsg.cpp` | 122 | `pstale` | 127 | `구입하시겠습니까?` |
| `game:sinbaram/sinMsg.cpp` | 222 | `pstale` | 227 | `       스킬 포인트 : %d` |
| `game:sinbaram/sinMsg.cpp` | 223 | `pstale` | 228 | `엘리트 스킬 포인트 : %d` |
| `game:sinbaram/sinMsg.cpp` | 238 | `pstale` | 243 | `특화블럭율: \r` |
| `game:sinbaram/sinMsg.cpp` | 238 | `pstale` | 243 | `마법숙련도: \r` |
| `game:sinbaram/sinMsg.cpp` | 238 | `pstale` | 243 | `특화이동속도: \r` |
| `game:sinbaram/sinMsg.cpp` | 238 | `pstale` | 243 | `특화생체속성: \r` |
| `game:sinbaram/sinMsg.cpp` | 238 | `pstale` | 243 | `특화자연속성: \r` |
| `game:sinbaram/sinMsg.cpp` | 245 | `pstale` | 250 | `%s Ưȭ\r` |
| `game:sinbaram/sinMsg.cpp` | 246 | `pstale` | 251 | `아이템 믹스쳐 결과` |
| `game:sinbaram/sinMsg.cpp` | 247 | `pstale` | 252 | `현재 레벨\r` |
| `game:sinbaram/sinMsg.cpp` | 248 | `pstale` | 253 | `다음 레벨\r` |
| `game:sinbaram/sinMsg.cpp` | 251 | `pstale` | 256 | `%s (요구레벨:%d)\r` |
| `game:sinbaram/sinMsg.cpp` | 252 | `pstale` | 257 | `사용가능 아이템군\r` |
| `game:sinbaram/sinMsg.cpp` | 254 | `pstale` | 259 | `독 속성:\r` |
| `game:sinbaram/sinMsg.cpp` | 255 | `pstale` | 260 | `불 속성:\r` |
| `game:sinbaram/sinMsg.cpp` | 256 | `pstale` | 261 | `공격범위:\r` |
| `game:sinbaram/sinMsg.cpp` | 257 | `pstale` | 262 | `명중률:\r` |
| `game:sinbaram/sinMsg.cpp` | 258 | `pstale` | 263 | `최대 데미지증가:\r` |
| `game:sinbaram/sinMsg.cpp` | 259 | `pstale` | 264 | `공격력가중:\r` |
| `game:sinbaram/sinMsg.cpp` | 260 | `pstale` | 265 | `공격스피드증가:\r` |
| `game:sinbaram/sinMsg.cpp` | 261 | `pstale` | 266 | `블럭율증가:\r` |
| `game:sinbaram/sinMsg.cpp` | 262 | `pstale` | 267 | `유지시간:\r` |
| `game:sinbaram/sinMsg.cpp` | 263 | `pstale` | 268 | `사용기력:\r` |
| `game:sinbaram/sinMsg.cpp` | 264 | `pstale` | 269 | `공격력:\r` |
| `game:sinbaram/sinMsg.cpp` | 265 | `pstale` | 270 | `사정거리:\r` |
| `game:sinbaram/sinMsg.cpp` | 266 | `pstale` | 271 | `흡수량증가:\r` |
| `game:sinbaram/sinMsg.cpp` | 267 | `pstale` | 272 | `초` |
| `game:sinbaram/sinMsg.cpp` | 268 | `pstale` | 273 | `공격력추가치:\r` |
| `game:sinbaram/sinMsg.cpp` | 269 | `pstale` | 274 | `연타횟수:\r` |
| `game:sinbaram/sinMsg.cpp` | 270 | `pstale` | 275 | `생명력감소치:\r` |
| `game:sinbaram/sinMsg.cpp` | 271 | `pstale` | 276 | `명중력추가치:\r` |
| `game:sinbaram/sinMsg.cpp` | 272 | `pstale` | 277 | `크리티컬추가:\r` |
| `game:sinbaram/sinMsg.cpp` | 273 | `pstale` | 278 | `밀어내는거리:\r` |
| `game:sinbaram/sinMsg.cpp` | 275 | `pstale` | 280 | `빙 속성:\r` |
| `game:sinbaram/sinMsg.cpp` | 276 | `pstale` | 281 | `빙계 공격력 :\r` |
| `game:sinbaram/sinMsg.cpp` | 277 | `pstale` | 282 | `스턴확률:\r` |
| `game:sinbaram/sinMsg.cpp` | 278 | `pstale` | 283 | `블럭율 증가:\r` |
| `game:sinbaram/sinMsg.cpp` | 280 | `pstale` | 285 | `매의회전수:\r` |
| `game:sinbaram/sinMsg.cpp` | 281 | `pstale` | 286 | `무기스피드가중:\r` |
| `game:sinbaram/sinMsg.cpp` | 282 | `pstale` | 287 | `명중력증가:\r` |
| `game:sinbaram/sinMsg.cpp` | 283 | `pstale` | 288 | `발사 갯수:\r` |
| `game:sinbaram/sinMsg.cpp` | 284 | `pstale` | 289 | `연속 발사화살수:\r` |
| `game:sinbaram/sinMsg.cpp` | 285 | `pstale` | 290 | `사용근력:\r` |
| `game:sinbaram/sinMsg.cpp` | 286 | `pstale` | 291 | `스킬수련비용:\r` |
| `game:sinbaram/sinMsg.cpp` | 289 | `pstale` | 294 | `증정용 아이템\r` |
| `game:sinbaram/sinMsg.cpp` | 290 | `pstale` | 295 | `복사된 아이템\r` |
| `game:sinbaram/sinMsg.cpp` | 291 | `pstale` | 296 | `ȸ` |
| `game:sinbaram/sinMsg.cpp` | 292 | `pstale` | 297 | `스킬포인트` |
| `game:sinbaram/sinMsg.cpp` | 293 | `pstale` | 298 | `동결 시간:\r` |
| `game:sinbaram/sinMsg.cpp` | 294 | `pstale` | 299 | `유통기한이 지났습니다\r` |
| `game:sinbaram/sinMsg.cpp` | 295 | `pstale` | 300 | `이런! 상한 음식이잖아!` |
| `game:sinbaram/sinMsg.cpp` | 296 | `pstale` | 301 | `아이템 제련 결과` |
| `game:sinbaram/sinMsg.cpp` | 297 | `pstale` | 302 | `아이템 제작 결과` |
| `game:sinbaram/sinMsg.cpp` | 298 | `pstale` | 303 | `랜덤으로` |
| `game:sinbaram/sinMsg.cpp` | 301 | `pstale` | 306 | `최대 공격력 가중+\r` |
| `game:sinbaram/sinMsg.cpp` | 302 | `pstale` | 307 | `(관통 공격)\r` |
| `game:sinbaram/sinMsg.cpp` | 303 | `pstale` | 308 | `근력 추가량:\r` |
| `game:sinbaram/sinMsg.cpp` | 304 | `pstale` | 309 | `생명력 회복치:\r` |
| `game:sinbaram/sinMsg.cpp` | 306 | `pstale` | 311 | `Undead 계열에겐 50% 공격력 가중:\r` |
| `game:sinbaram/sinMsg.cpp` | 307 | `pstale` | 312 | `개수:\r` |
| `game:sinbaram/sinMsg.cpp` | 308 | `pstale` | 313 | `개\r` |
| `game:sinbaram/sinMsg.cpp` | 309 | `pstale` | 314 | `공격력 감소치:\r` |
| `game:sinbaram/sinMsg.cpp` | 310 | `pstale` | 315 | `변환량:\r` |
| `game:sinbaram/sinMsg.cpp` | 311 | `pstale` | 316 | `원소속성 상승치:\r` |

## Não resolvidos

Consulte `unresolved.tsv`. Esses casos não devem ser corrigidos por inferência automática; requerem outra fonte, matching específico ou análise semântica manual.

## Política recomendada

- Fonte prioritária é consultada primeiro.
- Fontes seguintes só são usadas quando a prioritária não produz candidato HIGH/MEDIUM para a linha.
- Nenhuma string é escrita de volta automaticamente.
- Strings de `Language/` são separadas das strings presentes na lógica.
- Recuperar texto não implica manter o idioma na lógica: a auditoria serve para descobrir a identidade/semântica original antes de migrar identificadores técnicos para ASCII/inglês.
