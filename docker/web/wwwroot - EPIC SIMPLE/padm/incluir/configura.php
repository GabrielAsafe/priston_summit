<?
define("PT","1");

/*----------------------------------------------------------------------
Painel de Adminstrao
PESSOAS QUE CONTRIBUIRAM NO DESENVOLVIMENTO DO PAINEL DE ADMINSTRAO.
Niik - Jeck - MecanicoGM - Nckolas - D kin
----------------------------------------------------------------------*/

$banco = "CoinShopDB";

// TITULO DAS PAGINAS DO PAINEL
$version="Painel de Administrao";

//Nome do Executavel do servidor.exe (coloque s o nome, sem a exteno .exe)
$Server = "Server";

//Porta do servidor
$portasv = "1997";

// IP DA MAQUINA QUE O PAINEL EST ATUALMENTE
$iphost = "127.0.0.1";

// DIRETORIO DA SERVER FILES NO HOST
$rootDir = "/srv/summit/Servidor/"; 
$rootDir2 = "/srv/summit/Servidor/Configs/"; 

// DIRETORIO DO PAINEL DE CONTROLE PLAYER EM PASTAS
$pastapainel = dirname(__DIR__, 2).'/painel';
$extrato = dirname(__DIR__, 2)."/painel/shop/historic/";
$creditos = dirname(__DIR__, 2)."/painel/shop/bonusplayer/";

// DIRETORIOS DA PASTA DATASERVER
// NECESSITA TODAS AS PERMIES
$dirUserData = $rootDir."DataServer/userdata/";
$dirUserInfo = $rootDir."DataServer/userinfo/";
$dirUserDelete = $rootDir."DataServer/deleted/";

include_once "class.func.php";
$func=new func;

// Configurao SQL!!
$host = 'summittale-mssql'; //Sua instancia do SQL
$user = 'sa'; //Seu usurio do SQL; Normalmente  SA
$pass = 'SummitTale@12345'; //Sua senha do SQL

$connection_string = 'SummitSQL';
$sod = 'DRIVER={SQL Server};SERVER='.$host.';DATABASE=Sod2db';
$clan = 'DRIVER={SQL Server};SERVER='.$host.';DATABASE=ClanDB';
$rPTDB = 'DRIVER={SQL Server};SERVER='.$host.';DATABASE=rPTDB';
$itemlog = 'DRIVER={SQL Server};SERVER='.$host.';DATABASE=ItemLogDB';

// Configurao SQL!!
$host2 = 'summittale-mssql'; //Sua instancia do SQL
$user2 = 'sa'; //Seu usurio do SQL; Normalmente  SA
$pass2 = 'SummitTale@12345'; //Sua senha do SQL

$connection_string2 = 'SummitSQL';

?>
