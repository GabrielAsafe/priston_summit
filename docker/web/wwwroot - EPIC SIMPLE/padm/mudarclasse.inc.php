<?if (PT!=1) exit;?>

<?
include_once "incluir/configura.php";
include_once "injection.php";
?>
<br>
<!DOCTYPE html PUBLIC "-//W3C//DTD XHTML 1.0 Transitional//EN" "http://www.w3.org/TR/xhtml1/DTD/xhtml1-transitional.dtd">
<html xmlns="http://www.w3.org/1999/xhtml">
<head>
<meta http-equiv="Content-Type" content="text/html; charset=utf-8" />
<title>Painel ADM</title>
<link href="imgs/style.css" rel="stylesheet" type="text/css" />
<style type="text/css">
<!--
.style1 {
	font-size: 12px;
	font-weight: bold;
}
-->
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
                    <td colspan="2" align="center" bgcolor="#000000"><b><font color="#FFFFFF">Sistema para mudança de Classe</span></th>
    </tr>
        <tr>
          <td>Nick</td>
          <td><label>
            <input type="text" name="nick" id="nick" />
          </label></td>
        </tr>
        <tr>
          <td>Nova classe</td>
          <td><select name="newclasse" id="newclasse">
            <option <?=($_SESSION["charClass"]=="Fighter")?"selected":""?>>Fighter</option>
            <option <?=($_SESSION["charClass"]=="Mechanician")?"selected":""?>>Mechanician</option>
            <option <?=($_SESSION["charClass"]=="Archer")?"selected":""?>>Archer</option>
            <option <?=($_SESSION["charClass"]=="Pikeman")?"selected":""?>>Pikeman</option>
            <option <?=($_SESSION["charClass"]=="Atalanta")?"selected":""?>>Atalanta</option>
            <option <?=($_SESSION["charClass"]=="Knight")?"selected":""?>>Knight</option>
            <option <?=($_SESSION["charClass"]=="Magician")?"selected":""?>>Magician</option>
            <option <?=($_SESSION["charClass"]=="Priestess")?"selected":""?>>Priestess</option>
          </select></td>
        </tr>
        <tr>
          <td>&nbsp;</td>
          <td><label>
            <input type="submit" name="button" id="button" class="button6" value="Mudar classe" />
            <input name="acao" type="hidden" id="acao" class="button6" value="alterar" />
          </label></td>
        </tr>
      </table>
        </form>
    </td>
  </tr>
</table>
<table width="300" border="0">
  <tr>
    <td><?
	if($_POST['acao'] == "alterar"){
	$nick = $_POST['nick'];
	$classe = $_POST['newclasse'];
	if(empty($nick))
	{
	echo"<b><center><font color='red'>preencha todos os campos!";
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

                $bin = $func->char2Num($classe);

                $bina= pack("h*",$bin);

                // Change character class ----------------------------------------------------------------
                $sourceStr = substr($fRead, 0, 48) . ($func->charResetHair($classe, 1)) . substr($fRead, 69, 43) . ($func->charResetHair($classe, 2)) . substr($fRead, 136, 60) . $bina . substr($fRead, 197, 7) . ($func->charResetState($classe)) . substr($fRead, 224, 284) . ($func->charResetSkill()) . substr($fRead, 524, 0) . ($func->charResetMastery()) . substr($fRead, 556);
                $fOpen = fopen($pasta, "wb"); 
                fwrite($fOpen, $sourceStr, strlen($sourceStr));
                fclose($fOpen);

                
                echo "<b><center><font color='green'>O Char <u>$nick</u> agora pertence a classe <u>$classe</u>";
                echo "<b><center><font color='green'>Todos os seus pontos de status e skill foram resetados!!!";
	
	
	}
	}
	
	?></td>
  </tr>
</table>
</body>
</html>
