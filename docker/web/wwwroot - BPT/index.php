<?php
require("config.php");
if (!$_REQUEST['conteudo']){
$Tituto_Pagina = 'Principal';
}else{
$Tituto_Pagina = $_REQUEST['conteudo'];
}
?>

<!DOCTYPE html PUBLIC "-//W3C//DTD XHTML 1.0 Transitional//EN" "http://www.w3.org/TR/xhtml1/DTD/xhtml1-transitional.dtd">
<html xmlns="http://www.w3.org/1999/xhtml">
<head>
<title>Bem vindo(a) ao <? echo $Nome ?> (<? echo $Website_URL ?>)</title>

<!-- ATENÇAO, VOCE NÃO ESTA AUTORIZADO A EXCLUIR AS LINHAS QUE ESTÃO ENTRE ESTE COMENTARIO -->
<meta http-equiv="Content-Type" content="text/html; charset=ISO-8859-1">
<meta name="robots" content="index, follow"> 
<meta name="keywords" content="Priston, tale, Tale, Priston Tale, priston, tale, Summit Tale, Private, Pirata, Privado, Server PT, PT, Priston Tale Brasil, Brasil, SummitTale, Summit Tale"> 
<link rel="shortcut icon" href="imagens/favicon2.ico" />

<!-- ATENÇAO, VOCE NÃO ESTA AUTORIZADO A EXCLUIR AS LINHAS QUE ESTÃO ENTRE ESTE COMENTARIO -->
<meta name="description" content="Summit Tale - www.SummitTale.com.br">
<meta name="author" content="Sergio Paes"> 
<meta name="verify-v1" content="dMbQEGmZOzfcCCTUT5a079pVqCxkqTCnzW8oOUMJCEk="> 
<meta name="robots" content="index,follow"></meta>
<meta name="classification" content="Internet"> 
<meta name="distribution" content="Global">
<meta name="rating" content="General">

<!-- ATENÇAO, VOCE NÃO ESTA AUTORIZADO A EXCLUIR AS LINHAS QUE ESTÃO ENTRE ESTE COMENTARIO -->
<meta name="resource-type" content="document">
<meta name="revisit-after" content="3 days"> 
<meta name="language" content="pt-br"> 
<meta name="doc-class" content="Completed">
<meta name="doc-rights" content="Public">

<link href="css/geral.css" rel="stylesheet" type="text/css">
<link href="css/style.css" rel="stylesheet" type="text/css">
<link href="css/home.css" rel="stylesheet" type="text/css">
<link href="css/dropdown.css" rel="stylesheet" type="text/css">

<script type="text/javascript" src="js/jquery-1.4.2.min.js"></script>

<script language="JavaScript" type="text/javascript">
var BMenuImgs = new Array(5);
var BMImgs = new Array(5);
BMImgs[0]			= new Image(32, 15);
BMenuImgs[0]		= new Image(32, 15);
BMenuImgs[0].src	= "imagens/over/over_03.png";

BMImgs[1]			= new Image(58, 15);
BMenuImgs[1]		= new Image(58, 15);
BMenuImgs[1].src	= "imagens/over/over_05.png";

BMImgs[2]			= new Image(70, 15);
BMenuImgs[2]		= new Image(70, 15);
BMenuImgs[2].src	= "imagens/over/over_07.png";

BMImgs[3]			= new Image(70, 15);
BMenuImgs[3]		= new Image(70, 15);
BMenuImgs[3].src	= "imagens/over/over_09.png";

BMImgs[4]			= new Image(71, 15);
BMenuImgs[4]		= new Image(71, 15);
BMenuImgs[4].src	= "imagens/over/over_11.png";

BMImgs[5]			= new Image(64, 15);
BMenuImgs[5]		= new Image(64, 15);
BMenuImgs[5].src	= "imagens/over/over_13.png";

BMImgs[6]			= new Image(26, 15);
BMenuImgs[6]		= new Image(26, 15);
BMenuImgs[6].src	= "imagens/over/over_15.png";

BMImgs[7]			= new Image(42, 15);
BMenuImgs[7]		= new Image(42, 15);
BMenuImgs[7].src	= "imagens/over/over_17.png";
</script>

