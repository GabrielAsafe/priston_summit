<?if (PT!=1) exit;?>

<?
include_once "incluir/configura.php";
include_once "injection.php";
?>
<link href="css/style.css" rel="stylesheet" type="text/css" />

<br />
<table background="imgs/fundo_textura1.gif" width="712" border="0" align="center" cellpadding="0" cellspacing="0">
              <tr>
                <td width="712"><table width="100%" border="0" align="center" cellpadding="4" cellspacing="2">
                  <tr>
                    <td colspan="2" align="center" bgcolor="#000000"><b><font color="#FFFFFF"> Administração de deletação de contas</font></b></td>
                  </tr></table>
<div align="center">Digite o id abaixo no campo em branco e clicar em deletar id para deletar que voc&ecirc; citou no campo.<br />
</div>
<form method="post" action="index.php?sessadm=delconta">
  <div align="center"><br>ID Para deletar:
  <input name="usuario" type="text" id="usuario" />
  <input type="submit" class="button6" onclick ="return confirm('Voc&ecirc; Tem certeza que deseja deletar a id citada!?')" value="Deletar ID" />
  </div><br />
  <table width="100%" border="0" align="center" cellpadding="4" cellspacing="2">
    <tr>
      <td colspan="2" align="center"><div align="center">
        <p><strong>Instru&ccedil;&otilde;es de como Utilizar o Sistema de Deletar conta<br />
          </strong>1 Passo - Procure saber o id que voc&ecirc; quer deletar.<br />
          2 Passo - Digite o id no campo em branco e clique em deletar ID.<br />
          3 Passo - quando voc&ecirc; clicar em deletar id automaticamente vai aparecer um alerta confirme oque deseja..<br />
Se voc&ecirc; deseja realmente deseja deletar a id citado clique em 'Ok' se quer cancelar a a&ccedil;&atilde;o clique em Cancelar<br />
3 Passo - Apos confirma, vai aparecer que a ID foi deletado com sucesso, ou ent&atilde;o vai falar que a ID n&atilde;o existe ou deu erro na hora de ser deletado.<br />
</table>
        </p>
</div></td>
    </tr>
  </table>
  <br />
</form>