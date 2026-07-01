<?
/*----------------------------------------------------------------------
Painel Player Versão 0.4
Desenvolvidor Por: Mak (sirmakloud@gmail.com)
Editado Por: Jiraya (richard.cva21@hotmail.com(
----------------------------------------------------------------------*/
include_once "incluir/configura.php";
include_once "injection.php";
?>
<html>
<head>
  <title><?=$version?></title>
                    <link rel="shortcut icon" href="imgs/Icon.ico">
<meta http-equiv="Content-Type" content="text/html; charset=ISO-8859-1">
<link href="css/style.css" rel="stylesheet" type="text/css">
<script language="JavaScript" type="text/JavaScript">
<!--
function disabledBttn(formname)
{
    if (document.all || document.getElementById) {
        for (i=0;i<formname.length;i++) {
            var bttn=formname.elements[i];
            if(bttn.type.toLowerCase()=="submit" || bttn.type.toLowerCase()=="reset" || bttn.type.toLowerCase()=="button")
                bttn.disabled=true;
        }
    }
}
//-->
</script>

<style type="text/css">
<!--
.style2 {	color: #FF0000;
	font-size: 24px;
}
-->
</style>
</head>
<body bgcolor="transparent" leftmargin="0" topmargin="0" allowtransparency="true" marginheight="0" marginwidth="0">
<table align="center" border="0" cellpadding="0" cellspacing="0" width="448">
  <tbody><tr>
    <td><img src="imgs/topo_contas.png" alt="Editado por Jiraya" height="100" width="448"></td>
  </tr>
</tbody></table>
<table align="center" border="0" cellpadding="0" cellspacing="0" width="448">
  <tbody><tr>
    <td><table align="center" border="0" cellpadding="0" cellspacing="0" width="448">
        <tbody><tr>
          <td background="imgs/fundo_textura.gif" bgcolor="#ffffff">

            <table align="center" border="0" cellpadding="0" cellspacing="0" width="448">

        <tbody><tr>
          <td class="padding_all" style="border: 1px solid rgb(206, 206, 206);" align="center" height="100"><p>Você está <font color="#ff0000"><b>Banido</b></font> do Server portanto<b> <br>
                NÃO PODERÁ </b> acessar nenhum dos <br>
              sistemas do Painel de Controle.</p>
            <p>caso isso tenha sido um mal entendido,voc&ecirc; deve falar com um de nossos GameMaster pelo nosso f&oacute;rum para ver qual é a sua situa&ccedil;&atilde;o.<br>
              <br>
              <a href="index.php?sess=logout">Deslogar do Painel de Controle</a></p></td>
        </tr>
      </tbody></table>

		  </td>
        </tr>
    </tbody></table>

      </td>
  </tr>

</tbody></table>

<table align="center" border="0" cellpadding="0" cellspacing="0" width="448">
  <tbody><tr>
    <td><span class="padding_all" style="border: 1px solid rgb(206, 206, 206);"> <img src="imgs/rodape.gif" alt="xpt" height="20" width="448"></span></td>
  </tr>
</tbody></table>
</body></html>