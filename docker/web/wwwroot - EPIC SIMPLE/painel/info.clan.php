<?
require "incluir/configura.php";
$connection = odbc_connect($connection_string,$user,$pass);
$ChName = $_SESSION["charName"];
$username = $_SESSION["charID"];
$query = "SELECT * FROM [clandb].[dbo].[UL] WHERE [UserID]='$username'";
$q = odbc_do($connection,$query);
while(odbc_fetch_row($q)) {
$nome_clan = odbc_result($q,7);
$ChName2 = odbc_result($q,4);
$img = odbc_result($q,9);
}
if($ChName == $ChName2){
echo $nome_clan;
} else {
echo "Personagem sem clan";
}
?>