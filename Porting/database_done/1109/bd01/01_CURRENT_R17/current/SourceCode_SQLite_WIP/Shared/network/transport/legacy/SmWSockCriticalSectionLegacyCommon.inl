void smEnterCSectionSocket()
{

	EnterCriticalSection(&cSocketSection);
}


void smLeaveCSectionSocket()
{

	LeaveCriticalSection(&cSocketSection);
}
