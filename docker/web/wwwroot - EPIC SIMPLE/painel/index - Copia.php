<?
ob_start('ob_gzhandler');

session_start();

header('p3p: CP="CAO PSA OUR"');
//$erro=error_reporting(0);
include_once "incluir/configura.php";
include_once "injection.php";

include_once "verifica_tempo.php";



if($_GET['sess']=="logout")
{
 session_destroy();
   /* $_SESSION["charDir"]='';
    $_SESSION["charNum"]='';
    $_SESSION["charID"]='';
    $_SESSION["charName"]='';
    $_SESSION["charLevel"]='';
    $_SESSION["charClass"]='';*/
    header("location: index.php");
}

//Segurança para o PHP

//foreach ($_GET as $key=>$getvar){ $_GET[$key] = mssql_escape($getvar); } 
//foreach ($_POST as $key=>$postvar){ $_POST[$key] = mssql_escape($postvar); } 


function escapestrings($string)
{
    //se magic_quotes não estiver ativado, escapa a string
    if (!get_magic_quotes_gpc())
    {
        return mysql_escape_string($string); // função nativa do php para escapar variáveis.
    }
    else
    {
        // caso contrario
        return $string; // retorna a variável sem necessidade de escapar duas vezes
    }
}



if(!$_SESSION["ID"])
{
?>
<!DOCTYPE HTML PUBLIC "-//W3C//DTD HTML 4.01 Transitional//EN">
<html>
<head>
<style type="text/css">
body
 {
     background-image: url(imgs/background.jpg);
     background-repeat: no-repeat;
	 background-size:130%;
 }
 </style>
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
function click() {
if (event.button==2||event.button==3) {
oncontextmenu='return false';
  }
}
document.onmousedown=click
document.oncontextmenu = new Function("return false;")

</script>
  <meta charset="utf-8">
  <meta name="viewport"    content="width=device-width, initial-scale=1.0">
  <meta name="description" content="">
  <meta name="author"      content="Bruno Adauto Krewer">
  
  <title>Summit Priston Tale</title>

  <link rel="shortcut icon" href="assets/images/gt_favicon.png">
  
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

<body>
  <!-- /.navbar -->
        <div class="col-md-6 col-md-offset-3 col-sm-8 col-sm-offset-2">
          <div class="panel panel-default">
            <div class="panel-body">
			<div class="col-md-6 col-md-offset-3 col-sm-8 col-sm-offset-2">
              
<?
    if($_POST['action']!="Logar")
    {
	require	'gerarrand.php';
	$strRand = gerar_string_randonica();
?>  
            <form method="post" onSubmit="disabledBttn(this)" action="index.php">

            <table width="100%" border="0" align="center" cellpadding="2" cellspacing="2">
              <div class="top-margin">
                  <label>ID: <span class="text-danger">*</span></label>
                  <input name="username" type="username" class="form-control">
                </div>
                    <div class="top-margin">
                  <label>Senha: <span class="text-danger">*</span></label>
                  <input name="password" type="password" class="form-control">
                </div>

                <hr>
              <div class="row">
                  <div class="col-lg-8">
                    <b><a href="recsenha.php">Esqueceu sua senha?</a></b>
                  </div>
                  <div class="col-lg-8">
                    <b><a href="cadtermos.php">Criar nova conta?</a></b>
                  </div>
				  <div class="col-lg-8">
                    <b><a href="http://124.198.128.16/gerenciador/">Painel de Clans</a></b>
                  </div>
                  <div class="col-lg-4 text-right">
                    <button class="btn btn-action" name="action" type="submit">ENTRAR</button>
                  </div>
                </div>
            </table>
            <input type="hidden" name="action" value="Logar">
            </form>
<?
    }
    else
    {
        $required=array(
            "Seu ID"=>$_POST[username],
            "Sua Senha"=>$_POST[password],
        );

//Obtendo login e senha
$usuarioAPT = anti_sqli(escapestrings($_POST['username']));
$senhaAPT = anti_sqli(escapestrings($_POST['password']));
$usuarioAPT = trim($usuarioAPT);
$senhaAPT = trim($senhaAPT);


if (anti_sql($usuarioAPT) != 0 || anti_sql($senhaAPT) != 0 ) {
echo "<meta HTTP-EQUIV='Refresh' CONTENT='0;URL=index.php'>";
} else {


        for($i=0;$i<count($required);$i++)
        {
            list($key,$value)=each($required);

            if(!$value)
                echo "Você esqueceu de informar <b>$key</b>.<br>";
            else
                $chkArr[]=true;
        }

        if(count($chkArr)==count($required))
        {
		
            $connection = odbc_connect( $connection_string, $user, $pass );
			
            $querysenha = "SELECT * FROM [accountdb].[dbo].[AllPersonalMember] WHERE [Userid]='$usuarioAPT'";
            $qsenha = odbc_exec($connection, $querysenha);

            $qtsenha = odbc_do($connection, $querysenha);
            $i2 = 0;
            while(odbc_fetch_row($qtsenha)) $i2++;
                        $email=odbc_result($qtsenha,12);

            if($i2>0)
            {

            $usernameP=anti_sqli($_POST[username]);
            $query = "SELECT * FROM [accountdb].[dbo].[".( strtoupper($usernameP[0]) ) ."GameUser] WHERE [userid]='$usuarioAPT' AND [Passwd]='$senhaAPT'";
            $q = odbc_exec($connection, $query);

            $qt = odbc_do($connection, $query);
            $i = 0;
            while(odbc_fetch_row($qt)) $i++;
                        $email=odbc_result($qt,12);

            if($i>0)
            {

		///////////////////////////////
                session_register("usercode");
                $farr = odbc_fetch_array($q);

                $_SESSION["ID"]=$farr[userid];

      echo "<table width=423 border=0 cellspacing=0 cellpadding=0>
  <tr>
    <td height=100 align=center><b><font color='black'>Carregando...<br><img src='imgs/loading.gif' width='20' height='20' /><br> </b></td>
  </tr>
</table>";
            }
            else
       echo "<table width=423 border=0 cellspacing=0 cellpadding=0>
  <tr>
    <td height=100 align=center><b><font color='black'>ID ou Senha incorretos!<br><img src='imgs/loading.gif' width='20' height='20' /><br></b></td>
  </tr>
</table>";

        } else {
	
	echo "<table width=423 border=0 cellspacing=0 cellpadding=0>
  <tr>
    <td height=100 align=center><b><font color='black'>ID ou Senha incorretos!<br><img src='imgs/loading.gif' width='20' height='20' /><br></b></td>
  </tr>
</table>";
}

        echo "<meta HTTP-EQUIV='Refresh' CONTENT='2;URL=index.php'>";



    }
    //} Verifica pela GD fechamento
    }
	}


?>
</div>
</div>
</div>
</body>
</html>


<?
exit;
}
require "index2.php";
ob_end_flush();
?>