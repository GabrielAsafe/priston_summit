#scp -P 2223 cli.zip server@127.0.0.1:"C:/Users/server/Desktop/cli"


#!/bin/bash
cp /usr/i686-w64-mingw32/sys-root/mingw/bin/libgcc_s_dw2-1.dll \
   dist_mingw_server_i686/

cp /usr/i686-w64-mingw32/sys-root/mingw/bin/libstdc++-6.dll \
   dist_mingw_server_i686/

cp /usr/i686-w64-mingw32/sys-root/mingw/bin/libwinpthread-1.dll \
   dist_mingw_server_i686/


#DEST='server@127.0.0.1:C:/SummitTale/Servidor'
DEST='server@127.0.0.1:C:/Servidor'
scp -P 2223 \
  dist_mingw_server_i686/Server.exe \
  dist_mingw_server_i686/libgcc_s_dw2-1.dll \
  dist_mingw_server_i686/libstdc++-6.dll \
  dist_mingw_server_i686/libwinpthread-1.dll \
  "$DEST"

# BUGFIX_001_HOTUK_DEPLOY
HOTUK="$(dirname "$0")/../Servidor/Configs/hotuk.ini"
HOTUK_DEST='server@127.0.0.1:C:/SummitTale/Servidor/Configs/hotuk.ini'

scp -P 2223 \
  "$HOTUK" \
  "$HOTUK_DEST"

