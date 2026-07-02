<?
ob_start('ob_gzhandler');
session_start();
header('p3p: CP="CAO PSA OUR"');

include_once "incluir/configura.php";
include_once "injection.php";


//Segurana para o PHP
function escapestrings($string)
{
    //se magic_quotes no estiver ativado, escapa a string
    if (!get_magic_quotes_gpc())
    {
        return mysql_escape_string($string); // funo nativa do php para escapar variveis.
    }
    else
    {
        // caso contrario
        return $string; // retorna a varivel sem necessidade de escapar duas vezes
    }
}



if(!session_is_registered("gmfodoes"))
{
?>
<!DOCTYPE HTML PUBLIC "-//W3C//DTD HTML 4.01 Transitional//EN">
<html>
<head>
<title><?=$version?></title>
<meta http-equiv="Content-Type" content="text/html; charset=iso-8859-1">
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
body {
	margin-top: 30px;
}
.style1 {color: #FF0000}
.style2 {color: #0000FF}
-->
</style></head>

<body background="imgs/fundo_cinza_conteudos.jpg" leftmargin="0" marginwidth="0" allowtransparency="true">
<table border="0" align="center" cellpadding="0" cellspacing="0">
  <tr>
    <td align="center" valign="middle">
      <table width="800" height="95" border="0" align="center" cellpadding="0" cellspacing="0">
        <tr>
		<td><img src="imgs/banner.gif"></td>
        </tr>
      </table>
      <table width="808" bgcolor="#FFFFFF" border="1" bordercolor="#333333" cellpadding="0" cellspacing="0">


        <tr>
          <td width="804" align="center" class="padding_all" style="border-bottom: solid 1px #cecece; border-top: solid 1px #cecece; border-left: solid 1px #cecece; border-right: solid 1px #cecece">
<?
    if($_POST['acao']!="Logar")
    {
?>

       <form method="post" onSubmit="disabledBttn(this)" action="index.php">
            <table width="100%" border="0" align="center" cellpadding="2" cellspacing="2">
              <tr>
                <td colspan="2"><div align="center">
                  <tr>
                <td colspan="2"><div align="center"><strong>Informe seus dados de acesso:<br>
                  <br>
                </strong></div></td>
              </tr>
                </div></td>
              </tr>
              <tr>
                <td width="40%"><div align="right"><font color="#000000"><strong>    CONTA: </strong></font></div></td>
                <td width="60%">             <input type="text" name="username" size="20" maxlength="10"></td>
              </tr>
              <tr>
                <td><div align="right"><font color="#000000"><strong> SENHA: </strong></font></div></td>
                <td><input type="password" name="password" size="20" maxlength="8"></td>
              </tr>


                  <td colspan="2" align="center"><input name="acao" id="acao" value="Entrar no Painel" class="button6" type="submit">
                    <input name="acao" id="acao" value="Logar" type="hidden"></td>
              </tr>
            </table>
          </form>



<?
    }
    else
    {
        $required=array(
            "IDADM"=>$_POST["username"],
            "PWADM"=>$_POST["password"],
        );

//Obtendo login e senha
$gmAPT =$_POST['username'];
$gmpassAPT = $_POST['password'];
$gmAPT = trim($gmAPT);
$gmpassAPT = trim($gmpassAPT);


if (anti_sql($gmAPT) != 0 || anti_sql($gmpassAPT) != 0) {
echo "<meta HTTP-EQUIV='Refresh' CONTENT='0;URL=index.php'>";
} else {


        if (!empty($gmAPT) && !empty($gmpassAPT))
        {
            $connection = odbc_connect($connection_string, $user, $pass);

            $query = "SELECT * FROM [ADM].[dbo].[loginGM] WHERE [idGM]='$gmAPT' AND [passGM]='$gmpassAPT'";
            $q = odbc_exec($connection, $query);

            if ($q && ($farr = odbc_fetch_array($q)))
            {
                session_register("gmfodoes");

                $_SESSION["gmfodoes"] = true;
                $_SESSION["IDADM"] = $farr["idGM"];
                $_SESSION["NICKGM"] = $farr["nickGM"];
                $_SESSION["permissao"] = $farr["permissao"];

                echo "<table width=440 border=0 cellspacing=0 cellpadding=0>
  <tr>
    <td height=100 align=center><b><font color=#008000>DADOS DE ACESSO CORRETOS! <br> REDIRECIONANDO PARA O PAINEL</font></b></td>
  </tr>
</table>";
            }
            else
            {
                echo "<table width=440 border=0 cellspacing=0 cellpadding=0>
  <tr>
    <td height=100 align=center><b><font color=#FF0000>LOGIN OU SENHA INCORRETO!</font></b></td>
  </tr>
</table>";
            }
        }

        echo "<meta HTTP-EQUIV='Refresh' CONTENT='2;URL=index.php'>";


    }
   }

?> 
        </tr>
      </table>
      <table width="808" border="0" align="center" cellpadding="0" cellspacing="0">
        <tr>
          <td width="665"><img src="imgs/rodape.gif" width="808" height="20"></td>
        </tr>
      </table>
      <br>
</body>
</html>


<?
exit;
}
include_once "index2.php";
ob_end_flush();
?>


