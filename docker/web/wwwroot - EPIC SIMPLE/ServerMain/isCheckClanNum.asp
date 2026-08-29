<?php require_once dirname(__FILE__).'/../_clan_port/lib.php';
require_params(array('num')); $c=db_connect_clan(); if(!$c) code_exit(100);
$s=q($c,'SELECT TOP 1 ClanName,Note FROM dbo.CL WHERE MIconCnt=?',array(req('num'))); if(!$s) fail_db($c);
$r=fetch_row($s); @odbc_close($c); if($r===false) code_exit(0);
proto_out('Code=1'."\r".'CName='.field($r,'ClanName')."\r".'CNote='.field($r,'Note')."\r"); ?>