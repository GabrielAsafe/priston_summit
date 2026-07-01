<% 
Dim BlackList, ErrorPage, s

BlackList = Array("--", ";")

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

Sub CheckTicket(useridcheck, ticketcheck)
	QUERY = "SELECT SNo FROM CT WHERE UserID='" & useridcheck & "' AND ServerName='" & gserver & "'"
	RS.Open QUERY, objConn, 3, 1

	Dim tticket

	If RS.RecordCount >= 1 Then
		tticket = RS("SNo").Value
		If CInt(ticketcheck) = CInt(tticket) Then
			RS.Close
		Else
			RS.Close
			Set RS = Nothing
			objConn.Close
			Set objConn = Nothing
			Response.Write("Code=100" & strSplit)
			Response.End
		End If
	Else
		RS.Close
		Set RS = Nothing
		objConn.Close
		Set objConn = Nothing
		Response.Write("Code=100" & strSplit)
		Response.End
	End If
End Sub
%>