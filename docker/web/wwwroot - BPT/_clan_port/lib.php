<?php
/* Shared PHP 5.x/ODBC helpers for the legacy clan endpoints. */
header('Content-Type: text/plain; charset=ISO-8859-1');

function proto_out($s) { echo $s; }
function proto_exit($s) { echo $s; exit; }
function code_exit($n) { proto_exit('Code=' . $n . "\r"); }
function req($name) {
    if (isset($_REQUEST[$name])) return trim((string)$_REQUEST[$name]);
    /* ASP Request() is case-insensitive; emulate it. */
    foreach ($_REQUEST as $k=>$v) if (strcasecmp($k,$name)===0) return trim((string)$v);
    return '';
}
function require_params($names) {
    foreach ($names as $n) if (req($n)==='') code_exit(100);
}
function cfg_value($cfg,$name) {
    $re='/\\$'.preg_quote($name,'/').'\\s*=\\s*[\'\"]([^\'\"]+)[\'\"]\\s*;/';
    return preg_match($re,$cfg,$m) ? $m[1] : false;
}
function db_connect_clan() {
    $paths=array('/var/www/html/padm/incluir/configura.php','/var/www/html/painel/incluir/configura.php');
    $cfg=false;
    foreach($paths as $p){ $x=@file_get_contents($p); if($x!==false){$cfg=$x; break;} }
    if($cfg===false) return false;
    $dsn=cfg_value($cfg,'connection_string'); $user=cfg_value($cfg,'user'); $pass=cfg_value($cfg,'pass');
    if($dsn===false||$user===false||$pass===false) return false;
    $c=@odbc_connect($dsn,$user,$pass); if(!$c) return false;
    if(!@odbc_exec($c,'USE ClanDB')) { @odbc_close($c); return false; }
    return $c;
}
function q($conn,$sql,$params=array()) {
    foreach($params as $v){
        $rep=($v===null)?'NULL':("'".str_replace("'","''",(string)$v)."'");
        $pos=strpos($sql,'?'); if($pos===false) return false;
        $sql=substr($sql,0,$pos).$rep.substr($sql,$pos+1);
    }
    if(strpos($sql,'?')!==false) return false;
    return @odbc_exec($conn,$sql);
}
function fetch_row($stmt) {
    if(!$stmt || !@odbc_fetch_row($stmt)) return false;
    $row=array(); $n=@odbc_num_fields($stmt);
    for($i=1;$i<=$n;$i++){ $name=@odbc_field_name($stmt,$i); $v=@odbc_result($stmt,$i); $row[$name]=($v===false||$v===null)?'':trim((string)$v); }
    return $row;
}
function fetch_all($stmt) { $a=array(); if(!$stmt)return $a; while(($r=fetch_row($stmt))!==false)$a[]=$r; return $a; }
function begin_tx($c){ return @odbc_exec($c,'BEGIN TRANSACTION')!==false; }
function commit_tx($c){ return @odbc_exec($c,'COMMIT TRANSACTION')!==false; }
function rollback_tx($c){ @odbc_exec($c,'ROLLBACK TRANSACTION'); }
function fail_db($c=null){ if($c)@odbc_close($c); code_exit(100); }
function field($r,$name,$default=''){ return isset($r[$name])?$r[$name]:$default; }
?>