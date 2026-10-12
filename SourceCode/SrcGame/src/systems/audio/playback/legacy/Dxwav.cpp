#include "diagnostics/RuntimeAssetProbe.h"
#include "SummitWavSlotReuse247.h"
#include "SummitWavPath250.h"
#include "SummitSoundBufferGuard253.h"

#include "Dxwav.h"
#include <windowsx.h>
#include <mmsystem.h>
#include <conio.h>
#include "systems/rendering/legacy_3d/smDsx.h"
#include "systems/configuration/client/Settings.h"
#include "Helper/common.h"

HRESULT     rval_g_lpDs;
HRESULT     dsrval_lpDirectSound;

int numSound = 0;

char dataDir[64];


int mouThRead;
BYTE *mouThdataOfs = NULL;

BOOL wav_opt = 1;

LPDIRECTSOUND           g_lpDS = NULL;
BOOL                    g_bSoundPresent = FALSE;
LPDIRECTSOUNDBUFFER     g_lpSounds[NUM_SOUNDS];

// Identical WAVs may reuse an already-filled DirectSound buffer within the
// same slot. Never share DirectSound buffers across slots or device resets.
static summit::audio::WavSlotReuse247<NUM_SOUNDS> summitWavSlots247;

static bool SummitWavStamp247(const char *path, summit::audio::WavStamp247 *stamp)
{
    if (!path || !*path || !stamp) return false;
    WIN32_FILE_ATTRIBUTE_DATA info = {};
    if (!GetFileAttributesExA(path, GetFileExInfoStandard, &info) ||
        (info.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)) return false;
    stamp->size = (static_cast<unsigned long long>(info.nFileSizeHigh) << 32) | info.nFileSizeLow;
    stamp->write_time = (static_cast<unsigned long long>(info.ftLastWriteTime.dwHighDateTime) << 32) |
                        info.ftLastWriteTime.dwLowDateTime;
    return true;
}

BOOL                    g_bSoundPaused = FALSE;


char sangJuWav[NUM_SOUNDS][64];
int  sangJuWav_F[NUM_SOUNDS] = { 1, };





int chk_load_sangJuWav(int wavN)
{
    if (wavN <= 0 || wavN >= NUM_SOUNDS) return 0;
	if(sangJuWav_F[wavN - 1] == 1) return 1;
	sangJuWav_F[wavN - 1] = 1;
	CreateBufferFromWaveFile(sangJuWav[wavN - 1], wavN);

	return 1;
}

char *sangJuWav2[NUM_SOUNDS] = {
"wav//rain01.wav",
"wav//thunder1.wav",
"wav//Button.wav",
"wav//New.wav",
"wav//CharacterDelete.WAV",
"wav//ButtonBack.wav",
"wav//CharacterSelect.wav",

"wav//NewChar.wav",
"wav//Fire.wav",
"wav//Thunder.wav",
"wav//GameStart.wav",
"wav//Login.wav",

"game//kill//sound//DoubleKill.wav",
"game//kill//sound//TripleKill.wav",
"game//kill//sound//QuadraKill.wav",
"game//kill//sound//PentaKill.wav",
"game//kill//sound//Shutdown.wav",
"game//kill//sound//Rampage.wav",
"game//kill//sound//Unstoppable.wav",
//"game//kill//sound//Legendary.wav",
//"game//kill//sound//KillingSpree.wav",

NULL
};

char *wnameBuf[NUM_SOUNDS] = {
"wav//bgm0.wav",
"wav//rain01.wav",
"wav//thunder1.wav",
"wav//step01.wav",
"wav//Effects//weapon//one hand swing - small 3.wav",
	NULL
};

int isPlaying(int i);


char *backMusicWav[] = {
"wav//622s.wav",
NULL,
NULL,
};


int  BackMusic_Load(int stageNumber)
{
    if (!g_bSoundPresent || stageNumber < 0 ||
        stageNumber >= static_cast<int>(_countof(backMusicWav)) ||
        !backMusicWav[stageNumber]) return 0;
    char strFilename[128];
    if (!summit::audio::JoinWavPath250(dataDir, sizeof(dataDir),
        backMusicWav[stageNumber], strlen(backMusicWav[stageNumber]) + 1,
        strFilename, sizeof(strFilename))) return 0;

	if(!CreateBufferFromWaveFile(strFilename, 0))
	{
		return FALSE;
	}
	return 1;
}

void BackMusicPlay()
{
	PlayWav(0, 1);
}


void BackMusicClr()
{
	StopAllWav();
	sBufferClear(0);
}

void PlayWavEffect(int num)
{
	PlayWav(num);
}

void BackMusic_Stop()
{
	StopWav(0);
}










BOOL InitDX_Wav(HWND g_hwnd)
{
	summitWavSlots247.Clear();
	if(!wav_opt)
	{
		g_bSoundPresent = FALSE;
		return 0;
	}

	int i;









	if(rval_g_lpDs == DS_OK)
		g_bSoundPresent = TRUE;
	else
		g_bSoundPresent = FALSE;


	if(g_bSoundPresent)
	{



		g_lpDS->SetCooperativeLevel(g_hwnd, DSSCL_NORMAL);


		for(i = 0; i < NUM_SOUNDS; i++)
		{
			g_lpSounds[i] = NULL;
		}

		for(i = 0; i < NUM_SOUNDS; i++)
		{
			if(wnameBuf[i] != NULL)
			{
				if(!CreateBufferFromWaveFile(wnameBuf[i], i))

					return FALSE;
				numSound++;
			}
		}
	}

	return TRUE;
}









void TermDX_Wav()
{
	summitWavSlots247.Clear();
	if(!g_bSoundPresent) return;


	for(int i = 0; i < NUM_SOUNDS; i++)
	{
		if(g_lpSounds[i])
		{
			g_lpSounds[i]->Release();
			g_lpSounds[i] = NULL;
		}
	}


	if(g_lpDS)
	{
		g_lpDS->Release();
		g_lpDS = NULL;
	}
}









