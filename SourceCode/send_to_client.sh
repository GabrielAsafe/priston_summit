#DEST='server@127.0.0.1:C:/Users/server/Desktop/cli'

#scp -P 2223 \
#  dist_mingw_client_i686/Game.exe \
#  dist_mingw_client_i686/libgcc_s_dw2-1.dll \
#  dist_mingw_client_i686/libstdc++-6.dll \
#  dist_mingw_client_i686/libwinpthread-1.dll \
#  "$DEST"


cp dist_mingw_client_i686/Game.exe "/home/gabriel/Área de Trabalho/summit/cliente"
cp dist_mingw_client_i686/libgcc_s_dw2-1.dll "/home/gabriel/Área de Trabalho/summit/cliente"
cp dist_mingw_client_i686/libstdc++-6.dll "/home/gabriel/Área de Trabalho/summit/cliente"
cp dist_mingw_client_i686/libwinpthread-1.dll "/home/gabriel/Área de Trabalho/summit/cliente"
