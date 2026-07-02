<?
define("Painel","1");
/*----------------------------------------------------------------------
----------------------------------------------------------------------*/

// TITULO DAS PAGINAS DO PAINEL
$version="Painel de Controle";

// NOME DO SERVIDOR
$nomedoserver="SummiTale";

// IP DO HOST/PC/SERVIDOR
$ipdoservidor = $_SERVER["HTTP_HOST"];

// PRISTON TALE SERVER DIRETORIO
$rootDir = "/srv/summit/Servidor/"; 

include_once "class.func.php";
$func=new func;

// PRISTON TALE MLTIPLAS DATASERVER 
// DIRETORIO DE PERMIO

$dirUserData = $rootDir."DataServer/userdata/";
$dirUserInfo = $rootDir."DataServer/userinfo/";
$dirUserDelete = $rootDir."DataServer/deleted/";

// MUDANA PARA O SEU NOME DO COMPUTADOR
$connection_string = 'SummitSQL';

// USUARIO DO SQLEXPRESS USER E PASSWORD 
$user = 'sa';
$pass = 'SummitTale@12345';
?>
