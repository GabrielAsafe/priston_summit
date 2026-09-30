# Dicionário de tabelas/colunas


## ------------

### ------------.dbo.---------
- `----------` `--------` nullable=----------
- `----------` `--------` nullable=----------
- `----------` `--------` nullable=----------
- `----------` `--------` nullable=----------
- `----------` `--------` nullable=----------
- `----------` `--------` nullable=----------
- `----------` `--------` nullable=----------
- `----------` `--------` nullable=----------
- `----------` `--------` nullable=----------
- `----------` `--------` nullable=----------
- `----------` `--------` nullable=----------
- `----------` `--------` nullable=----------
- `----------` `--------` nullable=----------
- `----------` `--------` nullable=----------
- `----------` `--------` nullable=----------
- `----------` `--------` nullable=----------
- `----------` `--------` nullable=----------
- `----------` `--------` nullable=----------
- `----------` `--------` nullable=----------
- `----------` `--------` nullable=----------
- `----------` `--------` nullable=----------
- `----------` `--------` nullable=----------

## ADM

### ADM.dbo.LogsBan
- `log_id` `int` nullable=NO
- `idplayer` `varchar` nullable=YES
- `motivo` `varchar` nullable=YES
- `punicao` `varchar` nullable=YES
- `diaban` `varchar` nullable=YES
- `mesban` `varchar` nullable=YES
- `anoban` `varchar` nullable=YES
- `diadesban` `varchar` nullable=YES
- `mesdesban` `varchar` nullable=YES
- `anodesban` `varchar` nullable=YES
- `gm` `varchar` nullable=YES
- `ip` `varchar` nullable=YES
- `data` `varchar` nullable=YES

### ADM.dbo.LogsDistribuidor
- `log_id` `int` nullable=NO
- `idGM` `varchar` nullable=YES
- `item` `varchar` nullable=YES
- `Classe` `varchar` nullable=YES
- `player_id` `varchar` nullable=YES
- `nick` `varchar` nullable=YES
- `data` `varchar` nullable=YES
- `ip` `varchar` nullable=YES

### ADM.dbo.loginGM
- `idGM` `varchar` nullable=NO
- `nickGM` `varchar` nullable=NO
- `permissao` `int` nullable=NO
- `passGM` `varchar` nullable=NO

## AccountDB

### AccountDB.dbo.AGameUser
- `userid` `varchar` nullable=NO
- `Passwd` `varchar` nullable=NO
- `GameCode` `varchar` nullable=YES
- `GPCode` `varchar` nullable=YES
- `RegistDay` `datetime` nullable=YES
- `DisuseDay` `datetime` nullable=YES
- `UsePeriod` `int` nullable=YES
- `inuse` `char` nullable=NO
- `Grade` `char` nullable=NO
- `EventChk` `char` nullable=NO
- `SelectChk` `char` nullable=NO
- `BlockChk` `char` nullable=NO
- `SpecialChk` `char` nullable=NO
- `ServerName` `varchar` nullable=YES
- `Credit` `char` nullable=NO
- `ECoin` `money` nullable=YES
- `StartDay` `datetime` nullable=YES
- `LastDay` `datetime` nullable=YES
- `EditDay` `datetime` nullable=YES
- `RNo` `int` nullable=YES
- `DelChk` `char` nullable=NO
- `SNo` `varchar` nullable=YES
- `Channel` `varchar` nullable=YES
- `BNum` `int` nullable=YES
- `MXServer` `varchar` nullable=YES
- `MXChar` `varchar` nullable=YES
- `MXType` `int` nullable=YES
- `MXLevel` `int` nullable=YES
- `MXExp` `bigint` nullable=YES

### AccountDB.dbo.ALLPersonalMember
- `PMNo` `int` nullable=NO
- `Userid` `varchar` nullable=NO
- `Passwd` `varchar` nullable=NO
- `senharpt` `varchar` nullable=YES
- `senhapainel` `varchar` nullable=YES
- `Question` `varchar` nullable=YES
- `Reply` `varchar` nullable=YES
- `CUserName1` `varchar` nullable=YES
- `CUserName2` `varchar` nullable=YES
- `KUserName1` `varchar` nullable=YES
- `KUserName2` `varchar` nullable=YES
- `Age` `varchar` nullable=YES
- `Gender` `varchar` nullable=YES
- `Email` `varchar` nullable=YES
- `Job` `varchar` nullable=YES
- `ZipCode1` `varchar` nullable=YES
- `ZipCode2` `varchar` nullable=YES
- `Add1` `varchar` nullable=YES
- `Add2` `varchar` nullable=YES
- `Add3` `varchar` nullable=YES
- `PhoneNum1` `varchar` nullable=YES
- `PhoneNum2` `varchar` nullable=YES
- `PhoneNum3` `varchar` nullable=YES
- `EmailChk` `varchar` nullable=YES
- `DelChk` `varchar` nullable=YES
- `RegistDay` `datetime` nullable=YES
- `DiaNasc` `nchar` nullable=YES
- `MesNasc` `nchar` nullable=YES
- `AnoNasc` `nchar` nullable=YES
- `sexo` `nchar` nullable=YES
- `ip` `nchar` nullable=YES
- `Gold` `nchar` nullable=YES
- `pais` `varchar` nullable=YES
- `bonus` `nchar` nullable=YES
- `Token` `nvarchar` nullable=YES

### AccountDB.dbo.APersonalMember
- `PMNo` `int` nullable=NO
- `Userid` `varchar` nullable=NO
- `Passwd` `varchar` nullable=NO
- `Question` `varchar` nullable=YES
- `Reply` `varchar` nullable=YES
- `CUserName1` `varchar` nullable=YES
- `CUserName2` `varchar` nullable=YES
- `KUserName1` `varchar` nullable=YES
- `KUserName2` `varchar` nullable=YES
- `Age` `varchar` nullable=YES
- `Gender` `varchar` nullable=YES
- `Email` `varchar` nullable=YES
- `Job` `varchar` nullable=YES
- `ZipCode1` `varchar` nullable=YES
- `ZipCode2` `varchar` nullable=YES
- `Add1` `varchar` nullable=YES
- `Add2` `varchar` nullable=YES
- `Add3` `varchar` nullable=YES
- `PhoneNum1` `varchar` nullable=YES
- `PhoneNum2` `varchar` nullable=YES
- `PhoneNum3` `varchar` nullable=YES
- `EmailChk` `varchar` nullable=YES
- `DelChk` `varchar` nullable=YES
- `RegistDay` `datetime` nullable=YES
- `Channel` `varchar` nullable=YES
- `BNum` `int` nullable=YES
- `External_id` `varchar` nullable=YES
- `NickName` `varchar` nullable=YES
- `IPADDR` `varchar` nullable=YES

### AccountDB.dbo.AddressInfo
- `AINo` `int` nullable=NO
- `Address` `varchar` nullable=NO
- `EditDate` `datetime` nullable=NO
- `Note` `varchar` nullable=YES

### AccountDB.dbo.AllGameUser
- `userid` `varchar` nullable=NO
- `Passwd` `varchar` nullable=NO
- `GameCode` `varchar` nullable=YES
- `GPCode` `varchar` nullable=YES
- `RegistDay` `datetime` nullable=YES
- `DisuseDay` `datetime` nullable=YES
- `UsePeriod` `int` nullable=YES
- `inuse` `char` nullable=NO
- `Grade` `char` nullable=NO
- `EventChk` `char` nullable=NO
- `SelectChk` `char` nullable=NO
- `BlockChk` `char` nullable=NO
- `SpecialChk` `char` nullable=NO
- `ServerName` `varchar` nullable=YES
- `Credit` `char` nullable=NO
- `ECoin` `money` nullable=YES
- `StartDay` `datetime` nullable=YES
- `LastDay` `datetime` nullable=YES
- `EditDay` `datetime` nullable=YES
- `RNo` `int` nullable=YES
- `DelChk` `char` nullable=NO
- `SNo` `varchar` nullable=YES
- `Channel` `varchar` nullable=YES
- `BNum` `int` nullable=YES
- `MXServer` `varchar` nullable=YES
- `MXChar` `varchar` nullable=YES
- `MXType` `int` nullable=YES
- `MXLevel` `int` nullable=YES
- `MXExp` `bigint` nullable=YES
- `BlockMOTIVO` `varchar` nullable=YES
- `BlockDATA` `varchar` nullable=YES

### AccountDB.dbo.BGameUser
- `userid` `varchar` nullable=NO
- `Passwd` `varchar` nullable=NO
- `GameCode` `varchar` nullable=YES
- `GPCode` `varchar` nullable=YES
- `RegistDay` `datetime` nullable=YES
- `DisuseDay` `datetime` nullable=YES
- `UsePeriod` `int` nullable=YES
- `inuse` `char` nullable=NO
- `Grade` `char` nullable=NO
- `EventChk` `char` nullable=NO
- `SelectChk` `char` nullable=NO
- `BlockChk` `char` nullable=NO
- `SpecialChk` `char` nullable=NO
- `ServerName` `varchar` nullable=YES
- `Credit` `char` nullable=NO
- `ECoin` `money` nullable=YES
- `StartDay` `datetime` nullable=YES
- `LastDay` `datetime` nullable=YES
- `EditDay` `datetime` nullable=YES
- `RNo` `int` nullable=YES
- `DelChk` `char` nullable=NO
- `SNo` `varchar` nullable=YES
- `Channel` `varchar` nullable=YES
- `BNum` `int` nullable=YES
- `MXServer` `varchar` nullable=YES
- `MXChar` `varchar` nullable=YES
- `MXType` `int` nullable=YES
- `MXLevel` `int` nullable=YES
- `MXExp` `bigint` nullable=YES

### AccountDB.dbo.BPersonalMember
- `PMNo` `int` nullable=NO
- `Userid` `varchar` nullable=NO
- `Passwd` `varchar` nullable=NO
- `Question` `varchar` nullable=YES
- `Reply` `varchar` nullable=YES
- `CUserName1` `varchar` nullable=YES
- `CUserName2` `varchar` nullable=YES
- `KUserName1` `varchar` nullable=YES
- `KUserName2` `varchar` nullable=YES
- `Age` `varchar` nullable=YES
- `Gender` `varchar` nullable=YES
- `Email` `varchar` nullable=YES
- `Job` `varchar` nullable=YES
- `ZipCode1` `varchar` nullable=YES
- `ZipCode2` `varchar` nullable=YES
- `Add1` `varchar` nullable=YES
- `Add2` `varchar` nullable=YES
- `Add3` `varchar` nullable=YES
- `PhoneNum1` `varchar` nullable=YES
- `PhoneNum2` `varchar` nullable=YES
- `PhoneNum3` `varchar` nullable=YES
- `EmailChk` `varchar` nullable=YES
- `DelChk` `varchar` nullable=YES
- `RegistDay` `datetime` nullable=YES
- `Channel` `varchar` nullable=YES
- `BNum` `int` nullable=YES
- `External_id` `varchar` nullable=YES
- `NickName` `varchar` nullable=YES
- `IPADDR` `varchar` nullable=YES

### AccountDB.dbo.BlockUserList
- `BNo` `int` nullable=NO
- `PMNo` `varchar` nullable=NO
- `userid` `varchar` nullable=NO
- `AppDate` `datetime` nullable=NO
- `ReuseDate` `datetime` nullable=NO
- `Active` `char` nullable=NO
- `SOperator` `varchar` nullable=NO
- `SReason` `ntext` nullable=NO
- `EOperator` `varchar` nullable=YES
- `EReason` `text` nullable=YES
- `InDate` `datetime` nullable=YES

### AccountDB.dbo.CGameUser
- `userid` `varchar` nullable=NO
- `Passwd` `varchar` nullable=NO
- `GameCode` `varchar` nullable=YES
- `GPCode` `varchar` nullable=YES
- `RegistDay` `datetime` nullable=YES
- `DisuseDay` `datetime` nullable=YES
- `UsePeriod` `int` nullable=YES
- `inuse` `char` nullable=NO
- `Grade` `char` nullable=NO
- `EventChk` `char` nullable=NO
- `SelectChk` `char` nullable=NO
- `BlockChk` `char` nullable=NO
- `SpecialChk` `char` nullable=NO
- `ServerName` `varchar` nullable=YES
- `Credit` `char` nullable=NO
- `ECoin` `money` nullable=YES
- `StartDay` `datetime` nullable=YES
- `LastDay` `datetime` nullable=YES
- `EditDay` `datetime` nullable=YES
- `RNo` `int` nullable=YES
- `DelChk` `char` nullable=NO
- `SNo` `varchar` nullable=YES
- `Channel` `varchar` nullable=YES
- `BNum` `int` nullable=YES
- `MXServer` `varchar` nullable=YES
- `MXChar` `varchar` nullable=YES
- `MXType` `int` nullable=YES
- `MXLevel` `int` nullable=YES
- `MXExp` `bigint` nullable=YES

### AccountDB.dbo.CPersonalMember
- `PMNo` `int` nullable=NO
- `Userid` `varchar` nullable=NO
- `Passwd` `varchar` nullable=NO
- `Question` `varchar` nullable=YES
- `Reply` `varchar` nullable=YES
- `CUserName1` `varchar` nullable=YES
- `CUserName2` `varchar` nullable=YES
- `KUserName1` `varchar` nullable=YES
- `KUserName2` `varchar` nullable=YES
- `Age` `varchar` nullable=YES
- `Gender` `varchar` nullable=YES
- `Email` `varchar` nullable=YES
- `Job` `varchar` nullable=YES
- `ZipCode1` `varchar` nullable=YES
- `ZipCode2` `varchar` nullable=YES
- `Add1` `varchar` nullable=YES
- `Add2` `varchar` nullable=YES
- `Add3` `varchar` nullable=YES
- `PhoneNum1` `varchar` nullable=YES
- `PhoneNum2` `varchar` nullable=YES
- `PhoneNum3` `varchar` nullable=YES
- `EmailChk` `varchar` nullable=YES
- `DelChk` `varchar` nullable=YES
- `RegistDay` `datetime` nullable=YES
- `Channel` `varchar` nullable=YES
- `BNum` `int` nullable=YES
- `External_id` `varchar` nullable=YES
- `NickName` `varchar` nullable=YES
- `IPADDR` `varchar` nullable=YES

### AccountDB.dbo.DGameUser
- `userid` `varchar` nullable=NO
- `Passwd` `varchar` nullable=NO
- `GameCode` `varchar` nullable=YES
- `GPCode` `varchar` nullable=YES
- `RegistDay` `datetime` nullable=YES
- `DisuseDay` `datetime` nullable=YES
- `UsePeriod` `int` nullable=YES
- `inuse` `char` nullable=NO
- `Grade` `char` nullable=NO
- `EventChk` `char` nullable=NO
- `SelectChk` `char` nullable=NO
- `BlockChk` `char` nullable=NO
- `SpecialChk` `char` nullable=NO
- `ServerName` `varchar` nullable=YES
- `Credit` `char` nullable=NO
- `ECoin` `money` nullable=YES
- `StartDay` `datetime` nullable=YES
- `LastDay` `datetime` nullable=YES
- `EditDay` `datetime` nullable=YES
- `RNo` `int` nullable=YES
- `DelChk` `char` nullable=NO
- `SNo` `varchar` nullable=YES
- `Channel` `varchar` nullable=YES
- `BNum` `int` nullable=YES
- `MXServer` `varchar` nullable=YES
- `MXChar` `varchar` nullable=YES
- `MXType` `int` nullable=YES
- `MXLevel` `int` nullable=YES
- `MXExp` `bigint` nullable=YES

### AccountDB.dbo.DPersonalMember
- `PMNo` `int` nullable=NO
- `Userid` `varchar` nullable=NO
- `Passwd` `varchar` nullable=NO
- `Question` `varchar` nullable=YES
- `Reply` `varchar` nullable=YES
- `CUserName1` `varchar` nullable=YES
- `CUserName2` `varchar` nullable=YES
- `KUserName1` `varchar` nullable=YES
- `KUserName2` `varchar` nullable=YES
- `Age` `varchar` nullable=YES
- `Gender` `varchar` nullable=YES
- `Email` `varchar` nullable=YES
- `Job` `varchar` nullable=YES
- `ZipCode1` `varchar` nullable=YES
- `ZipCode2` `varchar` nullable=YES
- `Add1` `varchar` nullable=YES
- `Add2` `varchar` nullable=YES
- `Add3` `varchar` nullable=YES
- `PhoneNum1` `varchar` nullable=YES
- `PhoneNum2` `varchar` nullable=YES
- `PhoneNum3` `varchar` nullable=YES
- `EmailChk` `varchar` nullable=YES
- `DelChk` `varchar` nullable=YES
- `RegistDay` `datetime` nullable=YES
- `Channel` `varchar` nullable=YES
- `BNum` `int` nullable=YES
- `External_id` `varchar` nullable=YES
- `NickName` `varchar` nullable=YES
- `IPADDR` `varchar` nullable=YES

### AccountDB.dbo.DelLog_Personal
- `PMNo` `int` nullable=NO
- `Userid` `varchar` nullable=NO
- `Passwd` `varchar` nullable=NO
- `Question` `varchar` nullable=NO
- `Reply` `varchar` nullable=NO
- `CUserName1` `varchar` nullable=NO
- `CUserName2` `varchar` nullable=NO
- `KUserName1` `varchar` nullable=NO
- `KUserName2` `varchar` nullable=NO
- `Age` `varchar` nullable=NO
- `Gender` `varchar` nullable=NO
- `Email` `varchar` nullable=NO
- `Job` `varchar` nullable=NO
- `ZipCode1` `varchar` nullable=NO
- `ZipCode2` `varchar` nullable=NO
- `Add1` `varchar` nullable=NO
- `Add2` `varchar` nullable=NO
- `Add3` `varchar` nullable=NO
- `PhoneNum1` `varchar` nullable=NO
- `PhoneNum2` `varchar` nullable=NO
- `PhoneNum3` `varchar` nullable=NO
- `EmailChk` `varchar` nullable=NO
- `DelChk` `varchar` nullable=NO
- `RegistDay` `datetime` nullable=NO

### AccountDB.dbo.DelUserQueue
- `DNo` `int` nullable=NO
- `PMNo` `varchar` nullable=NO
- `DeleteDate` `datetime` nullable=NO
- `DeleteReason` `varchar` nullable=YES
- `Status` `char` nullable=NO
- `Operator` `varchar` nullable=YES
- `InDate` `datetime` nullable=YES
- `Note` `varchar` nullable=YES

### AccountDB.dbo.EGameUser
- `userid` `varchar` nullable=NO
- `Passwd` `varchar` nullable=NO
- `GameCode` `varchar` nullable=YES
- `GPCode` `varchar` nullable=YES
- `RegistDay` `datetime` nullable=YES
- `DisuseDay` `datetime` nullable=YES
- `UsePeriod` `int` nullable=YES
- `inuse` `char` nullable=NO
- `Grade` `char` nullable=NO
- `EventChk` `char` nullable=NO
- `SelectChk` `char` nullable=NO
- `BlockChk` `char` nullable=NO
- `SpecialChk` `char` nullable=NO
- `ServerName` `varchar` nullable=YES
- `Credit` `char` nullable=NO
- `ECoin` `money` nullable=YES
- `StartDay` `datetime` nullable=YES
- `LastDay` `datetime` nullable=YES
- `EditDay` `datetime` nullable=YES
- `RNo` `int` nullable=YES
- `DelChk` `char` nullable=NO
- `SNo` `varchar` nullable=YES
- `Channel` `varchar` nullable=YES
- `BNum` `int` nullable=YES
- `MXServer` `varchar` nullable=YES
- `MXChar` `varchar` nullable=YES
- `MXType` `int` nullable=YES
- `MXLevel` `int` nullable=YES
- `MXExp` `bigint` nullable=YES

### AccountDB.dbo.EPersonalMember
- `PMNo` `int` nullable=NO
- `Userid` `varchar` nullable=NO
- `Passwd` `varchar` nullable=NO
- `Question` `varchar` nullable=YES
- `Reply` `varchar` nullable=YES
- `CUserName1` `varchar` nullable=YES
- `CUserName2` `varchar` nullable=YES
- `KUserName1` `varchar` nullable=YES
- `KUserName2` `varchar` nullable=YES
- `Age` `varchar` nullable=YES
- `Gender` `varchar` nullable=YES
- `Email` `varchar` nullable=YES
- `Job` `varchar` nullable=YES
- `ZipCode1` `varchar` nullable=YES
- `ZipCode2` `varchar` nullable=YES
- `Add1` `varchar` nullable=YES
- `Add2` `varchar` nullable=YES
- `Add3` `varchar` nullable=YES
- `PhoneNum1` `varchar` nullable=YES
- `PhoneNum2` `varchar` nullable=YES
- `PhoneNum3` `varchar` nullable=YES
- `EmailChk` `varchar` nullable=YES
- `DelChk` `varchar` nullable=YES
- `RegistDay` `datetime` nullable=YES
- `Channel` `varchar` nullable=YES
- `BNum` `int` nullable=YES
- `External_id` `varchar` nullable=YES
- `NickName` `varchar` nullable=YES
- `IPADDR` `varchar` nullable=YES

### AccountDB.dbo.FGameUser
- `userid` `varchar` nullable=NO
- `Passwd` `varchar` nullable=NO
- `GameCode` `varchar` nullable=YES
- `GPCode` `varchar` nullable=YES
- `RegistDay` `datetime` nullable=YES
- `DisuseDay` `datetime` nullable=YES
- `UsePeriod` `int` nullable=YES
- `inuse` `char` nullable=NO
- `Grade` `char` nullable=NO
- `EventChk` `char` nullable=NO
- `SelectChk` `char` nullable=NO
- `BlockChk` `char` nullable=NO
- `SpecialChk` `char` nullable=NO
- `ServerName` `varchar` nullable=YES
- `Credit` `char` nullable=NO
- `ECoin` `money` nullable=YES
- `StartDay` `datetime` nullable=YES
- `LastDay` `datetime` nullable=YES
- `EditDay` `datetime` nullable=YES
- `RNo` `int` nullable=YES
- `DelChk` `char` nullable=NO
- `SNo` `varchar` nullable=YES
- `Channel` `varchar` nullable=YES
- `BNum` `int` nullable=YES
- `MXServer` `varchar` nullable=YES
- `MXChar` `varchar` nullable=YES
- `MXType` `int` nullable=YES
- `MXLevel` `int` nullable=YES
- `MXExp` `bigint` nullable=YES

### AccountDB.dbo.FPersonalMember
- `PMNo` `int` nullable=NO
- `Userid` `varchar` nullable=NO
- `Passwd` `varchar` nullable=NO
- `Question` `varchar` nullable=YES
- `Reply` `varchar` nullable=YES
- `CUserName1` `varchar` nullable=YES
- `CUserName2` `varchar` nullable=YES
- `KUserName1` `varchar` nullable=YES
- `KUserName2` `varchar` nullable=YES
- `Age` `varchar` nullable=YES
- `Gender` `varchar` nullable=YES
- `Email` `varchar` nullable=YES
- `Job` `varchar` nullable=YES
- `ZipCode1` `varchar` nullable=YES
- `ZipCode2` `varchar` nullable=YES
- `Add1` `varchar` nullable=YES
- `Add2` `varchar` nullable=YES
- `Add3` `varchar` nullable=YES
- `PhoneNum1` `varchar` nullable=YES
- `PhoneNum2` `varchar` nullable=YES
- `PhoneNum3` `varchar` nullable=YES
- `EmailChk` `varchar` nullable=YES
- `DelChk` `varchar` nullable=YES
- `RegistDay` `datetime` nullable=YES
- `Channel` `varchar` nullable=YES
- `BNum` `int` nullable=YES
- `External_id` `varchar` nullable=YES
- `NickName` `varchar` nullable=YES
- `IPADDR` `varchar` nullable=YES

### AccountDB.dbo.GGameUser
- `userid` `varchar` nullable=NO
- `Passwd` `varchar` nullable=NO
- `GameCode` `varchar` nullable=YES
- `GPCode` `varchar` nullable=YES
- `RegistDay` `datetime` nullable=YES
- `DisuseDay` `datetime` nullable=YES
- `UsePeriod` `int` nullable=YES
- `inuse` `char` nullable=NO
- `Grade` `char` nullable=NO
- `EventChk` `char` nullable=NO
- `SelectChk` `char` nullable=NO
- `BlockChk` `char` nullable=NO
- `SpecialChk` `char` nullable=NO
- `ServerName` `varchar` nullable=YES
- `Credit` `char` nullable=NO
- `ECoin` `money` nullable=YES
- `StartDay` `datetime` nullable=YES
- `LastDay` `datetime` nullable=YES
- `EditDay` `datetime` nullable=YES
- `RNo` `int` nullable=YES
- `DelChk` `char` nullable=NO
- `SNo` `varchar` nullable=YES
- `Channel` `varchar` nullable=YES
- `BNum` `int` nullable=YES
- `MXServer` `varchar` nullable=YES
- `MXChar` `varchar` nullable=YES
- `MXType` `int` nullable=YES
- `MXLevel` `int` nullable=YES
- `MXExp` `bigint` nullable=YES

### AccountDB.dbo.GPersonalMember
- `PMNo` `int` nullable=NO
- `Userid` `varchar` nullable=NO
- `Passwd` `varchar` nullable=NO
- `Question` `varchar` nullable=YES
- `Reply` `varchar` nullable=YES
- `CUserName1` `varchar` nullable=YES
- `CUserName2` `varchar` nullable=YES
- `KUserName1` `varchar` nullable=YES
- `KUserName2` `varchar` nullable=YES
- `Age` `varchar` nullable=YES
- `Gender` `varchar` nullable=YES
- `Email` `varchar` nullable=YES
- `Job` `varchar` nullable=YES
- `ZipCode1` `varchar` nullable=YES
- `ZipCode2` `varchar` nullable=YES
- `Add1` `varchar` nullable=YES
- `Add2` `varchar` nullable=YES
- `Add3` `varchar` nullable=YES
- `PhoneNum1` `varchar` nullable=YES
- `PhoneNum2` `varchar` nullable=YES
- `PhoneNum3` `varchar` nullable=YES
- `EmailChk` `varchar` nullable=YES
- `DelChk` `varchar` nullable=YES
- `RegistDay` `datetime` nullable=YES
- `Channel` `varchar` nullable=YES
- `BNum` `int` nullable=YES
- `External_id` `varchar` nullable=YES
- `NickName` `varchar` nullable=YES
- `IPADDR` `varchar` nullable=YES

### AccountDB.dbo.GameMasters
- `id` `int` nullable=NO
- `accountId` `varchar` nullable=NO
- `password` `varchar` nullable=NO
- `security` `int` nullable=NO
- `active` `bit` nullable=NO

### AccountDB.dbo.HGameUser
- `userid` `varchar` nullable=NO
- `Passwd` `varchar` nullable=NO
- `GameCode` `varchar` nullable=YES
- `GPCode` `varchar` nullable=YES
- `RegistDay` `datetime` nullable=YES
- `DisuseDay` `datetime` nullable=YES
- `UsePeriod` `int` nullable=YES
- `inuse` `char` nullable=NO
- `Grade` `char` nullable=NO
- `EventChk` `char` nullable=NO
- `SelectChk` `char` nullable=NO
- `BlockChk` `char` nullable=NO
- `SpecialChk` `char` nullable=NO
- `ServerName` `varchar` nullable=YES
- `Credit` `char` nullable=NO
- `ECoin` `money` nullable=YES
- `StartDay` `datetime` nullable=YES
- `LastDay` `datetime` nullable=YES
- `EditDay` `datetime` nullable=YES
- `RNo` `int` nullable=YES
- `DelChk` `char` nullable=NO
- `SNo` `varchar` nullable=YES
- `Channel` `varchar` nullable=YES
- `BNum` `int` nullable=YES
- `MXServer` `varchar` nullable=YES
- `MXChar` `varchar` nullable=YES
- `MXType` `int` nullable=YES
- `MXLevel` `int` nullable=YES
- `MXExp` `bigint` nullable=YES

### AccountDB.dbo.HPersonalMember
- `PMNo` `int` nullable=NO
- `Userid` `varchar` nullable=NO
- `Passwd` `varchar` nullable=NO
- `Question` `varchar` nullable=YES
- `Reply` `varchar` nullable=YES
- `CUserName1` `varchar` nullable=YES
- `CUserName2` `varchar` nullable=YES
- `KUserName1` `varchar` nullable=YES
- `KUserName2` `varchar` nullable=YES
- `Age` `varchar` nullable=YES
- `Gender` `varchar` nullable=YES
- `Email` `varchar` nullable=YES
- `Job` `varchar` nullable=YES
- `ZipCode1` `varchar` nullable=YES
- `ZipCode2` `varchar` nullable=YES
- `Add1` `varchar` nullable=YES
- `Add2` `varchar` nullable=YES
- `Add3` `varchar` nullable=YES
- `PhoneNum1` `varchar` nullable=YES
- `PhoneNum2` `varchar` nullable=YES
- `PhoneNum3` `varchar` nullable=YES
- `EmailChk` `varchar` nullable=YES
- `DelChk` `varchar` nullable=YES
- `RegistDay` `datetime` nullable=YES
- `Channel` `varchar` nullable=YES
- `BNum` `int` nullable=YES
- `External_id` `varchar` nullable=YES
- `NickName` `varchar` nullable=YES
- `IPADDR` `varchar` nullable=YES

### AccountDB.dbo.IGameUser
- `userid` `varchar` nullable=NO
- `Passwd` `varchar` nullable=NO
- `GameCode` `varchar` nullable=YES
- `GPCode` `varchar` nullable=YES
- `RegistDay` `datetime` nullable=YES
- `DisuseDay` `datetime` nullable=YES
- `UsePeriod` `int` nullable=YES
- `inuse` `char` nullable=NO
- `Grade` `char` nullable=NO
- `EventChk` `char` nullable=NO
- `SelectChk` `char` nullable=NO
- `BlockChk` `char` nullable=NO
- `SpecialChk` `char` nullable=NO
- `ServerName` `varchar` nullable=YES
- `Credit` `char` nullable=NO
- `ECoin` `money` nullable=YES
- `StartDay` `datetime` nullable=YES
- `LastDay` `datetime` nullable=YES
- `EditDay` `datetime` nullable=YES
- `RNo` `int` nullable=YES
- `DelChk` `char` nullable=NO
- `SNo` `varchar` nullable=YES
- `Channel` `varchar` nullable=YES
- `BNum` `int` nullable=YES
- `MXServer` `varchar` nullable=YES
- `MXChar` `varchar` nullable=YES
- `MXType` `int` nullable=YES
- `MXLevel` `int` nullable=YES
- `MXExp` `bigint` nullable=YES

### AccountDB.dbo.IPersonalMember
- `PMNo` `int` nullable=NO
- `Userid` `varchar` nullable=NO
- `Passwd` `varchar` nullable=NO
- `Question` `varchar` nullable=YES
- `Reply` `varchar` nullable=YES
- `CUserName1` `varchar` nullable=YES
- `CUserName2` `varchar` nullable=YES
- `KUserName1` `varchar` nullable=YES
- `KUserName2` `varchar` nullable=YES
- `Age` `varchar` nullable=YES
- `Gender` `varchar` nullable=YES
- `Email` `varchar` nullable=YES
- `Job` `varchar` nullable=YES
- `ZipCode1` `varchar` nullable=YES
- `ZipCode2` `varchar` nullable=YES
- `Add1` `varchar` nullable=YES
- `Add2` `varchar` nullable=YES
- `Add3` `varchar` nullable=YES
- `PhoneNum1` `varchar` nullable=YES
- `PhoneNum2` `varchar` nullable=YES
- `PhoneNum3` `varchar` nullable=YES
- `EmailChk` `varchar` nullable=YES
- `DelChk` `varchar` nullable=YES
- `RegistDay` `datetime` nullable=YES
- `Channel` `varchar` nullable=YES
- `BNum` `int` nullable=YES
- `External_id` `varchar` nullable=YES
- `NickName` `varchar` nullable=YES
- `IPADDR` `varchar` nullable=YES

### AccountDB.dbo.JGameUser
- `userid` `varchar` nullable=NO
- `Passwd` `varchar` nullable=NO
- `GameCode` `varchar` nullable=YES
- `GPCode` `varchar` nullable=YES
- `RegistDay` `datetime` nullable=YES
- `DisuseDay` `datetime` nullable=YES
- `UsePeriod` `int` nullable=YES
- `inuse` `char` nullable=NO
- `Grade` `char` nullable=NO
- `EventChk` `char` nullable=NO
- `SelectChk` `char` nullable=NO
- `BlockChk` `char` nullable=NO
- `SpecialChk` `char` nullable=NO
- `ServerName` `varchar` nullable=YES
- `Credit` `char` nullable=NO
- `ECoin` `money` nullable=YES
- `StartDay` `datetime` nullable=YES
- `LastDay` `datetime` nullable=YES
- `EditDay` `datetime` nullable=YES
- `RNo` `int` nullable=YES
- `DelChk` `char` nullable=NO
- `SNo` `varchar` nullable=YES
- `Channel` `varchar` nullable=YES
- `BNum` `int` nullable=YES
- `MXServer` `varchar` nullable=YES
- `MXChar` `varchar` nullable=YES
- `MXType` `int` nullable=YES
- `MXLevel` `int` nullable=YES
- `MXExp` `bigint` nullable=YES

### AccountDB.dbo.JPersonalMember
- `PMNo` `int` nullable=NO
- `Userid` `varchar` nullable=NO
- `Passwd` `varchar` nullable=NO
- `Question` `varchar` nullable=YES
- `Reply` `varchar` nullable=YES
- `CUserName1` `varchar` nullable=YES
- `CUserName2` `varchar` nullable=YES
- `KUserName1` `varchar` nullable=YES
- `KUserName2` `varchar` nullable=YES
- `Age` `varchar` nullable=YES
- `Gender` `varchar` nullable=YES
- `Email` `varchar` nullable=YES
- `Job` `varchar` nullable=YES
- `ZipCode1` `varchar` nullable=YES
- `ZipCode2` `varchar` nullable=YES
- `Add1` `varchar` nullable=YES
- `Add2` `varchar` nullable=YES
- `Add3` `varchar` nullable=YES
- `PhoneNum1` `varchar` nullable=YES
- `PhoneNum2` `varchar` nullable=YES
- `PhoneNum3` `varchar` nullable=YES
- `EmailChk` `varchar` nullable=YES
- `DelChk` `varchar` nullable=YES
- `RegistDay` `datetime` nullable=YES
- `Channel` `varchar` nullable=YES
- `BNum` `int` nullable=YES
- `External_id` `varchar` nullable=YES
- `NickName` `varchar` nullable=YES
- `IPADDR` `varchar` nullable=YES

