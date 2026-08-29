# Summit Tale — Arquitetura

```text
Summit Tale
│
├── Cliente
│   ├── Source Summit Tale/SrcGame
│   ├── Assets
│   ├── Launcher
│   └── Linguagens
│
├── Servidor Windows
│   ├── Server.exe
│   ├── GameServer/
│   ├── DataServer/
│   ├── Configs/
│   ├── Proteção / anti-cheat
│   ├── Ferramentas administrativas
│   └── sql.reg
│
├── Web / Painéis
│   ├── wwwroot - BPT
│   ├── wwwroot - EPIC SIMPLE
│   ├── padm/
│   └── painel/
│
├── Docker
│   ├── SQL Server 2022
│   ├── CloudBeaver
│   ├── Apache/PHP BPT
│   └── Apache/PHP EPIC SIMPLE
│
└── Bases de Dados
    ├── AccountDB
    ├── BillingDb
    ├── ClanDB
    ├── GameLogDB
    ├── PristonDB
    ├── ServerDB
    ├── SodDb
    └── outras bases auxiliares
```

## Soluções Visual Studio

```text
cliente/Source Summit Tale/SrcServer/Server.sln
cliente/Source Summit Tale/SrcGame/Game.sln
cliente/Source Summit Tale/SrcGame/src/Library/LuaSrc/Lua.sln
```

## Projetos Visual Studio

```text
cliente/Source Summit Tale/SrcServer/src/priston.vcxproj
cliente/Source Summit Tale/SrcGame/src/priston.vcxproj
cliente/Source Summit Tale/SrcGame/src/Library/LuaSrc/Lua/Lua.vcxproj
```
