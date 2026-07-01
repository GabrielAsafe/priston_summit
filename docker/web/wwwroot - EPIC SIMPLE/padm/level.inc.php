<?if (PT!=1) exit;?>

<?
include_once "incluir/configura.php";
include_once "injection.php";
?>
</style>
<link href="css/style.css" rel="stylesheet" type="text/css">
<table background="imgs/fundo_textura1.gif">
              
                  
<link href="style.css" rel="stylesheet" type="text/css">
<body background="background.gif">
<form id="form1" name="form1" method="post" action="">
<table background="imgs/fundo_textura1.gif" width="712" border="0" align="center" cellpadding="0" cellspacing="0">
              <tr>
                <td width="712"><table width="100%" border="0" align="center" cellpadding="4" cellspacing="2">
                  <tr>
                    <td colspan="2" align="center" bgcolor="#000000"><b><font color="#FFFFFF">Sistema de Level</span></th>
    </tr>
<table background="imgs/fundo_textura1.gif" width="716" border="0" align="center" cellpadding="0" cellspacing="0">
              <tr>
                <td width="716"><table width="100%" border="0" align="center" cellpadding="4" cellspacing="2">
<table width="550" border="0">
  <tr>
    <td><form id="form1" name="form1" method="post" action="">
      <table width="550" border="0">
        <tr>
          <td width="130">Nick do char</td>
          <td width="410"><label>
            <input type="text" name="nick" id="nick" />
            </label></td>
        </tr>
        <tr>
          <td>Level</td>
          <td><select name="level">
            <?
                for($i=1; $i<250; $i++)
                {
                    echo "<option ". ( ($i == $_SESSION["charLevel"] )? "selected" : "" ) .">". $i ."</option>";
                }
?>
          </select></td>
        </tr>
        <tr>
          <td>&nbsp;</td>
          <td><label>
            <input type="submit" name="button" id="button" class="button6" value="Alterar N&iacute;vel" />
            <input name="acao" type="hidden" id="acao" class="button6" value="lvl" />
          </label></td>
        </tr>
      </table>
        </form>
    </td>
    <tr>
                  <td colspan="2" align="center"><div align="left">
                    <p><strong>Instru&ccedil;&otilde;es de como Utilizar o Sistema de level editor<br>
                    </strong>1 Passo - Localize o campo branco acima.<br> 
                    2 Passo - No campo em branco coloque o nick do player que voc&ecirc; deseja alterar o level, escolha o level que você deseja que o player que você citou vá. <br>3 Passo - Clique em Alterar Nivel.</p>
                    <p>Apos seguir os 3 passos acima, e clicar em alterar nivel vai abrir uma janela falando assim.<BR>
                     Informa&ccedil;&atilde;o da do Player, falando se o level do player foi atualizando com sucesso ou oque...</p>
                  </div></td>
                  </table>
                  
  
</table>
<table width="300" border="0">
  <tr>
    <td><?
	if($_POST['acao'] == "lvl"){
	
	$nick = $_POST['nick'];
	$lvl = $_POST['level'];
	if(empty($nick)){
	echo"<b><font color='red'><center>preencha todos os campos!!!";
	}else{
	$pasta = $dirUserData.$func->numDir($nick)."/".$nick.".dat";	
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

                $xphex = fopen('xphex.txt', "r");
                while (!feof($xphex)) 
                {
                @$xp =$xp . fread($xphex, filesize('xphex.txt') );
                }
                fclose($xphex);
                $xp = substr($xp, ($_POST[level] -1 ) * 14 , 12);

                $level1= pack('i', $_POST[level]);

                $level2 =substr ($xp, 0, 4);
                $level2=hexdec("$level2");
                $level2= pack('i', $level2);

                $level3 =substr ($xp, 4, 8);
                $level3=hexdec("$level3");
                $level3= pack('i', $level3);

                $sourceStr = substr($fRead, 0 , 200) . $level1 . substr($fRead, 204, 0). ($func->charResetState($class)) . substr($fRead, 224, 108) . $level3 . substr($fRead, 336 ,68) . $level2 . substr($fRead,408, 100) . ($func->charResetSkill()) . substr($fRead, 524);

                $fOpen = fopen($pasta, "wb"); 
                fwrite($fOpen, $sourceStr, strlen($sourceStr));
                fclose($fOpen);
				echo"<b><center><font color='green'>o char <u>$nick</u> agora está no nivel <u>$lvl</u>";
	
	}
	}
	
	
	
	?></td>
  </tr>
</table>
</body>
</html>
