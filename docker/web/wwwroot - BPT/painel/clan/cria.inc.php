<? if (Painel!=1) exit; ?>
    </div>
    <table width="100%" align="center" border="0" cellspacing="0" cellpadding="0">
        <tr>
            <td colspan="2" bgcolor="#ffcc00" height="25"><div align="center"><strong><font color="#000000">Gerenciamento de clan </font></strong>          
          </td>
    </table>
    <br>
    <table width="70%" align="center" border="0" cellspacing="0" cellpadding="0">
     
      <tr>
        <td><font color="Black">Olá Guerreiro,</font><br><br>
          <font color="Black">Seja bem vindo ao nosso sistema de clan!<br> Somente por aqui será possível criar seu clan, e deverá saber que para isso existem as seguintes regras:<br />
            			<br />
			<strong>1.</strong> O personagem que deseja colocar como líder do clãn deverá estar<font color="Red"><em> level 40+</em></font><br />
            <strong>2.</strong> Somente pode ter um líder por conta. <br /></font>

          <p align="justify">
            <?
			include_once "injection.php";
			if(!$_SESSION["charName"])
			{
			?>
            <b><img src="imgs/setinha.gif" width="11" height="11" /></b> <font color="blue">Antes de prosseguir na criação do clãn indique o personagem que será o líder do clãn clicando sobre o nome dele:</font></p>
          <p align="center"><b>
            <?
            $qCharID=($_SESSION["charID"])?$_SESSION["charID"]:$_SESSION["ID"];
            $charInfo=$dirUserInfo . ($func->numDir($qCharID)) . "/" . $qCharID . ".dat";

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

                if(count($charNameArr)>0)
                {
                    foreach($charNameArr as $key=>$value)
                    {
                        $expValue=explode("\x00",$value);
                        echo "<a href=\"?char=".$expValue[0]."\"&sess=criarclan>".$expValue[0]."</a>&nbsp;&nbsp;";
                    }
                }
                else
                {
                    echo "Vazio";
                }

            }
            else
            {
                echo "Vazio";
            }

}
else
{
?>
            </p>
            <br>
          <p align="center"><b><img src="imgs/setinha.gif" width="11" height="11" /><font color="Black"> Nome do Clan: </font></b></p>          </td>
      </tr>
      <tr>
        <td align="center"><form name="form1" method="post" action="index.php?sess=criarclan">
            <label>
            <input name="nomeclan" placeholder="Nome para o clãn" type="text" id="nomeclan" size="20" maxlength="12">
            </label>
            <label>
            <input name="acao" type="submit" id="acao" value="Criar">
            </label>
            <input name="acao" type="hidden" id="acao" value="Criar" />
        </form>

<?
}
?>

		</td>
      </tr>