BOOL CreateSoundBuffer(DWORD dwBuf, DWORD dwBufSize, DWORD dwFreq, DWORD dwBitsPerSample, DWORD dwBlkAlign, BOOL bStereo)
{
	if (dwBuf >= NUM_SOUNDS || !g_lpDS) return FALSE;
	summitWavSlots247.Forget(dwBuf);
	PCMWAVEFORMAT pcmwf;
	DSBUFFERDESC dsbdesc;


	memset(&pcmwf, 0, sizeof(PCMWAVEFORMAT));
	pcmwf.wf.wFormatTag = WAVE_FORMAT_PCM;
	pcmwf.wf.nChannels = bStereo ? 2 : 1;
	pcmwf.wf.nSamplesPerSec = dwFreq;
	pcmwf.wf.nBlockAlign = (WORD)dwBlkAlign;
	pcmwf.wf.nAvgBytesPerSec = pcmwf.wf.nSamplesPerSec * pcmwf.wf.nBlockAlign;
	pcmwf.wBitsPerSample = (WORD)dwBitsPerSample;


	memset(&dsbdesc, 0, sizeof(DSBUFFERDESC));
	dsbdesc.dwSize = sizeof(DSBUFFERDESC);

	dsbdesc.dwFlags = DSBCAPS_CTRLPAN | DSBCAPS_CTRLVOLUME | DSBCAPS_CTRLFREQUENCY;

	dsbdesc.dwBufferBytes = dwBufSize;
	dsbdesc.lpwfxFormat = (LPWAVEFORMATEX)&pcmwf;

    g_lpSounds[dwBuf] = NULL;
    return SUCCEEDED(g_lpDS->CreateSoundBuffer(&dsbdesc,
        &g_lpSounds[dwBuf], NULL)) && g_lpSounds[dwBuf] ? TRUE : FALSE;
}








BOOL ReadData(LPDIRECTSOUNDBUFFER lpDSB, FILE* pFile, DWORD dwSize, DWORD dwPos)
{
    if (!lpDSB || !pFile || !dwSize) return FALSE;

	if(dwPos != 0xffffffff)
	{
		if(fseek(pFile, dwPos, SEEK_SET) != 0)
		{
			return FALSE;
		}
	}


	LPVOID pData1;
	DWORD  dwData1Size;
	LPVOID pData2;
	DWORD  dwData2Size;
	HRESULT rval;
	rval = lpDSB->Lock(0, dwSize, &pData1, &dwData1Size, &pData2, &dwData2Size, DSBLOCK_FROMWRITECURSOR);
	if(rval != DS_OK)
	{
		return FALSE;
	}


    // Every successful Lock must be followed by exactly one Unlock, including
    // short/corrupt WAV reads. The old early returns left DirectSound locked.
    bool readOk = true;
    if (dwData1Size > 0)
    {
        readOk = fread(pData1, dwData1Size, 1, pFile) == 1;
        if (readOk && mouThRead == 0)
            mouThdataOfs = (BYTE*)pData1;
    }
    if (readOk && dwData2Size > 0)
        readOk = fread(pData2, dwData2Size, 1, pFile) == 1;
    rval = lpDSB->Unlock(pData1, dwData1Size, pData2, dwData2Size);
    return readOk && SUCCEEDED(rval) ? TRUE : FALSE;
}









BOOL CreateBufferFromWaveFile(char* FileName, DWORD dwBuf)
{
	if (!FileName || dwBuf >= NUM_SOUNDS) return FALSE;
	summitWavSlots247.Forget(dwBuf);
	summit::audio::WavStamp247 stampBefore = {};
	const bool couldRemember = SummitWavStamp247(FileName, &stampBefore);
	summit::diagnostics::RuntimeAssetProbeAttempt("AUDIO_WAV", "OPEN", FileName);
	DWORD dwSize;
	BOOL bStereo;

	FILE* pFile = fopen(FileName, "rb");
	summit::diagnostics::RuntimeAssetProbeResult("AUDIO_WAV", "OPEN", FileName, pFile != NULL);
	if(pFile == NULL) return FALSE;


	WaveHeader1 wavHdr1;
	WaveHeader2 wavHdr2;

	WaveHeader0 wavHdr;
	if(fread(&wavHdr, sizeof(wavHdr), 1, pFile) != 1)
	{
		fclose(pFile);
		return NULL;
	}

	if(wavHdr.DATA[0] == 'd' && wavHdr.DATA[1] == 'a')
	{

        if (fseek(pFile, 0, SEEK_SET) != 0 ||
            fread(&wavHdr1, sizeof(wavHdr1), 1, pFile) != 1)
        { fclose(pFile); return FALSE; }
		dwSize = wavHdr1.dwDSize;

		bStereo = wavHdr1.wChnls > 1 ? TRUE : FALSE;

        if (!dwSize) { fclose(pFile); return FALSE; }
        summit::audio::ReleaseSoundBuffer253(g_lpSounds[dwBuf]);
		if(!CreateSoundBuffer(dwBuf, dwSize, wavHdr1.dwSRate, wavHdr1.BitsPerSample, wavHdr1.wBlkAlign, bStereo))
		{

			fclose(pFile);
			return FALSE;
		}
		if(!g_lpSounds[dwBuf])
		{
			fclose(pFile);
			return FALSE;
		}

		mouThRead = dwBuf;

		if(!ReadData(g_lpSounds[dwBuf], pFile, dwSize, sizeof(wavHdr1)))
        {
            summit::audio::ReleaseSoundBuffer253(g_lpSounds[dwBuf]);
            fclose(pFile);
            return FALSE;
        }
	}
	else if(wavHdr.DATA[0] == 'f' && wavHdr.DATA[1] == 'a')
	{

        if (fseek(pFile, 0, SEEK_SET) != 0 ||
            fread(&wavHdr2, sizeof(wavHdr2), 1, pFile) != 1)
        { fclose(pFile); return FALSE; }

		dwSize = wavHdr2.dwDSize;

		bStereo = wavHdr2.wChnls > 1 ? TRUE : FALSE;


        if (!dwSize) { fclose(pFile); return FALSE; }
        summit::audio::ReleaseSoundBuffer253(g_lpSounds[dwBuf]);
		if(!CreateSoundBuffer(dwBuf, dwSize, wavHdr2.dwSRate, wavHdr2.BitsPerSample, wavHdr2.wBlkAlign, bStereo))
		{

			fclose(pFile);
			return FALSE;
		}


		mouThRead = dwBuf;

		if(!ReadData(g_lpSounds[dwBuf], pFile, dwSize, sizeof(wavHdr2)))
        {
            summit::audio::ReleaseSoundBuffer253(g_lpSounds[dwBuf]);
            fclose(pFile);
            return FALSE;
        }
	}
	else
	{


		dwSize = wavHdr.dwDSize;

		bStereo = wavHdr.wChnls > 1 ? TRUE : FALSE;


        if (!dwSize) { fclose(pFile); return FALSE; }
        summit::audio::ReleaseSoundBuffer253(g_lpSounds[dwBuf]);
		if(!CreateSoundBuffer(dwBuf, dwSize, wavHdr.dwSRate, wavHdr.BitsPerSample, wavHdr.wBlkAlign, bStereo))
		{

			fclose(pFile);
			return FALSE;
		}


		mouThRead = dwBuf;

		if(!ReadData(g_lpSounds[dwBuf], pFile, dwSize, sizeof(wavHdr)))
		{
			summit::audio::ReleaseSoundBuffer253(g_lpSounds[dwBuf]);
            fclose(pFile);
			return FALSE;
		}
	}


	fclose(pFile);
	// Same source both before and after read; failed loads never become reusable.
	summit::audio::WavStamp247 stampAfter = {};
	if (g_lpSounds[dwBuf] && couldRemember &&
	    SummitWavStamp247(FileName, &stampAfter) &&
	    stampBefore.size == stampAfter.size &&
	    stampBefore.write_time == stampAfter.write_time)
	    summitWavSlots247.Remember(dwBuf, FileName, stampAfter);

	return TRUE;
}









