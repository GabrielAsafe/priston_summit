<?
include_once "injection.php";
if($_SESSION["charDir"])
        {
            if($_POST[acao]=="Alterar"){
				// PEGANDO OS CREDITOS DELE
				$id = $_SESSION["ID"];
				$arquivo = "shop/bonusplayer/$id.arc";
				$creditos = "40";
				$necessario = "40";
				$tamanho = filesize($arquivo);
				
				$fp = @fopen($arquivo,"r");
				$creditos = fread($fp,$tamanho); // -> TOTAL DE CREDITOS
				fclose($fp);
				if( $func->saldo($_SESSION["ID"],$necessario) == false){ // VERIFICA SE ELE TEM CREDITO
	echo "<b><div align=\"center\"><img src=\"imgs/alerta.png\"><br><font color=red>Voc&ecirc; n&atilde;o tem cr&eacute;ditos suficientes para Alterar a Classe! </div><br><br>";
				}else{		
                $fRead=false;
                $fOpen = fopen($_SESSION["charDir"], "r");
                while (!feof($fOpen)) {
                @$fRead = "$fRead" . fread($fOpen, filesize($_SESSION["charDir"]) );
                }
                fclose($fOpen);

                $bin = $func->char2Num($_POST["class"]);

                $bina= pack("h*",$bin);

                // Change character class ----------------------------------------------------------------
                $sourceStr = substr($fRead, 0, 48) . ($func->charResetHair($_POST['class'], 1)) . substr($fRead, 69, 43) . ($func->charResetHair($_POST['class'], 2)) . substr($fRead, 136, 60) . $bina . substr($fRead, 197, 7) . ($func->charResetState($_POST['class'])) . substr($fRead, 224, 284) . ($func->charResetSkill()) . substr($fRead, 524, 0) . ($func->charResetMastery()) . substr($fRead, 556);
                $fOpen = fopen($_SESSION["charDir"], "wb"); 
                fwrite($fOpen, $sourceStr, strlen($sourceStr));
                fclose($fOpen);
						// RETIRANDO CREDITOS
				$func->atualiza_saldo($_SESSION["ID"],"40"); // funcao(ID,VALOR A RETIRAR);
				$_SESSION["charClass"]=$_POST['class'];
                echo "<b><div align=\"center\"><img src=\"imgs/alerta.png\"><br><font color=red> Parabéns sua classe foi alterada com sucesso para a escolhida.<br>Suas Skills,Stats e cabelo foram resetados de acordo com sua nova classe, Redistribua elas novamente conforme sua nova classe.<br></div><br><br>";

                echo "<b><div align=\"center\"><a href=\"$_SERVER[PHP_SELF]?char=$_SESSION[charName]\">Voltar para Página inicial</a>";
            }

        }
		}
		?>