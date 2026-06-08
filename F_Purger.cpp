#include "pch.h"
//#include "stdafx.h"
#include <stdio.h>
#include <stdlib.h>
#include <direct.h>
#include <io.h>
#include <time.h>
#include <math.h>
#include <winbase.h>
#include <winnt.h>
#include "F_Purger.h"




int IdleF_Purger(LPVOID obj)
{
	int RC=0;

	if(strlen(((F_Purger*)obj)->PathPurg)>3)
	{
// 		if(((F_Purger*)obj)->Srv->Timer(((F_Purger*)obj)->Timer_0,1000))
// 			((F_Purger*)obj)->PurgFile(((F_Purger*)obj)->PathPurg,1);

		((F_Purger*)obj)->PurgFile(((F_Purger*)obj)->PathPurg,1);
		if(((F_Purger*)obj)->Srv->Timer(((F_Purger*)obj)->Timer_0,10000))
			RC=0;
		else
			RC=1;

		if(((F_Purger*)obj)->Srv->Timer(((F_Purger*)obj)->Timer_1,30000))
		{
			time_t	TIME;
			TIME=time(NULL);
			WaitForSingleObject(((F_Purger*)obj)->hmtx,INFINITE);
			((F_Purger*)obj)->KolStatOut+=sprintf(((F_Purger*)obj)->StatOut+((F_Purger*)obj)->KolStatOut,"\r\nFree %3.2f%%   Date : %s",((F_Purger*)obj)->NewFreeDiskSpace,ctime(&TIME));
			ReleaseSemaphore(((F_Purger*)obj)->hmtx,1,NULL);
		}
	}

	return RC;
}

// void RecObr(F_Purger *P)
// {
// 	time_t		TIME;
// 	Sleep(10);
// 	while(P->Fl_On!=0)
// 	{
// 		if(P->Fl_StartMt==1)
// 		{
// 			if(strlen(P->PathPurg)>3)
// 			{
// 				//Диск установлен
// 				if(P->PurgFile(P->PathPurg,1)==0)
// 				{
// 					TIME=time(NULL);
// 					WaitForSingleObject(P->hmtx,INFINITE);
// 					P->KolStatOut+=sprintf(P->StatOut+P->KolStatOut,"\r\nFree %3.2f%%   Date : %s",P->NewFreeDiskSpace,ctime(&TIME));
// 					ReleaseSemaphore(P->hmtx,1,NULL);
// 					continue;
// 				}
// 			}
// 			P->Fl_StartMt=0;
// 			P->KolStatOut+=sprintf(P->StatOut+P->KolStatOut,"\r\nError open Path");
// 			if(P->KolStatOut>20000)P->KolStatOut=0;
// 		}
// 		else
// 			Sleep(10);
// 	}
// 	P->Fl_On=2;
// 	ExitThread(0);
// };
// -------------------------------------------------------------------------------------------------------------------------------------------------
F_Purger::F_Purger()
{
	// Этот фрагмент должен быть вставлен во все последующие объекты
	// F_Purger должен бить заменен на имя объекта
	Name_Mt=F_Purger_NameMt;Rem_Mt=F_Purger_RemMt;Ver_=F_Purger_Version;Auth_=F_Purger_Author;
	PinName_I=F_Purger_PinNameI;PinName_O=F_Purger_PinNameO;Prm_=F_Purger_Prm;Vol_=F_Purger_Vol;

	Num_PinI=0;while (PinName_I[2*Num_PinI][0]!=0x0) Num_PinI++;
	Num_PinO=0;while (PinName_O[2*Num_PinO][0]!=0x0) Num_PinO++;
	Num_Prm=0;while (Prm_[2*Num_Prm][0]!=0x0)Num_Prm ++;
	//---------------------------
	KolStatOut=0;
	strcpy(	NamePurgFileEXT[0],"*");
	// 1 D = 86400 S   1 H = 3600 S
	TimePurgFileEXT[0][0]=30*86400;TimePurgFileEXT[0][1]=15*86400;TimePurgFileEXT[0][2]=5*86400;
	TimePurgFileEXT[0][3]=1*86400;TimePurgFileEXT[0][4]=12*3600;TimePurgFileEXT[0][5]=0;

	for (int i=1;i<10;i++)
	{
		NamePurgFileEXT[i][0]=0x0;
		TimePurgFileEXT[i][0]=1000*86400;TimePurgFileEXT[i][1]=1000*86400;TimePurgFileEXT[i][2]=1000*86400;
		TimePurgFileEXT[i][3]=1000*86400;TimePurgFileEXT[i][4]=1000*86400;TimePurgFileEXT[i][5]=1000*86400;
	}

	KolAktivExt=1;
	OldFreeDiskSpace=1;
	NewFreeDiskSpace=1;
	RaznDiskSpaceOut=0.5;
	//
	IgnoreTimeClk=20;
	KolTimeClk=0;
	Fl_CalcDir=0;

	Timer_0=Timer_1=0;

	strcpy(PathPurg,"C:\\Out");
	Fl_StartMt=0;
//	Fl_On=0;
	ClThread=NULL;
	hmtx=NULL;
	fDeleteFolder=false;
/*
	//---------------------------
	//Создание цепочки "RecObr"
	Fl_On=1;
	hThreadRecObr = CreateThread(NULL,0,(LPTHREAD_START_ROUTINE)RecObr,(void*)this,0,(LPDWORD)&T_ID_RecObr);
	SetThreadPriority( hThreadRecObr, THREAD_PRIORITY_NORMAL );
	Sleep(10);
	sprintf(NameDmp,"F_Purger%08x",this);
	hmtx=OpenSemaphore(SEMAPHORE_ALL_ACCESS ,TRUE,NameDmp);
	if(hmtx==NULL)
		hmtx=CreateSemaphore(NULL,1,1,NameDmp);
*/
};

