<? if (Painel!=1) exit;?>
<?
/*----------------------------------------------------------------------
Painel Player Versão 0.4
Desenvolvidor Por: Mak (sirmakloud@gmail.com)
Editado Por: Jiraya (richard.cva21@hotmail.com(
----------------------------------------------------------------------*/

include_once "incluir/configura.php";
include_once "injection.php";

if(anti_sqli($_POST[action]!="changepw"))
{

$username=$_SESSION["ID"];

                    $connection = odbc_connect( $connection_string, $user, $pass );
                    $query = "SELECT * FROM [accountdb].[dbo].[AllPersonalMember]  WHERE [userid]='$username'";
                    $q = odbc_exec($connection, $query);
                    $qt = odbc_do($connection, $query);

                    odbc_fetch_row($qt);
                    $nome=odbc_result($qt,6);
                    $sobrenome=odbc_result($qt,7);
                    $email=odbc_result($qt,12);
					$cidade=odbc_result($qt,16);
					$estado=odbc_result($qt,17);
					$sexo=odbc_result($qt,28);
					$nasc_dia=odbc_result($qt,25);
					$nasc_mes=odbc_result($qt,26);
					$nasc_ano=odbc_result($qt,27);
					$perg=odbc_result($qt,4);
					$resp=odbc_result($qt,5);
                    if(!$email)
   		                 {
					     echo "<meta HTTP-EQUIV='Refresh' CONTENT='0;URL=index.php?sess=index'>";
					     }

?>
            <script language='JavaScript' src="Alfa.js"></script>
            </div>
            <form  name="form_cad" method="post" action="<?=$_SERVER[PHP_SELF]."?".$_SERVER[QUERY_STRING]?>" onSubmit="return verifica2()">
                  <hr>
                    <div class="top-margin" align="center">
                        <h4 class="pbg">Atualizar dados de seu cadastro</h4>

                        <hr class="col-md-5">
                        <h5 class="top-margin">Atualizar senha</h5>
                       <div class="top-margin">
                            <label align="right">Senha atual do jogo<span class="text-danger">*</span></label>
                            <input class=" col-md-6 form-control" name="oldpwd" placeholder="senha" size="20" maxlength="15" type="password">
                          </div>

                        <div class="top-margin">
                          <div>
                            <label>Nova senha <span class="text-danger">*</span></label>
                            <input class="col-md-6 form-control" name="pwd1" placeholder="senha" size="20" maxlength="15" type="password">
                          </div>
                          <div>
                            <label>Confirmar nova senha <span class="text-danger">*</span></label>
                            <input class="col-md-6 form-control" name="pwd2" placeholder="Repita a senha" size="20" maxlength="15" type="password">
                          </div>
                        </div>

                         <div align="center">
                                <button class="btn btn-action top-margin" type="submit">Atualizar senha</button>
                                <input type="hidden" name="action" value="changepw">
                             </div>

                        <hr class="col-md-5">
                        <h5 class="top-margin">Atualizar dados pessoais</h5>
                        <div class="top-margin">
                          <label>Nome <span class="text-danger">*</span></label>
                           <input class="col-md-6 form-control" name="nome" placeholder="Nome" size="20" maxlength="32" value="<? echo $nome; ?>" id="nome" type="text">
                        </div>

                        <div class="top-margin">
                          <label>Sobrenome <span class="text-danger">*</span></label>
                            <input class="col-md-6 form-control" name="sobrenome" placeholder="Sobrenome" size="20" maxlength="32" value="<? echo $sobrenome; ?>" id="sobrenome" type="text">
                        </div>


                        <div class="top-margin">
                          <label>Email <span class="text-danger">*</span></label>
                            <input class="col-md-6 form-control" name="email" size="20" placeholder="Email" maxlength="100" value="<? echo $email; ?>" id="email" type="text">
                        </div>
                        <div class="top-margin">
                          <label>Repita o email <span class="text-danger">*</span></label>
                            <input class="col-md-6 form-control" name="email2" size="20" placeholder="Repita o email" maxlength="100" value="<? echo $email; ?>" id="email2" type="text">
                        </div>

                        <div class="col-md-5" align="center">
                        <div class="row top-margin">
                          <div class="col-sm-4">
                            <label>Data de nascimento <span class="text-danger">*</span></label>
                          </div>
                          <div class="col-sm-8">
                            <td><select name="nasc_dia" class="imput1 form-control">
                              <option selected value="<? echo $nasc_dia; ?>"><? echo $nasc_dia; ?></option>
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
                                <option selected value="<? echo $nasc_mes; ?>"><? echo $nasc_mes; ?></option>
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
                                <option selected value="<? echo $nasc_ano; ?>"><? echo $nasc_ano; ?></option>
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
                                </select></td></div></div></div>

                            <div align="center">
                                <button class="btn btn-action top-margin" type="submit">Atualizar dados</button>
                                <input type="hidden" value="Atualizar dados" class="button">
                                <input type="hidden" name="action" value="changepw">
                             </div>
                             <hr>
                      </div>

            </form>

<?
            }
            else
            {
      
//Verificando se n&atilde;o vai ter injection no script
$senha0 = anti_sqli($_POST['oldpwd']);
$senha1 = anti_sqli($_POST['pwd1']);
$senha2 = anti_sqli($_POST['pwd2']);
$nome = anti_sqli($_POST['nome']);
$sobrenome = anti_sqli($_POST['sobrenome']);
$email = anti_sqli($_POST['email']);

if (anti_sql($chaveregistro) != 0 || anti_sql($senha0) != 0 || anti_sql($senha1) != 0 || anti_sql($senha2) != 0 || anti_sql($nome) != 0 || anti_sql($sobrenome) != 0 || anti_sql($email) != 0) {
echo "<center><font face=verdana size=2> Erro, caracteres não permitidos foram digitados! <br> Nãoo use UNDERLINE ( _ ) no Email ou em outro campo! <br> <a href='index.php'>Voltar e Tentar Novamente</a>";
} else {



                    $required=array(
                    "Senha Antiga"=>$_POST[oldpwd],
                    "Nova Senha"=>$_POST[pwd1],
                    "Repete Nova Senha"=>$_POST[pwd2],
                    "Seu Nome"=>$_POST[nome],
                    "Sobrenome"=>$_POST[sobrenome],
                    "E-mail"=>$_POST[email],
                    "Repete E-mail"=>$_POST[email2],
                    "Dia que nasceu"=>$_POST[nasc_dia],
                    "Mês que nasceu"=>$_POST[nasc_mes],
                    "Ano que nasceu"=>$_POST[nasc_ano],
                );

                for($i=0;$i<count($required);$i++)
                {
                    list($key,$value)=each($required);

                    if(!$value)
                        echo "<b>$key</b>  é obrigatório <br>";
                    else
                        $chkArr[]=true;
                }

                if(count($chkArr)==count($required))
                {

                    if($_POST[pwd1]==$_POST[pwd2])
                    {
                        $connection = odbc_connect( $connection_string, $user, $pass );
						
						$queryrpt = "SELECT * FROM [accountdb].[dbo].[AllPersonalMember] WHERE [userid]='$_SESSION[ID]'";
                        $qrpt = odbc_exec($connection, $queryrpt);

                        $qtrpt = odbc_do($connection, $queryrpt);
                        $irpt = 0;
                        while(odbc_fetch_row($qtrpt)) $irpt++;

                        if($irpt>0)
                        {

                        $usernameP=($_SESSION["ID"]==$_POST[username])?$_SESSION["ID"]:$_POST[username];

                        $query = "SELECT * FROM [accountdb].[dbo].[".( strtoupper($usernameP[0]) ) ."GameUser] WHERE [userid]='$usernameP' AND [Passwd]='$_POST[oldpwd]'";
                        $q = odbc_exec($connection, $query);

                        $qt = odbc_do($connection, $query);
                        $i = 0;
                        while(odbc_fetch_row($qt)) $i++;

                        if($i>0)
                        {

                            if(!$func->is_valid_string($_POST[pwd1]))
                            {

                   	$nome = $_POST[nome];
                    $sobrenome = $_POST[sobrenome];
                    $email = $_POST[email];
                    $nasc_dia = $_POST[nasc_dia];
                    $nasc_mes = $_POST[nasc_mes];
                    $nasc_ano = $_POST[nasc_ano];


                    //Tabela separada dos players por letras iniciais
					$query = "UPDATE [accountdb].[dbo].[".( strtoupper($usernameP[0]) ) ."GameUser] SET [Passwd]='$_POST[pwd1]' WHERE [userid]='$usernameP' AND [Passwd]='$_POST[oldpwd]'";
                    $q = odbc_exec($connection, $query);

					//Tabela All User
                    $query2 = "UPDATE [accountdb].[dbo].[AllGameUser] SET [Passwd]='$_POST[pwd1]' WHERE [userid]='$usernameP' AND [Passwd]='$_POST[oldpwd]'";
					 $q2 = odbc_exec($connection, $query2);
					//Tabela All Peronagem

                    $query3 = "UPDATE [accountdb].[dbo].[AllPersonalMember] SET [Passwd]='$_POST[pwd1]',[CUserName1]='$nome',[CUserName2]='$sobrenome',[Email]='$email',[DiaNasc]='$nasc_dia',[MesNasc]='$nasc_mes',[AnoNasc]='$nasc_ano',[ip]='$ip' WHERE [userid]='$usernameP' AND [Passwd]='$_POST[oldpwd]'";
					$q3 = odbc_exec($connection, $query3);



							     if ($q && $q2 && $q3)
                                {
echo "<table background='imgs/fundo_textura1.gif' width='448' border='0' align='center' cellpadding='0' cellspacing='0'>
  <tr>
    <td><table width='100%' border='0' align='center' cellpadding='4' cellspacing='2'>
      <tr>
        <td width='100%' align='center' bgcolor='#000000'><b><font color='#FFFFFF'>Atualizando Dados de Registro</font></b></td>
      </tr>
      <tr>
        <td height='100' align='center'><strong><font color='Green'>Todos os dados foram atualizados com sucesso!</font></strong> </td>
      </tr>
    </table></td>
  </tr>
</table>";
                                //Apagando arquivo da chave
                                }
                            }
                            else
                            {
                                    echo "FAVOR REMOVER CARACTERES ESPECIAIS E TENTAR NOVAMENTE<br>";
                            }
							
					   	}
                        else
                        {
									echo "<center><strong><font color='Red'>Senha Antiga está incorreta!</font></strong></center><br>";
							}

                     	}
                        else
                        {
                           			 echo "<center><strong><font color='Red'>Senha Antiga está incorreta!</font></strong></center><br>";
                             }
							 
						}
                    	else
                   		{
                       				 echo "NEW PW1 AND NEW PW2 ARE NOT THE SAME CHARACTER<br>";

                    }

                }

                echo "<center><a href=\"$_SERVER[PHP_SELF]?$_SERVER[QUERY_STRING]\">Voltar</a>";
            }

			}

 
?>