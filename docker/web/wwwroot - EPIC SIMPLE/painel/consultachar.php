<?php
$nomechar = $_SESSION["charName"];
$dia = date("d");
$mes = date("m");
$ano = date("Y");

//ARQUIVO TXT A SER PESQUISADO
$arquivo = "Logs/personagens.txt";

//ABRE O ARQUIVO
$ponteiro = fopen($arquivo, "r");

//LÊ
$conteudo = fread($ponteiro, filesize($arquivo) );

//FECHA O ARQUIVO
fclose($ponteiro);

//EXPLODE AS LINHAS QUANDO PULAR LINHA
$linha = explode(" ", $conteudo);

for($i = 0; $i <= sizeof($linha); $i++) {
 //SEPARANDO OS DADOS POR ; (PONTO E VIRGULA)
 $parte = explode(";", $linha[$i]);
 //NICK DO CHAR
 $parte_nick = trim($parte[0]);
 //DIA
 $parte_dia = trim($parte[1]);
 //MES
 $parte_mes = trim($parte[2]);
  //ANO
 $parte_ano = trim($parte[3]);
 
 //VERIFICA SE O USUÁRIO DIGITADO EXISTE E SE ELE CRIOU A POUCO TEMPO
 if( ($nomechar == $parte_nick) && ($dia ==$parte_dia) && ($mes ==$parte_mes) && ($ano ==$parte_ano) ) {
  //SOMA A VARIÁVEL EXISTE
  $naodeleta++;
 }//FECHA IF
}//FECHA FOR
?>

