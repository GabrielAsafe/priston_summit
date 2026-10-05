#!/usr/bin/env python3
from pathlib import Path
import sys
r=Path(__file__).resolve().parents[2]
checks={
'SrcGame/src/systems/configuration/client/Settings.h':['getLocale()','setLocale(const std::string& value)'],
'SrcGame/src/systems/configuration/client/Settings.cpp':['"Language", "Locale"','SetLocalizationLocale(getLocale().c_str())','file.write("Language", "Locale", getLocale())'],
'SrcGame/src/systems/ui/v2/windows/SettingsWindow.cpp':['"Idioma"','"Português (Brasil)"','state_.locale == "pt-BR" ? "en" : "pt-BR"','SetLocalizationLocale(SETTINGSHANDLER->getLocale().c_str())'],
}
errors=[]
for rel,tokens in checks.items():
 t=(r/rel).read_text(encoding='utf-8',errors='replace')
 for x in tokens:
  if x not in t:errors.append(f'{rel}: missing {x}')
if errors:print('\n'.join(errors),file=sys.stderr);sys.exit(1)
print('PATCH 182 gate: OK')