BOOL ReadDataWavBuf(LPDIRECTSOUNDBUFFER lpDSB, BYTE *wavBUF, DWORD dwSize)
{


	int cnt = 0;

	LPVOID pData1;
	DWORD  dwData1Size;
	LPVOID pData2;
	DWORD  dwData2Size;
	HRESULT rval;

	rval = lpDSB->Lock(0, dwSize, &pData1, &dwData1Size, &pData2, &dwData2Size, DSBLOCK_FROMWRITECURSOR);
	if(rval != DS_OK)
	{
		return FALSE;
	}


	if(dwData1Size > 0)
	{




		memcpy(pData1, &wavBUF[cnt], dwData1Size);
		cnt += dwData1Size;

		if(mouThRead == 0)
		{
			mouThdataOfs = (BYTE*)pData1;
		}

	}


	if(dwData2Size > 0)
	{




		memcpy(pData2, &wavBUF[cnt], dwData2Size);
	}


	rval = lpDSB->Unlock(pData1, dwData1Size, pData2, dwData2Size);
	if(rval != DS_OK)
	{
		return FALSE;
	}


	return TRUE;
}

BOOL CreateBufferFromWavBUF(unsigned char* wavBUf, unsigned int dwBuf)
{
	DWORD dwSize;
	BOOL bStereo;
	BYTE cnt = 0;






	WaveHeader1 wavHdr1;
	WaveHeader2 wavHdr2;

	WaveHeader0 wavHdr;






	memcpy(&wavHdr, &wavBUf[cnt], sizeof(wavHdr));
	cnt += sizeof(wavHdr);


	if(wavHdr.DATA[0] == 'd' && wavHdr.DATA[1] == 'a')
	{


		cnt = 0;
		memcpy(&wavHdr1, &wavBUf[cnt], sizeof(wavHdr1));
		cnt += sizeof(wavHdr1);


		dwSize = wavHdr1.dwDSize;

		bStereo = wavHdr1.wChnls > 1 ? TRUE : FALSE;



		if(!CreateSoundBuffer(dwBuf, dwSize, wavHdr1.dwSRate, wavHdr1.BitsPerSample, wavHdr1.wBlkAlign, bStereo))
		{


			return FALSE;
		}



		mouThRead = dwBuf;


		if(!ReadDataWavBuf(g_lpSounds[dwBuf], &wavBUf[cnt], dwSize))
		{

			return FALSE;
		}
	}

	else if(wavHdr.DATA[0] == 'f' && wavHdr.DATA[1] == 'a')
	{


		cnt = 0;
		memcpy(&wavHdr2, &wavBUf[cnt], sizeof(wavHdr2));
		cnt += sizeof(wavHdr2);


		dwSize = wavHdr2.dwDSize;

		bStereo = wavHdr2.wChnls > 1 ? TRUE : FALSE;




		if(!CreateSoundBuffer(dwBuf, dwSize, wavHdr2.dwSRate, wavHdr2.BitsPerSample, wavHdr2.wBlkAlign, bStereo))
		{


			return FALSE;
		}


		mouThRead = dwBuf;


		if(!ReadDataWavBuf(g_lpSounds[dwBuf], &wavBUf[cnt], dwSize))
		{

			return FALSE;
		}
	}
	else
	{

		dwSize = wavHdr.dwDSize;

		bStereo = wavHdr.wChnls > 1 ? TRUE : FALSE;




		if(!CreateSoundBuffer(dwBuf, dwSize, wavHdr.dwSRate, wavHdr.BitsPerSample, wavHdr.wBlkAlign, bStereo))
		{


			return FALSE;
		}


		mouThRead = dwBuf;


		if(!ReadDataWavBuf(g_lpSounds[dwBuf], &wavBUf[cnt], dwSize))
		{

			return FALSE;
		}
	}




	return TRUE;
}

















BOOL StopAllWav()
{
	if(!g_bSoundPresent) return FALSE;


	for(int i = 0; i < NUM_SOUNDS; i++)
	{
		if(g_lpSounds[i])
		{
			DWORD dwStatus;

			g_lpSounds[i]->GetStatus(&dwStatus);

			if((dwStatus & DSBSTATUS_PLAYING) == DSBSTATUS_PLAYING)
			{


				g_lpSounds[i]->Stop();

			}
		}
	}


	return TRUE;
}





#define VOL_SHIFT (-400)
#define VOL_MULT 10

#define PAN_SHIFT (-400)
#define PAN_MULT  10

#define FRQ_MULT  10









BOOL PlayWav(DWORD dwSound, DWORD dwFlags, DWORD dwVol, DWORD dwfreq)
{

	if(g_bSoundPaused) return TRUE;
	if(!g_bSoundPresent) return TRUE;
	
	if (!SETTINGSHANDLER->hasEffects()) return TRUE;

	if(dwSound >= NUM_SOUNDS) return FALSE;


	chk_load_sangJuWav((int)dwSound);



	if(g_lpSounds[dwSound])
	{
		DWORD dwStatus;
		g_lpSounds[dwSound]->GetStatus(&dwStatus);
		g_lpSounds[dwSound]->SetVolume(SETTINGSHANDLER->getEffectsVolume() ? (DSBVOLUME_MIN / 2) + ((abs(DSBVOLUME_MIN / 2) * SETTINGSHANDLER->getEffectsVolume()) / 100) : DSBVOLUME_MIN);
		g_lpSounds[dwSound]->SetFrequency(dwfreq*FRQ_MULT);
		if((dwStatus & DSBSTATUS_PLAYING) != DSBSTATUS_PLAYING)
		{

			
			g_lpSounds[dwSound]->Play(0, 0, dwFlags);

		}


		if(dwSound == 0)
			g_lpSounds[dwSound]->SetCurrentPosition(0);

	}


	return TRUE;
}



