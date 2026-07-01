<?
if (PT!=1) exit;
?>
<link href="css/style.css" rel="stylesheet" type="text/css" />

<br />
<table background="imgs/fundo_textura1.gif" width="600" border="0" align="center" cellpadding="0" cellspacing="0">
              <tr>
                <td><table width="100%" border="0" align="center" cellpadding="4" cellspacing="2">
                  <tr>
                    <td align="center" bgcolor="#000000"><b><font color="#FFFFFF"> Bem vindo ao Painel de Clans Administração</font></b></td>
                  </tr></table>
<div align="center">Basta digitar o nome do clan abaixo no campo e clicar em excluir clan para deletar o clan.<br />
</div>
<form id="form1" name="form1" method="post" action="">
  <div align="center">
    <table width="299" border="0">
      <tr><td width="110">Nome do clan</td>
        <td width="179"><input name="clan" type="text" /></td>
      </tr>
      <tr>
        <td>&nbsp;</td>
        <td><input name="button" type="submit" class="button6" id="button" value="Deletar" />
          <input name="acao" type="hidden" id="acao" value="deletar" /></td>
        
        </tr>
    </table>
  </div>
</form>
<table width="300" border="0">
  <tr>
    <td><?
	if($_SESSION['permissao'] < 3){
echo "Você não tem permissão para acessar esta área";
exit;
}
	if($_POST[acao] == "deletar"){
	$clan = $_POST['clan'];
	$conn = odbc_connect($connection_string,$user,$pass);
		if(empty($clan)){
	echo"<B><font color='red'><center><center>preencha o nome do clan";
	}else{
	$query = "SELECT * FROM [ClanDB].[dbo].[CL] where ClanName='$clan'";
	$deleta = "DELETE  FROM [ClanDB].[dbo].[CL] where ClanName='$clan'";
	$exec = odbc_exec($conn,$query);
	$del_clan = odbc_exec($conn,$deleta);
	$del_clan2 = odbc_exec($conn,"DELETE  FROM [ClanDB].[dbo].[UL] where ClanName='$clan'");
	echo"<center><b><font color='green'><center><center>Clan deletado com sucesso!!!!";
	}
	}
	?>
	
	</td>
  </tr>
</table>
</body>
</html>

  </div><br />
  <table width="100%" border="0" align="center" cellpadding="4" cellspacing="2">
    <tr>
      <td colspan="2" align="center"><div align="center">
        <p><strong>Instru&ccedil;&otilde;es de como Utilizar o Sistema de Deleta Clans<br />
          </strong>1 Passo - Procure saber o nome do clan que voc&ecirc; quer deletar.<br />
          2 Passo - Digite o nome do clan no campo em branco e clique em deletar clan.<br />
          3 Passo - quando voc&ecirc; clicar em deletar clan automaticamente vai aparecer um alert confirme oque deseja..<br />
Se voc&ecirc; deseja realmente deseja realmente deletar o clan citado clique em 'Ok' se quer cancelar a a&ccedil;&atilde;o clique em Cancel<br />
3 Passo - Apos confirma, vai aparecer que o clan foi deletado com sucesso, ou ent&atilde;o vai falar que o clan n&atilde;o existe ou deu erro na hora de ser deletado.<br />
        </p>
</div></td>
    </tr>
  </table>
  <br />
</form>