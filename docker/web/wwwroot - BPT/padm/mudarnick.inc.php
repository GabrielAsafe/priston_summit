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
                    <td colspan="2" align="center" bgcolor="#000000"><b><font color="#FFFFFF">Sistema para mudar o Nick de um Char</span></th>
    </tr>
        <tr>
          <td>ID da conta</td>
          <td><label>
            <input type="text" name="id" id="id" />
          </label></td>
        </tr>
        <tr>
          <td>Nick antigo</td>
          <td><label>
            <input type="text" name="antigo" id="antigo" />
          </label></td>
        </tr>
        <tr>
          <td>Novo nick</td>
          <td><label>
            <input type="text" name="novo" id="novo" />
          </label></td>
        </tr>
        <tr>
          <td>&nbsp;</td>
          <td><label>
            <input type="submit" name="button" id="button" class="button6" value="Alterar Nick!" />
            <input name="acao" type="hidden" id="acao" class="button6" value="troca" />
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
	if($_POST[acao] == "troca"){
$id =	$_POST['id'];
$antigo = $_POST['antigo'];
$novo = $_POST['novo'];
$pastaid = $func->numDir($antigo);
$pasta = $dirUserData . ( $func->numDir($antigo)) . "/" . $antigo . ".dat";
	$novonick=trim($func->char_filter(trim($novo)),"\x00");

                if(!$func->is_valid_string($novonick))
                {

                    $charInfo=$dirUserInfo . ($func->numDir($id)) . "/" . $id . ".dat";


                    if(file_exists($charInfo) && ( filesize($charInfo)==240) )
                    {

                        $fOpen = fopen($charInfo, "r");
                        $fRead =fread($fOpen,filesize($charInfo));
                        @fclose($fOpen);

                        // list char information
                        $charNameArr=array(
                            "48" => trim(substr($fRead,0x30,15),"\x00"),
                            "80" => trim(substr($fRead,0x50,15),"\x00"),
                            "112"=> trim(substr($fRead,0x70,15),"\x00"),
                            "144"=> trim(substr($fRead,0x90,15),"\x00"),
                            "176"=> trim(substr($fRead,0xb0,15),"\x00"),
                        );

                        $charDat = $dirUserData . ($func->numDir($novonick)) . "/" . $novonick . ".dat";

                        if(!file_exists($charDat))
                        {

                            unset($flagArr);
                            $flagArr=array();

                            $sessNameExp=explode("\x00",$antigo);

                            foreach($charNameArr as $key=>$value)
                            {
                                $expValue=explode("\x00",$value);
                                if($sessNameExp[0]==$expValue[0])
                                    $flagArr[]=$key;
                            }

                            // point to each information line
                            $startPoint=$flagArr[0];
                            $endPoint=$startPoint+15;
        // Write info

                            $fRead=false;
                            $fOpen = fopen($charInfo, "r");
                            while (!feof($fOpen)) {
                            @$fRead = "$fRead" . fread($fOpen, filesize($charInfo) );
                            }
                            fclose($fOpen);

                            // Fill in 00 to left character
                            $leftLen=15-strlen($novonick);
                            for($i=0;$i<$leftLen;$i++)
                            {
                                $addOnLeft.=pack("h*",00);
                            }
                            $writeName=$novonick.$addOnLeft;

                            $sourceStr = substr($fRead, 0, $startPoint) . $writeName . substr($fRead, $endPoint);

                            $fOpen = fopen($charInfo, "wb"); 
                            fwrite($fOpen, $sourceStr, strlen($sourceStr));
                            fclose($fOpen);


        // Write data
                            $fRead=false;
                            $fOpen = fopen($pasta, "r");
                            while (!feof($fOpen)) {
                            @$fRead = "$fRead" . fread($fOpen, filesize($pasta) );
                            }
                            fclose($fOpen);

                            $sourceStr = substr($fRead, 0, 16) . $writeName . substr($fRead, 31);
                            $fOpen = fopen($pasta, "wb"); 
                            fwrite($fOpen, $sourceStr, strlen($sourceStr));
                            fclose($fOpen);

                            rename($dirUserData . $pastaid . "/" . $antigo. ".dat", $dirUserData . ($func->numDir($novonick)) . "/" . $novonick. ".dat");

                            echo"<b><center><font color='green'>Nick alterado com sucesso!</font><br>o char <u>$antigo</u> agora se chama <u>$novonick</u>";
	
	
	}
                        else
                        {
                            echo "<b><center><font color='red'>o nome escolhido para trocar j existe,por favor selecione outro!!!";
                        }

                    }
                    else
                    {
                        echo "<b><center><font color='red'>a informao do char no existe ou est corrompida!";
                    }
                }
                else
                {
                    echo "<b><center><font color='red'>por favor no utilize caracteres especiais no nick do char!";
                }
	}
	
	
	
	
	?></td>
  </tr>
</table>
</body>
</html>
