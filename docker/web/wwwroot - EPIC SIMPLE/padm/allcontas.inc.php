<? if (PT!=1) exit;?>

<!DOCTYPE HTML PUBLIC "-//W3C//DTD XHTML 1.0 Transitional//EN" "http://www.w3.org/TR/xhtml1/DTD/xhtml1-transitional.dtd">
<html xmlns="http://www.w3.org/1999/xhtml"><head><style type="text/css">#adBegunavSZ7Vg {z-index:10;position:absolute;top:0;left:0;padding:1px 2px 2px 0px;}.cAdBavSZ7Vg {position:absolute;top:0px;left:0px;width:100%} div > div.cAdBavSZ7Vg {position:fixed;}#adTextavSZ7Vg {z-index:9;position:absolute;top:0;left:0;width:100%;line-height:22px;background:#FFCC00;border-bottom: 1px outset;}#adCloseavSZ7Vg {position:absolute;top:0;right:0;z-index:10;background:#FFCC00;line-height:22px;padding:3px 4px 3px 4px;border-bottom: 1px outset;}</style><!--[if gte IE 5.5]><![if lt IE 8]><style type="text/css">div#aBarVavSZ7Vg {width:expression(((ignoreMe=(document.documentElement.clientWidth?document.documentElement.clientWidth:document.body.clientWidth))<200?200:ignoreMe)+'px');position:absolute;top:expression((ignoreMe2=document.documentElement.scrollTop?document.documentElement.scrollTop:document.body.scrollTop)+'px');left:expression((ignoreMe=document.documentElement.scrollLeft?document.documentElement.scrollLeft:document.body.scrollLeft)+'px');}</style><![endif]><![endif]--><meta http-equiv="Content-Type" content="text/html; charset=ISO-8859-1"><title>Painel ADM</title><link type="text/css" rel="StyleSheet" href="imgs/896.css"><style type="text/css">
.uLogBlock {margin:0;padding:2px;clear:both;}.uLogDescr,.uLogRem {float:left;}.uLogField,.uLogSbm {float:right;}.style15 {color: #000000; font-weight: bold; }
.tbl {border:solid;
color:#9933FF;
background:#000000;
cursor:auto;
}
</style>

     <table width="100%" border="0" cellpadding="3" cellspacing="2">
    <tr>
      <td colspan="2" class="textoPPP" bgcolor="#cccccc"><div align="left"><strong><center> 
        <div align="center"><strong>Todas Contas Cadastradas no Servidor</strong></div>
      </div></td>
    </tr></table>
      <table width="100%" border="0" cellpadding="0" cellspacing="5"><tbody><tr>
    <td height="67" valign="top"><div align="center">
      <p>
        <? 
	  $conn = odbc_connect($connection_string,$user,$pass) or die ("Erro Ao Conectar");
	  $tabela = "SELECT Userid,Passwd,senhapainel FROM dbo.AllPersonalMember";
	  $executa = odbc_exec($conn,$tabela);
	  ?>
      </p>
 
      <table width="380" border="0" cellpadding="0" cellspacing="0">
        <tr>
          <td width="355" background="imgs/16.gif"><table width="108%" border="0" cellspacing="0" cellpadding="0">
              <tr>
                <td width="14%" height="25" align="left"><div align="center"><span class="style15"><font size="2" face="Tahoma">Posi&ccedil;&atilde;o</font></span></div></td>
                <td width="33%" height="25" align="left"><div align="center"><span class="style15"><font size="2" face="Tahoma">Usu&aacute;rio</font></span></div></td>
                <td width="28%" height="25" align="left"><div align="center"><span class="style15"><font size="2" face="Tahoma">Senha JOGO</font></span></div></td>
                <td width="24%" height="29" align="left"><div align="center"><span class="style15"><font size="2" face="Tahoma">Senha PAINEL</font></span> </div></td>
                <td width="1%" align="left">&nbsp;</td>
              </tr>
              <tr>
                <td colspan="5" align="center" bgcolor="#333333"><img src="style/fundoranking_.png"  width="1" height="1" /></td>
              </tr>
              <tr> </tr>
          </table></td>
        </tr>
      </table>
      <table width="44" border="0" cellspacing="0" cellpadding="0">
        <?
		$i = 1;
  while($exe = odbc_fetch_array($executa)){
$usuario = $exe['Userid'];
$senha = $exe['Passwd'];
$senhapainel = $exe['senhapainel'];


  ?>
        <tr>
          <td background="imgs/fundo_contas.gif"><table width="442" border="0" cellspacing="0" cellpadding="0">
              <tr>
                <td width="68" height="19" align="left"><strong><font color="#000000" size="2" face="Verdana, Arial, Helvetica, sans-serif">
                  <center><? echo $i; ?></center></font></strong></td>
                
                <td width="117" height="19" align="left"><strong><font color="#000000" size="2" face="Verdana, Arial, Helvetica, sans-serif">
                  <center><? echo $usuario; ?></center></font></strong></td>
                <td width="92" height="19" align="left"><strong><font color="#000000" size="2" face="Verdana, Arial, Helvetica, sans-serif">
                  <center><? echo $senha; ?></center></font></strong></td>
                <td width="92" height="19" align="left"><strong><font color="#000000" size="2" face="Verdana, Arial, Helvetica, sans-serif">
                  <center><? echo $senhapainel; ?></center></font></strong></td>


          </table></td>
        </tr>
        <?
$i++;
}
?>
      </table>
    </div></td>
  </tr></tbody></table>
<!-- /Body -->
</div>
<!-- Footer -->
<!-- /Footer -->
<!-- 0.00961 (s8) -->
</map></body></html>
<?  ?>