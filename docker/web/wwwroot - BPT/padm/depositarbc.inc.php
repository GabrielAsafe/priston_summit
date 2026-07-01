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
	$query = "SELECT * FROM [ClanDb].[dbo].[CL] WHERE [ClanName]='$username'";
	$q = odbc_exec($connection, $query);
	$ouro = odbc_fetch_array($q);
	$saldo = $ouro['SiegeMoney'];
	
	//Soma o Saldo existente do tal player, e soma com o que voce desejar depositar.
	$novaouro = $saldo + $valor;
	$sacou = "UPDATE [ClanDb].[dbo].[CL] SET [SiegeMoney]='$novaouro' WHERE [ClanName]='$username' ";
	$go = odbc_exec($connection, $sacou);

	if(!$go){
	echo "Erro ao salvar o saldo!";
	} else {
	echo "<br>Forão entregue o Gold virtual, com sucesso para o lider de BC do clan $username<br>So comunicar o lider para pegar o gold no gerenciador de bc 'aquele velho que fica dentro do castelo'";
	} ?>
<?  ?>