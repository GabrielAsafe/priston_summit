<?if (PT!=1) exit;?>
<?
if($_POST[acao]!="Enviar")
{
?>
            <script language='JavaScript' src='arcade.js'></script>
            <style type="text/css">
<!--
.v {
	color: #F00;
}
.vermelho {
	color: #F00;
}
-->
            </style>
            
            <link href="css/style.css" rel="stylesheet" type="text/css" />
            <form  name="form_cad" method="post" action="index.php?sessadm=banir" onSubmit="return verifica2()">

              <table background="imgs/fundo_textura1.gif" width="719" border="0" align="center" cellpadding="0" cellspacing="0">
              <tr>
                <td width="719"><table width="100%" border="0" align="center" cellpadding="4" cellspacing="2">
                  <tr>
                    <td colspan="2" align="center" bgcolor="#000000"><b><font color="#FFFFFF">Sistema de Puni&ccedil;&otilde;es em Player </font></b></td>
                  </tr>
                  <tr>
<td colspan="2">Informe os dados abaixo e escolha a puni&ccedil;&atilde;o.</td>
</tr>
<tr>
  <td width="35%" align="right"><strong><font color="#000000">ID do Personagem </font></strong></td>
  <td width="65%">                      <input name="idplayer" type="text" id="idplayer" size="20" maxlength="60">                    </td>
                </tr>
<tr>
<td align="right"><strong>Motivo</strong></td>
<td><input name="motivo" type="text" id="motivo" size="40">
  <br></td>
</tr>
<tr>
  <td colspan="2"><b>Exemplos:</b> uso de Hack, uso de bugs do Game, Ofensas contra Player, etc...</td>
  </tr>
<tr>
<td align="right"><strong>A&ccedil;&atilde;o a realizar: </strong></td>
<td><select name="punicao" id="punicao">
<option value="0" selected="selected">Alerta</option>
<option value="1">Banir</option>
<option value="2">Desbanir</option>
</select></td>
</tr>
<tr>
<td align="right"><strong>Data Ban Temp: </strong></td>
<td><select name="diadesban"  class="imput1" id="diadesban">
<option value="<? echo date("d"); ?>" selected="selected">Dia</option>
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
<select name="mesdesban"  class="imput1" id="mesdesban">
<option selected value="<? echo date("m"); ?>">M&ecirc;s</option>
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
<select name="anodesban"  class="imput1" id="anodesban">
<option selected value="<? echo date("Y"); ?>">Ano</option>
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
</select></td>
</tr>
<tr>
<td colspan="2">* Se o Banimento for tempor&aacute;rio informe acima o dia que o player deve ser desbloquiado.</td>
</tr>


                  <tr>
                  <td colspan="2" align="center"><input name="acao" type="submit" class="button6" id="acao" value="Enviar">
<input name="acao" type="hidden" id="acao" value="Enviar"></td>
                  </tr>
                </table></td>
                <tr>
                  <td colspan="2" align="center"><div align="left">
                    <p><strong>Instru&ccedil;&otilde;es de como Utilizar o Sistema de Puni&ccedil;&otilde;es em Players<br>
                    </strong>1 Passo - Digite a ID do Player que vai receber a puni&ccedil;&atilde;o no campo, id do personagem<br> 
                    2 Passo - Digite em Motivo por que o player est&aacute; recebendo esse Alerta/Ban/UnBan<br>
                    <strong>Exemplos</strong>: uso de Hack, uso de bugs do Game, Ofensas contra Player, etc.<br>
                    3 Passo - Em A&ccedil;&atilde;o a realizar, escolha a puni&ccedil;&atilde;o Alerta/Ban/UnBan<br>
                    4 Passo - Em Data ban Temp, coloque o dia que a conta vai ser desbanida automaticamente exemplo se o ban for de sete(7) dias configure o dia/mes/ano para desbloquear daqui 7 dias.<br />
                    5 Passo - Apos preencher todos campos clique em enviar.</p></div></td>
                  </tr>
              </tr>
            </table>
            </form>

<?
           }
        else
           {

if (empty($_POST['idplayer']))
{
echo"<script>alert ('Favor colocar o ID do Player')</script>";
echo"<script>history.go(-1);</script>";
$daerro = "1";
}

if (empty($_POST['motivo']))
{
echo"<script>alert ('Favor colocar o Motivo da Punição')</script>";
echo"<script>history.go(-1);</script>";
$daerro = "1";
}
if ($daerro == "1") {} else {

//Verificando se n&atilde;o vai ter injection no script
$idplayer = $_POST['idplayer'];
$motivo = $_POST['motivo'];
$punicao = $_POST['punicao'];
$diadesban = $_POST['diadesban'];
$mesdesban = $_POST['mesdesban'];
$anodesban = $_POST['anodesban'];

if ($punicao == 0) {
$punicaoNUM = 0;
$punicaoTXT = "Alerta";
}
if ($punicao == 1) {
$punicaoNUM = 1;
$punicaoTXT = "Banido";
}
if ($punicao == 2) {
$punicaoNUM = 0;
$punicaoTXT = "Desbanido";
}


if (anti_sql($idplayer) != 0 || anti_sql($motivo) != 0) {
echo "<center><font face=verdana size=2>Uso de Caracteres não Permitidos!</a>";
} else {

				//Verificando se Existe essa ID
				$connection = odbc_connect( $connection_string, $user, $pass );
                $idplayer=$_POST[idplayer];
                $query_verid = "SELECT * FROM [accountdb].[dbo].[".( strtoupper($idplayer[0]) ) ."GameUser] WHERE [userid]='$idplayer'";

                $q_verid = odbc_exec($connection, $query_verid, $query_verid1);
				$qt_verid = odbc_do($connection, $query_verid, $query_verid1);
                $i = 0;
                while(odbc_fetch_row($qt_verid)) $i++;
                           if($i==0) {
                    echo"NÃO EXISTE A ID <b>$_POST[idplayer]</b> FAVOR CONFERIR A ID NOVAMENTE!";

						   } else {
					$gm = $_SESSION["NICKGM"];
                    //Tabela separada dos players por letras iniciais
                                        $query = "UPDATE [accountdb].[dbo].[".( strtoupper($idplayer[0]) ) ."GameUser] SET [BlockChk]='$punicaoNUM' WHERE [userid]='$idplayer'";
                    $q = odbc_exec($connection, $query);
                                        $query1 = "UPDATE [accountdb].[dbo].[AllGameUser] SET [BlockChk]='$punicaoNUM' WHERE [userid]='$idplayer'";
                    $q = odbc_exec($connection, $query1);
					$diaban = date("d");
					$mesban = date("m");
					$anoban = date("Y");
					$ip = $_SERVER["REMOTE_ADDR"];
					$datahoje = date("m-d-Y h:i:s A");
                    $query2 = "INSERT INTO [ADM].[dbo].[LogsBan] ([idplayer],[motivo],[punicao],[diaban],[mesban],[anoban],[diadesban],[mesdesban],[anodesban],[gm],[ip],[data]) VALUES ('$idplayer','$motivo','$punicaoTXT','$diaban','$mesban','$anoban','$diadesban','$mesdesban','$anodesban','$gm','$ip','$datahoje')";

					$q = odbc_exec($connection, $query);
                    $q2 = odbc_exec($connection, $query2);

            if ($q && $q2)
            	{
?>
<table background='imgs/fundo_textura1.gif' width='734' border='0' align='center' cellpadding='0' cellspacing='0'>
  <tr>
    <td width="734"><table width='99%' border='0' align='center' cellpadding='4' cellspacing='2'>
      <tr>
        <td width='100%' align='center' bgcolor='#000000'><b><font color="#FFFFFF">Puni&ccedil;&otilde;es em Player</font></b></td>
      </tr>
      <tr>
        <td height='100' align='center'><p> ID <strong><? echo $idplayer; ?></strong> recebeu a Puni&ccedil;&atilde;o numero <? echo $punicao; ?></p>
          <p class="vermelho"><strong>Tabela de lista de numeros de Puni&ccedil;&otilde;es!</strong></p>
          <p><strong>Alerta - <span class="v">0</span><BR>
            Banido - <span class="v">1</span><br>
            Desbanido - <span class="v">2</span></strong></p></td>
      </tr>
    </table></td>
  </tr>
</table>
<?
}}}}}
?>



