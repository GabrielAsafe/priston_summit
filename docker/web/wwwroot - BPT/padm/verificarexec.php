<?php runAsynchronously("$rootDir/$Server.exe","-connect 1.2.3.4"); ?>

<?php
function runAsynchronously($path,$arguments) {
    $WshShell = new COM("WScript.Shell");
    $oShellLink = $WshShell->CreateShortcut("temp.lnk");
	$ligarsv=0;
    $oShellLink->TargetPath = $path;
    $oShellLink->Arguments = $arguments;
    $oShellLink->WorkingDirectory = dirname($path);
    $oShellLink->WindowStyle = 1;
    $oShellLink->Save();
    $oExec = $WshShell->Run("temp.lnk", 7, false);
    unset($WshShell,$oShellLink,$oExec);
	$oExec++;
} 
		return $oExec;
?>