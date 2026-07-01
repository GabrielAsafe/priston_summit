<?
if (PT!=1) exit;
?>
<?
include_once "injection.php";
include_once "incluir/configura.php";
?>
<table background="imgs/fundo_textura1.gif" width="716" border="0" align="center" cellpadding="0" cellspacing="0">
<td><table width="100%" border="0" align="center" cellpadding="4" cellspacing="2">
              <tr>
			    <td colspan="2" align="center" bgcolor="#000000"><b><font color="#FFFFFF">Analisa os arquivos .dat Bugados</font></b></td>
                  </tr>
                <td width="716"><table width="100%" border="0" align="center" cellpadding="4" cellspacing="2">
      <td align="center" bgcolor="#CCCCCC"><table border="1" align="center" cellpadding="0" cellspacing="0">
        <tr bgcolor="#0000FF">
          <table border="1" align="center" cellpadding="0" cellspacing="0">
            <tr bgcolor="#CCCCCC">
              <td width="80"><div align="center"><strong><font size="1" face="Verdana, Arial, Helvetica, sans-serif">Conta</font></strong></div></td>
              <td width="100"><div align="center"><strong><font size="1" face="Verdana, Arial, Helvetica, sans-serif">Char</font></strong></div></td>
              <td width="40"><div align="center"><strong><font size="1" face="Verdana, Arial, Helvetica, sans-serif">Bugado</font></strong></div></td>
              <td width="400"><div align="center"><strong><font size="1" face="Verdana, Arial, Helvetica, sans-serif">Pasta - Arquivo</font></strong></div></td>
            </tr>
            <?
$narq =  count(scan_Dir($dirUserData));     // Rastreia as sub-pastas da userdata
$charDat = scan_Dir($dirUserData);          // Pega os nomes dos arquivos para re-passalos para o leitor
$conta = 0;

for($i=0; $i<$narq; $i++) {                 //enquanto existir arquivos
?>
            <tr>
              <td><?        
           
        $fOpen = fopen($charDat[$i], "r");                 // Abre o Arquivo .dat
        $fRead =fread($fOpen,filesize($charDat[$i]));      // Le o arquivo .dat
        @fclose($fOpen);                                   // Fecha o arquivo .dat
      
      $charName = trim(substr($fRead,0x10,15),"\x00");  // Pega o Nome do char no offset 0x10
        $charAcc =  trim(substr($fRead,0x2c0,15),"\x00");              // Pega a ACC do dat
      $charLevel =   substr($fRead,0xc8,1);             // Pega o codigo do Nivel do Char
        $charLevel = ord($charLevel);                     // Converte o Nivel do Char para decimal (ascii)
      
       if(!is_numeric($charLevel) OR $charLevel == 0) {  //se level nao for numerico ou valor = 0
           $conta=$conta+1;                                //Some +1 ao contador        
           $bug = "Sim";                                   // Seta variavel como sim
           $pasta = $charDat[$i];                         // Seta variavel com o caminho completo do arquivo
               }
            else                                        // caso contrario
               {       
                 $bug = "Nao";                               // Seta variavel como nao  
                 $pasta = $charDat[$i];                   // Seta variavel com o caminho completo do arquivo
              }   
?>
              <tr>
                <?
         if($bug == Nao )   // Se a varivel for igual a Não escrever com letra de cor 0000FF (Azul)
         {
         ?>
                <td align="center" bgcolor="#FFFFFF"><font color="0000FF" ><? echo $charAcc; ?></font></td>
                <td align="center" bgcolor="#FFFFFF"><font color="0000FF" ><? echo $charName; ?></font></td>
                <td align="center" bgcolor="#FFFFFF"><font color="0000FF" ><? echo $bug; ?></font></td>
                <td align="center" bgcolor="#FFFFFF"><font color="0000FF" ><? echo $pasta; ?></td>
                <?
           }
         else      // Caso contrario escrever com letra de cor FF0000 (Vermelha)
         {
        ?>
                <td align="center" bgcolor="#FFFFFF"><font color="FF0000" ><? echo $charAcc; ?></font></td>
                <td align="center" bgcolor="#FFFFFF"><font color="FF0000" ><? echo $charName; ?></font></td>
                <td align="center" bgcolor="#FFFFFF"><font color="FF0000" ><? echo $bug; ?></font></td>
                <td align="center" bgcolor="#FFFFFF"><font color="FF0000" ><? echo $pasta; ?></td>
                <?        
        }
           ?>
                <?
}  
//Bloco da função para varredura dos diretorio em modo recursivo
function scan_Dir($dir) {
   $arrfiles = array();
   if (is_dir($dir)) {
       if ($handle = opendir($dir)) {
           chdir($dir);
           while (false !== ($file = readdir($handle))) {
               if ($file != "." && $file != "..") {
                   if (is_dir($file)) {
                       $arr = scan_Dir($file);
                       foreach ($arr as $value) {
                           $arrfiles[] = $dir."/".$value;
                       }
                   } else {
                       $arrfiles[] = $dir."/".$file;
                   }
               }
           }
           chdir("../");
       }
       closedir($handle);
   }
   return $arrfiles;
}
    
?>
            </table>
        </table>
        <table border="1" align="center" cellpadding="0" cellspacing="0">
          <tr>
            <td width="660" align="center" bgcolor="#FFFFFF"><strong>Foram lidos: <? echo $i  ?> .dat - Corrompidos encontrados: <? echo $conta  ?></strong></td>
          <tr>        
          <tr>
            <td width="660" align="center" bgcolor="#FFFFFF"> Script, desenvolvido por D-KiN: </td>
      </table></td>
    </tr>
  </table>
</form>
</body>
</html>