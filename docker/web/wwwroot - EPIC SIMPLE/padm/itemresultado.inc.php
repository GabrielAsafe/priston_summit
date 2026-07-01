<?
if (PT!=1) exit;

?>
<?
include_once "injection.php";
?>
	<table background="imgs/fundo_textura1.gif" width="723" border="0" align="center" cellpadding="0" cellspacing="0">
              <tr>
                <td width="723"><table width="100%" border="0" align="center" cellpadding="4" cellspacing="2">
                  <tr>
                    <td width="644" align="center" bgcolor="#000000"><b><font color="#FFFFFF"> Sistema de Procurar Items em um Personagem</font></b></td>
                  </tr></table>
<?
$item = $_POST['iten'];
$char = $_POST['id'];
$erro = 0;

if(empty($item) || empty($char)){
	echo "Preencha Todos os Campos!";
echo "<meta HTTP-EQUIV='Refresh' CONTENT='3;URL=index.php?sessadm=procuraritem'>";
$erro++;
}

 function numDir($name)
    {
        $character=array
        (
            "0"=>"48",
            "1"=>"49",
            "2"=>"50",
            "3"=>"51",
            "4"=>"52",
            "5"=>"53",
            "6"=>"54",
            "7"=>"55",
            "8"=>"56",
            "9"=>"57",

            "a"=>"65",
            "b"=>"66",
            "c"=>"67",
            "d"=>"68",
            "e"=>"69",
            "f"=>"70",
            "g"=>"71",
            "h"=>"72",
            "i"=>"73",
            "j"=>"74",
            "k"=>"75",
            "l"=>"76",
            "m"=>"77",
            "n"=>"78",
            "o"=>"79",
            "p"=>"80",
            "q"=>"81",
            "r"=>"82",
            "s"=>"83",
            "t"=>"84",
            "u"=>"85",
            "v"=>"86",
            "w"=>"87",
            "x"=>"88",
            "y"=>"89",
            "z"=>"90",

            "A"=>"65",
            "B"=>"66",
            "C"=>"67",
            "D"=>"68",
            "E"=>"69",
            "F"=>"70",
            "G"=>"71",
            "H"=>"72",
            "I"=>"73",
            "J"=>"74",
            "K"=>"75",
            "L"=>"76",
            "M"=>"77",
            "N"=>"78",
            "O"=>"79",
            "P"=>"80",
            "Q"=>"81",
            "R"=>"82",
            "S"=>"83",
            "T"=>"84",
            "U"=>"85",
            "V"=>"86",
            "W"=>"87",
            "X"=>"88",
            "Y"=>"89",
            "Z"=>"90",
        );

        for($i=0;$i<strlen($name);$i++)
        {
            $total+=$character[$name[$i]];
            if($total>=256)
                $total=$total-256;
        }

        return $total;
    }
if($erro == "0"){
$pasta = "".$rootDir."/DataServer/userdata/";
$arquivo = $pasta.numDir($char)."/".$char.".dat";

$arquivo_get = @file_get_contents("".$arquivo."");
if (@strstr($arquivo_get, $item)) {
echo "  O Item foi procurado e achado, $item está no seguinte personagem $char<br><br>";
echo "<meta HTTP-EQUIV='Refresh' CONTENT='10;URL=index.php?sessadm=procuraritem'>";
}else{
echo "  Não Existe nenhum item chamado $item no Char $char<br><br>";
echo "<meta HTTP-EQUIV='Refresh' CONTENT='10;URL=index.php?sessadm=procuraritem'>";

}
}
?>
<? ?>