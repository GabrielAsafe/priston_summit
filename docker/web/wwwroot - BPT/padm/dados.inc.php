<? if (PT!=1) exit;?>
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
            <td width="63%" align="center" bgcolor="#000000"><b><font color="#FFFFFF">Dados dos Chars</font></b></td>
                  </tr>
				  <tr>
                <td><table width="30%" border="0" align="center" cellpadding="1"  cellspacing="1">
                <tr>
          <td><b>ID: </b></td>
          <td><label>
            <input type="text" name="id"  id="id" />
          </label></td>
        </tr>
        <tr>
          <td>&nbsp;</td>
          <td><label>
            <input type="submit" name="button" id="button" class="button6" value="Obter informa&ccedil;&otilde;es" />
            <input name="acao" type="hidden" id="acao" class="button6" value="info" />
          </label></td>
        </tr>
      </table>
        </form>
    </td>
  </tr>
</table>
<?{
$id = $_POST['id'];
$pesquisa = $dirUserInfo . ( $func->numDir($id)) . "/" . $id . ".dat";
	   if(empty($id)){
	   echo"<b><font color='red'><center>Preencha a ID</center></font></b>";
	   }else{
	   
		   if(!file_exists($pesquisa)){
		   echo"<font color='red'>o arquivo a ser pesquisado n&atilde;o existe!!!";
		   }else{
		  $abre = fopen($pesquisa,"r");
$dados = fread($abre,filesize($pesquisa));
$personagens=array(
                    "48" => trim(substr($dados,0x30,15)),
                    "80" => trim(substr($dados,0x50,15)),
                    "112"=> trim(substr($dados,0x70,15)),
                    "144"=> trim(substr($dados,0x90,15)),
                    "176"=> trim(substr($dados,0xb0,15)),
                );
				                 
            }
            
				@fclose($abre);



$conn = odbc_connect($connection_string,$user,$pass);
$query_all = "SELECT * FROM [accountdb].[dbo].[AllGameUser]  WHERE [userid]='$id'";
$q_all = odbc_exec($conn, $query_all);
                    $qt_all = odbc_do($conn, $query_all);

                    odbc_fetch_row($qt_all);
					$estado_c = odbc_result($qt_all,12);

 $query = "SELECT * FROM [accountdb].[dbo].[AllPersonalMember]  WHERE [userid]='$id'";
                    $q = odbc_exec($conn, $query);
                    $qt = odbc_do($conn, $query);

                    odbc_fetch_row($qt);
					$senha = odbc_result($qt,3);
					$nome=odbc_result($qt,6);
                    $sobrenome=odbc_result($qt,7);
                    $email=odbc_result($qt,12);
					$cidade=odbc_result($qt,16);
					$estado=odbc_result($qt,17);
					$sexo=odbc_result($qt,28);
					$nasc_dia=odbc_result($qt,25);
					$nasc_mes=odbc_result($qt,26);
					$nasc_ano=odbc_result($qt,27);
					$perg=odbc_result($qt,4);
					$resp=odbc_result($qt,5);
					
					
                    ?>
<br />
<table width="580" height="0" border="0" colspan="0" align="right" cellpadding="0" cellspacing="0">
    <tr>
    <td><b>Chars na Conta</b></td>
	<font color="#FF0000">
    <td><?
	if(count($personagens)>0)
                {
	
	foreach($personagens as $chave=>$valor)
					 {
					echo "<b>$valor&nbsp;&nbsp;";
                    }
					}
					?></td></font></br>
  </tr>
</table>
</body>
</html>
<?
}
}

?>
