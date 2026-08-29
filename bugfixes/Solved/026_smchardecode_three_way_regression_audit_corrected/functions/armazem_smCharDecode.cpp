int smCharDecode( char *szCharFile , smCHAR_INFO *lpCharInfo , smCHAR_MONSTER_INFO	*lpMonInfo , char *lpDialogMessage )
{

	FILE	*fp;
	//char	*szDirecotry;
	char strBuff[256];
	char szFile[128];
	char *p;
	char *pb;
	int NextFlag;
	int cnt;
	int	percent;
	int	DialogFlag;

	DWORD	dwItemCode[32];
	int		ItemCodeCnt;


	lstrcpy( szFile , szCharFile );

	fp = fopen( szFile , "rb" );
	if ( fp==NULL ) return FALSE;

	//파일 이름으로 부터 디렉토리를 찾아 설정
	//szDirecotry = SetDirectoryFromFile( szCharFile );

	lpCharInfo->szModelName2[0] = 0;
	lpCharInfo->wPlayClass[1] = 0;
	lpCharInfo->ArrowPosi[0] = 0;
	lpCharInfo->ArrowPosi[1] = 0;

	if ( lpMonInfo ) {
		ZeroMemory( lpMonInfo , sizeof(smCHAR_MONSTER_INFO) );
		lpMonInfo->MoveRange = 64*fONE;
		lpMonInfo->FallItemCount = 0;
		lpMonInfo->FallItemMax = 0;
		lpMonInfo->FallItemPerMax = 0;

		lpMonInfo->DamageStunPers = 100;			//기본 스턴률 100%
		lpMonInfo->UseEventModel = 0;
	}

	while( !feof( fp ) )//  feof: file end까지 읽어라 
	{
		if( fgets( line, 255, fp ) == NULL)	break;

		p = GetWord( decode , line);

		NextFlag = 0;

		///////////////// 이름 ///////////////////////
		if ( lstrcmp( decode , "*이름" )==0 ) {
			pb=p;p=GetWord(strBuff,p);if(strBuff[0]==34)p=GetString(strBuff,pb);	//단어 찾기

			lstrcpy( lpCharInfo->szName , strBuff );
			NextFlag = TRUE;
		}

#ifdef	_LANGUAGE_CHINESE
		if ( lstrcmpi( decode , "*C_NAME" )==0 ) {
			pb=p;p=GetWord(strBuff,p);if(strBuff[0]==34)p=GetString(strBuff,pb);	//단어 찾기

			if ( lpMonInfo )
				lstrcpy( lpMonInfo->szName , strBuff );
			else
				lstrcpy( lpCharInfo->szName , strBuff );

			NextFlag = TRUE;
		}
#endif
#ifdef	_LANGUAGE_JAPANESE
		if ( lstrcmpi( decode , "*J_NAME" )==0 ) {
			pb=p;p=GetWord(strBuff,p);if(strBuff[0]==34)p=GetString(strBuff,pb);	//단어 찾기

			if ( lpMonInfo )
				lstrcpy( lpMonInfo->szName , strBuff );
			else
				lstrcpy( lpCharInfo->szName , strBuff );

			NextFlag = TRUE;
		}
#endif
#ifdef	_LANGUAGE_TAIWAN
		if ( lstrcmpi( decode , "*T_NAME" )==0 ) {
			pb=p;p=GetWord(strBuff,p);if(strBuff[0]==34)p=GetString(strBuff,pb);	//단어 찾기

			if ( lpMonInfo )
				lstrcpy( lpMonInfo->szName , strBuff );
			else
				lstrcpy( lpCharInfo->szName , strBuff );

			NextFlag = TRUE;
		}
#endif
#ifdef	_LANGUAGE_ENGLISH
		if ( lstrcmpi( decode , "*J_NAME" )==0 ) {
			pb=p;p=GetWord(strBuff,p);if(strBuff[0]==34)p=GetString(strBuff,pb);	//단어 찾기

			if ( lpMonInfo )
				lstrcpy( lpMonInfo->szName , strBuff );
			else
				lstrcpy( lpCharInfo->szName , strBuff );

			NextFlag = TRUE;
		}
#endif
#ifdef	_LANGUAGE_THAI
		if ( lstrcmpi( decode , "*TH_NAME" )==0 ) {
			pb=p;p=GetWord(strBuff,p);if(strBuff[0]==34)p=GetString(strBuff,pb);	//단어 찾기

			if ( lpMonInfo )
				lstrcpy( lpMonInfo->szName , strBuff );
			else
				lstrcpy( lpCharInfo->szName , strBuff );

			NextFlag = TRUE;
		}
#endif
#ifdef _LANGUAGE_VEITNAM
		if ( lstrcmpi( decode , "*V_NAME" )==0 ) {
			pb=p;p=GetWord(strBuff,p);if(strBuff[0]==34)p=GetString(strBuff,pb);	//단어 찾기

			if ( lpMonInfo )
				lstrcpy( lpMonInfo->szName , strBuff );
			else
				lstrcpy( lpCharInfo->szName , strBuff );

			NextFlag = TRUE;
		}
#endif
#ifdef _LANGUAGE_BRAZIL
		if ( lstrcmpi( decode , "*B_NAME" )==0 ) {
			pb=p;p=GetWord(strBuff,p);if(strBuff[0]==34)p=GetString(strBuff,pb);	//단어 찾기

			if ( lpMonInfo )
				lstrcpy( lpMonInfo->szName , strBuff );
			else
				lstrcpy( lpCharInfo->szName , strBuff );

			NextFlag = TRUE;
		}
#endif
#ifdef _LANGUAGE_ARGENTINA
		if ( lstrcmpi( decode , "*A_NAME" )==0 ) {
			pb=p;
			p=GetWord(strBuff,p);
			if(strBuff[0]==34)
			{
				p=GetString(strBuff,pb);	//단어 찾기
			}

			if ( lpMonInfo )
				lstrcpy( lpMonInfo->szName , strBuff );
			else
				lstrcpy( lpCharInfo->szName , strBuff );

			NextFlag = TRUE;
		}
#endif

		///////////////// 모양파일 ///////////////////////
		if ( !NextFlag && lstrcmp( decode , "*모양파일" )==0 ) {
			pb=p;p=GetWord(strBuff,p);if(strBuff[0]==34)p=GetString(strBuff,pb);	//단어 찾기

			//lstrcpy( lpCharInfo->szModelName, szDirecotry );
			//lstrcat( lpCharInfo->szModelName, strBuff );
			lstrcpy( lpCharInfo->szModelName, strBuff );
			NextFlag = TRUE;
		}

		///////////////// *대화 ///////////////////////

		DialogFlag = 0;

		if ( !NextFlag && lpDialogMessage && lstrcmp( decode , "*대화" )==0 ) {
			pb=p;p=GetWord(strBuff,p);if(strBuff[0]==34)p=GetString(strBuff,pb);	//단어 찾기

			lstrcpy( lpDialogMessage , strBuff );
			NextFlag = TRUE;
			DialogFlag++;
		}

#ifdef _LANGUAGE_CHINESE
		if ( lpDialogMessage && lstrcmpi( decode , "*C_CHAT" )==0 ) {
			pb=p;p=GetWord(strBuff,p);if(strBuff[0]==34)p=GetString(strBuff,pb);	//단어 찾기

			lstrcpy( lpDialogMessage , strBuff );
			NextFlag = TRUE;
			DialogFlag++;
		}
#endif

#ifdef _LANGUAGE_JAPANESE
		if ( lpDialogMessage && lstrcmpi( decode , "*J_CHAT" )==0 ) {
			pb=p;p=GetWord(strBuff,p);if(strBuff[0]==34)p=GetString(strBuff,pb);	//단어 찾기

			lstrcpy( lpDialogMessage , strBuff );
			NextFlag = TRUE;
			DialogFlag++;
		}
#endif

#ifdef _LANGUAGE_TAIWAN
		if ( lpDialogMessage && lstrcmpi( decode , "*T_CHAT" )==0 ) {
			pb=p;p=GetWord(strBuff,p);if(strBuff[0]==34)p=GetString(strBuff,pb);	//단어 찾기

			lstrcpy( lpDialogMessage , strBuff );
			NextFlag = TRUE;
			DialogFlag++;
		}
#endif
#ifdef _LANGUAGE_ENGLISH
		if ( lpDialogMessage && lstrcmpi( decode , "*E_CHAT" )==0 ) {
			pb=p;p=GetWord(strBuff,p);if(strBuff[0]==34)p=GetString(strBuff,pb);	//단어 찾기

			lstrcpy( lpDialogMessage , strBuff );
			NextFlag = TRUE;
			DialogFlag++;
		}
#endif
#ifdef _LANGUAGE_THAI
		if ( lpDialogMessage && lstrcmpi( decode , "*TH_CHAT" )==0 ) {
			pb=p;p=GetWord(strBuff,p);if(strBuff[0]==34)p=GetString(strBuff,pb);	//단어 찾기

			lstrcpy( lpDialogMessage , strBuff );
			NextFlag = TRUE;
			DialogFlag++;
		}
#endif
#ifdef _LANGUAGE_VEITNAM
		if ( lpDialogMessage && lstrcmpi( decode , "*V_CHAT" )==0 ) {
			pb=p;p=GetWord(strBuff,p);if(strBuff[0]==34)p=GetString(strBuff,pb);	//단어 찾기

			lstrcpy( lpDialogMessage , strBuff );
			NextFlag = TRUE;
			DialogFlag++;
		}
#endif
#ifdef _LANGUAGE_BRAZIL
		if ( lpDialogMessage && lstrcmpi( decode , "*B_CHAT" )==0 ) {
			pb=p;p=GetWord(strBuff,p);if(strBuff[0]==34)p=GetString(strBuff,pb);	//단어 찾기

			lstrcpy( lpDialogMessage , strBuff );
			NextFlag = TRUE;
			DialogFlag++;
		}
#endif
#ifdef _LANGUAGE_ARGENTINA
		if ( lpDialogMessage && lstrcmpi( decode , "*A_CHAT" )==0 ) {
			pb=p;p=GetWord(strBuff,p);if(strBuff[0]==34)p=GetString(strBuff,pb);	//단어 찾기

			lstrcpy( lpDialogMessage , strBuff );
			NextFlag = TRUE;
			DialogFlag++;
		}
#endif
		//대사목록 저장
		if ( DialogFlag && lpMonInfo ) {
			if ( lpMonInfo->NpcMsgCount<NPC_MESSAGE_MAX ) {
/*
#ifdef _LANGUAGE_ARGENTINA		//줄바꿈 kyle
				
				char szMessage[512];
				unsigned int iCount = 0;
				unsigned int nMessageSize = 0;

				ZeroMemory(szMessage, sizeof(szMessage));

				nMessageSize = strlen(lpDialogMessage); //npc 대사내용 길이

				for(unsigned int i=0;i<=nMessageSize;i++)
				{
					int y = 0;
					if( (i!= 0) && (iCount % 35) == 0 )
					{
						// 연결된 문자열 사이의 공백 체크
						if( lpDialogMessage[i-1] != 32	&&		
							lpDialogMessage[i] != 32	&&
							lpDialogMessage[i] != 46	)     // 두번째 문자열이 스페이스거나 '.' 이 아닐경우
						{
							szMessage[iCount] = '-';
							iCount++;
							szMessage[iCount] = ' ';//테스트중
							iCount++;
							szMessage[iCount] = ' ';//테스트중
							iCount++;
							szMessage[iCount] = lpDialogMessage[i];
						}
						else
						{
							szMessage[iCount] = lpDialogMessage[i];
						}
					}
					else
					{
						szMessage[iCount] = lpDialogMessage[i];
					}

					iCount++;	
				}

				lpMonInfo->lpNpcMessage[lpMonInfo->NpcMsgCount++] = SetNPCMsg(szMessage);
#else*/
				lpMonInfo->lpNpcMessage[lpMonInfo->NpcMsgCount++] = SetNPCMsg(lpDialogMessage);
//#endif

				
			}
		}

		///////////////// *속성 ///////////////////////
		if ( !NextFlag && lstrcmp( decode , "*속성" )==0 ) {
			pb=p;p=GetWord(strBuff,p);if(strBuff[0]==34)p=GetString(strBuff,pb);	//단어 찾기

			if ( lstrcmp( strBuff , "적" )==0 )
				lpCharInfo->State = TRUE;
			else
				lpCharInfo->State = FALSE;

			NextFlag = TRUE;
		}

		///////////////// *크기 ///////////////////////
		if ( !NextFlag && lstrcmp( decode , "*크기" )==0 ) {
			p=GetWord(strBuff,p);
			cnt = 0;

			lpCharInfo->SizeLevel = -1;

			while(1) {
				if ( szCharSizeCodeName[cnt][0]==0 ) break;
				if ( lstrcmp( szCharSizeCodeName[cnt], strBuff )==0 ) {
					lpCharInfo->SizeLevel = cnt;
				}
				cnt++;
			}
			NextFlag = TRUE;
		}

		///////////////// *소리 ///////////////////////
		if ( !NextFlag && ( lstrcmp( decode , "*소리" )==0 || lstrcmp( decode , "*효과음" )==0 ) ) {
			p=GetWord(strBuff,p);
			cnt = 0;

			while(1) {
				if ( dwCharSoundCode[cnt].dwCode==0 ) break;
				if ( lstrcmpi( dwCharSoundCode[cnt].szCodeName , strBuff )==0 ) {
					lpCharInfo->dwCharSoundCode = dwCharSoundCode[cnt].dwCode;
				}
				cnt++;
			}
			NextFlag = TRUE;
		}


		///////////////// *레벨 ///////////////////////
		if ( !NextFlag && lstrcmp( decode , "*레벨" )==0 ) {
			p=GetWord(strBuff,p);
			lpCharInfo->Level = atoi( strBuff );
			NextFlag = TRUE;
		}

		///////////////// *계급 ///////////////////////
		if ( !NextFlag && lstrcmp( decode , "*두목" )==0 ) {
			//p=GetWord(strBuff,p);
			lpCharInfo->wPlayClass[0] = MONSTER_CLASS_BOSS;
			NextFlag = TRUE;
		}

		if ( !NextFlag && lstrcmp( decode , "*계급" )==0 ) {
			p=GetWord(strBuff,p);
			lpCharInfo->wPlayClass[0] = atoi(strBuff);
			NextFlag = TRUE;
		}

		/////////////// *계급 크기 /////////////////////
		if ( !NextFlag && lstrcmp( decode , "*모델크기" )==0 ) {
			p=GetWord(strBuff,p);
			lpCharInfo->wPlayClass[1] = (short)(atof(strBuff)*fONE);
			if ( lpCharInfo->wPlayClass[1]==fONE ) lpCharInfo->wPlayClass[1]=0;
			NextFlag = TRUE;
		}


		///////////////// *이동속도 ///////////////////////
		if ( !NextFlag && lstrcmp( decode , "*이동력" )==0 ) {
			p=GetWord(strBuff,p);
			lpCharInfo->Move_Speed = ConvMoveSpeed( (float)atof( strBuff ) ); 
			NextFlag = TRUE;
		}

		///////////////// *공격력 ///////////////////////
		if ( !NextFlag && lstrcmp( decode , "*공격력" )==0 ) {
			p=GetWord(strBuff,p);
			lpCharInfo->Attack_Damage[0] = atoi(strBuff);
			p=GetWord(strBuff,p);
			lpCharInfo->Attack_Damage[1] = atoi(strBuff);

			NextFlag = TRUE;
		}

		///////////////// *공격속도 ///////////////////////
		if ( !NextFlag && lstrcmp( decode , "*공격속도" )==0 ) {
			p=GetWord(strBuff,p);
			lpCharInfo->Attack_Speed = (int)( atof(strBuff)*fONE );
			p=GetWord(strBuff,p);
			lpCharInfo->Attack_Speed = (int)( atof(strBuff)*fONE );

			NextFlag = TRUE;
		}

		///////////////// *공격범위 ///////////////////////
		if ( !NextFlag && lstrcmp( decode , "*공격범위" )==0 ) {
			p=GetWord(strBuff,p);
			lpCharInfo->Shooting_Range = atoi(strBuff)*fONE;
			NextFlag = TRUE;
		}



		///////////////// *명중률 ///////////////////////
		if ( !NextFlag && lstrcmp( decode , "*명중력" )==0 ) {
			p=GetWord(strBuff,p);
			lpCharInfo->Attack_Rating = atoi(strBuff);

			NextFlag = TRUE;
		}
	
		///////////////// *방어력 ///////////////////////
		if ( !NextFlag && lstrcmp( decode , "*방어력" )==0 ) {
			p=GetWord(strBuff,p);
			lpCharInfo->Defence = atoi(strBuff);

			NextFlag = TRUE;
		}
		///////////////// *흡수율 ///////////////////////
		if ( !NextFlag && lstrcmp( decode , "*흡수율" )==0 ) {
			p=GetWord(strBuff,p);
			lpCharInfo->Absorption = atoi(strBuff);

			NextFlag = TRUE;
		}
		///////////////// *블럭율 ///////////////////////
		if ( !NextFlag && lstrcmp( decode , "*블럭율" )==0 ) {
			p=GetWord(strBuff,p);
			lpCharInfo->Chance_Block = atoi(strBuff);

			NextFlag = TRUE;
		}
	
		///////////////// *라이프 ///////////////////////
		if ( !NextFlag && ( lstrcmp( decode , "*생명력" )==0 || lstrcmp( decode , "*라이프" )==0 ) ) {
			p=GetWord(strBuff,p);
			lpCharInfo->Life[0] = atoi(strBuff);
			lpCharInfo->Life[1] = lpCharInfo->Life[0];

			NextFlag = TRUE;
		}
	
		///////////////// *생체 ///////////////////////
		if ( !NextFlag && lstrcmp( decode , "*생체" )==0 ) {
			p=GetWord(strBuff,p);
			lpCharInfo->Resistance[sITEMINFO_BIONIC] = (short)atoi(strBuff);
			NextFlag = TRUE;
		}
		///////////////// *물 ///////////////////////
		if ( !NextFlag && lstrcmp( decode , "*물" )==0 ) {
			p=GetWord(strBuff,p);
			lpCharInfo->Resistance[sITEMINFO_WATER] = (short)atoi(strBuff);
			NextFlag = TRUE;
		}
		///////////////// *번개 ///////////////////////
		if ( !NextFlag && lstrcmp( decode , "*번개" )==0 ) {
			p=GetWord(strBuff,p);
			lpCharInfo->Resistance[sITEMINFO_LIGHTING] = (short)atoi(strBuff);

			NextFlag = TRUE;
		}
		///////////////// *얼음 ///////////////////////
		if ( !NextFlag && lstrcmp( decode , "*얼음" )==0 ) {
			p=GetWord(strBuff,p);
			lpCharInfo->Resistance[sITEMINFO_ICE] = (short)atoi(strBuff);
			NextFlag = TRUE;
		}
		///////////////// *바람 ///////////////////////
		if ( !NextFlag && lstrcmp( decode , "*바람" )==0 ) {
			p=GetWord(strBuff,p);
			lpCharInfo->Resistance[sITEMINFO_WIND] = (short)atoi(strBuff);
			NextFlag = TRUE;
		}
		///////////////// *지동력 ///////////////////////
		if ( !NextFlag && lstrcmp( decode , "*지동력" )==0 ) {
			p=GetWord(strBuff,p);
			lpCharInfo->Resistance[sITEMINFO_EARTH] = (short)atoi(strBuff);
			NextFlag = TRUE;
		}
		///////////////// *불 ///////////////////////
		if ( !NextFlag && lstrcmp( decode , "*불" )==0 ) {
			p=GetWord(strBuff,p);
			lpCharInfo->Resistance[sITEMINFO_FIRE] = (short)atoi(strBuff);
			NextFlag = TRUE;
		}
		///////////////// *독 ///////////////////////
		if ( !NextFlag && lstrcmp( decode , "*독" )==0 ) {
			p=GetWord(strBuff,p);
			lpCharInfo->Resistance[sITEMINFO_POISON] = (short)atof(strBuff);
			NextFlag = TRUE;
		}





//////////////////////////////// 추가분 //////////////////////////////////

		///////////////// *시야 ///////////////////////
		if ( !NextFlag && lstrcmp( decode , "*시야" )==0 ) {
			p=GetWord(strBuff,p);
			lpCharInfo->Sight = atoi( strBuff );// * fONE;
			if ( lpMonInfo ) lpMonInfo->Real_Sight = lpCharInfo->Sight;

			lpCharInfo->Sight *= lpCharInfo->Sight;			//몬스터 시야 제곱값
			NextFlag = TRUE;
		}

		///////////////// *미니화면 보정 ///////////////////////
		if ( !NextFlag && lstrcmp( decode , "*화면보정" )==0 ) {
			p=GetWord(strBuff,p);
			if ( strBuff[0] )
				lpCharInfo->ArrowPosi[0] = atoi( strBuff );

			p=GetWord(strBuff,p);
			if ( strBuff[0] )
				lpCharInfo->ArrowPosi[1] = atoi( strBuff );

		}

		if ( lpMonInfo && !NextFlag ) {

			///////////////// *예비모델 ///////////////////////
			if ( !NextFlag && lstrcmp( decode , "*예비모델" )==0 ) {
				pb=p;p=GetWord(strBuff,p);if(strBuff[0]==34)p=GetString(strBuff,pb);	//단어 찾기
				lstrcpy( lpCharInfo->szModelName2+1, strBuff ); 
				lpMonInfo->UseEventModel = TRUE;
				NextFlag = TRUE;
			}

			///////////////// *기술공격력 ///////////////////////
			if ( !NextFlag && lstrcmp( decode , "*기술공격력" )==0 ) {
				p=GetWord(strBuff,p);
				lpMonInfo->SkillDamage[0] = atoi(strBuff);
				p=GetWord(strBuff,p);
				lpMonInfo->SkillDamage[1] = atoi(strBuff);

				NextFlag = TRUE;
			}

			///////////////// *기술공격거리 ///////////////////////
			if ( !NextFlag && lstrcmp( decode , "*기술공격거리" )==0 ) {
				p=GetWord(strBuff,p);
				lpMonInfo->SkillDistance = atoi(strBuff);

				NextFlag = TRUE;
			}


			///////////////// *기술공격범위 ///////////////////////
			if ( !NextFlag && lstrcmp( decode , "*기술공격범위" )==0 ) {
				p=GetWord(strBuff,p);
				lpMonInfo->SkillRange = atoi(strBuff);

				NextFlag = TRUE;
			}

			///////////////// *기술공격률 ///////////////////////
			if ( !NextFlag && lstrcmp( decode , "*기술공격률" )==0 ) {
				p=GetWord(strBuff,p);
				lpMonInfo->SkillRating = atoi(strBuff);

				NextFlag = TRUE;
			}

			///////////////// *저주기술설정 ///////////////////////
			if ( !NextFlag && lstrcmp( decode , "*저주기술" )==0 ) {
				p=GetWord(strBuff,p);
				lpMonInfo->SkillCurse = atoi(strBuff);
				NextFlag = TRUE;
			}

			///////////////// *이동타입 ///////////////////////
			if ( !NextFlag && lstrcmp( decode , "*이동타입" )==0 ) {
				p=GetWord(strBuff,p);

				NextFlag = TRUE;
			}

			///////////////// *공격속도 ///////////////////////
			if ( !NextFlag && lstrcmp( decode , "*이동범위" )==0 ) {
				p=GetWord(strBuff,p);
				lpMonInfo->MoveRange = (int)( atof(strBuff)*fONE );
				NextFlag = TRUE;
			}


			///////////////// *활동시간 ///////////////////////
			if ( !NextFlag && lstrcmp( decode , "*활동시간" )==0 ) {
				p=GetWord(strBuff,p);
				lpMonInfo->ActiveHour = 0;
				if ( lstrcmpi( strBuff , "낮" )==0 )
					lpMonInfo->ActiveHour = 1;
				if ( lstrcmpi( strBuff , "밤" )==0 )
					lpMonInfo->ActiveHour = -1;
				NextFlag = TRUE;
			}

			///////////////// *조직 ///////////////////////
			if ( !NextFlag && lstrcmp( decode , "*조직" )==0 ) {
				p=GetWord(strBuff,p);
				lpMonInfo->GenerateGroup[0] = atoi( strBuff );
				p=GetWord(strBuff,p);
				lpMonInfo->GenerateGroup[1] = atoi( strBuff );
				NextFlag = TRUE;
			}

			///////////////// *지능 ///////////////////////
			if ( !NextFlag && lstrcmp( decode , "*지능" )==0 ) {
				p=GetWord(strBuff,p);
				lpMonInfo->IQ = atoi( strBuff );
				NextFlag = TRUE;
			}

			///////////////// *구별 코드 //////////////////
			if ( !NextFlag && lstrcmp( decode , "*구별코드" )==0 ) {
				p=GetWord(strBuff,p);
				lpMonInfo->ClassCode = atoi( strBuff );
				NextFlag = TRUE;
			}

			///////////////// *스턴율 ///////////////////////
			if ( !NextFlag && ( lstrcmp( decode , "*스턴율" )==0 || lstrcmp( decode , "*스턴률" )==0 )) {
				p=GetWord(strBuff,p);
				lpMonInfo->DamageStunPers = atoi( strBuff );
				NextFlag = TRUE;
			}

			///////////////// *품성 ///////////////////////
			if ( !NextFlag && lstrcmp( decode , "*품성" )==0 ) {
				lpMonInfo->Nature = smCHAR_MONSTER_NATURAL;
				//lpCharInfo->JOB_CODE = smCHAR_MONSTER_NATURAL;
				
				p=GetWord(strBuff,p);
				if ( lstrcmpi( strBuff , "good" )==0 ) {
					lpMonInfo->Nature = smCHAR_MONSTER_GOOD;
					//lpCharInfo->JOB_CODE = smCHAR_MONSTER_GOOD;
				}
				if ( lstrcmpi( strBuff , "evil" )==0 ) {
					lpMonInfo->Nature = smCHAR_MONSTER_EVIL;
					//lpCharInfo->JOB_CODE = smCHAR_MONSTER_EVIL;
				}
				NextFlag = TRUE;
			}

			///////////////// *이벤트 ///////////////////////
			if ( !NextFlag && lstrcmp( decode , "*이벤트코드" )==0 ) {
				p=GetWord(strBuff,p);
	
				lpMonInfo->EventCode = atoi( strBuff );
				NextFlag = TRUE;
			}
			if ( !NextFlag && lstrcmp( decode , "*이벤트정보" )==0 ) {
				p=GetWord(strBuff,p);
	
				lpMonInfo->EventInfo = atoi( strBuff );
				NextFlag = TRUE;
			}
			if ( !NextFlag && lstrcmp( decode , "*이벤트아이템" )==0 ) {
				p=GetWord(strBuff,p);

				//코드검색 하여 설정
				for(cnt=0;cnt<MAX_ITEM;cnt++) {
					if ( lstrcmpi( strBuff , sItem[cnt].LastCategory )==0 ) {
						lpMonInfo->dwEvnetItem = sItem[cnt].CODE;
						break;
					}
				}
				NextFlag = TRUE;
			}


			///////////////// *특수공격률 ///////////////////////
			if ( !NextFlag && lstrcmp( decode , "*특수공격률" )==0 ) {
				p=GetWord(strBuff,p);
				lpMonInfo->SpAttackPercetage = ConvPercent8( atoi( strBuff ) );
				NextFlag = TRUE;
			}
			///////////////// *언데드 ///////////////////////
			if ( !NextFlag && lstrcmp( decode , "*언데드" )==0 ) {
				p=GetWord(strBuff,p);
				if ( lstrcmp( strBuff , "유" )==0 || lstrcmp( strBuff , "있음" )==0 ) {
					lpMonInfo->Undead = TRUE;
					lpCharInfo->Brood = smCHAR_MONSTER_UNDEAD;
				}
				else {
					lpMonInfo->Undead = FALSE;
					lpCharInfo->Brood = smCHAR_MONSTER_NORMAL;
				}

				NextFlag = TRUE;
			}

			if ( !NextFlag && lstrcmp( decode , "*몬스터종족" )==0 ) {
				p=GetWord(strBuff,p);
				if ( strBuff[0] ) {

					lpCharInfo->Brood = smCHAR_MONSTER_NORMAL;

					if ( lstrcmp( strBuff , "언데드" )==0 ) {
						lpCharInfo->Brood = smCHAR_MONSTER_UNDEAD;
					}
					else if ( lstrcmp( strBuff , "뮤턴트" )==0 ) {
						lpCharInfo->Brood = smCHAR_MONSTER_MUTANT;
					}
					else if ( lstrcmp( strBuff , "디먼" )==0 ) {
						lpCharInfo->Brood = smCHAR_MONSTER_DEMON;
					}
					else if ( lstrcmp( strBuff , "메카닉" )==0 ) {
						lpCharInfo->Brood = smCHAR_MONSTER_MECHANIC;
					}
				}
			}


			///////////////// *경험치 ///////////////////////
			if ( !NextFlag && lstrcmp( decode , "*경험치" )==0 ) {
				p=GetWord(strBuff,p);
				lpMonInfo->GetExp = atoi( strBuff );
				lpCharInfo->Exp = lpMonInfo->GetExp;
				NextFlag = TRUE;
			}

			///////////////// *물약 보유 ///////////////////////
			if ( !NextFlag && lstrcmp( decode , "*물약보유수" )==0 ) {
				p=GetWord(strBuff,p);
				lpMonInfo->PotionCount = atoi( strBuff );
				NextFlag = TRUE;
			}

			///////////////// *물약 보유 ///////////////////////
			if ( !NextFlag && lstrcmp( decode , "*물약보유률" )==0 ) {
				p=GetWord(strBuff,p);
				lpMonInfo->PotionPercent = atoi( strBuff );
				NextFlag = TRUE;
			}
//	int		PotionCount;		//물약 보유수
//	int		PotionPercent;		//물약 보유률


			if ( !NextFlag && lstrcmp( decode , "*아이템모두" )==0 ) {	//땅에 떨어질때 모두에게 아이템 보임
				lpMonInfo->AllSeeItem = TRUE;
				NextFlag = TRUE;
			}

			if ( !NextFlag && lstrcmp( decode , "*아이템카운터" )==0 ) {
				p = GetWord( strBuff , p );			//확률
				lpMonInfo->FallItemMax = atoi( strBuff );
				NextFlag = TRUE;
			}

			if ( !NextFlag && lstrcmp( decode , "*추가아이템" )==0 ) {
				p = GetWord( strBuff , p );			//확률
				lpMonInfo->FallItems_Plus[ lpMonInfo->FallItemPlusCount ].Percentage = atoi(strBuff);

				p = GetWord( strBuff , p );			//아이템 코드
				if ( strBuff[0] && lpMonInfo->FallItemPlusCount<FALLITEM2_MAX ) {
					for(cnt=0;cnt<MAX_ITEM;cnt++) {
						if ( lstrcmpi( strBuff , sItem[cnt].LastCategory )==0 ) {
							lpMonInfo->FallItems_Plus[ lpMonInfo->FallItemPlusCount ].dwItemCode = sItem[cnt].CODE;
							lpMonInfo->FallItemPlusCount++;
							break;
						}
					}
				}
				NextFlag = TRUE;
			}

			if ( !NextFlag && lstrcmp( decode , "*아이템" )==0 ) {
				p = GetWord( strBuff , p );			//확률
				percent = atoi( strBuff );

				lpMonInfo->FallItemPerMax += percent;

				p = GetWord( strBuff , p );			//확률
				if ( lstrcmp( strBuff , "없음" )==0 ) {
					//테스트 서버 안나올 확률 적계
					if ( rsServerConfig.TestSeverMode ) {
						lpMonInfo->FallItemPerMax -= percent;
						percent /= 2;
						lpMonInfo->FallItemPerMax += percent;
					}

					//안떨어질 확률
					lpMonInfo->FallItems[ lpMonInfo->FallItemCount ].dwItemCode = 0;
					lpMonInfo->FallItems[ lpMonInfo->FallItemCount ].Percentage = percent;
					lpMonInfo->FallItemCount ++;
				}
				else {
					if ( lstrcmp( strBuff , "돈" )==0 ) {
						//돈 떨어질 확률
						lpMonInfo->FallItems[ lpMonInfo->FallItemCount ].dwItemCode = sinGG1|sin01;
						p=GetWord( strBuff , p );
						lpMonInfo->FallItems[ lpMonInfo->FallItemCount ].sPrice[0] = atoi(strBuff);
						p=GetWord( strBuff , p );
						lpMonInfo->FallItems[ lpMonInfo->FallItemCount ].sPrice[1] = atoi(strBuff);
						lpMonInfo->FallItems[ lpMonInfo->FallItemCount ].Percentage = percent;
						lpMonInfo->FallItemCount ++;
					}
					else {
						//아이템 떨어질 확률
						ItemCodeCnt = 0;

						while(1) {
							if ( strBuff[0] ) {
								//코드검색 하여 설정
								for(cnt=0;cnt<MAX_ITEM;cnt++) {
									if ( lstrcmpi( strBuff , sItem[cnt].LastCategory )==0 ) {
										dwItemCode[ItemCodeCnt++] = sItem[cnt].CODE;
										break;
									}
								}
							}
							else 
								{ break; }

							strBuff[0] = 0;
							if ( p[0]==0 ) break;
							p = GetWord( strBuff , p );
						}

						if ( ItemCodeCnt>0 ) {
							//아이템 테이블에 저장
							percent = percent/ItemCodeCnt;
							for( cnt=0;cnt<ItemCodeCnt;cnt++ ) {
								lpMonInfo->FallItems[ lpMonInfo->FallItemCount ].dwItemCode = dwItemCode[cnt];
								lpMonInfo->FallItems[ lpMonInfo->FallItemCount ].Percentage = percent;
								lpMonInfo->FallItemCount ++;
							}
						}
					}
				}
				NextFlag = TRUE;
			}

			///////////////// NPC 물건 판매 정보 /////////////////////
			if ( !NextFlag && lstrcmp( decode , "*무기판매" )==0 ) {
				while(1) {
					pb=p;p=GetWord(strBuff,p);if(strBuff[0]==34)p=GetString(strBuff,pb);	//단어 찾기
					if ( strBuff[0]==0 ) break;
					if ( lpMonInfo->SellAttackItemCount>=32 ) break;
					for( cnt=0;cnt<MAX_ITEM;cnt++) {
						if ( lstrcmpi( strBuff , sItem[cnt].LastCategory )==0 ) {
							lpMonInfo->SellAttackItem[lpMonInfo->SellAttackItemCount++] =
								(void *)sItem[cnt].CODE;
							break;
						}
					}
				}
				NextFlag = TRUE;
			}

			if ( !NextFlag && lstrcmp( decode , "*방어구판매" )==0 ) {
				while(1) {
					pb=p;p=GetWord(strBuff,p);if(strBuff[0]==34)p=GetString(strBuff,pb);	//단어 찾기
					if ( strBuff[0]==0 ) break;
					if ( lpMonInfo->SellDefenceItemCount>=32 ) break;
					for( cnt=0;cnt<MAX_ITEM;cnt++) {
						if ( lstrcmpi( strBuff , sItem[cnt].LastCategory )==0 ) {
							lpMonInfo->SellDefenceItem[lpMonInfo->SellDefenceItemCount++] =
								(void *)sItem[cnt].CODE;
							break;
						}
					}
				}
				NextFlag = TRUE;
			}

			if ( !NextFlag && lstrcmp( decode , "*잡화판매" )==0 ) {
				while(1) {
					pb=p;p=GetWord(strBuff,p);if(strBuff[0]==34)p=GetString(strBuff,pb);	//단어 찾기
					if ( strBuff[0]==0 ) break;
					if ( lpMonInfo->SellEtcItemCount>=32 ) break;
					for( cnt=0;cnt<MAX_ITEM;cnt++) {
						if ( lstrcmpi( strBuff , sItem[cnt].LastCategory )==0 ) {
							lpMonInfo->SellEtcItem[lpMonInfo->SellEtcItemCount++] =
								(void *)sItem[cnt].CODE;
							break;
						}
					}
				}
				NextFlag = TRUE;
			}

			if ( !NextFlag && lstrcmp( decode , "*스킬수련" )==0 ) {
				lpMonInfo->SkillMaster = TRUE;
				NextFlag = TRUE;
			}
			if ( !NextFlag && lstrcmp( decode , "*직업전환" )==0 ) {
				lpMonInfo->SkillChangeJob = TRUE;

				p=GetWord( strBuff , p );
				if ( strBuff[0] )
					lpMonInfo->SkillChangeJob = atoi(strBuff);

				NextFlag = TRUE;
			}

			if ( !NextFlag && lstrcmp( decode , "*이벤트매표소" )==0 ) {
				p=GetWord( strBuff , p );
				if ( strBuff[0] )
					lpMonInfo->EventNPC = atoi(strBuff);
				else
					lpMonInfo->EventNPC = 1;

				NextFlag = TRUE;
			}
			if ( !NextFlag && lstrcmp( decode , "*아이템보관" )==0 ) {
				lpMonInfo->WareHouseMaster = TRUE;
				NextFlag = TRUE;
			}
			if ( !NextFlag && lstrcmp( decode , "*아이템조합" )==0 ) {
				lpMonInfo->ItemMix = TRUE;
				NextFlag = TRUE;
			}
			// pluto 제련
			if( !NextFlag && lstrcmp( decode , "*아이템제련" ) == 0 )
			{
				lpMonInfo->Smelting = TRUE;
				NextFlag = TRUE;
			}
			// pluto 제작
			if( !NextFlag && lstrcmp( decode, "*아이템제작" ) == 0 )
			{
                lpMonInfo->Manufacture = TRUE;
				NextFlag = TRUE;
			}
			if ( !NextFlag && lstrcmp( decode , "*아이템연금" )==0 ) {
				lpMonInfo->ItemMix = 200;
				NextFlag = TRUE;
			}
			if ( !NextFlag && lstrcmp( decode , "*아이템에이징" )==0 ) {
				lpMonInfo->ItemAging = TRUE;
				NextFlag = TRUE;
			}
			// 석지용 - 믹스쳐 리셋
			if( !NextFlag && lstrcmp( decode, "*믹스처리셋" ) == 0 )
			{
				lpMonInfo->MixtureReset = TRUE;
				NextFlag = TRUE;
			}
			if ( !NextFlag && lstrcmp( decode , "*모금함" )==0 ) {
				lpMonInfo->CollectMoney = TRUE;
				NextFlag = TRUE;
			}
#ifdef _LANGUAGE_VEITNAM
			if ( !NextFlag && lstrcmp( decode , "*꽝이지롱" )==0 ) {
				lpMonInfo->WowEvent = TRUE;
				NextFlag = TRUE;
			}
#endif
			if ( !NextFlag && lstrcmp( decode , "*경품추첨" )==0 ) {
				lpMonInfo->EventGift = TRUE;
				NextFlag = TRUE;
			}
			if ( !NextFlag && lstrcmp( decode , "*클랜기능" )==0 ) {
				lpMonInfo->ClanNPC = TRUE;
				NextFlag = TRUE;
			}
			if ( !NextFlag && lstrcmp( decode , "*경품배달" )==0 ) {
				lpMonInfo->GiftExpress = TRUE;
				NextFlag = TRUE;
			}

			if ( !NextFlag && lstrcmp( decode , "*윙퀘스트" )==0 ) {
				p=GetWord( strBuff , p );
				if ( strBuff[0] )
					lpMonInfo->WingQuestNpc = atoi(strBuff);
				else
					lpMonInfo->WingQuestNpc = 1;
				NextFlag = TRUE;
			}

			if ( !NextFlag && lstrcmp( decode , "*퀘스트이벤트" )==0 ) {

				p=GetWord( strBuff , p );
				if ( strBuff[0] )
					lpMonInfo->WingQuestNpc = atoi(strBuff);
				else
					lpMonInfo->WingQuestNpc = 2;

				NextFlag = TRUE;
			}

			if ( !NextFlag && lstrcmp( decode , "*별포인트적립" )==0 ) {
				p=GetWord( strBuff , p );
				if ( strBuff[0] )
					lpMonInfo->StarPointNpc = atoi(strBuff);
				else
					lpMonInfo->StarPointNpc = 20;

				NextFlag = TRUE;
			}
			if ( !NextFlag && lstrcmp( decode , "*기부함" )==0 ) {
				lpMonInfo->GiveMoneyNpc = TRUE;
				NextFlag = TRUE;
			}

			if ( !NextFlag && lstrcmp( decode , "*텔레포트" )==0 ) {
				p=GetWord( strBuff , p );
				if ( strBuff[0] )
					lpMonInfo->TelePortNpc = atoi(strBuff);
				else
					lpMonInfo->TelePortNpc = 1;

				NextFlag = TRUE;
			}
			if ( !NextFlag && lstrcmp( decode , "*블레스캐슬" )==0 ) {
				p=GetWord( strBuff , p );
				if ( strBuff[0] )
					lpMonInfo->BlessCastleNPC = atoi(strBuff);
				else
					lpMonInfo->BlessCastleNPC = 1;

				NextFlag = TRUE;
			}
			if ( !NextFlag && lstrcmp( decode , "*설문조사" )==0 ) {
				p=GetWord( strBuff , p );
				if ( strBuff[0] )
					lpMonInfo->PollingNpc = atoi(strBuff);
				else
					lpMonInfo->PollingNpc = 1;

				NextFlag = TRUE;
			}

			if ( !NextFlag && lstrcmp( decode , "*동영상제목" )==0 ) {
				pb=p;p=GetWord(strBuff,p);if(strBuff[0]==34)p=GetString(strBuff,pb);	//단어 찾기
				if ( strBuff[0] ) {
					lpMonInfo->szMediaPlayNPC_Title = SetNPCMsg(strBuff);
				}
				NextFlag = TRUE;
			}
			if ( !NextFlag && lstrcmp( decode , "*동영상경로" )==0 ) {
				pb=p;p=GetWord(strBuff,p);if(strBuff[0]==34)p=GetString(strBuff,pb);	//단어 찾기
				if ( strBuff[0] ) {
					lpMonInfo->szMediaPlayNPC_Path = SetNPCMsg(strBuff);
				}
				NextFlag = TRUE;
			}
			
			if ( !NextFlag && lstrcmp( decode , "*출현간격" )==0 ) {
				p=GetWord( strBuff , p );
				lpMonInfo->OpenCount[0] = atoi(strBuff);		//출현 번호
				p=GetWord( strBuff , p );
				lpMonInfo->OpenCount[1] = atoi(strBuff);		//랜덤 카운트
				NextFlag = TRUE;
			}
			if ( !NextFlag && lstrcmp( decode , "*퀘스트코드" )==0 ) {
				p=GetWord( strBuff , p );
				lpMonInfo->QuestCode = atoi(strBuff);		//퀘스트 코드
				p=GetWord( strBuff , p );
				lpMonInfo->QuestParam = atoi(strBuff);		//퀘스트 파라메터
				NextFlag = TRUE;
			}
		}

		////////////////// 다른 파일에서 계속 읽어 드림 ///////////////
		if ( !NextFlag && lstrcmp( decode , "*연결파일" )==0 ) {
			pb=p;p=GetWord(strBuff,p);if(strBuff[0]==34)p=GetString(strBuff,pb);	//단어 찾기

			char szNextFile[128];
			GetDirectoryFromFile( szFile , szNextFile );
			lstrcat( szNextFile , strBuff );
			NextFlag = TRUE;
			fclose(fp);

			fp = fopen( szNextFile , "rb" );
			if ( fp==NULL ) break;
		}

	}

	//몬스터 구조체 연결
	if ( lpMonInfo )
		lpCharInfo->lpMonInfo = lpMonInfo;
	else
		lpCharInfo->lpMonInfo = 0;


	if (fp) fclose(fp);

	return TRUE;
}
