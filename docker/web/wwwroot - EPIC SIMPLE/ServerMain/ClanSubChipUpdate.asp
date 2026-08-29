<?php require_once dirname(__FILE__).'/../_clan_port/lib.php';
require_params(array('chname','gserver')); $c=db_connect_clan(); if(!$c) code_exit(100); if(!begin_tx($c))fail_db($c);
$ch=req('chname'); $a=q($c,'UPDATE dbo.UL SET Permi=0 WHERE ClanName IN (SELECT ClanName FROM dbo.UL WHERE ChName=?)',array($ch)); $b=$a?q($c,'UPDATE dbo.UL SET Permi=2 WHERE ChName=?',array($ch)):false;
if(!$a||!$b){rollback_tx($c);@odbc_close($c);code_exit(100);} commit_tx($c);@odbc_close($c);code_exit(1); ?>