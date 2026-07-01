<?
if (PT!=1) exit;
?>
<?
if($_SESSION['permissao'] < 3){
echo "Você não tem permissão para acessar esta área";
exit;
}
include_once "injection.php";
include_once "incluir/configura.php";

$nomedoclan = $_POST['nomeclan'];

if(empty($nomeclan)){
echo "Digite o nome do Clan que você deseja deletar!";
echo "<meta HTTP-EQUIV='Refresh' CONTENT='3;URL=index.php?sessadm=clan'>";
}

$conexao = odbc_connect($clan, $user, $pass);

$sql = odbc_exec($conexao, "SELECT * FROM [ClanDb].[dbo].[CL]");

while($exe = odbc_fetch_array($sql)){
$clannome = $exe['ClanName'];
$totalmembros = $exe['MemCnt'];
}

if($nomeclan != $clannome){
echo "O Clan $nomeclan não foi encontrado em nosso banco de dados.";
echo "<meta HTTP-EQUIV='Refresh' CONTENT='3;URL=index.php?sessadm=clan'>";
}

else{
odbc_do($conexao, "DELETE FROM [ClanDb].[dbo].[UL] WHERE ClanName='$nomeclan'");
odbc_do($conexao, "DELETE FROM [ClanDb].[dbo].[CL] WHERE ClanName='$nomeclan'");
echo "O clan $nomeclan foi excluido com sucesso!";
echo "<meta HTTP-EQUIV='Refresh' CONTENT='3;URL=index.php?sessadm=clan'>";
}
?>