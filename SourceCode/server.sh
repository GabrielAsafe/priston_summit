#!bin/bash

bash build_server_mingw_i686_j8.sh
#link_client_mingw_i686.sh
sh link_server_mingw_i686.sh

sh send_to_s.sh


cat initandtestServerfile.txt
