<?
if (PT!=1) exit;
?>
<table background="imgs/fundo_textura1.gif" width="600" border="0" align="center" cellpadding="0" cellspacing="0">
              <tr>
                <td><table width="100%" border="0" align="center" cellpadding="4" cellspacing="2">
                  <tr>
                    <td align="center" bgcolor="#000000"><b><font color="#FFFFFF">
					Sistema para resetar SOD</font></b></td>
                  </tr>
<tr>

      </label></th>
    </tr>
  </table>
</form>
<form id="form2" name="form2" method="post" action="">
  <table width="728" height="236" border="0" align="center" cellpadding="0" cellspacing="0">
    <tr>
      <th width="728" height="38" scope="row"><span class="style2"><br>Zerando os pontos de SoD Individual</span></th>
    </tr>
    <tr>
      <th scope="row"><label>
                <input type="hidden" name="acao" value="nick"><input type="submit" class="button6" onclick ="return confirm('Você tem certeza que deseja, zerar a pontuação de sod individual !?')" value="zerar sod individual"><br>
        <input name="acao" type="hidden" id="acao" value="deletsod" />
      
        <?
	  if($_POST[acao] == "deletsod")
	  {
if($_SESSION['permissao'] < 3){
echo "Você não tem permissão para acessar esta área";
exit;
}

$conn = odbc_connect($sod,$user,$pass) or die("Erro ao conectar");

$seleciona = "SELECT * FROM dbo.SOD2RecBySandurr";

$sql = odbc_exec($conn,$seleciona);
odbc_do($conn,"DELETE FROM dbo.SOD2RecBySandurr");
echo"<font color='red'><b>Todos os dados do sod individual foram zerados com sucesso!<br>";
}
?>
<table width="235" height="81" border="0" align="center" cellpadding="0" cellspacing="0">
</table>
      </label></th>
    </tr>
  </tr>
<tr>
                  <td colspan="2" align="center"><div align="left">
                    <p><strong>Obs: cuidado ao usar está ferramenta.<br>
                    </strong>Ferramenta protegida com confirmação de zeração, se por acaso você clicar em zerar sem querer, aparecera uma mensagem se realmente deseja fazer está ação basta clicar cancelar, caso deseja fazer a ação clique em zerar e confirme clicando Ok<br> 
                    </table>
                  </div></td>
                  </tr>

                  <tr>
  </table>
</form>
<? ?>    