BOOL PlayWav2(DWORD dwSound, DWORD dwFlags, DWORD dwVol, DWORD dwfreq)
{
	if(g_bSoundPaused) return TRUE;
	if(!g_bSoundPresent) return TRUE;
	if (!SETTINGSHANDLER->hasEffects()) return TRUE;


	if(dwSound >= NUM_SOUNDS) return FALSE;


	chk_load_sangJuWav((int)dwSound);



	if(g_lpSounds[dwSound])
	{

		DWORD dwStatus;
		g_lpSounds[dwSound]->GetStatus(&dwStatus);
		if((dwStatus & DSBSTATUS_PLAYING) == DSBSTATUS_PLAYING)

			g_lpSounds[dwSound]->SetCurrentPosition(0);

		g_lpSounds[dwSound]->SetVolume(SETTINGSHANDLER->getEffectsVolume() ? (DSBVOLUME_MIN / 2) + ((abs(DSBVOLUME_MIN / 2) * SETTINGSHANDLER->getEffectsVolume()) / 100) : DSBVOLUME_MIN);
		g_lpSounds[dwSound]->SetFrequency(dwfreq*FRQ_MULT);
		g_lpSounds[dwSound]->Play(0, 0, dwFlags);
	}

	return TRUE;
}


BOOL IsPlayWav(DWORD dwSound)
{
	DWORD dwStatus;

	if(g_bSoundPaused) return FALSE;
	if(!g_bSoundPresent) return FALSE;
	if(dwSound >= NUM_SOUNDS) return FALSE;
	if(!g_lpSounds[dwSound]) return FALSE;

	g_lpSounds[dwSound]->GetStatus(&dwStatus);

	if((dwStatus & DSBSTATUS_PLAYING) != DSBSTATUS_PLAYING) return FALSE;

	return TRUE;
}





BOOL PlayWav3(DWORD dwSound, DWORD dwFlags, DWORD dwVol, DWORD dwfreq, DWORD dwPan)
{
	if(g_bSoundPaused) return TRUE;
	if(!g_bSoundPresent) return TRUE;
	if (!SETTINGSHANDLER->hasEffects()) return TRUE;


	if(dwSound >= NUM_SOUNDS) return FALSE;


	chk_load_sangJuWav((int)dwSound);


	if(g_lpSounds[dwSound])
	{

		DWORD dwStatus;
		g_lpSounds[dwSound]->GetStatus(&dwStatus);
		if((dwStatus & DSBSTATUS_PLAYING) == DSBSTATUS_PLAYING)

			g_lpSounds[dwSound]->SetCurrentPosition(0);


		g_lpSounds[dwSound]->SetPan((dwPan + PAN_SHIFT)*PAN_MULT);


		g_lpSounds[dwSound]->SetFrequency(dwfreq*FRQ_MULT);


		g_lpSounds[dwSound]->SetVolume(SETTINGSHANDLER->getEffectsVolume() ? (DSBVOLUME_MIN / 2) + ((abs(DSBVOLUME_MIN / 2) * SETTINGSHANDLER->getEffectsVolume()) / 100) : DSBVOLUME_MIN);
		g_lpSounds[dwSound]->Play(0, 0, dwFlags);
	}

	return TRUE;
}




BOOL PlayWav_VolChange(DWORD dwSound, DWORD dwVol)
{
	if(g_bSoundPaused) return TRUE;
	if(!g_bSoundPresent) return TRUE;
	if (!SETTINGSHANDLER->hasEffects()) return TRUE;


	if(dwSound >= NUM_SOUNDS) return FALSE;


	chk_load_sangJuWav((int)dwSound);


	if(g_lpSounds[dwSound])
	{
		DWORD dwStatus;
		g_lpSounds[dwSound]->GetStatus(&dwStatus);
		if((dwStatus & DSBSTATUS_PLAYING) == DSBSTATUS_PLAYING)
		{

			g_lpSounds[dwSound]->SetVolume(SETTINGSHANDLER->getEffectsVolume() ? (DSBVOLUME_MIN / 2) + ((abs(DSBVOLUME_MIN / 2) * SETTINGSHANDLER->getEffectsVolume()) / 100) : DSBVOLUME_MIN);
		}

	}
	return TRUE;
}

void PlayWav_UpdateVolume()
{
	for(int i = 0; i < _countof(g_lpSounds); i++)
		if(g_lpSounds[i])
			g_lpSounds[i]->SetVolume(SETTINGSHANDLER->getEffectsVolume() ? (DSBVOLUME_MIN / 2) + ((abs(DSBVOLUME_MIN / 2) * SETTINGSHANDLER->getEffectsVolume()) / 100) : DSBVOLUME_MIN);
}

BOOL PlayWav_FrequencyChange(DWORD dwSound, DWORD dwfreq)
{
	if(g_bSoundPaused) return TRUE;
	if(!g_bSoundPresent) return TRUE;


	if(dwSound >= NUM_SOUNDS) return FALSE;


	if(g_lpSounds[dwSound])
	{
		DWORD dwStatus;
		g_lpSounds[dwSound]->GetStatus(&dwStatus);
		if((dwStatus & DSBSTATUS_PLAYING) == DSBSTATUS_PLAYING)

			g_lpSounds[dwSound]->SetFrequency(dwfreq*FRQ_MULT);
	}
	return TRUE;
}



BOOL PlayWav_PanChange(DWORD dwSound, DWORD dwPan)
{
	if(g_bSoundPaused) return TRUE;
	if(!g_bSoundPresent) return TRUE;


	if(dwSound >= NUM_SOUNDS) return FALSE;


	if(g_lpSounds[dwSound])
	{
		DWORD dwStatus;
		g_lpSounds[dwSound]->GetStatus(&dwStatus);
		if((dwStatus & DSBSTATUS_PLAYING) == DSBSTATUS_PLAYING)

			g_lpSounds[dwSound]->SetPan((dwPan + PAN_SHIFT)*PAN_MULT);
	}
	return TRUE;
}



BOOL StopWav(int i);


void check_backMus(int flag)
{
	if(flag == 0)
	{
		StopWav(0);
	}
	else
	{
		StopWav(0);
		PlayWav(0, 1);
	}
}
















