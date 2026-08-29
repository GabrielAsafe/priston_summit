<?php require_once dirname(__FILE__).'/../_clan_port/lib.php';
require_params(array('chname','gserver','clName')); $c=db_connect_clan(); if(!$c)code_exit(100); $ch=req('chname');$cn=req('clName');
$s=q($c,'SELECT TOP 1 userid FROM dbo.UL WHERE ChName=? AND ClanName=?',array($ch,$cn)); if(!$s)fail_db($c);$r=fetch_row($s);if($r===false){@odbc_close($c);code_exit(0);} $u=field($r,'userid');
$s=q($c,'UPDATE dbo.CL SET ClanZang=?,UserID=? WHERE ClanName=?',array($ch,$u,$cn));@odbc_close($c);code_exit($s?1:0); ?>