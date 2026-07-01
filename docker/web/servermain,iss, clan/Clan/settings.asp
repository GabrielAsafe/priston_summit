<% 
Dim BlackList, ErrorPage, s



BlackList = Array("'", "--", ";", "/*", "*/", "@@", "@",_
                  "char", "nchar", "varchar", "nvarchar",_
                  "alter", "begin", "cast", "create", "cursor",_
                  "declare", "delete", "drop", "end", "exec",_
                  "execute", "fetch", "insert", "kill", "open",_
                  "select", "sys", "sysobjects", "syscolumns",_
                  "table", "update")

ErrorPage = "ErrorPage.asp"
               
Function CheckStringForSQL(str) 
  On Error Resume Next 
  
  Dim lstr 
  
  ' If the string is empty, return true
  If ( IsEmpty(str) ) Then
    CheckStringForSQL = false
    Exit Function
  ElseIf ( StrComp(str, "") = 0 ) Then
    CheckStringForSQL = false
    Exit Function
  End If
  
  lstr = LCase(str)
  
  ' Check if the string contains any patterns in our
  ' black list
  For Each s in BlackList
  
    If ( InStr (lstr, s) <> 0 ) Then
      CheckStringForSQL = true
      Exit Function
    End If
  
  Next
  
  CheckStringForSQL = false
  
End Function 


'''''''''''''''''''''''''''''''''''''''''''''''''''
'  Check forms data
'''''''''''''''''''''''''''''''''''''''''''''''''''

For Each s in Request.Form
  If ( CheckStringForSQL(Request.Form(s)) ) Then
  
    ' Redirect to an error page
    Response.Redirect(ErrorPage)
  
  End If
Next

'''''''''''''''''''''''''''''''''''''''''''''''''''
'  Check query string
'''''''''''''''''''''''''''''''''''''''''''''''''''

For Each s in Request.QueryString
  If ( CheckStringForSQL(Request.QueryString(s)) ) Then
  
    ' Redirect to error page
    Response.Redirect(ErrorPage)

    End If
  
Next


'''''''''''''''''''''''''''''''''''''''''''''''''''
'  Check cookies
'''''''''''''''''''''''''''''''''''''''''''''''''''

For Each s in Request.Cookies
  If ( CheckStringForSQL(Request.Cookies(s)) ) Then
  
    ' Redirect to error page
    Response.Redirect(ErrorPage)
 End If
  
Next

Sub FillSettings()
       dbhost = "PTL\SQLEXPRESS"
        dbuser = "sa"
        dbpass = "#$pdl32xq"
        dbname = "ClanDB"
End Sub
%>