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
  <meta charset="utf-8">
  <meta name="viewport"    content="width=device-width, initial-scale=1.0">
  <meta name="description" content="">
  <meta name="author"      content="Bruno Adauto Krewer">
  
  <title>Nova Senha - Summit Tale</title>

  <link rel="shortcut icon" href="assets/images/gt_favicon2.png">
  
  <link rel="stylesheet" media="screen" href="http://fonts.googleapis.com/css?family=Open+Sans:300,400,700">
  <link rel="stylesheet" href="assets/css/bootstrap.min.css">
  <link rel="stylesheet" href="assets/css/font-awesome.min.css">

  <!-- Custom styles for our template -->
  <link rel="stylesheet" href="assets/css/bootstrap-theme.css" media="screen" >
  <link rel="stylesheet" href="assets/css/main.css">
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

</head>
<body style="background-color: #c6c6c6;">
<div style="margin-top: 60px;" class="col-md-6 col-md-offset-3 col-sm-8 col-sm-offset-2">
<div class="panel panel-default">
            <div class="panel-body">
                    <table align="center" border="0" cellpadding="6" cellspacing="2" width="100%">
                        <tr>
                          <td align="center" bgcolor="#333333"><font color="#ffffff" face="Verdana, Arial, Helvetica, sans-serif" size="2"><b>Conta Logada</b></font></td>
                        </tr> 
                        <tr>
          <td style="border: 1px solid rgb(206, 206, 206);" align="center" height="100"><p><b><font color="#ff0000">Para utilizar o painel de controle voc&ecirc; tem que estar fora do jogo.</font><br>
            VOC&Ecirc; NÃO PODERÁ </b> acessar nenhum dos <br>
              sistemas do Painel de Controle enquanto estiver dentro do jogo.</p>
            <p><br>
              <a href="index.php?sess=index">Clique aqui se você saiu do jogo.</a><br>
              <a href="index.php?sess=logout">Clique aqui para sair do painel.</a></p></td>
</table>
  </div>
</div>
</body></html>