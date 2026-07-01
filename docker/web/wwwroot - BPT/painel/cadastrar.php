<?
ob_start('ob_gzhandler');

session_start();

include_once "incluir/configura.php";
include_once "injection.php";

if (!$_POST['aceitatermos']) 
	{
	echo "<b><div align=\"center\"><img src=\"imgs/alerta.png\"><br><font color=red>Voc&ecirc;  tem que concordar com as regras do servidor<br> para poder fazer uma conta! </div><br><br>";
	echo "<meta HTTP-EQUIV='Refresh' CONTENT='2;URL=cadtermos.php'>";
	}
		else 
			{
?>
<!DOCTYPE html">
<html lang="pt-br">
<head>
  <script language='JavaScript' src="Alfa.js"></script>
  <meta charset="utf-8">
  <meta name="viewport"    content="width=device-width, initial-scale=1.0">
  <meta name="description" content="cadastro Shel Summit Priston Tale">
  <meta name="author"      content="Bruno Adauto Krewer">
  
  <title>Cadastrar - Summit Tale</title>

  <link rel="shortcut icon" href="assets/images/gt_favicon2.png">
  <link rel="stylesheet" media="screen" href="http://fonts.googleapis.com/css?family=Open+Sans:300,400,700">
  <link rel="stylesheet" href="assets/css/bootstrap.min.css">
  <link rel="stylesheet" href="assets/css/font-awesome.min.css">

  <!-- Custom styles for our template -->
  <link rel="stylesheet" href="assets/css/bootstrap-theme.css" media="screen" >
  <link rel="stylesheet" href="assets/css/main.css">

</head>

<body>

        <div class="col-md-6 col-md-offset-3 col-sm-8 col-sm-offset-2">
          <div class="panel panel-default">
            <div class="panel-body">
              <h3 class="thin text-center">Registre uma nova conta</h3>
              <p class="text-center text-muted">Já é cadastrado? <a href="index.php">Efetue Login</a></p>
              <hr>
			            <?
                    if($_POST[action]!="signup")
                    {
                  ?>
                  <form name="form_cad" action="<?=$_SERVER[PHP_SELF]."?".$_SERVER[QUERY_STRING]?>" method="post" enctype="multipart/form-data" onSubmit="disabledBttn(this);return verifica()">
                    


                    <div class="top-margin">
                      <label>ID: <span class="text-danger">*</span></label>
                      <input class="form-control" name="username" placeholder="ID" size="20" maxlength="15" type="text">
                    </div>

                    <div class="row top-margin">
                      <div class="col-sm-6">
                        <label>Senha <span class="text-danger">*</span></label>
                        <input class="form-control" name="password" placeholder="senha" size="20" maxlength="15" type="password">
                      </div>
                      <div class="col-sm-6">
                        <label>Comfirmar senha <span class="text-danger">*</span></label>
                        <input class="form-control" name="password2" placeholder="Repita a senha" size="20" maxlength="15" type="password">
                      </div>
                    </div>


                    <div class="top-margin">
                      <label>Nome <span class="text-danger">*</span></label>
                       <input class="form-control" name="nome" placeholder="Nome" size="20" maxlength="32" id="nome" type="text">
                    </div>

                    <div class="top-margin">
                      <label>Sobrenome <span class="text-danger">*</span></label>
                        <input class="form-control" name="sobrenome" placeholder="Sobrenome" size="20" maxlength="32" id="sobrenome" type="text">
                    </div>


                    <div class="top-margin">
                      <label>Email <span class="text-danger">*</span></label>
                        <input class="form-control" name="email" size="20" placeholder="Email" maxlength="100" id="email" type="text">
                    </div>
                    <div class="top-margin">
                      <label>Repita o email <span class="text-danger">*</span></label>
                        <input class="form-control" name="email2" size="20" placeholder="Repita o email" maxlength="100" id="email2" type="text">
                    </div>

                    <div class="row top-margin">
                      <div class="col-sm-4">
                        <label>Data de nascimento <span class="text-danger">*</span></label>
                      </div>
                      <div class="col-sm-8">
                        <td><select name="nasc_dia" class="imput1 form-control">
                          <option selected="selected" value="">Dia</option>
                          <option value="01">01</option>
                          <option value="02">02</option>
                          <option value="03">03</option>
                          <option value="04">04</option>
                          <option value="05">05</option>
                          <option value="06">06</option>
                          <option value="07">07</option>
                          <option value="08">08</option>
                          <option value="09">09</option>
                          <option value="10">10</option>
                          <option value="11">11</option>
                          <option value="12">12</option>
                          <option value="13">13</option>
                          <option value="14">14</option>
                          <option value="15">15</option>
                          <option value="16">16</option>
                          <option value="17">17</option>
                          <option value="18">18</option>
                          <option value="19">19</option>
                          <option value="20">20</option>
                          <option value="21">21</option>
                          <option value="22">22</option>
                          <option value="23">23</option>
                          <option value="24">24</option>
                          <option value="25">25</option>
                          <option value="26">26</option>
                          <option value="27">27</option>
                          <option value="28">28</option>
                          <option value="29">29</option>
                          <option value="30">30</option>
                          <option value="31">31</option>
                          </select>
                          <select name="nasc_mes" class="imput1 form-control top-margin">
                            <option selected="selected" value="">Mês</option>
                            <option value="01">01</option>
                            <option value="02">02</option>
                            <option value="03">03</option>
                            <option value="04">04</option>
                            <option value="05">05</option>
                            <option value="06">06</option>
                            <option value="07">07</option>
                            <option value="08">08</option>
                            <option value="09">09</option>
                            <option value="10">10</option>
                            <option value="11">11</option>
                            <option value="12">12</option>
                            </select>

                          <select name="nasc_ano" class="imput1 form-control top-margin">
                            <option selected="selected" value="">Ano</option>
              							<option value="2014">2014</option>
              							<option value="2013">2013</option>
              							<option value="2012">2012</option>
              							<option value="2011">2011</option>
              							<option value="2010">2010</option>
              							<option value="2009">2009</option>
                            <option value="2008">2008</option>
                            <option value="2007">2007</option>
                            <option value="2006">2006</option>
                            <option value="2005">2005</option>
                            <option value="2004">2004</option>
                            <option value="2003">2003</option>
                            <option value="2002">2002</option>
                            <option value="2001">2001</option>
                            <option value="2000">2000</option>
                            <option value="1999">1999</option>
                            <option value="1998">1998</option>
                            <option value="1997">1997</option>
                            <option value="1996">1996</option>
                            <option value="1995">1995</option>
                            <option value="1994">1994</option>
                            <option value="1993">1993</option>
                            <option value="1992">1992</option>
                            <option value="1991">1991</option>
                            <option value="1990">1990</option>
                            <option value="1989">1989</option>
                            <option value="1988">1988</option>
                            <option value="1987">1987</option>
                            <option value="1986">1986</option>
                            <option value="1985">1985</option>
                            <option value="1984">1984</option>
                            <option value="1983">1983</option>
                            <option value="1982">1982</option>
                            <option value="1981">1981</option>
                            <option value="1980">1980</option>
                            <option value="1979">1979</option>
                            <option value="1978">1978</option>
                            <option value="1977">1977</option>
                            <option value="1976">1976</option>
                            <option value="1975">1975</option>
                            <option value="1974">1974</option>
                            <option value="1973">1973</option>
                            <option value="1972">1972</option>
                            <option value="1971">1971</option>
                            <option value="1970">1970</option>
							</select></td></div></div>

              <div class="row top-margin">
                  <div class="col-sm-4">
                      <label>Pergunta secreta: <span class="text-danger">*</span></label></div>
                      <div class="col-sm-8">
                      <td colspan="2">
                        <select name="perg" id="perg" class="form-control">
                          <option value="Seu Número Favorito?">Seu Número Favorito?</option>
                          <option value="Nome do seu Melhor Amigo?">Nome do seu Melhor Amigo?</option>
                          <option value="Nome do seu Anímal Favorito?">Nome do seu Animal Favorito?</option>
                        </select>
                      </td></div>
              </div>

              <div class="row top-margin">
                  <div class="col-sm-4">
                      <label>Resposta <span class="text-danger">*</span></label></div>
                      <div class="col-sm-8">
                        <input class="form-control" type="text" name="resp" placeholder="Resposta" size="20" maxlength="50" id="resp">
                      </div>
              </div>

              <div align="center">
                    <button class="btn btn-action top-margin" type="submit">Registrar</button>
                    <input name="hidden" type="hidden" class="button" value="Registrar">
                            <input name="aceitatermos" type="hidden" class="button" id="aceitatermos" value="<? echo $_POST['aceitatermos']; ?>">
              </div>
                    <input type="hidden" name="action" value="signup">
                  </form>
                </div>
              </div>
                <?
    }
    else
    {

//Verificando se n&atilde;o vai ter injection no script

$login = anti_sqli(trim($_POST['username']));
$senha = anti_sqli(trim($_POST['password']));
$nome = anti_sqli(trim($_POST['nome']));
$sobrenome = anti_sqli(trim($_POST['sobrenome']));
$email = anti_sqli(trim($_POST['email']));
$nasc_dia = anti_sqli(trim($_POST['nasc_dia']));
$nasc_mes = anti_sqli(trim($_POST['nasc_mes']));
$nasc_ano = anti_sqli(trim($_POST['nasc_ano']));
$resp = anti_sqli(trim($_POST['resp']));

if (anti_sql($login) != 0 || anti_sql($senha) != 0 || anti_sql($nome) != 0 || anti_sql($sobrenome) != 0 || anti_sql($email) != 0 || anti_sql($resp) != 0) {
echo "<center><font face=verdana size=2> Erro, caracteres n&atilde;o permitidos foram digitados! <br> N&atilde;o use UNDERLINE ( _ ) no Email ou em outro campo! <br> <a href='cadtermos.php'>Voltar e Tentar Novamente</a>";
} else {

        $required=array(
            "Login"=>$_POST[username],
            "Senha do Jogo"=>$_POST[password],
			"E-mail"=>$_POST[email],
			"Nome"=>$_POST[nome],
			"Sobrenome"=>$_POST[sobrenome],
			"Dia do Nascimento"=>$_POST[nasc_dia],
			"Mes do Nascimento"=>$_POST[nasc_mes],
			"Ano do Nascimento"=>$_POST[nasc_ano],
			"Pergunta Secreta"=>$_POST[resp],
        );



        for($i=0;$i<count($required);$i++)
        {
            list($key,$value)=each($required);

            if(!$value)
                echo "<b>$key</b> é obrigatorio<br>";
            else
                $chkArr[]=true;
        }

        if(count($chkArr)==count($required))
        {

            $connection = odbc_connect( $connection_string, $user, $pass );

            if(!$func->is_valid_string($_POST[username]) && !$func->is_valid_string($_POST[password]))
            {

                $usernameP=anti_sqli($_POST[username]);
                $query = "SELECT * FROM [accountdb].[dbo].[".( strtoupper($usernameP[0]) ) ."GameUser] WHERE [userid]='$usernameP'";
                $q = odbc_exec($connection, $query);

                $qt = odbc_do($connection, $query);
                $i = 0;
                while(odbc_fetch_row($qt)) $i++;

                if($i>0)
                    echo"O Login <b>$_POST[username]</b> J&Aacute; EXISTE ESSA ID REGISTRADA!";
                else
                {
				$ipnob = $_SERVER['REMOTE_ADDR'];

$ini = 0;
$executando_email = odbc_do($connection,$tabela_ip) or die (odbc_error());
if($ini>0){
echo '<table width="100%" border="0" cellpadding="6" cellspacing="0"><tbody><tr><td><div align="justify"><img src="imgs/alerta.png" width="37" align="left" height="36"><font arial,="" helvetica,="" sans-serif="" face="Verdana," size="2">Olá <strong><font color="#990000">'.$nome.'</font></strong>
você já se registro em nosso Banco de Dados hoje, tente usar este sistema mais tarde.<br><br>Atenciosamente Equipe '.$nomedoserver.' </font></div></td></tr></tbody></table>';
}else{
	
	

$email = anti_sqli($_POST['email']);

$tabela_email = "SELECT * FROM [accountdb].[dbo].[ALLPersonalMember] WHERE [email]='$email'";
$ini = 0;
$executando_email = odbc_do($connection,$tabela_email) or die (odbc_error());
while(odbc_fetch_row($executando_email)) $ini++;
if($ini>0){
echo '<table width="100%" border="0" cellpadding="6" cellspacing="0"><tbody><tr><td><div align="justify"><img src="imgs/alerta.png" width="37" align="left" height="36"><font arial,="" helvetica,="" sans-serif="" face="Verdana," size="2">J&aacute; existe o email <strong><font color="#990000">'.$email.'</font></strong>
registrado em nosso Banco de Dados, favor informar outro diferente,
pois n&atilde;o pode-se utilizar o mesmo e-mail para v&aacute;rias contas. </font></div></td></tr></tbody></table>';
}else{



                    $datahoje = date("d-m-Y H:i:s");

                    $query = "INSERT INTO [accountdb].[dbo].[".( strtoupper($usernameP[0]) ) ."GameUser] ([userid],[Passwd],[GPCode],[RegistDay],[DisuseDay],[inuse],[Grade],[EventChk],[SelectChk],[BlockChk],[SpecialChk],[Credit],[DelChk],[Channel]) values('" . $_POST[username] . "','" . $_POST[password] . "','PTP-RUD001','$datahoje','12-12-2020','0','U','0','0','0','0','0','0','" . $_SERVER['REMOTE_IP'] . "')";
                    $query2 = "INSERT INTO [accountdb].[dbo].[AllGameUser] ([userid],[Passwd],[GPCode],[RegistDay],[DisuseDay],[inuse],[Grade],[EventChk],[SelectChk],[BlockChk],[SpecialChk],[Credit],[DelChk],[Channel]) values('" . $_POST[username] . "','" . $_POST[password] . "','PTP-RUD001','$datahoje','12-12-2020','0','U','0','0','0','0','0','0','" . $_SERVER['REMOTE_IP'] . "')";

                   //Cadastrando Dados do user
                    $login = stripslashes($_POST['username']);
                    $pw = stripslashes($_POST['pw']);
                    $email = stripslashes($_POST['email']);
					$perg = stripslashes($_POST['perg']);
                    $resp = stripslashes($_POST['resp']);
                    $dn = stripslashes($_POST['dn']);
                    $nome = stripslashes($_POST['nome']);
                    $sobrenome = stripslashes($_POST['sobrenome']);
                    $sexo = stripslashes($_POST['sexo']);

                    $ip = $_SERVER["REMOTE_ADDR"];
                    $query3 = "INSERT INTO [accountdb].[dbo].[AllPersonalMember] ([PMNo],[Userid],[Passwd],[Question],[Reply],[CUserName1],[CUserName2],[Age],[Email],[Add1],[Add2],[Add3],[RegistDay],[DiaNasc],[MesNasc],[AnoNasc],[sexo],[ip],[senhapainel],[pais]) VALUES ('0','" . $_POST[username] . "','" . $_POST[password] . "','$perg','$resp','$nome','$sobrenome','$dn','$email','$cidade','$estado','$amigo','$datahoje','$nasc_dia','$nasc_mes','$nasc_ano','$sexo','$ip','$senhapainel','$pais')";
                    $query4 = "INSERT INTO [accountdb].[dbo].[".( strtoupper($usernameP[0]) ) ."PersonalMember] ([PMNo],[Userid],[Passwd],[CUserName1],[CUserName2],[Email],[Add1],[Add2],[Add3],[RegistDay]) VALUES ('0','" . $_POST[username] . "','" . $_POST[password] . "','$nome','$sobrenome','$email','$cidade','$estado','$amigo','$datahoje')";

                    $q = odbc_exec($connection, $query);
                    $q2 = odbc_exec($connection, $query2);
                    $q3 = odbc_exec($connection, $query3);
                    $q4 = odbc_exec($connection, $query4);
                    if ($q && $q2 && $q3 && $q4)


echo"<table width='100%' border='0' align='center' cellpadding='4' cellspacing='2'>
  <tr>
    <td colspan='2' align='center'><font color='#000000' size='2' face='Verdana, Arial, Helvetica, sans-serif'><strong>Dados Cadastrados com sucesso!</strong></font></td>
  </tr>
  <tr>
    <td width='37%'><font color='#000000' size='2' face='Verdana, Arial, Helvetica, sans-serif'><strong><img src='imgs/setinha.gif' width='11' height='11'> ID:</strong></font></td>
    <td width='63%'><strong><font color='Green' size='2' face='Verdana, Arial, Helvetica, sans-serif'>
      $login
    </font></strong></td>
  </tr>
  <tr>
    <td><font color='#000000' size='2' face='Verdana, Arial, Helvetica, sans-serif'><strong><img src='imgs/setinha.gif' width='11' height='11'> Nome Completo:</strong></font></td>
    <td><strong><font color='Green' size='2' face='Verdana, Arial, Helvetica, sans-serif'>
      $nome $sobrenome
    </font></strong></td>
  </tr>
  <tr>
    <td><font color='#000000' size='2' face='Verdana, Arial, Helvetica, sans-serif'><strong><img src='imgs/setinha.gif' width='11' height='11'> E-mail:</strong></font></td>
    <td><strong><font color='Green' size='2' face='Verdana, Arial, Helvetica, sans-serif'>
      $email
    </font></strong></td>
  </tr>
  <tr>
    <td><font color='#000000' size='2' face='Verdana, Arial, Helvetica, sans-serif'><strong><img src='imgs/setinha.gif' width='11' height='11'> Data Nascimento:</strong></font></td>
    <td><strong><font color='Green' size='2' face='Verdana, Arial, Helvetica, sans-serif'>$nasc_dia/$nasc_mes/$nasc_ano</font></strong></td>
  </tr>
  </table>
";

              } }
 		}
            }else{
		echo '<table width="100%" border="0" cellpadding="6" cellspacing="0"><tbody><tr><td><div align="justify"><img src="imgs/alerta.png" width="37" align="left" height="36"><font arial,="" helvetica,="" sans-serif="" face="Verdana," size="2"><span class="pretto">O Codigo de Ativa&ccedil;&atilde;o do e-mail <strong><font color="#990000">'.$email.'</font></strong><span class="pretto">
informado est&aacute; incorreto!<br>Verifique em seu e-mail a chave correta. </font></div></td></tr></tbody></table>';
		}
		}
			else
                echo"POR FAVOR ENTRE NOVAMENTE COM ID E SENHA SEM UTILIZAR CARACTERES ESPECIAIS!";

        }
       echo "<meta HTTP-EQUIV='Refresh' CONTENT='5;URL=index.php'>";
    } }

?>
              </td>
            </tr>
          </table>
		  </td>
        </tr>
      </table>
      </td>
  </tr>
</table>



</body>
</html>
<?
exit;
ob_end_flush();
?>