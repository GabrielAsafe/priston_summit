int	DrawMapNPC(int px, int py)
{
	int cnt;
	int	x, y;
	int	size;
	float sz;
	float sx, sy;
	float fx, fy;
	int TransNum;

	size = FieldMapViewSize2;
	sz = (float)(FieldMapViewSize2 / 64);
	fx = (float)px;
	fy = (float)py;

	TransNum = 127;


	for (cnt = 0; cnt < OTHER_PLAYER_MAX; cnt++)
	{
		if (chrOtherPlayer[cnt].Flag && chrOtherPlayer[cnt].smCharInfo.State == smCHAR_STATE_NPC)
		{
			x = chrOtherPlayer[cnt].pX - lpCurPlayer->pX;
			y = chrOtherPlayer[cnt].pZ - lpCurPlayer->pZ;
			if (abs(x) < size && abs(y) < size)
			{

				sx = fx + (((float)x) / sz);
				sy = fy - (((float)y) / sz);
				dsDrawTexImageFloat(MatNpcPos, sx - 3, sy - 3, 8, 8, TransNum + 128, 0);
			}
		}
	}

	return TRUE;
}




int DrawMapArrow(int x, int y, int Angle)
{
	float fx[4], fy[4];
	int sinY, cosY;
	int dx[4], dy[4];
	int cnt;

	sinY = GetSin[Angle&ANGCLIP];
	cosY = GetCos[(Angle + ANGLE_180)&ANGCLIP];

	dx[0] = -8; dy[0] = -8;
	dx[1] = -8; dy[1] = 8;
	dx[2] = 8; dy[2] = -8;
	dx[3] = 8; dy[3] = 8;


	for (cnt = 0; cnt < 4; cnt++)
	{
		fx[cnt] = (float)((dy[cnt] * sinY + dx[cnt] * cosY) >> 8);
		fy[cnt] = (float)((dy[cnt] * cosY - dx[cnt] * sinY) >> 8);
		fx[cnt] /= 256;
		fy[cnt] /= 256;
		fx[cnt] += (float)x;
		fy[cnt] += (float)y;
	}


	psDrawTexImage2(smMaterial[MatArrow].smTexture[0], fx, fy, 0, 0, 1, 1, 255, 0);



	return TRUE;
}


int DrawFieldMap(sCOMPACT_MAP *lpCompactMap, int px, int py, int size)
{

	int x, z;
	int w, h;
	int left, right, bottom, top;
	float fx, fy, fw, fh;

	float dx, dy, dw, dh;
	float sc;

	if (!lpCompactMap->lpField || !lpCompactMap->hTexHandle) return FALSE;

	w = lpCompactMap->rx;
	h = lpCompactMap->ry;
	x = (lpCurPlayer->pX - lpCompactMap->StageRect.left);

	z = (lpCompactMap->StageRect.bottom - lpCurPlayer->pZ);

	left = x - size;
	right = x + size;
	top = z - size;
	bottom = z + size;

	dx = (float)px;
	dy = (float)py;
	dw = 126;
	dh = 126;

	float fpx;
	float fpy;
	float fmx, fmy;

	fpx = ((float)((size >> FLOATNS))) / (float)w;
	fpy = ((float)((size >> FLOATNS))) / (float)h;

	fmx = ((float)(x >> FLOATNS)) / (float)w;
	fmy = ((float)(z >> FLOATNS)) / (float)h;

	fx = fmx - fpx;
	fw = fmx + fpx;

	fy = fmy - fpy;
	fh = fmy + fpy;

	if ((fx < 0 && fw < 0) || (fx > 1 && fw > 1)) return FALSE;
	if ((fy < 0 && fh < 0) || (fy > 1 && fh > 1)) return FALSE;

	if (fx < 0)
	{
		sc = (-fx * (64 / fpx));
		dx = dx + sc;
		dw = dw - sc;
		fx = 0;
	}
	if (fw > 1)
	{
		sc = ((fw - 1)*(64 / fpx));
		dw = dw - sc;
		fw = 1;
	}

	if (fy < 0)
	{
		sc = (-fy * (64 / fpy));
		dy = dy + sc;
		dh = dh - sc;
		fy = 0;
	}
	if (fh > 1)
	{
		sc = ((fh - 1)*(64 / fpy));
		dh = dh - sc;
		fh = 1;
	}

	psDrawTexImage(lpCompactMap->hTexHandle, dx, dy, dw, dh, fx, fy, fw, fh, 255, 0);

	return TRUE;
}
