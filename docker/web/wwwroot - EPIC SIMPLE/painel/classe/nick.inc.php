<?
/*----------------------------------------------------------------------
Painel Player Versão 0.4
Desenvolvidor Por: Mak (sirmakloud@gmail.com)
Editado Por: Jiraya (richard.cva21@hotmail.com(
----------------------------------------------------------------------*/
include_once "injection.php";
if($func->saldo($_SESSION["ID"],"20") == false){
	echo "<b><div align=\"center\"><img src=\"imgs/alerta.png\"><br><font color=red>Voc&ecirc; n&atilde;o tem cr&eacute;ditos suficientes para Alterar seu Nick! </div><br><br>";
}else{
if($_POST[acao]=="nick"){
	$newName=trim($func->char_filter(trim($_POST["nick"])),"\x00");
	if(!$func->is_valid_string($newName)){
		$charInfo=$dirUserInfo . ($func->numDir($_SESSION["charID"])) . "/" . $_SESSION["charID"] . ".dat";
		if(file_exists($charInfo) && ( filesize($charInfo)==240) ){
			$fOpen = fopen($charInfo, "r");
			$fRead =fread($fOpen,filesize($charInfo));
			@fclose($fOpen);
			$charNameArr=array(
			"48" => trim(substr($fRead,0x30,15),"\x00"),
			"80" => trim(substr($fRead,0x50,15),"\x00"),
			"112"=> trim(substr($fRead,0x70,15),"\x00"),
			"144"=> trim(substr($fRead,0x90,15),"\x00"),
			"176"=> trim(substr($fRead,0xb0,15),"\x00"),
			);
			$charDat = $dirUserData .($func->numDir($newName))."/". $newName.".dat";
			if(!file_exists($charDat)){
				unset($flagArr);
				$flagArr=array();
				$sessNameExp=explode("\x00",$_SESSION["charName"]);
				foreach($charNameArr as $key=>$value){
					$expValue=explode("\x00",$value);
					if($sessNameExp[0]==$expValue[0])
					$flagArr[]=$key;
				}
				$startPoint=$flagArr[0];
				$endPoint=$startPoint+15;
				$fRead=false;
				$fOpen = fopen($charInfo, "r");
				while(!feof($fOpen)){
					@$fRead = "$fRead".fread($fOpen,filesize($charInfo));
				}
				fclose($fOpen);
				$leftLen=15-strlen($newName);
				for($i=0;$i<$leftLen;$i++){
					$addOnLeft.=pack("h*",00);
				}
				$writeName=$newName.$addOnLeft;
				$sourceStr = substr($fRead, 0, $startPoint) . $writeName . substr($fRead, $endPoint);
				$fOpen = fopen($charInfo, "wb"); 
				fwrite($fOpen, $sourceStr, strlen($sourceStr));
				fclose($fOpen);
				$fRead=false;
				$fOpen = fopen($_SESSION["charDir"], "r");
				while(!feof($fOpen)){
					@$fRead = "$fRead".fread($fOpen, filesize($_SESSION["charDir"]));
				}
				fclose($fOpen);
				$sourceStr = substr($fRead, 0, 16) . $writeName . substr($fRead, 31);
				$fOpen = fopen($_SESSION["charDir"], "wb"); 
				fwrite($fOpen, $sourceStr, strlen($sourceStr));
				fclose($fOpen);
				rename($dirUserData.$_SESSION["charNum"]."/".$_SESSION["charName"].".dat",$dirUserData.($func->numDir($newName))."/".$newName.".dat");
                                echo "<b><div align=\"center\"><img src=\"imgs/alerta.png\"><br><font color=red> Parab&eacute;ns seu nick foi alterado com sucesso, seu atual nick &eacute; ($newName)<br></div><br><br>";
				$func->atualiza_saldo($_SESSION["ID"],"20");
				$_SESSION["charDir"]=$dirUserData . ($func->numDir($newName)) . "/" . $newName. ".dat";
				$_SESSION["charName"]=$newName;
				$_SESSION["charNum"]=$func->numDir($newName);
			}else{
                                echo "<b><div align=\"center\"><img src=\"imgs/alerta.png\"><br><font color=red> Desculpe mais um Player, j&aacute; esta utilizando este Nick!<br>Escolha outro Nick para seu Personagem.<br></div><br><br>";
			}
		}else{
			echo "CHARACTER INFORMATION IS NOT EXISTING OR CORRUPTED!";
		}
	}else{
                echo "<b><div align=\"center\"><img src=\"imgs/alerta.png\"><br><font color=red> Por favor, use somente letras de A-Z e Numeros de 1-9 nao use caracteres invalidos, pois sao desconciderados!</div><br><br>";
	}
	echo "<b><div align=\"center\"><br><a href=\"$_SERVER[PHP_SELF]?char=$_SESSION[charName]\">Voltar para P&aacute;gina inicial</a>";
}}
?>