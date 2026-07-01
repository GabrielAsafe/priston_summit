<?
define("PT","1");

/*----------------------------------------------------------------------
Painel de Adminstra��o
PESSOAS QUE CONTRIBUIRAM NO DESENVOLVIMENTO DO PAINEL DE ADMINSTRA��O.
Niik - Jeck - MecanicoGM - N�ckolas - D kin
----------------------------------------------------------------------*/

$banco = "CoinShopDB";

// TITULO DAS PAGINAS DO PAINEL
$version="Painel de Administra��o";

//Nome do Executavel do servidor.exe (coloque s� o nome, sem a exten��o .exe)
$Server = "Server";

//Porta do servidor
$portasv = "1997";

// IP DA MAQUINA QUE O PAINEL EST� ATUALMENTE
$iphost = "124.198.128.16";

// DIRETORIO DA SERVER FILES NO HOST
$rootDir = "C:/Servidor/"; 
$rootDir2 = "C:/Servidor/Configs/"; 

// DIRETORIO DO PAINEL DE CONTROLE PLAYER EM PASTAS
$pastapainel = 'C:/inetpub/wwwroot/painel';
$extrato = "C:/inetpub/wwwroot/painel/shop/historic/";
$creditos = "C:/inetpub/wwwroot/painel/shop/bonusplayer/";

// DIRETORIOS DA PASTA DATASERVER
// NECESSITA TODAS AS PERMI��ES
$dirUserData = $rootDir."DataServer/userdata/";
$dirUserInfo = $rootDir."DataServer/userinfo/";
$dirUserDelete = $rootDir."DataServer/deleted/";

include_once "class.func.php";
$func=new func;

// Configura��o SQL!!
$host = 'PTL\SQLEXPRESS'; //Sua instancia do SQL
$user = 'sa'; //Seu usu�rio do SQL; Normalmente � SA
$pass = '#$pdl32xq'; //Sua senha do SQL

$connection_string = 'DRIVER={FreeTDS};SERVER='.$host.';PORT=1433;DATABASE=accountdb';
$sod = 'DRIVER={FreeTDS};SERVER='.$host.';PORT=1433;DATABASE=Sod2db';
$clan = 'DRIVER={FreeTDS};SERVER='.$host.';PORT=1433;DATABASE=ClanDB';
$rPTDB = 'DRIVER={FreeTDS};SERVER='.$host.';PORT=1433;DATABASE=rPTDB';
$itemlog = 'DRIVER={FreeTDS};SERVER='.$host.';PORT=1433;DATABASE=ItemLogDB';

// Configura��o SQL!!
$host2 = 'PTL\SQLEXPRESS'; //Sua instancia do SQL
$user2 = 'sa'; //Seu usu�rio do SQL; Normalmente � SA
$pass2 = '#$pdl32xq'; //Sua senha do SQL

$connection_string2 = 'DRIVER={FreeTDS};SERVER='.$host2.';PORT=1433;DATABASE=accountdb';

?>
