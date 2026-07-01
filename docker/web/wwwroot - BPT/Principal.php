<!DOCTYPE html>
<html>
<head>
<meta http-equiv="Content-Type" content="text/html; charset=UTF-8">
<?php
header('Content-Type: text/html; charset=UTF-8');
// Configuração adicional para garantir a codificaçãod
ini_set('default_charset', 'UTF-8');
?>
<script type="text/javascript">
//<![CDATA[
var numBanner = 0;
var bannerIntervalId;
$(init);
function init()
{
$($("#acoesBannerRotativo a")[0]).click(function() { moverBanner(-1); });
$($("#acoesBannerRotativo a")[1]).click(function() { moverBanner(1); });
iniciarRotacaoAutomatica();
}
function iniciarRotacaoAutomatica()
{
bannerIntervalId = setInterval("moverBanner(1)", 3000);
}
function pararRotacaoAutomatica()
{
clearInterval(bannerIntervalId);
}

function moverBanner(direcao)
{
pararRotacaoAutomatica();
var maxBanners = $("#bannersRotativos li").length - 1;
numBanner += direcao;

if (numBanner > maxBanners)
numBanner = 0;
else if (numBanner < 0)
numBanner = maxBanners;

$("#bannersRotativos li").each(
function(index) {
if (numBanner == index)
$(this).show();
else
$(this).hide();
}
);
iniciarRotacaoAutomatica();
}
function processarTwitter(t)
{
$("#twitter ul").html("");
for (var i = 0; i < t.results.length; i++)
{
$("#twitter ul").append("<li>"
+ t.results[i].text
+ "<a href='http://twitter.com/"
+ t.results[i].from_user
+ "/status/"
+ t.results[i].id
+ "'>"
+ new Date(t.results[i].created_at).format("dd/mm/yyyy HH:MM")
+ "</a></li>");
}
}

//]]>
</script>
<h1><span>Contents</span></h1>
<div id="destaques1">
<h2><span>Destaques</span></h2>

<table background="imagens/minhaconta/minhaconta-top.gif" width="172" height="35">
<tr><td><div align="center"><b><div id="titulo">Minha Conta</div></b></div></tr></td>
</table>
<div id="minhaconta">
<ul>
<li id="cadastrese"><a href="painel/cadtermos.php">Cadastre-se</a>&nbsp;<small>(jogue grátis)</small></li>

<li id="modificarconta"><a href="painel/">Modificar conta</a>&nbsp;<small>(modifique seus dados)</small></li>
</ul>
</div>
<div class="banners">
<h3><span>Banners</span></h3>
<ul>
<li><a href="https://wiki.priston.com.br/index.php/Sobre_o_Priston_Tale"><img src="imagens/botoes/bot1.gif" id="Img1" alt="banner" class="banner" /></a></li>

<li><a href="https://wiki.priston.com.br/index.php/Guia_Iniciante"><img src="imagens/botoes/bot2.gif" id="Img2" alt="banner" class="banner" /></a></li>
<li><a href="suporte/FAQ_Solucao.aspx"><img src="imagens/botoes/bot3.gif" id="Img3" alt="banner" class="banner" /></a></li>
</ul>
</div>

<table background="imagens/serverstatus/serverstatus-top.gif" width="172" height="35">
<tr><td><div align="center"><b><div id="titulo">Status</div></b></div></tr></td>
</table>
<div id="serverstatus">
<ul>
<li>
<span class="server">
Servidor
</span>
<ul>

<li>
<iframe src ="<? echo $URL_HostUP; ?>/scripts/servidorup/status.php" scrolling="No" frameborder="0" marginwidth="0" marginheight="0" vspace="0" hspace="0" allowtransparency="true" width="47" height="11"><p>Sem suporte iFrame</p></iframe>
</li>

</ul>
</li>




<li>
<span class="server">
Proteção
</span>

<ul>

<li class='serverOk xtrap'>
<span>(Ok)&nbsp;XTrap</span></li>

</ul>
</li>

</ul>
<p id="manutencao"><span>Próxima Manutenção</span><br />

<span id="horarioManutencao">Toda Quarta - 03:00h</span>
</p>
</div>