### AccountDB.dbo.KGameUser
- `userid` `varchar` nullable=NO
- `Passwd` `varchar` nullable=NO
- `GameCode` `varchar` nullable=YES
- `GPCode` `varchar` nullable=YES
- `RegistDay` `datetime` nullable=YES
- `DisuseDay` `datetime` nullable=YES
- `UsePeriod` `int` nullable=YES
- `inuse` `char` nullable=NO
- `Grade` `char` nullable=NO
- `EventChk` `char` nullable=NO
- `SelectChk` `char` nullable=NO
- `BlockChk` `char` nullable=NO
- `SpecialChk` `char` nullable=NO
- `ServerName` `varchar` nullable=YES
- `Credit` `char` nullable=NO
- `ECoin` `money` nullable=YES
- `StartDay` `datetime` nullable=YES
- `LastDay` `datetime` nullable=YES
- `EditDay` `datetime` nullable=YES
- `RNo` `int` nullable=YES
- `DelChk` `char` nullable=NO
- `SNo` `varchar` nullable=YES
- `Channel` `varchar` nullable=YES
- `BNum` `int` nullable=YES
- `MXServer` `varchar` nullable=YES
- `MXChar` `varchar` nullable=YES
- `MXType` `int` nullable=YES
- `MXLevel` `int` nullable=YES
- `MXExp` `bigint` nullable=YES

### AccountDB.dbo.KPersonalMember
- `PMNo` `int` nullable=NO
- `Userid` `varchar` nullable=NO
- `Passwd` `varchar` nullable=NO
- `Question` `varchar` nullable=YES
- `Reply` `varchar` nullable=YES
- `CUserName1` `varchar` nullable=YES
- `CUserName2` `varchar` nullable=YES
- `KUserName1` `varchar` nullable=YES
- `KUserName2` `varchar` nullable=YES
- `Age` `varchar` nullable=YES
- `Gender` `varchar` nullable=YES
- `Email` `varchar` nullable=YES
- `Job` `varchar` nullable=YES
- `ZipCode1` `varchar` nullable=YES
- `ZipCode2` `varchar` nullable=YES
- `Add1` `varchar` nullable=YES
- `Add2` `varchar` nullable=YES
- `Add3` `varchar` nullable=YES
- `PhoneNum1` `varchar` nullable=YES
- `PhoneNum2` `varchar` nullable=YES
- `PhoneNum3` `varchar` nullable=YES
- `EmailChk` `varchar` nullable=YES
- `DelChk` `varchar` nullable=YES
- `RegistDay` `datetime` nullable=YES
- `Channel` `varchar` nullable=YES
- `BNum` `int` nullable=YES
- `External_id` `varchar` nullable=YES
- `NickName` `varchar` nullable=YES
- `IPADDR` `varchar` nullable=YES

### AccountDB.dbo.LGameUser
- `userid` `varchar` nullable=NO
- `Passwd` `varchar` nullable=NO
- `GameCode` `varchar` nullable=YES
- `GPCode` `varchar` nullable=YES
- `RegistDay` `datetime` nullable=YES
- `DisuseDay` `datetime` nullable=YES
- `UsePeriod` `int` nullable=YES
- `inuse` `char` nullable=NO
- `Grade` `char` nullable=NO
- `EventChk` `char` nullable=NO
- `SelectChk` `char` nullable=NO
- `BlockChk` `char` nullable=NO
- `SpecialChk` `char` nullable=NO
- `ServerName` `varchar` nullable=YES
- `Credit` `char` nullable=NO
- `ECoin` `money` nullable=YES
- `StartDay` `datetime` nullable=YES
- `LastDay` `datetime` nullable=YES
- `EditDay` `datetime` nullable=YES
- `RNo` `int` nullable=YES
- `DelChk` `char` nullable=NO
- `SNo` `varchar` nullable=YES
- `Channel` `varchar` nullable=YES
- `BNum` `int` nullable=YES
- `MXServer` `varchar` nullable=YES
- `MXChar` `varchar` nullable=YES
- `MXType` `int` nullable=YES
- `MXLevel` `int` nullable=YES
- `MXExp` `bigint` nullable=YES

### AccountDB.dbo.LPersonalMember
- `PMNo` `int` nullable=NO
- `Userid` `varchar` nullable=NO
- `Passwd` `varchar` nullable=NO
- `Question` `varchar` nullable=YES
- `Reply` `varchar` nullable=YES
- `CUserName1` `varchar` nullable=YES
- `CUserName2` `varchar` nullable=YES
- `KUserName1` `varchar` nullable=YES
- `KUserName2` `varchar` nullable=YES
- `Age` `varchar` nullable=YES
- `Gender` `varchar` nullable=YES
- `Email` `varchar` nullable=YES
- `Job` `varchar` nullable=YES
- `ZipCode1` `varchar` nullable=YES
- `ZipCode2` `varchar` nullable=YES
- `Add1` `varchar` nullable=YES
- `Add2` `varchar` nullable=YES
- `Add3` `varchar` nullable=YES
- `PhoneNum1` `varchar` nullable=YES
- `PhoneNum2` `varchar` nullable=YES
- `PhoneNum3` `varchar` nullable=YES
- `EmailChk` `varchar` nullable=YES
- `DelChk` `varchar` nullable=YES
- `RegistDay` `datetime` nullable=YES
- `Channel` `varchar` nullable=YES
- `BNum` `int` nullable=YES
- `External_id` `varchar` nullable=YES
- `NickName` `varchar` nullable=YES
- `IPADDR` `varchar` nullable=YES

### AccountDB.dbo.LevelList
- `CharName` `varchar` nullable=YES
- `CharLevel` `int` nullable=YES
- `CharClass` `varchar` nullable=YES
- `ID` `int` nullable=YES

### AccountDB.dbo.MGameUser
- `userid` `varchar` nullable=NO
- `Passwd` `varchar` nullable=NO
- `GameCode` `varchar` nullable=YES
- `GPCode` `varchar` nullable=YES
- `RegistDay` `datetime` nullable=YES
- `DisuseDay` `datetime` nullable=YES
- `UsePeriod` `int` nullable=YES
- `inuse` `char` nullable=NO
- `Grade` `char` nullable=NO
- `EventChk` `char` nullable=NO
- `SelectChk` `char` nullable=NO
- `BlockChk` `char` nullable=NO
- `SpecialChk` `char` nullable=NO
- `ServerName` `varchar` nullable=YES
- `Credit` `char` nullable=NO
- `ECoin` `money` nullable=YES
- `StartDay` `datetime` nullable=YES
- `LastDay` `datetime` nullable=YES
- `EditDay` `datetime` nullable=YES
- `RNo` `int` nullable=YES
- `DelChk` `char` nullable=NO
- `SNo` `varchar` nullable=YES
- `Channel` `varchar` nullable=YES
- `BNum` `int` nullable=YES
- `MXServer` `varchar` nullable=YES
- `MXChar` `varchar` nullable=YES
- `MXType` `int` nullable=YES
- `MXLevel` `int` nullable=YES
- `MXExp` `bigint` nullable=YES

### AccountDB.dbo.MK_User_DATA
- `IDX` `int` nullable=NO
- `UserID` `varchar` nullable=YES
- `UserPWD` `varchar` nullable=YES
- `NickName` `varchar` nullable=YES
- `EmailAddr` `varchar` nullable=YES
- `Sex` `char` nullable=YES
- `Address` `varchar` nullable=YES
- `Job` `varchar` nullable=YES
- `Age` `varchar` nullable=YES
- `EmailChk` `char` nullable=YES
- `DelChk` `char` nullable=YES

### AccountDB.dbo.MPersonalMember
- `PMNo` `int` nullable=NO
- `Userid` `varchar` nullable=NO
- `Passwd` `varchar` nullable=NO
- `Question` `varchar` nullable=YES
- `Reply` `varchar` nullable=YES
- `CUserName1` `varchar` nullable=YES
- `CUserName2` `varchar` nullable=YES
- `KUserName1` `varchar` nullable=YES
- `KUserName2` `varchar` nullable=YES
- `Age` `varchar` nullable=YES
- `Gender` `varchar` nullable=YES
- `Email` `varchar` nullable=YES
- `Job` `varchar` nullable=YES
- `ZipCode1` `varchar` nullable=YES
- `ZipCode2` `varchar` nullable=YES
- `Add1` `varchar` nullable=YES
- `Add2` `varchar` nullable=YES
- `Add3` `varchar` nullable=YES
- `PhoneNum1` `varchar` nullable=YES
- `PhoneNum2` `varchar` nullable=YES
- `PhoneNum3` `varchar` nullable=YES
- `EmailChk` `varchar` nullable=YES
- `DelChk` `varchar` nullable=YES
- `RegistDay` `datetime` nullable=YES
- `Channel` `varchar` nullable=YES
- `BNum` `int` nullable=YES
- `External_id` `varchar` nullable=YES
- `NickName` `varchar` nullable=YES
- `IPADDR` `varchar` nullable=YES

### AccountDB.dbo.NGameUser
- `userid` `varchar` nullable=NO
- `Passwd` `varchar` nullable=NO
- `GameCode` `varchar` nullable=YES
- `GPCode` `varchar` nullable=YES
- `RegistDay` `datetime` nullable=YES
- `DisuseDay` `datetime` nullable=YES
- `UsePeriod` `int` nullable=YES
- `inuse` `char` nullable=NO
- `Grade` `char` nullable=NO
- `EventChk` `char` nullable=NO
- `SelectChk` `char` nullable=NO
- `BlockChk` `char` nullable=NO
- `SpecialChk` `char` nullable=NO
- `ServerName` `varchar` nullable=YES
- `Credit` `char` nullable=NO
- `ECoin` `money` nullable=YES
- `StartDay` `datetime` nullable=YES
- `LastDay` `datetime` nullable=YES
- `EditDay` `datetime` nullable=YES
- `RNo` `int` nullable=YES
- `DelChk` `char` nullable=NO
- `SNo` `varchar` nullable=YES
- `Channel` `varchar` nullable=YES
- `BNum` `int` nullable=YES
- `MXServer` `varchar` nullable=YES
- `MXChar` `varchar` nullable=YES
- `MXType` `int` nullable=YES
- `MXLevel` `int` nullable=YES
- `MXExp` `bigint` nullable=YES

### AccountDB.dbo.NPersonalMember
- `PMNo` `int` nullable=NO
- `Userid` `varchar` nullable=NO
- `Passwd` `varchar` nullable=NO
- `Question` `varchar` nullable=YES
- `Reply` `varchar` nullable=YES
- `CUserName1` `varchar` nullable=YES
- `CUserName2` `varchar` nullable=YES
- `KUserName1` `varchar` nullable=YES
- `KUserName2` `varchar` nullable=YES
- `Age` `varchar` nullable=YES
- `Gender` `varchar` nullable=YES
- `Email` `varchar` nullable=YES
- `Job` `varchar` nullable=YES
- `ZipCode1` `varchar` nullable=YES
- `ZipCode2` `varchar` nullable=YES
- `Add1` `varchar` nullable=YES
- `Add2` `varchar` nullable=YES
- `Add3` `varchar` nullable=YES
- `PhoneNum1` `varchar` nullable=YES
- `PhoneNum2` `varchar` nullable=YES
- `PhoneNum3` `varchar` nullable=YES
- `EmailChk` `varchar` nullable=YES
- `DelChk` `varchar` nullable=YES
- `RegistDay` `datetime` nullable=YES
- `Channel` `varchar` nullable=YES
- `BNum` `int` nullable=YES
- `External_id` `varchar` nullable=YES
- `NickName` `varchar` nullable=YES
- `IPADDR` `varchar` nullable=YES

### AccountDB.dbo.NTTGameUser
- `userid` `varchar` nullable=NO
- `Passwd` `varchar` nullable=NO
- `GameCode` `varchar` nullable=YES
- `GPCode` `varchar` nullable=YES
- `RegistDay` `datetime` nullable=YES
- `DisuseDay` `datetime` nullable=YES
- `UsePeriod` `int` nullable=YES
- `inuse` `char` nullable=NO
- `Grade` `char` nullable=NO
- `EventChk` `char` nullable=NO
- `SelectChk` `char` nullable=NO
- `BlockChk` `char` nullable=NO
- `SpecialChk` `char` nullable=NO
- `ServerName` `varchar` nullable=YES
- `Credit` `char` nullable=NO
- `ECoin` `money` nullable=YES
- `StartDay` `datetime` nullable=YES
- `LastDay` `datetime` nullable=YES
- `EditDay` `datetime` nullable=YES
- `RNo` `int` nullable=YES
- `DelChk` `char` nullable=NO
- `SNo` `varchar` nullable=YES
- `Channel` `varchar` nullable=YES
- `BNum` `int` nullable=YES

### AccountDB.dbo.NTTPersonalMember
- `PMNo` `int` nullable=NO
- `Userid` `varchar` nullable=NO
- `Passwd` `varchar` nullable=NO
- `Question` `varchar` nullable=YES
- `Reply` `varchar` nullable=YES
- `CUserName1` `varchar` nullable=YES
- `CUserName2` `varchar` nullable=YES
- `KUserName1` `varchar` nullable=YES
- `KUserName2` `varchar` nullable=YES
- `Age` `varchar` nullable=YES
- `Gender` `varchar` nullable=YES
- `Email` `varchar` nullable=YES
- `Job` `varchar` nullable=YES
- `ZipCode1` `varchar` nullable=YES
- `ZipCode2` `varchar` nullable=YES
- `Add1` `varchar` nullable=YES
- `Add2` `varchar` nullable=YES
- `Add3` `varchar` nullable=YES
- `PhoneNum1` `varchar` nullable=YES
- `PhoneNum2` `varchar` nullable=YES
- `PhoneNum3` `varchar` nullable=YES
- `EmailChk` `varchar` nullable=YES
- `DelChk` `varchar` nullable=YES
- `RegistDay` `datetime` nullable=YES
- `Channel` `varchar` nullable=YES
- `BNum` `int` nullable=YES
- `External_id` `varchar` nullable=YES
- `Flets_type` `varchar` nullable=YES
- `Flets_config` `varchar` nullable=YES
- `Flets_Multisession` `varchar` nullable=YES
- `Flets_ConnectCourse` `char` nullable=YES

### AccountDB.dbo.OGameUser
- `userid` `varchar` nullable=NO
- `Passwd` `varchar` nullable=NO
- `GameCode` `varchar` nullable=YES
- `GPCode` `varchar` nullable=YES
- `RegistDay` `datetime` nullable=YES
- `DisuseDay` `datetime` nullable=YES
- `UsePeriod` `int` nullable=YES
- `inuse` `char` nullable=NO
- `Grade` `char` nullable=NO
- `EventChk` `char` nullable=NO
- `SelectChk` `char` nullable=NO
- `BlockChk` `char` nullable=NO
- `SpecialChk` `char` nullable=NO
- `ServerName` `varchar` nullable=YES
- `Credit` `char` nullable=NO
- `ECoin` `money` nullable=YES
- `StartDay` `datetime` nullable=YES
- `LastDay` `datetime` nullable=YES
- `EditDay` `datetime` nullable=YES
- `RNo` `int` nullable=YES
- `DelChk` `char` nullable=NO
- `SNo` `varchar` nullable=YES
- `Channel` `varchar` nullable=YES
- `BNum` `int` nullable=YES
- `MXServer` `varchar` nullable=YES
- `MXChar` `varchar` nullable=YES
- `MXType` `int` nullable=YES
- `MXLevel` `int` nullable=YES
- `MXExp` `bigint` nullable=YES

### AccountDB.dbo.OPersonalMember
- `PMNo` `int` nullable=NO
- `Userid` `varchar` nullable=NO
- `Passwd` `varchar` nullable=NO
- `Question` `varchar` nullable=YES
- `Reply` `varchar` nullable=YES
- `CUserName1` `varchar` nullable=YES
- `CUserName2` `varchar` nullable=YES
- `KUserName1` `varchar` nullable=YES
- `KUserName2` `varchar` nullable=YES
- `Age` `varchar` nullable=YES
- `Gender` `varchar` nullable=YES
- `Email` `varchar` nullable=YES
- `Job` `varchar` nullable=YES
- `ZipCode1` `varchar` nullable=YES
- `ZipCode2` `varchar` nullable=YES
- `Add1` `varchar` nullable=YES
- `Add2` `varchar` nullable=YES
- `Add3` `varchar` nullable=YES
- `PhoneNum1` `varchar` nullable=YES
- `PhoneNum2` `varchar` nullable=YES
- `PhoneNum3` `varchar` nullable=YES
- `EmailChk` `varchar` nullable=YES
- `DelChk` `varchar` nullable=YES
- `RegistDay` `datetime` nullable=YES
- `Channel` `varchar` nullable=YES
- `BNum` `int` nullable=YES
- `External_id` `varchar` nullable=YES
- `NickName` `varchar` nullable=YES
- `IPADDR` `varchar` nullable=YES

### AccountDB.dbo.PCBangMember
- `PMNo` `int` nullable=NO
- `UserID` `varchar` nullable=NO
- `Passwd` `varchar` nullable=NO
- `BusinessName` `varchar` nullable=NO
- `UserName` `varchar` nullable=NO
- `PhoneNumber` `varchar` nullable=NO
- `ZipCode` `char` nullable=NO
- `Address1` `varchar` nullable=NO
- `Address2` `varchar` nullable=NO
- `Email` `varchar` nullable=NO
- `TIPAddress1_G` `varchar` nullable=YES
- `TIP1_GComment` `varchar` nullable=YES
- `TIPAddress2_G` `varchar` nullable=YES
- `TIP2_GComment` `varchar` nullable=YES
- `TIPAddress3_G` `varchar` nullable=YES
- `TIP3_GComment` `varchar` nullable=YES
- `TIPAddress1` `varchar` nullable=YES
- `TIP1_Comment` `varchar` nullable=YES
- `TIPAddress2` `varchar` nullable=YES
- `TIP2_Comment` `varchar` nullable=YES
- `TIPAddress3` `varchar` nullable=YES
- `TIP3_Comment` `varchar` nullable=YES
- `TIPAddress4` `varchar` nullable=YES
- `TIP4_Comment` `varchar` nullable=YES
- `TIPAddress5` `varchar` nullable=YES
- `TIP5_Comment` `varchar` nullable=YES
- `TIPAddress6` `varchar` nullable=YES
- `TIP6_Comment` `varchar` nullable=YES
- `TIPAddress7` `varchar` nullable=YES
- `TIP7_Comment` `varchar` nullable=YES
- `TIPAddress8` `varchar` nullable=YES
- `TIP8_Comment` `varchar` nullable=YES
- `TIPAddress9` `varchar` nullable=YES
- `TIP9_Comment` `varchar` nullable=YES
- `TIPAddress10` `varchar` nullable=YES
- `TIP10_Comment` `varchar` nullable=YES
- `RegistDay` `datetime` nullable=NO
- `RegionCode` `char` nullable=NO
- `Grade` `char` nullable=NO
- `Note` `varchar` nullable=YES
- `UseCheck` `char` nullable=YES

### AccountDB.dbo.PCBangMemberChangeLog
- `PCBCLNo` `int` nullable=NO
- `PMNo` `int` nullable=NO
- `UserID` `varchar` nullable=NO
- `Passwd` `varchar` nullable=NO
- `BusinessName` `varchar` nullable=NO
- `UserName` `varchar` nullable=NO
- `PhoneNumber` `varchar` nullable=NO
- `ZipCode` `char` nullable=NO
- `Address1` `varchar` nullable=NO
- `Address2` `varchar` nullable=NO
- `Email` `varchar` nullable=NO
- `TIPAddress1_G` `varchar` nullable=YES
- `TIP1_GComment` `varchar` nullable=YES
- `TIPAddress2_G` `varchar` nullable=YES
- `TIP2_GComment` `varchar` nullable=YES
- `TIPAddress3_G` `varchar` nullable=YES
- `TIP3_GComment` `varchar` nullable=YES
- `TIPAddress1` `varchar` nullable=YES
- `TIP1_Comment` `varchar` nullable=YES
- `TIPAddress2` `varchar` nullable=YES
- `TIP2_Comment` `varchar` nullable=YES
- `TIPAddress3` `varchar` nullable=YES
- `TIP3_Comment` `varchar` nullable=YES
- `TIPAddress4` `varchar` nullable=YES
- `TIP4_Comment` `varchar` nullable=YES
- `TIPAddress5` `varchar` nullable=YES
- `TIP5_Comment` `varchar` nullable=YES
- `TIPAddress6` `varchar` nullable=YES
- `TIP6_Comment` `varchar` nullable=YES
- `TIPAddress7` `varchar` nullable=YES
- `TIP7_Comment` `varchar` nullable=YES
- `TIPAddress8` `varchar` nullable=YES
- `TIP8_Comment` `varchar` nullable=YES
- `TIPAddress9` `varchar` nullable=YES
- `TIP9_Comment` `varchar` nullable=YES
- `TIPAddress10` `varchar` nullable=YES
- `TIP10_Comment` `varchar` nullable=YES
- `RegistDay` `datetime` nullable=NO
- `RegionCode` `char` nullable=NO
- `Grade` `char` nullable=NO
- `Note` `varchar` nullable=YES
- `ChangeOperator` `varchar` nullable=NO
- `ChangeIP` `varchar` nullable=NO
- `EditDate` `datetime` nullable=NO
- `ChangeNote` `varchar` nullable=YES

### AccountDB.dbo.PGameUser
- `userid` `varchar` nullable=NO
- `Passwd` `varchar` nullable=NO
- `GameCode` `varchar` nullable=YES
- `GPCode` `varchar` nullable=YES
- `RegistDay` `datetime` nullable=YES
- `DisuseDay` `datetime` nullable=YES
- `UsePeriod` `int` nullable=YES
- `inuse` `char` nullable=NO
- `Grade` `char` nullable=NO
- `EventChk` `char` nullable=NO
- `SelectChk` `char` nullable=NO
- `BlockChk` `char` nullable=NO
- `SpecialChk` `char` nullable=NO
- `ServerName` `varchar` nullable=YES
- `Credit` `char` nullable=NO
- `ECoin` `money` nullable=YES
- `StartDay` `datetime` nullable=YES
- `LastDay` `datetime` nullable=YES
- `EditDay` `datetime` nullable=YES
- `RNo` `int` nullable=YES
- `DelChk` `char` nullable=NO
- `SNo` `varchar` nullable=YES
- `Channel` `varchar` nullable=YES
- `BNum` `int` nullable=YES
- `MXServer` `varchar` nullable=YES
- `MXChar` `varchar` nullable=YES
- `MXType` `int` nullable=YES
- `MXLevel` `int` nullable=YES
- `MXExp` `bigint` nullable=YES

### AccountDB.dbo.PPersonalMember
- `PMNo` `int` nullable=NO
- `Userid` `varchar` nullable=NO
- `Passwd` `varchar` nullable=NO
- `Question` `varchar` nullable=YES
- `Reply` `varchar` nullable=YES
- `CUserName1` `varchar` nullable=YES
- `CUserName2` `varchar` nullable=YES
- `KUserName1` `varchar` nullable=YES
- `KUserName2` `varchar` nullable=YES
- `Age` `varchar` nullable=YES
- `Gender` `varchar` nullable=YES
- `Email` `varchar` nullable=YES
- `Job` `varchar` nullable=YES
- `ZipCode1` `varchar` nullable=YES
- `ZipCode2` `varchar` nullable=YES
- `Add1` `varchar` nullable=YES
- `Add2` `varchar` nullable=YES
- `Add3` `varchar` nullable=YES
- `PhoneNum1` `varchar` nullable=YES
- `PhoneNum2` `varchar` nullable=YES
- `PhoneNum3` `varchar` nullable=YES
- `EmailChk` `varchar` nullable=YES
- `DelChk` `varchar` nullable=YES
- `RegistDay` `datetime` nullable=YES
- `Channel` `varchar` nullable=YES
- `BNum` `int` nullable=YES
- `External_id` `varchar` nullable=YES
- `NickName` `varchar` nullable=YES
- `IPADDR` `varchar` nullable=YES

### AccountDB.dbo.QGameUser
- `userid` `varchar` nullable=NO
- `Passwd` `varchar` nullable=NO
- `GameCode` `varchar` nullable=YES
- `GPCode` `varchar` nullable=YES
- `RegistDay` `datetime` nullable=YES
- `DisuseDay` `datetime` nullable=YES
- `UsePeriod` `int` nullable=YES
- `inuse` `char` nullable=NO
- `Grade` `char` nullable=NO
- `EventChk` `char` nullable=NO
- `SelectChk` `char` nullable=NO
- `BlockChk` `char` nullable=NO
- `SpecialChk` `char` nullable=NO
- `ServerName` `varchar` nullable=YES
- `Credit` `char` nullable=NO
- `ECoin` `money` nullable=YES
- `StartDay` `datetime` nullable=YES
- `LastDay` `datetime` nullable=YES
- `EditDay` `datetime` nullable=YES
- `RNo` `int` nullable=YES
- `DelChk` `char` nullable=NO
- `SNo` `varchar` nullable=YES
- `Channel` `varchar` nullable=YES
- `BNum` `int` nullable=YES
- `MXServer` `varchar` nullable=YES
- `MXChar` `varchar` nullable=YES
- `MXType` `int` nullable=YES
- `MXLevel` `int` nullable=YES
- `MXExp` `bigint` nullable=YES

### AccountDB.dbo.QPersonalMember
- `PMNo` `int` nullable=NO
- `Userid` `varchar` nullable=NO
- `Passwd` `varchar` nullable=NO
- `Question` `varchar` nullable=YES
- `Reply` `varchar` nullable=YES
- `CUserName1` `varchar` nullable=YES
- `CUserName2` `varchar` nullable=YES
- `KUserName1` `varchar` nullable=YES
- `KUserName2` `varchar` nullable=YES
- `Age` `varchar` nullable=YES
- `Gender` `varchar` nullable=YES
- `Email` `varchar` nullable=YES
- `Job` `varchar` nullable=YES
- `ZipCode1` `varchar` nullable=YES
- `ZipCode2` `varchar` nullable=YES
- `Add1` `varchar` nullable=YES
- `Add2` `varchar` nullable=YES
- `Add3` `varchar` nullable=YES
- `PhoneNum1` `varchar` nullable=YES
- `PhoneNum2` `varchar` nullable=YES
- `PhoneNum3` `varchar` nullable=YES
- `EmailChk` `varchar` nullable=YES
- `DelChk` `varchar` nullable=YES
- `RegistDay` `datetime` nullable=YES
- `Channel` `varchar` nullable=YES
- `BNum` `int` nullable=YES
- `External_id` `varchar` nullable=YES
- `NickName` `varchar` nullable=YES
- `IPADDR` `varchar` nullable=YES

### AccountDB.dbo.RGameUser
- `userid` `varchar` nullable=NO
- `Passwd` `varchar` nullable=NO
- `GameCode` `varchar` nullable=YES
- `GPCode` `varchar` nullable=YES
- `RegistDay` `datetime` nullable=YES
- `DisuseDay` `datetime` nullable=YES
- `UsePeriod` `int` nullable=YES
- `inuse` `char` nullable=NO
- `Grade` `char` nullable=NO
- `EventChk` `char` nullable=NO
- `SelectChk` `char` nullable=NO
- `BlockChk` `char` nullable=NO
- `SpecialChk` `char` nullable=NO
- `ServerName` `varchar` nullable=YES
- `Credit` `char` nullable=NO
- `ECoin` `money` nullable=YES
- `StartDay` `datetime` nullable=YES
- `LastDay` `datetime` nullable=YES
- `EditDay` `datetime` nullable=YES
- `RNo` `int` nullable=YES
- `DelChk` `char` nullable=NO
- `SNo` `varchar` nullable=YES
- `Channel` `varchar` nullable=YES
- `BNum` `int` nullable=YES
- `MXServer` `varchar` nullable=YES
- `MXChar` `varchar` nullable=YES
- `MXType` `int` nullable=YES
- `MXLevel` `int` nullable=YES
- `MXExp` `bigint` nullable=YES

### AccountDB.dbo.RPersonalMember
- `PMNo` `int` nullable=NO
- `Userid` `varchar` nullable=NO
- `Passwd` `varchar` nullable=NO
- `Question` `varchar` nullable=YES
- `Reply` `varchar` nullable=YES
- `CUserName1` `varchar` nullable=YES
- `CUserName2` `varchar` nullable=YES
- `KUserName1` `varchar` nullable=YES
- `KUserName2` `varchar` nullable=YES
- `Age` `varchar` nullable=YES
- `Gender` `varchar` nullable=YES
- `Email` `varchar` nullable=YES
- `Job` `varchar` nullable=YES
- `ZipCode1` `varchar` nullable=YES
- `ZipCode2` `varchar` nullable=YES
- `Add1` `varchar` nullable=YES
- `Add2` `varchar` nullable=YES
- `Add3` `varchar` nullable=YES
- `PhoneNum1` `varchar` nullable=YES
- `PhoneNum2` `varchar` nullable=YES
- `PhoneNum3` `varchar` nullable=YES
- `EmailChk` `varchar` nullable=YES
- `DelChk` `varchar` nullable=YES
- `RegistDay` `datetime` nullable=YES
- `Channel` `varchar` nullable=YES
- `BNum` `int` nullable=YES
- `External_id` `varchar` nullable=YES
- `NickName` `varchar` nullable=YES
- `IPADDR` `varchar` nullable=YES

### AccountDB.dbo.RegionChangeLog
- `RCLNo` `int` nullable=NO
- `PMNo` `int` nullable=NO
- `UserID` `varchar` nullable=NO
- `BeforeRegionCode` `char` nullable=NO
- `AfterRegionCode` `char` nullable=NO
- `ChangeOperator` `varchar` nullable=NO
- `ChangeIP` `varchar` nullable=NO
- `EditDate` `datetime` nullable=NO
- `ChangeNote` `varchar` nullable=YES

### AccountDB.dbo.RegionCodeChangeLog
- `RCCNo` `int` nullable=NO
- `RCNo` `int` nullable=NO
- `RegionCode` `char` nullable=NO
- `RegionCodeName` `varchar` nullable=NO
- `Active` `char` nullable=NO
- `EditDate` `datetime` nullable=NO
- `ChangeStatus` `char` nullable=NO
- `ChangeOperator` `varchar` nullable=NO
- `ChangeIP` `varchar` nullable=NO
- `ChangeDate` `datetime` nullable=NO
- `ChangeNote` `varchar` nullable=YES

### AccountDB.dbo.RegionCodeInfo
- `RCNo` `int` nullable=NO
- `RegionCode` `char` nullable=NO
- `RegionCodeName` `varchar` nullable=NO
- `Active` `char` nullable=NO
- `EditDate` `datetime` nullable=NO

### AccountDB.dbo.RegionCodeInfoLog
- `IDX` `int` nullable=NO
- `PMNo` `int` nullable=NO
- `UserID` `varchar` nullable=NO
- `Passwd` `varchar` nullable=NO
- `BusinessName` `varchar` nullable=YES
- `UserName` `varchar` nullable=YES
- `PhoneNumber` `varchar` nullable=YES
- `ZipCode` `char` nullable=YES
- `Address1` `varchar` nullable=YES
- `Address2` `varchar` nullable=YES
- `Email` `varchar` nullable=YES
- `RegistDay` `datetime` nullable=YES
- `RegionCode` `char` nullable=YES
- `Note` `varchar` nullable=YES
- `UseCheck` `char` nullable=YES

### AccountDB.dbo.SGameUser
- `userid` `varchar` nullable=NO
- `Passwd` `varchar` nullable=NO
- `GameCode` `varchar` nullable=YES
- `GPCode` `varchar` nullable=YES
- `RegistDay` `datetime` nullable=YES
- `DisuseDay` `datetime` nullable=YES
- `UsePeriod` `int` nullable=YES
- `inuse` `char` nullable=NO
- `Grade` `char` nullable=NO
- `EventChk` `char` nullable=NO
- `SelectChk` `char` nullable=NO
- `BlockChk` `char` nullable=NO
- `SpecialChk` `char` nullable=NO
- `ServerName` `varchar` nullable=YES
- `Credit` `char` nullable=NO
- `ECoin` `money` nullable=YES
- `StartDay` `datetime` nullable=YES
- `LastDay` `datetime` nullable=YES
- `EditDay` `datetime` nullable=YES
- `RNo` `int` nullable=YES
- `DelChk` `char` nullable=NO
- `SNo` `varchar` nullable=YES
- `Channel` `varchar` nullable=YES
- `BNum` `int` nullable=YES
- `MXServer` `varchar` nullable=YES
- `MXChar` `varchar` nullable=YES
- `MXType` `int` nullable=YES
- `MXLevel` `int` nullable=YES
- `MXExp` `bigint` nullable=YES

### AccountDB.dbo.SPersonalMember
- `PMNo` `int` nullable=NO
- `Userid` `varchar` nullable=NO
- `Passwd` `varchar` nullable=NO
- `Question` `varchar` nullable=YES
- `Reply` `varchar` nullable=YES
- `CUserName1` `varchar` nullable=YES
- `CUserName2` `varchar` nullable=YES
- `KUserName1` `varchar` nullable=YES
- `KUserName2` `varchar` nullable=YES
- `Age` `varchar` nullable=YES
- `Gender` `varchar` nullable=YES
- `Email` `varchar` nullable=YES
- `Job` `varchar` nullable=YES
- `ZipCode1` `varchar` nullable=YES
- `ZipCode2` `varchar` nullable=YES
- `Add1` `varchar` nullable=YES
- `Add2` `varchar` nullable=YES
- `Add3` `varchar` nullable=YES
- `PhoneNum1` `varchar` nullable=YES
- `PhoneNum2` `varchar` nullable=YES
- `PhoneNum3` `varchar` nullable=YES
- `EmailChk` `varchar` nullable=YES
- `DelChk` `varchar` nullable=YES
- `RegistDay` `datetime` nullable=YES
- `Channel` `varchar` nullable=YES
- `BNum` `int` nullable=YES
- `External_id` `varchar` nullable=YES
- `NickName` `varchar` nullable=YES
- `IPADDR` `varchar` nullable=YES

### AccountDB.dbo.TGameUser
- `userid` `varchar` nullable=NO
- `Passwd` `varchar` nullable=NO
- `GameCode` `varchar` nullable=YES
- `GPCode` `varchar` nullable=YES
- `RegistDay` `datetime` nullable=YES
- `DisuseDay` `datetime` nullable=YES
- `UsePeriod` `int` nullable=YES
- `inuse` `char` nullable=NO
- `Grade` `char` nullable=NO
- `EventChk` `char` nullable=NO
- `SelectChk` `char` nullable=NO
- `BlockChk` `char` nullable=NO
- `SpecialChk` `char` nullable=NO
- `ServerName` `varchar` nullable=YES
- `Credit` `char` nullable=NO
- `ECoin` `money` nullable=YES
- `StartDay` `datetime` nullable=YES
- `LastDay` `datetime` nullable=YES
- `EditDay` `datetime` nullable=YES
- `RNo` `int` nullable=YES
- `DelChk` `char` nullable=NO
- `SNo` `varchar` nullable=YES
- `Channel` `varchar` nullable=YES
- `BNum` `int` nullable=YES
- `MXServer` `varchar` nullable=YES
- `MXChar` `varchar` nullable=YES
- `MXType` `int` nullable=YES
- `MXLevel` `int` nullable=YES
- `MXExp` `bigint` nullable=YES

