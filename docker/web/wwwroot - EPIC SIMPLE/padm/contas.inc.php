<?if (PT!=1) exit;?>
<?php
if($_POST[acao]!="contas")
{
?>
<link href="css/style.css" rel="stylesheet" type="text/css" />

 <form method="post" onSubmit="disabledBttn(this)" action="<?=$_SERVER[PHP_SELF]."?".$_SERVER[QUERY_STRING]?>">
   <div align="center"><br>
     <strong><font color="#000000">ID do Player </font></strong>
     <input type="text" name="id2" />
     <input name="submit" type="submit" class="button6" value="Procurar" />
     <input name="acao" type="hidden" id="acao" value="contas">	
   </div>
 </form>
 <?php
}

include_once "incluir/configura.php";
include_once "injection.php";

if($_POST[action]!="contas")
{

$username = $_POST['id2'];

                    $connection = odbc_connect( $connection_string, $user, $pass );
					$query = "SELECT * FROM [accountdb].[dbo].[AllPersonalMember]  WHERE [userid]='$username'";
                    $q = odbc_exec($connection, $query);
                    $qt = odbc_do($connection, $query);

                    odbc_fetch_row($qt);
                    $nome=odbc_result($qt,6);
                    $sobrenome=odbc_result($qt,7);
					$oldpwd=odbc_result($qt,3);
                    $email=odbc_result($qt,12);
					$nasc_dia=odbc_result($qt,25);
					$nasc_mes=odbc_result($qt,26);
					$nasc_ano=odbc_result($qt,27);
					$perg=odbc_result($qt,4);
					$resp=odbc_result($qt,5);
?>
 <form  name="form_cad" method="post" action="<?=$_SERVER[PHP_SELF]."?".$_SERVER[QUERY_STRING]?>" onSubmit="return verificaCP()">
           
            <table background="imgs/fundo_textura1.gif" width="636" border="0" align="center" cellpadding="0" cellspacing="0">
              <tr>
                <td width="636"><table width="94%" border="0" align="center" cellpadding="4" cellspacing="2">
                  <tr>
                    <td colspan="2" align="center" bgcolor="#000000"><b><font color="#FFFFFF"> Administração de Contas</font></b></td>
                  </tr>
                  <tr>
                    <td width="37%" align="right"><b>ID: <input name="idhidden" type="hidden" value="<? echo $username; ?>"> </b></td>
                    <td width="63%"><input name="nome" type="text" id="nome" disabled value="<? echo $username; ?>" size="20" maxlength="32"></td>
                  </tr>
                  <tr>
                    <td align="right"><b>Senha Jogo:</b></td>
                    <td><input name="oldpwd" type="text" id="oldpwd" value="<? echo $oldpwd; ?>" size="20" maxlength="100">  </td>
                  </tr>
                  <tr>
                    <td align="right"><font color="#000000"><strong>Nome:</strong></font></td>
                    <td>                      <input name="nome" type="text" id="nome" value="<? echo $nome; ?>" size="20" maxlength="32">                    </td>
                  </tr>
                  <tr>
                    <td align="right"><font color="#000000"><strong>Sobrenome:</strong></font></td>
                    <td>                      <input name="sobrenome" type="text" id="sobrenome" value="<? echo $sobrenome; ?>" size="20" maxlength="32">                    </td>
                  </tr>
                 <tr>
                    <td align="right"><font color="#000000"><strong>E-mail:</strong></font></td>
                    <td>                      <input name="email" type="text" id="email" value="<? echo $email; ?>" size="20" maxlength="100">                    </td>
                  </tr>
                  <tr>
                    <td align="right"><font color="#000000"><strong>Data de nascimento:</strong></font></td>
                    <td>                      <select name="nasc_dia"  class="imput1">
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
                      /
                      <select name="nasc_mes"  class="imput1">
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
                      /
                      <select name="nasc_ano"  class="imput1">
                        <option selected value="<? echo $nasc_ano; ?>"><? echo $nasc_ano; ?></option>
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
                        <option value="1969">1969</option>
                        <option value="1968">1968</option>
                        <option value="1967">1967</option>
                        <option value="1966">1966</option>
                        <option value="1965">1965</option>
                        <option value="1964">1964</option>
                        <option value="1963">1963</option>
                        <option value="1962">1962</option>
                        <option value="1961">1961</option>
                        <option value="1960">1960</option>
                        <option value="1959">1959</option>
                        <option value="1958">1958</option>
                        <option value="1957">1957</option>
                        <option value="1956">1956</option>
                        <option value="1955">1955</option>
                        <option value="1954">1954</option>
                        <option value="1953">1953</option>
                        <option value="1952">1952</option>
                        <option value="1951">1951</option>
                        <option value="1950">1950</option>
                        <option value="1949">1949</option>
                        <option value="1948">1948</option>
                        <option value="1947">1947</option>
                        <option value="1946">1946</option>
                        <option value="1945">1945</option>
                        <option value="1944">1944</option>
                        <option value="1943">1943</option>
                        <option value="1942">1942</option>
                        <option value="1941">1941</option>
                        <option value="1940">1940</option>
                        <option value="1939">1939</option>
                        <option value="1938">1938</option>
                        <option value="1937">1937</option>
                        <option value="1936">1936</option>
                        <option value="1935">1935</option>
                        <option value="1934">1934</option>
                        <option value="1933">1933</option>
                        <option value="1932">1932</option>
                        <option value="1931">1931</option>
                        <option value="1930">1930</option>
                        <option value="1929">1929</option>
                        <option value="1928">1928</option>
                        <option value="1927">1927</option>
                        <option value="1926">1926</option>
                        <option value="1925">1925</option>
                        <option value="1924">1924</option>
                        <option value="1923">1923</option>
                        <option value="1922">1922</option>
                        <option value="1921">1921</option>
                        <option value="1920">1920</option>
                        <option value="1919">1919</option>
                        <option value="1918">1918</option>
                        <option value="1917">1917</option>
                        <option value="1916">1916</option>
                        <option value="1915">1915</option>
                        <option value="1914">1914</option>
                        <option value="1913">1913</option>
                        <option value="1912">1912</option>
                        <option value="1911">1911</option>
                        <option value="1910">1910</option>
                        <option value="1909">1909</option>
                        <option value="1908">1908</option>
                        <option value="1907">1907</option>
                        <option value="1906">1906</option>
                        <option value="1905">1905</option>
                        <option value="1904">1904</option>
                        <option value="1903">1903</option>
                        <option value="1902">1902</option>
                        <option value="1901">1901</option>
                        <option value="1900">1900</option>
                        <option value="1899">1899</option>
                      </select> </td>
                  </tr>
                  <tr>
                    <td align="right"><font color="#000000"><strong>Pergunta Secreta:</strong></font></td>
                    <td><select name="perg"  class="imput1">
                        <option selected value="<? echo $perg; ?>"><? echo $perg; ?></option>
                            < <option value="Local de nascimento da sua mãe">Local de nascimento da sua mãe</option>
                            <option value="Melhor amigo de infância">Melhor amigo de infância</option>
                            <option value="Nome do primeiro animal doméstico">Nome do primeiro animal doméstico</option>
                            
                            <option value="Professor favorito">Professor favorito</option>
                             <option value="Professor favorito">Personagem histórico favorito</option>
                            <option value="Qual meu maior Idolo">Qual meu maior Idolo</option>
                            <option value="Nome do meu primeiro amor">Nome do meu primeiro amor</option>
                          </select>  </td>
                  </tr>
                  <tr>
                    <td align="right"><font color="#000000"><strong>Pergunta Secreta (R):</strong></font></td>
                    <td><input name="resp" type="text" id="resp" value="<? echo $resp; ?>" size="20" maxlength="100"></td>
                  </tr>

  <tr>
                    <td align="right"></td>
                    <td><input type="submit" class="button6" value="Atualizar dados!" onclick="alert('Aguarde enquanto os dados estão sendo atualizados');"></td>
                  </tr>
                  <tr>
                    <td colspan="2" align="center"><input type="hidden" name="action"  value="contas" />
</td>
                  </tr>
                </table></td>
              </tr>
            </table>
</form>

 <p>
   <?
            }
            else
            {


					//Verificando se n&atilde;o vai ter injection no script

                   	$nome = $_POST[nome];
                    $sobrenome = $_POST[sobrenome];
                    $email = $_POST[email];
                    $nasc_dia = $_POST[nasc_dia];
                    $nasc_mes = $_POST[nasc_mes];
                    $nasc_ano = $_POST[nasc_ano];
					$resp = $_POST[resp];
					$senha = $_POST[oldpwd];
		    $usernameP = $_POST["idhidden"];
			$ip=$_SERVER["REMOTE_ADDR"];
		$perg=$_POST["perg"];

if ( anti_sql($senha) != 0 || anti_sql($nome) != 0 || anti_sql($sobrenome) != 0 || anti_sql($email) != 0 || anti_sql($cidade) != 0 || anti_sql($resp) != 0) {
echo "<center><font face=verdana size=2> Erro, caracteres não permitidos foram digitados! <br> Nãoo use UNDERLINE ( _ ) no Email ou em outro campo! <br> <a href='cadastrar.php'>Voltar e Tentar Novamente</a>";
} else {
		$connection = odbc_connect($connection_string, $user, $pass);

		    $query = "UPDATE [accountdb].[dbo].[".( strtoupper($usernameP[0]) ) ."GameUser] SET [Passwd]='$senha' WHERE [userid]='$usernameP'";
                    
		    $q = odbc_exec($connection, $query);

                    $query2 = "UPDATE [accountdb].[dbo].[AllGameUser] SET [Passwd]='$senha' WHERE [userid]='$usernameP'";
					 $q2 = odbc_exec($connection, $query2);

                    $query3 = "UPDATE [accountdb].[dbo].[AllPersonalMember] SET [Passwd]='$senha',[CUserName1]='$nome',[CUserName2]='$sobrenome',[Email]='$email',[DiaNasc]='$nasc_dia',[MesNasc]='$nasc_mes',[AnoNasc]='$nasc_ano',[ip]='$ip',[reply]='$resp' WHERE [userid]='$usernameP'";
					$q3 = odbc_exec($connection, $query3);
		if($q && $q2 && $q3){
			echo '<script>alert("Dados modificados com sucesso!");</script>';
		}else{
			echo '<script>alert("Erro ao Modificar os dados!");history.go(-1);</script>';
		}


	}
}
?>
 </p>
 