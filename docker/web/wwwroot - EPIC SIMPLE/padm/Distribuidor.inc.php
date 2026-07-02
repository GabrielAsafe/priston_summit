<?if (PT!=1) exit;?>

 <table width="500" border="0" align="center" cellpadding="0" cellspacing="0">
        <tr>
                <td><table width="50%" border="0" align="center" cellpadding="4" cellspacing="2">
                  <tr>
                    <td colspan="2" align="center" bgcolor="#000000"><b><font color="#FFFFFF">Sistema para mandar item pelo Distribuidor </font></b></td>
                  </tr>
    <td><form id="form1" name="form1" method="post" action="">
      <table width="367" border="0" align="center">
        <tr>
        
<?
include_once "incluir/configura.php";

$connection = odbc_connect($connection_string, $user, $pass) or die ("Erro ao se conectar com o SQL Server");

$query = "SELECT * FROM [ADM].[dbo].[loginGM] WHERE [idGM]='".$_SESSION["NICKGM"]."'";
$excuta_query = odbc_exec($connection,$query);
$qt = odbc_do($connection,$query);
$permisao = odbc_result($qt,3);

   ?>
<table width="499" border="0" align="center" cellpadding="0" cellspacing="0" class="boxTable2">
      <tbody>
        <tr>
          <td height="91" align="center" class="boxContent2">
          <? if($_POST['acao']!="Enviar"){ ?>
          <form method="post" id=" " action="">
          <table width="100%" border="0" align="center" cellpadding="4" cellspacing="2">
		  <tr>
  <td width="35%" align="right"><strong>Nick do Personagem</strong></td>
  <td width="65%">                      <input name="nick" type="text" id="nick" size="20" maxlength="60">                    </td>
                </tr>
<tr>
<td align="right"><strong>Item</strong></td>
<td><input name="item" type="text" id="item" size="10" maxlength="6">
  </td>
</tr>
<tr>
<td align="right"><strong>Classe</strong></td>
<td><select name="classe">
                        <option selected value="0">Sem Classe</option>
                            <option value="1">Lutador</option>
                            <option value="2">Mecanico</option>
                            <option value="3">Arqueira</option>                            
                            <option value="4">Pike</option>
                             <option value="5">Atalanta</option>
                            <option value="6">Cavaleiro</option>
                            <option value="7">Mago</option>
							<option value="8">Prist</option>
                          </select> 
  </td>
</tr>



                  <tr>
                  <td colspan="2" align="center"><input name="acao" type="submit" class="button6" id="acao" value="Enviar"></td>
                  </tr>
                </table>
                <div align="left">
                  <p><strong>Instru&ccedil;&otilde;es de como Utilizar o Sistema
                        de Distribuidor PHP<br>
                    </strong>1 Passo - Digite a Nick do Player que vai receber
                    o item.<br> 
                    2 Passo - Digite em Item o Codigo do Item.(Ex: da125,da222)<br>
                    3 Passo - Em Classe , escolha a classe do Item.<br>
                    4 Passo - Apos preencher todos campos clique em enviar.</p>
                </div>
                    </form>
                    <?
					} else {
	
	$idGM = $_SESSION["NICKGM"];				
	$nick = $_POST['nick'];
	$item = $_POST['item'];
	$class = $_POST['classe'];
	
	function subDiretorio($pasta)
{
 	$total = 0;
	for($i=0;$i<strlen($pasta);$i++)
	{			
		$total += ord(strtoupper($pasta[$i]));
			if($total >= 256)
			{
				$total = $total - 256;
			}
		
	}
	return $total;
}
	
	if(empty($nick)){
		echo "<b>Digite o nick de um char para ver os dados!</b>";
	}elseif(anti_sql($nick) != 0){
		echo "<b>Caracteres especiais foram detectados!</b>";
	} else {
		$procuraPlayer = ''.$rootDir.'DataServer/userdata/'.subDiretorio($nick).'/'.$nick.'.dat';
		if(!file_exists($procuraPlayer)){
			echo "<b>Char no existe!</b>";
		} else {
			$aberto = fopen($procuraPlayer, "r");
			$conteudoDat =fread($aberto,filesize($procuraPlayer));
			@fclose($aberto);
			
			$procuraPlayer = ''.$rootDir.'DataServer/userdata/'.subDiretorio($nick).'/'.$nick.'.dat';
if(file_exists($procuraPlayer))
{

	// l o arquivo para descobrir a ID
	$aberto = fopen($procuraPlayer, "r");
	$conteudoDat =fread($aberto,filesize($procuraPlayer));
	@fclose($aberto);

	$PlayerID = trim(substr($conteudoDat,0x2c0,10),"\x00");
}
			//Dados do Item
$dados_item = "$nick $item  $class \"Obrigado por jogar SnowPT!!\"". "\r\n";

//Pasta de entrega
$pasta_entrega = "".$rootDir."PostBox/".subDiretorio($PlayerID)."/".$PlayerID.".dat";
//Enviando o Item para o Distribuidor
if (file_exists($pasta_entrega)) {
$fp = fopen($pasta_entrega, "a+");
//Escreve o pedido
$escreve = fwrite($fp, "$dados_item");
// Fecha o arquivo
fclose($fp);
} else {
copy("shop.dat",$pasta_entrega);
$fp = fopen($pasta_entrega, "r+");
//Escreve o pedido
$escreve = fwrite($fp, "$dados_item");
// Fecha o arquivo
fclose($fp);
}
	
	
	$ip = $_SERVER["REMOTE_ADDR"];
	$datahoje = date("d-m-Y h:i:s A");
		
	$query_4 = "INSERT INTO [ADM].[dbo].[LogsDistribuidor] ([idGM],[item],[Classe],[id],[nick],[data],[ip]) VALUES ('$idGM','$item','$class','$PlayerID','$nick','$datahoje','$ip')";


	$q_4 = odbc_exec($connection, $query_4);

    if ($q_4){
	echo "<b>O player <font color='#FF0000'> <u>".$PlayerID."</u></font> cujo o nick : <font color='#FF0000'>$nick</font> recebeu o item: <font color='#FF0000'><u>".$item."</u></font> com sucesso no Distribuidor!</b>";
	} else {
		echo "<b>Algum erro ocorreu, contacte o adminsitrador!</b>";
	}
			
}	} }
?>
      </td>
        </tr>
        <tr>
          <td width="499" height="10" background="imgs/23.gif">&nbsp;</td>
        </tr>
      </tbody>
    </table>
	<? } ?>