BOOL InitDX_Wav2(HWND g_hwnd)
{
	summitWavSlots247.Clear();
	char strFilename[128];
	if(!wav_opt)
	{
		g_bSoundPresent = FALSE;
		return 0;
	}

	int i;
	HRESULT     rval;




	rval = DirectSoundCreate(NULL, &g_lpDS, NULL);


	if(rval == DS_OK)
		g_bSoundPresent = TRUE;
	else
		g_bSoundPresent = FALSE;



	if(g_bSoundPresent)
	{

		g_lpDS->SetCooperativeLevel(g_hwnd, DSSCL_NORMAL);

		for(i = 0; i < NUM_SOUNDS; i++)
		{
			g_lpSounds[i] = NULL;
		}


		for(i = 1; i < NUM_SOUNDS; i++)
		{
			if(sangJuWav[i - 1] != NULL && sangJuWav[i - 1][0])
			{
				if(sangJuWav_F[i - 1] == 0)
					continue;

                if (!summit::audio::JoinWavPath250(dataDir, sizeof(dataDir),
                    sangJuWav[i - 1], sizeof(sangJuWav[i - 1]),
                    strFilename, sizeof(strFilename)))
                    continue;

				if(!CreateBufferFromWaveFile(strFilename, i))
				{




				}

			}
		}
	}







	Init_BGM(g_hwnd);
	return TRUE;
}





void TermDX_Wav2()
{
	summitWavSlots247.Clear();
	if(!g_bSoundPresent) return;

	for(int i = 0; i < NUM_SOUNDS; i++)
	{
		if(g_lpSounds[i])
		{
			PlayWav_VolChange(i, 0);
			g_lpSounds[i]->Release();	g_lpSounds[i] = NULL;
		}
	}
	if(g_lpDS)
	{
		g_lpDS->Release();
		g_lpDS = NULL;
	}

	Uninit_BGM();
}


BOOL CreateBufferFromWaveFile2(char* FileName, int sbufN)
{
    if (!g_bSoundPresent || !FileName || sbufN < 0 || sbufN >= NUM_SOUNDS)
        return FALSE;

    summit::audio::WavStamp247 stamp = {};
    if (g_lpSounds[sbufN] && SummitWavStamp247(FileName, &stamp) &&
        summitWavSlots247.Matches(sbufN, FileName, stamp))
    {
        StopWav(sbufN);
        // Even if PlayWav2 is called after the previous sound stopped, replay
        // must start at position zero, just as a freshly created buffer does.
        if (SUCCEEDED(g_lpSounds[sbufN]->SetCurrentPosition(0)))
        {
            mouThRead = sbufN; // Preserve CreateBufferFromWaveFile side effect.
            return TRUE;
        }
        summitWavSlots247.Forget(sbufN); // device lost: reload via legacy path
    }

    StopWav(sbufN);
    sBufferClear(sbufN);
    if (!CreateBufferFromWaveFile(FileName, sbufN))
    {
        sBufferClear(sbufN);
        return FALSE;
    }
    return TRUE;
}


BOOL sBufferClear(int sbufN)
{
    if (sbufN < 0 || sbufN >= NUM_SOUNDS) return FALSE;
    summitWavSlots247.Forget(sbufN);
	if(!g_bSoundPresent)
	{
		return FALSE;
	}

	if(g_lpSounds[sbufN])
	{
		g_lpSounds[sbufN]->Release();
		g_lpSounds[sbufN] = NULL;
		return TRUE;
	}
	return FALSE;
}


BOOL StopWav(int i)
{
    if (i < 0 || i >= NUM_SOUNDS) return FALSE;
	if(!g_bSoundPresent)
	{
		return FALSE;
	}


	if(g_lpSounds[i])
	{
		DWORD dwStatus;

		g_lpSounds[i]->GetStatus(&dwStatus);

		if((dwStatus & DSBSTATUS_PLAYING) == DSBSTATUS_PLAYING)
		{


			g_lpSounds[i]->Stop();
		}
	}

	return TRUE;
}



void allWAVE_playORstop(int flag)
{
	int i;
	if(flag == 0)
	{
		for(i = 0; i < NUM_SOUNDS; i++)
		{
			if(g_lpSounds[i])
			{
				StopWav(i);
			}
		}
	}
	else
	{

		if(g_lpSounds[0])
		{
			StopWav(0);
			PlayWav(0);
		}
	}
}












DWORD get_pos(DWORD dwSound)
{
	DWORD pos;
	if(g_bSoundPaused) return TRUE;
	if(!g_bSoundPresent) return TRUE;


	if(dwSound >= NUM_SOUNDS) return FALSE;


	if(g_lpSounds[dwSound])
	{
		DWORD dwStatus;
		g_lpSounds[dwSound]->GetStatus(&dwStatus);

		if((dwStatus & DSBSTATUS_PLAYING) == DSBSTATUS_PLAYING)
		{

			g_lpSounds[dwSound]->GetCurrentPosition(&pos, NULL);



			return pos;
		}
	}


	return 0;
}







BYTE getmouTH()
{
	if(mouThdataOfs == NULL) return 0;

	DWORD pos = get_pos(0);
	BYTE b = mouThdataOfs[pos];
	return b;
}

int isPlaying(int i)
{
	if(!g_bSoundPresent) return FALSE;


	if(g_lpSounds[i])
	{
		DWORD dwStatus;

		g_lpSounds[i]->GetStatus(&dwStatus);

		if((dwStatus & DSBSTATUS_PLAYING) == DSBSTATUS_PLAYING)
		{


			return 1;
		}
		else
		{
			return 0;
		}
	}

	return 0;
}











































































#include "Wave.h"
#include "SHELL.H"
#include "STREAM.H"




HRESULT SetupStreamBuffer(LPSTR lpszFilename);


LPDIRECTSOUND       lpDirectSound = NULL;

LPDIRECTSOUNDBUFFER lpDSBStreamBuffer = NULL;


HWND    m_hWnd;

LONG  m_lInTimer = FALSE;
BOOL  m_bTimerInit = FALSE;
BOOL  m_bPlaying = FALSE;
BOOL  m_bIsCloseOnRun = FALSE;
BOOL  m_bFileOpen = FALSE;
UINT  m_uTimerID = 0;
DWORD m_SecondBuffFocus = 0;

WAVEINFOCA  m_wiWave;



void LSStopBackground(DWORD dwState);
void LSResetWavePlayer(void);
void LSFillDataBuffer(void);
void CALLBACK LSTimeRunFuncForDSound(UINT uTimerID, UINT uMsg, DWORD dwUser, DWORD dw1, DWORD dw2);







void LSStopBackground(DWORD dwState)
{
	m_wiWave.bDonePlaying = TRUE;
	if(m_bTimerInit)
	{
		timeKillEvent(m_uTimerID);
		timeEndPeriod(PLAYBACK_TIMER_PERIOD / PLAYBACK_OVERSAMPLE);

		while(InterlockedExchange(&m_lInTimer, TRUE)) Sleep(100);
		m_bTimerInit = FALSE;
	}
	if(m_bPlaying)
	{
		m_bPlaying = FALSE;
		lpDSBStreamBuffer->Stop();
	}

	if(!(dwState & DSSTREAM_STOPF_NOREOPEN) && !m_bIsCloseOnRun)
	{
		LSResetWavePlayer();
		return;
	}
	else
	{
		if(m_bFileOpen)
		{
			WaveCloseReadFile(&m_wiWave.hmmio, &m_wiWave.pwfx);
			if(lpDSBStreamBuffer)
			{
				lpDSBStreamBuffer->Release();
			}

			lpDSBStreamBuffer = NULL;

			m_bFileOpen = FALSE;
		}
	}
}