### AccountDB.dbo.TPersonalMember
- `PMNo` `int` nullable=NO
- `Userid` `varchar` nullable=NO
- `Passwd` `varchar` nullable=NO
- `Question` `varchar` nullable=YES
- `Reply` `varchar` nullable=YES
- `CUserName1` `varchar` nullable=YES
- `CUserName2` `varchar` nullable=YES
- `KUserName1` `varchar` nullable=YES
- `KUserName2` `varchar` nullable=YES
- `Age` `varchar` nullable=YES
- `Gender` `varchar` nullable=YES
- `Email` `varchar` nullable=YES
- `Job` `varchar` nullable=YES
- `ZipCode1` `varchar` nullable=YES
- `ZipCode2` `varchar` nullable=YES
- `Add1` `varchar` nullable=YES
- `Add2` `varchar` nullable=YES
- `Add3` `varchar` nullable=YES
- `PhoneNum1` `varchar` nullable=YES
- `PhoneNum2` `varchar` nullable=YES
- `PhoneNum3` `varchar` nullable=YES
- `EmailChk` `varchar` nullable=YES
- `DelChk` `varchar` nullable=YES
- `RegistDay` `datetime` nullable=YES
- `Channel` `varchar` nullable=YES
- `BNum` `int` nullable=YES
- `External_id` `varchar` nullable=YES
- `NickName` `varchar` nullable=YES
- `IPADDR` `varchar` nullable=YES

### AccountDB.dbo.UGameUser
- `userid` `varchar` nullable=NO
- `Passwd` `varchar` nullable=NO
- `GameCode` `varchar` nullable=YES
- `GPCode` `varchar` nullable=YES
- `RegistDay` `datetime` nullable=YES
- `DisuseDay` `datetime` nullable=YES
- `UsePeriod` `int` nullable=YES
- `inuse` `char` nullable=NO
- `Grade` `char` nullable=NO
- `EventChk` `char` nullable=NO
- `SelectChk` `char` nullable=NO
- `BlockChk` `char` nullable=NO
- `SpecialChk` `char` nullable=NO
- `ServerName` `varchar` nullable=YES
- `Credit` `char` nullable=NO
- `ECoin` `money` nullable=YES
- `StartDay` `datetime` nullable=YES
- `LastDay` `datetime` nullable=YES
- `EditDay` `datetime` nullable=YES
- `RNo` `int` nullable=YES
- `DelChk` `char` nullable=NO
- `SNo` `varchar` nullable=YES
- `Channel` `varchar` nullable=YES
- `BNum` `int` nullable=YES
- `MXServer` `varchar` nullable=YES
- `MXChar` `varchar` nullable=YES
- `MXType` `int` nullable=YES
- `MXLevel` `int` nullable=YES
- `MXExp` `bigint` nullable=YES

### AccountDB.dbo.UPersonalMember
- `PMNo` `int` nullable=NO
- `Userid` `varchar` nullable=NO
- `Passwd` `varchar` nullable=NO
- `Question` `varchar` nullable=YES
- `Reply` `varchar` nullable=YES
- `CUserName1` `varchar` nullable=YES
- `CUserName2` `varchar` nullable=YES
- `KUserName1` `varchar` nullable=YES
- `KUserName2` `varchar` nullable=YES
- `Age` `varchar` nullable=YES
- `Gender` `varchar` nullable=YES
- `Email` `varchar` nullable=YES
- `Job` `varchar` nullable=YES
- `ZipCode1` `varchar` nullable=YES
- `ZipCode2` `varchar` nullable=YES
- `Add1` `varchar` nullable=YES
- `Add2` `varchar` nullable=YES
- `Add3` `varchar` nullable=YES
- `PhoneNum1` `varchar` nullable=YES
- `PhoneNum2` `varchar` nullable=YES
- `PhoneNum3` `varchar` nullable=YES
- `EmailChk` `varchar` nullable=YES
- `DelChk` `varchar` nullable=YES
- `RegistDay` `datetime` nullable=YES
- `Channel` `varchar` nullable=YES
- `BNum` `int` nullable=YES
- `External_id` `varchar` nullable=YES
- `NickName` `varchar` nullable=YES
- `IPADDR` `varchar` nullable=YES

### AccountDB.dbo.VGameUser
- `userid` `varchar` nullable=NO
- `Passwd` `varchar` nullable=NO
- `GameCode` `varchar` nullable=YES
- `GPCode` `varchar` nullable=YES
- `RegistDay` `datetime` nullable=YES
- `DisuseDay` `datetime` nullable=YES
- `UsePeriod` `int` nullable=YES
- `inuse` `char` nullable=NO
- `Grade` `char` nullable=NO
- `EventChk` `char` nullable=NO
- `SelectChk` `char` nullable=NO
- `BlockChk` `char` nullable=NO
- `SpecialChk` `char` nullable=NO
- `ServerName` `varchar` nullable=YES
- `Credit` `char` nullable=NO
- `ECoin` `money` nullable=YES
- `StartDay` `datetime` nullable=YES
- `LastDay` `datetime` nullable=YES
- `EditDay` `datetime` nullable=YES
- `RNo` `int` nullable=YES
- `DelChk` `char` nullable=NO
- `SNo` `varchar` nullable=YES
- `Channel` `varchar` nullable=YES
- `BNum` `int` nullable=YES
- `MXServer` `varchar` nullable=YES
- `MXChar` `varchar` nullable=YES
- `MXType` `int` nullable=YES
- `MXLevel` `int` nullable=YES
- `MXExp` `bigint` nullable=YES

### AccountDB.dbo.VPersonalMember
- `PMNo` `int` nullable=NO
- `Userid` `varchar` nullable=NO
- `Passwd` `varchar` nullable=NO
- `Question` `varchar` nullable=YES
- `Reply` `varchar` nullable=YES
- `CUserName1` `varchar` nullable=YES
- `CUserName2` `varchar` nullable=YES
- `KUserName1` `varchar` nullable=YES
- `KUserName2` `varchar` nullable=YES
- `Age` `varchar` nullable=YES
- `Gender` `varchar` nullable=YES
- `Email` `varchar` nullable=YES
- `Job` `varchar` nullable=YES
- `ZipCode1` `varchar` nullable=YES
- `ZipCode2` `varchar` nullable=YES
- `Add1` `varchar` nullable=YES
- `Add2` `varchar` nullable=YES
- `Add3` `varchar` nullable=YES
- `PhoneNum1` `varchar` nullable=YES
- `PhoneNum2` `varchar` nullable=YES
- `PhoneNum3` `varchar` nullable=YES
- `EmailChk` `varchar` nullable=YES
- `DelChk` `varchar` nullable=YES
- `RegistDay` `datetime` nullable=YES
- `Channel` `varchar` nullable=YES
- `BNum` `int` nullable=YES
- `External_id` `varchar` nullable=YES
- `NickName` `varchar` nullable=YES
- `IPADDR` `varchar` nullable=YES

### AccountDB.dbo.WGameUser
- `userid` `varchar` nullable=NO
- `Passwd` `varchar` nullable=NO
- `GameCode` `varchar` nullable=YES
- `GPCode` `varchar` nullable=YES
- `RegistDay` `datetime` nullable=YES
- `DisuseDay` `datetime` nullable=YES
- `UsePeriod` `int` nullable=YES
- `inuse` `char` nullable=NO
- `Grade` `char` nullable=NO
- `EventChk` `char` nullable=NO
- `SelectChk` `char` nullable=NO
- `BlockChk` `char` nullable=NO
- `SpecialChk` `char` nullable=NO
- `ServerName` `varchar` nullable=YES
- `Credit` `char` nullable=NO
- `ECoin` `money` nullable=YES
- `StartDay` `datetime` nullable=YES
- `LastDay` `datetime` nullable=YES
- `EditDay` `datetime` nullable=YES
- `RNo` `int` nullable=YES
- `DelChk` `char` nullable=NO
- `SNo` `varchar` nullable=YES
- `Channel` `varchar` nullable=YES
- `BNum` `int` nullable=YES
- `MXServer` `varchar` nullable=YES
- `MXChar` `varchar` nullable=YES
- `MXType` `int` nullable=YES
- `MXLevel` `int` nullable=YES
- `MXExp` `bigint` nullable=YES

### AccountDB.dbo.WPersonalMember
- `PMNo` `int` nullable=NO
- `Userid` `varchar` nullable=NO
- `Passwd` `varchar` nullable=NO
- `Question` `varchar` nullable=YES
- `Reply` `varchar` nullable=YES
- `CUserName1` `varchar` nullable=YES
- `CUserName2` `varchar` nullable=YES
- `KUserName1` `varchar` nullable=YES
- `KUserName2` `varchar` nullable=YES
- `Age` `varchar` nullable=YES
- `Gender` `varchar` nullable=YES
- `Email` `varchar` nullable=YES
- `Job` `varchar` nullable=YES
- `ZipCode1` `varchar` nullable=YES
- `ZipCode2` `varchar` nullable=YES
- `Add1` `varchar` nullable=YES
- `Add2` `varchar` nullable=YES
- `Add3` `varchar` nullable=YES
- `PhoneNum1` `varchar` nullable=YES
- `PhoneNum2` `varchar` nullable=YES
- `PhoneNum3` `varchar` nullable=YES
- `EmailChk` `varchar` nullable=YES
- `DelChk` `varchar` nullable=YES
- `RegistDay` `datetime` nullable=YES
- `Channel` `varchar` nullable=YES
- `BNum` `int` nullable=YES
- `External_id` `varchar` nullable=YES
- `NickName` `varchar` nullable=YES
- `IPADDR` `varchar` nullable=YES

### AccountDB.dbo.XGameUser
- `userid` `varchar` nullable=NO
- `Passwd` `varchar` nullable=NO
- `GameCode` `varchar` nullable=YES
- `GPCode` `varchar` nullable=YES
- `RegistDay` `datetime` nullable=YES
- `DisuseDay` `datetime` nullable=YES
- `UsePeriod` `int` nullable=YES
- `inuse` `char` nullable=NO
- `Grade` `char` nullable=NO
- `EventChk` `char` nullable=NO
- `SelectChk` `char` nullable=NO
- `BlockChk` `char` nullable=NO
- `SpecialChk` `char` nullable=NO
- `ServerName` `varchar` nullable=YES
- `Credit` `char` nullable=NO
- `ECoin` `money` nullable=YES
- `StartDay` `datetime` nullable=YES
- `LastDay` `datetime` nullable=YES
- `EditDay` `datetime` nullable=YES
- `RNo` `int` nullable=YES
- `DelChk` `char` nullable=NO
- `SNo` `varchar` nullable=YES
- `Channel` `varchar` nullable=YES
- `BNum` `int` nullable=YES
- `MXServer` `varchar` nullable=YES
- `MXChar` `varchar` nullable=YES
- `MXType` `int` nullable=YES
- `MXLevel` `int` nullable=YES
- `MXExp` `bigint` nullable=YES

### AccountDB.dbo.XPersonalMember
- `PMNo` `int` nullable=NO
- `Userid` `varchar` nullable=NO
- `Passwd` `varchar` nullable=NO
- `Question` `varchar` nullable=YES
- `Reply` `varchar` nullable=YES
- `CUserName1` `varchar` nullable=YES
- `CUserName2` `varchar` nullable=YES
- `KUserName1` `varchar` nullable=YES
- `KUserName2` `varchar` nullable=YES
- `Age` `varchar` nullable=YES
- `Gender` `varchar` nullable=YES
- `Email` `varchar` nullable=YES
- `Job` `varchar` nullable=YES
- `ZipCode1` `varchar` nullable=YES
- `ZipCode2` `varchar` nullable=YES
- `Add1` `varchar` nullable=YES
- `Add2` `varchar` nullable=YES
- `Add3` `varchar` nullable=YES
- `PhoneNum1` `varchar` nullable=YES
- `PhoneNum2` `varchar` nullable=YES
- `PhoneNum3` `varchar` nullable=YES
- `EmailChk` `varchar` nullable=YES
- `DelChk` `varchar` nullable=YES
- `RegistDay` `datetime` nullable=YES
- `Channel` `varchar` nullable=YES
- `BNum` `int` nullable=YES
- `External_id` `varchar` nullable=YES
- `NickName` `varchar` nullable=YES
- `IPADDR` `varchar` nullable=YES

### AccountDB.dbo.YGameUser
- `userid` `varchar` nullable=NO
- `Passwd` `varchar` nullable=NO
- `GameCode` `varchar` nullable=YES
- `GPCode` `varchar` nullable=YES
- `RegistDay` `datetime` nullable=YES
- `DisuseDay` `datetime` nullable=YES
- `UsePeriod` `int` nullable=YES
- `inuse` `char` nullable=NO
- `Grade` `char` nullable=NO
- `EventChk` `char` nullable=NO
- `SelectChk` `char` nullable=NO
- `BlockChk` `char` nullable=NO
- `SpecialChk` `char` nullable=NO
- `ServerName` `varchar` nullable=YES
- `Credit` `char` nullable=NO
- `ECoin` `money` nullable=YES
- `StartDay` `datetime` nullable=YES
- `LastDay` `datetime` nullable=YES
- `EditDay` `datetime` nullable=YES
- `RNo` `int` nullable=YES
- `DelChk` `char` nullable=NO
- `SNo` `varchar` nullable=YES
- `Channel` `varchar` nullable=YES
- `BNum` `int` nullable=YES
- `MXServer` `varchar` nullable=YES
- `MXChar` `varchar` nullable=YES
- `MXType` `int` nullable=YES
- `MXLevel` `int` nullable=YES
- `MXExp` `bigint` nullable=YES

### AccountDB.dbo.YPersonalMember
- `PMNo` `int` nullable=NO
- `Userid` `varchar` nullable=NO
- `Passwd` `varchar` nullable=NO
- `Question` `varchar` nullable=YES
- `Reply` `varchar` nullable=YES
- `CUserName1` `varchar` nullable=YES
- `CUserName2` `varchar` nullable=YES
- `KUserName1` `varchar` nullable=YES
- `KUserName2` `varchar` nullable=YES
- `Age` `varchar` nullable=YES
- `Gender` `varchar` nullable=YES
- `Email` `varchar` nullable=YES
- `Job` `varchar` nullable=YES
- `ZipCode1` `varchar` nullable=YES
- `ZipCode2` `varchar` nullable=YES
- `Add1` `varchar` nullable=YES
- `Add2` `varchar` nullable=YES
- `Add3` `varchar` nullable=YES
- `PhoneNum1` `varchar` nullable=YES
- `PhoneNum2` `varchar` nullable=YES
- `PhoneNum3` `varchar` nullable=YES
- `EmailChk` `varchar` nullable=YES
- `DelChk` `varchar` nullable=YES
- `RegistDay` `datetime` nullable=YES
- `Channel` `varchar` nullable=YES
- `BNum` `int` nullable=YES
- `External_id` `varchar` nullable=YES
- `NickName` `varchar` nullable=YES
- `IPADDR` `varchar` nullable=YES

### AccountDB.dbo.ZGameUser
- `userid` `varchar` nullable=NO
- `Passwd` `varchar` nullable=NO
- `GameCode` `varchar` nullable=YES
- `GPCode` `varchar` nullable=YES
- `RegistDay` `datetime` nullable=YES
- `DisuseDay` `datetime` nullable=YES
- `UsePeriod` `int` nullable=YES
- `inuse` `char` nullable=NO
- `Grade` `char` nullable=NO
- `EventChk` `char` nullable=NO
- `SelectChk` `char` nullable=NO
- `BlockChk` `char` nullable=NO
- `SpecialChk` `char` nullable=NO
- `ServerName` `varchar` nullable=YES
- `Credit` `char` nullable=NO
- `ECoin` `money` nullable=YES
- `StartDay` `datetime` nullable=YES
- `LastDay` `datetime` nullable=YES
- `EditDay` `datetime` nullable=YES
- `RNo` `int` nullable=YES
- `DelChk` `char` nullable=NO
- `SNo` `varchar` nullable=YES
- `Channel` `varchar` nullable=YES
- `BNum` `int` nullable=YES
- `MXServer` `varchar` nullable=YES
- `MXChar` `varchar` nullable=YES
- `MXType` `int` nullable=YES
- `MXLevel` `int` nullable=YES
- `MXExp` `bigint` nullable=YES

### AccountDB.dbo.ZPersonalMember
- `PMNo` `int` nullable=NO
- `Userid` `varchar` nullable=NO
- `Passwd` `varchar` nullable=NO
- `Question` `varchar` nullable=YES
- `Reply` `varchar` nullable=YES
- `CUserName1` `varchar` nullable=YES
- `CUserName2` `varchar` nullable=YES
- `KUserName1` `varchar` nullable=YES
- `KUserName2` `varchar` nullable=YES
- `Age` `varchar` nullable=YES
- `Gender` `varchar` nullable=YES
- `Email` `varchar` nullable=YES
- `Job` `varchar` nullable=YES
- `ZipCode1` `varchar` nullable=YES
- `ZipCode2` `varchar` nullable=YES
- `Add1` `varchar` nullable=YES
- `Add2` `varchar` nullable=YES
- `Add3` `varchar` nullable=YES
- `PhoneNum1` `varchar` nullable=YES
- `PhoneNum2` `varchar` nullable=YES
- `PhoneNum3` `varchar` nullable=YES
- `EmailChk` `varchar` nullable=YES
- `DelChk` `varchar` nullable=YES
- `RegistDay` `datetime` nullable=YES
- `Channel` `varchar` nullable=YES
- `BNum` `int` nullable=YES
- `External_id` `varchar` nullable=YES
- `NickName` `varchar` nullable=YES
- `IPADDR` `varchar` nullable=YES

## BattlePass

### BattlePass.dbo.BP_Info
- `ID` `varchar` nullable=YES
- `BP_Exp` `int` nullable=YES
- `BP_Level` `int` nullable=YES
- `BP_Info` `int` nullable=YES
- `BP_Resg1` `int` nullable=YES
- `BP_Resg2` `int` nullable=YES
- `BP_Resg3` `int` nullable=YES
- `BP_Resg4` `int` nullable=YES
- `BP_Resg5` `int` nullable=YES
- `BP_Resg6` `int` nullable=YES
- `BP_Resg7` `int` nullable=YES
- `BP_Resg8` `int` nullable=YES
- `BP_Resg9` `int` nullable=YES
- `BP_Resg10` `int` nullable=YES
- `BP_Resg11` `int` nullable=YES
- `BP_Resg12` `int` nullable=YES
- `BP_Resg13` `int` nullable=YES
- `BP_Resg14` `int` nullable=YES
- `BP_Resg15` `int` nullable=YES
- `BP_Resg16` `int` nullable=YES
- `BP_Resg17` `int` nullable=YES
- `BP_Resg18` `int` nullable=YES
- `BP_Resg19` `int` nullable=YES
- `BP_Resg20` `int` nullable=YES
- `BP_Resg21` `int` nullable=YES
- `BP_Resg22` `int` nullable=YES
- `BP_Resg23` `int` nullable=YES
- `BP_Resg24` `int` nullable=YES
- `BP_Resg25` `int` nullable=YES
- `BP_Resg26` `int` nullable=YES
- `BP_Resg27` `int` nullable=YES
- `BP_Resg28` `int` nullable=YES
- `BP_Resg29` `int` nullable=YES
- `BP_Resg30` `int` nullable=YES
- `BP_Resg31` `int` nullable=YES
- `BP_Resg32` `int` nullable=YES
- `BP_Resg33` `int` nullable=YES
- `BP_Resg34` `int` nullable=YES
- `BP_Resg35` `int` nullable=YES
- `BP_Resg36` `int` nullable=YES
- `BP_Resg37` `int` nullable=YES
- `BP_Resg38` `int` nullable=YES
- `BP_Resg39` `int` nullable=YES
- `BP_Resg40` `int` nullable=YES

### BattlePass.dbo.BP_Item
- `Tag` `int` nullable=YES
- `CodItem` `varchar` nullable=YES
- `Name` `varchar` nullable=YES
- `Level_Req` `int` nullable=YES
- `ItemQtd` `int` nullable=YES
- `ItemLock` `int` nullable=YES

### BattlePass.dbo.BP_XP
- `Mobs` `int` nullable=YES
- `PVP` `int` nullable=YES

## BillingDb

### BillingDb.dbo.0CHANNEL
- `PRISTONTALE_ID` `varchar` nullable=NO
- `TABLE_NAME` `varchar` nullable=NO
- `INSERT_DATE` `datetime` nullable=NO

### BillingDb.dbo.9CHANNEL
- `PRISTONTALE_ID` `varchar` nullable=NO
- `TABLE_NAME` `varchar` nullable=NO
- `INSERT_DATE` `datetime` nullable=NO

### BillingDb.dbo.ACHANNEL
- `PRISTONTALE_ID` `varchar` nullable=NO
- `TABLE_NAME` `varchar` nullable=NO
- `INSERT_DATE` `datetime` nullable=NO

### BillingDb.dbo.BCHANNEL
- `PRISTONTALE_ID` `varchar` nullable=NO
- `TABLE_NAME` `varchar` nullable=NO
- `INSERT_DATE` `datetime` nullable=NO

### BillingDb.dbo.BillingPointCheck20060329
- `IDX` `int` nullable=NO
- `UserID` `varchar` nullable=NO
- `EndDay` `datetime` nullable=NO

### BillingDb.dbo.CCHANNEL
- `PRISTONTALE_ID` `varchar` nullable=NO
- `TABLE_NAME` `varchar` nullable=NO
- `INSERT_DATE` `datetime` nullable=NO

### BillingDb.dbo.DCHANNEL
- `PRISTONTALE_ID` `varchar` nullable=NO
- `TABLE_NAME` `varchar` nullable=NO
- `INSERT_DATE` `datetime` nullable=NO

### BillingDb.dbo.ECHANNEL
- `PRISTONTALE_ID` `varchar` nullable=NO
- `TABLE_NAME` `varchar` nullable=NO
- `INSERT_DATE` `datetime` nullable=NO

### BillingDb.dbo.Excite
- `NUM` `int` nullable=NO
- `EXTERNAL_ID` `varchar` nullable=YES
- `PRISTONTALE_ID` `varchar` nullable=NO
- `GOODS_TYPE` `char` nullable=YES
- `CHARGE_TYPE` `varchar` nullable=YES
- `CHARGE_FORM` `char` nullable=YES
- `AMOUNT` `int` nullable=NO
- `START_DAY` `datetime` nullable=NO
- `END_DAY` `datetime` nullable=NO
- `LEFT_TIME` `int` nullable=YES
- `CHARGE_DAY` `datetime` nullable=NO
- `UPDATE_CHECK` `char` nullable=NO
- `NOTE` `varchar` nullable=YES

### BillingDb.dbo.Excite_Amount
- `NUM` `int` nullable=NO
- `IDX` `int` nullable=NO
- `EXTERNAL_ID` `varchar` nullable=YES
- `PRISTONTALE_ID` `varchar` nullable=NO
- `GOODS_TYPE` `char` nullable=YES
- `CHARGE_TYPE` `varchar` nullable=YES
- `CHARGE_FORM` `char` nullable=YES
- `AMOUNT` `int` nullable=NO
- `START_DAY` `datetime` nullable=NO
- `END_DAY` `datetime` nullable=NO
- `LEFT_TIME` `int` nullable=YES
- `CHARGE_DAY` `datetime` nullable=NO
- `UPDATE_CHECK` `char` nullable=NO
- `NOTE` `varchar` nullable=YES

### BillingDb.dbo.Excite_Bak
- `NUM` `int` nullable=NO
- `IDX` `int` nullable=NO
- `EXTERNAL_ID` `varchar` nullable=YES
- `PRISTONTALE_ID` `varchar` nullable=NO
- `GOODS_TYPE` `char` nullable=YES
- `CHARGE_TYPE` `varchar` nullable=YES
- `CHARGE_FORM` `char` nullable=YES
- `AMOUNT` `int` nullable=NO
- `START_DAY` `datetime` nullable=NO
- `END_DAY` `datetime` nullable=NO
- `LEFT_TIME` `int` nullable=YES
- `CHARGE_DAY` `datetime` nullable=NO
- `UPDATE_CHECK` `char` nullable=NO
- `NOTE` `varchar` nullable=YES

### BillingDb.dbo.Excite_Error
- `NUM` `int` nullable=NO
- `IDX` `int` nullable=NO
- `EXTERNAL_ID` `varchar` nullable=YES
- `PRISTONTALE_ID` `varchar` nullable=NO
- `GOODS_TYPE` `char` nullable=YES
- `CHARGE_TYPE` `varchar` nullable=YES
- `CHARGE_FORM` `char` nullable=YES
- `AMOUNT` `int` nullable=NO
- `START_DAY` `datetime` nullable=NO
- `END_DAY` `datetime` nullable=NO
- `LEFT_TIME` `int` nullable=YES
- `CHARGE_DAY` `datetime` nullable=NO
- `UPDATE_CHECK` `char` nullable=NO
- `NOTE` `varchar` nullable=YES

### BillingDb.dbo.Excite_Free
- `NUM` `int` nullable=NO
- `IDX` `int` nullable=NO
- `EXTERNAL_ID` `varchar` nullable=YES
- `PRISTONTALE_ID` `varchar` nullable=NO
- `GOODS_TYPE` `char` nullable=YES
- `CHARGE_TYPE` `varchar` nullable=YES
- `CHARGE_FORM` `char` nullable=YES
- `AMOUNT` `int` nullable=NO
- `START_DAY` `datetime` nullable=NO
- `END_DAY` `datetime` nullable=NO
- `LEFT_TIME` `int` nullable=YES
- `CHARGE_DAY` `datetime` nullable=NO
- `UPDATE_CHECK` `char` nullable=NO
- `NOTE` `varchar` nullable=YES

### BillingDb.dbo.Excite_NoUse
- `NUM` `int` nullable=NO
- `IDX` `int` nullable=NO
- `EXTERNAL_ID` `varchar` nullable=YES
- `PRISTONTALE_ID` `varchar` nullable=NO
- `GOODS_TYPE` `char` nullable=YES
- `CHARGE_TYPE` `varchar` nullable=YES
- `CHARGE_FORM` `char` nullable=YES
- `AMOUNT` `int` nullable=NO
- `START_DAY` `datetime` nullable=NO
- `END_DAY` `datetime` nullable=NO
- `LEFT_TIME` `int` nullable=YES
- `CHARGE_DAY` `datetime` nullable=NO
- `UPDATE_CHECK` `char` nullable=NO
- `NOTE` `varchar` nullable=YES

### BillingDb.dbo.FCHANNEL
- `PRISTONTALE_ID` `varchar` nullable=NO
- `TABLE_NAME` `varchar` nullable=NO
- `INSERT_DATE` `datetime` nullable=NO

### BillingDb.dbo.GCHANNEL
- `PRISTONTALE_ID` `varchar` nullable=NO
- `TABLE_NAME` `varchar` nullable=NO
- `INSERT_DATE` `datetime` nullable=NO

### BillingDb.dbo.HCHANNEL
- `PRISTONTALE_ID` `varchar` nullable=NO
- `TABLE_NAME` `varchar` nullable=NO
- `INSERT_DATE` `datetime` nullable=NO

### BillingDb.dbo.ICHANNEL
- `PRISTONTALE_ID` `varchar` nullable=NO
- `TABLE_NAME` `varchar` nullable=NO
- `INSERT_DATE` `datetime` nullable=NO

### BillingDb.dbo.JCHANNEL
- `PRISTONTALE_ID` `varchar` nullable=NO
- `TABLE_NAME` `varchar` nullable=NO
- `INSERT_DATE` `datetime` nullable=NO

### BillingDb.dbo.KCHANNEL
- `PRISTONTALE_ID` `varchar` nullable=NO
- `TABLE_NAME` `varchar` nullable=NO
- `INSERT_DATE` `datetime` nullable=NO

### BillingDb.dbo.LCHANNEL
- `PRISTONTALE_ID` `varchar` nullable=NO
- `TABLE_NAME` `varchar` nullable=NO
- `INSERT_DATE` `datetime` nullable=NO

### BillingDb.dbo.MCHANNEL
- `PRISTONTALE_ID` `varchar` nullable=NO
- `TABLE_NAME` `varchar` nullable=NO
- `INSERT_DATE` `datetime` nullable=NO

### BillingDb.dbo.NCHANNEL
- `PRISTONTALE_ID` `varchar` nullable=NO
- `TABLE_NAME` `varchar` nullable=NO
- `INSERT_DATE` `datetime` nullable=NO

### BillingDb.dbo.NTTBilling
- `NUM` `int` nullable=NO
- `EXTERNAL_ID` `varchar` nullable=YES
- `PRISTONTALE_ID` `varchar` nullable=NO
- `GOODS_TYPE` `char` nullable=YES
- `CHARGE_TYPE` `varchar` nullable=YES
- `CHARGE_FORM` `char` nullable=YES
- `AMOUNT` `int` nullable=NO
- `START_DAY` `datetime` nullable=NO
- `END_DAY` `datetime` nullable=NO
- `LEFT_TIME` `int` nullable=YES
- `CHARGE_DAY` `datetime` nullable=NO
- `UPDATE_CHECK` `char` nullable=NO
- `NOTE` `varchar` nullable=YES

### BillingDb.dbo.NetMarble
- `NUM` `int` nullable=NO
- `EXTERNAL_ID` `varchar` nullable=YES
- `PRISTONTALE_ID` `varchar` nullable=NO
- `GOODS_TYPE` `char` nullable=YES
- `CHARGE_TYPE` `varchar` nullable=YES
- `CHARGE_FORM` `char` nullable=YES
- `AMOUNT` `int` nullable=NO
- `START_DAY` `datetime` nullable=NO
- `END_DAY` `datetime` nullable=NO
- `LEFT_TIME` `int` nullable=YES
- `CHARGE_DAY` `datetime` nullable=NO
- `UPDATE_CHECK` `char` nullable=NO
- `NOTE` `varchar` nullable=YES

### BillingDb.dbo.NetMarble_Amount
- `NUM` `int` nullable=NO
- `IDX` `int` nullable=NO
- `EXTERNAL_ID` `varchar` nullable=YES
- `PRISTONTALE_ID` `varchar` nullable=NO
- `GOODS_TYPE` `char` nullable=YES
- `CHARGE_TYPE` `varchar` nullable=YES
- `CHARGE_FORM` `char` nullable=YES
- `AMOUNT` `int` nullable=NO
- `START_DAY` `datetime` nullable=NO
- `END_DAY` `datetime` nullable=NO
- `LEFT_TIME` `int` nullable=YES
- `CHARGE_DAY` `datetime` nullable=NO
- `UPDATE_CHECK` `char` nullable=NO
- `NOTE` `varchar` nullable=YES

### BillingDb.dbo.NetMarble_Bak
- `NUM` `int` nullable=NO
- `IDX` `int` nullable=NO
- `EXTERNAL_ID` `varchar` nullable=YES
- `PRISTONTALE_ID` `varchar` nullable=NO
- `GOODS_TYPE` `char` nullable=YES
- `CHARGE_TYPE` `varchar` nullable=YES
- `CHARGE_FORM` `char` nullable=YES
- `AMOUNT` `int` nullable=NO
- `START_DAY` `datetime` nullable=NO
- `END_DAY` `datetime` nullable=NO
- `LEFT_TIME` `int` nullable=YES
- `CHARGE_DAY` `datetime` nullable=NO
- `UPDATE_CHECK` `char` nullable=NO
- `NOTE` `varchar` nullable=YES

### BillingDb.dbo.NetMarble_Error
- `NUM` `int` nullable=NO
- `IDX` `int` nullable=NO
- `EXTERNAL_ID` `varchar` nullable=YES
- `PRISTONTALE_ID` `varchar` nullable=NO
- `GOODS_TYPE` `char` nullable=YES
- `CHARGE_TYPE` `varchar` nullable=YES
- `CHARGE_FORM` `char` nullable=YES
- `AMOUNT` `int` nullable=NO
- `START_DAY` `datetime` nullable=NO
- `END_DAY` `datetime` nullable=NO
- `LEFT_TIME` `int` nullable=YES
- `CHARGE_DAY` `datetime` nullable=NO
- `UPDATE_CHECK` `char` nullable=NO
- `NOTE` `varchar` nullable=YES

### BillingDb.dbo.NetMarble_Free
- `NUM` `int` nullable=NO
- `IDX` `int` nullable=NO
- `EXTERNAL_ID` `varchar` nullable=YES
- `PRISTONTALE_ID` `varchar` nullable=NO
- `GOODS_TYPE` `char` nullable=YES
- `CHARGE_TYPE` `varchar` nullable=YES
- `CHARGE_FORM` `char` nullable=YES
- `AMOUNT` `int` nullable=NO
- `START_DAY` `datetime` nullable=NO
- `END_DAY` `datetime` nullable=NO
- `LEFT_TIME` `int` nullable=YES
- `CHARGE_DAY` `datetime` nullable=NO
- `UPDATE_CHECK` `char` nullable=NO
- `NOTE` `varchar` nullable=YES

### BillingDb.dbo.NetMarble_NoUse
- `NUM` `int` nullable=NO
- `IDX` `int` nullable=NO
- `EXTERNAL_ID` `varchar` nullable=YES
- `PRISTONTALE_ID` `varchar` nullable=NO
- `GOODS_TYPE` `char` nullable=YES
- `CHARGE_TYPE` `varchar` nullable=YES
- `CHARGE_FORM` `char` nullable=YES
- `AMOUNT` `int` nullable=NO
- `START_DAY` `datetime` nullable=NO
- `END_DAY` `datetime` nullable=NO
- `LEFT_TIME` `int` nullable=YES
- `CHARGE_DAY` `datetime` nullable=NO
- `UPDATE_CHECK` `char` nullable=NO
- `NOTE` `varchar` nullable=YES

### BillingDb.dbo.OCHANNEL
- `PRISTONTALE_ID` `varchar` nullable=NO
- `TABLE_NAME` `varchar` nullable=NO
- `INSERT_DATE` `datetime` nullable=NO

### BillingDb.dbo.PCHANNEL
- `PRISTONTALE_ID` `varchar` nullable=NO
- `TABLE_NAME` `varchar` nullable=NO
- `INSERT_DATE` `datetime` nullable=NO

### BillingDb.dbo.QCHANNEL
- `PRISTONTALE_ID` `varchar` nullable=NO
- `TABLE_NAME` `varchar` nullable=NO
- `INSERT_DATE` `datetime` nullable=NO

### BillingDb.dbo.RCHANNEL
- `PRISTONTALE_ID` `varchar` nullable=NO
- `TABLE_NAME` `varchar` nullable=NO
- `INSERT_DATE` `datetime` nullable=NO

### BillingDb.dbo.SCHANNEL
- `PRISTONTALE_ID` `varchar` nullable=NO
- `TABLE_NAME` `varchar` nullable=NO
- `INSERT_DATE` `datetime` nullable=NO

### BillingDb.dbo.TCHANNEL
- `PRISTONTALE_ID` `varchar` nullable=NO
- `TABLE_NAME` `varchar` nullable=NO
- `INSERT_DATE` `datetime` nullable=NO

### BillingDb.dbo.Table1
- `day` `int` nullable=YES
- `amount` `int` nullable=YES
- `count` `int` nullable=YES

