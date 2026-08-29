<?php require_once dirname(__FILE__).'/../_clan_port/lib.php';
$c=db_connect_clan(); if(!$c) code_exit(100); $s=q($c,'SELECT TOP 1 ClanName FROM dbo.UL WHERE ChName=?',array(req('chname'))); if(!$s) fail_db($c);
$r=fetch_row($s); @odbc_close($c); code_exit($r===false?0:1); ?>