<table background="imagens/equipe/equipestatus-top.gif" width="172" height="45">
<tr><td><b><div id="titulo2" align="center">Status Equipe</div></b></tr></td>
</table>
<iframe src ="<? echo $URL_HostUP; ?>/scripts/servidorup/equipe.php" scrolling="No" frameborder="0" marginwidth="0" marginheight="0" vspace="0" hspace="0" allowtransparency="true" width="172" height="173"><p>Sem suporte iFrame</p></iframe>
</div>
<div id="principal">
<h2><span>Principal</span></h2>
<div id="bannerRotativo">
<h3><span>Banner Rotativo</span></h3>
<ul id="bannersRotativos">
<li><a href="https://discord.gg/9JbQsDD74X"><img src="imagens/banners/home0.jpg" alt="banner" class="banner" /></a></li>
<li style="display:none"><a href="https://discord.gg/9JbQsDD74X"><img src="imagens/banners/home2.jpg" alt="banner" class="banner" /></a></li>


</ul>

<h4><span>Ações do banner rotativo</span></h4>
<ul id="acoesBannerRotativo">
<li id="anterior"><a href="javascript:void(0)">Próximo</a></li>
<li id="proximo"><a href="javascript:void(0)">Anterior</a></li>
</ul>
</div>

<div id="novidades">

<h3><span>Novidades</span></h3>
<ul id="listaNovidades">

<li class='noticia'>
<span class="tipo">(Notícia)&nbsp;</span>
<a href='#' title='Sem novas notícias em nosso servidor'>
Sem novas notícias em nosso serv...
</a><span class="data">
30.08.2025
</span>

</li>

<li class='noticia'>
<span class="tipo">(Notícia)&nbsp;</span>
<a href='#' title='Sem novas notícias em nosso servidor'>
Sem novas notícias em nosso serv...
</a><span class="data">
30.08.2025
</span>
</li>

<li class='noticia'>
<span class="tipo">(Notícia)&nbsp;</span>
<a href='#' title='Sem novas notícias em nosso servidor'>
Sem novas notícias em nosso serv...
</a><span class="data">
30.08.2025
</span>
</li>

<li class='noticia'>
<span class="tipo">(Notícia)&nbsp;</span>
<a href='#' title='Sem novas notícias em nosso servidor'>
Sem novas notícias em nosso serv...
</a><span class="data">
30.08.2025
</span>
</li>

<li class='noticia'>
<span class="tipo">(Notícia)&nbsp;</span>
<a href='#' title='Sem novas notícias em nosso servidor'>
Sem novas notícias em nosso serv...
</a><span class="data">
30.08.2025
</span>
</li>

<li class='noticia'>
<span class="tipo">(Notícia)&nbsp;</span>
<a href='#' title='Sem novas notícias em nosso servidor'>
Sem novas notícias em nosso serv...
</a><span class="data">
30.08.2025
</span>
</li>

</ul>
<h4><span>Ver mais</span></h4>
<ul id="legendasnovidades">
<li id="legendaNoticias">

<a href="https://discord.gg/9JbQsDD74X">Notícias</a></li>
<li id="legendaEventos">
<a href="https://discord.gg/9JbQsDD74X">Eventos</a></li>
<li id="legendaPromocoes">
<a href="https://discord.gg/9JbQsDD74X">Promoções</a></li>
<li id="legendaUpdates">
<a href="https://discord.gg/9JbQsDD74X">Updates</a></li>

</ul>
</div>

<div id="loja">
<h3><span>Loja Summit Tale</span></h3>
<p>Na <a href="/painel">loja</a> Summit Tale você pode adquirir itens exclusivos que vão turbinar seu up!</p>
<ul>

<li><a href="/painel"><span>Cristais Místicos (H$ 1.500,00)</span><img src="imagens/loja/item1.png" id="Img7" alt="Item 1" /></a></li>
<li><a href="/painel"><span>Cristais Místicos (H$ 1.500,00)</span><img src="imagens/loja/item2.png" id="Img8" alt="Item 2" /></a></li>
<li><a href="/painel"><span>Cristais Místicos (H$ 1.500,00)</span><img src="imagens/loja/item3.png" id="Img9" alt="Item 3" /></a></li>
</ul>
</div>
<div id="parcerias">
<h3><span>Parcerias</span></h3>


</div>
<div id="fanArea">
<h3><span>Fan Area</span></h3>

