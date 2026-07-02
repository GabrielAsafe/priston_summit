<?
define("Painel","1");
/*----------------------------------------------------------------------
----------------------------------------------------------------------*/

// TITULO DAS PAGINAS DO PAINEL
$version="Painel de Controle";

// NOME DO SERVIDOR
$nomedoserver="SummiTale";

// IP DO HOST/PC/SERVIDOR
$ipdoservidor="124.198.128.16";

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
$connection_string = 'DRIVER={SQL Server};SERVER=PTL\SQLEXPRESS;DATABASE=accountdb';

// USUARIO DO SQLEXPRESS USER E PASSWORD 
$user = 'sa';
$pass = '#$pdl32xq';
?>