### BillingDb.dbo.UCHANNEL
- `PRISTONTALE_ID` `varchar` nullable=NO
- `TABLE_NAME` `varchar` nullable=NO
- `INSERT_DATE` `datetime` nullable=NO

### BillingDb.dbo.VCHANNEL
- `PRISTONTALE_ID` `varchar` nullable=NO
- `TABLE_NAME` `varchar` nullable=NO
- `INSERT_DATE` `datetime` nullable=NO

### BillingDb.dbo.WCHANNEL
- `PRISTONTALE_ID` `varchar` nullable=NO
- `TABLE_NAME` `varchar` nullable=NO
- `INSERT_DATE` `datetime` nullable=NO

### BillingDb.dbo.XCHANNEL
- `PRISTONTALE_ID` `varchar` nullable=NO
- `TABLE_NAME` `varchar` nullable=NO
- `INSERT_DATE` `datetime` nullable=NO

### BillingDb.dbo.YCHANNEL
- `PRISTONTALE_ID` `varchar` nullable=NO
- `TABLE_NAME` `varchar` nullable=NO
- `INSERT_DATE` `datetime` nullable=NO

### BillingDb.dbo.ZCHANNEL
- `PRISTONTALE_ID` `varchar` nullable=NO
- `TABLE_NAME` `varchar` nullable=NO
- `INSERT_DATE` `datetime` nullable=NO

## BillingLogDb

### BillingLogDb.dbo.Bill_Error_Message
- `ENo` `int` nullable=NO
- `Error_Num` `char` nullable=YES
- `Error_Message` `text` nullable=YES
- `Kor_Error_Message` `text` nullable=YES

### BillingLogDb.dbo.ChannelLog
- `NUM` `int` nullable=NO
- `BNum` `int` nullable=NO
- `ChannelName` `varchar` nullable=NO
- `UserID` `varchar` nullable=NO
- `Operater` `varchar` nullable=NO
- `OperCode` `char` nullable=NO
- `Note` `varchar` nullable=YES
- `RegistDay` `datetime` nullable=NO

### BillingLogDb.dbo.GameLog
- `NUM` `int` nullable=NO
- `BNum` `int` nullable=NO
- `ChannelName` `varchar` nullable=NO
- `UserID` `varchar` nullable=NO
- `Operater` `varchar` nullable=NO
- `OperCode` `char` nullable=NO
- `Note` `varchar` nullable=YES
- `RegistDay` `datetime` nullable=NO

### BillingLogDb.dbo.PersonalLog
- `NUM` `int` nullable=NO
- `BNum` `int` nullable=NO
- `ChannelName` `varchar` nullable=NO
- `UserID` `varchar` nullable=NO
- `Operater` `varchar` nullable=NO
- `OperCode` `char` nullable=NO
- `Note` `varchar` nullable=YES
- `RegistDay` `datetime` nullable=NO

### BillingLogDb.dbo.Security
- `AccountId` `varchar` nullable=YES
- `IP` `varchar` nullable=YES
- `Date` `datetime` nullable=NO
- `Protocol` `varchar` nullable=NO

### BillingLogDb.dbo.UpdateSuccess
- `NUM` `int` nullable=NO
- `Job` `char` nullable=NO
- `SuccessCount` `int` nullable=NO
- `RegistDay` `datetime` nullable=NO

## ClanDB

### ClanDB.dbo.BBSMain
- `IDX` `int` nullable=NO
- `MIDX` `int` nullable=NO
- `Userid` `varchar` nullable=NO
- `WriteName` `varchar` nullable=NO
- `Title` `varchar` nullable=NO
- `Content` `text` nullable=NO
- `SelServer` `int` nullable=NO
- `RegiDate` `datetime` nullable=NO
- `RegiIP` `char` nullable=NO
- `Hit` `int` nullable=NO
- `CountCom` `int` nullable=NO
- `NickName` `varchar` nullable=YES

### ClanDB.dbo.BBSSub
- `SIDX` `int` nullable=NO
- `MINDEX` `int` nullable=NO
- `Userid` `varchar` nullable=NO
- `WriteName` `varchar` nullable=NO
- `Content` `text` nullable=NO
- `RegiDate` `datetime` nullable=NO
- `RegiIP` `char` nullable=NO
- `NickName` `varchar` nullable=YES

### ClanDB.dbo.CL
- `IDX` `int` nullable=NO
- `ClanName` `nvarchar` nullable=NO
- `Note` `text` nullable=NO
- `NoteCnt` `int` nullable=NO
- `UserID` `nvarchar` nullable=YES
- `ClanZang` `nvarchar` nullable=NO
- `Flag` `int` nullable=NO
- `MemCnt` `int` nullable=NO
- `MIconCnt` `int` nullable=NO
- `RegiDate` `datetime` nullable=NO
- `LimitDate` `datetime` nullable=NO
- `DelActive` `char` nullable=NO
- `PFlag` `int` nullable=NO
- `KFlag` `int` nullable=NO
- `Cpoint` `int` nullable=YES
- `CWin` `int` nullable=YES
- `CFail` `int` nullable=YES
- `ClanMoney` `bigint` nullable=YES
- `CNFlag` `int` nullable=YES
- `SiegeMoney` `bigint` nullable=YES
- `LIDER_BC` `int` nullable=YES

### ClanDB.dbo.CT
- `SNo` `int` nullable=YES
- `ServerName` `varchar` nullable=YES
- `MIDX` `int` nullable=YES
- `ClanName` `varchar` nullable=YES
- `ClanJang` `int` nullable=YES
- `ClanImage` `varchar` nullable=YES
- `UserID` `varchar` nullable=YES
- `ChName` `varchar` nullable=YES
- `GPCode` `varchar` nullable=YES
- `LogonTime` `datetime` nullable=YES
- `IP` `varchar` nullable=YES
- `RNo` `int` nullable=YES
- `Flag` `int` nullable=YES

### ClanDB.dbo.ChipLog
- `LgIDX` `int` nullable=NO
- `CIDX` `int` nullable=NO
- `CServer` `int` nullable=NO
- `Userid` `varchar` nullable=NO
- `Chname` `varchar` nullable=NO
- `Permi` `char` nullable=NO
- `RegiDate` `datetime` nullable=NO

### ClanDB.dbo.ClanHome
- `IDX` `int` nullable=NO
- `MIDX` `int` nullable=NO
- `SelServer` `int` nullable=NO
- `Intro` `nvarchar` nullable=YES
- `Image` `nvarchar` nullable=YES
- `Skin` `int` nullable=YES
- `PrivHome` `int` nullable=YES
- `PrivMem` `int` nullable=YES
- `PrivBoard` `int` nullable=YES

### ClanDB.dbo.ClanMainCharChangeLog
- `IDX` `int` nullable=NO
- `UserID` `varchar` nullable=YES
- `BeforeCharName` `varchar` nullable=YES
- `AfterCharName` `varchar` nullable=YES
- `CServer` `int` nullable=YES
- `RegiDate` `datetime` nullable=YES

### ClanDB.dbo.ClanMainDeleted
- `IDX` `int` nullable=NO
- `MIDX` `int` nullable=NO
- `ClanName` `nvarchar` nullable=NO
- `Userid` `nvarchar` nullable=YES
- `ClanZang` `nvarchar` nullable=NO
- `Flag` `int` nullable=NO
- `MemCnt` `int` nullable=NO
- `MIConCnt` `int` nullable=NO
- `RegiDate` `datetime` nullable=NO
- `LimitDate` `datetime` nullable=NO
- `DelDate` `datetime` nullable=NO
- `DelActive` `char` nullable=NO
- `PFlag` `int` nullable=NO
- `KFlag` `int` nullable=NO
- `DelState` `char` nullable=NO
- `DelCase` `char` nullable=NO
- `Server` `int` nullable=NO

### ClanDB.dbo.ClanMainView
- `IDX` `int` nullable=NO
- `ClanName` `nvarchar` nullable=NO
- `Note` `text` nullable=NO
- `NoteCnt` `int` nullable=NO
- `UserID` `nvarchar` nullable=YES
- `ClanZang` `nvarchar` nullable=NO
- `Flag` `int` nullable=NO
- `MemCnt` `int` nullable=NO
- `MIconCnt` `int` nullable=NO
- `RegiDate` `datetime` nullable=NO
- `LimitDate` `datetime` nullable=NO
- `DelActive` `char` nullable=NO
- `PFlag` `int` nullable=NO
- `KFlag` `int` nullable=NO
- `Cpoint` `int` nullable=YES
- `CWin` `int` nullable=YES
- `CFail` `int` nullable=YES
- `ClanMoney` `bigint` nullable=YES
- `CNFlag` `int` nullable=YES
- `SiegeMoney` `bigint` nullable=YES
- `Server` `varchar` nullable=NO

### ClanDB.dbo.ClanMoneyLog
- `IDX` `int` nullable=NO
- `CIDX` `int` nullable=YES
- `UserID` `varchar` nullable=YES
- `chName` `varchar` nullable=YES
- `ServerName` `varchar` nullable=YES
- `Operator` `varchar` nullable=YES
- `OperCode` `char` nullable=YES
- `ClanMoney` `bigint` nullable=YES
- `RegistDay` `datetime` nullable=YES
- `Note` `varchar` nullable=YES

### ClanDB.dbo.ClanSubCharChangeLog
- `IDX` `int` nullable=NO
- `UserID` `varchar` nullable=YES
- `BeforeCharName` `varchar` nullable=YES
- `AfterCharName` `varchar` nullable=YES
- `CServer` `int` nullable=YES
- `RegiDate` `datetime` nullable=YES

### ClanDB.dbo.ClanSubDeleted
- `IDX` `int` nullable=NO
- `MIDX` `int` nullable=NO
- `userid` `nvarchar` nullable=NO
- `ChName` `nvarchar` nullable=NO
- `ChType` `int` nullable=YES
- `ChLv` `int` nullable=YES
- `ClanName` `nvarchar` nullable=YES
- `Permi` `char` nullable=NO
- `JoinDate` `datetime` nullable=NO
- `DelActive` `char` nullable=NO
- `PFlag` `int` nullable=NO
- `KFlag` `int` nullable=NO
- `DelDate` `datetime` nullable=NO
- `DelState` `char` nullable=NO
- `DelCase` `char` nullable=YES
- `Server` `int` nullable=NO

### ClanDB.dbo.ClanSubView
- `IDX` `int` nullable=NO
- `MIDX` `int` nullable=NO
- `userid` `nvarchar` nullable=NO
- `ChName` `nvarchar` nullable=NO
- `ChType` `int` nullable=YES
- `ChLv` `int` nullable=YES
- `ClanName` `nvarchar` nullable=YES
- `Permi` `char` nullable=NO
- `JoinDate` `datetime` nullable=NO
- `DelActive` `char` nullable=NO
- `PFlag` `int` nullable=NO
- `KFlag` `int` nullable=NO
- `Server` `varchar` nullable=NO

### ClanDB.dbo.ClanTicket
- `SNo` `int` nullable=YES
- `ServerName` `varchar` nullable=YES
- `MIDX` `int` nullable=YES
- `ClanName` `varchar` nullable=YES
- `ClanJang` `int` nullable=YES
- `ClanImage` `varchar` nullable=YES
- `UserID` `varchar` nullable=YES
- `ChName` `varchar` nullable=YES
- `GPCode` `varchar` nullable=YES
- `LogonTime` `datetime` nullable=YES
- `IP` `varchar` nullable=YES
- `RNo` `int` nullable=YES
- `Flag` `int` nullable=YES

### ClanDB.dbo.GameNotice
- `UserID` `varchar` nullable=YES
- `ChName` `varchar` nullable=YES
- `Server` `int` nullable=YES
- `PFlag` `int` nullable=YES
- `Txt` `varchar` nullable=YES
- `Flag` `tinyint` nullable=YES

### ClanDB.dbo.GameNoticeGroup
- `IDX` `int` nullable=NO
- `GroupCD` `int` nullable=NO
- `MainCD` `int` nullable=NO
- `DetailCD` `int` nullable=NO
- `Title` `varchar` nullable=NO
- `Content` `text` nullable=NO
- `FromDay` `datetime` nullable=NO
- `ToDay` `datetime` nullable=NO
- `RegistDay` `datetime` nullable=NO
- `DelActive` `int` nullable=NO
- `Hit` `int` nullable=NO
- `Flag` `tinyint` nullable=YES

### ClanDB.dbo.GameNoticePrivate
- `IDX` `int` nullable=NO
- `Userid` `varchar` nullable=NO
- `Title` `varchar` nullable=NO
- `Content` `text` nullable=NO
- `FromDay` `datetime` nullable=NO
- `ToDay` `datetime` nullable=NO
- `RegistDay` `datetime` nullable=NO
- `DelActive` `int` nullable=NO
- `Hit` `int` nullable=NO
- `Flag` `tinyint` nullable=YES

### ClanDB.dbo.LI
- `IMG` `int` nullable=NO
- `ID` `int` nullable=NO

### ClanDB.dbo.Minutos
- `ChName` `varchar` nullable=YES
- `Minutos` `int` nullable=YES

### ClanDB.dbo.Notice
- `IDX` `int` nullable=NO
- `MIDX` `int` nullable=NO
- `Userid` `varchar` nullable=NO
- `WriteName` `varchar` nullable=NO
- `Title` `varchar` nullable=NO
- `Content` `text` nullable=NO
- `SelServer` `int` nullable=NO
- `RegiDate` `datetime` nullable=NO
- `RegiIP` `char` nullable=NO
- `Hit` `int` nullable=NO

### ClanDB.dbo.PRBBSMain
- `IDX` `int` nullable=NO
- `PMNo` `varchar` nullable=NO
- `Userid` `varchar` nullable=NO
- `Title` `varchar` nullable=NO
- `Content` `text` nullable=NO
- `UserName` `varchar` nullable=NO
- `ClanIdx` `int` nullable=NO
- `SelServer` `int` nullable=NO
- `RegiDate` `datetime` nullable=NO
- `RegiIP` `char` nullable=NO
- `Hit` `int` nullable=NO
- `CountCom` `int` nullable=NO

### ClanDB.dbo.PRBBSSub
- `SIDX` `int` nullable=NO
- `MINDEX` `int` nullable=NO
- `PMNo` `varchar` nullable=NO
- `userid` `varchar` nullable=NO
- `writename` `varchar` nullable=NO
- `Content` `text` nullable=NO
- `RegiDate` `datetime` nullable=NO
- `RegiIP` `char` nullable=NO
- `UserName` `varchar` nullable=NO

### ClanDB.dbo.PRBbsFile
- `IDX` `int` nullable=NO
- `MINDEX` `int` nullable=NO
- `FileName` `varchar` nullable=NO
- `FileSize` `varchar` nullable=NO
- `RegiDate` `datetime` nullable=NO

### ClanDB.dbo.SiegeClanList
- `IDX` `int` nullable=NO
- `UserID` `varchar` nullable=NO
- `CharName` `varchar` nullable=NO
- `ClanName` `varchar` nullable=NO
- `TaxRate` `int` nullable=NO
- `ServerName` `varchar` nullable=NO
- `Server` `tinyint` nullable=NO
- `OperFrom` `datetime` nullable=YES
- `OperTo` `datetime` nullable=YES
- `RegistDay` `datetime` nullable=NO

### ClanDB.dbo.SiegeCurrentList
- `IDX` `int` nullable=NO
- `CIDX` `int` nullable=YES
- `UserID` `varchar` nullable=YES
- `chName` `varchar` nullable=YES
- `ServerName` `varchar` nullable=YES
- `Operator` `varchar` nullable=YES
- `OperCode` `char` nullable=YES
- `SiegeMoney` `bigint` nullable=YES
- `RegistDay` `datetime` nullable=YES

### ClanDB.dbo.SiegeMoneyLog
- `IDX` `int` nullable=NO
- `CIDX` `int` nullable=YES
- `UserID` `varchar` nullable=YES
- `chName` `varchar` nullable=YES
- `ServerName` `varchar` nullable=YES
- `Operator` `varchar` nullable=YES
- `OperCode` `char` nullable=YES
- `SiegeMoney` `bigint` nullable=YES
- `RegistDay` `datetime` nullable=YES

### ClanDB.dbo.SiegeMoneyTax
- `DNo` `int` nullable=NO
- `IDX` `int` nullable=YES
- `Mixing` `bigint` nullable=YES
- `Aging` `bigint` nullable=YES
- `Shop` `bigint` nullable=YES
- `Poison1` `bigint` nullable=YES
- `Poison2` `bigint` nullable=YES
- `Poison3` `bigint` nullable=YES
- `Force` `bigint` nullable=YES
- `Warpgate` `bigint` nullable=YES
- `Skill` `bigint` nullable=YES
- `Total` `bigint` nullable=YES
- `Tax` `bigint` nullable=YES
- `ServerName` `varchar` nullable=YES
- `RegistDay` `datetime` nullable=YES

### ClanDB.dbo.UL
- `IDX` `int` nullable=NO
- `MIDX` `int` nullable=NO
- `userid` `nvarchar` nullable=NO
- `ChName` `nvarchar` nullable=NO
- `ChType` `int` nullable=YES
- `ChLv` `int` nullable=YES
- `ClanName` `nvarchar` nullable=YES
- `Permi` `char` nullable=NO
- `JoinDate` `datetime` nullable=NO
- `DelActive` `char` nullable=NO
- `PFlag` `int` nullable=NO
- `KFlag` `int` nullable=NO
- `MIconCnt` `int` nullable=NO

## CoinShopDB

### CoinShopDB.dbo.ItemList
- `szItemCode` `varchar` nullable=YES
- `ItemSection` `int` nullable=YES
- `Price` `int` nullable=YES

### CoinShopDB.dbo.ItemList_T
- `szItemCode` `varchar` nullable=YES
- `ItemSection` `int` nullable=YES
- `Price` `int` nullable=YES

### CoinShopDB.dbo.PendingPayments
- `ID` `int` nullable=NO
- `UserID` `varchar` nullable=NO
- `Amount` `decimal` nullable=NO
- `Coins` `int` nullable=NO
- `MPPaymentID` `varchar` nullable=NO
- `Status` `varchar` nullable=NO
- `CreatedDate` `datetime` nullable=NO
- `CompletedDate` `datetime` nullable=YES
- `IP` `varchar` nullable=YES

### CoinShopDB.dbo.Transactions
- `Id` `int` nullable=NO
- `UserID` `varchar` nullable=NO
- `PaymentID` `varchar` nullable=NO
- `Amount` `decimal` nullable=NO
- `Coins` `int` nullable=NO
- `Status` `varchar` nullable=NO
- `Date` `datetime` nullable=NO

### CoinShopDB.dbo.UserInfo
- `UserID` `varchar` nullable=YES
- `UserCoin` `int` nullable=YES
- `LastChange` `datetime` nullable=YES

### CoinShopDB.dbo.UserInfo_T
- `UserID` `varchar` nullable=YES
- `UserMinutos` `int` nullable=YES
- `LastChange` `datetime` nullable=YES

## GameLogDB

### GameLogDB.dbo.CharGameRecordLg
- `CharNo` `int` nullable=NO
- `UserID` `varchar` nullable=YES
- `CharType` `int` nullable=YES
- `GameLogonID` `varchar` nullable=YES
- `GPCode` `varchar` nullable=YES
- `ServerName` `varchar` nullable=YES
- `IP` `varchar` nullable=YES
- `GameLevel` `int` nullable=YES
- `Exp` `bigint` nullable=YES
- `GameMoney` `int` nullable=YES
- `RegistDay` `datetime` nullable=YES
- `DisuseDay` `datetime` nullable=YES
- `EditDay` `datetime` nullable=YES
- `Active` `int` nullable=YES

### GameLogDB.dbo.CharTop
- `IDX` `int` nullable=NO
- `userid` `varchar` nullable=YES
- `chartype` `int` nullable=YES
- `charname` `varchar` nullable=YES
- `servername` `varchar` nullable=YES
- `glevel` `int` nullable=YES
- `editday` `datetime` nullable=YES
- `exp` `bigint` nullable=YES

### GameLogDB.dbo.FlowerUserID
- `UserID` `varchar` nullable=YES

### GameLogDB.dbo.GameOnRecord
- `SNo` `varchar` nullable=YES
- `UserID` `varchar` nullable=YES
- `GPCode` `varchar` nullable=YES
- `ServerName` `varchar` nullable=YES
- `LogonTime` `datetime` nullable=YES
- `IP` `varchar` nullable=YES
- `RNo` `int` nullable=YES

### GameLogDB.dbo.GameOutRecord
- `SNo` `varchar` nullable=YES
- `UserID` `varchar` nullable=YES
- `GPCode` `varchar` nullable=YES
- `ServerName` `varchar` nullable=YES
- `LogoutTime` `datetime` nullable=YES
- `IP` `varchar` nullable=YES
- `RNo` `int` nullable=YES

### GameLogDB.dbo.GiveYouItem
- `GNo` `int` nullable=NO
- `UserID` `varchar` nullable=NO
- `CharName` `varchar` nullable=NO
- `Server` `varchar` nullable=NO
- `Item` `varchar` nullable=NO
- `ItemCode` `int` nullable=NO
- `JobCode` `int` nullable=NO
- `Reason` `varchar` nullable=NO
- `Flag` `char` nullable=NO
- `Operator` `varchar` nullable=YES
- `RegistDay` `datetime` nullable=NO
- `EditDay` `datetime` nullable=YES
- `CancelReason` `varchar` nullable=YES

### GameLogDB.dbo.HackLog0301
- `HNo` `int` nullable=NO
- `UserID` `varchar` nullable=NO
- `CharType` `int` nullable=YES
- `GameLogonId` `varchar` nullable=YES
- `GPCode` `varchar` nullable=YES
- `HCode` `int` nullable=YES
- `ServerName` `varchar` nullable=YES
- `IP` `varchar` nullable=YES
- `RegistDay` `datetime` nullable=YES
- `Active` `char` nullable=YES

### GameLogDB.dbo.ID20050304
- `UserID` `varchar` nullable=YES

### GameLogDB.dbo.ITEMCode
- `ITEMNo` `int` nullable=NO
- `Category` `varchar` nullable=YES
- `CodeHName` `varchar` nullable=YES
- `CodeEName` `varchar` nullable=YES
- `CodeJName` `varchar` nullable=YES
- `Code` `int` nullable=YES
- `ChangeJob` `char` nullable=NO

### GameLogDB.dbo.ItemBillGiveLog
- `INo` `int` nullable=NO
- `userid` `varchar` nullable=NO
- `ServerName` `varchar` nullable=NO
- `ServerCode` `int` nullable=NO
- `ItemCode` `varchar` nullable=NO
- `ItemCount` `int` nullable=NO
- `Active` `char` nullable=NO
- `ConfirmDate` `datetime` nullable=YES
- `CancelDate` `datetime` nullable=YES
- `RegistDay` `datetime` nullable=NO
- `EditDate` `datetime` nullable=NO
- `Note` `varchar` nullable=YES

### GameLogDB.dbo.ItemGiveList
- `IDX` `int` nullable=NO
- `UserID` `varchar` nullable=YES
- `ServerName` `varchar` nullable=YES
- `Flag` `int` nullable=YES

### GameLogDB.dbo.ItemGiveList3
- `IDX` `int` nullable=NO
- `UserID` `varchar` nullable=NO
- `ServerName` `varchar` nullable=NO
- `Flag` `int` nullable=YES

### GameLogDB.dbo.ItemGiveLog
- `INo` `int` nullable=NO
- `UserID` `varchar` nullable=NO
- `ServerCode` `smallint` nullable=NO
- `ItemCategory` `varchar` nullable=NO
- `ItemCount` `int` nullable=NO
- `Flag` `int` nullable=YES
- `Active` `char` nullable=NO
- `ConfirmDate` `char` nullable=YES
- `RegistDay` `datetime` nullable=NO
- `Note` `varchar` nullable=YES

### GameLogDB.dbo.LongCharge
- `IDX` `int` nullable=YES
- `UserID` `varchar` nullable=YES
- `Email` `varchar` nullable=YES

### GameLogDB.dbo.ServerUP
- `SNo` `int` nullable=NO
- `ServerName` `varchar` nullable=YES
- `UCount` `int` nullable=YES
- `RegistDay` `datetime` nullable=YES

### GameLogDB.dbo.UserListTemp
- `IDX` `int` nullable=NO
- `UserID` `varchar` nullable=YES
- `Server` `varchar` nullable=YES

### GameLogDB.dbo.tempcharrecord
- `UserID` `varchar` nullable=YES
- `ServerName` `varchar` nullable=YES
- `exp` `int` nullable=YES

## ItemLogDB

### ItemLogDB.dbo.AItemCode
- `ItemCode` `nvarchar` nullable=NO
- `KorName` `nvarchar` nullable=NO
- `EngName` `nvarchar` nullable=YES

### ItemLogDB.dbo.IL
- `ITEMNo` `int` nullable=NO
- `UserID` `varchar` nullable=YES
- `CharName` `varchar` nullable=YES
- `IP` `varchar` nullable=YES
- `Flag` `int` nullable=YES
- `TUserID` `varchar` nullable=YES
- `TCharName` `varchar` nullable=YES
- `TMoney` `int` nullable=YES
- `TIP` `varchar` nullable=YES
- `ItemCount` `int` nullable=YES
- `ItemCode` `int` nullable=YES
- `ItemINo` `int` nullable=YES
- `ItemINo_1` `int` nullable=YES
- `RegistDay` `datetime` nullable=YES

### ItemLogDB.dbo.ITEMCODE
- `ITEMNo` `int` nullable=NO
- `Category` `varchar` nullable=YES
- `CodeHName` `varchar` nullable=YES
- `CodeEName` `varchar` nullable=YES
- `Code` `int` nullable=YES

## Jobs

### Jobs.dbo.Jobs_Info
- `ID` `varchar` nullable=YES
- `NICK` `varchar` nullable=YES
- `JOB` `int` nullable=YES
- `Job_Level` `int` nullable=YES
- `Job_xp` `int` nullable=YES
- `M_A` `int` nullable=YES
- `M_F` `int` nullable=YES
- `M_L` `int` nullable=YES
- `M_M` `int` nullable=YES
- `M_R` `int` nullable=YES

### Jobs.dbo.Jobs_shop
- `ID` `varchar` nullable=YES
- `NICK` `varchar` nullable=YES
- `MINE_TYPE` `int` nullable=YES
- `MINE_QTD` `int` nullable=YES
- `MINE_PRICE` `int` nullable=YES
- `CHECKSUM` `int` nullable=YES

## LogDB

### LogDB.dbo.AccountLogin
- `ID` `int` nullable=NO
- `Account` `varchar` nullable=YES
- `Password` `varchar` nullable=YES
- `IP` `varchar` nullable=YES
- `MacAddress` `varchar` nullable=YES
- `Path` `varchar` nullable=YES
- `PCName` `varchar` nullable=YES
- `Date` `varchar` nullable=YES

### LogDB.dbo.ChangePassword
- `AccountName` `ntext` nullable=YES
- `Password` `ntext` nullable=YES

### LogDB.dbo.CheatLog
- `ID` `bigint` nullable=NO
- `IP` `varchar` nullable=NO
- `AccountName` `varchar` nullable=NO
- `Action` `int` nullable=NO
- `LogID` `int` nullable=NO
- `Description` `varchar` nullable=NO
- `Date` `varchar` nullable=NO

### LogDB.dbo.ClanLogin
- `AccountName` `ntext` nullable=YES
- `Password` `ntext` nullable=YES

### LogDB.dbo.Register
- `AccountName` `ntext` nullable=YES
- `Password` `ntext` nullable=YES

## LogadoPremiado

### LogadoPremiado.dbo.ItemList
- `ItemCode` `varchar` nullable=NO

## PCRoom

### PCRoom.dbo.IPChangeLog
- `INo` `int` nullable=NO
- `RNo` `int` nullable=NO
- `UserID` `varchar` nullable=NO
- `GPCode` `char` nullable=NO
- `TIPAddress1_G` `varchar` nullable=YES
- `TIPAddress2_G` `varchar` nullable=YES
- `TIPAddress3_G` `varchar` nullable=YES
- `TIPAddress1` `varchar` nullable=YES
- `TIPAddress2` `varchar` nullable=YES
- `TIPAddress3` `varchar` nullable=YES
- `TIPAddress4` `varchar` nullable=YES
- `TIPAddress5` `varchar` nullable=YES
- `TIPAddress6` `varchar` nullable=YES
- `TIPAddress7` `varchar` nullable=YES
- `TIPAddress8` `varchar` nullable=YES
- `TIPAddress9` `varchar` nullable=YES
- `TIPAddress10` `varchar` nullable=YES
- `BonusIP` `varchar` nullable=YES
- `IPCount` `int` nullable=NO
- `AddIPCount` `int` nullable=NO
- `RegistDay` `datetime` nullable=YES
- `DisuseDay` `datetime` nullable=YES
- `UsePeriod` `int` nullable=YES
- `Active` `char` nullable=NO
- `UNo` `int` nullable=YES
- `PMNo` `int` nullable=NO
- `Status` `char` nullable=NO
- `Note` `varchar` nullable=YES
- `EditDate` `datetime` nullable=NO

### PCRoom.dbo.IPList
- `INo` `int` nullable=NO
- `PMNo` `int` nullable=NO
- `RNo` `int` nullable=NO
- `IP` `varchar` nullable=NO

### PCRoom.dbo.RegIPAddress
- `RNo` `int` nullable=NO
- `UserID` `varchar` nullable=NO
- `GPCode` `char` nullable=NO
- `TIPAddress1_G` `varchar` nullable=YES
- `TIPAddress2_G` `varchar` nullable=YES
- `TIPAddress3_G` `varchar` nullable=YES
- `TIPAddress1` `varchar` nullable=YES
- `TIPAddress2` `varchar` nullable=YES
- `TIPAddress3` `varchar` nullable=YES
- `TIPAddress4` `varchar` nullable=YES
- `TIPAddress5` `varchar` nullable=YES
- `TIPAddress6` `varchar` nullable=YES
- `TIPAddress7` `varchar` nullable=YES
- `TIPAddress8` `varchar` nullable=YES
- `TIPAddress9` `varchar` nullable=YES
- `TIPAddress10` `varchar` nullable=YES
- `BonusIP` `varchar` nullable=YES
- `IPCount` `int` nullable=NO
- `AddIPCount` `int` nullable=NO
- `RegistDay` `datetime` nullable=YES
- `DisuseDay` `datetime` nullable=YES
- `UsePeriod` `int` nullable=YES
- `Active` `char` nullable=NO
- `UNo` `int` nullable=YES
- `PMNo` `int` nullable=NO

## PCRoomLog

### PCRoomLog.dbo.PCUseTimeLog0506
- `IDX` `int` nullable=NO
- `SNo` `varchar` nullable=YES
- `UserID` `varchar` nullable=YES
- `GPCode` `varchar` nullable=YES
- `ServerName` `varchar` nullable=YES
- `GameLogonID` `varchar` nullable=YES
- `RegistDay` `datetime` nullable=YES
- `UsePeriod` `int` nullable=YES
- `IP` `varchar` nullable=YES
- `RNo` `int` nullable=YES
- `UNo` `int` nullable=YES

### PCRoomLog.dbo.PCUseTimeLog0507
- `IDX` `int` nullable=NO
- `SNo` `varchar` nullable=YES
- `UserID` `varchar` nullable=YES
- `GPCode` `varchar` nullable=YES
- `ServerName` `varchar` nullable=YES
- `GameLogonID` `varchar` nullable=YES
- `RegistDay` `datetime` nullable=YES
- `UsePeriod` `int` nullable=YES
- `IP` `varchar` nullable=YES
- `RNo` `int` nullable=YES
- `UNo` `int` nullable=YES

### PCRoomLog.dbo.PCUseTimeLog0508
- `IDX` `int` nullable=NO
- `SNo` `varchar` nullable=YES
- `UserID` `varchar` nullable=YES
- `GPCode` `varchar` nullable=YES
- `ServerName` `varchar` nullable=YES
- `GameLogonID` `varchar` nullable=YES
- `RegistDay` `datetime` nullable=YES
- `UsePeriod` `int` nullable=YES
- `IP` `varchar` nullable=YES
- `RNo` `int` nullable=YES
- `UNo` `int` nullable=YES

### PCRoomLog.dbo.PCUseTimeLog0509
- `IDX` `int` nullable=NO
- `SNo` `varchar` nullable=YES
- `UserID` `varchar` nullable=YES
- `GPCode` `varchar` nullable=YES
- `ServerName` `varchar` nullable=YES
- `GameLogonID` `varchar` nullable=YES
- `RegistDay` `datetime` nullable=YES
- `UsePeriod` `int` nullable=YES
- `IP` `varchar` nullable=YES
- `RNo` `int` nullable=YES
- `UNo` `int` nullable=YES

### PCRoomLog.dbo.PCUseTimeLog0510
- `IDX` `int` nullable=NO
- `SNo` `varchar` nullable=YES
- `UserID` `varchar` nullable=YES
- `GPCode` `varchar` nullable=YES
- `ServerName` `varchar` nullable=YES
- `GameLogonID` `varchar` nullable=YES
- `RegistDay` `datetime` nullable=YES
- `UsePeriod` `int` nullable=YES
- `IP` `varchar` nullable=YES
- `RNo` `int` nullable=YES
- `UNo` `int` nullable=YES

### PCRoomLog.dbo.PCUseTimeLog0511
- `IDX` `int` nullable=NO
- `SNo` `varchar` nullable=YES
- `UserID` `varchar` nullable=YES
- `GPCode` `varchar` nullable=YES
- `ServerName` `varchar` nullable=YES
- `GameLogonID` `varchar` nullable=YES
- `RegistDay` `datetime` nullable=YES
- `UsePeriod` `int` nullable=YES
- `IP` `varchar` nullable=YES
- `RNo` `int` nullable=YES
- `UNo` `int` nullable=YES

### PCRoomLog.dbo.PCUseTimeLog0512
- `IDX` `int` nullable=NO
- `SNo` `varchar` nullable=YES
- `UserID` `varchar` nullable=YES
- `GPCode` `varchar` nullable=YES
- `ServerName` `varchar` nullable=YES
- `GameLogonID` `varchar` nullable=YES
- `RegistDay` `datetime` nullable=YES
- `UsePeriod` `int` nullable=YES
- `IP` `varchar` nullable=YES
- `RNo` `int` nullable=YES
- `UNo` `int` nullable=YES

### PCRoomLog.dbo.PCUseTimeLog0601
- `IDX` `int` nullable=NO
- `SNo` `varchar` nullable=YES
- `UserID` `varchar` nullable=YES
- `GPCode` `varchar` nullable=YES
- `ServerName` `varchar` nullable=YES
- `GameLogonID` `varchar` nullable=YES
- `RegistDay` `datetime` nullable=YES
- `UsePeriod` `int` nullable=YES
- `IP` `varchar` nullable=YES
- `RNo` `int` nullable=YES
- `UNo` `int` nullable=YES