<img src="imagens/fanarea/destaque_fanarea.jpg" id="imagemDestaqueFanArea" alt="[Destaque do Fan Area]" />
<ul>
<li id="fanArt"><a href="#fanart#">Fan Art</a></li>
<li id="fanShot"><a href="#fanshot#">Fan Shot</a></li>
</ul>
</div>
<div id="multimidia">
<h3><span>Multimídia</span></h3>

<p>Conteúdo Oficial do Summit Tale.</p>
<ul>
<li id="wallpapers"><a href="https://discord.gg/9JbQsDD74X">Wallpapers</a></li>
<li id="screenshots"><a href="https://discord.gg/9JbQsDD74X">Screenshots</a></li>
<li id="videos"><a href="https://discord.gg/9JbQsDD74X">Vídeos</a></li>
</ul>
</div>

</div>
<div id="destaques2">
<h2><span>Destaques</span></h2>

<table background="imagens/ranking/ranking-top.gif" width="172" height="35">
<tr><td><b><div align="center" id="titulo">Top Ranking</div></b></tr></td>
</table>
<div id="ranking">
<ul>
<li>
<iframe src ="<? echo $URL_HostUP; ?>/scripts/servidorup/top10.php" scrolling="No" frameborder="0" marginwidth="0" marginheight="0" vspace="0" hspace="0" allowtransparency="true" width="155" height="100%"><p>Sem suporte iFrame</p></iframe>
</li>
</ul>
<a href="rank100.php">Ver mais</a>
</div>
<div class="banners">

<h3><span>Banners</span></h3>
<ul>
<li><a href="https://discord.gg/9JbQsDD74X"><img src="imagens/botoes/forum_bot.gif" id="Img10" alt="banner" class="banner" /></a></li>
<li><a href="https://discord.gg/9JbQsDD74X"><img src="imagens/botoes/cast_bot.gif" id="Img11" alt="banner" class="banner" /></a></li>
</ul>
</div>

<table background="imagens/blesscastle/blesscastle-top.gif" width="172" height="35">
<tr><td><b><div align="center" id="titulo">Bless Castle</div></b></tr></td>
</table>
<div id="blesscastle">
<ul>
<li>
<center><b>
<iframe src ="<? echo $URL_HostUP; ?>/scripts/servidorup/blesscastle.php" scrolling="No" frameborder="0" marginwidth="0" marginheight="0" vspace="0" hspace="0" allowtransparency="true" width="154" height="70"><p>Sem suporte iFrame</p></iframe>
</b></center>
</li>
</ul>
</div>

<table background="imagens/bellatra/bellatra-top.gif" width="172" height="45">
<tr><td><b><div align="center" id="titulo2">Bellatra</div></b></tr></td>
</table>
<div id="bellatra">
<ul>
<li>
<center><b>
<iframe src ="<? echo $URL_HostUP; ?>/scripts/servidorup/bellatra.php" scrolling="No" frameborder="0" marginwidth="0" marginheight="0" vspace="0" hspace="0" allowtransparency="true" width="154" height="70"><p>Sem suporte iFrame</p></iframe>
</b></center>
</li>
<a href="rank100c.php">Ver mais</a>
</ul>
</div>

<table background="imagens/last_update/up_back.gif" width="172" height="110">
<tr><td><div align="center"><b><div id="titulo">Atualizações</div>
<div id="ultimoUpdate">
<span class="data"><center>Última att: 30/08/2025</span><span class="descricao">Atualize seu jogo!</span><a href="<? echo $URL_WebsiteUP; ?>?conteudo=Downloads"">Saiba Mais</a></b></div></tr></td>
</table>
</div>
</div>

<table background="imagens/redes_sociais/comunidades_back.gif" width="184" height="110">
<tr><td><div align="center"><b><div id="titulo3">Comunidades</div>

<div id="comunidades">
<h3><span>Comunidades Summit Tale</span></h3>
<ul>
<li id="comunidadeYoutube">
<a href="javascript:void(window.open('https://discord.gg/9JbQsDD74X'))">Youtube</a></li>
<li id="comunidadeTwitter">
<a href="javascript:void(window.open('https://www.instagram.com/summittalebr/'))">Twitter</a></li>
<li id="comunidadeOrkut">

<a href="javascript:void(window.open('https://chat.whatsapp.com/FPbAxvnr8bh3cWbvJbIsEc'))">Orkut</a></li>
</ul>
</div></b></div></tr></td>
</table>
</div>