void LSResetWavePlayer(void)
{
	WaveStartDataRead(&m_wiWave.hmmio, &m_wiWave.mmck, &m_wiWave.mmckInRIFF);
	m_wiWave.bFoundEnd = FALSE;
	m_wiWave.dwBytesRemaining = 0;

	LSFillDataBuffer();

	m_wiWave.bDonePlaying = FALSE;
}




void LSFillDataBuffer(void)
{
	LPBYTE   lpWrite1, lpWrite2;
	DWORD    dwLen1, dwLen2;
	UINT     uActualBytesWritten;
	HRESULT  dsrval;
	dsrval = lpDSBStreamBuffer->Lock(0, m_wiWave.dwBufferSize,

		(LPVOID*)&lpWrite1,
									 &dwLen1,

									 (LPVOID *)&lpWrite2,

									 &dwLen2, 0);
	if(dsrval != DS_OK) return;


	if(dwLen1)
	{

		WaveReadFile(m_wiWave.hmmio, (UINT)dwLen1, lpWrite1, &m_wiWave.mmck, &uActualBytesWritten);
		if(uActualBytesWritten < dwLen1)
		{
			if(m_wiWave.bLoopFile)
			{



				LPBYTE lpTemp = lpWrite1;
				DWORD  cbReadLoopTotal = dwLen1;
				do
				{





					lpTemp += uActualBytesWritten;
					cbReadLoopTotal -= uActualBytesWritten;
					WaveStartDataRead(&m_wiWave.hmmio, &m_wiWave.mmck, &m_wiWave.mmckInRIFF);
					WaveReadFile(m_wiWave.hmmio, (UINT)cbReadLoopTotal, lpTemp, &m_wiWave.mmck, &uActualBytesWritten);
				} while(uActualBytesWritten < cbReadLoopTotal);
			}
			else
			{
				m_wiWave.bFoundEnd = TRUE;
				m_wiWave.dwBytesRemaining = (DWORD)uActualBytesWritten;

				FillMemory(lpWrite1 + uActualBytesWritten, dwLen1 - uActualBytesWritten,
					(BYTE)(m_wiWave.pwfx->wBitsPerSample == 8 ? 128 : 0));
			}
		}
	}
	dsrval = lpDSBStreamBuffer->Unlock((LPVOID)lpWrite1, dwLen1, (LPVOID)lpWrite2, 0);
	m_wiWave.dwNextWriteOffset = m_wiWave.dwProgress = 0;
}






