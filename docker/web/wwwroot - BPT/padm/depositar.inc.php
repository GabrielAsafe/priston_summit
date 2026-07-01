<?
if (PT!=1) exit;
?>
<?
include_once "injection.php";
include_once "incluir/configura.php";
?>
<?

//não mecha!

$connection = odbc_connect($connection_string,$user,$pass); //aqui ele usa a func. odbc_connect para se conectar com o SQL, usando a String, o User e o passwd!

	$query4 = "SELECT * FROM [accountdb].[dbo].[AllGameUser] WHERE [userid] <> '' ";
	$q4 = odbc_exec($connection,$query4);
	$totalcontas = AcertarLinhasODBC($connection,$query4);
	
	function AcertarLinhasODBC($conexao,$query){
		$resultado = odbc_exec($conexao, $query);
		$contador=0;
		while($temp = odbc_fetch_into($resultado, &$counter)){
			$contador++;
		}
		return $contador;
	}
?>
<?
	include_once "injection.php";
	include_once "incluir/configura.php";
	
	$username=$_POST['ID2'];
	$valor=$_POST['quantia2'];

	//Pega Saldo existente no arquivo do player.
	$query = "SELECT * FROM [accountdb].[dbo].[AllPersonalMember] WHERE [userid]='$username'";
	$q = odbc_exec($connection, $query);
	$ouro = odbc_fetch_array($q);
	$saldo = $ouro['Gold'];
	
	//Soma o Saldo existente do tal player, e soma com o que voce desejar depositar.
	$novaouro = $saldo + $valor;
	$sacou = "UPDATE [accountdb].[dbo].[AllPersonalMember] SET [Gold]='$novaouro' WHERE [userid]='$username' ";
	$go = odbc_exec($connection, $sacou);

	if(!$go){
	echo "Erro ao salvar o saldo!";
	} else {
	echo "<br>O gold virtual foi entregue com sucesso ao banco virtual do player, $username";
	} ?>
<?  ?>