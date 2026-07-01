<? if (PT!=1) exit; 
if($_POST['acao'] == ''){
?>

<form id="form" name="form" method="post">
	<table background="imgs/fundo_textura1.gif" width="600" border="0" align="center" cellpadding="0" cellspacing="0">
              <tr>
                <td><table width="100%" border="0" align="center" cellpadding="4" cellspacing="2">
                  
          <tr> 
            <td width="63%" align="center" bgcolor="#000000"><b><font color="#FFFFFF">Sistema para zerar rank</font></b></td>
                  </tr>
                  <tr>
      <th width="721" scope="row"><span class="style2">Resetar o Rank de Players</span></th>
    </tr>
            <td align="center"><input type="submit" class="button6" onclick ="return confirm('Você tem certeza que deseja, resetar o Rank?')" value="Zerar" />
              <input name="acao" type="hidden" id="acao" value="del"></td>
                  </tr>
                </table></td>
              </tr>
  </table>
           </form>
<?
}else{
?>
<form id="form" name="form" method="post">
	<table background="imgs/fundo_textura1.gif" width="600" border="0" align="center" cellpadding="0" cellspacing="0">
              <tr>
                <td><table width="100%" border="0" align="center" cellpadding="4" cellspacing="2">
                  
          <tr> 
            <td width="63%" align="center" bgcolor="#000000"><b><font color="#FFFFFF">Sistema para zerar rank</font></b></td>
                  </tr>
                  <tr>
                  
            <td align="center">
<?php
if($_SESSION['permissao'] < 3){
echo "Você não tem permissão para acessar esta área";
exit;
}
$conn = odbc_connect($connection_string, $user, $pass);
$del = odbc_exec($conn, "DELETE FROM [rPTDB].[dbo].[LevelList]");

if($del){
echo 'Zerado com sucesso!';
}else{
echo 'Ocorreu um erro, tente novamente!';
}
?>

            </td>
                  </tr>
                </table></td>
              </tr>
  </table>
           </form>
<?
}
?>