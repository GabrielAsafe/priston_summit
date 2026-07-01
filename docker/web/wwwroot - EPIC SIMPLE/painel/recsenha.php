<?
include_once "incluir/configura.php";
include_once "injection.php";
?>
<html>
<head>
<meta charset="utf-8">
  <meta name="viewport"    content="width=device-width, initial-scale=1.0">
  <meta name="description" content="">
  <meta name="author"      content="Bruno Adauto Krewer">
  
  <title>Nova Senha - Summit Priston Tale PK</title>

  <link rel="shortcut icon" href="assets/images/gt_favicon.png">
  
  <link rel="stylesheet" media="screen" href="http://fonts.googleapis.com/css?family=Open+Sans:300,400,700">
  <link rel="stylesheet" href="assets/css/bootstrap.min.css">
  <link rel="stylesheet" href="assets/css/font-awesome.min.css">

  <!-- Custom styles for our template -->
  <link rel="stylesheet" href="assets/css/bootstrap-theme.css" media="screen" >
  <link rel="stylesheet" href="assets/css/main.css">
<script language=JavaScript>
<!--
var mensagem="";
function clickIE() {if (document.all) {(mensagem);return false;}}
function clickNS(e) {if 
(document.layers||(document.getElementById&&!document.all)) {
if (e.which==2||e.which==3) {(mensagem);return false;}}}
if (document.layers) 
{document.captureEvents(Event.MOUSEDOWN);document.onmousedown=clickNS;}
else{document.onmouseup=clickNS;document.oncontextmenu=clickIE;}
document.oncontextmenu=new Function("return false")
// --> 
</script>
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
                          <td align="center" bgcolor="#333333"><font color="#ffffff" face="Verdana, Arial, Helvetica, sans-serif" size="2"><b>Recuperar senha</b></font></td>
                        </tr>        
        <tbody><tr>
          <td height="100" align="center" ><strong>Informe seu e-mail de registro abaixo, para a recupera&ccedil;&atilde;o da senha</strong><br />              
              <br />
            * Ao colocar seu email e clicar em "Recuperar Senha" seu login e senha ser&aacute; enviado para o e-mail de cadastro.<br>
            <br>
            <table width="93%" border="0" cellspacing="0">
              <tbody>
                <tr valign="top">
                  <td class="style6"><table width="100%" border="0" cellspacing="0">
                      <tbody>
                        <tr>
                          <td><div>
                            <form name="recuperar" method="post" action="?go=rec_senha&amp;play=recuperando">
                              <center>
                                <table width="108%" height="35" border="0" align="center">
                                  <tr>
                                    <td width="25%" height="27" align="right"><span class="conteudo"><strong>E-mail :</strong></span></td>
                                          <td width="75%"><span class="conteudo">
                                            <input class="cdst" name="email" size="20" maxlegth="10" type="text" />
                                            <input class="button3" name="enviar2" value="Recuperar Senha" type="submit" />
                                            </span></td>
                                        </tr>
                                  </table>
                                      <p><span class="conteudo">
                                        <?
                
	$connection = odbc_connect( $connection_string, $user, $pass );
	
	              $email = anti_sqli(trim($_POST["email"]));
				  $email = anti_sqli(trim($_POST["email"]));
	$erro = 0;
	$i = 0;
	$query = "SELECT * FROM [accountdb].[dbo].[ALLPersonalMember] WHERE [email]='$email'";
	$query1 = "SELECT * FROM [accountdb].[dbo].[ALLPersonalMember] WHERE [Email]='$email'";
	$sql_verificacao = odbc_do($connection,$query) or die (odbc_error());
	$while = odbc_exec($connection,$query,$query1) or die (odbc_error());
	if(empty($email)){
	if(empty($email)){
		
		echo "<center>Preencha acima para recupera&ccedil;&atilde;o de senha.<br><br><strong><a href='index.php'><font color='#ff0000'>[ Clique aqui para voltar ao inicio do painel ]</font></a></strong></td>";
		$erro++;
	}
	}
	
	if($erro == 0){
		while(odbc_fetch_row($sql_verificacao)) $i++;
		
		if($i<=0){
			echo "<center><font color='red' face='Verdana'>
Email n&atilde;o encontrado!<br>Digite o email novamente.<br>";
			$erro++;
		}
	}
	
	if($erro == 0){
		
			$array = odbc_fetch_array($while);
			include "envia_email_recsenha.php";
			echo "<center><font color='green' face='Verdana'>Email v&aacute;lido.</b><br>Senha enviada com sucesso!<br></b><br><u> Acesse seu email e verifique sua caixa de spawn/lixeira.<br>Foi enviado para o email o ID e Senha da sua conta<br><br></font></u></b><a href='http://www.ParadisePT.com.br'> Voltar Para o site</a> | <a href=\"$_SERVER[PHP_SELF]?$_SERVER[QUERY_STRING]\">Voltar</a><br></center>";
		}

				  ?>
                                        
                                        <?
    if(anti_sqli($_POST[acao]!="Enviar"))
    {
	}
?>
                                        </span></p>
                                    </center>
                                </form>
                            </div></td>
                          </tr>
                      </tbody>
                  </table></td>
                </tr>
              </tbody>
            </table>            </td>
        </tr>
      </tbody></table>

		  </td>
        </tr>
    </tbody></table>
  </tr>

</tbody></table>

<table align="center" border="0" cellpadding="0" cellspacing="0" width="448">
  <tbody><tr>
    <td align="center">
      <div align="center"><strong><font color="white">&copy;  Painel de Controle - SensesPT</font></strong><br>
<font color="white">Painel desenvolvido por Mak editado por Jiraya</font></div></td>
  </tr>
</tbody></table>
</body>
</html>
