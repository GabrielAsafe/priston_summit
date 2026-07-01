<?if (Painel!=1) exit;?>
<?
include_once "injection.php";
$username=$_SESSION["ID"];
$pasta = $func->numDir($qCharID);
$error = 0;
$arquivo_cl = $rootDir."DataServer/warehouse/$pasta/".$username."_backup.war";
					if(file_exists($arquivo_cl)){
					$error++;	
					}
if($error != 0){
	?>
    <script>
	function red(){
		window.location.href='index.php?sess=grava';
	}
	</script>
    <table width="423" border="0" align="center" cellpadding="6" cellspacing="0">
      <tr>
        <td align="center" bgcolor="#996633"><font color="#FFFFFF" size="2" face="Verdana, Arial, Helvetica, sans-serif"><b>Ba&uacute; Limpeza Aberto</b></font></td>
      </tr>
      <tr>
        <td align="center"><p align="justify"><b> Desculpe, mas nosso sistema verificou que seu ba&uacute; est&aacute; aberto, sendo assim, voc&ecirc; n&atilde;o poder&aacute; trocar seu ba&uacute;.</b></p>
          <p align="justify"><b><img src="../imgs/alerta.gif" align="left" width="37" height="36" /></b><strong> <font color="#990000">O Ba&uacute; de Limpeza est&aacute; ABERTO, at&eacute; que o sistema dele seja completado seus Ba&uacute;s comuns estar&atilde;o bloqueados para uso. </font></strong></p>
          <p align="justify"><b>Para Fechar (Cancelar) o Bau limpeza clique no bot&atilde;o abaixo. 
          </b></p>
    <label>
    <?
    $_SESSION['passou'] = "YES";
	?>
      <input type="button" name="button" id="button" value="Fechar Meu Ba&uacute; limpeza" onclick="red()"/>
    </label></td>
      </tr>
    </table>
    <?
}else{
//Verificando se os baua existem
$arquivo_bau_char = $rootDir."DataServer/warehouse/$pasta/$username.war";
$arquivo_bau_char2 = $rootDir."DataServer/warehouse/$pasta/".$username."2.war";
$arquivo_bau_charXD = $rootDir."DataServer/warehouse/$pasta/".$username."XD.war";
$arquivo_ladobau_char = "bau/lado/".$username.".side";
//Arquivo Padrão do lado do Bau
$arquivo_lado_padrao = "bau/lado.side";

//Bau Padrão limpo
$arquivo_bau_clean = "bau/baulimpo.war";


//Verificando se o bau padrão existe
if (file_exists($arquivo_bau_char)) {
} else {
//Criando Bau 1 para o player
copy($arquivo_bau_clean, $arquivo_bau_char);
}
//Verificando se Segundo Bau existe
if (file_exists($arquivo_bau_char2)) {
} else {
//Criando Bau 2 para o player
copy($arquivo_bau_clean, $arquivo_bau_char2);
}

//Verificando se o arquivo do lado dos baus existe
if (file_exists($arquivo_ladobau_char)) {
} else {
//Criando Bau 1 para o player
copy($arquivo_lado_padrao, $arquivo_ladobau_char);
}

//Pegando o lado que o bau esta\\
//Abre o arquivo em modo somente leitura
$abre = fopen($arquivo_ladobau_char, "r");
//Lê o arquivo
$lado_bau = fread($abre, filesize($arquivo_ladobau_char));
//Fecha o arquivo
fclose($abre);

					//Verificando se a ID do player esta logada
                    $connection = odbc_connect( $connection_string, $user, $pass );
                    $query_Bau = "SELECT * FROM [ClanDb].[dbo].[CT] WHERE [UserID]='$username' ";
                    $q_Bau = odbc_exec($connection, $query_Bau);
                    $qt_Bau = odbc_do($connection, $query_Bau);
					
					$i = 0;
					while(odbc_fetch_row($qt_Bau)) $i++;
					if($i>0)
					{
					  $BlockBau++;
					}


if(anti_sqli($_POST[acao]!="Muda" AND !$BlockBau))
{

?>

<table background="imgs/fundo_textura1.gif" width="423" border="0" align="center" cellpadding="0" cellspacing="0">
              <tr>
                <td><table width="100%" border="0" align="center" cellpadding="6" cellspacing="2">
                  <tr>
                    <td align="center" bgcolor="#333333"><font color="#FFFFFF" size="2" face="Verdana, Arial, Helvetica, sans-serif"><b>Gerenciamento de Ba&uacute;s da ACC</b></font></td>
                  </tr>
                  <tr>
                    <td align="center"><p align="justify"><b><img src="imgs/setinha.gif" width="11" height="11"><font color="#990000"> Agora você possui dois baús, você só pode trocar de baú por aqui, basta clicar na imagem que está abaixo.</font></b></p>
<?
if (!$BlockBau) {

?>
                      <table width="100%" border="0" cellspacing="0" cellpadding="0">
                        <tr>
                          <td align="center"><form method="post" action="<?="?".$_SERVER[QUERY_STRING]?>" onSubmit="disabledBttn(this)">

<?
if ($lado_bau == 1) {
?>
<input type="image" name="bau" src="imgs/bau_l1.gif" border="0">
<?
} else {
?>
<input type="image" name="bau" src="imgs/bau_l2.gif" border="0">
<? } ?>
                            <input name="acao" type="hidden" id="acao" value="Muda">
                          </form>
                          </td>
                        </tr>
                      </table>
<?
}
?>


                    </td>
                  </tr>

                </table></td>
              </tr>
</table>
<?
}
if(anti_sqli($_POST[acao] == "Muda" AND !$BlockBau)) {

//Renomeando bau1 para bauXD
rename($arquivo_bau_char, $arquivo_bau_charXD);
//Renomeando Bau 2 para bau1
rename($arquivo_bau_char2, $arquivo_bau_char);
//Renomeando BauXD para Bau2
rename($arquivo_bau_charXD, $arquivo_bau_char2);


//Abre o arquivo
$abre = fopen($arquivo_ladobau_char, "w");
//Soma + 1 ao valor do contador
if ($lado_bau == 1) {
$lado_bau = $lado_bau + 1;
}
elseif ($lado_bau == 2) {
$lado_bau = $lado_bau - 1;
}
//Escreve no arquivo o valor total
$salva = fwrite($abre, $lado_bau);
//Fecha o arquivo
fclose($abre);
?>

           <table background="imgs/fundo_textura1.gif" width="423" border="0" align="center" cellpadding="0" cellspacing="0">
             <tr>
               <td><table width="423" border="0" align="center" cellpadding="6" cellspacing="0">
                   <tr>
                     <td align="center" bgcolor="#996633"><font color="#FFFFFF" size="2" face="Verdana, Arial, Helvetica, sans-serif"><b>Gerenciamento de Ba&uacute;s da ACC</b></font></td>
                   </tr>
                   <tr>
                     <td align="center"><p align="justify"><b>Isso mesmo voc&ecirc; agora
                           possui dois ba&uacute;s em sua ACC, sistema in&eacute;dito.</b></p>
                         <p align="justify"><b><img src="imgs/setinha.gif" alt="" width="11" height="11"> Clique na imagem abaixo para Voltar para a Troca de Ba&uacute;s. </b></p>

                         <table width="423" border="0" cellspacing="0" cellpadding="0">
                        <tr>
                          <td align="center"><form method="post" action="<?="?".$_SERVER[QUERY_STRING]?>" onSubmit="disabledBttn(this)">
<input type="image" name="bau" src="imgs/bau_voltar.gif" border="0">
                            <input name="acao" type="hidden" id="acao" value="Voltar">
                          </form>                          </td>
                        </tr>
                        <tr>
                          <td align="center"><b><font color="#006600">BA&Uacute; ALTERADO COM SUCESSO!</font></b></td>
                        </tr>
                      </table>

                     </td>
                   </tr>
               </table></td>
             </tr>
           </table>
<?
}
if(anti_sqli($_POST[acao] != "Muda" AND $BlockBau)) {
?>
<table background="imgs/fundo_textura1.gif" width="448" border="0" align="center" cellpadding="0" cellspacing="0">
  <tr>
    <td><table width="100%" border="0" align="center" cellpadding="6" cellspacing="2">
      <tr>
        <td align="center" bgcolor="#996633"><font color="#FFFFFF" size="2" face="Verdana, Arial, Helvetica, sans-serif"><b>Gerenciamento de Ba&uacute;s da ACC </b></font></td>
      </tr>
      <tr>
        <td align="center"><p align="justify"><b>Isso mesmo voc&ecirc; agora
              possui dois ba&uacute;s em sua ACC, sistema in&eacute;dito.</b></p>
              <p align="justify"><b><font color="#FF0000"><img src="imgs/alerta.gif" width="37" height="36" hspace="5" vspace="5" align="left"></font><font color="#FF0000">Voc&ecirc; est&aacute; logado n&atilde;o pode Trocar o Ba&uacute;. Se voc&ecirc; j&aacute; saiu neste instante aguarde s&oacute; mais um pouco ( 1 a 2 minutos) e clique em Voltar abaixo. </font></b></p>
    <table width="100%" border="0" cellspacing="0" cellpadding="0">
                <tr>
                  <td align="center"><form method="post" action="<?="?".$_SERVER[QUERY_STRING]?>" onSubmit="disabledBttn(this)">
                  <img src="imgs/bau.gif" width="341" height="250">                  <br>
                      <input name="acao" type="submit" id="acao" value="Voltar">
                      </form></td>
                </tr>
            </table></td>
      </tr>
    </table></td>
  </tr>
</table>
<?
}
}
?>