</head>
<body>
<div id="container">
<div id="intro">

<h1><span>Intro</span></h1>
<table id="Tabela_01" width="831" height="231" border="0" cellpadding="0" cellspacing="0">
<tr>
<td colspan="20">
<img src="imagens/menu_01.png" width="830" height="21" alt="" border="0"></td>
<td>
<img src="imagens/spacer.gif" width="1" height="21" alt="" border="0"></td>
</tr>
<tr>
<td>
<img src="imagens/menu_02.png" width="64" height="15" alt="" border="0"></td>
<td>
<a href="<? echo $URL_WebsiteUP; ?>?conteudo=Principal"><img src="imagens/menu_03.png" width="32" height="15" alt="" border="0"  onmouseover="BMImgs[0].src=this.src; this.src=BMenuImgs[0].src;" onmouseout="this.src=BMImgs[0].src;"></a></td>
<td>
<img src="imagens/menu_04.png" width="22" height="15" alt="" border="0"></td>
<td>
<ul class="sf-menu">
<li class="current">
<a href="https://discord.gg/9JbQsDD74X"><img src="imagens/menu_05.png" width="58" height="15" alt="" border="0" onmouseover="BMImgs[1].src=this.src; this.src=BMenuImgs[1].src;" onmouseout="this.src=BMImgs[1].src;"></a>
</ul>
</li>
</ul>
</td>

<td>
<img src="imagens/menu_06.png" width="16" height="15" alt="" border="0"></td>
<td>
<ul class="sf-menu">
<li class="current">
<a href="/painel"><img src="imagens/menu_07.png" width="70" height="15" alt="" border="0" onmouseover="BMImgs[2].src=this.src; this.src=BMenuImgs[2].src;" onmouseout="this.src=BMImgs[2].src;"></a>
<ul>
</ul>
</li>
</ul>
</td>
<td>
<img src="imagens/menu_08.png" width="18" height="15" alt="" border="0"></td>
<td colspan="2">
<ul class="sf-menu">
<li class="current">
<a href="https://wiki.priston.com.br/index.php/Guia_Iniciante"><img src="imagens/menu_09.png" width="70" height="15" alt="" border="0" onmouseover="BMImgs[3].src=this.src; this.src=BMenuImgs[3].src;" onmouseout="this.src=BMImgs[3].src;"></a>
<ul>
</ul>
</li>
</ul>
</td>