void CALLBACK LSTimeRunFuncForDSound(UINT uTimerID, UINT uMsg, DWORD dwUser, DWORD dw1, DWORD dw2)
{
	LPBYTE  lpWrite1, lpWrite2, lpTemp;
	DWORD   dwLen1, dwLen2, dwPlay, dwWrite, dwPlayedLength, dwWriteLength;
	DWORD   dwLeftToRead, dwStatus;
	UINT    uChkErr;
	BOOL    fRefillLostBuffer = FALSE;
	HRESULT dsrval;




	if(InterlockedExchange(&m_lInTimer, TRUE)) return;


	m_wiWave.lpDSBStreamBuffer->GetStatus(&dwStatus);
	if(DSBSTATUS_BUFFERLOST & dwStatus)
	{


		dsrval = m_wiWave.lpDSBStreamBuffer->Restore();
		if(FAILED(dsrval))
		{
			InterlockedExchange(&m_lInTimer, FALSE);
			return;
		}
		m_wiWave.dwNextWriteOffset = 0;
		fRefillLostBuffer = TRUE;
	}


	m_wiWave.lpDSBStreamBuffer->GetCurrentPosition(&dwPlay, &dwWrite);







	if(dwPlay == m_wiWave.dwNextWriteOffset && !fRefillLostBuffer)
	{
		InterlockedExchange(&m_lInTimer, FALSE);
		return;
	}

	if(m_wiWave.bFoundEnd && !m_wiWave.dwBytesRemaining)
	{
		if(!m_wiWave.bDonePlaying)
		{
			m_wiWave.bDonePlaying = TRUE;
			LSStopBackground(0L);
		}
		InterlockedExchange(&m_lInTimer, FALSE);
		return;
	}

	if(dwPlay < m_wiWave.dwNextWriteOffset)
	{

		dwPlayedLength = (dwPlay + m_wiWave.dwBufferSize - m_wiWave.dwNextWriteOffset);
	}
	else
	{

		dwPlayedLength = (dwPlay - m_wiWave.dwNextWriteOffset);
	}




	if(fRefillLostBuffer)
	{
		dwWriteLength = 2 * m_wiWave.dwBufferSize / NUM_BUFFER_SEGMENTS;
	}
	else
	{
		dwWriteLength = dwPlayedLength;
	}

	m_wiWave.dwProgress += dwPlayedLength;




	if(m_wiWave.bFoundEnd && m_wiWave.dwBytesRemaining)
	{


		if(dwPlayedLength > m_wiWave.dwBytesRemaining)
		{
			m_wiWave.dwBytesRemaining = 0;
		}
		else
		{
			m_wiWave.dwBytesRemaining -= dwPlayedLength;
		}
		if(m_wiWave.lpDSBStreamBuffer->Lock(m_wiWave.dwNextWriteOffset, dwWriteLength,

			(LPVOID *)&lpWrite1,
											&dwLen1,

											(LPVOID *)&lpWrite2,

											&dwLen2, 0) != 0)
		{
			OutputDebugString("LSTimeRunFuncForDSound() could not lock DirectSoundBuffer");
			InterlockedExchange(&m_lInTimer, FALSE);
			return;
		}


		memset(lpWrite1, m_wiWave.pwfx->wBitsPerSample == 8 ? 128 : 0, dwLen1);

		if(lpWrite2 && dwLen2)
		{
			memset(lpWrite2, m_wiWave.pwfx->wBitsPerSample == 8 ? 128 : 0, dwLen2);
		}
		m_wiWave.lpDSBStreamBuffer->Unlock((LPVOID)lpWrite1, dwLen1, (LPVOID)lpWrite2, dwLen2);



		m_wiWave.dwNextWriteOffset += dwWriteLength;
		if(m_wiWave.dwNextWriteOffset >= m_wiWave.dwBufferSize)
		{
			m_wiWave.dwNextWriteOffset -= m_wiWave.dwBufferSize;
		}
		InterlockedExchange(&m_lInTimer, FALSE);
		return;
	}


	if(m_wiWave.lpDSBStreamBuffer->Lock(m_wiWave.dwNextWriteOffset, dwWriteLength,

		(LPVOID *)&lpWrite1,
										&dwLen1,

										(LPVOID *)&lpWrite2,
										&dwLen2, 0) != 0)
	{
		OutputDebugString("LSTimeRunFuncForDSound() could not lock DirectSoundBuffer");
		InterlockedExchange(&m_lInTimer, FALSE);
		return;
	}

	if(dwLen1 && !m_wiWave.bDonePlaying)
	{
		WaveReadFile(m_wiWave.hmmio, (UINT)dwLen1, lpWrite1, &m_wiWave.mmck, &uChkErr);
		if(uChkErr < (UINT)dwLen1)
		{
			if(!m_wiWave.bLoopFile)
			{

				if(m_wiWave.pwfx->wBitsPerSample == 8)
				{
					memset(lpWrite1 + uChkErr, 128, ((UINT)dwLen1 - uChkErr));
				}
				else if(m_wiWave.pwfx->wBitsPerSample == 16)
				{
					memset(lpWrite1 + uChkErr, 0, ((UINT)dwLen1 - uChkErr));
				}


				m_wiWave.bFoundEnd = TRUE;
				if(dwPlay > m_wiWave.dwNextWriteOffset)
				{
					m_wiWave.dwBytesRemaining = (m_wiWave.dwNextWriteOffset + m_wiWave.dwBufferSize - dwPlay);
				}
				else
				{
					m_wiWave.dwBytesRemaining = (m_wiWave.dwNextWriteOffset - dwPlay);
				}
			}
			else
			{
				lpTemp = lpWrite1;
				dwLeftToRead = dwLen1;
				do
				{





					lpTemp += uChkErr;
					dwLeftToRead -= uChkErr;
					WaveStartDataRead(&m_wiWave.hmmio, &m_wiWave.mmck, &m_wiWave.mmckInRIFF);
					WaveReadFile(m_wiWave.hmmio, (UINT)dwLeftToRead, lpTemp, &m_wiWave.mmck, &uChkErr);
				} while(uChkErr < dwLeftToRead);
			}
		}
	}




	else if(dwLen1 && m_wiWave.bDonePlaying)
	{

		_fmemset(lpWrite1, m_wiWave.pwfx->wBitsPerSample == 8 ? 128 : 0, dwLen1);
	}

	if(dwLen2 && !m_wiWave.bDonePlaying)
	{
		WaveReadFile(m_wiWave.hmmio, (UINT)dwLen2, lpWrite2, &m_wiWave.mmck, &uChkErr);
		if(uChkErr < (UINT)dwLen2)
		{
			if(!m_wiWave.bLoopFile)
			{

				if(m_wiWave.pwfx->wBitsPerSample == 8)
				{
					memset(lpWrite2 + uChkErr, 128, ((UINT)dwLen2 - uChkErr));
				}
				else if(m_wiWave.pwfx->wBitsPerSample == 16)
				{
					memset(lpWrite2 + uChkErr, 0, ((UINT)dwLen2 - uChkErr));
				}


				m_wiWave.bFoundEnd = TRUE;
				if(dwPlay > m_wiWave.dwNextWriteOffset)
				{
					m_wiWave.dwBytesRemaining = (m_wiWave.dwNextWriteOffset + m_wiWave.dwBufferSize - dwPlay);
				}
				else
				{
					m_wiWave.dwBytesRemaining = (m_wiWave.dwNextWriteOffset - dwPlay);
				}
			}
			else
			{
				lpTemp = lpWrite2;
				dwLeftToRead = dwLen2;
				do
				{





					lpTemp += uChkErr;
					dwLeftToRead -= uChkErr;
					WaveStartDataRead(&m_wiWave.hmmio, &m_wiWave.mmck, &m_wiWave.mmckInRIFF);
					WaveReadFile(m_wiWave.hmmio, (UINT)dwLeftToRead, lpTemp, &m_wiWave.mmck, &uChkErr);
				} while(uChkErr < dwLeftToRead);
			}
		}
	}
	else if(lpWrite2 && dwLen2 && m_wiWave.bDonePlaying)
	{

		_fmemset(lpWrite2, m_wiWave.pwfx->wBitsPerSample == 8 ? 128 : 0, dwLen2);
	}
	m_wiWave.lpDSBStreamBuffer->Unlock((LPVOID)lpWrite1, dwLen1, (LPVOID)lpWrite2, dwLen2);
	m_wiWave.dwNextWriteOffset += dwWriteLength;
	if(m_wiWave.dwNextWriteOffset >= m_wiWave.dwBufferSize)
	{
		m_wiWave.dwNextWriteOffset -= m_wiWave.dwBufferSize;
	}
	if(fRefillLostBuffer)
	{
		m_wiWave.lpDSBStreamBuffer->Play(0, 0, DSBPLAY_LOOPING);
	}
	InterlockedExchange(&m_lInTimer, FALSE);

	return;
}









BOOL OpenBGM(LPSTR lpszFileName)
{
	summit::diagnostics::RuntimeAssetProbeAttempt("AUDIO_BGM", "OPEN", lpszFileName);
	if(LSEnable == TRUE)
	{
		if(m_bFileOpen)
		{
			m_bIsCloseOnRun = FALSE;

			LSStopBackground(DSSTREAM_STOPF_NOREOPEN);
		}
		if((SetupStreamBuffer(lpszFileName)) != DS_OK)
		{
			return FALSE;
		}
		else
		{
			m_bFileOpen = TRUE;
		}
	}
	return TRUE;
}