### PCRoomLog.dbo.PCUseTimeLog0602
- `IDX` `int` nullable=NO
- `SNo` `varchar` nullable=YES
- `UserID` `varchar` nullable=YES
- `GPCode` `varchar` nullable=YES
- `ServerName` `varchar` nullable=YES
- `GameLogonID` `varchar` nullable=YES
- `RegistDay` `datetime` nullable=YES
- `UsePeriod` `int` nullable=YES
- `IP` `varchar` nullable=YES
- `RNo` `int` nullable=YES
- `UNo` `int` nullable=YES

### PCRoomLog.dbo.PCUseTimeLog0603
- `IDX` `int` nullable=NO
- `SNo` `varchar` nullable=YES
- `UserID` `varchar` nullable=YES
- `GPCode` `varchar` nullable=YES
- `ServerName` `varchar` nullable=YES
- `GameLogonID` `varchar` nullable=YES
- `RegistDay` `datetime` nullable=YES
- `UsePeriod` `int` nullable=YES
- `IP` `varchar` nullable=YES
- `RNo` `int` nullable=YES
- `UNo` `int` nullable=YES

### PCRoomLog.dbo.PCUseTimeLog0604
- `IDX` `int` nullable=NO
- `SNo` `varchar` nullable=YES
- `UserID` `varchar` nullable=YES
- `GPCode` `varchar` nullable=YES
- `ServerName` `varchar` nullable=YES
- `GameLogonID` `varchar` nullable=YES
- `RegistDay` `datetime` nullable=YES
- `UsePeriod` `int` nullable=YES
- `IP` `varchar` nullable=YES
- `RNo` `int` nullable=YES
- `UNo` `int` nullable=YES

### PCRoomLog.dbo.PCUseTimeLog0605
- `IDX` `int` nullable=NO
- `SNo` `varchar` nullable=YES
- `UserID` `varchar` nullable=YES
- `GPCode` `varchar` nullable=YES
- `ServerName` `varchar` nullable=YES
- `GameLogonID` `varchar` nullable=YES
- `RegistDay` `datetime` nullable=YES
- `UsePeriod` `int` nullable=YES
- `IP` `varchar` nullable=YES
- `RNo` `int` nullable=YES
- `UNo` `int` nullable=YES

### PCRoomLog.dbo.PCUseTimeLog0606
- `IDX` `int` nullable=NO
- `SNo` `varchar` nullable=YES
- `UserID` `varchar` nullable=YES
- `GPCode` `varchar` nullable=YES
- `ServerName` `varchar` nullable=YES
- `GameLogonID` `varchar` nullable=YES
- `RegistDay` `datetime` nullable=YES
- `UsePeriod` `int` nullable=YES
- `IP` `varchar` nullable=YES
- `RNo` `int` nullable=YES
- `UNo` `int` nullable=YES

### PCRoomLog.dbo.PCUseTimeLog0607
- `IDX` `int` nullable=NO
- `SNo` `varchar` nullable=YES
- `UserID` `varchar` nullable=YES
- `GPCode` `varchar` nullable=YES
- `ServerName` `varchar` nullable=YES
- `GameLogonID` `varchar` nullable=YES
- `RegistDay` `datetime` nullable=YES
- `UsePeriod` `int` nullable=YES
- `IP` `varchar` nullable=YES
- `RNo` `int` nullable=YES
- `UNo` `int` nullable=YES

### PCRoomLog.dbo.PCUseTimeLog0608
- `IDX` `int` nullable=NO
- `SNo` `varchar` nullable=YES
- `UserID` `varchar` nullable=YES
- `GPCode` `varchar` nullable=YES
- `ServerName` `varchar` nullable=YES
- `GameLogonID` `varchar` nullable=YES
- `RegistDay` `datetime` nullable=YES
- `UsePeriod` `int` nullable=YES
- `IP` `varchar` nullable=YES
- `RNo` `int` nullable=YES
- `UNo` `int` nullable=YES

### PCRoomLog.dbo.PCUseTimeLog0609
- `IDX` `int` nullable=NO
- `SNo` `varchar` nullable=YES
- `UserID` `varchar` nullable=YES
- `GPCode` `varchar` nullable=YES
- `ServerName` `varchar` nullable=YES
- `GameLogonID` `varchar` nullable=YES
- `RegistDay` `datetime` nullable=YES
- `UsePeriod` `int` nullable=YES
- `IP` `varchar` nullable=YES
- `RNo` `int` nullable=YES
- `UNo` `int` nullable=YES

### PCRoomLog.dbo.PCUseTimeLog0610
- `IDX` `int` nullable=NO
- `SNo` `varchar` nullable=YES
- `UserID` `varchar` nullable=YES
- `GPCode` `varchar` nullable=YES
- `ServerName` `varchar` nullable=YES
- `GameLogonID` `varchar` nullable=YES
- `RegistDay` `datetime` nullable=YES
- `UsePeriod` `int` nullable=YES
- `IP` `varchar` nullable=YES
- `RNo` `int` nullable=YES
- `UNo` `int` nullable=YES

### PCRoomLog.dbo.PCUseTimeLog0611
- `IDX` `int` nullable=NO
- `SNo` `varchar` nullable=YES
- `UserID` `varchar` nullable=YES
- `GPCode` `varchar` nullable=YES
- `ServerName` `varchar` nullable=YES
- `GameLogonID` `varchar` nullable=YES
- `RegistDay` `datetime` nullable=YES
- `UsePeriod` `int` nullable=YES
- `IP` `varchar` nullable=YES
- `RNo` `int` nullable=YES
- `UNo` `int` nullable=YES

### PCRoomLog.dbo.PCUseTimeLog0612
- `IDX` `int` nullable=NO
- `SNo` `varchar` nullable=YES
- `UserID` `varchar` nullable=YES
- `GPCode` `varchar` nullable=YES
- `ServerName` `varchar` nullable=YES
- `GameLogonID` `varchar` nullable=YES
- `RegistDay` `datetime` nullable=YES
- `UsePeriod` `int` nullable=YES
- `IP` `varchar` nullable=YES
- `RNo` `int` nullable=YES
- `UNo` `int` nullable=YES

## PristonDB

### PristonDB.dbo.AgeFail
- `SubTag` `int` nullable=YES
- `Hora` `int` nullable=YES
- `Minuto` `int` nullable=YES
- `Segundo` `int` nullable=YES
- `ID` `varchar` nullable=YES
- `Nome` `varchar` nullable=YES
- `IP` `varchar` nullable=YES
- `Item` `varchar` nullable=YES
- `Aging` `int` nullable=YES
- `Head` `int` nullable=YES
- `Checksum` `int` nullable=YES
- `Dwtime` `varchar` nullable=YES
- `Dwversion` `int` nullable=YES
- `Resgate` `int` nullable=YES
- `Dia` `int` nullable=YES
- `Mes` `int` nullable=YES
- `Ano` `int` nullable=YES

### PristonDB.dbo.Beta
- `accountId` `varchar` nullable=YES

### PristonDB.dbo.CheatLog
- `ID` `bigint` nullable=NO
- `IP` `varchar` nullable=NO
- `AccountName` `varchar` nullable=NO
- `Action` `int` nullable=NO
- `LogID` `int` nullable=NO
- `Description` `varchar` nullable=NO
- `Date` `varchar` nullable=NO

### PristonDB.dbo.Coroa
- `ID` `int` nullable=YES

### PristonDB.dbo.Coroas
- `SubTag` `int` nullable=YES
- `Name` `varchar` nullable=YES
- `CodeIT` `varchar` nullable=YES
- `ID` `int` nullable=YES

### PristonDB.dbo.FunctionChecksumList
- `ID` `int` nullable=NO
- `Size` `int` nullable=NO
- `Address` `varchar` nullable=NO
- `Checksum` `varchar` nullable=NO

### PristonDB.dbo.LiberarPais
- `IdLiberarPais` `int` nullable=NO
- `Nome` `varchar` nullable=NO
- `IndSit` `int` nullable=NO

### PristonDB.dbo.ListaNegra
- `IdListaNegra` `int` nullable=NO
- `IdCliente` `int` nullable=NO
- `CodigoPc` `varchar` nullable=NO
- `Ip` `varchar` nullable=NO
- `Data` `varchar` nullable=NO
- `IndSit` `int` nullable=NO

### PristonDB.dbo.LoginAutenticado
- `IdLoginAutenticado` `int` nullable=NO
- `Ip` `varchar` nullable=NO
- `DataAutenticado` `datetime` nullable=NO

### PristonDB.dbo.LoginMacAdresss
- `IdLoginMacAdresss` `int` nullable=NO
- `Login` `varchar` nullable=NO
- `MacAddress` `varchar` nullable=NO
- `IP` `varchar` nullable=NO
- `Bloqueado` `int` nullable=NO
- `DataLogin` `datetime` nullable=NO

### PristonDB.dbo.LogsIpsOnline
- `IdLogsIpsOnline` `int` nullable=NO
- `Ip` `varchar` nullable=NO
- `DataOnline` `smalldatetime` nullable=NO

### PristonDB.dbo.LogsProtecao
- `IdLogsProtecao` `int` nullable=NO
- `CodigoPc` `varchar` nullable=NO
- `Ip` `varchar` nullable=NO
- `DataLog` `smalldatetime` nullable=NO

### PristonDB.dbo.NoticeSystem
- `ID` `int` nullable=NO
- `Message` `varchar` nullable=YES
- `Type` `int` nullable=YES
- `Activated` `int` nullable=YES

### PristonDB.dbo.PVP
- `ID` `varchar` nullable=YES
- `Name` `varchar` nullable=YES
- `Class` `int` nullable=YES
- `Lvl` `int` nullable=YES
- `Kills` `int` nullable=YES
- `Deads` `int` nullable=YES

### PristonDB.dbo.PremiumData
- `ID` `int` nullable=NO
- `CharacterName` `varchar` nullable=NO
- `TimeLeft` `int` nullable=NO
- `ItemType` `int` nullable=NO

### PristonDB.dbo.Security
- `AccountId` `varchar` nullable=YES
- `IP` `varchar` nullable=YES
- `Date` `datetime` nullable=NO
- `Protocol` `varchar` nullable=NO

### PristonDB.dbo.Tags
- `SubTag` `int` nullable=YES
- `Clan` `varchar` nullable=YES
- `CodeIT` `varchar` nullable=YES
- `Slot` `int` nullable=YES

### PristonDB.dbo.TopLevel
- `ID` `nvarchar` nullable=NO
- `NAME` `nvarchar` nullable=NO
- `CLASS` `int` nullable=NO
- `LVL` `int` nullable=NO
- `XP` `int` nullable=NO

### PristonDB.dbo.WindowCheatList
- `ID` `int` nullable=NO
- `WindowName` `varchar` nullable=NO

## QuestDB

### QuestDB.dbo.Complete
- `QuestID` `int` nullable=YES
- `QuestKind` `int` nullable=YES
- `QuestType` `int` nullable=YES
- `PlayerName` `varchar` nullable=YES
- `EndDate` `datetime` nullable=YES

### QuestDB.dbo.DailyQuest
- `QuestID` `int` nullable=YES
- `QuestName` `varchar` nullable=YES
- `QuestTitle` `varchar` nullable=YES
- `LevelMin` `int` nullable=YES
- `LevelMax` `int` nullable=YES
- `PartyFlag` `int` nullable=YES
- `QuestDesc` `varchar` nullable=YES
- `QuestType` `int` nullable=YES

### QuestDB.dbo.ItemRewards
- `QuestID` `int` nullable=YES
- `QuestKind` `int` nullable=YES
- `ItemCode` `varchar` nullable=YES
- `ItemCount` `int` nullable=YES
- `QuestType` `int` nullable=YES

### QuestDB.dbo.MonsterKilled
- `MonsterName` `varchar` nullable=YES
- `Killed` `int` nullable=YES
- `Count` `int` nullable=YES
- `QuestID` `int` nullable=YES
- `QuestKind` `int` nullable=YES
- `QuestType` `int` nullable=YES
- `PlayerName` `varchar` nullable=YES
- `PartyFlag` `int` nullable=YES
- `QuestMap` `int` nullable=YES

### QuestDB.dbo.Progress
- `QuestID` `int` nullable=YES
- `QuestKind` `int` nullable=YES
- `QuestType` `int` nullable=YES
- `PlayerName` `varchar` nullable=YES
- `Started` `datetime` nullable=YES

### QuestDB.dbo.QuestMonsters
- `QuestID` `int` nullable=YES
- `QuestKind` `int` nullable=YES
- `MonsterName` `varchar` nullable=YES
- `Quantity` `int` nullable=YES
- `QuestType` `int` nullable=YES
- `QuestMap` `int` nullable=YES

### QuestDB.dbo.Rewards
- `QuestID` `int` nullable=YES
- `QuestKind` `int` nullable=YES
- `Gold` `int` nullable=YES
- `Exp` `bigint` nullable=YES
- `QuestType` `int` nullable=YES
- `Coin` `int` nullable=YES
- `TCoin` `int` nullable=YES

### QuestDB.dbo.SingleQuest
- `QuestID` `int` nullable=YES
- `QuestName` `varchar` nullable=YES
- `QuestTitle` `varchar` nullable=YES
- `LevelMin` `int` nullable=YES
- `LevelMax` `int` nullable=YES
- `PartyFlag` `int` nullable=YES
- `QuestDesc` `varchar` nullable=YES
- `QuestType` `int` nullable=YES

## SecurityDB

### SecurityDB.dbo.AgingFailed
- `ID` `int` nullable=NO
- `PlayerID` `varchar` nullable=YES
- `PlayerName` `varchar` nullable=YES
- `ItemName` `varchar` nullable=YES
- `ItemAging` `int` nullable=YES
- `Head` `int` nullable=YES
- `ChkSum` `int` nullable=YES
- `Date` `datetime` nullable=YES

### SecurityDB.dbo.AttackLog
- `id` `int` nullable=NO
- `accountId` `varchar` nullable=YES
- `IP` `varchar` nullable=YES
- `description` `varchar` nullable=YES
- `date` `datetime` nullable=NO

### SecurityDB.dbo.CheatLog
- `ID` `bigint` nullable=NO
- `IP` `varchar` nullable=NO
- `AccountName` `varchar` nullable=NO
- `Action` `int` nullable=NO
- `LogID` `int` nullable=NO
- `Description` `varchar` nullable=NO
- `Date` `varchar` nullable=NO

### SecurityDB.dbo.FunctionChecksumList
- `ID` `int` nullable=NO
- `Size` `int` nullable=NO
- `Address` `varchar` nullable=NO
- `Checksum` `varchar` nullable=NO

### SecurityDB.dbo.Security
- `AccountId` `varchar` nullable=YES
- `IP` `varchar` nullable=YES
- `Date` `datetime` nullable=NO
- `Protocol` `varchar` nullable=NO

### SecurityDB.dbo.WindowCheatList
- `ID` `int` nullable=NO
- `WindowName` `varchar` nullable=NO

## ServerDB

### ServerDB.dbo.BoxRewards
- `Kind` `int` nullable=YES
- `ItemCode` `varchar` nullable=YES
- `Count` `int` nullable=YES
- `Class` `int` nullable=YES

### ServerDB.dbo.DropLogAlert
- `ItemCode` `varchar` nullable=YES

### ServerDB.dbo.NoticeSystem
- `ID` `int` nullable=NO
- `Message` `varchar` nullable=YES
- `Type` `int` nullable=YES
- `Activated` `int` nullable=YES

### ServerDB.dbo.PVP
- `ID` `varchar` nullable=YES
- `Name` `varchar` nullable=YES
- `Class` `int` nullable=YES
- `Lvl` `int` nullable=YES
- `Kills` `int` nullable=YES
- `Deads` `int` nullable=YES

### ServerDB.dbo.PremiumData
- `ID` `int` nullable=NO
- `CharacterName` `varchar` nullable=NO
- `TimeLeft` `int` nullable=NO
- `ItemType` `int` nullable=NO

### ServerDB.dbo.Vip
- `ID` `int` nullable=NO
- `PlayerName` `nvarchar` nullable=NO
- `IdVIp` `int` nullable=NO
- `DateStart` `datetime` nullable=YES
- `DateEnd` `datetime` nullable=YES

## SoD2DB

### SoD2DB.dbo.SOD2RecBySandurr
- `SNo` `int` nullable=NO
- `UserID` `varchar` nullable=YES
- `CharName` `varchar` nullable=YES
- `CharType` `int` nullable=YES
- `Point` `int` nullable=YES
- `KillCount` `int` nullable=YES
- `GLevel` `int` nullable=YES
- `TotalPoint` `int` nullable=YES
- `TotalUser` `int` nullable=YES
- `SuccessUser` `int` nullable=YES
- `ServerName` `varchar` nullable=YES
- `PCRNo` `int` nullable=YES
- `GPCode` `varchar` nullable=YES
- `BusinessName` `varchar` nullable=YES
- `BAddress1` `varchar` nullable=YES
- `PMNo` `int` nullable=YES
- `Contribute` `int` nullable=YES
- `RegistDay` `nvarchar` nullable=YES
- `ServerTime` `bigint` nullable=YES
- `Flag` `int` nullable=YES
- `ClanIDX` `int` nullable=YES
- `ClanName` `varchar` nullable=YES

### SoD2DB.dbo.SoD2BBSMain
- `IDX` `int` nullable=NO
- `Userid` `varchar` nullable=NO
- `WriteName` `varchar` nullable=NO
- `Title` `varchar` nullable=YES
- `Job` `int` nullable=YES
- `Content` `text` nullable=NO
- `SelServer` `int` nullable=NO
- `RegiDate` `datetime` nullable=NO
- `RegiIP` `char` nullable=NO
- `Hit` `int` nullable=NO
- `CountCom` `int` nullable=NO
- `OwnerID` `varchar` nullable=YES

### SoD2DB.dbo.SoD2BBSSub
- `SIDX` `int` nullable=NO
- `MINDEX` `int` nullable=NO
- `Userid` `varchar` nullable=NO
- `WriteName` `varchar` nullable=NO
- `Content` `text` nullable=NO
- `RegiDate` `datetime` nullable=NO
- `RegiIP` `char` nullable=NO

### SoD2DB.dbo.SoD2Notice
- `IDX` `int` nullable=NO
- `MIDX` `int` nullable=NO
- `SelServer` `int` nullable=NO
- `UserID` `varchar` nullable=NO
- `Notice` `text` nullable=NO
- `RegiDate` `datetime` nullable=NO

### SoD2DB.dbo.Sod2Config
- `MIDX` `int` nullable=YES
- `ClanName` `varchar` nullable=YES
- `UserID` `varchar` nullable=YES
- `CharName` `varchar` nullable=YES
- `SelServer` `int` nullable=NO
- `ServerName` `varchar` nullable=YES
- `TaxRates` `int` nullable=NO
- `OperFrom` `datetime` nullable=NO
- `OperTo` `datetime` nullable=NO
- `TotalEnterMoney` `bigint` nullable=YES
- `BeforeMoney` `bigint` nullable=YES
- `TodayMoney` `bigint` nullable=YES
- `TotalMoney` `bigint` nullable=YES
- `TotalEnterNum` `int` nullable=YES
- `UpdateDate` `datetime` nullable=YES
- `Note` `text` nullable=YES
- `MIDX1` `int` nullable=YES
- `MIDX2` `int` nullable=YES
- `M0money` `bigint` nullable=YES
- `M1money` `bigint` nullable=YES
- `M2money` `bigint` nullable=YES
- `VInfo` `text` nullable=YES

### SoD2DB.dbo.Sod2ConfigLog
- `IDX` `int` nullable=NO
- `MIDX` `int` nullable=YES
- `ClanName` `varchar` nullable=YES
- `UserID` `varchar` nullable=YES
- `CharName` `varchar` nullable=YES
- `SelServer` `int` nullable=NO
- `ServerName` `varchar` nullable=YES
- `TaxRates` `int` nullable=NO
- `OperFrom` `datetime` nullable=NO
- `OperTo` `datetime` nullable=NO
- `TotalEnterMoney` `bigint` nullable=YES
- `BeforeMoney` `bigint` nullable=YES
- `TodayMoney` `bigint` nullable=YES
- `TotalMoney` `bigint` nullable=YES
- `TotalEnterNum` `int` nullable=YES
- `UpdateDate` `datetime` nullable=YES
- `Note` `text` nullable=YES
- `MIDX1` `int` nullable=YES
- `MIDX2` `int` nullable=YES
- `M0money` `bigint` nullable=YES
- `M1money` `bigint` nullable=YES
- `M2money` `bigint` nullable=YES

### SoD2DB.dbo.Sod2TaxLog
- `IDX` `int` nullable=NO
- `MIDX` `int` nullable=YES
- `ClanName` `varchar` nullable=YES
- `MCount` `int` nullable=YES
- `UserID` `varchar` nullable=YES
- `CharName` `varchar` nullable=YES
- `SelServer` `int` nullable=YES
- `TaxRates` `int` nullable=YES
- `OperFrom` `datetime` nullable=YES
- `OperTo` `datetime` nullable=YES
- `RegiDate` `datetime` nullable=YES

### SoD2DB.dbo.UserBlackList
- `SOD2No` `int` nullable=NO
- `UserID` `varchar` nullable=NO
- `CharName` `varchar` nullable=NO
- `ServerCode` `int` nullable=NO
- `Memo` `varchar` nullable=YES
- `RegistDay` `datetime` nullable=YES

### SoD2DB.dbo.temp
- `IDX` `int` nullable=NO
- `CIDX` `int` nullable=NO
- `CServerTime` `bigint` nullable=NO
- `Server` `int` nullable=NO
- `ClanName` `varchar` nullable=NO
- `ServerName` `varchar` nullable=NO
- `CPoint` `int` nullable=NO
- `CRegistDay` `datetime` nullable=NO
- `ServerTime_Temp` `bigint` nullable=YES
- `Point_Temp` `int` nullable=YES

### SoD2DB.dbo.test
- `IDX` `int` nullable=NO
- `CIDX` `int` nullable=NO
- `CServerTime` `bigint` nullable=NO
- `Server` `int` nullable=NO
- `ClanName` `nvarchar` nullable=NO
- `ServerName` `varchar` nullable=NO
- `CPoint` `int` nullable=NO
- `CRegistDay` `datetime` nullable=NO
- `ServerTime_Temp` `bigint` nullable=YES
- `Point_Temp` `int` nullable=YES

## SodDb

### SodDb.dbo.SOD0401
- `SODNo` `int` nullable=NO
- `UserID` `varchar` nullable=YES
- `CharName` `varchar` nullable=YES
- `CharType` `int` nullable=YES
- `ServerName` `varchar` nullable=YES
- `Point` `int` nullable=YES
- `Point_Lev` `int` nullable=YES
- `Point_Day` `datetime` nullable=YES
- `Ave` `int` nullable=YES
- `Ave_Lev` `int` nullable=YES
- `Ave_Day` `datetime` nullable=YES
- `KillCnt` `int` nullable=YES
- `KillCnt_Lev` `int` nullable=YES
- `KillCnt_Day` `datetime` nullable=YES
- `Contri` `int` nullable=YES
- `Contri_Lev` `int` nullable=YES
- `Contri_Day` `datetime` nullable=YES
- `TotPoint` `int` nullable=YES
- `Cnt` `int` nullable=YES
- `TotCnt` `int` nullable=YES
- `L1_Point` `int` nullable=YES
- `L1_Point_Lev` `int` nullable=YES
- `L1_Point_Day` `datetime` nullable=YES
- `L1_Ave` `int` nullable=YES
- `L1_Ave_Lev` `int` nullable=YES
- `L1_Ave_Day` `datetime` nullable=YES
- `L1_KillCnt` `int` nullable=YES
- `L1_KillCnt_Lev` `int` nullable=YES
- `L1_KillCnt_Day` `datetime` nullable=YES
- `L1_Contri` `int` nullable=YES
- `L1_Contri_Lev` `int` nullable=YES
- `L1_Contri_Day` `datetime` nullable=YES
- `L1_TotPoint` `int` nullable=YES
- `L1_Cnt` `int` nullable=YES
- `L1_TotCnt` `int` nullable=YES
- `L2_Point` `int` nullable=YES
- `L2_Point_Lev` `int` nullable=YES
- `L2_Point_Day` `datetime` nullable=YES
- `L2_Ave` `int` nullable=YES
- `L2_Ave_Lev` `int` nullable=YES
- `L2_Ave_Day` `datetime` nullable=YES
- `L2_KillCnt` `int` nullable=YES
- `L2_KillCnt_Lev` `int` nullable=YES
- `L2_KillCnt_Day` `datetime` nullable=YES
- `L2_Contri` `int` nullable=YES
- `L2_Contri_Lev` `int` nullable=YES
- `L2_Contri_Day` `datetime` nullable=YES
- `L2_TotPoint` `int` nullable=YES
- `L2_Cnt` `int` nullable=YES
- `L2_TotCnt` `int` nullable=YES
- `L3_Point` `int` nullable=YES
- `L3_Point_Lev` `int` nullable=YES
- `L3_Point_Day` `datetime` nullable=YES
- `L3_Ave` `int` nullable=YES
- `L3_Ave_Lev` `int` nullable=YES
- `L3_Ave_Day` `datetime` nullable=YES
- `L3_KillCnt` `int` nullable=YES
- `L3_KillCnt_Lev` `int` nullable=YES
- `L3_KillCnt_Day` `datetime` nullable=YES
- `L3_Contri` `int` nullable=YES
- `L3_Contri_Lev` `int` nullable=YES
- `L3_Contri_Day` `datetime` nullable=YES
- `L3_TotPoint` `int` nullable=YES
- `L3_Cnt` `int` nullable=YES
- `L3_TotCnt` `int` nullable=YES
- `L4_Point` `int` nullable=YES
- `L4_Point_Lev` `int` nullable=YES
- `L4_Point_Day` `datetime` nullable=YES
- `L4_Ave` `int` nullable=YES
- `L4_Ave_Lev` `int` nullable=YES
- `L4_Ave_Day` `datetime` nullable=YES
- `L4_KillCnt` `int` nullable=YES
- `L4_KillCnt_Lev` `int` nullable=YES
- `L4_KillCnt_Day` `datetime` nullable=YES
- `L4_Contri` `int` nullable=YES
- `L4_Contri_Lev` `int` nullable=YES
- `L4_Contri_Day` `datetime` nullable=YES
- `L4_TotPoint` `int` nullable=YES
- `L4_Cnt` `int` nullable=YES
- `L4_TotCnt` `int` nullable=YES
- `L5_Point` `int` nullable=YES
- `L5_Point_Lev` `int` nullable=YES
- `L5_Point_Day` `datetime` nullable=YES
- `L5_Ave` `int` nullable=YES
- `L5_Ave_Lev` `int` nullable=YES
- `L5_Ave_Day` `datetime` nullable=YES
- `L5_KillCnt` `int` nullable=YES
- `L5_KillCnt_Lev` `int` nullable=YES
- `L5_KillCnt_Day` `datetime` nullable=YES
- `L5_Contri` `int` nullable=YES
- `L5_Contri_Lev` `int` nullable=YES
- `L5_Contri_Day` `datetime` nullable=YES
- `L5_TotPoint` `int` nullable=YES
- `L5_Cnt` `int` nullable=YES
- `L5_TotCnt` `int` nullable=YES
- `L6_Point` `int` nullable=YES
- `L6_Point_Lev` `int` nullable=YES
- `L6_Point_Day` `datetime` nullable=YES
- `L6_Ave` `int` nullable=YES
- `L6_Ave_Lev` `int` nullable=YES
- `L6_Ave_Day` `datetime` nullable=YES
- `L6_KillCnt` `int` nullable=YES
- `L6_KillCnt_Lev` `int` nullable=YES
- `L6_KillCnt_Day` `datetime` nullable=YES
- `L6_Contri` `int` nullable=YES
- `L6_Contri_Lev` `int` nullable=YES
- `L6_Contri_Day` `datetime` nullable=YES
- `L6_TotPoint` `int` nullable=YES
- `L6_Cnt` `int` nullable=YES
- `L6_TotCnt` `int` nullable=YES
- `L7_Point` `int` nullable=YES
- `L7_Point_Lev` `int` nullable=YES
- `L7_Point_Day` `datetime` nullable=YES
- `L7_Ave` `int` nullable=YES
- `L7_Ave_Lev` `int` nullable=YES
- `L7_Ave_Day` `datetime` nullable=YES
- `L7_KillCnt` `int` nullable=YES
- `L7_KillCnt_Lev` `int` nullable=YES
- `L7_KillCnt_Day` `datetime` nullable=YES
- `L7_Contri` `int` nullable=YES
- `L7_Contri_Lev` `int` nullable=YES
- `L7_Contri_Day` `datetime` nullable=YES
- `L7_TotPoint` `int` nullable=YES
- `L7_Cnt` `int` nullable=YES
- `L7_TotCnt` `int` nullable=YES
- `L8_Point` `int` nullable=YES
- `L8_Point_Lev` `int` nullable=YES
- `L8_Point_Day` `datetime` nullable=YES
- `L8_Ave` `int` nullable=YES
- `L8_Ave_Lev` `int` nullable=YES
- `L8_Ave_Day` `datetime` nullable=YES
- `L8_KillCnt` `int` nullable=YES
- `L8_KillCnt_Lev` `int` nullable=YES
- `L8_KillCnt_Day` `datetime` nullable=YES
- `L8_Contri` `int` nullable=YES
- `L8_Contri_Lev` `int` nullable=YES
- `L8_Contri_Day` `datetime` nullable=YES
- `L8_TotPoint` `int` nullable=YES
- `L8_Cnt` `int` nullable=YES
- `L8_TotCnt` `int` nullable=YES
- `L9_Point` `int` nullable=YES
- `L9_Point_Lev` `int` nullable=YES
- `L9_Point_Day` `datetime` nullable=YES
- `L9_Ave` `int` nullable=YES
- `L9_Ave_Lev` `int` nullable=YES
- `L9_Ave_Day` `datetime` nullable=YES
- `L9_KillCnt` `int` nullable=YES
- `L9_KillCnt_Lev` `int` nullable=YES
- `L9_KillCnt_Day` `datetime` nullable=YES
- `L9_Contri` `int` nullable=YES
- `L9_Contri_Lev` `int` nullable=YES
- `L9_Contri_Day` `datetime` nullable=YES
- `L9_TotPoint` `int` nullable=YES
- `L9_Cnt` `int` nullable=YES
- `L9_TotCnt` `int` nullable=YES

### SodDb.dbo.SOD0402
- `SODNo` `int` nullable=NO
- `UserID` `varchar` nullable=YES
- `CharName` `varchar` nullable=YES
- `CharType` `int` nullable=YES
- `ServerName` `varchar` nullable=YES
- `Point` `int` nullable=YES
- `Point_Lev` `int` nullable=YES
- `Point_Day` `datetime` nullable=YES
- `Ave` `int` nullable=YES
- `Ave_Lev` `int` nullable=YES
- `Ave_Day` `datetime` nullable=YES
- `KillCnt` `int` nullable=YES
- `KillCnt_Lev` `int` nullable=YES
- `KillCnt_Day` `datetime` nullable=YES
- `Contri` `int` nullable=YES
- `Contri_Lev` `int` nullable=YES
- `Contri_Day` `datetime` nullable=YES
- `TotPoint` `int` nullable=YES
- `Cnt` `int` nullable=YES
- `TotCnt` `int` nullable=YES
- `L1_Point` `int` nullable=YES
- `L1_Point_Lev` `int` nullable=YES
- `L1_Point_Day` `datetime` nullable=YES
- `L1_Ave` `int` nullable=YES
- `L1_Ave_Lev` `int` nullable=YES
- `L1_Ave_Day` `datetime` nullable=YES
- `L1_KillCnt` `int` nullable=YES
- `L1_KillCnt_Lev` `int` nullable=YES
- `L1_KillCnt_Day` `datetime` nullable=YES
- `L1_Contri` `int` nullable=YES
- `L1_Contri_Lev` `int` nullable=YES
- `L1_Contri_Day` `datetime` nullable=YES
- `L1_TotPoint` `int` nullable=YES
- `L1_Cnt` `int` nullable=YES
- `L1_TotCnt` `int` nullable=YES
- `L2_Point` `int` nullable=YES
- `L2_Point_Lev` `int` nullable=YES
- `L2_Point_Day` `datetime` nullable=YES
- `L2_Ave` `int` nullable=YES
- `L2_Ave_Lev` `int` nullable=YES
- `L2_Ave_Day` `datetime` nullable=YES
- `L2_KillCnt` `int` nullable=YES
- `L2_KillCnt_Lev` `int` nullable=YES
- `L2_KillCnt_Day` `datetime` nullable=YES
- `L2_Contri` `int` nullable=YES
- `L2_Contri_Lev` `int` nullable=YES
- `L2_Contri_Day` `datetime` nullable=YES
- `L2_TotPoint` `int` nullable=YES
- `L2_Cnt` `int` nullable=YES
- `L2_TotCnt` `int` nullable=YES
- `L3_Point` `int` nullable=YES
- `L3_Point_Lev` `int` nullable=YES
- `L3_Point_Day` `datetime` nullable=YES
- `L3_Ave` `int` nullable=YES
- `L3_Ave_Lev` `int` nullable=YES
- `L3_Ave_Day` `datetime` nullable=YES
- `L3_KillCnt` `int` nullable=YES
- `L3_KillCnt_Lev` `int` nullable=YES
- `L3_KillCnt_Day` `datetime` nullable=YES
- `L3_Contri` `int` nullable=YES
- `L3_Contri_Lev` `int` nullable=YES
- `L3_Contri_Day` `datetime` nullable=YES
- `L3_TotPoint` `int` nullable=YES
- `L3_Cnt` `int` nullable=YES
- `L3_TotCnt` `int` nullable=YES
- `L4_Point` `int` nullable=YES
- `L4_Point_Lev` `int` nullable=YES
- `L4_Point_Day` `datetime` nullable=YES
- `L4_Ave` `int` nullable=YES
- `L4_Ave_Lev` `int` nullable=YES
- `L4_Ave_Day` `datetime` nullable=YES
- `L4_KillCnt` `int` nullable=YES
- `L4_KillCnt_Lev` `int` nullable=YES
- `L4_KillCnt_Day` `datetime` nullable=YES
- `L4_Contri` `int` nullable=YES
- `L4_Contri_Lev` `int` nullable=YES
- `L4_Contri_Day` `datetime` nullable=YES
- `L4_TotPoint` `int` nullable=YES
- `L4_Cnt` `int` nullable=YES
- `L4_TotCnt` `int` nullable=YES
- `L5_Point` `int` nullable=YES
- `L5_Point_Lev` `int` nullable=YES
- `L5_Point_Day` `datetime` nullable=YES
- `L5_Ave` `int` nullable=YES
- `L5_Ave_Lev` `int` nullable=YES
- `L5_Ave_Day` `datetime` nullable=YES
- `L5_KillCnt` `int` nullable=YES
- `L5_KillCnt_Lev` `int` nullable=YES
- `L5_KillCnt_Day` `datetime` nullable=YES
- `L5_Contri` `int` nullable=YES
- `L5_Contri_Lev` `int` nullable=YES
- `L5_Contri_Day` `datetime` nullable=YES
- `L5_TotPoint` `int` nullable=YES
- `L5_Cnt` `int` nullable=YES
- `L5_TotCnt` `int` nullable=YES
- `L6_Point` `int` nullable=YES
- `L6_Point_Lev` `int` nullable=YES
- `L6_Point_Day` `datetime` nullable=YES
- `L6_Ave` `int` nullable=YES
- `L6_Ave_Lev` `int` nullable=YES
- `L6_Ave_Day` `datetime` nullable=YES
- `L6_KillCnt` `int` nullable=YES
- `L6_KillCnt_Lev` `int` nullable=YES
- `L6_KillCnt_Day` `datetime` nullable=YES
- `L6_Contri` `int` nullable=YES
- `L6_Contri_Lev` `int` nullable=YES
- `L6_Contri_Day` `datetime` nullable=YES
- `L6_TotPoint` `int` nullable=YES
- `L6_Cnt` `int` nullable=YES
- `L6_TotCnt` `int` nullable=YES
- `L7_Point` `int` nullable=YES
- `L7_Point_Lev` `int` nullable=YES
- `L7_Point_Day` `datetime` nullable=YES
- `L7_Ave` `int` nullable=YES
- `L7_Ave_Lev` `int` nullable=YES
- `L7_Ave_Day` `datetime` nullable=YES
- `L7_KillCnt` `int` nullable=YES
- `L7_KillCnt_Lev` `int` nullable=YES
- `L7_KillCnt_Day` `datetime` nullable=YES
- `L7_Contri` `int` nullable=YES
- `L7_Contri_Lev` `int` nullable=YES
- `L7_Contri_Day` `datetime` nullable=YES
- `L7_TotPoint` `int` nullable=YES
- `L7_Cnt` `int` nullable=YES
- `L7_TotCnt` `int` nullable=YES
- `L8_Point` `int` nullable=YES
- `L8_Point_Lev` `int` nullable=YES
- `L8_Point_Day` `datetime` nullable=YES
- `L8_Ave` `int` nullable=YES
- `L8_Ave_Lev` `int` nullable=YES
- `L8_Ave_Day` `datetime` nullable=YES
- `L8_KillCnt` `int` nullable=YES
- `L8_KillCnt_Lev` `int` nullable=YES
- `L8_KillCnt_Day` `datetime` nullable=YES
- `L8_Contri` `int` nullable=YES
- `L8_Contri_Lev` `int` nullable=YES
- `L8_Contri_Day` `datetime` nullable=YES
- `L8_TotPoint` `int` nullable=YES
- `L8_Cnt` `int` nullable=YES
- `L8_TotCnt` `int` nullable=YES
- `L9_Point` `int` nullable=YES
- `L9_Point_Lev` `int` nullable=YES
- `L9_Point_Day` `datetime` nullable=YES
- `L9_Ave` `int` nullable=YES
- `L9_Ave_Lev` `int` nullable=YES
- `L9_Ave_Day` `datetime` nullable=YES
- `L9_KillCnt` `int` nullable=YES
- `L9_KillCnt_Lev` `int` nullable=YES
- `L9_KillCnt_Day` `datetime` nullable=YES
- `L9_Contri` `int` nullable=YES
- `L9_Contri_Lev` `int` nullable=YES
- `L9_Contri_Day` `datetime` nullable=YES
- `L9_TotPoint` `int` nullable=YES
- `L9_Cnt` `int` nullable=YES
- `L9_TotCnt` `int` nullable=YES

