<?if (PT!=1) exit;?>

<?
include_once "injection.php";
include_once "incluir/configura.php";

$connection = odbc_connect($connection_string, $user, $pass) or die ("Erro ao se conectar com o SQL Server");

      $query= "SELECT * FROM [ClanDB].[dbo].[CT]";		//Query que pega usuarios online
      $excuta_query = odbc_exec($connection, $query);	//Executa a query e verifica os usuarios Online
      $contador_total = AcertarLinhasODBC($connection, $query);	//conta quantas linhas tem na TABELA CT COM FUNÇÃO PARA ARRUMAR BUG DO ODBC_NUM_ROWS

function AcertarLinhasODBC($connection, $query){
	$resultado = odbc_exec($connection, $query);
	$contador=0;
	while($temp = odbc_fetch_into($resultado, &$counter)){
		$contador++;
	}
	return $contador;
}

?>

<style type="text/css">
<!--
#form table tr td table tr td p .style2 {
	color: #060;
}
#form table tr td table tr td p .style1 {
	color: #F00;
}
#form table tr td table tr td b font {
	font-weight: bold;
}
#form table tr td table tr td p {
}
-->
</style>
<body>
<form id="form" name="form" method="post" action="pagina.php">
	<table width="600" border="0" align="center" cellpadding="0" cellspacing="0">
              <tr>
                <td><table width="100%" border="0" align="center" cellpadding="4" cellspacing="2">
                  
          <tr> 
            <td width="63%" align="center" bgcolor="#000000"><b><font color="#FFFFFF">Player's online</font></b></td>
                  </tr>
                  <tr>
                  
            <td height="46" align="center"><p>Player's online: <b><?php echo $contador_total?></b><br></p></td>
                  </tr>


                </table></td>
              </tr>
</table>			  
  </table>
</form>
</body>
</html>