HRESULT SetupStreamBuffer(LPSTR lpszFilename)
{
	DSBUFFERDESC dsbd;
	HRESULT      dsRetVal;



	if(WaveOpenFile(lpszFilename, &m_wiWave.hmmio, &m_wiWave.pwfx, &m_wiWave.mmckInRIFF) != 0)
	{
		return ERR_WAVE_OPEN_FAILED;
	}

	if(m_wiWave.pwfx->wFormatTag != WAVE_FORMAT_PCM)
	{
		WaveCloseReadFile(&m_wiWave.hmmio, &m_wiWave.pwfx);
		return ERR_WAVE_INVALID_FORMAT;
	}

	if(WaveStartDataRead(&m_wiWave.hmmio, &m_wiWave.mmck, &m_wiWave.mmckInRIFF) != 0)
	{
		WaveCloseReadFile(&m_wiWave.hmmio, &m_wiWave.pwfx);
		return ERR_WAVE_CORRUPTED_FILE;
	}





	m_wiWave.dwBufferSize = ((DWORD)m_wiWave.pwfx->nAvgBytesPerSec * (((NUM_BUFFER_SEGMENTS * PLAYBACK_TIMER_PERIOD) / 10)) / 100);


	memset(&dsbd, 0, sizeof(DSBUFFERDESC));
	dsbd.dwSize = sizeof(DSBUFFERDESC);
	dsbd.dwFlags = DSBCAPS_CTRLPAN | DSBCAPS_CTRLVOLUME | DSBCAPS_CTRLFREQUENCY | DSBCAPS_GETCURRENTPOSITION2 | m_SecondBuffFocus;
	dsbd.dwBufferBytes = m_wiWave.dwBufferSize;
	dsbd.lpwfxFormat = m_wiWave.pwfx;




	dsRetVal = lpDirectSound->CreateSoundBuffer(&dsbd, &lpDSBStreamBuffer, NULL);

	if(dsRetVal != DS_OK)
	{
		return ERR_CREATEDSB_FAILED;
	}

	m_wiWave.lpDSBStreamBuffer = lpDSBStreamBuffer;
	m_wiWave.bFoundEnd = FALSE;
	m_wiWave.dwBytesRemaining = 0;
	m_wiWave.bLoopFile = TRUE;

	LSFillDataBuffer();

	m_wiWave.bDonePlaying = FALSE;

	if(lpDSBStreamBuffer)
	{
		lpDSBStreamBuffer->SetVolume(SETTINGSHANDLER->getSoundVolume() ? (DSBVOLUME_MIN / 2) + ((abs(DSBVOLUME_MIN / 2) * SETTINGSHANDLER->getSoundVolume()) / 100) : DSBVOLUME_MIN);
		lpDSBStreamBuffer->SetPan(((((PAN_MAX / 2) + PAN_SHIFT) * PAN_MULT)));
		lpDSBStreamBuffer->SetFrequency(m_wiWave.pwfx->nSamplesPerSec);
	}
	return DS_OK;
}


void PlayBGM(void)
{
	
	if(LSEnable == TRUE)
	{
		if(m_bPlaying)
		{
			m_bIsCloseOnRun = FALSE;
			LSStopBackground(0L);
		}
		if(m_bFileOpen && lpDSBStreamBuffer)
		{

			lpDSBStreamBuffer->SetCurrentPosition(0);
			lpDSBStreamBuffer->Play(0, 0, DSBPLAY_LOOPING);
		}
		else
		{
			m_bPlaying = FALSE;
			m_bTimerInit = FALSE;
			return;
		}

		if(timeBeginPeriod(PLAYBACK_TIMER_PERIOD / PLAYBACK_OVERSAMPLE) != 0)
		{

			lpDSBStreamBuffer->Stop();
			m_bPlaying = FALSE;
			m_bTimerInit = FALSE;
			return;
		}
		else
		{
			m_lInTimer = FALSE;
			if((m_uTimerID = timeSetEvent(PLAYBACK_TIMER_PERIOD / PLAYBACK_OVERSAMPLE,
										  PLAYBACK_TIMER_ACCURACY, LSTimeRunFuncForDSound,
										  (DWORD)0, TIME_PERIODIC)) != 0)
			{
				m_bTimerInit = TRUE;
			}
		}
		m_bPlaying = TRUE;
	}
}

void StopBGM(void)
{
	if(LSEnable == TRUE && m_bPlaying)
		LSStopBackground(0);
}

void SetVolumeBGM(DWORD dwVol)
{
	if(LSEnable == TRUE && m_bPlaying)
	{
		if (SETTINGSHANDLER->getSoundVolume())
			lpDSBStreamBuffer->SetVolume((DSBVOLUME_MIN / 2) + ((abs(DSBVOLUME_MIN / 2) * SETTINGSHANDLER->getSoundVolume()) / 100));
		else
			lpDSBStreamBuffer->SetVolume(DSBVOLUME_MIN);
	}
}

// Compatibility overload for Settings V2.  The settings window originally
// declared SetVolumeBGM(int), while the legacy audio implementation exports
// SetVolumeBGM(DWORD).  Keep both symbols linkable and forward to the canonical
// DWORD implementation.
void SetVolumeBGM(int dwVol)
{
	SetVolumeBGM(static_cast<DWORD>(dwVol));
}

BOOL Init_BGM(HWND hwnd)
{
	m_hWnd = hwnd;

	lpDirectSound = NULL;

	lpDSBStreamBuffer = NULL;

	m_lInTimer = FALSE;
	m_bTimerInit = FALSE;
	m_bPlaying = FALSE;
	m_bIsCloseOnRun = FALSE;
	m_bFileOpen = FALSE;
	m_uTimerID = 0;
	m_SecondBuffFocus = 0;

	HRESULT         dsrval;

	dsrval = DirectSoundCreate(NULL, &lpDirectSound, NULL);

	if(SUCCEEDED(dsrval) && (NULL != lpDirectSound))
	{



		dsrval = lpDirectSound->SetCooperativeLevel(hwnd, DSSCL_PRIORITY);




		if(!SUCCEEDED(dsrval))
		{
			dsrval = FALSE;
			lpDirectSound->Release();
			lpDirectSound = NULL;
		}
	}

	else
	{


	}


	if(SUCCEEDED(dsrval))
	{
		LSEnable = TRUE;
		return TRUE;
	}
	else
	{

		LSEnable = FALSE;
		return FALSE;
	}




	LSEnable = TRUE;

	return TRUE;
}


void Uninit_BGM(void)
{
	if(LSEnable == TRUE)
	{

		LSStopBackground(DSSTREAM_STOPF_NOREOPEN | DSSTREAM_STOPF_NOEXIT);


		if(m_bTimerInit)
		{
			timeKillEvent(m_uTimerID);
			timeEndPeriod(PLAYBACK_TIMER_PERIOD / PLAYBACK_OVERSAMPLE);

			while(InterlockedExchange(&m_lInTimer, TRUE)) Sleep(100);
			m_bTimerInit = FALSE;
		}


		if(lpDSBStreamBuffer)
		{
			lpDSBStreamBuffer->Stop();
			lpDSBStreamBuffer->Release();
			lpDSBStreamBuffer = NULL;
		}



		if(lpDirectSound != NULL)
		{
			lpDirectSound->Release();
			lpDirectSound = NULL;

		}


	}
}


void InitDirectSound()
{
	lpDirectSound = NULL;

	rval_g_lpDs = DirectSoundCreate(NULL, &g_lpDS, NULL);

	dsrval_lpDirectSound = DirectSoundCreate(NULL, &lpDirectSound, NULL);
}