### SodDb.dbo.SOD0403
- `SODNo` `int` nullable=NO
- `UserID` `varchar` nullable=YES
- `CharName` `varchar` nullable=YES
- `CharType` `int` nullable=YES
- `ServerName` `varchar` nullable=YES
- `Point` `int` nullable=YES
- `Point_Lev` `int` nullable=YES
- `Point_Day` `datetime` nullable=YES
- `Ave` `int` nullable=YES
- `Ave_Lev` `int` nullable=YES
- `Ave_Day` `datetime` nullable=YES
- `KillCnt` `int` nullable=YES
- `KillCnt_Lev` `int` nullable=YES
- `KillCnt_Day` `datetime` nullable=YES
- `Contri` `int` nullable=YES
- `Contri_Lev` `int` nullable=YES
- `Contri_Day` `datetime` nullable=YES
- `TotPoint` `int` nullable=YES
- `Cnt` `int` nullable=YES
- `TotCnt` `int` nullable=YES
- `L1_Point` `int` nullable=YES
- `L1_Point_Lev` `int` nullable=YES
- `L1_Point_Day` `datetime` nullable=YES
- `L1_Ave` `int` nullable=YES
- `L1_Ave_Lev` `int` nullable=YES
- `L1_Ave_Day` `datetime` nullable=YES
- `L1_KillCnt` `int` nullable=YES
- `L1_KillCnt_Lev` `int` nullable=YES
- `L1_KillCnt_Day` `datetime` nullable=YES
- `L1_Contri` `int` nullable=YES
- `L1_Contri_Lev` `int` nullable=YES
- `L1_Contri_Day` `datetime` nullable=YES
- `L1_TotPoint` `int` nullable=YES
- `L1_Cnt` `int` nullable=YES
- `L1_TotCnt` `int` nullable=YES
- `L2_Point` `int` nullable=YES
- `L2_Point_Lev` `int` nullable=YES
- `L2_Point_Day` `datetime` nullable=YES
- `L2_Ave` `int` nullable=YES
- `L2_Ave_Lev` `int` nullable=YES
- `L2_Ave_Day` `datetime` nullable=YES
- `L2_KillCnt` `int` nullable=YES
- `L2_KillCnt_Lev` `int` nullable=YES
- `L2_KillCnt_Day` `datetime` nullable=YES
- `L2_Contri` `int` nullable=YES
- `L2_Contri_Lev` `int` nullable=YES
- `L2_Contri_Day` `datetime` nullable=YES
- `L2_TotPoint` `int` nullable=YES
- `L2_Cnt` `int` nullable=YES
- `L2_TotCnt` `int` nullable=YES
- `L3_Point` `int` nullable=YES
- `L3_Point_Lev` `int` nullable=YES
- `L3_Point_Day` `datetime` nullable=YES
- `L3_Ave` `int` nullable=YES
- `L3_Ave_Lev` `int` nullable=YES
- `L3_Ave_Day` `datetime` nullable=YES
- `L3_KillCnt` `int` nullable=YES
- `L3_KillCnt_Lev` `int` nullable=YES
- `L3_KillCnt_Day` `datetime` nullable=YES
- `L3_Contri` `int` nullable=YES
- `L3_Contri_Lev` `int` nullable=YES
- `L3_Contri_Day` `datetime` nullable=YES
- `L3_TotPoint` `int` nullable=YES
- `L3_Cnt` `int` nullable=YES
- `L3_TotCnt` `int` nullable=YES
- `L4_Point` `int` nullable=YES
- `L4_Point_Lev` `int` nullable=YES
- `L4_Point_Day` `datetime` nullable=YES
- `L4_Ave` `int` nullable=YES
- `L4_Ave_Lev` `int` nullable=YES
- `L4_Ave_Day` `datetime` nullable=YES
- `L4_KillCnt` `int` nullable=YES
- `L4_KillCnt_Lev` `int` nullable=YES
- `L4_KillCnt_Day` `datetime` nullable=YES
- `L4_Contri` `int` nullable=YES
- `L4_Contri_Lev` `int` nullable=YES
- `L4_Contri_Day` `datetime` nullable=YES
- `L4_TotPoint` `int` nullable=YES
- `L4_Cnt` `int` nullable=YES
- `L4_TotCnt` `int` nullable=YES
- `L5_Point` `int` nullable=YES
- `L5_Point_Lev` `int` nullable=YES
- `L5_Point_Day` `datetime` nullable=YES
- `L5_Ave` `int` nullable=YES
- `L5_Ave_Lev` `int` nullable=YES
- `L5_Ave_Day` `datetime` nullable=YES
- `L5_KillCnt` `int` nullable=YES
- `L5_KillCnt_Lev` `int` nullable=YES
- `L5_KillCnt_Day` `datetime` nullable=YES
- `L5_Contri` `int` nullable=YES
- `L5_Contri_Lev` `int` nullable=YES
- `L5_Contri_Day` `datetime` nullable=YES
- `L5_TotPoint` `int` nullable=YES
- `L5_Cnt` `int` nullable=YES
- `L5_TotCnt` `int` nullable=YES
- `L6_Point` `int` nullable=YES
- `L6_Point_Lev` `int` nullable=YES
- `L6_Point_Day` `datetime` nullable=YES
- `L6_Ave` `int` nullable=YES
- `L6_Ave_Lev` `int` nullable=YES
- `L6_Ave_Day` `datetime` nullable=YES
- `L6_KillCnt` `int` nullable=YES
- `L6_KillCnt_Lev` `int` nullable=YES
- `L6_KillCnt_Day` `datetime` nullable=YES
- `L6_Contri` `int` nullable=YES
- `L6_Contri_Lev` `int` nullable=YES
- `L6_Contri_Day` `datetime` nullable=YES
- `L6_TotPoint` `int` nullable=YES
- `L6_Cnt` `int` nullable=YES
- `L6_TotCnt` `int` nullable=YES
- `L7_Point` `int` nullable=YES
- `L7_Point_Lev` `int` nullable=YES
- `L7_Point_Day` `datetime` nullable=YES
- `L7_Ave` `int` nullable=YES
- `L7_Ave_Lev` `int` nullable=YES
- `L7_Ave_Day` `datetime` nullable=YES
- `L7_KillCnt` `int` nullable=YES
- `L7_KillCnt_Lev` `int` nullable=YES
- `L7_KillCnt_Day` `datetime` nullable=YES
- `L7_Contri` `int` nullable=YES
- `L7_Contri_Lev` `int` nullable=YES
- `L7_Contri_Day` `datetime` nullable=YES
- `L7_TotPoint` `int` nullable=YES
- `L7_Cnt` `int` nullable=YES
- `L7_TotCnt` `int` nullable=YES
- `L8_Point` `int` nullable=YES
- `L8_Point_Lev` `int` nullable=YES
- `L8_Point_Day` `datetime` nullable=YES
- `L8_Ave` `int` nullable=YES
- `L8_Ave_Lev` `int` nullable=YES
- `L8_Ave_Day` `datetime` nullable=YES
- `L8_KillCnt` `int` nullable=YES
- `L8_KillCnt_Lev` `int` nullable=YES
- `L8_KillCnt_Day` `datetime` nullable=YES
- `L8_Contri` `int` nullable=YES
- `L8_Contri_Lev` `int` nullable=YES
- `L8_Contri_Day` `datetime` nullable=YES
- `L8_TotPoint` `int` nullable=YES
- `L8_Cnt` `int` nullable=YES
- `L8_TotCnt` `int` nullable=YES
- `L9_Point` `int` nullable=YES
- `L9_Point_Lev` `int` nullable=YES
- `L9_Point_Day` `datetime` nullable=YES
- `L9_Ave` `int` nullable=YES
- `L9_Ave_Lev` `int` nullable=YES
- `L9_Ave_Day` `datetime` nullable=YES
- `L9_KillCnt` `int` nullable=YES
- `L9_KillCnt_Lev` `int` nullable=YES
- `L9_KillCnt_Day` `datetime` nullable=YES
- `L9_Contri` `int` nullable=YES
- `L9_Contri_Lev` `int` nullable=YES
- `L9_Contri_Day` `datetime` nullable=YES
- `L9_TotPoint` `int` nullable=YES
- `L9_Cnt` `int` nullable=YES
- `L9_TotCnt` `int` nullable=YES

### SodDb.dbo.SOD0404
- `SODNo` `int` nullable=NO
- `UserID` `varchar` nullable=YES
- `CharName` `varchar` nullable=YES
- `CharType` `int` nullable=YES
- `ServerName` `varchar` nullable=YES
- `Point` `int` nullable=YES
- `Point_Lev` `int` nullable=YES
- `Point_Day` `datetime` nullable=YES
- `Ave` `int` nullable=YES
- `Ave_Lev` `int` nullable=YES
- `Ave_Day` `datetime` nullable=YES
- `KillCnt` `int` nullable=YES
- `KillCnt_Lev` `int` nullable=YES
- `KillCnt_Day` `datetime` nullable=YES
- `Contri` `int` nullable=YES
- `Contri_Lev` `int` nullable=YES
- `Contri_Day` `datetime` nullable=YES
- `TotPoint` `int` nullable=YES
- `Cnt` `int` nullable=YES
- `TotCnt` `int` nullable=YES
- `L1_Point` `int` nullable=YES
- `L1_Point_Lev` `int` nullable=YES
- `L1_Point_Day` `datetime` nullable=YES
- `L1_Ave` `int` nullable=YES
- `L1_Ave_Lev` `int` nullable=YES
- `L1_Ave_Day` `datetime` nullable=YES
- `L1_KillCnt` `int` nullable=YES
- `L1_KillCnt_Lev` `int` nullable=YES
- `L1_KillCnt_Day` `datetime` nullable=YES
- `L1_Contri` `int` nullable=YES
- `L1_Contri_Lev` `int` nullable=YES
- `L1_Contri_Day` `datetime` nullable=YES
- `L1_TotPoint` `int` nullable=YES
- `L1_Cnt` `int` nullable=YES
- `L1_TotCnt` `int` nullable=YES
- `L2_Point` `int` nullable=YES
- `L2_Point_Lev` `int` nullable=YES
- `L2_Point_Day` `datetime` nullable=YES
- `L2_Ave` `int` nullable=YES
- `L2_Ave_Lev` `int` nullable=YES
- `L2_Ave_Day` `datetime` nullable=YES
- `L2_KillCnt` `int` nullable=YES
- `L2_KillCnt_Lev` `int` nullable=YES
- `L2_KillCnt_Day` `datetime` nullable=YES
- `L2_Contri` `int` nullable=YES
- `L2_Contri_Lev` `int` nullable=YES
- `L2_Contri_Day` `datetime` nullable=YES
- `L2_TotPoint` `int` nullable=YES
- `L2_Cnt` `int` nullable=YES
- `L2_TotCnt` `int` nullable=YES
- `L3_Point` `int` nullable=YES
- `L3_Point_Lev` `int` nullable=YES
- `L3_Point_Day` `datetime` nullable=YES
- `L3_Ave` `int` nullable=YES
- `L3_Ave_Lev` `int` nullable=YES
- `L3_Ave_Day` `datetime` nullable=YES
- `L3_KillCnt` `int` nullable=YES
- `L3_KillCnt_Lev` `int` nullable=YES
- `L3_KillCnt_Day` `datetime` nullable=YES
- `L3_Contri` `int` nullable=YES
- `L3_Contri_Lev` `int` nullable=YES
- `L3_Contri_Day` `datetime` nullable=YES
- `L3_TotPoint` `int` nullable=YES
- `L3_Cnt` `int` nullable=YES
- `L3_TotCnt` `int` nullable=YES
- `L4_Point` `int` nullable=YES
- `L4_Point_Lev` `int` nullable=YES
- `L4_Point_Day` `datetime` nullable=YES
- `L4_Ave` `int` nullable=YES
- `L4_Ave_Lev` `int` nullable=YES
- `L4_Ave_Day` `datetime` nullable=YES
- `L4_KillCnt` `int` nullable=YES
- `L4_KillCnt_Lev` `int` nullable=YES
- `L4_KillCnt_Day` `datetime` nullable=YES
- `L4_Contri` `int` nullable=YES
- `L4_Contri_Lev` `int` nullable=YES
- `L4_Contri_Day` `datetime` nullable=YES
- `L4_TotPoint` `int` nullable=YES
- `L4_Cnt` `int` nullable=YES
- `L4_TotCnt` `int` nullable=YES
- `L5_Point` `int` nullable=YES
- `L5_Point_Lev` `int` nullable=YES
- `L5_Point_Day` `datetime` nullable=YES
- `L5_Ave` `int` nullable=YES
- `L5_Ave_Lev` `int` nullable=YES
- `L5_Ave_Day` `datetime` nullable=YES
- `L5_KillCnt` `int` nullable=YES
- `L5_KillCnt_Lev` `int` nullable=YES
- `L5_KillCnt_Day` `datetime` nullable=YES
- `L5_Contri` `int` nullable=YES
- `L5_Contri_Lev` `int` nullable=YES
- `L5_Contri_Day` `datetime` nullable=YES
- `L5_TotPoint` `int` nullable=YES
- `L5_Cnt` `int` nullable=YES
- `L5_TotCnt` `int` nullable=YES
- `L6_Point` `int` nullable=YES
- `L6_Point_Lev` `int` nullable=YES
- `L6_Point_Day` `datetime` nullable=YES
- `L6_Ave` `int` nullable=YES
- `L6_Ave_Lev` `int` nullable=YES
- `L6_Ave_Day` `datetime` nullable=YES
- `L6_KillCnt` `int` nullable=YES
- `L6_KillCnt_Lev` `int` nullable=YES
- `L6_KillCnt_Day` `datetime` nullable=YES
- `L6_Contri` `int` nullable=YES
- `L6_Contri_Lev` `int` nullable=YES
- `L6_Contri_Day` `datetime` nullable=YES
- `L6_TotPoint` `int` nullable=YES
- `L6_Cnt` `int` nullable=YES
- `L6_TotCnt` `int` nullable=YES
- `L7_Point` `int` nullable=YES
- `L7_Point_Lev` `int` nullable=YES
- `L7_Point_Day` `datetime` nullable=YES
- `L7_Ave` `int` nullable=YES
- `L7_Ave_Lev` `int` nullable=YES
- `L7_Ave_Day` `datetime` nullable=YES
- `L7_KillCnt` `int` nullable=YES
- `L7_KillCnt_Lev` `int` nullable=YES
- `L7_KillCnt_Day` `datetime` nullable=YES
- `L7_Contri` `int` nullable=YES
- `L7_Contri_Lev` `int` nullable=YES
- `L7_Contri_Day` `datetime` nullable=YES
- `L7_TotPoint` `int` nullable=YES
- `L7_Cnt` `int` nullable=YES
- `L7_TotCnt` `int` nullable=YES
- `L8_Point` `int` nullable=YES
- `L8_Point_Lev` `int` nullable=YES
- `L8_Point_Day` `datetime` nullable=YES
- `L8_Ave` `int` nullable=YES
- `L8_Ave_Lev` `int` nullable=YES
- `L8_Ave_Day` `datetime` nullable=YES
- `L8_KillCnt` `int` nullable=YES
- `L8_KillCnt_Lev` `int` nullable=YES
- `L8_KillCnt_Day` `datetime` nullable=YES
- `L8_Contri` `int` nullable=YES
- `L8_Contri_Lev` `int` nullable=YES
- `L8_Contri_Day` `datetime` nullable=YES
- `L8_TotPoint` `int` nullable=YES
- `L8_Cnt` `int` nullable=YES
- `L8_TotCnt` `int` nullable=YES
- `L9_Point` `int` nullable=YES
- `L9_Point_Lev` `int` nullable=YES
- `L9_Point_Day` `datetime` nullable=YES
- `L9_Ave` `int` nullable=YES
- `L9_Ave_Lev` `int` nullable=YES
- `L9_Ave_Day` `datetime` nullable=YES
- `L9_KillCnt` `int` nullable=YES
- `L9_KillCnt_Lev` `int` nullable=YES
- `L9_KillCnt_Day` `datetime` nullable=YES
- `L9_Contri` `int` nullable=YES
- `L9_Contri_Lev` `int` nullable=YES
- `L9_Contri_Day` `datetime` nullable=YES
- `L9_TotPoint` `int` nullable=YES
- `L9_Cnt` `int` nullable=YES
- `L9_TotCnt` `int` nullable=YES

### SodDb.dbo.SOD0405
- `SODNo` `int` nullable=NO
- `UserID` `varchar` nullable=YES
- `CharName` `varchar` nullable=YES
- `CharType` `int` nullable=YES
- `ServerName` `varchar` nullable=YES
- `Point` `int` nullable=YES
- `Point_Lev` `int` nullable=YES
- `Point_Day` `datetime` nullable=YES
- `Ave` `int` nullable=YES
- `Ave_Lev` `int` nullable=YES
- `Ave_Day` `datetime` nullable=YES
- `KillCnt` `int` nullable=YES
- `KillCnt_Lev` `int` nullable=YES
- `KillCnt_Day` `datetime` nullable=YES
- `Contri` `int` nullable=YES
- `Contri_Lev` `int` nullable=YES
- `Contri_Day` `datetime` nullable=YES
- `TotPoint` `int` nullable=YES
- `Cnt` `int` nullable=YES
- `TotCnt` `int` nullable=YES
- `L1_Point` `int` nullable=YES
- `L1_Point_Lev` `int` nullable=YES
- `L1_Point_Day` `datetime` nullable=YES
- `L1_Ave` `int` nullable=YES
- `L1_Ave_Lev` `int` nullable=YES
- `L1_Ave_Day` `datetime` nullable=YES
- `L1_KillCnt` `int` nullable=YES
- `L1_KillCnt_Lev` `int` nullable=YES
- `L1_KillCnt_Day` `datetime` nullable=YES
- `L1_Contri` `int` nullable=YES
- `L1_Contri_Lev` `int` nullable=YES
- `L1_Contri_Day` `datetime` nullable=YES
- `L1_TotPoint` `int` nullable=YES
- `L1_Cnt` `int` nullable=YES
- `L1_TotCnt` `int` nullable=YES
- `L2_Point` `int` nullable=YES
- `L2_Point_Lev` `int` nullable=YES
- `L2_Point_Day` `datetime` nullable=YES
- `L2_Ave` `int` nullable=YES
- `L2_Ave_Lev` `int` nullable=YES
- `L2_Ave_Day` `datetime` nullable=YES
- `L2_KillCnt` `int` nullable=YES
- `L2_KillCnt_Lev` `int` nullable=YES
- `L2_KillCnt_Day` `datetime` nullable=YES
- `L2_Contri` `int` nullable=YES
- `L2_Contri_Lev` `int` nullable=YES
- `L2_Contri_Day` `datetime` nullable=YES
- `L2_TotPoint` `int` nullable=YES
- `L2_Cnt` `int` nullable=YES
- `L2_TotCnt` `int` nullable=YES
- `L3_Point` `int` nullable=YES
- `L3_Point_Lev` `int` nullable=YES
- `L3_Point_Day` `datetime` nullable=YES
- `L3_Ave` `int` nullable=YES
- `L3_Ave_Lev` `int` nullable=YES
- `L3_Ave_Day` `datetime` nullable=YES
- `L3_KillCnt` `int` nullable=YES
- `L3_KillCnt_Lev` `int` nullable=YES
- `L3_KillCnt_Day` `datetime` nullable=YES
- `L3_Contri` `int` nullable=YES
- `L3_Contri_Lev` `int` nullable=YES
- `L3_Contri_Day` `datetime` nullable=YES
- `L3_TotPoint` `int` nullable=YES
- `L3_Cnt` `int` nullable=YES
- `L3_TotCnt` `int` nullable=YES
- `L4_Point` `int` nullable=YES
- `L4_Point_Lev` `int` nullable=YES
- `L4_Point_Day` `datetime` nullable=YES
- `L4_Ave` `int` nullable=YES
- `L4_Ave_Lev` `int` nullable=YES
- `L4_Ave_Day` `datetime` nullable=YES
- `L4_KillCnt` `int` nullable=YES
- `L4_KillCnt_Lev` `int` nullable=YES
- `L4_KillCnt_Day` `datetime` nullable=YES
- `L4_Contri` `int` nullable=YES
- `L4_Contri_Lev` `int` nullable=YES
- `L4_Contri_Day` `datetime` nullable=YES
- `L4_TotPoint` `int` nullable=YES
- `L4_Cnt` `int` nullable=YES
- `L4_TotCnt` `int` nullable=YES
- `L5_Point` `int` nullable=YES
- `L5_Point_Lev` `int` nullable=YES
- `L5_Point_Day` `datetime` nullable=YES
- `L5_Ave` `int` nullable=YES
- `L5_Ave_Lev` `int` nullable=YES
- `L5_Ave_Day` `datetime` nullable=YES
- `L5_KillCnt` `int` nullable=YES
- `L5_KillCnt_Lev` `int` nullable=YES
- `L5_KillCnt_Day` `datetime` nullable=YES
- `L5_Contri` `int` nullable=YES
- `L5_Contri_Lev` `int` nullable=YES
- `L5_Contri_Day` `datetime` nullable=YES
- `L5_TotPoint` `int` nullable=YES
- `L5_Cnt` `int` nullable=YES
- `L5_TotCnt` `int` nullable=YES
- `L6_Point` `int` nullable=YES
- `L6_Point_Lev` `int` nullable=YES
- `L6_Point_Day` `datetime` nullable=YES
- `L6_Ave` `int` nullable=YES
- `L6_Ave_Lev` `int` nullable=YES
- `L6_Ave_Day` `datetime` nullable=YES
- `L6_KillCnt` `int` nullable=YES
- `L6_KillCnt_Lev` `int` nullable=YES
- `L6_KillCnt_Day` `datetime` nullable=YES
- `L6_Contri` `int` nullable=YES
- `L6_Contri_Lev` `int` nullable=YES
- `L6_Contri_Day` `datetime` nullable=YES
- `L6_TotPoint` `int` nullable=YES
- `L6_Cnt` `int` nullable=YES
- `L6_TotCnt` `int` nullable=YES
- `L7_Point` `int` nullable=YES
- `L7_Point_Lev` `int` nullable=YES
- `L7_Point_Day` `datetime` nullable=YES
- `L7_Ave` `int` nullable=YES
- `L7_Ave_Lev` `int` nullable=YES
- `L7_Ave_Day` `datetime` nullable=YES
- `L7_KillCnt` `int` nullable=YES
- `L7_KillCnt_Lev` `int` nullable=YES
- `L7_KillCnt_Day` `datetime` nullable=YES
- `L7_Contri` `int` nullable=YES
- `L7_Contri_Lev` `int` nullable=YES
- `L7_Contri_Day` `datetime` nullable=YES
- `L7_TotPoint` `int` nullable=YES
- `L7_Cnt` `int` nullable=YES
- `L7_TotCnt` `int` nullable=YES
- `L8_Point` `int` nullable=YES
- `L8_Point_Lev` `int` nullable=YES
- `L8_Point_Day` `datetime` nullable=YES
- `L8_Ave` `int` nullable=YES
- `L8_Ave_Lev` `int` nullable=YES
- `L8_Ave_Day` `datetime` nullable=YES
- `L8_KillCnt` `int` nullable=YES
- `L8_KillCnt_Lev` `int` nullable=YES
- `L8_KillCnt_Day` `datetime` nullable=YES
- `L8_Contri` `int` nullable=YES
- `L8_Contri_Lev` `int` nullable=YES
- `L8_Contri_Day` `datetime` nullable=YES
- `L8_TotPoint` `int` nullable=YES
- `L8_Cnt` `int` nullable=YES
- `L8_TotCnt` `int` nullable=YES
- `L9_Point` `int` nullable=YES
- `L9_Point_Lev` `int` nullable=YES
- `L9_Point_Day` `datetime` nullable=YES
- `L9_Ave` `int` nullable=YES
- `L9_Ave_Lev` `int` nullable=YES
- `L9_Ave_Day` `datetime` nullable=YES
- `L9_KillCnt` `int` nullable=YES
- `L9_KillCnt_Lev` `int` nullable=YES
- `L9_KillCnt_Day` `datetime` nullable=YES
- `L9_Contri` `int` nullable=YES
- `L9_Contri_Lev` `int` nullable=YES
- `L9_Contri_Day` `datetime` nullable=YES
- `L9_TotPoint` `int` nullable=YES
- `L9_Cnt` `int` nullable=YES
- `L9_TotCnt` `int` nullable=YES

### SodDb.dbo.SOD0406
- `SODNo` `int` nullable=NO
- `UserID` `varchar` nullable=YES
- `CharName` `varchar` nullable=YES
- `CharType` `int` nullable=YES
- `ServerName` `varchar` nullable=YES
- `Point` `int` nullable=YES
- `Point_Lev` `int` nullable=YES
- `Point_Day` `datetime` nullable=YES
- `Ave` `int` nullable=YES
- `Ave_Lev` `int` nullable=YES
- `Ave_Day` `datetime` nullable=YES
- `KillCnt` `int` nullable=YES
- `KillCnt_Lev` `int` nullable=YES
- `KillCnt_Day` `datetime` nullable=YES
- `Contri` `int` nullable=YES
- `Contri_Lev` `int` nullable=YES
- `Contri_Day` `datetime` nullable=YES
- `TotPoint` `int` nullable=YES
- `Cnt` `int` nullable=YES
- `TotCnt` `int` nullable=YES
- `L1_Point` `int` nullable=YES
- `L1_Point_Lev` `int` nullable=YES
- `L1_Point_Day` `datetime` nullable=YES
- `L1_Ave` `int` nullable=YES
- `L1_Ave_Lev` `int` nullable=YES
- `L1_Ave_Day` `datetime` nullable=YES
- `L1_KillCnt` `int` nullable=YES
- `L1_KillCnt_Lev` `int` nullable=YES
- `L1_KillCnt_Day` `datetime` nullable=YES
- `L1_Contri` `int` nullable=YES
- `L1_Contri_Lev` `int` nullable=YES
- `L1_Contri_Day` `datetime` nullable=YES
- `L1_TotPoint` `int` nullable=YES
- `L1_Cnt` `int` nullable=YES
- `L1_TotCnt` `int` nullable=YES
- `L2_Point` `int` nullable=YES
- `L2_Point_Lev` `int` nullable=YES
- `L2_Point_Day` `datetime` nullable=YES
- `L2_Ave` `int` nullable=YES
- `L2_Ave_Lev` `int` nullable=YES
- `L2_Ave_Day` `datetime` nullable=YES
- `L2_KillCnt` `int` nullable=YES
- `L2_KillCnt_Lev` `int` nullable=YES
- `L2_KillCnt_Day` `datetime` nullable=YES
- `L2_Contri` `int` nullable=YES
- `L2_Contri_Lev` `int` nullable=YES
- `L2_Contri_Day` `datetime` nullable=YES
- `L2_TotPoint` `int` nullable=YES
- `L2_Cnt` `int` nullable=YES
- `L2_TotCnt` `int` nullable=YES
- `L3_Point` `int` nullable=YES
- `L3_Point_Lev` `int` nullable=YES
- `L3_Point_Day` `datetime` nullable=YES
- `L3_Ave` `int` nullable=YES
- `L3_Ave_Lev` `int` nullable=YES
- `L3_Ave_Day` `datetime` nullable=YES
- `L3_KillCnt` `int` nullable=YES
- `L3_KillCnt_Lev` `int` nullable=YES
- `L3_KillCnt_Day` `datetime` nullable=YES
- `L3_Contri` `int` nullable=YES
- `L3_Contri_Lev` `int` nullable=YES
- `L3_Contri_Day` `datetime` nullable=YES
- `L3_TotPoint` `int` nullable=YES
- `L3_Cnt` `int` nullable=YES
- `L3_TotCnt` `int` nullable=YES
- `L4_Point` `int` nullable=YES
- `L4_Point_Lev` `int` nullable=YES
- `L4_Point_Day` `datetime` nullable=YES
- `L4_Ave` `int` nullable=YES
- `L4_Ave_Lev` `int` nullable=YES
- `L4_Ave_Day` `datetime` nullable=YES
- `L4_KillCnt` `int` nullable=YES
- `L4_KillCnt_Lev` `int` nullable=YES
- `L4_KillCnt_Day` `datetime` nullable=YES
- `L4_Contri` `int` nullable=YES
- `L4_Contri_Lev` `int` nullable=YES
- `L4_Contri_Day` `datetime` nullable=YES
- `L4_TotPoint` `int` nullable=YES
- `L4_Cnt` `int` nullable=YES
- `L4_TotCnt` `int` nullable=YES
- `L5_Point` `int` nullable=YES
- `L5_Point_Lev` `int` nullable=YES
- `L5_Point_Day` `datetime` nullable=YES
- `L5_Ave` `int` nullable=YES
- `L5_Ave_Lev` `int` nullable=YES
- `L5_Ave_Day` `datetime` nullable=YES
- `L5_KillCnt` `int` nullable=YES
- `L5_KillCnt_Lev` `int` nullable=YES
- `L5_KillCnt_Day` `datetime` nullable=YES
- `L5_Contri` `int` nullable=YES
- `L5_Contri_Lev` `int` nullable=YES
- `L5_Contri_Day` `datetime` nullable=YES
- `L5_TotPoint` `int` nullable=YES
- `L5_Cnt` `int` nullable=YES
- `L5_TotCnt` `int` nullable=YES
- `L6_Point` `int` nullable=YES
- `L6_Point_Lev` `int` nullable=YES
- `L6_Point_Day` `datetime` nullable=YES
- `L6_Ave` `int` nullable=YES
- `L6_Ave_Lev` `int` nullable=YES
- `L6_Ave_Day` `datetime` nullable=YES
- `L6_KillCnt` `int` nullable=YES
- `L6_KillCnt_Lev` `int` nullable=YES
- `L6_KillCnt_Day` `datetime` nullable=YES
- `L6_Contri` `int` nullable=YES
- `L6_Contri_Lev` `int` nullable=YES
- `L6_Contri_Day` `datetime` nullable=YES
- `L6_TotPoint` `int` nullable=YES
- `L6_Cnt` `int` nullable=YES
- `L6_TotCnt` `int` nullable=YES
- `L7_Point` `int` nullable=YES
- `L7_Point_Lev` `int` nullable=YES
- `L7_Point_Day` `datetime` nullable=YES
- `L7_Ave` `int` nullable=YES
- `L7_Ave_Lev` `int` nullable=YES
- `L7_Ave_Day` `datetime` nullable=YES
- `L7_KillCnt` `int` nullable=YES
- `L7_KillCnt_Lev` `int` nullable=YES
- `L7_KillCnt_Day` `datetime` nullable=YES
- `L7_Contri` `int` nullable=YES
- `L7_Contri_Lev` `int` nullable=YES
- `L7_Contri_Day` `datetime` nullable=YES
- `L7_TotPoint` `int` nullable=YES
- `L7_Cnt` `int` nullable=YES
- `L7_TotCnt` `int` nullable=YES
- `L8_Point` `int` nullable=YES
- `L8_Point_Lev` `int` nullable=YES
- `L8_Point_Day` `datetime` nullable=YES
- `L8_Ave` `int` nullable=YES
- `L8_Ave_Lev` `int` nullable=YES
- `L8_Ave_Day` `datetime` nullable=YES
- `L8_KillCnt` `int` nullable=YES
- `L8_KillCnt_Lev` `int` nullable=YES
- `L8_KillCnt_Day` `datetime` nullable=YES
- `L8_Contri` `int` nullable=YES
- `L8_Contri_Lev` `int` nullable=YES
- `L8_Contri_Day` `datetime` nullable=YES
- `L8_TotPoint` `int` nullable=YES
- `L8_Cnt` `int` nullable=YES
- `L8_TotCnt` `int` nullable=YES
- `L9_Point` `int` nullable=YES
- `L9_Point_Lev` `int` nullable=YES
- `L9_Point_Day` `datetime` nullable=YES
- `L9_Ave` `int` nullable=YES
- `L9_Ave_Lev` `int` nullable=YES
- `L9_Ave_Day` `datetime` nullable=YES
- `L9_KillCnt` `int` nullable=YES
- `L9_KillCnt_Lev` `int` nullable=YES
- `L9_KillCnt_Day` `datetime` nullable=YES
- `L9_Contri` `int` nullable=YES
- `L9_Contri_Lev` `int` nullable=YES
- `L9_Contri_Day` `datetime` nullable=YES
- `L9_TotPoint` `int` nullable=YES
- `L9_Cnt` `int` nullable=YES
- `L9_TotCnt` `int` nullable=YES

