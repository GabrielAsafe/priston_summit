<?if (PT!=1) exit;?>

<?
include_once "injection.php";
include_once "incluir/configura.php";

$connection = odbc_connect($connection_string, $user, $pass) or die ("Erro ao se conectar com o SQL Server");
?>
<form id="form" name="form" method="post">
	<table background="imgs/fundo_textura1.gif" width="600" border="0" align="center" cellpadding="0" cellspacing="0">
              <tr>
                <td><table width="100%" border="0" align="center" cellpadding="4" cellspacing="2">
				<tr> 
				<td width="63%" align="center" bgcolor="#000000"><b><font color="#FFFFFF">Status das Contas</font></b></td>
                  </tr>
<form id="form1" name="form1" method="post" action="">
  <table width="555" border="0">
    <tr>
      <td width="130">ID</td>
      <td width="130">SENHA </td>
	  <td width="150">STATUS</td>
      <td width="70">DELETAR</td>
    </tr>
  </table>
  <br />
  <?
 $conn = odbc_connect($connection_string,$user,$pass);
	$query_contas = "SELECT * FROM [accountdb].[dbo].[AllGameUser]";
	$exec_contas = odbc_exec($conn,$query_contas);
	$do_contas = odbc_do($conn,$query_contas);
	while(odbc_fetch_row($do_contas)){
	$usuario = odbc_result($do_contas,1);
	$senha = odbc_result($do_contas,2);
	$status = odbc_result($do_contas,12);
		
	?>
  <table width="555" border="0">
    <tr>
      <td width="130"><? echo $usuario?></td>
      <td width="130"><? echo $senha?></td>
     <td width="150"><?if($status == "0"){
	echo"<font color='green'><b>Unban</font>";
	}else{
	echo"<font color='red'><b>Banido</font>";
	}?></td>
      <td width="70"><label><span class="alert_color">
        <input name="usuario[]" type="checkbox" id="usuario[]" value="<? echo $usuario; ?>" />
      </span></label>      </td>
    </tr>
  </table>
  <?
  }
  ?>
  <table width="308" border="0" align="center">
    <tr>
      <td width="302" align="center"><label>
        <input type="submit" name="button" id="button" class="button6" value="Deletar selecionados" />
        <input name="acao" type="hidden" id="acao" class="button6" value="del" />
       <br />
        <?
		if($_POST['acao'] = "del"){
		$usuario = $_POST['usuario'];
		$pri = ucfirst($usuario);
		$conn = odbc_connect($connection_string,$user,$pass);
		for($i = 0; $i < count($usuario); $i++){
		$sql = "DELETE FROM [accountdb].[dbo].[AllGameUser] where userid='$usuario[$i]'";
		$sql2 = "DELETE FROM [accountdb].[dbo].[".$pri ."GameUser] where userid='$usuario[$i]'";
		$sql3 = "DELETE FROM [accountdb].[dbo].[AllPersonalMember] where Userid='$usuario[$i]'";
		$sql4 = "DELETE FROM [accountdb].[dbo].[".$pri ."PersonalMember] where Userid='$usuario[$i]'";
		$exec = odbc_exec($conn,$sql);
		$exec = odbc_exec($conn,$sql2);
		$exec = odbc_exec($conn,$sql3);
		$exec = odbc_exec($conn,$sql4);
				}
		
		echo"<font color='green'><b><center>CONTA DELETADA COM SUCESSO!";
		
		
		
		
		}
		
		
		
		?>
      </label></td>
    </tr>
  </table>
</form>
</body>
</html>