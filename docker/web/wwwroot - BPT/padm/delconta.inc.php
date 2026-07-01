<?
if (PT!=1) exit;
?>
<?

include_once "injection.php";
include_once "incluir/configura.php";

$usuario = $_POST['usuario'];

if(empty($usuario)){
echo "Digite o id que você deseja deletar do banco de dados!";
echo "<meta HTTP-EQUIV='Refresh' CONTENT='3;URL=index.php?sessadm=deletarconta'>";
}

$conexao = odbc_connect($connection_string, $user, $pass);

$sql = odbc_exec($conexao, "SELECT * FROM [accountdb].[dbo].[AllGameUser]");

while($exe = odbc_fetch_array($sql)){
$usuariob = $exe['userid'];
}

if($usuario == $usuariob){
echo "O id $usuario não foi encontrado em nosso banco de dados.";
echo "<meta HTTP-EQUIV='Refresh' CONTENT='3;URL=index.php?sessadm=deletarconta'>";
}

else{
odbc_do($conexao, "DELETE FROM [accountdb].[dbo].[AllGameUser] WHERE userid='$usuario'");
odbc_do($conexao, "DELETE FROM [accountdb].[dbo].[".( strtoupper($usuario[0]) ) ."GameUser] WHERE userid='$usuario'");
odbc_do($conexao, "DELETE FROM [accountdb].[dbo].[AllPersonalMember] WHERE userid='$usuario'");
odbc_do($conexao, "DELETE FROM [accountdb].[dbo].[".( strtoupper($usuario[0]) ) ."PersonalMember] WHERE userid='$usuario'");
echo "ID $usuario foi deletada com sucesso do banco de dados!";
echo "<meta HTTP-EQUIV='Refresh' CONTENT='3;URL=index.php?sessadm=deletarconta'>";
}
?>