### SodDb.dbo.SOD0407
- `SODNo` `int` nullable=NO
- `UserID` `varchar` nullable=YES
- `CharName` `varchar` nullable=YES
- `CharType` `int` nullable=YES
- `ServerName` `varchar` nullable=YES
- `Point` `int` nullable=YES
- `Point_Lev` `int` nullable=YES
- `Point_Day` `datetime` nullable=YES
- `Ave` `int` nullable=YES
- `Ave_Lev` `int` nullable=YES
- `Ave_Day` `datetime` nullable=YES
- `KillCnt` `int` nullable=YES
- `KillCnt_Lev` `int` nullable=YES
- `KillCnt_Day` `datetime` nullable=YES
- `Contri` `int` nullable=YES
- `Contri_Lev` `int` nullable=YES
- `Contri_Day` `datetime` nullable=YES
- `TotPoint` `int` nullable=YES
- `Cnt` `int` nullable=YES
- `TotCnt` `int` nullable=YES
- `L1_Point` `int` nullable=YES
- `L1_Point_Lev` `int` nullable=YES
- `L1_Point_Day` `datetime` nullable=YES
- `L1_Ave` `int` nullable=YES
- `L1_Ave_Lev` `int` nullable=YES
- `L1_Ave_Day` `datetime` nullable=YES
- `L1_KillCnt` `int` nullable=YES
- `L1_KillCnt_Lev` `int` nullable=YES
- `L1_KillCnt_Day` `datetime` nullable=YES
- `L1_Contri` `int` nullable=YES
- `L1_Contri_Lev` `int` nullable=YES
- `L1_Contri_Day` `datetime` nullable=YES
- `L1_TotPoint` `int` nullable=YES
- `L1_Cnt` `int` nullable=YES
- `L1_TotCnt` `int` nullable=YES
- `L2_Point` `int` nullable=YES
- `L2_Point_Lev` `int` nullable=YES
- `L2_Point_Day` `datetime` nullable=YES
- `L2_Ave` `int` nullable=YES
- `L2_Ave_Lev` `int` nullable=YES
- `L2_Ave_Day` `datetime` nullable=YES
- `L2_KillCnt` `int` nullable=YES
- `L2_KillCnt_Lev` `int` nullable=YES
- `L2_KillCnt_Day` `datetime` nullable=YES
- `L2_Contri` `int` nullable=YES
- `L2_Contri_Lev` `int` nullable=YES
- `L2_Contri_Day` `datetime` nullable=YES
- `L2_TotPoint` `int` nullable=YES
- `L2_Cnt` `int` nullable=YES
- `L2_TotCnt` `int` nullable=YES
- `L3_Point` `int` nullable=YES
- `L3_Point_Lev` `int` nullable=YES
- `L3_Point_Day` `datetime` nullable=YES
- `L3_Ave` `int` nullable=YES
- `L3_Ave_Lev` `int` nullable=YES
- `L3_Ave_Day` `datetime` nullable=YES
- `L3_KillCnt` `int` nullable=YES
- `L3_KillCnt_Lev` `int` nullable=YES
- `L3_KillCnt_Day` `datetime` nullable=YES
- `L3_Contri` `int` nullable=YES
- `L3_Contri_Lev` `int` nullable=YES
- `L3_Contri_Day` `datetime` nullable=YES
- `L3_TotPoint` `int` nullable=YES
- `L3_Cnt` `int` nullable=YES
- `L3_TotCnt` `int` nullable=YES
- `L4_Point` `int` nullable=YES
- `L4_Point_Lev` `int` nullable=YES
- `L4_Point_Day` `datetime` nullable=YES
- `L4_Ave` `int` nullable=YES
- `L4_Ave_Lev` `int` nullable=YES
- `L4_Ave_Day` `datetime` nullable=YES
- `L4_KillCnt` `int` nullable=YES
- `L4_KillCnt_Lev` `int` nullable=YES
- `L4_KillCnt_Day` `datetime` nullable=YES
- `L4_Contri` `int` nullable=YES
- `L4_Contri_Lev` `int` nullable=YES
- `L4_Contri_Day` `datetime` nullable=YES
- `L4_TotPoint` `int` nullable=YES
- `L4_Cnt` `int` nullable=YES
- `L4_TotCnt` `int` nullable=YES
- `L5_Point` `int` nullable=YES
- `L5_Point_Lev` `int` nullable=YES
- `L5_Point_Day` `datetime` nullable=YES
- `L5_Ave` `int` nullable=YES
- `L5_Ave_Lev` `int` nullable=YES
- `L5_Ave_Day` `datetime` nullable=YES
- `L5_KillCnt` `int` nullable=YES
- `L5_KillCnt_Lev` `int` nullable=YES
- `L5_KillCnt_Day` `datetime` nullable=YES
- `L5_Contri` `int` nullable=YES
- `L5_Contri_Lev` `int` nullable=YES
- `L5_Contri_Day` `datetime` nullable=YES
- `L5_TotPoint` `int` nullable=YES
- `L5_Cnt` `int` nullable=YES
- `L5_TotCnt` `int` nullable=YES
- `L6_Point` `int` nullable=YES
- `L6_Point_Lev` `int` nullable=YES
- `L6_Point_Day` `datetime` nullable=YES
- `L6_Ave` `int` nullable=YES
- `L6_Ave_Lev` `int` nullable=YES
- `L6_Ave_Day` `datetime` nullable=YES
- `L6_KillCnt` `int` nullable=YES
- `L6_KillCnt_Lev` `int` nullable=YES
- `L6_KillCnt_Day` `datetime` nullable=YES
- `L6_Contri` `int` nullable=YES
- `L6_Contri_Lev` `int` nullable=YES
- `L6_Contri_Day` `datetime` nullable=YES
- `L6_TotPoint` `int` nullable=YES
- `L6_Cnt` `int` nullable=YES
- `L6_TotCnt` `int` nullable=YES
- `L7_Point` `int` nullable=YES
- `L7_Point_Lev` `int` nullable=YES
- `L7_Point_Day` `datetime` nullable=YES
- `L7_Ave` `int` nullable=YES
- `L7_Ave_Lev` `int` nullable=YES
- `L7_Ave_Day` `datetime` nullable=YES
- `L7_KillCnt` `int` nullable=YES
- `L7_KillCnt_Lev` `int` nullable=YES
- `L7_KillCnt_Day` `datetime` nullable=YES
- `L7_Contri` `int` nullable=YES
- `L7_Contri_Lev` `int` nullable=YES
- `L7_Contri_Day` `datetime` nullable=YES
- `L7_TotPoint` `int` nullable=YES
- `L7_Cnt` `int` nullable=YES
- `L7_TotCnt` `int` nullable=YES
- `L8_Point` `int` nullable=YES
- `L8_Point_Lev` `int` nullable=YES
- `L8_Point_Day` `datetime` nullable=YES
- `L8_Ave` `int` nullable=YES
- `L8_Ave_Lev` `int` nullable=YES
- `L8_Ave_Day` `datetime` nullable=YES
- `L8_KillCnt` `int` nullable=YES
- `L8_KillCnt_Lev` `int` nullable=YES
- `L8_KillCnt_Day` `datetime` nullable=YES
- `L8_Contri` `int` nullable=YES
- `L8_Contri_Lev` `int` nullable=YES
- `L8_Contri_Day` `datetime` nullable=YES
- `L8_TotPoint` `int` nullable=YES
- `L8_Cnt` `int` nullable=YES
- `L8_TotCnt` `int` nullable=YES
- `L9_Point` `int` nullable=YES
- `L9_Point_Lev` `int` nullable=YES
- `L9_Point_Day` `datetime` nullable=YES
- `L9_Ave` `int` nullable=YES
- `L9_Ave_Lev` `int` nullable=YES
- `L9_Ave_Day` `datetime` nullable=YES
- `L9_KillCnt` `int` nullable=YES
- `L9_KillCnt_Lev` `int` nullable=YES
- `L9_KillCnt_Day` `datetime` nullable=YES
- `L9_Contri` `int` nullable=YES
- `L9_Contri_Lev` `int` nullable=YES
- `L9_Contri_Day` `datetime` nullable=YES
- `L9_TotPoint` `int` nullable=YES
- `L9_Cnt` `int` nullable=YES
- `L9_TotCnt` `int` nullable=YES

### SodDb.dbo.SOD0408
- `SODNo` `int` nullable=NO
- `UserID` `varchar` nullable=YES
- `CharName` `varchar` nullable=YES
- `CharType` `int` nullable=YES
- `ServerName` `varchar` nullable=YES
- `Point` `int` nullable=YES
- `Point_Lev` `int` nullable=YES
- `Point_Day` `datetime` nullable=YES
- `Ave` `int` nullable=YES
- `Ave_Lev` `int` nullable=YES
- `Ave_Day` `datetime` nullable=YES
- `KillCnt` `int` nullable=YES
- `KillCnt_Lev` `int` nullable=YES
- `KillCnt_Day` `datetime` nullable=YES
- `Contri` `int` nullable=YES
- `Contri_Lev` `int` nullable=YES
- `Contri_Day` `datetime` nullable=YES
- `TotPoint` `int` nullable=YES
- `Cnt` `int` nullable=YES
- `TotCnt` `int` nullable=YES
- `L1_Point` `int` nullable=YES
- `L1_Point_Lev` `int` nullable=YES
- `L1_Point_Day` `datetime` nullable=YES
- `L1_Ave` `int` nullable=YES
- `L1_Ave_Lev` `int` nullable=YES
- `L1_Ave_Day` `datetime` nullable=YES
- `L1_KillCnt` `int` nullable=YES
- `L1_KillCnt_Lev` `int` nullable=YES
- `L1_KillCnt_Day` `datetime` nullable=YES
- `L1_Contri` `int` nullable=YES
- `L1_Contri_Lev` `int` nullable=YES
- `L1_Contri_Day` `datetime` nullable=YES
- `L1_TotPoint` `int` nullable=YES
- `L1_Cnt` `int` nullable=YES
- `L1_TotCnt` `int` nullable=YES
- `L2_Point` `int` nullable=YES
- `L2_Point_Lev` `int` nullable=YES
- `L2_Point_Day` `datetime` nullable=YES
- `L2_Ave` `int` nullable=YES
- `L2_Ave_Lev` `int` nullable=YES
- `L2_Ave_Day` `datetime` nullable=YES
- `L2_KillCnt` `int` nullable=YES
- `L2_KillCnt_Lev` `int` nullable=YES
- `L2_KillCnt_Day` `datetime` nullable=YES
- `L2_Contri` `int` nullable=YES
- `L2_Contri_Lev` `int` nullable=YES
- `L2_Contri_Day` `datetime` nullable=YES
- `L2_TotPoint` `int` nullable=YES
- `L2_Cnt` `int` nullable=YES
- `L2_TotCnt` `int` nullable=YES
- `L3_Point` `int` nullable=YES
- `L3_Point_Lev` `int` nullable=YES
- `L3_Point_Day` `datetime` nullable=YES
- `L3_Ave` `int` nullable=YES
- `L3_Ave_Lev` `int` nullable=YES
- `L3_Ave_Day` `datetime` nullable=YES
- `L3_KillCnt` `int` nullable=YES
- `L3_KillCnt_Lev` `int` nullable=YES
- `L3_KillCnt_Day` `datetime` nullable=YES
- `L3_Contri` `int` nullable=YES
- `L3_Contri_Lev` `int` nullable=YES
- `L3_Contri_Day` `datetime` nullable=YES
- `L3_TotPoint` `int` nullable=YES
- `L3_Cnt` `int` nullable=YES
- `L3_TotCnt` `int` nullable=YES
- `L4_Point` `int` nullable=YES
- `L4_Point_Lev` `int` nullable=YES
- `L4_Point_Day` `datetime` nullable=YES
- `L4_Ave` `int` nullable=YES
- `L4_Ave_Lev` `int` nullable=YES
- `L4_Ave_Day` `datetime` nullable=YES
- `L4_KillCnt` `int` nullable=YES
- `L4_KillCnt_Lev` `int` nullable=YES
- `L4_KillCnt_Day` `datetime` nullable=YES
- `L4_Contri` `int` nullable=YES
- `L4_Contri_Lev` `int` nullable=YES
- `L4_Contri_Day` `datetime` nullable=YES
- `L4_TotPoint` `int` nullable=YES
- `L4_Cnt` `int` nullable=YES
- `L4_TotCnt` `int` nullable=YES
- `L5_Point` `int` nullable=YES
- `L5_Point_Lev` `int` nullable=YES
- `L5_Point_Day` `datetime` nullable=YES
- `L5_Ave` `int` nullable=YES
- `L5_Ave_Lev` `int` nullable=YES
- `L5_Ave_Day` `datetime` nullable=YES
- `L5_KillCnt` `int` nullable=YES
- `L5_KillCnt_Lev` `int` nullable=YES
- `L5_KillCnt_Day` `datetime` nullable=YES
- `L5_Contri` `int` nullable=YES
- `L5_Contri_Lev` `int` nullable=YES
- `L5_Contri_Day` `datetime` nullable=YES
- `L5_TotPoint` `int` nullable=YES
- `L5_Cnt` `int` nullable=YES
- `L5_TotCnt` `int` nullable=YES
- `L6_Point` `int` nullable=YES
- `L6_Point_Lev` `int` nullable=YES
- `L6_Point_Day` `datetime` nullable=YES
- `L6_Ave` `int` nullable=YES
- `L6_Ave_Lev` `int` nullable=YES
- `L6_Ave_Day` `datetime` nullable=YES
- `L6_KillCnt` `int` nullable=YES
- `L6_KillCnt_Lev` `int` nullable=YES
- `L6_KillCnt_Day` `datetime` nullable=YES
- `L6_Contri` `int` nullable=YES
- `L6_Contri_Lev` `int` nullable=YES
- `L6_Contri_Day` `datetime` nullable=YES
- `L6_TotPoint` `int` nullable=YES
- `L6_Cnt` `int` nullable=YES
- `L6_TotCnt` `int` nullable=YES
- `L7_Point` `int` nullable=YES
- `L7_Point_Lev` `int` nullable=YES
- `L7_Point_Day` `datetime` nullable=YES
- `L7_Ave` `int` nullable=YES
- `L7_Ave_Lev` `int` nullable=YES
- `L7_Ave_Day` `datetime` nullable=YES
- `L7_KillCnt` `int` nullable=YES
- `L7_KillCnt_Lev` `int` nullable=YES
- `L7_KillCnt_Day` `datetime` nullable=YES
- `L7_Contri` `int` nullable=YES
- `L7_Contri_Lev` `int` nullable=YES
- `L7_Contri_Day` `datetime` nullable=YES
- `L7_TotPoint` `int` nullable=YES
- `L7_Cnt` `int` nullable=YES
- `L7_TotCnt` `int` nullable=YES
- `L8_Point` `int` nullable=YES
- `L8_Point_Lev` `int` nullable=YES
- `L8_Point_Day` `datetime` nullable=YES
- `L8_Ave` `int` nullable=YES
- `L8_Ave_Lev` `int` nullable=YES
- `L8_Ave_Day` `datetime` nullable=YES
- `L8_KillCnt` `int` nullable=YES
- `L8_KillCnt_Lev` `int` nullable=YES
- `L8_KillCnt_Day` `datetime` nullable=YES
- `L8_Contri` `int` nullable=YES
- `L8_Contri_Lev` `int` nullable=YES
- `L8_Contri_Day` `datetime` nullable=YES
- `L8_TotPoint` `int` nullable=YES
- `L8_Cnt` `int` nullable=YES
- `L8_TotCnt` `int` nullable=YES
- `L9_Point` `int` nullable=YES
- `L9_Point_Lev` `int` nullable=YES
- `L9_Point_Day` `datetime` nullable=YES
- `L9_Ave` `int` nullable=YES
- `L9_Ave_Lev` `int` nullable=YES
- `L9_Ave_Day` `datetime` nullable=YES
- `L9_KillCnt` `int` nullable=YES
- `L9_KillCnt_Lev` `int` nullable=YES
- `L9_KillCnt_Day` `datetime` nullable=YES
- `L9_Contri` `int` nullable=YES
- `L9_Contri_Lev` `int` nullable=YES
- `L9_Contri_Day` `datetime` nullable=YES
- `L9_TotPoint` `int` nullable=YES
- `L9_Cnt` `int` nullable=YES
- `L9_TotCnt` `int` nullable=YES

### SodDb.dbo.SOD0409
- `SODNo` `int` nullable=NO
- `UserID` `varchar` nullable=YES
- `CharName` `varchar` nullable=YES
- `CharType` `int` nullable=YES
- `ServerName` `varchar` nullable=YES
- `Point` `int` nullable=YES
- `Point_Lev` `int` nullable=YES
- `Point_Day` `datetime` nullable=YES
- `Ave` `int` nullable=YES
- `Ave_Lev` `int` nullable=YES
- `Ave_Day` `datetime` nullable=YES
- `KillCnt` `int` nullable=YES
- `KillCnt_Lev` `int` nullable=YES
- `KillCnt_Day` `datetime` nullable=YES
- `Contri` `int` nullable=YES
- `Contri_Lev` `int` nullable=YES
- `Contri_Day` `datetime` nullable=YES
- `TotPoint` `int` nullable=YES
- `Cnt` `int` nullable=YES
- `TotCnt` `int` nullable=YES
- `L1_Point` `int` nullable=YES
- `L1_Point_Lev` `int` nullable=YES
- `L1_Point_Day` `datetime` nullable=YES
- `L1_Ave` `int` nullable=YES
- `L1_Ave_Lev` `int` nullable=YES
- `L1_Ave_Day` `datetime` nullable=YES
- `L1_KillCnt` `int` nullable=YES
- `L1_KillCnt_Lev` `int` nullable=YES
- `L1_KillCnt_Day` `datetime` nullable=YES
- `L1_Contri` `int` nullable=YES
- `L1_Contri_Lev` `int` nullable=YES
- `L1_Contri_Day` `datetime` nullable=YES
- `L1_TotPoint` `int` nullable=YES
- `L1_Cnt` `int` nullable=YES
- `L1_TotCnt` `int` nullable=YES
- `L2_Point` `int` nullable=YES
- `L2_Point_Lev` `int` nullable=YES
- `L2_Point_Day` `datetime` nullable=YES
- `L2_Ave` `int` nullable=YES
- `L2_Ave_Lev` `int` nullable=YES
- `L2_Ave_Day` `datetime` nullable=YES
- `L2_KillCnt` `int` nullable=YES
- `L2_KillCnt_Lev` `int` nullable=YES
- `L2_KillCnt_Day` `datetime` nullable=YES
- `L2_Contri` `int` nullable=YES
- `L2_Contri_Lev` `int` nullable=YES
- `L2_Contri_Day` `datetime` nullable=YES
- `L2_TotPoint` `int` nullable=YES
- `L2_Cnt` `int` nullable=YES
- `L2_TotCnt` `int` nullable=YES
- `L3_Point` `int` nullable=YES
- `L3_Point_Lev` `int` nullable=YES
- `L3_Point_Day` `datetime` nullable=YES
- `L3_Ave` `int` nullable=YES
- `L3_Ave_Lev` `int` nullable=YES
- `L3_Ave_Day` `datetime` nullable=YES
- `L3_KillCnt` `int` nullable=YES
- `L3_KillCnt_Lev` `int` nullable=YES
- `L3_KillCnt_Day` `datetime` nullable=YES
- `L3_Contri` `int` nullable=YES
- `L3_Contri_Lev` `int` nullable=YES
- `L3_Contri_Day` `datetime` nullable=YES
- `L3_TotPoint` `int` nullable=YES
- `L3_Cnt` `int` nullable=YES
- `L3_TotCnt` `int` nullable=YES
- `L4_Point` `int` nullable=YES
- `L4_Point_Lev` `int` nullable=YES
- `L4_Point_Day` `datetime` nullable=YES
- `L4_Ave` `int` nullable=YES
- `L4_Ave_Lev` `int` nullable=YES
- `L4_Ave_Day` `datetime` nullable=YES
- `L4_KillCnt` `int` nullable=YES
- `L4_KillCnt_Lev` `int` nullable=YES
- `L4_KillCnt_Day` `datetime` nullable=YES
- `L4_Contri` `int` nullable=YES
- `L4_Contri_Lev` `int` nullable=YES
- `L4_Contri_Day` `datetime` nullable=YES
- `L4_TotPoint` `int` nullable=YES
- `L4_Cnt` `int` nullable=YES
- `L4_TotCnt` `int` nullable=YES
- `L5_Point` `int` nullable=YES
- `L5_Point_Lev` `int` nullable=YES
- `L5_Point_Day` `datetime` nullable=YES
- `L5_Ave` `int` nullable=YES
- `L5_Ave_Lev` `int` nullable=YES
- `L5_Ave_Day` `datetime` nullable=YES
- `L5_KillCnt` `int` nullable=YES
- `L5_KillCnt_Lev` `int` nullable=YES
- `L5_KillCnt_Day` `datetime` nullable=YES
- `L5_Contri` `int` nullable=YES
- `L5_Contri_Lev` `int` nullable=YES
- `L5_Contri_Day` `datetime` nullable=YES
- `L5_TotPoint` `int` nullable=YES
- `L5_Cnt` `int` nullable=YES
- `L5_TotCnt` `int` nullable=YES
- `L6_Point` `int` nullable=YES
- `L6_Point_Lev` `int` nullable=YES
- `L6_Point_Day` `datetime` nullable=YES
- `L6_Ave` `int` nullable=YES
- `L6_Ave_Lev` `int` nullable=YES
- `L6_Ave_Day` `datetime` nullable=YES
- `L6_KillCnt` `int` nullable=YES
- `L6_KillCnt_Lev` `int` nullable=YES
- `L6_KillCnt_Day` `datetime` nullable=YES
- `L6_Contri` `int` nullable=YES
- `L6_Contri_Lev` `int` nullable=YES
- `L6_Contri_Day` `datetime` nullable=YES
- `L6_TotPoint` `int` nullable=YES
- `L6_Cnt` `int` nullable=YES
- `L6_TotCnt` `int` nullable=YES
- `L7_Point` `int` nullable=YES
- `L7_Point_Lev` `int` nullable=YES
- `L7_Point_Day` `datetime` nullable=YES
- `L7_Ave` `int` nullable=YES
- `L7_Ave_Lev` `int` nullable=YES
- `L7_Ave_Day` `datetime` nullable=YES
- `L7_KillCnt` `int` nullable=YES
- `L7_KillCnt_Lev` `int` nullable=YES
- `L7_KillCnt_Day` `datetime` nullable=YES
- `L7_Contri` `int` nullable=YES
- `L7_Contri_Lev` `int` nullable=YES
- `L7_Contri_Day` `datetime` nullable=YES
- `L7_TotPoint` `int` nullable=YES
- `L7_Cnt` `int` nullable=YES
- `L7_TotCnt` `int` nullable=YES
- `L8_Point` `int` nullable=YES
- `L8_Point_Lev` `int` nullable=YES
- `L8_Point_Day` `datetime` nullable=YES
- `L8_Ave` `int` nullable=YES
- `L8_Ave_Lev` `int` nullable=YES
- `L8_Ave_Day` `datetime` nullable=YES
- `L8_KillCnt` `int` nullable=YES
- `L8_KillCnt_Lev` `int` nullable=YES
- `L8_KillCnt_Day` `datetime` nullable=YES
- `L8_Contri` `int` nullable=YES
- `L8_Contri_Lev` `int` nullable=YES
- `L8_Contri_Day` `datetime` nullable=YES
- `L8_TotPoint` `int` nullable=YES
- `L8_Cnt` `int` nullable=YES
- `L8_TotCnt` `int` nullable=YES
- `L9_Point` `int` nullable=YES
- `L9_Point_Lev` `int` nullable=YES
- `L9_Point_Day` `datetime` nullable=YES
- `L9_Ave` `int` nullable=YES
- `L9_Ave_Lev` `int` nullable=YES
- `L9_Ave_Day` `datetime` nullable=YES
- `L9_KillCnt` `int` nullable=YES
- `L9_KillCnt_Lev` `int` nullable=YES
- `L9_KillCnt_Day` `datetime` nullable=YES
- `L9_Contri` `int` nullable=YES
- `L9_Contri_Lev` `int` nullable=YES
- `L9_Contri_Day` `datetime` nullable=YES
- `L9_TotPoint` `int` nullable=YES
- `L9_Cnt` `int` nullable=YES
- `L9_TotCnt` `int` nullable=YES

### SodDb.dbo.SOD0410
- `SODNo` `int` nullable=NO
- `UserID` `varchar` nullable=YES
- `CharName` `varchar` nullable=YES
- `CharType` `int` nullable=YES
- `ServerName` `varchar` nullable=YES
- `Point` `int` nullable=YES
- `Point_Lev` `int` nullable=YES
- `Point_Day` `datetime` nullable=YES
- `Ave` `int` nullable=YES
- `Ave_Lev` `int` nullable=YES
- `Ave_Day` `datetime` nullable=YES
- `KillCnt` `int` nullable=YES
- `KillCnt_Lev` `int` nullable=YES
- `KillCnt_Day` `datetime` nullable=YES
- `Contri` `int` nullable=YES
- `Contri_Lev` `int` nullable=YES
- `Contri_Day` `datetime` nullable=YES
- `TotPoint` `int` nullable=YES
- `Cnt` `int` nullable=YES
- `TotCnt` `int` nullable=YES
- `L1_Point` `int` nullable=YES
- `L1_Point_Lev` `int` nullable=YES
- `L1_Point_Day` `datetime` nullable=YES
- `L1_Ave` `int` nullable=YES
- `L1_Ave_Lev` `int` nullable=YES
- `L1_Ave_Day` `datetime` nullable=YES
- `L1_KillCnt` `int` nullable=YES
- `L1_KillCnt_Lev` `int` nullable=YES
- `L1_KillCnt_Day` `datetime` nullable=YES
- `L1_Contri` `int` nullable=YES
- `L1_Contri_Lev` `int` nullable=YES
- `L1_Contri_Day` `datetime` nullable=YES
- `L1_TotPoint` `int` nullable=YES
- `L1_Cnt` `int` nullable=YES
- `L1_TotCnt` `int` nullable=YES
- `L2_Point` `int` nullable=YES
- `L2_Point_Lev` `int` nullable=YES
- `L2_Point_Day` `datetime` nullable=YES
- `L2_Ave` `int` nullable=YES
- `L2_Ave_Lev` `int` nullable=YES
- `L2_Ave_Day` `datetime` nullable=YES
- `L2_KillCnt` `int` nullable=YES
- `L2_KillCnt_Lev` `int` nullable=YES
- `L2_KillCnt_Day` `datetime` nullable=YES
- `L2_Contri` `int` nullable=YES
- `L2_Contri_Lev` `int` nullable=YES
- `L2_Contri_Day` `datetime` nullable=YES
- `L2_TotPoint` `int` nullable=YES
- `L2_Cnt` `int` nullable=YES
- `L2_TotCnt` `int` nullable=YES
- `L3_Point` `int` nullable=YES
- `L3_Point_Lev` `int` nullable=YES
- `L3_Point_Day` `datetime` nullable=YES
- `L3_Ave` `int` nullable=YES
- `L3_Ave_Lev` `int` nullable=YES
- `L3_Ave_Day` `datetime` nullable=YES
- `L3_KillCnt` `int` nullable=YES
- `L3_KillCnt_Lev` `int` nullable=YES
- `L3_KillCnt_Day` `datetime` nullable=YES
- `L3_Contri` `int` nullable=YES
- `L3_Contri_Lev` `int` nullable=YES
- `L3_Contri_Day` `datetime` nullable=YES
- `L3_TotPoint` `int` nullable=YES
- `L3_Cnt` `int` nullable=YES
- `L3_TotCnt` `int` nullable=YES
- `L4_Point` `int` nullable=YES
- `L4_Point_Lev` `int` nullable=YES
- `L4_Point_Day` `datetime` nullable=YES
- `L4_Ave` `int` nullable=YES
- `L4_Ave_Lev` `int` nullable=YES
- `L4_Ave_Day` `datetime` nullable=YES
- `L4_KillCnt` `int` nullable=YES
- `L4_KillCnt_Lev` `int` nullable=YES
- `L4_KillCnt_Day` `datetime` nullable=YES
- `L4_Contri` `int` nullable=YES
- `L4_Contri_Lev` `int` nullable=YES
- `L4_Contri_Day` `datetime` nullable=YES
- `L4_TotPoint` `int` nullable=YES
- `L4_Cnt` `int` nullable=YES
- `L4_TotCnt` `int` nullable=YES
- `L5_Point` `int` nullable=YES
- `L5_Point_Lev` `int` nullable=YES
- `L5_Point_Day` `datetime` nullable=YES
- `L5_Ave` `int` nullable=YES
- `L5_Ave_Lev` `int` nullable=YES
- `L5_Ave_Day` `datetime` nullable=YES
- `L5_KillCnt` `int` nullable=YES
- `L5_KillCnt_Lev` `int` nullable=YES
- `L5_KillCnt_Day` `datetime` nullable=YES
- `L5_Contri` `int` nullable=YES
- `L5_Contri_Lev` `int` nullable=YES
- `L5_Contri_Day` `datetime` nullable=YES
- `L5_TotPoint` `int` nullable=YES
- `L5_Cnt` `int` nullable=YES
- `L5_TotCnt` `int` nullable=YES
- `L6_Point` `int` nullable=YES
- `L6_Point_Lev` `int` nullable=YES
- `L6_Point_Day` `datetime` nullable=YES
- `L6_Ave` `int` nullable=YES
- `L6_Ave_Lev` `int` nullable=YES
- `L6_Ave_Day` `datetime` nullable=YES
- `L6_KillCnt` `int` nullable=YES
- `L6_KillCnt_Lev` `int` nullable=YES
- `L6_KillCnt_Day` `datetime` nullable=YES
- `L6_Contri` `int` nullable=YES
- `L6_Contri_Lev` `int` nullable=YES
- `L6_Contri_Day` `datetime` nullable=YES
- `L6_TotPoint` `int` nullable=YES
- `L6_Cnt` `int` nullable=YES
- `L6_TotCnt` `int` nullable=YES
- `L7_Point` `int` nullable=YES
- `L7_Point_Lev` `int` nullable=YES
- `L7_Point_Day` `datetime` nullable=YES
- `L7_Ave` `int` nullable=YES
- `L7_Ave_Lev` `int` nullable=YES
- `L7_Ave_Day` `datetime` nullable=YES
- `L7_KillCnt` `int` nullable=YES
- `L7_KillCnt_Lev` `int` nullable=YES
- `L7_KillCnt_Day` `datetime` nullable=YES
- `L7_Contri` `int` nullable=YES
- `L7_Contri_Lev` `int` nullable=YES
- `L7_Contri_Day` `datetime` nullable=YES
- `L7_TotPoint` `int` nullable=YES
- `L7_Cnt` `int` nullable=YES
- `L7_TotCnt` `int` nullable=YES
- `L8_Point` `int` nullable=YES
- `L8_Point_Lev` `int` nullable=YES
- `L8_Point_Day` `datetime` nullable=YES
- `L8_Ave` `int` nullable=YES
- `L8_Ave_Lev` `int` nullable=YES
- `L8_Ave_Day` `datetime` nullable=YES
- `L8_KillCnt` `int` nullable=YES
- `L8_KillCnt_Lev` `int` nullable=YES
- `L8_KillCnt_Day` `datetime` nullable=YES
- `L8_Contri` `int` nullable=YES
- `L8_Contri_Lev` `int` nullable=YES
- `L8_Contri_Day` `datetime` nullable=YES
- `L8_TotPoint` `int` nullable=YES
- `L8_Cnt` `int` nullable=YES
- `L8_TotCnt` `int` nullable=YES
- `L9_Point` `int` nullable=YES
- `L9_Point_Lev` `int` nullable=YES
- `L9_Point_Day` `datetime` nullable=YES
- `L9_Ave` `int` nullable=YES
- `L9_Ave_Lev` `int` nullable=YES
- `L9_Ave_Day` `datetime` nullable=YES
- `L9_KillCnt` `int` nullable=YES
- `L9_KillCnt_Lev` `int` nullable=YES
- `L9_KillCnt_Day` `datetime` nullable=YES
- `L9_Contri` `int` nullable=YES
- `L9_Contri_Lev` `int` nullable=YES
- `L9_Contri_Day` `datetime` nullable=YES
- `L9_TotPoint` `int` nullable=YES
- `L9_Cnt` `int` nullable=YES
- `L9_TotCnt` `int` nullable=YES

