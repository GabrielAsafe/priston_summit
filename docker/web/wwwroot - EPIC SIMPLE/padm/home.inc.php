<?if (PT!=1) exit;?>
<?php
include_once "incluir/configura.php";
if($_POST[acao]!="search")
{
?>
<style type="text/css">
<!--
.branco {
	color: #FFF;
}
.vermelho {
	color: #F00;
}
.branco {
	color: #FFF;
}
.cinza {
	color: #C0C0C0;
}
-->
</style>
  <link href="css/style.css" rel="stylesheet" type="text/css" />
<form method="post" onSubmit="disabledBttn(this)" action="<?=$_SERVER[PHP_SELF]."?".$_SERVER[QUERY_STRING]?>">
	<table background="imgs/fundo_textura1.gif" width="730" border="0" align="center" cellpadding="0" cellspacing="0">
              <tr>
                <td width="730"><table width="99%" border="0" align="center" cellpadding="4" cellspacing="2">
                  <tr>
                    <td colspan="2" align="center" bgcolor="#000000"><b><font color="#FFFFFF"> Bem vindo ao Painel de Administração - INDEX </font></b></td>
                  </tr>
                  <td colspan="2" align="center"><div align="left">
                    <p align="center"><strong>Apenas Game Master do servidor tem acesso a está area.<br>
                    </strong>Seu IP está sendo gravado, no sistema para seguran&ccedil;a do servidor<br> 
                    Alguns sistemas do painel possui informações de como ser utilizado siga-o caso tenha dificuldade para usar o sistema.<br>
                    Para seguran&ccedil;a de sua conta, apos utilizar o painel deslogue caso n&atilde;o deslogue sem problemas sera deslogado em alguns minutos automaticamente</p>
                  </div></td>

                  <tr>
                  </tr>
                </table></td>
              </tr>
  </table>
</form>
<p>
  <?
}
?>
</p>