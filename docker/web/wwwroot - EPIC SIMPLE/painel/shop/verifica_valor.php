<?php

$arquivo = trim($_POST['arquivo']);

//ABRE O ARQUIVO
$ponteiro = fopen($arquivo, "r");

//LÊ
$conteudo = fread($ponteiro, filesize($arquivo) );

//FECHA O ARQUIVO
fclose($ponteiro);

//EXPLODE AS LINHAS QUANDO PULAR LINHA
$linha = explode("\r", $conteudo);

for($i = 0; $i <= sizeof($linha); $i++) {
 //SEPARANDO OS DADOS POR ; (PONTO E VIRGULA)
 $parte = explode(";", $linha[$i]);
 //ID Item
 $cod_itemV = trim($parte[0]);
  //ANO
 $valor_itemV = trim($parte[3]);

 //VERIFICA SE O USUÁRIO DIGITADO EXISTE E SE ELE CRIOU A POUCO TEMPO
 if( ($valor_itemV == $valor) && ($cod_itemV ==$coditem) ) {
  //SOMA A VARIÁVEL EXISTE
  $valorcorreto++;
 }//FECHA IF
}//FECHA FOR
?>