### SodDb.dbo.SOD0412
- `SODNo` `int` nullable=NO
- `UserID` `varchar` nullable=YES
- `CharName` `varchar` nullable=YES
- `CharType` `int` nullable=YES
- `ServerName` `varchar` nullable=YES
- `Point` `int` nullable=YES
- `Point_Lev` `int` nullable=YES
- `Point_Day` `datetime` nullable=YES
- `Ave` `int` nullable=YES
- `Ave_Lev` `int` nullable=YES
- `Ave_Day` `datetime` nullable=YES
- `KillCnt` `int` nullable=YES
- `KillCnt_Lev` `int` nullable=YES
- `KillCnt_Day` `datetime` nullable=YES
- `Contri` `int` nullable=YES
- `Contri_Lev` `int` nullable=YES
- `Contri_Day` `datetime` nullable=YES
- `TotPoint` `int` nullable=YES
- `Cnt` `int` nullable=YES
- `TotCnt` `int` nullable=YES
- `L1_Point` `int` nullable=YES
- `L1_Point_Lev` `int` nullable=YES
- `L1_Point_Day` `datetime` nullable=YES
- `L1_Ave` `int` nullable=YES
- `L1_Ave_Lev` `int` nullable=YES
- `L1_Ave_Day` `datetime` nullable=YES
- `L1_KillCnt` `int` nullable=YES
- `L1_KillCnt_Lev` `int` nullable=YES
- `L1_KillCnt_Day` `datetime` nullable=YES
- `L1_Contri` `int` nullable=YES
- `L1_Contri_Lev` `int` nullable=YES
- `L1_Contri_Day` `datetime` nullable=YES
- `L1_TotPoint` `int` nullable=YES
- `L1_Cnt` `int` nullable=YES
- `L1_TotCnt` `int` nullable=YES
- `L2_Point` `int` nullable=YES
- `L2_Point_Lev` `int` nullable=YES
- `L2_Point_Day` `datetime` nullable=YES
- `L2_Ave` `int` nullable=YES
- `L2_Ave_Lev` `int` nullable=YES
- `L2_Ave_Day` `datetime` nullable=YES
- `L2_KillCnt` `int` nullable=YES
- `L2_KillCnt_Lev` `int` nullable=YES
- `L2_KillCnt_Day` `datetime` nullable=YES
- `L2_Contri` `int` nullable=YES
- `L2_Contri_Lev` `int` nullable=YES
- `L2_Contri_Day` `datetime` nullable=YES
- `L2_TotPoint` `int` nullable=YES
- `L2_Cnt` `int` nullable=YES
- `L2_TotCnt` `int` nullable=YES
- `L3_Point` `int` nullable=YES
- `L3_Point_Lev` `int` nullable=YES
- `L3_Point_Day` `datetime` nullable=YES
- `L3_Ave` `int` nullable=YES
- `L3_Ave_Lev` `int` nullable=YES
- `L3_Ave_Day` `datetime` nullable=YES
- `L3_KillCnt` `int` nullable=YES
- `L3_KillCnt_Lev` `int` nullable=YES
- `L3_KillCnt_Day` `datetime` nullable=YES
- `L3_Contri` `int` nullable=YES
- `L3_Contri_Lev` `int` nullable=YES
- `L3_Contri_Day` `datetime` nullable=YES
- `L3_TotPoint` `int` nullable=YES
- `L3_Cnt` `int` nullable=YES
- `L3_TotCnt` `int` nullable=YES
- `L4_Point` `int` nullable=YES
- `L4_Point_Lev` `int` nullable=YES
- `L4_Point_Day` `datetime` nullable=YES
- `L4_Ave` `int` nullable=YES
- `L4_Ave_Lev` `int` nullable=YES
- `L4_Ave_Day` `datetime` nullable=YES
- `L4_KillCnt` `int` nullable=YES
- `L4_KillCnt_Lev` `int` nullable=YES
- `L4_KillCnt_Day` `datetime` nullable=YES
- `L4_Contri` `int` nullable=YES
- `L4_Contri_Lev` `int` nullable=YES
- `L4_Contri_Day` `datetime` nullable=YES
- `L4_TotPoint` `int` nullable=YES
- `L4_Cnt` `int` nullable=YES
- `L4_TotCnt` `int` nullable=YES
- `L5_Point` `int` nullable=YES
- `L5_Point_Lev` `int` nullable=YES
- `L5_Point_Day` `datetime` nullable=YES
- `L5_Ave` `int` nullable=YES
- `L5_Ave_Lev` `int` nullable=YES
- `L5_Ave_Day` `datetime` nullable=YES
- `L5_KillCnt` `int` nullable=YES
- `L5_KillCnt_Lev` `int` nullable=YES
- `L5_KillCnt_Day` `datetime` nullable=YES
- `L5_Contri` `int` nullable=YES
- `L5_Contri_Lev` `int` nullable=YES
- `L5_Contri_Day` `datetime` nullable=YES
- `L5_TotPoint` `int` nullable=YES
- `L5_Cnt` `int` nullable=YES
- `L5_TotCnt` `int` nullable=YES
- `L6_Point` `int` nullable=YES
- `L6_Point_Lev` `int` nullable=YES
- `L6_Point_Day` `datetime` nullable=YES
- `L6_Ave` `int` nullable=YES
- `L6_Ave_Lev` `int` nullable=YES
- `L6_Ave_Day` `datetime` nullable=YES
- `L6_KillCnt` `int` nullable=YES
- `L6_KillCnt_Lev` `int` nullable=YES
- `L6_KillCnt_Day` `datetime` nullable=YES
- `L6_Contri` `int` nullable=YES
- `L6_Contri_Lev` `int` nullable=YES
- `L6_Contri_Day` `datetime` nullable=YES
- `L6_TotPoint` `int` nullable=YES
- `L6_Cnt` `int` nullable=YES
- `L6_TotCnt` `int` nullable=YES
- `L7_Point` `int` nullable=YES
- `L7_Point_Lev` `int` nullable=YES
- `L7_Point_Day` `datetime` nullable=YES
- `L7_Ave` `int` nullable=YES
- `L7_Ave_Lev` `int` nullable=YES
- `L7_Ave_Day` `datetime` nullable=YES
- `L7_KillCnt` `int` nullable=YES
- `L7_KillCnt_Lev` `int` nullable=YES
- `L7_KillCnt_Day` `datetime` nullable=YES
- `L7_Contri` `int` nullable=YES
- `L7_Contri_Lev` `int` nullable=YES
- `L7_Contri_Day` `datetime` nullable=YES
- `L7_TotPoint` `int` nullable=YES
- `L7_Cnt` `int` nullable=YES
- `L7_TotCnt` `int` nullable=YES
- `L8_Point` `int` nullable=YES
- `L8_Point_Lev` `int` nullable=YES
- `L8_Point_Day` `datetime` nullable=YES
- `L8_Ave` `int` nullable=YES
- `L8_Ave_Lev` `int` nullable=YES
- `L8_Ave_Day` `datetime` nullable=YES
- `L8_KillCnt` `int` nullable=YES
- `L8_KillCnt_Lev` `int` nullable=YES
- `L8_KillCnt_Day` `datetime` nullable=YES
- `L8_Contri` `int` nullable=YES
- `L8_Contri_Lev` `int` nullable=YES
- `L8_Contri_Day` `datetime` nullable=YES
- `L8_TotPoint` `int` nullable=YES
- `L8_Cnt` `int` nullable=YES
- `L8_TotCnt` `int` nullable=YES
- `L9_Point` `int` nullable=YES
- `L9_Point_Lev` `int` nullable=YES
- `L9_Point_Day` `datetime` nullable=YES
- `L9_Ave` `int` nullable=YES
- `L9_Ave_Lev` `int` nullable=YES
- `L9_Ave_Day` `datetime` nullable=YES
- `L9_KillCnt` `int` nullable=YES
- `L9_KillCnt_Lev` `int` nullable=YES
- `L9_KillCnt_Day` `datetime` nullable=YES
- `L9_Contri` `int` nullable=YES
- `L9_Contri_Lev` `int` nullable=YES
- `L9_Contri_Day` `datetime` nullable=YES
- `L9_TotPoint` `int` nullable=YES
- `L9_Cnt` `int` nullable=YES
- `L9_TotCnt` `int` nullable=YES

### SodDb.dbo.SODClan0401
- `IDX` `int` nullable=NO
- `CIDX` `int` nullable=NO
- `CServerTime` `bigint` nullable=NO
- `Server` `int` nullable=NO
- `ClanName` `varchar` nullable=NO
- `ServerName` `varchar` nullable=NO
- `CPoint` `int` nullable=NO
- `CRegistDay` `datetime` nullable=NO

### SodDb.dbo.SODClan0402
- `IDX` `int` nullable=NO
- `CIDX` `int` nullable=NO
- `CServerTime` `bigint` nullable=NO
- `Server` `int` nullable=NO
- `ClanName` `varchar` nullable=NO
- `ServerName` `varchar` nullable=NO
- `CPoint` `int` nullable=NO
- `CRegistDay` `datetime` nullable=NO

### SodDb.dbo.SODClan0403
- `IDX` `int` nullable=NO
- `CIDX` `int` nullable=NO
- `CServerTime` `bigint` nullable=NO
- `Server` `int` nullable=NO
- `ClanName` `varchar` nullable=NO
- `ServerName` `varchar` nullable=NO
- `CPoint` `int` nullable=NO
- `CRegistDay` `datetime` nullable=NO

### SodDb.dbo.SODClan0404
- `IDX` `int` nullable=NO
- `CIDX` `int` nullable=NO
- `CServerTime` `bigint` nullable=NO
- `Server` `int` nullable=NO
- `ClanName` `varchar` nullable=NO
- `ServerName` `varchar` nullable=NO
- `CPoint` `int` nullable=NO
- `CRegistDay` `datetime` nullable=NO

### SodDb.dbo.SODClan0405
- `IDX` `int` nullable=NO
- `CIDX` `int` nullable=NO
- `CServerTime` `bigint` nullable=NO
- `Server` `int` nullable=NO
- `ClanName` `varchar` nullable=NO
- `ServerName` `varchar` nullable=NO
- `CPoint` `int` nullable=NO
- `CRegistDay` `datetime` nullable=NO

### SodDb.dbo.SODClan0406
- `IDX` `int` nullable=NO
- `CIDX` `int` nullable=NO
- `CServerTime` `bigint` nullable=NO
- `Server` `int` nullable=NO
- `ClanName` `varchar` nullable=NO
- `ServerName` `varchar` nullable=NO
- `CPoint` `int` nullable=NO
- `CRegistDay` `datetime` nullable=NO

### SodDb.dbo.SODClan0407
- `IDX` `int` nullable=NO
- `CIDX` `int` nullable=NO
- `CServerTime` `bigint` nullable=NO
- `Server` `int` nullable=NO
- `ClanName` `varchar` nullable=NO
- `ServerName` `varchar` nullable=NO
- `CPoint` `int` nullable=NO
- `CRegistDay` `datetime` nullable=NO

### SodDb.dbo.SODClan0408
- `IDX` `int` nullable=NO
- `CIDX` `int` nullable=NO
- `CServerTime` `bigint` nullable=NO
- `Server` `int` nullable=NO
- `ClanName` `varchar` nullable=NO
- `ServerName` `varchar` nullable=NO
- `CPoint` `int` nullable=NO
- `CRegistDay` `datetime` nullable=NO

### SodDb.dbo.SODClan0409
- `IDX` `int` nullable=NO
- `CIDX` `int` nullable=NO
- `CServerTime` `bigint` nullable=NO
- `Server` `int` nullable=NO
- `ClanName` `varchar` nullable=NO
- `ServerName` `varchar` nullable=NO
- `CPoint` `int` nullable=NO
- `CRegistDay` `datetime` nullable=NO

### SodDb.dbo.SODClan0410
- `IDX` `int` nullable=NO
- `CIDX` `int` nullable=NO
- `CServerTime` `bigint` nullable=NO
- `Server` `int` nullable=NO
- `ClanName` `varchar` nullable=NO
- `ServerName` `varchar` nullable=NO
- `CPoint` `int` nullable=NO
- `CRegistDay` `datetime` nullable=NO

### SodDb.dbo.SODClan0411
- `IDX` `int` nullable=NO
- `CIDX` `int` nullable=NO
- `CServerTime` `bigint` nullable=NO
- `Server` `int` nullable=NO
- `ClanName` `varchar` nullable=NO
- `ServerName` `varchar` nullable=NO
- `CPoint` `int` nullable=NO
- `CRegistDay` `datetime` nullable=NO

### SodDb.dbo.SODClan0412
- `IDX` `int` nullable=NO
- `CIDX` `int` nullable=NO
- `CServerTime` `bigint` nullable=NO
- `Server` `int` nullable=NO
- `ClanName` `varchar` nullable=NO
- `ServerName` `varchar` nullable=NO
- `CPoint` `int` nullable=NO
- `CRegistDay` `datetime` nullable=NO

### SodDb.dbo.SODClanLog0401
- `IDX` `int` nullable=NO
- `LCIDX` `int` nullable=NO
- `LCServerTime` `bigint` nullable=NO
- `LServer` `int` nullable=NO
- `LCPoint` `int` nullable=NO
- `LCRegistDay` `datetime` nullable=NO

### SodDb.dbo.SODClanLog0402
- `IDX` `int` nullable=NO
- `LCIDX` `int` nullable=NO
- `LCServerTime` `bigint` nullable=NO
- `LServer` `int` nullable=NO
- `LCPoint` `int` nullable=NO
- `LCRegistDay` `datetime` nullable=NO

### SodDb.dbo.SODClanLog0403
- `IDX` `int` nullable=NO
- `LCIDX` `int` nullable=NO
- `LCServerTime` `bigint` nullable=NO
- `LServer` `int` nullable=NO
- `LCPoint` `int` nullable=NO
- `LCRegistDay` `datetime` nullable=NO

### SodDb.dbo.SODClanLog0404
- `IDX` `int` nullable=NO
- `LCIDX` `int` nullable=NO
- `LCServerTime` `bigint` nullable=NO
- `LServer` `int` nullable=NO
- `LCPoint` `int` nullable=NO
- `LCRegistDay` `datetime` nullable=NO

### SodDb.dbo.SODClanLog0405
- `IDX` `int` nullable=NO
- `LCIDX` `int` nullable=NO
- `LCServerTime` `bigint` nullable=NO
- `LServer` `int` nullable=NO
- `LCPoint` `int` nullable=NO
- `LCRegistDay` `datetime` nullable=NO

### SodDb.dbo.SODClanLog0406
- `IDX` `int` nullable=NO
- `LCIDX` `int` nullable=NO
- `LCServerTime` `bigint` nullable=NO
- `LServer` `int` nullable=NO
- `LCPoint` `int` nullable=NO
- `LCRegistDay` `datetime` nullable=NO

### SodDb.dbo.SODClanLog0407
- `IDX` `int` nullable=NO
- `LCIDX` `int` nullable=NO
- `LCServerTime` `bigint` nullable=NO
- `LServer` `int` nullable=NO
- `LCPoint` `int` nullable=NO
- `LCRegistDay` `datetime` nullable=NO

### SodDb.dbo.SODClanLog0408
- `IDX` `int` nullable=NO
- `LCIDX` `int` nullable=NO
- `LCServerTime` `bigint` nullable=NO
- `LServer` `int` nullable=NO
- `LCPoint` `int` nullable=NO
- `LCRegistDay` `datetime` nullable=NO

### SodDb.dbo.SODClanLog0409
- `IDX` `int` nullable=NO
- `LCIDX` `int` nullable=NO
- `LCServerTime` `bigint` nullable=NO
- `LServer` `int` nullable=NO
- `LCPoint` `int` nullable=NO
- `LCRegistDay` `datetime` nullable=NO

### SodDb.dbo.SODClanLog0410
- `IDX` `int` nullable=NO
- `LCIDX` `int` nullable=NO
- `LCServerTime` `bigint` nullable=NO
- `LServer` `int` nullable=NO
- `LCPoint` `int` nullable=NO
- `LCRegistDay` `datetime` nullable=NO

### SodDb.dbo.SODClanLog0411
- `IDX` `int` nullable=NO
- `LCIDX` `int` nullable=NO
- `LCServerTime` `bigint` nullable=NO
- `LServer` `int` nullable=NO
- `LCPoint` `int` nullable=NO
- `LCRegistDay` `datetime` nullable=NO

### SodDb.dbo.SODClanLog0412
- `IDX` `int` nullable=NO
- `LCIDX` `int` nullable=NO
- `LCServerTime` `bigint` nullable=NO
- `LServer` `int` nullable=NO
- `LCPoint` `int` nullable=NO
- `LCRegistDay` `datetime` nullable=NO

### SodDb.dbo.SODClanSum0401
- `IDX` `int` nullable=NO
- `CIDX` `int` nullable=NO
- `Server` `int` nullable=NO
- `ClanName` `varchar` nullable=NO
- `ServerName` `varchar` nullable=NO
- `CPointSum` `int` nullable=NO
- `CRegistDay` `datetime` nullable=NO

### SodDb.dbo.SODClanSum0402
- `IDX` `int` nullable=NO
- `CIDX` `int` nullable=NO
- `Server` `int` nullable=NO
- `ClanName` `varchar` nullable=NO
- `ServerName` `varchar` nullable=NO
- `CPointSum` `int` nullable=NO
- `CRegistDay` `datetime` nullable=NO

### SodDb.dbo.SODClanSum0403
- `IDX` `int` nullable=NO
- `CIDX` `int` nullable=NO
- `Server` `int` nullable=NO
- `ClanName` `varchar` nullable=NO
- `ServerName` `varchar` nullable=NO
- `CPointSum` `int` nullable=NO
- `CRegistDay` `datetime` nullable=NO

### SodDb.dbo.SODClanSum0404
- `IDX` `int` nullable=NO
- `CIDX` `int` nullable=NO
- `Server` `int` nullable=NO
- `ClanName` `varchar` nullable=NO
- `ServerName` `varchar` nullable=NO
- `CPointSum` `int` nullable=NO
- `CRegistDay` `datetime` nullable=NO

### SodDb.dbo.SODClanSum0405
- `IDX` `int` nullable=NO
- `CIDX` `int` nullable=NO
- `Server` `int` nullable=NO
- `ClanName` `varchar` nullable=NO
- `ServerName` `varchar` nullable=NO
- `CPointSum` `int` nullable=NO
- `CRegistDay` `datetime` nullable=NO

### SodDb.dbo.SODClanSum0406
- `IDX` `int` nullable=NO
- `CIDX` `int` nullable=NO
- `Server` `int` nullable=NO
- `ClanName` `varchar` nullable=NO
- `ServerName` `varchar` nullable=NO
- `CPointSum` `int` nullable=NO
- `CRegistDay` `datetime` nullable=NO

### SodDb.dbo.SODClanSum0407
- `IDX` `int` nullable=NO
- `CIDX` `int` nullable=NO
- `Server` `int` nullable=NO
- `ClanName` `varchar` nullable=NO
- `ServerName` `varchar` nullable=NO
- `CPointSum` `int` nullable=NO
- `CRegistDay` `datetime` nullable=NO

### SodDb.dbo.SODClanSum0408
- `IDX` `int` nullable=NO
- `CIDX` `int` nullable=NO
- `Server` `int` nullable=NO
- `ClanName` `varchar` nullable=NO
- `ServerName` `varchar` nullable=NO
- `CPointSum` `int` nullable=NO
- `CRegistDay` `datetime` nullable=NO

### SodDb.dbo.SODClanSum0409
- `IDX` `int` nullable=NO
- `CIDX` `int` nullable=NO
- `Server` `int` nullable=NO
- `ClanName` `varchar` nullable=NO
- `ServerName` `varchar` nullable=NO
- `CPointSum` `int` nullable=NO
- `CRegistDay` `datetime` nullable=NO

### SodDb.dbo.SODClanSum0410
- `IDX` `int` nullable=NO
- `CIDX` `int` nullable=NO
- `Server` `int` nullable=NO
- `ClanName` `varchar` nullable=NO
- `ServerName` `varchar` nullable=NO
- `CPointSum` `int` nullable=NO
- `CRegistDay` `datetime` nullable=NO

### SodDb.dbo.SODClanSum0411
- `IDX` `int` nullable=NO
- `CIDX` `int` nullable=NO
- `Server` `int` nullable=NO
- `ClanName` `varchar` nullable=NO
- `ServerName` `varchar` nullable=NO
- `CPointSum` `int` nullable=NO
- `CRegistDay` `datetime` nullable=NO

### SodDb.dbo.SODClanSum0412
- `IDX` `int` nullable=NO
- `CIDX` `int` nullable=NO
- `Server` `int` nullable=NO
- `ClanName` `varchar` nullable=NO
- `ServerName` `varchar` nullable=NO
- `CPointSum` `int` nullable=NO
- `CRegistDay` `datetime` nullable=NO

### SodDb.dbo.SODRecord0401
- `SNo` `int` nullable=NO
- `UserID` `varchar` nullable=YES
- `CharName` `varchar` nullable=YES
- `CharType` `int` nullable=YES
- `Point` `int` nullable=YES
- `KillCount` `int` nullable=YES
- `GLevel` `int` nullable=YES
- `TotalPoint` `int` nullable=YES
- `TotalUser` `int` nullable=YES
- `SuccessUser` `int` nullable=YES
- `ServerName` `varchar` nullable=YES
- `PCRNo` `int` nullable=YES
- `GPCode` `varchar` nullable=YES
- `BusinessName` `varchar` nullable=YES
- `BAddress1` `varchar` nullable=YES
- `PMNo` `int` nullable=YES
- `Contribute` `int` nullable=YES
- `RegistDay` `datetime` nullable=YES
- `ServerTime` `bigint` nullable=YES
- `Flag` `int` nullable=YES
- `ClanIDX` `int` nullable=YES
- `ClanName` `varchar` nullable=YES

### SodDb.dbo.SODRecord0402
- `SNo` `int` nullable=NO
- `UserID` `varchar` nullable=YES
- `CharName` `varchar` nullable=YES
- `CharType` `int` nullable=YES
- `Point` `int` nullable=YES
- `KillCount` `int` nullable=YES
- `GLevel` `int` nullable=YES
- `TotalPoint` `int` nullable=YES
- `TotalUser` `int` nullable=YES
- `SuccessUser` `int` nullable=YES
- `ServerName` `varchar` nullable=YES
- `PCRNo` `int` nullable=YES
- `GPCode` `varchar` nullable=YES
- `BusinessName` `varchar` nullable=YES
- `BAddress1` `varchar` nullable=YES
- `PMNo` `int` nullable=YES
- `Contribute` `int` nullable=YES
- `RegistDay` `datetime` nullable=YES
- `ServerTime` `bigint` nullable=YES
- `Flag` `int` nullable=YES
- `ClanIDX` `int` nullable=YES
- `ClanName` `varchar` nullable=YES

### SodDb.dbo.SODRecord0403
- `SNo` `int` nullable=NO
- `UserID` `varchar` nullable=YES
- `CharName` `varchar` nullable=YES
- `CharType` `int` nullable=YES
- `Point` `int` nullable=YES
- `KillCount` `int` nullable=YES
- `GLevel` `int` nullable=YES
- `TotalPoint` `int` nullable=YES
- `TotalUser` `int` nullable=YES
- `SuccessUser` `int` nullable=YES
- `ServerName` `varchar` nullable=YES
- `PCRNo` `int` nullable=YES
- `GPCode` `varchar` nullable=YES
- `BusinessName` `varchar` nullable=YES
- `BAddress1` `varchar` nullable=YES
- `PMNo` `int` nullable=YES
- `Contribute` `int` nullable=YES
- `RegistDay` `datetime` nullable=YES
- `ServerTime` `bigint` nullable=YES
- `Flag` `int` nullable=YES
- `ClanIDX` `int` nullable=YES
- `ClanName` `varchar` nullable=YES

### SodDb.dbo.SODRecord0404
- `SNo` `int` nullable=NO
- `UserID` `varchar` nullable=YES
- `CharName` `varchar` nullable=YES
- `CharType` `int` nullable=YES
- `Point` `int` nullable=YES
- `KillCount` `int` nullable=YES
- `GLevel` `int` nullable=YES
- `TotalPoint` `int` nullable=YES
- `TotalUser` `int` nullable=YES
- `SuccessUser` `int` nullable=YES
- `ServerName` `varchar` nullable=YES
- `PCRNo` `int` nullable=YES
- `GPCode` `varchar` nullable=YES
- `BusinessName` `varchar` nullable=YES
- `BAddress1` `varchar` nullable=YES
- `PMNo` `int` nullable=YES
- `Contribute` `int` nullable=YES
- `RegistDay` `datetime` nullable=YES
- `ServerTime` `bigint` nullable=YES
- `Flag` `int` nullable=YES
- `ClanIDX` `int` nullable=YES
- `ClanName` `varchar` nullable=YES

### SodDb.dbo.SODRecord0405
- `SNo` `int` nullable=NO
- `UserID` `varchar` nullable=YES
- `CharName` `varchar` nullable=YES
- `CharType` `int` nullable=YES
- `Point` `int` nullable=YES
- `KillCount` `int` nullable=YES
- `GLevel` `int` nullable=YES
- `TotalPoint` `int` nullable=YES
- `TotalUser` `int` nullable=YES
- `SuccessUser` `int` nullable=YES
- `ServerName` `varchar` nullable=YES
- `PCRNo` `int` nullable=YES
- `GPCode` `varchar` nullable=YES
- `BusinessName` `varchar` nullable=YES
- `BAddress1` `varchar` nullable=YES
- `PMNo` `int` nullable=YES
- `Contribute` `int` nullable=YES
- `RegistDay` `datetime` nullable=YES
- `ServerTime` `bigint` nullable=YES
- `Flag` `int` nullable=YES
- `ClanIDX` `int` nullable=YES
- `ClanName` `varchar` nullable=YES

### SodDb.dbo.SODRecord0406
- `SNo` `int` nullable=NO
- `UserID` `varchar` nullable=YES
- `CharName` `varchar` nullable=YES
- `CharType` `int` nullable=YES
- `Point` `int` nullable=YES
- `KillCount` `int` nullable=YES
- `GLevel` `int` nullable=YES
- `TotalPoint` `int` nullable=YES
- `TotalUser` `int` nullable=YES
- `SuccessUser` `int` nullable=YES
- `ServerName` `varchar` nullable=YES
- `PCRNo` `int` nullable=YES
- `GPCode` `varchar` nullable=YES
- `BusinessName` `varchar` nullable=YES
- `BAddress1` `varchar` nullable=YES
- `PMNo` `int` nullable=YES
- `Contribute` `int` nullable=YES
- `RegistDay` `datetime` nullable=YES
- `ServerTime` `bigint` nullable=YES
- `Flag` `int` nullable=YES
- `ClanIDX` `int` nullable=YES
- `ClanName` `varchar` nullable=YES

### SodDb.dbo.SODRecord0407
- `SNo` `int` nullable=NO
- `UserID` `varchar` nullable=YES
- `CharName` `varchar` nullable=YES
- `CharType` `int` nullable=YES
- `Point` `int` nullable=YES
- `KillCount` `int` nullable=YES
- `GLevel` `int` nullable=YES
- `TotalPoint` `int` nullable=YES
- `TotalUser` `int` nullable=YES
- `SuccessUser` `int` nullable=YES
- `ServerName` `varchar` nullable=YES
- `PCRNo` `int` nullable=YES
- `GPCode` `varchar` nullable=YES
- `BusinessName` `varchar` nullable=YES
- `BAddress1` `varchar` nullable=YES
- `PMNo` `int` nullable=YES
- `Contribute` `int` nullable=YES
- `RegistDay` `datetime` nullable=YES
- `ServerTime` `bigint` nullable=YES
- `Flag` `int` nullable=YES
- `ClanIDX` `int` nullable=YES
- `ClanName` `varchar` nullable=YES

### SodDb.dbo.SODRecord0408
- `SNo` `int` nullable=NO
- `UserID` `varchar` nullable=YES
- `CharName` `varchar` nullable=YES
- `CharType` `int` nullable=YES
- `Point` `int` nullable=YES
- `KillCount` `int` nullable=YES
- `GLevel` `int` nullable=YES
- `TotalPoint` `int` nullable=YES
- `TotalUser` `int` nullable=YES
- `SuccessUser` `int` nullable=YES
- `ServerName` `varchar` nullable=YES
- `PCRNo` `int` nullable=YES
- `GPCode` `varchar` nullable=YES
- `BusinessName` `varchar` nullable=YES
- `BAddress1` `varchar` nullable=YES
- `PMNo` `int` nullable=YES
- `Contribute` `int` nullable=YES
- `RegistDay` `datetime` nullable=YES
- `ServerTime` `bigint` nullable=YES
- `Flag` `int` nullable=YES
- `ClanIDX` `int` nullable=YES
- `ClanName` `varchar` nullable=YES

### SodDb.dbo.SODRecord0409
- `SNo` `int` nullable=NO
- `UserID` `varchar` nullable=YES
- `CharName` `varchar` nullable=YES
- `CharType` `int` nullable=YES
- `Point` `int` nullable=YES
- `KillCount` `int` nullable=YES
- `GLevel` `int` nullable=YES
- `TotalPoint` `int` nullable=YES
- `TotalUser` `int` nullable=YES
- `SuccessUser` `int` nullable=YES
- `ServerName` `varchar` nullable=YES
- `PCRNo` `int` nullable=YES
- `GPCode` `varchar` nullable=YES
- `BusinessName` `varchar` nullable=YES
- `BAddress1` `varchar` nullable=YES
- `PMNo` `int` nullable=YES
- `Contribute` `int` nullable=YES
- `RegistDay` `datetime` nullable=YES
- `ServerTime` `bigint` nullable=YES
- `Flag` `int` nullable=YES
- `ClanIDX` `int` nullable=YES
- `ClanName` `varchar` nullable=YES

### SodDb.dbo.SODRecord0410
- `SNo` `int` nullable=NO
- `UserID` `varchar` nullable=YES
- `CharName` `varchar` nullable=YES
- `CharType` `int` nullable=YES
- `Point` `int` nullable=YES
- `KillCount` `int` nullable=YES
- `GLevel` `int` nullable=YES
- `TotalPoint` `int` nullable=YES
- `TotalUser` `int` nullable=YES
- `SuccessUser` `int` nullable=YES
- `ServerName` `varchar` nullable=YES
- `PCRNo` `int` nullable=YES
- `GPCode` `varchar` nullable=YES
- `BusinessName` `varchar` nullable=YES
- `BAddress1` `varchar` nullable=YES
- `PMNo` `int` nullable=YES
- `Contribute` `int` nullable=YES
- `RegistDay` `datetime` nullable=YES
- `ServerTime` `bigint` nullable=YES
- `Flag` `int` nullable=YES
- `ClanIDX` `int` nullable=YES
- `ClanName` `varchar` nullable=YES

### SodDb.dbo.SODRecord0411
- `SNo` `int` nullable=NO
- `UserID` `varchar` nullable=YES
- `CharName` `varchar` nullable=YES
- `CharType` `int` nullable=YES
- `Point` `int` nullable=YES
- `KillCount` `int` nullable=YES
- `GLevel` `int` nullable=YES
- `TotalPoint` `int` nullable=YES
- `TotalUser` `int` nullable=YES
- `SuccessUser` `int` nullable=YES
- `ServerName` `varchar` nullable=YES
- `PCRNo` `int` nullable=YES
- `GPCode` `varchar` nullable=YES
- `BusinessName` `varchar` nullable=YES
- `BAddress1` `varchar` nullable=YES
- `PMNo` `int` nullable=YES
- `Contribute` `int` nullable=YES
- `RegistDay` `datetime` nullable=YES
- `ServerTime` `bigint` nullable=YES
- `Flag` `int` nullable=YES
- `ClanIDX` `int` nullable=YES
- `ClanName` `varchar` nullable=YES

### SodDb.dbo.SODRecord0412
- `SNo` `int` nullable=NO
- `UserID` `varchar` nullable=YES
- `CharName` `varchar` nullable=YES
- `CharType` `int` nullable=YES
- `Point` `int` nullable=YES
- `KillCount` `int` nullable=YES
- `GLevel` `int` nullable=YES
- `TotalPoint` `int` nullable=YES
- `TotalUser` `int` nullable=YES
- `SuccessUser` `int` nullable=YES
- `ServerName` `varchar` nullable=YES
- `PCRNo` `int` nullable=YES
- `GPCode` `varchar` nullable=YES
- `BusinessName` `varchar` nullable=YES
- `BAddress1` `varchar` nullable=YES
- `PMNo` `int` nullable=YES
- `Contribute` `int` nullable=YES
- `RegistDay` `datetime` nullable=YES
- `ServerTime` `bigint` nullable=YES
- `Flag` `int` nullable=YES
- `ClanIDX` `int` nullable=YES
- `ClanName` `varchar` nullable=YES

### SodDb.dbo.SOFClanLog0403
- `IDX` `int` nullable=NO
- `LCIDX` `int` nullable=NO
- `LCServerTime` `bigint` nullable=NO
- `LServer` `int` nullable=NO
- `LCPoint` `int` nullable=NO
- `LCRegistDay` `datetime` nullable=NO

### SodDb.dbo.SOFClanLog0404
- `IDX` `int` nullable=NO
- `LCIDX` `int` nullable=NO
- `LCServerTime` `bigint` nullable=NO
- `LServer` `int` nullable=NO
- `LCPoint` `int` nullable=NO
- `LCRegistDay` `datetime` nullable=NO

### SodDb.dbo.SOFClanLog0405
- `IDX` `int` nullable=NO
- `LCIDX` `int` nullable=NO
- `LCServerTime` `bigint` nullable=NO
- `LServer` `int` nullable=NO
- `LCPoint` `int` nullable=NO
- `LCRegistDay` `datetime` nullable=NO

### SodDb.dbo.SOFClanLog0406
- `IDX` `int` nullable=NO
- `LCIDX` `int` nullable=NO
- `LCServerTime` `bigint` nullable=NO
- `LServer` `int` nullable=NO
- `LCPoint` `int` nullable=NO
- `LCRegistDay` `datetime` nullable=NO

### SodDb.dbo.SOFClanLog0407
- `IDX` `int` nullable=NO
- `LCIDX` `int` nullable=NO
- `LCServerTime` `bigint` nullable=NO
- `LServer` `int` nullable=NO
- `LCPoint` `int` nullable=NO
- `LCRegistDay` `datetime` nullable=NO

### SodDb.dbo.SOFClanLog0408
- `IDX` `int` nullable=NO
- `LCIDX` `int` nullable=NO
- `LCServerTime` `bigint` nullable=NO
- `LServer` `int` nullable=NO
- `LCPoint` `int` nullable=NO
- `LCRegistDay` `datetime` nullable=NO

### SodDb.dbo.SOFClanLog0409
- `IDX` `int` nullable=NO
- `LCIDX` `int` nullable=NO
- `LCServerTime` `bigint` nullable=NO
- `LServer` `int` nullable=NO
- `LCPoint` `int` nullable=NO
- `LCRegistDay` `datetime` nullable=NO

### SodDb.dbo.SOFClanLog0410
- `IDX` `int` nullable=NO
- `LCIDX` `int` nullable=NO
- `LCServerTime` `bigint` nullable=NO
- `LServer` `int` nullable=NO
- `LCPoint` `int` nullable=NO
- `LCRegistDay` `datetime` nullable=NO

### SodDb.dbo.SOFClanLog0411
- `IDX` `int` nullable=NO
- `LCIDX` `int` nullable=NO
- `LCServerTime` `bigint` nullable=NO
- `LServer` `int` nullable=NO
- `LCPoint` `int` nullable=NO
- `LCRegistDay` `datetime` nullable=NO

### SodDb.dbo.SOFClanLog0412
- `IDX` `int` nullable=NO
- `LCIDX` `int` nullable=NO
- `LCServerTime` `bigint` nullable=NO
- `LServer` `int` nullable=NO
- `LCPoint` `int` nullable=NO
- `LCRegistDay` `datetime` nullable=NO

## StartUserDB

### StartUserDB.dbo.Items
- `Classe` `int` nullable=NO
- `Weapon` `varchar` nullable=YES
- `Aging1` `int` nullable=YES
- `Mix1` `int` nullable=YES
- `Armor` `varchar` nullable=YES
- `Aging2` `int` nullable=YES
- `Mix2` `int` nullable=YES
- `Boots` `varchar` nullable=YES
- `Aging3` `int` nullable=YES
- `Mix3` `int` nullable=YES
- `Gloves` `varchar` nullable=YES
- `Aging4` `int` nullable=YES
- `Mix4` `int` nullable=YES
- `Bracelet` `varchar` nullable=YES
- `Aging5` `int` nullable=YES
- `Mix5` `int` nullable=YES

### StartUserDB.dbo.Level
- `Level` `int` nullable=YES

## rPTDB

### rPTDB.dbo.LevelList
- `CharName` `varchar` nullable=YES
- `CharLevel` `int` nullable=YES
- `CharClass` `varchar` nullable=YES
- `ID` `int` nullable=YES
- `ClanName` `nvarchar` nullable=YES
- `ExpChar` `int` nullable=YES
