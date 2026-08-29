<?php require_once dirname(__FILE__).'/../_clan_port/lib.php';
require_params(array('userid','gserver','chname')); $c=db_connect_clan(); if(!$c) code_exit(100);
$s=q($c,'SELECT TOP 1 ClanName FROM dbo.UL WHERE ChName=?',array(req('chname'))); if(!$s) fail_db($c); $u=fetch_row($s); if($u===false){@odbc_close($c);code_exit(0);} $cn=field($u,'ClanName');
$s=q($c,'SELECT TOP 1 ClanZang FROM dbo.CL WHERE ClanName=?',array($cn)); if(!$s) fail_db($c); $cl=fetch_row($s); if($cl===false){@odbc_close($c);code_exit(0);}
$out='Code=1'."\r".'CClanName='.$cn."\r".'CClanZang='.field($cl,'ClanZang')."\r";
$s=q($c,'SELECT ChName FROM dbo.UL WHERE ClanName=?',array($cn)); if(!$s) fail_db($c); foreach(fetch_all($s) as $m)$out.='CMem='.field($m,'ChName')."\r"; @odbc_close($c); proto_out($out); ?>