#!/usr/bin/env python3
from pathlib import Path
import re

ROOT = Path(__file__).resolve().parents[1]
SETTINGS = ROOT / "SrcGame/src/systems/configuration/client/Settings.cpp"
NETPLAY = ROOT / "SrcGame/src/systems/network/client/netplay.cpp"

s = SETTINGS.read_text(encoding="latin1")
n = NETPLAY.read_text(encoding="latin1")

fail = []

required_settings = [
    "R27_CLIENT_NETWORK_FROM_GAME_INI",
    "GetPrivateProfileStringA(",
    '"Network", "Server"',
    'GetPrivateProfileIntA("Network", "Port", 1997',
    "setServer1(networkServer1)",
    "setServer2(networkServer2)",
    "setServer3(networkServer3)",
    "setPort(networkPort)",
]
for token in required_settings:
    if token not in s:
        fail.append(f"Settings missing: {token}")

required_netplay = [
    'szDefaultServIP[] = "127.0.0.1"',
    "R27_CLIENT_NETWORK_CONFIG_AUTHORITY_2",
    "R27_CLIENT_NETWORK_CONFIG_AUTHORITY_3",
]
for token in required_netplay:
    if token not in n:
        fail.append(f"netplay missing: {token}")

for forbidden in ("192.168.1.111", "211.50.44.170"):
    if forbidden in s:
        fail.append(f"hardcoded endpoint in Settings.cpp: {forbidden}")
    if forbidden in n:
        fail.append(f"hardcoded endpoint in client netplay.cpp: {forbidden}")

for ip in re.findall(r'setServer[123]\("([0-9]+\.[0-9]+\.[0-9]+\.[0-9]+)"\)', s):
    fail.append(f"direct setServer hardcode remains: {ip}")

if fail:
    for x in fail:
        print("FAIL:", x)
    print("RESULT=FAIL")
    raise SystemExit(1)

print("client_network_source=game.ini")
print("single_server_key=Network.Server")
print("fallback=127.0.0.1:1997")
print("hardcoded_personal_ip=NO")
print("server_list_override=game.ini")
print("RESULT=PASS")
