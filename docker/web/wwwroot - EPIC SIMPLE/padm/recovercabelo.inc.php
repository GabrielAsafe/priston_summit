<?
if (PT!=1)  exit;
?>

<table background="imgs/fundo_textura1.gif" width="716" border="0" align="center" cellpadding="0" cellspacing="0">
              <tr>
                <td width="716"><table width="82%" border="0" align="center" cellpadding="4" cellspacing="2">
                  <tr>
                    <td colspan="2" align="center" bgcolor="#000000"><b><font color="#FFFFFF">Restaurar cabelo do Personagem</font></b></td>
                  </tr>
  <tr>
    <td><form id="form1" name="form1" method="post" action="">
      <table width="773" height="102" border="0" align="center">
        <tr>
          <td width="193"><div align="center">Nick do char</div></td>
          <td width="570"><label>
            <div align="left">
              <input type="text" name="nick" id="nick" />
              </div>
            </label></td>
        </tr>
        <tr>
          <td><div align="center"></div></td>
          <td><label>
            <div align="left">
              <input type="submit" name="button" id="button" value="Recuperar Cabelo" />
              <input name="acao" type="hidden" id="acao" value="recupera" />
            </div>
          </label></td>
        </tr>
        <tr>
                  <td colspan="2" align="center"><div align="left">
                    <p><strong>Instru&ccedil;&otilde;es de como usar o sistema de restaurar cabelo.<br>
                    </strong>1 Passo - Localize o campo branco acima.<br> 
                    2 Passo - No campo em branco coloque o nick do player que voc&ecirc; deseja restaura o cabelo.<br>
                    3 Passo - Clique em recuperar cabelo.</p>
                    <p>Ps: esse sistema eh apenas para ser usado caso.<br>
                    Player, estava com cliente full bugado usou poção capilar ficou sem cabeça usando este sistema a cabeça dele                    volta ao normal. </p>
                    <p>Apos seguir os 3 passos acima, e clicar em restaurar vai abrir abrir uma mensagem falando.<BR>
                    qual a&ccedil;&atilde;o que deu, se foi restaurado com sucesso, se o char n&atilde;o existe etc...</p>
                    </div>
                  </div></td>
                  </tr>
      </table>
      <p>&nbsp;</p>
    </form>
    </td>
  </tr>
</table>
<table width="300" border="0">
  <tr>
    <td><?
	if($_POST['acao'] == "recupera"){
	$nick = $_POST['nick'];
	if(empty($nick)){
	echo"<b><center><font color='red'>preencha o nick do char!";
	}else{
	$pasta = $dirUserData . ( $func->numDir($nick)) . "/" . $nick . ".dat";
	
	$fp = fopen($pasta,"r");
		$dados = fread($fp,filesize($pasta));
		$classe_char = substr($dados,0xc4,1);
		
		switch (ord($classe_char))
                {
                    case 1: $class = 'Fighter'; break;
                    case 2: $class = 'Mechanician'; break;
                    case 3: $class = 'Archer'; break;
                    case 4: $class = 'Pikeman'; break;
                    case 5: $class = 'Atalanta'; break;
                    case 6: $class = 'Knight'; break;
                    case 7: $class = 'Magician'; break;
                    case 8: $class = 'Priestess'; break;
                }
	
	$fRead=false;
                $fOpen = fopen($pasta, "r");
                while (!feof($fOpen)) {
                @$fRead = "$fRead" . fread($fOpen, filesize($pasta) );
                }
                fclose($fOpen);

                // Change hair ----------------------------------------------------------------
                $sourceStr = substr($fRead, 0, 48) . ($func->charResetHair($class, 1)) . substr($fRead, 69, 43) . ($func->charResetHair($class, 2)) . substr($fRead, 136);
                $fOpen = fopen($pasta, "wb"); 
                fwrite($fOpen, $sourceStr, strlen($sourceStr));
                fclose($fOpen);

                echo "<b><center><font color='green'>Cabelo do char foi recuperado com sucesso!!";
	
	
	
	
	}
	}
	
	
	
	
	?></td>
  </tr>
</table>
</body>
</html>