// -------------------------------------------------------------------------------------------------------------------------------------------------
void F_Purger::Param(char* Prm,char* Vol)
{
	//---------------------------
	if (strcmp(Prm,"PathPurg")==0)
	{	
		strcpy(PathPurg,Vol);
	}
	if (strcmp(Prm,PathPurg)==0)
	{
		strcpy(PathPurg,Vol);
	}
	if (strcmp(Prm,"IgnoreTimeClk")==0) IgnoreTimeClk=atoi(Vol);
	if (strcmp(Prm,"KolAktivExt")==0) KolAktivExt=atoi(Vol);

	if (strcmp(Prm,"Ext0")==0) strcpy(NamePurgFileEXT[0],Vol);
	if (strcmp(Prm,"Ext1")==0) strcpy(NamePurgFileEXT[1],Vol);
	if (strcmp(Prm,"Ext2")==0) strcpy(NamePurgFileEXT[2],Vol);
	if (strcmp(Prm,"Ext3")==0) strcpy(NamePurgFileEXT[3],Vol);
	if (strcmp(Prm,"Ext4")==0) strcpy(NamePurgFileEXT[4],Vol);
	if (strcmp(Prm,"Ext5")==0) strcpy(NamePurgFileEXT[5],Vol);
	if (strcmp(Prm,"Ext6")==0) strcpy(NamePurgFileEXT[6],Vol);
	if (strcmp(Prm,"Ext7")==0) strcpy(NamePurgFileEXT[7],Vol);
	if (strcmp(Prm,"Ext8")==0) strcpy(NamePurgFileEXT[8],Vol);
	if (strcmp(Prm,"Ext9")==0) strcpy(NamePurgFileEXT[9],Vol);

	if (strcmp(Prm,"TimeExt0")==0) ReadParam(0,Vol);
	if (strcmp(Prm,"TimeExt1")==0) ReadParam(1,Vol);
	if (strcmp(Prm,"TimeExt2")==0) ReadParam(2,Vol);
	if (strcmp(Prm,"TimeExt3")==0) ReadParam(3,Vol);
	if (strcmp(Prm,"TimeExt4")==0) ReadParam(4,Vol);
	if (strcmp(Prm,"TimeExt5")==0) ReadParam(5,Vol);
	if (strcmp(Prm,"TimeExt6")==0) ReadParam(6,Vol);
	if (strcmp(Prm,"TimeExt7")==0) ReadParam(7,Vol);
	if (strcmp(Prm,"TimeExt8")==0) ReadParam(8,Vol);
	if (strcmp(Prm,"TimeExt9")==0) ReadParam(9,Vol);
	if (strcmp(Prm,"DeleteFolder")==0)// ReadParam(9,Vol);
	{
		if (stricmp(Vol,"Yes")==0)
		{
			fDeleteFolder=true;
		}
		else
			fDeleteFolder=false;
	}
};
// -------------------------------------------------------------------------------------------------------------------------------------------------
int F_Purger::ReadParam(int Num,char* Vol)
{
	int		RP[6]={8640000,8640000,8640000,8640000,8640000,8640000};
	int		KolRP=0;
	char	TempChar[20];
	int		KolTempChar=0;
	int		ReadVol=0;
	size_t i;
	
//	while((Vol[i0]<0x21)&&(Vol[i0]!=0))i0++;//Пропуск разделительных символов
	for(i=0;i<strlen(Vol)+1;i++)
	{
		if(Vol[i]<0x30)
		{
			if(KolTempChar==0)
			{	//Нет значения
				KolRP++;
			}
			else
			{
				//Есть значение
				TempChar[KolTempChar]=0;
				if(TempChar[0]=='H')
				{	//1 H = 3600 S
					RP[KolRP++]=atoi(&TempChar[1])*3600;
					KolTempChar=0;
				}
				else
				if(TempChar[0]=='D')
				{	//D = 86400 S
					RP[KolRP++]=atoi(&TempChar[1])*86400;
					KolTempChar=0;
				}
				else
				{
					KolRP++;
					KolTempChar=0;
				}
			}
		}
		else	TempChar[KolTempChar++]=Vol[i];
	}
	for(i=0;i<6;i++)TimePurgFileEXT[Num][i]=RP[i];

	return 1;
};
// -------------------------------------------------------------------------------------------------------------------------------------------------
F_Purger::~F_Purger()
{
	Stop();
/*
	//Уничтожение цепочки "RecObr"
	Fl_On=0; while (Fl_On!=2) Sleep(100);
	CloseHandle(hmtx);
*/
};
// -------------------------------------------------------------------------------------------------------------------------------------------------
void F_Purger::Start()
{
	char str[128];

	hmtx=OpenSemaphore(SEMAPHORE_ALL_ACCESS ,TRUE,NameDmp);
	if(hmtx==NULL)
		hmtx=CreateSemaphore(NULL,1,1,NameDmp);

	sprintf(str,"F_Purger_%d",(int)this);
	ClThread=new ClassThread(str,IdleF_Purger,(LPVOID)this,THREAD_PRIORITY_NORMAL);
// 	Fl_On=1;
// 	hThreadRecObr = CreateThread(NULL,0,(LPTHREAD_START_ROUTINE)RecObr,(void*)this,0,(LPDWORD)&T_ID_RecObr);
// 	SetThreadPriority( hThreadRecObr, THREAD_PRIORITY_NORMAL );
// 	Sleep(10);
// 	sprintf(NameDmp,"F_Purger%08x",this);

	Fl_StartMt=1;
};
// -------------------------------------------------------------------------------------------------------------------------------------------------
void F_Purger::Stop()
{
// 	Fl_On=0;
// 	while(Fl_On!=2)
// 		Sleep(100);
	
	Fl_StartMt=0;
	if(ClThread)
	{
		delete ClThread;
		ClThread=NULL;
	}
	if(hmtx)
	{
		CloseHandle(hmtx);
		hmtx=NULL;
	}

//	Fl_StartMt=0;
};
// -------------------------------------------------------------------------------------------------------------------------------------------------
int F_Purger::DataFF(int N_Pin,UCHAR* pmas,size_t Cnt)
{
	if(Fl_StartMt==0) 
		return 0;
//	Fl_DataMt=1;
	//-----------------
	//*** Нулевая входная ножка
	//-----------------
	if(N_Pin==0)
	{
		if((++KolTimeClk)>=IgnoreTimeClk) 
		{
			Fl_CalcDir=1;
			KolTimeClk=0;
			if(KolStatOut)
			{
				WaitForSingleObject(hmtx,INFINITE);
				memcpy(StatOut0,StatOut,KolStatOut);
				KolStatOut0=KolStatOut;
				KolStatOut=0;
				ReleaseSemaphore(hmtx,1,NULL);
				ODataFF(0,(UCHAR*)StatOut0,KolStatOut0);
// 				pmas=(UCHAR*)StatOut0;
// 				Cnt=KolStatOut0;
// 				return Cnt;
			}

		};
// 		return 1;
	}//+++

	return 0;
};
// -------------------------------------------------------------------------------------------------------------------------------------------------
int F_Purger::IdleFunc(int NumberTr)			// Idle function  if rc!=0 recall function (?)  NumberTr=-1 System NumberTr+0x8000 - DinTr
{
	int	rc=ChildIdleFunc(NumberTr);

// 	if((++KolTimeClk)>=IgnoreTimeClk) 
// 	{
// 		Fl_CalcDir=1;
// 		KolTimeClk=0;
		if(KolStatOut)
		{
			WaitForSingleObject(hmtx,INFINITE);
			memcpy(StatOut0,StatOut,KolStatOut);
			KolStatOut0=KolStatOut;
			KolStatOut=0;
			ReleaseSemaphore(hmtx,1,NULL);
			ODataFF(0,(UCHAR*)StatOut0,KolStatOut0);
		}
// 	};

	return rc;
}
// -------------------------------------------------------------------------------------------------------------------------------------------------
float F_Purger::GetFree_Space(char disk)
{
	WCHAR t[16];
	ULONG SectorsPerCluster;
	ULONG BytesPerSector;
	ULONG NumberOfFreeClusters;
	ULONG TotalNumberOfClusters;
//	strcpy(t,"x:\\");
	wsprintf(t, _T("x:\\"));
	t[0] = disk;
	if(GetDiskFreeSpace( t,&SectorsPerCluster,&BytesPerSector,&NumberOfFreeClusters,&TotalNumberOfClusters))
	{
		float PrFreeSpace=(float)100*(float)NumberOfFreeClusters/(float)TotalNumberOfClusters;
//		Free  = NumberOfFreeClusters  * SectorsPerCluster * (BytesPerSector>>8);
//		Total = TotalNumberOfClusters * SectorsPerCluster * (BytesPerSector>>8);
//		Free=Free>>12;Total=Total>>12;
		return PrFreeSpace;
	}
	else return 0;
};
// -------------------------------------------------------------------------------------------------------------------------------------------------
int	F_Purger::CalcTimeL()
{
//[>25%] [15%] [10%] [5%] [1%] [0%]
	if(NewFreeDiskSpace>=25)
	{
		for(UINT i=0;i<KolAktivExt;i++)
		{
			TimePurgFile[i]=TimePurgFileEXT[i][0];
		}
	}
	else
	if(NewFreeDiskSpace>15)
	{
		LinApr(25.f,15.f,0,1,NewFreeDiskSpace);
	}
	else
	if(NewFreeDiskSpace>10)
	{
		LinApr(15.f,10.f,1,2,NewFreeDiskSpace);
	}
	else
	if(NewFreeDiskSpace>5)
	{
		LinApr(10.f,5.f,2,3,NewFreeDiskSpace);
	}
	else
	if(NewFreeDiskSpace>1)
	{
		LinApr(5.f,1.f,3,4,NewFreeDiskSpace);
	}
	else
	{
		LinApr(1.f,0.f,4,5,NewFreeDiskSpace);
	}
	return 0;
};
// -------------------------------------------------------------------------------------------------------------------------------------------------
int	F_Purger::LinApr(float Fr1,float Fr2,int NTime1,int NTime2,float FrX)
{
	for(UINT i=0;i<KolAktivExt;i++)
	{
		TimePurgFile[i]=TimePurgFileEXT[i][NTime1]+(int)((TimePurgFileEXT[i][NTime2]-TimePurgFileEXT[i][NTime1])*(FrX-Fr1)/(Fr2-Fr1));
	};
	return 0;
};
// -------------------------------------------------------------------------------------------------------------------------------------------------
// Процедура обработки одного подкаталога
//0-Подкаталог обработан
//1-Выход из рекурсии
int	F_Purger::PurgFile(char* FullPath,UINT	GlRecurs)
{
	struct _finddata_t	FileInfo;    
	size_t				hFile;
	char				FullPath_File[2048];
	time_t				TIME=time(NULL);
	int					rc;

	if(Fl_StartMt==0)
		return 1;
// 	if(Fl_On==0)
// 		return 1;

	NewFreeDiskSpace=GetFree_Space(PathPurg[0]);
	CalcTimeL();
	if(fabs(OldFreeDiskSpace-NewFreeDiskSpace)>RaznDiskSpaceOut)
	{
		WaitForSingleObject(hmtx,INFINITE);
		KolStatOut+=sprintf(StatOut+KolStatOut,"\r\nFree %3.2f%%   Date : %s",NewFreeDiskSpace,ctime(&TIME));
		ReleaseSemaphore(hmtx,1,NULL);

		OldFreeDiskSpace=NewFreeDiskSpace;
	}
	//
	if(GlRecurs>100)
	{
		KolStatOut+=sprintf(StatOut+KolStatOut,"\r\nGlRec !!!100!!!");
		if(KolStatOut>20000)
			KolStatOut=0;
		return 1;
	};

// 	if(Fl_CalcDir==1)
// 		Fl_CalcDir=0;
// 	else
// 	{
// 		do
// 		{
// 			if(Fl_On==0)return 1;
// 			Sleep(20);
// 		}
// 		while(Fl_CalcDir==0);
// 	}

	//Анализ файлов
	for(UINT i=0;i<KolAktivExt;i++)
	{
		sprintf(FullPath_File,"%s\\*.%s",FullPath,NamePurgFileEXT[i]);
		if((hFile=_findfirst(FullPath_File,&FileInfo))!=-1L)
		{
			do
			{
				if(FileInfo.attrib&_A_SUBDIR)
				{
					if (fDeleteFolder)
					{
						if((FileInfo.time_create!=-1L) && ((TIME-FileInfo.time_write)/*time_create*/>TimePurgFile[i]))
						{
						sprintf(FullPath_File,"%s\\%s",FullPath,FileInfo.name);
						_rmdir(FullPath_File);
						}
					};

				}
				else
				{
					//Анализ даты последней записи
					if((FileInfo.time_create!=-1L) && ((TIME-FileInfo.time_write)>TimePurgFile[i]))
					{
						//Файл устарел
						sprintf(FullPath_File,"%s\\%s",FullPath,FileInfo.name);
						FILETIME ft;
						SYSTEMTIME STime;
						UnixTimeToFileTime(TIME, &ft);
						FileTimeToSystemTime(&ft,&STime);

						WaitForSingleObject(hmtx,INFINITE);
						KolStatOut+=sprintf(StatOut+KolStatOut,"\r\nDel %s Time=%02d-%02d-%04d %02d:%02d:%02d ",FullPath_File,STime.wDay,STime.wMonth,STime.wYear,STime.wHour,STime.wMinute,STime.wSecond);if(KolStatOut>20000)KolStatOut=0;
						UnixTimeToFileTime(FileInfo.time_write, &ft);
						FileTimeToSystemTime(&ft,&STime);

						KolStatOut+=sprintf(StatOut+KolStatOut,"FTime=%02d-%02d-%04d %02d:%02d:%02d ",STime.wDay,STime.wMonth,STime.wYear,STime.wHour,STime.wMinute,STime.wSecond);if(KolStatOut>20000)KolStatOut=0;
						KolStatOut+=sprintf(StatOut+KolStatOut,"(HDD_dTime=%x Prg_dTime=%x)",(INT64)(TIME-FileInfo.time_write),TimePurgFile[i]);if(KolStatOut>20000)KolStatOut=0;
						ReleaseSemaphore(hmtx,1,NULL);
						rc = SetFileAttributesA(FullPath_File, FILE_ATTRIBUTE_NORMAL);
						rc = remove(FullPath_File);
						KolStatOut += sprintf(StatOut + KolStatOut, " RC=%d", rc); if (KolStatOut > 20000)KolStatOut = 0;
					}
				} 
				if(Fl_StartMt==0)
				{
					_findclose( hFile );
					return 1;
				}

// 				if(Fl_On==0) 
// 				{
// 					_findclose( hFile );
// 					return 1;
// 				};
				Sleep(100);
			}
			while(!_findnext(hFile,&FileInfo));
			_findclose(hFile);   
		}
	}
	//Анализ подкаталога	
	sprintf(FullPath_File,"%s\\*.*",FullPath);
	if((hFile=_findfirst(FullPath_File,&FileInfo))!=-1L)
	{
		do
		{
			if((FileInfo.attrib&_A_SUBDIR) && (strcmp(FileInfo.name,".")) && (strcmp(FileInfo.name,"..")))
			{
				sprintf(FullPath_File,"%s\\%s",FullPath,FileInfo.name);
				int RetCod=PurgFile(FullPath_File,GlRecurs+1);
				if(RetCod==1)
				{
					_findclose(hFile);
					return 1;
				}
			}
			if(Fl_StartMt==0)
			{
				_findclose(hFile);
				return 1;
			};
			
// 			if(Fl_On==0)
// 			{
// 				_findclose(hFile);
// 				return 1;
// 			};
			Sleep(100);
		}
		while(!_findnext(hFile,&FileInfo));
		_findclose(hFile);   
	}

	return 0;
};
// -------------------------------------------------------------------------------------------------------------------------------------------------
void F_Purger::UnixTimeToFileTime(time_t t, LPFILETIME pft)
{
	// Note that LONGLONG is a 64-bit value
	LONGLONG ll;

	ll = Int32x32To64(t, 10000000) + 116444736000000000;
	pft->dwLowDateTime = (DWORD)ll;
	pft->dwHighDateTime = ll >> 32;
};
// -------------------------------------------------------------------------------------------------------------------------------------------------

