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
                    <td colspan="2" align="center" bgcolor="#000000"><b><font color="#FFFFFF">Mover o Char para outra conta</span></th>
    </tr>
<body background="imgs/fundo.jpg">
<table width="550" border="0">
  <tr>
    <td><form id="form1" name="form1" method="post" action="">
      <table width="367" border="0" align="center">
        <tr>
          <td width="206">Mover um char para outra conta</td>
          <td width="151">&nbsp;</td>
        </tr>
        <tr>
          <td>Nome do char</td>
          <td><label>
            <input type="text" name="nick" id="nick" />
          </label></td>
        </tr>
        <tr>
          <td>ID para onde o char vai</td>
          <td><label>
            <input type="text" name="id" id="id" />
          </label></td>
        </tr>
        <tr>
          <td>&nbsp;</td>
          <td><label>
            <input type="submit" name="button" id="button" class="button6" value="Mover char" />
            <input name="acao" type="hidden" id="acao" class="button6" value="move" />
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
	if($_POST[acao] == "move"){
	$id = $_POST['id'];
	$char = $_POST['nick'];
	$pasta = $dirUserData . ( $func->numDir($char)) . "/" . $char . ".dat";
	
$accMove=trim($func->char_filter(trim($id)),"\x00");

                if(!$func->is_valid_string($accMove))
                {

                    $charInfo=$dirUserInfo . ($func->numDir($accMove)) . "/" . $accMove . ".dat";

                    if(file_exists($charInfo) && ( filesize($charInfo)==240) )
                    {

                        $fRead=false;
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

                        $chkEmpArr=array();
                        $chkChar=array();
                        foreach($charNameArr as $key=>$value)
                        {
                            if(empty($value)) $chkEmpArr[]=$key;
                            else $chkChar[]=$key;
                        }

                        if(count($chkChar)<5)
                        {

                            // point to each information line
                            $startPoint=$chkEmpArr[0];
                            $endPoint=$startPoint+15;

                            // update account information for new account information-------------------------
                            // Fill in 00 to left character
                            $addOnLeft=false;
                            $leftLen=15-strlen($char);
                            for($i=0;$i<$leftLen;$i++)
                            {
                                $addOnLeft.=pack("h*",00);
                            }
                            $writeName=$char.$addOnLeft;

                            $fRead=false;
                            $fOpen = fopen($charInfo, "r");
                            while (!feof($fOpen)) {
                            @$fRead = "$fRead" . fread($fOpen, filesize($charInfo) );
                            }
                            fclose($fOpen);

                            $sourceStr = substr($fRead, 0, $startPoint) . $writeName . substr($fRead, $endPoint);
                            $fOpen = fopen($charInfo, "wb"); 
                            fwrite($fOpen, $sourceStr, strlen($sourceStr));
                            fclose($fOpen);


                            // empty character in account information-------------------------------------
                            $charInfo=$dirUserInfo . ($func->numDir($id)) . "/" . $id . ".dat";

                            $fRead=false;
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

                            $chkCharLine=array();
                            foreach($charNameArr as $key=>$value)
                            {
                                if($char==$value) $chkCharLine[]=$key;
                            }

                            // point to each information line
                            $startPoint=$chkCharLine[0];
                            $endPoint=$startPoint+15;

                            // Fill in 00 to left character
                            $addOnLeft=false;
                            for($i=0;$i<15;$i++)
                            {
                                $addOnLeft.=pack("h*",00);
                            }

                            $fRead=false;
                            $fOpen = fopen($charInfo, "r");
                            while (!feof($fOpen)) {
                            @$fRead = "$fRead" . fread($fOpen, filesize($charInfo) );
                            }
                            fclose($fOpen);


                            $sourceStr = substr($fRead, 0, $startPoint) . $addOnLeft . substr($fRead, $endPoint);
                            $fOpen = fopen($charInfo, "wb"); 
                            fwrite($fOpen, $sourceStr, strlen($sourceStr));
                            fclose($fOpen);

                            // update data information---------------------------------------------------------------------
                            // Fill in 00 to left character
                            $addOnLeft=false;
                            $leftLen=10-strlen($accMove);
                            for($i=0;$i<$leftLen;$i++)
                            {
                                $addOnLeft.=pack("h*",00);
                            }
                            $writeAccName=$accMove.$addOnLeft;

                            $fRead=false;
                            $fOpen = fopen($pasta, "r");
                            while (!feof($fOpen)) {
                            @$fRead = "$fRead" . fread($fOpen, filesize($pasta) );
                            }
                            fclose($fOpen);

                            $sourceStr = substr($fRead, 0, 704) . $writeAccName . substr($fRead, 714);
                            $fOpen = fopen($pasta, "wb"); 
                            fwrite($fOpen, $sourceStr, strlen($sourceStr));
                            fclose($fOpen);

                            echo "<b><center><font color='green'>O char ". $char ." Foi movido para a conta<u> ". $accMove."com sucesso!";

                            
                        }
                        else
                        {
                            echo "<font color='red'><b> conta esta cheia,impossivel mover o char!!";
                        }


                    }
                    else
                    {
                        echo "<font color='red'><b>as informacoes do char nao existem ou estao corrompidas!!!";
                    }



                }
                else
                {
                    echo "<font color='red'><b>preencha os campos corretamente!!!";
                }


                

            }
			
	
	
	
	
	
	
	
	
	?></td>
  </tr>
</table>
</body>
</html>