<td>
<img src="imagens/menu_10.png" width="135" height="15" alt="" border="0"></td>
<td colspan="2">
<ul class="sf-menu">
<li class="current">
<a href="https://chat.whatsapp.com/FPbAxvnr8bh3cWbvJbIsEc"><img src="imagens/menu_11.png" width="71" height="15" alt="" border="0" onmouseover="BMImgs[4].src=this.src; this.src=BMenuImgs[4].src;" onmouseout="this.src=BMImgs[4].src;"></a>
<ul>
</ul>
</li>
</ul>
</td>
<td>
<img src="imagens/menu_12.png" width="22" height="15" alt="" border="0"></td>
<td>
<ul class="sf-menu">
<li class="current">
<a href="https://www.instagram.com/summittalebr/"><img src="imagens/menu_13.png" width="64" height="15" alt="" border="0" onmouseover="BMImgs[5].src=this.src; this.src=BMenuImgs[5].src;" onmouseout="this.src=BMImgs[5].src;"></a>
<ul>
</ul>
</li>
</ul>
</td>
<td colspan="2">
<img src="imagens/menu_14.png" width="21" height="15" alt="" border="0"></td>
<td>
<ul class="sf-menu">
<li class="current">
<a href="/painel"><img src="imagens/menu_15.png" width="26" height="15" alt="" border="0" onmouseover="BMImgs[6].src=this.src; this.src=BMenuImgs[6].src;" onmouseout="this.src=BMImgs[6].src;"></a>
<ul>
</ul>
</li>
</ul>
</td>
<td>
<img src="imagens/menu_16.png" width="25" height="15" alt="" border="0"></td>
<td>
<ul class="sf-menu">
<li class="current">
<a href="https://discord.gg/9JbQsDD74X"><img src="imagens/menu_17.png" width="42" height="15" alt="" border="0" onmouseover="BMImgs[7].src=this.src; this.src=BMenuImgs[7].src;" onmouseout="this.src=BMImgs[7].src;">
<ul>
</ul>
</li>
</ul>
</td>
<td>
<img src="imagens/menu_18.png" width="74" height="15" alt="" border="0"></td>
<td>
<img src="imagens/spacer.gif" width="1" height="15" alt="" border="0"></td>
</tr>
<tr>
<td colspan="8" rowspan="3">
<img src="imagens/menu_19.png" width="336" height="194" alt="" border="0"></td>
<td colspan="3">
<img src="imagens/menu_20.png" width="162" height="78" alt="" border="0"></td>
<td colspan="9" rowspan="2">
<img src="imagens/menu_21.png" width="332" height="102" alt="" border="0"></td>
<td>
<img src="imagens/spacer.gif" width="1" height="78" alt="" border="0"></td>
</tr>
<tr>
<td colspan="3" rowspan="2">
<img src="imagens/menu_22.png" width="162" height="116" alt="" border="0"></td>
<td>
<img src="imagens/spacer.gif" width="1" height="24" alt="" border="0"></td>
</tr>
<tr>
<td colspan="4">
<a href="<? echo $URL_WebsiteUP; ?>?conteudo=Downloads"><img src="imagens/menu_23.png" width="162" height="92" alt="" border="0"></a></td>
<td colspan="5">
<img src="imagens/menu_24.png" width="170" height="92" alt="" border="0"></td>
<td>
<img src="imagens/spacer.gif" width="1" height="92" alt="" border="0"></td>
</tr>
<tr>
<td>
<img src="imagens/spacer.gif" width="64" height="1" alt="" border="0"></td>
<td>
<img src="imagens/spacer.gif" width="32" height="1" alt="" border="0"></td>
<td>
<img src="imagens/spacer.gif" width="22" height="1" alt="" border="0"></td>
<td>
<img src="imagens/spacer.gif" width="58" height="1" alt="" border="0"></td>
<td>
<img src="imagens/spacer.gif" width="16" height="1" alt="" border="0"></td>
<td>
<img src="imagens/spacer.gif" width="70" height="1" alt="" border="0"></td>
<td>
<img src="imagens/spacer.gif" width="18" height="1" alt="" border="0"></td>
<td>
<img src="imagens/spacer.gif" width="56" height="1" alt="" border="0"></td>
<td>
<img src="imagens/spacer.gif" width="14" height="1" alt="" border="0"></td>
<td>
<img src="imagens/spacer.gif" width="135" height="1" alt="" border="0"></td>
<td>
<img src="imagens/spacer.gif" width="13" height="1" alt="" border="0"></td>
<td>
<img src="imagens/spacer.gif" width="58" height="1" alt="" border="0"></td>
<td>
<img src="imagens/spacer.gif" width="22" height="1" alt="" border="0"></td>
<td>
<img src="imagens/spacer.gif" width="64" height="1" alt="" border="0"></td>
<td>
<img src="imagens/spacer.gif" width="18" height="1" alt="" border="0"></td>
<td>
<img src="imagens/spacer.gif" width="3" height="1" alt="" border="0"></td>
<td>
<img src="imagens/spacer.gif" width="26" height="1" alt="" border="0"></td>
<td>
<img src="imagens/spacer.gif" width="25" height="1" alt="" border="0"></td>
<td>
<img src="imagens/spacer.gif" width="42" height="1" alt="" border="0"></td>
<td>
<img src="imagens/spacer.gif" width="74" height="1" alt="" border="0"></td>
<td></td>
</tr>
</table>
</div>
<div id="contents">
<?
if($_REQUEST['conteudo']==""){
require("Principal.php");
}else{
if(!file_exists($_REQUEST['conteudo'].".php")){
echo 'sasa';
}else{
require($_REQUEST['conteudo'].".php");
}
}
?>
</div>

<div id="footer">
<br/>
<div id="copyright">
  <p style="COLOR: #ffffff">Copyright 2025 <a href="javascript:void(window.open('https://summittale.com.br/'))">Summit Tale</a> Games</a>  - Todos os direitos reservados.</p></div>
<br/>
</div>
</div>
</body>
</html>