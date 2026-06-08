// --------------------------
#include "pch.h"
//#include	"stdafx.h"
#include <fcntl.h>
#include <sys/types.h>
#include <io.h>
#include <stdio.h>
#include <Shellapi.h>
#include	<sys\stat.h>
#include	<share.h>

#include	"DecodB.h"
//#include	<DLLCntrl.h>
#include	<Math.h>
#include	<Psapi.h>
//#include	<PackLib.h>
//#include	<PackDec.h>
//#include	<DebugTract.h>
#include	<intrin.h>
#include	"F_Purger.h"


//#pragma comment( lib, <Psapi.lib> )



#ifdef _Neuron
#include	<Winuser.h>
//#include	<Iads.h>
#if WIN64
#else
#pragma comment( lib, "ADSIid.lib" )
#endif
#endif



#ifdef _DEBUG
#ifdef DEBUG_NEW
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif
#endif



//extern		Services		*Service;
//extern		ServerDecod		*Dec;

static int	SizePrm[12]=
{
		2,				//	PrmType_SHORT,
		4,				//	PrmType_INT=0,
		8,				//	PrmType_INT64,
		4,				//	PrmType_FLOAT,
		8,				//	PrmType_DOUBLE,
		1,				//	PrmType_CHAR,
		0,				//	PrmType_STRING,
		0,				//	PrmType_USER,
		0,				//	PrmType_BLOB,	// "Áåñêîíå÷íàÿ" ñòðóêòóðà. Â áàçå õðàíèòñÿ Çàãîëîâîê = int32(0..2147483647) = ðàçìåð ïîñëåäóþùèõ äàííûõ. (c)Mao
		0,				//	PrmType_TAG,	// "Ïóñòàÿ" ñòðóêòóðà íóëåâîé äëèíîé. Îáîçíà÷àåò êàêîé-ëèáî ëîãè÷åñêèé ïðèçíàê. PrmType_INDICATOR (c)Mao
		0,				//	PrmType_GeoPos, // Ãåîãðàôè÷åñêèå êîîðäèíàòû íà çåìíîì øàðå - ñòðóêòóðà tGeoPos ðàçìåðîì sizeof(float)*3.
		0				//	PrmType_GeoDir, // Äàííûå óãëîìåðíîãî öåëåóêàçàíèÿ - ñòðóêòóðà tDirPos ðàçìåðîì sizeof(float)*2 (ñ ïîçèöèè íà ïîâåðõíîñòè çåìëè).
};

//#pragma comment( lib, "netname.lib" )
//extern char*	NetName();
// ------------------------
BOOL MyTryLock(char* FileName,int NLine,FlagStruct*	StrFlag,Decoder* Dec)
{
	BOOL rc=TryEnterCriticalSection(&StrFlag->CrSection);
	if (rc)
	{
		StrFlag->NumLine=NLine;
		StrFlag->FileName=FileName;
		StrFlag->CntIn++;
// 		if (StrFlag->CntIn>1)
// 		{
// 			size_t iasm=1;
// 		}
		StrFlag->PDec=Dec;
	}
	return rc;
};

void MyLock(char* FileName,int NLine,FlagStruct*	StrFlag,Decoder* Dec)
{
	EnterCriticalSection(&StrFlag->CrSection);
	StrFlag->NumLine=NLine;
	StrFlag->FileName=FileName;
	StrFlag->CntIn++;
// 	if (StrFlag->CntIn>1)
// 	{
// 		size_t iasm=1;
// 	}
	StrFlag->PDec=Dec;
};

void MyUnLock(char* FileName,int NLine,FlagStruct* StrFlag,Decoder* Dec)
{
/*
	char*	AdrFlag=&StrFlag->Flag;
//	_asm	mov	al,1;
//	_asm	mov	bl,0;
//	_asm	mov	edx,AdrFlag;
//	_asm	cmpxchg	[edx],bl;
	if (StrFlag->Flag==0)
		{size_t iasm=1;}
	StrFlag->CntIn--;
	if (StrFlag->CntIn<0)
		{size_t iasm=1;}
	StrFlag->PDec=NULL;
	StrFlag->Flag2=0;
	StrFlag->Flag=0; 
*/
	if (--StrFlag->CntIn==0) StrFlag->PDec=NULL;
	if (StrFlag->CntIn<0)
	{
		size_t iasm=1;
	}
//	StrFlag->NumLine=NLine;
//	StrFlag->FileName=FileName;
	LeaveCriticalSection(&StrFlag->CrSection);
};


#include <Tlhelp32.h>

bool FindProcess(WCHAR *name, PROCESSENTRY32 *pEntry)
{ 
	// íà âûõîäå äàåò pEntry, íàì èíòåðåñíî pInfo.th32ProcessID
	HANDLE hSnapshot;
	PROCESSENTRY32 pe;
	
	hSnapshot=CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
	if(hSnapshot==INVALID_HANDLE_VALUE)	
		return 0;
	
	memset(&pe,0,sizeof(pe));
	pe.dwSize=sizeof(pe);
	if(!Process32First(hSnapshot, &pe))
	{
		CloseHandle(hSnapshot);
		return 0;		//îïÿòü íå ñðîñëîñü;
	}
	
	do
	{
		if(!lstrcmpi(pe.szExeFile,name))
		{
			// âîò îí
			*pEntry=pe;
			CloseHandle(hSnapshot);
			return true;
		}
	}
	while(Process32Next(hSnapshot, &pe));
	CloseHandle(hSnapshot);
	
	return 0;
}

void SetThreadName( DWORD dwThreadID, LPCSTR szThreadName)
{
	THREADNAME_INFO info;
	{
		info.dwType = 0x1000;
		info.szName = szThreadName;
		info.dwThreadID = dwThreadID;
		info.dwFlags = 0;
	}
	__try
	{
		RaiseException( 0x406D1388, 0, sizeof(info)/sizeof(DWORD),(ULONG_PTR *) &info );
	}
	__except (EXCEPTION_CONTINUE_EXECUTION)
	{
	}
}

int	GetInt(uchar* pBuf,int OffsBit,int CntBit,BOOL SignMinus)
{	
	int ret=0;
	int	i,j;
	i=(OffsBit+CntBit)/8;
	j=(OffsBit+CntBit)%8;
	ret=((pBuf[i]&iArrAnd[j])>>(8-j))&iArrAnd2[j];
	while (i)
	{
		i--;
		if ((CntBit-j)>8)
		{
			ret+=((pBuf[i]&iArrAnd2[8])<<j);
			j+=8;
		}
		else
		{
			ret+=((pBuf[i]&iArrAnd2[CntBit-j])<<j);
			break;
		}
		
	}
	ret&=iArrAnd2[CntBit];
	if (SignMinus==TRUE)
		ret=(ret&iArrAnd3[CntBit])?(ret|iArrAnd4[CntBit]):ret;
	return ret;
};

double		Sin(double Arg)
{
	register	int	V;
	size_t i;
	double	Vol;
	if (Fl_Sin)
	{
		double	arg;
		KSin=2*3.1415926536/SinSize;
		KSin_1=1/KSin;
		for (i=0;i<SinSize;i++)
		{
			arg=(double)i*KSin;
			TabSin[i]=sin(arg);
			TabCos[i]=cos(arg);
		}
		Fl_Sin=FALSE;
	}
	if (Arg<0)
	{
		V=(int)(-Arg*KSin_1);
		V&=0x7fff;
		Vol=-TabSin[V];
	}
	else
	{
		V=(int)(Arg*KSin_1);
		V&=0x7fff;
		Vol=TabSin[V];
	}
	return Vol;
};

double		Cos(double Arg)
{
	register	int	V;
	size_t i;
	if (Fl_Sin)
	{
		double	arg;
		KSin=2*3.1415926536/SinSize;
		KSin_1=1/KSin;
		for (i=0;i<SinSize;i++)
		{
			arg=(double)i*KSin;
			TabSin[i]=sin(arg);
			TabCos[i]=cos(arg);
		}
		Fl_Sin=FALSE;
	}
	if (Arg<0)
		V=(int)(-Arg*KSin_1);
	else
		V=(int)(Arg*KSin_1);
	V&=0x7fff;
	return	TabCos[V];
};

float		MedFltf(float tMas[],int NTF)
{
	size_t i,j,t;
	if (NTF>NumTF) 
		NTF=NumTF;
	int	nNTF=NTF/2;
	double rc;
	for (j=0;j<NTF;j++)
		tMFf[j]=tMas[j];
	for (i=0;i<nNTF;i++)
	{
		rc=-1e+200;t=0;
		for (j=0;j<NTF;j++)
		{
			if (rc<tMFf[j])
			{
				rc=tMFf[j];
				t=j;
			}
		}
		tMFf[t]=-1e+200;
	}
	return (float)rc;
};

double			MedFlt(double tMas[],int NTF)
{
	size_t i,j,t;
	if (NTF>NumTF) 
		NTF=NumTF;
	int	nNTF=NTF/2;
	double rc;
	for (j=0;j<NTF;j++)
		tMF[j]=tMas[j];
	for (i=0;i<nNTF;i++)
	{
		rc=-1e+200;t=0;
		for (j=0;j<NTF;j++)
		{
			if (rc<tMF[j])
			{
				rc=tMF[j];
				t=j;
			}
		}
		tMF[t]=-1e+200;
	}
	return rc;
};

double			Max(double tMas[],int NTF)
{
	size_t i;
	if (NTF>NumTF) 
		NTF=NumTF;
	double rc=tMas[0];
	for (i=1;i<NTF;i++)
	{
		if (rc<tMas[i])
		{
			rc=tMas[i];
		}
	}
	return rc;
};

double			Min(double tMas[],int NTF)
{
	size_t i;
	if (NTF>NumTF) 
		NTF=NumTF;
	double rc=tMas[0];
	for (i=1;i<NTF;i++)
	{
		if (rc>tMas[i])
		{
			rc=tMas[i];
		}
	}
	return rc;
};

float	Max(float tMas[],int NTF)
{
	size_t i;
	if (NTF>NumTF) 
		NTF=NumTF;
	float rc=tMas[0];
	for (i=1;i<NTF;i++)
	{
		if (rc<tMas[i])
		{
			rc=tMas[i];
		}
	}
	return rc;
};

float	Min(float tMas[],int NTF)
{
	size_t i;
	if (NTF>NumTF) 
		NTF=NumTF;
	float rc=tMas[0];
	for (i=1;i<NTF;i++)
	{
		if (rc>tMas[i])
		{
			rc=tMas[i];
		}
	}
	return rc;
};

double			AddFlt(double tMas[],int NTF)
{
	int j;
	double rc=0;
	for (j=0;j<NTF;j++)
		rc+=tMas[j];
	rc=rc/NTF;
	return rc;
};

size_t	_mSize(void* tB)
{
	size_t	size;
	try
	{
		size=*(size_t*)((UCHAR*)tB-0x10);
	}
	catch(...)
	{
		size=0;
	}
	return size;
};



INT64	GetNClk()	// Âîçâðàùàåò ñ÷åò÷èê òàêòîâ ïðîöåññîðà
{

	
//	NumClk	NC;
//	QueryPerformanceCounter((LARGE_INTEGER*)&NC.NClk);

//#if _MSC_VER > 1000	
//#if defined X64
	return __rdtsc();
//#else
//	_asm
//	{
//		rdtsc;
//		mov		NC.DWNClk.lo,eax
//		mov		NC.DWNClk.hi,edx
//	};
//#endif // _MSC_VER > 1000
//	return	NC.NClk;
};

StrOwner::StrOwner() 
{
	Prm=0;pObj=NULL;pDec=NULL;Type=0;
};

void	StrOwner::Init(Decoder* pD,void* pO,int Pr,int Tp)
{
	pDec=pD;pObj=pO;Prm=Pr;Type=Tp;
};

// ------------------------
void ThreadFun(LPVOID poi)
{
	SetThreadName(GetCurrentThreadId(),((ClassThread*)poi)->NameThread);
	while (((ClassThread*)poi)->GetOn()==1)
	{
		if (((ClassThread*)poi)->Calc()==0)
			Sleep(10);
	};
	((ClassThread*)poi)->ModOn(2);
	ExitThread(0);
};

char*		cTimeLocal(FileTime* FlTime)
{
	if (SmLocalZone==0)
	{
		FileTime LocFlTime;
		FileTimeToLocalFileTime(&FlTime->FTime,&LocFlTime.FTime); 
		SmLocalZone=LocFlTime.Time-FlTime->Time;
	}
	SYSTEMTIME	STime;
	FileTime LocTime;
	LocTime.Time=FlTime->Time+SmLocalZone;
	try
	{
		FileTimeToSystemTime(&LocTime.FTime,&STime);
		sprintf(TimeBuf,"%02d-%02d-%04d %02d:%02d:%02d.%03d ",STime.wDay,STime.wMonth,STime.wYear,STime.wHour,STime.wMinute,STime.wSecond,STime.wMilliseconds);
//		sprintf(TimeBuf,"%02d-%02d-%04d %02d:%02d:%02d",STime.wDay,STime.wMonth,STime.wYear,STime.wHour,STime.wMinute,STime.wSecond);
	}
	catch(...)
	{
		TimeBuf[0]=0;
	}
	return	TimeBuf;
}

char*		cTime(FileTime* FlTime)
{
	SYSTEMTIME	STime;
	try
	{
		FileTimeToSystemTime(&FlTime->FTime,&STime);
		sprintf(TimeBuf,"%02d-%02d-%04d %02d:%02d:%02d.%03d ",STime.wDay,STime.wMonth,STime.wYear,STime.wHour,STime.wMinute,STime.wSecond,STime.wMilliseconds);
	}
	catch(...)
	{
		TimeBuf[0]=0;
	}
	return	TimeBuf;
};

char*		cTimeAll(FileTime* FlTime)
{
//	SYSTEMTIME	STime;
	try
	{
		INT64	Tmp=FlTime->Time;
		INT64	Years=Tmp/KtimeY;
		Tmp=Tmp-Years*KtimeY;
		INT64	Days=Tmp/KtimeD;
		Tmp=Tmp-Days*KtimeD;
		INT64	Hours=Tmp/KtimeH;
		Tmp=Tmp-Hours*KtimeH;
		INT64	Minutes=Tmp/KtimeM;
		Tmp=Tmp-Minutes*KtimeM;
		INT64	Seconds=Tmp/KtimeS;
		Tmp=Tmp-Seconds*KtimeS;
		sprintf(TimeBuf,"Years:%02d Days:%02d Hours:%02d Minutes:%02d Seconds:%02d Millisecond:%03d ",(int)Years,(int)Days,(int)Hours,(int)Minutes,(int)Seconds,(int)(Tmp/Ktime));
	}
	catch(...)
	{
		TimeBuf[0]=0;
	}
	return	TimeBuf;
};

void	ID_Elem64::Init(INT64	tVol)
{
	Vol=tVol;
	Buff=(UCHAR*)&Vol;
	Len=sizeof(INT64);
	GetServices()->GetRealTime(&FT.FTime);
}

		El_Unit::El_Unit(UnitInfo* UI)
{
	Init(UI);
	GetServices()->NLAddbyUnitName(UInf.UnitName,inet_ntoa(*(in_addr*)&UInf.IP));
	pRow=NULL;
};
void	El_Unit::Init(UnitInfo*	UI)
{
	memcpy(&UInf,UI,sizeof(UnitInfo));
	Buff=(UCHAR*)&UInf.PersID;
	Len=sizeof(INT64);
};
		El_Unit::~El_Unit()
{
	GetServices()->NLDelbyUnitName(UInf.UnitName,inet_ntoa(*(in_addr*)&UInf.IP));
}

ID_ElemAdrList::ID_ElemAdrList(LPVOID	tVol) 
{
	List=new  ID_List8();
	Init(tVol);
};

ID_ElemAdrList::~ID_ElemAdrList()
{
	List->DelAllElem();
	delete	List;
};

void	ID_ElemAdrList::Init(LPVOID	tVol)
{
	Vol=tVol;
	Buff=(UCHAR*)&Vol;
	Len=sizeof(LPVOID);
};

ID_ElemAdrStr::ID_ElemAdrStr(INT64	tVol) 
{
	Init(tVol);
};
void	ID_ElemAdrStr::Init(INT64	tVol)
{
	Vol=tVol;
	Buff=(UCHAR*)&Vol;
	Len=sizeof(INT64);
	GetServices()->GetRealTime(&FT.FTime);
};


Decoder::Decoder()
{
// Ýòîò ôðàãìåíò äîëæåí áûòü âñòàâëåí âî âñå ïîñëåäóþùèå îáúåêòû
// XXX_ äîëæåí áûòü çàìåíåí íà èìÿ îáúåêòà
		Name_Mt=XXX_NameMt;Rem_Mt=XXX_RemMt;Ver_=XXX_Version;Auth_=XXX_Author;
		PinName_I=XXX_PinNameI;PinName_O=XXX_PinNameO;Prm_=XXX_Prm;Vol_=XXX_Vol;
		Type_Mt=XXX_Type;

		Num_PinI=0;while (PinName_I[2*Num_PinI][0]!=0x0) Num_PinI++;
		Num_PinO=0;while (PinName_O[2*Num_PinO][0]!=0x0) Num_PinO++;
		Num_Prm=0;while (Prm_[2*Num_Prm][0]!=0x0)Num_Prm ++;
// -------------------------------------------------------------
		ParentMt=NULL;pODataFFF=NULL;pDecODataFFF=NULL;
//		NStr=-1;
//		FlRSim=NULL;

#ifndef _NOSRV
		Srv=GetServices();	// Ïî óìîë÷àíèþ íå èñïîëüçóåòñÿ, à ðàáîòàåò òîëüêî â Neuron !!! (Ýòî èìåííî _ñåðâèñíàÿ_ âîçìîæíîñòü !!!)
#else
		Srv=0;
#endif
		NumLink=0;SizeAllocMem=0;Ext_Mt=0;NCI=NCD=NCO=0;
		if ((/*(int)*/Srv!=(Services*)-1)&&/*((int)*/(Srv!=0))
		{
			strcpy(UInfo.Host,Srv->ShortHostName);
//			 strcpy(UInfo.DomainName,UInfo.DomainName);
		}
		UInfo.Mode[0]=0;
		UInfo.PersID=0;
		UInfo.State[0]=0;
		UInfo.TractID=-1;
		UInfo.NumChan=0;
		strcpy(UInfo.UnitName,Name_Mt);
		UInfo.pObj=this;
		Anchor.Init(this,NULL,0);
		Fl_BufData=0; DmData=NULL; DmIDInfoUCB=NULL; DmIDInfoInput=NULL;DmParam=NULL;
		Fl_ReCurse=0;//Fl_ReCurse2=0;
};
Decoder::~Decoder()
{
	MyLock(__FILE__,__LINE__,&FlSim,this);
	if (strcmp(Type_Mt,"Unit")==0)
	{
		El_Tab*	ETbl=NULL;
		if (Srv->IDT_NeuronLocalSite)
		{
	 		ETbl=Srv->IDT_NeuronLocalSite->FindNested(&Anchor,AcsMd_Read,N_Domain,UInfo.DomainName,N_Host,Srv->ShortHostName,NULL);
			if (ETbl)
			{
				Srv->IDT_NeuronLocalSite->UnLockEl(&Anchor,ETbl);
				ETbl=Srv->IDT_NeuronLocalSite->FindNested(&Anchor,AcsMd_Read,N_Domain,UInfo.DomainName,N_Host,Srv->ShortHostName,N_Unit,Name_Mt,NULL);
// 				if (ETbl==NULL)
// 					ETbl=Srv->IDT_NeuronLocalSite->AttainNested(&Anchor,"%s%s%s%s%s",N_Domain,UInfo.DomainName,N_Host,Srv->ShortHostName,N_Unit,Name_Mt,-1);
				if (ETbl!=NULL)
				{
	//				Srv->IDT_NeuronLocalSite->AddParam(&Anchor,ETbl,IDState,VolumeProcessed[IDVolProc_Off]);
					Srv->IDT_NeuronLocalSite->UnLockEl(&Anchor,ETbl);
				}
			}
		}
	}
	if (DmIDInfoUCB)
	{
		delete	DmIDInfoUCB; Fl_BufData&=0xfffd; DmIDInfoUCB=NULL;
	}
	if (DmIDInfoInput)
	{
		delete	DmIDInfoInput; Fl_BufData&=0xfffb;	DmIDInfoInput=NULL;
	}
	if (DmData)
	{
		delete	DmData; Fl_BufData&=0xfffe; DmData=NULL;
	}
	if (DmParam)
	{
		delete	DmParam; Fl_BufData&=0xfff7;	DmParam=NULL;
	}
	MyLock(__FILE__,__LINE__,&FlSim,this);
};

void Decoder::Param(char* Prm,char* Vol)
{
};

void Decoder::Start()
{
};

void Decoder::Stop()
{
};

int Decoder::Data(int N_Pin,UCHAR* pmas,size_t Cnt)
{
	return 0;
};

int Decoder::OutData(int N_Pin,UCHAR* &pmas,size_t &Cnt)
{
	return 0;
};

int Decoder::UserCallBack(int	Handle,int N_Pin,UCHAR* pmas,size_t Cnt)
{
	return 0;
};

int Decoder::UserCallBackIDI(int	Handle,IDInfo* IDI, StrOwner	*Own)
{
	return 0;
};

void Decoder::Initialization()
{
};
void Decoder::ParamEx(		// Èçìåíèòü ïàðàìåòð
	char* Prm,	// Èäåíòèôèêàòîð ïàðàìåòðà
	char* Vol)	// Çíà÷åíèå ïàðàìåòðà
{
	if (!MyTryLock(__FILE__,__LINE__,&FlSim,this))
	{
		MyLock(__FILE__,__LINE__,&FlSimBuf,this);
		PrmDec	pDec;
		pDec.Set(Prm,Vol);
		if (!(Fl_BufData&8))
		{
			if (DmParam==NULL)
			{
				DmParam=new DamperPS();
			}
			Fl_BufData|=8;
		}
		DmParam->Data((UCHAR*)&pDec,sizeof(PrmDec),0);
		MyUnLock(__FILE__,__LINE__,&FlSimBuf,this);
		return;
	}
//	while (!MyTryLock(__FILE__,__LINE__,&FlSim,this))
//		Sleep(10);
	IntCalcPrm();
	Param(Prm,Vol);
	MyUnLock(__FILE__,__LINE__,&FlSim,this);
}
void Decoder::StartEx()		// Íà÷àëî ñåàíñà
{
	if (!MyTryLock(__FILE__,__LINE__,&FlSim,this))
	{
		MyLock(__FILE__,__LINE__,&FlSimBuf,this);
		PrmDec	pDec;
		pDec.Set("StartEx","On");
		if (!(Fl_BufData&8))
		{
			if (DmParam==NULL)
			{
				DmParam=new DamperPS();
			}
			Fl_BufData|=8;
		}
		DmParam->Data((UCHAR*)&pDec,sizeof(PrmDec),0);
		MyUnLock(__FILE__,__LINE__,&FlSimBuf,this);
		return;
	}
//	while (!MyTryLock(__FILE__,__LINE__,&FlSim,this))
//		Sleep(10);
	IntCalcPrm();
	Start();
	MyUnLock(__FILE__,__LINE__,&FlSim,this);
}
void Decoder::StopEx()		// Êîíåö ñåàíñà
{
	if (!MyTryLock(__FILE__,__LINE__,&FlSim,this))
	{
		MyLock(__FILE__,__LINE__,&FlSimBuf,this);
		PrmDec	pDec;
		pDec.Set("StopEx","On");
		if (!(Fl_BufData&8))
		{
			if (DmParam==NULL)
			{
				DmParam=new DamperPS();
			}
			Fl_BufData|=8;
		}
		DmParam->Data((UCHAR*)&pDec,sizeof(PrmDec),0);
		MyUnLock(__FILE__,__LINE__,&FlSimBuf,this);
		return;
	}
//	while (!MyTryLock(__FILE__,__LINE__,&FlSim,this))
//		Sleep(10);
	IntCalcPrm();
	Stop();
	MyUnLock(__FILE__,__LINE__,&FlSim,this);
}

int Decoder::DataIDI(			// Äàííûå äëÿ äåêîäèðîâàíèÿ
	int N_Pin,		// Íîìåð âõîäíîé íîæêè
	IDInfo* pIDI)		// Äëèíà äàííûõ
{
	int	rc=0;
	rc=DataFF(N_Pin,(UCHAR*)pIDI,sizeof(IDInfo));
	return rc;
}

int Decoder::BufDataIDIEx(			// Äàííûå äëÿ äåêîäèðîâàíèÿ
	int N_Pin,		// Íîìåð âõîäíîé íîæêè
	IDInfo* pIDI)		// Äëèíà äàííûõ
{
	int	rc=0;
	if (!(Fl_BufData&4))
	{
		if (DmIDInfoInput==NULL)
		{
			DmIDInfoInput=new DamperIDInfo();
		}
		Fl_BufData|=4;
	}
	rc=DmIDInfoInput->SendIDI(N_Pin,pIDI);
	return rc;
};

int Decoder::DataIDIEx(			// Äàííûå äëÿ äåêîäèðîâàíèÿ
	int N_Pin,		// Íîìåð âõîäíîé íîæêè
	IDInfo* pIDI,
	int FlagBuf)		// Äëèíà äàííûõ
{
	int	rc=0;
	switch(FlagBuf)
	{
	case 0:
		if (MyTryLock(__FILE__,__LINE__,&FlSim,this))
		{
//			Fl_ReCurse2++;
			rc=IntCalcDataIDI();
			rc=DataIDI(N_Pin,pIDI);
//			rc=IntCalcDataIDI();
//			Fl_ReCurse2--;
			MyUnLock(__FILE__,__LINE__,&FlSim,this);
		}
		else
		{
			MyLock(__FILE__,__LINE__,&FlSimBuf,this);
			if (!(Fl_BufData&4))
			{
				if (DmIDInfoInput==NULL)
				{
					DmIDInfoInput=new DamperIDInfo();
				}
				Fl_BufData|=4;
			}
			rc=DmIDInfoInput->SendIDI(N_Pin,pIDI);
			MyUnLock(__FILE__,__LINE__,&FlSimBuf,this);
		}
		break;
	case 1:
		MyLock(__FILE__,__LINE__,&FlSimBuf,this);
		if (!(Fl_BufData&4))
		{
			if (DmIDInfoInput==NULL)
			{
				DmIDInfoInput=new DamperIDInfo();
			}
			Fl_BufData|=4;
		}
		rc=DmIDInfoInput->SendIDI(N_Pin,pIDI);
		MyUnLock(__FILE__,__LINE__,&FlSimBuf,this);
		break;
	case 2:
		MyLock(__FILE__,__LINE__,&FlSim,this);
//		Fl_ReCurse2++;
		rc=IntCalcDataIDI();
		rc=DataIDI(N_Pin,pIDI);
//		rc=IntCalcDataIDI();
//		Fl_ReCurse2--;
		MyUnLock(__FILE__,__LINE__,&FlSim,this);
		break;
	}
	return rc;
}

int	Decoder::IntCalcPrm()
{
	int	rc=0;
	if ((Fl_BufData)&&(Fl_ReCurse==0))
	{	
//		IDInfo*		tIDI;
		UCHAR*		tBuf;
		size_t		tCnt;
//		int			tN_Pin;
		Fl_ReCurse++;
		if (Fl_BufData&8)
		{
			while (DmParam->LockOData(tBuf,tCnt))
			{
				PrmDec*	pDec=(PrmDec*)(tBuf+sizeof(SockHeader));
				if (strcmp(pDec->Prm,"StartEx")==0)
				{
					Start();
				}
				else
				{
					if (strcmp(pDec->Prm,"StopEx")==0)
					{
						Stop();
					}
					else
					{
						Param(pDec->Prm,pDec->Vol);
					}
				}
				DmParam->UnLockOData();
			}
			DmParam->UnLockOData();
		}
		Fl_ReCurse--;
	}
	return rc;		
};

int	Decoder::IntCalcDataIDI()
{
	int	rc=0;
	if ((Fl_BufData)&&(Fl_ReCurse==0))
	{	
		IDInfo*		tIDI;
//		UCHAR*		tBuf;
//		size_t		tCnt;
		int			tN_Pin;
		Fl_ReCurse++;
		if (Fl_BufData&4)
		{
			while ((DmIDInfoInput->GetIDI(tN_Pin,tIDI))!=NULL)	
			{
				DataIDI(tN_Pin,tIDI);
				DmIDInfoInput->DestroyIDI(tIDI);
			};
		}
		Fl_ReCurse--;
	}
	return rc;		
};

int	Decoder::IntCalcUCB()
{
	int	rc=0;
	if ((Fl_BufData)&&(Fl_ReCurse==0))
	{	
//		IDInfo*		tIDI;
		UCHAR*		tBuf;
		size_t		tCnt;
//		int			tN_Pin;
		Fl_ReCurse++;
		if (Fl_BufData&1)
		{
			while (DmData->LockOData(tBuf,tCnt))
			{
				int	tVol=(((SockHeader*)tBuf)->CntPack>>16)&0xffff;
				UserCallBack(tVol,((SockHeader*)tBuf)->CntPack&0xffff,tBuf+sizeof(SockHeader),tCnt-sizeof(SockHeader));
				DmData->UnLockOData();
			}
			DmData->UnLockOData();
		}
		Fl_ReCurse--;
	}
	return rc;		
};

int	Decoder::IntCalcUCBIDI()
{
	int	rc=0;
	if ((Fl_BufData)&&(Fl_ReCurse==0))
	{	
		IDInfo*		tIDI;
//		UCHAR*		tBuf;
//		size_t		tCnt;
		int			tN_Pin;
		Fl_ReCurse++;
		if (Fl_BufData&2)
		{
			while ((DmIDInfoUCB->GetIDI(tN_Pin,tIDI))!=NULL)	
			{
				void*		tVol=NULL;
				StrOwner*	Own;
				tIDI->pPList->GetParam(IDStrOwner,(void*)tVol);
				Own=(StrOwner*)tVol;
				if ((tN_Pin==1)&&(tIDI->ID_SeansFrom==TabNeuronGlobal))
				{
					El_Tab* tElT=(El_Tab*)Srv->IDT_NeuronGlobal->IDL->Find((UCHAR*)&tIDI->ID_File,sizeof(INT64));
					if (tElT!=NULL)
					{
						if(tElT->Fl_Del!=1)
							UserCallBackIDI(tN_Pin,tIDI,Own);
						else
						{
							if((tIDI->Command&Command_DEL) || (tIDI->Command&Command_MOVE))
								UserCallBackIDI(tN_Pin,tIDI,Own);
						}
					}
					else
					{
						if((tIDI->Command&Command_DEL) || (tIDI->Command&Command_MOVE))
							UserCallBackIDI(tN_Pin,tIDI,Own);
					}
				}
				else
					UserCallBackIDI(tN_Pin,tIDI,Own);
				DmIDInfoUCB->DestroyIDI(tIDI);
			};
		}
		Fl_ReCurse--;
	}
	return rc;		
};

int	Decoder::IntCalc()
{
	int	rc=0;
	if ((Fl_BufData)&&(Fl_ReCurse==0))
	{	
		IDInfo*		tIDI;
		UCHAR*		tBuf;
		size_t		tCnt;
		int			tN_Pin;
		Fl_ReCurse++;
		if (Fl_BufData&8)
		{
			while (DmParam->LockOData(tBuf,tCnt))
			{
				PrmDec*	pDec=(PrmDec*)(tBuf+sizeof(SockHeader));
				if (strcmp(pDec->Prm,"StartEx")==0)
				{
					Start();
				}
				else
				{
					if (strcmp(pDec->Prm,"StopEx")==0)
					{
						Stop();
					}
					else
					{
						Param(pDec->Prm,pDec->Vol);
					}
				}
				DmParam->UnLockOData();
			}
			DmParam->UnLockOData();
		}
		if (Fl_BufData&1)
		{
			while (DmData->LockOData(tBuf,tCnt))
			{
				int	tVol=(((SockHeader*)tBuf)->CntPack>>16)&0xffff;
//				Fl_ReCurse2++;
				UserCallBack(tVol,((SockHeader*)tBuf)->CntPack&0xffff,tBuf+sizeof(SockHeader),tCnt-sizeof(SockHeader));
//				Fl_ReCurse2--;
				DmData->UnLockOData();
			}
			DmData->UnLockOData();
		}
		if (Fl_BufData&2)
		{
			while ((DmIDInfoUCB->GetIDI(tN_Pin,tIDI))!=NULL)	
			{
				void*		tVol=NULL;
				StrOwner*	Own;
				tIDI->pPList->GetParam(IDStrOwner,(void*)tVol);
				Own=(StrOwner*)tVol;
//				Fl_ReCurse2++;
//				UserCallBackIDI(tN_Pin,tIDI,Own);
				if ((tN_Pin==1)&&((tIDI->ID_SeansFrom==TabNeuronGlobal)||(tIDI->ID_SeansFrom==TabNeuronLocalCustomer)||(tIDI->ID_SeansFrom==TabNeuronLocalSite)))
				{
					El_Tab* tElT=(El_Tab*)Srv->IDT_NeuronGlobal->IDL->Find((UCHAR*)&tIDI->ID_File,sizeof(INT64));
					if (tElT==NULL)
						tElT=(El_Tab*)Srv->IDT_NeuronLocalCustomer->IDL->Find((UCHAR*)&tIDI->ID_File,sizeof(INT64));
					if (tElT==NULL)
						tElT=(El_Tab*)Srv->IDT_NeuronLocalSite->IDL->Find((UCHAR*)&tIDI->ID_File,sizeof(INT64));
					if (tElT!=NULL)
					{
						if (tElT->Fl_Del!=1)
							UserCallBackIDI(tN_Pin,tIDI,Own);
					}

				}
				else
					UserCallBackIDI(tN_Pin,tIDI,Own);
//				Fl_ReCurse2--;
				DmIDInfoUCB->DestroyIDI(tIDI);
			};
		}
		if (Fl_BufData&4)
		{
			while ((DmIDInfoInput->GetIDI(tN_Pin,tIDI))!=NULL)	
			{
				DataIDI(tN_Pin,tIDI);
				DmIDInfoInput->DestroyIDI(tIDI);
			};
		}
		Fl_ReCurse--;
	}
	return rc;		
};

int Decoder::DataEx(int N_Pin,UCHAR* pmas,size_t Cnt)
{
	int	rc;
	MyLock(__FILE__,__LINE__,&FlSim,this);
//	Fl_ReCurse2++;
//	rc=IntCalc();
	//
	rc=Data(N_Pin,pmas,Cnt);
//	Fl_ReCurse2--;
	MyUnLock(__FILE__,__LINE__,&FlSim,this);
	return rc;
};

int Decoder::ODataFFEx(				// Âûõîäíûå äàííûå
	int N_Pin,			// Íîìåð âûõîäíîé íîæêè
	UCHAR* pmas,		// Óêàçàòåëü íà áóôåð ñ äàííûìè
	size_t Cnt)			// Óêàçàòåëü íà äëèíó äàííûõ
{
	int rc;
	MyLock(__FILE__,__LINE__,&FlSim,this);
//	rc=IntCalc();
	rc=ODataFF(N_Pin, pmas, Cnt);
//	rc=IntCalc();
	MyUnLock(__FILE__,__LINE__,&FlSim,this);
	return rc;
}

int Decoder::ODataIDIEx(				// Âûõîäíûå äàííûå
	int N_Pin,			// Íîìåð âûõîäíîé íîæêè
	IDInfo* pIDI,int FlBuf)			// Óêàçàòåëü íà äëèíó äàííûõ
{
	int rc;
	MyLock(__FILE__,__LINE__,&FlSim,this);
	rc=ODataIDI(N_Pin, pIDI,FlBuf);
//	rc=IntCalc();
	MyUnLock(__FILE__,__LINE__,&FlSim,this);
	return rc;
}

int Decoder::OutDataEx(int N_Pin,UCHAR* &pmas,size_t &Cnt)
{
	int	rc;
	MyLock(__FILE__,__LINE__,&FlSim,this);
	rc=OutData(N_Pin,pmas,Cnt);
// 	if (Cnt==0)
// 	{
// 		rc=IntCalc();
// 	}
	MyUnLock(__FILE__,__LINE__,&FlSim,this);
	return rc;
};

int Decoder::DataFFEx(int N_Pin,UCHAR* pmas,size_t Cnt)
{
	int rc=0;
	int i=0;
//	if (Cnt==sizeof(IDInfo))
//	{
//		return DataIDIEx(N_Pin,(IDInfo*)pmas);
//	}
// 	if (Fl_Buf)
// 	{
// 		while (i<100)
// 		{
// 			i++;
// 			if (MyTryLock(__FILE__,__LINE__,&FlSim,this))	// !!!!!!!!!!!!!!
// 			{
// 	//			MyLock(__FILE__,__LINE__,&FlSim,this);
// 				//	Fl_ReCurse2++;
// 				//	rc=IntCalc();
// 				rc=DataFF( N_Pin,pmas,Cnt);
// 				//	Fl_ReCurse2--;
// 				MyUnLock(__FILE__,__LINE__,&FlSim,this);
// 				break;
// 			}
// 		}
// 	}
// 	else
// 	{
		MyLock(__FILE__,__LINE__,&FlSim,this);
		rc=DataFF( N_Pin,pmas,Cnt);
		MyUnLock(__FILE__,__LINE__,&FlSim,this);
//	}
	return rc;
};

int Decoder::DataFF(int N_Pin,UCHAR* pmas,size_t Cnt)
{
	if (Data(N_Pin,pmas,Cnt)!=0)	//
	{
		int Fl_Pin=1;
		while (Fl_Pin!=0)
		{
			Fl_Pin=0;
			for (int Pin=0;Pin<Num_PinO;Pin++)
			{
				NC2=GetNClk();
				NCD+=NC2-NC1;
				if (OutData(Pin,tBuf,tCnt/*,tNStrim*/)!=0)
				{
					NC1=GetNClk();
					NCO+=GetNClk()-NC2;
					Fl_Pin|=1;
					ODataFF(Pin,tBuf,tCnt/*,tNStrim*/);
				}
				else
				{
					NC1=GetNClk();
					NCO+=GetNClk()-NC2;
				}
			}
		}
	}
	return 0;
};

void	Decoder::InitDataCB(int (CALLBACK *pOD)(int N_Pin,unsigned char *pData,size_t Cnt,Decoder* pDec,Decoder* pOwnDec),Decoder* pDec)
{
	pODataFFF=pOD;
	pDecODataFFF=pDec;
};

int Decoder::ODataFF(int N_Pin,UCHAR* pmas,size_t Cnt)
{
//	
//	MyUnLock(__FILE__,__LINE__,&FlSim,this);
//	
	if (pODataFFF)
	{
		pODataFFF(N_Pin,pmas,Cnt,pDecODataFFF,this);
	}
	else
	{
		for (size_t i=0;i<NumLink;i++)
		{
			if ((TabDecLink[i].pObj)&&(TabDecLink[i].PinO==N_Pin))
			{
				char	vBuf[128];
				NC2=GetNClk();
				NCD+=NC2-NC1; 
				TabDecLink[i].pObj->NC1=NC2;
				if (TabDecLink[i].PinI<0)
				{
					switch(TabDecLink[i].PinI)
					{
					case -1:
						sprintf(vBuf," >%s:Sp",TabDecLink[i].pObj->Name_Mt);
						Srv->AddDebugLevel(this,vBuf);
						TabDecLink[i].pObj->StopEx();
						Srv->SubDebugLevel(this);
						break;
					case -2:
						sprintf(vBuf," >%s:St",TabDecLink[i].pObj->Name_Mt);
						Srv->AddDebugLevel(this,vBuf);
						TabDecLink[i].pObj->StartEx();
						Srv->SubDebugLevel(this);
						break;
					case -3:
						sprintf(vBuf," >%s:Pr",TabDecLink[i].pObj->Name_Mt);
						Srv->AddDebugLevel(this,vBuf);
						if (Cnt>40)
							TabDecLink[i].pObj->ParamEx((char*)pmas,(char*)pmas+40);
						Srv->SubDebugLevel(this);
						break;
					}
				}
				else
				{
					sprintf(vBuf," >%s:O%d",TabDecLink[i].pObj->Name_Mt,N_Pin);
					Srv->AddDebugLevel(this,vBuf);
					TabDecLink[i].pObj->DataFFEx(TabDecLink[i].PinI,pmas,Cnt);
					Srv->SubDebugLevel(this);
				}
				if (TabDecLink[i].pObj)
				{
					TabDecLink[i].pObj->NC2=GetNClk();
					TabDecLink[i].pObj->NCD+=TabDecLink[i].pObj->NC2-TabDecLink[i].pObj->NC1;
					NC1=TabDecLink[i].pObj->NC2;
				}
			}
		}
	}
//	
//	MyLock(__FILE__,__LINE__,&FlSim,this);
//	
	return 0;
};
int Decoder::ODataIDI(int N_Pin,IDInfo* pIDI,int FlBuf)
{
//	
//	MyUnLock(__FILE__,__LINE__,&FlSim,this);
//	
	for (size_t i=0;i<NumLink;i++)
	{
		if ((TabDecLink[i].pObj)&&(TabDecLink[i].PinO==N_Pin))
		{
			char	vBuf[128];
			NC2=GetNClk();
			NCD+=NC2-NC1; 
			TabDecLink[i].pObj->NC1=NC2;
			if (TabDecLink[i].PinI<0)
			{
				switch(TabDecLink[i].PinI)
				{
				case -1:
					sprintf(vBuf," >%s:Sp",TabDecLink[i].pObj->Name_Mt);
					Srv->AddDebugLevel(this,vBuf);
					TabDecLink[i].pObj->StopEx();
					Srv->SubDebugLevel(this);
					break;
				case -2:
					sprintf(vBuf," >%s:St",TabDecLink[i].pObj->Name_Mt);
					Srv->AddDebugLevel(this,vBuf);
					TabDecLink[i].pObj->StartEx();
					Srv->SubDebugLevel(this);
					break;
				}
			}
			else
			{
				sprintf(vBuf," >%s:O%d",TabDecLink[i].pObj->Name_Mt,N_Pin);
				Srv->AddDebugLevel(this,vBuf);
				TabDecLink[i].pObj->DataIDIEx(TabDecLink[i].PinI,pIDI,FlBuf);
				Srv->SubDebugLevel(this);
			}
			if (TabDecLink[i].pObj)
			{
				TabDecLink[i].pObj->NC2=GetNClk();
				TabDecLink[i].pObj->NCD+=TabDecLink[i].pObj->NC2-TabDecLink[i].pObj->NC1;
				NC1=TabDecLink[i].pObj->NC2;
			}
		}
	}
//	
//	MyLock(__FILE__,__LINE__,&FlSim,this);
//	
	return 0;
}
/*
int Decoder::BufDataFF(			// Äàííûå äëÿ äåêîäèðîâàíèÿ
					  int N_Pin,		// Íîìåð âõîäíîé íîæêè
					  UCHAR* pmas,	// Óêàçàòåëü íà áóôåð ñ äàííûìè
					  size_t Cnt)		// Äëèíà äàííûõ
{
	if (!(Fl_BufData&1))
	{
		if (DmData==NULL)
		{
			DmData=new DamperPS();
		}
		Fl_BufData|=1;
	}
	DmData->Data(pmas,Cnt,N_Pin);
	return 0;
}
int Decoder::BufODataFF(				// Âûõîäíûå äàííûå
					   int N_Pin,			// Íîìåð âûõîäíîé íîæêè
					   UCHAR* pmas,		// Óêàçàòåëü íà áóôåð ñ äàííûìè
					   size_t Cnt)			// Óêàçàòåëü íà äëèíó äàííûõ
{
	for (size_t i=0;i<NumLink;i++)
	{
		if ((TabDecLink[i].pObj)&&(TabDecLink[i].PinO==N_Pin))
		{
			char	vBuf[128];
			NC2=GetNClk();
			NCD+=NC2-NC1;
			TabDecLink[i].pObj->NC1=NC2;
			if (TabDecLink[i].PinI<0)
			{
				switch(TabDecLink[i].PinI)
				{
				case -1:
					sprintf(vBuf," >%s:Sp",TabDecLink[i].pObj->Name_Mt);
					Srv->AddDebugLevel(this,vBuf);
					TabDecLink[i].pObj->Stop();
					Srv->SubDebugLevel(this);
					break;
				case -2:
					sprintf(vBuf," >%s:St",TabDecLink[i].pObj->Name_Mt);
					Srv->AddDebugLevel(this,vBuf);
					TabDecLink[i].pObj->Start();
					Srv->SubDebugLevel(this);
					break;
				case -3:
					sprintf(vBuf," >%s:Pr",TabDecLink[i].pObj->Name_Mt);
					Srv->AddDebugLevel(this,vBuf);
					if (Cnt>40)
						TabDecLink[i].pObj->Param((char*)pmas,(char*)pmas+40);
					Srv->SubDebugLevel(this);
					break;
				}
			}
			else
			{
				sprintf(vBuf," >%s:O%d",TabDecLink[i].pObj->Name_Mt,N_Pin);
				Srv->AddDebugLevel(this,vBuf);
				TabDecLink[i].pObj->BufDataFF(TabDecLink[i].PinI,pmas,Cnt);
				Srv->SubDebugLevel(this);
			}
			if (TabDecLink[i].pObj)
			{
				TabDecLink[i].pObj->NC2=GetNClk();
				TabDecLink[i].pObj->NCD+=TabDecLink[i].pObj->NC2-TabDecLink[i].pObj->NC1;
				NC1=TabDecLink[i].pObj->NC2;
			}
		}
	}
	return 0;
}

int Decoder::BufDataFF(			// Äàííûå äëÿ äåêîäèðîâàíèÿ
					  int N_Pin,		// Íîìåð âõîäíîé íîæêè
					  IDInfo* IDI)	// Óêàçàòåëü íà áóôåð ñ äàííûìè
{
	if (!(Fl_BufData&2))
	{
		if (DmIDInfo==NULL)
		{
			DmIDInfo=new DamperIDInfo();
		}
		Fl_BufData|=2;
	}
//	IDI->pPList->SetParam(IDN_Pin,&N_Pin,sizeof(N_Pin),this);
	DmIDInfo->SendIDI(N_Pin,IDI);
	return 0;
};

int Decoder::BufODataFF(				// Âûõîäíûå äàííûå
					   int N_Pin,			// Íîìåð âûõîäíîé íîæêè
					   IDInfo* IDI)		// Óêàçàòåëü íà áóôåð ñ äàííûìè
{
	for (size_t i=0;i<NumLink;i++)
	{
		if ((TabDecLink[i].pObj)&&(TabDecLink[i].PinO==N_Pin))
		{
			char	vBuf[128];
			NC2=GetNClk();
			NCD+=NC2-NC1;
			TabDecLink[i].pObj->NC1=NC2;
			if (TabDecLink[i].PinI>=0)
			{
				sprintf(vBuf," >%s:O%d",TabDecLink[i].pObj->Name_Mt,N_Pin);
				Srv->AddDebugLevel(this,vBuf);
				TabDecLink[i].pObj->BufDataFF(TabDecLink[i].PinI,IDI);
				Srv->SubDebugLevel(this);
			}
			if (TabDecLink[i].pObj)
			{
				TabDecLink[i].pObj->NC2=GetNClk();
				TabDecLink[i].pObj->NCD+=TabDecLink[i].pObj->NC2-TabDecLink[i].pObj->NC1;
				NC1=TabDecLink[i].pObj->NC2;
			}
		}
	}
	return 0;
};
*/
int Decoder::BufUserCallBackEx(	// Ôóíêöèÿ îáðàòíîãî âûçîâà
	int	Handle,		// Èäåíòèôèêàòîð ñîáûòèÿ
	int N_Pin,		// Íîìåð âõîäíîé íîæêè (ðàñøèðåíèå äëÿ îïðåäåëåíèÿ íåñêîëüêèõ ôóíêöèé)
	UCHAR* pmas,	// Óêàçàòåëü íà áóôåð ñ äàííûìè
	size_t Cnt)
{
	int rc=0;
	int	tVol=N_Pin+(Handle<<16);
	if (!(Fl_BufData&1))
	{
		if (DmData==NULL)
		{
			DmData=new DamperPS();
		}
		Fl_BufData|=1;
	}
	if (DmData)
		rc=DmData->Data(pmas,Cnt,tVol);
	return rc;
}

int Decoder::UserCallBackEx(	// Ôóíêöèÿ îáðàòíîãî âûçîâà
	int	Handle,		// Èäåíòèôèêàòîð ñîáûòèÿ
	int N_Pin,		// Íîìåð âõîäíîé íîæêè (ðàñøèðåíèå äëÿ îïðåäåëåíèÿ íåñêîëüêèõ ôóíêöèé)
	UCHAR* pmas,	// Óêàçàòåëü íà áóôåð ñ äàííûìè
	size_t Cnt,
	int FlagBuf)		// Äëèíà äàííûõ
{
	int rc=0;
	switch(FlagBuf)
	{
	case 0:
		if (MyTryLock(__FILE__,__LINE__,&FlSim,this))
		{
			rc=IntCalcUCB();
			rc=UserCallBack(Handle, N_Pin, pmas, Cnt);
//			rc=IntCalcUCB();
			MyUnLock(__FILE__,__LINE__,&FlSim,this);
		}
		else
		{
			MyLock(__FILE__,__LINE__,&FlSimBuf,this);
			int	tVol=N_Pin+(Handle<<16);
			if (!(Fl_BufData&1))
			{
				if (DmData==NULL)
				{
					DmData=new DamperPS();
				}
				Fl_BufData|=1;
			}
			if (DmData)
				rc=DmData->Data(pmas,Cnt,tVol);
			MyUnLock(__FILE__,__LINE__,&FlSimBuf,this);
		}
		break;
	case 1:
		{
			MyLock(__FILE__,__LINE__,&FlSimBuf,this);
			int	tVol=N_Pin+(Handle<<16);
			if (!(Fl_BufData&1))
			{
				if (DmData==NULL)
				{
					DmData=new DamperPS();
				}
				Fl_BufData|=1;
			}
			if (DmData)
				rc=DmData->Data(pmas,Cnt,tVol);
			MyUnLock(__FILE__,__LINE__,&FlSimBuf,this);
		}
		break;
	case 2:
		MyLock(__FILE__,__LINE__,&FlSim,this);
		rc=IntCalcUCB();
		rc=UserCallBack(Handle, N_Pin, pmas, Cnt);
//		rc=IntCalcUCB();
		MyUnLock(__FILE__,__LINE__,&FlSim,this);
		break;
	case 3:
		rc=UserCallBack(Handle, N_Pin, pmas, Cnt);
		break;
	}
	return rc;
}
int Decoder::BufUserCallBackIDIEx(int	Handle, IDInfo* IDI,	StrOwner	*Own)
{
	int rc=0;
	if (!(Fl_BufData&2))
	{
		if (DmIDInfoUCB==NULL)
		{
			DmIDInfoUCB=new DamperIDInfo();
		}
		Fl_BufData|=2;
	}
	if (Own) IDI->pPList->SetParam(IDStrOwner,(void*)Own,sizeof(StrOwner),this);
	if (DmIDInfoUCB)
		rc=DmIDInfoUCB->SendIDI(Handle,IDI);
	return rc;
};

int Decoder::UserCallBackIDIEx(	// Ôóíêöèÿ îáðàòíîãî âûçîâà
	int	Handle,		// Èäåíòèôèêàòîð ñîáûòèÿ
	IDInfo* IDI,
	StrOwner	*Own,
	int FlagBuf)		// Äëèíà äàííûõ
{
	int rc=0;
	//  [11/30/2011 San]

/*
	if(FlagBuf==1)//???
		FlagBuf=0;
*/

	switch (FlagBuf)
	{
	case 0:
		if (MyTryLock(__FILE__,__LINE__,&FlSim,this))
		{
			rc=IntCalcUCBIDI();
			rc=UserCallBackIDI(Handle,IDI,Own);
			//			rc=IntCalcUCBIDI();
			MyUnLock(__FILE__,__LINE__,&FlSim,this);
		}
		else
		{
			MyLock(__FILE__,__LINE__,&FlSimBuf,this);
			if (!(Fl_BufData&2))
			{
				if (DmIDInfoUCB==NULL)
				{
					DmIDInfoUCB=new DamperIDInfo();
				}
				Fl_BufData|=2;
			}
			if (Own) IDI->pPList->SetParam(IDStrOwner,(void*)Own,sizeof(StrOwner),this);
			if (DmIDInfoUCB)
				rc=DmIDInfoUCB->SendIDI(Handle,IDI);
			MyUnLock(__FILE__,__LINE__,&FlSimBuf,this);
		}
		break;
	case 1:
		MyLock(__FILE__,__LINE__,&FlSimBuf,this);
		if (!(Fl_BufData&2))
		{
			if (DmIDInfoUCB==NULL)
			{
				DmIDInfoUCB=new DamperIDInfo();
			}
			Fl_BufData|=2;
		}
		if (Own) IDI->pPList->SetParam(IDStrOwner,(void*)Own,sizeof(StrOwner),this);
		if (DmIDInfoUCB)
			rc=DmIDInfoUCB->SendIDI(Handle,IDI);
		MyUnLock(__FILE__,__LINE__,&FlSimBuf,this);
		break;
	case 2:
		MyLock(__FILE__,__LINE__,&FlSim,this);
		rc=IntCalcUCBIDI();
		rc=UserCallBackIDI(Handle,IDI,Own);
//		rc=IntCalcUCBIDI();
		MyUnLock(__FILE__,__LINE__,&FlSim,this);
		break;
	}
	return rc;
}


int Decoder::IdleFunc(int NumberTr)			// Idle function  if rc!=0 recall function (?)  NumberTr=-1 System NumberTr+0x8000 - DinTr
{
	int	rc=ChildIdleFunc(NumberTr);
	return rc;
}

int Decoder::ChildIdleFunc(int NumberTr)			// Idle function  if rc!=0 recall function (?) 
{
	int	rc=0;
// 	if (!MyTryLock(__FILE__,__LINE__,&FlSim,this))
// 	{
// 		return rc;
// 	}
	FindStrList8	FSL8;
	ID_ElemPTR	*tEl=(ID_ElemPTR*)ChildList.Begin(FSL8);
	while (tEl)
	{
//		((Decoder*)(tEl->Vol))->DataIdle();
		if (((Decoder*)(tEl->Vol))->UInfo.TractID==NumberTr)
		{
			if (MyTryLock(__FILE__,__LINE__,&((Decoder*)(tEl->Vol))->FlSim,this))
			{
				rc+=((Decoder*)(tEl->Vol))->IdleFunc(NumberTr);
				MyUnLock(__FILE__,__LINE__,&((Decoder*)(tEl->Vol))->FlSim,this);
			}
		}
		else
		{
			rc=-1;
		}
		tEl=(ID_ElemPTR*)ChildList.Next(FSL8);
	}
//	Fl_ReCurse2++;
	IntCalc();
//	Fl_ReCurse2--;
//	MyUnLock(__FILE__,__LINE__,&FlSim,this);
	return rc;
};

int	Decoder::AddChildMt(Decoder* pDec)
{
	int	rc;
	ID_ElemPTR	*tEl=new ID_ElemPTR((LPVOID)pDec);
	rc=ChildList.Add(tEl);
	if (rc==-1)
		delete tEl;
	return rc;
};

int	Decoder::DelChildMt(Decoder* pDec)
{
	int	rc=-1;
	ID_ElemPTR	*tEl=(ID_ElemPTR*)ChildList.Find((UCHAR*)&pDec,sizeof(LPVOID));
	if (tEl)	
		rc=ChildList.Del(tEl);
	return rc;
};

/*
int Decoder::DataIdle(void)			// Buffered data
{
	if(Fl_BufData)
	{	
		IDInfo*	tIDI;
		UCHAR*	tBuf;
		size_t		tCnt;
		int		tN_Pin;
		if (Fl_BufData&1)
		{
			while (DmData->LockOData(tBuf,tCnt))
			{
				MyLock(__FILE__,__LINE__,&FlSim,this);
				DataFF(((SockHeader*)tBuf)->CntPack,tBuf+sizeof(SockHeader),tCnt-sizeof(SockHeader));
				MyUnLock(__FILE__,__LINE__,&FlSim,this);
				DmData->UnLockOData();
			}
			DmData->UnLockOData();
		}
		if (Fl_BufData&2)
		{
			while ((DmIDInfo->GetIDI(tN_Pin,tIDI))!=NULL)	
			{
				MyLock(__FILE__,__LINE__,&FlSim,this);
				DataFF(tN_Pin,(UCHAR*)tIDI,sizeof(IDInfo));
				MyUnLock(__FILE__,__LINE__,&FlSim,this);
				DmIDInfo->DestroyIDI(tIDI);
			};
		}
	}
	return 0;
};
*/
// --------------------------------------------------------


char* Decoder::GetInfoStr(char* Str)
{
	return 0;
};

int	Decoder::GetInfoMethod(InfoMt* InfM)
{
	InfM->Name=Name_Mt;InfM->Rem=Rem_Mt;InfM->Ver=Ver_;InfM->Auth=Auth_;
	InfM->NPinI=Num_PinI;InfM->NPinO=Num_PinO;InfM->NParam=Num_Prm;
	InfM->Type=Type_Mt;
	int	DigitSize=32;
#ifdef _WIN64
	DigitSize=64;
#endif
#ifdef _DEBUG
	sprintf (InfM->SourceType,"D[%d]",DigitSize);
#else
	sprintf (InfM->SourceType,"R[%d]",DigitSize);
#endif
	return 0;
};
int	Decoder::GetInfoPinIn(int Num,InfoPin* InfP)
{
	if (Num>=Num_PinI) return -1;
	InfP->Name=PinName_I[2*Num];InfP->Rem=PinName_I[2*Num+1];
	return 0;
};
int	Decoder::GetInfoPinOut(int Num,InfoPin* InfP)
{
	if (Num>=Num_PinO) return -1;
	InfP->Name=PinName_O[2*Num];InfP->Rem=PinName_O[2*Num+1];
	return 0;
};
int	Decoder::GetInfoParam(int Num,InfoPrm* InfP)
{
	size_t i=0,j=0;
	if (Num>=Num_Prm) return -1;
	InfP->Name=Prm_[2*Num];InfP->RemPrm=Prm_[2*Num+1];
	// Ôîðìèðîâàíèå çíà÷åíèé äàííîãî ïàðàìåòðà
	while (Vol_[3*j][0]!=0x0)
		{
		if (strcmp(Prm_[2*Num],Vol_[3*j])==0)
		  { InfP->Vol[i]=Vol_[3*j+1];InfP->Rem[i]=Vol_[3*j+2];i++;};
		;
		j++;
		};
		InfP->Vol[i]=Vol_[3*j];
		InfP->Rem[i]=Vol_[3*j];
	return 0;
};
void*	Decoder::malloc(size_t size)
{	
	char	ttb[128];
	void*	tB=::malloc(size);
	if (tB!=NULL)
	{
		SizeAllocMem+=_msize(tB);
		DBGC.MemSize+=_msize(tB);
		if (SizeAllocMem>10240000)
		{
			sprintf(ttb,"%s_SizeAllocMem",Name_Mt);
			Srv->Telemetr(this,ttb,(double)SizeAllocMem);
		}
		sprintf(ttb,"%08X Alloc=%d(%d) All=%d Mt=%s",tB,_msize(tB),size,SizeAllocMem,Name_Mt);
		Srv->OutLog(ttb,this,"Memory");
	}
	else
	{
		sprintf(ttb,"Error Alloc=%d All=%d Mt=%s",size,SizeAllocMem,Name_Mt);
		Srv->OutLog(ttb,this,"Memory");
	}
	return tB;
}
void*	Decoder::realloc(void*tBuf,size_t size)
{	
	char	ttb[128];
	size_t	tSize=_msize(tBuf);
	void*	tB=::realloc(tBuf,size);
	if (tB!=NULL)
	{
		if (SizeAllocMem==0)
			SizeAllocMem=tSize;
		SizeAllocMem-=tSize;
		DBGC.MemSize-=tSize;
		if (SizeAllocMem<0)
		{
			Srv->OutLog("ErrorReAllocMemSize",this,"Memory");
			SizeAllocMem=0;
			Srv->SetCriticalError(this,"ErrorReAllocMemSize","Error");
		}
		SizeAllocMem+=_msize(tB);
		DBGC.MemSize+=_msize(tB);
		if (SizeAllocMem>10240000)
		{
			sprintf(ttb,"%s_SizeAllocMem",Name_Mt);
			Srv->Telemetr(this,ttb,(double)SizeAllocMem);
		}
		sprintf(ttb,"%08X->%08X ReAlloc=%d(%d) Old=%d All=%d Mt=%s",tBuf,tB,size,_msize(tB),tSize,SizeAllocMem,Name_Mt);
		Srv->OutLog(ttb,this,"Memory");
	}
	else
	{
		sprintf(ttb,"Error ReAlloc=%d All=%d Mt=%s",size,SizeAllocMem,Name_Mt);
		Srv->OutLog(ttb,this,"Memory");
	}
	return tB;
}
void	Decoder::free(void* tB)
{	
	if (tB!=NULL)
	{
		char	ttb[128];
		if (SizeAllocMem==0)
			SizeAllocMem=_msize(tB);
		SizeAllocMem-=_msize(tB);
		DBGC.MemSize-=_msize(tB);
		if (SizeAllocMem<0)
		{
			char	ttBuf[128];
			sprintf(ttBuf,"ErrorFreeMemSize=%d",SizeAllocMem);
			Srv->OutLog(ttBuf,this,"Memory");
			Srv->SetCriticalError(this,ttBuf,"Error");
			SizeAllocMem=0;
		}
		sprintf(ttb,"%08X Free=%d All=%d Mt=%s",tB,_msize(tB),SizeAllocMem,Name_Mt);
		Srv->OutLog(ttb,this,"Memory");
		::free(tB);
	}
}

int	Decoder::AddDecLink(Decoder* pDec,int PinO,int PinI)
{
	int	rc=0;

	if (NumLink>=512)
	{
		Srv->SetCriticalError(this,"To many linck's for method!","Error");
		rc=-1;
	}
	else
	{
		TabDecLink[NumLink].pObj=pDec;
		TabDecLink[NumLink].PinO=PinO;
		TabDecLink[NumLink].PinI=PinI;
		strcpy(TabDecLink[NumLink].MtName,pDec->Name_Mt);
		rc=NumLink++;
	}
// 	if (NumLink>=512)
// 	{
// 		Srv->SetCriticalError(this,"To many linck's for method!","Error");
// 		rc=-1;
// 	}
	return rc;
};

int	Decoder::DelDecLink(Decoder* pDec)
{
	int	rc=0;
	size_t i,j;
	for (i=0;i<NumLink;i++)
	{
		if (TabDecLink[i].pObj==pDec)
		{
			NumLink--;
			TabDecLink[i].pObj=NULL;
			for (j=i;j<NumLink;j++)
			{
				TabDecLink[j].pObj=TabDecLink[j+1].pObj;
				TabDecLink[j].PinO=TabDecLink[j+1].PinO;
				TabDecLink[j].PinI=TabDecLink[j+1].PinI;
			};
			i--;
		};

	}
	return rc;
};

int	Decoder::DelAllDecLink()
{
	int	rc=0;
	NumLink=0;
	return rc;
}

StrChron* Chronometry::Measuring(int command,uint NumIntervals)	// command 0 - begin 1 - end, NumIntervals 0,1,2,...,9
{
	StrChron	*SCh=&StrCh[NumIntervals];
	INT64		tTime;
	if (NumIntervals<10)
	{
		switch(command)
		{
		case 0:
			tTime=GetNClk();
			SCh->T=tTime-SCh->t0;
			if (SCh->t0)
			{
				if (SCh->T_m<SCh->T)
				{
					SCh->T_m=SCh->T;SCh->T_max=(double)SCh->T_m/(double)Srv->Fcpu;
				}
				SCh->T_average=0.99*SCh->T_average+0.01*(double)SCh->T/(double)Srv->Fcpu;
			}
			SCh->t0=tTime;
			break;
		case 1:
			SCh->t1=GetNClk();
			SCh->t=SCh->t1-SCh->t0;
			if (SCh->t_m<SCh->t) {SCh->t_m=SCh->t;SCh->t_max=(double)SCh->t_m/(double)Srv->Fcpu;}
			SCh->t_average=0.99*SCh->t_average+0.01*(double)SCh->t/(double)Srv->Fcpu;
			if (SCh->T_average) SCh->porosity=100*SCh->t_average/SCh->T_average;
			break;
		}
	}
	else
		SCh=NULL;
	return SCh;
};


ClassThread::ClassThread(char*	Name,size_t Priority)
{
	char	tBuf[256];
	strcpy(NameThread,Name);
	On=1;//CntV=0;
	pFun=NULL;
	pObj=NULL;
	tTime=GetNClk();
	Loads=0;
	hThrd=CreateThread(
			   NULL,
			   64*1024,
			   (LPTHREAD_START_ROUTINE)ThreadFun,
			   (LPVOID)this,
			   CREATE_SUSPENDED,
			   &IDThread);
	ResumeThread(hThrd);
	Srv=GetServices();
	if (Srv)
	{
		//sprintf(tBuf,"Create Thread:%s=%d",NameThread,hThrd);
		sprintf(tBuf,"Create Thread:%s",NameThread);
		Srv->OutLog(tBuf,NULL,"ClassThread");
	}
	if (Priority!=NULL)
		SetThreadPriority(hThrd,Priority);	// THREAD_PRIORITY_HIGHEST
	Fl_Stop = 0;
//	TRACE1("Create Thread=%d\n",IDThread);
};

ClassThread::ClassThread(char*	Name,int (*Fun)(LPVOID) ,LPVOID Prm,size_t Priority)
{
	char	tBuf[256];
	strcpy(NameThread,Name);
	On=1;//CntV=0;
	pFun=Fun;
	pObj=Prm;
	tTime=GetNClk();
	Load_t=Load_T=Loads=0;
	hThrd=CreateThread(
			   NULL,
			   64*1024,
			   (LPTHREAD_START_ROUTINE)ThreadFun,
			   (LPVOID)this,
			   CREATE_SUSPENDED|THREAD_TERMINATE,
			   &IDThread);
	ResumeThread(hThrd);
	Srv=GetServices();
	if (Srv)
	{	
		//sprintf(tBuf,"Create Thread:%s=%d",NameThread,hThrd);
		sprintf(tBuf,"Create Thread:%s",NameThread);
		Srv->OutLog(tBuf,NULL,"ClassThread");
	}
	if (Priority!=NULL)
		SetThreadPriority(hThrd,Priority);
	Fl_Stop = 0;
//	TRACE1("Create Thread=%x\n",IDThread);
};

int	ClassThread::Calc()
{
	INT64	tTime_1;
	INT64	tTime_2;
	int	rc=0;
	MyLock(__FILE__, __LINE__, &Fl_Th, NULL);
	if (Fl_Stop != 1)
	{
		if (pFun == NULL) return 0;
		tTime_1 = GetNClk();
		rc = pFun(pObj);
		tTime_2 = GetNClk();
		Load_t = 0.99*Load_t + (double)(tTime_2 - tTime_1);
		Load_T = 0.99*Load_T + (double)(tTime_1 - tTime);
		//	Loads=0.99*Loads+(double)(tTime2-tTime1)/(double)(tTime2-tTime);
		Loads = 100.*(Load_t / (Load_T + Load_t));
		tTime = tTime_2;
	}
	MyUnLock(__FILE__, __LINE__, &Fl_Th, NULL);
	return rc;
};

void	ClassThread::Stop()
{
	MyLock(__FILE__, __LINE__, &Fl_Th, NULL);
	Fl_Stop = 1;
	MyUnLock(__FILE__, __LINE__, &Fl_Th, NULL);
};

void	ClassThread::Start()
{
	MyLock(__FILE__, __LINE__, &Fl_Th, NULL);
	Fl_Stop = 0;
	MyUnLock(__FILE__, __LINE__, &Fl_Th, NULL);
};


void	ClassThread::ModOn(int Vol)
{
	MyLock(__FILE__,__LINE__,&Fl_Th,NULL);
	On=Vol;
	MyUnLock(__FILE__,__LINE__,&Fl_Th,NULL);
};
int	ClassThread::GetOn()
{
	int Vol;
	MyLock(__FILE__,__LINE__,&Fl_Th,NULL);
	Vol=On;
	MyUnLock(__FILE__,__LINE__,&Fl_Th,NULL);
	return Vol;
};

ClassThread::~ClassThread()
{
	size_t i=0;
	char	tBuf[256];

	if(hThrd!=NULL) // 22.06.2012 Developer
	{
	MyLock(__FILE__,__LINE__,&Fl_Th,NULL);
	if(On!=2)
	{
		On=0;
		i=0;
		while (On!=2)
		{
			MyUnLock(__FILE__,__LINE__,&Fl_Th,NULL);
			if (++i>100)
			{
				Sleep(10);
// 				if (Srv)
// 				{
// 					//sprintf(tBuf,"TerminateThread:%s=%d",NameThread,hThrd);
// 					sprintf(tBuf,"TerminateThread:%s",NameThread);
// 					Srv->OutLog(tBuf,NULL,"ClassThread");
// 				}
// 				TerminateThread(hThrd,0);
// 				MyLock(__FILE__,__LINE__,&Fl_Th,NULL);
// 				break;
					if (i>300)//  [6/22/2012 Developer]
					{
						MyLock(__FILE__,__LINE__,&Fl_Th,NULL);
						TerminateThread(hThrd,0);break;

					}
			}
			Sleep(10);
			MyLock(__FILE__,__LINE__,&Fl_Th,NULL);
		}
	}
	if (i<300)
	{
		if (Srv)
		{
			sprintf(tBuf,"Close Thread:%s",NameThread);
			Srv->OutLog(tBuf,NULL,"ClassThread");
		}
	}
	MyUnLock(__FILE__,__LINE__,&Fl_Th,NULL);
	CloseHandle(hThrd);hThrd=NULL;
	}
};


	PackNetObjArray::PackNetObjArray(int q)
{
	idx=0; N=q;
	Array=(pPackNetObj)malloc(N*sizeof(PackNetObj));
	memset(Array,0,N*sizeof(PackNetObj));
}
	PackNetObjArray::~PackNetObjArray()
{
	free(Array);
}
int PackNetObjArray::Reset()
{
	idx=0;
	return 0;
}
pPackNetObj PackNetObjArray::GetPack()
{
	if(idx==N) idx=N-1;
	return &Array[idx++];
}

void PackNetObjArray::PostPack(char* Descriptor,int Command)
{
	pPackNetObj p=GetPack();
	strcpy(p->descriptor,Descriptor);
	p->command=Command;
}

void PackNetObjArray::PostPack(char* Descriptor,int Command,char* Param, char* Vol)
{
	pPackNetObj p=GetPack();
	strcpy(p->descriptor,Descriptor);
	p->command=Command;
	strcpy(p->param,Param);
	strcpy(p->vol,Vol);
}

void PackNetObjArray::PostPack(char* Descriptor,int Command,UCHAR* pBuf, size_t Cnt)
{
	if(*Descriptor=='_')
		PostPack(Descriptor,Command,-1,pBuf,Cnt);
	else
		PostPack(Descriptor,Command,0,pBuf,Cnt);
}

void PackNetObjArray::PostPack(char* Descriptor,int Command,int pin,UCHAR* pBuf, size_t Cnt)
{
	pPackNetObj p;
	if(*Descriptor=='_')
	{
		// pin - äëÿ ñåðâåðà - íîìåð êëèåíòà
		p=GetPack();
		strcpy(p->descriptor,Descriptor);
		p->command=Command; p->data=pin;
		p->pin=0; p->buf=(PUCHAR)&p->data; p->cnt=4;
		p=GetPack();
		strcpy(p->descriptor,Descriptor);
		p->command=Command;
		p->pin=1; p->buf=pBuf; p->cnt=Cnt;
	}
	else
	{
		// pin - äëÿ êëèåíòà - pin
		p=GetPack();
		strcpy(p->descriptor,Descriptor);
		p->command=Command;
		p->pin=pin; p->buf=pBuf; p->cnt=Cnt;
	}
}

int PackNetObjArray::GetBuf(PUCHAR &p,int &c)
{
	p=(PUCHAR)Array;
	c=idx*sizeof(PackNetObj);
	return 0;
}


		El_DllFile::El_DllFile(char* DName,char* HName,FileTime* tCr,FileTime* tAc,FileTime* tWr)
{
	strcpy(DllName,DName);
	strcpy(HostName,HName);
	Cr.Time=tCr->Time;
	Ac.Time=tAc->Time;
	Wr.Time=tWr->Time;
	Init((uchar*)DllName,strlen(DllName)+1);
};


		El_Log::El_Log(LogParam* LP,char* PathLog,Services* pSrv):ID_Elem((UCHAR*)LPrm.KeyVol,strlen(LP->KeyVol)+1)
{
	Srv=pSrv;
	LPrm.Fl=LP->Fl;
	strcpy(LPrm.KeyVol,LP->KeyVol);
	Log=new Savelog();
	//////////////////////////////////////////////////////////////////////////
	Log->Srv=pSrv;
	Log->Param("PathLog",PathLog);
	Log->Param("AppendLF","OnEnd");
	Log->Param("AppendTime","on");
	Log->Param("ExtLog",LP->KeyVol);
	Log->Param("NDay",LP->NDay);
	if (LP->TypeTime==0)
		Log->Param("TypeTime","System");
	else
		Log->Param("TypeTime","Local");
	Log->Start();
//	RSizeBuf=1024;
//	tBuf=(char*)malloc(RSizeBuf);
	WCHAR	ttB[64];
	//sprintf(ttB,"SrvSemLog%d",(int)this);
	wsprintf(ttB,_T("SrvSemLog"));
	hmtxlog=OpenSemaphore(SEMAPHORE_ALL_ACCESS	,TRUE,ttB);
	if (hmtxlog==NULL)
		hmtxlog=CreateSemaphore(NULL,1,1,ttB);
};

void	El_Log::OutLog(char* Buf,Decoder *Poi)
{
	size_t	SizeBuf=strlen(Buf)+1;
/*	if (SizeBuf>RSizeBuf)
	{
		char*	ttBuf;
		ttBuf=(char*)realloc(tBuf,SizeBuf);
		if (ttBuf)
		{
			RSizeBuf=SizeBuf;
			tBuf=ttBuf;
			strcpy(tBuf,Buf);
		}
		else
		{
			tBuf[0]=0;
			Srv->SetCriticalError(Poi,"Log-data loss","Error");
		}
	}
	else
		strcpy(tBuf,Buf);
	int NewSizeBuf=strlen(tBuf);
	if ((NewSizeBuf+1)!=SizeBuf)
	{
		Srv->SetCriticalError(Poi,"Log-data not equal","Error");
	}
*/
	DWORD rc=WaitForSingleObject(hmtxlog,1000);
	if (rc==0)	Log->Data(0,(UCHAR*)Buf,SizeBuf);	//+1
	ReleaseSemaphore(hmtxlog,1,NULL); 

//	Log->Data(0,(UCHAR*)tBuf,NewSizeBuf);	//+1
};

void	El_Log::Idle()
{
	Log->Data(1,(UCHAR*)this,1);
};

		El_Log::~El_Log()
{
	Log->Stop();
	delete Log;
//	free (tBuf);
	CloseHandle(hmtxlog);
};
	ID_ElemIDI::ID_ElemIDI(Decoder* Mt) 
{
	IDI=new IDInfo(Mt);
	Buff=(UCHAR*)&IDI;
	Len=sizeof(void*);
};
	ID_ElemIDI::~ID_ElemIDI() 
{
	delete IDI;
};
void	ID_ElemIDI::Init(Decoder* Mt)
{
	IDI->ReInit(Mt);
}

ID_ElemIDI2::ID_ElemIDI2(IDInfo* pIDI,Decoder* Mt) 
{
	tPIDI=pIDI;
	IDI=GetServices()->GetNewIDI(Mt);
//	IDI=new IDInfo(Mt);
	Buff=(UCHAR*)&tPIDI;
	Len=sizeof(void*);
};
ID_ElemIDI2::~ID_ElemIDI2() 
{
	GetServices()->DeleteIDI(IDI);
};
void	ID_ElemIDI2::Init(IDInfo* pIDI,Decoder* Mt)
{
	IDI->ReInit(Mt);
	tPIDI=pIDI;
	Buff=(UCHAR*)&tPIDI;
	Len=sizeof(void*);
}

	ClasterMem::ClasterMem(int SizeCM,Decoder* pDec)
{
#ifndef _NOSRV
	Srv=GetServices();
	Dec=pDec;
#else
	Srv=0;Dec=0;
#endif
	Buf = NULL;
	Size=SizeCM;
	INT64	Hand=0;
//_mm1:
#ifndef _NOSRV
	if (Dec) Buf=(UCHAR*)Dec->malloc(Size);
	else Buf=(UCHAR*)Srv->malloc(Size);
#else
	Buf=(UCHAR*)malloc(Size);
#endif
	if (Buf==NULL)
	{
#ifndef _NOSRV
		char	tB[256];
		sprintf(tB,"Cannot alloc memory Size=%d",Size);
		Srv->SetCriticalError(Dec,tB,"CriticalError","ClasterMem",Hand);
#endif
//		Sleep(100);
//		goto _mm1;
		Size=0;
	}
	Wr=Rd=Fl_Rd=Fl_Wr=0;
	PreviousCM=NextCM=NULL;
};
void	ClasterMem::ReSize(int NewSize)
{
#ifndef _NOSRV
	if (Dec) Dec->free(Buf);
	else Srv->free(Buf);
#else
	free(Buf);
#endif

	Size=NewSize;
	INT64	Hand=0;
//_mm2:
#ifndef _NOSRV
	if (Dec) Buf=(UCHAR*)Dec->malloc(Size);
	else Buf=(UCHAR*)Srv->malloc(Size);
#else
	Buf=(UCHAR*)malloc(Size);
#endif
	if (Buf==NULL) 
	{
#ifndef _NOSRV
		char	tB[256];
		sprintf(tB,"Cannot realloc memory Size=%d",Size);
		Srv->SetCriticalError(Dec,tB,"CriticalError","ClasterMem",Hand);
#endif
//		Sleep(100);
//		goto _mm2;
		Size=0;
	}
	Wr=Rd=Fl_Rd=Fl_Wr=0;
};
	ClasterMem::~ClasterMem()
{
	if (Buf)
	{
#ifndef _NOSRV
		if (Dec) Dec->free(Buf);
		else Srv->free(Buf);
#else
		free(Buf);
#endif
		Buf=NULL;
	};
};

	El_UnitName::El_UnitName(El_Unit *tElU)
{
	ElU=tElU;
	strcpy(UnitName,tElU->UInf.UnitName);
	Buff=(UCHAR*)UnitName;
	Len=strlen(UnitName);
	DomenList=new ID_List8();
}
	El_UnitName::~El_UnitName()
{
	DomenList->DelAllElem();
	delete DomenList;
}
void	El_UnitName::Init(El_Unit *tElU)
{
	ElU=tElU;
	strcpy(UnitName,tElU->UInf.UnitName);
	Buff=(UCHAR*)UnitName;
	Len=strlen(UnitName);
}

void	El_UnitName::AddDomen(El_Unit *tElU)
{
	int	rc=-1;
	El_DomName	*tEl=(El_DomName*) DomenList->Find((UCHAR*)&tElU->UInf.DomainName,strlen(tElU->UInf.DomainName));
	if (tEl==NULL)
	{
		tEl=new El_DomName(tElU);
		rc=DomenList->Add(tEl);
		if (rc==-1)
		{
			delete tEl;
		}
		else
		{
			tEl->AddUIEl(tElU);
		}
	}
	else
	{
		tEl->AddUIEl(tElU);
	}
}

int	El_UnitName::DelDomen(El_Unit *tElU)
{
	El_DomName	*tEl=(El_DomName*) DomenList->Find((UCHAR*)&tElU->UInf.DomainName,strlen(tElU->UInf.DomainName));
	if (tEl)
	{
		if (tEl->DelUIEl(tElU)==0)
			DomenList->Del(tEl);
	}
	return DomenList->NumElem;
};

El_DomName::El_DomName(El_Unit *tElU)
{
	ElU=tElU;
	strcpy(DomName,tElU->UInf.DomainName);
	Buff=(UCHAR*)DomName;
	Len=strlen(DomName);
	UnitList=new ID_List8();
}
El_DomName::~El_DomName()
{
	FindStrList8	FSL8;
	El_Unit	*tEl=(El_Unit*)UnitList->Begin(FSL8);
	while (tEl)
	{
		UnitList->Remove(tEl);
		tEl=(El_Unit*)UnitList->Next(FSL8);
	}
	FSL8.DestroyStr();
	delete UnitList;
}

void	El_DomName::Init(El_Unit *tElU)
{
	ElU=tElU;
	strcpy(DomName,tElU->UInf.DomainName);
	Buff=(UCHAR*)DomName;
	Len=strlen(DomName);
}

void	El_DomName::AddUIEl(El_Unit *tElU)
{
	int	rc=-1;
	El_Unit	*tEl=(El_Unit*) UnitList->Find((UCHAR*)&tElU->UInf.PersID,sizeof(tElU->UInf.PersID));
	if (tEl==NULL)
	{
//		tEl=new El_DomName(ElU);
		rc=UnitList->Add(tElU);
		if (rc==-1)
		{
			tElU=NULL;
		}
	}
	else
		tElU=NULL;

}

int	El_DomName::DelUIEl(El_Unit *tElU)
{
	El_DomName	*tEl=(El_DomName*) UnitList->Find((UCHAR*)&tElU->UInf.PersID,sizeof(tElU->UInf.PersID));
	if (tEl)
	{
		UnitList->Remove(tEl);
	}
	return UnitList->NumElem;
};


UnitMng::UnitMng()
{
	Srv=GetServices();
//	MyLock(__FILE__,__LINE__,&Fl_RE,NULL);
	UnitList=new ID_List8();
	UnitNameList=new ID_List8();
//	MyUnLock(__FILE__,__LINE__,&Fl_RE,NULL);
};

void		UnitMng::AddUnit(UnitInfo* UI)
{
	int rc;
	MyLock(__FILE__,__LINE__,&Fl_RE,NULL);
	Srv->GetRealTime(&UI->CrTime.FTime);
	Srv->NLAddbyUnitName(UI->UnitName,inet_ntoa(*(in_addr*)&UI->IP));
	El_Unit* tEl=(El_Unit*)UnitList->Find((UCHAR*)&UI->PersID,sizeof(INT64));
	if (tEl==NULL)
	{
		Srv->GetRealTime(&UI->ModTime.FTime);
		tEl=new El_Unit(UI);
		rc=UnitList->Add(tEl);
		El_UnitName* tEl2=(El_UnitName*)UnitNameList->Find((UCHAR*)&UI->UnitName,strlen(UI->UnitName));
		if (tEl2==NULL)
		{
			tEl2=new El_UnitName(tEl);
			rc=UnitNameList->Add(tEl2);
			if (rc==-1)
			{
				delete tEl2;
			}
			else
			{
				tEl2->AddDomen(tEl);
			}
		}
		else
		{
			tEl2->AddDomen(tEl);
		}
//		MyUnLock(__FILE__,__LINE__,&Fl_RE,NULL);
		Srv->SendEvent(EV_AddUnit,(UCHAR*)UI,sizeof(UnitInfo));
	}
	else
	{
		Srv->GetRealTime(&UI->ModTime.FTime);
//		Srv->NLAddbyUnitName(UI->UnitName,UI->IP);
		memcpy(&tEl->UInf,UI,sizeof(UnitInfo));
//		MyUnLock(__FILE__,__LINE__,&Fl_RE,NULL);
		Srv->SendEvent(EV_ModifUnit,(UCHAR*)UI,sizeof(UnitInfo));
	}
	MyUnLock(__FILE__,__LINE__,&Fl_RE,NULL);
};

void		UnitMng::DelUnit(UnitInfo* UI)
{
	int rc;
	MyLock(__FILE__,__LINE__,&Fl_RE,NULL);
	El_Unit* tEl=(El_Unit*)UnitList->Find((UCHAR*)&UI->PersID,sizeof(INT64));
//	MyUnLock(__FILE__,__LINE__,&Fl_RE,NULL);
	if (tEl)
	{
		Srv->SendEvent(EV_DelUnit,(UCHAR*)UI,sizeof(UnitInfo));
//		MyLock(__FILE__,__LINE__,&Fl_RE,NULL);
		El_UnitName* tEl2=(El_UnitName*)UnitNameList->Find((UCHAR*)&UI->UnitName,strlen(UI->UnitName));
		if (tEl2)
			if (tEl2->DelDomen(tEl)==0)
				UnitNameList->Del(tEl2);
		rc=UnitList->Del(tEl);
//		MyUnLock(__FILE__,__LINE__,&Fl_RE,NULL);
	}
//	else
//		MyUnLock(__FILE__,__LINE__,&Fl_RE,NULL);
	MyUnLock(__FILE__,__LINE__,&Fl_RE,NULL);
};

El_Unit*	UnitMng::FindUnitbyPersID(INT64 PersID)
{
	MyLock(__FILE__,__LINE__,&Fl_RE,NULL);
	El_Unit* tEl=(El_Unit*)UnitList->Find((UCHAR*)&PersID,sizeof(INT64));
//	tEl=(El_Unit*)UnitList->Begin();
//	while (tEl)
//	{
//		if (tEl->UInf.PersID==PersID)
//		{
//			break;
//		}
//		tEl=(El_Unit*)UnitList->Next();
//	}
	MyUnLock(__FILE__,__LINE__,&Fl_RE,NULL);
	return tEl;
};

int	UnitMng::CalcUnitNumber(char* UnitName,char* Mode)
{
	int rc=0;
	El_Unit* tEl=NULL;
	MyLock(__FILE__,__LINE__,&Fl_RE,NULL);
	FindStrList8	FSL8;
	tEl=(El_Unit*)UnitList->Begin(FSL8);
	while (tEl)
	{
		if (strcmp(tEl->UInf.UnitName,UnitName)==0)
		{
			if (tEl->UInf.NumChan==0)
			{
				if (Mode)
				{
					if (strcmp(tEl->UInf.Mode,Mode)==0)
					{
						rc++;
					}
				}
				else
				{
					rc++;
				}
			}
		}
		tEl=(El_Unit*)UnitList->Next(FSL8);
	}
	MyUnLock(__FILE__,__LINE__,&Fl_RE,NULL);
	return rc;
};
/*
El_Unit*	UnitMng::FindUnit(char* UnitName, int UnitID)
{
	El_Unit* tEl=NULL;
	MyLock(__FILE__,__LINE__,&Fl_RE,NULL);
	FindStrList8	FSL8;
	tEl=(El_Unit*)UnitList->Begin(FSL8);
	while (tEl)
	{
		if (strcmp(tEl->UInf.UnitName,UnitName)==0)
		{
			if(UnitID==0)
				break;
			else
			{
				if(UnitID==tEl->UInf.UnitID)
					break;
			}
		}
		tEl=(El_Unit*)UnitList->Next(FSL8);
	}
	MyUnLock(__FILE__,__LINE__,&Fl_RE,NULL);
	return tEl;
};
*/
ID_List8*	UnitMng::FindUnit(char* UnitName)
{
	ID_List8	*IDL=NULL;
	MyLock(__FILE__,__LINE__,&Fl_RE,NULL);
	El_UnitName* tEl=(El_UnitName*)UnitNameList->Find((UCHAR*)UnitName,strlen(UnitName));
	if (tEl)
	{
		IDL=tEl->DomenList;
	}
	MyUnLock(__FILE__,__LINE__,&Fl_RE,NULL);
	return IDL;
};

ID_List8*	UnitMng::FindUnit(char* UnitName, char* DomainName)
{
	ID_List8	*IDL=NULL;
	MyLock(__FILE__,__LINE__,&Fl_RE,NULL);
	El_UnitName* tEl=(El_UnitName*)UnitNameList->Find((UCHAR*)UnitName,strlen(UnitName));
	if (tEl)
	{
		El_DomName	*tEl2=(El_DomName*) tEl->DomenList->Find((UCHAR*)DomainName,strlen(DomainName));
		if (tEl2)
		{
			IDL=tEl2->UnitList;
		}
	}
	MyUnLock(__FILE__,__LINE__,&Fl_RE,NULL);
	return IDL;
};

El_Unit*	UnitMng::FindUnit(char* UnitName, char* DomainName,char* HostName,char* Mode,int Condition)
{
	El_Unit* tEl=NULL;
	El_Unit* tEl2=NULL;
	MyLock(__FILE__,__LINE__,&Fl_RE,NULL);
	FindStrList8	FSL8;
	ID_List8	*tIDL=FindUnit(UnitName,DomainName);
	if (tIDL)
	{
		float	IndexVol=100.1;
		tEl2=(El_Unit*)tIDL->Begin(FSL8);
		while (tEl2)
		{
			if (((HostName==NULL)||(HostName[0]==0)||(strcmp(tEl2->UInf.Host,HostName)==0))&&((Mode==NULL)||(Mode[0]==0)||(strcmp(tEl2->UInf.Mode,Mode)==0)))
			{
				float	LCPU=Srv->SInt->GetLoadsCPU(&tEl2->UInf);
				float	LRAM=Srv->SInt->GetLoadsRAM(&tEl2->UInf);
				switch(Condition)
				{
				case 0:	// Balance

					if (IndexVol>(LCPU+0.5*LRAM))
					{
						IndexVol=LCPU+0.5*LRAM;
						tEl=tEl2;
					}
					break;
				case 1:	// CPU
					if (IndexVol>LCPU)
					{
						IndexVol=LCPU;
						tEl=tEl2;
					}
					break;
				case 2:	// RAM
					if (IndexVol>LRAM)
					{
						IndexVol=LRAM;
						tEl=tEl2;
					}
					break;
				case 3:	// First
					tEl=tEl2;
					break;
				}
				if (Condition==3)
					break;
			}
			tEl2=(El_Unit*)tIDL->Next(FSL8);
		}
	}
	MyUnLock(__FILE__,__LINE__,&Fl_RE,NULL);
	return tEl;
};

int	UnitMng::UnitSelect(El_Unit* ElUnit)
{
	int rc=0;
	if (ElUnit)
		rc=Srv->SInt->SelectUnit(&ElUnit->UInf);
	return rc;
}

int	UnitMng::UnitSetLoad(El_Unit* ElUnit,float Vol)
{
	int rc=0;
	if (ElUnit)
		rc=Srv->SInt->SetUnitLoad(&ElUnit->UInf,Vol);
	return rc;
}

El_Unit*	UnitMng::FindUnitChan(char* UnitName, char* DomainName, char* HostName, char* Mode, int NumChan, int Condition)
{
	El_Unit* tEl=NULL;
	El_Unit* tEl2=NULL;
	MyLock(__FILE__,__LINE__,&Fl_RE,NULL);
	FindStrList8	FSL8;
	ID_List8	*tIDL=FindUnit(UnitName,DomainName);
	if (tIDL)
	{
		float	IndexVol=100.1;
		tEl2=(El_Unit*)tIDL->Begin(FSL8);
		while (tEl2)
		{
			if (((HostName==NULL)||(HostName[0]==0)||(strcmp(tEl2->UInf.Host,HostName)==0))&&((Mode==NULL)||(Mode[0]==0)||(strcmp(tEl2->UInf.Mode,Mode)==0))&&(tEl2->UInf.NumChan==NumChan))
			{
				float	LCPU=Srv->SInt->GetLoadsCPU(&tEl2->UInf);
				float	LRAM=Srv->SInt->GetLoadsRAM(&tEl2->UInf);
				switch(Condition)
				{
				case 0:	// Balance

					if (IndexVol>(LCPU+0.5*LRAM))
					{
						IndexVol=LCPU+0.5*LRAM;
						tEl=tEl2;
					}
					break;
				case 1:	// CPU
					if (IndexVol>LCPU)
					{
						IndexVol=LCPU;
						tEl=tEl2;
					}
					break;
				case 2:	// RAM
					if (IndexVol>LRAM)
					{
						IndexVol=LRAM;
						tEl=tEl2;
					}
					break;
				case 3:	// First
					tEl=tEl2;
					break;
				}
				if (Condition==3)
					break;
			}
			tEl2=(El_Unit*)tIDL->Next(FSL8);
		}
	}
	MyUnLock(__FILE__,__LINE__,&Fl_RE,NULL);
	return tEl;
};

bool	UnitMng::TestMode(char* IP,char* Mode,char* iUnitName)
{
	bool	rc=FALSE;
	El_Unit* tEl=NULL;
	MyLock(__FILE__,__LINE__,&Fl_RE,NULL);
	FindStrList8	FSL8;
	tEl=(El_Unit*)UnitList->Begin(FSL8);
	while (tEl)
	{
		if (strcmp(inet_ntoa(*(in_addr*)&tEl->UInf.IP),IP)==0)
		{
			if (strcmp(tEl->UInf.Mode,Mode)==0)
			{
				if (iUnitName)
				{
					if (strcmp(iUnitName,"*")==0)
						rc=TRUE;
					else if (iUnitName[0]==0)
						rc=TRUE;
					else if (strcmp(iUnitName,tEl->UInf.UnitName)==0)
						rc=TRUE;

				}
				else
					rc=TRUE;
				if (rc)
				break;
			}
		}
		tEl=(El_Unit*)UnitList->Next(FSL8);
	}
	MyUnLock(__FILE__,__LINE__,&Fl_RE,NULL);
	return rc;
};

UnitMng::~UnitMng()
{
	MyLock(__FILE__,__LINE__,&Fl_RE,NULL);
	UnitNameList->DelAllElem();
	delete UnitNameList;
	UnitList->DelAllElem();
	delete UnitList;
	MyUnLock(__FILE__,__LINE__,&Fl_RE,NULL);
};
/*
IntTest::IntTest()
{
	NumberEl=0;
};

IntTest::~IntTest()
{
	
};

int		IntTest::InternalTest()
{
	Srv=GetServices();
	int	rc=-1;
	Phase=0;
	NumberEl=0;
	El_Tab*	TempEl=NULL;
	El_Tab*	ElTParent=NULL;
	if (Srv->IDT_Neuron)
	{
		FindStruct	FStr;
		TempEl=Srv->IDT_Neuron->FindFirstET(Srv,&FStr,AcsMd_Read,Srv->IDT_Neuron->ElTbl);
		while(TempEl)
		{

			ReadTabl(TempEl);
			//			TestList
			
			Srv->IDT_Neuron->UnLockEl(&Anchor,TempEl);
			TempEl=Srv->IDT_Neuron->FindNextET(Srv,&FStr,AcsMd_Read,Srv->IDT_Neuron->ElTbl);
		}

		Phase=1;

		TempEl=Srv->IDT_Neuron->FindFirstET(Srv,&FStr,AcsMd_Read,Srv->IDT_Neuron->ElTbl);
		while(TempEl)
		{
			
			ReadTabl(TempEl);
			//			TestList
			
			Srv->IDT_Neuron->UnLockEl(&Anchor,TempEl);
			TempEl=Srv->IDT_Neuron->FindNextET(Srv,&FStr,AcsMd_Read,Srv->IDT_Neuron->ElTbl);
		}

		Phase=2;

		TempEl=Srv->IDT_Neuron->FindFirstET(Srv,&FStr,AcsMd_Read,Srv->IDT_Neuron->ElTbl);
		while(TempEl)
		{
			
			ReadTabl(TempEl);
			//			TestList
			
			Srv->IDT_Neuron->UnLockEl(&Anchor,TempEl);
			TempEl=Srv->IDT_Neuron->FindNextET(Srv,&FStr,AcsMd_Read,Srv->IDT_Neuron->ElTbl);
		}
	}
	return rc;
};
int		IntTest::TestEl(El_Tab*	El)
{
	ID_Elem64*	tEl;
	switch (Phase)
	{
	case 0:
		NumberEl++;
		tEl=new ID_Elem64(El->GetIDI()->ID_File);
		if (IList.Add(tEl)==-1)
		{
			{size_t iasm=1;}
		};
		break;
	case 1:
		--NumberEl;
		if (IList.Find((UCHAR*)&El->GetIDI()->ID_File,sizeof(INT64))==NULL)
		{
			{size_t iasm=1;}
		}
		break;
	case 2:
		++NumberEl;
		tEl=(ID_Elem64*)IList.Find((UCHAR*)&El->GetIDI()->ID_File,sizeof(INT64));
		if (tEl)
		{
			IList.Del(tEl);
		}
		else
		{
			{size_t iasm=1;}
		}
		break;
	}
};

int		IntTest::ReadTabl(El_Tab*	ElTParent)
{
	int	rc=-1;
	TestEl(ElTParent);
	El_Tab*	TempEl=NULL;
	if (Srv->IDT_Neuron)
	{
		FindStruct	FStr;
		TempEl=Srv->IDT_Neuron->FindFirstET(Srv,&FStr,AcsMd_Read,ElTParent);
		while(TempEl)
		{
			ReadTabl(TempEl);
			//			TestList
			Srv->IDT_Neuron->UnLockEl(&Anchor,TempEl);
			TempEl=Srv->IDT_Neuron->FindNextET(Srv,&FStr,AcsMd_Read,ElTParent);
		}
	}
	return rc;
}
*/
// 
int SrvIdle(LPVOID poi)
{
  ((ServerDecod*)poi)->ServicesIdle();
  return 0;
};

int SysIdle(LPVOID poi)
{
	((ServerDecod*)poi)->SystemIdle();
	return 0;
};

int TrtIdle(LPVOID poi)
{
	((ServerDecod*)GetServices()->Neuron)->TractIdle((size_t)poi);
	return 0;
};

int DinTrtIdle(LPVOID poi)
{
	((ServerDecod*)GetServices()->Neuron)->DinTractIdle((size_t)poi);
	return 0;
};

void	CalcDec(int N_Pin,UCHAR* Buf,int Len,LPVOID Poi,int ID)
{
	((ServerDecod*)Poi)->Calc(N_Pin,Buf,Len,ID);
};

//int		tTemp=-1;

void	OutData(int N_Pin,UCHAR* Buf,size_t Len,LPVOID Poi)
{
	((ServerDecod*)Poi)->SendData(N_Pin,Buf,Len);
};
ServerDecod::ServerDecod(Services *pSrv)
{
Fl_Strat=0;pList=NULL;
Srv=pSrv;
int	i,j;
SrvThread=NULL;

SysThread=new ClassThread("SysNeuron",SysIdle,(LPVOID)this);

m_pItem.iItem=0;
m_pItem.iSubItem=0;
m_pItem.mask=LVIF_IMAGE;
kImage=0;

for (i=0;i<MaxID;i++)
{
	Tract[i]=NULL;
	ClThread[i]=NULL;
	DinTract[i]=NULL;
	DinThread[i]=NULL;
	//	Tsk[i]=NULL;
	CntRun[i]=0;
	CntDinRun[i]=0;
	CntTry[i]=0;
	CntDinTry[i]=0;
	TractUsage[i]=0;
	DinTractUsage[i]=0;
}

ClkWDog=0;
TmpIDTrct=-1;
//char	FullName[256];

//char    LoadError[100];
//char    ProcName[100];
//int	(*FindNewDecod)(char*);
//char*	(*FindFirst)();
//char*	(*FindNext)();
//Decoder	*PoiMt;
InfoMt	InfMt;
InfoPin	InfPin;
InfoPrm	InfPrm;

DataImEx	*DataIE;
char		FullName[512];

// Îòêðûòü ÁÄ
int	Fl=0;
//Map1=OpenFileMapping(FILE_MAP_WRITE,TRUE,"ServerDec_DB");
//if (Map1==NULL)
//{
//	Fl=1;
//	Map1=CreateFileMapping((HANDLE)-1/*0xFFFFFFFF*/,NULL,PAGE_READWRITE|SEC_COMMIT,0,sizeof(Task),"ServerDec_DB");
//};
//	Tsk=(Task*)MapViewOfFile(Map1,FILE_MAP_WRITE,0,0,0);
	TTsk=pSrv->Tsk;
//if (DosAllocSharedMem(&(void*)TTsk,"\\SHAREMEM\\ServerDec_DB",sizeof(Task), PAG_COMMIT | PAG_WRITE | PAG_READ)!=0)
//   DosGetNamedSharedMem(&(void*)TTsk,"\\SHAREMEM\\ServerDec_DB",PAG_WRITE | PAG_READ);
//if (Fl==1)
   {
	   DataIE=new DataImEx();
	   memset(TTsk,0x0,sizeof(Task));
	   strcpy(FullName,Srv->M_Cfg->MData.PathCFG[Srv->M_Cfg->MData.Active]);
	   strcat(FullName,"\\Neuron.cfg");
	   DataIE->ImportTsk(TTsk,FullName);
	   delete DataIE;
	   TTsk->Max_BMt=0;
	   TTsk->Max_BPr=0;
	   TTsk->Max_BIn=0;
	   TTsk->Max_BOut=0;
   };
// ×èòàåì äàííûå ñ DLL
j=0;
char*	Name;
int	rc;
	FindStrList8	FSL8;
	Name=Srv->DLLL->FirstMeth(FSL8);
	while (Name!=NULL)
	{
		if (Srv->DLLL->FindMeth(Name)==0) break;
		 rc=Srv->DLLL->GetInfoMethod(&InfMt,Name);
		 if (rc!=-1)
		 {
			 strcpy(TTsk->BMt[TTsk->Max_BMt].Name,InfMt.Name);
			 TTsk->BMt[TTsk->Max_BMt].N_PI=InfMt.NPinI;
			 TTsk->BMt[TTsk->Max_BMt].N_PO=InfMt.NPinO;
			 for (i=0;i<InfMt.NPinI;i++)
				{
				Srv->DLLL->GetInfoPinIn(i,&InfPin,Name);
				strcpy(TTsk->BIn[TTsk->Max_BIn].Name,InfPin.Name);
				TTsk->BIn[TTsk->Max_BIn++].ID_BMt=TTsk->Max_BMt;
				};
			 for (i=0;i<InfMt.NPinO;i++)
				{
				Srv->DLLL->GetInfoPinOut(i,&InfPin,Name);
				strcpy(TTsk->BOut[TTsk->Max_BOut].Name,InfPin.Name);
				TTsk->BOut[TTsk->Max_BOut++].ID_BMt=TTsk->Max_BMt;
				};
			 for (i=0;i<InfMt.NParam;i++)
				{
				Srv->DLLL->GetInfoParam(i,&InfPrm,Name);
				strcpy(TTsk->BPr[TTsk->Max_BPr].Name,InfPrm.Name);
				TTsk->BPr[TTsk->Max_BPr].ID_BMt=TTsk->Max_BMt;
				strcpy(TTsk->BPr[TTsk->Max_BPr++].Vol,InfPrm.Vol[0]);
				};
			 TTsk->Max_BMt++;
		 }
		 // ------------------------------
//		 delete PoiMt;
		Name=Srv->DLLL->NextMeth(FSL8);
	}
SpeedIn=0;
SpeedOut=0;

LoadCfg();

//SDec=new SharedNet("Decoding Server",MCfg->MData.SizeSB);
//SDec->SData->Srv=pSrv;
//SDec->ExtPoi=(LPVOID)this;
//SDec->ExtCalc=&CalcDec;

SrvThread=new ClassThread("Neuron",SrvIdle,(LPVOID)this);
Srv->Neuron=this;		// New Neuron Tech
};

//int	ttTemp=-1;

void	ServerDecod::SendData(int N_Pin,UCHAR* Buf,int Len)
{
	SpeedOut+=Len;
// 	if (SDec!=NULL)	//
// 	while (Len>MaxShPack-sizeof(HPack))	// Âàæíîå äîïîëíåíèå ê ñïîñîáó ïåðåäà÷è èíôîðìàöèè...  òàêèì îáðàçîì äëèííûé ïàêåò ðàçáèâàåòñÿ íà ìàëåíüêèå !!!
// 	{
// 		SDec->SendData(N_Pin,Buf,MaxShPack-sizeof(HPack)-1,TmpIDTrct);
// 		Len-=(MaxShPack-sizeof(HPack)-1);
// 		Buf+=(MaxShPack-sizeof(HPack)-1);
// 	}
// 	if (SDec!=NULL)	//
// 	SDec->SendData(N_Pin,Buf,Len,TmpIDTrct);
};


ConstrDecF*	ServerDecod::CreateDinTract(char* ModeName,char* DomainName,int NumChan)
{
	int		i;
	int		j;
	int		ModeNum=-1;
	ConstrDecF*	TempRC=NULL;
	MyLock(__FILE__,__LINE__,&Fl_REDin,NULL);

	for (i=0;i<MaxID;i++)
	{
		if (DinTract[i]==NULL)
		{
			char	DinTrName[80];
			sprintf(DinTrName,"VirtTr%d",i);
			DinTract[i]=new Pack_Dec(Srv);
			MyLock(__FILE__,__LINE__,&DinTract[i]->FlSim,NULL);
			strcpy(DinTract[i]->UInfo.DomainName,DomainName);
			strcpy(DinTract[i]->UInfo.Mode,DinTrName);
			DinTract[i]->UInfo.NumChan=NumChan;
			DinTract[i]->UInfo.TractID=i+0x8000;
			for (j=0;j<TTsk->Max_Md;j++)
			{
				if (strcmp(TTsk->Md[j].Name,ModeName)==0)
				{
					ModeNum=j;
					break;
				}
			}
			if (ModeNum!=-1) 
				DinTract[i]->ReStruct(TTsk,ModeNum,i);
			DinThread[i]=new ClassThread(DinTrName,DinTrtIdle,(LPVOID)i);
			TempRC=DinTract[i];
			MyUnLock(__FILE__,__LINE__,&DinTract[i]->FlSim,NULL);
			break;
		}
	}
	MyUnLock(__FILE__,__LINE__,&Fl_REDin,NULL);
	return	TempRC;
};

int ServerDecod::DeleteDinTract(ConstrDecF* HandlePointer)
{
	int	i;
	int	RC=0;
	MyLock(__FILE__,__LINE__,&Fl_REDin,NULL);
	for (i=0;i<MaxID;i++)
	{
		if (DinTract[i]==HandlePointer)
		{
			ConstrDecF	*tCD=DinTract[i];
			MyLock(__FILE__,__LINE__,&tCD->FlSim,NULL);
			SendToDestroy(tCD,DinThread[i]);
			DinTract[i]=NULL;DinThread[i]=NULL;
			MyUnLock(__FILE__,__LINE__,&tCD->FlSim,NULL);
			RC=1;
			break;
		}
	}
	MyUnLock(__FILE__,__LINE__,&Fl_REDin,NULL);
	return RC;
};

void	ServerDecod::SendToDestroy(ConstrDecF* HandlePointer,ClassThread* ThreadPointer)
{
	TempSrvDecPoi	TSDP;
	TSDP.HandlePointer=HandlePointer;
	TSDP.ThreadPointer=ThreadPointer;
	DinDmp.Data((UCHAR*)&TSDP,sizeof(TempSrvDecPoi),0);
};

void	ServerDecod::RePackTr()
{
	int	i;
	int	j=0;
	MyLock(__FILE__,__LINE__,&Fl_RE,NULL);
	for (i=0;i<MaxID;i++)
	{
		if (SimTsk.Cli[i].Status==1)
		{
			if (j!=i)
			{
				memcpy(&SimTsk.Cli[j],&SimTsk.Cli[i],sizeof(SimClient));
				SimTsk.Cli[i].Status=0;
			}
			j++;
		}
	};
	MyUnLock(__FILE__,__LINE__,&Fl_RE,NULL);
};

void	ServerDecod::StartTr(char* DomainName)
{
	size_t i;
	MyLock(__FILE__,__LINE__,&Fl_RE,NULL);
	for (i=0;i<MaxID;i++)
	{
		if (SimTsk.Cli[i].Status==1)
		{
			if (strcmp(SimTsk.Cli[i].DomainName,DomainName)==0)
				break;
		}
	}
	if (i<MaxID)
	{
		SimTsk.Cli[i].Start=1;
	}
	MyUnLock(__FILE__,__LINE__,&Fl_RE,NULL);
};

void	ServerDecod::StopTr(char* DomainName)
{
	size_t i;
	MyLock(__FILE__,__LINE__,&Fl_RE,NULL);
	for (i=0;i<MaxID;i++)
	{
		if (SimTsk.Cli[i].Status==1)
		{
			if (strcmp(SimTsk.Cli[i].DomainName,DomainName)==0)
				break;
		}
	}
	if (i<MaxID)
	{
		SimTsk.Cli[i].Start=0;
	}
	MyUnLock(__FILE__,__LINE__,&Fl_RE,NULL);
};

void	ServerDecod::ReStartTr(char* DomainName)
{
	size_t i;
	MyLock(__FILE__,__LINE__,&Fl_RE,NULL);
	for (i=0;i<MaxID;i++)
	{
		if (SimTsk.Cli[i].Status==1)
		{
			if (strcmp(SimTsk.Cli[i].DomainName,DomainName)==0)
				break;
		}
	}
	if (i<MaxID)
	{
		if (Tract[i]!=NULL)
		{
			MyLock(__FILE__,__LINE__,&Tract[i]->FlSim,NULL);
			Tract[i]->Stop();
			delete ClThread[i];ClThread[i]=NULL;
			sprintf (TBuf,"Trackt%d -> %s Destroy !!!",i,Tract[i]->Str.Md.Name);
			Srv->OutLog(TBuf);
			Srv->DelDecLink(Tract[i]);
			MyUnLock(__FILE__,__LINE__,&Tract[i]->FlSim,NULL);
			delete Tract[i];Tract[i]=NULL;//FlagCli[i]=0;
			SimTsk.Cli[i].State=0;
		}
		if (SimTsk.Cli[i].Status==1)
		{
			SimTsk.Cli[i].State=0;
			Tract[i]=new Pack_Dec(Srv);
			MyLock(__FILE__,__LINE__,&Tract[i]->FlSim,NULL);
			strcpy(Tract[i]->UInfo.DomainName,SimTsk.Cli[i].DomainName);
			Tract[i]->UInfo.TractID=i;
			Tract[i]->Ext_Mt=i;
			Tract[i]->Srv=Srv;
			Tract[i]->OData=&OutData;
			Tract[i]->ExtPoi=(LPVOID)this;
			Tract[i]->IDOut=i;
			// Äëÿ íåéðîíà óæå íå íóæíî (ïðèâîäèëî ê äåäëîêó)
			Tract[i]->AddDecLink(Srv,2,0);
			Tract[i]->AddDecLink(Srv,3,1);
			Tract[i]->AddDecLink(Srv,4,2);
			Tract[i]->AddDecLink(Srv,5,3);
			
			
			sprintf (TBuf,"Create Trackt number %d",i);
			Srv->OutLog(TBuf);
			sprintf (SimTsk.Cli[i].CliName,"Tract%d",i);
			if ((SimTsk.Cli[i].ModeNum>=0)&&(SimTsk.Cli[i].ModeNum<TTsk->Max_Md))
				Tract[i]->ReStruct(TTsk,SimTsk.Cli[i].ModeNum,i);

			ClThread[i]=new ClassThread(SimTsk.Cli[i].CliName,TrtIdle,(LPVOID)i);
			MyUnLock(__FILE__,__LINE__,&Tract[i]->FlSim,NULL);
		}
	}
	MyUnLock(__FILE__,__LINE__,&Fl_RE,NULL);
};

int	ServerDecod::AddTract(char* DomainName,char* PathOut)
{
	int	i;
	int	rc=-1;
	MyLock(__FILE__,__LINE__,&Fl_RE,NULL);
	for(i=0;i<MaxID;i++)
	{
		if ((SimTsk.Cli[i].Status!=1)||(strcmp(SimTsk.Cli[i].DomainName,DomainName)==NULL))
		{
			SimTsk.Cli[i].Status=1;
			strcpy(SimTsk.Cli[i].DomainName,DomainName);
			strcpy(SimTsk.Cli[i].PathOut,PathOut);
			SimTsk.Cli[i].Start=1;
			strcpy(SimTsk.Cli[i].ModeName,"Role");
			for (SimTsk.Cli[i].ModeNum=0;SimTsk.Cli[i].ModeNum<Num_Mode;SimTsk.Cli[i].ModeNum++)
			{
				if (strcmp(Srv->Tsk->Md[SimTsk.Cli[i].ModeNum].Name,"Role")==NULL)
					break;
			}
			sprintf(SimTsk.Cli[i].CliName,"Tract%d",i);
			break;
		}
	}; 
	RePackTr();
	ReStartTr(DomainName);
	SaveCfg();
	MyUnLock(__FILE__,__LINE__,&Fl_RE,NULL);
	return rc;
};
int	ServerDecod::DelTract(char* DomainName)
{
	int	rc=-1;
	size_t i;
	MyLock(__FILE__,__LINE__,&Fl_RE,NULL);
	for (i=0;i<MaxID;i++)
	{
		if (SimTsk.Cli[i].Status==1)
		{
			if (strcmp(SimTsk.Cli[i].DomainName,DomainName)==0)
				break;
		}
	}
	if (i<MaxID)
	{
		StopTr(DomainName);
		SimTsk.Cli[i].Status=0;
		ReStartTr(i);
		SaveCfg();
	}
	IDInfo*	IDI=Srv->GetNewIDI(Srv);
	AddressInfo	AdrI;
	IDI->AdrInfo=&AdrI;
	Srv->InitCommIDI(IDI);
	//strcpy(AdrI.DestinAI.HostName,"");  
	AdrI.DestinAI.HostName[0]=0;
//	IDI->pPList->SetParam(IDUnitName,N_Service,sizeof(N_Service)+1,Srv);
	strcpy(AdrI.DestinAI.UnitName,N_Service);
	IDI->Command=Command_C_DelHostFromDomain;
	IDI->pPList->SetParam(IDString,DomainName,strlen(DomainName)+1,Srv);
	IDI->Cnt=0;
	IDI->NumPack++;
	Srv->SendIDI(IDI,Srv);
	Srv->DeleteIDI(IDI);
	Srv->SInt->DelHostFromDomain(Srv->ShortHostName,DomainName);
	MyUnLock(__FILE__,__LINE__,&Fl_RE,NULL);
	return rc;
};


void	ServerDecod::ReStartTr(int Number)
{
	int	i;
	int	Min,Max;
	MyLock(__FILE__,__LINE__,&Fl_RE,NULL);
	RePackTr();
	if (Number==-1)
	{
		Min=0;Max=MaxID;
	}
	else
	{
		Min=Number;
		Max=Number+1;
	}
	for (i=Min;i<Max;i++)
	{
		if (Tract[i]!=NULL)
		{
				MyLock(__FILE__,__LINE__,&Tract[i]->FlSim,NULL);
				Tract[i]->Stop();
				delete ClThread[i];ClThread[i]=NULL;
				sprintf (TBuf,"Trackt%d -> %s Destroy !!!",i,Tract[i]->Str.Md.Name);
				Srv->OutLog(TBuf);
				Srv->DelDecLink(Tract[i]);
				MyUnLock(__FILE__,__LINE__,&Tract[i]->FlSim,NULL);
				delete Tract[i];Tract[i]=NULL;//FlagCli[i]=0;
				SimTsk.Cli[i].State=0;
		}
		if (SimTsk.Cli[i].Status==1)
		{
			SimTsk.Cli[i].State=0;
			Tract[i]=new Pack_Dec(Srv);
			MyLock(__FILE__,__LINE__,&Tract[i]->FlSim,NULL);
			strcpy(Tract[i]->UInfo.DomainName,SimTsk.Cli[i].DomainName);
			Tract[i]->UInfo.TractID=i;
			Tract[i]->Ext_Mt=i;
			Tract[i]->Srv=Srv;
			Tract[i]->OData=&OutData;
			Tract[i]->ExtPoi=(LPVOID)this;
			Tract[i]->IDOut=i;
// Äëÿ íåéðîíà óæå íå íóæíî (ïðèâîäèëî ê äåäëîêó)
 			Tract[i]->AddDecLink(Srv,2,0);
 			Tract[i]->AddDecLink(Srv,3,1);
 			Tract[i]->AddDecLink(Srv,4,2);
 			Tract[i]->AddDecLink(Srv,5,3);
// Äëÿ íåéðîíà óæå íå íóæíî (ïðèâîäèëî ê äåäëîêó)
// 			Srv->AddDecLink(Tract[i],1000,10);	// ThIDView
// 			Srv->AddDecLink(Tract[i],1001,11);	// IDI_FSrv
// 			Srv->AddDecLink(Tract[i],1002,12);	// BDBInfo
// 			Srv->AddDecLink(Tract[i],1003,13); // NetComm


			sprintf (TBuf,"Create Trackt number %d",i);
			Srv->OutLog(TBuf);
			sprintf (SimTsk.Cli[i].CliName,"Tract%d",i);
			if ((SimTsk.Cli[i].ModeNum>=0)&&(SimTsk.Cli[i].ModeNum<TTsk->Max_Md))
				Tract[i]->ReStruct(TTsk,SimTsk.Cli[i].ModeNum,i);

			ClThread[i]=new ClassThread(SimTsk.Cli[i].CliName,TrtIdle,(LPVOID)i);
			MyUnLock(__FILE__,__LINE__,&Tract[i]->FlSim,NULL);
		}
	}
	MyUnLock(__FILE__,__LINE__,&Fl_RE,NULL);
};

void	ServerDecod::SaveCfg()
{
	int	FH;
	//int		Tmp;
	char	FullName[512];
	MyLock(__FILE__,__LINE__,&Fl_RE,NULL);
	strcpy(FullName,Srv->M_Cfg->MData.PathCFG[Srv->M_Cfg->MData.Active]);
	strcat(FullName,"\\Tracts.cfg");
	
	SetFileAttributesA(FullName,GetFileAttributesA(FullName)&~FILE_ATTRIBUTE_READONLY);
	FH=open(FullName,O_CREAT|O_BINARY|O_RDWR);
	if (FH!=-1)
	{
		USHORT	tLock=Srv->M_Cfg->MData.CRC16SrvCfg^Srv->M_Cfg->MData.CRC16FCli^Srv->M_Cfg->MData.CRC16ShCli^Srv->M_Cfg->MData.CRC16NetCli;
		Srv->M_Cfg->MData.CRC16ShCli=calc_crc16((UCHAR*)&SimTsk,sizeof(SimTsk));
		if (Srv->M_Cfg->MData.Lock==tLock)
			Srv->M_Cfg->MData.Lock=Srv->M_Cfg->MData.CRC16SrvCfg^Srv->M_Cfg->MData.CRC16FCli^Srv->M_Cfg->MData.CRC16ShCli^Srv->M_Cfg->MData.CRC16NetCli;
		write(FH,&SimTsk,sizeof(SimCTask));
		close(FH);
	}
	MyUnLock(__FILE__,__LINE__,&Fl_RE,NULL);
};

void	ServerDecod::LoadCfg()
{
	int	FH;
	char	FullName[512];
	MyLock(__FILE__,__LINE__,&Fl_RE,NULL);
	strcpy(FullName,Srv->M_Cfg->MData.PathCFG[Srv->M_Cfg->MData.Active]);
	strcat(FullName,"\\Tracts.cfg");
	
	FH=open(FullName,O_BINARY);
	if (FH!=-1)
	{
		read(FH,&SimTsk,sizeof(SimCTask));
		close(FH);
	}
	else memset (&SimTsk,0,sizeof(SimCTask));
// 	for (size_t i=0;i<MaxID;i++)
// 	{
// 		SimTsk.Cli[i].ClDec=0;
// 	}
	MyUnLock(__FILE__,__LINE__,&Fl_RE,NULL);
};

void	ServerDecod::SetUsage(size_t ID,double Vol)
{
	if (ID<0x8000)
	{
		TractUsage[ID]=Vol;
	}
	else
	{
		DinTractUsage[ID-0x8000]=Vol;
	}
};

void	ServerDecod::SetDifUsage(size_t ID,double Vol)
{
	if (ID<0x8000)
	{
		TractUsage[ID]+=Vol;
	}
	else
	{
		DinTractUsage[ID-0x8000]+=Vol;
	}
};

void	ServerDecod::SystemIdle()
{
	UCHAR*	tVol;
	size_t	tCnt;
	while (DinDmp.LockOData(tVol,tCnt))
	{
		tVol+=sizeof(SockHeader);
		MyLock(__FILE__,__LINE__,&((TempSrvDecPoi*)tVol)->HandlePointer->FlSim,NULL);
		((TempSrvDecPoi*)tVol)->HandlePointer->Stop();
		delete ((TempSrvDecPoi*)tVol)->ThreadPointer;
		MyUnLock(__FILE__,__LINE__,&((TempSrvDecPoi*)tVol)->HandlePointer->FlSim,NULL);
		delete ((TempSrvDecPoi*)tVol)->HandlePointer;
		DinDmp.UnLockOData();
	}
	
}

void	ServerDecod::ServicesIdle()
{
//	MyLock(__FILE__,__LINE__,&Fl_RE,NULL);
	if (Fl_Strat!=1)
	{
		ReStartTr();
		Fl_Strat=1;
	};
//	Srv->DataIdle();
	MyLock(__FILE__,__LINE__,&Srv->FlSim,NULL);
	m_pItem.iItem=0;
	if ((Srv->Fl_Stop==0)&&(pList)&&(Srv->SData->Fl_StateApp<3))
	{
		if (pList->GetItem(&m_pItem))
		{
			m_pItem.iImage=kImage%32;
			kImage++;
			pList->SetItem(&m_pItem);
		};
	}
	Srv->IdleFunc(-1);
	MyUnLock(__FILE__,__LINE__,&Srv->FlSim,NULL);
	for (size_t i=0;i<MaxID;i++)
	{
		if (Tract[i])
		{
			if (Tract[i]->CntRun!=CntRun[i])
			{
				CntRun[i]=Tract[i]->CntRun;CntTry[i]=0;Tract[i]->Fl_Error=0;
			}
			else
			{
				if (CntTry[i]++>10)
				{
					if (CntTry[i]<100)
					{
						if (ClThread[i])
						{
							if (ClThread[i]->Loads<CntTry[i])
								ClThread[i]->Loads=(double)CntTry[i];
						}
					}
					else
					{
						if (CntTry[i]==102)
							Tract[i]->Fl_Error=1;
						if (CntTry[i]==502)
							Tract[i]->Fl_Error=2;
					}
				}
			}
		}
		if (DinTract[i])
		{
			if (DinTract[i]->CntRun!=CntDinRun[i])
			{
				CntDinRun[i]=DinTract[i]->CntRun;CntDinTry[i]=0;DinTract[i]->Fl_Error=0;
			}
			else
			{
				if (CntDinTry[i]++>10)
				{
					if (CntDinTry[i]<100)
					{
						if (DinThread[i])
						{
							if (DinThread[i]->Loads<CntDinTry[i])
								DinThread[i]->Loads=(double)CntDinTry[i];
						}
					}
					else
					{
						if (CntDinTry[i]==102)
							DinTract[i]->Fl_Error=1;
						if (CntDinTry[i]==502)
							DinTract[i]->Fl_Error=2;
					}
				}
			}
		}
// 		if (Tract[i])
// 		{
//			TractIdle(i);
//		}
	}
//	MyUnLock(__FILE__,__LINE__,&Fl_RE,NULL);
};

void	ServerDecod::TractIdle(size_t ID)
{
	if (MyTryLock(__FILE__,__LINE__,&Tract[ID]->FlSim,NULL))
	{
		if (Tract[ID])
		{
			TractUsage[ID]*=0.99;
			//Glukator
			Tract[ID]->m_pItem.iItem=ID+1;
			if ((Srv->Fl_Stop==0)&&(pList)&&(Srv->SData->Fl_StateApp<3))
			{
				if (pList->GetItem(&Tract[ID]->m_pItem))
				{
					Tract[ID]->m_pItem.iImage=Tract[ID]->kImage%32;
					Tract[ID]->kImage++;
					pList->SetItem(&Tract[ID]->m_pItem);
				};
			}
			if (Srv->System_On==1)
			{
				if (SimTsk.Cli[ID].OnWork!=1)
					SimTsk.Cli[ID].OnWork=1;
				if (SimTsk.Cli[ID].Start==1)
				{
					if (SimTsk.Cli[ID].State!=1)
					{
						//			Tract[ID]->Param("SetPrmPath","Path");
						Tract[ID]->Param("Path",Srv->MainPath);
						MakeDir(SimTsk.Cli[ID].PathOut);
						Tract[ID]->Param("PathOut",SimTsk.Cli[ID].PathOut);
						Tract[ID]->Start();
					}
					SimTsk.Cli[ID].State=1;
				}
				else
				{
					if (SimTsk.Cli[ID].State!=0)
						Tract[ID]->Stop();
					SimTsk.Cli[ID].State=0;
				}
			}
			else
			{
				if (SimTsk.Cli[ID].OnWork!=0)
					SimTsk.Cli[ID].OnWork=0;
			}
			//		MyLock(__FILE__,__LINE__,&Tract[ID]->FlSim,NULL);
			//		Tract[ID]->DataIdle();				// ?????
			Tract[ID]->IdleFunc(ID);
			Tract[ID]->CntRun++;
			//		MyUnLock(__FILE__,__LINE__,&Tract[ID]->FlSim,NULL);
//			Calc(6,(UCHAR*)&ID,1,ID);
		}
		MyUnLock(__FILE__,__LINE__,&Tract[ID]->FlSim,NULL);
	}
//	MyUnLock(__FILE__,__LINE__,&Fl_RE,NULL);
};

void	ServerDecod::DinTractIdle(size_t ID)
{
	ConstrDecF	*tDT=DinTract[ID];
	if (tDT)
	{
		if (MyTryLock(__FILE__,__LINE__,&tDT->FlSim,NULL))
		{
//			tDT->m_pItem.iItem=i;
			DinTractUsage[ID]*=0.99;
 			tDT->IdleFunc(0x8000+ID);
 			tDT->CntRun++;
//			tDT->DataFF(6,(UCHAR*)&ID,1);
			MyUnLock(__FILE__,__LINE__,&tDT->FlSim,NULL);
			if ((Srv->Fl_Stop==0)&&(pList2)&&(Srv->SData->Fl_StateApp<3))
			{
				if (pList2->GetItem(&tDT->m_pItem))
				{
					tDT->m_pItem.iImage=tDT->kImage%32;
					tDT->kImage++;
					pList2->SetItem(&tDT->m_pItem);
				};
			};
		}
	}
	//	MyUnLock(__FILE__,__LINE__,&Fl_RE,NULL);
};

void	ServerDecod::CloseTr()
{
//	MyLock(__FILE__,__LINE__,&Fl_RE,NULL);
//	MyUnLock(__FILE__,__LINE__,&Fl_RE,NULL);
};
// 
// void	ServerDecod::WDog()
// {
// 	// Ñåðâèñ îòêëþ÷åí (íåîáõîäèìî ïðîðàáîòàòü ìåõàíèçì åãî ôóíêöèîíèðîâàíèÿ)
// /*	if (++ClkWDog>10000)
// 	{
// 		ClkWDog=0;
// 		for (size_t i=0;i<MaxID;i++)
// 		  if (Tract[i]!=NULL)
// 			{
// 			if (FlagCli[i]==0)
// 			  {
// 		//		long	t = time(NULL);
// 				sprintf (TBuf,"WDog Close Track %d !!!",i);
// 				Srv->Data(0,(UCHAR*)TBuf,strlen(TBuf)-1);
// 				delete Tract[i];Tract[i]=NULL;SDec->SData->Ab[i].ID=-1;
// 			  }
// 			else
// 			  {
// 			  FlagCli[i]=0;
// 			  SDec->SendData(100,NULL,0,i);
// 			  };
// 			};
// 	}
// */
// };
// 

void	ServerDecod::Calc(int N_Pin,UCHAR* pBuf,int Len,int ID)
{
	int	Fl=0;
//	int	i;
	char	FullName[256];
//	MyLock(__FILE__,__LINE__,&Fl_RE,NULL);
	MyLock(__FILE__,__LINE__,&Tract[ID]->FlSim,NULL);
	if (Tract[ID]!=NULL)
	{
		TmpIDTrct=ID;
		Srv->ClearDebugLine();
		sprintf(FullName,"%s:%s",Tract[ID]->Name_Mt,Tract[ID]->Str.Md.Name);
		Srv->AddDebugLevel(Tract[ID],FullName);
		Tract[ID]->Data(N_Pin,pBuf,Len);
		Tract[ID]->CntRun++;
		Srv->SubDebugLevel(Tract[ID]);
		TmpIDTrct=-1;
	};
	MyUnLock(__FILE__,__LINE__,&Tract[ID]->FlSim,NULL);
//	MyUnLock(__FILE__,__LINE__,&Fl_RE,NULL);
/*
	MyLock(__FILE__,__LINE__,&Srv->hmtxMainThr,Srv);
//		int rc=WaitForSingleObject(Srv->hmtxMainThr,INFINITE);
//long	t;
if (TmpIDTrct!=-1)
{
	Srv->SetCriticalError(Srv,"Error using Neuron::Calc Reenterable","Warning","Neuron");
}
if (ID!=-1)
	TmpIDTrct=ID;

//,FH;
//char	TBuf[256];
char	FullName[512];
if (((SD_Pack*)pBuf)->Type!=15) SpeedIn+=Len;
//if (Len==521)
// Len=Len*2-Len;
if (Srv->M_Cfg->MData.ProtectMode!=1) goto _tm01;
_tm01:
//if (On==1)
// 
//Srv->Idle();
switch(((SD_Pack*)pBuf)->Type)
	{
	case 1:		// Ñîçäàòü òðàêò äåêîäåðà
		if (Tract[ID]==NULL)
		  {
			FlagCli[ID]=1;
			Tract[ID]=new Pack_Dec(Srv);
			Tract[ID]->Ext_Mt=ID;
			Tract[ID]->Srv=Srv;
			Tract[ID]->OData=&OutData;
			Tract[ID]->ExtPoi=(LPVOID)this;
			Tract[ID]->IDOut=ID;
			Tract[ID]->AddDecLink(Srv,2,0);
			Tract[ID]->AddDecLink(Srv,3,1);
			Tract[ID]->AddDecLink(Srv,4,2);
			Tract[ID]->AddDecLink(Srv,5,3);
			Srv->AddDecLink(Tract[ID],1000,10);	// ThIDView
			Srv->AddDecLink(Tract[ID],1001,11);	// IDI_FSrv
			Srv->AddDecLink(Tract[ID],1002,12);	// BDBInfo
			Srv->AddDecLink(Tract[ID],1003,13); // NetComm
			sprintf (TBuf,"Create Trackt number %d",ID);
			Srv->OutLog(TBuf);
		  };
	break;
	case 2:		// Óêàçàòü øàðåíóþ ïàìÿòü
		if (Tsk[ID]!=NULL)
			 {UnmapViewOfFile(Tsk[ID]); CloseHandle(MapT[ID]);Tsk[ID]=NULL;}
		sprintf(TBuf,"%s",pBuf+sizeof(SD_Pack));
//		DosGetNamedSharedMem(&(void*)Tsk[ID],TBuf,PAG_WRITE | PAG_READ);
	MapT[ID]=OpenFileMapping(FILE_MAP_ALL_ACCESS,TRUE,TBuf);
	Tsk[ID]=(Task*)MapViewOfFile(MapT[ID],FILE_MAP_ALL_ACCESS,0,0,0);

		break;
	case 3:		// Óêàçàòü íîìåð ðåæèìà â øàðåíîé ïàìÿòè
		if (Tract[ID]!=0) Tract[ID]->ReStruct(Tsk[ID],((SD_Pack*)pBuf)->Param);
		break;
	case 4:		// Íàñòðîèòü øèðîêîâåùàòåëüíûå ïàðàìåòðû
		if (Tract[ID]!=0) Tract[ID]->Param((((PrmDec*)(pBuf+sizeof(SD_Pack)))->Prm),(((PrmDec*)(pBuf+sizeof(SD_Pack)))->Vol));
//		Tract[ID]->DirectParam(":BM_SubTr_Exampl\\SubTr@0:BM_FS-1045\\SaveFile@1#Ext",".bin");
		break;
	case 5:		// Start
		if (Tract[ID]!=0)
		{
			sprintf (TBuf,"Post Start in Trackt number %d",ID);
			Srv->OutLog(TBuf);
			Tract[ID]->Start();
		};
		break;
	case 6:		// Stop
		if (Tract[ID]!=0)
		{
			sprintf (TBuf,"Post Stop in Trackt number %d",ID);
			Srv->OutLog(TBuf);
			Tract[ID]->Stop();
		};
		break;
	case 7:		// Óêàçàòü øàðåíóþ ïàìÿòü

		if (Tsk[ID]!=NULL) {UnmapViewOfFile(Tsk[ID]); CloseHandle(MapT[ID]);Tsk[ID]=NULL;}
//		sprintf(TBuf,"\\SHAREMEM\\%s",Buf+sizeof(SD_Pack));

//int	Fl=0;	
		MapT[ID]=OpenFileMapping(FILE_MAP_ALL_ACCESS,TRUE,"ServerDec_DB");
		if (MapT[ID]==NULL)
		{
			Fl=1;
			MapT[ID]=CreateFileMapping((HANDLE)-1,NULL,PAGE_READWRITE|SEC_COMMIT,0,sizeof(Task),"ServerDec_DB");
		};
		Tsk[ID]=(Task*)MapViewOfFile(MapT[ID],FILE_MAP_ALL_ACCESS,0,0,0);
		if (Tsk[ID]==NULL)
		{
//
			sprintf (TBuf,"Error open Shared Memory in Trackt number %d",ID);
			Srv->OutLog(TBuf);
			throw;
		};
		if (Fl==1)
		   {
			   DataImEx	*DataIE;
			   DataIE=new DataImEx();
			   memset(Tsk[ID],0x0,sizeof(Task));
			   strcpy(FullName,Srv->M_Cfg->MData.PathCFG[Srv->M_Cfg->MData.Active]);
			   strcat(FullName,"\\Neuron.cfg");
			   DataIE->ImportTsk(Tsk[ID],FullName);
			   delete DataIE;
		   };
		break;
	case 8:		// Óêàçàòü èìÿ ðåæèìà â øàðåíîé ïàìÿòè
		for (i=0;i<Tsk[ID]->Max_Md;i++)
		   if (strcmp(Tsk[ID]->Md[i].Name,(char*)(pBuf+sizeof(SD_Pack)))==0)
		     if (Tract[ID]!=NULL)
			 {
				Tract[ID]->ReStruct(Tsk[ID],i);
//				t = time(NULL);
				sprintf (TBuf,"Create Trackt -> %s",Tsk[ID]->Md[i].Name);
				Srv->OutLog(TBuf);
				break;
			 }
		break;
	case 9:		// Ñïèñîê
		for (i=0;i<Tsk[ID]->Max_Md;i++)
		   SDec->SendData(20,(UCHAR*)Tsk[ID]->Md[i].Name,40,ID);
		SDec->SendData(20,(UCHAR*)"End",40,ID);
		break;
	case 10:	// Ïàêåò ñ äàííûìè
		if (Tract[ID]!=NULL)
		{
			TmpIDTrct=ID;
			Srv->ClearDebugLine();
			sprintf(FullName,"%s:%s",Tract[ID]->Name_Mt,Tract[ID]->Str.Md.Name);
			Srv->AddDebugLevel(FullName);
			Tract[ID]->Data(N_Pin,pBuf+sizeof(SD_Pack),Len-sizeof(SD_Pack));
			Srv->SubDebugLevel();
		};
	break;
	case 11:	// Loop Back         ??????????????????????
		SDec->SendData(20,pBuf+sizeof(SD_Pack),Len-sizeof(SD_Pack),ID);
		break;
	case 15:	// Ñîáûòèå îò òàéìåðà äëÿ âñåõ òðàêòîâ
		for(ID=0;ID<MaxID;ID++)
		{
			if (Tract[ID]!=NULL)
			{
				TmpIDTrct=ID;
				Srv->ClearDebugLine();
				sprintf(FullName,"%s|%s",Tract[ID]->Name_Mt,Tract[ID]->Str.Md.Name);
				Srv->AddDebugLevel(FullName);
				Tract[ID]->Data(N_Pin,pBuf+sizeof(SD_Pack),Len-sizeof(SD_Pack));
				Srv->SubDebugLevel();
			};
		}
		Srv->Idle();
		break;
	case 20:	// ReStruct Trackt
		if (Tsk[ID]!=NULL) {UnmapViewOfFile(Tsk[ID]); CloseHandle(MapT[ID]);Tsk[ID]=NULL;}
		if (Tract[ID]!=NULL) 
		{
//		    t = time(NULL);
			Tract[ID]->Stop();
			sprintf (TBuf,"Trackt -> %s Destroy !!!",Tract[ID]->Str.Md.Name);
			Srv->OutLog(TBuf);
//			delete Tract[ID];Tract[ID]=NULL;FlagCli[ID]=0;
			if (Srv->M_Cfg->MData.ProtectMode!=1) goto _tm02;
			try
			{
_tm02:

				Srv->DelDecLink(Tract[ID]);
			    delete Tract[ID];Tract[ID]=NULL;FlagCli[ID]=0;
			}
			catch(...)
			{
				Srv->M_Cfg->MData.NumError++;
//			    t = time(NULL);
				sprintf (TBuf,"Error Destroing !!!");
				Srv->OutLog(TBuf);
			}
		};
		break;
	case 100:		// Stop
		FlagCli[ID]=1;
		break;
	default:;
	};
*/
/*
}
catch(...)
{	// Íåïîíÿòíàÿ îøèáêà
	_asm;
}
	Srv->M_Cfg->MData.NumError++;
//    t = time(NULL);
	if (Tract[ID]->Act_N_Mt!=-1)
		sprintf (TBuf,"Unknow Error Neuron !!! Trackt -> %s Meth -> %s",Tract[ID]->Str.Md.Name,Tract[ID]->Str.Mt[Tract[ID]->Act_N_Mt].Name);
	else
		sprintf (TBuf,"Unknow Error Neuron !!! Trackt -> %s Meth -> ?",Tract[ID]->Str.Md.Name);

	Srv->OutLog(TBuf);
// Ïûòàåìñÿ óäàëèòü òðàêò
	if (Tract[ID]!=NULL)
	{
//	    t = time(NULL);
		sprintf (TBuf,"Trackt -> %s Destroy !!!",Tract[ID]->Str.Md.Name);
		Srv->OutLog(TBuf);
		SDec->SendData(22,(UCHAR*)TBuf,strlen(TBuf)+1,ID);
		if (Srv->M_Cfg->MData.ProtectMode!=1) goto _tm03;
		try

		{
_tm03:
			Srv->DelDecLink(Tract[ID]);
		    delete Tract[ID];Tract[ID]=NULL;FlagCli[ID]=0;
		}
		catch(...)
		{
//		    t = time(NULL);
			sprintf (TBuf,"Error Destroing !!!");
			Srv->OutLog(TBuf);
			// Âåðîÿòíî íåîáõîäèìî çàêðûòü ñåðâåð
		}
	}
}
*/
//
//ReleaseSemaphore(Srv->hmtxMainThr,1,NULL);
//MyUnLock(__FILE__,__LINE__,&Srv->hmtxMainThr,Srv);

};

// -----------------------------------------------------------------------------------

void	ConnectFunctionN(ElSockCliEx* obj)
{
	if(obj->Flag==1)
	{
		obj->DisConnect();
	}
	obj->Connect();
	if(obj->Flag==1)
	{
		if (obj->State!=2)
		{
			//obj->State=2;
			//obj->FlagEvent|=SockCliConnect;
			obj->SetEvent(0);
		}
	}
	else
	{
		obj->State=0;
	}
	obj->StartConn=2;
	obj->Debug("ExitThread");
	ExitThread(16);
}


int ElSockCliEx::SetParam(char* Prm,char* Vol)
{
	int	rc=0;
	if (strcmp(Prm,"Mode")==0) 
	{
		if (strcmp(Vol,"NoBufNoPack")==0) Mode=0;
		if (strcmp(Vol,"BufNoPack")==0) Mode=1;
		if (strcmp(Vol,"BufPack")==0) Mode=2;
	}
	if (strcmp(Prm,"ServerIP")==0)
	{
		rc=1;
		strcpy(IPAddr,Vol);
		GetHostName();
	}
	if (strcmp(Prm,"ServerPort")==0)	{rc=1; Port=atoi(Vol);}

	if (strcmp(Prm,"SizeTrBuf")==0)
	{SizeCB(atoi(Vol));}
	if (strcmp(Prm,"BlockTimeOut")==0)	{TimeOut=atoi(Vol);}

	if (rc==1)
	{
		int fl=Enable;
		
//		if(fl) Stop(); 
		ReInit(IPAddr,Port,HostName); 
//		if(fl) Start(); 
	}
	return 0;
}
int ElSockCliEx::Start()
{
	Enable=1;
	return 0;
}
int ElSockCliEx::Stop()
{
	Enable=0;
	if(State==2)
	{
		State=3;
		DisConnect();
		//State=0;
		//FlagEvent|=SockCliDisConnect;
		SetEvent(1);
	}
	return 0;
}
int	ElSockCliEx::ObjIdle()
{
/*
	State:
	0 - DISCONNECT
	1 - ïîïûòêà êîíåêòà
	2 - CONNECT
	3 - ïîïûòêà äèñêîíåêòà

*/
	if(State==0)
	{
		if(StartConn==2)
		{
			if(hThConn)
			{
				Debug("CloseHandle");
				CloseHandle(hThConn);
				hThConn=0;

				DebugCount=1;
			}
		}
	}

	if(DebugCount)
	{
		if(++DebugCount<20) 
			return 0;
		DebugCount=0;
	}	
	
	if(Enable==1) 
	{
		if(State==0)
		{
			State=1;
			if (StartConn==2)
			{
				StartConn=0;
				if(hThConn)
				{
					CloseHandle(hThConn);
					hThConn=0;
				}
			}
			StartConn=1;
			Debug("CreateThread");
			hThConn = CreateThread(NULL,// pointer to security attributes
				0,						// initial thread stack size
				(LPTHREAD_START_ROUTINE)ConnectFunctionN,// pointer to thread function
				(void*)this,			// argument for new thread
				0,						// creation flags
				(LPDWORD)&TID_Conn);	// pointer to receive thread ID); 
		}
		if(State==2) if(Mode==2)
		{
			// ElSockCliEx	:	public NetObject, public ClientSockEx
			if (--tDiv<0)
			{
				tDiv=SetDiv;
				SendBuf((char*)&WatchDogPack,sizeof(int));
			}
		}
		// íåïðåäíàìåðåííûé ðàçðûâ ñîêåòà
		if(State==2) if(Flag==0)
		{
			State=3;
			DisConnect();
			//State=0;
			//FlagEvent|=SockCliDisConnect;
			SetEvent(1);
		}
	}
	return 0;
}

int	ElSockCliEx::SetEvent(int Event)
{
	int FlagEvent=0; State=0;
	
	switch(Event)
	{
	case 0:
		State=2;
		FlagEvent=SockCliConnect;
		break;
	case 1:
		State=0;
		FlagEvent=SockCliDisConnect;
		break;
	}

	if(FlagEvent)
	{
		FlagEventArray[index_w]=FlagEvent;
		index_w=(index_w+1)&index_m;
	}

	return FlagEvent;
};

int	ElSockCliEx::RdEvent(PUCHAR &p, size_t &c)
{
	c=0; if(index_w==index_r) return 0;

	int FlagEvent=FlagEventArray[index_r];
	index_r=(index_r+1)&index_m;

	switch(FlagEvent) {
	case SockCliConnect:
		{
			tEMsg.Event=CBE_Connect;
			p=(UCHAR*)&tEMsg.Event;
			c=sizeof(tEMsg.Event);
		}
		break;
	case SockCliDisConnect:
		{
			tEMsg.Event=CBE_DisCon;
			p=(UCHAR*)&tEMsg.Event;
			c=sizeof(tEMsg.Event);
		}
		break;
	default:;
	}

	return c;
}

int	ElSockCliEx::GetHostName()
{
	//int i;
	*HostName=0;

	//////////////////////////////////////////////////////////////////////////
	HostName[0]=0;//????
	/////////////////////////////////////////////////////////////////////////]
	int FirstChar=HostIP[0];
	if(('0'<=FirstChar)&&(FirstChar<='9'))
	{
		if (pSrv->SInt)
		{				
			FindStrList8	FSL85;
			El_CliInt*	Cli=(El_CliInt*)pSrv->SInt->CliLAN.Begin(FSL85);
			while (Cli)
			{
//				if (pSrv->SInt)
				{				
//					if(pSrv->SInt->CliLAN[i])
					{
						if(strcmp(Cli->Cli.IPAddr,IPAddr)==0)
						{
							strcpy(HostName,Cli->Cli.Host);
							return 1;
						}
					}
				}	
				Cli=(El_CliInt*)pSrv->SInt->CliLAN.Next(FSL85);
			}
		}
	}
	else
	{
		// à òóò ïîêà òóïîå êîïèðîâàíèå
		strcpy(HostName,IPAddr);
	}

	return 0;
}

int ElSockCliEx::Debug (char *q)
{
	return 0;

	char s[1024];
	SYSTEMTIME time;
	GetLocalTime(&time);

	sprintf(s,"%02ld/%02ld/%02ld %02ld:%02ld:%02ld.%03d  %s\r\n",
		time.wDay,time.wMonth,time.wYear,
		time.wHour,time.wMinute,time.wSecond,time.wMilliseconds,q);
	sprintf(s,"%02ld:%02ld:%02ld.%03d  %s\r\n",
		time.wHour,time.wMinute,time.wSecond,time.wMilliseconds,q);

	int h=open("\\ElSockCliEx.log",O_BINARY|O_CREAT|O_WRONLY,S_IWRITE);
	if(h!=-1)
	{
		lseek(h,0,SEEK_END);
		write(h,s,strlen(s));
		close(h);
	}

	return 0;
}

// -----------------------------------------------------------------------------------
int fCallBack(ClientSock* S,int Event)
{
	return ((ElSockCliEx*)S)->SetEvent(Event);
};

int	ElSockSrvEx_EventFun(ServerClientSock *csock,int Event)
{
	pElSockSrvEx ptr=(pElSockSrvEx)csock->servSock;
	ptr->WrEvent(csock,Event);
	return 0;
}
int ElSockSrvEx::WrEvent(ServerClientSock* csock,int Event)
{
	int q=WrMsg-1;
	if(q<0) q=0x3ff;
	EventMsg* ptr=&EvMsg[q];

	EvMsg[WrMsg].Event=Event;
	EvMsg[WrMsg].Handle=csock->Handle;
//	strcpy(EvMsg[WrMsg].Host,csock->namehost);
	EvMsg[WrMsg].IPadr=csock->IPadr;
	strcpy(EvMsg[WrMsg].IPhost,csock->IPhost);
	strcpy(EvMsg[WrMsg].namehost,csock->namehost);
	EvMsg[WrMsg].Number=csock->NumInList;
	EvMsg[WrMsg].port=csock->port;
	WrMsg=(WrMsg+1)&0x3ff;
	return 0;
}
EventMsg* ElSockSrvEx::RdEvent()
{
	EventMsg *pMess=0;
	if (WrMsg!=RdMsg)
	{
		pMess=&EvMsg[RdMsg];
		RdMsg=(RdMsg+1)&0x3ff;
	}
	return pMess;
}
int ElSockSrvEx::SetParam(char* Prm,char* Vol)
{
	if (strcmp(Prm,"Mode")==0) 
	{
		if (strcmp(Vol,"NoBufNoPack")==0) Mode=0;
		if (strcmp(Vol,"BufNoPack")==0) Mode=1;
		if (strcmp(Vol,"BufPack")==0) Mode=2;
	}
	if (strcmp(Prm,"ServerPort")==0)	{ReInit(atoi(Vol));}
	if (strcmp(Prm,"SizeTrBuf")==0)	
	{SizeCB(atoi(Vol));}
	if (strcmp(Prm,"BlockTimeOut")==0)	{TimeOut(atoi(Vol));}

	return 0;
}
int ElSockSrvEx::Start()
{
	return 0;
}
int ElSockSrvEx::Stop()
{
	return 0;
}
int	ElSockSrvEx::ObjIdle()
{
	if(Mode==2)
	{
		// ElSockSrvEx	:	public NetObject, public ServerSockEx
		if (--tDiv<0)
		{
			tDiv=SetDiv;
			SendBuf(-1,(UCHAR*)&WatchDogPack,sizeof(int));
		}
	}
	
	return 0;
}
// -----------------------------------------------------------------------------------

ServerDecod::~ServerDecod()
{
	size_t i;
	
	for(i=0;i<MaxID;i++)
	{
//		if (Tsk[i]!=NULL) {UnmapViewOfFile(Tsk[i]); CloseHandle(MapT[i]);Tsk[i]=NULL;}
		if (Tract[i]!=NULL)
		{
			MyLock(__FILE__,__LINE__,&Tract[i]->FlSim,NULL);
			delete ClThread[i];ClThread[i]=NULL;
			Tract[i]->Stop();
			sprintf (TBuf,"Trackt%d -> %s Destroy !!!",i,Tract[i]->Str.Md.Name);
			Srv->OutLog(TBuf);
			Srv->DelDecLink(Tract[i]);
			ConstrDecF*	tTract=Tract[i];
			Tract[i]=NULL;
			MyUnLock(__FILE__,__LINE__,&tTract->FlSim,NULL);
			delete tTract;
//			FlagCli[i]=0;
// 
// 			delete	ClThread[i];
// 			Tract[i]->Stop();
// 			Srv->DelDecLink(Tract[i]);
// 			delete Tract[i];
// 			Tract[i]=NULL;
// 			ClThread[i]=NULL;
// //			FlagCli[i]=0;
		};
	};
	while(DinDmp.CntInside)
	{
		Sleep(10);
	};
	delete	SrvThread;
	for(i=0;i<MaxID;i++)
	{
		if (DinTract[i]!=NULL)
		{
			ConstrDecF*	tTract=DinTract[i];
			MyLock(__FILE__,__LINE__,&tTract->FlSim,NULL);
			delete DinThread[i];DinThread[i]=NULL;
			DinTract[i]->Stop();
			DinTract[i]=NULL;
			MyUnLock(__FILE__,__LINE__,&tTract->FlSim,NULL);
			delete tTract;
		};
	};
	delete	SysThread;
	Srv->Neuron=NULL;
};


	Services::Services(HANDLE	MApp,char	*PLog,char	*Path,char	*PathDll,char	*PBD,int FlRI, CMainCfg* MCfg,int NDay,int FlLocGlob,int FlSetTime,int NDCE)
{
// Ýòîò ôðàãìåíò äîëæåí áûòü âñòàâëåí âî âñå ïîñëåäóþùèå îáúåêòû
// Services_ äîëæåí áûòü çàìåíåí íà èìÿ îáúåêòà
		 Name_Mt=Services_NameMt;Rem_Mt=Services_RemMt;Ver_=Services_Version;Auth_=Services_Author;
		 PinName_I=Services_PinNameI;PinName_O=Services_PinNameO;Prm_=Services_Prm;Vol_=Services_Vol;

		 Num_PinI=0;while (PinName_I[2*Num_PinI][0]!=0x0) Num_PinI++;
		 Num_PinO=0;while (PinName_O[2*Num_PinO][0]!=0x0) Num_PinO++;
		 Num_Prm=0;while (Prm_[2*Num_Prm][0]!=0x0)Num_Prm ++;
		 Type_Mt=Services_Type;UserStatus=0;
// -------NetServices--------------------------------------------
		 CntCliPack=0;CntNumDec=0;
		 ElHeap=0;
		 State=0;
		 Counter=0;
		 TimeClick=0;; 
//		 pElement=NULL;		// îïðåäåëÿåò ïîâåäåíèå ìîäóëÿ ïî õâ. âîçäåéñòâèþ
//		 pElementOut=NULL;	// ïîñëå TimeClick ïðîèçâîäèòñÿ îïðîñ âñåõ îáúåêòîâ ñïèñêà
		 NetDmp.Srv=this;
		 StateOut=0;
		 CustomerInfo=NULL;
//		 pMess=NULL;
		 SockBuf=NULL; 
		 SockCnt=0;
		UserID=-1;
		CntGUIActivities=0;
		RCntGUIActivities=0;
		LastUserActivitiesTime.Time=0;

		 StopProcessing=0;

		 TelemetryCount=0;

		 ClientList=new ID_List8();
		 MonClientList=new ID_List8();
		 ActiveDecoderList=new ID_List8();
		 ID_ElemPTR*	ElPTR=new	ID_ElemPTR(this);
		 ActiveDecoderList->Add(ElPTR);

		 ListUDPCli=new ID_List8();

//--------------------------------------------------------------
	Neuron=NULL; 	DbgLine[0]=0;
	AddList=NULL;
	Fl_Recurce=0;
	WANCli1FT.Time=0;
	WANCli2FT.Time=0;
	WANSISrv=Cli1=Cli2=0; ShortHostName[0]=0;// DomainName[0]=0;
	SizeNetMen=1024;
//	NetMenBuf=(uchar*)malloc(SizeNetMen);
	CDiv3=5;
	ActCli=-1;NumWANCli=0;
	MapApp=MApp;
	Fl_UpData=0;
	Fl_UpData2=0;
	SData=(StrData*)MapViewOfFile(MapApp,FILE_MAP_ALL_ACCESS,0,0,0);
	SData->Fl_StateApp=1;
	int	i=0;
	HANDLE	FlMod;
	char	FullMod[256];
	// ===========Neuron.exe
	pMod[i].ModuleHandle=(HINSTANCE)-1;
	strcpy(pMod[i].Name,"Neuron.exe");
	sprintf(FullMod,"%s\\%s",Path,pMod[i].Name);
	FlMod=CreateFileA(FullMod,GENERIC_READ,FILE_SHARE_READ,NULL,OPEN_EXISTING,FILE_ATTRIBUTE_NORMAL,NULL);  
	GetFileTime(FlMod,&pMod[i].CrTime.FTime,&pMod[i].AcTime.FTime,&pMod[i].WrTime.FTime);
	CloseHandle(FlMod);i++;
	// ===========BDBcon.exe
	pMod[i].ModuleHandle=(HINSTANCE)-1;
	strcpy(pMod[i].Name,"BDBcon.exe");
	sprintf(FullMod,"%s\\%s",Path,pMod[i].Name);
	FlMod=CreateFileA(FullMod,GENERIC_READ,FILE_SHARE_READ,NULL,OPEN_EXISTING,FILE_ATTRIBUTE_NORMAL,NULL);  
	GetFileTime(FlMod,&pMod[i].CrTime.FTime,&pMod[i].AcTime.FTime,&pMod[i].WrTime.FTime);
	CloseHandle(FlMod);i++;
	// ===========TD.cfg
	pMod[i].ModuleHandle=(HINSTANCE)-1;
	strcpy(pMod[i].Name,"TD.cfg");
	sprintf(FullMod,"%s\\%s",Path,pMod[i].Name);
	FlMod=CreateFileA(FullMod,GENERIC_READ,FILE_SHARE_READ,NULL,OPEN_EXISTING,FILE_ATTRIBUTE_NORMAL,NULL);  
	GetFileTime(FlMod,&pMod[i].CrTime.FTime,&pMod[i].AcTime.FTime,&pMod[i].WrTime.FTime);
	CloseHandle(FlMod);i++;
	// ===========GeoIPCountryWhois.csv
	pMod[i].ModuleHandle=(HINSTANCE)-1;
	strcpy(pMod[i].Name,"GeoIPCountryWhois.csv");
	sprintf(FullMod,"%s\\%s",Path,pMod[i].Name);
	FlMod=CreateFileA(FullMod,GENERIC_READ,FILE_SHARE_READ,NULL,OPEN_EXISTING,FILE_ATTRIBUTE_NORMAL,NULL);  
	GetFileTime(FlMod,&pMod[i].CrTime.FTime,&pMod[i].AcTime.FTime,&pMod[i].WrTime.FTime);
	CloseHandle(FlMod);i++;
	// ===========SmartStarter.exe
	pMod[i].ModuleHandle=(HINSTANCE)-1;
	strcpy(pMod[i].Name,"SmartStarter.exe");
	sprintf(FullMod,"%s\\%s",Path,pMod[i].Name);
	FlMod=CreateFileA(FullMod,GENERIC_READ,FILE_SHARE_READ,NULL,OPEN_EXISTING,FILE_ATTRIBUTE_NORMAL,NULL);  
	GetFileTime(FlMod,&pMod[i].CrTime.FTime,&pMod[i].AcTime.FTime,&pMod[i].WrTime.FTime);
	CloseHandle(FlMod);i++;
	//----------------------
	MemClasterCnt=0;

	for (i=0;i<SizeUDPPort;i++)
	{
		UDPPortFl[i]=FALSE;
	};

	for (i=0;i<SizeMC;i++)
	{
		AllocMC[i].Fl=0;
		AllocMC[i].Cnt=0;
		AllocMC[i].Buf=NULL;
		FreeMC[i].Fl=0;
		FreeMC[i].Cnt=0;
		FreeMC[i].Buf=NULL;
	}
	for (i=0;i<MaxNumWANCli;i++)
	{
		FlWANCli[i]=FALSE;
	}
	for (i=0;i<SizeMFBuf;i++)
	{
		MedFltBuf[i]=0;
	}
	IndexMF=0;
	Fl_ExtSin=0;
	DBGC.Cnt_El_Tab=DBGC.Cnt_IDI_Info=DBGC.Cnt_IDList8=DBGC.Cnt_pList=DBGC.MemSize=0;
	SetServices(this);Fl_HardLock=1;
//	IDT_Prm=NULL;
	IDT_NeuronGlobal=NULL;IDT_NeuronLocalCustomer=NULL;IDT_NeuronLocalSite=NULL;/*IDT_ComSys=NULL;*/Fl_Prm=0;Fl_ReadyRepl=0;Fl_StU=6000;pMyErr=0;
	FPurger=NULL;
//	Mt_IDIRepl=NULL;
//	Fl_GIDI=0;
	SInt=NULL;UM=0;
//	Fl_MT=Fl_MTSI=Fl_MTSI2=Fl_MTSG=Fl_MTSID=Fl_Idle=0;Fl_hmtx=0;
	M_Cfg=MCfg;
	memset((UCHAR*)&IPList,0,sizeof(IPList));
	memset((UCHAR*)&LCond,0,sizeof(LCond));
	LCond.CustomerID = 1;
	memset((UCHAR*)&NetLCond,0,sizeof(LCond));
	memset((UCHAR*)&NetLUinfo,0,sizeof(NetLUinfo));
	IDInfoCLst=new ID_List8();
	ActIDIList=new ID_List8();
	DelIDIList=new ID_List8();
	DllFileQwe=new ID_List8();
	MainFileQwe=new ID_List8();
	VDlg=NULL;Fl_SGUI=0;CntIdle=0;
	ThIDView=NULL;NetCommLAN=NULL;//NetCommWAN=NULL;BDBIDInfo=NULL;
//	UnitView=NULL;
	Srv=this;
	CntHost=0;

	TimeConst=GetNClk();
	Sleep(1000);	// 1 s.
	NClickOld=GetNClk();
	Fcpu=double(NClickOld-TimeConst);
	Fcpu_1=10000000.0/Fcpu;
	//	Fcpu=double(NClickOld-TimeConst)/(((DOUBLE)FTime.Time-(DOUBLE)FTime1.Time)/KtimeS);

	FileTime	FTime;
	GetSystemTimeAsFileTime(&FTime.FTime);
	TimeConst=INT64 (Fcpu/1000);
	RealTime=FTime.Time;
	RndVol+=RealTime;

	DmpCE=new DamperPS();
	DmpCE->SetPrm(this);DmpCE->Srv=this;
	DmpDCE=new DamperPS();
	DmpDCE->SetPrm(this);DmpDCE->Srv=this;
	Log=new Savelog();
	IDL_Log=new Log_List();
	PathLog=PLog;
	PathBD=PBD;
	Log->Param("PathLog",PLog);
	Log->Param("AppendTime","on");
	Log->Param("AppendLF","OnEnd");
	Log->Start();
	FSrv=NULL;TableList=NULL;
	BL_Log=new BandleList();
	BL_Telemetr=new BandleList();
	DbgBList=new BandleList();
	SizeAllocMem=0;
	DLLL=0;System_On=0;Fl_TimeSyn=0;
	Fl_ConnectSrv=0;
	NDayCE=NDCE;NDayLog=NDay;CntTK=0;tObj=NULL;MemUsage=0;TimeInt=5000;
	TypeTime=FlLocGlob;TGID.Time=0;//InternalErr=0;
	FlSetT=FlSetTime;FlReCur=0;CE1=0;
	CntTime=990;OldErr=0;difErrOld=0;FreqErr=0;sErr=0;DispErr=0;DispdifErr=0;KdifCnt=0;
	oNClk=oldNClk=0;dFcpu=odFcpu=0;VardifErr=0;
	SmTime=0;MVarErr=0;
	TimeErr=0;
	Fl_FSyn=0;
	PoiNTF2=0;PoiNTF=0;TS_CE=0;OldDErr=0;sDErr=0;
	for (i=0;i<NumTF;i++)
	{
		Err[i]=difErr[i]=0;
	};
	FlReInit=FlRI;
//	ECl=ECl2=NULL;
	Ver=mVersion;
	NumEvent=NumSICli;
//	Chron=new Chronometry(this);
//	CString	UName;
	EventFn=new ID_List8();
	TraceLst=new ID_List8();
	ULONG	Cnt=256;
	char	tUsN[256];
	Fl_Repl=0;	// Off
	TimGUI=0;
	CntV=0;CntT=0;CntT2=0;CntT3=0;CntT4=0;CntT5=0;CntT6=0;CntT7=0;CntT8=0;CntT9=0;CntTS4=0; CPUUsage=0;CPUUsageS=0;CPUUsageSS=0;
	CDiv2=0;
//	NC=0;
//	hmtxMainThr=OpenSemaphore(SEMAPHORE_ALL_ACCESS	,TRUE,"MainThrNeuron");
//	if (hmtxMainThr==NULL)
//		hmtxMainThr=CreateSemaphore(NULL,1,1,"MainThrNeuron");

//	hmtx=OpenSemaphore(SEMAPHORE_ALL_ACCESS	,TRUE,"ServiceSem");
//	if (hmtx==NULL)
//		hmtx=CreateSemaphore(NULL,1,1,"ServiceSem");
	hmtxlog=OpenSemaphore(SEMAPHORE_ALL_ACCESS	,TRUE,_T("SrvSemLog"));
	if (hmtxlog==NULL)
		hmtxlog=CreateSemaphore(NULL,1,1,_T("SrvSemLog"));
//	strcpy(MainPath,Path);
	Tsk=NULL;
	PortD=CreateFile( _T("\\\\.\\PORTD"), 
	GENERIC_READ | GENERIC_WRITE, 
	0, NULL, OPEN_EXISTING, 0, 0);	
	CDiv=0;
//	Èìÿ è àäðåññ êîìïüþòåðà
//	UName=GetUserName();
	GetUserNameA(tUsN,&Cnt);
	NumIP=0;
	strcpy(HostName,"Host");
#ifndef Win9x
	gethostname (HostName,255);
#endif
	PathCFG[0]=0;
#ifndef Win9x
	hoststr=gethostbyname(HostName);
	if (hoststr)
	{
		int IPDef=0x7f000001;
		memcpy((char*)&IPAddrPAN,&IPDef,4);
		memcpy((char*)&IPAddrLAN,&IPDef,4);
		memcpy((char*)&IPAddrWAN,&IPDef,4);
		for (int jj=0;jj<5;jj++)
		{
			try
			{
				if (hoststr->h_addr_list[jj]==0)
					break;
				memcpy((char*)&IPDef,hoststr->h_addr_list[jj],4);
				if ((IPDef&MCfg->MData.IP_MaskPAN)==(MCfg->MData.IP_PAN&MCfg->MData.IP_MaskPAN))
					memcpy((char*)&IPAddrPAN,hoststr->h_addr_list[jj],4);
				if ((IPDef&MCfg->MData.IP_MaskLAN)==(MCfg->MData.IP_LAN&MCfg->MData.IP_MaskLAN))
					memcpy((char*)&IPAddrLAN,hoststr->h_addr_list[jj],4);
				if ((IPDef&MCfg->MData.IP_MaskWAN)==(MCfg->MData.IP_WAN&MCfg->MData.IP_MaskWAN))
					memcpy((char*)&IPAddrWAN,hoststr->h_addr_list[jj],4);
				NumIP++;
			}
			catch(...)
			{
				break;
			}
		}
		strcpy(HostName,hoststr->h_name);
	}
#endif
	*(ULONG*)&IPAddrWANCli1=MCfg->MData.IP_WANIS1;
	*(ULONG*)&IPAddrWANCli2=MCfg->MData.IP_WANIS2;
	tFl_PAN=MCfg->MData.Fl_PAN;
	tFl_WAN=MCfg->MData.Fl_WAN;
	tFl_RunSI=MCfg->MData.Fl_RunSI;
	tFl_RunSICli=MCfg->MData.Fl_RunSICli;

	sprintf(UserName,"%s\\%s",HostName,tUsN);
	i=0;
	MainDomainName[0]=0;
	if (hoststr)
	while (hoststr->h_name[i]!=0)
	{
		if (hoststr->h_name[i++]=='.')
		{
//			strcpy(MainDomainName,&hoststr->h_name[i]);
//			sprintf(MainDomainName,"#%s",&hoststr->h_name[i]);
			i=0;
			while (MainDomainName[i]!=0)
			{
				if (MainDomainName[i++]=='.')
				{
					MainDomainName[i-1]=0;
					break;
				}
			}
			break;
		}
	}
	sprintf(MainDomainName,"#%x",M_Cfg->MData.CustomerID);

	sprintf(WorkgroupName,"");
	int Len=strlen(HostName);
	for (i=0;i<Len;i++)
	{
		char tCh=HostName[i];
		if ((tCh=='.')||(tCh==0))
		{
			break;

		}
		ShortHostName[i]=HostName[i];
	}
	ShortHostName[i]=0;
// 	if (MainDomainName[0]==0)
// 	{
// //		strcpy(MainDomainName,N_Default);
// 		sprintf(MainDomainName,"#%s",N_Default);
// //	GetCurrentUserAndDomain(User,chuser,Domain,chDomain)
// //		char	FullMod[256];
// 		sprintf(FullMod,"%s\\NetName.dll",Path);
// 		ModuleHandle = LoadLibrary(FullMod);
// 		if (ModuleHandle!=NULL)
//  	   {
// //	//		Workgroup
// 			char* (*NetName)();
// 			NetName=(char*(*)()) GetProcAddress(ModuleHandle,"NetName");
// 			strcpy(MainDomainName,NetName());
// 			FreeLibrary(ModuleHandle);
// 		}
// 	}
// 	if (MCfg->MData.Fl_WorkGroup!=0)
// 		strcpy(DomainName,MCfg->MData.WorkGroup);

/*
#ifdef _Neuron

	HRESULT hr;
	
	hr = CoInitialize(NULL);
	
    IADsADSystemInfo *pSys;
    hr = CoCreateInstance(CLSID_ADSystemInfo,
		NULL,
		CLSCTX_INPROC_SERVER,
		IID_IADsADSystemInfo,
		(void**)&pSys);
	char	tttB[80];
	BSTR bstr;
//	hr = pSys->get_UserName(&bstr);
//	if (SUCCEEDED(hr)) {
//		sprintf(tttB,"User: %S", bstr);
//		SysFreeString(bstr);
//	}
	
	hr = pSys->get_ComputerName(&bstr);
	if (SUCCEEDED(hr)) {
		sprintf(WorkgroupName,"%S", bstr);
		SysFreeString(bstr);
	}
	
	hr = pSys->get_DomainDNSName(&bstr);
	if (SUCCEEDED(hr)) {
		sprintf(DomainName,"%S", bstr);
		SysFreeString(bstr);
	}
	
//	hr = pSys->get_PDCRoleOwner(&bstr);
//	if (SUCCEEDED(hr)) {
//		sprintf(tttB,"PDC Role owner: %S", bstr);
//		SysFreeString(bstr);
//	}
	
	if(pSys) {
//		delete pSys;
		pSys->Release();
	}
	
	CoUninitialize();

#endif
*/

//	TimeConst=1000000;
	strcpy(MainPath,Path);
	LoadCfg();
//	FileTime	FTime1;
//	GetSystemTimeAsFileTime(&FTime1.FTime);
	IDICnv=new IDInfoConverter();
// 	TimeConst=GetNClk();
// 	Sleep(1000);	// 1 s.
// 	NClickOld=GetNClk();
// 	Fcpu=double(NClickOld-TimeConst);
// 	Fcpu_1=10000000.0/Fcpu;
// 	//	Fcpu=double(NClickOld-TimeConst)/(((DOUBLE)FTime.Time-(DOUBLE)FTime1.Time)/KtimeS);
// 
// 	GetSystemTimeAsFileTime(&FTime.FTime);
// 	TimeConst=INT64 (Fcpu/1000);
// 	RealTime=FTime.Time;
// 	RndVol+=RealTime;
//	handle = HASP_INVALID_HANDLE_VALUE;
//	status = hasp_login(1, vendor_code, &handle);Fl_NetLock=1;
	/* check if operation was successful */
/*	int rc=-1;
	{
		while (rc==-1)
		{
			Fl_HardLock=1;
			short	LC_Sum,LC_Sum2,LC_Sum3,LC_Sum4,LC_Sum5,LC_Sum6,LC_Sum7;
			status = hasp_read(handle,HASP_FILEID_RW,0,sizeof(LCond),&LCond);	// Main
			if (status != HASP_STATUS_OK)
				break;
			status = hasp_decrypt(handle,&LCond,sizeof(LCond));
			if (status != HASP_STATUS_OK)
				break;
			LC_Sum=LCond.Sum;LCond.Sum=0;LC_Sum2=LCond.Sum2;LCond.Sum2=0;LC_Sum3=LCond.Sum3;LCond.Sum3=0;LC_Sum4=LCond.Sum4;LCond.Sum4=0;LC_Sum5=LCond.Sum5;LCond.Sum5=0;
			LC_Sum6=LCond.Sum6;LCond.Sum6=0;LC_Sum7=LCond.Sum7;LCond.Sum7=0;
			short	Rc=calc_crc16((UCHAR*)&LCond,sizeof(LCond));
			if (LC_Sum!=Rc)
			{
				memset((UCHAR*)&LCond,0,sizeof(LCond));
				break;
			}
			status = hasp_read(handle,HASP_FILEID_RW,LCond.OffsetSysInfo,sizeof(LSinfo),&LSinfo);	// SysInfo
			if (status != HASP_STATUS_OK)
				break;
			status = hasp_decrypt(handle,&LSinfo,sizeof(LSinfo));
			if (status != HASP_STATUS_OK)
				break;
			status = hasp_read(handle,HASP_FILEID_RW,LCond.OffsetUnitInfo,sizeof(LUinfo),&LUinfo);	// SysInfo
			if (status != HASP_STATUS_OK)
				break;
			status = hasp_decrypt(handle,&LUinfo,sizeof(LUinfo));
			if (status != HASP_STATUS_OK)
				break;

			status = hasp_read(handle,HASP_FILEID_RW,LCond.OffsetUserInfo,sizeof(LUSinfo),&LUSinfo);	// SysInfo
			if (status != HASP_STATUS_OK)
				break;
			status = hasp_decrypt(handle,&LUSinfo,sizeof(LUSinfo));
			if (status != HASP_STATUS_OK)
				break;
			status = hasp_read(handle,HASP_FILEID_RW,LCond.OffsetDomainInfo,sizeof(LDinfo),&LDinfo);	// SysInfo
			if (status != HASP_STATUS_OK)
				break;
			status = hasp_decrypt(handle,&LDinfo,sizeof(LDinfo));
			if (status != HASP_STATUS_OK)
				break;
			Rc=calc_crc16((UCHAR*)&LUSinfo,sizeof(LUSinfo));
			if (LC_Sum6!=Rc)
			{
				memset((UCHAR*)&LUSinfo,0,sizeof(LUSinfo));
				break;
			}
			Rc=calc_crc16((UCHAR*)&LDinfo,sizeof(LDinfo));
			if (LC_Sum7!=Rc)
			{
				memset((UCHAR*)&LDinfo,0,sizeof(LDinfo));
				break;
			}

			status = hasp_read(handle,HASP_FILEID_RW,LCond.OffsetStatistic,sizeof(LRStat),&LRStat);	// Stat
			if (status != HASP_STATUS_OK)
				break;
			status = hasp_decrypt(handle,&LRStat,sizeof(LRStat));
			if (status != HASP_STATUS_OK)
				break;
			status = hasp_read(handle,HASP_FILEID_RW,LCond.OffsetPermittedHost,LCond.SizePermittedHost*sizeof(LPHost),&LPHost);	// Stat
			if (status != HASP_STATUS_OK)
				break;
			if (LCond.SizePermittedHost)
			{
				status = hasp_decrypt(handle,&LPHost,LCond.SizePermittedHost*sizeof(LPHost));
				if (status != HASP_STATUS_OK)
				break;
			}
			Rc=calc_crc16((UCHAR*)&LUinfo,sizeof(LUinfo));
			if (LC_Sum5!=Rc)
			{
				memset((UCHAR*)&LUinfo,0,sizeof(LUinfo));
				break;
			}
			Rc=calc_crc16((UCHAR*)&LSinfo,sizeof(LSinfo));
			if (LC_Sum2!=Rc)
			{
				memset((UCHAR*)&LCond,0,sizeof(LCond));
				memset((UCHAR*)&LSinfo,0,sizeof(LSinfo));
				break;
			}
	//		Rc=calc_crc16((UCHAR*)&LRStat,sizeof(LRStat));
	//		if (LC_Sum3!=Rc)
	//		{
	//			memset((UCHAR*)&LCond,0,sizeof(LCond));
	//			memset((UCHAR*)&LRStat,0,sizeof(LRStat)); 
	//		}
			LRStat.TotallRunTime.Time+=(LRStat.LastRunTime.Time-LRStat.StartRunTime.Time);
			GetRealTime(&LRStat.StartRunTime.FTime);
			if (LRStat.TimeExpiry.Time<LRStat.StartRunTime.Time)
			{
				// Time protection
				memset((UCHAR*)&LUinfo,0,sizeof(LUinfo));
				memset((UCHAR*)&LCond,0,sizeof(LCond));
				memset((UCHAR*)&LSinfo,0,sizeof(LSinfo));
				break;
			}
			if ((LRStat.FirstRunTime.Time==0)||(LRStat.StartRunTime.Time-LRStat.FirstRunTime.Time<0))
			{
				GetRealTime(&LRStat.FirstRunTime.FTime);
			}
			if (LRStat.TotallRunTime.Time<0)
				LRStat.TotallRunTime.Time=0;
	//		Rc=calc_crc16((UCHAR*)&LRStat,sizeof(LRStat));
	//		if (LC_Sum4!=Rc)
	//		{
	//			memset((UCHAR*)LPHost,0,50*sizeof(LPHost)); 
	//		}
			LRStat.RunCnt++;
			CopyLic(&LCond);
			if (M_Cfg->MData.CustomerID!=LCond.CustomerID)
			{
				M_Cfg->MData.CustomerID=LCond.CustomerID;
				if (SData->Fl_StateSmartSt==0)
				{
					PROCESSENTRY32 pEntry;
					if(!FindProcess("SmartStarter.exe",&pEntry))
					{
						HWND hwnd=NULL;
						char	FullPath[256];
						sprintf(FullPath,"%s\\SmartStarter.exe", M_Cfg->MData.Path);
						HINSTANCE  rcc=ShellExecute(hwnd, "open",FullPath, NULL, M_Cfg->MData.Path, SW_SHOWNORMAL);
					}
				}
				SData->Fl_StateNeuron=3;
			}
			CopyLicUnit(&LUinfo);
			rc=0;
			CustomerInfo=GetCustomerInfo(M_Cfg->MData.CustomerID);
		}
		if (rc==-1)
		{
			status = hasp_logout(handle);
			Fl_HardLock=0;
			handle=HASP_INVALID_HANDLE_VALUE;
		}
	}
*/
// ×èòàåò ñîíôèãóðàöèþ
// 	LoadCfg();
	if (fabs((double)(Fcpu-Cfg.OldFcpu)/(double)Fcpu)<0.05)
		Fcpu=(double)Cfg.OldFcpu;
	else
		Fcpu=Fcpu*2-Fcpu;
	Fcpu_1=10000000.0/Fcpu;
	FcpuMax=Fcpu*2;
	FcpuMin=Fcpu/2;
//	INT64	iFcpu;
//	QueryPerformanceFrequency((LARGE_INTEGER*)&iFcpu);
//	Fcpu=iFcpu;

	CountMode=0;
	Net_Flag=0;Fl_SI=0;//SysInt=NULL;
// -------------------------------------------------
//	IDI_IS=0;
	SInt=new Integrator(UDPPortLAN,UDPPortPAN,UDPPortWAN);
	SInt->Srv=this;		
	UM=new UnitMng();
// -------------------------------------------------
//	PrmDec	PDec;
	MyNum=SInt->MyLANNum;
//	IDI_IS=new ClassList(this);
//	sprintf(NameUniCfg,"C:\\ClassLst.cfg");
//	IDI_IS->LoadCfg(NameUniCfg);
// --------  Êîððåêöèÿ UID  ------------------------
/*
	ECl=IDI_IS->GetClass(NULL,"SystemSetting");
	if (!ECl)
	{
		ECl=IDI_IS->AddClass(NULL,"SystemSetting");
		IDI_IS->SetLastWrTime(ECl);
	};
	ECl2=IDI_IS->GetMember(ECl,MyNum);
	if (!ECl2)
	{
		ECl2=IDI_IS->AddMember(ECl,MyNum);
		IDI_IS->SetLastWrTime(ECl2);
	}
*/
	char	tmBuf[256];
	sprintf(tmBuf,"%s\\VocAll.dll",Path);
	ModuleVocAll = LoadLibraryA(tmBuf);
	DWORD	Rc= GetLastError();
	
	DLLL=new DLLCntrl(PathDll);

//	El_Tab		*ETbl;
//	IDT_ComSys=OpenTable(this,TabCommSystem);
//	Mt_IDIRepl=Srv->NewDecod("IDI_MReplicator");
//	if (Mt_IDIRepl)
//	{
//		Mt_IDIRepl->AddDecLink(this,0,2006);
//		Mt_IDIRepl->Ext_Mt=0;
//		Mt_IDIRepl->Srv=this;
//		Mt_IDIRepl->Start();
//	}
	if (Cfg.UID<Cfg.PID)
	{
		Cfg.UID=Cfg.PID;
		Cfg.PID+=100;
		SaveCfg();
	}


// -------------------------------------------------
	for (i=0;i<NumSICli;i++)
	{
		CliList[i].Fl=0;
		CliList[i].FlCon=0;
	}
	RSizeSysPack=sizeof(SysPack);
	SysPackMem=(UCHAR*)malloc(RSizeSysPack);
//	NetMen=NULL;
//	NetMenWAN=NULL;
	char	tB[128];
	sprintf(tB,"UID=%I64x ",Cfg.UID);
	OutLog(tB,NULL,"UID");
	Fl_Stop=0;
	strcpy(UInfo.Mode,"Wait");
//	strcpy(UInfo.DomainName,DomainName);
	FlagEnter=0;
	GetRealTime(&TGID.FTime);
	if (M_Cfg->MData.CustomerID)
	{
		NetLCond.CustomerID=M_Cfg->MData.CustomerID;
		CustomerInfo=GetCustomerInfo(M_Cfg->MData.CustomerID);
	}
	strcpy(UInfo.DomainName,MainDomainName);
	InitUnit(this);
	NetThread=new 	ClassThread("SrvNet",SrvNetFun,(LPVOID)this,THREAD_PRIORITY_ABOVE_NORMAL);
	Sys_Thread=new 	ClassThread("SrvSys",SrvSysFun,(LPVOID)this,THREAD_PRIORITY_ABOVE_NORMAL);
//	dtTime1=dtTime2=dtTime3=dtTimeAll=0;
};

int		Services::UserCallBack(int	Handle,int N_Pin,UCHAR* pmas,size_t Cnt)
{
	IDInfo*	tIDInfo;
//	MyLock(__FILE__,__LINE__,&Fl_RE,this);
	if (Handle==Cli1)
	{
		switch (*(int*)pmas)
		{
		case COM_WatchDog:
			break;
		case SYS_IDInfo:
			tIDInfo=IDICnv->Buf2IDInfo(pmas+sizeof(int),Cnt-sizeof(int));
			DataIDI(4,tIDInfo);
	 		IDICnv->ReleaseIDInfo(tIDInfo);
			break;
		case SYS_UDPPack:
			if (Cnt-sizeof(int)==sizeof(UDPPack))
			{
				El_CliInt*	tCli=SInt->AddWANCli((UDPPack*)(pmas+sizeof(int)));
				GetRealTime(&WANCli1FT.FTime);
			}
			break;
		default:
			;
		}
	};
	if (Handle==Cli2)
	{
		switch (*(int*)pmas)
		{
		case COM_WatchDog:
			break;
		case SYS_IDInfo:
			tIDInfo=IDICnv->Buf2IDInfo(pmas+sizeof(int),Cnt-sizeof(int));
			DataIDI(4,tIDInfo);
			IDICnv->ReleaseIDInfo(tIDInfo);
			break;
		case SYS_UDPPack:
			if (Cnt-sizeof(int)==sizeof(UDPPack))
			{
				El_CliInt*	tCli=SInt->AddWANCli((UDPPack*)(pmas+sizeof(int)));
				GetRealTime(&WANCli2FT.FTime);
			}
			break;
		default:
			;
		}
	};
	if(Handle==WANSISrv)
	{
		switch (N_Pin)
		{
		case 0:		// Âèðòóàëüíûé íîìåð êëèåíòà
			ActCli=*(int*)pmas;
			break;
		case 1:		// Ñîîáùåíèÿ î ñîñòîÿíèÿõ ñåðâåðà ñîêåòîâ
			switch(*(int*)(pmas))
			{
			case 0:	// Connect
				if (NumWANCli<(ActCli+1)) NumWANCli=(ActCli+1);
				FlWANCli[ActCli]=TRUE;
				break;
			case 1: // DisConnect	
				FlWANCli[ActCli]=FALSE;
				ActCli=-1;
				break;
			}
		case 2:		// IP - àäðåññ ïðèñîåäèíèâøåéñÿ ìàøèíû êëèåíòà
			if (ActCli)
			{
//					(char*)pmas;
			}
			break;
		case 3:		// Èìÿ ìàøèíû êëèåíòà
			if (ActCli)
			{
//					(char*)pmas;
			}
			break;
		case 4:		// Ïðèíèìàìûé ïàêåò
			if (ActCli!=-1)
			{
				int	i;
				switch (*(int*)pmas)
				{
				case COM_WatchDog:
					//{size_t iasm=1;}
					break;
				case SYS_UDPPack:
					if (Cnt-sizeof(int)==sizeof(UDPPack))
					{
						El_CliInt*	tCli=SInt->AddWANCli((UDPPack*)(pmas+sizeof(int)));
					}
					break;
				case SYS_IDInfo:
					for (i=0;i<NumWANCli;i++)
					{
						if (FlWANCli[i])
						{
//							if (i!=ActCli)
//								SendSock(WANSISrv,i,pmas,Cnt);
//								SIPack(WANSISrv,-1,SYS_IDInfo,pmas,Cnt);
						}
					}
//					DataFF(1004,(uchar*)IDICnv->Buf2IDInfo(pmas+sizeof(int),Cnt-sizeof(int)),sizeof(IDInfo));
					tIDInfo=IDICnv->Buf2IDInfo(pmas+sizeof(int),Cnt-sizeof(int));
					DataIDI(4,tIDInfo);
					IDICnv->ReleaseIDInfo(tIDInfo);
					break;

				default:
					;
				}
			}
			break;
		}
	}
//	MyUnLock(__FILE__,__LINE__,&Fl_RE,this);
	return 0;
};

int		Services::NLSpecialNetIP(IDInfo* IDI,char* pUnitHost,char* pUnitName,INT64 pUnitID)
{
	int			PageIndex=10;
	IPList.CntIP[PageIndex]=0;
	if (pUnitID)
	{
		El_Unit* ElU=UM->FindUnitbyPersID(pUnitID);
		if (ElU)
		{
			strcpy(IPList.IP[PageIndex][IPList.CntIP[PageIndex]++],inet_ntoa(*(in_addr*)&ElU->UInf.IP));
		}
	}
	else
	{	// 
		if ((pUnitHost==0) || (strcmp(pUnitHost,"*")==0) || (pUnitHost[0]==0))
		{
			//if (strcmp(pUnitName,"*")==0)
			if ((pUnitName==0) || (strcmp(pUnitName,"*")==0) || (pUnitName[0]==0))
			{
				PageIndex=0;
			}
			else
			{
				El_Unit* tEl=NULL;
//				MyLock(__FILE__,__LINE__,&UM->Fl_RE,this);
				FindStrList8	FSL8;
				tEl=(El_Unit*)UM->UnitList->Begin(FSL8);
				while (tEl)
				{
					if (strcmp(tEl->UInf.UnitName,pUnitName)==0)
					{
						strcpy(IPList.IP[PageIndex][IPList.CntIP[PageIndex]++],inet_ntoa(*(in_addr*)&tEl->UInf.IP));
					}
					tEl=(El_Unit*)UM->UnitList->Next(FSL8);
				};
//				MyUnLock(__FILE__,__LINE__,&UM->Fl_RE,this);
			}
		}
		else
		{
			char* j=SInt->FindHost(pUnitHost);
			if (j!=NULL)
			{
				strcpy(IPList.IP[PageIndex][IPList.CntIP[PageIndex]++],j);
			}
		}
	};
	return PageIndex;
};

int		Services::NLFind(int IndPage,char* IP)	// NumLine
{
	size_t i;
	size_t j=-1;
	for (i=0;i<IPList.CntIP[IndPage];i++)
	{
		if (strcmp(IPList.IP[IndPage][i],IP)==0)
		{
			j=i;
			break;
		}
	}
//	if (i>=IPList.CntIP[IndPage])
//		i=-1;
	return j;
}

void	Services::NLAdd(int IndPage,char* IP)
{
	if (NLFind(IndPage,IP)==-1)
	{
/*		switch(IndPage)
		{
		case 0:
			if (NetLCond.CntHost<=IPList.CntIP[0])
				return;
			break;
		case 1:
			if (NetLCond.CntHost<=IPList.CntIP[1])
				return;
			break;
		case 2:
			if (NetLCond.CntRole_DomCtr<=IPList.CntIP[2])
				return;
			break;
		case 3:
			if (NetLCond.CntRole_Fsrv<=IPList.CntIP[3])
				return;
			break;
		case 4:
			if (NetLCond.CntRole_SPrc<=IPList.CntIP[4])
				return;
			break;
		case 5:
			if (NetLCond.CntRole_MDBas<=IPList.CntIP[5])
				return;
			break;
		case 6:
			if (NetLCond.CntRole_WSAd<=IPList.CntIP[6])
				return;
			break;
		case 7:
			if (NetLCond.CntRole_WSOp<=IPList.CntIP[7])
				return;
			break;
		case 8:
			if (NetLCond.CntRole_DBas<=IPList.CntIP[8])
				return;
			break;
		case 9:
			if (NetLCond.CntRole_SSSrv<=IPList.CntIP[9])
				return;
			break;
		}
*/
//		if (strcmp(IP,inet_ntoa(IPAddr))!=0)
		{
			char tB[128];
			strcpy(IPList.IP[IndPage][IPList.CntIP[IndPage]++],IP);
			sprintf(tB,"Add Page=%d IP=%s",IndPage,IP);
			OutLog(tB,this,"NetIP");
		}
	}
};

void	Services::NLAddbyUnitName(char* UnitName,char* IP)
{
	int	PageIndex=NLReturnIndex(UnitName);
	if (PageIndex!=-1)
	{
		if (NLFind(PageIndex,IP)==-1)
		{
/*			switch(PageIndex)
			{
			case 0:
				if (NetLCond.CntHost<=IPList.CntIP[0])
					return;
			case 1:
				if (NetLCond.CntHost<=IPList.CntIP[1])
					return;
			case 2:
				if (NetLCond.CntRole_DomCtr<=IPList.CntIP[2])
					return;
			case 3:
				if (NetLCond.CntRole_Fsrv<=IPList.CntIP[3])
					return;
			case 4:
				if (NetLCond.CntRole_SPrc<=IPList.CntIP[4])
					return;
			case 5:
				if (NetLCond.CntRole_MDBas<=IPList.CntIP[5])
					return;
			case 6:
				if (NetLCond.CntRole_WSAd<=IPList.CntIP[6])
					return;
			case 7:
				if (NetLCond.CntRole_WSOp<=IPList.CntIP[7])
					return;
			case 8:
				if (NetLCond.CntRole_DBas<=IPList.CntIP[8])
					return;
			case 9:
				if (NetLCond.CntRole_SSSrv<=IPList.CntIP[9])
					return;
				break;
			}
*/
//			if (strcmp(IP,inet_ntoa(IPAddr))!=0)
			{
				char tB[128];
				strcpy(IPList.IP[PageIndex][IPList.CntIP[PageIndex]++],IP);
				sprintf(tB,"AddByUnit=%s Page=%d IP=%s",UnitName,PageIndex,IP);
				OutLog(tB,this,"NetIP");
			}
		}
	}
};

int	Services::NLReturnIndex(char* UnitName)
{
	int	PageIndex=-1;
	if (strcmp(UnitName,N_Service)==0) PageIndex=1;
	else
	if (strcmp(UnitName,"UnitDomainCtrl")==0) PageIndex=2;
	else
	if (strcmp(UnitName,"IDI_FSrv")==0) PageIndex=3;
	else
	if (strcmp(UnitName,"IDI_Protocol")==0) PageIndex=4;
	else
	if (strcmp(UnitName,"UnitMainDB")==0) PageIndex=5;
	else
	if (strcmp(UnitName,N_WSACfg)==0) PageIndex=6;
	else
	if (strcmp(UnitName,"OperWS")==0) PageIndex=7;
	else
	if (strcmp(UnitName,"BDMan")==0) PageIndex=8;
	else
	if (strcmp(UnitName,"TlfSrv")==0) PageIndex=9;
	return PageIndex;
}

void	Services::NLDelbyUnitName(char* UnitName,char* IP)
{
	int rc;
	int	PageIndex=NLReturnIndex(UnitName);
	
	if (PageIndex!=-1)
	{
		if ((rc=NLFind(PageIndex,IP))!=-1)
		{
			char tB[128];
			sprintf(tB,"DelByUnit=%s Page=%d IP=%s",UnitName,PageIndex,IP);
			OutLog(tB,this,"NetIP");
			if (IPList.CntIP[PageIndex]>(rc+1))
				strcpy((char*)IPList.IP[PageIndex][rc],(char*)IPList.IP[PageIndex][IPList.CntIP[PageIndex]-1]);
//				memcpy((char*)&IPList.IP[PageIndex][rc],(char*)&IPList.IP[PageIndex][rc+1],16*(IPList.CntIP[PageIndex]-(rc+1)));
			IPList.CntIP[PageIndex]--;
		}
	}
};


void	Services::NLDel(int IndPage,char* IP)
{
	int rc;
	if ((rc=NLFind(IndPage,IP))!=-1)
	{
		char tB[128];
		sprintf(tB,"Del Page=%d IP=%s",IndPage,IP);
		OutLog(tB,this,"NetIP");
		if (IPList.CntIP[IndPage]-rc)
			strcpy((char*)IPList.IP[IndPage][rc],(char*)IPList.IP[IndPage][IPList.CntIP[IndPage]-1]);			
//			memcpy((char*)&IPList.IP[IndPage][rc],(char*)&IPList.IP[IndPage][rc+1],16*(IPList.CntIP[IndPage]-rc));
		IPList.CntIP[IndPage]--;
	}
};

int		Services::CalcNetIP(IDInfo* IDI)	// return active page of IP list
{
	int			PageIndex=-1;
// 	INT64		pUnitID=0;
// 	void*		pVol;
// 	size_t		pCnt;
	Decoder*	ParentMt=NULL;
	AddressInfo	*AdrI=IDI->AdrInfo;
	if (AdrI==NULL)
		return PageIndex;
	MyLock(__FILE__,__LINE__,&Fl_RE7,this);

	if (AdrI->DestinAI.UnitID==0)
	{
		if ((AdrI->DestinAI.HostName[0]==0)&&(AdrI->DestinAI.UnitName[0]==0))
		{
			char	ttB[128];
			sprintf(ttB,"Not Adress information Mt=%s",IDI->ParentMtName);
//			sprintf(ttB,"Not Adress information Mt=%s",IDI->ParentMt->Name_Mt);
			SetCriticalError(NULL,ttB,"Warning","CalcNetIP");
		}
	}
// Àíàëèç àäðåñà
	if ((AdrI->DestinAI.HostName[0]==0)||(AdrI->DestinAI.HostName[0]=='*'))
	{
		if (AdrI->DestinAI.UnitName[0]==0)
		{
			if (AdrI->DestinAI.UnitID==0)
				PageIndex=0;	// All active Host
			else
				PageIndex=NLSpecialNetIP(IDI,AdrI->DestinAI.HostName,AdrI->DestinAI.UnitName,AdrI->DestinAI.UnitID);
		}
		else
		{
			PageIndex=NLReturnIndex(AdrI->DestinAI.UnitName);
			if (PageIndex==-1)
				PageIndex=NLSpecialNetIP(IDI,AdrI->DestinAI.HostName,AdrI->DestinAI.UnitName,AdrI->DestinAI.UnitID);
		}
	}
	else
	{
		PageIndex=NLSpecialNetIP(IDI,AdrI->DestinAI.HostName,AdrI->DestinAI.UnitName,AdrI->DestinAI.UnitID);
	}
	if ((AdrI->DestinAI.UnitMode[0])||(AdrI->DestinAI.DomainName[0]))
	{
		int TPI=PageIndex;
		if (TPI<10)
		{
			El_Unit* tEl=NULL;
			PageIndex=10;
			IPList.CntIP[PageIndex]=0;
			
			for (size_t i=0;i<IPList.CntIP[TPI];i++)
			{
//				MyLock(__FILE__,__LINE__,&UM->Fl_RE,this);
				FindStrList8	FSL8;
				tEl=(El_Unit*)UM->UnitList->Begin(FSL8);
				while (tEl)
				{
					if (strcmp(inet_ntoa(*(in_addr*)&tEl->UInf.IP),IPList.IP[TPI][i])==0)
					{
						if (AdrI->DestinAI.UnitName[0])
						{
							if (strcmp(tEl->UInf.UnitName,AdrI->DestinAI.UnitName)==0)
							{
								if ((AdrI->DestinAI.UnitMode[0])&&(strcmp(tEl->UInf.Mode,AdrI->DestinAI.UnitMode)==0))
								{
									if (AdrI->DestinAI.DomainName[0])
									{
										if ((strcmp(tEl->UInf.DomainName,AdrI->DestinAI.DomainName)==0)||(strcmp(AdrI->DestinAI.DomainName,"*")==0)|| (AdrI->DestinAI.DomainName[0]==0))
										{
											strcpy(IPList.IP[PageIndex][IPList.CntIP[PageIndex]++],IPList.IP[TPI][i]);
											break;
										}
									}
									else
									{
										strcpy(IPList.IP[PageIndex][IPList.CntIP[PageIndex]++],IPList.IP[TPI][i]);
										break;
									}
									
								}
								else
								{
									if ((AdrI->DestinAI.DomainName[0])&&(strcmp(tEl->UInf.DomainName,AdrI->DestinAI.DomainName)==0))
									{
										strcpy(IPList.IP[PageIndex][IPList.CntIP[PageIndex]++],IPList.IP[TPI][i]);
										break;
									}
								}
							}

						}
						else
						{
							if ((AdrI->DestinAI.DomainName[0])&&(strcmp(tEl->UInf.DomainName,AdrI->DestinAI.DomainName)==0))
							{
								strcpy(IPList.IP[PageIndex][IPList.CntIP[PageIndex]++],IPList.IP[TPI][i]);
								break;
							}
						}
					}
//					UM->UnitList->Find((UCHAR*)&tEl->UInf.PersID,sizeof(INT64),1);
					tEl=(El_Unit*)UM->UnitList->Next(FSL8);
				}
//				MyUnLock(__FILE__,__LINE__,&UM->Fl_RE,this);
			}
		}
	}
	MyUnLock(__FILE__,__LINE__,&Fl_RE7,this);
	return PageIndex;
};	// return active page of IP list

int		Services::CopyIDInfo(IDInfo* IDISrs,IDInfo* IDIDis)
{
	int rc=-1;
	INT64		IDName;
	void*		pVol;
	size_t			pCnt;
	Decoder*	ParentMt=NULL;
	MyLock(__FILE__,__LINE__,&Fl_RE6,this);
	memcpy((UCHAR*)IDIDis+sizeof(ID_Elem),(UCHAR*)IDISrs+sizeof(ID_Elem),sizeof(IDIStat)-sizeof(ID_Elem));
	IDIDis->pPList->DelAllElem();
	if(IDISrs->pPList->FindFirst(IDName,pVol,pCnt,ParentMt)!=-1)
	{
		do
		{
			IDIDis->pPList->SetParam(IDName,pVol,pCnt,ParentMt);
		}
		while(IDISrs->pPList->FindNext(IDName,pVol,pCnt,ParentMt)!=-1);
	};
	if (IDIDis->SeansFrom)
	{
		IDIDis->ID_SeansFrom=IDIDis->SeansFrom->ID_File;IDIDis->SeansFrom=NULL;
	}
	if (IDIDis->SeansTo)
	{
		IDIDis->ID_SeansTo=IDIDis->SeansTo->ID_File;IDIDis->SeansTo=NULL;
	}
	if (IDIDis->ParentFile)
	{
		IDIDis->ID_ParentFile=IDIDis->ParentFile->ID_File;IDIDis->ParentFile=NULL;
	}
	IDIDis->Buf=NULL;
	IDIDis->Cnt=0;
	IDIDis->OldBuf=NULL;
	IDIDis->OldCnt=0;
//	IDIDis->Buf=IDISrs->Buf;
//	IDIDis->Cnt=IDISrs->Cnt;
//	IDIDis->OldBuf=IDISrs->OldBuf;
//	IDIDis->OldCnt=IDISrs->OldCnt;
	MyUnLock(__FILE__,__LINE__,&Fl_RE6,this);

	return rc;
};

void	Services::CopyLicUnit(LC_UnitInfo* LU)
{
//	MyLock(__FILE__,__LINE__,&Fl_RE,this);
	memcpy(&NetLUinfo,LU,sizeof(LC_UnitInfo));
/*
	if (LU->PrcSrv_St_ECC>NetLUinfo.PrcSrv_St_ECC)
		NetLUinfo.PrcSrv_St_ECC=LU->PrcSrv_St_ECC;
	if (LU->PrcSrv_E1>NetLUinfo.PrcSrv_E1)
		NetLUinfo.PrcSrv_E1=LU->PrcSrv_E1;
	if (LU->PrcSrv_IP>NetLUinfo.PrcSrv_IP)
		NetLUinfo.PrcSrv_IP=LU->PrcSrv_IP;
	if (LU->PrcSrv_VoIP>NetLUinfo.PrcSrv_VoIP)
		NetLUinfo.PrcSrv_VoIP=LU->PrcSrv_VoIP;
	if (LU->PrcSrv_Abis>NetLUinfo.PrcSrv_Abis)
		NetLUinfo.PrcSrv_Abis=LU->PrcSrv_Abis;
	if (LU->PrcSrv_VSAT_H>NetLUinfo.PrcSrv_VSAT_H)
		NetLUinfo.PrcSrv_VSAT_H=LU->PrcSrv_VSAT_H;
	if (LU->PrcSrv_VSAT_G>NetLUinfo.PrcSrv_VSAT_G)
		NetLUinfo.PrcSrv_VSAT_G=LU->PrcSrv_VSAT_G;
	if (LU->PrcSrv_DVB>NetLUinfo.PrcSrv_DVB)
		NetLUinfo.PrcSrv_DVB=LU->PrcSrv_DVB;
	if (LU->PrcSrv_ExtTC>NetLUinfo.PrcSrv_ExtTC)
		NetLUinfo.PrcSrv_ExtTC=LU->PrcSrv_ExtTC;
	if (LU->PrcSrv_V_F>NetLUinfo.PrcSrv_V_F)
		NetLUinfo.PrcSrv_V_F=LU->PrcSrv_V_F;
	if (LU->PrcSrv_SS5>NetLUinfo.PrcSrv_SS5)
		NetLUinfo.PrcSrv_SS5=LU->PrcSrv_SS5;
	if (LU->PrcSrv_1VF>NetLUinfo.PrcSrv_1VF)
		NetLUinfo.PrcSrv_1VF=LU->PrcSrv_1VF;
	if (LU->PrcSrv_R2>NetLUinfo.PrcSrv_R2)
		NetLUinfo.PrcSrv_R2=LU->PrcSrv_R2;
	if (LU->PrcSrv_DCME_Cl>NetLUinfo.PrcSrv_DCME_Cl)
		NetLUinfo.PrcSrv_DCME_Cl=LU->PrcSrv_DCME_Cl;
	if (LU->PrcSrv_DCME_Prc>NetLUinfo.PrcSrv_DCME_Prc)
		NetLUinfo.PrcSrv_DCME_Prc=LU->PrcSrv_DCME_Prc;
	if (LU->PrcSrv_Alcatel3612>NetLUinfo.PrcSrv_Alcatel3612)
		NetLUinfo.PrcSrv_Alcatel3612=LU->PrcSrv_Alcatel3612;
	if (LU->SSSrv_SS7>NetLUinfo.SSSrv_SS7)
		NetLUinfo.SSSrv_SS7=LU->SSSrv_SS7;
	if (LU->SSSrv_SS7_CIC_Map>NetLUinfo.SSSrv_SS7_CIC_Map)
		NetLUinfo.SSSrv_SS7_CIC_Map=LU->SSSrv_SS7_CIC_Map;
	if (LU->SSSrv_A>NetLUinfo.SSSrv_A)
		NetLUinfo.SSSrv_A=LU->SSSrv_A;
	if (LU->SSSrv_Ater>NetLUinfo.SSSrv_Ater)
		NetLUinfo.SSSrv_Ater=LU->SSSrv_Ater;
	if (LU->SSSrv_Abis>NetLUinfo.SSSrv_Abis)
		NetLUinfo.SSSrv_Abis=LU->SSSrv_Abis;
	if (LU->SSSrv_V51>NetLUinfo.SSSrv_V51)
		NetLUinfo.SSSrv_V51=LU->SSSrv_V51;
	if (LU->SSSrv_EDSS>NetLUinfo.SSSrv_EDSS)
		NetLUinfo.SSSrv_EDSS=LU->SSSrv_EDSS;
	if (LU->Prog_Feature>NetLUinfo.Prog_Feature)
		NetLUinfo.Prog_Feature=LU->Prog_Feature;
	if (LU->Prog_WSAAutoStart>NetLUinfo.Prog_WSAAutoStart)
		NetLUinfo.Prog_WSAAutoStart=LU->Prog_WSAAutoStart;
	if (LU->PrcSrv_TiburonSpeed>NetLUinfo.PrcSrv_TiburonSpeed)
		NetLUinfo.PrcSrv_TiburonSpeed=LU->PrcSrv_TiburonSpeed;
	if (LU->PrcSrv_DTX240>NetLUinfo.PrcSrv_DTX240)
		NetLUinfo.PrcSrv_DTX240=LU->PrcSrv_DTX240;
	if (LU->PrcSrv_DTX360>NetLUinfo.PrcSrv_DTX360)
		NetLUinfo.PrcSrv_DTX360=LU->PrcSrv_DTX360;
	if (LU->PrcSrv_DTX600>NetLUinfo.PrcSrv_DTX600)
		NetLUinfo.PrcSrv_DTX600=LU->PrcSrv_DTX600;
	if (LU->PrcSrv_DX3000>NetLUinfo.PrcSrv_DX3000)
		NetLUinfo.PrcSrv_DX3000=LU->PrcSrv_DX3000;
	if (LU->PrcSrv_DX7000>NetLUinfo.PrcSrv_DX7000)
		NetLUinfo.PrcSrv_DX7000=LU->PrcSrv_DX7000;
	if (LU->PrcSrv_NCM501>NetLUinfo.PrcSrv_NCM501)
		NetLUinfo.PrcSrv_NCM501=LU->PrcSrv_NCM501;
	if (LU->PrcSrv_TC2000>NetLUinfo.PrcSrv_TC2000)
		NetLUinfo.PrcSrv_TC2000=LU->PrcSrv_TC2000;
	if (LU->PrcSrv_Celtic3G>NetLUinfo.PrcSrv_Celtic3G)
		NetLUinfo.PrcSrv_Celtic3G=LU->PrcSrv_Celtic3G;
	if (LU->PrcSrv_PCME>NetLUinfo.PrcSrv_PCME)
		NetLUinfo.PrcSrv_PCME=LU->PrcSrv_PCME;
	if (LU->PrcSrv_iDirect>NetLUinfo.PrcSrv_iDirect)
		NetLUinfo.PrcSrv_iDirect=LU->PrcSrv_iDirect;
	if (LU->PrcSrv_MUXNewbridge>NetLUinfo.PrcSrv_MUXNewbridge)
		NetLUinfo.PrcSrv_MUXNewbridge=LU->PrcSrv_MUXNewbridge;
	if (LU->PrcSrv_MUXV10>NetLUinfo.PrcSrv_MUXV10)
		NetLUinfo.PrcSrv_MUXV10=LU->PrcSrv_MUXV10;
	if (LU->PrcSrv_MUXV16>NetLUinfo.PrcSrv_MUXV16)
		NetLUinfo.PrcSrv_MUXV16=LU->PrcSrv_MUXV16;
	if (LU->PrcSrv_MUXT1876>NetLUinfo.PrcSrv_MUXT1876)
		NetLUinfo.PrcSrv_MUXT1876=LU->PrcSrv_MUXT1876;
*/
//	MyUnLock(__FILE__,__LINE__,&Fl_RE,this);
};

void	Services::CopyLic(LicenseCond* pLCond)
{
//	MyLock(__FILE__,__LINE__,&Fl_RE,this);
	if (pLCond->CntHost>NetLCond.CntHost)
		NetLCond.CntHost=pLCond->CntHost;
	if (pLCond->CntRole_DBas>NetLCond.CntRole_DBas)
		NetLCond.CntRole_DBas=pLCond->CntRole_DBas;
	if (pLCond->CntRole_DomCtr>NetLCond.CntRole_DomCtr)
		NetLCond.CntRole_DomCtr=pLCond->CntRole_DomCtr;
	if (pLCond->CntRole_Fsrv>NetLCond.CntRole_Fsrv)
		NetLCond.CntRole_Fsrv=pLCond->CntRole_Fsrv;
	if (pLCond->CntRole_MDBas>NetLCond.CntRole_MDBas)
		NetLCond.CntRole_MDBas=pLCond->CntRole_MDBas;
	if (pLCond->CntRole_SPrc>NetLCond.CntRole_SPrc)
		NetLCond.CntRole_SPrc=pLCond->CntRole_SPrc;
	if (pLCond->CntRole_SSSrv>NetLCond.CntRole_SSSrv)
		NetLCond.CntRole_SSSrv=pLCond->CntRole_SSSrv;
	if (pLCond->CntRole_WSAd>NetLCond.CntRole_WSAd)
		NetLCond.CntRole_WSAd=pLCond->CntRole_WSAd;
	if (pLCond->CntRole_WSOp>NetLCond.CntRole_WSOp)
		NetLCond.CntRole_WSOp=pLCond->CntRole_WSOp;

	if (pLCond->NumCh_Fsrv>NetLCond.NumCh_Fsrv)
		NetLCond.NumCh_Fsrv=pLCond->NumCh_Fsrv;
	if (pLCond->NumCh_DBas>NetLCond.NumCh_DBas)
		NetLCond.NumCh_DBas=pLCond->NumCh_DBas;
	if (pLCond->NumCh_SPrc>NetLCond.NumCh_SPrc)
		NetLCond.NumCh_SPrc=pLCond->NumCh_SPrc;

 	if (pLCond->CntRole_PPSrv>NetLCond.CntRole_PPSrv)
 		NetLCond.CntRole_PPSrv=pLCond->CntRole_PPSrv;
	if (pLCond->CntRole_WSAnalitic>NetLCond.CntRole_WSAnalitic)
		NetLCond.CntRole_WSAnalitic=pLCond->CntRole_WSAnalitic;
	if (pLCond->CntRole_WSTelemetry>NetLCond.CntRole_WSTelemetry)
		NetLCond.CntRole_WSTelemetry=pLCond->CntRole_WSTelemetry;

	if (pLCond->SizePermittedHost>NetLCond.SizePermittedHost)
		NetLCond.SizePermittedHost=pLCond->SizePermittedHost;
	if (M_Cfg->MData.CustomerID==0)
	{
		M_Cfg->MData.CustomerID=pLCond->CustomerID;
		if ((SData->Fl_StateSmartSt==0)&&(M_Cfg->MData.Fl_RunSmSt))

		{
			PROCESSENTRY32 pEntry;
			if(!FindProcess(_T("SmartStarter.exe"),&pEntry))
			{
				HWND hwnd=NULL;
				char	FullPath[256];
				sprintf(FullPath,"%s\\SmartStarter.exe", M_Cfg->MData.Path);
				HINSTANCE  rcc=ShellExecuteA(hwnd, "open",FullPath, NULL, M_Cfg->MData.Path, SW_SHOWNORMAL);
			}
		}
		SData->Fl_StateApp=3;
	}
	if (NetLCond.CustomerID==0)
	{
		NetLCond.CustomerID=M_Cfg->MData.CustomerID;
		CustomerInfo=GetCustomerInfo(M_Cfg->MData.CustomerID);
	}
//	MyUnLock(__FILE__,__LINE__,&Fl_RE,this);
}

int Services::UserCallBackIDI(int Handle,IDInfo* tIDI,StrOwner* tOwn)
{
//	char	tBf[256];
	int rc=0;
	switch (Handle)
	{
	case 0:
		DataIDI(4,tIDI);
		break;
	case 1:	// ID Table Prm/Main
		switch(tIDI->ID_SeansFrom)
		{
		case TabParametr:
			break;
		case TabNeuronGlobal:
			break;
		case TabNeuronLocalCustomer:
			break;
		case TabNeuronLocalSite:
			break;
		}
		break;
	case 2:	// ????? 
		break;
	}
	return rc;
}
/*
int	Method::UserCallBackIDI(int Handle, IDInfo* tIDI,StrOwner* tOwn)
{
	int rc=0;
	switch (Handle)
	{
	case 0:	// NetCom
		break;
	case 1:	// ID Table Prm/Main
		switch(tIDI->ID_SeansFrom)
		{
		case TabParametr:
			break;
		case TabNeuron:
			break;
		}
		break;
	case 2:	// ????? 
		break;
	}
	return rc;
};
*/


int	Services::DataIDI(			// Äàííûå äëÿ äåêîäèðîâàíèÿ
	int N_Pin,		// Íîìåð âõîäíîé íîæêè
	IDInfo* pIDI)		// Äëèíà äàííûõ
{
	int rc=0;
// 	char*		pMyUnitHost=NULL;
// 	char*		pSourceAI.UnitName=NULL;
// 	char*		pUnitHost=NULL;
// 	char*		pUnitName=NULL;
// 	INT64		pUnitID=0;
// 	char*		pUnitMode=NULL;
// 	char*		pDomainName=NULL;
// 	char*		pStrategic=NULL;
// 
// 	char		UnitMode[80];
	void*		pVol;
	size_t		pCnt;
	El_Unit*	tEl;
	Decoder*	ParentMt=NULL;
//	UnitMode[0]=0;
//	char*		NetType=NULL;
//	int			Fl_Local;
	if (Fl_NetLock==0)
		return rc;
	if (N_Pin==0)
		N_Pin=4;
	switch(N_Pin)
	{
	case 0:
		rc=ThIDView->DataIDIEx(0,pIDI);
		break;
	case 1:
		rc=FSrv->DataIDIEx(0,pIDI);
		break;
	case 2:
		rc=1;//BDBIDInfo->DataIDIEx(0,pIDI);
		break;
	case 3:
		if (pIDI->AdrInfo==NULL)
		{
			return rc;
		}
//		pIDI->pPList->SetParam(IDMyHostName,ShortHostName,1+strlen(ShortHostName),this);
		strcpy(pIDI->AdrInfo->SourceAI.HostName,ShortHostName);
// 		if (pIDI->pPList->GetParam(IDNetType,pVol,pCnt,ParentMt)!=-1)
// 		{
// 			NetType=(char*)pVol;
// 		}
// 		if (pIDI->pPList->GetParam(IDHostName,pVol,pCnt,ParentMt)!=-1)
// 		{
// 			pUnitHost=(char*)pVol;
// 		}
// 		if (pIDI->pPList->GetParam(IDDomainName,pVol,pCnt,ParentMt)!=-1)
// 		{
// 			pDomainName=(char*)pVol;
// 		}
// 		if (pIDI->pPList->GetParam(IDUnitName,pVol,pCnt,ParentMt)!=-1)
// 		{
// 			pUnitName=(char*)pVol;
// 		}
// 		if (pIDI->pPList->GetParam(IDUnitID,pVol,pCnt,ParentMt)!=-1)
// 		{
// 			pUnitID=*(INT64*)pVol;
// 		}
// 		if (pIDI->pPList->GetParam(IDUnitMode,pVol,pCnt,ParentMt)!=-1)
// 		{
// 			pUnitMode=(char*)pVol;
// 			if (pUnitMode[0]!='*')
// 			{
// 				strcpy(UnitMode,pUnitMode);
// 			}
// 			//					tIDI->pPList->DelParam(IDUnitMode);
// 		}
//		Fl_Local=0;
		if (pIDI->AdrInfo->DestinAI.UnitID)
		{
			tEl=(El_Unit*)UM->UnitList->Find((UCHAR*)&pIDI->AdrInfo->DestinAI.UnitID,sizeof(INT64));
			if (tEl)
			{
				if (strcmp(tEl->UInf.Host,Srv->ShortHostName)==0)
				{
					tEl->UInf.pObj->UserCallBackIDIEx(0,pIDI,NULL,1);
				}
				else
				{
					if((pIDI->AdrInfo->NetType==NT_ANY)||(pIDI->AdrInfo->NetType==NT_LAN))
					{
						rc=NetCommLAN->DataIDIEx(0,pIDI);
					}
				}
			}
			else
			{
				if((pIDI->AdrInfo->NetType==NT_ANY)||(pIDI->AdrInfo->NetType==NT_LAN))
				{
					rc=NetCommLAN->DataIDIEx(0,pIDI);
				}
			}
		}
		else
		{
//			if ((pIDI->AdrInfo->DestinAI.HostName[0]==0)||(strcmp(pIDI->AdrInfo->DestinAI.HostName,ShortHostName)==0))
			if (strcmp(pIDI->AdrInfo->DestinAI.HostName,ShortHostName)==0)
			{
				SendToLocal(pIDI);
			}
			else
			{
				if((pIDI->AdrInfo->NetType==NT_ANY)||(pIDI->AdrInfo->NetType==NT_LAN))
				{
					if (NetCommLAN)
					{
						rc=NetCommLAN->DataIDIEx(0,pIDI);
					}
				}
			};
		};
		// ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
		if((pIDI->AdrInfo->NetType==NT_ANY)||(pIDI->AdrInfo->NetType==NT_WAN))
		{
			uchar*	tBf=NULL;
			size_t	tCn;
			if((Cli1)||(Cli2)||(WANSISrv))
			{
//				pIDI->pPList->SetParam(IDHostName,"*",2,this);
				tBf=IDICnv->IDInfo2Buf(pIDI,tCn);
			}
			if((Cli1)&&(tBf))
			{
				SIPack(Cli1,SYS_IDInfo,tBf,tCn);
			}
			if((Cli2)&&(tBf))
			{
				SIPack(Cli2,SYS_IDInfo,tBf,tCn);
			}
			if((WANSISrv)&&(tBf))
			{
				SIPack(WANSISrv,SYS_IDInfo,tBf,tCn);
			}
		}
		// ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
		break;
	case 4:
//		if (Cnt==sizeof(IDInfo))
		{
			char		tBf[256];
			char		*pDllName=NULL;
			UnitInfo*	pUInfo;

			HANDLE hToken;
			TOKEN_PRIVILEGES tkp;

/*
			char		*pMyUnitHost;
			void*		pVol;
			Decoder*	ParentMt=NULL;

//			LicenseCond*	pLCond;
			El_Unit*	tEl;
			int			pCnt;
//			void*		IPAdr;
			void*		tID;
//			IDInfo*	tIDI=(IDInfo*)pmas;
*/
			switch (pIDI->TypeIDI)
			{
			case TypeIDI_COMMAND:
				switch (pIDI->Command)
				{
				case	Command_C_WANCli:
					{
/*						Srv->GetRealTime(&SInt->laWANSvrTime.FTime);
						UDPCli	*tCli=(UDPCli*)tIDI->Buf;
						UDPPack	tPack;
						ULONG	uniID=inet_addr(tCli->IPAddr);
						if (*(ULONG*)&Srv->IPAddrWAN!=uniID)
						{
							tPack.Command=tCli->Command;
							tPack.CPUUsage=tCli->CPUUsage;
							strcpy(tPack.MainDomainName,tCli->MainDomainName);
							tPack.FTime=tCli->crTime;
							strcpy(tPack.HostName,tCli->ShortHost);
							tPack.MemUsage=tCli->MemUsage;
							tPack.Rez=0;
							tPack.Ver=tCli->Ver;
							//UDPCli
							El_CliInt*	WANCli=SInt->AddWANCli(&tPack);
						}
						else
						{
							switch(tCli->Command)
							{
							case SICom_ServerWait:
							case SICom_ServerActive:
								if (SInt->NumLANSrv==SInt->MyLANNum)
								{
									if (tIDI->pPList->GetParam(IDID,tID,pCnt)!=-1)
									{
										FileTime	ttTime;
										MedFltBuf[IndexMF++]=(double)(*(INT64*)tID);
										if (IndexMF>=SizeMFBuf)
										{
											IndexMF=0;
										}
										Srv->SmTime=MedFlt(MedFltBuf,SizeMFBuf);
										if (Srv->SmTime>11*KtimeS) Srv->SmTime=11*KtimeS;
										if (Srv->SmTime<-11*KtimeS) Srv->SmTime=-11*KtimeS;
										Srv->Telemetr(NULL,"_SmTime",Srv->SmTime/10000.);
										Srv->GetRealTime(&ttTime.FTime);
										ttTime.Time-=(INT64)Srv->SmTime;
										Srv->ExtSynTime(ttTime);
									}
								}
								Srv->SmTime=0;
								break;
							};

						}
*/				
					}
					break;
				case Command_C_GetMainCfg:
//					if (pIDI->pPList->GetParam(IDMyHostName,pVol,pCnt,ParentMt)!=-1)
					if (pIDI->AdrInfo)
					{
						if (pIDI->AdrInfo->NativeAI.HostName[0])
						{
							IDInfo*	IDI=GetNewIDI(this);
							AddressInfo	AdrI;
							InitCommIDI(IDI);
							IDI->AdrInfo=&AdrI;
//							IDI->pPList->SetParam(IDHostName,pVol,pCnt,this);
							strcmp(AdrI.DestinAI.HostName,pIDI->AdrInfo->NativeAI.HostName);
//							if (pIDI->pPList->GetParam(IDUnitID,pVol,pCnt,ParentMt)!=-1)
							if (pIDI->AdrInfo->DestinAI.UnitID)
							{
//								pUnitID=*(INT64*)pVol;
//								IDI->pPList->SetParam(IDUnitID,pVol,sizeof(INT64),this);
								AdrI.DestinAI.UnitID=pIDI->AdrInfo->DestinAI.UnitID;
							}
//							if (pIDI->pPList->GetParam(IDUnitName,pVol,pCnt,ParentMt)!=-1)
							if (pIDI->AdrInfo->DestinAI.UnitName[0])
							{
//								IDI->pPList->SetParam(IDUnitName,pVol,pCnt,this);
								strcpy(AdrI.DestinAI.UnitName,pIDI->AdrInfo->DestinAI.UnitName);
							}
							IDI->Command=Command_C_MainCfg;
							IDI->Buf=(UCHAR*)&M_Cfg->MData;
							IDI->Cnt=sizeof(MainData);
							IDI->NumPack++;
							SendIDI(IDI,this);
							DeleteIDI(IDI);	
						}
					}
					break;
				case Command_C_SendMainCfg:
					if (pIDI->Cnt==sizeof(MainData))
					{
						memcpy((UCHAR*)&M_Cfg->MData,pIDI->Buf,pIDI->Cnt);
						Srv->SaveCfg();
					};
					break;
				case Command_C_GetIDI:
				case Command_C_GetIDIFromSrv:
//					MyUnLock(&Fl_RE6,this);
					Pack2Table(pIDI);
//					MyLock(&Fl_RE6,this);
					break;
				case Command_C_GetQBE:
					break;
				case Command_C_GetFPrn:
					break;
				case Command_C_SendUnitInfo:
					if (pIDI->Cnt==sizeof(UnitInfo))
					{
						char tB[128];
						pUInfo=(UnitInfo*)pIDI->Buf;
						// 2008.09.25
						if(mStrCmpHost(ShortHostName,pUInfo->Host)!=0)
						{							
							pUInfo->pObj=NULL;
							if (UM)
							{
								if (pUInfo->PersID!=0)
								{
									UM->AddUnit(pUInfo);
									sprintf(tB,"SendUnit PID=%I64x Name=%s",pUInfo->PersID,pUInfo->UnitName);
									OutLog(tB,this,"Unit");
								}
								else
								{
									sprintf(tB,"SendUnit Error PID=%I64x Nost=%s",pUInfo->PersID,pUInfo->Host);
									OutLog(tB,this,"Unit");
								}
							}
						}
						else
						{
							sprintf(tB,"Error add unit own host PID=%I64x Name=%s",pUInfo->PersID,pUInfo->UnitName);
							OutLog(tB,this,"Unit");
						}
					}
					break;
				case Command_C_DelUnitInfo:
					if (pIDI->Cnt==sizeof(UnitInfo))
					{
						pUInfo=(UnitInfo*)pIDI->Buf;
						if (UM)
							UM->DelUnit(pUInfo);
					}
					break;
				case Command_C_GetUnitInfo:
//					MyLock(&UM->Fl_RE,this);
					{

					tEl=NULL;
//					MyLock(__FILE__,__LINE__,&UM->Fl_RE,this);
					FindStrList8	FSL8;
					if (UM)
						tEl=(El_Unit*)UM->UnitList->Begin(FSL8);
#ifdef DEBUG
					char ccStr[512];
					sprintf(ccStr,"%s",pIDI->AdrInfo->SourceAI.HostName);
					OutLog(ccStr,this,"GetUI");
#endif
					while (tEl)
					{
						if (strcmp(ShortHostName,tEl->UInf.Host)==0)
						{
// 							if (PoiObj->UInfo.PersID==0)
// 							{
// 								InitUnit(PoiObj);
// 							}
// 							else
							{
								Srv->GetRealTime(&tEl->UInf.ModTime.FTime);
								IDInfo*	IDI=GetNewIDI(this);
								AddressInfo	AdrI;
								IDI->AdrInfo=&AdrI;
								InitCommIDI(IDI);
//								IDI->pPList->SetParam(IDHostName,"*",2,this);
//								IDI->pPList->SetParam(IDUnitName,N_Service,9,this);
								strcpy(AdrI.DestinAI.UnitName,N_Service);
								//////////////////////////////////////////////////////////////////////////
								//  [2/19/2012 San]
//								AdrI.DestinAI.UnitID=pIDI->AdrInfo->SourceAI.UnitID;
								strcpy(AdrI.DestinAI.HostName,pIDI->AdrInfo->SourceAI.HostName);
								strcpy(AdrI.DestinAI.DomainName,pIDI->AdrInfo->SourceAI.DomainName);
	
								//////////////////////////////////////////////////////////////////////////

								IDI->Command=Command_C_SendUnitInfo;
								IDI->Buf=(UCHAR*)&tEl->UInf;
								IDI->Cnt=sizeof(UnitInfo);
								IDI->NumPack++;
								SendIDI(IDI,this);
								DeleteIDI(IDI);	
							}
//							ModifUnit(tEl->UInf.pObj);
						}
//						MyLock(&UM->Fl_RE,this);
//						UM->UnitList->Find((UCHAR*)&tEl->UInf.PersID,sizeof(INT64),1);
						tEl=(El_Unit*)UM->UnitList->Next(FSL8);
					}
//					MyUnLock(__FILE__,__LINE__,&UM->Fl_RE,this);
					}
					break;
				case Command_C_UserLogIn:
					{
						if (pIDI->pPList->GetParam(IDUserLogin,pVol,pCnt,ParentMt)!=-1)
						{
							if (UserID!=-1)
							{
								if (strcmp((char*)pVol,UserLogin)==0)
									Logoff();
							}
						}
					}
					break;
				case Command_C_GetLCond:
					if (Fl_HardLock)
					{
						IDInfo*	IDI=GetNewIDI(this);
						AddressInfo	AdrI;
						IDI->AdrInfo=&AdrI;
						InitCommIDI(IDI);
//						IDI->pPList->SetParam(IDHostName,"*",2,this);
//						IDI->pPList->SetParam(IDUnitName,N_Service,9,this);
						strcpy(AdrI.DestinAI.UnitName,N_Service);
						IDI->Command=Command_C_SendLCond;
						IDI->Buf=(UCHAR*)&LCond;
						IDI->Cnt=sizeof(LicenseCond);
						IDI->NumPack++;
						SendIDI(IDI,this);
						DeleteIDI(IDI);
						IDI=GetNewIDI(this);
						AddressInfo	AdrI2;
						IDI->AdrInfo=&AdrI2;
						InitCommIDI(IDI);
//						IDI->pPList->SetParam(IDHostName,"*",2,this);
//						IDI->pPList->SetParam(IDUnitName,N_Service,9,this);
						strcpy(AdrI2.DestinAI.UnitName,N_Service);
						IDI->Command=Command_C_SendLUinfo;
						IDI->Buf=(UCHAR*)&LUinfo;
						IDI->Cnt=sizeof(LC_UnitInfo);
						IDI->NumPack++;
						SendIDI(IDI,this);
						DeleteIDI(IDI);
					}
					break;
				case Command_C_DelHostFromDomain:
					pDllName=NULL;
					if (pIDI->AdrInfo)
					{
//						if (pIDI->pPList->GetParam(IDMyHostName,pVol,pCnt,ParentMt)!=-1)
						if (pIDI->AdrInfo->NativeAI.HostName[0])
						{
							pDllName=pIDI->AdrInfo->NativeAI.HostName;
						}
						if (pIDI->pPList->GetParam(IDString,pVol,pCnt,ParentMt)!=-1)
						{
							if (pDllName)
								SInt->DelHostFromDomain((char*)pDllName,(char*)pVol);
						};
					}
					break;
				case Command_C_AddTract:
					pDllName=NULL;
					if (pIDI->pPList->GetParam(IDPathStorage,pVol,pCnt,ParentMt)!=-1)
					{
						pDllName=(char*)pVol;
					}
					if (pIDI->pPList->GetParam(IDString,pVol,pCnt,ParentMt)!=-1)
					{
						if (pDllName==NULL)
							Neuron->AddTract((char*)pVol,"\\Out");
						else
							Neuron->AddTract((char*)pVol,(char*)pDllName);
					};
					Neuron->SaveCfg();
					break;
				case Command_C_DelTract:
					if (pIDI->pPList->GetParam(IDString,pVol,pCnt,ParentMt)!=-1)
					{
						Neuron->DelTract((char*)pVol);
					};
					MyUnLock(__FILE__,__LINE__,&FlSim,this);
					Neuron->ReStartTr(-1);
					MyLock(__FILE__,__LINE__,&FlSim,this);
					Neuron->SaveCfg();
					break;
				case Command_C_StopTr:
					if (pIDI->pPList->GetParam(IDString,pVol,pCnt,ParentMt)!=-1)
					{
						Neuron->StopTr((char*)pVol);
					};
					Neuron->SaveCfg();
					break;
				case Command_C_StartTr:
					if (pIDI->pPList->GetParam(IDString,pVol,pCnt,ParentMt)!=-1)
					{
						Neuron->StartTr((char*)pVol);
					};
					Neuron->SaveCfg();
					break;
				case Command_C_ReStartTr:
					if (pIDI->pPList->GetParam(IDString,pVol,pCnt,ParentMt)!=-1)
					{
						MyUnLock(__FILE__,__LINE__,&FlSim,this);
						Neuron->ReStartTr((char*)pVol);
						MyLock(__FILE__,__LINE__,&FlSim,this);
					};
					Neuron->SaveCfg();
					break;
				case Command_C_OpenSmStarter:
					M_Cfg->MData.Fl_RunSmSt=0;
					Srv->SaveCfg();
					break;
				case Command_C_CloseSmStarter:
					M_Cfg->MData.Fl_RunSmSt=1;
					Srv->SaveCfg();
					SData->Fl_StateSmartSt=3;
					break;
				case	Command_C_ShutDownHost:
					OpenProcessToken(GetCurrentProcess(), TOKEN_ADJUST_PRIVILEGES |
						TOKEN_QUERY, &hToken);
					LookupPrivilegeValueA(NULL, "SeShutdownPrivilege",
						&tkp.Privileges[0].Luid);

					tkp.PrivilegeCount = 1;
					tkp.Privileges[0].Attributes = SE_PRIVILEGE_ENABLED;


					AdjustTokenPrivileges(hToken, FALSE, &tkp, 0,
						(PTOKEN_PRIVILEGES)NULL, 0);

					ExitWindowsEx(EWX_SHUTDOWN|EWX_FORCEIFHUNG,SHTDN_REASON_MAJOR_OTHER | SHTDN_REASON_MINOR_OTHER | SHTDN_REASON_FLAG_PLANNED);
					break;
				case	Command_C_Restart_Host:
					OpenProcessToken(GetCurrentProcess(), TOKEN_ADJUST_PRIVILEGES |
						TOKEN_QUERY, &hToken);
					LookupPrivilegeValueA(NULL, "SeShutdownPrivilege",
						&tkp.Privileges[0].Luid);

					tkp.PrivilegeCount = 1;
					tkp.Privileges[0].Attributes = SE_PRIVILEGE_ENABLED;


					AdjustTokenPrivileges(hToken, FALSE, &tkp, 0,
						(PTOKEN_PRIVILEGES)NULL, 0);

					ExitWindowsEx(EWX_REBOOT|EWX_FORCEIFHUNG,SHTDN_REASON_MAJOR_OTHER | SHTDN_REASON_MINOR_OTHER | SHTDN_REASON_FLAG_PLANNED);
					break;
				case	Command_C_Restart_Neuron:
					SData->Fl_StateApp=3;
					if ((SData->Fl_StateSmartSt==0)&&(M_Cfg->MData.Fl_RunSmSt))

					{
						PROCESSENTRY32 pEntry;
						if(!FindProcess(_T("SmartStarter.exe"),&pEntry))
						{
							HWND hwnd=NULL;
							char	FullPath[256];
							sprintf(FullPath,"%s\\SmartStarter.exe", M_Cfg->MData.Path);
							HINSTANCE  rcc=ShellExecuteA(hwnd, "open",FullPath, NULL, M_Cfg->MData.Path, SW_SHOWNORMAL);
						}
					}
					break;

				case	Command_C_Close_Neuron:
					SData->Fl_StateApp=4;
					if(SData->Fl_StateSmartSt==1)
						SData->Fl_StateSmartSt=2;
					else
						SData->Fl_StateSmartSt=0;
					break;		
				case Command_C_SendLCond:
					if (pIDI->Cnt==sizeof(LicenseCond))
					{
						if (M_Cfg->MData.CustomerID==((LicenseCond*)pIDI->Buf)->CustomerID)
							CopyLic((LicenseCond*)pIDI->Buf);
					}
					break;
				case Command_C_SendLUinfo:
					if (pIDI->Cnt==sizeof(LC_UnitInfo))
					{
						if (M_Cfg->MData.CustomerID==((LC_UnitInfo*)pIDI->Buf)->CustomerID)
							CopyLicUnit((LC_UnitInfo*)pIDI->Buf);
					}
					break;
				case Command_C_GetDllFile:
					if ((pIDI->AdrInfo)&&(M_Cfg->MData.Fl_SendUpD))
					{
//						if (pIDI->pPList->GetParam(IDMyHostName,pVol,pCnt,ParentMt)!=-1)
//						{
//							pMyUnitHost=(char*)pVol;
//						}
						pDllName=NULL;
						if (pIDI->pPList->GetParam(IDFileName,pVol,pCnt,ParentMt)!=-1)
						{
							pDllName=(char*)pVol;
						}
						Module*	tMd=DLLL->FindModule(pDllName);
						if (tMd)
						{
							El_DllFile*	tEl_DF=new El_DllFile(pDllName,pIDI->AdrInfo->NativeAI.HostName,&tMd->CrTime,&tMd->AcTime,&tMd->WrTime);
							if (DllFileQwe->Add(tEl_DF)==-1)
							{
								sprintf(tBf,"Repeat GetDllFile=%s From Host=%s",pDllName,pIDI->AdrInfo->NativeAI.HostName);
								OutLog(tBf,this,"UpDate");
								delete	tEl_DF;
							}
							else
							{
								sprintf(tBf,"GetDllFile=%s From Host=%s",pDllName,pIDI->AdrInfo->NativeAI.HostName);
								OutLog(tBf,this,"UpDate");
							};
						}
					}
					break;
				case Command_C_GetMainFile:
					if ((pIDI->AdrInfo)&&(M_Cfg->MData.Fl_SendUpD))
					{
//						if (pIDI->pPList->GetParam(IDMyHostName,pVol,pCnt,ParentMt)!=-1)
//						{
//							pMyUnitHost=(char*)pVol;
//						}
						if (pIDI->pPList->GetParam(IDFileName,pVol,pCnt,ParentMt)!=-1)
						{
							pDllName=(char*)pVol;
						}
//						Module*	tMd=DLLL->FindModule(pDllName);
						Module*	tMd=FindModule(pDllName);
						if (tMd)
						{
							El_DllFile*	tEl_DF=new El_DllFile(pDllName,pIDI->AdrInfo->NativeAI.HostName,&tMd->CrTime,&tMd->AcTime,&tMd->WrTime);
							if (MainFileQwe->Add(tEl_DF)==-1)
							{
								sprintf(tBf,"Repeat GetMainFile=%s From Host=%s",pDllName,pIDI->AdrInfo->NativeAI.HostName);
								OutLog(tBf,this,"UpDate");
								delete	tEl_DF;
							}
							else
							{
								sprintf(tBf,"GetMainFile=%s From Host=%s",pDllName,pIDI->AdrInfo->NativeAI.HostName);
								OutLog(tBf,this,"UpDate");
							};
						}
					}
					break;
				case Command_C_UpDllData:
					if ((pIDI->AdrInfo)&&(M_Cfg->MData.Fl_RecUpD))
					{
						char	FullFileName1[512];
						char	FullFileName2[512];
// 						if (pIDI->pPList->GetParam(IDMyHostName,pVol,pCnt,ParentMt)!=-1)
// 						{
// 							pMyUnitHost=(char*)pVol;
// 						};
						sprintf(FullFileName1,"%s\\Load",MainPath);
						sprintf(FullFileName2,"%s\\Dll\\New",MainPath);
						MoveFileA(FullFileName1,FullFileName2);
						sprintf(tBf,"UpDllDate From Host=%s",pIDI->AdrInfo->NativeAI.HostName);
						OutLog(tBf,this,"UpDate");
						CntT4=0;TimGUI=0;
					}
					break;
				case Command_C_UpMainData:
					if ((pIDI->AdrInfo)&&(M_Cfg->MData.Fl_RecUpD))
					{
						char	FullFileName1[512];
						char	FullFileName2[512];
// 						if (pIDI->pPList->GetParam(IDMyHostName,pVol,pCnt,ParentMt)!=-1)
// 						{
// 							pMyUnitHost=(char*)pVol;
// 						};
						sprintf(FullFileName1,"%s\\LoadInstall",MainPath);
						sprintf(FullFileName2,"%s\\Install",MainPath);
						MoveFileA(FullFileName1,FullFileName2);
						sprintf(tBf,"UpMainDate From Host=%s",pIDI->AdrInfo->NativeAI.HostName);
						OutLog(tBf,this,"UpDate");
						CntT4=0;
					}
					break;
				case Command_C_SendDllFile:
					if ((pIDI->AdrInfo)&&(M_Cfg->MData.Fl_RecUpD))
					{
						char	FullFileName[512];
						char	FullFileName2[512];
						int		FileDll;
						int		FileSize=0;
						int		RFileSize;
// 						if (pIDI->pPList->GetParam(IDMyHostName,pVol,pCnt,ParentMt)!=-1)
// 						{
// 							pMyUnitHost=(char*)pVol;
// 						};
						if (pIDI->pPList->GetParam(IDFileName,pVol,pCnt,ParentMt)!=-1)
						{
							pDllName=(char*)pVol;
						}
						if (pDllName)
						{
							sprintf(FullFileName,"%s\\Load",MainPath);
							CreateDirectoryA(FullFileName,NULL);
							sprintf(FullFileName,"%s\\Load\\%s_",MainPath,pDllName);
							switch (pIDI->StateBlock)
							{
							case StateBlock_BEGIN:
								sprintf(tBf,"Begin SendDllFile=%s From Host=%s",pDllName,pIDI->AdrInfo->NativeAI.HostName);
								OutLog(tBf,this,"UpDate");
								_unlink(FullFileName);
								if ((FileDll = _open(FullFileName, _O_RDWR|_O_CREAT|_O_APPEND|_O_BINARY,_S_IREAD|_S_IWRITE ))!=-1)
								{
									_close(FileDll);
								}
								break;
							case 0:
								if ((FileDll = _open(FullFileName, _O_RDWR|_O_APPEND|_O_BINARY,_S_IREAD|_S_IWRITE ))!=-1)
								{
									_lseek( FileDll, 0L, SEEK_END );
									_write(FileDll,(void *)pIDI->Buf,pIDI->Cnt);
									_close(FileDll);
								}
								break;
							case StateBlock_END:
								if (pIDI->pPList->GetParam(IDFileLen,pVol,pCnt,ParentMt)!=-1)
								{
									FileSize=*(int*)pVol;
								};
								FileDll=-1;
								RFileSize=0;
								if ((FileDll = _open(FullFileName, _O_RDWR|_O_APPEND|_O_BINARY,_S_IREAD|_S_IWRITE ))!=-1)
								{
									RFileSize=_lseek( FileDll, 0L, SEEK_END );
									_close(FileDll);
								}
								if (FileSize==RFileSize)
								{
									sprintf(FullFileName2,"%s\\Load\\%s",MainPath,pDllName);
									_unlink(FullFileName2);
									MoveFileA(FullFileName,FullFileName2);
									HANDLE	FlMod=CreateFileA(FullFileName2,GENERIC_WRITE,FILE_SHARE_WRITE,NULL,OPEN_EXISTING,FILE_ATTRIBUTE_NORMAL,NULL);  
									BOOL brc=SetFileTime(FlMod,&pIDI->TimeBegin.FTime,&pIDI->TimeClose.FTime,&pIDI->TimeLastWr.FTime);
									CloseHandle(FlMod);
									sprintf(tBf,"End   SendDllFile=%s From Host=%s",pDllName,pIDI->AdrInfo->NativeAI.HostName);
									OutLog(tBf,this,"UpDate");
								}
								break;
							}
						}
					}
					break;
				case Command_C_SendMainFile:
					if ((pIDI->AdrInfo)&&(M_Cfg->MData.Fl_RecUpD))
					{
						char	FullFileName[512];
						char	FullFileName2[512];
						int		FileDll;
						int		FileSize=0;
						int		RFileSize=0;
// 						if (pIDI->pPList->GetParam(IDMyHostName,pVol,pCnt,ParentMt)!=-1)
// 						{
// 							pMyUnitHost=(char*)pVol;
// 						};
						if (pIDI->pPList->GetParam(IDFileName,pVol,pCnt,ParentMt)!=-1)
						{
							pDllName=(char*)pVol;
						}
						if (pDllName)
						{
							sprintf(FullFileName,"%s\\LoadInstall",MainPath);
							CreateDirectoryA(FullFileName,NULL);
							sprintf(FullFileName,"%s\\LoadInstall\\%s_",MainPath,pDllName);
							switch (pIDI->StateBlock)
							{
							case StateBlock_BEGIN:
								_unlink(FullFileName);
								if ((FileDll = _open(FullFileName, _O_RDWR|_O_CREAT|_O_APPEND|_O_BINARY,_S_IREAD|_S_IWRITE ))!=-1)
								{
									_close(FileDll);
								}
								break;
							case 0:
								if ((FileDll = _open(FullFileName, _O_RDWR|_O_APPEND|_O_BINARY,_S_IREAD|_S_IWRITE ))!=-1)
								{
									_lseek( FileDll, 0L, SEEK_END );
									_write(FileDll,(void *)pIDI->Buf,pIDI->Cnt);
									_close(FileDll);
								}
								break;
							case StateBlock_END:
								if (pIDI->pPList->GetParam(IDFileLen,pVol,pCnt,ParentMt)!=-1)
								{
									FileSize=*(int*)pVol;
								};
								FileDll=-1;
								if ((FileDll = _open(FullFileName, _O_RDWR|_O_APPEND|_O_BINARY,_S_IREAD|_S_IWRITE ))!=-1)
								{
									RFileSize=_lseek( FileDll, 0L, SEEK_END );
									_close(FileDll);
								}
								if (FileSize==RFileSize)
								{
									sprintf(FullFileName2,"%s\\LoadInstall\\%s",MainPath,pDllName);
									_unlink(FullFileName2);
									MoveFileA(FullFileName,FullFileName2);
									HANDLE	FlMod=CreateFileA(FullFileName2,GENERIC_WRITE,FILE_SHARE_WRITE,NULL,OPEN_EXISTING,FILE_ATTRIBUTE_NORMAL,NULL);  
									BOOL brc=SetFileTime(FlMod,&pIDI->TimeBegin.FTime,&pIDI->TimeClose.FTime,&pIDI->TimeLastWr.FTime);
									CloseHandle(FlMod);
									sprintf(tBf,"SendDllFile=%s From Host=%s",pDllName,pIDI->AdrInfo->NativeAI.HostName);
									OutLog(tBf,this,"UpDate");
								}
								break;
							}
						}
					}
					break;
				case Command_C_SendMainTime:
					if ((pIDI->AdrInfo)&&(M_Cfg->MData.Fl_RecUpD)&&(MainFileQwe->NumElem==0))
					{
						int	rSizeMod=pIDI->Cnt/sizeof(Module);
						if (rSizeMod*sizeof(Module)==pIDI->Cnt)
						{
// 							if (pIDI->pPList->GetParam(IDMyHostName,pVol,pCnt,ParentMt)!=-1)
// 							{
// 								pMyUnitHost=(char*)pVol;
// 							}
							sprintf(tBf,"SendMainTime From Host=%s",pIDI->AdrInfo->NativeAI.HostName);
							OutLog(tBf,this,"UpDate");
							Module*	pMod=(Module*)pIDI->Buf;
//							pMod++;rSizeMod--;
							while(rSizeMod)
							{
//								Module* pM=DLLL->FindModule(pMod->Name);
								Module* pM=FindModule(pMod->Name);
								if (pM)
								{
									char	TbT1[256];
									char	TbT2[256];
									strcpy(TbT1,cTime(&pM->WrTime));
									strcpy(TbT2,cTime(&pMod->WrTime));
									if (pM->WrTime.Time+10*KtimeS<pMod->WrTime.Time)
									{	// íàøà ñòàðåå
										if (pIDI->AdrInfo->NativeAI.HostName[0])
										{
											sprintf(tBf,"Our MainFile=%s is oldest. Request From Host=%s",pM->Name,pIDI->AdrInfo->NativeAI.HostName);
											OutLog(tBf,this,"UpDate");
											sprintf(tBf,"Our Date=%s Host=%s Date=%s",TbT1, pIDI->AdrInfo->NativeAI.HostName,TbT2);
											OutLog(tBf,this,"UpDate");

											IDInfo*	IDI=GetNewIDI(this);
											InitCommIDI(IDI);
											AddressInfo	AdrI;
											IDI->AdrInfo=&AdrI;
//											IDI->pPList->SetParam(IDHostName,pMyUnitHost,strlen(pMyUnitHost)+1,this);
											strcpy(AdrI.DestinAI.HostName,pIDI->AdrInfo->NativeAI.HostName);
//											IDI->pPList->SetParam(IDUnitName,N_Service,9,this);
											strcpy(AdrI.DestinAI.HostName,pIDI->AdrInfo->NativeAI.UnitName);
											IDI->pPList->SetParam(IDFileName,pM->Name,strlen(pM->Name)+1,this);
											IDI->Command=Command_C_GetMainFile;
											IDI->NumPack++;
											IDI->Buf=NULL;
											IDI->Cnt=0;
											SendIDI(IDI,this);
											DeleteIDI(IDI);	
										}
									}
									else
									{
										if (pM->WrTime.Time>pMod->WrTime.Time+10*KtimeS)
										{	// Íàøà íîâåå
											if (M_Cfg->MData.Fl_SendUpD)
											{
												if (pIDI->AdrInfo->NativeAI.HostName[0])
												{
													El_DllFile*	tEl_DF=new El_DllFile(pM->Name,pIDI->AdrInfo->NativeAI.HostName,&pM->CrTime,&pM->AcTime,&pM->WrTime);
													if (MainFileQwe->Add(tEl_DF)==-1)
													{
														delete	tEl_DF;
													}
													else
													{
														sprintf(tBf,"Our MainFile=%s is newest. Request From Host=%s",pM->Name,pIDI->AdrInfo->NativeAI.HostName);
														OutLog(tBf,this,"UpDate");
														sprintf(tBf,"Our Date=%s Host=%s Date=%s",TbT1, pIDI->AdrInfo->NativeAI.HostName,TbT2);
														OutLog(tBf,this,"UpDate");
													};
												}
											}
										}
										else
										{	// Îáìåí äàííûìè íå íóæåí
//											_asm	nop;
										}
									}
								}
								pMod++;rSizeMod--;
							}

						};
					}
					break;
				case Command_C_SendDllTime:
					if ((pIDI->AdrInfo)&&(M_Cfg->MData.Fl_RecUpD)&&(DllFileQwe->NumElem==0))
					{
						int	rSizeMod=pIDI->Cnt/sizeof(Module);
						if (rSizeMod*sizeof(Module)==pIDI->Cnt)
						{
// 							if (pIDI->pPList->GetParam(IDMyHostName,pVol,pCnt,ParentMt)!=-1)
// 							{
// 								pMyUnitHost=(char*)pVol;
// 							}
							char	TbT1[256];
							char	TbT2[256];
							sprintf(tBf,"SendDllTime From Host=%s", pIDI->AdrInfo->NativeAI.HostName);
							OutLog(tBf,this,"UpDate");
							Module*	pMod=(Module*)pIDI->Buf;
							pMod++;rSizeMod--;
							while(rSizeMod)
							{
								Module* pM=DLLL->FindModule(pMod->Name);
								if (pM)
								{
									strcpy(TbT1,cTime(&pM->WrTime));
									strcpy(TbT2,cTime(&pMod->WrTime));
									if (pM->WrTime.Time+10*KtimeS<pMod->WrTime.Time)
									{	// íàøà ñòàðåå
										if ( pIDI->AdrInfo->NativeAI.HostName[0])
										{
											sprintf(tBf,"Our DllFile=%s is oldest. Request From Host=%s",pM->Name, pIDI->AdrInfo->NativeAI.HostName);
											OutLog(tBf,this,"UpDate");
											sprintf(tBf,"Our Date=%s Host=%s Date=%s",TbT1,  pIDI->AdrInfo->NativeAI.HostName,TbT2);
											OutLog(tBf,this,"UpDate");
											IDInfo*	IDI=GetNewIDI(this);
											AddressInfo	AdrI;
											InitCommIDI(IDI);
											IDI->AdrInfo=&AdrI;
//											IDI->pPList->SetParam(IDHostName,pMyUnitHost,strlen(pMyUnitHost)+1,this);
											strcpy(AdrI.DestinAI.HostName,pIDI->AdrInfo->NativeAI.HostName);
//											IDI->pPList->SetParam(IDUnitName,N_Service,9,this);
											strcpy(AdrI.DestinAI.UnitName,N_Service);
											IDI->pPList->SetParam(IDFileName,pM->Name,strlen(pM->Name)+1,this);
											IDI->Command=Command_C_GetDllFile;
											IDI->NumPack++;
											IDI->Buf=NULL;
											IDI->Cnt=0;
											SendIDI(IDI,this);
											DeleteIDI(IDI);	
										}
									}
									else
									{
										if (pM->WrTime.Time>pMod->WrTime.Time+10*KtimeS)
										{	// Íàøà íîâåå
											if (M_Cfg->MData.Fl_SendUpD)
											{
												if (pIDI->AdrInfo->NativeAI.HostName[0])
												{
													El_DllFile*	tEl_DF=new El_DllFile(pM->Name,pIDI->AdrInfo->NativeAI.HostName,&pM->CrTime,&pM->AcTime,&pM->WrTime);
													if (DllFileQwe->Add(tEl_DF)==-1)
													{
														delete	tEl_DF;
													}
													else
													{
														sprintf(tBf,"Our DllFile=%s is newest. Request From Host=%s",pM->Name,pIDI->AdrInfo->NativeAI.HostName);
														OutLog(tBf,this,"UpDate");
														sprintf(tBf,"Our Date=%s Host=%s Date=%s",TbT1, pIDI->AdrInfo->NativeAI.HostName,TbT2);
														OutLog(tBf,this,"UpDate");
													};
												}
											}
										}
										else
										{	// Îáìåí äàííûìè íå íóæåí
//											_asm	nop;
										}
									}
								}
								pMod++;rSizeMod--;
							}
							
						};
					}
					break;
				}
				break;
			case TypeIDI_TABLE:
//				MyUnLock(&Fl_RE6,this);
				Pack2Table(pIDI);
//				MyLock(&Fl_RE6,this);
				break;
			default:
				break;
			}
		}
		break;
	case 1002:
//		void*		pVol;
//		size_t		pCnt;
		ParentMt=NULL;
		El_TabList*	ElTL;
		if (pIDI->pPList->GetParam(IDIDFileSnsFr,pVol,pCnt,ParentMt)!=-1)
		{
			ElTL=(El_TabList*)TableList->Find((UCHAR*)pVol,pCnt);
			if (ElTL)
			{
				ElTL->IDT->DataIDIEx(0,pIDI);
			}
		}
		else
		{
			ElTL=(El_TabList*)TableList->Find((UCHAR*)&pIDI->ID_File,sizeof(INT64));
			if (ElTL==NULL) ElTL=(El_TabList*)TableList->Find((UCHAR*)&pIDI->ID_SeansFrom,sizeof(INT64));
			if (ElTL)
			{
				ElTL->IDT->DataIDIEx(0,pIDI);
			}
		}
		break;
	case 1003:
		if (pIDI->AdrInfo)
		{

// 			UnitMode[0]=0;
// 			if (pIDI->pPList->GetParam(IDHostName,pVol,pCnt,ParentMt)!=-1)
// 			{
// 				pUnitHost=(char*)pVol;
// 				if (strcmp(pUnitHost,"*")==0)
// 				{
// 					pUnitHost=ShortHostName;
// 				}
// 			}
// 			if (pIDI->pPList->GetParam(IDDomainName,pVol,pCnt,ParentMt)!=-1)
// 			{
// 				pDomainName=(char*)pVol;
// 				if (strcmp(pDomainName,"*")==0)
// 				{
// 					pDomainName=UInfo.DomainName;
// 				}
// 			}
// 			if (pIDI->pPList->GetParam(IDUnitName,pVol,pCnt,ParentMt)!=-1)
// 			{
// 				pUnitName=(char*)pVol;
// 				if (pUnitName[0]==0)
// 				{
// 					{size_t iasm=1;}
// 					//						strcpy(UnitMode,pUnitMode);
// 				}
// 			}
// 			if (pIDI->pPList->GetParam(IDUnitMode,pVol,pCnt,ParentMt)!=-1)
// 			{
// 				pUnitMode=(char*)pVol;
// 				if (pUnitMode[0]!='*')
// 				{
// 					strcpy(UnitMode,pUnitMode);
// 				}
// 				//					tIDI->pPList->DelParam(IDUnitMode);
// 			}
// 			if (pIDI->pPList->GetParam(IDUnitID,pVol,pCnt,ParentMt)!=-1)
// 			{
// 				pUnitID=*(INT64*)pVol;
// 			}
// 
// 			if (pIDI->pPList->GetParam(IDNCStrategic,pVol,pCnt,ParentMt)!=-1)
// 			{
// 				pStrategic=(char*)pVol;
// 			}
			//				if (tIDI->pPList->GetParam(IDMyHostName,pVol,pCnt,ParentMt)!=-1)
			//				{
			//					pMyUnitHost=(char*)pVol;
			//				}
			//				if (tIDI->pPList->GetParam(IDMyUnitName,pVol,pCnt,ParentMt)!=-1)
			//				{
			//					pSourceAI.UnitName=(char*)pVol;
			//				}
			//				if ((pMyUnitHost)&&(pSourceAI.UnitName)&&(mStrCmpHost(pMyUnitHost,HostName)==0)&&((strcmp(pSourceAI.UnitName,pUnitName)==0)))
			//				{
			//					break;
			//				}
			//				
			if (pIDI->AdrInfo->DestinAI.UnitID)
			{
	//			MyLock(__FILE__,__LINE__,&UM->Fl_RE,this);
				tEl=(El_Unit*)UM->UnitList->Find((UCHAR*)&pIDI->AdrInfo->DestinAI.UnitID,sizeof(INT64));
	//			MyUnLock(__FILE__,__LINE__,&UM->Fl_RE,this);
				if (tEl)
				{
					if (strcmp(tEl->UInf.Host,Srv->ShortHostName)==0)
					{
						tEl->UInf.pObj->UserCallBackIDIEx(0,pIDI,NULL,1);
					}
				}
			}
			else
			{
				//					if ((pDomainName==NULL)||(strcmp(pDomainName,UInfo.DomainName)==0))
				if ((pIDI->AdrInfo->DestinAI.HostName[0]==0)||(mStrCmpHost(pIDI->AdrInfo->DestinAI.HostName,ShortHostName)==0))
				{
					if (pIDI->AdrInfo->DestinAI.UnitName[0]==0)
					{
//						pUnitName="*";
					}
					else
					{
						if (strcmp(pIDI->AdrInfo->DestinAI.UnitName,N_Service)==0)
						{
							//								MyUnLock(__FILE__,__LINE__,&Fl_RE6,this);
							DataIDI(4,pIDI);
							//								MyLock(__FILE__,__LINE__,&Fl_RE6,this);
						}
						else
						{
							if ((pIDI->AdrInfo->NCStrategic&B_Alternate)==0)
							{
								FindStrList8	FSL84;
								tEl=(El_Unit*)UM->UnitList->Begin(FSL84);
								while (tEl)
								{
									//									MyUnLock(__FILE__,__LINE__,&UM->Fl_RE,this);
									if (mStrCmpHost(ShortHostName,tEl->UInf.Host)==0)
									{
										if ((pIDI->AdrInfo->DestinAI.DomainName[0]==0)||(strcmp(pIDI->AdrInfo->DestinAI.DomainName,tEl->UInf.DomainName)==0))
										{
											if (strcmp(pIDI->AdrInfo->DestinAI.UnitName,tEl->UInf.UnitName)==0)
											{
												if(tEl->UInf.pObj)
												{
													if (pIDI->AdrInfo->DestinAI.UnitMode[0])
													{
														if (strcmp(pIDI->AdrInfo->DestinAI.UnitMode,tEl->UInf.pObj->UInfo.Mode)==0)
														{
															//															MyUnLock(__FILE__,__LINE__,&Fl_RE6,this);

															tEl->UInf.pObj->UserCallBackIDIEx(0,pIDI,NULL,1);
															//															tEl->UInf.pObj->DataFF(0,pmas,Cnt);
															//															MyLock(__FILE__,__LINE__,&Fl_RE6,this);
														}
													}
													else
													{
														//														MyUnLock(__FILE__,__LINE__,&Fl_RE6,this);
														tEl->UInf.pObj->UserCallBackIDIEx(0,pIDI,NULL,1);
														//														tEl->UInf.pObj->DataFF(0,pmas,Cnt);
														//														MyLock(__FILE__,__LINE__,&Fl_RE6,this);
													}
												}
											}
										}
									}
									//									MyLock(__FILE__,__LINE__,&UM->Fl_RE,this);
									//						UM->UnitList->Find((UCHAR*)&tEl->UInf.PersID,sizeof(INT64),1);
									tEl=(El_Unit*)UM->UnitList->Next(FSL84);
								}
							}
							else
							if ((pIDI->AdrInfo->NCStrategic&B_Alternate)!=0)
							{
								if (pIDI->StateBlock&StateBlock_BEGIN)
								{// Sasha's improvement
									tEl=(El_Unit*)UM->FindUnit(pIDI->AdrInfo->DestinAI.UnitName,pIDI->AdrInfo->DestinAI.DomainName,ShortHostName/*pIDI->AdrInfo->DestinAI.HostName*/,pIDI->AdrInfo->DestinAI.UnitMode,1);
									if (tEl)
									{
//										pIDI->pPList->SetParam(IDUnitID,&tEl->UInf.PersID,sizeof(INT64),this);
										pIDI->AdrInfo->DestinAI.UnitID=tEl->UInf.PersID;
										if(tEl->UInf.pObj)
											tEl->UInf.pObj->UserCallBackIDIEx(0,pIDI,NULL,1);
									}
								}
								else
								{//Error!!!
									Srv->OutLog("Error",this,"Strategic");
								}
							}
						}
					}
	// 				if (strcmp(pUnitName,N_Service)==0)
	// 				{
	// 					//								MyUnLock(__FILE__,__LINE__,&Fl_RE6,this);
	// 					DataIDI(4,pIDI);
	// 					//								MyLock(__FILE__,__LINE__,&Fl_RE6,this);
	// 				}
	// 				else
	//				{	// Find other units
	//					MyLock(__FILE__,__LINE__,&UM->Fl_RE,this);

	//					MyUnLock(__FILE__,__LINE__,&UM->Fl_RE,this);
	//				}
				}

			}
		}
		break;
	case 1004:// ???????????????? no need
//		pIDI->pPList->SetParam(IDNetType,"LAN",4,this);
		switch(pIDI->TypeIDI)
		{
		case TypeIDI_COMMAND:
			switch(pIDI->Command)
			{
			case Command_C_GetIDI:
				//{size_t iasm=1;}
				break;
			}
			break;
		case TypeIDI_TABLE:
			//{size_t iasm=1;}
			break;
		}
		DataIDI(1003,pIDI);
		break;
	case 2000:
//		if (BDBIDInfo)
//		{
//			BDBIDInfo->DataIDIEx(0,pIDI);
//		}
		break;
	case 2002:
		El_Tab*		tElT;
		El_Owner*	tElO;
		// !!!!!!!!!
//		char tBbb[512];
//		sprintf(tBbb,"UpD El ID=%I64x to All Unit Command=%d",pIDI->ID_File,pIDI->Command);
//		OutLog(tBbb,this,"ServicesUpData");

		if ((pIDI->ID_SeansFrom<TabParametr)/*&&(IDT_Neuron)*/)
		{
			tElT=(El_Tab*)IDT_NeuronGlobal->IDL->Find((UCHAR*)&pIDI->ID_File,sizeof(INT64));
			if (tElT==NULL) tElT=(El_Tab*)IDT_NeuronLocalCustomer->IDL->Find((UCHAR*)&pIDI->ID_File,sizeof(INT64));
			if (tElT==NULL) tElT=(El_Tab*)IDT_NeuronLocalSite->IDL->Find((UCHAR*)&pIDI->ID_File,sizeof(INT64));
//			if (tElT==NULL)
//			{
//				tElT=IDT_Neuron->GetElTab(&Anchor,pIDI->ID_File,1);
//			}
			if (tElT)
			{
				if ((tElT->OwnerList)&&(tElT->OwnerList->NumElem))
				{
					FindStrList8	FSL8;
					tElO=(El_Owner*)tElT->OwnerList->Begin(FSL8);
					while (tElO)
					{
						if ((IDT_NeuronGlobal!=tElO->Own.pDec)&&(IDT_NeuronLocalCustomer!=tElO->Own.pDec)&&(IDT_NeuronLocalSite!=tElO->Own.pDec))
						{

							if ((!IsBadReadPtr((void*)tElO->Own.pDec,sizeof(Decoder)))&&(!IsBadReadPtr((void*)tElO->Own.pDec->Name_Mt,40))&&(strcmp(tElO->NameMt,tElO->Own.pDec->Name_Mt)==NULL))
							{
	//							char tBbb[512];
//								sprintf(tBbb,"UpD El ID=%I64x to Unit=%s Command=%d",pIDI->ID_File,tElO->Own.pDec->Name_Mt,pIDI->Command);
//								OutLog(tBbb,this,"ServicesUpData");
								int	Fl_Buf=0;
//								if ( pIDI->Command==Command_DEL)
									Fl_Buf=1;
								tElO->Own.pDec->UserCallBackIDIEx(1, pIDI, &tElO->Own,Fl_Buf);
							}
							else
								tElT->DelChild(pIDI->ID_File);
						}
						tElO=(El_Owner*)tElT->OwnerList->Next(FSL8);
					};
				};
			}
			else
			{
				tElT=NULL;
			};
		};
		break;
	case 2003:
		if ((pIDI->AdrInfo)&&(Fl_Repl==1)&&(Fl_Prm==1)&&(NetCommLAN!=NULL))
		{
//			void*	Prm;
//			if (pIDI->pPList->GetParam(IDHostName,Prm)==-1)
//				pIDI->pPList->SetParam(IDHostName,"*",2,this);
//			pIDI->pPList->SetParam(IDUnitName,N_Service,9,this);
			strcpy(pIDI->AdrInfo->DestinAI.UnitName,N_Service);
			// Adress From
//			pIDI->pPList->SetParam(IDMyUnitName,this->Name_Mt,1+strlen(this->Name_Mt),this);
//			strcpy(pIDI->AdrInfo->NativeAI.UnitName,this->Name_Mt);
//			SendMt=this;
			if (pIDI->AdrInfo->NativeAI.HostName[0]==0)
				strcpy(pIDI->AdrInfo->NativeAI.HostName,ShortHostName);
			strcpy(pIDI->AdrInfo->SourceAI.HostName,ShortHostName);
			if (pIDI->AdrInfo->NativeAI.UnitName[0]==0)
				strcpy(pIDI->AdrInfo->NativeAI.UnitName,this->Name_Mt);
			strcpy(pIDI->AdrInfo->SourceAI.UnitName,this->Name_Mt);
			if (pIDI->AdrInfo->NativeAI.DomainName[0]==0)
				strcpy(pIDI->AdrInfo->NativeAI.DomainName,this->UInfo.DomainName);
			strcpy(pIDI->AdrInfo->SourceAI.DomainName,this->UInfo.DomainName);

			rc=NetCommLAN->DataIDIEx(0,pIDI);
//			if (pIDI->pPList->GetParam(IDLocal,pVol,pCnt,ParentMt)!=-1)
//			{
//				Fl_Local=1;
//				pIDI->pPList->DelParam(IDLocal);
//			}
//			DataIDI(3,pIDI);
		}
		break;
	}
	return rc;
}

int	Services::SendToLocal(IDInfo* pIDI)
{
	int			rc=0;
	int			rc2=0;
	void*		pVol;
	size_t		pCnt;
//	char*		pUnitName="*";
//	char*		pUnitMode="*";
//	char*		pDomainName="*";
//	char*		pSourceAI.UnitName="*";
	El_Unit*	tEl;
	char		UnitMode[80];
//_sss0:
	UnitMode[0]=0;

	if (pIDI->pPList->GetParam(IDLocal,pVol,pCnt,ParentMt)!=-1)
	{
		pIDI->pPList->DelParam(IDLocal);
		return rc;
	};
	if (pIDI->AdrInfo==NULL)
		return rc;

// 	if (pIDI->pPList->GetParam(IDMyUnitName,pVol,pCnt,ParentMt)!=-1)
// 	{
// 		pSourceAI.UnitName=(char*)pVol;
// 	}

// 	if (pIDI->pPList->GetParam(IDDomainName,pVol,pCnt,ParentMt)!=-1)
// 	{
// 		pDomainName=(char*)pVol;
// 	}
// 	if (pIDI->pPList->GetParam(IDUnitName,pVol,pCnt,ParentMt)!=-1)
// 	{
// 		pUnitName=(char*)pVol;
// 	}
// 	if (pIDI->pPList->GetParam(IDUnitMode,pVol,pCnt,ParentMt)!=-1)
// 	{
// 		pUnitMode=(char*)pVol;
// 		if (pUnitMode[0]!='*')
// 		{
// 			strcpy(UnitMode,pUnitMode);
// 		}
// 		//					tIDI->pPList->DelParam(IDUnitMode);
// 	}
		//						if ((pDomainName==NULL)||((pDomainName)&&((strcmp(pDomainName,UInfo.DomainName)==0)||(pDomainName[0]=='*'))))
	if (pIDI->AdrInfo->DestinAI.UnitID!=0)
	{
		tEl=(El_Unit*)UM->FindUnitbyPersID(pIDI->AdrInfo->DestinAI.UnitID);
		if (tEl)
		{
			if (tEl->UInf.pObj)
				tEl->UInf.pObj->UserCallBackIDIEx(0,pIDI,NULL,1);
		}
	}
	else
	{
		if (pIDI->AdrInfo->DestinAI.UnitName[0]!=0)
		{
			//					MyLock(__FILE__,__LINE__,&UM->Fl_RE,this);
			if ((pIDI->AdrInfo->NCStrategic&B_Alternate)==0)
			{
				FindStrList8	FSL8;
				tEl=(El_Unit*)UM->UnitList->Begin(FSL8);
				while (tEl)
				{
					//								MyUnLock(__FILE__,__LINE__,&UM->Fl_RE,this);
					if (mStrCmpHost(ShortHostName,tEl->UInf.Host)==0)
					{
						if ((pIDI->AdrInfo->DestinAI.DomainName[0]==0)||(strcmp(pIDI->AdrInfo->DestinAI.DomainName,tEl->UInf.DomainName)==0))
						{
							if (strcmp(pIDI->AdrInfo->DestinAI.UnitName,tEl->UInf.UnitName)==0)
							{
								if (tEl->UInf.PersID!=pIDI->AdrInfo->NativeAI.UnitID)	//!!!!!!!!!!!!!!!!!!!!!
									//						if ((tEl->UInf.pObj)&&(strcmp(tEl->UInf.pObj->Name_Mt,pIDI->AdrInfo->SourceAI.UnitName)!=0))	//!!!!!!!!!!!!!!!!!!!!!
								{
									//												IDInfo*		tIDIC=CrIDICopy(tIDI,this);
									if (pIDI->AdrInfo->DestinAI.UnitMode[0])
									{
										if (strcmp(pIDI->AdrInfo->DestinAI.UnitMode,tEl->UInf.pObj->UInfo.Mode)==0)
										{
											//														CopyIDInfo(tIDI,tIDIC);
											//														tIDIC->Buf=tIDI->Buf;
											//														tIDIC->Cnt=tIDI->Cnt;
											//														tIDIC->OldBuf=tIDI->OldBuf;
											//														tIDIC->OldCnt=tIDI->OldCnt;
											//														MyLock(__FILE__,__LINE__,&Fl_RE6,this);
											tEl->UInf.pObj->UserCallBackIDIEx(0,pIDI,NULL,1); rc++;
											//														MyUnLock(__FILE__,__LINE__,&Fl_RE6,this);
										}
										else
										{
											rc2|=0x100000;
										}
									}
									else
									{
										//													CopyIDInfo(tIDI,tIDIC);
										//													tIDIC->Buf=tIDI->Buf;
										//													tIDIC->Cnt=tIDI->Cnt;
										//													tIDIC->OldBuf=tIDI->OldBuf;
										//													tIDIC->OldCnt=tIDI->OldCnt;
										//													MyUnLock(__FILE__,__LINE__,&Fl_RE6,this);
										tEl->UInf.pObj->UserCallBackIDIEx(0,pIDI,NULL,1); rc++;
										//													MyLock(__FILE__,__LINE__,&Fl_RE6,this);
									}
									//												DelIDICopy(tIDI,tIDIC);
								}
								else
								{
									rc2|=0x10000;
								}
							}
							else
							{
								rc2|=0x20000;
							}
						}
						else
						{
							rc2|=0x40000;
						}
					}
					else
					{
						rc2|=0x80000;
					}
					//								MyLock(__FILE__,__LINE__,&UM->Fl_RE,this);
					//						UM->UnitList->Find(tEl->Buff,tEl->Len,1);
					tEl=(El_Unit*)UM->UnitList->Next(FSL8);
				}
			}
			else
			{	// Alternate
				tEl=(El_Unit*)UM->FindUnit(pIDI->AdrInfo->DestinAI.UnitName,pIDI->AdrInfo->DestinAI.DomainName,pIDI->AdrInfo->DestinAI.HostName,pIDI->AdrInfo->DestinAI.UnitMode);
				if (tEl)
				{
					if (mStrCmpHost(ShortHostName,tEl->UInf.Host)==0)
					{
						if ((pIDI->StateBlock&(StateBlock_BEGIN|StateBlock_END))!=(StateBlock_BEGIN|StateBlock_END))
						{pIDI->AdrInfo->DestinAI.UnitID=tEl->UInf.PersID;}
						tEl->UInf.pObj->UserCallBackIDIEx(0,pIDI,NULL,1);
					}
					else
					{
						rc2|=0x400000;
					}
				}
			}
			//					MyUnLock(__FILE__,__LINE__,&UM->Fl_RE,this);
		}
		else
		{
			rc2|=0x200000;
		}
	}
	/*if (rc==0)
	{
		if(pIDI->ID_ParentFile)
			if (pIDI->TypeIDI==TypeIDI_ANSWER)
				if(strcmp(pUnitName,N_UnitMainDB)==0)
					goto _sss0;
	} */
	return rc+rc2;
};

int	Services::ReplElTabl(El_Tab* tElT,int Mode)
{
	int rc=0;
	if ((Fl_Repl==1)&&(Fl_Prm==1)&&(NetCommLAN))
	{
//		void*	pVol;
//		size_t	pCnt;
//		MyLock(__FILE__,__LINE__,&Fl_RE4,this);
//		void*	Prm;
		IDInfo*	pIDI=tElT->GetIDI();
		if (pIDI->AdrInfo)
		{
			pIDI->StateBlock=StateBlock_BEGIN|StateBlock_END;
			pIDI->NumPack=1;
// 			if (pIDI->pPList->GetParam(IDHostName,Prm)==-1)
// 				pIDI->pPList->SetParam(IDHostName,"*",2,this);
//			pIDI->pPList->SetParam(IDUnitName,N_Service,9,this);
			switch(Mode)
			{
			case 0:
				strcpy(pIDI->AdrInfo->DestinAI.UnitName,N_Service);
				pIDI->AdrInfo->NetType=NT_ANY;
				break;
			case 1:
				strcpy(pIDI->AdrInfo->DestinAI.UnitName,N_DomainController);
				pIDI->AdrInfo->NetType=NT_ANY;
				break;
			}

			// Adress From
//			pIDI->pPList->SetParam(IDMyUnitName,this->Name_Mt,1+strlen(this->Name_Mt),this);
			if (pIDI->AdrInfo->NativeAI.HostName[0]==0)
				strcpy(pIDI->AdrInfo->NativeAI.HostName,ShortHostName);
			strcpy(pIDI->AdrInfo->SourceAI.HostName,ShortHostName);
			if (pIDI->AdrInfo->NativeAI.UnitName[0]==0)
				strcpy(pIDI->AdrInfo->NativeAI.UnitName,this->Name_Mt);
			strcpy(pIDI->AdrInfo->SourceAI.UnitName,this->Name_Mt);
			if (pIDI->AdrInfo->NativeAI.DomainName[0]==0)
				strcpy(pIDI->AdrInfo->NativeAI.DomainName,this->UInfo.DomainName);
			strcpy(pIDI->AdrInfo->SourceAI.DomainName,this->UInfo.DomainName);

			//		SendMt=this;
			pIDI->pPList->SetParam(IDLocal,&Fl_Repl,sizeof(int),this);
			if (NetCommLAN)	
				rc=NetCommLAN->DataIDIEx(0,pIDI);	// ,1!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
			pIDI->pPList->DelParam(IDLocal);
			//		if (pIDI->pPList->GetParam(IDLocal,pVol,pCnt,ParentMt)!=-1)
			//		{
			//			pIDI->pPList->DelParam(IDLocal);
			//		}
			//			DataIDI(3,pIDI);
			if((pIDI->AdrInfo->NetType==NT_ANY)||(pIDI->AdrInfo->NetType==NT_WAN))
			{
				uchar*	tBf=NULL;
				size_t	tCn;
				if((Cli1)||(Cli2)||(WANSISrv))
				{
					//				pIDI->pPList->SetParam(IDHostName,"*",2,this);
					tBf=IDICnv->IDInfo2Buf(pIDI,tCn);
				}
				if((Cli1)&&(tBf))
				{
					SIPack(Cli1,SYS_IDInfo,tBf,tCn);
				}
				if((Cli2)&&(tBf))
				{
					SIPack(Cli2,SYS_IDInfo,tBf,tCn);
				}
				if((WANSISrv)&&(tBf))
				{
					SIPack(WANSISrv,SYS_IDInfo,tBf,tCn);
				}
			}
		}
//		MyUnLock(__FILE__,__LINE__,&Fl_RE4,this);
	}
	return rc;
}

int	Services::SendElTabl(El_Tab* tElT)
{
	int rc=0;
	El_Owner*	tElO;
	if (tElT)
	{
		if ((tElT->GetIDI()->ID_SeansFrom<TabParametr)/*&&(IDT_Neuron)*/)
		{
			if ((tElT->OwnerList)&&(tElT->OwnerList->NumElem))
			{
				FindStrList8	FSL8;
				tElO=(El_Owner*)tElT->OwnerList->Begin(FSL8);
				while (tElO)
				{
					if ((IDT_NeuronGlobal!=tElO->Own.pDec)&&(IDT_NeuronLocalCustomer!=tElO->Own.pDec)&&(IDT_NeuronLocalSite!=tElO->Own.pDec))
					{

						if (TestPointer(tElO->Own.pDec))
//						if ((!IsBadReadPtr((void*)tElO->Own.pDec,sizeof(Decoder)))&&(!IsBadReadPtr((void*)tElO->Own.pDec->Name_Mt,40))&&(strcmp(tElO->NameMt,tElO->Own.pDec->Name_Mt)==NULL))
						{
							int	Fl_Buf=0;
//							if ((tElT->GetIDI()->Command==Command_DEL)||( tElT->GetIDI()->Command==Command_MOVE))
//								Fl_Buf=2;
							tElO->Own.pDec->UserCallBackIDIEx(1, tElT->GetIDI(), &tElO->Own,Fl_Buf);
						}
						else
						{
							char	tcBuf[256];
							sprintf(tcBuf,"Err UserCall=%s",tElO->NameMt);
							OutLog(tcBuf,this,"ErrUserCallBack");
							tElT->DelChild(tElT->GetIDI()->ID_File);
						}
					}
					tElO=(El_Owner*)tElT->OwnerList->Next(FSL8);
				};
			};
		}
 	}
	return rc;
}

bool	Services::TestPointer(PVOID	pObj)
{
	return (ActiveDecoderList->Find((UCHAR*)&pObj,sizeof(LPVOID))!=0);
};

int	Services::DataFF(int N_Pin,UCHAR* pmas,size_t Cnt)
{
	int rc=Cnt;
//	char	tBf[256];
	IDInfo*		tIDI=(IDInfo*)pmas;
//	IDInfo*		tIDITr;
	if (Fl_NetLock==0) return rc;
//	MyLock(__FILE__,__LINE__,&Fl_RE6,this);
	strcpy(UInfo.State,"DataFF");
	if (N_Pin==0)
		N_Pin=4;
	switch(N_Pin)
	{
	case 0:	//	"ThIDView","Âõîä äëÿ îòîáðàæåíèÿ íà ThIDView",
		if (ThIDView)
			if (Cnt==sizeof(IDInfo))
				DataIDI(N_Pin,tIDI);
		break;
	case 1:	//	"IDI_FSrv","Âõîä äëÿ çàïèñè èëè ôîðìèðîâàíèÿ çàïðîñîâ ê Ôàéëîâîìó ñåðâåðó",
		if (FSrv)
			if (Cnt==sizeof(IDInfo))
				DataIDI(N_Pin,tIDI);
		break;
	case 2:
//		if(BDBIDInfo)
			if (Cnt==sizeof(IDInfo))
				DataIDI(N_Pin,tIDI);
		break;
	case 3:		// Íà ÍåòÊîì ðàçáîð ïàêåòîâ ñâîèì - áóôåðèçèðîâàííûå äðóãèì â íåò êîì
		if(NetCommLAN)
			if (Cnt==sizeof(IDInfo))
				DataIDI(N_Pin,tIDI);
		break;
	case 4:
		break;
	case 1000:	// Îò FSrv 
		if (Cnt==sizeof(IDInfo))
		{
			DataIDI(N_Pin,tIDI);
//			void*		pVol;
//			int			pCnt;
//			Decoder*	ParentMt=NULL;
//			El_TabList*	ElTL;
//			tIDITr=(IDInfo*)pmas;
//			if (Fl_Stop==0) ODataFF(1001,pmas,Cnt);
		}
		break;
	case 1001:	// Îò ThIDView
//		if (Fl_Stop==0)
//		{
//			MyUnLock(__FILE__,__LINE__,&Fl_RE6,this);
//			ODataFF(1000,pmas,Cnt);
//			MyLock(__FILE__,__LINE__,&Fl_RE6,this);
//		}
		break;
	case 1002:	// Îò BDBInfo
		if (Cnt==sizeof(IDInfo))
		{
			DataIDI(N_Pin,tIDI);
//			if (Fl_Stop==0)
//			{
//				MyUnLock(__FILE__,__LINE__,&Fl_RE6,this);
//				ODataFF(1002,pmas,Cnt);
//				MyLock(__FILE__,__LINE__,&Fl_RE6,this);
//			}
		}
		break;
	case 1003:	// From NetCommLAN
			if (Fl_Stop==0)
			{
				DataIDI(N_Pin,tIDI);
			}
		break;
 	case 1004:	// Îò NetCommWAN	!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
 			if (Fl_Stop==0)
 			{
				DataIDI(N_Pin,tIDI);
 			}
 		break;
	case 1005:	// From NetServices
		{
//			Fl_DD=1;

			//				ncc++;
			//				GetRealTime(&tFT2.FTime);
			//				T1+=tFT2.Time-tFT.Time;
			pPackNetObj	pack;
//			UCHAR*		pmas;
//			rpt|=1;
//			pack=(pPackNetObj)tBuf;
			pack=(pPackNetObj)pmas;
			//				pmas=pack->buf;
//			Cnt=pack->cnt;
// 			if (SizeNetMen<Cnt)
// 			{
// 				uchar*	tNetMenBuf;
// 				SizeNetMen=Cnt;
// 				tNetMenBuf=(uchar*)realloc(NetMenBuf,SizeNetMen);
// 				if (tNetMenBuf)
// 				{
// 					NetMenBuf=tNetMenBuf;
// 				}
// 				else
// 				{
// 
// 				};
// 			}
// 			if (NetMenBuf)
// 			{
// 				memcpy(NetMenBuf,pack->buf,pack->cnt);
// 			}
//			pmas=NetMenBuf;
// 			if(!strcmp(pack->descriptor,"_SysInfo"))
// 			{
// 			}
// 			else
			{	// Clients
//				MyLock(__FILE__,__LINE__,&Fl_RE9,this);
				int		NumCli=atoi(&pack->descriptor[5]);
				if ((CliList[NumCli].Fl!=0)&&(CliList[NumCli].Fl_Data!=0)&&(CliList[NumCli].PObj))
				{
					if (CliList[NumCli].Mode==0)
					{
						switch (pack->pin)
						{
						case 0:
							break;
						case 1:
							switch (*(int*)pmas)
							{
							case 0:	// Connect
								CliList[NumCli].FlCon=1;
								break;
							case 1:	// DisConnect
								CliList[NumCli].FlCon=0;
								break;
							}
							break;
						}
					}
					tObj=CliList[NumCli].PObj;
					GetRealTime(&tTm.FTime);
					CliList[NumCli].PObj->UserCallBackEx(NumCli,10,pmas,Cnt,2);
					GetRealTime(&tTm2.FTime);
					if (tTm2.Time-tTm.Time>4*KtimeS)
					{
						char	tBf[256];tBf[0]=0;
						sprintf(tBf,"Mt=%s UCB Time=%d mS",tObj->Name_Mt,(int)((tTm2.Time-tTm.Time)/Ktime));
						CE1=SetCriticalError(this,tBf,"Warning","UCB",CE1);
					}
					tObj=NULL;
				}
//				MyUnLock(__FILE__,__LINE__,&Fl_RE9,this);
			}
		}
		break;
	case 2000:	// To FileSrv
		if (Cnt==sizeof(IDInfo))	// Save
		{ 
			DataIDI(N_Pin,tIDI);
		}
		break;
	case 2001:	// Îò Table			// TC
		if (FSrv)
		{
//			MyUnLock(__FILE__,__LINE__,&Fl_RE6,this);
			FSrv->DataFFEx(1,NULL,1);
//			MyLock(__FILE__,__LINE__,&Fl_RE6,this);
		}
//   	if (BDBIDInfo)
// 			BDBIDInfo->DataFF(1,NULL,1);
		break;
	case 2002:
		if (Cnt==sizeof(IDInfo))
		{
			DataIDI(N_Pin,tIDI);
		};
		break;
	case 2003:	// To NetCom	// Repl
		if (Cnt==sizeof(IDInfo))
		{
			DataIDI(N_Pin,tIDI);
		}
		break;
	case 2005:
//		if (Cnt==sizeof(IDInfo))
//		{
//			if (FSrv)
//				FSrv->DataFF(0,pmas,Cnt);
//		}
		break;
/*
	case 2006:	// Âûõîä ñ XML2IDI
		if ((Cnt==sizeof(IDInfo))&&(Fl_Repl==1)&&(Fl_Prm==1))
		{
			void*		pVol;
			int			pCnt;
			Decoder*	ParentMt=NULL;
			El_TabList*	ElTL;
			tIDITr=(IDInfo*)pmas;
			if (tIDITr->pPList->GetParam(IDIDFileSnsFr,pVol,pCnt,ParentMt)!=-1)
			{
				ElTL=(El_TabList*)TableList->Find((UCHAR*)pVol,pCnt);
				if (ElTL) ElTL->IDT->DataFF(2,pmas,Cnt);
			}
			else
			{
				ElTL=(El_TabList*)TableList->Find((UCHAR*)&tIDITr->ID_File,sizeof(INT64));
				if (ElTL==NULL) ElTL=(El_TabList*)TableList->Find((UCHAR*)&tIDITr->ID_SeansFrom,sizeof(INT64));
				if (ElTL)
				{
					ElTL->IDT->DataFF(2,pmas,Cnt);
				}
				else
				{ // Íåò òàêîé òàáëèöû !!!
					if (tIDITr->ID_SeansFrom)
					{
						IDTable*	tIDT=OpenTable(this,tIDITr->ID_SeansFrom);
						tIDT->DataFF(2,pmas,Cnt);
						CloseTable(this,tIDT);
					}
				}
			}
		}
		break;
*/
	case 2007:
		{
			char	ttBf[128];
			int		ii,ij;
			ii=2;
			ij=0;
			if (Cnt<128)
			{
				while(pmas[ii]!='D')
				{
					ttBf[ij++]=pmas[ii++];	
					if (ii>=Cnt)
						break;
				}
				ttBf[ij]=0;
				OutLog((char*)ttBf,this);
			}
		}
		break;
	default:
		;
	};
	UInfo.State[0]=0;
//	MyUnLock(__FILE__,__LINE__,&Fl_RE6,this);
	return rc;
}

void		Services::Pack2Table(IDInfo* IDI)
{
//	if (IDI->CustomerID==M_Cfg->MData.CustomerID)
	{
		if ((Fl_Repl==1)&&(Fl_Prm==1))
		{
			void*		pVol;
			size_t		pCnt;
			Decoder*	ParentMt=NULL;
			El_TabList*	ElTL;
			MyLock(__FILE__,__LINE__,&Fl_RE12,this);
			if (IDI->pPList->GetParam(IDIDFileSnsFr,pVol,pCnt,ParentMt)!=-1)
			{
				ElTL=(El_TabList*)TableList->Find((UCHAR*)pVol,pCnt);
				if (ElTL)
					ElTL->IDT->DataIDIEx(2,IDI);
			}
			else
			{
				ElTL=(El_TabList*)TableList->Find((UCHAR*)&IDI->ID_File,sizeof(INT64));
				if (ElTL==NULL)
					ElTL=(El_TabList*)TableList->Find((UCHAR*)&IDI->ID_SeansFrom,sizeof(INT64));
				if (ElTL)
				{
					ElTL->IDT->DataIDIEx(2,IDI);
				}
				else
				{ // Íåò òàêîé òàáëèöû !!!
					if (IDI->ID_SeansFrom)
					{
						//					IDTable*	tIDT=OpenTable(this,IDI->ID_SeansFrom);
						//					tIDT->DataFF(2,(UCHAR*)IDI,sizeof(IDInfo));
						//					CloseTable(this,tIDT);			// !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
					}
					else
					{

					}
				}
			}
			MyUnLock(__FILE__,__LINE__,&Fl_RE12,this);
		}
	}
};

INT64	AddL(INT64 Vol)
{
	Vol^=Vol>>32;
	Vol^=Vol>>16;
	Vol^=Vol>>8;
	Vol^=Vol>>4;
	Vol^=Vol>>2;
	if((Vol^Vol>>1)&1)
		return 0x8000000000000000;
	else
		return 0;
};

INT64	Services::Randomize()
{
	for(char j=0;j<8;j++)
	{
		INT64 tt=AddL(RndVol&0x8000001000001);
		RndVol=(RndVol>>1)|tt;
	};
	return	RndVol+(RealTime<<32);
};

void*	Services::MallocHuge(size_t size)
{
	size_t i,j;
	void *TB=NULL;
	for (i=0;i<SizeMC;i++)
	{
		if (FreeMC[i].Fl)
		{
			if (FreeMC[i].Cnt==size)
			{
				TB=FreeMC[i].Buf;
				for (j=0;j<SizeMC;j++)
				{
					if (AllocMC[j].Fl==0)
					{
						AllocMC[j].Fl=1;FreeMC[i].Fl=0;	
						AllocMC[j].Buf=FreeMC[i].Buf;FreeMC[i].Buf=NULL;
						AllocMC[j].Cnt=FreeMC[i].Cnt;FreeMC[i].Cnt=0;
						break;
					}
				}
				break;
			}
		}
	}
	if (TB==NULL)
	{
		for (j=0;j<SizeMC;j++)
		{
			if (AllocMC[j].Fl==0)
			{
				AllocMC[j].Fl=1;	
				AllocMC[j].Buf=malloc(size);
				AllocMC[j].Cnt=size;
				if (AllocMC[j].Buf)
				{
					TB=AllocMC[j].Buf;MemClasterCnt++;
					Telemetr(this,"MemClasterCnt",MemClasterCnt,NULL);
				}
				else
					AllocMC[j].Fl=0;
				break;
			}
		}
	};
	return TB;
};
void	Services::	FreeHuge(void* tB)
{
	size_t i,j;
	for (j=0;j<SizeMC;j++)
	{
		if (AllocMC[j].Fl)
		{
			if (AllocMC[j].Buf==tB)
			{
				for (i=0;i<SizeMC;i++)
				{
					if (FreeMC[i].Fl==0)
					{
						FreeMC[i].Fl=1;AllocMC[j].Fl=0;	
						FreeMC[i].Buf=AllocMC[j].Buf;AllocMC[j].Buf=NULL;
						FreeMC[i].Cnt=AllocMC[j].Cnt;AllocMC[j].Cnt=0;
						break;
					}
				}
			}
		}
	}
};

/*
UINT  CalcPolynom(char* Vol)
{
int		PoiVolIn=0;
int		TempVol;
char	VolOut[16];
int		PoiVolOut=0;
UINT	Polynom=0;
while(Vol[PoiVolIn])
{
if((Vol[PoiVolIn]<0x30)||(Vol[PoiVolIn]>0x39))
{	//Âñòðå÷åí ðàçäåëèòåëüíûé ñèìâîë
while((Vol[PoiVolIn]!=0)&&((Vol[PoiVolIn]<0x30)||(Vol[PoiVolIn]>0x39)))
{
PoiVolIn++;
}
VolOut[PoiVolOut]=0;
TempVol=atoi(VolOut);
VolOut[0]=PoiVolOut=0;
if((TempVol>=0)&&(TempVol<=31))	
{
Polynom|=1<<(31-TempVol);
}
}
else
{	//×èñëî
VolOut[PoiVolOut++]=Vol[PoiVolIn++];
};
};
VolOut[PoiVolOut]=0;
TempVol=atoi(VolOut);
VolOut[0]=PoiVolOut=0;
if((TempVol>=0)&&(TempVol<=31))	
{
Polynom|=1<<(31-TempVol);
}
return Polynom;
};

  UINT	AddL(UINT Vol)
  {
  Vol^=Vol>>16;
  Vol^=Vol>>8;
  Vol^=Vol>>4;
  Vol^=Vol>>2;
  if((Vol^Vol>>1)&1)
		return 0x80000000;
		else
		return 0;
		};
		
		  void SetScremTabl(UINT _Polynom,UINT _InitReg)
		  {	
		  
			UINT	RRegOut=0;
			UINT	RReg=_InitReg;
			UINT	Polynom=_Polynom;
			
			  for(UINT i=0; i<100; i++)
			  {
			  SCREMB_TABL[i]=0;
			  for(char j=0;j<8;j++)
			  {
			  UINT tt=AddL(RReg&Polynom);
			  RReg=(RReg>>1)|tt;
			  RRegOut=(RRegOut>>1)|tt;
			  // 			if((PeriodInit)&&(++ReriodInitCnt>=PeriodInit))
			  // 			{
			  // 				ReriodInitCnt=0;
			  // 				RReg=InitSeq;
			  // 			}
			  };
			  SCREMB_TABL[i]=((UCHAR)(RRegOut>>24));
			  }
}
 */


INT64	Services::GetUID()
{
//	INT64	rID;
	El_Tab		*ETbl=NULL,*ETbl2=NULL;
	NumClk	rID;
	MyLock(__FILE__,__LINE__,&Fl_RE13,this);
	rID.NClk=(++Cfg.UID);
	rID.DWNClk.hi=(rID.DWNClk.hi&0x0000ffff)|((*(ulong*)&IPAddrLAN)&0xffff0000);
	if (Cfg.UID>=Cfg.PID)
	{
		Cfg.PID=Cfg.UID+1000;
		SaveCfg();
		GetRealTime(&tTm.FTime);
		if ((tTm.Time-TGID.Time>(INT64)10*KtimeM)/*&&(IDT_Neuron)*/)
		{
			TGID.Time=tTm.Time;
			char	tB[128];
			sprintf(tB,"UID=%I64x ",Cfg.UID);
			OutLog(tB,this,"UID");
 			MyUnLock(__FILE__,__LINE__,&Fl_RE13,this);//Fl_MTSID=0;
//  			ETbl=IDT_Neuron->FindNested(&Anchor,AcsMd_Read,N_Domain,MainDomainName,N_Host,ShortHostName,NULL);
// 			if (ETbl)
// 			{
//				INT64*	PVol;
// 				if (IDT_Neuron->GetParam(&Anchor,ETbl,IDTopUID,PVol)==-1)
// 				{
// 					IDT_Neuron->AddParam(&Anchor,ETbl,IDTopUID,Cfg.PID);
// 				}
// 				else
// 				{
// 					MyLock(__FILE__,__LINE__,&Fl_RE13,this);
// 					if (Cfg.PID<*PVol)
//					{
// 						Cfg.PID=*PVol+1000;
// 						Cfg.UID=*PVol;
// 						SaveCfgID();
// 					}
// 					MyUnLock(__FILE__,__LINE__,&Fl_RE13,this);//Fl_MTSID=0;
// 					IDT_Neuron->AddParam(&Anchor,ETbl,IDTopUID,Cfg.PID,1);
// 				}
// 			}
// 			IDT_Neuron->UnLockEl(&Anchor,ETbl);
 		}
 		else
			MyUnLock(__FILE__,__LINE__,&Fl_RE13,this);//Fl_MTSID=0;
	}
	else
	{
		MyUnLock(__FILE__,__LINE__,&Fl_RE13,this);//Fl_MTSID=0;
	}
	return	rID.NClk;
};
 
Task*	Services::GetTask()
{
	return Tsk;
}
void	Services::ReplCfgOn()
{
	Fl_Repl=1;
};

void	Services::ReplCfgOff()
{
	Fl_Repl=0;
};

void	Services::DelMt4Tlm(Decoder* PDec)
{
	El_Telem	*ElT;
//	MyLock(__FILE__,__LINE__,&Fl_RE3,this);
	FindStrList8	FSL8;
	ElT=(El_Telem*)TraceLst->Begin(FSL8);
	while(ElT!=NULL)
	{
		if (ElT->TD.PObj==PDec)
		{
			TraceLst->Del(ElT);
		}
		ElT=(El_Telem*)TraceLst->Next(FSL8);
	}
//	MyUnLock(__FILE__,__LINE__,&Fl_RE3,this);
};

void	Services::Telemetr(Decoder* PDec,char* NamePrm,double Vol,PVOID pOO)
{
	TlmData	TlmD;
	if (PDec)
		TlmD.Set(PDec,NamePrm,pOO);
	else
		TlmD.Set(this,NamePrm,pOO);
	Telemetr(&TlmD,Vol);
};
void	Services::Telemetr(TlmData *NamePrm,double Vol)
{
	El_Telem	*ElT;
//	MyLock(__FILE__,__LINE__,&Fl_RE3,this);
	ElT=(El_Telem*)TraceLst->Find((UCHAR*)NamePrm,sizeof(PVOID)+sizeof(Decoder*)+strlen(NamePrm->Prm)+1);
	if (ElT==NULL)
	{
		ElT=(El_Telem*)TraceLst->NewElem();
		if (ElT==NULL)
		{
			ElT=new El_Telem(NamePrm);
		}
		else
		{
			ElT->Init(NamePrm);
		}
		if (TraceLst->Add(ElT)==-1)
		{
			delete	ElT;	ElT=NULL;
		};
	}
	if (ElT)
		ElT->SetVol(Vol);
//	MyUnLock(__FILE__,__LINE__,&Fl_RE3,NULL);//Fl_MT=0;
};

void	Services::GetRealTime(FILETIME* RTime) 
{
	INT64	CntT_3=GetNClk();
//	INT64	tRTime=RealTime+(INT64)((10000000.0*(CntT_3-NClickOld))/Fcpu);//-(INT64)SmTime;
	INT64	tRTime=RealTime+(INT64)((CntT_3-NClickOld)*Fcpu_1);//-(INT64)SmTime;
	*(INT64*)RTime=tRTime;
};

void	Services::AddDebugLevel(Decoder* pDec,char* tBuf)
{
// 	MyLock(__FILE__,__LINE__,&Fl_RE,this);
// 	DbgBList->AddElem((UCHAR*)tBuf,strlen(tBuf)+1);
// 	int	rc=0;
// 	UCHAR		*tBDL;
// 	int			tCnt;
// 	BandleFind	tBF;
// 	DbgLine[0]=0;
// 	rc=DbgBList->FindFirst(tBDL,tCnt,&tBF);
// 	while (rc>0)
// 	{
// 		if (strlen(DbgLine)+strlen((char*)tBDL)<1023)
// 			strcat(DbgLine,(char*)tBDL);
// 		rc=DbgBList->FindNext(tBDL,tCnt,&tBF);
// 	}
// 	MyUnLock(__FILE__,__LINE__,&Fl_RE,this);
};

void	Services::SubDebugLevel(Decoder* pDec)
{
// 	MyLock(__FILE__,__LINE__,&Fl_RE,this);
// 	UCHAR		*tBDL;
// 	int			tCnt;
// 	DbgBList->DelLast(tBDL,tCnt);
// 	int	rc=0;
// 	BandleFind	tBF;
// 	DbgLine[0]=0;
// 	rc=DbgBList->FindFirst(tBDL,tCnt,&tBF);
// 	while (rc>0)
// 	{
// 		if (strlen(DbgLine)+strlen((char*)tBDL)<1023)
// 			strcat(DbgLine,(char*)tBDL);
// 		rc=DbgBList->FindNext(tBDL,tCnt,&tBF);
// 	}
// 	MyUnLock(__FILE__,__LINE__,&Fl_RE,this);
};

char*	Services::GetDebugLine(Decoder* pDec)
{
	return DbgLine;
};

FileTime	Services::GetDeltaTime(INT64 NClk)
{
	FileTime	FT;
//	FT.Time=INT64((10000000*NClk)/Fcpu);
	FT.Time=INT64(NClk*Fcpu_1);
	return FT;
};

Decoder*	Services::NewDecod(char* Name,Decoder* pDec)
{
	Decoder*	tDec;
//	MyLock(__FILE__,__LINE__,&Fl_RE8,this);
	if (DLLL!=NULL)
	{
		int rc=-1;
		ID_ElemPTR*	ElPTR;
		char	tcBuf[256];
		tDec=DLLL->NewDecod(Name,pDec);
		if (tDec)
		{
			ElPTR=(ID_ElemPTR*)ActiveDecoderList->NewElem();
			if (ElPTR)
			{
				ElPTR->Init(tDec);
			}
			else
				ElPTR=new	ID_ElemPTR(tDec);
			rc=ActiveDecoderList->Add(ElPTR);
			if (rc==-1)
			{
				sprintf(tcBuf,"Error Adding Decod=%s Ptr=%I64x",Name,(INT64)tDec);
				OutLog(tcBuf,this,"NewDec");
				SetCriticalError(this,"Error decoder creating","Error","Neuron");
			}
			else
			{
				sprintf(tcBuf,"New Decod=%s Ptr=%I64x",Name,(INT64)tDec);
				OutLog(tcBuf,this,"NewDec");
			}
			if (Timer(CntNumDec,1000))
				Telemetr(this,"AllDecoderNumber",(double)ActiveDecoderList->NumElem);
		}
		else
		{
			sprintf(tcBuf,"Error Creating Decod=%s",Name);
			OutLog(tcBuf,this,"NewDec");
			SetCriticalError(this,tcBuf,"Error","Neuron");
//			tDec=DLLL->NewDecod("NullMt",pDec);
		}

	}
	else
		tDec=NULL;
//	MyUnLock(__FILE__,__LINE__,&Fl_RE8,this);
	return tDec;
};

void		Services::DeleteDecoder(Decoder* pDec)
{
//	MyLock(__FILE__,__LINE__,&Fl_RE8,this);
	char	tcBuf[256];
	ID_Elem*	ElPTR=ActiveDecoderList->Find((UCHAR*)&pDec,sizeof(LPVOID));
	if (ElPTR)
	{
		sprintf(tcBuf,"Del Decod=%s Ptr=%I64x",pDec->Name_Mt,(INT64)pDec);
		OutLog(tcBuf,this,"NewDec");
		ActiveDecoderList->Del(ElPTR);
	}
	else
	{
		sprintf(tcBuf,"Error Del Decod=%s Ptr=%I64x",pDec->Name_Mt,(INT64)pDec);
		OutLog(tcBuf,this,"NewDec");
	}
	if (Timer(CntNumDec,1000))
		Telemetr(this,"AllDecoderNumber",(double)ActiveDecoderList->NumElem);
	DLLL->DeleteDecoder(pDec);
//	MyUnLock(__FILE__,__LINE__,&Fl_RE8,this);
};

UnitInfo*	Services::GetActiveDC(char* DomName)
{
	El_Unit* tEl=NULL;
	UnitInfo	*tUI=NULL;
	if (DomName)
		tEl=UM->FindUnit(N_UnitDomainCtrl,DomName,NULL,N_Master);
//	else
//		tEl=UM->FindUnit(N_UnitDomainCtrl,DomainName,N_Master);
	if (tEl)
		tUI=&tEl->UInf;
	return tUI;
};


LicUnitInfo*	Services::TestMaxNumRole(char* RoleName)
{
//	int	RoleNum=-1;

	if (strcmp(RoleName,N_DomainController)==0)
	{
		tLUInfo.NumLic=NetLCond.CntRole_DomCtr;
		tLUInfo.NumReal=UM->CalcUnitNumber(N_UnitDomainCtrl,N_Master);
	}
	else
	{
		if (strcmp(RoleName,N_FileServer)==0)
		{
			tLUInfo.NumLic=NetLCond.CntRole_Fsrv;
			tLUInfo.NumReal=UM->CalcUnitNumber(N_UnitFileSrv,N_Storage);
		}
		else
		{
			if (strcmp(RoleName,N_DataBase)==0)
			{
				tLUInfo.NumLic=NetLCond.CntRole_DBas;
				tLUInfo.NumReal=UM->CalcUnitNumber(N_UnitDB);
			}
			else
			{
				if (strcmp(RoleName,N_MainDataBase)==0)
				{
					tLUInfo.NumLic=NetLCond.CntRole_MDBas;
					tLUInfo.NumReal=UM->CalcUnitNumber(N_UnitMainDB,N_Server);
				}
				else
				{
					if (strcmp(RoleName,N_ProcServer)==0)
					{
						tLUInfo.NumLic=NetLCond.CntRole_SPrc;
						tLUInfo.NumReal=UM->CalcUnitNumber(N_ProcessingServer,N_Master);
					}
					else
					{
						if (strcmp(RoleName,N_OperatorWorkstation)==0)
						{
							tLUInfo.NumLic=NetLCond.CntRole_WSOp;
							tLUInfo.NumReal=UM->CalcUnitNumber(N_OperWS,"Start");
						}
						else
						{
							if (strcmp(RoleName,N_AdministratorWorkstation)==0)
							{
								tLUInfo.NumLic=NetLCond.CntRole_WSAd;
								tLUInfo.NumReal=UM->CalcUnitNumber(N_WSACfg,"Start");
							}
							else
							{
								if (strcmp(RoleName,N_AnalyticWorkstation)==0)
								{
									tLUInfo.NumLic=NetLCond.CntRole_WSAnalitic;
									tLUInfo.NumReal=UM->CalcUnitNumber(N_Analytic,"Start");
								}
								else
								{
									if (strcmp(RoleName,N_SignalingSystemServer)==0)
									{
										tLUInfo.NumLic=NetLCond.CntRole_SSSrv;
										tLUInfo.NumReal=UM->CalcUnitNumber(N_UnitTlfSrv,N_Server);
									}
									else
									{
										if (strcmp(RoleName,N_Server)==0)
										{
											tLUInfo.NumLic=10000;
											tLUInfo.NumReal=UM->CalcUnitNumber(N_Service,N_Server);
										}
										else
										{
											if (strcmp(RoleName,N_TelemetrViewer)==0)
											{
												tLUInfo.NumLic=NetLCond.CntRole_WSTelemetry;
												tLUInfo.NumReal=UM->CalcUnitNumber(N_TelemetrViewer,"Start");
											}
											else
											{
												if(strcmp(RoleName,N_PostProcServer)==0)
												{
													tLUInfo.NumLic=NetLCond.CntRole_PPSrv;
													tLUInfo.NumReal=UM->CalcUnitNumber(N_UnitPostProcSrv);
//													tLUInfo.NumLic=-1;
//													tLUInfo.NumReal=0;
												}
												else
												{
													tLUInfo.NumLic=-1;
												}
											}
										}
									}
								}
							}
						}
					}
				}
			}
		}
	}
	if (tLUInfo.NumLic==-1) tLUInfo.NumLic=32768;
	return &tLUInfo;
};

int		Services::RoleSwOn(char* RoleName)
{
	int rc=-1;
	El_Tab*	ElTbl=NULL;
	El_Tab*	ElTbl2=NULL;
//	char	tB[128];
//	void*	tVol;
	if(IDT_NeuronLocalSite)
	{
//		MyLock(__FILE__,__LINE__,&Fl_RE,this);
		ElTbl=IDT_NeuronLocalSite->FindNested(&Anchor,AcsMd_Read,N_Domain,UInfo.DomainName,N_Role,RoleName,NULL);
		if(ElTbl==NULL)
			ElTbl=IDT_NeuronLocalSite->AttainNested(&Anchor,"%s%s%s",N_Domain,UInfo.DomainName,N_Role,RoleName,-1);
		if(ElTbl)
		{
			LicUnitInfo* MaxNum=TestMaxNumRole(RoleName);
			if (MaxNum->NumLic<=MaxNum->NumReal)
			{
				rc=1;
			}
			else
			{
				rc=0;
			}
			IDT_NeuronLocalSite->UnLockEl(&Anchor,ElTbl);
		}
//		MyUnLock(__FILE__,__LINE__,&Fl_RE,this);
	}
	return rc;
};

int		Services::RoleSwOff(char* RoleName)
{
	int rc=-1;
	El_Tab*	ElTbl=NULL;
	El_Tab*	ElTbl2=NULL;
//	char	tB[128];
	if (IDT_NeuronLocalSite)
	{
//		MyLock(__FILE__,__LINE__,&Fl_RE,this);
		ElTbl=IDT_NeuronLocalSite->FindNested(&Anchor,AcsMd_Read,N_Domain,UInfo.DomainName,N_Role,RoleName,NULL);
		if(ElTbl==NULL)
			ElTbl=IDT_NeuronLocalSite->AttainNested(&Anchor,"%s%s%s",N_Domain,UInfo.DomainName,N_Role,RoleName,-1);
		if(ElTbl)
		{
			ElTbl2=IDT_NeuronLocalSite->FindNestedStr(&Anchor,AcsMd_Read,ShortHostName,ElTbl);
			if (ElTbl2)
			{
				IDT_NeuronLocalSite->DelElTab(&Anchor,ElTbl2);
				rc=0;
			}
			IDT_NeuronLocalSite->UnLockEl(&Anchor,ElTbl);
		}
//		MyUnLock(__FILE__,__LINE__,&Fl_RE,this);
	}
	return rc;
};

void	Services::ExtSynTime(FileTime FTime,int Fl)
{
//	return;
//	char			tBuf[256];
//	INT64		Err;
//	double		VarErr,VardifErr;
//	SYSTEMTIME	tSTime;
	TimeLES.Time=RealTime;
	Fl_ExtSin=1;
	CntT2=GetNClk();
//	RealTime+=(INT64)((10000000.0*(double)(CntT2-NClickOld))/Fcpu);
	RealTime+=(INT64)((CntT2-NClickOld)*Fcpu_1);
	NClickOld=CntT2;
	tErr=(double)(RealTime-FTime.Time);
	Err[PoiNTF]=tErr;
	if (++PoiNTF>=NumTF) PoiNTF=0;
	VarErr=0.0001*MedFlt(Err,NumTF);// ms
	if (Fl_FSyn==2)
	{
		if (VarErr>0)
		{
			RealTime-=(INT64)(100*VarErr);
			if (VarErr>200)
			{
				MVarErr=MVarErr*0.99+(VarErr-19)*(VarErr-19);
				if (VarErr>1000)
				{
					if (MVarErr>10000) MVarErr=10000;
					RealTime-=((RealTime-FTime.Time)/10);
				}
				else
				{
					if (MVarErr>1000) MVarErr=1000;
					RealTime-=((RealTime-FTime.Time)/20);
				}
				if (MVarErr<0)
					MVarErr=0;
				if (0.0001*Min(Err,NumTF)>200)
					Fcpu+=1*MVarErr;
				else
					Fcpu+=0.1*MVarErr;
			}
			else
			{
				MVarErr=0;
				if (VarErr>20)
					Fcpu+=0.1*VarErr*VarErr;
				else
					Fcpu+=0.01*VarErr*VarErr;
			}
		}
		else
		{
			RealTime-=(INT64)(100*VarErr);
			if (VarErr<-200)
			{
				MVarErr=MVarErr*0.99-(VarErr+19)*(VarErr+19);
				if (VarErr<-1000)
				{
					RealTime+=((FTime.Time-RealTime)/10);
					if (MVarErr<-10000) MVarErr=-10000;
				}
				else
				{
					RealTime+=((FTime.Time-RealTime)/20);
					if (MVarErr<-1000) MVarErr=-1000;
				}
				if (MVarErr>0)
					MVarErr=0;
				if (0.0001*Max(Err,NumTF)<-200)
					Fcpu+=1*MVarErr;
				else
					Fcpu+=0.1*MVarErr;
			}
			else
			{
				MVarErr=0;
				if (VarErr<-20)
					Fcpu-=0.1*VarErr*VarErr;
				else
					Fcpu-=0.01*VarErr*VarErr;
			}
		}
		Fcpu_1=10000000.0/Fcpu;
	}
	else
	{
		Fl_TimeSyn=0;
	}
	if (++CntTK>NumTF/2)
	{
		oNClk=CntT2;
		CntTK=0;
		VardifErr=1e+12*(VarErr-OldErr)/(oNClk-oldNClk);
//		VardifErr=1000*(VarErr-OldErr)/(oNClk-oldNClk);
		if ((Fl_FSyn<2)&&(Fl==0))
		{
			RealTime=FTime.Time;
			Fl_FSyn++;
			OldErr=0;oldNClk=0;
			for (size_t i=0;i<NumTF;i++)
			{
				Err[i]=difErr[i]=0;
			};
		}
		else
		{
			// Ôîðcèðîâàíèå èçìåíåíèÿ ÷àñòîòû
			if (VarErr>0)
			{
				float	TVmin=(float)(0.0001*Min(Err,NumTF));
				if (TVmin>0)
				{
					if (TVmin>2000)TVmin=2000;
					Fcpu+=100*TVmin*TVmin;
				}
			}
			else
			{
				float	TVmax=(float)(0.0001*Max(Err,NumTF));
				if (TVmax<0)
				{
					if (TVmax<-2000)TVmax=-2000;
					Fcpu-=100*TVmax*TVmax;
				}
			};
			if ((fabs(VarErr)>1000)&&(Fl_FSyn>=2))
			{
				char	tBf[1286];
				sprintf(tBf,"TimeSynErr dT,ms=%d",(int)VarErr);
				TS_CE=SetCriticalError(this,tBf,"Warning",N_Service,TS_CE);
	//			tErr=0;OldErr=0;
				if (fabs(VarErr)>2000)
					VardifErr*=10;
				else
					VardifErr*=5;
				RealTime-=(INT64)VarErr;
				Fl_TimeSyn=0;

			}
			else
			{
				if (TS_CE)
				{
					DelCriticalError(TS_CE);TS_CE=0;
				}
				Fl_TimeSyn=1;
			}
			if (CntTime++>100)
			{
				FileTime	tFTime;
				FileTime	sFTime;
				SYSTEMTIME	sTime;
				tFTime.Time=RealTime;
				if ((FlSetT!=0)&&(SInt->NumLANSrv!=SInt->MyLANNum)) 
				{
					GetSystemTimeAsFileTime((FILETIME*)&sFTime.FTime);
					sFTime.Time+=(tFTime.Time-sFTime.Time)/2;
					FileTimeToSystemTime(&tFTime.FTime,&sTime);
					SetSystemTime(&sTime);
				}
				CntTime=0;
			};
			if (Fl==1)
			{
				VardifErr*=0.1;VarErr*=0.1;
			}
			if (VardifErr>10000) VardifErr=10000; 
			if (VardifErr<-10000) VardifErr=-10000; 
			sDErr=0.99*sDErr+0.01*VardifErr;
			if (VardifErr*OldDErr<0) sDErr=0;

			if (fabs(VarErr)>20)
				Fcpu+=(10*sDErr+VarErr/*+tErr*/);
			else
				Fcpu+=(0.01*sDErr+0.01*VarErr);

			if (Fcpu<100000000) Fcpu=100000000;
			OldDErr=VardifErr;
		}
		if (Fcpu>FcpuMax)
			Fcpu=FcpuMax;
		if (Fcpu<FcpuMin)
			Fcpu=FcpuMin;
		Fcpu_1=10000000.0/Fcpu;
		OldErr=VarErr;
		if (Fl_FSyn==2)
		{
//			Telemetr(NULL,"__tTimeErr,ms",(double)tErr);
			Telemetr(this,"__sTimeErr,ms",sDErr);
			Telemetr(this,"__TimeErr,ms",(double)VarErr);
			Telemetr(this,"__Fcpu",Fcpu);
			Telemetr(this,"__difErr,ms",(double)VardifErr);
		}
		oldNClk=oNClk;
	}
	else
	{
		if ((CntTK>NumTF/5)&&(Fl_FSyn>=1))
			Fl_TimeSyn=1;
	}
};

void	Services::Start()
{
	Fl_Stop=0;
//----------------NetServices---------
	State=1;
	pNetObject		pCurrnet;
	if(StopProcessing)
	{
		FindStrList8	FSL81;
		pCurrnet=(pNetObject)ClientList->Begin(FSL81);
		while(pCurrnet)
		{
			pCurrnet->Start();
			pCurrnet=(pNetObject)ClientList->Next(FSL81);
		}
	};
	TelemetryCount=0;
// -----------------------------------
// int			UDPP=GetUDPPort();
// ElUDPCli*	ElUDP=OpenUDPCilent(UDPP,(INT64)this);
// 			CloseUDPCilent(UDPP,(INT64)this);
// 			ReleaseUDPPort(UDPP);

}
void	Services::Stop()
{	
	Fl_Stop=1;
	//----------------NetServices---------
	State=2;
	pNetObject		pCurrnet;
	if(StopProcessing)
	{
		FindStrList8	FSL81;
		pCurrnet=(pNetObject)ClientList->Begin(FSL81);
		while(pCurrnet)
		{
			pCurrnet->Stop();
			pCurrnet=(pNetObject)ClientList->Next(FSL81);
		}
	};
	//----------------NetServices---------
};
int Services::GetTelemetry()
{
	if(++TelemetryCount==100)
	{
		TelemetryCount=0;

		char s[256];
		pNetObject pLocalElement;
		pLocalElement=GetTelemetryPtr(1);
//		sprintf(s,"-------------------------------------------"); WriteToLog(s);
		while(1)
		{
			if(pLocalElement!=NULL)
			{
				sprintf(s,"%8s: %2d, %2d, %4d, %8dk, %4d, %8dk",
					pLocalElement->Descriptor,
					pLocalElement->PackCtrl,
					pLocalElement->Param,

					pLocalElement->PackIn,
					pLocalElement->CountIn>>10,

					pLocalElement->PackOut,
					pLocalElement->CountOut>>10
					);
//				WriteToLog(s);
			}
			else break;
			pLocalElement=GetTelemetryPtr();
		}
//		sprintf(s,""); WriteToLog(s);
	};
	return 0;
}
pNetObject Services::GetTelemetryPtr(int first)
{
	if(first) return (pNetObject)ClientList->Begin(CFSL8);
	else return (pNetObject)ClientList->Next(CFSL8);
}

int	Services::IdleNet()
{
	int rc=0;
	// ------------NetServices---------------
	pElSockSrvEx pServer;
	pNetObject		pCurrnet;
	pPackNetObj		pack;
	PackNetObj		mypack;
//	pNetObject		pElement;
	pNetObject		pElementOut;
	MyLock(__FILE__,__LINE__,&FlSim,this);//Fl_MTSI=0;
//	MyLock(__FILE__,__LINE__,&Fl_RE9,this);//Fl_MTSI=0;
	pCurrnet=(pNetObject)ClientList->Begin(CFSL8);
	while(pCurrnet)
	{
		pCurrnet->ObjIdle();
		if(pCurrnet->TypeObj==ObjectServer)
		{
			pServer=(pElSockSrvEx)pCurrnet;
			pServer->CurrentClient=0;
		}
		pCurrnet=(pNetObject)ClientList->Next(CFSL8);
	}
	pElementOut=(pNetObject)ClientList->Begin(CFSL8);
	TimeClick=1;
	GetTelemetry();

	pElSockCliEx pClient;
//	char s[256];
	if(State==1)
	{
		pack=&mypack;
		if(pElementOut==NULL)
		{
			pElementOut=(pNetObject)ClientList->Begin(CFSL8);
		}
		if (pElementOut!=NULL)
		{
			while(1)
			{
				if(TimeClick)
				{
					try
					{
						if(pElementOut->FlagSelectProc)
							goto NextClientJump;
					}
					catch (...) 
					{
						break;
					}
				}

				pack->handle=pElementOut->handle;
				strcpy(pack->descriptor,pElementOut->Descriptor);

				pack->command=NetSockOutData; pack->cnt=0;

				if(pElementOut->TypeObj==ObjectClient)
				{
					pack->numinlist=-2;
					pClient=(pElSockCliEx)pElementOut;
					pack->pin=1; // read event
					pClient->RdEvent(pack->buf,pack->cnt);
					if(pack->cnt) 
					{
						if(*(int*)pack->buf==CBE_Connect)
						{
							pClient->Param=1;
							//						Debug(pack->descriptor,"Connect","HostName",pClient->HostName);
						}
						else if(*(int*)pack->buf==CBE_DisCon)
						{
							pClient->Param=0;
							//						Debug(pack->descriptor,"Disconnect","HostName",pClient->HostName);
							pElementOut=NULL;
						}
						DataFF(1005,(PUCHAR)pack,sizeof(PackNetObj));
						rc++;
					}
					else
					{
						pack->pin=0; // read socket
						pClient->CalcClick(2);
						pClient->ReadSock(pack->buf, pack->cnt);
						pClient->CalcClick(3);
						if(pack->cnt)
						{
							pClient->PackIn++;
							pClient->CountIn+=pack->cnt;
							DataFF(1005,(PUCHAR)pack,sizeof(PackNetObj));
							rc++;
						}
					}
				}
				else if(pElementOut->TypeObj==ObjectServer)
				{
					pServer=(pElSockSrvEx)pElementOut;
					// -------------------------------------------------------
					EventMsg		*pMess=NULL;

					pack->pin=0; // num client
					if(pack->pin==StateOut)
					{
						pMess=pServer->RdEvent();
						if(pMess)
						{
							if (pMess->Event!=1) pack->numinlist=pMess->Number;
							//else pack->numinlist=-1;  //  [11/15/2011 San] ??????
							pack->buf=(UCHAR*)&pMess->Number;
							pack->cnt=sizeof(pMess->Number);
							StateOut=1;
							DataFF(1005,(PUCHAR)pack,sizeof(PackNetObj));
							rc++;
							//						sprintf(s,"%d",pMess->Number); Debug2(s); //  [5/27/2005]
							//							break;
						}
						else
						{
NextClient:
							if (CntCliPack++>0500)
							{
								CntCliPack=0;
								if (++pServer->CurrentClient>=NumSockCli)
									pServer->CurrentClient=0;
							}
							pServer->CalcClick(2);
							pServer->ReadSock(
								pServer->CurrentClient,
								SockBuf,SockCnt);
							pServer->CalcClick(3);
							if(SockCnt) 
							{
								pack->numinlist=pServer->CurrentClient;
								//								pack->buf=(UCHAR*)&pServer->CurrentClient;
								//								pack->cnt=sizeof(pServer->CurrentClient);
								pServer->PackIn++;
								pServer->CountIn+=SockCnt;
								//								StateOut=4;
								//								pack->numinlist=pMess->Number;		
								pack->buf=SockBuf;
								pack->cnt=SockCnt;
								StateOut=0;
								pack->pin=4;
								DataFF(1005,(PUCHAR)pack,sizeof(PackNetObj));
								rc++;
								//								DataFF(1005,(PUCHAR)pack,sizeof(PackNetObj));
								//								break;
								goto NextClient;
							}
							else
							{
								CntCliPack=0;
								if (++pServer->CurrentClient<NumSockCli)
									goto NextClient;
							}
						}
					}
					// -------------------------------------------------------
					pack->pin=1; // event
					if(pack->pin==StateOut)
					{
						if(1) // ïàðàíîéÿ
						{
							pack->handle=pElementOut->handle;
							strcpy(pack->descriptor,pElementOut->Descriptor);

							pack->command=NetSockOutData; pack->cnt=0;
						}

						pack->buf=(PUCHAR)&pMess->Event;
						pack->cnt=sizeof(pMess->Event);
						if (pMess->Event==CBE_Connect) 
						{
							pack->numinlist=pMess->Number;		
							StateOut=2;
							pServer->Param++;
							//						sprintf(s,"event connect"); Debug2(s); //  [5/27/2005]
						}
						else 
						{
							StateOut=0;
							//						Debug(pack->descriptor,"Disconnect");
							pServer->Param--;
							pack->numinlist=pMess->Number;		
							//						sprintf(s,"event disconnect"); Debug2(s); //  [5/27/2005]
						}
						DataFF(1005,(PUCHAR)pack,sizeof(PackNetObj));
						rc++;
					}
					// -------------------------------------------------------
					pack->pin=2; // host IP
					if(pack->pin==StateOut)
					{
						if(1) // ïàðàíîéÿ
						{
							pack->handle=pElementOut->handle;
							strcpy(pack->descriptor,pElementOut->Descriptor);
							pack->command=NetSockOutData; pack->cnt=0;
						}

						pack->numinlist=pMess->Number;		
						pack->buf=(PUCHAR)&pMess->IPhost;
						pack->cnt=sizeof(pMess->IPhost);
						StateOut=3; 
						//					sprintf(s,"ip-address %s",pMess->IPhost); Debug2(s); //  [5/27/2005]
						DataFF(1005,(PUCHAR)pack,sizeof(PackNetObj));
						rc++;
					}
					// -------------------------------------------------------
					pack->pin=3; // host name
					if(pack->pin==StateOut)
					{
						pack->numinlist=pMess->Number;		
						pack->buf=(PUCHAR)&pMess->namehost;
						pack->cnt=sizeof(pMess->namehost);
						StateOut=0;
						//					Debug(pack->descriptor,"Connect",
						//						(PCHAR)&pMess->IPhost,
						//						(PCHAR)&pMess->namehost,
						//						pMess->Number);
						//					sprintf(s,"hostname %s",pMess->namehost); Debug2(s); //  [5/27/2005]
						//					sprintf(s,"%d - %d",pMess->port,pMess->Handle); Debug2(s); //  [5/27/2005]
						DataFF(1005,(PUCHAR)pack,sizeof(PackNetObj));
						rc++;
					}
					// -------------------------------------------------------
					pack->pin=4; // read sock
					if(pack->pin==StateOut)
					{
						pack->numinlist=pMess->Number;		
						pack->buf=SockBuf;
						pack->cnt=SockCnt;
						StateOut=0;
						DataFF(1005,(PUCHAR)pack,sizeof(PackNetObj));
						rc++;
					}
					// -------------------------------------------------------
				} 

NextClientJump:
				if(TimeClick) // áóäåì ëè èñêàòü îñòàëüíûõ êëèåíòîâ?
				{
					pElementOut=(pNetObject)ClientList->Next(CFSL8);
					if(pElementOut==NULL) {TimeClick=0; break; }
				}
				else break;
			}

			// 			if(pack->cnt)
			// 			{
			// 				DataFF(1005,(PUCHAR)pack,sizeof(PackNetObj));
			// 			}
			// 			else
			// 			{
			// 				//			pmas=(PUCHAR)pack; 
			// 			}
		}
	};
	// ------------NetServices---------------
//	MyUnLock(__FILE__,__LINE__,&Fl_RE9,this);//Fl_MTSI=0;
	MyUnLock(__FILE__,__LINE__,&FlSim,this);//Fl_MTSI=0;
	return rc;
};

int	Services::IdleSys()
{
	int rc=0;
	UCHAR*	tVol;
	size_t	tCnt;
	CalcTable();

	if (M_Cfg->MData.CustomerID != LCond.CustomerID)
	{
		M_Cfg->MData.CustomerID = LCond.CustomerID;
		if (((SData->Fl_StateSmartSt == 0) || (SData->Fl_StateSmartSt == 4)) && (M_Cfg->MData.Fl_RunSmSt))
		{
			PROCESSENTRY32 pEntry;
			if (!FindProcess(_T("SmartStarter.exe"), &pEntry))
			{
				HWND hwnd = NULL;
				char	FullPath[256];
				sprintf(FullPath, "%s\\SmartStarter.exe", M_Cfg->MData.Path);
				HINSTANCE  rcc = ShellExecuteA(hwnd, "open", FullPath, NULL, M_Cfg->MData.Path, SW_SHOWNORMAL);
			}
		}
		SData->Fl_StateApp = 2;
	}

/*	// Test HASP
	if (Timer(CntTS4,60000))
	{
		if (handle == HASP_INVALID_HANDLE_VALUE)
		{
			status = hasp_login(1, vendor_code, &handle);
			rc=-1;
			if (status != HASP_STATUS_OK)
			{
				switch (status)
				{
					case HASP_FEATURE_NOT_FOUND:
						memset((UCHAR*)&LCond,0,sizeof(LCond));
						memset((UCHAR*)&LUinfo,0,sizeof(LUinfo));
//							memset((UCHAR*)&NetLCond,0,sizeof(LCond));
//							memset((UCHAR*)&NetLUinfo,0,sizeof(NetLUinfo));
						memset((UCHAR*)&LSinfo,0,sizeof(LSinfo));
						Fl_HardLock=0;
						break;
					case HASP_HASP_NOT_FOUND:
						memset((UCHAR*)&LCond,0,sizeof(LCond));
						memset((UCHAR*)&LUinfo,0,sizeof(LUinfo));
//							memset((UCHAR*)&NetLCond,0,sizeof(LCond));
//							memset((UCHAR*)&NetLUinfo,0,sizeof(NetLUinfo));
						memset((UCHAR*)&LSinfo,0,sizeof(LSinfo));
						Fl_HardLock=0;
						break;
					case HASP_OLD_DRIVER:
						break;
					case HASP_NO_DRIVER:
						break;
					case HASP_INV_VCODE:
						break;
					case HASP_FEATURE_TYPE_NOT_IMPL:
						break;
					case HASP_TMOF:
						break;
					case HASP_TS_DETECTED:
						break;
					default:
						break;
				}
			}
			else
			{
				while (rc==-1)
				{
					Fl_HardLock=1;
					short	LC_Sum,LC_Sum2,LC_Sum3,LC_Sum4,LC_Sum5,LC_Sum6,LC_Sum7;
					status = hasp_read(handle,HASP_FILEID_RW,0,sizeof(LCond),&LCond);	// Main
					if (status != HASP_STATUS_OK)
						break;
					status = hasp_decrypt(handle,&LCond,sizeof(LCond));
					if (status != HASP_STATUS_OK)
						break;
					LC_Sum=LCond.Sum;LCond.Sum=0;LC_Sum2=LCond.Sum2;LCond.Sum2=0;LC_Sum3=LCond.Sum3;LCond.Sum3=0;LC_Sum4=LCond.Sum4;LCond.Sum4=0;LC_Sum5=LCond.Sum5;LCond.Sum5=0;
					LC_Sum6=LCond.Sum6;LCond.Sum6=0;
					LC_Sum7=LCond.Sum7;LCond.Sum7=0;
					short	Rc=calc_crc16((UCHAR*)&LCond,sizeof(LCond));
					if (LC_Sum!=Rc)
					{
						memset((UCHAR*)&LCond,0,sizeof(LCond));
						break;
					}
					status = hasp_read(handle,HASP_FILEID_RW,LCond.OffsetSysInfo,sizeof(LSinfo),&LSinfo);		// SysInfo
					if (status != HASP_STATUS_OK)
						break;
					status = hasp_decrypt(handle,&LSinfo,sizeof(LSinfo));
					if (status != HASP_STATUS_OK)
						break;
					status = hasp_read(handle,HASP_FILEID_RW,LCond.OffsetUnitInfo,sizeof(LUinfo),&LUinfo);		// UnitInfo
					if (status != HASP_STATUS_OK)
						break;
					status = hasp_decrypt(handle,&LUinfo,sizeof(LUinfo));
					if (status != HASP_STATUS_OK)
						break;

					status = hasp_read(handle,HASP_FILEID_RW,LCond.OffsetUserInfo,sizeof(LUSinfo),&LUSinfo);	// UserInfo
					if (status != HASP_STATUS_OK)
						break;
					status = hasp_decrypt(handle,&LUSinfo,sizeof(LUSinfo));
					if (status != HASP_STATUS_OK)
						break;
					status = hasp_read(handle,HASP_FILEID_RW,LCond.OffsetDomainInfo,sizeof(LDinfo),&LDinfo);	// DomainInfo
					if (status != HASP_STATUS_OK)
						break;
					status = hasp_decrypt(handle,&LDinfo,sizeof(LDinfo));
					if (status != HASP_STATUS_OK)
						break;
					Rc=calc_crc16((UCHAR*)&LUSinfo,sizeof(LUSinfo));
					if (LC_Sum6!=Rc)
					{
						memset((UCHAR*)&LUSinfo,0,sizeof(LUSinfo));
						break;
					}
					Rc=calc_crc16((UCHAR*)&LDinfo,sizeof(LDinfo));
					if (LC_Sum7!=Rc)
					{
						memset((UCHAR*)&LDinfo,0,sizeof(LDinfo));
						break;
					}

					status = hasp_read(handle,HASP_FILEID_RW,LCond.OffsetStatistic,sizeof(LRStat),&LRStat);	// Stat
					if (status != HASP_STATUS_OK)
						break;
					status = hasp_decrypt(handle,&LRStat,sizeof(LRStat));
					if (status != HASP_STATUS_OK)
						break;
					status = hasp_read(handle,HASP_FILEID_RW,LCond.OffsetPermittedHost,LCond.SizePermittedHost*sizeof(LPHost),&LPHost);	// Stat
					if (status != HASP_STATUS_OK)
						break;
					if (LCond.SizePermittedHost)
					{
						status = hasp_decrypt(handle,&LPHost,LCond.SizePermittedHost*sizeof(LPHost));
						if (status != HASP_STATUS_OK)
						break;
					}
					Rc=calc_crc16((UCHAR*)&LUinfo,sizeof(LUinfo));
					if (LC_Sum5!=Rc)
					{
						memset((UCHAR*)&LUinfo,0,sizeof(LUinfo));
						break;
					}
					Rc=calc_crc16((UCHAR*)&LSinfo,sizeof(LSinfo));
					if (LC_Sum2!=Rc)
					{
						memset((UCHAR*)&LCond,0,sizeof(LCond));
						memset((UCHAR*)&LSinfo,0,sizeof(LSinfo));
						break;
					}
			//		Rc=calc_crc16((UCHAR*)&LRStat,sizeof(LRStat));
			//		if (LC_Sum3!=Rc)
			//		{
			//			memset((UCHAR*)&LCond,0,sizeof(LCond));
			//			memset((UCHAR*)&LRStat,0,sizeof(LRStat)); 
			//		}
					if (LRStat.FirstRunTime.Time==0)
					{
						GetRealTime(&LRStat.FirstRunTime.FTime);
					}
					LRStat.TotallRunTime.Time+=(LRStat.LastRunTime.Time-LRStat.StartRunTime.Time);
					GetRealTime(&LRStat.StartRunTime.FTime);
					if (LRStat.TimeExpiry.Time<LRStat.StartRunTime.Time)
					{
						// Time protection
						memset((UCHAR*)&LUinfo,0,sizeof(LUinfo));
						memset((UCHAR*)&LCond,0,sizeof(LCond));
						memset((UCHAR*)&LSinfo,0,sizeof(LSinfo));
						break;
					}
			//		Rc=calc_crc16((UCHAR*)&LRStat,sizeof(LRStat));
			//		if (LC_Sum4!=Rc)
			//		{
			//			memset((UCHAR*)LPHost,0,50*sizeof(LPHost)); 
			//		}
					LRStat.RunCnt++;
					CopyLic(&LCond);
					if (M_Cfg->MData.CustomerID!=LCond.CustomerID)
					{
						M_Cfg->MData.CustomerID=LCond.CustomerID;
						if (SData->Fl_StateSmartSt==0)
						{
							PROCESSENTRY32 pEntry;
							if(!FindProcess("SmartStarter.exe",&pEntry))
							{
								HWND hwnd=NULL;
								char	FullPath[256];
								sprintf(FullPath,"%s\\SmartStarter.exe", M_Cfg->MData.Path);
								HINSTANCE  rcc=ShellExecute(hwnd, "open",FullPath, NULL, M_Cfg->MData.Path, SW_SHOWNORMAL);
							}
						}
						SData->Fl_StateNeuron=3;
					}
					CopyLicUnit(&LUinfo);
					CustomerInfo=GetCustomerInfo(M_Cfg->MData.CustomerID);
					rc=0;
				}
				if (rc==-1)
				{
					status = hasp_logout(handle);
					Fl_HardLock=0;
					handle=HASP_INVALID_HANDLE_VALUE;
				}
			}
		}
		else
		{
			if (handle!=HASP_INVALID_HANDLE_VALUE)
			{
				GetRealTime(&LRStat.LastRunTime.FTime);
//					LRStat.TotallRunTime.Time+=(LRStat.LastRunTime.Time-LRStat.StartRunTime.Time);
				status = hasp_encrypt(handle,&LRStat,sizeof(LRStat));
				status = hasp_write(handle,HASP_FILEID_RW,LCond.OffsetStatistic,sizeof(LRStat),&LRStat);	// Stat
				status = hasp_decrypt(handle,&LRStat,sizeof(LRStat));
//					status = hasp_logout(handle);
				if (status != HASP_STATUS_OK)
				{
					switch (status)
					{
					case HASP_INV_HND:
						break;
					default:
						break;
					}
				}
			}
			hasp_size_t memSize;
			status = hasp_get_size(handle, HASP_FILEID_RW, &memSize);
			if (status != HASP_STATUS_OK)
			{
				switch (status)
				{
					case HASP_FEATURE_NOT_FOUND:
						break;
					case HASP_INV_HND:
						break;
					case HASP_INV_FILEID:
						break;
					default:
						break;
				}
				Fl_HardLock=0;
				status = hasp_logout(handle);
				handle = HASP_INVALID_HANDLE_VALUE;
				if (strcmp(UserLogin,LUSinfo.UserLogin)==0)
				{
					LUSinfo.UserLogin[0]=0;
					Logoff();
				}
			}
			else
			{
				if (LRStat.TimeExpiry.Time<LRStat.StartRunTime.Time)
				{
					Fl_HardLock=0;
					status = hasp_logout(handle);
					handle = HASP_INVALID_HANDLE_VALUE;
					if (strcmp(UserLogin,LUSinfo.UserLogin)==0)
					{
						LUSinfo.UserLogin[0]=0;
						Logoff();
					}
				}
			}
//				status=hasp_read(handle,HASP_FILEID_RW,0,100,BufHasp);
//				Fl_HardLock=1;
		}
	}
*/
// 	if (DelIDIList->NumElem)
// 	{
// 		ID_ElemIDI*	TmpIDE;
// 		int	tNum=DelIDIList->NumElem/10;	//
// 		for (size_t sti=0;sti<tNum;sti++)
// 		{
// 			FindStrList8	FSL8;
// 			TmpIDE=(ID_ElemIDI*)DelIDIList->Begin(FSL8);
// 			if (TmpIDE)
// 				DelIDIList->Del(TmpIDE);
// 		}	
// 	}
// 	while (NetDmp.LockOData(tVol,tCnt))
// 	{
// 		tVol+=sizeof(SockHeader);
// 		InternalCloseSock(*(int*)tVol);
// 		NetDmp.UnLockOData();
// 	}

	return rc;
};

int Services::NS_DataFF(int N_Pin,UCHAR* pmas,size_t Cnt,Decoder* pDec)
{
	pElSockCliEx pClient;
	pElSockSrvEx pServer;
	PackNetObj		mypack;
	pPackNetObj		pack;
	pNetObject		pElement;
	pNetObject		pElementOut;
	if (!pDec)
	{
		pDec=this;
	}
	int rc,OCnt=0;
	if(State) //if(State==1)

		if(
			(StopProcessing==0) && (State) ||
			(StopProcessing==1) && (State==1)
			)
		{
			switch(N_Pin)
			{
			case 0:
				while(Cnt>0)
				{
					pack=(pPackNetObj)pmas;
					pmas+=sizeof(PackNetObj);
					Cnt-=sizeof(PackNetObj);

					memcpy(&mypack,pack,sizeof(PackNetObj));
					pElement=(pNetObject)ClientList->Find(
						(PUCHAR)pack->descriptor,
						strlen(pack->descriptor));

					if(pElement==NULL)
					{
						if(pack->command==NetSockNew)
						{
//							Debug(pack->descriptor,"create");
							if(pack->descriptor[0]!='_')
							{	// client
								pClient=new ElSockCliEx(ObjectClient,pack->descriptor,Srv,pDec);
								pElement=pClient; if(pElement==NULL) continue;
								//pElement->Start();
								ClientList->Add(pElement);MonClientList->Add(pElement); Counter++;
								continue; // !
							}
							else
							{	// server
								pServer=new ElSockSrvEx(ObjectServer,pack->descriptor,pack->NetType,Srv,pDec);
								pElement=pServer; if(pElement==NULL) continue;
								ClientList->Add(pElement);MonClientList->Add(pElement); Counter++;
								continue; // !
							}
						}
						else 
						{
							//						MyUnLock(__FILE__,__LINE__,&Fl_RE,this);
							return 0;
						}
					}
					if (pElement)
					{
						switch(pack->command)
						{
						case NetSockDel:
//							Debug(pack->descriptor,"NetSockDel");
							MonClientList->Remove(pElement);
							rc=ClientList->Del(pElement);
							if(rc!=-1) Counter--;
							pElement=NULL;
							break;
						case NetSockParam:
							//if(!strcmp(pack->param,"ServerIP")) strcpy(pack->host,pack->vol);
//							Debug(pack->descriptor,"NetSockParam",pack->param,pack->vol);
							pElement->SetParam(pack->param,pack->vol);
							pElement->PackCtrl++;
							break;
						case NetSockStart:
//							Debug(pack->descriptor,"NetSockStart");
							pElement->Start();
							pElement->PackCtrl++;
							break;
						case NetSockStop:
//							Debug(pack->descriptor,"NetSockStop");
							pElement->Stop();
							pElement->PackCtrl++;
							break;
						case NetSockData:
							pElement->PackOut++;
							pElement->CountOut+=pack->cnt;
							if(pElement->TypeObj==ObjectClient)
							{
								// without pack->pin
								pClient=(pElSockCliEx)pElement;
								pClient->CalcClick(0);
								pClient->SendBuf((PCHAR)pack->buf,pack->cnt);
								pClient->CalcClick(1);
							}
							else if(pElement->TypeObj==ObjectServer)
							{
								pServer=(pElSockSrvEx)pElement;
								switch(pack->pin)
								{
								case 0:	// NumCli
									pServer->CurrentClient=*(int*)(pack->buf);
									if(pServer->CurrentClient>=NumSockCli) 
										pServer->CurrentClient=-1;
									break;
								case 1:	// Transmit
									pServer->CalcClick(0);
									pServer->SendBuf(
										pServer->CurrentClient,
										pack->buf,pack->cnt);
									pServer->CalcClick(1);
									Srv->Telemetr(this,"Load_t/T%",100.*pServer->sLoadOutt/pServer->sLoadOutT);
									break;
								}
							}
							break;
						}
					}
				}
				break;
			}
		};
	return OCnt;
}

int		Services::IdleFunc(int NumberTr)
{
//	FileTime	tFT0,tFT,tFT2;
	if (Fl_Recurce)
		return 0;
	Fl_Recurce++;
//	tTime1=GetNClk();
	int rrc=ChildIdleFunc(NumberTr);
//	tTime2=GetNClk();
	int i,rc=0;
	int	rpt=0;
	UCHAR*	tPoi;
	CritErrStr*	tCErStr;
	size_t	tCnt;
//	MyLock(__FILE__,__LINE__,&Fl_RE,this);
//	Gluc.Step();


	CntIdle++;
//	Telemetr(this,"_CntIdle",CntIdle);
	
//	if(IDT_Prm==NULL)
//		IDT_Prm=OpenTable(this,TabParametr);
	if(IDT_NeuronGlobal==NULL)
		IDT_NeuronGlobal=OpenTable(this,TabNeuronGlobal);
	if(IDT_NeuronLocalCustomer==NULL)
		IDT_NeuronLocalCustomer=OpenTable(this,TabNeuronLocalCustomer);
	if(IDT_NeuronLocalSite==NULL)
		IDT_NeuronLocalSite=OpenTable(this,TabNeuronLocalSite);

	//	SInt->Idle2();
	if (Net_Flag==1)
	{
//		UCHAR*	tBuf;
//		UCHAR	Buf[80];
//		size_t	Cnt;
		switch(SInt->State)
		{
		case 0:
/*			if (Fl_SI==1)
			{
				strcpy(PNetObj.descriptor,"_SysInfo");
//				PNetObj.command=NetSockStop;
//				NetMenLAN->Data(0,(UCHAR*)&PNetObj,sizeof(PNetObj));
				PNetObj.command=NetSockDel;
				NetMenLAN->Data(0,(UCHAR*)&PNetObj,sizeof(PNetObj));
				DeleteDecoder(SysInt);SysInt=NULL;
				Fl_SI=0;
			}
*/			break;
		case 1:
/*			if (Fl_SI!=1)
			{	// Create SockSrv (SI)
				PNetObj.command=NetSockNew;
				strcpy(PNetObj.descriptor,"_SysInfo");
				NetMenLAN->Data(0,(UCHAR*)&PNetObj,sizeof(PNetObj));
				PNetObj.command=NetSockParam;
				strcpy(PNetObj.param,"Mode");
				strcpy(PNetObj.vol,"BufPack");
				NetMenLAN->Data(0,(UCHAR*)&PNetObj,sizeof(PNetObj));
				strcpy(PNetObj.param,"ServerPort");
				strcpy(PNetObj.vol,"499");
				NetMenLAN->Data(0,(UCHAR*)&PNetObj,sizeof(PNetObj));
				PNetObj.command=NetSockStart;
				NetMenLAN->Data(0,(UCHAR*)&PNetObj,sizeof(PNetObj));
//				SysInt=NewDecod("SysIntegrator");		// !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
				if (SysInt==NULL)
				{
					SInt->FatalError=1;
				}
				else
				{
					SysInt->Start();
					Fl_SI=1;
				}
			}
*/			break;
		};
//_gs001:
//		rc=NetMen->DataEx(1,Buf,1);
//		int ncc=0;
/*		if (rc!=0)
		{
			int		Fl_DD=0;
//_gs001:
//			INT64	T1=0,T2=0,T3=0;
//			GetRealTime(&tFT.FTime);
//			tFT0.Time=tFT.Time;
//			MyUnLock(__FILE__,__LINE__,&Fl_RE,this);
			while (NetMen->OutDataEx(0,tBuf,Cnt)!=0)
			{
				Fl_DD=1;

//				ncc++;
//				GetRealTime(&tFT2.FTime);
//				T1+=tFT2.Time-tFT.Time;
				pPackNetObj	pack;
				UCHAR*		pmas;
				rpt|=1;
				pack=(pPackNetObj)tBuf;
//				pmas=pack->buf;
				Cnt=pack->cnt;
				if (SizeNetMen<Cnt)
				{
					uchar*	tNetMenBuf;
					SizeNetMen=Cnt;
					tNetMenBuf=(uchar*)realloc(NetMenBuf,SizeNetMen);
					if (tNetMenBuf)
					{
						NetMenBuf=tNetMenBuf;
					}
					else
					{
						
					};
				}
				if (NetMenBuf)
				{
					memcpy(NetMenBuf,pack->buf,pack->cnt);
				}
				pmas=NetMenBuf;
				if(!strcmp(pack->descriptor,"_SysInfo"))
				{
				}
				else
				{	// Clients
					int		NumCli=atoi(&pack->descriptor[5]);
					if (CliList[NumCli].Fl!=0)
					{
						if (CliList[NumCli].Mode==0)
						{
							switch (pack->pin)
							{
							case 0:
								break;
							case 1:
								switch (*(int*)pmas)
								{
								case 0:	// Connect
									CliList[NumCli].FlCon=1;
									break;
								case 1:	// DisConnect
									CliList[NumCli].FlCon=0;
									break;
								}
								break;
							}
						}
						tObj=CliList[NumCli].PObj;
						GetRealTime(&tTm.FTime);
						CliList[NumCli].PObj->UserCallBackEx(NumCli,pack->pin,pmas,Cnt,2);
						GetRealTime(&tTm2.FTime);
						if (tTm2.Time-tTm.Time>4*KtimeS)
						{
							char	tBf[256];tBf[0]=0;
							sprintf(tBf,"Mt=%s UCB Time=%d mS",tObj->Name_Mt,(int)((tTm2.Time-tTm.Time)/Ktime));
							CE1=SetCriticalError(this,tBf,"Warning","UCB",CE1);
						}
						tObj=NULL;
					}
				}
			}
			if (Fl_DD==1)
				goto	_gs001;

//			GetRealTime(&tFT2.FTime);
//			if (tFT2.Time-tFT0.Time>KtimeS/3)
//				{size_t iasm=1;}
		}
*/
	}
	else
	{
		if (SInt->State==1)
		{
// 			NetMen=NewDecod("NetService",this);
// 			if (NetMen!=NULL)
// 			{
// 				NetMen->AddDecLink(this,0,1005);
// 				NetMen->Ext_Mt=0;
// 				NetMen->Srv=this;
// 				NetMen->StartEx();
				Net_Flag=1;
// 			}
// 			else
// 				SInt->FatalError=1;

		}
	}

	if (VDlg)
	{
//		((CDialog*)VDlg)->SetFocus();
		//OnTimer(0);
	}
	if (Fl_SGUI)
	{
		Srv->ModifUnit(Srv);
#ifdef DEBUG		
		OutLog("Send GetUI IdleFunc:Fl_SGUI",this,"SendGetUI");
#endif
		Srv->SendGetUnitInfo();
		Fl_SGUI=0;
	}
//	if (FlagEnter)
//	{
//		return;
//	}
//	FlagEnter=1;
	strcpy(UInfo.State,"Idle");
	if (System_On==1)
	{
		if (DmpCE->LockOData(tPoi,tCnt))
		{
			tCErStr=(CritErrStr*)(tPoi+sizeof(SockHeader));
			SetCriticalErrorI(tCErStr->ObjName,tCErStr->Msg,tCErStr->TypeMsg,tCErStr->MSystem,tCErStr->Handle);
		}
		DmpCE->UnLockOData();
		if (DmpDCE->LockOData(tPoi,tCnt))
		{
			INT64 CEHandle=*(INT64*)(tPoi+sizeof(SockHeader));
			DelCriticalErrorI(CEHandle);
		}
		DmpDCE->UnLockOData();

		if (FPurger==NULL)
		{
			char	TBufLogDay[128];
			FPurger = new F_Purger();
			FPurger->ParamEx("PathPurg", PathLog);
			FPurger->ParamEx("IgnoreTimeClk", "200000");
			sprintf(TBufLogDay, "D%d,D%d,D%d,D%d,D1,H0", M_Cfg->MData.NumDayLog, M_Cfg->MData.NumDayLog, M_Cfg->MData.NumDayLog, M_Cfg->MData.NumDayLog);
			FPurger->ParamEx("TimeExt0", TBufLogDay);
			//				FPurger->ParamEx("TimeExt0","D30,D30,D30,D30,D1,H0");
			FPurger->StartEx();

			/*
			FPurger=NewDecod("MultThrdMt",this);
			if (FPurger)
			{
				char	TBufLogDay[128];
//				strcpy(FPurger->UInfo.DomainName,UInfo.DomainName);
				FPurger->ParamEx("NameMt","F_Purger");
				FPurger->AddDecLink(this,0,2007);
				FPurger->ParamEx("PathPurg",PathLog);
				FPurger->ParamEx("IgnoreTimeClk","200000");
				sprintf(TBufLogDay,"D%d,D%d,D%d,D%d,D1,H0",M_Cfg->MData.NumDayLog,M_Cfg->MData.NumDayLog,M_Cfg->MData.NumDayLog,M_Cfg->MData.NumDayLog);
				FPurger->ParamEx("TimeExt0",TBufLogDay);
//				FPurger->ParamEx("TimeExt0","D30,D30,D30,D30,D1,H0");
				FPurger->StartEx();
			}
*/
		}
 		else
 		{
 			FPurger->DataFFEx(0,(UCHAR*)&FPurger,1);
//			FPurger->DataFFEx(0, (UCHAR*)&FPurger, 1);
		}
		if ((M_Cfg->MData.Fl_RunSI)&&(WANSISrv==0))
		{
			WANSISrv=InitSock(this,2,NT_WAN,"803");
		};
		if (M_Cfg->MData.Fl_WAN)
		{
	 		if ((M_Cfg->MData.IP_WANIS1)&&(Cli1==0))
			{
				if((M_Cfg->MData.Fl_RunSICli==0)||(SInt->MyLANNum==SInt->NumLANSrv))
					Cli1=InitSock(this,1,NT_WAN,"803",inet_ntoa(*(in_addr*)&Srv->M_Cfg->MData.IP_WANIS1));
			}
			if ((M_Cfg->MData.IP_WANIS2)&&(Cli2==0))
			{
				if((M_Cfg->MData.Fl_RunSICli==0)||(SInt->MyLANNum==SInt->NumLANSrv))
					Cli2=InitSock(this,1,NT_WAN,"803",inet_ntoa(*(in_addr*)&Srv->M_Cfg->MData.IP_WANIS2));
			}
		};
		if (Timer(CntT9,1000))
		{
			if ((LUSinfo.UserLogin[0]!=0)&&(strcmp(LUSinfo.UserLogin,UserLogin)!=0))	//	UserID
			{
				Logoff();
				UserID=TestLoginPassword(LUSinfo.UserLogin,LUSinfo.UserPassword);
				UserStatus=LUSinfo.UserStatus;
			};

			FileTime	RFT;
			GetRealTime(&RFT.FTime);
			if (Cli1)
			{
				if ((M_Cfg->MData.Fl_WAN==0)||(M_Cfg->MData.IP_WANIS1==0)||((M_Cfg->MData.Fl_RunSICli==1)/*&&(SInt->MyLANNum!=SInt->NumLANSrv)*/)||(RFT.Time-WANCli1FT.Time>KtimeM))
				{
					CloseSock(Cli1);Cli1=0;
					WANCli1FT.Time=RFT.Time;
				}
			};
			if (Cli2)
			{
				if ((M_Cfg->MData.Fl_WAN==0)||(M_Cfg->MData.IP_WANIS2==0)||((M_Cfg->MData.Fl_RunSICli==1)/*&&(SInt->MyLANNum!=SInt->NumLANSrv)*/)||(RFT.Time-WANCli2FT.Time>KtimeM))
				{
					CloseSock(Cli2);Cli2=0;
					WANCli2FT.Time=RFT.Time;
				}
			};
			if (WANSISrv)
			{
				if (M_Cfg->MData.Fl_RunSI==0)
				{
					CloseSock(WANSISrv);WANSISrv=0;
				}
			}
		}
	};
//	tTime1=GetNClk();
	Srv->M_Cfg->MData.Fl_LAN = 0;	// !!!
	if ((NetCommLAN==0)&&(Srv->M_Cfg->MData.Fl_LAN!=0))
	{
		NetCommLAN=NewDecod("NetComm",this);
		if (NetCommLAN)
		{
//			strcpy(NetCommLAN->UInfo.DomainName,UInfo.DomainName);
			NetCommLAN->ParamEx("NetType","LAN");
			NetCommLAN->ParamEx("PortSrv",TCPPort);
			NetCommLAN->AddDecLink(this,0,1003);
			NetCommLAN->StartEx();
		}
	}
// 	if ((NetCommWAN==0)&&(Srv->M_Cfg->MData.Fl_WAN!=0))
// 	{
// 		NetCommWAN=NewDecod("NetComm");
// 		if (NetCommWAN)
// 		{
// 			NetCommWAN->Param("NetType","WAN");
// 			NetCommWAN->Param("PortSrv","803");
// 			NetCommWAN->AddDecLink(this,0,1004);	//?????????????????????????????????????
// 			NetCommWAN->Start();
// 		}
// 	}
//	#ifdef _Neuron
// 	if (!UnitView)
// 	{
// 		UnitView=NewDecod("MUnitView",this);
// //		if (UnitView)
// //			strcpy(UnitView->UInfo.DomainName,UInfo.DomainName);
// 	}
//	#endif
/*
	if (Fl_Prm==0)
	{
	// Èíèöèàëèçàöèÿ ñïèñêà ñèñòåìíûõ ïàðàìåðòîâ
		i=LastPrm;
		Parametr*	pPrm;
		El_Tab*		ElTab;
		El_Tab*		ElRoot;
		El_Tab*		ElVol;
		ElRoot=IDT_Prm->GetElTab(&Anchor,TabParametr,3);
		pPrm=GetTablParametrID(i);
// 		if (pPrm->UID!=i)
// 		{
// 			{size_t iasm=1;}
// 		}
		if (ElRoot)
		{
			while (pPrm!=0)
			{
				if (pPrm->UID>=LastPrm)
				{
					ElTab=IDT_Prm->GetElTab(&Anchor,pPrm->UID);
					if (ElTab==NULL)
					{
						ElTab=IDT_Prm->AddET(&Anchor,pPrm->UID,ElRoot);
					}
					if (ElTab)
					{
						if (pPrm->Regularity)
						{
							ElVol=IDT_Prm->Find(&Anchor,AcsMd_Read,"Value",ElTab);
							if (ElVol==NULL)
								ElVol=IDT_Prm->AddET(&Anchor,"Value",ElTab);
							IDT_Prm->UnLockEl(&Anchor,ElVol);
						};
//						IDT_Prm->MoLockEl(&Anchor,ElTab,AcsMd_Write);
						IDT_Prm->AddParam(&Anchor,ElTab,IDNameElTab,pPrm->Name,1);
	//					ElTab->GetIDI()->pPList->SetParam(IDNameElTab,pPrm->Name,strlen(pPrm->Name)+1,this);
						IDT_Prm->AddParam(&Anchor,ElTab,IDPrmType,pPrm->Type,1);
	//					ElTab->GetIDI()->pPList->SetParam(IDPrmType,&pPrm->Type,sizeof(char),this);
						IDT_Prm->AddParam(&Anchor,ElTab,IDPrmIndex,pPrm->Index,1);
	//					ElTab->GetIDI()->pPList->SetParam(IDPrmIndex,&pPrm->Index,sizeof(char),this);
						IDT_Prm->AddParam(&Anchor,ElTab,IDPrmRegularity,pPrm->Regularity,1);
	//					ElTab->GetIDI()->pPList->SetParam(IDPrmRegularity,&pPrm->Regularity,sizeof(char),this);
						IDT_Prm->AddParam(&Anchor,ElTab,IDPrmStatus,pPrm->Status,1);
	//					ElTab->GetIDI()->pPList->SetParam(IDPrmStatus,&pPrm->Status,sizeof(int),this);
						IDT_Prm->AddParam(&Anchor,ElTab,IDPrmSize,pPrm->Size,1);
	//					ElTab->GetIDI()->pPList->SetParam(IDPrmSize,&pPrm->Size,sizeof(int),this);
						IDT_Prm->AddParam(&Anchor,ElTab,IDPrmView,pPrm->View,1);
	//					ElTab->GetIDI()->pPList->SetParam(IDPrmView,&pPrm->View,sizeof(int),this);
						IDT_Prm->AddParam(&Anchor,ElTab,IDRemark,pPrm->Remark,1);
						IDT_Prm->ModifElTab(&Anchor,ElTab);
					}
					IDT_Prm->UnLockEl(&Anchor,ElTab);
				}
				pPrm=GetTablParametrID(++i);
			}
			IDT_Prm->ModifElTab(&Anchor,ElRoot);
			Fl_Prm=1;
			IDT_Prm->UnLockEl(&Anchor,ElRoot);
		}
	};
*/
	Fl_Prm=1;
/*
	if (BDBIDInfo==0)
	{
		BDBIDInfo=NewDecod("IDI_FSrv",this);
 		if (BDBIDInfo)
 		{
			BDBIDInfo->ParamEx("Mode",N_Standalone);
			BDBIDInfo->ParamEx("Path",MainPath);
			BDBIDInfo->ParamEx("Transit","Off");

			BDBIDInfo->AddDecLink(this,0,1002);	// 1000 pin Services
 			BDBIDInfo->StartEx();
 		}
	}
*/
//	char			tBuf[256];
//   StrChron        *StrCh;
//	Chron->Measuring(0,0);
	if (ThIDView)	// TC
	{
//		ThIDView->DataIdle();
		ThIDView->DataFFEx(1,(UCHAR*)ThIDView,1);
	}
	if (FSrv)		// TC
	{
//		FSrv->DataIdle();
		FSrv->DataFFEx(1,(UCHAR*)FSrv,1);
	}
	if (NetCommLAN)		// TC
	{
		ID_ElemString*	tEl=new ID_ElemString(MainDomainName);
		El_CliInt*	Cli=(El_CliInt*)SInt->CliLAN.Find((UCHAR*)&SInt->INetAdrLAN,sizeof(ULONG));
		if (Cli)
		{
			if (MainDomainName[0]=='#')
				strcpy(Cli->Cli.MainDomainName,MainDomainName);
			if (Cli->Cli.DomainList==NULL)
				Cli->Cli.DomainList=new ID_List8();
			if (Cli->Cli.DomainList->Add(tEl)==-1)
			{
				delete tEl;
			}
		}
		else
			delete tEl;
//		NetCommLAN->DataIdle();
//		NetCommLAN->DataFFEx(1,(UCHAR*)NetCommLAN,1);
	}
// 	if (NetCommWAN)		// TC
// 		NetCommWAN->DataFF(1,(UCHAR*)NetCommWAN,1);
//	if (UnitView)
//	{
//		UnitView->DataIdle();
//		UnitView->DataFFEx(1,(UCHAR*)UnitView,1);
//	}
//	if (BDBIDInfo)	// TC
//	{
//		BDBIDInfo->DataIdle();
//		BDBIDInfo->DataFFEx(1,(UCHAR*)BDBIDInfo,1); //QueueSync
//	}
	El_Tab		*ETbl;
	El_Unit*	tEl;
	if (Timer(CntT7,5000))
	{
		if (M_Cfg->MData.Fl_SendUpD)
		{
			char	tBf[256];
			El_DllFile	*tEl_DF;
			FindStrList8	FSL8;
			tEl_DF=(El_DllFile*)DllFileQwe->Begin(FSL8);
			if (tEl_DF)
			{	// Ïåðåäà÷à ôàéëà äðóãîìó õîñòó
				Fl_UpData=1;
				char	FullFileName[512];
				int		FileDll;
				sprintf(FullFileName,"%s\\Dll\\%s",MainPath,tEl_DF->DllName);
				if ((FileDll = _open(FullFileName, _O_RDONLY|_O_BINARY,_S_IREAD))!=-1)
				{
					int		FileSize=0;
					uchar	*BufIn=(UCHAR*)malloc(32768);
					int		RCnt;
					sprintf(tBf,"SendDllFile=%s to Host=%s",tEl_DF->DllName,tEl_DF->HostName);
					OutLog(tBf,this,"UpDate");

					IDInfo*	IDI=GetNewIDI(this);
					InitCommIDI(IDI);
					AddressInfo	AdrI;
					IDI->AdrInfo=&AdrI;
//					IDI->pPList->SetParam(IDHostName,tEl_DF->HostName,strlen(tEl_DF->HostName)+1,this);
					strcpy(AdrI.DestinAI.HostName,tEl_DF->HostName);
//					IDI->pPList->SetParam(IDUnitName,N_Service,9,this);
					strcpy(AdrI.DestinAI.UnitName,N_Service);
					IDI->pPList->SetParam(IDFileName,tEl_DF->DllName,strlen(tEl_DF->DllName)+1,this);
					IDI->Command=Command_C_SendDllFile;
					IDI->NumPack++;
					IDI->Buf=BufIn;
					IDI->Cnt=0;
					IDI->StateBlock=StateBlock_BEGIN;
					SendIDI(IDI,this);
					RCnt=_read(FileDll,(void *)BufIn,32768);
					while (RCnt)
					{
						FileSize+=RCnt;
						IDI->Cnt=RCnt;
						IDI->StateBlock=0;
						IDI->NumPack++;
						SendIDI(IDI,this);
						RCnt=_read(FileDll,(void *)BufIn,32768);
					};
					IDI->Cnt=0;
					IDI->StateBlock=StateBlock_END;
					IDI->pPList->SetParam(IDFileLen,&FileSize,sizeof(INT),this);
					IDI->TimeBegin.Time=tEl_DF->Cr.Time;
					IDI->TimeLastWr.Time=tEl_DF->Wr.Time;
					IDI->TimeClose.Time=tEl_DF->Ac.Time;
					IDI->NumPack++;
					SendIDI(IDI,this);
					DeleteIDI(IDI);	
					_close(FileDll);
					free(BufIn);
				};
				DllFileQwe->Del(tEl_DF);
			}
			else
			{
				if ((Fl_UpData==1)&&(MainFileQwe->NumElem==0))
				{
					Fl_UpData=0;
					sprintf(tBf,"Send UpDllDate");
					OutLog(tBf,this,"UpDate");
					IDInfo*	IDI=GetNewIDI(this);
					InitCommIDI(IDI);
					AddressInfo	AdrI;
					IDI->AdrInfo=&AdrI;
//					IDI->pPList->SetParam(IDHostName,"*",2,this);
//					strcpy(AdrI.DestinAI.HostName,tEl_DF->HostName);
//					IDI->pPList->SetParam(IDUnitName,N_Service,9,this);
					strcpy(AdrI.DestinAI.UnitName,N_Service);
					IDI->Command=Command_C_UpDllData;
					IDI->NumPack++;
					IDI->Buf=NULL;
					IDI->Cnt=0;
					SendIDI(IDI,this);
					DeleteIDI(IDI);	
				}
			}
			FindStrList8	FSL82;
			tEl_DF=(El_DllFile*)MainFileQwe->Begin(FSL82);
			if (tEl_DF)
			{	// Ïåðåäà÷à ôàéëà äðóãîìó õîñòó
				Fl_UpData2=1;
				char	FullFileName[512];
				int		FileDll;
				sprintf(FullFileName,"%s\\%s",MainPath,tEl_DF->DllName);
				if ((FileDll = _open(FullFileName, _O_RDONLY|_O_BINARY,_S_IREAD))!=-1)
				{
					int		FileSize=0;
					uchar	*BufIn=(UCHAR*)malloc(32768);
					int		RCnt;
					sprintf(tBf,"SendMainFile=%s to Host=%s",tEl_DF->DllName,tEl_DF->HostName);
					OutLog(tBf,this,"UpDate");
					IDInfo*	IDI=GetNewIDI(this);
					InitCommIDI(IDI);
					AddressInfo	AdrI;
					IDI->AdrInfo=&AdrI;
//					IDI->pPList->SetParam(IDHostName,tEl_DF->HostName,strlen(tEl_DF->HostName)+1,this);
					strcpy(AdrI.DestinAI.HostName,tEl_DF->HostName);
//					IDI->pPList->SetParam(IDUnitName,N_Service,9,this);
					strcpy(AdrI.DestinAI.UnitName,N_Service);
					IDI->pPList->SetParam(IDFileName,tEl_DF->DllName,strlen(tEl_DF->DllName)+1,this);
					IDI->Command=Command_C_SendMainFile;
					IDI->NumPack++;
					IDI->Buf=BufIn;
					IDI->Cnt=0;
					IDI->StateBlock=StateBlock_BEGIN;
					SendIDI(IDI,this);
					RCnt=_read(FileDll,(void *)BufIn,32768);
					while (RCnt)
					{
						FileSize+=RCnt;
						IDI->Cnt=RCnt;
						IDI->StateBlock=0;
						IDI->NumPack++;
						SendIDI(IDI,this);
						RCnt=_read(FileDll,(void *)BufIn,32768);
					};
					IDI->Cnt=0;
					IDI->StateBlock=StateBlock_END;
					IDI->pPList->SetParam(IDFileLen,&FileSize,sizeof(INT),this);
					IDI->TimeBegin.Time=tEl_DF->Cr.Time;
					IDI->TimeLastWr.Time=tEl_DF->Wr.Time;
					IDI->TimeClose.Time=tEl_DF->Ac.Time;
					IDI->NumPack++;
					SendIDI(IDI,this);
					DeleteIDI(IDI);	
					_close(FileDll);
					free (BufIn);
				};
				MainFileQwe->Del(tEl_DF);
			}
			else
			{
				if ((Fl_UpData2==1)&&(DllFileQwe->NumElem==0))
				{
					Fl_UpData2=0;
					sprintf(tBf,"Send UpMainDate");
					OutLog(tBf,this,"UpDate");
					IDInfo*	IDI=GetNewIDI(this);
					InitCommIDI(IDI);
					AddressInfo	AdrI;
					IDI->AdrInfo=&AdrI;
//					IDI->pPList->SetParam(IDHostName,"*",2,this);
//					IDI->pPList->SetParam(IDUnitName,N_Service,9,this);
					strcpy(AdrI.DestinAI.UnitName,N_Service);
					IDI->Command=Command_C_UpMainData;
					IDI->NumPack++;
					IDI->Buf=NULL;
					IDI->Cnt=0;
					SendIDI(IDI,this);
					DeleteIDI(IDI);	
				}
			}
		}
		if (System_On)
		{
			PROCESSENTRY32 pEntry;
			char	TmpDir[512];
			char	TmpDir2[512];
			char lpBuffer[512];
			bool	brc;

			MyLock(__FILE__, __LINE__, &FlSim, this);
			sprintf(TmpDir, "%s\\Install", MainPath);
			if (SetCurrentDirectoryA(TmpDir))
			{
				SetCurrentDirectoryA(MainPath);
				if (SData->Fl_StateSmartSt == 0)
				{
					sprintf(TmpDir, "%s\\Install\\SmartStarter.exe", MainPath);
					sprintf(TmpDir2, "%s\\SmartStarter.exe", MainPath);
					brc = MoveFileExA(TmpDir, TmpDir2, MOVEFILE_REPLACE_EXISTING);
//					SetCurrentDirectoryA(lpBuffer);
//					sprintf(TmpDir, "%s\\Install\\Neuron.exe_", MainPath); unlink(TmpDir);
						//					sprintf(TmpDir,"%s\\Install\\BDBcon.exe_",MainPath);unlink(TmpDir);
						//					sprintf(TmpDir,"%s\\Install\\TD.cfg_",MainPath);unlink(TmpDir);
						//					sprintf(TmpDir,"%s\\Install\\GeoIPCountryWhois.csv_",MainPath);unlink(TmpDir);
//						sprintf(TmpDir, "%s\\Install\\SmartStarter.exe_", MainPath); unlink(TmpDir);
					sprintf(TmpDir, "%s\\Install", MainPath);
					brc = RemoveDirectoryA(TmpDir);
					SData->Fl_StateSmartSt = 0;
				}
				else
				{
					if (SData->Fl_StateSmartSt==1)
						SData->Fl_StateSmartSt = 3;
				}
			};
			MyUnLock(__FILE__, __LINE__, &FlSim, this);

			if (((SData->Fl_StateSmartSt==0)|| (SData->Fl_StateSmartSt == 4))&&(SData->Fl_StateApp!=3))
			{
//				GetCurrentDirectoryA(512,lpBuffer);
#ifndef		_DEBUG
				if (M_Cfg->MData.Fl_RunSmSt) 
				{
					if(!FindProcess(_T("SmartStarter.exe"),&pEntry))
					{
						HWND hwnd=NULL;
						char	FullPath[256];
						sprintf(FullPath,"%s\\SmartStarter.exe", M_Cfg->MData.Path);
						HINSTANCE  rcc=ShellExecuteA(hwnd, "open",FullPath, NULL, M_Cfg->MData.Path, SW_SHOWNORMAL);
					}
				}
#endif
			}
			else
			{
				if(!FindProcess(_T("SmartStarter.exe"),&pEntry))
				{
					SData->Fl_StateSmartSt=0;
				}
			}
		}
	}
//	tTime2=GetNClk();
	if (Timer(CntT5,1000))
	{
		SData->CntRun++;
#ifndef Win9x
		int	tNumIP=0;
		int	Fl_TempReSt=0;
		hoststr=gethostbyname(HostName);
		if (hoststr)
		{
			int IPDef=0x7f000001;
//			memcpy((char*)&IPAddrPAN,&IPDef,4);
//			memcpy((char*)&IPAddrLAN,&IPDef,4);
//			memcpy((char*)&IPAddrWAN,&IPDef,4);
			for (int jj=0;jj<5;jj++)
			{
				try
				{
					if (hoststr->h_addr_list[jj]==0)
						break;
					memcpy((char*)&IPDef,hoststr->h_addr_list[jj],4);
					if ((IPDef&M_Cfg->MData.IP_MaskPAN)==(M_Cfg->MData.IP_PAN&M_Cfg->MData.IP_MaskPAN))
					{
						if (*(ULONG*)&IPAddrPAN!=*(ULONG*)hoststr->h_addr_list[jj])
						{
							memcpy((char*)&IPAddrPAN,hoststr->h_addr_list[jj],4);
							SInt->Fl_ReInit=1;Fl_TempReSt++;
						}
					}
					if ((IPDef&M_Cfg->MData.IP_MaskLAN)==(M_Cfg->MData.IP_LAN&M_Cfg->MData.IP_MaskLAN))
					{
						if (*(ULONG*)&IPAddrLAN!=*(ULONG*)hoststr->h_addr_list[jj])
						{
							memcpy((char*)&IPAddrLAN,hoststr->h_addr_list[jj],4);
							SInt->Fl_ReInit=1;Fl_TempReSt++;
						}
					}
					if ((IPDef&M_Cfg->MData.IP_MaskWAN)==(M_Cfg->MData.IP_WAN&M_Cfg->MData.IP_MaskWAN))
					{
						if (*(ULONG*)&IPAddrWAN!=*(ULONG*)hoststr->h_addr_list[jj])
						{
							memcpy((char*)&IPAddrWAN,hoststr->h_addr_list[jj],4);
							SInt->Fl_ReInit=1;Fl_TempReSt++;
						}
					}
					tNumIP++;
				}
				catch(...)
				{
					break;
				}
			}
			strcpy(HostName,hoststr->h_name);
		}
		if ((*(ULONG*)&IPAddrWANCli1!=M_Cfg->MData.IP_WANIS1)||(*(ULONG*)&IPAddrWANCli2!=M_Cfg->MData.IP_WANIS2)||(M_Cfg->MData.Fl_PAN!=tFl_PAN)||(M_Cfg->MData.Fl_WAN!=tFl_WAN)||(M_Cfg->MData.Fl_RunSI!=tFl_RunSI)||(M_Cfg->MData.Fl_RunSICli!=tFl_RunSICli))
		{
			*(ULONG*)&IPAddrWANCli1=M_Cfg->MData.IP_WANIS1;
			*(ULONG*)&IPAddrWANCli2=M_Cfg->MData.IP_WANIS2;
			tFl_PAN=M_Cfg->MData.Fl_PAN;
			tFl_WAN=M_Cfg->MData.Fl_WAN;
			tFl_RunSI=M_Cfg->MData.Fl_RunSI;
			tFl_RunSICli=M_Cfg->MData.Fl_RunSICli;
			SInt->Fl_ReInit=1;
		};
		if(NumIP!=tNumIP)
		{
			NumIP=tNumIP;
			SInt->Fl_ReInit=1;Fl_TempReSt++;
		}
		if (Fl_TempReSt)
		{
			if (NetCommLAN)
			{
				DeleteDecoder(NetCommLAN);NetCommLAN=NULL;
			};
// 			if (NetCommWAN)
// 			{
// 				DeleteDecoder(NetCommWAN);NetCommWAN=NULL;
// 			};
			if ((NetCommLAN==0)&&(Srv->M_Cfg->MData.Fl_LAN!=0))
			{
				NetCommLAN=NewDecod("NetComm",this);
				if (NetCommLAN)
				{
					NetCommLAN->ParamEx("NetType","LAN");
					NetCommLAN->ParamEx("PortSrv",TCPPort);
					NetCommLAN->AddDecLink(this,0,1003);
					NetCommLAN->StartEx();
				}
			}
// 			if ((NetCommWAN==0)&&(Srv->M_Cfg->MData.Fl_WAN!=0))
// 			{
// 				NetCommWAN=NewDecod("NetComm");
// 				if (NetCommWAN)
// 				{
// 					NetCommWAN->Param("NetType","WAN");
// 					NetCommWAN->Param("PortSrv","803");
// 					NetCommWAN->AddDecLink(this,0,1004);
// 					NetCommWAN->Start();
// 				}
// 			}
		}

#endif
		FileTime	Real_FT;
		Srv->GetRealTime(&Real_FT.FTime);
		if (Fl_ExtSin)
		{
			if ((Real_FT.Time-TimeLES.Time)>KtimeM)
				Fl_ExtSin=0;
		}
//		MyLock(__FILE__,__LINE__,&UM->Fl_RE,this);
		FindStrList8	FSL83;
		tEl=(El_Unit*)UM->UnitList->Begin(FSL83);
		while (tEl)
		{
//			MyUnLock(__FILE__,__LINE__,&UM->Fl_RE,this);
			if (strcmp(ShortHostName,tEl->UInf.Host)==0)
			{
				if (tEl->UInf.pObj) 
					if(Real_FT.Time-tEl->UInf.ModTime.Time>KtimeM)
						ModifUnit(tEl->UInf.pObj);

				tEl=(El_Unit*)UM->UnitList->Next(FSL83);
			}
			else
			{
				if (SInt->FindHost(tEl->UInf.Host)==NULL)
				{
					UM->DelUnit(&tEl->UInf);
					tEl=(El_Unit*)UM->UnitList->Next(FSL83);
				}
				else
				{
					if (Real_FT.Time-tEl->UInf.ModTime.Time>(10*(INT64)KtimeM))
					{
						UM->DelUnit(&tEl->UInf);
					}
					tEl=(El_Unit*)UM->UnitList->Next(FSL83);
				}
			}
		}
//		MyUnLock(__FILE__,__LINE__,&UM->Fl_RE,this);
		if (System_On==1)
		{
			El_CliInt	*Cli=(El_CliInt*)SInt->CliLAN.Next(FSL85);
			if (Cli==NULL)
				Cli=(El_CliInt*)SInt->CliLAN.Begin(FSL85);
			if ((Cli)&&(Cli->Addr!=SInt->INetAdrLAN))
			{
				if (Cli->Cli.DomainList)
				{
					FindStrList8	FSL8;
					ID_Elem*	tIDEl=Cli->Cli.DomainList->Begin(FSL8);
					while (tIDEl)
					{
						ETbl=IDT_NeuronLocalSite->FindNested(&Anchor,AcsMd_Read,N_Domain,tIDEl->Buff,N_Host,Cli->Cli.ShortHost,NULL);
						if (ETbl)
						{
							char*	tVol;
							char*	tVol2;
							if (IDT_NeuronLocalSite->GetParam(&Anchor,ETbl,IDState,tVol)!=-1)
							{
								INT64	HostPersID=0;
//								MyLock(__FILE__,__LINE__,&UM->Fl_RE,this);
								FindStrList8	FSL82;
								tEl=(El_Unit*)UM->UnitList->Begin(FSL82);
								while (tEl)
								{
									if ((strcmp(Cli->Cli.Host,tEl->UInf.Host)==0)&&(strcmp(tEl->UInf.UnitName,N_Service)==0))
									{
										HostPersID=tEl->UInf.PersID;
										break;
									}
									tEl=(El_Unit*)UM->UnitList->Next(FSL82);
								}
//								MyUnLock(__FILE__,__LINE__,&UM->Fl_RE,this);
								if (IDT_NeuronLocalSite->GetParam(&Anchor,ETbl,IDPersonalID,tVol2)!=-1)
								{
									INT64	tPersID=*(INT64*)tVol2;
									if ((tPersID!=HostPersID)&&(HostPersID!=0))
										IDT_NeuronLocalSite->AddParam(&Anchor,ETbl,IDPersonalID,HostPersID,1);
								}
								switch (Cli->Cli.Command)
								{
								case 0:
									if (strcmp(tVol,"Off")!=0)
									{
										IDT_NeuronLocalSite->AddParam(&Anchor,ETbl,IDPersonalID,HostPersID,1);
										IDT_NeuronLocalSite->AddParam(&Anchor,ETbl,IDProcessed,VolumeProcessed[IDVolProc_On],1);
										IDT_NeuronLocalSite->AddParam(&Anchor,ETbl,IDState,VolumeProcessed[IDVolProc_Off]);
									}
									break;
								case SICom_ServerWait:
								case SICom_ClientWait:
									if (strcmp(tVol,"Setup")!=0)
									{
										IDT_NeuronLocalSite->AddParam(&Anchor,ETbl,IDPersonalID,HostPersID,1);
										IDT_NeuronLocalSite->AddParam(&Anchor,ETbl,IDProcessed,VolumeProcessed[IDVolProc_On],1);
										IDT_NeuronLocalSite->AddParam(&Anchor,ETbl,IDState,VolumeProcessed[IDVolProc_Setup]);
									}
									break;
								case SICom_Standalone:
								case SICom_ServerActive:
								case SICom_ClientActive:
								case SICom_SuperVisorActive:
									if (strcmp(tVol,"On")!=0)
									{
										IDT_NeuronLocalSite->AddParam(&Anchor,ETbl,IDPersonalID,HostPersID,1);
										IDT_NeuronLocalSite->AddParam(&Anchor,ETbl,IDProcessed,VolumeProcessed[IDVolProc_On],1);
										IDT_NeuronLocalSite->AddParam(&Anchor,ETbl,IDState,VolumeProcessed[IDVolProc_On]);
									}
									break;
								case SICom_Error:
									if (strcmp(tVol,"Error")!=0)
									{
										IDT_NeuronLocalSite->AddParam(&Anchor,ETbl,IDPersonalID,HostPersID,1);
										IDT_NeuronLocalSite->AddParam(&Anchor,ETbl,IDProcessed,VolumeProcessed[IDVolProc_On],1);
										IDT_NeuronLocalSite->AddParam(&Anchor,ETbl,IDState,VolumeProcessed[IDVolProc_Error]);
									}
									break;
								}
							}
							IDT_NeuronLocalSite->UnLockEl(&Anchor,ETbl);
						}
						if (Cli->Cli.DomainList)
							tIDEl=Cli->Cli.DomainList->Next(FSL8);
						else
							tIDEl=NULL;
					}
				}
			}
			else
			{
				if (Cli)
				{
					if ((Cli->Cli.Host[0]!=0)&&(Cli->Addr!=SInt->INetAdrLAN))
					{
						if (Cli->Cli.DomainList)
						{
							FindStrList8	FSL8;
							ID_Elem*	tIDEl=Cli->Cli.DomainList->Begin(FSL8);
							while (tIDEl)
							{
								ETbl=IDT_NeuronLocalSite->FindNested(&Anchor,AcsMd_Read,N_Domain,tIDEl->Buff,N_Host,Cli->Cli.ShortHost,NULL);
								if (ETbl)
								{
									char*	tVol;
									if (IDT_NeuronLocalSite->GetParam(&Anchor,ETbl,IDState,tVol)!=-1)
									{
										if (strcmp(tVol,"Off")!=0)
										{
											//							IDT_NeuronLocalSite->AddParam(&Anchor,ETbl,IDPersonalID,UInfo.PersID,1);
											IDT_NeuronLocalSite->AddParam(&Anchor,ETbl,IDProcessed,VolumeProcessed[IDVolProc_On],1);
											IDT_NeuronLocalSite->AddParam(&Anchor,ETbl,IDState,VolumeProcessed[IDVolProc_Off]);
										}
									}
									IDT_NeuronLocalSite->UnLockEl(&Anchor,ETbl);
								}
								tIDEl=Cli->Cli.DomainList->Next(FSL8);
							}
						}
					}
				}
			}

			if (++CntHost>255) CntHost=0;

			if (Timer(CntT8,10000))
			{
				Cli=(El_CliInt*)SInt->CliLAN.Find((UCHAR*)&SInt->INetAdrLAN,sizeof(ULONG));
				if ((Cli!=0)&&(Cli->Cli.DomainList))
				{
					FindStrList8	FSL8;
					ID_Elem*	t_El=Cli->Cli.DomainList->Begin(FSL8);
					while (t_El!=NULL)
					{
						ETbl=IDT_NeuronLocalSite->FindNested(&Anchor,AcsMd_Read,N_Domain,t_El->Buff,N_Host,ShortHostName,NULL);
						if (ETbl)
						{
							char*	tVol;
							if (IDT_NeuronLocalSite->GetParam(&Anchor,ETbl,IDPersonalID,tVol)!=-1)
							{
								INT64	tPersID=*(INT64*)tVol;
								if ((tPersID!=UInfo.PersID)&&(UInfo.PersID!=0))
									IDT_NeuronLocalSite->AddParam(&Anchor,ETbl,IDPersonalID,UInfo.PersID);
								else
								{
									if (Timer(CntT6,60000))
										IDT_NeuronLocalSite->AddParam(&Anchor,ETbl,IDPersonalID,UInfo.PersID);
								}
							}
							else
								IDT_NeuronLocalSite->AddParam(&Anchor,ETbl,IDPersonalID,UInfo.PersID);
							if (IDT_NeuronLocalSite->GetParam(&Anchor,ETbl,IDState,tVol)!=-1)
							{
								switch (SInt->Status)
								{
								case 0:
									if (strcmp(tVol,"Off")!=0)
									{
										IDT_NeuronLocalSite->AddParam(&Anchor,ETbl,IDPersonalID,UInfo.PersID,1);
										IDT_NeuronLocalSite->AddParam(&Anchor,ETbl,IDProcessed,VolumeProcessed[IDVolProc_On],1);
										IDT_NeuronLocalSite->AddParam(&Anchor,ETbl,IDState,VolumeProcessed[IDVolProc_Off]);
									}
									break;
								case 1:
									if (strcmp(tVol,"Setup")!=0)
									{
										IDT_NeuronLocalSite->AddParam(&Anchor,ETbl,IDPersonalID,UInfo.PersID,1);
										IDT_NeuronLocalSite->AddParam(&Anchor,ETbl,IDProcessed,VolumeProcessed[IDVolProc_On],1);
										IDT_NeuronLocalSite->AddParam(&Anchor,ETbl,IDState,VolumeProcessed[IDVolProc_Setup]);
									}
									break;
								case 2:
									if (strcmp(tVol,"On")!=0)
									{
										IDT_NeuronLocalSite->AddParam(&Anchor,ETbl,IDPersonalID,UInfo.PersID,1);
										IDT_NeuronLocalSite->AddParam(&Anchor,ETbl,IDProcessed,VolumeProcessed[IDVolProc_On],1);
										IDT_NeuronLocalSite->AddParam(&Anchor,ETbl,IDState,VolumeProcessed[IDVolProc_On]);
									}
									break;
								case 3:
									if (strcmp(tVol,"Error")!=0)
									{
										IDT_NeuronLocalSite->AddParam(&Anchor,ETbl,IDPersonalID,UInfo.PersID,1);
										IDT_NeuronLocalSite->AddParam(&Anchor,ETbl,IDProcessed,VolumeProcessed[IDVolProc_On],1);
										IDT_NeuronLocalSite->AddParam(&Anchor,ETbl,IDState,VolumeProcessed[IDVolProc_Error]);
									}
									break;
								}
							}
							else
							{
								IDT_NeuronLocalSite->AddParam(&Anchor,ETbl,IDPersonalID,UInfo.PersID,1);
								IDT_NeuronLocalSite->AddParam(&Anchor,ETbl,IDProcessed,VolumeProcessed[IDVolProc_On],1);
								IDT_NeuronLocalSite->AddParam(&Anchor,ETbl,IDState,VolumeProcessed[IDVolProc_On]);
							}
							IDT_NeuronLocalSite->UnLockEl(&Anchor,ETbl);
						}
						else
						{
							//				if (CntHost==1)
							if (System_On==1)
							{
								ETbl=IDT_NeuronLocalSite->AttainNested(&Anchor,"%s%s%s",N_Domain,t_El->Buff,N_Host,ShortHostName,-1);
								IDT_NeuronLocalSite->UnLockEl(&Anchor,ETbl);
							}
						}
						if (Cli)
							t_El=Cli->Cli.DomainList->Next(FSL8);	//!!!!!!!!!!!!!!!!!!!
						
					}
				}
			}
		}
// 		Telemetr(this,"__CntIPList0",IPList.CntIP[0]);	
// 		Telemetr(this,"__CntIPList1",IPList.CntIP[1]);	
// 		Telemetr(this,"__CntIPList2",IPList.CntIP[2]);	
// 		Telemetr(this,"__CntIPList3",IPList.CntIP[3]);	
// 		Telemetr(this,"__CntIPList4",IPList.CntIP[4]);	
// 		Telemetr(this,"__CntIPList5",IPList.CntIP[5]);	
// 		Telemetr(this,"__CntIPList6",IPList.CntIP[6]);	
// 		Telemetr(this,"__CntIPList7",IPList.CntIP[7]);	
// 		Telemetr(this,"__CntIPList8",IPList.CntIP[8]);	
// 		Telemetr(this,"__CntIPList9",IPList.CntIP[9]);	
	}
//	tTime1=GetNClk();
//	MyUnLock(__FILE__,__LINE__,&Fl_RE,this);
//	CalcTable();
//	MyLock(__FILE__,__LINE__,&Fl_RE,this);
	

// 	if (!Timer(CntT3,250))
// 		goto _ret1;
	FileTime		tFTime,tFTime2;
	GetRealTime(&tFTime.FTime);
	if ((tFTime.Time-TimeNetLock.Time)>(INT64)(10*(INT64)KtimeM))
	{
		Fl_NetLock=0;
	}
	else
	{
		Fl_NetLock=1;
	}

	if (UInfo.OwnerID!=UserID)
	{
		if (UserID==-1)
		{
			UInfo.OwnerID=-1;
		}
		else
		{
			// Send LogIn command
			IDInfo*	IDI=GetNewIDI(this);
			InitCommIDI(IDI);
			AddressInfo	AdrI;
			IDI->AdrInfo=&AdrI;
			strcpy(IDI->AdrInfo->DestinAI.UnitName,N_Service);
			
			IDI->pPList->SetParam(IDUserLogin,UserLogin,1+strlen(UserLogin),this);
			IDI->pPList->SetParam(IDUserPersID,(void*)&UserID,sizeof(UserID),this);

			IDI->Command=Command_C_UserLogIn;
			IDI->NumPack++;
			SendIDI(IDI,this);
			DeleteIDI(IDI);	
			UInfo.OwnerID=UserID;
			ModifUnit(this);
		}
	}
	if (UInfo.OwnerID!=-1)
	{
		if (CntGUIActivities!=RCntGUIActivities)
		{
			RCntGUIActivities=CntGUIActivities;
			LastUserActivitiesTime.Time=tFTime.Time;
		}
		else
		{
			if (M_Cfg->MData.Flag_AutoLogon!=0)
			{
				if (tFTime.Time-LastUserActivitiesTime.Time>10*(INT64)KtimeM)
				{
					Logoff();
				}
			}
		}
	}

_ret0:
	rpt=0;
//    IDI_IS->Idle();

//	if ((Mt_IDIRepl)&&(Fl_Repl==1))
//		Mt_IDIRepl->DataFF(1,NULL,1);

	//  [6/22/2012 Igor]
//	if (Timer(CntST,60*60000))	//Êàæäûå 1 ÷àñ
//	{
//#ifdef DEBUG		
//		OutLog("Send GetUI IdleFunc:TimerCntST",this,"SendGetUI");
//#endif
//		Srv->SendGetUnitInfo();
//		SaveCfg();
//	}
	// Ðàñ÷åò ïàìÿòè
	MEMORYSTATUS stat;				
	double		MemNeuronUsage;
	GlobalMemoryStatus (&stat);
	PROCESS_MEMORY_COUNTERS pmc;
	HANDLE hProcess;
	hProcess=GetCurrentProcess();
	GetProcessMemoryInfo(hProcess,&pmc,sizeof(pmc));
	CloseHandle(hProcess);
	//	ReadProcessMemory(hProcess);

#ifdef _WIN64
	MemUsage=100.0*(double)((double)(stat.dwTotalPhys-stat.dwAvailPhys)/(double)(stat.dwTotalPhys));//+(stat.dwTotalVirtual-stat.dwAvailVirtual)/(double)stat.dwTotalPhys);
	MemNeuronUsage=100.0*(double)((double)(pmc.WorkingSetSize)/(double)(stat.dwTotalPhys));
#else
	MemUsage=100.0*((double)(pmc.PagefileUsage+pmc.WorkingSetSize)/2400000000.0);
	MemNeuronUsage=100.0*(double)((double)(pmc.WorkingSetSize)/2400000000.0);
#endif
	if (MemUsage>=100.0) MemUsage=100;
	if (MemNeuronUsage>=100.0) MemNeuronUsage=100;
	Telemetr(this,"_MemoryUsed(%)",MemUsage);
	Telemetr(this,"_MemoryNeuron(%)",MemNeuronUsage);
	Telemetr(this,"_MemoryDifference(%)",MemUsage-MemNeuronUsage);
	//	Srv->Telemetr(this,"GlobVirtMem(kb)",(double)(stat.dwTotalVirtual-stat.dwAvailVirtual)/1024);
/*
	// Ðàñ÷åò ïàìÿòè
	MEMORYSTATUS stat;				
	GlobalMemoryStatus (&stat);
	PROCESS_MEMORY_COUNTERS pmc;
	HANDLE hProcess;
	hProcess=GetCurrentProcess();
	GetProcessMemoryInfo(hProcess,&pmc,sizeof(pmc));
	CloseHandle(hProcess);
//	ReadProcessMemory(hProcess);

#ifdef _WIN64
	MemUsage=100.0*(double)((double)(stat.dwTotalPhys-stat.dwAvailPhys)/(double)(stat.dwTotalPhys));//+(stat.dwTotalVirtual-stat.dwAvailVirtual)/(double)stat.dwTotalPhys);
#else
	MemUsage=100.0*((double)(pmc.PagefileUsage+pmc.WorkingSetSize)/2400000000.0);
#endif
	if (MemUsage>=100.0) MemUsage=100;
	Telemetr(this,"_MemoryUsed(%)",MemUsage);
//	Srv->Telemetr(this,"GlobVirtMem(kb)",(double)(stat.dwTotalVirtual-stat.dwAvailVirtual)/1024);
*/
// Êîíòðîëü ðàñïðåäåëåíèÿ àëëîêàöèé IDInfo
	Telemetr(this,"_NumActIDI",ActIDIList->NumElem);
	Telemetr(this,"_NumDelIDI",DelIDIList->NumElem);
// Êîíòðîëü àëëîêèðîâàííîé ïàìÿòè
	Telemetr(this,"_SrvAllocMem",SizeAllocMem);
	Telemetr(this,"_DBGC.AllocMem",DBGC.MemSize);
	Telemetr(this,"_DBGC.El_Tab",DBGC.Cnt_El_Tab);
	Telemetr(this,"_DBGC.IDInfo",DBGC.Cnt_IDI_Info);
	Telemetr(this,"_DBGC.IDList8",DBGC.Cnt_IDList8);
	Telemetr(this,"_DBGC.pList",DBGC.Cnt_pList);
// Ñîñòîÿíèå ôëàæêà On
	Telemetr(this,"_System_On",System_On);
	Telemetr(this,"_Fl_NetLock",Fl_NetLock);
	Telemetr(this,"_Fl_HardLock",Fl_HardLock);

	if (System_On==0)
	{
		if (Fl_Repl==0)
			Fl_TimeSyn=1;
		if (Fl_TimeSyn!=0)
		{
			if (Fl_ReadyRepl<50)
			{
				if (Fl_ReadyRepl==10)
				{
					El_Tab*		ElTt=IDT_NeuronGlobal->GetElTab(&Anchor,TabNeuronGlobal,3);
					IDT_NeuronGlobal->QueryEl(TabNeuronGlobal);
					IDT_NeuronGlobal->UnLockEl(&Anchor,ElTt);
					ElTt=IDT_NeuronLocalCustomer->GetElTab(&Anchor,TabNeuronLocalCustomer,3);
					IDT_NeuronGlobal->QueryEl(TabNeuronLocalCustomer);
					IDT_NeuronLocalCustomer->UnLockEl(&Anchor,ElTt);
					ElTt=IDT_NeuronLocalSite->GetElTab(&Anchor,TabNeuronLocalSite,3);
					IDT_NeuronGlobal->QueryEl(TabNeuronLocalSite);
					IDT_NeuronLocalSite->UnLockEl(&Anchor,ElTt);
				}
				if (Fl_ReadyRepl==20)
				{
//					El_Tab*		ETbl;
					El_CliInt*	Cli=(El_CliInt*)SInt->CliLAN.Find((UCHAR*)&SInt->INetAdrLAN,sizeof(ULONG));
					if (Cli)
					if (Cli->Cli.DomainList)
					{
						FindStrList8	FSL8;
						ID_Elem*	t_El=Cli->Cli.DomainList->Begin(FSL8);
						while (t_El!=NULL)
						{
							ETbl=IDT_NeuronLocalSite->FindNested(&Anchor,AcsMd_Read,N_Domain,t_El->Buff,N_Host,ShortHostName,NULL);
							if (ETbl)
							{
								IDT_NeuronLocalSite->AddParam(&Anchor,ETbl,IDState,VolumeProcessed[IDVolProc_Setup]);
								IDT_NeuronLocalSite->UnLockEl(&Anchor,ETbl);
							}
							else
							{
								IDT_NeuronGlobal->QueryEl(TabNeuronGlobal);
								IDT_NeuronGlobal->QueryEl(TabNeuronLocalCustomer);
								IDT_NeuronGlobal->QueryEl(TabNeuronLocalSite);
							}
							t_El=Cli->Cli.DomainList->Next(FSL8);
						}
					}
				}
				if (Fl_ReadyRepl==30)
				{
//					El_Tab*		ETbl;
					InitUnit(this);
					El_CliInt*	Cli=(El_CliInt*)SInt->CliLAN.Find((UCHAR*)&SInt->INetAdrLAN,sizeof(ULONG));
					if (Cli)
					if (Cli->Cli.DomainList)
					{
						FindStrList8	FSL8;
						ID_Elem*	t_El=Cli->Cli.DomainList->Begin(FSL8);
						while (t_El!=NULL)
						{
							ETbl=IDT_NeuronLocalSite->FindNested(&Anchor,AcsMd_Read,N_Domain,t_El->Buff,N_Host,ShortHostName,NULL);
							if (ETbl)
							{
								IDT_NeuronLocalSite->AddParam(&Anchor,ETbl,IDPersonalID,UInfo.PersID,1);
								IDT_NeuronLocalSite->AddParam(&Anchor,ETbl,IDProcessed,VolumeProcessed[IDVolProc_On],1);
								IDT_NeuronLocalSite->AddParam(&Anchor,ETbl,IDState,VolumeProcessed[IDVolProc_Setup]);
								IDT_NeuronLocalSite->UnLockEl(&Anchor,ETbl);
							}
							else
							{
								IDT_NeuronGlobal->QueryEl(TabNeuronGlobal);
								IDT_NeuronGlobal->QueryEl(TabNeuronLocalCustomer);
								IDT_NeuronGlobal->QueryEl(TabNeuronLocalSite);
							}
							t_El=Cli->Cli.DomainList->Next(FSL8);
						}

					}
				}
				if (Fl_ReadyRepl==40)
				{
					MyLock(__FILE__,__LINE__,&FlSimCT,this);//Fl_MTSI=0;
//					MyLock(__FILE__,__LINE__,&ElTL->IDT->FlSim,this);
					if (NetCommLAN)
					{
						DeleteDecoder(NetCommLAN);NetCommLAN=NULL;
					};
// 					if (NetCommWAN)
// 					{
// 						DeleteDecoder(NetCommWAN);NetCommWAN=NULL;
// 					};
					if (Net_Flag==1)
					{
//						DeleteDecoder(NetMen);NetMen=NULL;
						Net_Flag=0;
					};
// 					IDT_NeuronGlobal=OpenTable(this,TabNeuronGlobal);
// 					IDT_NeuronLocalCustomer=OpenTable(this,TabNeuronLocalCustomer);
// 					IDT_NeuronLocalSite=OpenTable(this,TabNeuronLocalSite);
					DLLL->Init();
					if (Net_Flag!=1)
					{
// 						NetMen=NewDecod("NetService",this);
// 						if (NetMen!=NULL)
// 						{
// //							strcpy(NetMen->UInfo.DomainName,UInfo.DomainName);
// 							NetMen->AddDecLink(this,0,1005);
// 							NetMen->Ext_Mt=0;
// 							NetMen->Srv=this;
// 							NetMen->StartEx();
							Net_Flag=1;
// 						}
// 						else
// 							SInt->FatalError=1;
					}
					if ((NetCommLAN==0)&&(Srv->M_Cfg->MData.Fl_LAN!=0))
					{
						NetCommLAN=NewDecod("NetComm",this);
						if (NetCommLAN)
						{
//							strcpy(NetCommLAN->UInfo.DomainName,UInfo.DomainName);
							NetCommLAN->ParamEx("NetType","LAN");
							NetCommLAN->ParamEx("PortSrv",TCPPort);
							NetCommLAN->AddDecLink(this,0,1003);
							NetCommLAN->StartEx();
						}
					}
					MyUnLock(__FILE__,__LINE__,&FlSimCT,this);//Fl_MTSI=0;
//					MyUnLock(__FILE__,__LINE__,&ElTL->IDT->FlSim,this);
// 					if ((NetCommWAN==0)&&(Srv->M_Cfg->MData.Fl_WAN!=0))
// 					{
// 						NetCommWAN=NewDecod("NetComm");
// 						if (NetCommWAN)
// 						{
// 							NetCommWAN->Param("NetType","WAN");
// 							NetCommWAN->Param("PortSrv","803");
// 							NetCommWAN->AddDecLink(this,0,1004);
// 							NetCommWAN->Start();
// 						}
// 					}
// 					//!!!!!!!!!!!!
					{
						IDT_NeuronGlobal->QueryEl(TabNeuronGlobal);
						IDT_NeuronGlobal->QueryEl(TabNeuronLocalCustomer);
						IDT_NeuronGlobal->QueryEl(TabNeuronLocalSite);
					}
				}
				Fl_ReadyRepl++;
			}
			else
			{
//				
				El_Tab*		ETbl;
				//				dtTime1=dtTime2=dtTime3=dtTimeAll=0;
// 				ETbl=IDT_NeuronLocalSite->FindNested(&Anchor,AcsMd_Read,N_Domain,UInfo.DomainName,N_Role,RoleName,NULL);
// 				if(ETbl==NULL)
// 					ETbl=IDT_NeuronLocalSite->AttainNested(&Anchor,"%s%s%s",N_Domain,UInfo.DomainName,N_Role,RoleName,-1);
				System_On=1;
				SData->Fl_StateApp=2;
/*
				if (CustomerInfo==NULL)
					{
						// Выдать предупреждение о выходе их программы !!!

						SData->Fl_StateApp=4;
						SData->Fl_StateSmartSt=3;
					}
					else
					{
						if (CustomerInfo->Flag==0)
						{
							SData->Fl_StateApp=4;
							SData->Fl_StateSmartSt=3;
						}
					};
*/
				IDInfo*	IDI=GetNewIDI(this);
				InitCommIDI(IDI);
				AddressInfo	AdrI;
				IDI->AdrInfo=&AdrI;
//				IDI->pPList->SetParam(IDHostName,"*",2,this);
//				IDI->pPList->SetParam(IDUnitName,N_Service,9,this);
#ifdef DEBUG
				char ccStr[512];
				sprintf(ccStr,"Send GetUI IdleFunc:Fl_ReadyRepl;%d",Fl_ReadyRepl);
				OutLog(ccStr,this,"SendGetUI");
#endif
				strcpy(IDI->AdrInfo->DestinAI.UnitName,N_Service);
				IDI->Command=Command_C_GetUnitInfo;
				IDI->NumPack++;
				SendIDI(IDI,this);
				DeleteIDI(IDI);	
//				strcpy(UInfo.Mode,"Active");
				El_CliInt*	Cli=(El_CliInt*)SInt->CliLAN.Find((UCHAR*)&SInt->INetAdrLAN,sizeof(ULONG));
				if (Cli)
				if (Cli->Cli.DomainList)
				{
					FindStrList8	FSL8;
					ID_Elem*	t_El=Cli->Cli.DomainList->Begin(FSL8);
					while (t_El!=NULL)
					{
						ETbl=IDT_NeuronLocalSite->FindNested(&Anchor,AcsMd_Read,N_Domain,t_El->Buff,N_Host,ShortHostName,NULL);
 						if (ETbl==NULL)
 							ETbl=IDT_NeuronLocalSite->AttainNested(&Anchor,"%s%s%s",N_Domain,t_El->Buff,N_Host,ShortHostName,-1);
						if (ETbl)
						{
							IDT_NeuronLocalSite->AddParam(&Anchor,ETbl,IDPersonalID,UInfo.PersID,1);
							IDT_NeuronLocalSite->AddParam(&Anchor,ETbl,IDProcessed,VolumeProcessed[IDVolProc_On],1);
							IDT_NeuronLocalSite->AddParam(&Anchor,ETbl,IDState,VolumeProcessed[IDVolProc_On]);
							IDT_NeuronLocalSite->UnLockEl(&Anchor,ETbl);
						}
						t_El=Cli->Cli.DomainList->Next(FSL8);	//!!!!!!!!!!!!!!!!!!!!!!!!!!
					}
				}
//				ETbl=IDT_Neuron->AttainNested(&Anchor,"%s%s",N_Host,HostName,-1);
//				El_Tab*		ETbl1=IDT_Neuron->GetElTab(&Anchor,0x282d000000b7ba9d);
//				El_Tab*		ETbl2=IDT_Neuron->FindNested(&Anchor,AcsMd_Read,"Domain","Sigmatech.local",NULL);
//				IDT_Neuron->MoveET(this,ETbl1,ETbl2);
//				IDT_Neuron->UnLockEl(&Anchor,ETbl2);
//				IDT_Neuron->UnLockEl(&Anchor,ETbl1);
//				ITest.InternalTest();

// 				if(LUinfo.Prog_WSAAutoStart==1)
// 				{
// 					ETbl=IDT_NeuronLocalSite->FindNested(&Anchor,AcsMd_Read,N_Domain,MainDomainName,N_Host,ShortHostName,N_Role,N_AdministratorWorkstation,NULL);
// 					if(ETbl)
// 					{
// 						IDT_NeuronLocalSite->AddParam(&Anchor,ETbl,IDProcessed,VolumeProcessed[IDVolProc_On],1);
// 						IDT_NeuronLocalSite->UnLockEl(&Anchor,ETbl);
// 					}
// 					else
// 					{
// 						ETbl=IDT_NeuronLocalSite->AttainNested(&Anchor,"%s%s%s%s%s",N_Domain,MainDomainName,N_Host,ShortHostName,N_Role,N_AdministratorWorkstation,-1);
// 						IDT_NeuronLocalSite->AddParam(&Anchor,ETbl,IDProcessed,VolumeProcessed[IDVolProc_On],1);
// 						IDT_NeuronLocalSite->UnLockEl(&Anchor,ETbl);
// 					}
// 				}
			}
		}
	}
	else
	{
		Fl_ReadyRepl=0;
//		if (Fl_TimeSyn==0)
//		if ((IDI_IS->ReadyRepl==0)||(Fl_TimeSyn==0))
//		if (IDI_IS->ReadyRepl==0)
//		{
//			System_On=0;
//			Service->CloseUnit(Service);
//		}
	}

	// ----------------------------------
	// Ñèñòåìà âû÷èñëåíèÿ òî÷íîãî âðåìåíè
	// ----------------------------------
	FileTime		FTime;
//	tTime2=GetNClk();
	
//	LARGE_INTEGER	CntTime;//,FreqTime;
	if (SInt)
	if ((MyNum==SInt->NumLANSrv)&&(Fl_ExtSin==0))
	{
		Fl_TimeSyn=1;
		CntT2=GetNClk();
		GetSystemTimeAsFileTime(&FTime.FTime);
		RealTime+=(INT64)((CntT2-NClickOld)*Fcpu_1);
		NClickOld=CntT2;
		tErr=(double)(RealTime-FTime.Time);
		Err[PoiNTF]=tErr;
		if (++PoiNTF>=4*NumTF) PoiNTF=0;
		VarErr=0.0001*MedFlt(Err,4*NumTF);
		if (VarErr>0)
		{
			if (VarErr>15500)
			{
				odFcpu+=100*(VarErr-15500);
				if (VarErr>20000)
				{
					odFcpu+=2000*(VarErr-15500);
				}
				RealTime-=(INT64)((VarErr-15500)*10);
			}
			else
			{
				if (VarErr>1000)
				{
					odFcpu+=1*VarErr;
					RealTime-=(INT64)(VarErr-1000);
				}
				else
				{
					odFcpu+=0.01*VarErr;
					RealTime-=(INT64)VarErr;
				}
			}
		}
		else
		{
			if (VarErr<-15500)
			{
				odFcpu+=100*(15500+VarErr);
				if (VarErr<-20000)
				{
					odFcpu+=2000*(15500+VarErr);
				}
				RealTime-=(INT64)((15500+VarErr)*10);
			}
			else
			{
				if (VarErr<-1000)
				{
					odFcpu=1*VarErr;
					RealTime-=(INT64)(1000+VarErr);
				}
				else
				{
					odFcpu=0.01*VarErr;
					RealTime-=(INT64)VarErr;
				}

			}
		}
		if (++CntTK>4*NumTF)
		{
			oNClk=NClickOld;
			CntTK=0;
			VardifErr=0.99*VardifErr+0.01*(1e+12*(VarErr-OldErr)/(oNClk-oldNClk));
//			VardifErr=1e+12*(VarErr-OldErr)/(oNClk-oldNClk);
//			difErr[PoiNTF2]=1e+12*(VarErr-OldErr)/(oNClk-oldNClk);
//			if (++PoiNTF2>=NumTF) PoiNTF2=0;
//			VardifErr=MedFlt(difErr,NumTF);
			if (fabs(VarErr)>4500)
			{
				if (VardifErr>1000000)
					VardifErr=1000000; 
				if (VardifErr<-1000000)
					VardifErr=-1000000; 
			}
			else
			{
				if (VardifErr>30000)
					VardifErr=30000; 
				if (VardifErr<-30000)
					VardifErr=-30000; 
			}
			if (VardifErr*OldDErr<0)
			{
				sDErr=0;
				KdifCnt=0;			
			}
			else
			{
				if (++KdifCnt<10)
				{
					sDErr=0.99*sDErr+0.01*VardifErr;
				}
				else
				{
					sDErr=0.9*sDErr+0.1*VardifErr;
				}
			}
			if (fabs(VarErr)>30000)
			{
				dFcpu=(fabs(sDErr)*sDErr+10*VarErr/fabs(VarErr));
			}
			else
			{
				dFcpu=(fabs(sDErr)*sDErr+0.01*VarErr);
			}
			if (odFcpu*dFcpu<0) odFcpu=dFcpu*0.01;
			if (fabs(odFcpu)<fabs(dFcpu))
			{
				odFcpu=odFcpu*0.9+dFcpu*0.1;
			}
			else
			{
				odFcpu=odFcpu*0.5+dFcpu*0.5;
			}
			if (fabs(VarErr)>45000)
			{
				if (odFcpu>100000) odFcpu=55000+fabs(VarErr);
				if (odFcpu<-100000) odFcpu=-55000-fabs(VarErr);
			}
			else
			{
				if (odFcpu>100000) odFcpu=100000;
				if (odFcpu<-100000) odFcpu=-100000;
			}
			Fcpu+=(VarErr*0.1);
//				odFcpu;
			if (Fcpu<100000000)
				Fcpu=100000000;
			if (Fcpu>10000000000)
				Fcpu=10000000000;
			Fcpu_1=10000000.0/Fcpu;
			OldErr=VarErr;
			OldDErr=VardifErr;
			Telemetr(this,"_sTimeErr,ms",sDErr);
			Telemetr(this,"_dFcpuErr,Hz",odFcpu);
			Telemetr(this,"_TimeErr,ms",VarErr);
			Telemetr(this,"_Fcpu",Fcpu);
			Telemetr(this,"_difErr",VardifErr);
			oldNClk=oNClk;
		}
	}
	// ----------------------------------
	if (SInt->NumLANSrv==0)
		Fl_TimeSyn=1;

	// ----------------------------------
	// ----------------------------------
    SInt->OnLine();
// 	if (CntV>0)
// 				goto _ret1;	// Return !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
// 	CntV++;
// 	if (CntV>1)
// 	{
// 		CntV--;
// 				goto _ret1;	// Return !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
// 	}
//	tTime1=GetNClk();
	if (++CDiv>10)
	{

//		Chron->Measuring(0,2);
		IDL_Log->Idle();
//		StrCh=Chron->Measuring(1,2);
//		Telemetr(this,"Log_t_max,s",StrCh->t_max,0);
//		Telemetr(this,"Log_T_max,s",StrCh->T_max,0);
//		Telemetr(this,"Log_t_average,s",StrCh->t_average,0);
//		Telemetr(this,"Log_T_average,s",StrCh->T_average,0);
//		Telemetr(this,"Log_Porocity,%",StrCh->porosity,0);
		CDiv=0;
		if (Timer(CntT4,60000))					// !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
		{
			if ((SInt->NumLANSrv>0)&&(System_On))
			{
				char	tBf[256];
				if (CDiv3++>5)
				{
					if ((M_Cfg->MData.Fl_SendUpD)&&(SInt->NumLANSrv!=SInt->MyLANNum))
					{
						
						IDInfo*	IDI=GetNewIDI(this);
						El_CliInt*	Cli=(El_CliInt*)SInt->CliLAN.Find((UCHAR*)&SInt->NumLANSrv,sizeof(ULONG));
						if ((Cli)&&(MainFileQwe->NumElem==0))
						{
							InitCommIDI(IDI);
							AddressInfo	AdrI;
							IDI->AdrInfo=&AdrI;
//							IDI->pPList->SetParam(IDHostName,Cli->Cli.ShortHost,strlen(Cli->Cli.ShortHost)+1,this);
//							IDI->pPList->SetParam(IDUnitName,N_Service,9,this);
							strcpy(IDI->AdrInfo->DestinAI.HostName,Cli->Cli.ShortHost);
							strcpy(IDI->AdrInfo->DestinAI.UnitName,N_Service);
							IDI->Command=Command_C_SendMainTime;
							IDI->NumPack++;
							IDI->Buf=(UCHAR*)pMod;
							IDI->Cnt=(SizeMainFiles)*sizeof(Module);	// 1+ ýòî internal method
							SendIDI(IDI,this);
							sprintf(tBf,"SendMainTime To Host=%s",Cli->Cli.ShortHost);
							OutLog(tBf,this,"UpDate");
						}
						DeleteIDI(IDI);	
					}
					CDiv3=0;
//				}
//				if (CDiv3==2)
//				{
					if ((M_Cfg->MData.Fl_SendUpD)&&(SInt->NumLANSrv!=SInt->MyLANNum))
					{
						
						IDInfo*	IDI=GetNewIDI(this);
						El_CliInt*	Cli=(El_CliInt*)SInt->CliLAN.Find((UCHAR*)&SInt->NumLANSrv,sizeof(ULONG));
						if ((Cli)&&(DllFileQwe->NumElem==0))
						{
							InitCommIDI(IDI);
							AddressInfo	AdrI;
							IDI->AdrInfo=&AdrI;
//							IDI->pPList->SetParam(IDHostName,Cli->Cli.ShortHost,strlen(Cli->Cli.ShortHost)+1,this);
//							IDI->pPList->SetParam(IDUnitName,N_Service,9,this);
							strcpy(IDI->AdrInfo->DestinAI.HostName,Cli->Cli.ShortHost);
							strcpy(IDI->AdrInfo->DestinAI.UnitName,N_Service);
							IDI->Command=Command_C_SendDllTime;
							IDI->NumPack++;
							IDI->Buf=(UCHAR*)DLLL->pMod;
							IDI->Cnt=(1+DLLL->rModSize)*sizeof(Module);	// 1+ ýòî internal method
							SendIDI(IDI,this);
							sprintf(tBf,"SendDllTime To Host=%s",Cli->Cli.ShortHost);
							OutLog(tBf,this,"UpDate");
						}
						DeleteIDI(IDI);	
					}
					
				}
			}

//			char		BufHasp[4096];
			if (DelIDIList->NumElem)
			{
				ID_ElemIDI*	TmpIDE;
				int	tNum=DelIDIList->NumElem/100+1;	//
				for (size_t sti=0;sti<tNum;sti++)
				{
					FindStrList8	FSL8;
					TmpIDE=(ID_ElemIDI*)DelIDIList->Begin(FSL8);
					if (TmpIDE)
						DelIDIList->Del(TmpIDE);
				}	
			}
			Srv->ModifUnit(Srv);
			if (Srv->Timer(TimGUI,111000))
			{

#ifdef DEBUG		
			OutLog("Send GetUI IdleFunc:TimerCntT4",this,"SendGetUI");
#endif
			Srv->SendGetUnitInfo();
			}
			IDInfo*	IDI=GetNewIDI(this);
			InitCommIDI(IDI);
			AddressInfo	AdrI;
			IDI->AdrInfo=&AdrI;
//			IDI->pPList->SetParam(IDHostName,"*",2,this);
//			IDI->pPList->SetParam(IDUnitName,N_Service,9,this);
			strcpy(IDI->AdrInfo->DestinAI.UnitName,N_Service);
			IDI->Command=Command_C_GetLCond;
			IDI->NumPack++;
			SendIDI(IDI,this);
			DeleteIDI(IDI);	
			if (Fl_HardLock==1)
			{
				CopyLic(&LCond);
				if (M_Cfg->MData.CustomerID!=LCond.CustomerID)
				{
					M_Cfg->MData.CustomerID=LCond.CustomerID;
					if ((SData->Fl_StateSmartSt==0) && (M_Cfg->MData.Fl_RunSmSt))
					{
						PROCESSENTRY32 pEntry;
						if(!FindProcess(_T("SmartStarter.exe"),&pEntry))
						{
							HWND hwnd=NULL;
							char	FullPath[256];
							sprintf(FullPath,"%s\\SmartStarter.exe", M_Cfg->MData.Path);
							HINSTANCE  rcc=ShellExecuteA(hwnd, "open",FullPath, NULL, M_Cfg->MData.Path, SW_SHOWNORMAL);
						}
					}
					SData->Fl_StateApp=3;
				}
				CustomerInfo=GetCustomerInfo(M_Cfg->MData.CustomerID);
				CopyLicUnit(&LUinfo);
				GetRealTime(&TimeNetLock.FTime);
			}
		}
//			IDI_IS->ReLoadCfg(NameUniCfg);
		if (SInt->Fl_ReConnect==1)
		{
//			MyLock(__FILE__,__LINE__,&Fl_RE9,this);
			for (i=0;i<NumSICli;i++)
			{
				if (CliList[i].Fl==1)
				{
//					MyLock(__FILE__,__LINE__,&Fl_RE1,this);
					char	tBuf[80];
					if (CliList[i].Mode==0)
					{
						PackNetObj		PNetObj;
						sprintf(tBuf,"CliSI%03d",i);
						strcpy(PNetObj.descriptor,tBuf);
						PNetObj.command=NetSockParam;
						if (CliList[i].Mode==0)
						{
							strcpy(PNetObj.param,"ServerIP");
							El_CliInt*	Cli=(El_CliInt*)SInt->CliLAN.Find((UCHAR*)&SInt->NumLANSrv,sizeof(ULONG));
							strcpy(PNetObj.vol,Cli->Cli.IPAddr);
							NS_DataFF(0,(UCHAR*)&PNetObj,sizeof(PNetObj));
						}
					}
//					MyUnLock(__FILE__,__LINE__,&Fl_RE1,this);
//					PNetObj.command=NetSockStart;
//					NetMenLAN->Data(0,(UCHAR*)&PNetObj,sizeof(PNetObj));
				};
			}
//			MyUnLock(__FILE__,__LINE__,&Fl_RE9,this);
			SInt->Fl_ReConnect=0;
		}
		if (++CDiv2>1000)
		{
			CDiv2=0;
			// ×èñòèòèòü ñïèñîê ñ ïàðàìåòðàìè
			El_Telem		*ElT;
			FileTime		tTime;
//			MyLock(__FILE__,__LINE__,&Fl_RE3,NULL);//Fl_MT=0;
			GetRealTime(&tTime.FTime);
			FindStrList8	FSL8;
			ElT=(El_Telem*)TraceLst->Begin(FSL8);
			while (ElT!=NULL)
			{
				if (tTime.Time-ElT->LWTime.Time>600000000)
				{
					TraceLst->Del(ElT);
//					ElT=(El_Telem*)TraceLst->Begin(FSL8);
				}
//				else
					ElT=(El_Telem*)TraceLst->Next(FSL8);
			}
//			MyUnLock(__FILE__,__LINE__,&Fl_RE3,NULL);//Fl_MT=0;
		}
/*		// Ïðîâåðêà SIM
		for (i=0;i<SIM->RLen;i++)
		{
			if (SIM->TSysCliLAN[i]->Fl)
			{
				int tNumHost=-1;
				int lip=strlen(SIM->TSysCliLAN[i]->IP);
				for (int j=lip;j>0;j--)
				{
					if (SIM->TSysCliLAN[i]->IP[j]=='.')
					{
						tNumHost=atoi(&SIM->TSysCliLAN[i]->IP[j+1]);
						break;
					}
				}
				if (tNumHost!=-1)
				{
					if ((SInt->CliLAN[tNumHost].fl==0)||(SInt->CliLAN[tNumHost].Command==SICom_Error))
					{
						SIM->TSysCliLAN[i]->Fl=0;
					};
				}
			}
		}
*/		// ------------
	}
//	tTime2=GetNClk();

// 	CntV--;
// 	GetRealTime(&tFTime2.FTime);
// 	if (tFTime2.Time-tFTime.Time<KtimeS)
// 	{
// 		if (rpt>0)
// 			goto _ret0;
// 	}
//_ret1:
//	StrCh=Chron->Measuring(1,0);
//	Telemetr(this,"Idle_t_max,s",StrCh->t_max,0);
//	Telemetr(this,"Idle_T_max,s",StrCh->T_max,0);
//	Telemetr(this,"Idle_t_average,s",StrCh->t_average,0);
//	Telemetr(this,"Idle_T_average,s",StrCh->T_average,0);
//	Telemetr(this,"Idle_Porocity,%",StrCh->porosity,0);
	
	UInfo.State[0]=0;
	FlagEnter=0;
//	tTime3=GetNClk();
	
	if ((System_On!=0)&&(Fl_Stop==0))
	{
		if (M_Cfg->MData.fErrView)
			if (pMyErr)
				pMyErr->DataFFEx(0,(uchar*)pMyErr,4);
	}
	Fl_Recurce--;
// 	tTime4=GetNClk();
// 	tTimeInt= (tTime4-tTime1)*Fcpu_1;
// 	if (tTimeInt>KtimeS)
// 	{
// 		tTimeInt/=KtimeS;
// 	};
// //	if (dtTime1<tTime2-tTime1)
// 		dtTime1=tTime2-tTime1;
// //	if (dtTime2<tTime3-tTime2)
// 		dtTime2=tTime3-tTime2;
// //	if (dtTime3<tTime4-tTime3)
// 		dtTime3=tTime4-tTime3;
// //	if (dtTimeAll<tTime4-tTime1)
// 		dtTimeAll=tTime4-tTime1;
// 	Telemetr(this,"Idle_t1,s",dtTime1,0);
// 	Telemetr(this,"Idle_t2,s",dtTime2,0);
// 	Telemetr(this,"Idle_t3,s",dtTime3,0);
// 	Telemetr(this,"Idle_tAll,s",dtTimeAll,0);
	//	dtTime1=dtTime2=dtTime3=dtTimeAll=0;

//	MyUnLock(__FILE__,__LINE__,&Fl_RE,this);
	return rrc;
};

void	Services::SendGetUnitInfo()
{
	IDInfo*	IDI=GetNewIDI(this);
	InitCommIDI(IDI);
	AddressInfo	AdrI;
	IDI->AdrInfo=&AdrI;
//	IDI->pPList->SetParam(IDHostName,"*",2,this);
//	IDI->pPList->SetParam(IDUnitName,N_Service,9,this);
	strcpy(IDI->AdrInfo->DestinAI.UnitName,N_Service);
	IDI->Command=Command_C_GetUnitInfo;
	IDI->NumPack++;
	SendIDI(IDI,this);
	DeleteIDI(IDI);	
}

char*	Services::FirstMode()
{	
	CountMode=0;
	if (Tsk!=NULL)
		if (Tsk->Max_Md>0)
			return	Tsk->Md[CountMode++].Name;
	return NULL;
};

char*	Services::NextMode()
{	
	if (Tsk!=NULL)
		if (Tsk->Max_Md>CountMode)
			return	Tsk->Md[CountMode++].Name;
	return NULL;
};

char*	Services::GetNameMode(int NumMode)
{
	if (Tsk!=NULL)
		if (Tsk->Max_Md>NumMode)
			return	Tsk->Md[NumMode].Name;
	return NULL;
}

void	Services::OutLog(char* pBuf,Decoder* PObj,char* Type)
{
	MyLock(__FILE__,__LINE__,&Fl_RE5,this);
	if (IDL_Log)
	{
		if (Type==NULL)
		{
	//		DWORD rc=WaitForSingleObject(hmtxlog,1000);
	//		if (rc==0)
			Log->Data(0,(UCHAR*)pBuf,strlen(pBuf));	// Ïî ïðîcüáå òðóäÿùèõñÿ
	//		ReleaseSemaphore(hmtxlog,1,NULL); 
		}
		else
		{
			El_Log*	tElL=(El_Log*)IDL_Log->Find((UCHAR*)Type,strlen(Type)+1);
			if (tElL==NULL)
			{
				char	tBuf[128];
				LogParam	LP;
				strcpy(LP.KeyVol,Type);
				LP.Fl=0;
				LP.TypeTime=TypeTime;// 
				sprintf(LP.NDay,"%d",NDayLog);
				BL_Log->AddElem((UCHAR*)&LP,sizeof(LP));
				tElL=new El_Log(&LP,PathLog,this);
				tElL->Log->Srv=this;
				IDL_Log->Add(tElL);
				sprintf(tBuf,"New Log Type=%s",Type);
				OutLog(tBuf);	// Çàïèñü â îñí ëîã
			}
			if (tElL->LPrm.Fl==0)
				tElL->OutLog(pBuf,PObj);
		}
	}
	MyUnLock(__FILE__,__LINE__,&Fl_RE5,this);
};

int	Services::LoadCfg()
{
	int	FH=-1;
	int RLen;
	UCHAR*		iLP;
	LogParam*	mLP;
	size_t	Cnt;
	int rc=0;
	char	tBuf[256];
	sprintf(tBuf,"%s\\Services.Cfg",MainPath);
	FH=open(tBuf,O_BINARY);
	if (FH!=-1)
	{
		RLen=read(FH,&Cfg,sizeof(SrvCfg));
		BL_Log->LoadFromFile(FH,sizeof(SrvCfg));
		close(FH);
		rc=BL_Log->FindFirst(iLP,Cnt);
		mLP=(LogParam*)iLP;
		while (rc!=0)
		{
			El_Log*	tElL=(El_Log*)IDL_Log->Find((UCHAR*)mLP->KeyVol,strlen(mLP->KeyVol)+1);
			if (tElL==NULL)
			{
				LogParam	LP;
				LP.TypeTime=TypeTime;// 
				sprintf(LP.NDay,"%d",NDayLog);
				strcpy(LP.KeyVol,mLP->KeyVol);
				LP.Fl=mLP->Fl;	// 
				sprintf(tBuf,"Old Log Type=%s",mLP->KeyVol);
				OutLog(tBuf);	// Çàïèñü â îñí ëîã
				tElL=new El_Log(&LP,PathLog,this);
				tElL->Log->Srv=this;
				IDL_Log->Add(tElL);
			}
			rc=BL_Log->FindNext(iLP,Cnt);
			mLP=(LogParam*)iLP;
		}
		if (RLen<sizeof(SrvCfg))
		{
			memset (&Cfg,0,sizeof(SrvCfg));
		}
	}
	else
	{
		memset (&Cfg,0,sizeof(SrvCfg));
	};

	return FH;
};

int		Services::SaveLCfg(char* UnitID,void* pCfg,int Size,Decoder* pObj)
{
	int	rc=-1;
	char	FullName[512];
	int		FH;
	MyLock(__FILE__,__LINE__,&Fl_RE8,this);
	CalcPath(PathCFG);
	if (pObj)
		sprintf(FullName,"%s%s%03d.%s%d.LCfg",PathCFG,UnitID,pObj->Ext_Mt,pObj->UInfo.DomainName,pObj->UInfo.NumChan);
	else
		sprintf(FullName,"%s%s.LCfg",PathCFG,UnitID);
	SetFileAttributesA(FullName,GetFileAttributesA(FullName)&~FILE_ATTRIBUTE_READONLY);
	FH=open(FullName,O_CREAT|O_BINARY|O_RDWR);
	if (FH!=-1)
	{
		write(FH,pCfg,Size);
		close(FH);
		rc=0;
	} 
	else
	{
		OutLog("Error Save Local Config!",pObj);
		SetCriticalError(this,"Error Save Local Config!","Critical",N_Service);
	};
	MyUnLock(__FILE__,__LINE__,&Fl_RE8,this);
	return rc;
};

int		Services::LoadLCfg(char* UnitID,void* pCfg,int Size,Decoder* pObj)
{
	int	rc=-1;
	char	FullName[512];
	char	tBB[128];
	int		FH,RLen;
	MyLock(__FILE__,__LINE__,&Fl_RE8,this);
	CalcPath(PathCFG);
	if (pObj)
		sprintf(FullName,"%s%s%03d.%s%d.LCfg",PathCFG,UnitID,pObj->Ext_Mt,pObj->UInfo.DomainName,pObj->UInfo.NumChan);
	else
		sprintf(FullName,"%s%s.LCfg",PathCFG,UnitID);
	FH=open(FullName,O_BINARY);
	if (FH!=-1)
	{
		RLen=lseek(FH,0L, SEEK_END);
		lseek(FH,0L, SEEK_SET);
		if (RLen==Size)
		{
			RLen=read(FH,pCfg,Size);
		}
		close(FH);
		if (RLen!=Size)
		{
			sprintf(tBB,"Error Load Local=%s Config!",UnitID);
			OutLog(tBB,pObj);
			SetCriticalError(this,tBB,"Warning",N_Service);
			_unlink(FullName);
		}
		else rc=0;
	}
	else
	{
		sprintf(tBB,"Error Open Load Local=%s Config!",UnitID);
		OutLog(tBB,pObj);
		SetCriticalError(this,tBB,"Warning",N_Service);
		_unlink(FullName);
	};
	MyUnLock(__FILE__,__LINE__,&Fl_RE8,this);
	return rc;
};
/*
int	Services::SaveCfgID()
{
	int	FH;
	char	tBuf[256];
	sprintf(tBuf,"%s\\Services.Cfg",MainPath);
	SetFileAttributesA(tBuf,GetFileAttributesA(tBuf)&~FILE_ATTRIBUTE_READONLY);
	FH=open(tBuf,O_CREAT|O_BINARY|O_RDWR);
	if (FH!=-1)
	{
		write(FH,&Cfg, sizeof(SrvCfg));
		close(FH);
	}
	return FH;
};
*/
int	Services::SaveCfg()
{
	int	FH;
	char	tBuf[256];
	sprintf(tBuf,"%s\\Services.Cfg",MainPath);
	SetFileAttributesA(tBuf,GetFileAttributesA(tBuf)&~FILE_ATTRIBUTE_READONLY);
	FH=open(tBuf,O_CREAT|O_BINARY|O_RDWR);
	if (FH!=-1)
	{
		write(FH,&Cfg,sizeof(SrvCfg));
		BL_Log->SaveToFile(FH,sizeof(SrvCfg));
		close(FH);
	}
	return FH;
};

bool	Services::TestHost(int NumHost)
{
	bool	rc=FALSE;
	if (SInt)
	{
		El_CliInt*	Cli=(El_CliInt*)SInt->CliLAN.Find((UCHAR*)&NumHost,sizeof(ULONG));
		if (Cli)
		{
			if (Cli->Cli.Command!=SICom_Error)
				rc=TRUE;
		}
		else
		{
			rc=TRUE;
		}
	}
	return rc;
};

bool	Services::TestHost(CHAR* Host)
{
	bool	rc=FALSE;
//	int		i;
	if (SInt)
	{
		FindStrList8 FSL8;
		El_CliInt*	Cli=(El_CliInt*)SInt->CliLAN.Begin(FSL8);
		while (Cli)
		{
//			if (SInt->CliLAN[i])
			{
				if (strcmp(Cli->Cli.ShortHost,Host)==0)
				{
					rc=TRUE; break;
				}
			}
			Cli=(El_CliInt*)SInt->CliLAN.Next(FSL8);
		}
	}
	return rc;
};

void	Services::SetCriticalErrorI(char* ObjName,char* Msg,char* TypeMsg,char* MSystem,INT64 Handle)
{
//	return;
//	INT64	RC=0;
	El_Tab*	ElTbl3=NULL;
	El_Tab*	ElTbl4=NULL;
//	PrmDec	PDec;
	char	ttB[1024];
	char	ttB3[1024];
//	char	*ttB2;
	char	tCh;
	int	i=0;
	if (FlReCur==1) 
		return;
	FlReCur=1;
	strcpy(ttB,Msg);
	// Debug
	tCh=ttB[5];
	ttB[5]=0;
	if (strcmp(ttB,"Error")==0)
		ttB[5]=tCh;
	ttB[5]=tCh;
//	if (PObj==NULL) PObj=this;
	// -------------------------
	while (ttB[i]!=0)
	{
		if (ttB[i]==0x20) ttB[i]='_';
		i++;
	};
	if (IDT_NeuronLocalSite==NULL) return;
//	RC=Handle;
	ElTbl4=IDT_NeuronLocalSite->FindNested(&Anchor,AcsMd_Read,N_SystemSetting,N_MonitoringSystem,N_CriticalError,NULL);
	if (ElTbl4)
	{	
		if (ElTbl4->GetNumChild()>=100)
		{
			if (ElTbl4->GetNumChild()==100)
			{
				ElTbl3=IDT_NeuronLocalSite->AddET(&Anchor,ElTbl4);
				if (ElTbl3!=NULL)
				{
					{
						sprintf(ttB3,"%I64d",ElTbl3->GetIDI()->ID_File);
						IDT_NeuronLocalSite->AddParam(&Anchor,ElTbl3,IDNameElTab,ttB3,1);
						IDT_NeuronLocalSite->AddParam(&Anchor,ElTbl3,IDRemark,"Overflow CriticalError number!",1);
						IDT_NeuronLocalSite->AddParam(&Anchor,ElTbl3,IDMonSystem,MSystem,1);
						IDT_NeuronLocalSite->AddParam(&Anchor,ElTbl3,IDTypeMsg,TypeMsg,1);
						IDT_NeuronLocalSite->AddParam(&Anchor,ElTbl3,IDMyHostName,ShortHostName,1);
						IDT_NeuronLocalSite->ModifElTab(&Anchor,ElTbl3);
					}
				}
				IDT_NeuronLocalSite->UnLockEl(&Anchor,ElTbl3);
				ElTbl3=NULL;
			}
		}
		else
		{
			ElTbl3=IDT_NeuronLocalSite->Find(&Anchor,AcsMd_Read,Handle,ElTbl4);
			if (ElTbl3==NULL)
			{
				ElTbl3=IDT_NeuronLocalSite->AddET(&Anchor,Handle,ElTbl4);
			}
		}
	}
	else
	{
		ElTbl4=IDT_NeuronLocalSite->AttainNested(&Anchor,"%s%s",N_SystemSetting,N_MonitoringSystem,N_CriticalError,-1);
	}
	if (ElTbl3!=NULL)
	{
		{
			sprintf(ttB3,"%I64d",ElTbl3->GetIDI()->ID_File);
			IDT_NeuronLocalSite->AddParam(&Anchor,ElTbl3,IDNameElTab,ttB3,1);
			IDT_NeuronLocalSite->AddParam(&Anchor,ElTbl3,IDRemark,ttB,1);
			IDT_NeuronLocalSite->AddParam(&Anchor,ElTbl3,IDMonSystem,MSystem,1);
			IDT_NeuronLocalSite->AddParam(&Anchor,ElTbl3,IDTypeMsg,TypeMsg,1);
			IDT_NeuronLocalSite->AddParam(&Anchor,ElTbl3,IDMyHostName,ShortHostName,1);
			IDT_NeuronLocalSite->ModifElTab(&Anchor,ElTbl3);
		}
	}
	IDT_NeuronLocalSite->UnLockEl(&Anchor,ElTbl4);
	IDT_NeuronLocalSite->UnLockEl(&Anchor,ElTbl3);
	sprintf(ttB,"Msg:%s in Dec=%s Type=%s MSystem=%s",Msg,ObjName,TypeMsg,MSystem);
//	if (Handle==0) Win32LogErr("Neuron.Exe",ttB,HostName);
	FlReCur=0;
	return;
};

void	Services::DelCriticalErrorI(INT64 Handle)
{
	El_Tab*	ElTbl4=NULL;
	El_Tab*	ElTbl3;
	if (Handle!=0)
	{
		ElTbl4=IDT_NeuronLocalSite->FindNested(&Anchor,AcsMd_Read,N_SystemSetting,N_MonitoringSystem,N_CriticalError,NULL);
		if (ElTbl4)
		{
			IDT_NeuronLocalSite->LockEl(&Anchor,ElTbl4);
			ElTbl3=IDT_NeuronLocalSite->Find(&Anchor,AcsMd_Read,Handle,ElTbl4);
			if (ElTbl3)
			{
				IDT_NeuronLocalSite->DelElTab(&Anchor,ElTbl3);
//				IDT_NeuronLocalSite->DelElTabWDL(&Anchor,ElTbl3);
			}
		}
		else
		{
			ElTbl4=IDT_NeuronLocalSite->AttainNested(&Anchor,"%s%s",N_SystemSetting,N_MonitoringSystem,N_CriticalError,-1);
		}
		IDT_NeuronLocalSite->UnLockEl(&Anchor,ElTbl4);
	}
};

INT64	Services::SetCriticalError(Decoder* PObj,char* Msg,char* TypeMsg,char* MSystem,INT64 Handle)
{
	CritErrStr	CErStr;
	INT64	RC=0;
	if (DmpCE)
	{
		if (Handle==0)
		{	// New
			RC=GetUID();
		}
		else
		{
			RC=Handle;
		}
		if (PObj) strcpy(CErStr.ObjName,PObj->Name_Mt);
		else strcpy(CErStr.ObjName,this->Name_Mt);
		strcpy(CErStr.Msg,Msg);
		strcpy(CErStr.TypeMsg,TypeMsg);
		strcpy(CErStr.MSystem,MSystem);
		CErStr.Handle=RC;
		DmpCE->Data((UCHAR*)&CErStr,sizeof(CritErrStr));
	}
	return	RC;
};

void	Services::DelCriticalError(INT64 Handle)
{
	if (Handle)
		DmpDCE->Data((UCHAR*)&Handle,sizeof(INT64));
/*	El_Tab*	ElTbl4=NULL;
	El_Tab*	ElTbl3;
	if (Handle!=0)
	{
		ElTbl4=IDT_Neuron->FindNested(&Anchor,AcsMd_Read,N_SystemSetting,N_MonitoringSystem,N_CriticalError,NULL);
//		ElTbl4=IDI_IS->GetNestedMember("SystemSetting","MonitoringSystem","CriticalError",NULL);
		if (ElTbl4)
		{
			ElTbl3=IDT_Neuron->Find(&Anchor,AcsMd_Read,Handle,ElTbl4);
			if (ElTbl3)
			{
				IDT_Neuron->DelElTab(&Anchor,ElTbl3);
//				IDI_IS->SetLastWrTime(ECl4);
			}
			IDT_Neuron->UnLockEl(&Anchor,ElTbl4);
		}
	}
*/
};
/*
virtual	int			InitEventFn(Decoder* PoiObj);	// Èíèöèàëèçàöèÿ CallBack-ôóíêöèè äëÿ Ñîáûòèé
virtual	void		CloseEventFn(int Handle);	// Çàêðûòü CallBack-ôóíêöèþ Ñîáûòèÿ
virtual	void		SendEvent(int Handle, int NumPin, UCHAR* Packet, size_t Cnt);
*/
int		Services::InitEventFn(Decoder* PoiObj)	// Èíèöèàëèçàöèÿ CallBack-ôóíêöèè äëÿ Ñîáûòèé
{
	El_Event	*IDE;
	int	rcH=NumEvent++;
//	IDE=(El_Event*)EventFn->Find((UCHAR*)&PoiObj,sizeof(Decoder*));
//	if (IDE==NULL)
//	{	// New Init
		IDE=(El_Event*)EventFn->NewElem();
		if (IDE==NULL)
			IDE=new El_Event(PoiObj,rcH);
		else
			IDE->Init(PoiObj,rcH);
		EventFn->Add(IDE);
//	}
//	else
//	{	// ???????????
//		OutLog("Error Init Event Fn!",this,"Error");
//		SetCriticalError(this,"Error Init Event Fn!","Critical",N_Service);
//	};
	if (rcH>0x7f000000)
		SetCriticalError(this,"NumEvent Overflow!","Warning",N_Service);
	return rcH;
};
/*

El_Event*	Services::GetEventObj(int Handle)
{
	return (El_Event*)EventFn->Find((UCHAR*)&Handle,sizeof(int));
}

*/
void	Services::CloseEventFn(int Handle)	// Çàêðûòü CallBack-ôóíêöèþ Ñîáûòèÿ
{
	El_Event	*IDE;
	IDE=(El_Event*)EventFn->Find((UCHAR*)&Handle,sizeof(int));
	if (IDE==NULL)
	{
		OutLog("Error Close Event Fn",this,"Error");
		SetCriticalError(this,"Error Close Event Fn!","Critical",N_Service);
	}
	else
	{
		EventFn->Del(IDE);
	}

};

void Services::SendEvent(int Event,UCHAR* Packet,size_t Cnt)
{
	El_Event	*IDE;
	FindStrList8	FSL8;
	IDE=(El_Event*)EventFn->Begin(FSL8);
	while (IDE!=NULL)
	{
//		MyLock(__FILE__,__LINE__,&IDE->PObj->FlSim,this);
//		MyUnLock(__FILE__,__LINE__,&Fl_RE,this);
//		MyLock(__FILE__,__LINE__,&IDE->PObj->FlSim,this);
		IDE->PObj->UserCallBackEx(IDE->Handle,Event,Packet,Cnt);
//		MyUnLock(__FILE__,__LINE__,&IDE->PObj->FlSim,this);
//		MyUnLock(__FILE__,__LINE__,&IDE->PObj->FlSim,this);
		IDE=(El_Event*)EventFn->Next(FSL8);
	};
};
void	Services::InitCommIDI(IDInfo* IDI,int typeidi)
{
	if (IDI)
	{
		IDI->ID_File=Srv->GetUID();
		Srv->GetRealTime(&IDI->TimeBegin.FTime);
		IDI->TimeOper.FTime=IDI->TimeBegin.FTime;
		IDI->TimeLastWr.FTime=IDI->TimeBegin.FTime;
		IDI->TimeClose.FTime=IDI->TimeBegin.FTime;
		IDI->TypeIDI=typeidi;
		IDI->StateBlock=StateBlock_BEGIN|StateBlock_END|StateBlock_EMPTY;
	}
};

int	Services::InitUnit(Decoder* PoiObj)
{
	int rc=-1;
	if (PoiObj->UInfo.PersID==0)
	{
// 		if (PoiObj->UInfo.TractID==-1)
// 		{
// 			PoiObj->UInfo.LoadsCPU=Neuron->SrvThread->Loads;
// 			PoiObj->UInfo.LoadsRAM=MemUsage;
// 		}
// 		else
// 		{
// 			if (Neuron->ClThread[PoiObj->UInfo.TractID])
// 				PoiObj->UInfo.LoadsCPU=Neuron->ClThread[PoiObj->UInfo.TractID]->Loads;
// 			PoiObj->UInfo.LoadsRAM=MemUsage;
// 		}
		PoiObj->UInfo.Mode[0]=0;
		PoiObj->UInfo.PersID=GetUID();
		PoiObj->UInfo.State[0]=0;
//		PoiObj->UInfo.TractID=0;
//		PoiObj->UInfo.NumChan=PoiObj->Ext_Mt;
		strcpy(PoiObj->UInfo.Host,ShortHostName);
		PoiObj->UInfo.IP=*(uint*)&IPAddrLAN;
		strcpy(PoiObj->UInfo.UnitName,PoiObj->Name_Mt);
		if (UM) UM->AddUnit(&PoiObj->UInfo);
		// -------------------------------------
		IDInfo*	IDI=GetNewIDI(this);
		InitCommIDI(IDI);
		AddressInfo	AdrI;
		IDI->AdrInfo=&AdrI;
//		IDI->pPList->SetParam(IDHostName,"*",2,this);
//		IDI->pPList->SetParam(IDUnitName,N_Service,9,this);
		strcpy(IDI->AdrInfo->DestinAI.UnitName,N_Service);
		IDI->Command=Command_C_SendUnitInfo;
		IDI->Buf=(UCHAR*)&PoiObj->UInfo;
		IDI->Cnt=sizeof(UnitInfo);
		IDI->NumPack++;
		SendIDI(IDI,PoiObj);
		DeleteIDI(IDI);	
		char tB[128];
		//sprintf(tB,"InitUnit PID=%I64x Meth[%x]=%s",PoiObj->UInfo.PersID,(int)PoiObj,PoiObj->Name_Mt);
		sprintf(tB,"InitUnit PID=%I64x Meth=%s",PoiObj->UInfo.PersID,PoiObj->Name_Mt);
		OutLog(tB,this,"Unit");
	}
	else
	{
		ModifUnit(PoiObj);
	}
	return rc;
};

int	Services::ModifUnit(Decoder* PoiObj)
{
	int rc=-1;
	if (PoiObj->UInfo.PersID==0)
	{
		InitUnit(PoiObj);
	}
	else
	{
// 		if (PoiObj->UInfo.TractID==-1)
// 		{
// 			PoiObj->UInfo.LoadsCPU=Neuron->SrvThread->Loads;
// 			PoiObj->UInfo.LoadsRAM=MemUsage;
// 		}
// 		else
// 		{
// 			if (Neuron->ClThread[PoiObj->UInfo.TractID])
// 				PoiObj->UInfo.LoadsCPU=Neuron->ClThread[PoiObj->UInfo.TractID]->Loads;
// 			PoiObj->UInfo.LoadsRAM=MemUsage;
// 		}
		UM->AddUnit(&PoiObj->UInfo);
		IDInfo*	IDI=GetNewIDI(this);
		InitCommIDI(IDI);
		AddressInfo	AdrI;
		IDI->AdrInfo=&AdrI;
//		IDI->pPList->SetParam(IDHostName,"*",2,this);
//		IDI->pPList->SetParam(IDUnitName,N_Service,9,this);
		strcpy(IDI->AdrInfo->DestinAI.UnitName,N_Service);
		IDI->Command=Command_C_SendUnitInfo;
		IDI->Buf=(UCHAR*)&PoiObj->UInfo;
		IDI->Cnt=sizeof(UnitInfo);
		IDI->NumPack++;
		SendIDI(IDI,PoiObj);
		DeleteIDI(IDI);	
	}
	return rc;
};
int	Services::CloseUnit(Decoder* PoiObj)
{
	int rc=-1;
	IDInfo*	IDI=GetNewIDI(this);
	InitCommIDI(IDI);
	AddressInfo	AdrI;
	IDI->AdrInfo=&AdrI;
	//		IDI->pPList->SetParam(IDHostName,"*",2,this);
	//		IDI->pPList->SetParam(IDUnitName,N_Service,9,this);
	strcpy(IDI->AdrInfo->DestinAI.UnitName,N_Service);
	IDI->Command=Command_C_DelUnitInfo;
	IDI->Buf=(UCHAR*)&PoiObj->UInfo;
	IDI->Cnt=sizeof(UnitInfo);
	IDI->NumPack++;
	SendIDI(IDI,PoiObj);
	DeleteIDI(IDI);	
	UM->DelUnit(&PoiObj->UInfo);
	char tB[128];
	//sprintf(tB,"CloseUnit PID=%I64x Meth[%x]=%s",PoiObj->UInfo.PersID,(int)PoiObj,PoiObj->Name_Mt);
	sprintf(tB,"CloseUnit PID=%I64x Meth=%s",PoiObj->UInfo.PersID,PoiObj->Name_Mt);
	OutLog(tB,this,"Unit");
	return rc;
};

int	Services::InitSock(Decoder* PoiObj,int Mode,short NetType, char* Port,char* IP)
{	// Èíèöèàëèçàöèÿ CallBack-ôóíêöèè
	int		rc=0;
	int		i;
	int		FlCB=0;
	PackNetObj		PNetObj;
//	MyLock(__FILE__,__LINE__,&FlSim,this);
	if (Mode&16)
		FlCB=1;
	Mode&=~16;
	if (Net_Flag!=1)
	{
// 		NetMen=NewDecod("NetService",this);
// 		if (NetMen==NULL)
// 		{
// 			SInt->FatalError=1;
// 			rc=-1;
// 			MyUnLock(__FILE__,__LINE__,&FlSim,this);
// 			return rc;
// 		}
// 		else
// 		{
// //			strcpy(NetMen->UInfo.DomainName,UInfo.DomainName);
// 			NetMen->AddDecLink(this,0,1005);
// 			NetMen->Ext_Mt=0;
// 			NetMen->Srv=this;
// 			NetMen->StartEx();
			Net_Flag=1;
// 		}
	}
	PNetObj.NetType=NetType;
// Ñîçäàåì êëèåíòà èëè ñåðâåðà
//	MyLock(__FILE__,__LINE__,&Fl_RE9,this);
	for (i=0;i<NumSICli;i++)
	{
		if (CliList[i].Fl==0)
		{
//			MyLock(__FILE__,__LINE__,&Fl_RE1,this);
			char	tBuf[80];
			CliList[i].Fl=1;
			CliList[i].Fl_Data=1;
			CliList[i].Mode=Mode&0x3;
			int	ModeSock=(Mode&0x0c)>>2;
			CliList[i].PObj=PoiObj;
			El_CliInt*	Cli=(El_CliInt*)SInt->CliLAN.Find((UCHAR*)&SInt->NumLANSrv,sizeof(ULONG));
			switch(CliList[i].Mode)
			{
			case 0:
				sprintf(tBuf,"CliSI%03d",i);
				strcpy(PNetObj.descriptor,tBuf);
				PNetObj.command=NetSockNew;
				NS_DataFF(0,(UCHAR*)&PNetObj,sizeof(PNetObj),PoiObj);
				PNetObj.command=NetSockParam;
				strcpy(PNetObj.param,"Mode");
				switch(ModeSock)
				{
				default:
				case 0:
					strcpy(PNetObj.vol,"BufPack");
					break;
				case 1:
					strcpy(PNetObj.vol,"BufNoPack");
					break;
				case 2:
					strcpy(PNetObj.vol,"NoBufNoPack");
					break;
				}
				NS_DataFF(0,(UCHAR*)&PNetObj,sizeof(PNetObj),PoiObj);
				strcpy(PNetObj.param,"ServerPort");
				strcpy(PNetObj.vol,"499");
				NS_DataFF(0,(UCHAR*)&PNetObj,sizeof(PNetObj),PoiObj);
				strcpy(PNetObj.param,"ServerIP");
				strcpy(PNetObj.vol,Cli->Cli.IPAddr);
				NS_DataFF(0,(UCHAR*)&PNetObj,sizeof(PNetObj),PoiObj);
				break;
			case 1:
				sprintf(tBuf,"CliSo%03d",i);
				strcpy(PNetObj.descriptor,tBuf);
				PNetObj.command=NetSockNew;
				NS_DataFF(0,(UCHAR*)&PNetObj,sizeof(PNetObj),PoiObj);
				PNetObj.command=NetSockParam;
				strcpy(PNetObj.param,"Mode");
				switch(ModeSock)
				{
				default:
				case 0:
					strcpy(PNetObj.vol,"BufPack");
					break;
				case 1:
					strcpy(PNetObj.vol,"BufNoPack");
					break;
				case 2:
					strcpy(PNetObj.vol,"NoBufNoPack");
					break;
				}
				NS_DataFF(0,(UCHAR*)&PNetObj,sizeof(PNetObj),PoiObj);
				if (Port!=NULL)
				{
					strcpy(PNetObj.param,"ServerPort");
					strcpy(PNetObj.vol,Port);
					NS_DataFF(0,(UCHAR*)&PNetObj,sizeof(PNetObj),PoiObj);
				}
				if (IP!=NULL)
				{
					strcpy(PNetObj.param,"ServerIP");
					strcpy(PNetObj.vol,IP);
					NS_DataFF(0,(UCHAR*)&PNetObj,sizeof(PNetObj),PoiObj);
				}
				break;
			case 2:
				sprintf(tBuf,"_SrvS%03d",i);
				strcpy(PNetObj.descriptor,tBuf);
				PNetObj.command=NetSockNew;
				NS_DataFF(0,(UCHAR*)&PNetObj,sizeof(PNetObj),PoiObj);
				PNetObj.command=NetSockParam;
				strcpy(PNetObj.param,"Mode");
				switch(ModeSock)
				{
				default:
				case 0:
					strcpy(PNetObj.vol,"BufPack");
					break;
				case 1:
					strcpy(PNetObj.vol,"BufNoPack");
					break;
				case 2:
					strcpy(PNetObj.vol,"NoBufNoPack");
					break;
				}
				NS_DataFF(0,(UCHAR*)&PNetObj,sizeof(PNetObj),PoiObj);
				if (Port!=NULL)
				{
					strcpy(PNetObj.param,"ServerPort");
					strcpy(PNetObj.vol,Port);
					NS_DataFF(0,(UCHAR*)&PNetObj,sizeof(PNetObj),PoiObj);
				}
				break;
			};
//			if (SizeBuf!=0)
//			{
//				PNetObj.command=NetSockParam;
//				strcpy(PNetObj.param,"SizeTrBuf");
//				sprintf(PNetObj.vol,"%d",SizeBuf);
//				NetMenLAN->Data(0,(UCHAR*)&PNetObj,sizeof(PNetObj));
//			};
			PNetObj.command=NetSockStart;
			NS_DataFF(0,(UCHAR*)&PNetObj,sizeof(PNetObj));
// 			if (FlCB==1)
// 			{
// 				pElementOut=(pNetObject)ClientList->Find(
// 					(PUCHAR)PNetObj.descriptor,
// 					strlen(PNetObj.descriptor));
// 				if(pElementOut) pElementOut->FlagSelectProc=1;
// 
// 			}
//			MyUnLock(__FILE__,__LINE__,&Fl_RE9,this);
//			MyUnLock(__FILE__,__LINE__,&FlSim,this);
			return i;
		};
	}
//	MyUnLock(__FILE__,__LINE__,&Fl_RE9,this);
//	MyUnLock(__FILE__,__LINE__,&FlSim,this);
	rc=-1;
	return rc;
};

void	Services::PrmSock(int Handle,char* Port,char* IP,int SizeBuf,int TimeOut)
{
	PackNetObj		PNetObj;
	if (Handle!=-1)
	{
		char	tBuf[80];
//		MyLock(__FILE__,__LINE__,&FlSim,this);
//		MyLock(__FILE__,__LINE__,&Fl_RE9,this);
//		MyLock(__FILE__,__LINE__,&Fl_RE,this);
		switch(CliList[Handle].Mode)
		{
		case 0:
			sprintf(tBuf,"CliSI%03d",Handle);
			strcpy(PNetObj.descriptor,tBuf);
			break;
		case 1:
			sprintf(tBuf,"CliSo%03d",Handle);
			strcpy(PNetObj.descriptor,tBuf);
			PNetObj.command=NetSockParam;
			if (Port!=NULL)
			{
				strcpy(PNetObj.param,"ServerPort");
				strcpy(PNetObj.vol,Port);
				NS_DataFF(0,(UCHAR*)&PNetObj,sizeof(PNetObj));
			}
			if (IP!=NULL)
			{
				strcpy(PNetObj.param,"ServerIP");
				strcpy(PNetObj.vol,IP);
				NS_DataFF(0,(UCHAR*)&PNetObj,sizeof(PNetObj));
			}
			break;
		case 2:
			sprintf(tBuf,"_SrvS%03d",Handle);
			strcpy(PNetObj.descriptor,tBuf);
			PNetObj.command=NetSockParam;
			if (Port!=NULL)
			{
				strcpy(PNetObj.param,"ServerPort");
				strcpy(PNetObj.vol,Port);
				NS_DataFF(0,(UCHAR*)&PNetObj,sizeof(PNetObj));
			}
			break;
		}
		if (SizeBuf!=0)
		{
			PNetObj.command=NetSockParam;
			strcpy(PNetObj.param,"SizeTrBuf");
			sprintf(PNetObj.vol,"%d",SizeBuf);
			NS_DataFF(0,(UCHAR*)&PNetObj,sizeof(PNetObj));
		};
		if (TimeOut!=0)
		{
			PNetObj.command=NetSockParam;
			strcpy(PNetObj.param,"BlockTimeOut");
			sprintf(PNetObj.vol,"%d",TimeOut);
			NS_DataFF(0,(UCHAR*)&PNetObj,sizeof(PNetObj));
		};
//		MyUnLock(__FILE__,__LINE__,&FlSim,this);
//		MyUnLock(__FILE__,__LINE__,&Fl_RE9,this);
//		MyUnLock(__FILE__,__LINE__,&Fl_RE,this);
	}
};

char*		Services::GetHostByAddr(char* Adr)
{
	int	dontblock;
	HOSTENT *he;
	FindStrList8	FSL8;
	El_CliInt*	Cli=NULL;
	if (SInt!=NULL)
		Cli=(El_CliInt*)SInt->CliLAN.Begin(FSL8);
	while (Cli)
	{
//		if (SInt->CliLAN[i])
		{
			if (strcmp(Cli->Cli.IPAddr,Adr)==0)
			{
				FSL8.DestroyStr();
				return Cli->Cli.ShortHost;
			}
		}
		Cli=(El_CliInt*)SInt->CliLAN.Next(FSL8);
	}
	FSL8.DestroyStr();
	dontblock=inet_addr(Adr);
	he=gethostbyaddr((const char*)&dontblock,4,AF_INET);
	if (he)
	{
		strcpy(namehost, he->h_name);
		TestNetName(namehost);
		return namehost;
	}
	else
		return NULL;
};

SysPack*	Services::GetSysPack(size_t Size)
{
	UCHAR*	tSysPackMem;
	size_t	tRSizeSysPack;
	MyLock(__FILE__,__LINE__,&Fl_RE11,this);
	if (RSizeSysPack<Size+sizeof(SysPack))
	{
		tRSizeSysPack=Size+sizeof(SysPack);
		tSysPackMem=(UCHAR*)realloc(SysPackMem,tRSizeSysPack);
		if (tSysPackMem)
		{
			RSizeSysPack=tRSizeSysPack;
			SysPackMem=tSysPackMem;
		}
		else
		{	// !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
			SetCriticalError(this,"Error Realloc SysPack","Critical");
		}
	}
	MyUnLock(__FILE__,__LINE__,&Fl_RE11,this);
	return	(SysPack*)SysPackMem;
};

int	Services::SIPack(int Handle,int Type,UCHAR* Prm,size_t Cnt)
{
	SysPack	*SIPack=GetSysPack(Cnt);
	int		N_Pin=-1;
	if (Handle!=-1)
	{
		if (CliList[Handle].Mode==2)
			SendSock(Handle,0,(UCHAR*)&N_Pin,sizeof(N_Pin));
		SIPack->N_Cli=-1;
		SIPack->Type=Type;
		SIPack->Cnt=Cnt;
		if (Prm!=NULL) memcpy(SIPack->Prm,Prm,Cnt);
		if (CliList[Handle].Mode==2)
			SendSock(Handle,1,(UCHAR*)&SIPack->Type,SIPack->Cnt+sizeof(int));
		else
			SendSock(Handle,0,(UCHAR*)&SIPack->Type,SIPack->Cnt+sizeof(int));

	}
	return 0;
};

int	Services::SIPack(int Handle,int N_Cli,int Type,UCHAR* Prm,size_t Cnt)
{
	SysPack	*SIPack=GetSysPack(Cnt);
	if (Handle!=-1)
	{
		if (CliList[Handle].Mode==2)
			SendSock(Handle,0,(UCHAR*)&N_Cli,sizeof(N_Cli));
		SIPack->N_Cli=-1;
		SIPack->Type=Type;
		SIPack->Cnt=Cnt;
		if (Prm!=NULL) memcpy(SIPack->Prm,Prm,Cnt);
		if (CliList[Handle].Mode==2)
			SendSock(Handle,1,(UCHAR*)&SIPack->Type,SIPack->Cnt+sizeof(int));
		else
			SendSock(Handle,0,(UCHAR*)&SIPack->Type,SIPack->Cnt+sizeof(int));

	}
	return 0;
};

static PackNetObj tPack;

/*
int	Services::ReadSock(int Handle,int &N_Pin,UCHAR* &Buf,size_t &Cnt)
{
	int		rc=0;
	UCHAR*	tBuf;
	size_t	tCnt;
	if (Handle!=-1)
	{
		if (CliList[Handle].Fl==1)
		{ 
			MyLock(__FILE__,__LINE__,&Fl_RE9,this);
			switch(CliList[Handle].Mode)
			{
			case 0:
				sprintf(PNetObj.descriptor,"CliSI%03d",Handle);
				break;
			case 1:
				sprintf(PNetObj.descriptor,"CliSo%03d",Handle);
				break;
			case 2:
				sprintf(PNetObj.descriptor,"_SrvS%03d",Handle);
				break;
			}
//			NetMen->ParamEx("SetElemProperties",(char*)&PNetObj);
			pElementOut=(pNetObject)ClientList->Find(
				(PUCHAR)PNetObj.descriptor,
				strlen(PNetObj.descriptor));
			if(pElementOut) pElementOut->FlagSelectProc=1;

			NetMen->OutDataEx(N_Pin,tBuf,tCnt);
			if(tBuf) if(tCnt)
			{
				pPackNetObj	pack;
				memcpy(&tPack,tBuf,sizeof(PackNetObj));
				pack=(pPackNetObj)tBuf;
				N_Pin=pack->pin;
				Buf=pack->buf;
				Cnt=pack->cnt;
				rc=Cnt;

					
			}

			MyUnLock(__FILE__,__LINE__,&Fl_RE9,this);
		}
	}
	return rc;
};
*/
/*
int	Services::PostIDI(IDInfo* IDI,Decoder* Obj,char* NetType)	// Îòïðàâèòü ïàêåò ÷åðåç NetComm
{
	int		rc=0;
	//	if (NetCommLAN||NetCommWAN)
	//	MyLock(__FILE__,__LINE__,&Fl_RE,this);
	if (NetCommLAN)
	{
		//		MyLock(__FILE__,__LINE__,&Fl_MTSI2,this);
		IDI->pPList->SetParam(IDMyHostName,ShortHostName,1+strlen(ShortHostName),this);
		IDI->pPList->SetParam(IDMyUnitName,Obj->Name_Mt,1+strlen(Obj->Name_Mt),this);
		IDI->pPList->SetParam(IDMyDomainName,Obj->UInfo.DomainName,1+strlen(Obj->UInfo.DomainName),this);
		IDI->pPList->SetParam(IDNetType,NetType,1+strlen(NetType),this);
		//		if (Obj->UInfo.PersID) IDI->pPList->SetParam(IDMyPersonalID,&Obj->UInfo.PersID,sizeof(INT64),this);		// !!!!!!!!!!!!!!!!!!!!!!!!!!!!!
		
		//		CalcNetIP(IDI);
		SendMt=Obj;
//		BufDataFF(3,IDI);
		DataFF(3,(UCHAR*)IDI,sizeof(IDInfo));
		//		MyUnLock(__FILE__,__LINE__,&Fl_MTSI2,this);//Fl_MTSI2=0;
	};
	//	MyUnLock(__FILE__,__LINE__,&Fl_RE,this);//Fl_MTSI=0;
	return	rc;
}
*/
int	Services::SendIDI(IDInfo* IDI,Decoder* Obj,int NetType)	// Îòïðàâèòü ïàêåò ÷åðåç NetComm
{
	int		rc=0;
//	if (NetCommLAN||NetCommWAN)
//	MyLock(__FILE__,__LINE__,&Fl_RE,this);
	if (NetCommLAN)
	{
		if (IDI->AdrInfo)
		{
			AddressInfo*	AdrI=IDI->AdrInfo;
			ID_ElemAdrList *El_List=NULL;
			ID_ElemAdrStr *El_Adr=NULL;
			if (((IDI->AdrInfo->NCStrategic&B_Alternate)!=0)&&((IDI->StateBlock&(StateBlock_BEGIN|StateBlock_END))!=(StateBlock_BEGIN|StateBlock_END)))
			{
				if (AddList==NULL)
				{
					AddList=new ID_List8();
				}
				El_List=(ID_ElemAdrList*)AddList->Find((UCHAR*)&Obj,sizeof(Decoder*));
				if (El_List==NULL)
				{
					El_List=new ID_ElemAdrList(Obj);
					AddList->Add(El_List);
				};
				El_Adr=(ID_ElemAdrStr*)El_List->List->Find((UCHAR*)&IDI->ID_File,sizeof(INT64));
				if (El_Adr==NULL)
				{
					El_Adr=new ID_ElemAdrStr(IDI->ID_File);
					El_List->List->Add(El_Adr);
					memcpy(&El_Adr->AdrI,IDI->AdrInfo,sizeof(AddressInfo));
				}
				IDI->AdrInfo=&El_Adr->AdrI;
			}
			if (IDI->AdrInfo->NativeAI.HostName[0]==0)
				strcpy(IDI->AdrInfo->NativeAI.HostName,ShortHostName);
			strcpy(IDI->AdrInfo->SourceAI.HostName,ShortHostName);
			if (IDI->AdrInfo->NativeAI.UnitName[0]==0)
				strcpy(IDI->AdrInfo->NativeAI.UnitName,Obj->Name_Mt);
			strcpy(IDI->AdrInfo->SourceAI.UnitName,Obj->Name_Mt);
			if (IDI->AdrInfo->NativeAI.DomainName[0]==0)
				strcpy(IDI->AdrInfo->NativeAI.DomainName,Obj->UInfo.DomainName);
			strcpy(IDI->AdrInfo->SourceAI.DomainName,Obj->UInfo.DomainName);
			IDI->AdrInfo->NetType=NetType;
			if (IDI->AdrInfo->NativeAI.UnitID==0)
				IDI->AdrInfo->NativeAI.UnitID=Obj->UInfo.PersID;
			IDI->AdrInfo->SourceAI.UnitID=Obj->UInfo.PersID;
			rc=DataIDI(3,IDI);
			if ((IDI->StateBlock&StateBlock_END)&&(El_Adr)&&(El_List))
			{
				El_List->List->Del(El_Adr);
				if (El_List->List->NumElem==0)
				{
					AddList->Del(El_List);
				}
			}
			IDI->AdrInfo=AdrI;
		}
		else
		{
			rc=-1;	// !!!
		}
	}
//	MyUnLock(__FILE__,__LINE__,&Fl_RE,this);//Fl_MTSI=0;
	return	rc;
}

int	Services::SendSock(int Handle,int N_Pin,UCHAR* Buf,size_t Cnt)
{	// Îòïðàâèòü ïàêåò SI
	int		rc=0;
	PackNetObj		PNetObj;
//	MyLock(__FILE__,__LINE__,&Fl_RE9,this);
	if (Handle!=-1)
		if (CliList[Handle].Fl==1)
		{
//			char	tBuf[80];
			switch(CliList[Handle].Mode)
			{
			case 0:
				sprintf(PNetObj.descriptor,"CliSI%03d",Handle);
				break;
			case 1:
				sprintf(PNetObj.descriptor,"CliSo%03d",Handle);
				break;
			case 2:
				sprintf(PNetObj.descriptor,"_SrvS%03d",Handle);
				break;
			}
//			strcpy(PNetObj.descriptor,tBuf);
			PNetObj.command=NetSockData;
			PNetObj.buf=Buf; PNetObj.cnt=Cnt;
			PNetObj.pin=N_Pin;
			rc=NS_DataFF(0,(UCHAR*)&PNetObj,sizeof(PNetObj));
			rc=0;
		};
//	MyUnLock(__FILE__,__LINE__,&Fl_RE9,this);//Fl_MTSI=0;
	return rc;	// !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!1
};

 int	Services::CloseSock(int Handle)
{	// Çàêðûòü CallBack-ôóíêöèþ
	int rc;
	//if (CliList[Handle].Fl==1)
	//	CliList[Handle].PObj=NULL;
 	StopSock(Handle);
// 	rc=NetDmp.Data((UCHAR*)&Handle,sizeof(Handle),0);
//	Sleep(1);
	rc=InternalCloseSock(Handle);
	return rc;
};

 
 int	Services::InternalCloseSock(int Handle)
 {	// Çàêðûòü CallBack-ôóíêöèþ
	 int		rc=-1;
	 PackNetObj	PNetObj;
//	 MyLock(__FILE__,__LINE__,&FlSim,this);//Fl_MTSI=0;
//	 MyLock(__FILE__,__LINE__,&Fl_RE9,this);//Fl_MTSI=0;
	 if (CliList[Handle].Fl==1)
	 {
		 //		MyLock(__FILE__,__LINE__,&Fl_RE1,this);
		 char	tBuf[80];
		 switch(CliList[Handle].Mode)
		 {
		 case 0:
			 sprintf(tBuf,"CliSI%03d",Handle);
			 break;
		 case 1:
			 sprintf(tBuf,"CliSo%03d",Handle);
			 break;
		 case 2:
			 sprintf(tBuf,"_SrvS%03d",Handle);
			 break;
		 }
		 strcpy(PNetObj.descriptor,tBuf);
		 PNetObj.command=NetSockDel;
		 NS_DataFF(0,(UCHAR*)&PNetObj,sizeof(PNetObj));
		 CliList[Handle].Fl=0;CliList[Handle].PObj=NULL;
		 //		MyUnLock(__FILE__,__LINE__,&Fl_RE1,this);
		 rc=0;
	 };
//	 MyUnLock(__FILE__,__LINE__,&FlSim,this);//Fl_MTSI=0;
//	 MyUnLock(__FILE__,__LINE__,&Fl_RE9,this);//Fl_MTSI=0;
	 return rc;
 };

void		Services::StopSock(int Handle)
{
	CliList[Handle].Fl_Data=0;
};		// 

void		Services::StartSock(int Handle)
{
	CliList[Handle].Fl_Data=1;
};		// 

 IDInfo*		Services::GetNewIDI(Decoder* ParMt,int Fl_CreateAdrInfo)	// ¦õúþüõýôþòðýð ôû  ø¸ÿþû¹÷þòðýø 
 {
	 IDInfo*		IDI=NULL;
	 ID_ElemIDI*	TmpIDE=NULL;	
	 ID_ElemIDI*	TmpIDE2=NULL;	
	 MyLock(__FILE__,__LINE__,&Fl_RE2,this);
_mmm:
	 TmpIDE=(ID_ElemIDI*)DelIDIList->Next(FSL_8);
	 if (TmpIDE==NULL)
		 TmpIDE=(ID_ElemIDI*)DelIDIList->Begin(FSL_8);
	 if (TmpIDE!=0)
	 {
		 DelIDIList->Remove(TmpIDE);
		 TmpIDE->Init(ParMt);
	 }
	 else
	 {
		 TmpIDE=new ID_ElemIDI(ParMt);
	 }
	 while (ActIDIList->Add(TmpIDE)==-1)
	 {
		 DelIDIList->Remove(TmpIDE);
		 ActIDIList->Remove(TmpIDE);
		 SetCriticalError(ParMt,"Error allocation IDInfo","Error",N_Service);
		 delete TmpIDE;
		 TmpIDE=new ID_ElemIDI(ParMt);
	 }
	 IDI=TmpIDE->IDI;
	 if (IDI==NULL)
		 goto _mmm;
	 strcpy(IDI->ParentMtName,ParMt->Name_Mt);IDI->ParentMtExt=ParMt->Ext_Mt;
	 if (Fl_CreateAdrInfo)
		 IDI->AdrInfo=new AddressInfo();
	 MyUnLock(__FILE__,__LINE__,&Fl_RE2,this);//Fl_MTSG=0;
	 return	IDI;
 }
IDInfo*	Services::CrIDICopy(IDInfo* IDI,Decoder* pDec)
{
	IDInfo* retIDI=NULL;
//	ID_ElemIDI2*	ElIDI=NULL;
	retIDI=GetNewIDI(pDec);
	return retIDI;
};

void	Services::DelIDICopy(IDInfo* IDIS,IDInfo* IDID)
{
//	ID_ElemIDI2*	ElIDI=NULL;
	DeleteIDI(IDID);
};

int			Services::DeleteIDI(IDInfo* IDI,int Fl_DeleteAdrInfo)	// -1 îøèáêà 0 îk
{
	MyLock(__FILE__,__LINE__,&Fl_RE2,this);
	int rc=-1;
	if (IDI==NULL)
		return rc;
	ID_ElemIDI*	TmpIDE=NULL;	
	if ((Fl_DeleteAdrInfo)&&(IDI->AdrInfo))
		delete IDI->AdrInfo;
	IDI->AdrInfo=NULL;
	TmpIDE=(ID_ElemIDI*)ActIDIList->Find((UCHAR*)&IDI,sizeof(void*));
	if (TmpIDE!=0)
	{
		rc=ActIDIList->Remove(TmpIDE);
		rc=DelIDIList->Add(TmpIDE);
		TmpIDE->IDI->pPList->DelAllElem();
	}
	else
	{
		Srv->SetCriticalError(this,"Error deleting IDInfo","Critical","Services");
	}
	MyUnLock(__FILE__,__LINE__,&Fl_RE2,this);//Fl_MTSG=0;
	return rc;
}

Parametr*	Services::GetParamVol(char* Name,BandleList* CSL)
{
	FindStruct	FStr;
	Parametr*	pPrm=GetParam(Name);
/*	El_Tab*		tEl=IDT_Prm->GetElTab(&Anchor,pPrm->UID);
	El_Tab*		tEl2;
	El_Tab*		tEl3;
	if (tEl)
	{
//		MyLock(__FILE__,__LINE__,&Fl_RE,this);
		tEl2=IDT_Prm->Find(&Anchor,AcsMd_Read,"Value",tEl);
		if (tEl2)
		{
			CSL->DelAll();
			tEl3=IDT_Prm->FindFirstET(&Anchor,&FStr,AcsMd_Read,tEl2);
			while (tEl3)
			{
				char*	tVol;
				if (IDT_Prm->GetParam(&Anchor,tEl3,IDNameElTab,tVol)==0)
				{
					CSL->AddElem((UCHAR*)tVol,strlen(tVol)+1);
				};
				IDT_Prm->UnLockEl(&Anchor,tEl3);
				tEl3=IDT_Prm->FindNextET(&Anchor,&FStr,AcsMd_Read,tEl2);
			}
			IDT_Prm->UnLockEl(&Anchor,tEl2);
		}
		IDT_Prm->UnLockEl(&Anchor,tEl);
//		MyUnLock(__FILE__,__LINE__,&Fl_RE,this);
	}
*/	return		pPrm;
};

Parametr*	Services::GetParamVol(INT64 ID,BandleList* CSL)
{
	FindStruct	FStr;
	Parametr*	pPrm=GetParam(ID);
/*	El_Tab*		tEl=IDT_Prm->GetElTab(&Anchor,ID);
	El_Tab*		tEl2;
	El_Tab*		tEl3;
	if (tEl)
	{
//		MyLock(__FILE__,__LINE__,&Fl_RE,this);
		tEl2=IDT_Prm->Find(&Anchor,AcsMd_Read,"Value",tEl);
		if (tEl2)
		{
			CSL->DelAll();
			tEl3=IDT_Prm->FindFirstET(&Anchor,&FStr,AcsMd_Read,tEl2);
			while (tEl3)
			{
				char*	tVol;
				if (IDT_Prm->GetParam(&Anchor,tEl3,IDNameElTab,tVol)==0)
				{
					CSL->AddElem((UCHAR*)tVol,strlen(tVol)+1);
				};
				IDT_Prm->UnLockEl(&Anchor,tEl3);
				tEl3=IDT_Prm->FindNextET(&Anchor,&FStr,AcsMd_Read,tEl2);
			}
			IDT_Prm->UnLockEl(&Anchor,tEl2);
		}
		IDT_Prm->UnLockEl(&Anchor,tEl);
//		MyUnLock(__FILE__,__LINE__,&Fl_RE,this);
	}
*/	return		pPrm;
};

Parametr*	Services::GetParam(INT64 ID)
{
	Parametr*	pPrm=NULL;
//	int	i;
	pPrm=GetTablParametrID(ID);
	return		pPrm;
};

Parametr* Services::GetParam(char* Name)
{
	Parametr* pPrm=NULL;
	size_t i;
	for (i=FirstPrm;i<EndFirstPrm;i++)
	{
		pPrm=GetTablParametrID(i);
		if (pPrm)
		{
			if (strcmp(pPrm->Name,Name)==0)
			{
				return  pPrm;
			}
		}
	}
	for (i=LastPrm;i<FinishPrm;i++)
	{
		pPrm=GetTablParametrID(i);
		if (pPrm)
		{
			if (strcmp(pPrm->Name,Name)==0)
			{
				return  pPrm;
			}
		}
	}
	return 0;// pPrm;
};
// Parametr*	Services::GetParam(char* Name)
// {
// 	Parametr*	pPrm=NULL;
// 	size_t i;
// 	for (i=FirstPrm;i<EndFirstPrm;i++)
// 	{
// 		pPrm=GetTablParametrID(i);
// 		if (strcmp(pPrm->Name,Name)==0)
// 		{
// 			break;
// 		}
// 		if (pPrm==0)
// 		{
// 			pPrm=NULL; break;
// 		}
// 		pPrm=NULL;
// 	}
// 	if (pPrm==NULL)
// 	{
// 		for (i=LastPrm;i<FinishPrm;i++)
// 		{
// 			pPrm=GetTablParametrID(i);
// 			if (strcmp(pPrm->Name,Name)==0)
// 			{
// 				break;
// 			}
// 			if (pPrm==0)
// 			{
// 				pPrm=NULL; break;
// 			}
// 		}
// 	}
// 	return		pPrm;
// };

IDTable*	Services::OpenTable(Decoder* ParMt,INT64 ID,int CustomID)		// -> ReadyTable
{
	MyLock(__FILE__,__LINE__,&Fl_RE10,ParMt);
// 	if (BDBIDInfo==0)
// 	{
// 		BDBIDInfo=NewDecod("IDI_FSrv",this);
//  		if (BDBIDInfo)
//  		{
// 			strcpy(BDBIDInfo->UInfo.DomainName,UInfo.DomainName);
// 			InitUnit(BDBIDInfo);
// 			BDBIDInfo->ParamEx("Mode",N_Standalone);
// 			BDBIDInfo->ParamEx("Path",MainPath);
// 			BDBIDInfo->ParamEx("Transit","Off");
// 
// 			BDBIDInfo->AddDecLink(this,0,1002);	// 1000 pin Services
//  			BDBIDInfo->StartEx();
//  		}
// 	}
/*	if (FSrv==NULL)
	{
		FSrv=NewDecod("IDI_FSrv");
		if (FSrv)
		{
			InitUnit(FSrv);
			FSrv->AddDecLink(this,1,1000);	// 1000 pin Services
			FSrv->Param("Path",MainPath);
			FSrv->Start();
		}
	}
*/
	if (TableList==NULL)
	{
		TableList=new ID_List8();
	}
	El_TabList*		ElTL=(El_TabList*)TableList->Find((UCHAR*)&ID,sizeof(INT64));
	if (ElTL==NULL)
	{
		ElTL=new El_TabList(ID,this); 
		if (CustomID==0)
		{
			ElTL->IDT->CustomerID=NetLCond.CustomerID;
		}
		else
			ElTL->IDT->CustomerID=CustomID;
//		ElTL->IDT->Mt=this;
		TableList->Add(ElTL);
//		ElTL->IDT->AddDecLink(this,0,2000);	// Îò òàáëèöû pin 0
//		ElTL->IDT->AddDecLink(this,1,2001);	// Îò òàáëèöû pin 1
		ElTL->IDT->AddDecLink(this,2,2002);	// Îò òàáëèöû pin 1
		ElTL->IDT->AddDecLink(this,3,2003);	// Îò òàáëèöû pin 1
		ElTL->AddLink(ParMt);
		ElTL->IDT->StartEx();
	}
	else
	{
		ElTL->AddLink(ParMt);
	}
	IDTable*	ttEl=ElTL->IDT;
	MyUnLock(__FILE__,__LINE__,&Fl_RE10,ParMt);
	return ttEl;
};
/*
int			Services::SetElemTable(IDTable* Tbl,El_Tab* ElT)	// -> MofifyElemTable
{
	int	rc=0;
	return rc;
}
*/
void	Services::CalcTable(void)
{
	if (TableList)
	{
		FindStrList8	FSL8;
		MyLock(__FILE__,__LINE__,&FlSimCT,this);//Fl_MTSI=0;
		El_TabList*		ElTL=(El_TabList*)TableList->Begin(FSL8);
		while (ElTL)
		{
			if (MyTryLock(__FILE__,__LINE__,&ElTL->IDT->FlSim,this))
			{
				ElTL->IDT->IdleFunc(-1);
				MyUnLock(__FILE__,__LINE__,&ElTL->IDT->FlSim,this);
			}
//			ElTL->IDT->DataFFEx(1,(UCHAR*)ElTL,1);
			ElTL=(El_TabList*)TableList->Next(FSL8);
		}	
		MyUnLock(__FILE__,__LINE__,&FlSimCT,this);//Fl_MTSI=0;
	}
};

int		Services::OpenThIDView(void)
{
	int rc=-1;
	if (ThIDView==NULL)
	{
//		MyLock(__FILE__,__LINE__,&Fl_RE,this);
		ThIDView=NewDecod("ThIDView",this);
		ThIDView->AddDecLink(this,0,1001);	// 1000 pin Services
		ThIDView->StartEx();
		rc=0;
//		MyUnLock(__FILE__,__LINE__,&Fl_RE,this);
	}
	return rc;
};

int		Services::CloseThIDView(void)
{
	int rc=-1;
	if (ThIDView)
	{
		ThIDView->StopEx();
		DeleteDecoder(ThIDView); ThIDView=NULL;
		rc=0;
	}
	return rc;
};

int		Services::AddFSrvLink(Decoder* pDec,int PinI)
{
	int rc=-1;
//	MyLock(__FILE__,__LINE__,&Fl_RE,pDec);
	if (FSrv)
	{
		FSrv->AddDecLink(pDec,0,PinI); rc=0;
	}
//	MyUnLock(__FILE__,__LINE__,&Fl_RE,pDec);
	return rc;
};

int		Services::CloseTable(Decoder* ParMt,IDTable* Tbl)					// -> Immediately closed
{
	int	rc=0;
	MyLock(__FILE__,__LINE__,&Fl_RE10,ParMt);
	El_TabList*		ElTL=(El_TabList*)TableList->Find((UCHAR*)&Tbl->ID,sizeof(INT64));
	Tbl->UnLockEl(&Anchor,Tbl->GetRoot());
	ElTL->DelLink(ParMt);
	Tbl->DelDecLink(ParMt);
	if (ElTL->BL.NumElem==0)
	{
		TableList->Del(ElTL);
	}
	MyUnLock(__FILE__,__LINE__,&Fl_RE10,ParMt);
	return rc;
}

Module*	Services::FindModule(char* Name)
{
	int	i;
	Module*	tMd=NULL;
	for (i=0;i<SizeMainFiles;i++)
	{
		if (strcmp(pMod[i].Name,Name)==0)
		{
			tMd=&pMod[i];
			break;
		}
	}
	return	tMd;
};

void	Services::ReInitUnit(char* Host)
{
	El_Unit*	tEl;
	if (UM)
	{
//		MyLock(__FILE__,__LINE__,&UM->Fl_RE,this);
		if (UM->UnitList)
		{
			FindStrList8	FSL8;
			tEl=(El_Unit*)UM->UnitList->Begin(FSL8);
			while (tEl)
			{
				if (strcmp(Host,tEl->UInf.Host)==0)
				{
					UM->DelUnit(&tEl->UInf);
				}
				//			tEl=(El_Unit*)UM->UnitList->Find(tEl);
				tEl=(El_Unit*)UM->UnitList->Next(FSL8);	// Next(tEl)	// ?????????????????????
			}
		}
//		MyUnLock(__FILE__,__LINE__,&UM->Fl_RE,this);
	}
};

// ---------------------------------------------------
int		Services::GetUDPPort(void)
{	//	IPPORT_DYNAMIC_MIN - IPPORT_DYNAMIC_MAX
	int Port=-1;
	int i;
	for (i=0;i<SizeUDPPort;i++)
	{
		if (UDPPortFl[i]==FALSE)
		{
			UDPPortFl[i]=TRUE;
			Port=IPPORT_DYNAMIC_MIN+i;
			break;
		}
	};
	return Port;
};
bool	Services::ReleaseUDPPort(int Port)
{
	bool	rc=FALSE;
	if ((Port>=IPPORT_DYNAMIC_MIN)&&(Port<IPPORT_DYNAMIC_MAX))
	{
		UDPPortFl[Port-IPPORT_DYNAMIC_MIN]=FALSE;
		rc=TRUE;
	}
	return rc;
};
ElUDPCli*	Services::OpenUDPCilent(int NetType,int Port, Decoder* PObj)
{
	INT64	FindVol=Port+((INT64)NetType<<16);
	ElUDPCli* ElUDP=(ElUDPCli*)ListUDPCli->Find((UCHAR*)&FindVol,sizeof(INT64));
	if (ElUDP==NULL)
	{
		ElUDP = new	ElUDPCli(NetType,Port); 
		if (ListUDPCli->Add(ElUDP)==-1)
		{
			delete ElUDP;
			ElUDP=NULL;
		}
		else
		{
			ElUDP->AddUser((INT64)PObj);
		}
	}
	else
	{
		ElUDP->AddUser((INT64)PObj);
	}
	return ElUDP;
};

bool		Services::CloseUDPCilent(int NetType,int Port, Decoder* PObj)
{
	bool	rc=FALSE;
	INT64	FindVol=Port+((INT64)NetType<<16);
	ElUDPCli* ElUDP=(ElUDPCli*)ListUDPCli->Find((UCHAR*)&FindVol,sizeof(INT64));
	if (ElUDP)
	{
		ElUDP->DelUser((INT64)PObj);
		if (ElUDP->UserList.NumElem==0)
		{
			ListUDPCli->Del(ElUDP);
		}
		rc=TRUE;
	}
	return rc;
};
// ---------------------------------------------------
bool	Services::AddGroupInGroup(char *Group,char *GroupIn)
{
	bool RC=FALSE;
	El_Tab*		tEl;
	El_Tab*		tEl2;
	El_Tab*		tEl3;
	El_Tab*		tEl4;
	FindStruct	FSLG;
	void		*pVol;
	tEl=Srv->IDT_NeuronLocalCustomer->FindNested(&Anchor,AcsMd_Read,N_APCS,N_WorkGroups,GroupIn,N_WorkGroups,NULL);
	if (tEl==NULL)
		tEl=Srv->IDT_NeuronLocalCustomer->AttainNested(&Anchor,"%s%s%s%s",N_APCS,N_WorkGroups,GroupIn,N_WorkGroups,-1);
	if (tEl)
	{
		tEl2=Srv->IDT_NeuronLocalCustomer->AddET(&Anchor,Group,tEl);
		tEl3=Srv->IDT_NeuronLocalCustomer->FindNested(&Anchor,AcsMd_Read,N_APCS,N_WorkGroups,Group,N_WorkGroupsIn,NULL);
		if (tEl3==NULL)
			tEl3=Srv->IDT_NeuronLocalCustomer->AttainNested(&Anchor,"%s%s%s%s",N_APCS,N_WorkGroups,Group,N_WorkGroupsIn,-1);
		if (tEl3)
		{
			tEl4=Srv->IDT_NeuronLocalCustomer->AddET(&Anchor,GroupIn,tEl3);
			if (tEl4)
			{
				Srv->IDT_NeuronGlobal->UnLockEl(&Anchor,tEl4);
			}
			Srv->IDT_NeuronGlobal->UnLockEl(&Anchor,tEl3);
		}
		Srv->IDT_NeuronGlobal->UnLockEl(&Anchor,tEl2);
		Srv->IDT_NeuronGlobal->UnLockEl(&Anchor,tEl);
	}
	return RC;
};

bool	Services::AddUserInGroup(char *Name,char *Group)
{
	bool RC=FALSE;
	El_Tab*		tEl;
	El_Tab*		tEl2;
	El_Tab*		tEl3;
	El_Tab*		tEl4;
	FindStruct	FSLG;
	void		*pVol;
	tEl=Srv->IDT_NeuronLocalCustomer->FindNested(&Anchor,AcsMd_Read,N_APCS,N_WorkGroups,Group,N_Users,NULL);
	if (tEl==NULL)
		tEl=Srv->IDT_NeuronLocalCustomer->AttainNested(&Anchor,"%s%s%s%s",N_APCS,N_WorkGroups,Group,N_Users,-1);
	if (tEl)
	{
		tEl2=Srv->IDT_NeuronLocalCustomer->AddET(&Anchor,Name,tEl);
		tEl3=Srv->IDT_NeuronLocalCustomer->FindNested(&Anchor,AcsMd_Read,N_APCS,N_Users,Name,N_WorkGroupsIn,NULL);
		if (tEl3==NULL)
			tEl3=Srv->IDT_NeuronLocalCustomer->AttainNested(&Anchor,"%s%s%s%s",N_APCS,N_Users,Name,N_WorkGroupsIn,-1);
		if (tEl3)
		{
			tEl4=Srv->IDT_NeuronLocalCustomer->AddET(&Anchor,Group,tEl3);
			if (tEl4)
			{
				Srv->IDT_NeuronGlobal->UnLockEl(&Anchor,tEl4);
			}
			Srv->IDT_NeuronGlobal->UnLockEl(&Anchor,tEl3);
		}
		Srv->IDT_NeuronGlobal->UnLockEl(&Anchor,tEl2);
		Srv->IDT_NeuronGlobal->UnLockEl(&Anchor,tEl);
	}
	return RC;
};

bool	Services::AddNewGroup(char *Group)
{
	bool RC=FALSE;
	El_Tab	*tEl;
	tEl=IDT_NeuronLocalCustomer->AttainNested(&Anchor,"%s%s%s",N_APCS,N_WorkGroups,Group,-1);
	if (tEl)
	{
		RC=TRUE;
//		IDT_NeuronLocalCustomer->AddParam(&Anchor,tEl,IDUserPassword,(void*)(Password),1+strlen(Password));
		IDT_NeuronLocalCustomer->UnLockEl(&Anchor,tEl);
	}
	return RC;
};
// ---------------------------------------------------
El_Tab*		Services::FindUsersGroupBegin(char *Name,FindStruct*	FStr)
{
	El_Tab*	tEl;
	El_Tab*	tEl2;
	El_Tab*	tEl3;
	El_Tab*	tEl_R=NULL;

	tEl=Srv->IDT_NeuronLocalCustomer->FindNested(&Anchor,AcsMd_Read,N_APCS,N_WorkGroups,NULL);
	if (tEl)
	{
		tEl2=Srv->IDT_NeuronLocalCustomer->FindFirstET(&Anchor,FStr,AcsMd_Read,tEl);
		while(tEl2)
		{
			tEl3=IDT_NeuronLocalCustomer->FindNestedStr(&Anchor,AcsMd_Read,N_Users,tEl2);
			if (tEl3)
			{
				tEl_R=IDT_NeuronLocalCustomer->FindNestedStr(&Anchor,AcsMd_Read,Name,tEl3);
				if(tEl_R)
				{
					Srv->IDT_NeuronLocalCustomer->UnLockEl(&Anchor,tEl3);
					Srv->IDT_NeuronLocalCustomer->UnLockEl(&Anchor,tEl2);
					break;
				}
				Srv->IDT_NeuronLocalCustomer->UnLockEl(&Anchor,tEl3);
			}
			Srv->IDT_NeuronLocalCustomer->UnLockEl(&Anchor,tEl2);
			tEl2=Srv->IDT_NeuronLocalCustomer->FindNextET(&Anchor,FStr,AcsMd_Read,tEl);
		}
		Srv->IDT_NeuronLocalCustomer->UnLockEl(&Anchor,tEl);
	}
	return tEl_R;
};

El_Tab*		Services::FindUsersGroupNext(char *Name,FindStruct*	FStr)
{
	El_Tab*	tEl;
	El_Tab*	tEl2;
	El_Tab*	tEl3;
	El_Tab*	tEl_R=NULL;

	tEl=Srv->IDT_NeuronLocalCustomer->FindNested(&Anchor,AcsMd_Read,N_APCS,N_WorkGroups,NULL);
	if (tEl)
	{
		tEl2=Srv->IDT_NeuronLocalCustomer->FindNextET(&Anchor,FStr,AcsMd_Read,tEl);
		while(tEl2)
		{
			tEl3=IDT_NeuronLocalCustomer->FindNestedStr(&Anchor,AcsMd_Read,N_Users,tEl2);
			if (tEl3)
			{
				tEl_R=IDT_NeuronLocalCustomer->FindNestedStr(&Anchor,AcsMd_Read,Name,tEl3);
				if(tEl_R)
				{
					Srv->IDT_NeuronLocalCustomer->UnLockEl(&Anchor,tEl3);
					Srv->IDT_NeuronLocalCustomer->UnLockEl(&Anchor,tEl2);
					break;
				}
				Srv->IDT_NeuronLocalCustomer->UnLockEl(&Anchor,tEl3);
			}
			Srv->IDT_NeuronLocalCustomer->UnLockEl(&Anchor,tEl2);
			tEl2=Srv->IDT_NeuronLocalCustomer->FindNextET(&Anchor,FStr,AcsMd_Read,tEl);
		}
		Srv->IDT_NeuronLocalCustomer->UnLockEl(&Anchor,tEl);
	}
	return tEl_R;
};

El_Tab*		Services::GetGroupElByUser(El_Tab* User_El)
{
	El_Tab*	tEl_R=NULL;
	if (User_El->pParent)
		tEl_R=User_El->pParent->pParent;
	if (tEl_R) 
		Srv->IDT_NeuronLocalCustomer->LockEl(&Anchor,tEl_R);
	return	tEl_R;
};

bool	Services::DelUser(char *Name)
{
	bool RC=FALSE;
	El_Tab*	tEl;
	El_Tab*	tEl2;
	El_Tab*	tEl3;
	tEl=Srv->IDT_NeuronLocalCustomer->FindNested(&Anchor,AcsMd_Read,N_APCS,N_Users,Name,NULL);
	if (tEl)
	{
		tEl2=Srv->IDT_NeuronLocalCustomer->FindNested(&Anchor,AcsMd_Read,N_APCS,N_Users,Name,N_WorkGroupsIn,NULL);
		if(tEl2)
		{
			char		tBuf[128];
			FindStruct	FStr;
			void*		pVol;
			tEl3=Srv->IDT_NeuronLocalCustomer->FindFirstET(&Anchor,&FStr,AcsMd_Read,tEl2);
			while(tEl3)
			{
				if (Srv->IDT_NeuronLocalCustomer->GetParam(&Anchor,tEl3,IDNameElTab,pVol)!=-1)
				{
					Srv->DelUserFromGroup(Name,(char*)pVol);
				};
				IDT_NeuronLocalCustomer->UnLockEl(&Anchor,tEl3);
				tEl3=Srv->IDT_NeuronLocalCustomer->FindNextET(&Anchor,&FStr,AcsMd_Read,tEl2);
			}
			IDT_NeuronLocalCustomer->UnLockEl(&Anchor,tEl2);
		}
		Srv->IDT_NeuronLocalCustomer->DelElTab(&Anchor,tEl);
		RC=true;
	}
	return RC;
};
bool	Services::DelGroup(char *Group)
{
	bool RC=FALSE;
	El_Tab*	tEl;
	El_Tab*	tEl2;
	El_Tab*	tEl3;
	tEl=Srv->IDT_NeuronLocalCustomer->FindNested(&Anchor,AcsMd_Read,N_APCS,N_WorkGroups,Group,NULL);
	if (tEl)
	{
		char		tBuf[128];
		FindStruct	FStr;
		void*		pVol;
		tEl2=Srv->IDT_NeuronLocalCustomer->FindNested(&Anchor,AcsMd_Read,N_APCS,N_WorkGroups,Group,N_Users,NULL);
		if(tEl2)
		{
			tEl3=Srv->IDT_NeuronLocalCustomer->FindFirstET(&Anchor,&FStr,AcsMd_Read,tEl2);
			while(tEl3)
			{
				if (Srv->IDT_NeuronLocalCustomer->GetParam(&Anchor,tEl3,IDNameElTab,pVol)!=-1)
				{
					Srv->DelUserFromGroup((char*)pVol,Group);
				};
				IDT_NeuronLocalCustomer->UnLockEl(&Anchor,tEl3);
				tEl3=Srv->IDT_NeuronLocalCustomer->FindNextET(&Anchor,&FStr,AcsMd_Read,tEl2);
			}
			IDT_NeuronLocalCustomer->UnLockEl(&Anchor,tEl2);
		}
		tEl2=Srv->IDT_NeuronLocalCustomer->FindNested(&Anchor,AcsMd_Read,N_APCS,N_WorkGroups,Group,N_WorkGroupsIn,NULL);
		if(tEl2)
		{
			tEl3=Srv->IDT_NeuronLocalCustomer->FindFirstET(&Anchor,&FStr,AcsMd_Read,tEl2);
			while(tEl3)
			{
				if (Srv->IDT_NeuronLocalCustomer->GetParam(&Anchor,tEl3,IDNameElTab,pVol)!=-1)
				{
					Srv->DelGroupFromGroup(Group,(char*)pVol);
				};
				IDT_NeuronLocalCustomer->UnLockEl(&Anchor,tEl3);
				tEl3=Srv->IDT_NeuronLocalCustomer->FindNextET(&Anchor,&FStr,AcsMd_Read,tEl2);
			}
			IDT_NeuronLocalCustomer->UnLockEl(&Anchor,tEl2);
		}
		tEl2=Srv->IDT_NeuronLocalCustomer->FindNested(&Anchor,AcsMd_Read,N_APCS,N_WorkGroups,Group,N_WorkGroups,NULL);
		if(tEl2)
		{
			tEl3=Srv->IDT_NeuronLocalCustomer->FindFirstET(&Anchor,&FStr,AcsMd_Read,tEl2);
			while(tEl3)
			{
				if (Srv->IDT_NeuronLocalCustomer->GetParam(&Anchor,tEl3,IDNameElTab,pVol)!=-1)
				{
					Srv->DelGroupFromGroup((char*)pVol,Group);
				};
				IDT_NeuronLocalCustomer->UnLockEl(&Anchor,tEl3);
				tEl3=Srv->IDT_NeuronLocalCustomer->FindNextET(&Anchor,&FStr,AcsMd_Read,tEl2);
			}
			IDT_NeuronLocalCustomer->UnLockEl(&Anchor,tEl2);
		}
		Srv->IDT_NeuronLocalCustomer->DelElTab(&Anchor,tEl);
		RC=true;
	}
	return RC;
};

bool	Services::DelUserFromGroup(char *Name,char *Group)
{
	bool RC=FALSE;
	El_Tab	*tEl;
	tEl=Srv->IDT_NeuronLocalCustomer->FindNested(&Anchor,AcsMd_Read,N_APCS,N_WorkGroups,Group,N_Users,Name,NULL);
	if (tEl)
	{
		Srv->IDT_NeuronLocalCustomer->DelElTab(&Anchor,tEl);
		RC=TRUE;
		tEl=Srv->IDT_NeuronLocalCustomer->FindNested(&Anchor,AcsMd_Read,N_APCS,N_Users,Name,N_WorkGroupsIn,Group,NULL);
		if (tEl)
		{
			Srv->IDT_NeuronLocalCustomer->DelElTab(&Anchor,tEl);
		}
	}
	return RC;
};

bool	Services::DelGroupFromGroup(char *Group,char *GroupFrom)
{
	bool RC=FALSE;
	El_Tab	*tEl;
	tEl=Srv->IDT_NeuronLocalCustomer->FindNested(&Anchor,AcsMd_Read,N_APCS,N_WorkGroups,GroupFrom,N_WorkGroups,Group,NULL);
	if (tEl)
	{
		Srv->IDT_NeuronLocalCustomer->DelElTab(&Anchor,tEl);
		RC=TRUE;
		tEl=Srv->IDT_NeuronLocalCustomer->FindNested(&Anchor,AcsMd_Read,N_APCS,N_WorkGroups,Group,N_WorkGroupsIn,GroupFrom,NULL);
		if (tEl)
		{
			Srv->IDT_NeuronLocalCustomer->DelElTab(&Anchor,tEl);
		}
	}
	return RC;
};
// ---------------------------------------------------
bool	Services::TestLoginName(char *Name)
{
	bool RC=FALSE;
	El_Tab	*tEl;
	tEl=IDT_NeuronLocalCustomer->FindNested(&Anchor,AcsMd_Read,N_APCS,N_Users,Name,NULL);
	if (tEl) RC=TRUE;
	IDT_NeuronLocalCustomer->UnLockEl(&Anchor,tEl);
	return RC;
};

INT64		Services::GetUserID(char *Name)
{
	El_Tab	*tEl;
	INT64	UID=0;
	tEl=IDT_NeuronLocalCustomer->FindNested(&Anchor,AcsMd_Read,N_APCS,N_Users,Name,NULL);
	if (tEl) UID=tEl->GetIDI()->ID_File;
	IDT_NeuronLocalCustomer->UnLockEl(&Anchor,tEl);
	return UID;
}
INT64		Services::GetWorkGroupID(char *Group)
{
	El_Tab	*tEl;
	INT64	WGID=0;
	tEl=IDT_NeuronLocalCustomer->FindNested(&Anchor,AcsMd_Read,N_APCS,N_WorkGroups,Group,NULL);
	if (tEl) WGID=tEl->GetIDI()->ID_File;
	IDT_NeuronLocalCustomer->UnLockEl(&Anchor,tEl);
	return WGID;
}
char*		Services::GetUserNameByID(INT64	UserID)
{
	El_Tab	*tEl;
	void*	pVol=NULL;
	tEl=IDT_NeuronLocalCustomer->GetElTab(&Anchor,UserID);
	if (tEl)
	{
		if (IDT_NeuronLocalCustomer->GetParam(&Anchor,tEl->pParent,IDNameElTab,pVol)!=-1)
		{
			if (strcmp((char*)pVol,N_Users)==0)
			{
				if (IDT_NeuronLocalCustomer->GetParam(&Anchor,tEl,IDNameElTab,pVol)==-1)
					pVol=NULL;
			}
			else
			{
				pVol=NULL;
			}
		}
		else
		{
			pVol=NULL;
		}
		IDT_NeuronLocalCustomer->UnLockEl(&Anchor,tEl);
	}
	return (char*)pVol;
}
char*		Services::GetWorkGroupNameByID(INT64	UserID)
{
	El_Tab	*tEl;
	void*	pVol=NULL;
	tEl=IDT_NeuronLocalCustomer->GetElTab(&Anchor,UserID);
	if (tEl)
	{
		if (IDT_NeuronLocalCustomer->GetParam(&Anchor,tEl->pParent,IDNameElTab,pVol)!=-1)
		{
			if (strcmp((char*)pVol,N_WorkGroups)==0)
			{
				if (IDT_NeuronLocalCustomer->GetParam(&Anchor,tEl,IDNameElTab,pVol)==-1)
					pVol=NULL;
			}
			else
			{
				pVol=NULL;
			}
		}
		else
		{
			pVol=NULL;
		}
		IDT_NeuronLocalCustomer->UnLockEl(&Anchor,tEl);
	}
	return (char*)pVol;
}

bool	Services::TestGroupLevelDown(char *Group)
{
	bool RC=FALSE;
	El_Tab	*tEl;
	El_Tab	*tEl2;
	El_Tab	*tEl3;
	void	*pVol;
	if (UserID!=-1)
	{
		tEl=IDT_NeuronLocalCustomer->FindNested(&Anchor,AcsMd_Read,N_APCS,N_WorkGroups,Group,N_WorkGroupsIn,NULL);
		if (tEl)
		{
			FindStruct	FStrG;
			tEl2=IDT_NeuronLocalCustomer->FindFirstET(&Anchor,&FStrG,AcsMd_Read,tEl);
			while(tEl2)
			{
				if (IDT_NeuronLocalCustomer->GetParam(&Anchor,tEl2,IDNameElTab,pVol)!=-1)
				{
					tEl3=IDT_NeuronLocalCustomer->FindNested(&Anchor,AcsMd_Read,N_APCS,N_WorkGroups,(char*)pVol,N_Users,UserLogin,NULL);
					if (tEl3)
					{
						RC=TRUE;
						IDT_NeuronLocalCustomer->UnLockEl(&Anchor,tEl3);
					}
				};
				IDT_NeuronLocalCustomer->UnLockEl(&Anchor,tEl2);
				if (RC)
					break;
				tEl2=IDT_NeuronLocalCustomer->FindNextET(&Anchor,&FStrG,AcsMd_Read,tEl);
			}
			IDT_NeuronLocalCustomer->UnLockEl(&Anchor,tEl);
		}
	};
	return RC;
};

bool	Services::TestUserLevelDown(char *Name)
{
	bool RC=FALSE;
	El_Tab	*tEl;
	El_Tab	*tEl2;
	El_Tab	*tEl3;
	El_Tab	*tEl4;
	void	*pVol;
	if (UserID!=-1)
	{
		tEl=IDT_NeuronLocalCustomer->FindNested(&Anchor,AcsMd_Read,N_APCS,N_Users,Name,N_WorkGroupsIn,NULL);
		if (tEl)
		{
			FindStruct	FStrG;
			tEl2=IDT_NeuronLocalCustomer->FindFirstET(&Anchor,&FStrG,AcsMd_Read,tEl);
			while(tEl2)
			{
				if (IDT_NeuronLocalCustomer->GetParam(&Anchor,tEl2,IDNameElTab,pVol)!=-1)
				{
					tEl3=IDT_NeuronLocalCustomer->FindNested(&Anchor,AcsMd_Read,N_APCS,N_WorkGroups,(char*)pVol,N_Users,UserLogin,NULL);
					if (tEl3)
					{
						tEl4=IDT_NeuronLocalCustomer->FindNested(&Anchor,AcsMd_Read,N_APCS,N_WorkGroups,(char*)pVol,N_Users,Name,NULL);
						if(tEl4)
						{
							int PriorityUser=0;
							int PriorityMy=0;
							if (IDT_NeuronLocalCustomer->GetParam(&Anchor,tEl3,IDPriority,pVol)!=-1)
								PriorityMy=*(int*)pVol;
							if (IDT_NeuronLocalCustomer->GetParam(&Anchor,tEl4,IDPriority,pVol)!=-1)
								PriorityUser=*(int*)pVol;
							if (PriorityMy>=PriorityUser)
								RC=TRUE;
							IDT_NeuronLocalCustomer->UnLockEl(&Anchor,tEl4);
							IDT_NeuronLocalCustomer->UnLockEl(&Anchor,tEl3);
						}
					}
				};
				IDT_NeuronLocalCustomer->UnLockEl(&Anchor,tEl2);
				if (RC)
					break;
				tEl2=IDT_NeuronLocalCustomer->FindNextET(&Anchor,&FStrG,AcsMd_Read,tEl);
			}
			IDT_NeuronLocalCustomer->UnLockEl(&Anchor,tEl);
		}
	};
	return RC;
};

bool	Services::TestGroupLevelUp(char *Group)
{
	bool RC=FALSE;
	El_Tab	*tEl;
	El_Tab	*tEl2;
	El_Tab	*tEl3;
	void	*pVol;
	if (UserID!=-1)
	{
		tEl=IDT_NeuronLocalCustomer->FindNested(&Anchor,AcsMd_Read,N_APCS,N_Users,UserLogin,N_WorkGroupsIn,NULL);
		if (tEl)
		{
			FindStruct	FStrG;
			tEl2=IDT_NeuronLocalCustomer->FindFirstET(&Anchor,&FStrG,AcsMd_Read,tEl);
			while(tEl2)
			{
				if (IDT_NeuronLocalCustomer->GetParam(&Anchor,tEl2,IDNameElTab,pVol)!=-1)
				{
					tEl3=IDT_NeuronLocalCustomer->FindNested(&Anchor,AcsMd_Read,N_APCS,N_WorkGroups,Group,N_WorkGroups,(char*)pVol,NULL);
					if (tEl3)
					{
						RC=TRUE;
						IDT_NeuronLocalCustomer->UnLockEl(&Anchor,tEl3);
					}
				};
				IDT_NeuronLocalCustomer->UnLockEl(&Anchor,tEl2);
				if (RC)
					break;
				tEl2=IDT_NeuronLocalCustomer->FindNextET(&Anchor,&FStrG,AcsMd_Read,tEl);
			}
			IDT_NeuronLocalCustomer->UnLockEl(&Anchor,tEl);
		}
	};
	return RC;
};

bool	Services::TestUserLevelUp(char *Name)
{
	bool RC=FALSE;
	El_Tab	*tEl;
	El_Tab	*tEl2;
	El_Tab	*tEl3;
	El_Tab	*tEl4;
	void	*pVol;
	if (UserID!=-1)
	{
		tEl=IDT_NeuronLocalCustomer->FindNested(&Anchor,AcsMd_Read,N_APCS,N_Users,Name,N_WorkGroupsIn,NULL);
		if (tEl)
		{
			FindStruct	FStrG;
			tEl2=IDT_NeuronLocalCustomer->FindFirstET(&Anchor,&FStrG,AcsMd_Read,tEl);
			while(tEl2)
			{
				if (IDT_NeuronLocalCustomer->GetParam(&Anchor,tEl2,IDNameElTab,pVol)!=-1)
				{
					tEl3=IDT_NeuronLocalCustomer->FindNested(&Anchor,AcsMd_Read,N_APCS,N_WorkGroups,(char*)pVol,N_Users,UserLogin,NULL);
					if (tEl3)
					{
						tEl4=IDT_NeuronLocalCustomer->FindNested(&Anchor,AcsMd_Read,N_APCS,N_WorkGroups,(char*)pVol,N_Users,Name,NULL);
						if(tEl4)
						{
							int PriorityUser=0;
							int PriorityMy=0;
							if (IDT_NeuronLocalCustomer->GetParam(&Anchor,tEl3,IDPriority,pVol)!=-1)
								PriorityMy=*(int*)pVol;
							if (IDT_NeuronLocalCustomer->GetParam(&Anchor,tEl4,IDPriority,pVol)!=-1)
								PriorityUser=*(int*)pVol;
							if (PriorityMy<PriorityUser)
								RC=TRUE;
							IDT_NeuronLocalCustomer->UnLockEl(&Anchor,tEl4);
							IDT_NeuronLocalCustomer->UnLockEl(&Anchor,tEl3);
						}
					}
				};
				IDT_NeuronLocalCustomer->UnLockEl(&Anchor,tEl2);
				if (RC)
					break;
				tEl2=IDT_NeuronLocalCustomer->FindNextET(&Anchor,&FStrG,AcsMd_Read,tEl);
			}
			IDT_NeuronLocalCustomer->UnLockEl(&Anchor,tEl);
		}
	};
	return RC;
};

bool	Services::GetActiveUser(INT64 UserID)
{
	FindStrList8	FSL_UL;
	FindStrList8	FSL_UL2;
	bool RC=FALSE;
	El_DomName*	ElDN;
	El_Unit	*tEl;
	ID_List8*	UList=Srv->UM->FindUnit("Services");
	if (UList)
	{
		ElDN=(El_DomName*)UList->Begin(FSL_UL);
		while(ElDN)
		{
			tEl=(El_Unit*)ElDN->UnitList->Begin(FSL_UL2);
			while (tEl)
			{
				if (tEl->UInf.OwnerID==UserID)
				{
					RC=TRUE;
					break;
				}
				tEl=(El_Unit*)ElDN->UnitList->Next(FSL_UL2);
			}
			if(RC)
				break;
			ElDN=(El_DomName*)UList->Next(FSL_UL);
		}
	}
	return RC;
};

bool	Services::GetActiveUser(char *Name)
{
	FindStrList8	FSL_UL;
	FindStrList8	FSL_UL2;
	bool RC=FALSE;
	El_DomName*	ElDN;
	El_Unit	*tEl;
	ID_List8*	UList=Srv->UM->FindUnit("Services");
	if (UList)
	{
		char *pNameRet;
		ElDN=(El_DomName*)UList->Begin(FSL_UL);
		while(ElDN)
		{
			tEl=(El_Unit*)ElDN->UnitList->Begin(FSL_UL2);
			while (tEl)
			{
				pNameRet=GetUserNameByID(tEl->UInf.OwnerID);
				if (pNameRet)
				{
					if (strcmp(pNameRet,Name)==0)
					{
						RC=TRUE;
						break;
					}
				}
				tEl=(El_Unit*)ElDN->UnitList->Next(FSL_UL2);
			}
			if(RC)
				break;
			ElDN=(El_DomName*)UList->Next(FSL_UL);
		}
	}
	return RC;
};

bool	Services::AddHistoryLine(El_Tab* tEl,char* sBuf,int Fl_UnRead)
{
	bool	rc=FALSE;
	El_Tab* tEl2;
	El_Tab* tEl3;
	tEl2=IDT_NeuronLocalCustomer->Find(&Anchor,AcsMd_Read,"History",tEl);
	if (tEl2==NULL)
	{
		tEl2=IDT_NeuronLocalCustomer->AddET(&Anchor,"History",tEl);
	};
	if (tEl2)
	{
		tEl3=Srv->IDT_NeuronLocalCustomer->AddET(&Anchor,tEl2);
		if (tEl3)
		{
			char	ttBf[80];
			sprintf(ttBf,"%I64x",tEl3->GetIDI()->ID_File);
			IDT_NeuronLocalCustomer->AddParam(&Anchor,tEl3,IDNameElTab,ttBf,1);
			IDT_NeuronLocalCustomer->AddParam(&Anchor,tEl3,IDRemark,sBuf);
			IDT_NeuronLocalCustomer->UnLockEl(&Anchor,tEl3);
			rc=TRUE;
		};
		IDT_NeuronLocalCustomer->UnLockEl(&Anchor,tEl2);
	};
	if (Fl_UnRead==0)
	{
		UnSelectMsg(tEl);
		Is_SelectingMsg(tEl);
	}
	return rc;
};

bool	Services::Is_IncomingMsg(El_Tab* tEl)
{
	bool RC=FALSE;
	El_Tab* tElU;
	El_Tab* tEl2;
	El_Tab* tEl3;
	El_Tab* tEl4;
	INT64	IDResponPersonID;
	void	*pVol;

	tElU=IDT_NeuronLocalCustomer->FindNested(&Anchor,AcsMd_Read,N_APCS,N_Users,UserLogin,N_WorkGroupsIn,NULL);
	if (tElU)
	{
		if (IDT_NeuronLocalCustomer->GetParam(&Anchor,tEl,IDResponsiblePersonID,pVol)!=-1)
		{
			IDResponPersonID=*(INT64*)pVol;
			if (IDResponPersonID==UserID)
				RC=TRUE;
		};
		if (RC)
			goto _mmm2;
		tEl2=IDT_NeuronLocalCustomer->FindNestedStr(&Anchor,AcsMd_Read,N_Recipients,tEl);
		if (tEl2)
		{
			tEl3=IDT_NeuronLocalCustomer->FindNestedStr(&Anchor,AcsMd_Read,UserLogin,tEl2);
			if(tEl3)
			{
				RC=TRUE;
				IDT_NeuronLocalCustomer->UnLockEl(&Anchor,tEl3);
			}
			IDT_NeuronLocalCustomer->UnLockEl(&Anchor,tEl2);
		}
		if (RC)
			goto _mmm2;
		tEl2=IDT_NeuronLocalCustomer->FindNestedStr(&Anchor,AcsMd_Read,N_RecipientGroups,tEl);
		if (tEl2)
		{
			FindStruct	FStrGr;
			tEl3=IDT_NeuronLocalCustomer->FindFirstET(&Anchor,&FStrGr,AcsMd_Read,tEl2);
			while(tEl3)
			{
				if (IDT_NeuronLocalCustomer->GetParam(&Anchor,tEl3,IDNameElTab,pVol)!=-1)
				{
					tEl4=IDT_NeuronLocalCustomer->FindNestedStr(&Anchor,AcsMd_Read,(char*)pVol,tElU);
					if (tEl4)
					{
						RC=TRUE;
						IDT_NeuronLocalCustomer->UnLockEl(&Anchor,tEl4);
						break;
					}
				}
				IDT_NeuronLocalCustomer->UnLockEl(&Anchor,tEl3);
				tEl3=IDT_NeuronLocalCustomer->FindNextET(&Anchor,&FStrGr,AcsMd_Read,tEl2);
			}
			IDT_NeuronLocalCustomer->UnLockEl(&Anchor,tEl2);
		}
		if (RC)
			goto _mmm2;
		tEl2=IDT_NeuronLocalCustomer->FindNestedStr(&Anchor,AcsMd_Read,N_Implementers,tEl);
		if (tEl2)
		{
			tEl3=IDT_NeuronLocalCustomer->FindNestedStr(&Anchor,AcsMd_Read,UserLogin,tEl2);
			if(tEl3)
			{
				RC=TRUE;
				IDT_NeuronLocalCustomer->UnLockEl(&Anchor,tEl3);
			}
			IDT_NeuronLocalCustomer->UnLockEl(&Anchor,tEl2);
		}
		if (RC)
			goto _mmm2;
		tEl2=IDT_NeuronLocalCustomer->FindNestedStr(&Anchor,AcsMd_Read,N_Auditors,tEl);
		if (tEl2)
		{
			tEl3=IDT_NeuronLocalCustomer->FindNestedStr(&Anchor,AcsMd_Read,UserLogin,tEl2);
			if(tEl3)
			{
				RC=TRUE;
				IDT_NeuronLocalCustomer->UnLockEl(&Anchor,tEl3);
			}
			IDT_NeuronLocalCustomer->UnLockEl(&Anchor,tEl2);
		}
		if (RC)
			goto _mmm2;
		tEl2=IDT_NeuronLocalCustomer->FindNestedStr(&Anchor,AcsMd_Read,N_AuditorGroups,tEl);
		if (tEl2)
		{
			FindStruct	FStrGr;
			tEl3=IDT_NeuronLocalCustomer->FindFirstET(&Anchor,&FStrGr,AcsMd_Read,tEl2);
			while(tEl3)
			{
				if (IDT_NeuronLocalCustomer->GetParam(&Anchor,tEl3,IDNameElTab,pVol)!=-1)
				{
					tEl4=IDT_NeuronLocalCustomer->FindNestedStr(&Anchor,AcsMd_Read,(char*)pVol,tElU);
					if (tEl4)
					{
						RC=TRUE;
						IDT_NeuronLocalCustomer->UnLockEl(&Anchor,tEl4);
						break;
					}
				}
				IDT_NeuronLocalCustomer->UnLockEl(&Anchor,tEl3);
				tEl3=IDT_NeuronLocalCustomer->FindNextET(&Anchor,&FStrGr,AcsMd_Read,tEl2);
			}
			IDT_NeuronLocalCustomer->UnLockEl(&Anchor,tEl2);
		}
_mmm2:
		IDT_NeuronLocalCustomer->UnLockEl(&Anchor,tElU);
	}
	return RC;
};

bool	Services::Is_SelectingMsg(El_Tab* tEl)
{
	bool RC=FALSE;
	El_Tab* tElU;
	El_Tab* tEl2;
	El_Tab* tEl3;
	El_Tab* tEl4;
	INT64	IDResponPersonID;
	void	*pVol;
	void	*pVol2;
	INT64	Attr;
	char	tsBf[256];
	tElU=IDT_NeuronLocalCustomer->FindNested(&Anchor,AcsMd_Read,N_APCS,N_Users,UserLogin,N_WorkGroupsIn,NULL);
	if (tElU)
	{
		if (IDT_NeuronLocalCustomer->GetParam(&Anchor,tEl,IDResponsiblePersonID,pVol)!=-1)
		{
			IDResponPersonID=*(INT64*)pVol;
			if (IDResponPersonID==UserID)
			{
				Attr=MA_ResponsibleReaded;
				if (IDT_NeuronLocalCustomer->GetParam(&Anchor,tEl,IDMsgAttribute,pVol)!=-1)
				{
					Attr=*(INT64*)pVol;
					if ((Attr&MA_ResponsibleReaded)==0)
					{
						sprintf(tsBf,"ResponsiblePerson %s already read this message",UserLogin);
						Srv->AddHistoryLine(tEl,tsBf,1);
						Attr|=MA_ResponsibleReaded;Srv->IDT_NeuronLocalCustomer->AddParam(&Anchor,tEl,IDMsgAttribute,Attr);
					}
				}
				else
				{
					Srv->IDT_NeuronLocalCustomer->AddParam(&Anchor,tEl,IDMsgAttribute,Attr);
					sprintf(tsBf,"ResponsiblePerson %s already read this message",UserLogin);
					Srv->AddHistoryLine(tEl,tsBf,1);
				}
//				RC=TRUE;
			}
		};
		if (IDT_NeuronLocalCustomer->GetParam(&Anchor,tEl,IDOriginatorID,pVol)!=-1)
		{
			IDResponPersonID=*(INT64*)pVol;
			if (IDResponPersonID==UserID)
			{
				Attr=MA_OriginatorReaded;
				if (IDT_NeuronLocalCustomer->GetParam(&Anchor,tEl,IDMsgAttribute,pVol2)!=-1)
				{
					Attr=*(INT64*)pVol2;
					if ((Attr&MA_OriginatorReaded)==0)
					{
						sprintf(tsBf,"OriginatorPerson %s already read this message",UserLogin);
						Srv->AddHistoryLine(tEl,tsBf,1);
						Attr|=MA_OriginatorReaded;Srv->IDT_NeuronLocalCustomer->AddParam(&Anchor,tEl,IDMsgAttribute,Attr);
					}
				}
				else
				{
					Srv->IDT_NeuronLocalCustomer->AddParam(&Anchor,tEl,IDMsgAttribute,Attr);
					sprintf(tsBf,"OriginatorPerson %s already read this message",UserLogin);
					Srv->AddHistoryLine(tEl,tsBf,1);
				}
				//				RC=TRUE;
			}
		};
// 		if (RC)
// 			goto _mmm2;
		tEl2=IDT_NeuronLocalCustomer->FindNestedStr(&Anchor,AcsMd_Read,N_Recipients,tEl);
		if (tEl2)
		{
			tEl3=IDT_NeuronLocalCustomer->FindNestedStr(&Anchor,AcsMd_Read,UserLogin,tEl2);
			if(tEl3)
			{
				Attr=MA_Readed;
				if (IDT_NeuronLocalCustomer->GetParam(&Anchor,tEl3,IDMsgAttribute,pVol)!=-1)
				{
					Attr=*(INT64*)pVol;
					if ((Attr&MA_Readed)==0)
					{
						sprintf(tsBf,"Recipient %s already read this message",UserLogin);
						Srv->AddHistoryLine(tEl,tsBf,1);
						Attr|=MA_Readed;Srv->IDT_NeuronLocalCustomer->AddParam(&Anchor,tEl3,IDMsgAttribute,Attr);
					}
				}
				else
				{
					Srv->IDT_NeuronLocalCustomer->AddParam(&Anchor,tEl3,IDMsgAttribute,Attr);
					sprintf(tsBf,"Recipient %s already read this message",UserLogin);
					Srv->AddHistoryLine(tEl,tsBf,1);
				}
//				RC=TRUE;// !!!
				IDT_NeuronLocalCustomer->UnLockEl(&Anchor,tEl3);
			}
			IDT_NeuronLocalCustomer->UnLockEl(&Anchor,tEl2);
		}
// 		if (RC)
// 			goto _mmm2;
		tEl2=IDT_NeuronLocalCustomer->FindNestedStr(&Anchor,AcsMd_Read,N_RecipientGroups,tEl);
		if (tEl2)
		{
			FindStruct	FStrGr;
			tEl3=IDT_NeuronLocalCustomer->FindFirstET(&Anchor,&FStrGr,AcsMd_Read,tEl2);
			while(tEl3)
			{
				if (IDT_NeuronLocalCustomer->GetParam(&Anchor,tEl3,IDNameElTab,pVol)!=-1)
				{
					tEl4=IDT_NeuronLocalCustomer->FindNestedStr(&Anchor,AcsMd_Read,(char*)pVol,tElU);
					if (tEl4)
					{
						Attr=MA_Readed;
						if (IDT_NeuronLocalCustomer->GetParam(&Anchor,tEl4,IDMsgAttribute,pVol2)!=-1)
						{
							Attr=*(INT64*)pVol2;
							if ((Attr&MA_Readed)==0)
							{
								sprintf(tsBf,"Recipient Group %s already read this message",(char*)pVol);
								Srv->AddHistoryLine(tEl,tsBf,1);
								Attr|=MA_Readed;Srv->IDT_NeuronLocalCustomer->AddParam(&Anchor,tEl4,IDMsgAttribute,Attr);
							}
						}
						else
						{
							Srv->IDT_NeuronLocalCustomer->AddParam(&Anchor,tEl4,IDMsgAttribute,Attr);
							sprintf(tsBf,"Recipient Group %s already read this message",(char*)pVol);
							Srv->AddHistoryLine(tEl,tsBf,1);
						}
//						RC=TRUE;	// !!!
						IDT_NeuronLocalCustomer->UnLockEl(&Anchor,tEl4);
//						break;
					}
				}
				IDT_NeuronLocalCustomer->UnLockEl(&Anchor,tEl3);
				tEl3=IDT_NeuronLocalCustomer->FindNextET(&Anchor,&FStrGr,AcsMd_Read,tEl2);
			}
			IDT_NeuronLocalCustomer->UnLockEl(&Anchor,tEl2);
		}
//		if (RC)
//			goto _mmm2;
		tEl2=IDT_NeuronLocalCustomer->FindNestedStr(&Anchor,AcsMd_Read,N_Implementers,tEl);
		if (tEl2)
		{
			tEl3=IDT_NeuronLocalCustomer->FindNestedStr(&Anchor,AcsMd_Read,UserLogin,tEl2);
			if(tEl3)
			{
				Attr=MA_Readed;
				if (IDT_NeuronLocalCustomer->GetParam(&Anchor,tEl3,IDMsgAttribute,pVol2)!=-1)
				{
					Attr=*(INT64*)pVol2;
					if ((Attr&MA_Readed)==0)
					{
						sprintf(tsBf,"Implementer %s already read this message",UserLogin);
						Srv->AddHistoryLine(tEl,tsBf,1);
						Attr|=MA_Readed;Srv->IDT_NeuronLocalCustomer->AddParam(&Anchor,tEl3,IDMsgAttribute,Attr);
					}
				}
				else
				{
					Srv->IDT_NeuronLocalCustomer->AddParam(&Anchor,tEl3,IDMsgAttribute,Attr);
					sprintf(tsBf,"Implementer %s already read this message",UserLogin);
					Srv->AddHistoryLine(tEl,tsBf,1);
				}
//				RC=TRUE;	// !!!
				IDT_NeuronLocalCustomer->UnLockEl(&Anchor,tEl3);
			}
			IDT_NeuronLocalCustomer->UnLockEl(&Anchor,tEl2);
		}
// 		if (RC)
// 			goto _mmm2;
		tEl2=IDT_NeuronLocalCustomer->FindNestedStr(&Anchor,AcsMd_Read,N_Auditors,tEl);
		if (tEl2)
		{
			tEl3=IDT_NeuronLocalCustomer->FindNestedStr(&Anchor,AcsMd_Read,UserLogin,tEl2);
			if(tEl3)
			{
				Attr=MA_Readed;
				if (IDT_NeuronLocalCustomer->GetParam(&Anchor,tEl3,IDMsgAttribute,pVol)!=-1)
				{
					Attr=*(INT64*)pVol; 
					if ((Attr&MA_Readed)==0)
					{
						sprintf(tsBf,"Auditor %s already read this message",UserLogin);
						Srv->AddHistoryLine(tEl,tsBf,1);
						Attr|=MA_Readed;Srv->IDT_NeuronLocalCustomer->AddParam(&Anchor,tEl3,IDMsgAttribute,Attr);
					}
				}
				else
				{
					Srv->IDT_NeuronLocalCustomer->AddParam(&Anchor,tEl3,IDMsgAttribute,Attr);
					sprintf(tsBf,"Auditor %s already read this message",UserLogin);
					Srv->AddHistoryLine(tEl,tsBf,1);
				}
//				RC=TRUE;	// !!!
				IDT_NeuronLocalCustomer->UnLockEl(&Anchor,tEl3);
			}
			IDT_NeuronLocalCustomer->UnLockEl(&Anchor,tEl2);
		}
// 		if (RC)
// 			goto _mmm2;
		tEl2=IDT_NeuronLocalCustomer->FindNestedStr(&Anchor,AcsMd_Read,N_AuditorGroups,tEl);
		if (tEl2)
		{
			FindStruct	FStrGr;
			tEl3=IDT_NeuronLocalCustomer->FindFirstET(&Anchor,&FStrGr,AcsMd_Read,tEl2);
			while(tEl3)
			{
				if (IDT_NeuronLocalCustomer->GetParam(&Anchor,tEl3,IDNameElTab,pVol)!=-1)
				{
					tEl4=IDT_NeuronLocalCustomer->FindNestedStr(&Anchor,AcsMd_Read,(char*)pVol,tElU);
					if (tEl4)
					{
						Attr=MA_Readed;
						if (IDT_NeuronLocalCustomer->GetParam(&Anchor,tEl4,IDMsgAttribute,pVol2)!=-1)
						{
							Attr=*(INT64*)pVol2;
							if ((Attr&MA_Readed)==0)
							{
								sprintf(tsBf,"Auditor Group %s already read this message",(char*)pVol);
								Srv->AddHistoryLine(tEl,tsBf,1);
								Attr|=MA_Readed;Srv->IDT_NeuronLocalCustomer->AddParam(&Anchor,tEl4,IDMsgAttribute,Attr);
							}
						}
						else
						{
							Srv->IDT_NeuronLocalCustomer->AddParam(&Anchor,tEl4,IDMsgAttribute,Attr);
							sprintf(tsBf,"Auditor Group %s already read this message",(char*)pVol);
							Srv->AddHistoryLine(tEl,tsBf,1);
						}
//						RC=TRUE;	// !!!
						IDT_NeuronLocalCustomer->UnLockEl(&Anchor,tEl4);
						break;
					}
				}
				IDT_NeuronLocalCustomer->UnLockEl(&Anchor,tEl3);
				tEl3=IDT_NeuronLocalCustomer->FindNextET(&Anchor,&FStrGr,AcsMd_Read,tEl2);
			}
			IDT_NeuronLocalCustomer->UnLockEl(&Anchor,tEl2);
		}
//_mmm2:
		IDT_NeuronLocalCustomer->UnLockEl(&Anchor,tElU);
	}
	return RC;
};

bool	Services::UnSelectMsg(El_Tab* tEl)
{
	bool RC=FALSE;
	El_Tab* tElU;
	El_Tab* tEl2;
	El_Tab* tEl3;
	El_Tab* tEl4;
	INT64	IDResponPersonID;
	void	*pVol;
	INT64	Attr;
	char	tsBf[256];
	Attr=0;
	Srv->IDT_NeuronLocalCustomer->AddParam(&Anchor,tEl,IDMsgAttribute,Attr);
	tEl2=IDT_NeuronLocalCustomer->FindNestedStr(&Anchor,AcsMd_Read,N_Recipients,tEl);
	if (tEl2)
	{
		tEl3=IDT_NeuronLocalCustomer->FindNestedStr(&Anchor,AcsMd_Read,UserLogin,tEl2);
		if(tEl3)
		{
			Attr=0;
			Srv->IDT_NeuronLocalCustomer->AddParam(&Anchor,tEl3,IDMsgAttribute,Attr);
			IDT_NeuronLocalCustomer->UnLockEl(&Anchor,tEl3);
		}
		IDT_NeuronLocalCustomer->UnLockEl(&Anchor,tEl2);
	}
	tEl2=IDT_NeuronLocalCustomer->FindNestedStr(&Anchor,AcsMd_Read,N_Implementers,tEl);
	if (tEl2)
	{
		tEl3=IDT_NeuronLocalCustomer->FindNestedStr(&Anchor,AcsMd_Read,UserLogin,tEl2);
		if(tEl3)
		{
			Attr=0;
			Srv->IDT_NeuronLocalCustomer->AddParam(&Anchor,tEl3,IDMsgAttribute,Attr);
			IDT_NeuronLocalCustomer->UnLockEl(&Anchor,tEl3);
		}
		IDT_NeuronLocalCustomer->UnLockEl(&Anchor,tEl2);
	}
	tEl2=IDT_NeuronLocalCustomer->FindNestedStr(&Anchor,AcsMd_Read,N_Auditors,tEl);
	if (tEl2)
	{
		tEl3=IDT_NeuronLocalCustomer->FindNestedStr(&Anchor,AcsMd_Read,UserLogin,tEl2);
		if(tEl3)
		{
			Attr=0;
			Srv->IDT_NeuronLocalCustomer->AddParam(&Anchor,tEl3,IDMsgAttribute,Attr);
			IDT_NeuronLocalCustomer->UnLockEl(&Anchor,tEl3);
		}
		IDT_NeuronLocalCustomer->UnLockEl(&Anchor,tEl2);
	}
	tElU=IDT_NeuronLocalCustomer->FindNested(&Anchor,AcsMd_Read,N_APCS,N_Users,UserLogin,N_WorkGroupsIn,NULL);
	if (tElU)
	{
		tEl2=IDT_NeuronLocalCustomer->FindNestedStr(&Anchor,AcsMd_Read,N_RecipientGroups,tEl);
		if (tEl2)
		{
			FindStruct	FStrGr;
			tEl3=IDT_NeuronLocalCustomer->FindFirstET(&Anchor,&FStrGr,AcsMd_Read,tEl2);
			while(tEl3)
			{
				if (IDT_NeuronLocalCustomer->GetParam(&Anchor,tEl3,IDNameElTab,pVol)!=-1)
				{
					tEl4=IDT_NeuronLocalCustomer->FindNestedStr(&Anchor,AcsMd_Read,(char*)pVol,tElU);
					if (tEl4)
					{
						Attr=0;
						Srv->IDT_NeuronLocalCustomer->AddParam(&Anchor,tEl4,IDMsgAttribute,Attr);
						IDT_NeuronLocalCustomer->UnLockEl(&Anchor,tEl4);
					}
				}
				IDT_NeuronLocalCustomer->UnLockEl(&Anchor,tEl3);
				tEl3=IDT_NeuronLocalCustomer->FindNextET(&Anchor,&FStrGr,AcsMd_Read,tEl2);
			}
			IDT_NeuronLocalCustomer->UnLockEl(&Anchor,tEl2);
		}
		tEl2=IDT_NeuronLocalCustomer->FindNestedStr(&Anchor,AcsMd_Read,N_AuditorGroups,tEl);
		if (tEl2)
		{
			FindStruct	FStrGr;
			tEl3=IDT_NeuronLocalCustomer->FindFirstET(&Anchor,&FStrGr,AcsMd_Read,tEl2);
			while(tEl3)
			{
				if (IDT_NeuronLocalCustomer->GetParam(&Anchor,tEl3,IDNameElTab,pVol)!=-1)
				{
					tEl4=IDT_NeuronLocalCustomer->FindNestedStr(&Anchor,AcsMd_Read,(char*)pVol,tElU);
					if (tEl4)
					{
						Attr=0;
						Srv->IDT_NeuronLocalCustomer->AddParam(&Anchor,tEl4,IDMsgAttribute,Attr);
						IDT_NeuronLocalCustomer->UnLockEl(&Anchor,tEl4);
						break;
					}
				}
				IDT_NeuronLocalCustomer->UnLockEl(&Anchor,tEl3);
				tEl3=IDT_NeuronLocalCustomer->FindNextET(&Anchor,&FStrGr,AcsMd_Read,tEl2);
			}
			IDT_NeuronLocalCustomer->UnLockEl(&Anchor,tEl2);
		}
		IDT_NeuronLocalCustomer->UnLockEl(&Anchor,tElU);
	}
	return RC;
};

bool	Services::Is_ReadedMsg(El_Tab* tEl)
{
	bool RC1=TRUE;
	bool RC2=TRUE;
	bool RC3=TRUE;
	bool RC4=TRUE;
	bool RC5=TRUE;
	bool RC6=TRUE;
	El_Tab* tElU;
	El_Tab* tEl2;
	El_Tab* tEl3;
	El_Tab* tEl4;
	INT64	IDResponPersonID;
	void	*pVol;
	INT64	Attr;
	tElU=IDT_NeuronLocalCustomer->FindNested(&Anchor,AcsMd_Read,N_APCS,N_Users,UserLogin,N_WorkGroupsIn,NULL);
	if (tElU)
	{
		if (IDT_NeuronLocalCustomer->GetParam(&Anchor,tEl,IDResponsiblePersonID,pVol)!=-1)
		{
			IDResponPersonID=*(INT64*)pVol;
			if (IDResponPersonID==UserID)
			{
				if (IDT_NeuronLocalCustomer->GetParam(&Anchor,tEl,IDMsgAttribute,pVol)!=-1)
				{
					Attr=*(INT64*)pVol;
					if ((Attr&MA_ResponsibleReaded)==0)
					{
						RC1=FALSE;
					}
				}
				else
					RC1=FALSE;
			}
		};
		if (IDT_NeuronLocalCustomer->GetParam(&Anchor,tEl,IDOriginatorID,pVol)!=-1)
		{
			IDResponPersonID=*(INT64*)pVol;
			if (IDResponPersonID==UserID)
			{
				if (IDT_NeuronLocalCustomer->GetParam(&Anchor,tEl,IDMsgAttribute,pVol)!=-1)
				{
					Attr=*(INT64*)pVol;
					if ((Attr&MA_OriginatorReaded)==0)
					{
						RC1=FALSE;
					}
				}
				else
					RC1=FALSE;
			}
		};
		tEl2=IDT_NeuronLocalCustomer->FindNestedStr(&Anchor,AcsMd_Read,N_Recipients,tEl);
		if (tEl2)
		{
			tEl3=IDT_NeuronLocalCustomer->FindNestedStr(&Anchor,AcsMd_Read,UserLogin,tEl2);
			if(tEl3)
			{
				if (IDT_NeuronLocalCustomer->GetParam(&Anchor,tEl3,IDMsgAttribute,pVol)!=-1)
				{
					Attr=*(INT64*)pVol;
					if ((Attr&MA_Readed)==0)
					{
						RC2=FALSE;
					}
				}
				else
					RC2=FALSE;
				IDT_NeuronLocalCustomer->UnLockEl(&Anchor,tEl3);
			}
			IDT_NeuronLocalCustomer->UnLockEl(&Anchor,tEl2);
		}
		tEl2=IDT_NeuronLocalCustomer->FindNestedStr(&Anchor,AcsMd_Read,N_RecipientGroups,tEl);
		if (tEl2)
		{
			FindStruct	FStrGr;
			tEl3=IDT_NeuronLocalCustomer->FindFirstET(&Anchor,&FStrGr,AcsMd_Read,tEl2);
			while(tEl3)
			{
				if (IDT_NeuronLocalCustomer->GetParam(&Anchor,tEl3,IDNameElTab,pVol)!=-1)
				{
					tEl4=IDT_NeuronLocalCustomer->FindNestedStr(&Anchor,AcsMd_Read,(char*)pVol,tElU);
					if (tEl4)
					{
						if (IDT_NeuronLocalCustomer->GetParam(&Anchor,tEl4,IDMsgAttribute,pVol)!=-1)
						{
							Attr=*(INT64*)pVol;
							if ((Attr&MA_Readed)==0)
							{
								RC3=FALSE;
							}
						}
						else
							RC3=FALSE;
						IDT_NeuronLocalCustomer->UnLockEl(&Anchor,tEl4);
					}
				}
				IDT_NeuronLocalCustomer->UnLockEl(&Anchor,tEl3);
				tEl3=IDT_NeuronLocalCustomer->FindNextET(&Anchor,&FStrGr,AcsMd_Read,tEl2);
			}
			IDT_NeuronLocalCustomer->UnLockEl(&Anchor,tEl2);
		}
		tEl2=IDT_NeuronLocalCustomer->FindNestedStr(&Anchor,AcsMd_Read,N_Implementers,tEl);
		if (tEl2)
		{
			tEl3=IDT_NeuronLocalCustomer->FindNestedStr(&Anchor,AcsMd_Read,UserLogin,tEl2);
			if(tEl3)
			{
				if (IDT_NeuronLocalCustomer->GetParam(&Anchor,tEl3,IDMsgAttribute,pVol)!=-1)
				{
					Attr=*(INT64*)pVol;
					if ((Attr&MA_Readed)==0)
					{
						RC4=FALSE;
					}
				}
				else
					RC4=FALSE;
				IDT_NeuronLocalCustomer->UnLockEl(&Anchor,tEl3);
			}
			IDT_NeuronLocalCustomer->UnLockEl(&Anchor,tEl2);
		}
		tEl2=IDT_NeuronLocalCustomer->FindNestedStr(&Anchor,AcsMd_Read,N_Auditors,tEl);
		if (tEl2)
		{
			tEl3=IDT_NeuronLocalCustomer->FindNestedStr(&Anchor,AcsMd_Read,UserLogin,tEl2);
			if(tEl3)
			{
				if (IDT_NeuronLocalCustomer->GetParam(&Anchor,tEl3,IDMsgAttribute,pVol)!=-1)
				{
					Attr=*(INT64*)pVol;
					if ((Attr&MA_Readed)==0)
					{
						RC5=FALSE;
					}
				}
				else
					RC5=FALSE;
				IDT_NeuronLocalCustomer->UnLockEl(&Anchor,tEl3);
			}
			IDT_NeuronLocalCustomer->UnLockEl(&Anchor,tEl2);
		}
		tEl2=IDT_NeuronLocalCustomer->FindNestedStr(&Anchor,AcsMd_Read,N_AuditorGroups,tEl);
		if (tEl2)
		{
			FindStruct	FStrGr;
			tEl3=IDT_NeuronLocalCustomer->FindFirstET(&Anchor,&FStrGr,AcsMd_Read,tEl2);
			while(tEl3)
			{
				if (IDT_NeuronLocalCustomer->GetParam(&Anchor,tEl3,IDNameElTab,pVol)!=-1)
				{
					tEl4=IDT_NeuronLocalCustomer->FindNestedStr(&Anchor,AcsMd_Read,(char*)pVol,tElU);
					if (tEl4)
					{
						if (IDT_NeuronLocalCustomer->GetParam(&Anchor,tEl4,IDMsgAttribute,pVol)!=-1)
						{
							Attr=*(INT64*)pVol;
							if ((Attr&MA_Readed)==0)
							{
								RC6=FALSE;
							}
						}
						else
							RC6=FALSE;
						IDT_NeuronLocalCustomer->UnLockEl(&Anchor,tEl4);
					}
				}
				IDT_NeuronLocalCustomer->UnLockEl(&Anchor,tEl3);
				tEl3=IDT_NeuronLocalCustomer->FindNextET(&Anchor,&FStrGr,AcsMd_Read,tEl2);
			}
			IDT_NeuronLocalCustomer->UnLockEl(&Anchor,tEl2);
		}
		_mmm2:
		IDT_NeuronLocalCustomer->UnLockEl(&Anchor,tElU);
	}
	return RC1&RC2&RC3&RC4&RC5&RC6;
};


bool	Services::AddNewUser(char *Name,char *Password)
{
	bool RC=FALSE;
	El_Tab	*tEl;
	if (!TestLoginName(Name))
	{
		tEl=IDT_NeuronLocalCustomer->AttainNested(&Anchor,"%s%s%s",N_APCS,N_Users,Name,-1);
		if (tEl)
		{
			RC=TRUE;
			IDT_NeuronLocalCustomer->AddParam(&Anchor,tEl,IDUserPassword,(void*)(Password),1+strlen(Password));
			IDT_NeuronLocalCustomer->UnLockEl(&Anchor,tEl);
		}
	}
	return RC;
};

INT64	Services::TestLoginPassword(char *Name,char *Password)	// Return UserID
{
	INT64	User_ID=-1;
	El_Tab	*tEl;
	char*		pVol;
	int			tCnt;
	char	tBuf[128];
	IDTable*	ActTable;
	tEl=IDT_NeuronLocalCustomer->FindNested(&Anchor,AcsMd_Read,N_APCS,N_Users,Name,NULL);
	ActTable=IDT_NeuronLocalCustomer;
	if (tEl)
	{
// 		strcpy(tBuf,"SigmaTech-101271");
// 		ActTable->AddParam(&Anchor,tEl,IDUserPassword,(void*)(tBuf),1+strlen(tBuf));

		if ((tCnt=ActTable->GetParam(&Anchor,tEl,IDUserPassword,pVol))!=-1)
		{
			if (strcmp(Password,pVol)==0)
			{
				User_ID=tEl->GetIDI()->ID_File;
				strcpy(UserLogin,Name);
			}
		}

	};
	IDT_NeuronGlobal->UnLockEl(&Anchor,tEl);
	return	User_ID;
}

void	Services::GUIActivities()
{
	CntGUIActivities++;
};

void	Services::Logoff()
{
	UserID=-1;
	strcpy(UserLogin,"");
};

	Services::~Services()
{
	PackNetObj	PNetObj;
	if(SInt->MyLANNum==SInt->NumLANSrv)
		RoleSwOff("Server");
	System_On=0;
	if(NetThread)
	{
	delete NetThread; NetThread=NULL;
	}
	if(Sys_Thread)
	{
	delete Sys_Thread; Sys_Thread=NULL;
	}
	if (Cli1)
	{
		CloseSock(Cli1);Cli1=0;
	};
	if (Cli2)
	{
		CloseSock(Cli2);Cli2=0;
	};
	if (WANSISrv)
	{
		CloseSock(WANSISrv);WANSISrv=0;
	}
	delete	IDICnv;IDICnv=NULL;
	if (FPurger)
	{
		FPurger->StopEx();
		DeleteDecoder(FPurger);FPurger=NULL;
	}
// 	if (UnitView)
// 	{
// 		UnitView->StopEx();
// 		DeleteDecoder(UnitView);UnitView=NULL;
// 	}
	if (Net_Flag==1)
	{
//		NetMenLAN->Stop();
	}
	if (FSrv) 
	{
//		FSrv->Stop();
	}
// 	if (BDBIDInfo)
// 	{
// 		BDBIDInfo->Stop();
// 	}
	int	i;
//	PrmDec	PDec;
	El_Tab*	ETbl=NULL;
	El_Tab*	ElTbl3=NULL;
	El_Tab*	ElTbl4=NULL;
	El_Tab*	ElTbl5=NULL;
	char	tB[128];
	Srv->CloseUnit(this);
	Fl_Stop=1;
	CloseThIDView();
	sprintf(tB,"UID=%I64x",Cfg.UID);
	OutLog(tB,this,"UID");
	if (IDT_NeuronLocalSite)
	{
		ElTbl3=IDT_NeuronLocalSite->FindNested(&Anchor,AcsMd_Read,N_SystemSetting,N_MonitoringSystem,N_CriticalError,NULL);
		if(ElTbl3==NULL) ElTbl3=IDT_NeuronLocalSite->AttainNested(&Anchor,"%s%s",N_SystemSetting,N_MonitoringSystem,N_CriticalError,-1);
		
//		ETbl=IDT_NeuronLocalSite->AddET(&Anchor,N_Device,IDT_NeuronLocalSite->GetRoot());
		//	ETbl=IDT_Neuron->AttainNested(&Anchor,"%s",N_Device,-1);
//		IDT_NeuronLocalSite->UnLockEl(&Anchor,ETbl);
		El_CliInt*	Cli=(El_CliInt*)SInt->CliLAN.Find((UCHAR*)&SInt->INetAdrLAN,sizeof(ULONG));
		if (Cli)
		if (Cli->Cli.DomainList)
		{
			FindStrList8	FSL8;
			ID_Elem*	t_El=Cli->Cli.DomainList->Begin(FSL8);
			while (t_El!=NULL)
			{
				ETbl=IDT_NeuronLocalSite->AttainNested(&Anchor,"%s%s",N_Domain,t_El->Buff,N_Device,-1);
				IDT_NeuronLocalSite->UnLockEl(&Anchor,ETbl);
				ETbl=IDT_NeuronLocalSite->FindNested(&Anchor,AcsMd_Read,N_Domain,t_El->Buff,N_Host,ShortHostName,NULL);
				if (ETbl==NULL)
					ETbl=IDT_NeuronLocalSite->AttainNested(&Anchor,"%s%s%s",N_Domain,t_El->Buff,N_Host,ShortHostName,-1);
				if (ETbl)
				{
					
					ElTbl4=IDT_NeuronLocalSite->FindNestedStr(&Anchor,AcsMd_Read,N_Role,ETbl);
					if (ElTbl4==NULL)
					{
						ElTbl5=IDT_NeuronLocalSite->AddET(&Anchor,N_Role,ETbl);
						IDT_NeuronLocalSite->UnLockEl(&Anchor,ElTbl5);
					}
					IDT_NeuronLocalSite->UnLockEl(&Anchor,ElTbl4);
					
					INT64*	PVol;
					IDT_NeuronLocalSite->AddParam(&Anchor,ETbl,IDState,VolumeProcessed[IDVolProc_Off],1);
					IDT_NeuronLocalSite->DelParam(&Anchor,ETbl,IDPersonalID,2);
					if (IDT_NeuronLocalSite->GetParam(&Anchor,ETbl,IDTopUID,PVol)==-1)
					{
						IDT_NeuronLocalSite->AddParam(&Anchor,ETbl,IDTopUID,Cfg.PID);
					}
					else
					{
						if (Cfg.PID<*PVol)
						{
							Cfg.PID=*PVol+100;
							Cfg.UID=*PVol;
							SaveCfg();
						}
						IDT_NeuronLocalSite->AddParam(&Anchor,ETbl,IDTopUID,Cfg.PID);
					}
				}
				IDT_NeuronLocalSite->UnLockEl(&Anchor,ElTbl3);
				if (ETbl)
				{
					ElTbl3=IDT_NeuronLocalSite->Find(&Anchor,AcsMd_Read,(INT64)MyNum,ETbl);
				}
				IDT_NeuronLocalSite->UnLockEl(&Anchor,ETbl);
				//	ElTbl=
				//	ElTbl=IDT_Neuron->AttainNested(&Anchor,"%s%s%d","SystemSetting","Neuron","NetCli",(INT64)MyNum,-1);
				//	ElTbl=IDI_IS->AttainNestedMember("%s%s%d","SystemSetting","Neuron","NetCli",(INT64)MyNum,-1);
				if (ElTbl3)
				{
					INT64*	PVol;
					//		IDT_Neuron->MoLockEl(&Anchor,ElTbl3,AcsMd_Write);
					//		IDT_Neuron->AddParam(&Anchor,ElTbl3,IDHostName,HostName);
					if (IDT_NeuronLocalSite->GetParam(&Anchor,ElTbl3,IDTopUID,PVol)==-1)
					{
						IDT_NeuronLocalSite->AddParam(&Anchor,ElTbl3,IDTopUID,Cfg.PID);
					}
					else
					{
						if (Cfg.PID<*PVol)
						{
							Cfg.PID=*PVol;
						}
						else
						{
							IDT_NeuronLocalSite->AddParam(&Anchor,ElTbl3,IDTopUID,Cfg.PID);
						}
					};
					IDT_NeuronLocalSite->ModifElTab(&Anchor,ElTbl3);
				}
				IDT_NeuronLocalSite->UnLockEl(&Anchor,ElTbl3);
				
				t_El=Cli->Cli.DomainList->Next(FSL8);
			}
		}
	}
	Sleep(200);
//	if (NetComm)
//	{
//		NetComm->Stop();
//	}
	if (TS_CE)
	{
		DelCriticalError(TS_CE);TS_CE=0;
	}
	if (CE1)
	{
		DelCriticalError(CE1);CE1=0;
	}
	FSL85.DestroyStr();
	delete	SInt;SInt=0;
	for (i=0;i<NumSICli;i++)
	{
		if (CliList[i].Fl!=0)
		{
//			MyLock(__FILE__,__LINE__,&Fl_RE1,this);
			char	tBuf[80];
			switch(CliList[i].Mode)
			{
			case 0:
				sprintf(tBuf,"CliSI%03d",i);
				break;
			case 1:
				sprintf(tBuf,"CliSo%03d",i);
				break;
			case 2:
				sprintf(tBuf,"_SrvS%03d",i);
				break;
			}
			strcpy(PNetObj.descriptor,tBuf);
			PNetObj.command=NetSockDel;
			NS_DataFF(0,(UCHAR*)&PNetObj,sizeof(PNetObj));
			CliList[i].Fl=0;CliList[i].PObj=NULL;
//			MyUnLock(__FILE__,__LINE__,&Fl_RE1,this);
		}
	}
	
	if (NetCommLAN)
	{
		DeleteDecoder(NetCommLAN);NetCommLAN=NULL;
	}
// 	if (NetCommWAN)
// 	{
// 		DeleteDecoder(NetCommWAN);NetCommWAN=NULL;
// 	}
//	delete	IDI_IS;
	if (Net_Flag==1)
	{
//		DeleteDecoder(NetMen);
		Net_Flag=0;
	}
/*	if (SysInt!=NULL)
	{
		SysInt->Stop();
		DeleteDecoder(SysInt);
		SysInt=NULL;
	}
*/
//	if (Cfg.PID<Cfg.UID+1)
	Cfg.PID=Cfg.UID+1;
	Cfg.OldFcpu=(INT64)Fcpu;
	SaveCfg();
	free(SysPackMem);
	if (ModuleVocAll!=NULL)
	{
		FreeLibrary(ModuleVocAll);
		ModuleVocAll=NULL;
	}

	if (IDT_NeuronLocalSite) CloseTable(this,IDT_NeuronLocalSite); IDT_NeuronLocalSite=NULL;
	if (IDT_NeuronLocalCustomer) CloseTable(this,IDT_NeuronLocalCustomer); IDT_NeuronLocalCustomer=NULL;
	if (IDT_NeuronGlobal) CloseTable(this,IDT_NeuronGlobal); IDT_NeuronGlobal=NULL;
//	if (IDT_Prm) CloseTable(this,IDT_Prm); IDT_Prm=NULL;
	if (TableList) delete TableList;TableList=NULL;

	if (FSrv) 
	{
		CloseUnit(FSrv);
		DeleteDecoder(FSrv);FSrv=NULL;
	}
// 	if (BDBIDInfo)
// 	{
// 		CloseUnit(BDBIDInfo);
// 		DeleteDecoder(BDBIDInfo);BDBIDInfo=NULL;
// 	}


	delete	UM;UM=0;

	delete	TraceLst;TraceLst=0;
	delete	EventFn;EventFn=0;
//	delete	Chron;
	delete	DbgBList;DbgBList=0;
	delete	BL_Telemetr;BL_Telemetr=0;
	delete	BL_Log;BL_Log=0;
	delete	IDL_Log;IDL_Log=0;
	delete	Log;Log=0;
	delete	DLLL;DLLL=0;
	delete	DmpCE;DmpCE=0;
	delete	DmpDCE;DmpDCE=0;
/*
	if (handle!=HASP_INVALID_HANDLE_VALUE)
	{
		GetRealTime(&LRStat.LastRunTime.FTime);
//		LRStat.TotallRunTime.Time+=(LRStat.LastRunTime.Time-LRStat.StartRunTime.Time);
		status = hasp_encrypt(handle,&LRStat,sizeof(LRStat));
		status = hasp_write(handle,HASP_FILEID_RW,LCond.OffsetStatistic,sizeof(LRStat),&LRStat);	// Stat
		status = hasp_logout(handle);
		if (status != HASP_STATUS_OK)
		{
			switch (status)
			{
				case HASP_INV_HND:
					break;
				default:
					break;
			}
		}
	}
*/
	delete	IDInfoCLst;
	if (ActIDIList->NumElem>0)
	{
		FindStrList8	FSL8;
		ID_ElemIDI*	TmpIDE=(ID_ElemIDI*)ActIDIList->Begin(FSL8);
		ActIDIList->DelAllElem();
	};
	if (DelIDIList->NumElem>0)
	{
		DelIDIList->DelAllElem();
	};
	FSL_8.DestroyStr();
//	CloseHandle(hmtx);
	delete	ActIDIList;
	delete	DelIDIList;
	delete	DllFileQwe;
	delete	MainFileQwe;
	CloseHandle(hmtxlog);
	CloseHandle(PortD);
//	CloseHandle(hmtxMainThr);
	ChildIdleFunc(-1);
//----NetServices-----------------------------
	CFSL8.DestroyStr();
//	IFSL8.DestroyStr();
	MonClientList->RemoveAllPoiElem();
	delete ListUDPCli; ListUDPCli=NULL;
	delete ClientList; ClientList=NULL;
	delete MonClientList; MonClientList=NULL;
	PVOID	temPtr=this;
	ID_Elem*	ElPTR=ActiveDecoderList->Find((UCHAR*)&temPtr,sizeof(LPVOID));
	if (ElPTR)
	{
		ActiveDecoderList->Del(ElPTR);
	}
	if (ActiveDecoderList->NumElem!=0)
	{
		{size_t iasm=1;}
	};
	delete ActiveDecoderList; ActiveDecoderList=NULL; 
//--------------------------------------------
	if (DBGC.Cnt_El_Tab!=0)
	{
		{size_t iasm=1;}
	};
	if (DBGC.Cnt_IDI_Info!=0)
	{
		{size_t iasm=1;}
	};
	if (DBGC.Cnt_IDList8!=0)
	{
		{size_t iasm=1;}
	};
	if (DBGC.Cnt_pList!=0)
	{
		{size_t iasm=1;}
	};
	for (i=0;i<SizeMC;i++)
	{
		if (AllocMC[i].Fl)
		{
			free(AllocMC[i].Buf);
		};
		if (FreeMC[i].Fl)
		{
			free(FreeMC[i].Buf);
		};
	}
	if (SData->Fl_StateApp!=3)
		SData->Fl_StateSmartSt=3;
	SData->Fl_StateApp=0;
	UnmapViewOfFile(SData); 
//	if (NetMenBuf)
//		free(NetMenBuf);
	if (AddList)
	{
		AddList->DelAllElem();
		delete AddList;
	}
};

	ElUDPCli::ElUDPCli(int tNetType,int tPort)
{
	int rc;
	NetType=tNetType;
	Port=tPort;
	Srv=GetServices();
	SizeSource=sizeof(SourceUDP);
	Init(NetType,Port);
	sprintf(StrPort,"%d",Port);
	if(NetType==NT_PAN)
	{
		set_address( inet_ntoa(Srv->IPAddrPAN), StrPort, &peerUDP, "udp" );
		set_address( CreateBroadcastPANAddress(host), StrPort, &peerUDPOut, "udp" );	//	sc = udp_client( CreateBroadcastLANAddress(host),PortL, &peerLAN );
		peerUDPIn = *(struct sockaddr*)&peerUDPOut;
	}
	else
	{
		set_address( inet_ntoa(Srv->IPAddrLAN), StrPort, &peerUDP, "udp" );
		set_address( CreateBroadcastLANAddress(host), StrPort, &peerUDPOut, "udp" );	//	sc = udp_client( CreateBroadcastLANAddress(host),PortL, &peerLAN );
		peerUDPIn = *(struct sockaddr*)&peerUDPOut;
	}

	ss = socket( AF_INET, SOCK_DGRAM, 0 );
	if ( !isvalidsock( ss ) )
	{};
	if ( rc=bind( ss, ( struct sockaddr * ) &peerUDP,
		sizeof( peerUDP ) ) )
	{};
	dontblock=0;
	rc=ioctlsocket( ss, FIONBIO, (PULONG)&dontblock);
	dontblock=5000;
	rc=setsockopt(ss,SOL_SOCKET,SO_RCVTIMEO,(char*)&dontblock,sizeof(int));
	dontblock=32536;
	rc=ioctlsocket( ss, FIONREAD, (PULONG)&dontblock);
	dontblock=32*0x2003 /*SO_MAX_MSG_SIZE*/;
	rc=setsockopt(ss,SOL_SOCKET,SO_RCVBUF,(char*)&dontblock,sizeof(int));
// 	BOOL sFl=TRUE;
// 	rc=setsockopt(ss,SOL_SOCKET,SO_BROADCAST,(char*)&sFl,sizeof(BOOL));
	dontblock=1;
	if (setsockopt(ss, IPPROTO_UDP, 1 /*UDP_CHECKSUM_COVERAGE=20*/, (char *)&dontblock, sizeof(dontblock)) == SOCKET_ERROR)
	{rc=WSAGetLastError();	}
	sprintf(UDPFunName,"UDPFun%d",Port);
	ElThr=new ClassThread(UDPFunName,UDPFun,(LPVOID)this,THREAD_PRIORITY_HIGHEST);
// 	sprintf(UDPFunName,"UDPFunExt%d",Port);
// 	ElThrExt=new ClassThread(UDPFunName,UDPFunExt,(LPVOID)this,THREAD_PRIORITY_ABOVE_NORMAL);
};

bool ElUDPCli::AddUser(INT64 UniqUserID)
{
	bool rc=FALSE;
	ID_Elem64* ElU=(ID_Elem64*)UserList.Find((UCHAR*)&UniqUserID,sizeof(INT64));
	if (ElU==NULL)
	{
		ElU = new	ID_Elem64(UniqUserID); 
		if (UserList.Add(ElU)==-1)
		{
			delete ElU;
		}
		else
		{
			rc=TRUE;
		}
	}
	return rc;
};

bool ElUDPCli::DelUser(INT64 UniqUserID)
{
	bool rc=FALSE;
	ID_Elem64* ElU=(ID_Elem64*)UserList.Find((UCHAR*)&UniqUserID,sizeof(INT64));
	if (ElU)
	{
		UserList.Del(ElU);
		rc=TRUE;
	}
	return rc;
};

int	ElUDPCli::Send(UCHAR* Buf,int Cnt)
{
	int rc;
	rc = sendto( ss, (char*)Buf,Cnt, 0,( struct sockaddr * )&peerUDPOut, sizeof( struct sockaddr ) );
	return rc;
};

int	ElUDPCli::SendTo(UCHAR* Buf,int Cnt,struct sockaddr * Peer)
{
	int rc;
	rc = sendto( ss, (char*)Buf,Cnt, 0,Peer, sizeof( struct sockaddr ) );
	return rc;
};
/*
int		ElUDPCli::Recive()
{
	int cnt=0;
	cnt = recvfrom( ss, InBuf, sizeof( InBuf ), 0, &SourceUDP, &SizeSource);
	return cnt;
};
*/
void	ElUDPCli::Idle()
{
	int cnt=0;
	cnt = recvfrom( ss, InBuf, sizeof( InBuf ), 0, &SourceUDP, &SizeSource);
	if(cnt>0)
	{
		if(cnt>6)
			cnt=cnt*2-cnt;
		if (*(long* )&SourceUDP.sa_data[2]!=*(long*)(&Srv->IPAddrLAN))
		{
			FindStrList8	FSL;
			ID_Elem64* ElU=(ID_Elem64*)UserList.Begin(FSL);
			while (ElU)
			{
				((Decoder*)ElU->Vol)->UserCallBackEx(HandleUDP,Port,(UCHAR*)&SourceUDP,cnt+sizeof(SourceUDP),3);
				ElU=(ID_Elem64*)UserList.Next(FSL);
			}
		}
	}
	else
	{
		cnt=WSAGetLastError();
	}
// 	if (cnt>0)
// 	{
// 		Dmp.Data((UCHAR*)&SourceUDP,cnt+sizeof(SourceUDP));
// 	}
};

/*
void	ElUDPCli::IdleExt()
{
	uchar*		tBuf;
	size_t		tCnt=0;

	while(Dmp.LockOData(tBuf,tCnt))
	{
		FindStrList8	FSL;
		ID_Elem64* ElU=(ID_Elem64*)UserList.Begin(FSL);
		while(ElU)
		{
			((Decoder*)ElU->Vol)->UserCallBackEx(HandleUDP,Port,tBuf+sizeof(SockHeader),tCnt-sizeof(SockHeader));
			ElU=(ID_Elem64*)UserList.Next(FSL);
		}
		Dmp.UnLockOData();
	}
	Dmp.UnLockOData();
// 	Dmp.LockOData((UCHAR*)&SourceUDP,cnt+sizeof(SourceUDP));
//	DmpProc(pObj,tBuf,tCnt);
// 	if (cnt>0)
// 	{
// 		FindStrList8	FSL;
// 		ID_Elem64* ElU=(ID_Elem64*)UserList.Begin(FSL);
// 		while (ElU)
// 		{
// 			((Decoder*)ElU->Vol)->UserCallBackEx(HandleUDP,Port,(UCHAR*)&SourceUDP,cnt+sizeof(SourceUDP));
// 			ElU=(ID_Elem64*)UserList.Next(FSL);
// 		}
// 	}
};
*/


	ElUDPCli::~ElUDPCli()
{
	InBuf[0]=SICom_DisConnect;
	ElThr->On = 0;
	SendTo((UCHAR*)InBuf, sizeof(char), &peerUDPIn);
	ElThr->Stop();
//	delete	ElThrExt;
	delete	ElThr;
	UserList.DelAllElem();
	closesocket(ss);
};

void	TlmData::Set(Decoder* PO,char* P,PVOID POO) 
{
	PObj=PO;pObjO=POO;strcpy(Prm,P);
	strcpy(NameMt,PO->Name_Mt);
	ExtMt=PO->Ext_Mt;
}

void	El_Telem::Init(TlmData *NPrm)
{
	memcpy(&TD,NPrm,sizeof(TlmData));
	sVol=0;
	Len=sizeof(Decoder*)+strlen(NPrm->Prm)+1;
	Srv->GetRealTime(&LWTime.FTime);
//	GetSystemTimeAsFileTime(&LWTime.FTime);
};

void	El_Telem::SetVol(double Vol)
{
	mVol=Vol;
	sVol=sVol*0.99+mVol*0.01;
	dVol=dVol*0.99+sqrt((sVol-mVol)*(sVol-mVol))*0.01;
	if (mVol>maxVol) maxVol=mVol;
	if (mVol<minVol) minVol=mVol;
	if (sVol>maxVol) sVol=maxVol;
	if (sVol<minVol) sVol=minVol;
	Srv->GetRealTime(&LWTime.FTime);
//	GetSystemTimeAsFileTime(&LWTime.FTime);
	if (Srv->Timer(tTCnt,Srv->TimeInt))	// 60 s
	{
		char	OBuf[256];
		sprintf(OBuf,"0x%08x 0x%08x %s %1.15e %1.15e",(int)TD.PObj,(int)TD.pObjO,TD.Prm,mVol,sVol);
		Srv->OutLog(OBuf,NULL,"Telemetr");	
	};
};

/*
ParamList::ParamList()
{
	FlNew=1;LWrTime=0;//	time(NULL);
};

void*	ParamList::AllocParam(char* IDName,int Cnt,Decoder* ParentMt)
{
	void*		Poi=NULL;
	PL_Elem*	Elem;
	int			rc=0;
	if (IDName[0]==0) return Poi;
	LWrTime++;	//	=time(NULL);
	Elem=(PL_Elem*)pList.Find((UCHAR*)IDName,strlen(IDName)+1);
	if (Elem==NULL)
	{
		Elem=(PL_Elem*)pList.NewElem();
		if (Elem==NULL)
			Elem=new PL_Elem(IDName,Cnt,ParentMt);
		else
			Elem->Init(IDName,Cnt,ParentMt);
		rc=pList.Add(Elem);
		if (rc==-1)
			rc=-1;
	}
	else
		Elem->ReAlloc(Cnt,ParentMt);
	if (Elem!=NULL)
	{
		Poi=Elem->pVol;
	};
	FlNew=1;
	return Poi;
};

int		ParamList::SetParam(char* IDName,void* Buf,int Cnt,Decoder* ParentMt)
{
	int	rc=-1;
	if (IDName[0]==0) return rc;
	PL_Elem*	Elem;
	LWrTime++;//	=time(NULL);
	Elem=(PL_Elem*)pList.Find((UCHAR*)IDName,strlen(IDName)+1);
	if (Elem==NULL)
	{
		Elem=(PL_Elem*)pList.NewElem();
		if (Elem==NULL)
			Elem=new PL_Elem(IDName,Cnt,ParentMt);
		else
			Elem->Init(IDName,Cnt,ParentMt);
		rc=pList.Add(Elem);
		if (rc==-1)
			rc=-1;
	}
	if(Elem!=NULL)
	{
		Elem->SetElem(Buf,Cnt,ParentMt);
		rc=0;
	}
	FlNew=1;
	return rc;
};

int		ParamList::GetParam(char* IDName,void* &Buf,size_t &Cnt,Decoder* &ParentMt)
{
	int	rc=-1;
	PL_Elem	*Elem;
	ParentMt=NULL;
	Elem=(PL_Elem*)pList.Find((UCHAR*)IDName,strlen(IDName)+1);
	if (Elem!=NULL) 
	{
		Buf=Elem->pVol;
		Cnt=Elem->Size;
		ParentMt=Elem->ParentMt;
		rc=0;
	}
	return rc;
};

int		ParamList::GetParam(char* IDName,void* &Buf,Decoder* &ParentMt)
{
	int	rc=-1;
	PL_Elem	*Elem;
	ParentMt=NULL;
	Elem=(PL_Elem*)pList.Find((UCHAR*)IDName,strlen(IDName)+1);
	if (Elem!=NULL) 
	{
		Buf=Elem->pVol;
		ParentMt=Elem->ParentMt;
		rc=0;
	}
	return rc;
};

int		ParamList::GetParam(char* IDName,void* &Buf,size_t &Cnt)
{
	int	rc=-1;
	PL_Elem	*Elem;
	Elem=(PL_Elem*)pList.Find((UCHAR*)IDName,strlen(IDName)+1);
	if (Elem!=NULL) 
	{
		Buf=Elem->pVol;
		Cnt=Elem->Size;
		rc=0;
	}
	return rc;
};

int		ParamList::GetParam(char* IDName,void* &Buf)
{
	int	rc=-1;
	PL_Elem	*Elem;
	Elem=(PL_Elem*)pList.Find((UCHAR*)IDName,strlen(IDName)+1);
	if (Elem!=NULL) 
	{
		Buf=Elem->pVol;
		rc=0;
	}
	return rc;
};

int		ParamList::DelParam(char* IDName)
{
	int	rc=-1;
	if (IDName[0]==0) return rc;
	PL_Elem	*Elem;
	LWrTime++;//	=time(NULL);
	Elem=(PL_Elem*)pList.Find((UCHAR*)IDName,strlen(IDName)+1);
	if (Elem!=NULL) 
	{
		pList.Del(Elem);
		rc=0;
	}
	return rc;
};

// int		ParamList::FindStart()	// rc=-1 íåóäà÷à
// {
// 	int	rc=0;
// 	pList.BeforeBegin();
// 	return rc;
// };

int		ParamList::FindFirst(char* &IDName,void* &Vol,size_t &Cnt,Decoder* &ParentMt)	// rc=-1 íåóäà÷à
{
	int	rc=-1;
	PL_Elem*	Elem;
	ParentMt=NULL;
	Elem=(PL_Elem*)pList.Begin();
	if (Elem!=0)
	{
		IDName=(char*)Elem->Buff;Vol=Elem->pVol;Cnt=Elem->Size;
		ParentMt=Elem->ParentMt;
		rc=0;
	}
	return rc;
};
int		ParamList::FindNext(char* &IDName,void* &Vol,size_t &Cnt,Decoder* &ParentMt)	// rc=-1 íåóäà÷à
{
	int	rc=-1;
	PL_Elem*	Elem;
	ParentMt=NULL;
	Elem=(PL_Elem*)pList.Next();
	if (Elem!=0)
	{
		IDName=(char*)Elem->Buff;Vol=Elem->pVol;Cnt=Elem->Size;
		ParentMt=Elem->ParentMt;
		rc=0;
	}
	return rc;
};

PL_Elem*		ParamList::Find(UCHAR* IDName)
{
	PL_Elem*	Elem=NULL;
	if (IDName[0]!=0)
		Elem=(PL_Elem*)pList.Find(IDName,strlen((char*)IDName)+1,1);
	return Elem;
};	// Óñòàíàâëèâàåò óêàçàòåëü ïîèñêà íà ïîçèöèþ ñîîòâåòñòâóþùóþ óñëîâèþ ïîèñêà

int		ParamList::FindFirst(char* &IDName,void* &Vol,size_t &Cnt)	// rc=-1 íåóäà÷à
{
	int	rc=-1;
	PL_Elem*	Elem;
	Elem=(PL_Elem*)pList.Begin();
	if (Elem!=0)
	{
		IDName=(char*)Elem->Buff;Vol=Elem->pVol;Cnt=Elem->Size;
		rc=0;
	}
	return rc;
};

int		ParamList::FindNext(char* &IDName,void* &Vol,size_t &Cnt)	// rc=-1 íåóäà÷à
{
	int	rc=-1;
	PL_Elem*	Elem;
	Elem=(PL_Elem*)pList.Next();
	if (Elem!=0)
	{
		IDName=(char*)Elem->Buff;Vol=Elem->pVol;Cnt=Elem->Size;
		rc=0;
	}
	return rc;
};

int		ParamList::FindFirst(char* &IDName,void* &Vol,size_t &Cnt,long &mLWrTime)	// rc=-1 íåóäà÷à
{
	int	rc=-1;
	PL_Elem*	Elem;
	Elem=(PL_Elem*)pList.Begin();
	if (Elem!=0)
	{
		IDName=(char*)Elem->Buff;Vol=Elem->pVol;Cnt=Elem->Size;mLWrTime=Elem->LWrTime;
		rc=0;
	}
	return rc;
};

int		ParamList::FindNext(char* &IDName,void* &Vol,size_t &Cnt,long &mLWrTime)	// rc=-1 íåóäà÷à
{
	int	rc=-1;
	PL_Elem*	Elem;
	Elem=(PL_Elem*)pList.Next();
	if (Elem!=0)
	{
		IDName=(char*)Elem->Buff;Vol=Elem->pVol;Cnt=Elem->Size;mLWrTime=Elem->LWrTime;
		rc=0;
	}
	return rc;
};


ParamList::~ParamList()
{
};
*/

// -------------------------------------------------------------------------------------------------------
/*
#if	TypeOfParamListID != New	

ParamListID::ParamListID()
{
	DBGC.Cnt_pList++;
	FlNew=1;LWrTime=0;//	time(NULL);
};

void*	ParamListID::AllocParam(INT64 ID,int Cnt,Decoder* ParentMt)
{
	void*		Poi=NULL;
	PLID_Elem*	Elem;
	int			rc=0;
	if (ID==0) return Poi;
	MyLock(__FILE__,__LINE__,&Fl_RE,ParentMt);
	LWrTime++;	//	=time(NULL);
	Elem=(PLID_Elem*)pList.Find((UCHAR*)&ID,sizeof(INT64));
	if (Elem==NULL)
	{
		Elem=(PLID_Elem*)pList.NewElem();
		if (Elem==NULL)
			Elem=new PLID_Elem(ID,Cnt,ParentMt);
		else
			Elem->Init(ID,Cnt,ParentMt);
		rc=pList.Add(Elem);
		if (rc==-1)
			rc=-1;
	}
	else
		Elem->ReAlloc(Cnt,ParentMt);
	if (Elem!=NULL)
	{
		Poi=Elem->pVol;
	};
	FlNew=1;
	MyUnLock(__FILE__,__LINE__,&Fl_RE,ParentMt);
	return Poi;
};

int		ParamListID::SetParam(INT64 ID,void* Buf,int Cnt,Decoder* ParentMt)
{
	int	rc=-1;
	if (ID==0) return rc;
	PLID_Elem*	Elem;
	MyLock(__FILE__,__LINE__,&Fl_RE,NULL);
	LWrTime++;//	=time(NULL);
	Elem=(PLID_Elem*)pList.Find((UCHAR*)&ID,sizeof(INT64));
	if (Elem==NULL)
	{
		Elem=(PLID_Elem*)pList.NewElem();
		if (Elem==NULL)
			Elem=new PLID_Elem(ID,Cnt,ParentMt);
		else
			Elem->Init(ID,Cnt,ParentMt);
		rc=pList.Add(Elem);
		if (rc==-1)
			rc=-1;
	}
	if(Elem!=NULL)
	{
		Elem->SetElem(Buf,Cnt,ParentMt);
		rc=0;
	}
	FlNew=1;
	MyUnLock(__FILE__,__LINE__,&Fl_RE,NULL);
	return rc;
};

int		ParamListID::GetParam(INT64 ID,void* &Buf,size_t &Cnt,Decoder* &ParentMt)
{
	int	rc=-1;
	PLID_Elem	*Elem;
	ParentMt=NULL;
	MyLock(__FILE__,__LINE__,&Fl_RE,NULL);
	Elem=(PLID_Elem*)pList.Find((UCHAR*)&ID,sizeof(INT64));
	if (Elem!=NULL) 
	{
		Buf=Elem->pVol;
		Cnt=Elem->Size;
		ParentMt=Elem->ParentMt;
		rc=0;
	}
	MyUnLock(__FILE__,__LINE__,&Fl_RE,NULL);
	return rc;
};

int		ParamListID::GetParam(INT64 ID,void* &Buf,Decoder* &ParentMt)
{
	int	rc=-1;
	PLID_Elem	*Elem;
	ParentMt=NULL;
	MyLock(__FILE__,__LINE__,&Fl_RE,NULL);
	Elem=(PLID_Elem*)pList.Find((UCHAR*)&ID,sizeof(INT64));
	if (Elem!=NULL) 
	{
		Buf=Elem->pVol;
		ParentMt=Elem->ParentMt;
		rc=0;
	}
	MyUnLock(__FILE__,__LINE__,&Fl_RE,NULL);
	return rc;
};

int		ParamListID::GetParam(INT64 ID,void* &Buf,size_t &Cnt)
{
	int	rc=-1;
	PLID_Elem	*Elem;
	MyLock(__FILE__,__LINE__,&Fl_RE,NULL);
	Elem=(PLID_Elem*)pList.Find((UCHAR*)&ID,sizeof(INT64));
	if (Elem!=NULL) 
	{
		Buf=Elem->pVol;
		Cnt=Elem->Size;
		rc=0;
	}
	MyUnLock(__FILE__,__LINE__,&Fl_RE,NULL);
	return rc;
};

int		ParamListID::GetParam(INT64 ID,void* &Buf)
{
	int	rc=-1;
	PLID_Elem	*Elem;
	MyLock(__FILE__,__LINE__,&Fl_RE,NULL);
	Elem=(PLID_Elem*)pList.Find((UCHAR*)&ID,sizeof(INT64),1);
	if (Elem!=NULL) 
	{
		Buf=Elem->pVol;
		rc=0;
	}
	MyUnLock(__FILE__,__LINE__,&Fl_RE,NULL);
	return rc;
};

int		ParamListID::DelParam(INT64 ID)
{
	int	rc=-1;
	if (ID==0) return rc;
	PLID_Elem	*Elem;
	MyLock(__FILE__,__LINE__,&Fl_RE,NULL);
	LWrTime++;//	=time(NULL);
	Elem=(PLID_Elem*)pList.Find((UCHAR*)&ID,sizeof(INT64));
	if (Elem!=NULL) 
	{
		pList.Del(Elem);
		rc=0;
	}
	MyUnLock(__FILE__,__LINE__,&Fl_RE,NULL);
	return rc;
};

void	ParamListID::DelAllElem()
{
	int	i;
	pList.DelAllElem();
};


int		ParamListID::FindFirst(INT64 &ID,void* &Vol,size_t &Cnt,Decoder* &ParentMt)	// rc=-1 íåóäà÷à
{
	int	rc=-1;
	PLID_Elem*	Elem;
	MyLock(__FILE__,__LINE__,&Fl_RE,NULL);
	ParentMt=NULL;
	Elem=(PLID_Elem*)pList.Begin();
	if (Elem!=0)
	{
		ID=*(INT64*)Elem->Buff;Vol=Elem->pVol;Cnt=Elem->Size;
		ParentMt=Elem->ParentMt;
		rc=0;
	}
	MyUnLock(__FILE__,__LINE__,&Fl_RE,NULL);
	return rc;
};
int		ParamListID::FindNext(INT64 &ID,void* &Vol,size_t &Cnt,Decoder* &ParentMt)	// rc=-1 íåóäà÷à
{
	int	rc=-1;
	PLID_Elem*	Elem;
	ParentMt=NULL;
	MyLock(__FILE__,__LINE__,&Fl_RE,NULL);
	Elem=(PLID_Elem*)pList.Next();
	if (Elem!=0)
	{
		ID=*(INT64*)Elem->Buff;Vol=Elem->pVol;Cnt=Elem->Size;
		ParentMt=Elem->ParentMt;
		rc=0;
	}
	MyUnLock(__FILE__,__LINE__,&Fl_RE,NULL);
	return rc;
};

void	ParamListID::TestValid()
{
};

PLID_Elem*		ParamListID::Find(INT64 ID)
{
	PLID_Elem*	Elem=NULL;
	MyLock(__FILE__,__LINE__,&Fl_RE,NULL);
	if (ID!=0)
		Elem=(PLID_Elem*)pList.Find((UCHAR*)&ID,sizeof(INT64),1);
	MyUnLock(__FILE__,__LINE__,&Fl_RE,NULL);
	return Elem;
};	// Óñòàíàâëèâàåò óêàçàòåëü ïîèñêà íà ïîçèöèþ ñîîòâåòñòâóþùóþ óñëîâèþ ïîèñêà

int		ParamListID::FindFirst(INT64 &ID,void* &Vol,size_t &Cnt)	// rc=-1 íåóäà÷à
{
	int	rc=-1;
	PLID_Elem*	Elem;
	MyLock(__FILE__,__LINE__,&Fl_RE,NULL);
	Elem=(PLID_Elem*)pList.Begin();
	if (Elem!=0)
	{
		ID=*(INT64*)Elem->Buff;Vol=Elem->pVol;Cnt=Elem->Size;
		rc=0;
	}
	MyUnLock(__FILE__,__LINE__,&Fl_RE,NULL);
	return rc;
};

int		ParamListID::FindNext(INT64 &ID,void* &Vol,size_t &Cnt)	// rc=-1 íåóäà÷à
{
	int	rc=-1;
	PLID_Elem*	Elem;
	MyLock(__FILE__,__LINE__,&Fl_RE,NULL);
	Elem=(PLID_Elem*)pList.Next();
	if (Elem!=0)
	{
		ID=*(INT64*)Elem->Buff;Vol=Elem->pVol;Cnt=Elem->Size;
		rc=0;
	}
	MyUnLock(__FILE__,__LINE__,&Fl_RE,NULL);
	return rc;
};

int		ParamListID::FindFirst(INT64 &ID,void* &Vol,size_t &Cnt,long &mLWrTime)	// rc=-1 íåóäà÷à
{
	int	rc=-1;Cnt=0;
	PLID_Elem*	Elem;
	MyLock(__FILE__,__LINE__,&Fl_RE,NULL);
	Elem=(PLID_Elem*)pList.Begin();
	if (Elem!=0)
	{
		ID=*(INT64*)Elem->Buff;Vol=Elem->pVol;Cnt=Elem->Size;mLWrTime=Elem->LWrTime;
		rc=0;
	}
	MyUnLock(__FILE__,__LINE__,&Fl_RE,NULL);
	return rc;
};

int		ParamListID::FindNext(INT64 &ID,void* &Vol,size_t &Cnt,long &mLWrTime)	// rc=-1 íåóäà÷à
{
	int	rc=-1;
	PLID_Elem*	Elem;
	MyLock(__FILE__,__LINE__,&Fl_RE,NULL);
	Elem=(PLID_Elem*)pList.Next();
	if (Elem!=0)
	{
		ID=*(INT64*)Elem->Buff;Vol=Elem->pVol;Cnt=Elem->Size;mLWrTime=Elem->LWrTime;
		rc=0;
	}
	MyUnLock(__FILE__,__LINE__,&Fl_RE,NULL);
	return rc;
};


ParamListID::~ParamListID()
{
	--DBGC.Cnt_pList;
};

#endif
*/
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// -------------------------------------------------------------------------------------------------------

#if	TypeOfParamListID == New

ParamListID::ParamListID(char*	Name)
{
	int	i;
//	DBGC.Cnt_pList++;
	strcpy(pName,Name);
#ifdef	_DEBUG
	DBGC.AddPList(pName);
#endif
	FlNew=1;LWrTime=0;//	time(NULL);
	Srv=GetServices();
	for (i=0;i<NumDPrm;i++)
	{
		tV[i].iV.VI64=0;
		Fl[i]=FALSE;
	}
	FindPoi=0;
	NumElem=0;
};

void*	ParamListID::AllocParam(INT64 ID,int Cnt,Decoder* ParentMt)
{
	void*		Poi=NULL;
//	PLID_Elem*	Elem;
	int			rc=0;
	int			index=ID-FirstPrm;
	Parametr*	Prm=GetTablParametrID(ID);
	if (ID==0) return Poi;
	if (ID>=FirstPrm)
	{
		if(ID>=LastPrm)
		{
			index=ID-LastPrm+EndFirstPrm-FirstPrm;
		}
		MyLock(__FILE__,__LINE__,&Fl_RE,ParentMt);
		LWrTime++;	//	=time(NULL);
		if (Fl[index]==FALSE)
		{
			Fl[index]=TRUE; NumElem++;
		};
		if ((Prm->Type>=PrmType_STRING)&&(Cnt>0))
		{
			if (tV[index].dV.tBf==NULL)
			{
				tV[index].dV.tBf=(char*)malloc(Cnt);
//				tV[index].dV.tBf=(char*)Srv->malloc(Cnt);
				tV[index].dV.tSize=Cnt;
			}
			else
			{
				char*	tB;
				if (tV[index].dV.tSize!=Cnt)
				{
					tB=(char*)realloc(tV[index].dV.tBf,Cnt);
//					tB=(char*)Srv->realloc(tV[index].dV.tBf,Cnt);
					if (tB!=NULL)
					{
						tV[index].dV.tBf=tB;
						tV[index].dV.tSize=Cnt;
					}
					else
					{
						free(tV[index].dV.tBf);
//						Srv->free(tV[index].dV.tBf);
						tV[index].dV.tBf=NULL;
						tV[index].dV.tSize=Cnt;
						Fl[index]=FALSE; NumElem--;
					}

				}
			}
			Poi=tV[index].dV.tBf;
		}
		Srv->GetRealTime(&tV[index].dV.TimeModification.FTime);
		FlNew=1;
		MyUnLock(__FILE__,__LINE__,&Fl_RE,ParentMt);
	}
	return Poi;
};

int		ParamListID::SetParam(INT64 ID,void* Buf,size_t Cnt,Decoder* ParentMt,FileTime* FTime)
{
	int	rc=-1;
	if (ID==0) return rc;
	int			index=ID-FirstPrm;

	Parametr*	Prm=GetTablParametrID(ID);
//	PLID_Elem*	Elem;
	if(ID>=LastPrm)
	{
		index=ID-LastPrm+EndFirstPrm-FirstPrm;
	};
	if (Prm==NULL)
		return rc;
	MyLock(__FILE__,__LINE__,&Fl_RE,NULL);
	LWrTime++;//	=time(NULL);
	if (Fl[index]==FALSE)
	{
		Fl[index]=TRUE; NumElem++;
	};
	if ((Prm->Type>=PrmType_STRING)&&(Cnt>0))
	{
		int	CntTry=5;
		do 
		{
			if (tV[index].dV.tBf==NULL)
			{
				//			tV[index].dV.tBf=(char*)Srv->malloc(Cnt);
				tV[index].dV.tBf=(char*)malloc(Cnt);
				tV[index].dV.tSize=Cnt;
			}
			else
			{
//				char*	tB;
				if (tV[index].dV.tSize!=Cnt)
				{
					free(tV[index].dV.tBf);tV[index].dV.tBf=0;
					tV[index].dV.tBf=(char*)malloc(Cnt);
					tV[index].dV.tSize=Cnt;
	//				tB=(char*)Srv->realloc(tV[index].dV.tBf,Cnt);
/*					tB=(char*)realloc(tV[index].dV.tBf,Cnt);
					if (tB!=NULL)
					{
						tV[index].dV.tBf=tB;
						tV[index].dV.tSize=Cnt;
					}
					else
					{
						//					Srv->free(tV[index].dV.tBf);
						free(tV[index].dV.tBf);
						tV[index].dV.tBf=NULL;
						tV[index].dV.tSize=Cnt;
						Fl[index]=FALSE; NumElem--;
					}
*/				}
			}
		} while ((--CntTry>0)&&(tV[index].dV.tBf==NULL));

		if (tV[index].dV.tBf)
		{
			memcpy(tV[index].dV.tBf,Buf,Cnt);
			rc=0;
		}
		else
		{
			Fl[index]=FALSE; NumElem--;
			rc=-1;
			tV[index].dV.tSize=0;
		}
	}
	else
	{
		if (Cnt>SizePrm[Prm->Type])
			Cnt=SizePrm[Prm->Type];
		memcpy((char*)&tV[index].iV.VI64,Buf,Cnt);
		rc=0;
	}
	if (FTime)
		tV[index].dV.TimeModification.Time=FTime->Time;
	else
		Srv->GetRealTime(&tV[index].dV.TimeModification.FTime);
// 	Elem=(PLID_Elem*)pList.Find((UCHAR*)&ID,sizeof(INT64));
// 	if (Elem==NULL)
// 	{
// 		Elem=(PLID_Elem*)pList.NewElem();
// 		if (Elem==NULL)
// 			Elem=new PLID_Elem(ID,Cnt,ParentMt);
// 		else
// 			Elem->Init(ID,Cnt,ParentMt);
// 		rc=pList.Add(Elem);
// 		if (rc==-1)
// 			rc=-1;
// 	}
// 	if(Elem!=NULL)
// 	{
// 		Elem->SetElem(Buf,Cnt,ParentMt);
// 		rc=0;
// 	}
	FlNew=1;
	MyUnLock(__FILE__,__LINE__,&Fl_RE,NULL);
	return rc;
};

int		ParamListID::GetParam(INT64 ID,void* &Buf,size_t &Cnt,Decoder* &ParentMt)
{
	int	rc=-1;
//	PLID_Elem	*Elem;
	ParentMt=NULL;
	int			index=ID-FirstPrm;
	if(ID>=LastPrm)
	{
		index=ID-LastPrm+EndFirstPrm-FirstPrm;
	}
	if (Fl[index])
	{
		Parametr*	Prm=GetTablParametrID(ID);
		MyLock(__FILE__,__LINE__,&Fl_RE,NULL);
		if (Prm->Type>=PrmType_STRING)
		{
			Buf=tV[index].dV.tBf;
			if ((Prm->Size==0)||(tV[index].dV.tSize<Prm->Size))
			{
				Cnt=tV[index].dV.tSize;
			}
			else
			{
				Cnt=Prm->Size;
				tV[index].dV.tBf[Cnt-1]=0;
			}
		}
		else
		{
			Buf=&tV[index].iV.VI64;
			Cnt=SizePrm[Prm->Type];
		}
//		ParentMt=Srv;
		rc=0;
// 	Elem=(PLID_Elem*)pList.Find((UCHAR*)&ID,sizeof(INT64));
// 	if (Elem!=NULL) 
// 	{
// 		Buf=Elem->pVol;
// 		Cnt=Elem->Size;
// 		ParentMt=Elem->ParentMt;
// 		rc=0;
// 	}
		MyUnLock(__FILE__,__LINE__,&Fl_RE,NULL);
	}
	if (Buf==NULL)
		rc=-1;
	return rc;
};

int		ParamListID::GetParam(INT64 ID,void* &Buf,Decoder* &ParentMt)
{
	int	rc=-1;
//	PLID_Elem	*Elem;
	ParentMt=NULL;
	int			index=ID-FirstPrm;
	if(ID>=LastPrm)
	{
		index=ID-LastPrm+EndFirstPrm-FirstPrm;
	}
	if (Fl[index])
	{
		Parametr*	Prm=GetTablParametrID(ID);
		MyLock(__FILE__,__LINE__,&Fl_RE,NULL);
		if (Prm->Type>=PrmType_STRING)
		{
			Buf=tV[index].dV.tBf;
		}
		else
		{
			Buf=&tV[index].iV.VI64;
		}
//		ParentMt=Srv;
		rc=0;
// 	Elem=(PLID_Elem*)pList.Find((UCHAR*)&ID,sizeof(INT64));
// 	if (Elem!=NULL) 
// 	{
// 		Buf=Elem->pVol;
// 		ParentMt=Elem->ParentMt;
// 		rc=0;
// 	}
		MyUnLock(__FILE__,__LINE__,&Fl_RE,NULL);
	}
	if (Buf==NULL)
		rc=-1;
	return rc;
};

int		ParamListID::GetParam(INT64 ID,void* &Buf,size_t &Cnt)
{
	int	rc=-1;
//	PLID_Elem	*Elem;
	int			index=ID-FirstPrm;
	if(ID>=LastPrm)
	{
		index=ID-LastPrm+EndFirstPrm-FirstPrm;
	}
	if (Fl[index])
	{
		Parametr*	Prm=GetTablParametrID(ID);
		MyLock(__FILE__,__LINE__,&Fl_RE,NULL);
		if (Prm->Type>=PrmType_STRING)
		{
			Buf=tV[index].dV.tBf;
			if ((Prm->Size==0)||(tV[index].dV.tSize<Prm->Size))
			{
				Cnt=tV[index].dV.tSize;
			}
			else
			{
				Cnt=Prm->Size;
				tV[index].dV.tBf[Cnt-1]=0;
			}
		}
		else
		{
			Buf=&tV[index].iV.VI64;
			Cnt=SizePrm[Prm->Type];
		};
		rc=0;
// 	Elem=(PLID_Elem*)pList.Find((UCHAR*)&ID,sizeof(INT64));
// 	if (Elem!=NULL) 
// 	{
// 		Buf=Elem->pVol;
// 		Cnt=Elem->Size;
// 		rc=0;
// 	}
		MyUnLock(__FILE__,__LINE__,&Fl_RE,NULL);
	}
	if (Buf==NULL)
		rc=-1;
	return rc;
};

int		ParamListID::GetParam(INT64 ID,void* &Buf)
{
	int	rc=-1;
//	PLID_Elem	*Elem;
	int			index=ID-FirstPrm;
	if(ID>=LastPrm)
	{
		index=ID-LastPrm+EndFirstPrm-FirstPrm;
	}
	if (Fl[index])
	{
		Parametr*	Prm=GetTablParametrID(ID);
		MyLock(__FILE__,__LINE__,&Fl_RE,NULL);
		if (Prm->Type>=PrmType_STRING)
		{
			Buf=tV[index].dV.tBf;
		}
		else
		{
			Buf=&tV[index].iV.VI64;
		}
		rc=0;
// 	Elem=(PLID_Elem*)pList.Find((UCHAR*)&ID,sizeof(INT64),1);
// 	if (Elem!=NULL) 
// 	{
// 		Buf=Elem->pVol;
// 		rc=0;
// 	}
		MyUnLock(__FILE__,__LINE__,&Fl_RE,NULL);
	}
	if (Buf==NULL)
		rc=-1;
	return rc;
};

int		ParamListID::GetParam(INT64 ID,VolPrm* &PrmVol,void* &Buf,size_t &Cnt,Decoder* &ParentMt)
{
	int	rc=-1;
//	PLID_Elem	*Elem;
	ParentMt=NULL;
	int			index=ID-FirstPrm;
	if(ID>=LastPrm)
	{
		index=ID-LastPrm+EndFirstPrm-FirstPrm;
	}
	PrmVol=&tV[index];
	if (Fl[index])
	{
		Parametr*	Prm=GetTablParametrID(ID);
		MyLock(__FILE__,__LINE__,&Fl_RE,NULL);
		if (Prm->Type>=PrmType_STRING)
		{
			Buf=tV[index].dV.tBf;
			if ((Prm->Size==0)||(tV[index].dV.tSize<Prm->Size))
			{
				Cnt=tV[index].dV.tSize;
			}
			else
			{
				Cnt=Prm->Size;
				tV[index].dV.tBf[Cnt-1]=0;
			}
		}
		else
		{
			Buf=&tV[index].iV.VI64;
			Cnt=SizePrm[Prm->Type];
		}
//		ParentMt=Srv;
		rc=0;
		MyUnLock(__FILE__,__LINE__,&Fl_RE,NULL);
	}
	if (Buf==NULL)
		rc=-1;
	return rc;
};

int		ParamListID::GetParam(INT64 ID,VolPrm* &PrmVol,void* &Buf,Decoder* &ParentMt)
{
	int	rc=-1;
//	PLID_Elem	*Elem;
	ParentMt=NULL;
	int			index=ID-FirstPrm;
	if(ID>=LastPrm)
	{
		index=ID-LastPrm+EndFirstPrm-FirstPrm;
	}
	PrmVol=&tV[index];
	if (Fl[index])
	{
		Parametr*	Prm=GetTablParametrID(ID);
		MyLock(__FILE__,__LINE__,&Fl_RE,NULL);
		if (Prm->Type>=PrmType_STRING)
		{
			Buf=tV[index].dV.tBf;
		}
		else
		{
			Buf=&tV[index].iV.VI64;
		}
//		ParentMt=Srv;
		rc=0;
		MyUnLock(__FILE__,__LINE__,&Fl_RE,NULL);
	}
	if (Buf==NULL)
		rc=-1;
	return rc;
};

int		ParamListID::GetParam(INT64 ID,VolPrm* &PrmVol,void* &Buf,size_t &Cnt)
{
	int	rc=-1;
//	PLID_Elem	*Elem;
	int			index=ID-FirstPrm;
	if(ID>=LastPrm)
	{
		index=ID-LastPrm+EndFirstPrm-FirstPrm;
	}
	PrmVol=&tV[index];
	if (Fl[index])
	{
		Parametr*	Prm=GetTablParametrID(ID);
		MyLock(__FILE__,__LINE__,&Fl_RE,NULL);
		if (Prm->Type>=PrmType_STRING)
		{
			Buf=tV[index].dV.tBf;
			if ((Prm->Size==0)||(tV[index].dV.tSize<Prm->Size))
			{
				Cnt=tV[index].dV.tSize;
			}
			else
			{
				Cnt=Prm->Size;
				tV[index].dV.tBf[Cnt-1]=0;
			}
		}
		else
		{
			Buf=&tV[index].iV.VI64;
			Cnt=SizePrm[Prm->Type];
		};
		rc=0;
		MyUnLock(__FILE__,__LINE__,&Fl_RE,NULL);
	}
	if (Buf==NULL)
		rc=-1;
	return rc;
};

int		ParamListID::GetParam(INT64 ID,VolPrm* &PrmVol,void* &Buf)
{
	int	rc=-1;
//	PLID_Elem	*Elem;
	int			index=ID-FirstPrm;
	if(ID>=LastPrm)
	{
		index=ID-LastPrm+EndFirstPrm-FirstPrm;
	}
	PrmVol=&tV[index];
	if (Fl[index])
	{
		Parametr*	Prm=GetTablParametrID(ID);
		MyLock(__FILE__,__LINE__,&Fl_RE,NULL);
		if (Prm->Type>=PrmType_STRING)
		{
			Buf=tV[index].dV.tBf;
		}
		else
		{
			Buf=&tV[index].iV.VI64;
		}
		rc=0;
		MyUnLock(__FILE__,__LINE__,&Fl_RE,NULL);
	}
	if (Buf==NULL)
		rc=-1;
	return rc;
};

int		ParamListID::DelParam(INT64 ID)
{
	int	rc=-1;
	if (ID==0) return rc;
//	PLID_Elem	*Elem;
	int			index=ID-FirstPrm;
	if(ID>=LastPrm)
	{
		index=ID-LastPrm+EndFirstPrm-FirstPrm;
	}
	Parametr*	Prm=GetTablParametrID(ID);
	MyLock(__FILE__,__LINE__,&Fl_RE,NULL);
	LWrTime++;//	=time(NULL);
	if (Fl[index])
	{
		if (Prm->Type>=PrmType_STRING)
		{
			if (tV[index].dV.tBf)
			{
//				Srv->free(tV[index].dV.tBf);
				free(tV[index].dV.tBf);
				tV[index].dV.tBf=NULL;
				tV[index].dV.tSize=0;
			}
		}
		else
		{
			tV[index].iV.VI64=0;
		};
		rc=0;
		NumElem--;
		Fl[index]=FALSE;
	}
// 	Elem=(PLID_Elem*)pList.Find((UCHAR*)&ID,sizeof(INT64));
// 	if (Elem!=NULL) 
// 	{
// 		pList.Del(Elem);
// 		rc=0;
// 	}
	MyUnLock(__FILE__,__LINE__,&Fl_RE,NULL);
	return rc;
};

int		ParamListID::FindFirst(INT64 &ID,void* &Vol,size_t &Cnt,Decoder* &ParentMt)	// rc=-1 íåóäà÷à
{
	int	rc=-1;Cnt=0;
//	PLID_Elem*	Elem;
//	int			index=ID-FirstPrm;
	Parametr*	Prm;
	MyLock(__FILE__,__LINE__,&Fl_RE,NULL);
	ParentMt=NULL;
	for (FindPoi=0;FindPoi<NumDPrm;FindPoi++)
	{
		if(Fl[FindPoi])
		{
			if (FindPoi<(EndFirstPrm-FirstPrm))
				Prm=GetTablParametrID(FindPoi+FirstPrm);
			else
				Prm=GetTablParametrID(FindPoi-(EndFirstPrm-FirstPrm)+LastPrm);
			ID=Prm->UID;

			if (Prm->Type>=PrmType_STRING)
			{
				Vol=tV[FindPoi].dV.tBf;
				if ((Prm->Size==0)||(tV[FindPoi].dV.tSize<Prm->Size))
				{
					Cnt=tV[FindPoi].dV.tSize;
				}
				else
				{
					Cnt=Prm->Size;
					tV[FindPoi].dV.tBf[Cnt-1]=0;
				}
			}
			else
			{
				Vol=&tV[FindPoi].iV.VI64;
				Cnt=SizePrm[Prm->Type];
			}
//			ParentMt=Srv;
			rc=0;
			break;
		}
	}
// 	Elem=(PLID_Elem*)pList.Begin();
// 	if (Elem!=0)
// 	{
// 		ID=*(INT64*)Elem->Buff;Vol=Elem->pVol;Cnt=Elem->Size;
// 		ParentMt=Elem->ParentMt;
// 		rc=0;
// 	}
	MyUnLock(__FILE__,__LINE__,&Fl_RE,NULL);
	return rc;
};
int		ParamListID::FindNext(INT64 &ID,void* &Vol,size_t &Cnt,Decoder* &ParentMt)	// rc=-1 íåóäà÷à
{
	int	rc=-1;Cnt=0;
	ParentMt=NULL;
	Parametr*	Prm;
	int			index=ID-FirstPrm;
	MyLock(__FILE__,__LINE__,&Fl_RE,NULL);
	if (++FindPoi<NumDPrm)
	{
		for (FindPoi;FindPoi<NumDPrm;FindPoi++)
		{
			if(Fl[FindPoi])
			{
				if (FindPoi<(EndFirstPrm-FirstPrm))
					Prm=GetTablParametrID(FindPoi+FirstPrm);
				else
					Prm=GetTablParametrID(FindPoi-(EndFirstPrm-FirstPrm)+LastPrm);
				ID=Prm->UID;

				if (Prm->Type>=PrmType_STRING)
				{
					Vol=tV[FindPoi].dV.tBf;
					if ((Prm->Size==0)||(tV[FindPoi].dV.tSize<Prm->Size))
					{
						Cnt=tV[FindPoi].dV.tSize;
					}
					else
					{
						Cnt=Prm->Size;
						tV[FindPoi].dV.tBf[Cnt-1]=0;
					}
				}
				else
				{
					Vol=&tV[FindPoi].iV.VI64;
					Cnt=SizePrm[Prm->Type];
				}
//				ParentMt=Srv;
				rc=0;
				break;
			}
		}
	}
	else
	{
		FindPoi=0;
	}
// 	Elem=(PLID_Elem*)pList.Next();
// 	if (Elem!=0)
// 	{
// 		ID=*(INT64*)Elem->Buff;Vol=Elem->pVol;Cnt=Elem->Size;
// 		ParentMt=Elem->ParentMt;
// 		rc=0;
// 	}
	MyUnLock(__FILE__,__LINE__,&Fl_RE,NULL);
	return rc;
};


int		ParamListID::FindFirst(INT64 &ID,VolPrm* &VPrm,void* &Vol,size_t &Cnt,Decoder* &ParentMt)	// rc=-1 íåóäà÷à
{
	int	rc=-1;Cnt=0;
	//	PLID_Elem*	Elem;
	//	int			index=ID-FirstPrm;
	Parametr*	Prm;
	MyLock(__FILE__,__LINE__,&Fl_RE,NULL);
	ParentMt=NULL;
	for (FindPoi=0;FindPoi<NumDPrm;FindPoi++)
	{
		if(Fl[FindPoi])
		{
			if (FindPoi<(EndFirstPrm-FirstPrm))
				Prm=GetTablParametrID(FindPoi+FirstPrm);
			else
				Prm=GetTablParametrID(FindPoi-(EndFirstPrm-FirstPrm)+LastPrm);
			ID=Prm->UID;

			if (Prm->Type>=PrmType_STRING)
			{
				Vol=tV[FindPoi].dV.tBf;
				if ((Prm->Size==0)||(tV[FindPoi].dV.tSize<Prm->Size))
				{
					Cnt=tV[FindPoi].dV.tSize;
				}
				else
				{
					Cnt=Prm->Size;
					tV[FindPoi].dV.tBf[Cnt-1]=0;
				}
			}
			else
			{
				Vol=&tV[FindPoi].iV.VI64;
				Cnt=SizePrm[Prm->Type];
			}
			VPrm=&tV[FindPoi];
			//			ParentMt=Srv;
			rc=0;
			break;
		}
	}
	// 	Elem=(PLID_Elem*)pList.Begin();
	// 	if (Elem!=0)
	// 	{
	// 		ID=*(INT64*)Elem->Buff;Vol=Elem->pVol;Cnt=Elem->Size;
	// 		ParentMt=Elem->ParentMt;
	// 		rc=0;
	// 	}
	MyUnLock(__FILE__,__LINE__,&Fl_RE,NULL);
	return rc;
};
int		ParamListID::FindNext(INT64 &ID,VolPrm* &VPrm,void* &Vol,size_t &Cnt,Decoder* &ParentMt)	// rc=-1 íåóäà÷à
{
	int	rc=-1;Cnt=0;
	ParentMt=NULL;
	Parametr*	Prm;
	int			index=ID-FirstPrm;
	MyLock(__FILE__,__LINE__,&Fl_RE,NULL);
	if (++FindPoi<NumDPrm)
	{
		for (FindPoi;FindPoi<NumDPrm;FindPoi++)
		{
			if(Fl[FindPoi])
			{
				if (FindPoi<(EndFirstPrm-FirstPrm))
					Prm=GetTablParametrID(FindPoi+FirstPrm);
				else
					Prm=GetTablParametrID(FindPoi-(EndFirstPrm-FirstPrm)+LastPrm);
				ID=Prm->UID;

				if (Prm->Type>=PrmType_STRING)
				{
					Vol=tV[FindPoi].dV.tBf;
					if ((Prm->Size==0)||(tV[FindPoi].dV.tSize<Prm->Size))
					{
						Cnt=tV[FindPoi].dV.tSize;
					}
					else
					{
						Cnt=Prm->Size;
						tV[FindPoi].dV.tBf[Cnt-1]=0;
					}
				}
				else
				{
					Vol=&tV[FindPoi].iV.VI64;
					Cnt=SizePrm[Prm->Type];
				}
				//				ParentMt=Srv;
				rc=0;
				VPrm=&tV[FindPoi];
				break;
			}
		}
	}
	else
	{
		FindPoi=0;
	}
	MyUnLock(__FILE__,__LINE__,&Fl_RE,NULL);
	return rc;
};

VolPrm*		ParamListID::Find(INT64 ID)
{
	VolPrm*	Elem=NULL;
	MyLock(__FILE__,__LINE__,&Fl_RE,NULL);
	int			index=ID-FirstPrm;
	if(ID>=LastPrm)
	{
		index=ID-LastPrm+EndFirstPrm-FirstPrm;
	}
	Elem=&tV[index];
// 	if (ID!=0)
// 		Elem=(PLID_Elem*)pList.Find((UCHAR*)&ID,sizeof(INT64),1);
	MyUnLock(__FILE__,__LINE__,&Fl_RE,NULL);
	return Elem;
};	// Óñòàíàâëèâàåò óêàçàòåëü ïîèñêà íà ïîçèöèþ ñîîòâåòñòâóþùóþ óñëîâèþ ïîèñêà

int		ParamListID::FindFirst(INT64 &ID,void* &Vol,size_t &Cnt)	// rc=-1 íåóäà÷à
{
	int	rc=-1;Cnt=0;
//	PLID_Elem*	Elem;
	Parametr*	Prm;
	MyLock(__FILE__,__LINE__,&Fl_RE,NULL);
	for (FindPoi=0;FindPoi<NumDPrm;FindPoi++)
	{
		if(Fl[FindPoi])
		{
			if (FindPoi<(EndFirstPrm-FirstPrm))
				Prm=GetTablParametrID(FindPoi+FirstPrm);
			else
				Prm=GetTablParametrID(FindPoi-(EndFirstPrm-FirstPrm)+LastPrm);
			ID=Prm->UID;
			if (Prm->Type>=PrmType_STRING)
			{
				Vol=tV[FindPoi].dV.tBf;
				if ((Prm->Size==0)||(tV[FindPoi].dV.tSize<Prm->Size))
				{
					Cnt=tV[FindPoi].dV.tSize;
				}
				else
				{
					Cnt=Prm->Size;
					tV[FindPoi].dV.tBf[Cnt-1]=0;
				}
			}
			else
			{
				Vol=&tV[FindPoi].iV.VI64;
				Cnt=SizePrm[Prm->Type];
			}
			rc=0;
			break;
		}
	}
// 	Elem=(PLID_Elem*)pList.Begin();
// 	if (Elem!=0)
// 	{
// 		ID=*(INT64*)Elem->Buff;Vol=Elem->pVol;Cnt=Elem->Size;
// 		rc=0;
// 	}
	MyUnLock(__FILE__,__LINE__,&Fl_RE,NULL);
	return rc;
};

int		ParamListID::FindNext(INT64 &ID,void* &Vol,size_t &Cnt)	// rc=-1 íåóäà÷à
{
	int	rc=-1;Cnt=0;
//	PLID_Elem*	Elem;
	Parametr*	Prm;
	int			index=ID-FirstPrm;
	MyLock(__FILE__,__LINE__,&Fl_RE,NULL);
	if (++FindPoi<NumDPrm)
	{
		for (FindPoi;FindPoi<NumDPrm;FindPoi++)
		{
			if(Fl[FindPoi])
			{
				if (FindPoi<(EndFirstPrm-FirstPrm))
					Prm=GetTablParametrID(FindPoi+FirstPrm);
				else
					Prm=GetTablParametrID(FindPoi-(EndFirstPrm-FirstPrm)+LastPrm);
				ID=Prm->UID;
				if (Prm->Type>=PrmType_STRING)
				{
					Vol=tV[FindPoi].dV.tBf;
					if ((Prm->Size==0)||(tV[FindPoi].dV.tSize<Prm->Size))
					{
						Cnt=tV[FindPoi].dV.tSize;
					}
					else
					{
						Cnt=Prm->Size;
						tV[FindPoi].dV.tBf[Cnt-1]=0;
					}
				}
				else
				{
					Vol=&tV[FindPoi].iV.VI64;
					Cnt=SizePrm[Prm->Type];
				};
				rc=0;
				break;
			}
		}
	}
	else
	{
		FindPoi=0;
	}
// 	Elem=(PLID_Elem*)pList.Next();
// 	if (Elem!=0)
// 	{
// 		ID=*(INT64*)Elem->Buff;Vol=Elem->pVol;Cnt=Elem->Size;
// 		rc=0;
// 	}
	MyUnLock(__FILE__,__LINE__,&Fl_RE,NULL);
	return rc;
};


int		ParamListID::FindFirst(INT64 &ID,VolPrm* &VPrm,void* &Vol,size_t &Cnt)	// rc=-1 íåóäà÷à
{
	int	rc=-1;Cnt=0;
	//	PLID_Elem*	Elem;
	Parametr*	Prm;
	MyLock(__FILE__,__LINE__,&Fl_RE,NULL);
	for (FindPoi=0;FindPoi<NumDPrm;FindPoi++)
	{
		if(Fl[FindPoi])
		{
			if (FindPoi<(EndFirstPrm-FirstPrm))
				Prm=GetTablParametrID(FindPoi+FirstPrm);
			else
				Prm=GetTablParametrID(FindPoi-(EndFirstPrm-FirstPrm)+LastPrm);
			ID=Prm->UID;
			if (Prm->Type>=PrmType_STRING)
			{
				Vol=tV[FindPoi].dV.tBf;
				if ((Prm->Size==0)||(tV[FindPoi].dV.tSize<Prm->Size))
				{
					Cnt=tV[FindPoi].dV.tSize;
				}
				else
				{
					Cnt=Prm->Size;
					tV[FindPoi].dV.tBf[Cnt-1]=0;
				}
			}
			else
			{
				Vol=&tV[FindPoi].iV.VI64;
				Cnt=SizePrm[Prm->Type];
			}
			rc=0;
			VPrm=&tV[FindPoi];
			break;
		}
	}
	MyUnLock(__FILE__,__LINE__,&Fl_RE,NULL);
	return rc;
};

int		ParamListID::FindNext(INT64 &ID,VolPrm* &VPrm,void* &Vol,size_t &Cnt)	// rc=-1 íåóäà÷à
{
	int	rc=-1;Cnt=0;
	//	PLID_Elem*	Elem;
	Parametr*	Prm;
	int			index=ID-FirstPrm;
	MyLock(__FILE__,__LINE__,&Fl_RE,NULL);
	if (++FindPoi<NumDPrm)
	{
		for (FindPoi;FindPoi<NumDPrm;FindPoi++)
		{
			if(Fl[FindPoi])
			{
				if (FindPoi<(EndFirstPrm-FirstPrm))
					Prm=GetTablParametrID(FindPoi+FirstPrm);
				else
					Prm=GetTablParametrID(FindPoi-(EndFirstPrm-FirstPrm)+LastPrm);
				ID=Prm->UID;
				if (Prm->Type>=PrmType_STRING)
				{
					Vol=tV[FindPoi].dV.tBf;
					if ((Prm->Size==0)||(tV[FindPoi].dV.tSize<Prm->Size))
					{
						Cnt=tV[FindPoi].dV.tSize;
					}
					else
					{
						Cnt=Prm->Size;
						tV[FindPoi].dV.tBf[Cnt-1]=0;
					}
				}
				else
				{
					Vol=&tV[FindPoi].iV.VI64;
					Cnt=SizePrm[Prm->Type];
				};
				rc=0;
				VPrm=&tV[FindPoi];
				break;
			}
		}
	}
	else
	{
		FindPoi=0;
	}
	MyUnLock(__FILE__,__LINE__,&Fl_RE,NULL);
	return rc;
};
/*
int		ParamListID::FindNext(INT64 &ID,void* &Vol,size_t &Cnt,long &mLWrTime)	// rc=-1 íåóäà÷à
{
	int	rc=-1;Cnt=0;
//	PLID_Elem*	Elem;
	Parametr*	Prm;
	MyLock(__FILE__,__LINE__,&Fl_RE,NULL);
	if (++FindPoi>=NumDPrm)FindPoi=0;
	for (FindPoi;FindPoi<NumDPrm;FindPoi++)
	{
		if(Fl[FindPoi])
		{
			if (FindPoi<(EndFirstPrm-FirstPrm))
				Prm=GetTablParametrID(FindPoi+FirstPrm);
			else
				Prm=GetTablParametrID(FindPoi-(EndFirstPrm-FirstPrm)+LastPrm);
			ID=Prm->UID;
			if (Prm->Type>=PrmType_STRING)
			{
				Vol=tV[FindPoi].dV.tBf;
				if ((Prm->Size==0)||(tV[FindPoi].dV.tSize<Prm->Size))
				{
					Cnt=tV[FindPoi].dV.tSize;
				}
				else
				{
					Cnt=Prm->Size;
					tV[FindPoi].dV.tBf[Cnt-1]=0;
				}
			}
			else
			{
				Vol=&tV[FindPoi].iV.VI64;
				Cnt=SizePrm[Prm->Type];
			}
			rc=0;
		}
	}
// 	Elem=(PLID_Elem*)pList.Next();
// 	if (Elem!=0)
// 	{
// 		ID=*(INT64*)Elem->Buff;Vol=Elem->pVol;Cnt=Elem->Size;mLWrTime=Elem->LWrTime;
// 		rc=0;
// 	}
	MyUnLock(__FILE__,__LINE__,&Fl_RE,NULL);
	return rc;
};
*/
void	ParamListID::TestValid()
{
};

void	ParamListID::DelAllElem()
{
	int	i;
	Parametr*	Prm;
	MyLock(__FILE__,__LINE__,&Fl_RE,NULL);
	for (i=0;i<NumDPrm;i++)
	{
		if (Fl[i])
		{
			if (i<(EndFirstPrm-FirstPrm))
				Prm=GetTablParametrID(i+FirstPrm);
			else
				Prm=GetTablParametrID(i-(EndFirstPrm-FirstPrm)+LastPrm);
			if (Prm->Type>=PrmType_STRING)
			{
				if (tV[i].dV.tBf)
				{
//					Srv->free(tV[i].dV.tBf);
					free(tV[i].dV.tBf);
					tV[i].dV.tBf=NULL;
					tV[i].dV.tSize=0;
				}
			}
			else
			{
				tV[i].iV.VI64=0;
			};
			NumElem--;
			Fl[i]=FALSE;
		}
	}
	MyUnLock(__FILE__,__LINE__,&Fl_RE,NULL);
};

ParamListID::~ParamListID()
{
	DelAllElem();
//	--DBGC.Cnt_pList;
#ifdef	_DEBUG
	DBGC.DelPList(pName);
#endif
};

#endif


Dbg_Cnt::Dbg_Cnt()
{
	AllName=0;
};
Dbg_Cnt::~Dbg_Cnt()
{
};

#ifdef _DEBUG
void	Dbg_Cnt::AddPList(char* Name)
{
	Cnt_pList++;
	int i;
	for (i=0;i<AllName;i++)
	{
		if (strcmp(NameMas[i],Name)==0)
		{
			C_pList[i]++;break;
		}
	}
	if (i==AllName)
	{
		strcpy(NameMas[i],Name);
		C_pList[i]++;
		AllName++;
	}
};
void	Dbg_Cnt::DelPList(char* Name)
{
	Cnt_pList--;
	int i;
	for (i=0;i<AllName;i++)
	{
		if (strcmp(NameMas[i],Name)==0)
		{
			C_pList[i]--;break;
		}
	}
};
#endif

// -------------------------------------------------------------------------------------------





// -------------------------------------------------------------------------------------------
/*
ID_List8::ID_List8(int Lv,ID_List8*	PRoot)
{
	int	i;
	NumElem=0;FlReEn=0;
	Level=Lv;	
	tPElem=0;
	FlBeg=0;FlEnd=0;
	for (i=0;i<256;i++){ Lev[i]=NULL; Elem[i]=NULL;};
	PoiRoot=PRoot;
	DelPool=NULL;
	NumDelPool=0;
//	SizeDelPool=0;
	DelPoolEl=NULL;
	NumDelPoolEl=0;
//	SizeDelPoolEl=0;
	Fl_DL=0;CntDelInsert=0;
}

// --------------------------
ID_List8::ID_List8()
{
	int	i;
	PoiRoot=this;FlReEn=0;
	NumElem=0;
	tPElem=0;
	Level=0;
	FlBeg=0;FlEnd=0;
//	Fl_Recurse=1;
	for (i=0;i<256;i++)
	{
		Lev[i]=NULL; Elem[i]=NULL;
	}
	DelPool=NULL;
	DelPoolEl=NULL;
	NumDelPool=0;
	NumDelPoolEl=0;
	Fl_DL=0;CntDelInsert=0;
};

void	ID_List8::Init(int Lv,ID_List8*	PRoot)
{
	int	i;
	NumElem=0;FlReEn=0;
	Level=Lv;	
	tPElem=0;
	FlBeg=0;FlEnd=0;
	for (i=0;i<256;i++){ Lev[i]=NULL; Elem[i]=NULL;};
	PoiRoot=PRoot;
	NumDelPool=0;
//	SizeDelPool=0;
	DelPool=NULL;
	DelPoolEl=NULL;
	NumDelPoolEl=0;CntDelInsert=0;
//	SizeDelPoolEl=0;
}

ID_List8::~ID_List8()
{
	int	i;
	for (i=0;i<256;i++)
	{
		if (Lev[i]!=NULL)
		{
			delete Lev[i]; Lev[i]=NULL;
		}
		if (Elem[i]!=NULL)
		{
			delete Elem[i]; Elem[i]=NULL;
		}
	}
	if (DelPool!=NULL)
	{
		for (i=0;i<NumDelPool;i++)
		{
			delete DelPool[i];
		}
		free(DelPool);DelPool=NULL;
	}
	if (DelPoolEl!=NULL)
	{
		for (i=0;i<NumDelPoolEl;i++)
		{
			delete DelPoolEl[i];
		}
		free(DelPoolEl);DelPoolEl=NULL;
	}
};


void ID_List8::DelAllElem()
{
	int	i;
	ID_Elem		*IDe;
	ID_List8	*IDl;
	for (i=0;i<256;i++)
	{
		if (Lev[i]!=NULL)
		{
			IDl=Lev[i];
			Lev[i]=NULL;
			DelLev(IDl);
//			delete Lev[i];
		}
		if (Elem[i]!=NULL)
		{
			IDe=Elem[i];
			Elem[i]=NULL;
			DelElem(IDe);
//			delete Elem[i];
//			Elem[i]=NULL;
		}
	};
	NumElem=0;
	tPElem=0;
	Level=0;
	FlBeg=0;FlEnd=0;
};

void ID_List8::RemoveAllPoiElem()
{
	int	i;
	ID_List8	*IDl;
	for (i=0;i<256;i++)
	{
		if (Lev[i]!=NULL)
		{
			Lev[i]->RemoveAllPoiElem();
			IDl=Lev[i];
			Lev[i]=NULL;
			DelRLev(IDl);
//			delete Lev[i]; 
		}
		if (Elem[i]!=NULL)
		{
			//delete Elem[i];
			Elem[i]=NULL;
		}
	};
	NumElem=0;
	tPElem=0;
	Level=0;
	FlBeg=0;FlEnd=0;
};

// --------------------------

void ID_List8::BeforeBegin()
{
	if (++FlReEn>1)
		{size_t iasm=1;}
	for(tPElem=0;tPElem<256;tPElem++)
	{
		if (Lev[tPElem]!=0)
		{
			Lev[tPElem]->BeforeBegin();
		}
	}
	tPElem=0;FlBeg=0;
	FlReEn--;
}

void ID_List8::BeforeEnd()
{
	if (++FlReEn>1)
		{size_t iasm=1;}
	for(tPElem=255;tPElem>=0;tPElem--)
	{
		if (Lev[tPElem]!=0)
		{
			Lev[tPElem]->BeforeEnd();
		}
	}
	tPElem=255;FlEnd=0;
	FlReEn--;
}

ID_Elem*	ID_List8::Begin()
{	
	ID_Elem*	elem;
	if (++FlReEn>1)
		{size_t iasm=1;}
	for(tPElem=0;tPElem<256;tPElem++)
	{
		if (Lev[tPElem]!=0)
		{
			Lev[tPElem]->BeforeBegin();
		}
	}
	tPElem=0;FlBeg=0;	for (tPElem=0;tPElem<256;tPElem++)
	{
		elem=Elem[tPElem];
		if (FlBeg==0)
		{
			if (elem!=0)
			{
				FlBeg=1;
				FlReEn--;
				return elem;
			}
		};
		if (Lev[tPElem]!=0)
		{
			FlReEn--;
			if (elem!=0)
				return elem;
			else
				return Lev[tPElem]->Begin();
		}
		else
		{
			if (FlBeg==0)
				if (elem!=0)
				{
					FlBeg=0;
					tPElem++;
					FlReEn--;
					return elem;
				};
		};
		FlBeg=0;
	};
	FlReEn--;
	return 0;
};

ID_Elem*	ID_List8::End()
{	
	ID_Elem*	elem;
	if (++FlReEn>1)
		{size_t iasm=1;}
	for(tPElem=255;tPElem>=0;tPElem--)
	{
		if (Lev[tPElem]!=0)
		{
			Lev[tPElem]->BeforeEnd();
		}
	}
	tPElem=255;FlEnd=0;	for (tPElem=255;tPElem>=0;tPElem--)
	{
		elem=Elem[tPElem];
		if (FlEnd==0)
		{
			if (elem!=0)
			{
				FlEnd=1;
				return elem;
			}
		};
		if (Lev[tPElem]!=0)
		{
			if (elem!=0)
				return elem;
			else
				return Lev[tPElem]->End();
		}
		else
		{
			if (FlEnd==0)
				if (elem!=0)
				{
					FlEnd=0;
					tPElem--;
					return elem;
				};
		};
		FlEnd=0;
	};
	FlReEn--;
	return 0;
};

ID_Elem*	ID_List8::GetActive()
{
	ID_Elem*	tPoi=NULL;
	if (tPElem<256)
	{
		if (Lev[tPElem])
		{
			tPoi=Lev[tPElem]->GetActive();
		}
		if (Elem[tPElem])
			tPoi=Elem[tPElem];
	}
	return tPoi;
};

ID_Elem*	ID_List8::Perviose()
{
	if (++FlReEn>1)
		{size_t iasm=1;}
	ID_Elem*	tPoi=NULL;
	if (tPElem>255) 
	{
		tPElem=255;
	}
_Pre00:
	if (tPElem>0)
	{
		if (Elem[tPElem]!=NULL)
		{
			tPElem--;
			for (tPElem;tPElem>=0;--tPElem)
			{
				if (Elem[tPElem]!=NULL)
				{
					tPoi=Elem[tPElem];
					FlBeg=1;
					break;
				}
				if (Lev[tPElem]!=NULL)
				{
					Lev[tPElem]->tPElem=255;
					tPoi=Lev[tPElem]->Perviose();
					if (tPoi!=NULL)
						break;
				}
			}
		}
		else
		{
			if (Lev[tPElem]!=NULL)
			{
				tPoi=Lev[tPElem]->Perviose();
				if (tPoi==NULL)
				{
					tPElem--;
					if  (Lev[tPElem]) Lev[tPElem]->tPElem=255;
					goto _Pre00;
				}
			}
			else
			{
				tPElem--;
				for (tPElem;tPElem>=0;--tPElem)
				{
					if (Elem[tPElem]!=NULL)
					{
						tPoi=Elem[tPElem];
						FlBeg=1;
						break;
					}
					if (Lev[tPElem]!=NULL)
					{
						Lev[tPElem]->tPElem=255;
						tPoi=Lev[tPElem]->Perviose();
						if (tPoi!=NULL)
							break;
					}
				}
			}
		};
	}
	else
	{
		tPElem=0;
	}
	FlReEn--;
	return tPoi;
};

ID_Elem*	ID_List8::Next()
{
	if (++FlReEn>1)
		{size_t iasm=1;}
	ID_Elem*	tPoi=NULL;
	ID_Elem*	elem;
	if (tPElem>255) 
	{
		tPElem=0;
		FlReEn--;
		return 0;
	}
	if (FlBeg==0)
	{
		if (Elem[tPElem]!=0)
		{
			FlBeg=1;
			FlReEn--;
			return Elem[tPElem];
		}
	};
	if (Lev[tPElem]!=0)
	{
		tPoi=Lev[tPElem]->Next();
		if (tPoi!=0)
		{
			FlReEn--;
			return tPoi;
		}
		tPElem++;
	}
	for (tPElem=tPElem;tPElem<256;tPElem++)
	{
		elem=Elem[tPElem];
		if (FlBeg==0)
		{
			if (elem!=0)
			{
				FlBeg=1;
				FlReEn--;
				return elem;
			}
		};
		if (Lev[tPElem]!=0)
		{
			FlBeg=0;
			Lev[tPElem]->tPElem=0;
			tPoi=Lev[tPElem]->Next();
			if (tPoi!=0)
			{
				FlReEn--;
				return tPoi;
			}
		}
		else
		{
			if (FlBeg==0)
				if (elem!=0)
				{
					FlBeg=0;
					tPElem++;
					FlReEn--;
					return elem;
				};
		}
		FlBeg=0;
	}
	FlReEn--;
	return tPoi;
};

int			ID_List8::CalcObjSize()
{
	int	i;
	int	Size=sizeof(ID_List8);	// ªøÿð   ¸ðü ;)
	for (i=0;i<256;i++)
	if (Lev[i]!=0)
	{
		Size+=Lev[i]->CalcObjSize();
	};
	return Size;
};

int			ID_List8::Add(ID_Elem* IDE)
{
	int	rc=0;
	if (++FlReEn>1)
		{size_t iasm=1;}
	if (IDE)
	{
		UCHAR	TempPtr=IDE->Buff[Level];
		if(Lev[TempPtr]==0)
		{
			if (Elem[TempPtr]==NULL)
			{
				// ýþòvù ¤ûõüõýª
				Elem[TempPtr]=IDE;
			}
			else
			{
				if (Elem[TempPtr]->Len>Level+1)
				{
					Lev[TempPtr]=NewLev(Level+1);
					//				Lev[TempPtr]=new ID_List8(Level+1,PoiRoot);
					Lev[TempPtr]->Add(Elem[TempPtr]);
					Elem[TempPtr]=NULL;
					if (IDE->Len<=Level+1)
						Elem[TempPtr]=IDE;		// +¸ªðõª¸  ÷ôõ¸¹
					else
						rc=Lev[TempPtr]->Add(IDE); // ¦õ¨õüõ•ðõü ôðû¹°õ
				}
				else
				{
					if (IDE->Len<=Level+1)	
						rc=-1;				// ¦þûýþõ ¸þòÿðôõýøõ
					else
					{
						Lev[TempPtr]=NewLev(Level+1);
						//					Lev[TempPtr]=new ID_List8(Level+1,PoiRoot);
						Lev[TempPtr]->Add(IDE);
					};
				};
			}
		}
		else
		{
			if (IDE->Len<=Level+1)
			{	// !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
				if (Elem[TempPtr]==0)
					Elem[TempPtr]=IDE;
				else
				{
					if (Elem[TempPtr]->Len<=Level+1)
						rc=-1;	// ¦ðÿø¸ðª¹ ýõòþ÷üþöýþ
					else
					{
						rc=Lev[TempPtr]->Add(Elem[TempPtr]);
						Elem[TempPtr]=IDE;
					}
				}
			}
			else
			{
				rc=Lev[TempPtr]->Add(IDE);
			}
		};
	}
	else
	{
		rc=-1;
	}
	if (rc!=-1) NumElem++;
	FlReEn--;
	return rc;
};

int			ID_List8::Del(ID_Elem* IDE)
{
	int	rc=0;
	if (++FlReEn>1)
		{size_t iasm=1;}
	ID_Elem		*IDe;
	ID_List8	*IDl;
	if (IDE)
	{
		UCHAR	TempPtr=IDE->Buff[Level];
		if(Lev[TempPtr]==0)
		{
			if (Elem[TempPtr]==IDE)
			{	// ¤ªþ þý !!!
				IDe=Elem[TempPtr];
				Elem[TempPtr]=0;
				DelElem(IDe);
	//			delete Elem[TempPtr];
			}
			else
			{
				rc=-1;
			}
		}
		else
		{
			if (IDE->Len<=Level+1)
				rc=-1;	// =ðùªø ýõòþ÷üþöýþ
			else
			{
				if ((rc=Lev[TempPtr]->Del(IDE))==0)
				{
					IDl=Lev[TempPtr];
					Lev[TempPtr]=0;
					DelLev(IDl);
				};
			};
		}
		if (rc!=-1)
			rc=--NumElem;
	}
	FlReEn--;
	return rc;
};

ID_List8*	ID_List8::NewLev(int	Lev)
{
	ID_List8*	pLev;
	if (PoiRoot->NumDelPool!=0)
	{
		pLev=PoiRoot->DelPool[--PoiRoot->NumDelPool];
		pLev->Init(Lev,PoiRoot);
	}
	else
	{
		pLev=new ID_List8(Lev,PoiRoot);
	}
	return	pLev;
};

void		ID_List8::DelLev(ID_List8* pLev)
{
//	if (++FlReEn>1)
//		{size_t iasm=1;}
	int	i;
	ID_Elem		*IDe;
	ID_List8	*IDl;
	for (i=0;i<256;i++)
	{
		if (pLev->Lev[i]!=NULL)
		{
			IDl=pLev->Lev[i];
			pLev->Lev[i]=NULL;
			DelLev(IDl);
		}
		if (pLev->Elem[i]!=NULL)
		{
			IDe=pLev->Elem[i];
			pLev->Elem[i]=NULL;
			DelElem(IDe);
		}
	}
	if (PoiRoot->DelPool==0)
	{
		delete pLev;
	}
	else
	{
		if (PoiRoot->NumDelPool>=SizeDPool)
			delete pLev;
		else
			PoiRoot->DelPool[PoiRoot->NumDelPool++]=pLev;
	}
//	PoiRoot->DelPool[PoiRoot->NumDelPool++]=pLev;
//	if (PoiRoot->NumDelPool>=PoiRoot->SizeDelPool)
//	{
//		PoiRoot->SizeDelPool+=100;
//		PoiRoot->DelPool=(ID_List8**)realloc(PoiRoot->DelPool,PoiRoot->SizeDelPool*sizeof(ID_List8*));
//	}
	FlReEn--;
};

void		ID_List8::DelRLev(ID_List8* pLev)
{
	int	i;
	ID_List8	*IDe;
	for (i=0;i<256;i++)
	{
		if (pLev->Lev[i]!=NULL)
		{
			// !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!! Tðúþóþ ýõ ôþûöýþ ñvª¹ !!!!
			IDe=pLev->Lev[i];
			pLev->Lev[i]=NULL;
			DelRLev(IDe); 
		}
	}
	if (PoiRoot->DelPool==0)
	{
			delete pLev;
	}
	else
	{
		if (PoiRoot->NumDelPool>=SizeDPool)
			delete pLev;
		else
			PoiRoot->DelPool[PoiRoot->NumDelPool++]=pLev;
	}
//	PoiRoot->SizeDelPool+=100;
//	PoiRoot->DelPool=(ID_List8**)realloc(PoiRoot->DelPool,PoiRoot->SizeDelPool*sizeof(ID_List8*));
	FlReEn--;
};

int			ID_List8::Remove(ID_Elem* IDE)
{
	if (++FlReEn>1)
		{size_t iasm=1;}
	int	rc=0;
	ID_List8	*IDl;
	UCHAR	TempPtr=IDE->Buff[Level];
	if(Lev[TempPtr]==0)
	{
		if (Elem[TempPtr]==IDE)
		{	// ¤ªþ þý !!!
			//delete Elem[TempPtr]; 
			Elem[TempPtr]=0;
//			delete Lev[TempPtr]; Lev[TempPtr]=0;
		}
		else
		{
			// ýº ýø¯øóð ¸õñõ !!!
			rc=-1;
		}
	} 
	else
	{
		if (IDE->Len<=Level+1)
			rc=-1;	// =ðùªø ýõòþ÷üþöýþ
		else ;
		{
			if ((rc=Lev[TempPtr]->Remove(IDE))==0)
			{
				IDl=Lev[TempPtr];
				Lev[TempPtr]=0;
				DelRLev(IDl);
//				delete Lev[TempPtr];
			};
		};
	}
	if (rc!=-1) 
		rc=--NumElem;
	FlReEn--;
	return rc;
};

//-------------------------------------------------------------------------------------------------
ID_Elem* ID_List8::SearchSIGN(UCHAR* Buff,size_t Cnt)
{
	if (++FlReEn>1)
		{size_t iasm=1;}
	UCHAR	TempPtr=Buff[Level];
	ID_List8*	lev=Lev[TempPtr];
	ID_Elem*	elem=Elem[TempPtr];
	if(lev==0)
	{
		if(elem!=NULL)
		{
//			if(elem->Len==Cnt)
				if(memcmp(elem->Buff,Buff,elem->Len)==0)
				{	
					FlReEn--;
					return elem;
				}
		}
	}
	else
	{
		if(elem!=NULL)
		{
//			if(elem->Len==Cnt)
				if(memcmp(elem->Buff,Buff,elem->Len)==0)
				{
					FlReEn--;
					return elem;
				}
		}
//		if (Cnt<=1+Level)
		FlReEn--;

		if (Cnt<=1+Level)
			return 0;	// =ðùªø ýõòþ÷üþöýþ
		return lev->SearchSIGN(Buff,Cnt);
//		return Lev[TempPtr]->Find(Buff,Cnt);
	}
	FlReEn--;
	return 0;
}
//-------------------------------------------------------------------------------------------------
ID_Elem*	ID_List8::Find(UCHAR* Buff,int Cnt,int Cond)
{
	if (++FlReEn>1)
		{size_t iasm=1;}
	UCHAR	TempPtr=Buff[Level];
	ID_List8*	lev=Lev[TempPtr];
	ID_Elem*	elem=Elem[TempPtr];
	if (Cond!=0) tPElem=TempPtr;
	if(lev==0)
	{
		if (elem!=NULL)
		{
			if (elem->Len==Cnt)
				if (memcmp(elem->Buff,Buff,Cnt)==0)
				{
					FlBeg=1;
					FlReEn--;
					return elem;
				}
		}
	}
	else
	{
		if (elem!=NULL)
		{
			if (elem->Len==Cnt)
				if (memcmp(elem->Buff,Buff,Cnt)==0)
				{
					FlBeg=1;
					FlReEn--;
					return elem;
				}
		}
		FlReEn--;
		if (Cnt<=1+Level)
			return 0;	// =ðùªø ýõòþ÷üþöýþ
		return lev->Find(Buff,Cnt,Cond);
	}
	FlReEn--;
	return 0;	// 
};
*/


ID_List8::ID_List8(int Lv,ID_List8*	PRoot)
{
	DBGC.Cnt_IDList8++;
	CriErr=0;
	int	i;
	NumElem=0;
//	FlReEn=0;
	Level=Lv;	
	tPElem=0;
	FlBeg=0;FlEnd=0;
	for (i=0;i<=256;i++){ Lev[i]=NULL; ElemM[i].Elem=NULL;};
	PoiRoot=PRoot;
	DelPool=NULL;
	NumDelPool=0;
//	SizeDelPool=0;
	DelPoolEl=NULL;
	NumDelPoolEl=0;
//	SizeDelPoolEl=0;
	Fl_DL=0;CntDelInsert=0;
	/*ActiveNEl=ActivePEl=ActiveEl=*/BeginEl=EndEl=NULL;
//	hmtx=NULL;
	Fl_Del=0;
	RStr=NULL;
}

// --------------------------
ID_List8::ID_List8()
{
	DBGC.Cnt_IDList8++;
	CriErr=0;
	int	i;
	PoiRoot=this;
//	FlReEn=0;
	NumElem=0;
	tPElem=0;
	Level=0;
	FlBeg=0;FlEnd=0;
//	Fl_Recurse=1;
	for (i=0;i<=256;i++)
	{
		Lev[i]=NULL; ElemM[i].Elem=NULL;
	}
	DelPool=NULL;
	DelPoolEl=NULL;
	NumDelPool=0;
	NumDelPoolEl=0;
	Fl_DL=0;CntDelInsert=0;
	/*ActiveNEl=ActivePEl=ActiveEl=*/BeginEl=EndEl=NULL;
	char	tBB[128];
	sprintf(tBB,"IDList%p",this);
//	hmtx=OpenSemaphore(SEMAPHORE_ALL_ACCESS	,TRUE,tBB);
//	if (hmtx==NULL)
//		hmtx=CreateSemaphore(NULL,1,1,tBB);
	Fl_Del=0;
	RStr=new RootSturct();
};

void	ID_List8::Init(int Lv,ID_List8*	PRoot)
{
	int	i;
	NumElem=0;
//	FlReEn=0;
	Level=Lv;	
	tPElem=0;
	FlBeg=0;FlEnd=0;
	for (i=0;i<=256;i++){ Lev[i]=NULL; ElemM[i].Elem=NULL;};
	PoiRoot=PRoot;
	NumDelPool=0;
	DelPool=NULL;
	DelPoolEl=NULL;
	NumDelPoolEl=0;CntDelInsert=0;
	/*ActiveNEl=ActivePEl=ActiveEl=*/BeginEl=EndEl=NULL;
	Fl_Del=0;
}

ID_List8::~ID_List8()
{
	int	i;
	for (i=0;i<=256;i++)
	{
		if (Lev[i]!=NULL)
		{
			DelLev(Lev[i]);/*delete Lev[i];*/ Lev[i]=NULL;
		}
		if (ElemM[i].Elem!=NULL)
		{
			DelElem(&ElemM[i]);/*delete ElemM[i].Elem;*/ ElemM[i].Elem=NULL;
		}
	}
	if (DelPool!=NULL)
	{
		for (i=0;i<NumDelPool;i++)
		{
			delete DelPool[i];
		}
		free(DelPool);DelPool=NULL;NumDelPool=0;
	}
	if (DelPoolEl!=NULL)
	{
		for (i=0;i<NumDelPoolEl;i++)
		{
			delete DelPoolEl[i];
		}
		free(DelPoolEl);DelPoolEl=NULL;NumDelPoolEl=0;
	}
//	if(hmtx)
//		CloseHandle(hmtx);
	--DBGC.Cnt_IDList8;
	if (RStr)
		delete RStr;
	RStr=NULL;
};

int	ID_List8::Add(ID_Elem* IDE)
{
	int	rc=-1;
	if (Fl_Del==0)
	{
//		WaitForSingleObject(hmtx,INFINITE);
		MyLock(__FILE__,__LINE__,&FlReEn,NULL);//strcpy(LastFunc,"Add");
//		if (++FlReEn>1)
//		{
//			CriErr=GetServices()->SetCriticalError(NULL,"ID_List8:Add:FlReEn","Error","ID_List8",CriErr);
//		}
		ID_ElemM* tIDEM=AddI(IDE);
		if (tIDEM)
			rc=0;
//		LastFunc[0]=0;
		MyUnLock(__FILE__,__LINE__,&FlReEn,NULL);//FlReEn=0;
//		FlReEn--;
//		ReleaseSemaphore(hmtx,1,NULL);
	}
	return rc;
};

bool ID_List8::TestValid()
{
int			TNEl=0;
ID_Elem*	tvEl;
//ID_ElemM*	tActiveEl;
	MyLock(__FILE__,__LINE__,&FlReEn,NULL);//FlReEn=0;
//	tActiveEl=ActiveEl;
	FindStrList8	FSL8;
	tvEl=Begin(FSL8);
	while(tvEl)
	{
		TNEl++;
		tvEl=Next(FSL8);
	};
//	ActiveEl=tActiveEl;
	if (TNEl!=NumElem)
	{
//_mmmm4:
//		Beep(2000,100);
//		Sleep(1000);
//			goto	_mmmm4;
	}
	MyUnLock(__FILE__,__LINE__,&FlReEn,NULL);//FlReEn=0;
	return TRUE;
};

void	FindStrList8::DestroyStr(void)
{
	if (IDL)
	{
		IDL->DelFSL(ID_FSL);
		ID_FSL=255;IDL=NULL;
	}
};

FindStrList8::~FindStrList8()
{
	if (IDL)
	{
		IDL->DelFSL(ID_FSL);
		ID_FSL=255;IDL=NULL;
	}
}

uchar ID_List8::AddFSL(FindStrList8* FSL)
{
	if (RStr)
	{
		if(FSL->ID_FSL!=255)
		{
			if (FSL->ID_FSL>=RStr->MaxPoi)
			{
				FSL->DestroyStr();
			}
			else
			{
				if (FSL->IDL!=this)
				{	// Error!!!
					FSL->DestroyStr();
				};
			}
		}
//		else
		if(FSL->ID_FSL==255)
		{
			FSL->IDL=this;
			uchar	i;
			for (i=0;i<RStr->MaxPoi;i++)
			{
				if(RStr->Fl_PoiFSL[i]==0)
				{
					RStr->Fl_PoiFSL[i]=1;
					RStr->PoiFSL[i]=(size_t)FSL;	//?????
					FSL->ID_FSL=i;
					RStr->ActiveEl[i]=NULL;
					RStr->ActiveElP[i]=NULL;
					RStr->ActiveElN[i]=NULL;
					break;
				};
			};
			if (i==RStr->MaxPoi)
			{
				RStr->Fl_PoiFSL[i]=1;
				RStr->PoiFSL[i]=(size_t)FSL;		//?????
				FSL->ID_FSL=i;
				RStr->ActiveEl[i]=NULL;
				RStr->ActiveElP[i]=NULL;
				RStr->ActiveElN[i]=NULL;
				if (++RStr->MaxPoi>=254)
				{
					RStr->ActiveEl[i]=NULL;	// Только для отладки!!!
				};
			}
		}
	}
	return	FSL->ID_FSL;
};

bool	ID_List8::DelFSL(uchar	ID_SFL)
{
	bool	brc=TRUE;
	RStr->Fl_PoiFSL[ID_SFL]=0;
//	if (RStr->MaxPoi==ID_SFL+1)
//		RStr->MaxPoi--;
	return	brc;
};


int ID_List8::Compare(ID_ElemM*	tEl1,ID_ElemM*	tEl2)
{
	int	rc=0;
	size_t	tCnt=tEl1->Elem->Len;
	if (tCnt>tEl2->Elem->Len)
	{
		tCnt=tEl2->Elem->Len;
		rc=memcmp(tEl1->Elem->Buff,tEl2->Elem->Buff,tCnt);
		if (rc==0) rc=1;
	}
	else
	{
		rc=memcmp(tEl1->Elem->Buff,tEl2->Elem->Buff,tCnt);
		if (rc==0) rc=-1;
	}
	return rc;
};

int	ID_List8::Del(ID_Elem* IDE)
{
	int	rc=-1;
	if (Fl_Del==0)
	{
//		TestValid();
		rc=Remove(IDE);
//		TestValid();
//		WaitForSingleObject(hmtx,INFINITE);
//		if (++FlReEn>1)
//		{
//			CriErr=GetServices()->SetCriticalError(NULL,"ID_List8:Del;FlReEn","Error","ID_List8",CriErr);
//		}
//		ID_ElemM* tIDEM=DelI(IDE);
//		if (tIDEM)
//		{
//			rc=0;
//		}
//		FlReEn--;
//		ReleaseSemaphore(hmtx,1,NULL);
//
//		if (++FlReEn>1)
//		{
//			CriErr=GetServices()->SetCriticalError(NULL,"ID_List8:Del;FlReEn","Error","ID_List8",CriErr);
//		}
		if (rc!=-1)
		{
			if(Fl_DL==0)
			{
				delete IDE;
			}
			else
			{
				//							int	i;
				if (DelPoolEl==0)
				{
					delete IDE;
				}
				else
				{
					MyLock(__FILE__,__LINE__,&FlReEn,NULL);//strcpy(LastFunc,"Del");
					if (NumDelPoolEl>=SizeDPoolEl)
					{
						delete DelPoolEl[CntDelInsert];
						DelPoolEl[CntDelInsert]=IDE;
						if (++CntDelInsert>=SizeDPoolEl)
							CntDelInsert=0;
					}
					else
						DelPoolEl[NumDelPoolEl++]=IDE;
//					LastFunc[0]=0;
					MyUnLock(__FILE__,__LINE__,&FlReEn,NULL);//FlReEn=0;
				}
			};
		}
	}
	if ((BeginEl==NULL)&&(NumElem!=0))
	{
//_mmmm:
//		Beep(3000,100);
//		Sleep(1000);
//		goto	_mmmm;
	}
	return rc;
};

int	ID_List8::Remove(ID_Elem* IDE)
{
	int	rc=-1;
	if (Fl_Del==0)
	{
//		WaitForSingleObject(hmtx,INFINITE);
		MyLock(__FILE__,__LINE__,&FlReEn,NULL);
//		strcpy(LastFunc,"Remove");
//		if (++FlReEn>1)
//		{
//			CriErr=GetServices()->SetCriticalError(NULL,"ID_List8:Remove:FlReEn","Error","ID_List8",CriErr);
//		}
		ID_ElemM* tIDEM=RemoveI(IDE);

// 		if (ActiveNEl==tIDEM)
// 			ActiveNEl=NULL;
// 		if (ActivePEl==tIDEM)
// 			ActivePEl=NULL;
// 		if (ActiveEl==tIDEM)
// 		{
// 			ActiveNEl=NULL;
// 			ActivePEl=NULL;
// 		}
		if (tIDEM)
			rc=0;
//		LastFunc[0]=0;
		MyUnLock(__FILE__,__LINE__,&FlReEn,NULL);//FlReEn=0;
//		ReleaseSemaphore(hmtx,1,NULL);
	}
	return rc;
};

void ID_List8::DelAllElem()
{
	int	i;
//	ID_ElemM	IDe;
	ID_List8	*IDl;
//	WaitForSingleObject(hmtx,INFINITE);
	Fl_Del++;
	MyLock(__FILE__,__LINE__,&FlReEn,NULL);//strcpy(LastFunc,"DelAllElem");
//	if (++FlReEn>1)
//	{
//		CriErr=GetServices()->SetCriticalError(NULL,"ID_List8:DelAllElem:FlReEn","Error","ID_List8",CriErr);
//	}
	for (i=0;i<256;i++)
	{
		if (Lev[i]!=NULL)
		{
			IDl=Lev[i];
			Lev[i]=NULL;
			DelLev(IDl);
//			delete Lev[i];
		}
		if (ElemM[i].Elem!=NULL)
		{
			DelElem(&ElemM[i]);
			ElemM[i].Elem=NULL;
//			delete Elem[i];
//			Elem[i]=NULL;
		}
	};
	if (DelPool!=NULL)
	{
		for (i=0;i<NumDelPool;i++)
		{
			delete DelPool[i];
		}
		free(DelPool);DelPool=NULL;NumDelPool=0;
	}
	if (DelPoolEl!=NULL)
	{
		for (i=0;i<NumDelPoolEl;i++)
		{
			delete DelPoolEl[i];
		}
		free(DelPoolEl);DelPoolEl=NULL;NumDelPoolEl=0;
	}
	NumElem=0;
	tPElem=0;
	Level=0;
	FlBeg=0;FlEnd=0;
	/*ActiveEl=*/BeginEl=EndEl=NULL;
//	LastFunc[0]=0;
	MyUnLock(__FILE__,__LINE__,&FlReEn,NULL);//FlReEn=0;
	Fl_Del--;
//	ReleaseSemaphore(hmtx,1,NULL);
};

void ID_List8::RemoveAllPoiElem()
{
	int	i;
	ID_List8	*IDl;
	if (Fl_Del==0)
	{
//		WaitForSingleObject(hmtx,INFINITE);
		MyLock(__FILE__,__LINE__,&FlReEn,NULL);//strcpy(LastFunc,"RemoveAllPoiElem");
//		if (++FlReEn>1)
//		{
//			CriErr=GetServices()->SetCriticalError(NULL,"ID_List8:RemoveAllPoiElem:FlReEn","Error","ID_List8",CriErr);
//		}
		for (i=0;i<256;i++)
		{
			if (Lev[i]!=NULL)
			{
				Lev[i]->RemoveAllPoiElem();
				IDl=Lev[i];
				Lev[i]=NULL;
				DelRLev(IDl);
				//			delete Lev[i]; 
			}
			if (ElemM[i].Elem!=NULL)
			{
				DelLink(&ElemM[i]);
				ElemM[i].Elem=NULL;
			}
		};
		NumElem=0;
		tPElem=0;
		Level=0;
		FlBeg=0;FlEnd=0;
		/*ActiveEl=*/BeginEl=EndEl=NULL;
//		LastFunc[0]=0;
		MyUnLock(__FILE__,__LINE__,&FlReEn,NULL);//FlReEn=0;
//		ReleaseSemaphore(hmtx,1,NULL);
	}
};

// --------------------------

ID_Elem*	ID_List8::Begin(FindStrList8& FSL8)
{	
	ID_Elem*	tEl=NULL;

	if (Fl_Del==0)
	{
		MyLock(__FILE__,__LINE__,&FlReEn,NULL);//strcpy(LastFunc,"Begin");
//		if (FSL8.ID_FSL==255)
			AddFSL(&FSL8);
		if (RStr)
		{
			if ((RStr->ActiveEl[FSL8.ID_FSL]=BeginEl)!=NULL)
				tEl=RStr->ActiveEl[FSL8.ID_FSL]->Elem;
			else
				tEl=NULL;
			RStr->ActiveElN[FSL8.ID_FSL]=RStr->ActiveElP[FSL8.ID_FSL]=NULL;
		}
// 		FSL8.Active=BeginEl;
// 		if (BeginEl)
// 		{
// 			FSL8.Perv=BeginEl->Perviose;
// 			FSL8.Next=BeginEl->Next;
// 		}
// 		else
// 		{
// 			FSL8.Perv=NULL;
// 			FSL8.Next=NULL;
// 		}
		MyUnLock(__FILE__,__LINE__,&FlReEn,NULL);//FlReEn=0;
	}
	return tEl;
};

ID_Elem*	ID_List8::NewElem()	// ¦þûº¢øª¹ ºúð÷ðªõû¹ ýð ¤ûõüõýª ø÷ ¸ÿø¸úð ÿþôóþªþòûõýýv¿ ú ºôðûõýø¦
{
	ID_Elem*	tEl=NULL;
	if (Fl_Del==0)
	{
//		WaitForSingleObject(hmtx,INFINITE);
		MyLock(__FILE__,__LINE__,&FlReEn,NULL);
//		if (++FlReEn>1)
//		{
//			CriErr=GetServices()->SetCriticalError(NULL,"ID_List8:NewElem:FlReEn","Error","ID_List8",CriErr);
//		}
		if (PoiRoot->Fl_DL==0)
		{
			DelPool=(ID_List8**)malloc(SizeDPool*sizeof(ID_List8*));
			DelPoolEl=(ID_Elem**)malloc(SizeDPoolEl*sizeof(ID_Elem*));
		}
		PoiRoot->Fl_DL|=1;
		if (PoiRoot->NumDelPoolEl>0)
			tEl=PoiRoot->DelPoolEl[--PoiRoot->NumDelPoolEl];
//		LastFunc[0]=0;
//		ReleaseSemaphore(hmtx,1,NULL);
		if (tEl)
		{
			try
			{
				char	tCh;
				for (size_t i=0;i<tEl->Len;i++)
				{
					tCh=tEl->Buff[i];
				}
			}
			catch (...)
			{
				tEl=NULL;
			}
		}
		MyUnLock(__FILE__,__LINE__,&FlReEn,NULL);//FlReEn=0;
	}
	return tEl; 
};

ID_Elem*	ID_List8::End(FindStrList8& FSL8)
{	
	ID_Elem*	tEl=NULL;
	if (Fl_Del==0)
	{
//		WaitForSingleObject(hmtx,INFINITE);
		MyLock(__FILE__,__LINE__,&FlReEn,NULL);//strcpy(LastFunc,"End");
//		if (FSL8.ID_FSL==255)
			AddFSL(&FSL8);
		if (RStr)
		{
			if ((RStr->ActiveEl[FSL8.ID_FSL]=EndEl)!=NULL)
				tEl=RStr->ActiveEl[FSL8.ID_FSL]->Elem;
			else
				tEl=NULL;
			RStr->ActiveElN[FSL8.ID_FSL]=RStr->ActiveElP[FSL8.ID_FSL]=NULL;
		}

// 		FSL8.Active=EndEl;
// 		FSL8.Perv=EndEl->Perviose;
// 		FSL8.Next=EndEl->Next;
//  		if (FSL8.Active)
//  			tEl=FSL8.Active->Elem;
//  		else
//  			tEl=NULL;
		MyUnLock(__FILE__,__LINE__,&FlReEn,NULL);//FlReEn=0;
//		ReleaseSemaphore(hmtx,1,NULL);
		
	}
	return tEl;
};
/*
ID_Elem*	ID_List8::GetActive()
{
	ID_Elem*	tEl=NULL;
	if (Fl_Del==0)
	{
//		WaitForSingleObject(hmtx,INFINITE);
		MyLock(__FILE__,__LINE__,&FlReEn,NULL);//strcpy(LastFunc,"GetActive");
//		if (++FlReEn>1)
//		{
//			CriErr=GetServices()->SetCriticalError(NULL,"ID_List8:GetActive:FlReEn","Error","ID_List8",CriErr);
//		}
// 		if (ActiveEl)
// 			tEl=ActiveEl->Elem;
// 		else
// 			tEl=NULL;
//		LastFunc[0]=0;
		MyUnLock(__FILE__,__LINE__,&FlReEn,NULL);//FlReEn=0;
//		ReleaseSemaphore(hmtx,1,NULL);
		
	}
	return tEl;
};
*/

ID_ElemM*	ID_List8::PervioseI()
{
	ID_ElemM*	tPoi=NULL;
	if ((tPElem>256)||(tPElem<0)) 
	{
		tPElem=256;
	}
_Pre00:
	if (tPElem>=0)
	{
		if (ElemM[tPElem].Elem!=NULL)
		{
			tPElem--;
			for (tPElem;tPElem>=0;--tPElem)
			{
				if (ElemM[tPElem].Elem!=NULL)
				{
					tPoi=&ElemM[tPElem];
					FlBeg=1;
					break;
				}
				if (Lev[tPElem]!=NULL)
				{
					Lev[tPElem]->tPElem=256;
					tPoi=Lev[tPElem]->PervioseI();
					if (tPoi!=NULL)
						break;
				}
			}
		}
		else
		{
			if (Lev[tPElem]!=NULL)
			{
				tPoi=Lev[tPElem]->PervioseI();
				if (tPoi==NULL)
				{
					tPElem--;
					if  (tPElem>=0)
					{
						if(Lev[tPElem])
						{
							Lev[tPElem]->tPElem=256;
							goto _Pre00;
						}
						else
						{
							if (ElemM[tPElem].Elem!=NULL)
							{
								tPoi=&ElemM[tPElem];
								FlBeg=1;
							}
							else
								goto _Pre00;
						};
					}
					else
						tPElem=0;
				}
			}
			else
			{
				tPElem--;
				for (tPElem;tPElem>=0;--tPElem)
				{
					if (ElemM[tPElem].Elem!=NULL)
					{
						tPoi=&ElemM[tPElem];
						FlBeg=1;
						break;
					}
					if (Lev[tPElem]!=NULL)
					{
						Lev[tPElem]->tPElem=256;
						tPoi=Lev[tPElem]->PervioseI();
						if (tPoi!=NULL)
							break;
					}
				}
			}
		};
	}
	else
	{
		tPElem=0;
	}
	return tPoi;
};

ID_ElemM*	ID_List8::NextI()
{
	ID_ElemM*	tPoi=NULL;
	ID_Elem*	elem;
	if (tPElem<0)
		tPElem=0;
	if (tPElem>255) 
	{
		tPElem=0;
		return 0;
	}
	if (FlBeg==0)
	{
		if (ElemM[tPElem].Elem!=0)
		{
			FlBeg=1;
			return &ElemM[tPElem];
		}
	};
	if (Lev[tPElem]!=0)
	{
		tPoi=Lev[tPElem]->NextI();
		if (tPoi!=0)
		{
			return tPoi;
		}
		tPElem++;
	}
	for (tPElem=tPElem;tPElem<256;tPElem++)
	{
		elem=ElemM[tPElem].Elem;
		if (FlBeg==0)
		{
			if (elem!=0)
			{
				FlBeg=1;
				return &ElemM[tPElem];
			}
		};
		if (Lev[tPElem]!=0)
		{
			FlBeg=0;
			Lev[tPElem]->tPElem=0;
			tPoi=Lev[tPElem]->NextI();
			if (tPoi!=0)
			{
				return tPoi;
			}
		}
		else
		{
			if (FlBeg==0)
				if (elem!=0)
				{
					FlBeg=0;
					tPElem++;
					return &ElemM[tPElem];
				};
		}
		FlBeg=0;
	}
	return tPoi;
};


ID_Elem*	ID_List8::Perviose(FindStrList8& FSL8)
{
	ID_Elem*	tEl=NULL;
	if (Fl_Del==0)
	{
//		WaitForSingleObject(hmtx,INFINITE);
		MyLock(__FILE__,__LINE__,&FlReEn,NULL);//strcpy(LastFunc,"Perviose");
//		if (FSL8.ID_FSL==255)
			AddFSL(&FSL8);
		if (RStr)
		{
			if (RStr->ActiveEl[FSL8.ID_FSL])
			{
				RStr->ActiveEl[FSL8.ID_FSL]=RStr->ActiveEl[FSL8.ID_FSL]->Perviose;
				if (RStr->ActiveEl[FSL8.ID_FSL])
					tEl=RStr->ActiveEl[FSL8.ID_FSL]->Elem;
				else
					tEl=NULL;
			}
			else
			{
				if (RStr->ActiveElP[FSL8.ID_FSL])
				{
					RStr->ActiveEl[FSL8.ID_FSL]=RStr->ActiveElP[FSL8.ID_FSL];
					tEl=RStr->ActiveEl[FSL8.ID_FSL]->Elem;
//					RStr->ActiveElN[FSL8.ID_FSL]=RStr->ActiveElP[FSL8.ID_FSL]=NULL;
				}
				else
					tEl=NULL;
			}
			RStr->ActiveElN[FSL8.ID_FSL]=RStr->ActiveElP[FSL8.ID_FSL]=NULL;
		}
// 		if (FSL8.Active)
// 		{
// 			if	(FSL8.Active->Elem)
// 			{
// 				if (FSL8.Active->Perviose)
// 				{
// 					FSL8.Active=FSL8.Active->Perviose;
// 				}
// 				else
// 				{
// 					FSL8.Next=FSL8.Active;
// 					FSL8.Active=NULL;
// 				}
// 			}
// 			else
// 			{
// 				if (FSL8.Perv)
// 					FSL8.Active=FSL8.Perv;
// 				else
// 				{
// 					FSL8.Active=PervioseI();
// 				}
// 			}
// 		}
// 		else
// 		{
// 			FSL8.Active=PervioseI();
// 		}
// 		if (FSL8.Active)
// 		{
// 			FSL8.Perv=FSL8.Active->Perviose;
// 			FSL8.Next=FSL8.Active->Next;
// 			tEl=FSL8.Active->Elem;
// 		}
// 		else
// 		{
// 			tEl=NULL;
// 		}

// 		if (ActiveEl)
// 		{
// 			ActiveEl=ActiveEl->Perviose;
// 			if (ActiveEl)
// 				tEl=ActiveEl->Elem;
// 			else
// 				tEl=NULL;
// 		}
// 		else
// 		{
// 			if (ActivePEl)
// 			{
// 				ActiveEl=ActivePEl;
// 				if (ActiveEl)
// 					tEl=ActiveEl->Elem;
// 				else
// 					tEl=NULL;
// 				ActivePEl=0;
// 			}
// 			else
// 			{
// 				ActiveEl=PoiRoot->PervioseI();
// 				if (ActiveEl)
// 					tEl=ActiveEl->Elem;
// 				else
// 					tEl=NULL;
// 			}
// 		}
// 		PoiRoot->ActiveNEl=NULL;
//		LastFunc[0]=0;
		MyUnLock(__FILE__,__LINE__,&FlReEn,NULL);//FlReEn=0;
//		ReleaseSemaphore(hmtx,1,NULL);
		
	}
	return tEl;
};
/*
ID_Elem*	ID_List8::Next(ID_Elem* tElem)
{
//	if (!IsBadReadPtr((void*)tElem,sizeof(IDInfo)))
	bool	rc=FALSE;
	try
	{
		rc=!IsBadReadPtr(tElem->Buff,tElem->Len);
	}
	catch(...)
	{
		
	};
	if (rc)
	{
 		Find(tElem->Buff,tElem->Len,1);
	}
	return Next();
}
*/
ID_Elem*	ID_List8::Next(FindStrList8& FSL8)
{
	ID_Elem*	tEl=NULL;
	if (Fl_Del==0)
	{
//		WaitForSingleObject(hmtx,INFINITE);
		MyLock(__FILE__,__LINE__,&FlReEn,NULL);//strcpy(LastFunc,"Next");
//		if (FSL8.ID_FSL==255)
			AddFSL(&FSL8);
		if (RStr)
		{
			if (RStr->ActiveEl[FSL8.ID_FSL])
			{
				RStr->ActiveEl[FSL8.ID_FSL]=RStr->ActiveEl[FSL8.ID_FSL]->Next;
				if (RStr->ActiveEl[FSL8.ID_FSL])
					tEl=RStr->ActiveEl[FSL8.ID_FSL]->Elem;
				else
					tEl=NULL;
			}
			else
			{
				if (RStr->ActiveElN[FSL8.ID_FSL])
				{
					RStr->ActiveEl[FSL8.ID_FSL]=RStr->ActiveElN[FSL8.ID_FSL];
					tEl=RStr->ActiveEl[FSL8.ID_FSL]->Elem;
//					RStr->ActiveElN[FSL8.ID_FSL]=RStr->ActiveElP[FSL8.ID_FSL]=NULL;
				}
				else
					tEl=NULL;
			}
			RStr->ActiveElN[FSL8.ID_FSL]=RStr->ActiveElP[FSL8.ID_FSL]=NULL;
		}

// 		if (FSL8.Active)
// 		{
// 			if (IsBadReadPtr(FSL8.Active,sizeof(ID_ElemM)))
// 			{
// 				if (IsBadReadPtr(FSL8.Next,sizeof(ID_ElemM)))
// 				{
// 					FSL8.Active=NULL;
// 				}
// 				else
// 				{
// 					FSL8.Active=FSL8.Next;
// 				}
// 			}
// 			else
// 			{
// 				if (FSL8.Next)
// 				{
// 					if (IsBadReadPtr(FSL8.Next,sizeof(ID_ElemM)))
// 					{
// 						if ((FSL8.Active->Next)&&(FSL8.Active==FSL8.Active->Next->Perviose))
// 						{
// 							FSL8.Active=FSL8.Active->Next;
// 						}
// 						else
// 						{
// 							ID_ElemM	*tEl=FSL8.Active;
// 							FSL8.Active==NextI();
// 							if (tEl==FSL8.Active)
// 								FSL8.Active=FSL8.Active->Next;
// 						}
// 					}
// 					else
// 					{
// 						if (FSL8.Active->Next==FSL8.Next)
// 						{
// 							FSL8.Active=FSL8.Next;
// 						}
// 						else
// 						{
// 							if (FSL8.Next->Perviose==FSL8.Perv)
// 							{
// 								FSL8.Active=FSL8.Next;
// 							}
// 							else
// 							{
// 								if ((FSL8.Active->Next)&&(FSL8.Active==FSL8.Active->Next->Perviose))
// 								{
// 									FSL8.Active=FSL8.Active->Next;
// 								}
// 								else
// 								{
// 									ID_ElemM	*tEl=FSL8.Active;
// 									FSL8.Active==NextI();
// 									if (tEl==FSL8.Active)
// 										FSL8.Active=FSL8.Active->Next;
// 								}
// 							}
// 						}
// 					}
// 				}
// 				else
// 				{
// 					FSL8.Next=FSL8.Perv=FSL8.Active=NULL;
// 				}
// 			}
// 		}
// 		else
// 		{
// 			FSL8.Active=NextI();
// 		}
// 		if (FSL8.Active)
// 		{
// 			FSL8.Perv=FSL8.Active->Perviose;
// 			FSL8.Next=FSL8.Active->Next;
// 			tEl=FSL8.Active->Elem;
// 		}
// 		else
// 		{
// 			tEl=NULL;
// 		}

// 		if (ActiveEl)
// 		{
// 			ActiveNEl=NULL;
// 			ActiveEl=ActiveEl->Next;
// 			if (ActiveEl)
// 				tEl=ActiveEl->Elem;
// 			else
// 				tEl=NULL;
// 		}
// 		else
// 		{
// 			if (ActiveNEl)
// 			{
// 				ActiveEl=ActiveNEl;
// 				if (ActiveEl)
// 					tEl=ActiveEl->Elem;
// 				else
// 					tEl=NULL;
// 				ActiveNEl=0;
// 			}
// 			else
// 			{
// 				ActiveEl=PoiRoot->NextI();
// 				if (ActiveEl)
// 					tEl=ActiveEl->Elem;
// 				else
// 					tEl=NULL;
// 			}
// 		}
// 		PoiRoot->ActivePEl=NULL;
//		LastFunc[0]=0;
		MyUnLock(__FILE__,__LINE__,&FlReEn,NULL);//FlReEn=0;
//		ReleaseSemaphore(hmtx,1,NULL);
	}
	return tEl;
};

int			ID_List8::CalcObjSize()
{
	int	i;
	int	Size=0;
	if (Fl_Del==0)
	{
//		WaitForSingleObject(hmtx,INFINITE);
		MyLock(__FILE__,__LINE__,&FlReEn,NULL);//strcpy(LastFunc,"CalcObjSize");
//		if (++FlReEn>1)
//			CriErr=GetServices()->SetCriticalError(NULL,"ID_List8:CalcObjSize:FlReEn","Error","ID_List8",CriErr);
		Size=sizeof(ID_List8);	// ªøÿð   ¸ðü ;)
		for (i=0;i<256;i++)
			if (Lev[i]!=0)
			{
				Size+=Lev[i]->CalcObjSize();
			};
//		LastFunc[0]=0;
		MyUnLock(__FILE__,__LINE__,&FlReEn,NULL);//FlReEn=0;
//		ReleaseSemaphore(hmtx,1,NULL);
		
	}
	return Size;
};

ID_ElemM*	ID_List8::AddI(ID_Elem* IDE)
{
	ID_ElemM*	RetElemM=0;
	ID_ElemM*	TElemM=0;
	if (IDE)
	{
		tPElem=IDE->Buff[Level];
		if(Lev[tPElem]==0)
		{
			if (ElemM[tPElem].Elem==NULL)
			{
				RetElemM=&ElemM[tPElem];
				ElemM[tPElem].Elem=IDE;
			}
			else
			{
				if (ElemM[tPElem].Elem->Len>Level+1)
				{
					if (tPElem==256)
						CriErr=GetServices()->SetCriticalError(NULL,"ID_List8:tPElem=256","Error","ID_List8",CriErr);
					Lev[tPElem]=NewLev(Level+1);
//					DelLink(ElemM[tPElem]);
					if (Lev[tPElem]) 
						TElemM=Lev[tPElem]->AddI(ElemM[tPElem].Elem);
					if(TElemM)
					{
						if (ElemM[tPElem].Next)
						{
							TElemM->Next=ElemM[tPElem].Next;
							ElemM[tPElem].Next->Perviose=TElemM;
						}
						else
						{
							TElemM->Next=NULL;
							PoiRoot->EndEl=TElemM;
						}
						if (ElemM[tPElem].Perviose)
						{
							TElemM->Perviose=ElemM[tPElem].Perviose;
							ElemM[tPElem].Perviose->Next=TElemM;
						}
						else
						{
							TElemM->Perviose=NULL;
							PoiRoot->BeginEl=TElemM;
						}
// 						if (PoiRoot->ActiveEl==&ElemM[tPElem])
// 							PoiRoot->ActiveEl=TElemM;
						ElemM[tPElem].Elem=NULL;
						if (IDE->Len<=Level+1)
						{
							RetElemM=&ElemM[tPElem];
							ElemM[tPElem].Elem=IDE;		// +¸ªðõª¸  ÷ôõ¸¹
						}
						else
							RetElemM=Lev[tPElem]->AddI(IDE); // ¦õ¨õüõ•ðõü ôðû¹°õ
					}
					else
					{
						DelElem(&ElemM[tPElem]);
					}
				}
				else
				{
					if (IDE->Len<=Level+1)	
						RetElemM=NULL;				// ¦þûýþõ ¸þòÿðôõýøõ
					else
					{
						if (tPElem==256)
							CriErr=GetServices()->SetCriticalError(NULL,"ID_List8:tPElem=256","Error","ID_List8",CriErr);
						Lev[tPElem]=NewLev(Level+1);
						//					Lev[tPElem]=new ID_List8(Level+1,PoiRoot);
						RetElemM=Lev[tPElem]->AddI(IDE);
					};
				};
			}
		}
		else
		{
			if (IDE->Len<=Level+1)
			{	// !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
				if (ElemM[tPElem].Elem==0)
				{
					RetElemM=&ElemM[tPElem];
					ElemM[tPElem].Elem=IDE;
				}
				else
				{
					if (ElemM[tPElem].Elem->Len<=Level+1)
						RetElemM=NULL;	// 
					else
					{
//						DelLink(ElemM[tPElem]);
						TElemM=Lev[tPElem]->AddI(ElemM[tPElem].Elem);
						if(TElemM)
						{
							if (ElemM[tPElem].Next)
							{
								TElemM->Next=ElemM[tPElem].Next;
								ElemM[tPElem].Next->Perviose=TElemM;
							}
							else
							{
								TElemM->Next=NULL;
								PoiRoot->EndEl=TElemM;
							}
							if (ElemM[tPElem].Perviose)
							{
								TElemM->Perviose=ElemM[tPElem].Perviose;
								ElemM[tPElem].Perviose->Next=TElemM;
							}
							else
							{
								TElemM->Perviose=NULL;
								PoiRoot->BeginEl=TElemM;
							}
// 							if (PoiRoot->ActiveEl==&ElemM[tPElem])
// 								PoiRoot->ActiveEl=TElemM;
							RetElemM=&ElemM[tPElem];
							ElemM[tPElem].Elem=IDE;
						}
						else
						{
							DelElem(&ElemM[tPElem]);
						}
					}
				}
			}
			else
			{
				RetElemM=Lev[tPElem]->AddI(IDE);
			}
		};
	}
	else
	{
		RetElemM=NULL;
	}
	if (RetElemM!=NULL)
	{
		NumElem++;
//		if (NumElem==8)
//			{size_t iasm=1;}
		if (Level==0)
		{
			RetElemM->Perviose=RetElemM->Next=NULL;
_nmn1:		ID_ElemM*	tEl=PervioseI();
			if (tEl==RetElemM)
				tEl=PervioseI();
			if ((tEl)&&(tEl->Elem))
			{
				RetElemM->Perviose=tEl;
				RetElemM->Next=tEl->Next;
				if (tEl->Next)
					tEl->Next->Perviose=RetElemM;
				else
					EndEl=RetElemM;
				tEl->Next=RetElemM;
			}
			else
			{
				tEl=NextI();
				if (tEl==RetElemM)
					tEl=NextI();
				if (tEl)
				{
					RetElemM->Next=tEl;
					RetElemM->Perviose=tEl->Perviose;
					if (tEl->Perviose)
						tEl->Perviose->Next=RetElemM;
					else
						BeginEl=RetElemM;
					tEl->Perviose=RetElemM;
				}
				else
				{
					if (NumElem>1)
					{
						FindI(RetElemM->Elem->Buff,RetElemM->Elem->Len);	// ,1
						goto	_nmn1;
					}
					BeginEl=RetElemM;
					EndEl=RetElemM;
				}
			}
			//			if (NumElem==8)
			//				{size_t iasm=1;}
		}

/*
		if (Level==0)
		{
			ID_ElemM*	tEl1;
			ID_ElemM*	tEl2;
			int	rc;
			RetElemM->Perviose=RetElemM->Next=NULL;
			tEl1=PervioseI();
			if (tEl1==RetElemM)
				tEl1=PervioseI();
_nn01:		if (tEl1)
			{
				tEl2=tEl1->Next;
				if (tEl2)
				{
					rc=Compare(tEl1,RetElemM);
					if (rc>0)
					{
						while (TRUE)
						{
							Beep(2000,100);
							Sleep(1000);
						}
					}
					rc=Compare(tEl2,RetElemM);
					if (rc<0)
					{
						tEl1=tEl2;
						tEl2=tEl2->Next;
						goto	_nn01;
					}
					if (tEl1->Next==tEl2)
					{
						RetElemM->Perviose=tEl1;
						RetElemM->Next=tEl1->Next;
						if (tEl1->Next)
							tEl1->Next->Perviose=RetElemM;
						else
							EndEl=RetElemM;
						tEl1->Next=RetElemM;
					}
					else
					{
						while (TRUE)
						{
							Beep(2000,100);
							Sleep(1000);
						}
					};
				}
				else
				{
					if (tEl1->Next==0)
					{
						RetElemM->Perviose=tEl1;
						RetElemM->Next=tEl1->Next;
						if (tEl1->Next)
							tEl1->Next->Perviose=RetElemM;
						else
							EndEl=RetElemM;
						tEl1->Next=RetElemM;
					}
					else
					{
						while (TRUE)
						{
							Beep(2000,100);
							Sleep(1000);
						}
					}
				}
			}
			else
			{
				tEl2=BeginEl;
				if (tEl2==RetElemM)
					tEl2=tEl2->Next;
				if (tEl2)
				{
					rc=Compare(RetElemM,tEl2);
					if (rc<0)
					{
						if (tEl2->Perviose==0)
						{
							RetElemM->Next=tEl2;
							RetElemM->Perviose=tEl2->Perviose;
							if (tEl2->Perviose)
								tEl2->Perviose->Next=RetElemM;
							else
								BeginEl=RetElemM;
							tEl2->Perviose=RetElemM;
						}
						else
						{
							while (TRUE)
							{
								Beep(2000,100);
								Sleep(1000);
							}
						}

					}
					else
					{
						tEl1=tEl2;
						tEl2=tEl2->Next;
						goto	_nn01;
					}
				}
				else
				{
					if (NumElem>1)
					{
						while (TRUE)
						{
							Beep(2000,100);
							Sleep(1000);
						}
					}
					BeginEl=RetElemM;
					EndEl=RetElemM;
				}
			}
*/
//			rc=Compare(tEl,RetElemM);
/*			if (tEl)
			{
				if (tEl==RetElemM)
				{
					tEl=PervioseI();
					if (tEl)
					{
						RetElemM->Next=tEl;
						RetElemM->Perviose=tEl->Perviose;
						if (tEl->Perviose)
							tEl->Perviose->Next=RetElemM;
						else
							BeginEl=RetElemM;
						tEl->Perviose=RetElemM;
					}
					else
					{
_resh1:
						tEl=NextI();
						if ((tEl)&&(tEl->Elem))
						{
							RetElemM->Perviose=tEl;
							RetElemM->Next=tEl->Next;
							if (tEl->Next)
								tEl->Next->Perviose=RetElemM;
							else
								EndEl=RetElemM;
							tEl->Next=RetElemM;
						}
						else
						{
							if (NumElem>1)
							{
								tEl=PervioseI();
								while ((tEl)&&(tEl!=RetElemM))
								{
									tEl=PervioseI();
								}
								goto _resh1;
							}
							BeginEl=RetElemM;
							EndEl=RetElemM;
						};
					};
				}
				else
				{
					RetElemM->Next=tEl;
					RetElemM->Perviose=tEl->Perviose;
					if (tEl->Perviose)
						tEl->Perviose->Next=RetElemM;
					else
						BeginEl=RetElemM;
					tEl->Perviose=RetElemM;
				};
			}
			else
			{
				tEl=NextI();
				if (tEl)
				{
					if (tEl==RetElemM)
					{
						tEl=NextI();
					};
					if ((tEl)&&(tEl->Elem))
					{
						RetElemM->Perviose=tEl;
						RetElemM->Next=tEl->Next;
						if (tEl->Next)
							tEl->Next->Perviose=RetElemM;
						else
							EndEl=RetElemM;
						tEl->Next=RetElemM;
					}
					else
					{
						if (NumElem>1)
						{
							tEl=PervioseI();
							while ((tEl)&&(tEl!=RetElemM))
							{
								tEl=PervioseI();
							}
							goto _resh1;
						}
						BeginEl=RetElemM;
						EndEl=RetElemM;
					};
				}
				else
				{
					if (NumElem>1)
					{
						tEl=PervioseI();
						while ((tEl)&&(tEl!=RetElemM))
						{
							tEl=PervioseI();
						}
						goto _resh1;
					}
					else
					{
						BeginEl=RetElemM;
						EndEl=RetElemM;
					}


				}
			};
*/
///////////////////////////////////////////////////////
/*
			if ((tEl)&&(tEl->Elem))
			{
				RetElemM->Perviose=tEl;
				RetElemM->Next=tEl->Next;
				if (tEl->Next)
					tEl->Next->Perviose=RetElemM;
				else
					EndEl=RetElemM;
				tEl->Next=RetElemM;
				TestValid();
			}
			else
			{
				if (NumElem>1)
				{
					tEl=NextI();
					if (tEl==NULL)
					{
						tEl=PervioseI();
						tEl=NextI();
					}
				}
				else
					tEl=NextI();
				if (tEl==RetElemM)
				{
					tEl=NextI();
					if (tEl==NULL)
					{
						tEl=PervioseI();
					}
				}
				if (tEl)
				{
					RetElemM->Next=tEl;
					RetElemM->Perviose=tEl->Perviose;
					if (tEl->Perviose)
						tEl->Perviose->Next=RetElemM;
					else
						BeginEl=RetElemM;
					tEl->Perviose=RetElemM;
				}
				else
				{
					if (NumElem>1)
					{
						{size_t iasm=1;}
					}
					BeginEl=RetElemM;
					EndEl=RetElemM;
				}
				TestValid();
			}
//			if (NumElem==8)
//				{size_t iasm=1;}
		}
*/
//		PoiRoot->ActiveNEl=PoiRoot->ActivePEl=NULL;
	}
	return RetElemM;
};

ID_ElemM*	ID_List8::DelI(ID_Elem* IDE)
{
	ID_ElemM*	RetElemM=NULL;
//	int	rc=0;
//	ID_Elem		*IDe;
	ID_List8	*IDl;
	if (IDE)
	{
		UCHAR	TempPtr=IDE->Buff[Level];
		if(Lev[TempPtr]==0)
		{
			if (ElemM[TempPtr].Elem==IDE)
			{	// ¤ªþ þý !!!
				RetElemM=&ElemM[TempPtr];
				DelElem(&ElemM[TempPtr]);
				ElemM[TempPtr].Elem=0;
			}
			else
			{
				RetElemM=NULL;
			}
		}
		else
		{
			if (IDE->Len<=Level+1)
				RetElemM=NULL;	// =ðùªø ýõòþ÷üþöýþ
			else
			{
				if ((RetElemM=Lev[TempPtr]->DelI(IDE))==0)
				{
					IDl=Lev[TempPtr];
					Lev[TempPtr]=0;
					DelLev(IDl);
				};
			};
		}
		if (RetElemM!=NULL) 
			--NumElem;
// 		else
// 		{
// 			{size_t iasm=1;}
// 		}
	}
	return RetElemM;
};

ID_List8*	ID_List8::NewLev(int	Lev)
{
	ID_List8*	pLev;
	if (PoiRoot->NumDelPool!=0)
	{
		pLev=PoiRoot->DelPool[--PoiRoot->NumDelPool];
		pLev->Init(Lev,PoiRoot);
	}
	else
	{
		pLev=new ID_List8(Lev,PoiRoot);
	}
	return	pLev;
};

void		ID_List8::DelLev(ID_List8* pLev)
{
	int	i;
//	ID_ElemM	*IDe;
	ID_List8	*IDl;
	for (i=0;i<256;i++)
	{
		if (pLev->Lev[i]!=NULL)
		{
			IDl=pLev->Lev[i];
			pLev->Lev[i]=NULL;
			DelLev(IDl);
		}
		if (pLev->ElemM[i].Elem!=NULL)
		{
//			IDe=&pLev->ElemM[i];
			DelElem(&pLev->ElemM[i]);
			pLev->ElemM[i].Elem=NULL;
		}
	}
	if (PoiRoot->DelPool==0)
	{
		delete pLev;
	}
	else
	{
		if (PoiRoot->NumDelPool>=SizeDPool)
			delete pLev;
		else
			PoiRoot->DelPool[PoiRoot->NumDelPool++]=pLev;
	}
};

void		ID_List8::DelRLev(ID_List8* pLev)
{
	int	i;
	ID_List8	*IDe;
	for (i=0;i<256;i++)
	{
		if (pLev->Lev[i]!=NULL)
		{
			IDe=pLev->Lev[i];
			pLev->Lev[i]=NULL;
			DelRLev(IDe); 
		}
	}
	if (PoiRoot->DelPool==0)
	{
			delete pLev;
	}
	else
	{
		if (PoiRoot->NumDelPool>=SizeDPool)
			delete pLev;
		else
			PoiRoot->DelPool[PoiRoot->NumDelPool++]=pLev;
	}
};

ID_ElemM*	ID_List8::RemoveI(ID_Elem* IDE)
{
	ID_ElemM*	RetElemM=NULL;
	ID_List8	*IDl;
	UCHAR	TempPtr=IDE->Buff[Level];
	if(Lev[TempPtr]==0)
	{
		if (ElemM[TempPtr].Elem==IDE)
		{	// ¤ªþ þý !!!
			RetElemM=&ElemM[TempPtr];
			DelLink(RetElemM);
			ElemM[TempPtr].Elem=0;
		}
		else
		{
			RetElemM=NULL;
		}
	} 
	else
	{
		if (IDE->Len<=Level+1)
		{
			RetElemM=NULL;	// =ðùªø ýõòþ÷üþöýþ
		}
		else
		{
			RetElemM=Lev[TempPtr]->RemoveI(IDE);
			if (Lev[TempPtr]->NumElem==0)
			{
				IDl=Lev[TempPtr];
				Lev[TempPtr]=0;
				DelRLev(IDl);
			};
		};
	}
	if (RetElemM!=NULL) 
		--NumElem;
// 	else
// 		{size_t iasm=1;}
	return RetElemM;
};

//-------------------------------------------------------------------------------------------------
ID_Elem*	ID_List8::SearchSIGN(UCHAR* Buff,size_t Cnt)
{
	ID_ElemM*	tID_El=NULL;
	if (Fl_Del==0)
	{
//		WaitForSingleObject(hmtx,INFINITE);
		MyLock(__FILE__,__LINE__,&FlReEn,NULL);//strcpy(LastFunc,"SearchSIGN");
//		if (++FlReEn>1)
//			CriErr=GetServices()->SetCriticalError(NULL,"ID_List8:SearchSIGN:FlReEn","Error","ID_List8",CriErr);
		tID_El=SearchSIGNI(Buff,Cnt);
//		LastFunc[0]=0;
		MyUnLock(__FILE__,__LINE__,&FlReEn,NULL);//FlReEn=0;
//		ReleaseSemaphore(hmtx,1,NULL);
	}
	if (tID_El)
		return	tID_El->Elem;
	else
		return	NULL;
}

ID_ElemM* ID_List8::SearchSIGNI(UCHAR* Buff,size_t Cnt)
{
	UCHAR	TempPtr=Buff[Level];
	ID_List8*	lev=Lev[TempPtr];
	ID_ElemM*	elemM=&ElemM[TempPtr];
	ID_Elem*	elem=ElemM[TempPtr].Elem;
	if(lev==0)
	{
		if(elem!=NULL)
		{
			if(memcmp(elem->Buff,Buff,elem->Len)==0)
			{	
				return elemM;
			}
		}
	}
	else
	{
		if(elem!=NULL)
		{
			if(memcmp(elem->Buff,Buff,elem->Len)==0)
			{
				return elemM;
			}
		}
		if (Cnt<=1+Level)
		{
			return 0;	// =ðùªø ýõòþ÷üþöýþ
		}
		return lev->SearchSIGNI(Buff,Cnt);
	}
	return 0;
}
//-------------------------------------------------------------------------------------------------
ID_Elem*	ID_List8::FindNext(UCHAR* Buff,size_t Cnt,FindStrList8	&FSL8)	// Cond!=0 ªþ ÿõ¨õÿ¨ø¸òðøòðõª¸  ºúð÷ðªõû¹ ÿþø¸úð 
{
	ID_ElemM*	tID_El=NULL;
	if (Fl_Del==0)
	{
//		WaitForSingleObject(hmtx,INFINITE);
		MyLock(__FILE__,__LINE__,&FlReEn,NULL);//strcpy(LastFunc,"FindNext");
//		if (++FlReEn>1)
//			CriErr=GetServices()->SetCriticalError(NULL,"ID_List8:FindNext:FlReEn","Error","ID_List8",CriErr);
//		if (FSL8.ID_FSL==255)
			AddFSL(&FSL8);
		tID_El=FindI(Buff,Cnt,FSL8);
		if (tID_El==NULL)
			tID_El=NextI();
		if (RStr)
		{
			if (tID_El)
			{
				RStr->ActiveEl[FSL8.ID_FSL]=tID_El;
			}
			else
			{
				RStr->ActiveElP[FSL8.ID_FSL]=PervioseI();
			}
		}
		
// 		FSL8.Active=tID_El;
// 		if (FSL8.Active)
// 		{
// 			FSL8.Perv=FSL8.Active->Perviose;
// 			FSL8.Next=FSL8.Active->Next;
// 		}

//		ActiveEl=tID_El;
//		LastFunc[0]=0;
		MyUnLock(__FILE__,__LINE__,&FlReEn,NULL);//FlReEn=0;
//		ReleaseSemaphore(hmtx,1,NULL);
	}
	if (tID_El)
		return	tID_El->Elem;
	else
		return	NULL;
};

ID_Elem*	ID_List8::FindPerviose(UCHAR* Buff,size_t Cnt,FindStrList8	&FSL8)	// Cond!=0 ªþ ÿõ¨õÿ¨ø¸òðøòðõª¸  ºúð÷ðªõû¹ ÿþø¸úð 
{
	ID_ElemM*	tID_El=NULL;
	if (Fl_Del==0)
	{
//		WaitForSingleObject(hmtx,INFINITE);
		MyLock(__FILE__,__LINE__,&FlReEn,NULL);//strcpy(LastFunc,"FindPerviose");
//		if (++FlReEn>1)
//			CriErr=GetServices()->SetCriticalError(NULL,"ID_List8:FindPerviose:FlReEn","Error","ID_List8",CriErr);
//		if (FSL8.ID_FSL==255)
			AddFSL(&FSL8);
		tID_El=FindI(Buff,Cnt);
		if (tID_El==NULL)
			tID_El=PervioseI();
		if (RStr)
		{
			if (tID_El)
			{
				RStr->ActiveEl[FSL8.ID_FSL]=tID_El;
			}
			else
			{
				RStr->ActiveElN[FSL8.ID_FSL]=NextI();
			}
		}
// 		FSL8.Active=tID_El;
// 		if (FSL8.Active)
// 		{
// 			FSL8.Perv=FSL8.Active->Perviose;
// 			FSL8.Next=FSL8.Active->Next;
// 		}
//		ActiveEl=tID_El;
//		LastFunc[0]=0;
		MyUnLock(__FILE__,__LINE__,&FlReEn,NULL);//FlReEn=0;
//		ReleaseSemaphore(hmtx,1,NULL);
	}
	if (tID_El)
		return	tID_El->Elem;
	else
		return	NULL;
};

//-------------------------------------------------------------------------------------------------
ID_Elem*	ID_List8::Find(UCHAR* Buff,size_t Cnt,FindStrList8	&FSL8)
{
	ID_ElemM*	tID_El=NULL;
	if (Fl_Del==0)
	{
		//		WaitForSingleObject(hmtx,INFINITE);
		MyLock(__FILE__,__LINE__,&FlReEn,NULL);//strcpy(LastFunc,"Find");
		//		if (++FlReEn>1)
		//			CriErr=GetServices()->SetCriticalError(NULL,"ID_List8:FindPerviose:FlReEn","Error","ID_List8",CriErr);
//		if (FSL8.ID_FSL==255)
			AddFSL(&FSL8);
		tID_El=FindI(Buff,Cnt,FSL8);
		if (RStr)
		{
			if (tID_El)
			{
				RStr->ActiveEl[FSL8.ID_FSL]=tID_El;
			}
			else
			{
				RStr->ActiveElN[FSL8.ID_FSL]=NextI();
				if (RStr->ActiveElN[FSL8.ID_FSL])
					RStr->ActiveElP[FSL8.ID_FSL]=RStr->ActiveElN[FSL8.ID_FSL]->Perviose;
				else
					RStr->ActiveElP[FSL8.ID_FSL]=PervioseI();
			}
		}
		//		LastFunc[0]=0;
		MyUnLock(__FILE__,__LINE__,&FlReEn,NULL);//FlReEn=0;
		//		ReleaseSemaphore(hmtx,1,NULL);

	}
	if (tID_El)
		return	tID_El->Elem;
	else
		return	NULL;
}

ID_Elem*	ID_List8::Find(UCHAR* Buff,size_t Cnt)
{
	ID_ElemM*	tID_El=NULL;
	if (Fl_Del==0)
	{
//		WaitForSingleObject(hmtx,INFINITE);
		MyLock(__FILE__,__LINE__,&FlReEn,NULL);//strcpy(LastFunc,"Find");
//		if (++FlReEn>1)
//			CriErr=GetServices()->SetCriticalError(NULL,"ID_List8:FindPerviose:FlReEn","Error","ID_List8",CriErr);
		try
		{
			tID_El=FindI(Buff,Cnt);
		}
		catch (...)
		{
			tID_El=NULL;
		}

//		LastFunc[0]=0;
		MyUnLock(__FILE__,__LINE__,&FlReEn,NULL);//FlReEn=0;
//		ReleaseSemaphore(hmtx,1,NULL);
		
	}
	if (tID_El)
		return	tID_El->Elem;
	else
		return	NULL;
}

ID_ElemM*	ID_List8::FindI(UCHAR* Buff,size_t Cnt,FindStrList8	&FSL8)
{
	UCHAR	TempPtr=Buff[Level];
	ID_List8*	lev=Lev[TempPtr];
	ID_ElemM*	elemM=&ElemM[TempPtr];
	ID_Elem*	elem=ElemM[TempPtr].Elem;
	tPElem=TempPtr;
	if(lev==0)
	{
		if (elem!=NULL)
		{
			if (elem->Len==Cnt)
			{
				if (memcmp(elem->Buff,Buff,Cnt)==0)
				{
					FlBeg=1;
//					FSL8.Active=elemM;
//					FSL8.Perv=FSL8.Active->Perviose;
//					FSL8.Next=FSL8.Active->Next;
// 						PoiRoot->ActiveEl=elemM;
// 					PoiRoot->ActiveNEl=PoiRoot->ActivePEl=NULL;
					return elemM;
				};
			};
		}
// 		if (Cond!=0)
// 		{
// 			PoiRoot->ActiveEl=NULL;
// 		}
	}
	else
	{
		if (elem!=NULL)
		{
			if (elem->Len==Cnt)
				if (memcmp(elem->Buff,Buff,Cnt)==0)
				{
					FlBeg=1;
//					FSL8.Active=elemM;
//					FSL8.Perv=FSL8.Active->Perviose;
//					FSL8.Next=FSL8.Active->Next;
// 					if (Cond!=0)
// 					{
// 						PoiRoot->ActiveEl=elemM;
// 					}
// 					PoiRoot->ActiveNEl=PoiRoot->ActivePEl=NULL;
					return elemM;
				}
		}
		if (Cnt<=1+Level)
		{
// 			if (Cond!=0)
// 			{
// 				PoiRoot->ActiveEl=NULL;
// 			}
// 			PoiRoot->ActiveNEl=PoiRoot->ActivePEl=NULL;
			return 0;	// =ðùªø ýõòþ÷üþöýþ
		}
		return lev->FindI(Buff,Cnt,FSL8);
	}
	return 0;	// 
};

ID_ElemM*	ID_List8::FindI(UCHAR* Buff,size_t Cnt)
{
	UCHAR	TempPtr=Buff[Level];
	ID_List8*	lev=Lev[TempPtr];
	ID_ElemM*	elemM=&ElemM[TempPtr];
	ID_Elem*	elem=ElemM[TempPtr].Elem;
//	if (Cond!=0) tPElem=TempPtr;
	tPElem=TempPtr;
	if(lev==0)
	{
		if (elem!=NULL)
		{
			if (elem->Len==Cnt)
			{
				if (memcmp(elem->Buff,Buff,Cnt)==0)
				{
					FlBeg=1;
// 					if (FSL8)
// 					{
// 						FSL8.Active=elemM;
// 						FSL8.Perv=FSL8.Active->Perviose;
// 						FSL8.Next=FSL8.Active->Next;
// 						// 						PoiRoot->ActiveEl=elemM;
// 					}
					// 					PoiRoot->ActiveNEl=PoiRoot->ActivePEl=NULL;
					return elemM;
				};
			};
		}
		// 		if (Cond!=0)
		// 		{
		// 			PoiRoot->ActiveEl=NULL;
		// 		}
	}
	else
	{
		if (elem!=NULL)
		{
			if (elem->Len==Cnt)
				if (memcmp(elem->Buff,Buff,Cnt)==0)
				{
					FlBeg=1;
					// 					if (Cond!=0)
					// 					{
					// 						PoiRoot->ActiveEl=elemM;
					// 					}
					// 					PoiRoot->ActiveNEl=PoiRoot->ActivePEl=NULL;
					return elemM;
				}
		}
		if (Cnt<=1+Level)
		{
			// 			if (Cond!=0)
			// 			{
			// 				PoiRoot->ActiveEl=NULL;
			// 			}
			// 			PoiRoot->ActiveNEl=PoiRoot->ActivePEl=NULL;
			return 0;	// =ðùªø ýõòþ÷üþöýþ
		}
		return lev->FindI(Buff,Cnt);
	}
	return 0;	// 
};

	Savelog::Savelog()
{
// Ýòîò ôðàãìåíò äîëæåí áûòü âñòàâëåí âî âñå ïîñëåäóþùèå îáúåêòû
// Savelog_ äîëæåí áèòü çàìåíåí íà èìÿ îáúåêòà
		Name_Mt=Savelog_NameMt;Rem_Mt=Savelog_RemMt;Ver_=Savelog_Version;Auth_=Savelog_Author;
		PinName_I=Savelog_PinNameI;PinName_O=Savelog_PinNameO;Prm_=Savelog_Prm;Vol_=Savelog_Vol;
		Num_PinI=0;while (PinName_I[2*Num_PinI][0]!=0x0) Num_PinI++;
		Num_PinO=0;while (PinName_O[2*Num_PinO][0]!=0x0) Num_PinO++;
		Num_Prm=0;while (Prm_[2*Num_Prm][0]!=0x0)Num_Prm ++;
// -------------------------------------------------------------
		//Fl_Stop=1;//Fl_Start=0;
		strcpy(Path,"\\Log");
		CalcPath(Path);
		PathName[0]=0;
		strcpy(ExtLog,"log");
		NDay=10;
		TimeWr=5000;	
		pBufOut=0;

//		FileLog=0;
		Pror=0;
//		time(&beg);
		AppendLF=AppendTime=0;
		TypeTime=0;
		Day=0;
};
	Savelog::~Savelog()
{
		Stop();
//		if (FileLog!=0) {_close(FileLog);FileLog=0;}
};

void	Savelog::Param(char* Prm,char* Vol)
{
	if (strcmp(Prm,"PathLog")==0)	
	{
		strcpy(Path,Vol);
		MakeDir(Path);
		CalcPath(Path);		
	}
	if (strcmp(Prm,"NDay")==0)		
		NDay=atoi(Vol);
	if (strcmp(Prm,"TimeWr")==0)	{TimeWr=atoi(Vol)*1000;if (TimeWr==0) TimeWr=5000;}
	if (strcmp(Prm,"AppendTime")==0)
	  {
	  if ((strcmp(Vol,"On")==0)||(strcmp(Vol,"ON")==0)||(strcmp(Vol,"on")==0))
		  AppendTime=1;
	  else 
		  if (strcmp(Vol,"OnEnd")==0)
			  AppendTime=2;
		  else 
			  AppendTime=0;

	  };

	if (strcmp(Prm,"AppendLF")==0)
	  {
	  if ((strcmp(Vol,"On")==0)||(strcmp(Vol,"ON")==0)||(strcmp(Vol,"on")==0))
		AppendLF=1;
	  else if (strcmp(Vol,"OnEnd")==0)
		AppendLF=2;
	  else
		AppendLF=0;
	  };	

	if (strcmp(Prm,"TypeTime")==0)
	  {
	  if ((strcmp(Vol,"Local")==0)||(strcmp(Vol,"local")==0))
		TypeTime=1;	  
	  else
		TypeTime=0;
	  };

	if (strcmp(Prm,"ExtLog")==0)	strcpy(ExtLog,Vol);

};

void	Savelog::Start()
{
	//if (FileLog!=0) {_close(FileLog);FileLog=0;}
	MakeDir(Path);
	CalcPath(Path);

	SYSTEMTIME	TTime;
	if (TypeTime==1) 
	{
		GetLocalTime (&TTime);
	}
	else
	{
		GetSystemTime (&TTime);
	}
	Day=TTime.wDay;Fl_FindF=1;
	
	sprintf(NameLog,"%02d%02d%04d.%s",TTime.wDay,TTime.wMonth,TTime.wYear,ExtLog);
	strcpy(PathName,Path);strcat(PathName,NameLog);

//	time(&beg);
	pBufOut=0;
//	Fl_Stop=0;
//	Fl_Start=1;
	Pror=0;
};

void	Savelog::Stop()
{	
//	Fl_Stop=1;
	if (PathName[0])
		WriteFileLog();
};

int Savelog::IdleFunc(int NumberTr)			// Idle function  if rc!=0 recall function (?)  NumberTr=-1 System NumberTr+0x8000 - DinTr
{
	int	rc=ChildIdleFunc(NumberTr);
	Data(1,(UCHAR*)&rc,1);
	return rc;
}

int	Savelog::Data(int N_Pin,UCHAR* pmas,size_t Cnt)
{
	//int	OCnt=0,
	int	FlR=0;

	if (Cnt==0) return 0;
	FILETIME ApT,ApLT;
	SYSTEMTIME StrT;
//	if (Fl_Stop!=1)
//	{
		switch (N_Pin)
		{
		case 0://"DataIn"
			if ((pBufOut+Cnt+12+4)>=MaxSizBufOut)	// !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
				WriteFileLog();

			if (AppendLF==1)
			{				
//				if ((*(short*)(BufOut+pBufOut-2))!=0x0a0d)
//				{
				//	*(short*)(BufOut+pBufOut)=0x0a0d;pBufOut+=2;					
				//  [11/22/2005] Ïî ïðîñüáå Mao
				//	*(short*)(BufOut+pBufOut)=0x0a0d;pBufOut+=2;
					BufOut[pBufOut++]=0x0a;
//				}
			}
			

			
			Srv->GetRealTime(&ApT);
			
			if (TypeTime==1) 
			{
				FileTimeToLocalFileTime(&ApT,&ApLT);
				FileTimeToSystemTime(&ApLT,&StrT);
			}
			else
			{
				FileTimeToSystemTime(&ApT,&StrT);
			}
			if (StrT.wDay!=Day) 
			{
				WriteFileLog();
				sprintf(NameLog,"%02d%02d%04d.%s",StrT.wDay,StrT.wMonth,StrT.wYear,ExtLog);
				strcpy(PathName,Path);strcat(PathName,NameLog);
				Day=StrT.wDay;
				Fl_FindF=1;
			}
			
			if (AppendTime)
			{
//				time_t ApT;time(&ApT);
//				tm *StrT;StrT=gmtime( &ApT);			
				
				//char tTBuf[16];
				pBufOut+=sprintf((BufOut+pBufOut),"%02d:%02d:%02d.%03d ",StrT.wHour,StrT.wMinute,StrT.wSecond,StrT.wMilliseconds);
				//sprintf(tTBuf,"%02d:%02d:%02d.%03d ",StrT.wHour,StrT.wMinute,StrT.wSecond,StrT.wMilliseconds);
					//>tm_hour,StrT->tm_min,StrT->tm_sec);
				//memcpy((BufOut+pBufOut),tTBuf,9);pBufOut+=9;
//				if ((*(short*)(BufOut+pBufOut-2))==0x0d0a) 
//					(*(short*)(BufOut+pBufOut-2))=0x2020;
//				memcpy((BufOut+pBufOut),ctime(&ApT),26);pBufOut+=26;
			}
			memcpy((BufOut+pBufOut),pmas,Cnt);pBufOut+=Cnt;

			if (AppendLF==2)
			{				
//				if ((*(short*)(BufOut+pBufOut-2))!=0x0a0d)
//				{
				//  [11/22/2005] Ïî ïðîñüáå Mao
				//	*(short*)(BufOut+pBufOut)=0x0a0d;pBufOut+=2;
					BufOut[pBufOut++]=0x0a;
//				}
			}
//			if (Fl_Stop==1)	WriteFileLog();			
			break;
		case 1://"TimeClick",
	//		if ((Pror<33) && (Fl_Stop!=1)) 
//			{Pror++;break;}
//			else 
//				Pror=0;
//			if (Srv->Timer(Pror,1000))//Ïðîðåæèâàòåëü
			if (Srv->Timer(Pror,TimeWr))
			{
//				if ((time(&end)-beg)>TimeWr) // || (Fl_Stop==1)) //Ñáðîñ òàéìåðà è íîìåðà â ôàéë
//				{
//					TLogEnd=(tm)*(tm*)localtime(&end);
//					//Fl_FindF=0;
//					TLogBeg=(tm)*(tm*)localtime(&beg);
//					if (TLogEnd.tm_yday!=TLogBeg.tm_yday)
//					if ((hFile=_findfirst(PathName,&c_file))!=-1)
//					{
//						TLogBeg=(tm)*(tm*)localtime(&c_file.time_create);
//						if (TLogEnd.tm_yday!=TLogBeg.tm_yday)
//						{
//							GetLocalTime (&TTime);
//							sprintf(NameLog,"%02d%02d%04d.%s",TTime.wDay,TTime.wMonth,TTime.wYear,ExtLog);
//							strcpy(PathName,Path);strcat(PathName,NameLog);
//							Fl_FindF=1;
//						};						
//						_findclose( hFile );
//					}				
										
					if (Fl_FindF==1)// || (Fl_Start==1))
					{
						Fl_FindF=0;
//						char tPat[128];
//						sprintf(tPat,"*.%s",ExtLog);
//						strcpy(PathNameOld,Path);
//						//strcat(PathNameOld,"*.log");
//						strcat(PathNameOld,tPat);

						char PathNameOld[256];

						sprintf(PathNameOld,"%s*.%s",Path,ExtLog);

						struct _finddata_t	c_file;
						intptr_t			hFile;

						if ((hFile=_findfirst(PathNameOld,&c_file))!=-1)
						{
							struct	tm TLogBeg,TLogEnd;
							time_t	end;time(&end);

							if (TypeTime==1) 
							{TLogEnd=(tm)*(tm*)localtime(&end);}
							else
							{TLogEnd=(tm)*(tm*)gmtime(&end);}
							do
							{
//								if (TypeTime==1) 
//								{TLogBeg=(tm)*(tm*)localtime(&c_file.time_write);}
//								else
//								{TLogBeg=(tm)*(tm*)gmtime(&c_file.time_write);}
								if (c_file.time_create!=-1)
								{
								if (TypeTime==1) 
								{TLogBeg=(tm)*(tm*)localtime(&c_file.time_create);}
								else
								{TLogBeg=(tm)*(tm*)gmtime(&c_file.time_create);}
								}
								else
								{
									if (TypeTime==1) 
									{TLogBeg=(tm)*(tm*)localtime(&c_file.time_write);}
									else
									{TLogBeg=(tm)*(tm*)gmtime(&c_file.time_write);}
								}

								//TLogBeg=(tm)*(tm*)localtime(&c_file.time_write);
								if (TLogEnd.tm_year-TLogBeg.tm_year==0)
								{
									if ((TLogEnd.tm_yday-TLogBeg.tm_yday)>NDay)
									{strcpy(PathNameOld,Path);strcat(PathNameOld,c_file.name);remove(PathNameOld);}
								}
								else if (TLogEnd.tm_year-TLogBeg.tm_year==1)
								{
									if ((365-TLogBeg.tm_yday+TLogEnd.tm_yday)>(NDay+1))
									{strcpy(PathNameOld,Path);strcat(PathNameOld,c_file.name);remove(PathNameOld);}
								}
								else
								{strcpy(PathNameOld,Path);strcat(PathNameOld,c_file.name);remove(PathNameOld);};
							} while( _findnext( hFile, &c_file ) == 0 );
							_findclose( hFile );
						};
					};
					if (pBufOut!=0)
						WriteFileLog();
					else
					{
						Srv->GetRealTime(&ApT);
						
						if (TypeTime==1) 
						{
							FileTimeToLocalFileTime(&ApT,&ApLT);
							FileTimeToSystemTime(&ApLT,&StrT);
						}
						else
						{
							FileTimeToSystemTime(&ApT,&StrT);
						}
						if (StrT.wDay!=Day) 
						{
							sprintf(NameLog,"%02d%02d%04d.%s",StrT.wDay,StrT.wMonth,StrT.wYear,ExtLog);
							strcpy(PathName,Path);strcat(PathName,NameLog);
							Day=StrT.wDay;
							Fl_FindF=1;
						}
					}					
//				};
			};
			break;
		default:
			break;
		}
//	};
	
	return 0;
};

void Savelog::WriteFileLog()
{
	if (pBufOut)
	{
#if _MSC_VER > 1000
		int		FileLog;//handle for log-file
		if ((FileLog = _open(PathName, _O_RDWR|_O_CREAT|_O_APPEND|_O_BINARY,_S_IREAD|_S_IWRITE ))!=-1)
		{
			_write(FileLog,(void *)BufOut,pBufOut);
//			_commit(FileLog);
			_close(FileLog);
		}
		else
		{
			Srv->SetCriticalError(this,"Can't open Log (Log Data Lost)","Warning");
		}
		
#endif // _MSC_VER > 1000
		pBufOut=0;		
		//time(&beg);
		//FileLog=0;
	};
};


void	CTask::DelMd(int ID_Md)		// Óäàëèòü ðåæèì (íîìåð ðåæìà àáñîëþòíûé)
{
	size_t i;
	// Óäàëÿåì ìåòîäû
	for (i=0;i<Tsk->Max_Mt;i++)
	{
		if (Tsk->Mt[i].ID_Md==ID_Md){DelMt(i);i--;}
	}
	// Óäàëÿåì ðåæèì
	for(i=ID_Md+1;i<Tsk->Max_Md;i++)
	{
		CopyMd(i-1,i);
	};
	Tsk->Max_Md--;
	// Ìîäèôèöèðóåì ìåòîäû
	for (i=0;i<Tsk->Max_Mt;i++)
	{
		if (Tsk->Mt[i].ID_Md>ID_Md) Tsk->Mt[i].ID_Md--;
	}
};
void	CTask::DelMdMt(int ID_Md,int ID_Mt)	// Óäàëèòü â äàííîì ðåæèìå ìåòîä (íîìåð ìåòîäà îòíîñèòåëüíûé) 
{
	size_t	i;
	size_t	j=0;
	for (i=0;i<Tsk->Max_Mt;i++)
		if (Tsk->Mt[i].ID_Md==ID_Md)
		{
			if (j==ID_Mt)
			{
				DelMt(i);
			}
			j++;
		}
};
void	CTask::DelMt(int ID_Mt)		// Óäàëèòü ìåòîä (íîìåð ìåòîäà àáñîëþòíûé)
{
	size_t i;
	// Óäàëÿåì ëèíêè
	for (i=0;i<Tsk->Max_LP;i++)
	{
		if (Tsk->LP[i].ID_Mt1==ID_Mt){ DelLP(i);i--;continue;}
		if (Tsk->LP[i].ID_Mt2==ID_Mt){ DelLP(i);i--;}
	}
	// Óäàëÿåì ïàðàìåòðû
	for (i=0;i<Tsk->Max_Pr;i++)
	{
		if (Tsk->Pr[i].ID_Mt==ID_Mt) { DelPr(i);i--;}
	}
	// Óäàëèì ìåòîä
	for(i=ID_Mt+1;i<Tsk->Max_Mt;i++)
	{
		CopyMt(i-1,i);
	};
	Tsk->Max_Mt--;
	// Ìîäèôèöèðóåì ëèíêè
	for (i=0;i<Tsk->Max_LP;i++)
	{
		if (Tsk->LP[i].ID_Mt1>ID_Mt) Tsk->LP[i].ID_Mt1--;
		if (Tsk->LP[i].ID_Mt2>ID_Mt) Tsk->LP[i].ID_Mt2--;
	}
	// Ìîäèôèöèðóåì ïàðàìåòðû
	for (i=0;i<Tsk->Max_Pr;i++)
	{
		if (Tsk->Pr[i].ID_Mt>ID_Mt) Tsk->Pr[i].ID_Mt--;
	}

};
void	CTask::DelPr(int ID_Pr)
{
	size_t i;
	// Óäàëèì ïàðàìåòð
	for(i=ID_Pr+1;i<Tsk->Max_Pr;i++)
	{
		CopyPr(i-1,i);
	};
	Tsk->Max_Pr--;
};

void	CTask::DelLP(int ID_LP)
{
	size_t i;
	// Óäàëèì ëèíê
	for(i=ID_LP+1;i<Tsk->Max_LP;i++)
	{
		CopyLP(i-1,i);
	};
	Tsk->Max_LP--;
};

void	CTask::CopyMd(int ID_Md1,int ID_Md2)
{
	strcpy(Tsk->Md[ID_Md1].Name,Tsk->Md[ID_Md2].Name);
	Tsk->Md[ID_Md1].ID_En=Tsk->Md[ID_Md2].ID_En;
	Tsk->Md[ID_Md1].ID_Sp=Tsk->Md[ID_Md2].ID_Sp;
	Tsk->Md[ID_Md1].ID_UD=Tsk->Md[ID_Md2].ID_UD;
	Tsk->Md[ID_Md1].MethObn=Tsk->Md[ID_Md2].MethObn;
	Tsk->Md[ID_Md1].TimeProb=Tsk->Md[ID_Md2].TimeProb;
        Tsk->Md[ID_Md1].X=Tsk->Md[ID_Md2].X;
        Tsk->Md[ID_Md1].Y=Tsk->Md[ID_Md2].Y;
};

void	CTask::CopyMt(int ID_Mt1,int ID_Mt2)
{
	strcpy(Tsk->Mt[ID_Mt1].Name,Tsk->Mt[ID_Mt2].Name);
	Tsk->Mt[ID_Mt1].ID_Md=Tsk->Mt[ID_Mt2].ID_Md;
	Tsk->Mt[ID_Mt1].N_PI=Tsk->Mt[ID_Mt2].N_PI;
	Tsk->Mt[ID_Mt1].N_PO=Tsk->Mt[ID_Mt2].N_PO;
	Tsk->Mt[ID_Mt1].Ext=Tsk->Mt[ID_Mt2].Ext;
	Tsk->Mt[ID_Mt1].PDec=Tsk->Mt[ID_Mt2].PDec;
        Tsk->Mt[ID_Mt1].X=Tsk->Mt[ID_Mt2].X;
        Tsk->Mt[ID_Mt1].Y=Tsk->Mt[ID_Mt2].Y;


};

void	CTask::CopyLP(int ID_LP1,int ID_LP2)
{
	Tsk->LP[ID_LP1].ID_Mt1=Tsk->LP[ID_LP2].ID_Mt1;
	Tsk->LP[ID_LP1].ID_Mt2=Tsk->LP[ID_LP2].ID_Mt2;
	Tsk->LP[ID_LP1].PI=Tsk->LP[ID_LP2].PI;
	Tsk->LP[ID_LP1].PO=Tsk->LP[ID_LP2].PO;
};

void	CTask::CopyPr(int ID_Pr1,int ID_Pr2)
{
	Tsk->Pr[ID_Pr1].ID_Mt=Tsk->Pr[ID_Pr2].ID_Mt;
	strcpy(Tsk->Pr[ID_Pr1].Name,Tsk->Pr[ID_Pr2].Name);
	strcpy(Tsk->Pr[ID_Pr1].Vol,Tsk->Pr[ID_Pr2].Vol);
};

int SrvNetFun(LPVOID	Prm)
{
	((Services*)Prm)->IdleNet();
	return 0;
};

int SrvSysFun(LPVOID	Prm)
{
	((Services*)Prm)->IdleSys();
	return 0;
};

int UDPFun(LPVOID	Prm)
{
	((ElUDPCli*)Prm)->Idle();
	return 1;
};

// int UDPFunExt(LPVOID	Prm)
// {
// 	((ElUDPCli*)Prm)->IdleExt();
// 	return 0;
// };

int SIFun(LPVOID	Prm)
{
	((Integrator*)Prm)->Idle();
	return 0;
};

int SIFunLAN(LPVOID	Prm)
{
	int rc=((Integrator*)Prm)->IdleLAN();
//	rc=1;		// Äëÿ áëîêèðóþùåãî ðåæèìà
	return rc;
};
int SIFunPAN(LPVOID	Prm)
{
	int rc=((Integrator*)Prm)->IdlePAN();
//	rc=1;		// Äëÿ áëîêèðóþùåãî ðåæèìà
	return rc;
};
// int SIFunWAN(ULONG	Prm)
// {
// 		int rc=((Integrator*)Prm)->IdleWAN();
// //	rc=1;		// Äëÿ áëîêèðóþùåãî ðåæèìà
// 	return rc;
// };

Integrator::Integrator(char* tPortLAN,char* tPortPAN,char* tPortWAN)
{
	int					rc;
	strcpy(PortL,tPortLAN);strcpy(PortP,tPortPAN);strcpy(PortW,tPortWAN);
	Fl_ReInit=0;
//	Fl_RE=0;
//	scc=NULL;
	ssPAN=ssLAN=scLAN=scPAN=NULL;	//=ssWAN=scWAN1=scWAN2
	Status=0;
	Srv=GetServices();Flag=0;FlagSyn=0;FlStop=0;
	Anchor.Init(Srv,this,0);
	Srv->GetRealTime(&RTime.FTime);	//=time(NULL);
	Srv->GetRealTime(&Srv->TimeNetLock.FTime);
	NumberHost=0;	// !!!!!!!!!!!!!!!!!!!!!!
	SendCnt=0;
	int		tnPort=atoi(PortL);
	sprintf(PortL2,"%d",tnPort+1);
	tnPort=atoi(PortP);
	sprintf(PortP2,"%d",tnPort+1);
	tnPort=atoi(PortW);
	sprintf(PortW2,"%d",tnPort+1);
//	char	Port3[5];
//	char	Port4[6];
//	sprintf(Port3,"%d",tnPort+1);
//	sprintf(Port4,"%d",tnPort-2);
//	sprintf(Port3,"%d",tnPort);
//	sprintf(Port4,"%d",tnPort);
	FatalError=0;
	dontblock=1;
	dgramsz = 1440;
	SizeSource=sizeof(SourceLAN);
	NumLANCli=0;
	NumPANCli=0;
//	NumWANCli=0;
	for (size_t i=0;i<256;i++)
	{
//		CliLAN[i]=NULL;
		CliPAN[i]=NULL;
// 		CliPAN[i]->fl=0;
// 		CliPAN[i]->IPAddr[0]=0;
// 		CliPAN[i]->Host[0]=0;
// 		CliPAN[i]->ShortHost[0]=0;
// 		CliPAN[i]->CPUUsage=0;
// 		CliPAN[i]->UID_CritMsg=0;
// 		CliLAN[i]->fl=0;
// 		CliLAN[i]->IPAddr[0]=0;
// 		CliLAN[i]->Host[0]=0;
// 		CliLAN[i]->ShortHost[0]=0;
// 		CliLAN[i]->CPUUsage=0;
// 		CliLAN[i]->UID_CritMsg=0;
 		TimeAdd[i]=0;
	}
	if (Srv->M_Cfg->MData.Fl_LAN)
	{
		scLAN = udp_client( CreateBroadcastLANAddress(host),PortL, &peerLAN );		// Local server port
//		scc = udp_client( CreateBroadcastLANAddress(host),Port2, &peerc );	// Common server port 
		rc=ioctlsocket( scLAN, FIONBIO, (PULONG)&dontblock);
//		rc=ioctlsocket( scc, FIONBIO, (PULONG)&dontblock);
		ssLAN = udp_server( inet_ntoa(Srv->IPAddrLAN),PortL );
		dontblock=0;
		rc=ioctlsocket( ssLAN, FIONBIO, (PULONG)&dontblock);
		dontblock=3000;
		rc=setsockopt(ssLAN,SOL_SOCKET,SO_RCVTIMEO,(char*)&dontblock,sizeof(BOOL));
	};
	if (Srv->M_Cfg->MData.Fl_PAN)
	{
		scPAN = udp_client( CreateBroadcastPANAddress(host),PortP, &peerPAN );		// Local server port
		rc=ioctlsocket( scPAN, FIONBIO, (PULONG)&dontblock);
		ssPAN = udp_server( inet_ntoa(Srv->IPAddrPAN),PortP );
		dontblock=0;
		rc=ioctlsocket( ssPAN, FIONBIO, (PULONG)&dontblock);
		dontblock=3000;
		rc=setsockopt(ssPAN,SOL_SOCKET,SO_RCVTIMEO,(char*)&dontblock,sizeof(BOOL));
	};
// 	if (Srv->M_Cfg->MData.Fl_WAN)
// 	{
// 		if (Srv->M_Cfg->MData.IP_WANIS1)
// 		{
// 			if((Srv->M_Cfg->MData.Fl_RunSICli==0)||(MyLANNum==NumLANSrv))
// 				scWAN1 = udp_client( inet_ntoa(*(in_addr*)&Srv->M_Cfg->MData.IP_WANIS1),Port4, &peerWAN1 );		// Local server port
// 			rc=ioctlsocket( scWAN1, FIONBIO, (PULONG)&dontblock);
// 		}
// 		if (Srv->M_Cfg->MData.IP_WANIS2)
// 		{
// 			if((Srv->M_Cfg->MData.Fl_RunSICli==0)||(MyLANNum==NumLANSrv))
// 				scWAN2 = udp_client( inet_ntoa(*(in_addr*)&Srv->M_Cfg->MData.IP_WANIS2),Port4, &peerWAN2 );		// Local server port
// 			rc=ioctlsocket( scWAN2, FIONBIO, (PULONG)&dontblock);
// 		}
// 		if (Srv->M_Cfg->MData.Fl_RunSI)
// 		{
// 			ssWAN = udp_server( inet_ntoa(Srv->IPAddrWAN),Port4 );
// 			dontblock=0;
// 			rc=ioctlsocket( ssWAN, FIONBIO, (PULONG)&dontblock);
// 			dontblock=3000;
// 			rc=setsockopt(ssWAN,SOL_SOCKET,SO_RCVTIMEO,(char*)&dontblock,sizeof(BOOL));
// 		}
// 	};

	State=0;	// Client
//	strcpy(Srv->UInfo.Mode,"Active");
	UDPP.CPUUsage=0;
	Srv->GetRealTime(&laLANSvrTime.FTime);	//=time(NULL);
	Srv->GetRealTime(&laLANSSvrTime.FTime);	//=time(NULL);
	UDPP.HostName[0]=0;INetAdrPAN=INetAdrLAN=0;	//INetAdrWAN=
#ifndef Win9x
	gethostname (UDPP.HostName,255);
#endif
#ifndef Win9x
//	hoststr=gethostbyname(UDPP.HostName);
//	if (hoststr)INetAdr=*(int*)hoststr->h_addr_list[0];
//	strcpy(UDPP.HostName,hoststr->h_name);
	INetAdrPAN=*(long*)&Srv->IPAddrPAN;
	INetAdrLAN=*(long*)&Srv->IPAddrLAN;
//	INetAdrWAN=*(long*)&Srv->IPAddrWAN;
	UDPP.WANADR=*(long*)&Srv->IPAddrWAN;
	strcpy(UDPP.HostName,Srv->ShortHostName);
	strcpy(UDPP.MainDomainName,Srv->MainDomainName);
#endif
//	if (INetAdr==0x0100007f) INetAdr=0xff00007f;
	MyPANNum=*(3+(UCHAR*)(&INetAdrPAN));
	NumPANSrv=0;
	MyLANNum=INetAdrLAN;
	NumLANSrv=0;
	NumLANSSrv=0;
	Srv->Fl_ConnectSrv=0;
	UDPP.Command=SICom_Connect;
	UDPP.Ver=mVersion;
	UDPP.MemUsage=0;
	if (scPAN)
	{
		Srv->GetRealTime(&UDPP.FTime.FTime);
		rc = sendto( scPAN, (char*)&UDPP.Command, sizeof(UDPPack) /*strlen(UDPP.HostName)+5+2*sizeof(UDPP.CPUUsage)+sizeof(FileTime)*/, 0,( struct sockaddr * )&peerPAN, sizeof( struct sockaddr ) );
	};
	Sleep(50);
	if (scLAN)
	{
		Srv->GetRealTime(&UDPP.FTime.FTime);
		rc = sendto( scLAN, (char*)&UDPP.Command, sizeof(UDPPack) /*strlen(UDPP.HostName)+5+2*sizeof(UDPP.CPUUsage)+sizeof(FileTime)*/, 0,( struct sockaddr * )&peerLAN, sizeof( struct sockaddr ) );
	};
// 	Sleep(50);
// 	if (scWAN1)
// 	{
// 		Srv->GetRealTime(&UDPP.FTime.FTime);
// 		if((Srv->M_Cfg->MData.Fl_RunSICli==0)||(MyLANNum==NumLANSrv))
// 			rc = sendto( scWAN1, (char*)&UDPP.Command, sizeof(UDPPack) /*strlen(UDPP.HostName)+5+2*sizeof(UDPP.CPUUsage)+sizeof(FileTime)*/, 0,( struct sockaddr * )&peerWAN1, sizeof( struct sockaddr ) );
// 	};
// 	Sleep(50);
// 	if (scWAN2)
// 	{
// 		Srv->GetRealTime(&UDPP.FTime.FTime);
// 		if((Srv->M_Cfg->MData.Fl_RunSICli==0)||(MyLANNum==NumLANSrv))
// 			rc = sendto( scWAN2, (char*)&UDPP.Command, sizeof(UDPPack) /*strlen(UDPP.HostName)+5+2*sizeof(UDPP.CPUUsage)+sizeof(FileTime)*/, 0,( struct sockaddr * )&peerWAN2, sizeof( struct sockaddr ) );
// 	};
	Fl_ReConnect=0;
	CThread=new	ClassThread("SIFun",SIFun,(LPVOID)this,THREAD_PRIORITY_ABOVE_NORMAL);
	CThreadPAN=new	ClassThread("SIFunPAN",SIFunPAN,(LPVOID)this,THREAD_PRIORITY_ABOVE_NORMAL);
	CThreadLAN=new	ClassThread("SIFunLAN",SIFunLAN,(LPVOID)this,THREAD_PRIORITY_ABOVE_NORMAL);
//	CThreadWAN=new	ClassThread("SIFunWAN",SIFunWAN,(LPVOID)this,THREAD_PRIORITY_ABOVE_NORMAL);
};

void	Integrator::OnLine()
{
	Srv->GetRealTime(&RTime.FTime);
};

char*		Integrator::FindHost(char* HostName)
{
	size_t i;
	El_CliInt*	Cli;
	FindStrList8	FSL8;
	Cli=(El_CliInt*)CliLAN.Begin(FSL8);
	while (Cli)
	{
//		if (CliLAN[i])
		{
			if ((strcmp(Cli->Cli.Host,HostName)==0)||(strcmp(Cli->Cli.ShortHost,HostName)==0))
			{
				return  Cli->Cli.IPAddr;
			}
		};
		Cli=(El_CliInt*)CliLAN.Next(FSL8);
	}
	for ( i=0;i<256;i++)
	{
		if (CliPAN[i])
		{
			if ((strcmp(CliPAN[i]->Host,HostName)==0)||(strcmp(CliPAN[i]->ShortHost,HostName)==0))
			{
				return CliPAN[i]->IPAddr;
			}
		};
	}
	FSL8.DestroyStr();
	Cli=(El_CliInt*)CliWAN.Begin(FSL8);
	while (Cli)
	{
//		if (Cli->Cli.fl!=0)
		{
			if ((strcmp(Cli->Cli.Host,HostName)==0)||(strcmp(Cli->Cli.ShortHost,HostName)==0))
			{
				return Cli->Cli.IPAddr;
			}
		};
		Cli=(El_CliInt*)CliWAN.Next(FSL8);
	}
	return NULL;
};

UDPCli*	Integrator::FindCliByHost(char* HostName)
{
	size_t i;
	El_CliInt*	Cli;
	FindStrList8	FSL8;
	Cli=(El_CliInt*)CliLAN.Begin(FSL8);
	while (Cli)
	{
//		if (CliLAN[i]!=0)
		{
			if ((strcmp(Cli->Cli.Host,HostName)==0)||(strcmp(Cli->Cli.ShortHost,HostName)==0))
			{
				return  &Cli->Cli;
			}
		};
		Cli=(El_CliInt*)CliLAN.Next(FSL8);
	}
	for ( i=0;i<256;i++)
	{
		if (CliPAN[i]!=0)
		{
			if ((strcmp(CliPAN[i]->Host,HostName)==0)||(strcmp(CliPAN[i]->ShortHost,HostName)==0))
			{
				return CliPAN[i];
			}
		};
	}
	FSL8.DestroyStr();
	Cli=(El_CliInt*)CliWAN.Begin(FSL8);
	while (Cli)
	{
//		if (Cli->Cli.fl!=0)
		{
			if ((strcmp(Cli->Cli.Host,HostName)==0)||(strcmp(Cli->Cli.ShortHost,HostName)==0))
			{
				return &Cli->Cli;
			}
		};
		Cli=(El_CliInt*)CliWAN.Next(FSL8);
	}
	return NULL;
};


int	Integrator::IdlePAN()
{
	int trc=0;
	int					rc;
//	FileTime	tFTime;
	char		tTBuf[128];
//	while (()>0)
	if (FlStop==1) return trc;
	if (Flag!=0) { return trc;};
	if (ssPAN==NULL) return trc;
	trc=1;
	rc = recvfrom( ssPAN, (char*)&bufPAN, sizeof( bufPAN ), 0, &SourcePAN, &SizeSource);
	MyLock(__FILE__,__LINE__,&Fl_RE,NULL);
	if (rc>0)
	{
		UINT Num;
		Num=*(UINT*)&SourcePAN.sa_data[2];
		if ((Num&Srv->M_Cfg->MData.IP_MaskPAN)==(Srv->M_Cfg->MData.IP_PAN&Srv->M_Cfg->MData.IP_MaskPAN))
		{
			bufPAN.HostName[39-sizeof(ULONG)]=0;
			bufPAN.MainDomainName[39]=0;
			trc=rc;
			if (((bufPAN.Command>=SICom_Connect)&&(bufPAN.Command<=SICom_Packet))||(bufPAN.Command==SICom_Hardware))
			{
				Num=(UCHAR)SourcePAN.sa_data[5];
				*((char*)&bufPAN+rc)=0;
				if (CliPAN[Num]==0)
				{	
					CliPAN[Num]=new UDPCli();
					NumPANCli++;
					CliPAN[Num]->fl=1;
					Srv->GetRealTime(&CliPAN[Num]->lwTime.FTime);
					strcpy(CliPAN[Num]->IPAddr,GetInternetStandardAddress(SourcePAN));
					sprintf(CliPAN[Num]->Host,"%s.%s",bufPAN.HostName,bufPAN.MainDomainName);
					strcpy(CliPAN[Num]->ShortHost,bufPAN.HostName);
//					strcpy(CliPAN[Num]->MainDomainName,bufPAN.MainDomainName);
					if (bufPAN.MainDomainName[0]!=0)
					{
						ID_ElemString*	tEl=new ID_ElemString(bufPAN.MainDomainName);
						if (bufPAN.MainDomainName[0]=='#')
							strcpy(CliPAN[Num]->MainDomainName,bufPAN.MainDomainName);
						if (CliPAN[Num]->DomainList==NULL)
						{
							CliPAN[Num]->DomainList=new ID_List8();
						}
						if (CliPAN[Num]->DomainList->Add(tEl)==-1)
						{
							delete tEl;
						}
					}

					if (/*(bufPAN.Ver>=mVersion)&&*/(bufPAN.Command!=SICom_Hardware))
					{
						Srv->NLAdd(0,CliPAN[Num]->IPAddr);
						Srv->NLAdd(1,CliPAN[Num]->IPAddr);
					}
				}
				else
				{
					sprintf(CliPAN[Num]->Host,"%s.%s",bufPAN.HostName,bufPAN.MainDomainName);
					strcpy(CliPAN[Num]->ShortHost,bufPAN.HostName);
//					strcpy(CliPAN[Num]->MainDomainName,bufPAN.MainDomainName);
					if (bufPAN.MainDomainName[0]!=0)
					{
						ID_ElemString*	tEl=new ID_ElemString(bufPAN.MainDomainName);
						if (bufPAN.MainDomainName[0]=='#')
							strcpy(CliPAN[Num]->MainDomainName,bufPAN.MainDomainName);
						if (CliPAN[Num]->DomainList==NULL)
						{
							CliPAN[Num]->DomainList=new ID_List8();
						}
						if (CliPAN[Num]->DomainList->Add(tEl)==-1)
						{
							delete tEl;
						}
					}
					Srv->GetRealTime(&CliPAN[Num]->lwTime.FTime);
				}
				CliPAN[Num]->crTime.Time=bufPAN.FTime.Time;
				CliPAN[Num]->Command=bufPAN.Command;
				CliPAN[Num]->CPUUsage=bufPAN.CPUUsage;
				CliPAN[Num]->MemUsage=bufPAN.MemUsage;
				CliPAN[Num]->Ver=bufPAN.Ver&0x1fff;
				if (bufPAN.Ver&0x8000)
				{
					Srv->GetRealTime(&Srv->TimeNetLock.FTime);
				}
				if (CliPAN[Num]->CPUUsage>=100.0) CliPAN[Num]->CPUUsage=100;
				if (CliPAN[Num]->CPUUsage<=0) CliPAN[Num]->CPUUsage=0;
				if (CliPAN[Num]->MemUsage>=100.0) CliPAN[Num]->MemUsage=100;
				if (CliPAN[Num]->MemUsage<=0) CliPAN[Num]->MemUsage=0;
				
				if (Srv->Timer(TimeAdd[Num],60000))
					if ((CliPAN[Num]->Command!=SICom_Error)/*&&(CliPAN[Num]->Ver>=mVersion)*/&&(bufPAN.Command!=SICom_Hardware))
					{
						Srv->NLAdd(0,CliPAN[Num]->IPAddr);
						Srv->NLAdd(1,CliPAN[Num]->IPAddr);
					}
				switch(CliPAN[Num]->Command)
				{
				case SICom_TimeSyn:
					Srv->Fl_FSyn=0;
					break;
				case SICom_Connect:
					{
//						MyLock(__FILE__,__LINE__,&Srv->hmtxMainThr,Srv);
//						int rc=WaitForSingleObject(Srv->hmtxMainThr,INFINITE);
						if (bufPAN.MainDomainName[0]!=0)
						{
							El_Tab*		ETbl=NULL;
							if (Srv->IDT_NeuronLocalSite)
							{
								ETbl=Srv->IDT_NeuronLocalSite->FindNested(&Anchor,AcsMd_Read,N_Domain,bufPAN.MainDomainName,N_Host,CliPAN[Num]->ShortHost,NULL);
							}
							if (ETbl)
							{
								Srv->IDT_NeuronLocalSite->AddParam(&Anchor,ETbl,IDState,VolumeProcessed[IDVolProc_Setup]);
								Srv->IDT_NeuronLocalSite->UnLockEl(&Anchor,ETbl);
							};
						}
//						MyUnLock(__FILE__,__LINE__,&Srv->hmtxMainThr,Srv);
//						ReleaseSemaphore(Srv->hmtxMainThr,1,NULL);
					}
					break;
				case SICom_Error:
					if (NumPANSrv==MyPANNum)
						if (CliPAN[Num]->UID_CritMsg==0)
						{
//							MyLock(__FILE__,__LINE__,&Srv->hmtxMainThr,Srv);
//							int rc=WaitForSingleObject(Srv->hmtxMainThr,INFINITE);
							Srv->NLDel(0,CliPAN[Num]->IPAddr);
							Srv->NLDel(1,CliPAN[Num]->IPAddr);
							sprintf(tTBuf,"Neuron.exe Error HostName=%s",CliPAN[Num]->Host);
							if (Srv!=NULL)
								CliPAN[Num]->UID_CritMsg=Srv->SetCriticalError(NULL,tTBuf,"Critical","Neuron.exe");
							if (bufPAN.MainDomainName[0]!=0)
							{
								El_Tab*		ETbl=NULL;
								if (Srv->IDT_NeuronLocalSite)
								{
									ETbl=Srv->IDT_NeuronLocalSite->FindNested(&Anchor,AcsMd_Read,N_Domain,bufPAN.MainDomainName,N_Host,CliPAN[Num]->ShortHost,NULL);
								}
								if (ETbl)
								{
									Srv->IDT_NeuronLocalSite->AddParam(&Anchor,ETbl,IDState,VolumeProcessed[IDVolProc_Error]);
									Srv->IDT_NeuronLocalSite->UnLockEl(&Anchor,ETbl);
								};
							};
//							MyUnLock(__FILE__,__LINE__,&Srv->hmtxMainThr,Srv);
//							ReleaseSemaphore(Srv->hmtxMainThr,1,NULL);
						}
					break;
				case SICom_Standalone:
					if (Num==MyPANNum)
						if (State==1)
						{	// Îòêëþ÷èòü ñâîé ñåðâåð
//							MyLock(__FILE__,__LINE__,&Srv->hmtxMainThr,Srv);
//							int rc=WaitForSingleObject(Srv->hmtxMainThr,INFINITE);
							Srv->OutLog("Delete Neuron Server");
							strcpy(Srv->UInfo.Mode,"Active");
							State=0;
							Srv->ModifUnit(Srv);
							Srv->RoleSwOff("Server");
							Srv->SendEvent(EV_StopServer,NULL,0);
//							MyUnLock(__FILE__,__LINE__,&Srv->hmtxMainThr,Srv);
//							ReleaseSemaphore(Srv->hmtxMainThr,1,NULL);
						}
					break;
				case SICom_ClientWait:
				case SICom_ClientActive:
					if (CliPAN[Num]->UID_CritMsg!=0)
					{
						if (Srv!=NULL)
							Srv->DelCriticalError(CliPAN[Num]->UID_CritMsg);
						CliPAN[Num]->UID_CritMsg=0;
						if (/*(CliPAN[Num]->Ver>=mVersion)&&*/(CliPAN[Num]->Command!=SICom_Hardware))
						{
							Srv->NLAdd(0,CliPAN[Num]->IPAddr);
							Srv->NLAdd(1,CliPAN[Num]->IPAddr);
						}
					}
					break;
				case SICom_ServerWait:
				case SICom_ServerActive:
					Srv->GetRealTime(&laPANSvrTime.FTime);
					if (NumPANSrv!=Num)
					{
						NumPANSrv=Num;
						// Ïåðåêîííåêòèòü êëèåíòîâ !!!
						Fl_ReConnect=1;
					}
					else
					{
						NumPANSrv=Num;
					}
					if (CliPAN[Num]->UID_CritMsg!=0)
					{
						if (Srv!=NULL)
							Srv->DelCriticalError(CliPAN[Num]->UID_CritMsg);
						CliPAN[Num]->UID_CritMsg=0;
					}
					break;
				case SICom_SuperVisorActive:
					break;
				case SICom_DisConnect: 
					if (CliPAN[Num])
					{
//						MyLock(__FILE__,__LINE__,&Srv->hmtxMainThr,Srv);
//						int rc=WaitForSingleObject(Srv->hmtxMainThr,INFINITE);
						if (bufPAN.MainDomainName[0]!=0)
						{
							El_Tab*		ETbl=NULL;
							if (Srv->IDT_NeuronLocalSite)
							{
								ETbl=Srv->IDT_NeuronLocalSite->FindNested(&Anchor,AcsMd_Read,N_Domain,bufPAN.MainDomainName,N_Host,CliPAN[Num]->ShortHost,NULL);
							}
							if (ETbl)
							{
								Srv->IDT_NeuronLocalSite->AddParam(&Anchor,ETbl,IDState,VolumeProcessed[IDVolProc_Off]);
								Srv->IDT_NeuronLocalSite->UnLockEl(&Anchor,ETbl);
							};
						}
//						MyUnLock(__FILE__,__LINE__,&Srv->hmtxMainThr,Srv);
//						ReleaseSemaphore(Srv->hmtxMainThr,1,NULL);
						NumPANCli--;
						Srv->NLDel(0,CliPAN[Num]->IPAddr);
						Srv->NLDel(1,CliPAN[Num]->IPAddr);
						CliPAN[Num]->fl=0;
					}
					if (CliPAN[Num]->UID_CritMsg!=0)
					{
						if (Srv!=NULL)
							Srv->DelCriticalError(CliPAN[Num]->UID_CritMsg);
						CliPAN[Num]->UID_CritMsg=0;
					}
					delete CliPAN[Num];	CliPAN[Num]=0;
					break;
				case SICom_Packet: 
					break;
				}
			}
			else
			{
				bufPAN.Command=bufPAN.Command*2;
			}
		}
		MyUnLock(__FILE__,__LINE__,&Fl_RE,NULL);
	}
	else
	{
		MyUnLock(__FILE__,__LINE__,&Fl_RE,NULL);
		if (rc==-1) Sleep(10);
	}
	return trc;
};

int	Integrator::IdleLAN()
{
	int					rc;
	int trc=0;
//	FileTime	tFTime;
//	char		tTBuf[128];
//	while (()>0)
	if (FlStop==1) return trc;
	if (Flag!=0) { return trc;};
	if (ssLAN==NULL) return trc;
	trc=1;

	
	rc = recvfrom( ssLAN, LanBuf, sizeof( LanBuf ), 0, &SourceLAN, &SizeSource);
//	rc = recvfrom( ssLAN, (char*)&bufLAN, sizeof( bufLAN ), 0, &SourceLAN, &SizeSource);
	MyLock(__FILE__,__LINE__,&Fl_RE,NULL);
	if (rc>0)
	{
		UINT Num;
		Num=*(UINT*)&SourceLAN.sa_data[2];
		if (rc==sizeof( UDPPack ))
		{
			if (((Num&Srv->M_Cfg->MData.IP_MaskLAN)==(Srv->M_Cfg->MData.IP_LAN&Srv->M_Cfg->MData.IP_MaskLAN))||(Srv->M_Cfg->MData.LANType==1))
			{
				memcpy(&bufLAN,&LanBuf,sizeof( UDPPack ));
				bufLAN.HostName[39-sizeof(ULONG)]=0;
				bufLAN.MainDomainName[39]=0;
				trc=rc;
				if ((((bufLAN.Command>=SICom_Connect)&&(bufLAN.Command<=SICom_Packet))||(bufLAN.Command==SICom_Hardware))&&((bufLAN.Ver&0xfff)/100==mVersion/100))
				{
					//			UINT Num;
					//				Num=(UCHAR)SourceLAN.sa_data[5];
					*((char*)&bufLAN+rc)=0;
					El_CliInt*	Cli=(El_CliInt*)CliLAN.Find((UCHAR*)&Num,sizeof(ULONG));
					if (Cli==0)
					{	
						Cli=new El_CliInt(Num);
						CliLAN.Add(Cli);
						Cli->Cli.fl=1;
						NumLANCli++;
						Srv->GetRealTime(&Cli->Cli.lwTime.FTime);
						strcpy(Cli->Cli.IPAddr,GetInternetStandardAddress(SourceLAN));
						sprintf(Cli->Cli.Host,"%s.%s",bufLAN.HostName,bufLAN.MainDomainName);
						strcpy(Cli->Cli.ShortHost,bufLAN.HostName);
						//					strcpy(Cli->Cli.MainDomainName,bufLAN.MainDomainName);
						if (bufLAN.MainDomainName[0]!=0)
						{
							ID_ElemString*	tEl=new ID_ElemString(bufLAN.MainDomainName);
							if (bufLAN.MainDomainName[0]=='#')
								strcpy(Cli->Cli.MainDomainName,bufLAN.MainDomainName);
							if (Cli->Cli.DomainList==NULL)
							{
								Cli->Cli.DomainList=new ID_List8();
							}
							if (Cli->Cli.DomainList->Add(tEl)==-1)
							{
								delete tEl;
							}
						}
						if (((bufLAN.Ver&0xfff)/100==mVersion/100)&&(bufLAN.Command!=SICom_Hardware))
						{
							Srv->NLAdd(0,Cli->Cli.IPAddr);
							Srv->NLAdd(1,Cli->Cli.IPAddr);
						}
					}
					else
					{
						sprintf(Cli->Cli.Host,"%s.%s",bufLAN.HostName,bufLAN.MainDomainName);
						strcpy(Cli->Cli.ShortHost,bufLAN.HostName);
						//					strcpy(Cli->Cli.MainDomainName,bufLAN.MainDomainName);
						if (bufLAN.MainDomainName[0]!=0)
						{
							ID_ElemString*	tEl=new ID_ElemString(bufLAN.MainDomainName);
							if (bufLAN.MainDomainName[0]=='#')
								strcpy(Cli->Cli.MainDomainName,bufLAN.MainDomainName);
							if (Cli->Cli.DomainList==NULL)
							{
								Cli->Cli.DomainList=new ID_List8();
							}
							if (Cli->Cli.DomainList->Add(tEl)==-1)
							{
								delete tEl;
							}
						}
						Srv->GetRealTime(&Cli->Cli.lwTime.FTime);
					}
					Cli->Cli.crTime.Time=bufLAN.FTime.Time;
					Cli->Cli.Command=bufLAN.Command;
					Cli->Cli.CPUUsage=bufLAN.CPUUsage;
					Cli->Cli.MemUsage=bufLAN.MemUsage;
					Cli->Cli.Ver=bufLAN.Ver&0x1fff;
					if (bufLAN.Ver&0x4000)
					{
						if (bufLAN.Ver&0x2000)
							Cli->Cli.CPUMode=0x2+0x1;
						else
							Cli->Cli.CPUMode=0x1;
					}
					else
					{
						if (bufLAN.Ver&0x2000)
							Cli->Cli.CPUMode=0x2;
						else
							Cli->Cli.CPUMode=0;
					};

					if (bufLAN.Ver&0x8000)
					{
						Srv->GetRealTime(&Srv->TimeNetLock.FTime);
					}
					if (Cli->Cli.CPUUsage>=100.0) Cli->Cli.CPUUsage=100;
					if (Cli->Cli.CPUUsage<=0) Cli->Cli.CPUUsage=0;
					if (Cli->Cli.MemUsage>=100.0) Cli->Cli.MemUsage=100;
					if (Cli->Cli.MemUsage<=0) Cli->Cli.MemUsage=0;

					if (Srv->Timer(Cli->TimeAdd,60000))
						if ((Cli->Cli.Command!=SICom_Error)&&((Cli->Cli.Ver&0xfff)/100==mVersion/100)&&(bufLAN.Command!=SICom_Hardware))
						{
							Srv->NLAdd(0,Cli->Cli.IPAddr);
							Srv->NLAdd(1,Cli->Cli.IPAddr);
						}
						ProcLANCommand(Num);
				}
				else
				{
					bufLAN.Command=bufLAN.Command*2;
				}
			}
		}
		if (rc==sizeof( UDPPack2 ))
		{
			memcpy(&bufLAN2,&LanBuf,sizeof( UDPPack2 ));
			trc=rc;
			if (bufLAN2.Command==SICom_CPUUsage)
			{
				El_CliInt*	Cli=(El_CliInt*)CliLAN.Find((UCHAR*)&Num,sizeof(ULONG));
				if (Cli)
				{
					Cli->CPULoadsSrv=bufLAN2.SrvCPUUsage;
					for (int i=0;i<MaxID;i++)
					{
						Cli->CPULoadsBuff[i]=bufLAN2.CPUUsage[i];
						Cli->VirtCPULoadsBuff[i]=bufLAN2.VirtCPUUsage[i];
					}
				}
			}
			
		}
		MyUnLock(__FILE__,__LINE__,&Fl_RE,NULL);
	}
	else
	{
		MyUnLock(__FILE__,__LINE__,&Fl_RE,NULL);
		if (rc==-1) Sleep(10);
	}
	return trc;
};

void	Integrator::ProcLANCommand(UINT	Num)
{
	FileTime	tFTime;
	char		tTBuf[128];
	El_CliInt*	Cli=(El_CliInt*)CliLAN.Find((UCHAR*)&Num,sizeof(ULONG));
	if (Cli)
	switch(Cli->Cli.Command)
	{
	case SICom_TimeSyn:
		Srv->Fl_FSyn=0;
		break;
	case SICom_Connect:
		{
//			int rc=WaitForSingleObject(Srv->hmtxMainThr,INFINITE);
// 			MyLock(__FILE__,__LINE__,&Srv->hmtxMainThr,Srv);
// 			El_Tab*		ETbl=NULL;
// 			if (Srv->IDT_Neuron)
// 			{
// 				ETbl=Srv->IDT_Neuron->FindNested(&Anchor,AcsMd_Read,N_Domain,Cli->Cli.DomainName,N_Host,Cli->Cli.ShortHost,NULL);
// 			}
// 			if (ETbl)
// 			{
// 				Srv->IDT_Neuron->AddParam(&Anchor,ETbl,IDState,VolumeProcessed[IDVolProc_Setup]);
// 				Srv->IDT_Neuron->UnLockEl(&Anchor,ETbl);
// 			};
// 			MyUnLock(__FILE__,__LINE__,&Srv->hmtxMainThr,Srv);
			if (Num!=Srv->MyNum)
				Srv->ReInitUnit(Cli->Cli.ShortHost);
//			ReleaseSemaphore(Srv->hmtxMainThr,1,NULL);
		}
		break;
	case SICom_Error:
		if (NumLANSrv==MyLANNum)
			if (Cli->Cli.UID_CritMsg==0)
			{
//				MyLock(__FILE__,__LINE__,&Srv->hmtxMainThr,Srv);
//				int rc=WaitForSingleObject(Srv->hmtxMainThr,INFINITE);
				Srv->NLDel(0,Cli->Cli.IPAddr);
				Srv->NLDel(1,Cli->Cli.IPAddr);
				sprintf(tTBuf,"Neuron.exe Error HostName=%s",Cli->Cli.Host);
				if (Srv!=NULL)
					Cli->Cli.UID_CritMsg=Srv->SetCriticalError(NULL,tTBuf,"Critical","Neuron.exe");
				El_Tab*		ETbl=NULL;
				if (Srv->IDT_NeuronLocalSite)
				{
					if (Cli->Cli.DomainList)
					{
						FindStrList8	FSL8;
						ID_Elem*	tIDEl=Cli->Cli.DomainList->Begin(FSL8);
						while (tIDEl)
						{
							ETbl=Srv->IDT_NeuronLocalSite->FindNested(&Anchor,AcsMd_Read,N_Domain,tIDEl->Buff,N_Host,Cli->Cli.ShortHost,NULL);
							//							ETbl=Srv->IDT_NeuronLocalSite->AttainNested(Srv,AcsMd_Read,"%s%s",N_Host,Cli->Cli.Host,-1);
							tIDEl=Cli->Cli.DomainList->Next(FSL8);
						}
					}
				}
				if (ETbl)
				{
					Srv->IDT_NeuronLocalSite->AddParam(&Anchor,ETbl,IDState,VolumeProcessed[IDVolProc_Error]);
					Srv->IDT_NeuronLocalSite->UnLockEl(&Anchor,ETbl);
				};
//				MyUnLock(__FILE__,__LINE__,&Srv->hmtxMainThr,Srv);
//				ReleaseSemaphore(Srv->hmtxMainThr,1,NULL);
			}
		break;
	case SICom_Standalone:
		if (Num==MyLANNum)
			if (State==1)
			{	// Îòêëþ÷èòü ñâîé ñåðâåð
//				MyLock(__FILE__,__LINE__,&Srv->hmtxMainThr,Srv);
//				int rc=WaitForSingleObject(Srv->hmtxMainThr,INFINITE);
				Srv->OutLog("Delete Neuron Server");
				strcpy(Srv->UInfo.Mode,"Active");
				State=0;
				Srv->ModifUnit(Srv);
				Srv->RoleSwOff("Server");
				Srv->SendEvent(EV_StopServer,NULL,0);
//				MyUnLock(__FILE__,__LINE__,&Srv->hmtxMainThr,Srv);
//				ReleaseSemaphore(Srv->hmtxMainThr,1,NULL);
			}
		break;
	case SICom_ClientWait:
//		if (Num!=Srv->MyNum)
//			Srv->ReInitUnit(Cli->Cli.ShortHost);
	case SICom_ClientActive:
		// Àëãîðèòì âêëþ÷åíèÿ ñåðâåðà
		Srv->GetRealTime(&tFTime.FTime);
		if (tFTime.Time-laLANSvrTime.Time>500000000)
		{
			int i=-1;
			FindStrList8	FSL8;
			El_CliInt*	CliL=(El_CliInt*)CliLAN.Begin(FSL8);
			while (CliL)
			{
				if (CliL->Cli.fl!=0)
				{
					if (strcmp(CliL->Cli.MainDomainName,Srv->MainDomainName)==0)
					{
						if ((CliL->Cli.Command==SICom_ServerActive)||(CliL->Cli.Command==SICom_SuperVisorActive))
						{
							i=CliL->Addr;break;
						}
					};
				}
				CliL=(El_CliInt*)CliLAN.Next(FSL8);
			}
			if (i==-1)
			{
				// Íóæåí ñåðâåð !!!
				FSL8.DestroyStr();
				CliL=(El_CliInt*)CliLAN.Begin(FSL8);
				while (CliL)
				{
//					if (CliLAN[i])
					if (strcmp(CliL->Cli.MainDomainName,Srv->MainDomainName)==0)
					{
						if ((CliL->Cli.Command!=SICom_Error)&&(CliL->Cli.Command!=SICom_Standalone))
						{
							if (CliL->Addr==MyLANNum)
							{	// Ôîðìèðóåì ó ñåáÿ ñåðâåð
								if (State==0)
								{
//									MyLock(__FILE__,__LINE__,&Srv->hmtxMainThr,Srv);
//									int rc=WaitForSingleObject(Srv->hmtxMainThr,INFINITE);
									State=1;
									strcpy(Srv->UInfo.Mode,"Server");
									Srv->OutLog("Create Neuron Server");
									NumLANSrv=MyLANNum;
									Srv->Fl_ConnectSrv=1;
									// Ïåðåêîííåêòèòü êëèåíòîâ !!!
									Fl_ReConnect=1;
									if(Srv->RoleSwOn("Server")==0)
									{
										Srv->SendEvent(EV_StartServer,NULL,0);
									}
									else
									{
										strcpy(Srv->UInfo.Mode,"Active");
										State=0;
									}
									Srv->ModifUnit(Srv);
//									MyUnLock(__FILE__,__LINE__,&Srv->hmtxMainThr,Srv);
//									ReleaseSemaphore(Srv->hmtxMainThr,1,NULL);
								}
							}
							break;
						}
					};
					CliL=(El_CliInt*)CliLAN.Next(FSL8);
				}
			}
		}
//				if (Srv!=NULL)
//				{
//					if ((Num!=MyNum)&&(NumSrv!=MyNum))
//					{
//						Srv->ExtSynTime(Cli->Cli.crTime,1);
//					}
//				}
		if (Cli->Cli.UID_CritMsg!=0)
		{
			if (Srv!=NULL)
				Srv->DelCriticalError(Cli->Cli.UID_CritMsg);
			Cli->Cli.UID_CritMsg=0;
			if (((Cli->Cli.Ver&0xfff)/100==mVersion/100)&&(Cli->Cli.Command!=SICom_Hardware))
			{
				Srv->NLAdd(0,Cli->Cli.IPAddr);
				Srv->NLAdd(1,Cli->Cli.IPAddr);
			}
		}
		break;
	case SICom_SuperVisorActive:
		Srv->GetRealTime(&laLANSSvrTime.FTime);
		if (NumLANSSrv!=MyLANNum)
		{
			Srv->SmTime=(double)(laLANSSvrTime.Time-Cli->Cli.crTime.Time);
			Srv->Telemetr(NULL,"_SmTime",Srv->SmTime/10000.);
			Srv->ExtSynTime(Cli->Cli.crTime);
		}
		else 
			Srv->SmTime=0;
		if (State>=1)
		{
			if (Num<MyLANNum)
			{	// Îòêëþ÷èòü ñâîé ñåðâåð
				Srv->OutLog("Delete Neuron SVisor");
				State=1;
				strcpy(Srv->UInfo.Mode,"Server");
				Srv->ModifUnit(Srv);
			}
		}
		if (NumLANSSrv!=Num)
		{
			NumLANSSrv=Num;
//			Srv->Fl_ConnectSrv=1;
			// Ïåðåêîííåêòèòü êëèåíòîâ !!!
		}
		if (strcmp(Cli->Cli.MainDomainName,Srv->MainDomainName)!=0)
			break;
	case SICom_ServerWait:
	case SICom_ServerActive:
		if (strcmp(Cli->Cli.MainDomainName,Srv->MainDomainName)==0)
		{
			Srv->GetRealTime(&laLANSvrTime.FTime);
			if (NumLANSrv!=Num)
			{
				NumLANSrv=Num;
				Srv->Fl_ConnectSrv=1;
				// Ïåðåêîííåêòèòü êëèåíòîâ !!!
				Fl_ReConnect=1;
			}
			else
			{
				NumLANSrv=Num;
				Srv->Fl_ConnectSrv=1;
			}
			// Àëãîðèòì êîíòðîëÿ äâóõ ñåðâåðîâ
			if (State>=1)
			{
				if (Num<MyLANNum)
				{	// Îòêëþ÷èòü ñâîé ñåðâåð
					Srv->OutLog("Delete Neuron Server");
					State=0;
					strcpy(Srv->UInfo.Mode,"Active");
					Srv->ModifUnit(Srv);
					Srv->RoleSwOff("Server");
					Srv->SendEvent(EV_StopServer,NULL,0);
				}
			}
			if (Cli->Cli.UID_CritMsg!=0)
			{
				if (Srv!=NULL)
					Srv->DelCriticalError(Cli->Cli.UID_CritMsg);
				Cli->Cli.UID_CritMsg=0;
			}
			if (Cli->Cli.Command==SICom_SuperVisorActive)
				break;
			Srv->GetRealTime(&tFTime.FTime);
			if (tFTime.Time-laLANSSvrTime.Time>500000000)
			{
				int i=-1;
				FindStrList8	FSL8;
				El_CliInt*	CliL=(El_CliInt*)CliLAN.Begin(FSL8);
				while (CliL)
				{
					if (CliL->Cli.fl!=0)
					{
						if (strcmp(CliL->Cli.MainDomainName,Srv->MainDomainName)==0)
						{
							if (CliL->Cli.Command==SICom_SuperVisorActive)
							{
								i=CliL->Addr;break;
							}
						};
					}
					CliL=(El_CliInt*)CliLAN.Next(FSL8);
				}
				if (i==-1)
				{
					// Íóæåí ñåðâåð !!!
					FSL8.DestroyStr();
					CliL=(El_CliInt*)CliLAN.Begin(FSL8);
					while (CliL)
					{
						//					if (CliLAN[i])
						if (strcmp(CliL->Cli.MainDomainName,Srv->MainDomainName)==0)
						{
							if ((CliL->Cli.Command!=SICom_Error)&&(CliL->Cli.Command!=SICom_Standalone))
							{
								if (CliL->Addr==MyLANNum)
								{	// Ôîðìèðóåì ó ñåáÿ ñåðâåð
									if (State<=1)
									{
										//									MyLock(__FILE__,__LINE__,&Srv->hmtxMainThr,Srv);
										//									int rc=WaitForSingleObject(Srv->hmtxMainThr,INFINITE);
										State=2;
										strcpy(Srv->UInfo.Mode,"SVisor");
										Srv->OutLog("Create Neuron SVisor");
										NumLANSSrv=MyLANNum;
//										Srv->Fl_ConnectSrv=1;
										// Ïåðåêîííåêòèòü êëèåíòîâ !!!
										Fl_ReConnect=1;
										if(Srv->RoleSwOn("Server")==0)
										{
											Srv->SendEvent(EV_StartServer,NULL,0);
										}
										else
										{
											strcpy(Srv->UInfo.Mode,"Active");
											State=0;
										}
										Srv->ModifUnit(Srv);
										//									MyUnLock(__FILE__,__LINE__,&Srv->hmtxMainThr,Srv);
										//									ReleaseSemaphore(Srv->hmtxMainThr,1,NULL);
									}
								}
								break;
							}
						};
						CliL=(El_CliInt*)CliLAN.Next(FSL8);
					}
				}
			}
		}
		break;
	case SICom_DisConnect: 
		if (Cli->Cli.fl!=0)
		{
//			MyLock(__FILE__,__LINE__,&Srv->hmtxMainThr,Srv);
//			int rc=WaitForSingleObject(Srv->hmtxMainThr,INFINITE);
			El_Tab*		ETbl=NULL;
			if (Srv->IDT_NeuronLocalSite)
			{
				if (Cli->Cli.DomainList)
				{
					FindStrList8	FSL8;
					ID_Elem*	tIDEl=Cli->Cli.DomainList->Begin(FSL8);
					while (tIDEl)
					{
						ETbl=Srv->IDT_NeuronLocalSite->FindNested(&Anchor,AcsMd_Read,N_Domain,tIDEl->Buff,N_Host,Cli->Cli.ShortHost,NULL);
						//						ETbl=Srv->IDT_NeuronLocalSite->AttainNested(Srv,AcsMd_Read,"%s%s",N_Host,Cli->Cli.Host,-1);
						tIDEl=Cli->Cli.DomainList->Next(FSL8);
					}
				}
			}
			if (ETbl)
			{
				Srv->IDT_NeuronLocalSite->AddParam(&Anchor,ETbl,IDState,VolumeProcessed[IDVolProc_Off]);
				Srv->IDT_NeuronLocalSite->UnLockEl(&Anchor,ETbl);
			};
//			MyUnLock(__FILE__,__LINE__,&Srv->hmtxMainThr,Srv);
//			ReleaseSemaphore(Srv->hmtxMainThr,1,NULL);
			NumLANCli--;
			Srv->NLDel(0,Cli->Cli.IPAddr);
			Srv->NLDel(1,Cli->Cli.IPAddr);
			Cli->Cli.fl=0;
		}

		if (Cli->Cli.UID_CritMsg!=0)
		{
			if (Srv!=NULL)
				Srv->DelCriticalError(Cli->Cli.UID_CritMsg);
			Cli->Cli.UID_CritMsg=0;
		}
		CliLAN.Del(Cli);
//		delete	CliLAN[Num]; CliLAN[Num]=0;
		break;
	case SICom_Packet: 
		break;
	}
};

void	Integrator::DelHostFromDomain(char* HostName,char* DomainName)
{
	FindStrList8 FSL8;
	El_CliInt*	Cli=(El_CliInt*)CliLAN.Begin(FSL8);
	while (Cli)
	{
//		if (CliLAN[i])
		{
			if (strcmp(Cli->Cli.ShortHost,HostName)==NULL)
			{
				if (Cli->Cli.DomainList)
				{
					ID_Elem*	tIDEl=Cli->Cli.DomainList->Find((UCHAR*)DomainName,strlen(DomainName)+1);
					if (tIDEl)
						Cli->Cli.DomainList->Del(tIDEl);
				}
				break;
			}

		}
		Cli=(El_CliInt*)CliLAN.Next(FSL8);
	}
};

float	Integrator::GetLoadsCPU(UnitInfo* UI)
{
	float Loads=0.0;
	El_CliInt*	Cli=(El_CliInt*)CliLAN.Find((UCHAR*)&UI->IP,sizeof(uint));
	if (Cli)
	{
		if (UI->TractID==-1)
			Loads=Cli->Cli.CPUUsage;
		else
		{
			if (UI->TractID>=0x8000)
			{
				Loads=Cli->VirtCPULoadsBuff[UI->TractID-0x8000];
			}
			else
			{
				Loads=Cli->CPULoadsBuff[UI->TractID];
			}
		}
	}
	return Loads;
};

int	Integrator::SelectUnit(UnitInfo* UI,float LoadVol)
{
	int rc=0;
	El_CliInt*	Cli=(El_CliInt*)CliLAN.Find((UCHAR*)&UI->IP,sizeof(uint));
	if (Cli)
	{
		if (UI->TractID==-1)
			Cli->Cli.CPUUsage+=1;
		else
		{
			if (UI->TractID>=0x8000)
			{
				Cli->VirtCPULoadsBuff[UI->TractID-0x8000]+=LoadVol;
			}
			else
			{
				Cli->CPULoadsBuff[UI->TractID]+=LoadVol;
			}
		}
	}
	return rc;
};

int	Integrator::SetUnitLoad(UnitInfo* UI,float LoadVol)
{
	int rc=0;
	El_CliInt*	Cli=(El_CliInt*)CliLAN.Find((UCHAR*)&UI->IP,sizeof(uint));
	if (Cli)
	{
		if (UI->TractID==-1)
			Cli->Cli.CPUUsage+=1;
		else
		{
			Srv->Neuron->SetUsage(UI->TractID,LoadVol);
		}
	}
	return rc;
};

float	Integrator::GetLoadsRAM(UnitInfo* UI)
{
	float Loads=0.0;
	El_CliInt*	Cli=(El_CliInt*)CliLAN.Find((UCHAR*)&UI->IP,sizeof(uint));
	if (Cli)
		Loads=Cli->Cli.MemUsage;
	return Loads;
};

void	Integrator::ReInit()
{
	int					rc;
//	MyLock(__FILE__,__LINE__,&Fl_RE,NULL);
	FlStop=1;
//	delete CThread;
	delete CThreadPAN;
	delete CThreadLAN;
//	delete CThreadWAN;
	if (scLAN)
		closesocket(scLAN);
	if (scPAN)
		closesocket(scPAN);
// 	if (scWAN1)
// 		closesocket(scWAN1);
// 	if (scWAN2)
// 		closesocket(scWAN2);
	if (ssLAN)
		closesocket(ssLAN);
	if (ssPAN)
		closesocket(ssPAN);
// 	if (ssWAN)
// 		closesocket(ssWAN);
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
	ssPAN=ssLAN=scLAN=scPAN=NULL;	//=ssWAN=scWAN1=scWAN2
//	Status=0;
	Srv=GetServices();//Flag=0;FlagSyn=0;
//	Anchor.Init(Srv,this,0);
	Srv->GetRealTime(&RTime.FTime);	//=time(NULL);
//	Srv->GetRealTime(&Srv->TimeNetLock.FTime);
	NumberHost=0;	// !!!!!!!!!!!!!!!!!!!!!!
	int		tnPort=atoi(PortL);
	sprintf(PortL2,"%d",tnPort+1);
	tnPort=atoi(PortP);
	sprintf(PortP2,"%d",tnPort+1);
	tnPort=atoi(PortW);
	sprintf(PortW2,"%d",tnPort+1);

	FatalError=0;
	dontblock=1;
	dgramsz = 1440;
	SizeSource=sizeof(SourceLAN);
	NumLANCli=0;
	NumPANCli=0;
	NumWANCli=0;
	for (size_t i=0;i<256;i++)
	{
		CliPAN[i]=0;
//		CliLAN[i]=0;
// 		CliPAN[i]->IPAddr[0]=0;
// 		CliPAN[i]->Host[0]=0;
// 		CliPAN[i]->ShortHost[0]=0;
// 		CliPAN[i]->CPUUsage=0;
// 		CliPAN[i]->UID_CritMsg=0;
// 		CliLAN[i]->fl=0;
// 		CliLAN[i]->IPAddr[0]=0;
// 		CliLAN[i]->Host[0]=0;
// 		CliLAN[i]->ShortHost[0]=0;
// 		CliLAN[i]->CPUUsage=0;
// 		CliLAN[i]->UID_CritMsg=0;
		TimeAdd[i]=0;
	}
	if (Srv->M_Cfg->MData.Fl_LAN)
	{
		scLAN = udp_client( CreateBroadcastLANAddress(host),PortL, &peerLAN );		// Local server port
		rc=ioctlsocket( scLAN, FIONBIO, (PULONG)&dontblock);
		ssLAN = udp_server( inet_ntoa(Srv->IPAddrLAN),PortL );
		dontblock=0;
		rc=ioctlsocket( ssLAN, FIONBIO, (PULONG)&dontblock);
		dontblock=3000;
		rc=setsockopt(ssLAN,SOL_SOCKET,SO_RCVTIMEO,(char*)&dontblock,sizeof(BOOL));
	};
	if (Srv->M_Cfg->MData.Fl_PAN)
	{
		scPAN = udp_client( CreateBroadcastPANAddress(host),PortP, &peerPAN );		// Local server port
		rc=ioctlsocket( scPAN, FIONBIO, (PULONG)&dontblock);
		ssPAN = udp_server( inet_ntoa(Srv->IPAddrPAN),PortP );
		dontblock=0;
		rc=ioctlsocket( ssPAN, FIONBIO, (PULONG)&dontblock);
		dontblock=3000;
		rc=setsockopt(ssPAN,SOL_SOCKET,SO_RCVTIMEO,(char*)&dontblock,sizeof(BOOL));
	};
// 	if (Srv->M_Cfg->MData.Fl_WAN)
// 	{
// 		if (Srv->M_Cfg->MData.IP_WANIS1)
// 		{
// 			if((Srv->M_Cfg->MData.Fl_RunSICli==0)||(MyLANNum==NumLANSrv))
// 				scWAN1 = udp_client( inet_ntoa(*(in_addr*)&Srv->M_Cfg->MData.IP_WANIS1),Port4, &peerWAN1 );		// Local server port
// 			rc=ioctlsocket( scWAN1, FIONBIO, (PULONG)&dontblock);
// 		}
// 		if (Srv->M_Cfg->MData.IP_WANIS2)
// 		{
// 			if((Srv->M_Cfg->MData.Fl_RunSICli==0)||(MyLANNum==NumLANSrv))
// 				scWAN2 = udp_client( inet_ntoa(*(in_addr*)&Srv->M_Cfg->MData.IP_WANIS2),Port4, &peerWAN2 );		// Local server port
// 			rc=ioctlsocket( scWAN2, FIONBIO, (PULONG)&dontblock);
// 		}
// 		if (Srv->M_Cfg->MData.Fl_RunSI)
// 		{
// 			ssWAN = udp_server( inet_ntoa(Srv->IPAddrWAN),Port4 );
// 			dontblock=0;
// 			rc=ioctlsocket( ssWAN, FIONBIO, (PULONG)&dontblock);
// 			dontblock=3000;
// 			rc=setsockopt(ssWAN,SOL_SOCKET,SO_RCVTIMEO,(char*)&dontblock,sizeof(BOOL));
// 		}
// 	};

	State=0;	// Client
	UDPP.CPUUsage=0;
	Srv->GetRealTime(&laLANSvrTime.FTime);	//=time(NULL);
	Srv->GetRealTime(&laLANSSvrTime.FTime);	//=time(NULL);
	UDPP.HostName[0]=0;INetAdrPAN=INetAdrLAN=0;	//=INetAdrWAN
#ifndef Win9x
	gethostname (UDPP.HostName,255);
#endif
#ifndef Win9x
	INetAdrPAN=*(long*)&Srv->IPAddrPAN;
	INetAdrLAN=*(long*)&Srv->IPAddrLAN;
//	INetAdrWAN=*(long*)&Srv->IPAddrWAN;
	UDPP.WANADR=*(long*)&Srv->IPAddrWAN;
	strcpy(UDPP.HostName,Srv->ShortHostName);
//	strcpy(UDPP.DomainName,UInfo.DomainName);
#endif
	MyPANNum=*(3+(UCHAR*)(&INetAdrPAN));
	NumPANSrv=0;
	MyLANNum=INetAdrLAN;
	NumLANSrv=0;
	NumLANSSrv=0;
	Srv->Fl_ConnectSrv=0;
	UDPP.Command=SICom_Connect;
	UDPP.Ver=mVersion;
	UDPP.MemUsage=0;
	if (scPAN)
	{
		Srv->GetRealTime(&UDPP.FTime.FTime);
		rc = sendto( scPAN, (char*)&UDPP.Command, sizeof(UDPPack) /*strlen(UDPP.HostName)+5+2*sizeof(UDPP.CPUUsage)+sizeof(FileTime)*/, 0,( struct sockaddr * )&peerPAN, sizeof( struct sockaddr ) );
	};
	Sleep(50);
	if (scLAN)
	{
		Srv->GetRealTime(&UDPP.FTime.FTime);
		rc = sendto( scLAN, (char*)&UDPP.Command, sizeof(UDPPack) /*strlen(UDPP.HostName)+5+2*sizeof(UDPP.CPUUsage)+sizeof(FileTime)*/, 0,( struct sockaddr * )&peerLAN, sizeof( struct sockaddr ) );
	};
// 	Sleep(50);
// 	if (scWAN1)
// 	{
// 		Srv->GetRealTime(&UDPP.FTime.FTime);
// 		if((Srv->M_Cfg->MData.Fl_RunSICli==0)||(MyLANNum==NumLANSrv))
// 			rc = sendto( scWAN1, (char*)&UDPP.Command, sizeof(UDPPack) /*strlen(UDPP.HostName)+5+2*sizeof(UDPP.CPUUsage)+sizeof(FileTime)*/, 0,( struct sockaddr * )&peerWAN1, sizeof( struct sockaddr ) );
// 	};
// 	Sleep(50);
// 	if (scWAN2)
// 	{
// 		Srv->GetRealTime(&UDPP.FTime.FTime);
// 		if((Srv->M_Cfg->MData.Fl_RunSICli==0)||(MyLANNum==NumLANSrv))
// 			rc = sendto( scWAN2, (char*)&UDPP.Command, sizeof(UDPPack) /*strlen(UDPP.HostName)+5+2*sizeof(UDPP.CPUUsage)+sizeof(FileTime)*/, 0,( struct sockaddr * )&peerWAN2, sizeof( struct sockaddr ) );
// 	};
//	Fl_ReConnect=0;
//	CThread=new	ClassThread("SIFun",SIFun,(LPVOID)this,THREAD_PRIORITY_ABOVE_NORMAL);
	CThreadPAN=new	ClassThread("SIFunPAN",SIFunPAN,(LPVOID)this,THREAD_PRIORITY_ABOVE_NORMAL);
	CThreadLAN=new	ClassThread("SIFunLAN",SIFunLAN,(LPVOID)this,THREAD_PRIORITY_ABOVE_NORMAL);
//	CThreadWAN=new	ClassThread("SIFunWAN",SIFunWAN,(LPVOID)this,THREAD_PRIORITY_ABOVE_NORMAL);
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
	strcpy(Srv->UInfo.Mode,"Wait");
	Srv->MyNum=MyLANNum;
//	MyUnLock(__FILE__,__LINE__,&Fl_RE,NULL);
	FlStop=0;
};

// 
// int	Integrator::IdleWAN()
// {
// 	int trc=0;
// //	FileTime	tFTime;
// //	while (()>0)
// 	if (FlStop==1) return trc;
// 	if (Flag!=0) { return trc;};
// 	if (ssWAN==NULL) return trc;
// 	trc=0;
// 	rc = recvfrom( ssWAN, (char*)&buf, sizeof( buf ), 0, &SourceWAN, &SizeSource);
// 	MyLock(__FILE__,__LINE__,&Fl_RE,NULL);
// 	if (rc>0)
// 	{
// 		ULONG Num;
// //		UDPPack	tPack;
// 		Num=*(ULONG*)&SourceWAN.sa_data[2];
// 		if ((Num&Srv->M_Cfg->MData.IP_MaskWAN)==(Srv->M_Cfg->MData.IP_WAN&Srv->M_Cfg->MData.IP_MaskWAN))
// 		{
// 			El_CliInt*	Cli=AddWANCli(Num,&buf,GetInternetStandardAddress(SourceWAN));
// 			IDInfo*	IDI=Srv->GetNewIDI(Srv);
// 			Srv->InitCommIDI(IDI);
// 			IDI->pPList->SetParam(IDHostName,"*",2,Srv);
// 			IDI->pPList->SetParam(IDUnitName,N_Service,9,Srv);
// 			IDI->Command=Command_C_WANCli;
// 			INT64	TimeErr=Cli->Cli.crTime.Time-Cli->Cli.lwTime.Time;
// 			IDI->pPList->SetParam(IDID,&TimeErr,sizeof(INT64),Srv);
// 			IDI->Buf=(UCHAR*)&Cli->Cli;
// 			IDI->Cnt=sizeof(UDPCli);
// 			IDI->NumPack++;
// 			Srv->SendIDI(IDI,Srv,"WAN");
// 			Srv->DeleteIDI(IDI);	
// 		}
// 		MyUnLock(__FILE__,__LINE__,&Fl_RE,NULL);
// 	}
// 	else
// 	{
// 		MyUnLock(__FILE__,__LINE__,&Fl_RE,NULL);
// 		if (rc==-1) Sleep(10);
// 	}
// 	return trc;
// };
// 
El_CliInt*		Integrator::AddWANCli(UDPPack* buf)
{
	char		tTBuf[128];
	int			rc=-1;
	El_CliInt*	Cli=NULL;
//	if ((buf->WANADR&Srv->M_Cfg->MData.IP_MaskWAN)==(Srv->M_Cfg->MData.IP_WAN&Srv->M_Cfg->MData.IP_MaskWAN))
//	if ((buf->WANADR==Srv->M_Cfg->MData.IP_WANIS1)||(buf->WANADR==Srv->M_Cfg->MData.IP_WANIS2))
	{
		buf->HostName[39-sizeof(ULONG)]=0;
		buf->MainDomainName[39]=0;
		if ((buf->Command>=SICom_Connect)&&(buf->Command<=SICom_Packet))
		{
			Cli=(El_CliInt*)CliWAN.Find((UCHAR*)&buf->WANADR,sizeof(ULONG));

			if (Cli==NULL)
			{	
				Cli=new El_CliInt(buf->WANADR);
				NumWANCli++;
				Cli->Cli.fl=1;
				Srv->GetRealTime(&Cli->Cli.lwTime.FTime);
				strcpy(Cli->Cli.IPAddr, inet_ntoa(*(in_addr*)&buf->WANADR));
				sprintf(Cli->Cli.Host,"%s.%s",buf->HostName,buf->MainDomainName);
				strcpy(Cli->Cli.ShortHost,buf->HostName);
//				strcpy(Cli->Cli.MainDomainName,buf->MainDomainName);
				if (buf->MainDomainName[0]!=0)
				{
					ID_ElemString*	tEl=new ID_ElemString(buf->MainDomainName);
					if (buf->MainDomainName[0]=='#')
						strcpy(Cli->Cli.MainDomainName,buf->MainDomainName);
					if (Cli->Cli.DomainList==NULL)
						Cli->Cli.DomainList=new ID_List8();
					if (Cli->Cli.DomainList->Add(tEl)==-1)
					{
						delete tEl;
					}
				}
				rc=CliWAN.Add(Cli);
				if (((buf->Ver&0xfff)/100==mVersion/100)&&(buf->Command!=SICom_Hardware))
				{
					Srv->NLAdd(0,Cli->Cli.IPAddr);
					Srv->NLAdd(1,Cli->Cli.IPAddr);
				}
			}
			else
			{
//				Cli->Cli.fl=1;
				Srv->GetRealTime(&Cli->Cli.lwTime.FTime);
				sprintf(Cli->Cli.Host,"%s.%s",buf->HostName,buf->MainDomainName);
				strcpy(Cli->Cli.ShortHost,buf->HostName);
//				strcpy(Cli->Cli.MainDomainName,buf->MainDomainName);
				if (buf->MainDomainName[0]!=0)
				{
					ID_ElemString*	tEl=new ID_ElemString(buf->MainDomainName);
					if (buf->MainDomainName[0]=='#')
						strcpy(Cli->Cli.MainDomainName,buf->MainDomainName);
					if (Cli->Cli.DomainList==NULL)
						Cli->Cli.DomainList=new ID_List8();
					if (Cli->Cli.DomainList->Add(tEl)==-1)
					{
						delete tEl;
					}
				}
			}
			Cli->Cli.crTime.Time=buf->FTime.Time;
			Cli->Cli.Command=buf->Command;
			Cli->Cli.CPUUsage=buf->CPUUsage;
			Cli->Cli.MemUsage=buf->MemUsage;
//////////////////////////////////////////////////////////////////////////////////////
// Òîëüêî íà âðåìÿ îòëàäêè (íåîáõîëäèìî óäàëèòü!!!)
			Cli->Cli.Ver=buf->Ver&0x7fff;
				if (buf->Ver&0x8000)
				{
					Srv->GetRealTime(&Srv->TimeNetLock.FTime);
				}
//////////////////////////////////////////////////////////////////////////////////////
			if (Cli->Cli.CPUUsage>=100.0) Cli->Cli.CPUUsage=100;
			if (Cli->Cli.CPUUsage<=0) Cli->Cli.CPUUsage=0;
			if (Cli->Cli.MemUsage>=100.0) Cli->Cli.MemUsage=100;
			if (Cli->Cli.MemUsage<=0) Cli->Cli.MemUsage=0;
			
			if (Srv->Timer(Cli->TimeAdd,60000))
				if ((Cli->Cli.Command!=SICom_Error)&&((Cli->Cli.Ver&0xfff)/100==mVersion/100)&&(buf->Command!=SICom_Hardware))
				{
					Srv->NLAdd(0,Cli->Cli.IPAddr);
					Srv->NLAdd(1,Cli->Cli.IPAddr);
				}

			switch(Cli->Cli.Command)
			{
			case SICom_TimeSyn:
				Srv->Fl_FSyn=0;
				break;
			case SICom_Connect:
				{
//					MyLock(__FILE__,__LINE__,&Srv->hmtxMainThr,Srv);
//					int rc=WaitForSingleObject(Srv->hmtxMainThr,INFINITE);
					if (buf->MainDomainName[0]!=0)
					{
						El_Tab*		ETbl=NULL;
						if (Srv->IDT_NeuronLocalSite)
						{
							ETbl=Srv->IDT_NeuronLocalSite->FindNested(&Anchor,AcsMd_Read,N_Domain,buf->MainDomainName,N_Host,Cli->Cli.ShortHost,NULL);
						}
						if (ETbl)
						{
							Srv->IDT_NeuronLocalSite->AddParam(&Anchor,ETbl,IDState,VolumeProcessed[IDVolProc_Setup]);
							Srv->IDT_NeuronLocalSite->UnLockEl(&Anchor,ETbl);
						};
					}
//					MyUnLock(__FILE__,__LINE__,&Srv->hmtxMainThr,Srv);
//					ReleaseSemaphore(Srv->hmtxMainThr,1,NULL);
				}
				break;
			case SICom_Error:
//					if (NumWANSrv==MyWANNum)
					if (Cli->Cli.UID_CritMsg==0)
					{
//						MyLock(__FILE__,__LINE__,&Srv->hmtxMainThr,Srv);
//						int rc=WaitForSingleObject(Srv->hmtxMainThr,INFINITE);
						Srv->NLDel(0,Cli->Cli.IPAddr);
						Srv->NLDel(1,Cli->Cli.IPAddr);
						sprintf(tTBuf,"Neuron.exe Error HostName=%s",Cli->Cli.Host);
						if (Srv!=NULL)
							Cli->Cli.UID_CritMsg=Srv->SetCriticalError(NULL,tTBuf,"Critical","Neuron.exe");
						if (buf->MainDomainName[0]!=0)
						{
							El_Tab*		ETbl=NULL;
							if (Srv->IDT_NeuronLocalSite)
							{
								ETbl=Srv->IDT_NeuronLocalSite->FindNested(&Anchor,AcsMd_Read,N_Domain,buf->MainDomainName,N_Host,Cli->Cli.ShortHost,NULL);
	//							ETbl=Srv->IDT_NeuronLocalSite->AttainNested(Srv,AcsMd_Read,"%s%s",N_Host,Cli->Cli.Host,-1);
							}
							if (ETbl)
							{
								Srv->IDT_NeuronLocalSite->AddParam(&Anchor,ETbl,IDState,VolumeProcessed[IDVolProc_Error]);
								Srv->IDT_NeuronLocalSite->UnLockEl(&Anchor,ETbl);
							};
						};
//						MyUnLock(__FILE__,__LINE__,&Srv->hmtxMainThr,Srv);
//						ReleaseSemaphore(Srv->hmtxMainThr,1,NULL);
					}
				break;
			case SICom_Standalone:
/*					if (Num==MyNum)
					if (State==1)
					{	// Îòêëþ÷èòü ñâîé ñåðâåð
						int rc=WaitForSingleObject(Srv->hmtxMainThr,INFINITE);
						Srv->OutLog("Delete Neuron Server");
						strcpy(Srv->UInfo.Mode,"Active");
						State=0;
						Srv->ModifUnit(Srv);
						Srv->RoleSwOff("Server");
						Srv->SendEvent(EV_StopServer,NULL,0);
						ReleaseSemaphore(Srv->hmtxMainThr,1,NULL);
					}
*/					break;
			case SICom_ClientWait:
			case SICom_ClientActive:
				// Àëãîðèòì âêëþ÷åíèÿ ñåðâåðà
/*					Srv->GetRealTime(&tFTime.FTime);
				if (tFTime.Time-laLANSvrTime.Time>500000000)
				{
					int i;
					for ( i=0;i<256;i++)
					{
						if (CliLAN[i]->fl!=0)
						{
							if (CliLAN[i]->Command==SICom_ServerActive)
							{
								break;
							}
						};
					}
					if (i==256)
					{
						// Íóæåí ñåðâåð !!!
						for (size_t i=0;i<256;i++)
						{
							if (CliLAN[i]->fl!=0)
							{
								if ((CliLAN[i]->Command!=SICom_Error)&&(CliLAN[i]->Command!=SICom_Standalone))
								{
									if (i==MyNum)
									{	// Ôîðìèðóåì ó ñåáÿ ñåðâåð
										if (State==0)
										{
											int rc=WaitForSingleObject(Srv->hmtxMainThr,INFINITE);
											State=1;
											strcpy(Srv->UInfo.Mode,"Server");
											Srv->OutLog("Create Neuron Server");
											NumSrv=MyNum;
											// Ïåðåêîííåêòèòü êëèåíòîâ !!!
											Fl_ReConnect=1;
											if(Srv->RoleSwOn("Server")==0)
											{
												Srv->SendEvent(EV_StartServer,NULL,0);
											}
											else
												State=0;
											Srv->ModifUnit(Srv);
											ReleaseSemaphore(Srv->hmtxMainThr,1,NULL);
										}
									}
									break;
								}
							};
						}
					}
				}
*/					if (Cli->Cli.UID_CritMsg!=0)
				{
					if (Srv!=NULL)
						Srv->DelCriticalError(Cli->Cli.UID_CritMsg);
					Cli->Cli.UID_CritMsg=0;
					if (((Cli->Cli.Ver&0xfff)/100==mVersion/100)&&(Cli->Cli.Command!=SICom_Hardware))
					{
						Srv->NLAdd(0,Cli->Cli.IPAddr);
						Srv->NLAdd(1,Cli->Cli.IPAddr);
					}
				}
				break;
			case SICom_ServerWait:
			case SICom_ServerActive:
// 				Srv->GetRealTime(&laWANSvrTime.FTime);
// 				if (NumLANSrv==MyLANNum)
// 				{
// 					Srv->SmTime=(laLANSvrTime.Time-Cli->Cli.crTime.Time);
// 					Srv->Telemetr(NULL,"_SmTime",Srv->SmTime/10000.);
// 					Srv->ExtSynTime(Cli->Cli.crTime);
// 				}
// 					Srv->SmTime=0;
/*				if (NumSrv!=Num)
				{
					NumSrv=Num;
					// Ïåðåêîííåêòèòü êëèåíòîâ !!!
					Fl_ReConnect=1;
				}
				else
				{
					NumSrv=Num;
				}
				// Àëãîðèòì êîíòðîëÿ äâóõ ñåðâåðîâ
				if (State==1)
					if (Num<MyNum)
					{	// Îòêëþ÷èòü ñâîé ñåðâåð
						Srv->OutLog("Delete Neuron Server");
						State=0;
						strcpy(Srv->UInfo.Mode,"Active");
						Srv->ModifUnit(Srv);
						Srv->RoleSwOff("Server");
						Srv->SendEvent(EV_StopServer,NULL,0);
					}
*/					if (Cli->Cli.UID_CritMsg!=0)
				{
					if (Srv!=NULL)
						Srv->DelCriticalError(Cli->Cli.UID_CritMsg);
					Cli->Cli.UID_CritMsg=0;
				}
				break;
			case SICom_DisConnect: 
//				if (Cli->Cli.fl!=0)
				{
//					MyLock(__FILE__,__LINE__,&Srv->hmtxMainThr,Srv);
//					int rc=WaitForSingleObject(Srv->hmtxMainThr,INFINITE);
					if (buf->MainDomainName[0]!=0)
					{
						El_Tab*		ETbl=NULL;
						if (Srv->IDT_NeuronLocalSite)
						{
							ETbl=Srv->IDT_NeuronLocalSite->FindNested(&Anchor,AcsMd_Read,N_Domain,buf->MainDomainName,N_Host,Cli->Cli.ShortHost,NULL);
	//						ETbl=Srv->IDT_NeuronLocalSite->AttainNested(Srv,AcsMd_Read,"%s%s",N_Host,Cli->Cli.Host,-1);
						}
						if (ETbl)
						{
							Srv->IDT_NeuronLocalSite->AddParam(&Anchor,ETbl,IDState,VolumeProcessed[IDVolProc_Off]);
							Srv->IDT_NeuronLocalSite->UnLockEl(&Anchor,ETbl);
						};
					};
//					MyUnLock(__FILE__,__LINE__,&Srv->hmtxMainThr,Srv);
//					ReleaseSemaphore(Srv->hmtxMainThr,1,NULL);
					NumWANCli--;
					Srv->NLDel(0,Cli->Cli.IPAddr);
					Srv->NLDel(1,Cli->Cli.IPAddr);
//					Cli->Cli.fl=0;
					rc=CliWAN.Del(Cli);//!!!!!!!!!!!!!!!!!!!!!????????????????????

				}

				if (Cli->Cli.UID_CritMsg!=0)
				{
					if (Srv!=NULL)
						Srv->DelCriticalError(Cli->Cli.UID_CritMsg);
					Cli->Cli.UID_CritMsg=0;
				}
				break;
			case SICom_Packet: 
				break;
			}
		}
		else
		{
			buf->Command=buf->Command*2;
		}
	}
	return	Cli;
};
void	Integrator::Idle()
{
	int					rc;
	FileTime	tFTime;
//	char	tBuf[256];
	if (FlStop==1) return;
	if (Flag!=0) {Sleep (100); return;}
	MyLock(__FILE__,__LINE__,&Fl_RE,NULL);
	if (Srv->Fl_Repl==0)
	{
		Status=2;
		if (Srv->Fl_ReadyRepl>10)
			UDPP.Command=SICom_Standalone;
		else
			UDPP.Command=SICom_Connect;
	}
	else
	{
		if (State==0)
		{
			if (Srv->System_On)
			{
				if (Status!=2)
				{
					Srv->Fl_SGUI=1;
				}
				Status=2;
				UDPP.Command=SICom_ClientActive;
			}
			else
			{
				Status=1;
				if (Srv->Fl_ReadyRepl>10)
					UDPP.Command=SICom_ClientWait;
				else
					UDPP.Command=SICom_Connect;
			}
		}
		else
		{
// 			if (State!=1)
// 			{
// 				if (Srv->RoleSwOn("Server")==0)
// 				{
// 					Srv->SendEvent(EV_StartServer,NULL,0);
// 					strcpy(Srv->UInfo.Mode,"Server");
// 					Srv->ModifUnit(Srv);
// 				}
// 			}
//			State=1;
			if (Srv->System_On)
			{
				if (Status!=2)
				{
					Srv->Fl_SGUI=1;
				}
				Status=2;
				switch (State)
				{
				case 1:
					UDPP.Command=SICom_ServerActive;
					break;
				case 2:
					UDPP.Command=SICom_SuperVisorActive;
					break;

				}
			}
			else
			{
				Status=1;
				UDPP.Command=SICom_ServerWait;
			}
		}
	}
	if (Srv->Fl_NetLock==0)
		UDPP.Command=SICom_SerDecLock;

	Srv->GetRealTime(&tFTime.FTime);
	UDPP.FTime.Time=tFTime.Time;
	UDPP.Ver=mVersion;
	if (Srv->Fl_HardLock)
	{
		UDPP.Ver|=0x8000;
	}
#ifdef _WIN64
	UDPP.Ver|=0x4000;
#endif
#ifdef _DEBUG
	UDPP.Ver|=0x2000;
#endif
	if ((tFTime.Time-RTime.Time>500000000)||(FatalError!=0))
	{
		Status=3;
		UDPP.Command=SICom_Error;
	}
	if (Srv!=NULL)
	{
		UDPP.CPUUsage=Srv->CPUUsage;
		UDPP.MemUsage=Srv->MemUsage;
	}
	if (FlagSyn==1)
	{
		UDPP.Command=SICom_TimeSyn;
		FlagSyn=0;
	}
	UDPP.HostName[39-sizeof(ULONG)]=0;
	if (Srv->Neuron)
	{
		while (SendCnt<MaxID)
		{
			if (Srv->Neuron->Tract[SendCnt]!=NULL)
			{
				strcpy(UDPP.MainDomainName,Srv->Neuron->Tract[SendCnt]->UInfo.DomainName);
				SendCnt++;
				break;
			}
			else
				SendCnt++;
		}
		if (SendCnt>MaxID)
		{
			strcpy(UDPP.MainDomainName,Srv->MainDomainName);
			SendCnt=0;
		}
	}
	else
	{
		strcpy(UDPP.MainDomainName,Srv->MainDomainName);
	}
	if ((SendCnt==MaxID)&&(scLAN))
	{
		SendCnt++;
		El_CliInt*	Cli=(El_CliInt*)CliLAN.Find((UCHAR*)&MyLANNum,sizeof(ULONG));
		UDPP2.Command=SICom_CPUUsage;
		if (Srv->Neuron)
		{
			UDPP2.SrvCPUUsage=Srv->Neuron->SrvThread->Loads;
			if (Cli)
				Cli->CPULoadsSrv=UDPP2.SrvCPUUsage;
			for (int ik=0;ik<MaxID;ik++)
			{
				if (Srv->Neuron->ClThread[ik])
				{
					UDPP2.CPUUsage[ik]=Srv->Neuron->ClThread[ik]->Loads+Srv->Neuron->TractUsage[ik];
					if (UDPP2.CPUUsage[ik]>100.0)
						UDPP2.CPUUsage[ik]=100.0;
				}
				else
					UDPP2.CPUUsage[ik]=100.0;
				if (Srv->Neuron->DinThread[ik])
				{
					UDPP2.VirtCPUUsage[ik]=Srv->Neuron->DinThread[ik]->Loads+Srv->Neuron->DinTractUsage[ik];
					if (UDPP2.CPUUsage[ik]>100.0)
						UDPP2.CPUUsage[ik]=100.0;
				}
				else
					UDPP2.VirtCPUUsage[ik]=100.0;
				if (Cli)
				{
					Cli->CPULoadsBuff[ik]=UDPP2.CPUUsage[ik];
					Cli->VirtCPULoadsBuff[ik]=UDPP2.VirtCPUUsage[ik];
				}
			}
			rc = sendto( scLAN, (char*)&UDPP2.Command, sizeof(UDPPack2), 0,( struct sockaddr * )&peerLAN, sizeof( struct sockaddr ) );
		}
	}
	else
	if (scLAN)
	{
		rc = sendto( scLAN, (char*)&UDPP.Command, sizeof(UDPPack) /*strlen(UDPP.HostName)+5+2*sizeof(UDPP.CPUUsage)+sizeof(FileTime)*/, 0,( struct sockaddr * )&peerLAN, sizeof( struct sockaddr ) );
//		rc = sendto( scc, (char*)&UDPP.Command, sizeof(UDPPack) /*strlen(UDPP.HostName)+5+2*sizeof(UDPP.CPUUsage)+sizeof(FileTime)*/, 0,( struct sockaddr * )&peerc, sizeof( struct sockaddr ) );
//		Num=*(((UCHAR*)&Srv->IPAddrLAN)+3);
		El_CliInt*	Cli=(El_CliInt*)CliLAN.Find((UCHAR*)&Srv->IPAddrLAN,sizeof(ULONG));
//		*((char*)&buf+rc)=0;
		if (Cli==0)
		{	
			Cli=new El_CliInt(*(ulong*)&Srv->IPAddrLAN);
//			CliLAN[Num]=new UDPCli();
			CliLAN.Add(Cli);
			NumLANCli++;
			Cli->Cli.fl=1;
			Srv->GetRealTime(&Cli->Cli.lwTime.FTime);
			strcpy(Cli->Cli.IPAddr,inet_ntoa(Srv->IPAddrLAN));
			sprintf(Cli->Cli.Host,"%s.%s",UDPP.HostName,UDPP.MainDomainName);
			strcpy(Cli->Cli.ShortHost,UDPP.HostName);
//			strcpy(Cli->Cli.MainDomainName,UDPP.MainDomainName);
			if (UDPP.MainDomainName[0]!=0)
			{
				ID_ElemString*	tEl=new ID_ElemString(UDPP.MainDomainName);
				if (UDPP.MainDomainName[0]=='#')
					strcpy(Cli->Cli.MainDomainName,UDPP.MainDomainName);
				if (Cli->Cli.DomainList==NULL)
					Cli->Cli.DomainList=new ID_List8();
				if (Cli->Cli.DomainList->Add(tEl)==-1)
				{
					delete tEl;
				}
			}
			if (((UDPP.Ver&0xfff)/100==mVersion/100)&&(UDPP.Command!=SICom_Hardware))
			{
				Srv->NLAdd(0,Cli->Cli.IPAddr);
				Srv->NLAdd(1,Cli->Cli.IPAddr);
			}
		}
		else
		{
			sprintf(Cli->Cli.Host,"%s.%s",UDPP.HostName,UDPP.MainDomainName);
			strcpy(Cli->Cli.ShortHost,UDPP.HostName);
//			strcpy(Cli->Cli.MainDomainName,UDPP.MainDomainName);
			if (UDPP.MainDomainName[0]!=0)
			{
				ID_ElemString*	tEl=new ID_ElemString(UDPP.MainDomainName);
				if (UDPP.MainDomainName[0]=='#')
					strcpy(Cli->Cli.MainDomainName,UDPP.MainDomainName);
				if (Cli->Cli.DomainList==NULL)
					Cli->Cli.DomainList=new ID_List8();
				if (Cli->Cli.DomainList->Add(tEl)==-1)
				{
					delete tEl;
				}
			}
			Srv->GetRealTime(&Cli->Cli.lwTime.FTime);
		}
		Cli->Cli.crTime.Time=UDPP.FTime.Time;
		Cli->Cli.Command=UDPP.Command;
		Cli->Cli.CPUUsage=UDPP.CPUUsage;
		Cli->Cli.MemUsage=UDPP.MemUsage;
		Cli->Cli.Ver=UDPP.Ver&0x1fff;
		if (Cli->Cli.CPUUsage>=100.0) Cli->Cli.CPUUsage=100;
		if (Cli->Cli.CPUUsage<=0) Cli->Cli.CPUUsage=0;
		if (Cli->Cli.MemUsage>=100.0) Cli->Cli.MemUsage=100;
		if (Cli->Cli.MemUsage<=0) Cli->Cli.MemUsage=0;
		
		if (Srv->Timer(Cli->TimeAdd,60000))
			if ((Cli->Cli.Command!=SICom_Error)&&((Cli->Cli.Ver&0xfff)/100==mVersion/100)&&(UDPP.Command!=SICom_Hardware))
			{
				Srv->NLAdd(0,Cli->Cli.IPAddr);
				Srv->NLAdd(1,Cli->Cli.IPAddr);
			}
		ProcLANCommand(*(uint*)&Srv->IPAddrLAN);
	};
	MyUnLock(__FILE__,__LINE__,&Fl_RE,NULL);
	Sleep(50);
	MyLock(__FILE__,__LINE__,&Fl_RE,NULL);
	if (scPAN)
	{
		Srv->GetRealTime(&UDPP.FTime.FTime);
		rc = sendto( scPAN, (char*)&UDPP.Command, sizeof(UDPPack) /*strlen(UDPP.HostName)+5+2*sizeof(UDPP.CPUUsage)+sizeof(FileTime)*/, 0,( struct sockaddr * )&peerPAN, sizeof( struct sockaddr ) );
	};
	MyUnLock(__FILE__,__LINE__,&Fl_RE,NULL);
	Sleep(50);
 	MyLock(__FILE__,__LINE__,&Fl_RE,NULL);
 	if (Srv->Cli1)
 	{
 		Srv->GetRealTime(&UDPP.FTime.FTime);
 		if((Srv->M_Cfg->MData.Fl_RunSICli==0)||(MyLANNum==NumLANSrv))
			Srv->SIPack(Srv->Cli1,SYS_UDPPack,(uchar*)&UDPP.Command, sizeof(UDPPack));
// 			rc = sendto( scWAN1, (char*)&UDPP.Command, sizeof(UDPPack) /*strlen(UDPP.HostName)+5+2*sizeof(UDPP.CPUUsage)+sizeof(FileTime)*/, 0,( struct sockaddr * )&peerWAN1, sizeof( struct sockaddr ) );

	};
	if (Srv->Cli2)
	{
		Srv->GetRealTime(&UDPP.FTime.FTime);
		if((Srv->M_Cfg->MData.Fl_RunSICli==0)||(MyLANNum==NumLANSrv))
			Srv->SIPack(Srv->Cli2,SYS_UDPPack,(uchar*)&UDPP.Command, sizeof(UDPPack));
//			rc = sendto( scWAN2, (char*)&UDPP.Command, sizeof(UDPPack) /*strlen(UDPP.HostName)+5+2*sizeof(UDPP.CPUUsage)+sizeof(FileTime)*/, 0,( struct sockaddr * )&peerWAN2, sizeof( struct sockaddr ) );
	};
	if (Srv->WANSISrv)
	{
		Srv->GetRealTime(&UDPP.FTime.FTime);
		if((Srv->M_Cfg->MData.Fl_RunSICli==0)||(MyLANNum==NumLANSrv))
			Srv->SIPack(Srv->WANSISrv,-1,SYS_UDPPack,(uchar*)&UDPP.Command, sizeof(UDPPack));
	};
	MyUnLock(__FILE__,__LINE__,&Fl_RE,NULL);
// 	if (ssWAN)
// 	{
// 		El_CliInt*	tCli=AddWANCli(*(ULONG*)&Srv->IPAddrWAN,&UDPP,inet_ntoa(Srv->IPAddrWAN));
// //		El_CliInt*	tCli=(El_CliInt*)Cli.Find((UCHAR*)&Srv->IPAddrWAN,sizeof(ULONG));
// 		if (tCli)
// 		{
// 			IDInfo*	IDI=Srv->GetNewIDI(Srv);
// 			Srv->InitCommIDI(IDI);
// 			IDI->pPList->SetParam(IDHostName,"*",2,Srv);
// 			IDI->pPList->SetParam(IDUnitName,N_Service,9,Srv);
// 			IDI->Command=Command_C_WANCli;
// 			IDI->Buf=(UCHAR*)&tCli->Cli;
// 			IDI->Cnt=sizeof(UDPCli);
// 			IDI->NumPack++;
// 			Srv->SendIDI(IDI,Srv,"WAN");
// 			Srv->DeleteIDI(IDI);	
// 		}
// 	}
// 	Sleep(50);
	MyLock(__FILE__,__LINE__,&Fl_RE,NULL);
	for (size_t i=0;i<256;i++)
	{
		if (CliPAN[i])
		{
			FileTime tTime;
			Srv->GetRealTime(&tTime.FTime);
			if (Srv->System_On)
				if (tTime.Time-CliPAN[i]->lwTime.Time>30*KtimeS)	// 10 c  1c=100 ns * 10 000 000
				{	// Çàâèñ
					NumPANCli--;
					Srv->NLDel(0,CliPAN[i]->IPAddr);
					Srv->NLDel(1,CliPAN[i]->IPAddr);
					CliPAN[i]->fl=0;
					delete	CliPAN[i]; CliPAN[i]=0;
				}
		};
	}
	El_CliInt*	Cli;
	FindStrList8	FSL8;
	Cli=(El_CliInt*)CliLAN.Begin(FSL8);
	while (Cli)
	{
		FileTime tTime;
		Srv->GetRealTime(&tTime.FTime);
		if (Srv->System_On)
			if (tTime.Time-Cli->Cli.lwTime.Time>30*KtimeS)	// 10 c  1c=100 ns * 10 000 000
			{	// Çàâèñ
				NumLANCli--;
				Srv->NLDel(0,Cli->Cli.IPAddr);
				Srv->NLDel(1,Cli->Cli.IPAddr);
				Cli->Cli.fl=0;
				CliLAN.Del(Cli);
//				delete	CliLAN[i]; CliLAN[i]=0;
			}
		Cli=(El_CliInt*)CliLAN.Next(FSL8);
	};
	FSL8.DestroyStr();
	Cli=(El_CliInt*)CliWAN.Begin(FSL8);
	while (Cli)
	{
//		if (Cli->Cli.fl!=0)
		{
			FileTime tTime;
			Srv->GetRealTime(&tTime.FTime);
			if (Srv->System_On)
				if (tTime.Time-Cli->Cli.lwTime.Time>30*KtimeS)	// 10 c  1c=100 ns * 10 000 000
				{	// Çàâèñ
					NumWANCli--;
					Srv->NLDel(0,Cli->Cli.IPAddr);
					Srv->NLDel(1,Cli->Cli.IPAddr);
					CliWAN.Del(Cli);
//					Cli->Cli.fl=0;
				}
		};
		Cli=(El_CliInt*)CliWAN.Next(FSL8);
	}
// Checking error servers
//	if ((Srv->M_Cfg->MData.Fl_LAN)&&(ssLAN==NULL))
//	{
//		{size_t iasm=1;}
//	}
//

	MyUnLock(__FILE__,__LINE__,&Fl_RE,NULL);
//	Srv->ModifUnit(Srv);
	if(Fl_ReInit==1)
	{
		ReInit();
		Fl_ReInit=0;
	}
	else
		Sleep(800);
};
			
Integrator::~Integrator()
{
	int					rc;
	delete CThread;
	UDPP.Command=SICom_DisConnect;
	Srv->GetRealTime(&UDPP.FTime.FTime);
	if (scPAN)
	{
		rc = sendto( scPAN, (char*)&UDPP.Command, sizeof(UDPPack) /*strlen(UDPP.HostName)+5+2*sizeof(UDPP.CPUUsage)+sizeof(FileTime)*/, 0,( struct sockaddr * )&peerPAN, sizeof( struct sockaddr ) );
	}
	delete CThreadPAN;
	if (scLAN)
	{
		rc = sendto( scLAN, (char*)&UDPP.Command, sizeof(UDPPack) /*strlen(UDPP.HostName)+5+2*sizeof(UDPP.CPUUsage)+sizeof(FileTime)*/, 0,( struct sockaddr * )&peerLAN, sizeof( struct sockaddr ) );
//		rc = sendto( scc, (char*)&UDPP.Command, sizeof(UDPPack) /*strlen(UDPP.HostName)+5+2*sizeof(UDPP.CPUUsage)+sizeof(FileTime)*/, 0,( struct sockaddr * )&peerc, sizeof( struct sockaddr ) );
	}
	delete CThreadLAN;
// 	if (scWAN1)
// 	{
// 		rc = sendto( scWAN1, (char*)&UDPP.Command, sizeof(UDPPack) /*strlen(UDPP.HostName)+5+2*sizeof(UDPP.CPUUsage)+sizeof(FileTime)*/, 0,( struct sockaddr * )&peerWAN1, sizeof( struct sockaddr ) );
// 	}
// 	if (scWAN2)
// 	{
// 		rc = sendto( scWAN2, (char*)&UDPP.Command, sizeof(UDPPack) /*strlen(UDPP.HostName)+5+2*sizeof(UDPP.CPUUsage)+sizeof(FileTime)*/, 0,( struct sockaddr * )&peerWAN2, sizeof( struct sockaddr ) );
// 	}
// 	delete CThreadWAN;
	FlStop=1;
//	Sleep(1000);
	if (scPAN)
		closesocket(scPAN);
	if (scLAN)
		closesocket(scLAN);
//	if (scc)
//		closesocket(scc);
// 	if (scWAN1)
// 		closesocket(scWAN1);
// 	if (scWAN2)
// 		closesocket(scWAN2);
	if (ssPAN)
		closesocket(ssPAN);
	if (ssLAN)
		closesocket(ssLAN);
// 	if (ssWAN)
// 		closesocket(ssWAN);
 	for (size_t i=0;i<256;i++)
 	{
//  		if (CliLAN[i])
// 			delete	CliLAN[i];
 		if (CliPAN[i])
			delete	CliPAN[i];
 	}
	El_CliInt*	Cli;
	FindStrList8	FSL8;
	Cli=(El_CliInt*)CliLAN.Begin(FSL8);
	while (Cli)
	{
		CliLAN.Del(Cli);
		Cli=(El_CliInt*)CliLAN.Next(FSL8);
	}
	FSL8.DestroyStr();
	Cli=(El_CliInt*)CliWAN.Begin(FSL8);
	while (Cli)
	{
		CliWAN.Del(Cli);
		Cli=(El_CliInt*)CliWAN.Next(FSL8);
	}
};


char* GetInternetStandardAddress(struct sockaddr host)
{
	struct sockaddr_in* Host=(struct sockaddr_in*)&host; 
	return inet_ntoa(Host->sin_addr);
}

char* CreateBroadcastPANAddress(char* host)
{
	PCHAR p; 
	char temp[256];
	struct sockaddr_in sap;
	sap.sin_addr = GetServices()->IPAddrPAN;
	strcpy(host,inet_ntoa(sap.sin_addr));
	strcpy(temp,host);
	p=strtok(temp,".");
	p=strtok(NULL,".");
	p=strtok(NULL,".");
	p=strtok(NULL,".");
	strcpy(host+(p-temp),"255");
	return host;
};
char* CreateBroadcastLANAddress(char* host)
{
	PCHAR p; 
	char temp[256];
	struct sockaddr_in sap;
	sap.sin_addr = GetServices()->IPAddrLAN;
	strcpy(host,inet_ntoa(sap.sin_addr));
	strcpy(temp,host);
	p=strtok(temp,".");
	p=strtok(NULL,".");
	p=strtok(NULL,".");
	if (GetServices()->M_Cfg->MData.LANType)
	{
		strcpy(host+(p-temp),"255.255");
	}
	else
	{
		p=strtok(NULL,".");
		strcpy(host+(p-temp),"255");
	}
	return host;
};
char* CreateBroadcastWANAddress(char* host)
{
	PCHAR p; 
	char temp[256];
	struct sockaddr_in sap;
	sap.sin_addr = GetServices()->IPAddrWAN;
	strcpy(host,inet_ntoa(sap.sin_addr));
	strcpy(temp,host);
	p=strtok(temp,".");
	p=strtok(NULL,".");
	p=strtok(NULL,".");
	p=strtok(NULL,".");
	strcpy(host+(p-temp),"255");
	return host;
};

/* set_address - fill in a sockaddr_in structure */
// set_address( "192.168.44.6", "9000", &local, "udp" );
static void set_address( char *hname, char *sname,
	struct sockaddr_in *sap, char *protocol )
{
	struct servent *sp;
	struct hostent *hp;
	char *endptr;
	short port;

	bzero( sap, sizeof( *sap ) );
	sap->sin_family = AF_INET;
	if ( hname != NULL )
	{
		//if ( !inet_aton( hname, &sap->sin_addr ) )
//		{
			hp = gethostbyname( hname );
			if ( hp == NULL )
			{
				int err=WSAGetLastError();
//				error( 1, 0, "unknown host: %s\n", hname );
			}
			else
 				sap->sin_addr = *( struct in_addr * )hp->h_addr;
//		sap->sin_addr = GetServices()->IPAddr;
//		}
	}
	else
		sap->sin_addr.s_addr = htonl( INADDR_ANY );
	port = (short)strtol( sname, &endptr, 0 );
	if ( *endptr == '\0' )
		sap->sin_port = htons( port );
	else
	{
		sp = getservbyname( sname, protocol );
		if ( sp == NULL )
		{
//			error( 1, 0, "unknown service: %s\n", sname );
		}
		else
			sap->sin_port = sp->s_port;
	}
}

// ---------------- TCP server function ----------------

/* tcp_server - set up for a TCP server */
SOCKET tcp_server( char *hname, char *sname );
SOCKET tcp_server( char *hname, char *sname )
{
	struct sockaddr_in local;
	SOCKET s;
	const int on = 1;

	set_address( hname, sname, &local, "tcp" );
	s = socket( AF_INET, SOCK_STREAM, 0 );
	if ( !isvalidsock( s ) ){};
//		error( 1, errno, "socket call failed" );

	if ( setsockopt( s, SOL_SOCKET, SO_REUSEADDR,
		( char * )&on, sizeof( on ) ) ){};
//		error( 1, errno, "setsockopt failed" );

	if ( bind( s, ( struct sockaddr * ) &local,
		sizeof( local ) ) ){};
//		error( 1, errno, "bind failed" );

	if ( listen( s, 1 ) ){};
//		error( 1, errno, "listen failed" );

	return s;
}

// ---------------- TCP client function ----------------

/* tcp_client - set up for a TCP client */
SOCKET tcp_client( char *hname, char *sname );
SOCKET tcp_client( char *hname, char *sname )
{
	struct sockaddr_in peer;
	SOCKET s;

	set_address( hname, sname, &peer, "tcp" );
	s = socket( AF_INET, SOCK_STREAM, 0 );
	if ( !isvalidsock( s ) ){};
//		error( 1, errno, "socket call failed" );

	if ( connect( s, ( struct sockaddr * )&peer,
		sizeof( peer ) ) ){};
//		error( 1, errno, "connect failed" );

	return s;
}

// ---------------- UDP server skelet ----------------

/* udp_server - set up a UDP server */
SOCKET udp_server( char *hname, char *sname );
SOCKET udp_server( char *hname, char *sname )
{
	SOCKET s;
	int		rc;
	struct sockaddr_in local;

	set_address( hname, sname, &local, "udp" );
	s = socket( AF_INET, SOCK_DGRAM, 0 );
	if ( !isvalidsock( s ) ){};
//		error( 1, errno, "socket call failed" );
	if ( rc=bind( s, ( struct sockaddr * ) &local,
		sizeof( local ) ) ){};
//		error( 1, errno, "bind failed" );
	return s;
}

// ---------------- UDP client function ----------------

/* udp_client - set up a UDP client */
SOCKET udp_client( char *hname, char *sname,
	struct sockaddr_in *sap );
SOCKET udp_client( char *hname, char *sname,
	struct sockaddr_in *sap )
{
	SOCKET s;

	set_address( hname, sname, sap, "udp" );
	s = socket( AF_INET, SOCK_DGRAM, 0 );
	if ( !isvalidsock( s ) ){};
//		error( 1, errno, "socket call failed" );
	return s;
}

BandleList::BandleList(int Size)
{
	sprintf(SemName,"BndlL%d",(int)this);
//	hmtx=OpenSemaphore(SEMAPHORE_ALL_ACCESS	,TRUE,SemName);
//	if (hmtx==NULL)
//		hmtx=CreateSemaphore(NULL,1,1,SemName);
	NumElem=0;
	Srv=GetServices();
	SizeBuf=Size;
	if (SizeBuf!=0)
	{
		DataBuf=(UCHAR*)Srv->malloc(SizeBuf);
	}
	else
	{
		DataBuf=NULL;
	}
//	Fl_MT=0;
	PoiRd=0;
	PoiWr=0;
	PoiEl=0;
};

void	BandleList::AddElem(UCHAR* Buf,size_t ttCnt)
{
	MyLock(__FILE__,__LINE__,&Fl_MT,NULL);
//	WaitForSingleObject(hmtx,INFINITE);
	if (PoiWr+ttCnt+sizeof(int)>SizeBuf)
	{
		UCHAR*	tDataBuf;	
		if (DataBuf==NULL)
			tDataBuf=(UCHAR*)Srv->malloc(PoiWr+10*(ttCnt+sizeof(int)));
		else
			tDataBuf=(UCHAR*)Srv->realloc(DataBuf,PoiWr+10*(ttCnt+sizeof(int)));
		if (tDataBuf)
		{
			SizeBuf=PoiWr+10*(ttCnt+sizeof(int));	// Àëëîêàöèÿ â 10 ðàç áîëüøå ÷åì íàäî
			DataBuf=tDataBuf;
		}
		else
		{
			Srv->SetCriticalError(NULL,"Error malloc BL AddEl","CriticalError","ClasterMem");
			return;
		}
	}
	*(int*)(DataBuf+PoiWr)=ttCnt;
	if (ttCnt!=0)
		memcpy(DataBuf+PoiWr+sizeof(int),Buf,ttCnt);
	PoiWr+=ttCnt+sizeof(int);
	NumElem++;
	MyUnLock(__FILE__,__LINE__,&Fl_MT,NULL);//Fl_MT=0;
//	ReleaseSemaphore(hmtx,1,NULL); 
};

int		BandleList::FindFirst(UCHAR* &Buf,size_t &ttCnt)
{
	int	rc;
	MyLock(__FILE__,__LINE__,&Fl_MT,NULL);
//	WaitForSingleObject(hmtx,INFINITE);
	PoiRd=0;
	PoiEl=0;
	rc=PoiWr-PoiRd;
	if (rc>0)
	{
		ttCnt=*(int*)(DataBuf+PoiRd);
		Buf=DataBuf+PoiRd+sizeof(int);
		PoiRd+=(ttCnt+sizeof(int));
	}
	else rc=0;
	MyUnLock(__FILE__,__LINE__,&Fl_MT,NULL);//Fl_MT=0;
//	ReleaseSemaphore(hmtx,1,NULL); 
	return rc;
};

int		BandleList::FindFirst(UCHAR* &Buf,size_t &ttCnt,BandleFind* BFind)
{
	int	rc;
	MyLock(__FILE__,__LINE__,&Fl_MT,NULL);
//	WaitForSingleObject(hmtx,INFINITE);
	BFind->PoiRd=0;
	BFind->PoiEl=0;
	rc=PoiWr-BFind->PoiRd;
	if (rc>0)
	{
		ttCnt=*(int*)(DataBuf+BFind->PoiRd);
		Buf=DataBuf+BFind->PoiRd+sizeof(int);
		BFind->PoiRd+=(ttCnt+sizeof(int));
	}
	else rc=0;
	MyUnLock(__FILE__,__LINE__,&Fl_MT,NULL);//Fl_MT=0;
//	ReleaseSemaphore(hmtx,1,NULL); 
	return rc;
};

int		BandleList::FindNext(UCHAR* &Buf,size_t &ttCnt)
{
	int	rc;
	MyLock(__FILE__,__LINE__,&Fl_MT,NULL);
//	WaitForSingleObject(hmtx,INFINITE);
	rc=PoiWr-PoiRd;
	if (rc>0)
	{
		ttCnt=*(int*)(DataBuf+PoiRd);
		Buf=DataBuf+PoiRd+sizeof(int);
		PoiEl=PoiRd;
		PoiRd+=(ttCnt+sizeof(int));
//		PoiEl+=(ttCnt+sizeof(int));
	}
	else
	{
		ttCnt=0;
		rc=0;
	};
	MyUnLock(__FILE__,__LINE__,&Fl_MT,NULL);//Fl_MT=0;
//	ReleaseSemaphore(hmtx,1,NULL); 
	return rc;
};

int		BandleList::FindNext(UCHAR* &Buf,size_t &ttCnt,BandleFind* BFind)
{
	int	rc;
	MyLock(__FILE__,__LINE__,&Fl_MT,NULL);
//	WaitForSingleObject(hmtx,INFINITE);
	rc=PoiWr-BFind->PoiRd;
	if (rc>0)
	{
		//if ((BFind->PoiEl+*(int*)(DataBuf+BFind->PoiEl))!=BFind->PoiRd)
		//	{size_t iasm=1;}
		ttCnt=*(int*)(DataBuf+BFind->PoiRd);
		BFind->PoiEl=BFind->PoiRd;
		Buf=DataBuf+BFind->PoiRd+sizeof(int);
		BFind->PoiRd+=(ttCnt+sizeof(int));
	}
	else
	{
		ttCnt=0;
		rc=0;
	}
	MyUnLock(__FILE__,__LINE__,&Fl_MT,NULL);//Fl_MT=0;
//	ReleaseSemaphore(hmtx,1,NULL); 
	return rc;
};

int		BandleList::DelElem(UCHAR* &Buf,size_t &ttCnt)
{
	int	rc;
	MyLock(__FILE__,__LINE__,&Fl_MT,NULL);
//	WaitForSingleObject(hmtx,INFINITE);
	rc=PoiWr-PoiEl;
	if (rc>0)
	{
		int	ReSz=*(int*)(DataBuf+PoiEl)+sizeof(int);
		if ((ReSz<SizeBuf)&&(ReSz>0))
		{
			PoiWr-=ReSz;
			memcpy(DataBuf+PoiEl,DataBuf+PoiEl+ReSz,rc-ReSz);
			rc=PoiWr-PoiEl;
			if (rc>0)
			{
				ttCnt=*(int*)(DataBuf+PoiEl);
				Buf=DataBuf+PoiEl+sizeof(int);
			}
			else rc=0;
		}
		else
		{
			PoiWr=PoiEl;
			rc=0;ttCnt=0;Buf=NULL;
		}
		NumElem--;
	}
	else rc=0;
	MyUnLock(__FILE__,__LINE__,&Fl_MT,NULL);//Fl_MT=0;
//	ReleaseSemaphore(hmtx,1,NULL); 
	return rc;
};


UCHAR*	BandleList::GetDataBuf(size_t &ttCnt)
{
	ttCnt=PoiWr;
	return DataBuf;
};

void	BandleList::SetDataBuf(UCHAR* Buf,size_t ttCnt)
{
	UCHAR*	tDataBuf;
	MyLock(__FILE__,__LINE__,&Fl_MT,NULL);
//	WaitForSingleObject(hmtx,INFINITE);
	if (DataBuf==NULL)
	{
		tDataBuf=(UCHAR*)Srv->malloc(ttCnt+sizeof(int));
	}
	else
	{
		tDataBuf=(UCHAR*)Srv->realloc(DataBuf,ttCnt+sizeof(int));
	}
	if (tDataBuf!=NULL)
	{
		DataBuf=tDataBuf;
		memcpy(DataBuf,Buf,ttCnt);
		SizeBuf=ttCnt+sizeof(int);
		PoiWr=ttCnt;
		PoiRd=0;PoiEl=0;
	}
	else
	{
		Srv->SetCriticalError(NULL,"Error malloc BL SetDataBuf","CriticalError","ClasterMem");
	}
	MyUnLock(__FILE__,__LINE__,&Fl_MT,NULL);//Fl_MT=0;
//	ReleaseSemaphore(hmtx,1,NULL); 
};

int	BandleList::LoadFromFile(int FH,int Offset)
{
	int	rc=0;
	int	RLen;
	UCHAR*	tDataBuf;
	long	pos;
	CHAR	tBuf[1024];
	MyLock(__FILE__,__LINE__,&Fl_MT,NULL);
//	WaitForSingleObject(hmtx,INFINITE);
	PoiWr=0;
	if (FH!=-1)
	{
		pos = lseek( FH, Offset, SEEK_SET );
		if( pos != -1L )
		{
			PoiWr=0;
			RLen=read(FH,&tBuf,1024);
			if (RLen!=0)
			{
				if (PoiWr+RLen>SizeBuf)
				{
					if (DataBuf==NULL)
						tDataBuf=(UCHAR*)Srv->malloc(PoiWr+RLen);
					else
						tDataBuf=(UCHAR*)Srv->realloc(DataBuf,PoiWr+RLen);
					if (tDataBuf)
					{
						SizeBuf=PoiWr+RLen;
						DataBuf=tDataBuf;
					}
					else
					{
						Srv->SetCriticalError(NULL,"Error malloc BL LoadFromFile","CriticalError","ClasterMem");
						return 0;
					}
				}
				memcpy(DataBuf+PoiWr,tBuf,RLen);
				PoiWr+=RLen;
			}
			while (RLen==1024)
			{
				RLen=read(FH,&tBuf,1024);
				if (RLen!=0)
				{
					if (PoiWr+RLen>SizeBuf)
					{
						if (DataBuf==NULL)
							tDataBuf=(UCHAR*)Srv->malloc(PoiWr+RLen);
						else
							tDataBuf=(UCHAR*)Srv->realloc(DataBuf,PoiWr+RLen);
						if (tDataBuf)
						{
							SizeBuf=PoiWr+RLen;
							DataBuf=tDataBuf;
						}
						else
						{
							Srv->SetCriticalError(NULL,"Error malloc BL LoadFromFile2","CriticalError","ClasterMem");
							return 0;
						}
					}
					memcpy(DataBuf+PoiWr,tBuf,RLen);
					PoiWr+=RLen;
				}
			};
		}
		else rc=pos;
	}
	else rc=FH;

	PoiRd=0;
	PoiEl=0;
	rc=PoiWr-PoiRd;
	while (rc>0)
	{
		UCHAR*	Buf;
		int		Cnt;
		Cnt=*(int*)(DataBuf+PoiRd);
		Buf=DataBuf+PoiRd+sizeof(int);
		rc=PoiWr-PoiRd-(Cnt+sizeof(int));
		NumElem++;
		if (rc>0)
		{
			PoiRd+=(Cnt+sizeof(int));
			PoiEl+=(Cnt+sizeof(int));
		};
	}
	PoiRd=0;PoiEl=0;
	MyUnLock(__FILE__,__LINE__,&Fl_MT,NULL);//Fl_MT=0;
//	ReleaseSemaphore(hmtx,1,NULL); 
	return rc;
};

int	BandleList::SaveToFile(int FH,int Offset)
{
	int		rc=0;
	long	pos;
	MyLock(__FILE__,__LINE__,&Fl_MT,NULL);
//	WaitForSingleObject(hmtx,INFINITE);
	if ((FH!=-1)&&(DataBuf!=NULL))
	{
		pos = lseek( FH, Offset, SEEK_SET );
		if( pos != -1L )
		{
			write(FH,DataBuf,PoiWr);
			rc=chsize( FH,Offset+PoiWr);
		}
		else
			rc=pos;
	}
	else rc=FH;
	MyUnLock(__FILE__,__LINE__,&Fl_MT,NULL);//Fl_MT=0;
//	ReleaseSemaphore(hmtx,1,NULL); 
	return rc;
};

void	BandleList::DelAll(void)
{
	MyLock(__FILE__,__LINE__,&Fl_MT,NULL);
//	WaitForSingleObject(hmtx,INFINITE);
	if (DataBuf!=NULL)
	{
		Srv->free(DataBuf);DataBuf=NULL; SizeBuf=0;
	}
//	if (SizeBuf!=0)
//	{
//		DataBuf=(UCHAR*)Srv->malloc(SizeBuf);
//	}
//	else
//	{
//		DataBuf=NULL;
//	}
	PoiRd=0;
	PoiWr=0;
	PoiEl=0;
	NumElem=0;
	MyUnLock(__FILE__,__LINE__,&Fl_MT,NULL);//Fl_MT=0;
//	ReleaseSemaphore(hmtx,1,NULL); 
};	

int		BandleList::DelLast(UCHAR* &Buf,size_t &Cnt)
{
	int	rc;
	MyLock(__FILE__,__LINE__,&Fl_MT,NULL);
//	WaitForSingleObject(hmtx,INFINITE);
	PoiRd=0;
	PoiEl=0;
	Buf=NULL;Cnt=0;
	rc=PoiWr-PoiRd;
	while (rc>0)
	{
		Cnt=*(int*)(DataBuf+PoiRd);
		Buf=DataBuf+PoiRd+sizeof(int);
		rc=PoiWr-PoiRd-(Cnt+sizeof(int));
		if (rc>0)
		{
			PoiRd+=(Cnt+sizeof(int));
			PoiEl+=(Cnt+sizeof(int));
		};
	}
	MyUnLock(__FILE__,__LINE__,&Fl_MT,NULL);//Fl_MT=0;
//	ReleaseSemaphore(hmtx,1,NULL); 
	if (Cnt)
		DelElem(Buf,Cnt);
	return rc;
};

BandleList::~BandleList()
{
	if (DataBuf!=NULL)
	{
		Srv->free(DataBuf);DataBuf=NULL;SizeBuf=0;
	}
//	CloseHandle(hmtx);
};
/*
bool	BLDelElem::Find(ReplPack *PPack)
{
	UCHAR	*tPPack;
	int		tCnt;
	int		rc;
	int		i=0;
	rc=FindFirst(tPPack,tCnt);
	while (rc!=0)
	{
		i++;
//		if (i==0x46)
//			{size_t iasm=1;}
		if (tCnt!=sizeof(ReplPack))
		{
			DelAll();
		}
		if (((ReplPack *)tPPack)->UID==PPack->UID)
		{
			if (((ReplPack *)tPPack)->TimeModif.Time<PPack->TimeModif.Time)
			{
//				return TRUE;
				rc=DelElem(tPPack,tCnt);
				continue;
			}
			else
				return TRUE;
		}
		rc=FindNext(tPPack,tCnt);
	}
	return FALSE;
};

void	BLDelElem::Add(ReplPack *PPack)
{
//	return;
	UCHAR	*tPPack;
	int		tCnt;
	int		rc;
	rc=FindFirst(tPPack,tCnt);
	while (rc!=0)
	{
		if (((ReplPack *)tPPack)->UID==PPack->UID)
		{
			if (PPack->TimeModif.Time>((ReplPack *)tPPack)->TimeModif.Time)
				((ReplPack *)tPPack)->TimeModif.Time=PPack->TimeModif.Time;
			return;
		}
		rc=FindNext(tPPack,tCnt);
	}
	AddElem((UCHAR*)PPack,sizeof(ReplPack));
};
*/
void	BLDelElem::DelOnTime(INT64 Time,IDTable* pIDT)
{	// Time in second
	FileTime	tTime;
	ID_Elem64	*tElP;
	Srv->GetRealTime(&tTime.FTime);	
	FindStrList8	FSL8;
	tElP=(ID_Elem64*)DelList.Begin(FSL8);
	while (tElP!=0)
	{
		if (tTime.Time/KtimeS-tElP->FT.Time/KtimeS>=Time)
		{
			DelElemID(tElP->Vol);
		}
		tElP=(ID_Elem64*)DelList.Next(FSL8);
	}

};
void	BLDelElem::Add(INT64 ID)
{
	ID_Elem64 *tEl=new ID_Elem64(ID);
	if (DelList.Add(tEl)==-1)
	{
		delete tEl;
	}
	Srv->Telemetr(NULL,"BDL_Num_Elem",(double)DelList.NumElem,this);
};

void	BLDelElem::DelElemID(INT64 ID)
{
	ID_Elem64 *pElD=(ID_Elem64*)DelList.Find((UCHAR*)&ID,sizeof(INT64));
	if (pElD)
		DelList.Del(pElD);
};

bool	BLDelElem::Find(INT64 ID)
{
	ID_Elem64 *pElD=(ID_Elem64*)DelList.Find((UCHAR*)&ID,sizeof(INT64));
	if (pElD)
	{
		return TRUE;
	}
	else
		return FALSE;
};

int	BLDelElem::CopyEl()
{
	int		rc=0;
	UCHAR	*tPPack;
	size_t		tCnt;
	rc=FindFirst(tPPack,tCnt);
	while (rc!=0)
	{
		ID_Elem64 *tEl=new ID_Elem64(((ID_IDTime*)tPPack)->ID);
		tEl->FT.Time=((ID_IDTime*)tPPack)->FT.Time;
		if (DelList.Add(tEl)==-1)
		{
			delete tEl;
		}
		rc=FindNext(tPPack,tCnt);
	}
	DelAll();
	return rc;	
};

int	BLDelElem::SaveEl()
{
	int		rc=0;
	ID_Elem64	*tElD;
	ID_IDTime	tPPack;
//	int		tCnt;
//	DelAll();
	FindStrList8	FSL8;
	tElD=(ID_Elem64*)DelList.Begin(FSL8);
	while (tElD!=0)
	{
		tPPack.ID=tElD->Vol;
		tPPack.FT.Time=tElD->FT.Time;
		AddElem((UCHAR*)&tPPack,sizeof(ID_IDTime));
		tElD=(ID_Elem64*)DelList.Next(FSL8);
	}
	return rc;	
};

bool	BLDelPrm::Find(PrmPack *PPack)
{
	UCHAR	*tPPack;
	size_t		tCnt;
	int		rc;
//	char	*tBuf;
	rc=FindFirst(tPPack,tCnt);
	while (rc!=0)
	{
		if (((PrmPack *)tPPack)->UID==PPack->UID)
		{
			if (strcmp(((PrmPack *)tPPack)->PDec.Prm,PPack->PDec.Prm)==0)
			{
				if (10000000+((PrmPack *)tPPack)->TimeModif.Time<PPack->TimeModif.Time)
				{
//					return TRUE;
//					tBuf=cTime(&((PrmPack *)tPPack)->Time);
//					tBuf=cTime(&PPack->Time);
					rc=DelElem(tPPack,tCnt);
					continue;
				}
				else
					return TRUE;
			}
		}
		rc=FindNext(tPPack,tCnt);
	}
	return FALSE;
};

void	BLDelPrm::Add(PrmPack *PPack)
{
	UCHAR	*tPPack;
	size_t		tCnt;
	int		rc;
	rc=FindFirst(tPPack,tCnt);
	while (rc!=0)
	{
		if (((PrmPack *)tPPack)->UID==PPack->UID)
		{
			if (strcmp(((PrmPack *)tPPack)->PDec.Prm,PPack->PDec.Prm)==0)
			{
				if (PPack->TimeModif.Time>((PrmPack *)tPPack)->TimeModif.Time)
					((PrmPack *)tPPack)->TimeModif.Time=PPack->TimeModif.Time;
				return;
			}
		}
		rc=FindNext(tPPack,tCnt);
	}
	AddElem((UCHAR*)PPack,sizeof(PrmPack));
};

void	BLDelPrm::DelOnTime(INT64 Time)
{	// Time in second
	UCHAR	*tPPack;
	size_t		tCnt;
	int		rc;
	FileTime	tTime;
	GetSystemTimeAsFileTime(&tTime.FTime);	
	rc=FindFirst(tPPack,tCnt);
	while (rc!=0)
	{
		if (tTime.Time-((PrmPack *)tPPack)->TimeModif.Time>=Time*10000000)
		{
			// Del Elem (Ýëåìåíò ñòàðûé)
			rc=DelElem(tPPack,tCnt);
//			continue;
		}
		rc=FindNext(tPPack,tCnt);
	}
};

int		DamperProc(void* Obj,UCHAR* tBuf,size_t Cnt)
{
	return 0;
};

		Damper::Damper()
{
//	int	i;
	Fl_Lock=0;SumSize=0;OptSize=0;CntInside=0;
#ifndef _NOSRV
	Srv=GetServices();
#endif
	CM_Wr=CM_Rd=NULL;
	PDec=NULL;
	pObj=NULL;
	DmpProc=&DamperProc;
	SizeCMdl=SizeCMax=RealSize=0;
	BufMaxSize=-1;
	NumCM=0;NumCMM=1;
	CrMSG=0;
	sprintf(SemName,"Damp%d",(int)this);
//	hmtx=OpenSemaphore(SEMAPHORE_ALL_ACCESS	,TRUE,SemName);
//	if (hmtx==NULL)
//		hmtx=CreateSemaphore(NULL,1,1,SemName);
//	Fl_MTi=0;Fl_MTo=0;
	Fl_Clean=0;
};
		Damper::Damper(int MaxSize)
{
//	int	i;
	SumSize=0;OptSize=0;CntInside=0;
#ifndef _NOSRV
	Srv=GetServices();
#endif
	CM_Wr=CM_Rd=NULL;
	PDec=NULL;
	pObj=NULL;
	DmpProc=&DamperProc;
	SizeCMdl=SizeCMax=RealSize=0;
	BufMaxSize=MaxSize;
	NumCM=0;NumCMM=1;
	CrMSG=0;
	sprintf(SemName,"Damp%d",(int)this);
//	hmtx=OpenSemaphore(SEMAPHORE_ALL_ACCESS	,TRUE,SemName);
//	if (hmtx==NULL)
//		hmtx=CreateSemaphore(NULL,1,1,SemName);
//	Fl_MTi=0;Fl_MTo=0;
	Fl_Clean=0;

};

void	Damper::Clear() // Âñå ïî÷èñòèòü
{
//	UCHAR*	tBuf;
//	int		tCnt;
	MyLock(__FILE__,__LINE__,&Fl_MT,PDec);
//	WaitForSingleObject(hmtx,INFINITE);
	SumSize=0;
	if (CM_Wr)	// óíè÷òîæèòü öåïî÷êó êëàñòåðîâ
	{
		if (Fl_Lock==1)
		{
			Fl_Clean=1;
		}
		else
		{
			ClasterMem*	TmpCM1;
			ClasterMem*	TmpCM2=CM_Rd;
			TmpCM2->PreviousCM->NextCM=NULL;	// Ðàçîðâàòü öåïî÷êó êëàñòåðîâ
			TmpCM2->PreviousCM=NULL;			// Ðàçîðâàòü öåïî÷êó êëàñòåðîâ
			while (TmpCM2)
			{
//				if (TmpCM2->Fl_Rd==0)
				{
					TmpCM1=TmpCM2->NextCM;RealSize-=TmpCM2->Size;
					delete TmpCM2;
					NumCM--;
					TmpCM2=TmpCM1;

				}
// 				else
// 				{
// 					int jj=0;
// 					ReleaseSemaphore(hmtx,1,NULL); 
// 					while (Fl_Lock)
// 					{
// 						Sleep(10);
// 						if (jj++>100)
// 						{
// 							Fl_Lock=0;
// 							break;
// 						}
// 					}
// 					WaitForSingleObject(hmtx,INFINITE);
// 					TmpCM2->Fl_Rd=0;TmpCM2->Rd=TmpCM2->Wr;
// 				}
			}
			CM_Rd=CM_Wr=NULL;
		}
	}
	CntInside=0;
	MyUnLock(__FILE__,__LINE__,&Fl_MT,PDec);//Fl_MTi=0;
//	ReleaseSemaphore(hmtx,1,NULL); 
};
void	Damper::SetStartSize(int Size) // Óñòàíîâèòü íà÷àëüíûé ðàçìåð áóôåðà
{
	SizeCMax=Size;
};
void	Damper::SetPrm(Decoder* pDec, void* Obj, int (*DmpPrc)(void*,UCHAR*,size_t))
{
	if (pDec)
	{
		PDec=pDec;
		pObj=Obj;DmpProc=DmpPrc;
	}
#ifndef _NOSRV
	else
	{
		Srv->SetCriticalError(NULL,"SetPrm:pDec=NULL","Warning","Damper");
	};
#endif
};
int		Damper::Data(UCHAR* Buf,size_t Cnt)	// Çàïèñàòü äàííûå â áóôåð
{
	int	tCnt=Cnt;
	int	rc=0;
	UCHAR*	tBuf=Buf;
	CntInside+=Cnt;
	SumSize+=Cnt;
	if (Cnt==0) return 0;
	MyLock(__FILE__,__LINE__,&Fl_MT,PDec);
//	MyLock(__FILE__,__LINE__,&Fl_MTi,PDec);
//	WaitForSingleObject(hmtx,INFINITE);
	if(OptSize<Cnt) OptSize=(float)Cnt;
	if (SizeCMax<OptSize*NumCMM)
	{
		SizeCMax=(int)(OptSize*NumCMM/32);
		SizeCMax=(SizeCMax+1)*32;
	}
	else
	{
		if ((SizeCMax>2*OptSize*NumCMM)&&(SizeCMax>32))
			SizeCMax-=32;
	};

	if (CM_Wr==NULL)
	{
		CM_Wr=new ClasterMem(SizeCMax,PDec);
		if(CM_Wr->Buf==NULL)
		{
			delete CM_Wr;
			SizeCMax=tCnt;
			CM_Wr=new ClasterMem(SizeCMax,PDec);
		}
		RealSize+=SizeCMax;NumCM++;
#ifndef _NOSRV
		if (Srv)
			Srv->GetRealTime(&TimeMemAlloc.FTime);
#else
		GetSystemTimeAsFileTime(&TimeMemAlloc.FTime);
#endif
		CM_Wr->NextCM=CM_Wr;CM_Wr->PreviousCM=CM_Wr; // Çàêîëüöåâàòü â öåïî÷êó
		CM_Rd=CM_Wr; // Íàñòðîèòü ÷òåíèå
	}
	while (CM_Wr->Size-CM_Wr->Wr<tCnt)
	{
/*		if ((CM_Wr->Wr==CM_Wr->Rd)&&(CM_Wr->Size<tCnt))
		{
			// Óäàëèòü êëàñòåð
			// Âñòàâèòü äëÿ çàïèñè
			ClasterMem*	TmpCM1=new ClasterMem(SizeCMax,PDec);NumCM++;NumCMM=(int)sqrt((double)NumCM);Srv->GetRealTime(&TimeMemAlloc.FTime);
			TmpCM1->NextCM=CM_Wr->NextCM;
			TmpCM1->NextCM->PreviousCM=TmpCM1;
			TmpCM1->PreviousCM=CM_Wr->PreviousCM;
			CM_Wr->PreviousCM->NextCM=TmpCM1;
			delete CM_Wr;NumCM--;			// Âñòàâèòü äëÿ çàïèñè
			CM_Wr=TmpCM1;
		}
*/		tCnt-=(CM_Wr->Size-CM_Wr->Wr);
		memcpy(CM_Wr->Buf+CM_Wr->Wr,tBuf,CM_Wr->Size-CM_Wr->Wr);tBuf+=(CM_Wr->Size-CM_Wr->Wr);CM_Wr->Wr+=(CM_Wr->Size-CM_Wr->Wr);
		CM_Wr->Fl_Wr=1;	// Çàêðûòûé áëîê äëÿ çàïèñè
		if (CM_Wr->NextCM->Fl_Wr==0)
		{
			CM_Wr=CM_Wr->NextCM;
		}
		else
		{	// Âñòàâèòü äëÿ çàïèñè
			ClasterMem*	TmpCM1=new ClasterMem(SizeCMax,PDec);
			if(TmpCM1->Buf==NULL)
			{
				delete TmpCM1;
				SizeCMax=tCnt;
				TmpCM1=new ClasterMem(SizeCMax,PDec);
			}
			RealSize+=SizeCMax;NumCM++;NumCMM=(int)sqrt((double)NumCM);
#ifndef _NOSRV
			Srv->GetRealTime(&TimeMemAlloc.FTime);
#else
			GetSystemTimeAsFileTime(&TimeMemAlloc.FTime);
#endif
//			Srv->Telemetr(PDec,"Damper N_CM",(double)NumCM,this);
//			Srv->Telemetr(PDec,"Damper SizeCMax",(double)SizeCMax,this);
			TmpCM1->NextCM=CM_Wr->NextCM;
			TmpCM1->NextCM->PreviousCM=TmpCM1;
			TmpCM1->PreviousCM=CM_Wr;
			CM_Wr->NextCM=TmpCM1;
			CM_Wr=TmpCM1;
		}
	}
	memcpy(CM_Wr->Buf+CM_Wr->Wr,tBuf,tCnt);CM_Wr->Wr+=tCnt;
	if (CM_Wr->Wr==CM_Wr->Size)
	{
		CM_Wr->Fl_Wr=1;	// Çàêðûòûé áëîê äëÿ çàïèñè
		if (CM_Wr->NextCM->Fl_Wr==0)
		{
			CM_Wr=CM_Wr->NextCM;
		}
		else
		{	// Âñòàâèòü äëÿ çàïèñè
			ClasterMem*	TmpCM1=new ClasterMem(SizeCMax,PDec);
			if(TmpCM1->Buf==NULL)
			{
				delete TmpCM1;
				SizeCMax=tCnt;
				TmpCM1=new ClasterMem(SizeCMax,PDec);
			}
			RealSize+=SizeCMax;NumCM++;NumCMM=(int)sqrt((double)NumCM);
#ifndef _NOSRV
			Srv->GetRealTime(&TimeMemAlloc.FTime);
#else
			GetSystemTimeAsFileTime(&TimeMemAlloc.FTime);
#endif
//			Srv->Telemetr(PDec,"Damper N_CM",(double)NumCM,this);
//			Srv->Telemetr(PDec,"Damper SizeCMax",(double)SizeCMax,this);
			TmpCM1->NextCM=CM_Wr->NextCM;
			TmpCM1->NextCM->PreviousCM=TmpCM1;
			TmpCM1->PreviousCM=CM_Wr;
			CM_Wr->NextCM=TmpCM1;
			CM_Wr=TmpCM1;
		}
	}
	FileTime	OperTime;
#ifndef _NOSRV
	Srv->GetRealTime(&OperTime.FTime);
#else
	GetSystemTimeAsFileTime(&OperTime.FTime);
#endif
	if (OperTime.Time-TimeMemAlloc.Time>KtimeM)
	{
		TimeMemAlloc.Time=OperTime.Time;
		if ((CM_Wr->NextCM->Wr==CM_Wr->NextCM->Rd)&&(CM_Wr->NextCM!=CM_Rd))
		{	// Óäàëèòü êëàñòåð
			ClasterMem*	TmpCM1=CM_Wr->NextCM;
			TmpCM1->NextCM->PreviousCM=CM_Wr;
			CM_Wr->NextCM=TmpCM1->NextCM;
			RealSize-=TmpCM1->Size;delete TmpCM1;NumCM--;
//			Srv->Telemetr(PDec,"Damper N_CM",(double)NumCM,this);
//			Srv->Telemetr(PDec,"Damper SizeCMax",(double)SizeCMax,this);
		}
	}
	MyUnLock(__FILE__,__LINE__,&Fl_MT,PDec);//Fl_MTi=0;
//	ReleaseSemaphore(hmtx,1,NULL); 
	return rc;
};
int		Damper::OData() // âûçîâ îñóùåñòâëÿåòñÿ èç OData()	CallBack function
{
	int	rc=0;
	MyLock(__FILE__,__LINE__,&Fl_MT,PDec);
//	WaitForSingleObject(hmtx,INFINITE);
	if (CM_Rd)
	{
		if (SumSize)
		{
			OptSize=OptSize*0.9+(SumSize/10);
			SumSize=0;
		}
		while(CM_Rd->Rd!=CM_Rd->Wr)
		{
			tWr=CM_Rd->Wr;
			int SizeRd=tWr-CM_Rd->Rd;
			if (SizeRd)
			{
				CM_Rd->Fl_Rd=1;
				CntInside-=SizeRd;
				MyUnLock(__FILE__,__LINE__,&Fl_MT,PDec);//Fl_MTo=0;
//				ReleaseSemaphore(hmtx,1,NULL); 
				DmpProc(pObj,CM_Rd->Buf+CM_Rd->Rd,SizeRd);
				MyLock(__FILE__,__LINE__,&Fl_MT,PDec);
//				WaitForSingleObject(hmtx,INFINITE);
				rc+=tWr-CM_Rd->Rd;
				CM_Rd->Rd=tWr;tWr=0;
			};
			if ((CM_Rd->Rd==CM_Rd->Size)||((CM_Rd->Rd==CM_Rd->Wr)&&(CM_Rd->Fl_Wr!=0)))
			{	// Âû÷èòàëè âñå!!!
				CM_Rd->Fl_Rd=0;
				CM_Rd->Fl_Wr=0;CM_Rd->Rd=CM_Rd->Wr=0;
				CM_Rd=CM_Rd->NextCM;
			}
			else
			{
				CM_Rd=CM_Rd;
			};
		}
	}
	MyUnLock(__FILE__,__LINE__,&Fl_MT,PDec);//Fl_MTo=0;
//	ReleaseSemaphore(hmtx,1,NULL); 
	return rc;
};
int		Damper::LockOData(UCHAR* &Buf,size_t &Cnt) // Ïîäãîòîâèòü äàííûå äëÿ îáðàáîòêè (çàëî÷èâ ïðè ýòîì áóôåð)
{
	int	rc=0;
	MyLock(__FILE__,__LINE__,&Fl_MT,PDec);
//	WaitForSingleObject(hmtx,INFINITE);
	if (CM_Rd)
	{
		if (SumSize)
		{
			OptSize=OptSize*0.9+0.1*(float)SumSize;
			SumSize=0;
		}
		if(CM_Rd->Rd!=CM_Rd->Wr)
		{
			tWr=CM_Rd->Wr;
			int SizeRd=tWr-CM_Rd->Rd;
			if (SizeRd)
			{
				CM_Rd->Fl_Rd=1;
				Buf=CM_Rd->Buf+CM_Rd->Rd;Cnt=SizeRd;
				CntInside-=Cnt;
				rc+=tWr-CM_Rd->Rd;
			};
		}
	}
	MyUnLock(__FILE__,__LINE__,&Fl_MT,PDec);//Fl_MTo=0;
//	ReleaseSemaphore(hmtx,1,NULL); 
	return rc;
};
void	Damper::UnLockOData() // Îñâîáîäèòü äàííûå äëÿ ïåðåçàïèñè
{
	MyLock(__FILE__,__LINE__,&Fl_MT,PDec);
//	WaitForSingleObject(hmtx,INFINITE);
	if (CM_Rd)
	{
		if(CM_Rd->Rd!=tWr)
		{
//			int	tWr=CM_Rd->Wr;
			int SizeRd=tWr-CM_Rd->Rd;
			if (SizeRd)
			{
				CM_Rd->Rd=tWr;
			};
			if ((CM_Rd->Rd==CM_Rd->Size)||((CM_Rd->Rd==CM_Rd->Wr)&&(CM_Rd->Fl_Wr!=0)))
			{	// Âû÷èòàëè âñå!!!
				CM_Rd->Fl_Rd=0;
				CM_Rd->Fl_Wr=0;CM_Rd->Rd=CM_Rd->Wr=0;
				CM_Rd=CM_Rd->NextCM;
				tWr=0;
			}
			else
			{
				CM_Rd=CM_Rd;
			};
		}
	}
	MyUnLock(__FILE__,__LINE__,&Fl_MT,PDec);//Fl_MTo=0;
//	ReleaseSemaphore(hmtx,1,NULL); 
};

void	Damper::ReSize(int Act)	// Îïòèìèçàöèÿ
{
};
		Damper::~Damper()
{
	Clear();
//	CloseHandle(hmtx);
};

		DamperPS::DamperPS()
{
		HndlP.SockHead.Syn=Syn1;CntInside=0;
		CntPackIn=CntPackOut=0;
		HndCE=0;
		Fl_Clean=0;
};

		DamperPS::~DamperPS()
{
	Clear();
}

int		DamperPS::Data(UCHAR *Buf,size_t Cnt,int NumCli)
{
	int rc=0;
	if (Cnt==0)
		return rc;
	MyLock(__FILE__,__LINE__,&Fl_MT,PDec);
//	WaitForSingleObject(hmtx,INFINITE);
	CntInside+=Cnt;
	HndlP.SockHead.Len=Cnt;
	HndlP.CntDebug=++CntPackIn;
	if (NumCli!=-1)
	{
		HndlP.SockHead.CntPack=NumCli;
	}
	size_t	tCnt=Cnt+sizeof(HndlP);
	if (SizeCMax<tCnt*NumCMM)
	{
		SizeCMax=tCnt*NumCMM;
	}
	else
	{
		if ((SizeCMax>2*tCnt*NumCMM)&&(SizeCMax>32))
			SizeCMax-=16;
	}
	if (CM_Wr==NULL)
	{
		CM_Wr=new ClasterMem(SizeCMax,PDec);
		if(CM_Wr->Buf==NULL)
		{
			delete CM_Wr;
			SizeCMax=tCnt;
			CM_Wr=new ClasterMem(SizeCMax,PDec);
		}
		RealSize+=SizeCMax;NumCM++;
#ifndef _NOSRV
		Srv->GetRealTime(&TimeMemAlloc.FTime);
#else
		GetSystemTimeAsFileTime(&TimeMemAlloc.FTime);
#endif
		CM_Wr->NextCM=CM_Wr;CM_Wr->PreviousCM=CM_Wr; // Çàêîëüöåâàòü â öåïî÷êó
		CM_Rd=CM_Wr; // Íàñòðîèòü ÷òåíèå
	}

	
	if (CM_Wr->Size-CM_Wr->Wr<tCnt)
	{
		CM_Wr->Fl_Wr=1;	// Çàêðûòûé áëîê äëÿ çàïèñè
		if (CM_Wr->NextCM->Fl_Wr==0)
		{
			if(CM_Wr->NextCM->Size-CM_Wr->NextCM->Wr>=tCnt)
			{
				CM_Wr=CM_Wr->NextCM;	// Ñëåäóþùèé áëîê ïîäõîäèò!
			}
			else
			{
				if ((CM_Wr->NextCM->Wr==CM_Wr->NextCM->Rd)&&(CM_Wr->NextCM->Fl_Wr==0)&&(CM_Wr->NextCM->Fl_Rd==0))
				{
					//	Óäàëèòü ñëåäóþùèé êëàñòåð òàê êàê îí ìàëåíüêîãî ðàçìåðà
					ClasterMem*	TmpCM1=CM_Wr->NextCM;
					CM_Wr->NextCM=TmpCM1->NextCM;
					TmpCM1->NextCM->PreviousCM=CM_Wr;
					if (TmpCM1==CM_Rd)
					{
						CM_Rd=CM_Rd->NextCM;
					}
					RealSize-=TmpCM1->Size;delete TmpCM1;NumCM--;
				}
			}
		}
		if (CM_Wr->Size-CM_Wr->Wr<tCnt)
		{	// Âñòàâèòü äëÿ çàïèñè
//			if (CM_Wr->Wr==0)
//			{	// Óäàëèòü êëàñòåð òàê êàê îí ìàëåíüêîãî ðàçìåðà
//				ClasterMem*	TmpCM1=CM_Wr;
//				
//				TmpCM1->NextCM->PreviousCM=CM_Wr;
//				CM_Wr->NextCM=TmpCM1->NextCM;
//				delete TmpCM1;NumCM--;
//				Srv->Telemetr(PDec,"DamperP N_CM",(double)NumCM,this);
//				Srv->Telemetr(PDec,"DamperP SizeCMax",(double)SizeCMax,this);
//			}
			ClasterMem*	TmpCM1=new ClasterMem(SizeCMax,PDec);
			if(TmpCM1->Buf==NULL)
			{
				delete TmpCM1;
				SizeCMax=tCnt;
				TmpCM1=new ClasterMem(SizeCMax,PDec);
			}
			RealSize+=SizeCMax;NumCM++;NumCMM=(int)sqrt((double)NumCM);
#ifndef _NOSRV
			Srv->GetRealTime(&TimeMemAlloc.FTime);
#else
			GetSystemTimeAsFileTime(&TimeMemAlloc.FTime);
#endif
//			Srv->Telemetr(PDec,"DamperP N_CM",(double)NumCM,this);
//			Srv->Telemetr(PDec,"DamperP SizeCMax",(double)SizeCMax,this);
			TmpCM1->NextCM=CM_Wr->NextCM;
			TmpCM1->NextCM->PreviousCM=TmpCM1;
			if /*((CM_Wr->Wr==0)&&*/(CM_Wr->Rd==CM_Wr->Wr)
			{
				// block dont needed;
				TmpCM1->PreviousCM=CM_Wr->PreviousCM;
				CM_Wr->PreviousCM->NextCM=TmpCM1;
				if (CM_Wr==CM_Rd)
				{
					CM_Rd=TmpCM1;
				}
				RealSize-=CM_Wr->Size;delete CM_Wr;NumCM--;
				CM_Wr=TmpCM1;
			}
			else
			{
				TmpCM1->PreviousCM=CM_Wr;
				CM_Wr->NextCM=TmpCM1;
				CM_Wr=TmpCM1;
			}
		}
	}
	if(CM_Wr->Buf!=NULL)
	{
		memcpy(CM_Wr->Buf+CM_Wr->Wr,(UCHAR*)&HndlP,sizeof (HndlP));
		memcpy(CM_Wr->Buf+CM_Wr->Wr+sizeof (HndlP),Buf,tCnt-sizeof (HndlP));CM_Wr->Wr+=tCnt;
		if(0)
		{
			int fh1;		 			
			fh1 = _open("\\DumpQ.dmp", _O_RDWR | _O_CREAT, 
                       _S_IREAD | _S_IWRITE|_O_BINARY );
			if( fh1 != -1 )
			{
				_write(fh1,CM_Wr->Buf,tCnt+sizeof (HndlP));
				_close( fh1 );
			}			
		}
	}
	if (CM_Wr->Wr==CM_Wr->Size)
	{
		CM_Wr->Fl_Wr=1;	// Çàêðûòûé áëîê äëÿ çàïèñè
		if (CM_Wr->NextCM->Fl_Wr==0)
		{
			CM_Wr=CM_Wr->NextCM;
		}
		else
		{	// Âñòàâèòü äëÿ çàïèñè
			ClasterMem*	TmpCM1=new ClasterMem(SizeCMax,PDec);
			if(TmpCM1->Buf==NULL)
			{
				delete TmpCM1;
				SizeCMax=tCnt;
				TmpCM1=new ClasterMem(SizeCMax,PDec);
			}
			RealSize+=SizeCMax;NumCM++;NumCMM=(int)sqrt((double)NumCM);
#ifndef _NOSRV
			Srv->GetRealTime(&TimeMemAlloc.FTime);
#else
			GetSystemTimeAsFileTime(&TimeMemAlloc.FTime);
#endif
//			Srv->Telemetr(PDec,"DamperP N_CM",(double)NumCM,this);
//			Srv->Telemetr(PDec,"DamperP SizeCMax",(double)SizeCMax,this);
			TmpCM1->NextCM=CM_Wr->NextCM;
			TmpCM1->NextCM->PreviousCM=TmpCM1;
			TmpCM1->PreviousCM=CM_Wr;
			CM_Wr->NextCM=TmpCM1;
			CM_Wr=TmpCM1;
		}
	}
	FileTime	OperTime;
#ifndef _NOSRV
	Srv->GetRealTime(&OperTime.FTime);
#else
	GetSystemTimeAsFileTime(&OperTime.FTime);
#endif
	if (OperTime.Time-TimeMemAlloc.Time>KtimeM)
	{
		TimeMemAlloc.Time=OperTime.Time;
		if ((CM_Wr->NextCM->Wr==CM_Wr->NextCM->Rd)&&(CM_Wr->NextCM!=CM_Rd)&&(CM_Wr->NextCM->Fl_Rd==0))
		{	// Óäàëèòü êëàñòåð
			ClasterMem*	TmpCM1=CM_Wr->NextCM;
			TmpCM1->NextCM->PreviousCM=CM_Wr;
			CM_Wr->NextCM=TmpCM1->NextCM;
			if (TmpCM1==CM_Rd)
			{
				CM_Rd=TmpCM1->NextCM;
			}
			RealSize-=TmpCM1->Size;delete TmpCM1;NumCM--;
//			Srv->Telemetr(PDec,"DamperP N_CM",(double)NumCM,this);
//			Srv->Telemetr(PDec,"DamperP SizeCMax",(double)SizeCMax,this);
		}
	}
	MyUnLock(__FILE__,__LINE__,&Fl_MT,PDec);//Fl_MTi=0;
//	ReleaseSemaphore(hmtx,1,NULL); 
	return rc;
};
int		DamperPS::OData()
{
	int	rc=0;
	uchar*	tBuf;
	size_t		tCnt;
	while (LockOData(tBuf,tCnt))
	{
		DmpProc(pObj,tBuf,tCnt);
		UnLockOData();
	}
	return rc;
};
int		DamperPS::LockOData(UCHAR* &Buf,size_t &Cnt)
{
	int	rc=0;
	MyLock(__FILE__,__LINE__,&Fl_MT,PDec);
//	WaitForSingleObject(hmtx,INFINITE);
	if (CM_Rd)
	{
		if ((CM_Rd->Rd!=CM_Rd->Wr)||(CM_Rd->Fl_Wr!=0))
		{
			while ((CM_Rd->Rd==CM_Rd->Wr)&&(CM_Rd->Fl_Wr!=0))
			{
				// Âû÷èòàëè âñå!!!
				CM_Rd->Fl_Rd=0;
				CM_Rd->Fl_Wr=0;CM_Rd->Rd=CM_Rd->Wr=0;
				CM_Rd=CM_Rd->NextCM;
			}
			if (CM_Rd->Rd!=CM_Rd->Wr)
			{
				tWr=CM_Rd->Wr;
				DmpHeader*	SH=(DmpHeader*)(CM_Rd->Buf+CM_Rd->Rd);
				SockHeader* HP=&SH->SockHead;
				//(SockHeader*)(CM_Rd->Buf+CM_Rd->Rd+);
				int SizeRd=sizeof(SockHeader)+HP->Len;
 				if (SH->CntDebug!=++CntPackOut)
 				{
 					CntPackOut=SH->CntDebug+1;
 					HndCE=Srv->SetCriticalError(PDec,"Error CntPack","Critical","Damper",HndCE);
 				}
				if (HP->Syn==Syn1)
				{
					CM_Rd->Fl_Rd=1;
					Buf=(uchar*)HP;/*CM_Rd->Buf+CM_Rd->Rd;*/Cnt=SizeRd;
					rc+=SizeRd;
					Fl_Lock=1;
				}
				else
				{
#ifndef _NOSRV
					Srv->SetCriticalError(PDec,"Beep:OData:Syn-Size","Error","DamperPS");
#endif
					CM_Rd->Rd=tWr;
				};
				CntInside-=(Cnt-sizeof(SockHeader));
			}
		}
	}
	MyUnLock(__FILE__,__LINE__,&Fl_MT,PDec);//Fl_MTo=0;
//	ReleaseSemaphore(hmtx,1,NULL); 
	return rc;
};

void	DamperPS::Clear()
{
//	UCHAR*	tBuf;
//	int		tCnt;
	MyLock(__FILE__,__LINE__,&Fl_MT,PDec);
	CntPackIn=CntPackOut=0;
	SumSize=0;
	if (CM_Wr)	// óíè÷òîæèòü öåïî÷êó êëàñòåðîâ
	{
		if (Fl_Lock==1)
		{	
			Fl_Clean=1;
		}
		else
		{
			ClasterMem*	TmpCM1;
			ClasterMem*	TmpCM2=CM_Rd;
			TmpCM2->PreviousCM->NextCM=NULL;	// Ðàçîðâàòü öåïî÷êó êëàñòåðîâ
			TmpCM2->PreviousCM=NULL;			// Ðàçîðâàòü öåïî÷êó êëàñòåðîâ
			while (TmpCM2)
			{
				{
					TmpCM1=TmpCM2->NextCM;RealSize-=TmpCM2->Size;delete TmpCM2;NumCM--;
					TmpCM2=TmpCM1;
					if (TmpCM2)
						TmpCM2->PreviousCM = NULL;
				}
			}
// 			if (NumCM!=0)
// 				{size_t iasm=1;}
			CM_Rd=CM_Wr=NULL;
		}
	}
	CntInside=0;
	MyUnLock(__FILE__,__LINE__,&Fl_MT,PDec);//Fl_MTi=0;
}; // Âñå ïî÷èñòèòü

void	DamperPS::UnLockOData()
{
	MyLock(__FILE__,__LINE__,&Fl_MT,PDec);
//	WaitForSingleObject(hmtx,INFINITE);
	if (CM_Rd)
	{
		if ((Fl_Lock==1)&&(CM_Rd->Rd!=tWr))
		{
//			int	tWr=CM_Rd->Wr;
			DmpHeader* SH=(DmpHeader*)(CM_Rd->Buf+CM_Rd->Rd);
			SockHeader* HP=&SH->SockHead;
			int SizeRd=sizeof(DmpHeader)+HP->Len;
			if (HP->Syn==Syn1)
			{
				CM_Rd->Rd+=sizeof(DmpHeader)+HP->Len;
			}
			else
			{
#ifndef _NOSRV
				Srv->SetCriticalError(PDec,"Beep:OData:Syn-Size","Error","DamperPS");
#endif
				CM_Rd->Rd=tWr;
			};
			if ((CM_Rd->Rd==CM_Rd->Size)||((CM_Rd->Rd==CM_Rd->Wr)&&(CM_Rd->Fl_Wr!=0)))
			{	// Âû÷èòàëè âñå!!!
				CM_Rd->Fl_Rd=0;
				CM_Rd->Fl_Wr=0;CM_Rd->Rd=CM_Rd->Wr=0;
				CM_Rd=CM_Rd->NextCM;
			}
			else
			{
				CM_Rd=CM_Rd;
			};
			Fl_Lock=0;
			if (Fl_Clean==1)
			{
				Clear();Fl_Clean=0;
			}
		}
	}
	MyUnLock(__FILE__,__LINE__,&Fl_MT,PDec);//Fl_MTo=0;
//	ReleaseSemaphore(hmtx,1,NULL); 
};


		DamperIDInfo::DamperIDInfo()
{
	TBuff=NULL;
	SizeTBuff=0;
	CntInSidePackets=0;
	Srv=0;
};

		DamperIDInfo::~DamperIDInfo()
{
	IDInfo*	tIDI;
	int		tN_Pin;
	MyLock(__FILE__,__LINE__,&Fl_MT,NULL);
	Clear();
	if (CntInSidePackets>0)
	{
		while ((GetIDI(tN_Pin,tIDI))!=NULL)	
		{
			DestroyIDI(tIDI);
		};
	}
#ifndef _NOSRV
	if (Srv)
	{
		if (TBuff)
			Srv->free(TBuff);
	}
	else
	if (TBuff)
		free(TBuff);
#else
	if (TBuff)
		free(TBuff);
#endif
	SizeTBuff=0;
	TBuff=NULL;
	MyUnLock(__FILE__,__LINE__,&Fl_MT,NULL);
};

int		DamperIDInfo::SendIDI(int	N_Pin,IDInfo*	IDI)
{
	MyLock(__FILE__,__LINE__,&Fl_RE,NULL);
	int	rSize=sizeof(IDInfo)+IDI->Cnt;
	if (SizeTBuff<rSize)
	{
#ifndef _NOSRV
		if (!Srv)
			Srv=GetServices();
		if (Srv)
		{
		if (TBuff)
			Srv->free(TBuff);
		SizeTBuff=rSize;
		TBuff=(uchar*)Srv->malloc(SizeTBuff);
		}
		else
		{
			if (TBuff)
				free(TBuff);
			SizeTBuff=rSize;
			TBuff=(uchar*)malloc(SizeTBuff);
		}
#else
		if (TBuff)
			free(TBuff);
		SizeTBuff=rSize;
		TBuff=(uchar*)malloc(SizeTBuff);
#endif
	}
	memcpy(TBuff,(void*)IDI,sizeof(IDInfo));
	if (IDI->OldCnt)
	{
#ifndef _NOSRV
		if (Srv)
		Srv->SetCriticalError(NULL,"OldErr","Error","Damper");
#endif
	}
	if (IDI->Cnt)
	{
		memcpy(TBuff+sizeof(IDInfo),IDI->Buf,IDI->Cnt);
		((IDInfo*)TBuff)->Buf=TBuff+sizeof(IDInfo);
	}
	((IDInfo*)TBuff)->Cnt=IDI->Cnt;
// 	}
// 	else
// 	{
// 		((IDInfo*)TBuff)->Cnt=0;
// 	};
	if (IDI->ParentFile)
		((IDInfo*)TBuff)->ID_ParentFile=IDI->ParentFile->ID_File;
	if (IDI->SeansFrom)
		((IDInfo*)TBuff)->ID_SeansFrom=IDI->SeansFrom->ID_File;
	if (IDI->SeansTo)
		((IDInfo*)TBuff)->ID_SeansTo=IDI->SeansTo->ID_File;
	((IDInfo*)TBuff)->ParentFile=NULL;
	((IDInfo*)TBuff)->SeansFrom=NULL;
	((IDInfo*)TBuff)->SeansTo=NULL;

	((IDInfo*)TBuff)->pPList=new ParamListID("DamperIDInfo");
	CntInSidePackets++;
	CopyPListID(((IDInfo*)TBuff)->pPList,IDI->pPList);
	if (IDI->AdrInfo)
	{
		((IDInfo*)TBuff)->AdrInfo=new AddressInfo();
		memcpy(((IDInfo*)TBuff)->AdrInfo,IDI->AdrInfo,sizeof(AddressInfo));
	}
	Data(TBuff,rSize,N_Pin);
	MyUnLock(__FILE__,__LINE__,&Fl_RE,NULL);
	return 0;
};

void DamperIDInfo::Clear()
{
	IDInfo* idi;
	int tint/*,rc*/;

	while(GetIDI(tint,idi))
		DestroyIDI(idi);
}

int		DamperIDInfo::CopyPListID(ParamListID* pPListD,ParamListID* pPListS)
{
	INT64		IDName;
	void*		pVol;
	size_t		pCnt;
	Decoder*	ParentMt=NULL;
	if(pPListS->FindFirst(IDName,pVol,pCnt,ParentMt)!=-1)
	{
		do
		{
			pPListD->SetParam(IDName,pVol,pCnt,ParentMt);
		}
		while(pPListS->FindNext(IDName,pVol,pCnt,ParentMt)!=-1);
	};
	return	0;
}

int		DamperIDInfo::GetIDI(int	&N_Pin,IDInfo*	&IDI)
{
	UCHAR*	InBuf;
	size_t	ICnt;
	if (LockOData(InBuf,ICnt))
	{
		N_Pin=((SockHeader*)InBuf)->CntPack;
		IDI=(IDInfo*)(InBuf+sizeof(SockHeader));
		IDI->Buf=InBuf+sizeof(SockHeader)+sizeof(IDInfo);
	}
	else
	{
//		UnLockOData();
		if (CntInSidePackets)
		{
			if (CntInside==0)
			{
				CntInSidePackets=0;
			}
			else
				ICnt=0;
		}
		ICnt=0;
	}
	return	ICnt;
};

int		DamperIDInfo::DestroyIDI(IDInfo*	IDI)
{
	delete IDI->pPList;
	if (IDI->AdrInfo)
		delete IDI->AdrInfo;
	CntInSidePackets--;
	UnLockOData();
	return	0;
};


	StackNumBuffer::StackNumBuffer()
{
	BufferDeep=1200;
	BufferClaster=(char*)malloc(BufferDeep*MaxStackBufSize);
};

	StackNumBuffer::~StackNumBuffer()
{
	free(BufferClaster); BufferClaster=NULL;
};

void	StackNumBuffer::SetBufferDeep(int Deep)
{
	if (BufferDeep!=Deep)
	{
		free(BufferClaster);
		BufferDeep=Deep;
		BufferClaster=(char*)malloc(BufferDeep*MaxStackBufSize);
	};
};

int		StackNumBuffer::Initialze(int	NumFrstPack)
{
	int	rc=-1;
	NumFirstPack=NumFrstPack;
	MaxNumPack=0;
	if(BufferClaster)
	{
		for (size_t i=0;i<BufferDeep;i++)
		{
			*(int*)(BufferClaster+i*MaxStackBufSize)=0;
		}
		rc=0;
	}
	return rc;
};

int		StackNumBuffer::AddPacket(int	Number, char* Buf, size_t Cnt)
{
	int	rc=-1;
	int	i;
	if (Number>=NumFirstPack)
	{
		i=Number-NumFirstPack;
		if (i<BufferDeep)
		{
			memcpy((char*)(BufferClaster+i*MaxStackBufSize+sizeof(int)),Buf,Cnt);
			*(int*)(BufferClaster+i*MaxStackBufSize)=1;
			rc=1;
			if (MaxNumPack<i)
				MaxNumPack=i;
			for (i=0;i<MaxNumPack;i++)
			{
				if (*(int*)(BufferClaster+i*MaxStackBufSize)==0)
				{
					rc=0;
					break;
				}
			}
		}
		else
		{
			rc=2;
		}
	}
	else
	{
		{size_t iasm=1;}
	}
	return rc;
};

int		StackNumBuffer::GetPacket(int	&Number, char* &Buf, size_t &Cnt)
{
	int	rc=-1;
	Cnt=0;
	if(BufferClaster)
	{
		rc=1;
		for (size_t i=0;i<BufferDeep;i++)
		{
			if (*(int*)(BufferClaster+i*MaxStackBufSize)==1)
			{
				Buf=BufferClaster+i*MaxStackBufSize+sizeof(int);
				Number=((SockHeader*)Buf)->CntPack;
				Cnt=((SockHeader*)Buf)->Len+sizeof(SockHeader);
				*(int*)(BufferClaster+i*MaxStackBufSize)=0;
				if (i>=MaxNumPack)
					rc=0;
				break;
			}
			if (i>=MaxNumPack)
			{
				rc=0;break;
			}
		}
	}
	return rc;
};

// ---------------------
void IDI2FileName (IDInfo* IDI,char* Path,char* Name)
{
	sprintf(Path,"%02X\\%02X\\",*(1+(UCHAR*)&IDI->ID_File),*(2+(UCHAR*)&IDI->ID_File));
	sprintf(Name,"%02X%02X%08X",*(UCHAR*)&IDI->ID_File,*(3+(UCHAR*)&IDI->ID_File),*(UINT*)(4+(UCHAR*)&IDI->ID_File));
};

typedef struct {SID_AND_ATTRIBUTES UserTokenInformation;char aBuffer[64];}tUserTokenBuffer;

inline PSID getProcessSID()//user Security Identifier
	{
	HANDLE aTokenHandle=0;
	DWORD aTokenActual=0;
//	PSID Result=NULL;
	//malloc new
	HANDLE hProcess;
	hProcess=GetCurrentProcess();
	tUserTokenBuffer aUserTokenBuffer;
	//tUserTokenBuffer* aUserTokenBuffer=new tUserTokenBuffer;
	OpenProcessToken(hProcess,TOKEN_READ,&aTokenHandle);
	//OpenProcessToken(hProcess,TOKEN_ALL_ACCESS,&aTokenHandle);//ERROR_INVALID_HANDLE
	//OpenProcessToken(hProcess,TOKEN_READ,&aTokenHandle);
//	DWORD OpenProcessTokenError=GetLastError();
	if(GetTokenInformation(aTokenHandle,TokenUser,&aUserTokenBuffer,sizeof(aUserTokenBuffer),&aTokenActual))
	//if(GetTokenInformation(aTokenHandle,TokenUser,aUserTokenBuffer,sizeof(*aUserTokenBuffer),&aTokenActual))
		//Result=aUserTokenBuffer->UserTokenInformation.Sid;
		return(aUserTokenBuffer.UserTokenInformation.Sid);
	else
		return(NULL);
	//return Result;
	CloseHandle(hProcess);
	}

inline int Win32Log(tStr aSourse,tStr aMessage,tStr aComputer,unsigned short wCategory)
	{
	int Result;
	HANDLE aTokenHandle;
	DWORD aTokenActual;
	PSID ProcessUserSid;
	HANDLE aLogHandle;
	///////////////////////////////////////getProcessSID
	tUserTokenBuffer aUserTokenBuffer;
	HANDLE hProcess;
	hProcess=GetCurrentProcess();
	OpenProcessToken(hProcess,TOKEN_QUERY,&aTokenHandle);
	if(GetTokenInformation(aTokenHandle,TokenUser,&aUserTokenBuffer,sizeof(aUserTokenBuffer),&aTokenActual))
		ProcessUserSid=aUserTokenBuffer.UserTokenInformation.Sid;
	else
		ProcessUserSid=NULL;
	///////////////////////////////////////SaveEvent
	aLogHandle=RegisterEventSourceA(aComputer,aSourse);
	if(aMessage)
		Result=ReportEventA(aLogHandle,wCategory,0,0,ProcessUserSid,1,0,&aMessage,NULL);
	else 
		Result=ReportEventA(aLogHandle,wCategory,0,0,ProcessUserSid,0,0,NULL,NULL);
//	DWORD OpenProcessTokenError=GetLastError();
	DeregisterEventSource(aLogHandle);
	CloseHandle(aTokenHandle);
	CloseHandle(hProcess);
	return Result;
	}

////////////////////////////////////////////////////////////////////

int Win32LogErr(tStr aSourse,tStr aMessage,tStr aComputer)
	{return Win32Log(aSourse,aMessage,aComputer,EVENTLOG_ERROR_TYPE);}
int Win32LogWrn(tStr aSourse,tStr aMessage,tStr aComputer)
	{return Win32Log(aSourse,aMessage,aComputer,EVENTLOG_WARNING_TYPE);}
int Win32LogInf(tStr aSourse,tStr aMessage,tStr aComputer)
	{return Win32Log(aSourse,aMessage,aComputer,EVENTLOG_INFORMATION_TYPE);}

////////////////////////////////////////////////////////////////////

Services*	GetServices()
{
	if (PoiServices==NULL)
	{
/*		Beep(2000,20);	// Åäèíñòâåííîå ìåñòî ñ áèïàìè!!! ... áûëî
		Beep(3000,20);
		Beep(2000,20);
		Beep(3000,20);
		Beep(4000,20);
		Beep(3000,20);
		Beep(2000,20);
*/	}
	return PoiServices;
};
void		SetServices(Services* Srv)
{
	PoiServices=Srv;
	GetServices();	// Äëÿ ïðîâåðêè
};

El_TabList::El_TabList(INT64 ID,Decoder* pDec)
{
	IDT=new IDTable(GetServices(),ID);
	strcpy(IDT->UInfo.DomainName,pDec->UInfo.DomainName);
	IDT->UInfo.TractID=pDec->UInfo.TractID;
//	IDT->ID=ID;
//	IDT->Start();
	Buff=(UCHAR*)&IDT->ID;
	Len=sizeof(INT64);
};

El_TabList::~El_TabList()
{
	delete IDT;
}

void	El_TabList::AddLink(Decoder* ParMt)
{
	BL.AddElem((UCHAR*)&ParMt,sizeof(ParMt));
};

void	El_TabList::DelLink(Decoder* ParMt)
{ 
	UCHAR*		tBuf;
	size_t		tCnt;
	int			rc;
	rc=BL.FindFirst(tBuf,tCnt);
	while (rc)
	{
		if ((size_t)ParMt==*(size_t*)tBuf)
		{
			rc=BL.DelElem(tBuf,tCnt);
			break;
		}
		else
		{
			rc=BL.FindNext(tBuf,tCnt);
		}
	}
}; 

El_Tab::El_Tab(StrOwner*	Owner,INT64 tID,IDTable *IDT)
{
	DBGC.Cnt_El_Tab++;
	pMt=Owner->pDec;
	Fl_Del=0;Fl_Query=0;Fl_Test=0;
	Srv=GetServices();
	IDI=Srv->GetNewIDI(pMt);
	IDI->ID_File=tID;
	ID=tID;
	Buff=(UCHAR*)&IDI->ID_File;
	Len=sizeof(INT64);
	IDL64=NULL;
	pParent=NULL;
	OwnerList=NULL;LastOwner=NULL;
	Fl_LockWR=0;
	IDTab=IDT;
	Fl_NeedWR=0;
	if (IDT->GetRoot())
		IDI->SeansFrom=IDT->GetRoot()->GetIDI();
	strcpy(LastOwnerName,pMt->Name_Mt);
//	Fl_Del=0;
	IDI->AdrInfo=&IDT->AdrI;
//		if ((tID==0x1d2d00004f7bf412)||(tID==0x1d2d00004f7aff00))
//		{size_t iasm=1;}
//	char	tB[128];
//	sprintf(tB,"Create El=0x%x ID=0x%I64x Mt=%s",this,tID,LastOwnerName);
//	Srv->OutLog(tB,Owner->pDec,"El_Tab");
	TestOwner(Owner);
};

void	El_Tab::Init(StrOwner*	Owner,INT64 tID,IDTable*	IDT)
{
	MyLock(__FILE__,__LINE__,&Fl_RE,Owner->pDec);
	IDTab=IDT;
	pMt=Owner->pDec;
	Fl_Del=0;Fl_Query=0;Fl_Test=0;
	Srv=GetServices();
	IDI->ReInit(Owner->pDec);
	IDI->ID_File=tID;
	ID=tID;
	if (IDL64)
		IDL64->DelAllElem();
	pParent=NULL;
	if (OwnerList!=NULL) delete OwnerList;
	OwnerList=NULL;LastOwner=NULL;
	Fl_LockWR=0;
	Fl_NeedWR=0;
	strcpy(LastOwnerName,Owner->pDec->Name_Mt);
//	Fl_Del=0;
	MyUnLock(__FILE__,__LINE__,&Fl_RE,Owner->pDec);
//		if ((tID==0x1d2d00004f7bf412)||(tID==0x1d2d00004f7aff00))
//		{size_t iasm=1;}
}

/*
void	El_Tab::AddText(UCHAR* Txt)
{
	if (TextBuf==NULL)
	{
		CntTB=strlen((char*)Txt)+1;
		TextBuf=(UCHAR*)pMt->malloc(CntTB);
	}
	else
	{
		int tCnt=strlen((char*)Txt)+1;
		if (CntTB<tCnt)
		{
			pMt->free(TextBuf);
			TextBuf=(UCHAR*)pMt->malloc(tCnt);
			CntTB=tCnt;
		}
	};
	strcpy((char*)TextBuf,(char*)Txt);
	if (pParent!=NULL)
	{
		pParent->AddChild(this);
	}
};
*/
bool	El_Tab::TestModif(FileTime *fTime)
{
	bool rc=(fTime->Time!=IDI->TimeLastWr.Time);
	if (rc)
	{
		fTime->Time=IDI->TimeLastWr.Time;
	};
	return rc;
};

int		El_Tab::GetNumChild()
{
	if (IDL64)
		return IDL64->NumElem;
	else return 0;
};

int		El_Tab::PreSave()
{
	if (Fl_NeedWR!=0)
		IDTab->SaveEl(this);
	Fl_NeedWR=0;
	return 0;
}

int	El_Tab::AddChild(INT64 ID,int Flag)
{
	int		rc=0;
	char	LogBuf[128];
	if (IDL64==NULL) IDL64=new ID_List8();
	ID_Elem64*		ElK=(ID_Elem64*)IDL64->Find((UCHAR*)&ID,sizeof(INT64));
	if (ElK==NULL)
	{
		ElK=(ID_Elem64*)IDL64->NewElem();
		if (ElK==NULL)
			ElK=new ID_Elem64(ID);
		else
			ElK->Init(ID);
		IDL64->Add(ElK);		// Äîáàâëÿåì
		if (IDTab->Fl_Debug)
		{
			sprintf(LogBuf,"AddChild El=%I64x ID=%I64x",IDI->ID_File,ID);Srv->OutLog(LogBuf,pMt,IDTab->TableName);
		}
		if (Flag)
			Fl_NeedWR|=1;
	}
	else
	{
		rc=-1;
	}
	return rc;
};

void	El_Tab::DelChild(INT64 ID)
{
	char	LogBuf[128];
	if (IDL64!=NULL)
	{
		ID_Elem64*		ElK=(ID_Elem64*)IDL64->Find((UCHAR*)&ID,sizeof(INT64));
		if (ElK)
		{
			IDL64->Del(ElK);
			Fl_NeedWR|=1;
			if (IDTab->Fl_Debug)
			{
				sprintf(LogBuf,"DelChild El=%I64x ID=%I64x",IDI->ID_File,ID);Srv->OutLog(LogBuf,pMt,IDTab->TableName);
			}
		}
	}
};


El_Tab::~El_Tab()
{
	Fl_Del=2;
	if (IDI) Srv->DeleteIDI(IDI);
	if (OwnerList)
	{
		if (OwnerList->NumElem>0)
		{
			try
			{
/*
				char	tBuf[128];
				El_Owner*	tEl;
				void*	tVol=NULL;
				tEl=(El_Owner*)OwnerList->Begin();
				if (this->IDTab->GetParam(tEl->PObj,this,IDNameElTab,tVol)==0)
				{
					sprintf(tBuf,"El_Tab(%s):0x%I64x not UnLocking Mt=%s!",(char*)tVol,IDI->ID_File,tEl->NameMt);				
				}
				else
				{
					sprintf(tBuf,"El_Tab:0x%I64x not UnLocking Mt=%s!",IDI->ID_File,tEl->NameMt);				
				}
				Srv->SetCriticalError(tEl->PObj,tBuf,"Warning","El_Tab");
*/
			}
			catch (...)
			{
				
			}
		};
		delete OwnerList;
	};
	if (IDL64) delete IDL64;
	--DBGC.Cnt_El_Tab;
};

void	El_Tab::AddOwner(StrOwner*	Owner)
{
	El_Owner*	tEl;
	MyLock(__FILE__,__LINE__,&Fl_RE,Owner->pDec);
	if (OwnerList==NULL) OwnerList=new ID_List8();
	tEl=(El_Owner*)OwnerList->Find((UCHAR*)Owner,sizeof(StrOwner));
	if (tEl==NULL)
	{
		tEl=(El_Owner*)OwnerList->NewElem();
		if (tEl==NULL)
			tEl=new El_Owner(Owner);
		else
			tEl->Init(Owner);
		OwnerList->Add(tEl);tEl->Counter++;
		strcpy(LastOwnerName,Owner->pDec->Name_Mt);
	}
	MyUnLock(__FILE__,__LINE__,&Fl_RE,Owner->pDec);
// Ïîïðàâêà 08.09.09
//	else
//	{
//		tEl->Counter++;
//	}
};

int		El_Tab::DelOwner(StrOwner*	Owner)
{
	int	rc=-1;
	El_Owner*	tEl;
	if (OwnerList!=NULL)
	{
		MyLock(__FILE__,__LINE__,&Fl_RE,Owner->pDec);
		tEl=(El_Owner*)OwnerList->Find((UCHAR*)Owner,sizeof(StrOwner));
		if (tEl)
		{
			rc=0;
			if (--tEl->Counter==0)
			{
				OwnerList->Del(tEl);
			}
			else
				{size_t iasm=1;}
		}
		MyUnLock(__FILE__,__LINE__,&Fl_RE,Owner->pDec);
	}
	return	rc;
};

bool	El_Tab::TestOwner(StrOwner*	Owner)
{
	El_Owner*	tEl;
	if (OwnerList!=NULL)
	{
		MyLock(__FILE__,__LINE__,&Fl_RE,Owner->pDec);
		tEl=(El_Owner*)OwnerList->Find((UCHAR*)Owner,sizeof(StrOwner));
		MyUnLock(__FILE__,__LINE__,&Fl_RE,Owner->pDec);
		if (tEl)
			return TRUE;
		else
		{
			char	tB[128];
			sprintf(tB,"Owner=%s has not locked this element=%I64x",Owner->pDec->Name_Mt,IDI->ID_File);
			Srv->OutLog(tB,pMt,"OwnErr");
			AddOwner(Owner);
		}
	}
	return FALSE;
};

/*
El_Key::El_Key (El_Tab* pElT)
{
	ElT=pElT;
	Buff=(UCHAR*)ElT->TextBuf;
	Len=ElT->CntTB;
};
*/
IDTable::IDTable()
{
// IDTable_ äîëæåí áûòü çàìåíåí íà èìÿ îáúåêòà
		Name_Mt=IDTable_NameMt;Rem_Mt=IDTable_RemMt;Ver_=IDTable_Version;Auth_=IDTable_Author;
		PinName_I=IDTable_PinNameI;PinName_O=IDTable_PinNameO;Prm_=IDTable_Prm;Vol_=IDTable_Vol;

		Num_PinI=0;while (PinName_I[2*Num_PinI][0]!=0x0) Num_PinI++;
		Num_PinO=0;while (PinName_O[2*Num_PinO][0]!=0x0) Num_PinO++;
		Num_Prm=0;while (Prm_[2*Num_Prm][0]!=0x0)Num_Prm ++;
// -------------------------------------------------------------
//	Mt=NULL;
	Fl_Debug=0;
	Fl_AntyRec=0;
	Fl_RR=0;CustomerID=0;
	ElTbl=NULL;
	ElTblForNew=ElTblDel=NULL;
	IDTHnd=NULL;
	IDL=NULL;IDL_Query=NULL;IDL_Req=NULL;//IDL_Repl=NULL;
	ID=0;Fl_Ready=0;CntActQuery=0;//CntChild=0;
	Fl_Long=0;BDL_Cnt=0;Scan_Index=0;DelTimer=0;FlagRE=0;
//	Fl_RE=0;
	char	tTB[256];
	sprintf(tTB,"%s\\DelList%I64d.BDL",Srv->MainPath,ID);
	int FH=_open(tTB,O_CREAT|O_BINARY|O_RDWR);
	if (FH!=-1)
	{
//		BLDel.LoadFromFile(FH,0);
//		BLDel.CopyEl();
		close(FH);
	}
	TmpAnchor.Init(this,this,NULL);
	pFSrv=NULL;
	strcpy(TableName,"IDTable");
};

IDTable::IDTable(Decoder* pMt,INT64 tID)
{
// IDTable_ äîëæåí áûòü çàìåíåí íà èìÿ îáúåêòà
		Name_Mt=IDTable_NameMt;Rem_Mt=IDTable_RemMt;Ver_=IDTable_Version;Auth_=IDTable_Author;
		PinName_I=IDTable_PinNameI;PinName_O=IDTable_PinNameO;Prm_=IDTable_Prm;Vol_=IDTable_Vol;

		Num_PinI=0;while (PinName_I[2*Num_PinI][0]!=0x0) Num_PinI++;
		Num_PinO=0;while (PinName_O[2*Num_PinO][0]!=0x0) Num_PinO++;
		Num_Prm=0;while (Prm_[2*Num_Prm][0]!=0x0)Num_Prm ++;
// -------------------------------------------------------------
	Fl_AntyRec=0;
	Fl_Debug=0;
	Fl_RR=0;CustomerID=0;
//	Mt=pMt;
	ElTbl=NULL;
	ElTblForNew=ElTblDel=NULL;

	IDTHnd=NULL;
	IDL=NULL;IDL_Query=NULL;IDL_Req=NULL;IDL_Test=NULL; 
	ID=tID;Fl_Ready=0;CntActQuery=0;//CntChild=0;
	Fl_Long=0;BDL_Cnt=0;Scan_Index=0;DelTimer=0;FlagRE=0;
//	Fl_RE=0;
	sprintf(TableName,"IDTab%I64x",tID);
	char	tTB[256];
	sprintf(tTB,"%s\\DelList%I64d.BDL",Srv->MainPath,ID);
	int FH=_open(tTB,O_CREAT|O_BINARY|O_RDWR);
	if (FH!=-1)
	{
//		BLDel.LoadFromFile(FH,0);
//		BLDel.CopyEl();
		close(FH);
	}
	TmpAnchor.Init(this,this,NULL);
	pFSrv=0;
};

void	IDTable::Param(char* Prm,char* Vol)
{
	if (strcmp(Prm,"Table_ID")==0) 
	{
		ID=_atoi64(Vol);
		if ((ID!=0)&&(Fl_Stop==0))
		{
			Stop();
			Start();
		}
	}
};

void	IDTable::Start()
{
//	MyLock(__FILE__,__LINE__,&Fl_RE,this);
	Fl_Stop=0;
//	if (pFSrv==0)
//		pFSrv=Srv->NewDecod("IDI_FSrv",this);
	if (pFSrv)
	{
		char	FullPath[256];
//		strcpy(pFSrv->UInfo.DomainName,UInfo.DomainName);
		Srv->InitUnit(pFSrv);
		pFSrv->ParamEx("Mode",N_Standalone);
		switch (ID)
		{
		case TabParametr:
			sprintf(FullPath,"%s\\PrmTbl",Srv->MainPath);
			break;
		case TabNeuronGlobal:
//			sprintf(FullPath,"%s\\T%x",Srv->MainPath,Srv->NetLCond.CustomerID);
			sprintf(FullPath,"%s\\GT",Srv->MainPath);
			break;
		case TabNeuronLocalCustomer:
			sprintf(FullPath,"%s\\LT%x",Srv->MainPath,(Srv->NetLCond.CustomerID&0xffff0000));
			break;
		case TabNeuronLocalSite:
			sprintf(FullPath,"%s\\LT%x",Srv->MainPath,Srv->NetLCond.CustomerID);
			break;
		default:
			strcpy(FullPath,Srv->MainPath);
		}
		pFSrv->ParamEx("Path",FullPath);
		pFSrv->ParamEx("Transit","Off");
		pFSrv->AddDecLink(this,0,0);	// 1000 pin Services
		pFSrv->StartEx();
	};
	if (IDL==NULL)
	{
		IDL=new ID_List8();
	}
 	if (IDL_Test==NULL)
 	{
 		IDL_Test=new ListID_FiFo();
 	}
	if (IDL_Query==NULL)
	{
		IDL_Query=new ID_List8();
	}
	if (IDL_Req==NULL)
	{
		IDL_Req=new ID_List8();
	}
	if (ID!=NULL)
	{
		/*QID=ID;*/ Fl_Ready=0;CntActQuery=0;
	}
	if (!ElTbl)
	{
		ElTbl=LoadEl(&TmpAnchor,ID);
		if (ElTbl==NULL) 
		{
			ElTbl=new El_Tab(&Anchor,ID,this);
			SaveEl(ElTbl);
			delete ElTbl;ElTbl=0;
			ElTbl=LoadEl(&TmpAnchor,ID);
		}
		QueryEl(ID);
	}
//	MyUnLock(__FILE__,__LINE__,&Fl_RE,this);
	if (Fl_Long)
	{
		ODataFF(1,NULL,1);
	}
};

void	IDTable::Stop()
{
//	MyLock(__FILE__,__LINE__,&Fl_RE,this);
	Fl_Stop=1;
//	MyUnLock(__FILE__,__LINE__,&Fl_RE,this);
};

int		IDTable::LockEl(StrOwner* Owner, El_Tab* ElT)
{
	int			rc=-1;
	int			tCnt=0;
//	char*		tVol;
	if ((ElT)&&(ElT->Fl_Del==0))
	{
//		MyLock(__FILE__,__LINE__,&Fl_RE,this);
//		Srv->GetRealTime(&ElT->GetIDI()->TimeOper.FTime);
		ElT->AddOwner(Owner);

//		MyUnLock(__FILE__,__LINE__,&Fl_RE,this);
		rc=0;
	}
	return rc;
};
int		IDTable::MoLockEl(StrOwner* Owner, El_Tab* ElT,int AcsMode)
{
	int			rc=-1;
	int			tCnt=0;
//	char*		tVol;
	int			CntWait=0;
	if ((ElT)&&(ElT->Fl_Del==0))
	{
//		MyLock(__FILE__,__LINE__,&Fl_RE,this);
		switch(AcsMode)
		{
		case AcsMd_Read:
			if ((ElT->Fl_LockWR==1)&&(ElT->LastOwner==Owner))
			{
//				Srv->GetRealTime(&ElT->GetIDI()->TimeLastWr.FTime);
				ElT->Fl_LockWR=0;
				ElT->LastOwner=NULL;
			}
			rc=0;
			break;
		case AcsMd_Write:
			do 
			{
				if (ElT->Fl_LockWR==0)	// !!!!!!!!!!!!!!!!!!!!!!!!!
				{
					ElT->Fl_LockWR=1;
					ElT->LastOwner=Owner;
					rc=0;
				}
				else
				{
					if ((ElT->LastOwner==Owner)||(ElT->LastOwner->pDec==Owner->pDec))
						rc=0;
					else
					{
						Sleep(10);
						if (CntWait++>100)
						{
							char tBo[128];
							sprintf(tBo,"Error Locking Element ID NameMt=%s",ElT->LastOwner->pDec->Name_Mt);
							Srv->SetCriticalError(this,tBo,"Warning",TableName);
							ElT->LastOwner=Owner;
							rc=0;
							break;
						}
					}
				}
			} while (rc==-1);
			break;
		}
//		MyUnLock(__FILE__,__LINE__,&Fl_RE,this);
	}
	return rc;
};

int		IDTable::UnLockEl(StrOwner* Owner, El_Tab* ElT,int Fl_NoOTime)
{
	int			rc=-1;
//	char*		tVol;
//	MyLock(__FILE__,__LINE__,&Fl_RE,this);
	if ((ElT)&&(!IsBadReadPtr((void*)ElT,sizeof(void*))))
	{
		if ((ElT->Fl_LockWR==1)&&(ElT->LastOwner==Owner))
		{
			ElT->Fl_LockWR=0;
			ElT->LastOwner=NULL;
			rc=0;
		}
		if (ElT->DelOwner(Owner)!=-1)
			if (Fl_NoOTime==0)
				Srv->GetRealTime(&ElT->GetIDI()->TimeOper.FTime);
		rc=0;
	}
//	MyUnLock(__FILE__,__LINE__,&Fl_RE,this);
	return rc;
};

El_Tab*		IDTable::GetElTab(StrOwner* Owner, INT64 tID,int NoRepl)
{
//	INT64	T1=0,T2=0,T3=0;
//	FileTime	tFT0,tFT1;
	El_Tab*		tElT=NULL;
	char*		tVol;
//	if(tID==0x1d2d00005369248c)
//		{size_t iasm=1;}
// 	if (tID==0x1d2d000053692279)
//		{size_t iasm=1;}
//	if (BLDel.Find(tID))
//		return NULL;
	if (tID==NULL)
	{
		char	ttt[128];
		sprintf(ttt,"GetElTab with ID=NULL Owner=%s",Owner->pDec->Name_Mt);
		Srv->SetCriticalError(this,ttt,"Warning",TableName);
		return NULL;
	}
//	MyLock(__FILE__,__LINE__,&FlSim,this);
	tElT=(El_Tab*)IDL->Find((UCHAR*)&tID,sizeof(INT64));
	if (tElT==NULL)
	{
		tElT=LoadEl(&TmpAnchor,tID);
		if (tElT==NULL)
		{
			if (NoRepl==0)
				QueryEl(tID);
			if(tID==ID)
			{
				tElT=NewElTab(&TmpAnchor,tID);
			}
		}
// 		else
// 		{
// 			if (NoRepl==0)
// 				ReplEl(tElT);	  	//1111111111111111111	 
// 		}
	}
	else
	{
		if (NoRepl==3)
			tElT->Fl_Query=0;
		//QueryEl(tID);
//		if ((NoRepl==0)&&(!tElT->TestOwner(Owner)))
//			{size_t iasm=1;}
//		Srv->GetRealTime(&tFT0.FTime);
//		if (tID==ID)
//			QueryEl(tID);
//		Srv->GetRealTime(&tFT1.FTime);
//		if (tFT1.Time-tFT0.Time>KtimeS/2)
//			{size_t iasm=1;}
	}
	if (tElT)
	{
		LockEl(&TmpAnchor,tElT);
		if (NoRepl!=1)
		{
			ReplEl(tElT);			// !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!1
		}
//		if (MoLockEl(&Anchor,tElT,AcsMd_Read)==-1)
//		{
//			Srv->SetCriticalError(this,"Error LockEl Read","Warning",TableName);
//		};
	}
	if (tElT)
	{
		LockEl(&TmpAnchor,tElT);
		if (!TestValid(tElT,tElT->pParent))
		{
//			DelElTab(&Anchor,tElT); tElT=NULL;
		};
//		TestValid(tElT,tElT->pParent);
	}
	if (tElT)
	{
		LockEl(&TmpAnchor,tElT);
		if (GetParam(&TmpAnchor,tElT,IDNameElTab,tVol)==0)
		{
			if (Fl_Debug)
			{
				sprintf(LogBuf,"GetElTab ID=%I64x Name=%s Owner=%s",tElT->GetIDI()->ID_File,tVol,Owner->pDec->Name_Mt);
			}
		}
		else
		{
			if (Fl_Debug)
			{
				sprintf(LogBuf,"GetElTab ID=%I64x Owner=%s",tElT->GetIDI()->ID_File,Owner->pDec->Name_Mt);
			}
		}
		if (Fl_Debug)
		{
			Srv->OutLog(LogBuf,this,TableName);
		}
		if (tElT->Fl_Del!=0)
			tElT=NULL;
		else
		{
			UnLockEl(&TmpAnchor,tElT);
			LockEl(Owner,tElT);
		}
	}
//	MyUnLock(__FILE__,__LINE__,&FlSim,this);
	return tElT;
};

bool	IDTable::TestNewest(IDInfo* ID1,IDInfo* ID2)
{
	bool	rc=ID1->TimeLastWr.Time>ID2->TimeLastWr.Time;
/*	INT64 absVol=0;
#ifdef	_MSC_VER
#if (_MSC_VER>=1310)
	absVol=_abs64(ID1->TimeLastWr.Time-ID2->TimeLastWr.Time);
#else
	absVol=abs((int)(ID1->TimeLastWr.Time-ID2->TimeLastWr.Time));
#endif
#endif


//INT64 absVol=abs(ID1->TimeLastWr.Time-ID2->TimeLastWr.Time);
	if (absVol<KtimeM)
	{
		if (ID1->ModifCnt>ID2->ModifCnt)
		{
			if (ID1->ModifCnt-ID2->ModifCnt>327680000)
			{
				rc=false;
			}
			else
			{
				rc=true;
			}
		}
		else
		{
			if (ID2->ModifCnt-ID1->ModifCnt!=0)
			{
				if (ID2->ModifCnt-ID1->ModifCnt<327680000)
				{
					rc=false;
				}
				else
				{
					rc=true;
				}
			}
			else
			{
// 				if ((ID1->ID_Standart>0)&&(ID2->ID_Standart>0))
// 				{
// 					if (ID1->ID_Standart>ID2->ID_Standart)
// 					{
// 						rc=true;
// 					}
// 					else
// 					{
// 						if (ID1->ID_Standart<ID2->ID_Standart)
// 							rc=false;
// 						else
// 							{size_t iasm=1;}
// 					}
// 				}
// 				if (ID2->Cnt!=ID1->Cnt)
// 				{
// 					if (ID2->Cnt>ID1->Cnt)
// 						rc=false;
// 					else
// 						rc=true;
// 				}
//				if (Srv->MyNum!=Srv->SInt->NumSrv)
//					rc=true;
			}
		};
	}
*/
	return rc;
};
bool	IDTable::TestOwnEl	(StrOwner* Owner,El_Tab* ETbl)
{
	MyLock(__FILE__,__LINE__,&FlSim,this);
	bool	rc=ETbl->TestOwner(Owner);
	if (!rc)
	{
		char	tBo[256];
		try
		{
			El_Tab*	tEl=GetElTab(Owner,ETbl->ID);
//			El_Tab*	tEl=GetElTab(&Anchor,ETbl->ID);
			if (tEl)
			{
				if (tEl!=ETbl)
				{
					sprintf(tBo,"Error IDElTab was DELETE ID=%I64x Owner=%s",ETbl->ID,Owner->pDec->Name_Mt);
					Srv->SetCriticalError(this,tBo,"Critical",TableName,IDTHnd);

				}

			}
		}
		catch (...)
		{
			sprintf(tBo,"Error using IDElTab ID=%I64x Owner=%s",ETbl->GetIDI()->ID_File,Owner->pDec->Name_Mt);
			Srv->SetCriticalError(this,tBo,"Critical",TableName,IDTHnd);
		}
	}
	MyUnLock(__FILE__,__LINE__,&FlSim,this);
	return rc;
};

bool	IDTable::Combine(INT64 ID1,INT64 ID2)
{
	bool rc=false;
	El_Tab* tElT1=GetElTab(&TmpAnchor,ID1);
	El_Tab* tElT2=GetElTab(&TmpAnchor,ID2);
	if ((tElT1)&&(tElT2))
	{
		Combine(tElT1,tElT2);
	}
	UnLockEl(&TmpAnchor,tElT1);
	UnLockEl(&TmpAnchor,tElT2);
	return rc;	/* ???ïåðåìåííàÿ íå îïðåäåëåíà??? vsv*/
};

bool	IDTable::Combine(El_Tab* &ETbl1,El_Tab* &ETbl2,int Fl_NoRepl)
{
	bool rc=TRUE;
//	return rc;
	El_Tab* tElT3;
	INT64		IDName;
	void*		pVol;
	size_t		pCnt;
	Decoder*	ParentMt=NULL;
	char*		tVol;
	if ((ETbl2==NULL)||(ETbl1==NULL))
	{
		rc=FALSE;
		return rc;
	}
//	if ((Srv->SInt->NumLANSrv!=0)&&(Srv->SInt->MyLANNum!=Srv->SInt->NumLANSrv))
//	{
//		ReplEl(ETbl2);
//		ReplEl(ETbl1);
//	}
//	else
	if (ETbl1->IDL64)
	{
		FindStrList8	cFSL80;
		ID_Elem64*	tElT64=(ID_Elem64*)ETbl1->IDL64->Begin(cFSL80);
		while (tElT64!=NULL)
		{
			tElT3=GetElTab(&TmpAnchor,tElT64->Vol);
			if (tElT3)
			{
				UnLockEl(&TmpAnchor,tElT3);
			}
			tElT64=(ID_Elem64*)ETbl1->IDL64->Next(cFSL80);
		}
	}
	if ((Srv->SInt->NumLANSrv==0)||(Srv->SInt->MyLANNum==Srv->SInt->NumLANSrv)||(ID==TabParametr))
	{
		if(ETbl1->pParent==ETbl2->pParent)
		{

//			RepairEl(ETbl1);
//			RepairEl(ETbl2);
//			BLDel.DelElemID(ETbl1->GetIDI(&TmpAnchor)->ID_File);
// 			if (ETbl1->GetIDI()->TimeBegin.Time>ETbl2->GetIDI()->TimeBegin.Time)	// >
// 			{	// Òåñòèðóåìûé ýëåìåíò ñîçäàí ïîçæå íàéäåííîãî
// 				ETbl1->GetIDI()->TimeBegin.Time=ETbl2->GetIDI()->TimeBegin.Time;
// 			}
			if (Fl_Debug)
			{
				if (GetParam(&TmpAnchor,ETbl1,IDNameElTab,tVol)==0)
					sprintf(LogBuf,"Combine ID=%I64x ID=%I64x pID=%I64x pID=%I64x El=%s",ETbl1->GetIDI()->ID_File,ETbl2->GetIDI()->ID_File,ETbl1->pParent->GetIDI()->ID_File,ETbl2->pParent->GetIDI()->ID_File,(char*)tVol);
				else
					sprintf(LogBuf,"Combine ID=%I64x ID=%I64x pID=%I64x pID=%I64x",ETbl1->GetIDI()->ID_File,ETbl2->GetIDI()->ID_File,ETbl1->pParent->GetIDI()->ID_File,ETbl2->pParent->GetIDI()->ID_File);
				Srv->OutLog(LogBuf,this,TableName);
			}
//			if (MoLockEl(&TmpAnchor,ETbl1,AcsMd_Write)==-1) return FALSE;
//			if (MoLockEl(&TmpAnchor,ETbl2,AcsMd_Write)==-1) return FALSE;
// 			if(ETbl2->GetIDI(&TmpAnchor)->pPList->FindFirst(IDName,pVol,pCnt,ParentMt)!=-1)
// 			{
// 				do
// 				{
// 					ETbl1->GetIDI()->pPList->SetParam(IDName,pVol,pCnt,ParentMt);
// 				}
// 				while(ETbl2->GetIDI()->pPList->FindNext(IDName,pVol,pCnt,ParentMt)!=-1);
// 			};
			if (ETbl2->IDL64)
			{
				if (ETbl2->IDL64->NumElem)
				{	// Åñòü äåòè :)
					int	CntTry=0;
_TR0:
					FindStrList8	cFSL8;
					ID_Elem64*	tElT64=(ID_Elem64*)ETbl2->IDL64->Begin(cFSL8);
					while (tElT64!=NULL)
					{
						tElT3=GetElTab(&TmpAnchor,tElT64->Vol);
						if (tElT3)
						{
							if (tElT3==ETbl1)
							{
								ETbl2->DelChild(tElT64->Vol);
							}
							else
							{
								tElT3=MoveET(&TmpAnchor,tElT3,ETbl1);
/*								ETbl2->DelChild(tElT64->Vol);
								ETbl1->AddChild(tElT64->Vol,1);
								if (tElT3)
								{
									if (MoLockEl(&TmpAnchor,tElT3,AcsMd_Write)!=-1)
									{
										tElT3->pParent=ETbl1;
										tElT3->GetIDI()->ParentFile=ETbl1->GetIDI();
										if (TestEl(tElT3)!=-1)
										{
											ReplEl(tElT3);
											UnLockEl(&TmpAnchor,tElT3);
										}
									}
									tElT3=0;
								}
*/							}
							UnLockEl(&TmpAnchor,tElT3);
						}
						else
						{

						};
//						ETbl2->IDL64->Find((UCHAR*)&tElT64->Vol,sizeof(INT64),1);
//						if (ETbl2->IDL64)
							tElT64=(ID_Elem64*)ETbl2->IDL64->Next(cFSL8);
//						else
//							tElT64=NULL;
					}
					if (CntTry++<3)
						goto	_TR0;

				}
			}
			UnLockEl(&TmpAnchor,ETbl2);
//			if (ETbl2->Fl_Del==0)
			DelElTab(&TmpAnchor,ETbl2);
//				DelElTabRO(&TmpAnchor,ETbl2);
			ETbl2=NULL;
//			try	//26.07.12
//			{
				MoLockEl(&TmpAnchor,ETbl1,AcsMd_Read);
				ETbl1->GetIDI()->pPList->SetParam(IDNoDelete,&ETbl1->GetIDI()->ID_File,sizeof(int),ParentMt);
//				BLDel.DelElemID(ETbl1->GetIDI()->ID_File);
				if (Fl_NoRepl!=1)
				{
					ETbl1->Fl_Query=0;
					ReplEl(ETbl1);
				}
				ETbl1->GetIDI()->pPList->DelParam(IDNoDelete);
//				ModifElTab(&TmpAnchor,ETbl1);
				UnLockEl(&TmpAnchor,ETbl1);
// 			}
// 			catch (...)
// 			{
// 				rc=FALSE;
// 			}

		}
		else
		{
			Srv->SetCriticalError(this,"Combine different Parent","Warning","ID_Table");
		}
	}
	else
	{
		ETbl2->Fl_Query=0;
		ReplEl(ETbl2);
		ETbl1->Fl_Query=0;
		ReplEl(ETbl1);
	}
	return rc;
};


El_Tab*		IDTable::LoadEl	(StrOwner* Owner,INT64 tID)
{
	El_Tab*		tElT=NULL;
	char*		tVol;
	INT64		QID=tID;
	IDInfo*			QIDI;
	MyLock(__FILE__,__LINE__,&FlSim,this);
	QIDI=Srv->GetNewIDI(this);
	QIDI->AdrInfo=&AdrI;
	QIDI->TypeIDI=TypeIDI_COMMAND;
	QIDI->Command=Command_C_GetIDI;
	QIDI->StateBlock=StateBlock_BEGIN|StateBlock_END|StateBlock_EMPTY;
	if ((QIDI->SeansFrom==NULL)&&(ElTbl))
	{
		LockEl(&Anchor,ElTbl);
		QIDI->SeansFrom=ElTbl->GetIDI(&TmpAnchor);
	}
 	if ((QIDI)&&(pFSrv))
	{
		QIDI->ID_File=Srv->GetUID();
		Srv->GetRealTime(&QIDI->TimeBegin.FTime);
		QIDI->TimeOper.FTime=QIDI->TimeBegin.FTime;
		QIDI->TimeLastWr.FTime=QIDI->TimeBegin.FTime;
		QIDI->TimeClose.FTime=QIDI->TimeBegin.FTime;
		QIDI->pPList->SetParam(IDID,&QID,sizeof(INT64),this);
		QIDI->TypeIDI=TypeIDI_COMMAND;
		QIDI->Command=Command_C_GetIDI;
		if ((QIDI->SeansFrom==NULL)&&(ElTbl))
			QIDI->SeansFrom=ElTbl->GetIDI();
		QIDI->StateBlock=StateBlock_BEGIN|StateBlock_END|StateBlock_EMPTY;
		CntActQuery++;

		pFSrv->DataIDIEx(0,QIDI);
		if (Fl_Debug)
		{
			sprintf(LogBuf,"LoadEl ID=%I64x",tID); Srv->OutLog(LogBuf,this,TableName);
		}
//		ODataFF(3,(UCHAR*)QIDI,sizeof(IDInfo));	// Çàïðîñèòü ó âñåõ!!!
		QID=NULL;
		tElT=(El_Tab*)IDL->Find((UCHAR*)&tID,sizeof(INT64));
// 		if (tElT==NULL)
// 		{
// 			int	i=1;
// //			MyUnLock(__FILE__,__LINE__,&FlSim,this);
// 			do 
// 			{
// 				Sleep(10);i++;
// 				ODataIDI(0,QIDI);
// 				tElT=(El_Tab*)IDL->Find((UCHAR*)&tID,sizeof(INT64));
// 			} while ((tElT==NULL)&&(i<100));
// //			MyLock(__FILE__,__LINE__,&FlSim,this);
// 		}
// 
		if (tElT)
		{
			LockEl(&TmpAnchor,tElT);
			if (Fl_Debug)
			{
				if (GetParam(&TmpAnchor,tElT,IDNameElTab,tVol)==0)
				{
					sprintf(LogBuf,"LoadEl ID=%I64x Name=%s",tElT->GetIDI()->ID_File,tVol);
				}
				else
				{
					sprintf(LogBuf,"LoadEl ID=%I64x",tElT->GetIDI()->ID_File);
				}
				Srv->OutLog(LogBuf,this,TableName);
			}
		}
	}
	if (QIDI) Srv->DeleteIDI(QIDI); QIDI=NULL;
	if (tElT)
	{
		if (tElT->GetIDI()->ID_File!=tID)
		{
			char	tB[128];
			sprintf(tB,"Error LoadEl ID=%I64x Loaded ID=%I64x",tID,tElT->GetIDI()->ID_File);
			Srv->SetCriticalError(this,tB,"Warning",TableName);
			tElT->Fl_Del=1;
			IDL->Del(tElT);
			tElT=NULL;
		}
		else
		{
			Srv->GetRealTime(&tElT->GetIDI()->TimeOper.FTime);
		}
		UnLockEl(&TmpAnchor,tElT);
		LockEl(Owner,tElT);
	}
	MyUnLock(__FILE__,__LINE__,&FlSim,this);
	return	tElT;
};

int			IDTable::SendEl(El_Tab* ElT,int Fl_AddChild)
{
	int rc=0;
	char*		tVol=NULL;
	if (ElT)
		if (ElT->OwnerList)
		{
			if (ElT->OwnerList->NumElem)
			{
				LockEl(&TmpAnchor,ElT);
				if (ElT->IDL64)
				{
					ElT->GetIDI()->ID_Standart=ElT->IDL64->NumElem;
				}
				else
				{
					ElT->GetIDI()->ID_Standart=0;
				};
//				if (Fl_AddChild)
//					ElT->GetIDI(&TmpAnchor)->Command=Command_ADD_CHILD;
//				else
					ElT->GetIDI(&TmpAnchor)->Command=Command_MODIFY;
				ElT->GetIDI()->StateBlock=StateBlock_BEGIN|StateBlock_END|StateBlock_EMPTY;
				ElT->GetIDI()->Cnt=0;
//				BufODataFF(2,ElT->GetIDI());
//				ODataIDI(2,ElT->GetIDI());
				Srv->SendElTabl(ElT);
				if (Fl_Debug)
				{
					if (GetParam(&TmpAnchor,ElT,IDNameElTab,tVol)==0)
					{
						sprintf(LogBuf,"SendEl ID=%I64x Name=%s",ElT->GetIDI()->ID_File,tVol);
					}
					else
					{
						sprintf(LogBuf,"SendEl ID=%I64x",ElT->GetIDI()->ID_File);
					}
					Srv->OutLog(LogBuf,this,TableName);
				}
			}
			UnLockEl(&TmpAnchor,ElT);
		}
	return rc;
};

El_Tab*		IDTable::ModifElTab	(StrOwner* Owner, El_Tab* ElT,int Fl_Mod)
{
	char*		tVol=NULL;
	MyLock(__FILE__,__LINE__,&FlSim,this);
	if (ElT)
	{
		ElT->Fl_Query=0;
		if (ElT->Fl_Test)
			ElT->Fl_Test=0;
		if (Fl_Debug)
		{
			if (GetParam(&TmpAnchor,ElT,IDNameElTab,tVol)==0)
			{
				sprintf(LogBuf,"ModifElTab ID=%I64x Name=%s Owner=%s",ElT->GetIDI()->ID_File,tVol,Owner->pDec->Name_Mt);
			}
			else
			{
				sprintf(LogBuf,"ModifElTab ID=%I64x Owner=%s",ElT->GetIDI()->ID_File,Owner->pDec->Name_Mt);
			}
			Srv->OutLog(LogBuf,this,TableName);
		}
//		if (MoLockEl(Owner,ElT,AcsMd_Write)!=-1)
		{
//			MyLock(__FILE__,__LINE__,&Fl_RE,this);
//			AddParam(Owner,ElT,IDHostName,"*",1);
			AddParam(Owner,ElT,IDMtName,Owner->pDec->Name_Mt,1);
			if (Fl_Mod==2)
			{
				ElT->GetIDI()->TimeLastWr.Time++;
				ElT->GetIDI()->ModifCnt++;
			}
			else
			{
				Srv->GetRealTime(&ElT->GetIDI()->TimeLastWr.FTime);
				ElT->GetIDI()->ModifCnt+=2;
			}
			UpDataEl(ElT,Fl_Mod);
			SaveEl(ElT);	 //111111111111111111111
//			MyUnLock(__FILE__,__LINE__,&Fl_RE,this);
			SendEl(ElT);
//			MyLock(__FILE__,__LINE__,&Fl_RE,this);
//			if (ElT)
//				if (MoLockEl(Owner,ElT,AcsMd_Read)==-1)
//				{
//					Srv->SetCriticalError(this,"Error LockEl Read","Warning",TableName);
//				};
			if (TestEl(ElT)!=-1)
			{
				strcpy(LastHost,"*");
				ReplEl(ElT);
//			UnLockEl(&TmpAnchor,ElT);
			}
			else
				ElT=NULL;
		}
//		else
//		{
//			Srv->SetCriticalError(this,"Error LockEl Write in ModifElTab","Error",TableName);
//		};
	}
	MyUnLock(__FILE__,__LINE__,&FlSim,this);
	return	ElT;
};

El_Tab*		IDTable::ModifElTabI	(StrOwner* Owner, El_Tab* ElT,int Fl_Mod)
{
//	if (MoLockEl(Owner,ElT,AcsMd_Write)!=-1)
	{
		ElT->Fl_Query=0;
		if (Fl_Mod==2)
		{
			ElT->GetIDI(Owner)->TimeOper.Time++;
			ElT->GetIDI()->TimeLastWr.Time++;
			ElT->GetIDI()->ModifCnt++;
		}
		else
		{
			Srv->GetRealTime(&ElT->GetIDI(Owner)->TimeOper.FTime);
			Srv->GetRealTime(&ElT->GetIDI()->TimeLastWr.FTime);
			ElT->GetIDI()->ModifCnt+=2;
		}
		UpDataEl(ElT);
		SendEl(ElT);
		//	if (ElT)
		//		if (LockEl(Owner,ElT,AcsMd_Read)==-1)
		//		{
		//			Srv->SetCriticalError(this,"Error LockEl Read","Warning",TableName);
		//		};
		if (TestEl(ElT)!=-1)
		{
			ReplEl(ElT);
		}
		else
			ElT=NULL;
	}
//	else
//	{
//		Srv->SetCriticalError(this,"Error LockEl Write in ModifElTabI","Error",TableName);
//	};
	return	ElT;
};

El_Tab*	IDTable::CreateNewEl()
{
	El_Tab*		tElT=NULL;
	if (Fl_AntyRec==0)
//	if (FALSE)
	{
		Fl_AntyRec=1;
		ElTblForNew=FindNested(&TmpAnchor,AcsMd_Read,N_Domain,Srv->MainDomainName,N_Host,Srv->ShortHostName,"ForNewList",NULL);
		if (ElTblForNew)
		{
			FindStruct	tFStr;
_CrNew001:
			tElT=FindFirstET(&TmpAnchor,&tFStr,AcsMd_Read,ElTblForNew);
			if (tElT)
			{
				if ((tElT->IDL64)&&(tElT->IDL64->NumElem!=0))
				{
					DelElTab(&TmpAnchor,tElT);
					tElT=NULL;
					goto _CrNew001;
				}
				else
				{
					ElTblForNew->DelChild(tElT->GetIDI()->ID_File);
					tElT->Init(&TmpAnchor,tElT->GetIDI()->ID_File,this);
				}
			}
		}
		Fl_AntyRec=0;
	}
	if (tElT==NULL)
	{
		tElT=(El_Tab*)IDL->NewElem();
		if (tElT==NULL)
			tElT=new El_Tab(&TmpAnchor,Srv->GetUID(),this);
		else
			tElT->Init(&TmpAnchor,Srv->GetUID(),this);
	}
	tElT->Fl_Query=0;
	return tElT;
};	// Used DelList


El_Tab*	IDTable::NewElTab(StrOwner* Owner, El_Tab* ElTParent)	// Ñîçäàòü çàïèñü â òàáëèöå
{
	El_Tab*		tElT=NULL;
	El_Tab*		tElTP=NULL;
	if (ElTParent==NULL)
	{
		if (ElTbl==NULL)
		{	// Åñëè åëåìåíò óæå åñòü òî íåîáõîäèìî åãî ïåðåîïðåäåëèòü (èëè óäàëèòü !!!)
			return tElT;	// Error
		}
		tElTP=(El_Tab*)IDL->Find((UCHAR*)&ElTbl->GetIDI()->ID_File,sizeof(INT64));
		if (tElTP==NULL)
		{
			return tElT;	// Error
		}
		if (MoLockEl(&TmpAnchor,ElTbl,AcsMd_Write)!=-1)
		{
			tElT=CreateNewEl();
			tElT->pParent=ElTbl;
			tElT->GetIDI()->ParentFile=ElTbl->GetIDI();
			tElT->GetIDI()->SeansFrom=ElTbl->GetIDI();
			ElTbl->AddChild(tElT->GetIDI()->ID_File,1);
//			LockEl(&TmpAnchor,tElT);
//			MoLockEl(&TmpAnchor,tElT,AcsMd_Write);
			if ((tElT->GetIDI()->ID_File==tElT->GetIDI()->ID_ParentFile)||((tElT->GetIDI()->ParentFile)&&(tElT->GetIDI()->ID_File==tElT->GetIDI()->ParentFile->ID_File)))
			{
				{size_t iasm=1;}
			}
			else
				IDL->Add(tElT);
			//		AddParam(&TmpAnchor,tElT,IDHostName,Srv->HostName);
			//		AddParam(&TmpAnchor,tElT,IDMtName,this->Name_Mt);
			//		ModifElTab(Owner,tElT);
			//		UpDataEl(tElT);	// ñàì ýëåìåíò
			//		SendEl(tElT);
			//		if (TestEl(tElT)!=-1)
			//		{
			//			UnLockEl(&TmpAnchor,tElT);
			//		}
			//		else
			//		{
			//			tElT=0;
			//		}
			ElTbl->GetIDI()->ModifCnt++;
			UpDataEl(ElTbl);	// ðîäèòåëü 
			SendEl(ElTbl);
			ElTbl->Fl_Query=0;
			ReplEl(ElTbl);
			UnLockEl(&TmpAnchor,ElTbl);
		}
		else
		{
			Srv->SetCriticalError(this,"Error LockEl Write in NewElTab","Error",TableName);
		};
	}
	else
	{
		tElTP=(El_Tab*)IDL->Find((UCHAR*)&ElTParent->GetIDI()->ID_File,sizeof(INT64));
		if (tElTP==NULL)
		{
			return tElT;	// Error
		}
		LockEl(&TmpAnchor,ElTParent);
		if (MoLockEl(&TmpAnchor,ElTParent,AcsMd_Write)!=-1)
		{
			tElT=CreateNewEl();
			tElT->pParent=ElTParent;
			tElT->GetIDI()->ParentFile=ElTParent->GetIDI();
			tElT->GetIDI()->SeansFrom=ElTbl->GetIDI();
			ElTParent->AddChild(tElT->GetIDI()->ID_File,1);
			LockEl(&TmpAnchor,tElT);
//			MoLockEl(&TmpAnchor,tElT,AcsMd_Write);
			if ((tElT->GetIDI()->ID_File==tElT->GetIDI()->ID_ParentFile)||((tElT->GetIDI()->ParentFile)&&(tElT->GetIDI()->ID_File==tElT->GetIDI()->ParentFile->ID_File)))
			{
				{size_t iasm=1;}
			}
			else
				IDL->Add(tElT);
			//		AddParam(&TmpAnchor,tElT,IDHostName,Srv->HostName);
			//		AddParam(&TmpAnchor,tElT,IDMtName,this->Name_Mt);
			//		ModifElTab(Owner,tElT);
			//		UpDataEl(tElT);	// ñàì ýëåìåíò
			//		SendEl(tElT);
			//		if (TestEl(tElT)!=-1)
			//		{
			//			UnLockEl(&TmpAnchor,tElT);
			//		}
			//		else
			//		{
			//			tElT=0;
			//		}
			ElTParent->GetIDI()->ModifCnt++;
			UpDataEl(ElTParent);	// ðîäèòåëü 
			SendEl(ElTParent);
			ElTParent->Fl_Query=0;
			ReplEl(ElTParent);
			UnLockEl(&TmpAnchor,ElTParent);
		}
		else
		{
			Srv->SetCriticalError(this,"Error LockEl Write in NewElTab","Error",TableName);
		};
	}
	if (tElT)
	{
//		LockEl(&TmpAnchor,tElT);
		if (MoLockEl(&TmpAnchor,tElT,AcsMd_Read)==-1)
		{
			Srv->SetCriticalError(this,"Error LockEl Read","Warning",TableName);
		};
		SaveEl(tElT);
		ReplEl(tElT);
		UnLockEl(&TmpAnchor,tElT);
		Srv->GetRealTime(&tElT->GetIDI()->TimeOper.FTime);
		LockEl(Owner,tElT);
		tElT->GetIDI()->AdrInfo=&AdrI;
	}
	if (tElT->Fl_Del)
		tElT=NULL;
	return tElT;
}


El_Tab*	IDTable::NewElTab(StrOwner* Owner,char* tVol, El_Tab* ElTParent)	// Ñîçäàòü çàïèñü â òàáëèöå
{
	El_Tab*		tElT=NULL;
	El_Tab*		tElTP=NULL;
	if (ElTParent==NULL)
	{
		if (ElTbl==NULL)
		{	// Åñëè åëåìåíò óæå åñòü òî íåîáõîäèìî åãî ïåðåîïðåäåëèòü (èëè óäàëèòü !!!)
			return tElT;
		}
		tElTP=(El_Tab*)IDL->Find((UCHAR*)&ElTbl->GetIDI()->ID_File,sizeof(INT64));
		if (tElTP==NULL)
		{
			return tElT;	// Error
		}
		if (MoLockEl(&TmpAnchor,ElTbl,AcsMd_Write)!=-1)
		{
			tElT=CreateNewEl();
			AddParam(&TmpAnchor,tElT,IDNameElTab,tVol,1);
			tElT->pParent=ElTbl;
			tElT->GetIDI()->ParentFile=ElTbl->GetIDI();
			tElT->GetIDI()->SeansFrom=ElTbl->GetIDI();
			ElTbl->AddChild(tElT->GetIDI()->ID_File,1);
			LockEl(&TmpAnchor,tElT);
//			MoLockEl(&TmpAnchor,tElT,AcsMd_Write);
			if ((tElT->GetIDI()->ID_File==tElT->GetIDI()->ID_ParentFile)||((tElT->GetIDI()->ParentFile)&&(tElT->GetIDI()->ID_File==tElT->GetIDI()->ParentFile->ID_File)))
			{
				{size_t iasm=1;}
			}
			else
				IDL->Add(tElT);
			//		AddParam(&TmpAnchor,tElT,IDHostName,Srv->HostName);
			//		AddParam(&TmpAnchor,tElT,IDMtName,this->Name_Mt);
			//		ModifElTab(Owner,tElT);
			//		UpDataEl(tElT);	// ñàì ýëåìåíò
			//		SendEl(tElT);
			//		if (TestEl(tElT)!=-1)
			//		{
			//			UnLockEl(&TmpAnchor,tElT);
			//		}
			//		else
			//		{
			//			tElT=0;
			//		}
			ElTbl->GetIDI()->ModifCnt++;
			UpDataEl(ElTbl);	// ðîäèòåëü 
			SendEl(ElTbl);
			ElTbl->Fl_Query=0;
			ReplEl(ElTbl);
		}
		else
		{
			Srv->SetCriticalError(this,"Error LockEl Write in NewElTab","Error",TableName);
		};
		UnLockEl(&TmpAnchor,ElTbl);
	}
	else
	{
		tElTP=(El_Tab*)IDL->Find((UCHAR*)&ElTParent->GetIDI()->ID_File,sizeof(INT64));
		if (tElTP==NULL)
		{
			return tElT;	// Error
		}
		LockEl(&TmpAnchor,ElTParent);
		if (MoLockEl(&TmpAnchor,ElTParent,AcsMd_Write)!=-1)
		{
			tElT=CreateNewEl();
			AddParam(&TmpAnchor,tElT,IDNameElTab,tVol,1);
			tElT->pParent=ElTParent;
			tElT->GetIDI()->ParentFile=ElTParent->GetIDI();
			tElT->GetIDI()->SeansFrom=ElTbl->GetIDI();
			ElTParent->AddChild(tElT->GetIDI()->ID_File,1);
			LockEl(&TmpAnchor,tElT);
//			MoLockEl(&TmpAnchor,tElT,AcsMd_Write);
			if ((tElT->GetIDI()->ID_File==tElT->GetIDI()->ID_ParentFile)||((tElT->GetIDI()->ParentFile)&&(tElT->GetIDI()->ID_File==tElT->GetIDI()->ParentFile->ID_File)))
			{
				{size_t iasm=1;}
			}
			else
				IDL->Add(tElT);
			ModifElTab(Owner,tElT);
			//		UpDataEl(tElT);	// ñàì ýëåìåíò
			//		SendEl(tElT);
			//		if (TestEl(tElT)!=-1)
			//		{
			//			UnLockEl(&TmpAnchor,tElT);
			//		}
			//		else
			//		{
			//			tElT=0;
			//		}
			ElTParent->GetIDI()->ModifCnt++;
			UpDataEl(ElTParent);	// ðîäèòåëü 
			SendEl(ElTParent);
			ElTParent->Fl_Query=0;
			ReplEl(ElTParent);

		}
		else
		{
			Srv->SetCriticalError(this,"Error LockEl Write in NewElTab","Error",TableName);
		};
		UnLockEl(&TmpAnchor,ElTParent);
	}
	if (tElT)
	{
//		LockEl(&TmpAnchor,tElT);
		if (MoLockEl(&TmpAnchor,tElT,AcsMd_Read)==-1)
		{
			Srv->SetCriticalError(this,"Error LockEl Read","Warning",TableName);
		};
		SaveEl(tElT);
		ReplEl(tElT);
		UnLockEl(&TmpAnchor,tElT);
		Srv->GetRealTime(&tElT->GetIDI()->TimeOper.FTime);
		LockEl(Owner,tElT);
	}
	if (tElT->Fl_Del)
		tElT=NULL;
	return tElT;
}
//!!!!!!!!!!!!!!!!!!!!!!1
El_Tab*	IDTable::NewElTab(StrOwner* Owner,INT64 tID, El_Tab* ElTParent)	// Ñîçäàòü çàïèñü â òàáëèöå
{
	El_Tab*		tElT=NULL;
	El_Tab*		tElTP=NULL;
	if (ElTParent==NULL)
	{
		if (ElTbl==NULL)
		{	// Åñëè åëåìåíò óæå åñòü òî íåîáõîäèìî åãî ïåðåîïðåäåëèòü (èëè óäàëèòü !!!)
			ElTbl=new El_Tab(&TmpAnchor,ID,this);
//			ElTbl->GetIDI()->ID_File=ID;
			SaveEl(ElTbl);
		}
		tElTP=(El_Tab*)IDL->Find((UCHAR*)&ElTbl->GetIDI()->ID_File,sizeof(INT64));
		if (tElTP==NULL)
		{
			return tElT;	// Error
		}
		LockEl(&Anchor,ElTbl);
		if (MoLockEl(&Anchor,ElTbl,AcsMd_Write)!=-1)
		{
			tElT=CreateNewEl();
			tElT->pParent=ElTbl;
			tElT->GetIDI()->ParentFile=ElTbl->GetIDI();
			tElT->GetIDI()->SeansFrom=ElTbl->GetIDI();
			ElTbl->AddChild(tElT->GetIDI()->ID_File,1);
			LockEl(&TmpAnchor,tElT);
//			MoLockEl(&TmpAnchor,tElT,AcsMd_Write);
			if ((tElT->GetIDI()->ID_File==tElT->GetIDI()->ID_ParentFile)||((tElT->GetIDI()->ParentFile)&&(tElT->GetIDI()->ID_File==tElT->GetIDI()->ParentFile->ID_File)))
			{
				{size_t iasm=1;}
			}
			else
				IDL->Add(tElT);
			ReplEl(tElT);
			//		ModifElTab(Owner,tElT);
			//		UpDataEl(tElT);	// ñàì ýëåìåíò
			//		SendEl(tElT);
			//		if (TestEl(tElT)!=-1)
			//		{
			//			UnLockEl(&TmpAnchor,tElT);
			//		}
			//		else
			//		{
			//			tElT=0;
			//		}
			Srv->GetRealTime(&tElT->GetIDI()->TimeOper.FTime);
			ElTbl->GetIDI()->ModifCnt++;
			UpDataEl(ElTbl);	// ðîäèòåëü 
			SendEl(ElTbl);
			ElTbl->Fl_Query=0;
			ReplEl(ElTbl);
		}
		else
		{
			Srv->SetCriticalError(this,"Error LockEl Write in NewElTab","Error",TableName);
		};
		UnLockEl(&Anchor,ElTbl);
	}
	else
	{
		tElTP=(El_Tab*)IDL->Find((UCHAR*)&ElTParent->GetIDI()->ID_File,sizeof(INT64));
		if (tElTP==NULL)
		{
			return tElT;	// Error
		}
//		LockEl(&TmpAnchor,ElTParent);
//		MoLockEl(Owner,ElTParent,AcsMd_Read);
		if (MoLockEl(&TmpAnchor,ElTParent,AcsMd_Write)!=-1)
		{
			tElT=CreateNewEl();
			tElT->pParent=ElTParent;
			tElT->GetIDI()->ParentFile=ElTParent->GetIDI();
			tElT->GetIDI()->SeansFrom=ElTbl->GetIDI();
			ElTParent->AddChild(tElT->GetIDI()->ID_File,1);
//			LockEl(&TmpAnchor,tElT);
//			MoLockEl(&TmpAnchor,tElT,AcsMd_Write);
			if ((tElT->GetIDI()->ID_File==tElT->GetIDI()->ID_ParentFile)||((tElT->GetIDI()->ParentFile)&&(tElT->GetIDI()->ID_File==tElT->GetIDI()->ParentFile->ID_File)))
			{
				{size_t iasm=1;}
			}
			else
				IDL->Add(tElT);
			//		ModifElTab(Owner,tElT);
			//		UpDataEl(tElT);	// ñàì ýëåìåíò
			//		SendEl(tElT);
			//		if (TestEl(tElT)!=-1)
			//		{
			//			UnLockEl(&TmpAnchor,tElT);
			//		}
			//		else
			//		{
			//			tElT=0;
			//		}
			ElTParent->GetIDI()->ModifCnt++;
			UpDataEl(ElTParent);	// ðîäèòåëü 
			SendEl(ElTParent);
			ElTParent->Fl_Query=0;
			ReplEl(ElTParent);
		}
		else
		{
			Srv->SetCriticalError(this,"Error LockEl Write in NewElTab","Error",TableName);
		};
		UnLockEl(&TmpAnchor,ElTParent);
	}
	if (tElT)
		{
//		LockEl(&TmpAnchor,tElT);
//		if (MoLockEl(&TmpAnchor,tElT,AcsMd_Read)==-1)
//		{
//			Srv->SetCriticalError(this,"Error LockEl Read","Warning",TableName);
//		};
		SaveEl(tElT);
		ReplEl(tElT);
		UnLockEl(&TmpAnchor,tElT);
		Srv->GetRealTime(&tElT->GetIDI()->TimeOper.FTime);
		LockEl(Owner,tElT);
	}
	if (tElT->Fl_Del)
		tElT=NULL;
	return tElT;
}
/*
int		IDTable::DelElTabStep	(StrOwner* Owner, El_Tab* ElT,int Lev,int Fl_NoR)
{
	int rc=0;
	El_Tab*			tElT=NULL;
	ID_Elem64*		tElT64=NULL;
	// Âòîðûì äåëîì ìî÷èì äåòåé 8)
	if (ElT->Fl_Del!=0)
	{
		Srv->SetCriticalError(Owner->pDec,"DelStep Fl_Del=1","Warning");
		return rc;
	}
	MoLockEl(&TmpAnchor,ElT,AcsMd_Write);
//	BLDel.Add(ElT->GetIDI()->ID_File);
	if (Lev<100)
	{
//		if (Lev>50)
//			{size_t iasm=1;}
		FindStrList8	FSL8;
		if (ElT->IDL64)
			tElT64=(ID_Elem64*)ElT->IDL64->Begin(FSL8);
		while (tElT64!=NULL)
		{
			if (!BLDel.Find(tElT64->Vol))
			{
				tElT=GetElTab(&TmpAnchor,tElT64->Vol,1);
				if ((tElT)&&(ElT!=tElT))
				{
					if ((tElT->pParent!=ElT)||(tElT->GetIDI()->ID_File!=tElT64->Vol))
					{
						UnLockEl(&TmpAnchor,tElT);
						ElT->DelChild(tElT64->Vol);
//						Srv->SetCriticalError(Owner->pDec,"Del Error","Warning");
					}
					else
					{
						if (tElT->Fl_Del==0)
							DelElTabStep(&TmpAnchor,tElT,Lev+1,1);
						else
						{
							UnLockEl(&TmpAnchor,tElT);							// !!!!!!
							ElT->DelChild(tElT64->Vol);
							Srv->SetCriticalError(Owner->pDec,"Fl_Del=1","Warning");
						}
					}

				} 
				else
				{
					UnLockEl(&TmpAnchor,tElT);
					ElT->DelChild(tElT64->Vol);
				}// !!!!!!
			}
			else
			{
				ElT->DelChild(tElT64->Vol);
			}
//			tElT64=(ID_Elem64*)ElT->IDL64->Find((UCHAR*)&tElT64->Vol,sizeof(INT64));
			tElT64=(ID_Elem64*)ElT->IDL64->Next(FSL8);
			if (tElT64==NULL)
				tElT64=(ID_Elem64*)ElT->IDL64->Begin(FSL8);
		}
	}
	else
	{
		Srv->SetCriticalError(Owner->pDec,"DelStep>100","Error");
	}
	if (Lev<100)
		if(ElT->pParent)
			ElT->pParent->DelChild(ElT->GetIDI()->ID_File);
	UnLockEl(&TmpAnchor,ElT);
	DelEl(ElT,Fl_NoR);
//	if (ElT->OwnerList)
//	{
//		if (ElT->OwnerList->NumElem)
//		{
//			ElT->GetIDI()->Command=Command_DEL;
//			ElT->GetIDI()->StateBlock=StateBlock_BEGIN|StateBlock_END;
//			ElT->GetIDI()->NumPack++;
//			ODataFF(2,(UCHAR*)ElT->GetIDI(),sizeof(IDInfo));
//			if (Fl_NoR==0)
//				ODataFF(3,(UCHAR*)ElT->GetIDI(),sizeof(IDInfo));
//		}
//	};
	return rc;
};				// Óäàëèòü åëåìåíò â òàáëèöå
*/
/*
int		IDTable::DelElTabRO(StrOwner* Owner, El_Tab* ElT,int Fl_NoR)
{
	int rc=0;
	El_Tab*		tElT=ElT;
//	MyLock(__FILE__,__LINE__,&Fl_RE,this);
	INT64		tID=ElT->GetIDI()->ID_File;
	// Ïåðâûì äåëîì ìî÷èì äåòåé 8)
	char*		tVol;
//	INT64*		tVol2;
	INT64		Atr=0;
//	AddParam(&TmpAnchor,ElT,IDAttribute,Atr,1);
//	Srv->GetRealTime(&ElT->GetIDI()->TimeLastWr.FTime);ElT->GetIDI()->ModifCnt++;
//	ReplEl(ElT);
	if (Fl_Debug)
	{
		if (GetParam(&TmpAnchor,ElT,IDNameElTab,tVol)==0)
		{
			sprintf(LogBuf,"DelElTabRO ID=%I64x Owner=%s Name=%s",ElT->GetIDI()->ID_File,Owner->pDec->Name_Mt,tVol);
		}
		else
		{
			sprintf(LogBuf,"DelElTabRO ID=%I64x Owner=%s",ElT->GetIDI()->ID_File,Owner->pDec->Name_Mt);
		}
		//	UnLockEl(&TmpAnchor,ElT);
		Srv->OutLog(LogBuf,this,TableName);
	}
	if (Fl_NoR==0)
	{
		char	ttBf[80];
		char	ttBf2[10];
		BLDel.Add(ElT->GetIDI()->ID_File);
		if (Fl_Debug)
		{
			if (tVol)
				sprintf(ttBf,"DelRO Elem ID=%I64x Owner=%s Name=%s",ElT->GetIDI()->ID_File,Owner->pDec->Name_Mt,tVol);
			else
				sprintf(ttBf,"DelRO Elem ID=%I64x Owner=%s ",ElT->GetIDI()->ID_File,Owner->pDec->Name_Mt);
			sprintf(ttBf2,"DEl%I64d",ID);
			Srv->OutLog(ttBf,this,ttBf2);
		}
	}
	tElT=ElT->pParent;
	DelElTabStep(&TmpAnchor,ElT,0,Fl_NoR);
	if (tElT)
	{
		tElT->DelChild(tID);
		if (Fl_NoR==0)
		{
			tElT->GetIDI()->ModifCnt++;
			UpDataEl(tElT);
//			MyUnLock(__FILE__,__LINE__,&Fl_RE,this);
			SendEl(tElT);
//			MyLock(__FILE__,__LINE__,&Fl_RE,this);
			ReplEl(tElT);
		}
	}
//	MyUnLock(__FILE__,__LINE__,&Fl_RE,this);
	return rc;
};

int		IDTable::DelElTabWDL(StrOwner* Owner, El_Tab* ElT)
{
	int rc=0;
	El_Tab*		tElT;
//	MyLock(__FILE__,__LINE__,&Fl_RE,this);
	INT64		tID=ElT->GetIDI()->ID_File;
	// Ïåðâûì äåëîì ìî÷èì äåòåé 8)
	char*		tVol;
//	INT64*		tVol2;
	if (Fl_Debug)
	{
		if (GetParam(&TmpAnchor,ElT,IDNameElTab,tVol)==0)
		{
			sprintf(LogBuf,"DelElTab ID=%I64x Owner=%s Name=%s",ElT->GetIDI()->ID_File,Owner->pDec->Name_Mt,tVol);
		}
		else
		{
			sprintf(LogBuf,"DelElTab ID=%I64x Owner=%s",ElT->GetIDI()->ID_File,Owner->pDec->Name_Mt);
		}
		Srv->OutLog(LogBuf,this,TableName);
	}
//	if (Fl_NoR==0)		// !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
//	{
//		char	ttBf[80];
//		char	ttBf2[10];
//		BLDel.Add(ElT->GetIDI()->ID_File);
//		if (tVol)
//			sprintf(ttBf,"Del Elem ID=%I64x Owner=%s Name=%s",ElT->GetIDI()->ID_File,Owner->pDec->Name_Mt,tVol);
//		else
//			sprintf(ttBf,"Del Elem ID=%I64x Owner=%s ",ElT->GetIDI()->ID_File,Owner->pDec->Name_Mt);
//		sprintf(ttBf2,"DEl%I64d",ID);
//		Srv->OutLog(ttBf,this,ttBf2);
//	}
	tElT=ElT->pParent;
	DelElTabStep(&TmpAnchor,ElT,0,0);
	if (tElT)
	{
		tElT->DelChild(tID);
//		if (Fl_NoR==0)
		{
			UpDataEl(tElT);
//			MyUnLock(__FILE__,__LINE__,&Fl_RE,this);
			SendEl(tElT);
//			MyLock(__FILE__,__LINE__,&Fl_RE,this);
			ReplEl(tElT);
		}
	}
//	MyUnLock(__FILE__,__LINE__,&Fl_RE,this);
	return rc;
}; 
*/
int		IDTable::DelElTab(StrOwner* Owner, El_Tab* ElT,int Fl_NoR)
{
	int rc=0;
	El_Tab*		tElT;
	El_Tab*		tElT2=NULL;
	char	ttBf[80];
//	MyLock(__FILE__,__LINE__,&Fl_RE,this);
	INT64		tID=ElT->GetIDI(Owner)->ID_File;

	if (ID==TabParametr)
	{
		rc=(int)IDL->Del(ElT);			//!!!!!!!!!!!!!!!!!!!!???????????????????
		return 0;
	}
	// Ïåðâûì äåëîì ìî÷èì äåòåé 8)
	char*		tVol=NULL;
//	INT64*		tVol2;
	if (Fl_Debug)
	{
		if (GetParam(&TmpAnchor,ElT,IDNameElTab,tVol)==0)
		{
			sprintf(LogBuf,"DelElTab ID=%I64x Owner=%s Name=%s",ElT->GetIDI()->ID_File,Owner->pDec->Name_Mt,tVol);
		}
		else
		{
			sprintf(LogBuf,"DelElTab ID=%I64x Owner=%s",ElT->GetIDI()->ID_File,Owner->pDec->Name_Mt);
		}
	//	UnLockEl(&TmpAnchor,ElT);
		Srv->OutLog(LogBuf,this,TableName);
	}
	if (Fl_NoR==0)
	{
		char	ttBf2[10];
//		BLDel.Add(ElT->GetIDI()->ID_File);
		if (Fl_Debug)
		{
			if (tVol)
				sprintf(ttBf,"Del Elem ID=%I64x Owner=%s Name=%s",ElT->GetIDI()->ID_File,Owner->pDec->Name_Mt,tVol);
			else
				sprintf(ttBf,"Del Elem ID=%I64x Owner=%s ",ElT->GetIDI()->ID_File,Owner->pDec->Name_Mt);
			sprintf(ttBf2,"DEl%I64d",ID);
			Srv->OutLog(ttBf,this,ttBf2);
		}
	}
	tElT=ElT->pParent;
//		tElT->DelChild(tID);
	ElTblDel=FindNested(&TmpAnchor,AcsMd_Read,"DelList",NULL);
	if (ElTblDel==NULL)
	{
//		ElTblDel=FindNested(&TmpAnchor,AcsMd_Read,"DelList",NULL);
			//			ElTblDel=AttainNested(&TmpAnchor,"%s","DelList",-1);
  		if (ElTbl)
  			ElTblDel=AddETI(&TmpAnchor,"DelList",ElTbl);
	}
	if (ElTblDel)
	{
		tElT2=MoveETInternal(&TmpAnchor,ElT,ElTblDel);
		if (tElT2)
		{
			tElT2->GetIDI()->pPList->DelAllElem();
			sprintf(ttBf,"%I64x",tElT2->GetIDI()->ID_File);
			AddParam(&TmpAnchor,tElT2,IDNameElTab,ttBf,1);
			Srv->GetRealTime(&tElT2->GetIDI()->TimeBegin.FTime);
		}
	}
	else
	{
		rc=(int)IDL->Del(ElT);
	}
//	ModifElTab(&TmpAnchor,tElT);
//	UnLockEl(&TmpAnchor,tElT);
//	DelElTabStep(&TmpAnchor,ElT,0,Fl_NoR);
 	if (tElT2)
 	{
// 		if (Fl_NoR==0)
// 		{
// 			tElT2->GetIDI()->ModifCnt++;
// 			UpDataEl(tElT2);
// 			SendEl(tElT2);
// 			ReplEl(tElT2);
// 		}
 		UnLockEl(&TmpAnchor,tElT2);
 	}
//	MyUnLock(__FILE__,__LINE__,&Fl_RE,this);
	return rc;
};
/*
int	IDTable::DelEl(El_Tab* ElT,int Fl_NoR)
{
	int rc=0;
	char*		tVol=NULL;
//	INT64*		tVol2;
	if (ElT)
	{
//		if (!BLDel.Find(ElT->GetIDI()->ID_File))
		ElT->Fl_Del=1;
		{
			if (Fl_Debug)
			{
				if (GetParam(&TmpAnchor,ElT,IDNameElTab,tVol)==0)
					sprintf(LogBuf,"DelEl ID=%I64x Name=%s",ElT->GetIDI()->ID_File,tVol);
				else
					sprintf(LogBuf,"DelEl ID=%I64x",ElT->GetIDI()->ID_File);
				Srv->OutLog(LogBuf,this,"IDTableDel");
			}
//			if (ElT->GetIDI()->ID_File==0x1d2d0000519d47b1)
//			{
//				while(true) {Beep(1000,1000); Sleep(1000); }
//			}
			ElT->GetIDI()->Command=Command_DEL;
			ElT->GetIDI()->TypeIDI=TypeIDI_TABLE;
			ElT->GetIDI()->StateBlock=StateBlock_BEGIN|StateBlock_END;
			ElT->GetIDI()->NumPack=1;
			Srv->GetRealTime(&ElT->GetIDI()->TimeOper.FTime);
//			strcpy(LastHost,"*");
//			ElT->GetIDI()->pPList->SetParam(IDHostName,&LastHost,strlen(LastHost)+1,this);
			ElT->GetIDI()->AdrInfo->DestinAI.HostName[0]=0;
//			ElT->GetIDI()->pPList->DelParam(IDDomainName);
			ElT->GetIDI()->AdrInfo->DestinAI.DomainName[0]=0;
//			ElT->GetIDI()->TimeOper.FTime=ElT->GetIDI()->TimeBegin.FTime;
//			ElT->GetIDI()->TimeLastWr.FTime=ElT->GetIDI()->TimeBegin.FTime;
			ElT->GetIDI()->TimeClose.FTime=ElT->GetIDI()->TimeOper.FTime;
//			char tBbb[512];
//			sprintf(tBbb,"SendDel El ID=%I64x Command=%d",ElT->GetIDI()->ID_File,ElT->GetIDI()->Command);
//			Srv->OutLog(tBbb,this,"ServicesUpData");
			Srv->SendElTabl(ElT);
//			ODataIDI(2,ElT->GetIDI(),2);	// !!!!!
			if (Fl_NoR==0)
				Srv->ReplElTabl(ElT);
//			ODataIDI(3,ElT->GetIDI());
			pFSrv->DataIDIEx(0,ElT->GetIDI());
			UnLockEl(&TmpAnchor,ElT);
		}
		if ((ElT->OwnerList)&&(ElT->OwnerList->NumElem))
		{
			El_Owner*	tDec;
			MyLock(__FILE__,__LINE__,&ElT->Fl_RE,this);
			do 
			{
				FindStrList8	FSL8;
				tDec=(El_Owner*)ElT->OwnerList->Begin(FSL8);
				if (tDec)
				{
					UnLockEl(&tDec->Own,ElT);
				}
			} while (tDec);
			MyUnLock(__FILE__,__LINE__,&ElT->Fl_RE,this);
		}

//		rc=(int)IDL->Del(ElT);			//!!!!!!!!!!!!!!!!!!!!???????????????????
	}
	return rc;
};
*/
void	IDTable::CreatePrmList	(StrOwner* Owner, FindStruct* FndStr,El_Tab* tEl)
{
	El_Tab*		tElT=NULL;
	ID_Elem64*	tID=NULL;
	FindStrList8	FSL8;
	tID=(ID_Elem64*)tEl->IDL64->Begin(FSL8);
	while (tID)
	{
		tElT=(El_Tab*)IDL->Find((UCHAR*)&tID->Vol,sizeof(INT64));
		if (tElT==NULL)
		{
			tElT=GetElTab(&TmpAnchor,tID->Vol);
		}
		if(tElT)
		{
			char*	tVol;
			if (GetParam(Owner,tElT,FndStr->IDPrm,tVol)==0)
			{
				ID_ElemChar*	ElCh=new ID_ElemChar(tVol,tID->Vol);
				if (FndStr->IDL->Add(ElCh)==-1)
				{
					delete ElCh;
				};
			}
			UnLockEl(&TmpAnchor,tElT);							// !!!!!!
		}
//		tID=(ID_Elem64*)tEl->IDL64->Find((UCHAR*)&tID->Vol,sizeof(INT64),1);
		tID=(ID_Elem64*)tEl->IDL64->Next(FSL8);
	}
}

int		IDTable::UnLockAllEl	(StrOwner* Owner)
{
	int	rc=-1;
	El_Tab*	tElS;
//	MyLock(__FILE__,__LINE__,&Fl_RE,this);
	FindStrList8	FSL8;
	tElS=(El_Tab*)IDL->Begin(FSL8);
	while (tElS)
	{
		UnLockEl(Owner,tElS);
		tElS=(El_Tab*)IDL->Next(FSL8);
	}
//	MyUnLock(__FILE__,__LINE__,&Fl_RE,this);
	return rc;
};

El_Tab*	IDTable::FindUnLockET	(StrOwner* Owner, FindStruct* FndStr, int AcsMode, El_Tab* ElTParent)
{
	El_Tab*		tElT=NULL;

	return		tElT;
};

void	IDTable::DelAllTables()
{
	MyLock(__FILE__,__LINE__,&FlSim,this);
	IDL_Query->DelAllElem();
	IDL_Test->DelAllEl();
	IDL->DelAllElem();
	SetZRoot();
	MyUnLock(__FILE__,__LINE__,&FlSim,this);
};


El_Tab*	IDTable::FindFirstET	(StrOwner* Owner, FindStruct* FndStr, int AcsMode, El_Tab* ElTParent,int fl_repl)
{
//	int		CCnt=0;
	El_Tab*		tElT=NULL;
	ID_Elem64*	tID=NULL;
//	FindStrList8	FSL8;
	MyLock(__FILE__,__LINE__,&FlSim,this);
//_m000:
//	FndStr->ReInit();
	if (ElTParent==NULL)
	{
		char	ttt[128];
		sprintf(ttt,"FindFirstET with ElTParent=NULL Owner=%s",Owner->pDec->Name_Mt);
		Srv->SetCriticalError(this,ttt,"Warning",TableName);
		MyUnLock(__FILE__,__LINE__,&FlSim,this);
		return NULL;
	}
	else
	{
		ElTParent->TestOwner(Owner);
_m002:
		tElT=NULL;
		if (ElTParent->IDL64) tID=(ID_Elem64*)ElTParent->IDL64->Begin(FndStr->FSL8);
_m0021:
//		CCnt++;
		if (tID)
		{
//			FndStr->FSL8=FSL8;
			if (tID->Vol==0)
			{
				ElTParent->DelChild(tID->Vol);
				goto _m002;
			}
//			MyUnLock(__FILE__,__LINE__,&Fl_RE,this);
			tElT=GetElTab(&TmpAnchor,tID->Vol,fl_repl);
//			MyLock(__FILE__,__LINE__,&Fl_RE,this);
			if (tElT!=NULL)
			{
 				/*if(tElT->GetIDI()->ID_File==0x892d0000006cf6f7)
 					{size_t iasm=1;}*/
				if (tElT->GetIDI()->ParentFile!=ElTParent->GetIDI())
				{
					ElTParent->DelChild(tID->Vol);
					UnLockEl(&TmpAnchor,tElT);
					goto _m002;
				}
				FndStr->ID=tElT->GetIDI()->ID_File;
				if (FndStr->IDPrm)
				{
					char*	tVol;
					switch(FndStr->Fl_Prm)
					{
					case 0:
						if (GetParam(&TmpAnchor,tElT,FndStr->IDPrm,tVol)==-1)
						{
							//						tID=(ID_Elem64*)ElTParent->IDL64->Find((UCHAR*)&tID->Vol,sizeof(INT64),1);
							tID=(ID_Elem64*)ElTParent->IDL64->Next(FndStr->FSL8);
							goto	_m0021;
						}
						if (strcmp(tVol,FndStr->VolPrm)!=0)
						{
							//						tID=(ID_Elem64*)ElTParent->IDL64->Find((UCHAR*)&tID->Vol,sizeof(INT64),1);
							tID=(ID_Elem64*)ElTParent->IDL64->Next(FndStr->FSL8);
							goto	_m0021;
						}
						break;
					case 1:
						if (GetParam(&TmpAnchor,tElT,FndStr->IDPrm,tVol)==-1)
						{
							//						tID=(ID_Elem64*)ElTParent->IDL64->Find((UCHAR*)&tID->Vol,sizeof(INT64),1);
							tID=(ID_Elem64*)ElTParent->IDL64->Next(FndStr->FSL8);
							goto	_m0021;
						}
						break;
					case 2:
						if (GetParam(&TmpAnchor,tElT,FndStr->IDPrm,tVol)!=-1)
						{
							//						tID=(ID_Elem64*)ElTParent->IDL64->Find((UCHAR*)&tID->Vol,sizeof(INT64),1);
							tID=(ID_Elem64*)ElTParent->IDL64->Next(FndStr->FSL8);
							goto	_m0021;
						}
						break;
					};
				}
			}
			else
			{
//				tElT=GetElTab(&TmpAnchor,tID->Vol);
				ElTParent->DelChild(tID->Vol);
				goto _m002;
			}
		}
	};
//	if (!TestValid(tElT,ElTParent))
//	{
//		goto _m000;
//		DelElTab(&TmpAnchor,tElT); tElT=NULL;
//	};
	if (tID==NULL)
		tElT=NULL;
	if (tElT!=NULL)
	{
//		FndStr->FSL8=FSL8;
		FndStr->ID=tElT->GetIDI()->ID_File;
	}
	else
	{
//		FndStr->IDEl=NULL;
//		FndStr->FSL8.Active=NULL;
		FndStr->ID=0;
	}
	UnLockEl(&TmpAnchor,tElT);
	LockEl(Owner,tElT);
//	FndStr->FSL8.DestroyStr();
	MyUnLock(__FILE__,__LINE__,&FlSim,this);
	return tElT;
};

bool	IDTable::TestValid(El_Tab* ElT,El_Tab* ParentElT)
{
	bool	RC=TRUE;
	El_Tab*	tPar;
	El_Tab*	tElTP;
	INT64	tIDP;
	if (ElT)
	{
		if ((ParentElT==NULL)&&(ElT->pParent!=NULL)) 
			tPar=ElTbl;
		else
			tPar=ParentElT;
		if (ElT->GetIDI()->ParentFile)
		{
			tIDP=ElT->GetIDI()->ParentFile->ID_File;
		}
		else
			tIDP=ElT->GetIDI()->ID_ParentFile;
		tElTP=(El_Tab*)IDL->Find((UCHAR*)&tIDP,sizeof(INT64));
		if (ParentElT!=tElTP)
		{
			if (tElTP==NULL)
			{
				tElTP=LoadEl(&TmpAnchor,tIDP);
			}
			ElT->pParent=tElTP;
			if (tElTP)
				UnLockEl(&TmpAnchor,tElTP);
		}
/*		if (ElT->pParent!=tPar)
		{
			char	tBB[128];
			void*	tVol;
			void*	tVol2;
		   	RC=FALSE;
			GetParam(&TmpAnchor,ElT,IDNameElTab,tVol);
			GetParam(&TmpAnchor,tPar,IDNameElTab,tVol2);
			sprintf(tBB,"Not valid ElTab=%I64x Name=%s ParentElTab=%I64x Name=%s",ElT->GetIDI()->ID_File,(char*)tVol,tPar->GetIDI()->ID_File,(char*)tVol2);
			Srv->SetCriticalError(this,tBB,"Error",TableName);
//			ElT->pParent=tPar;
			tPar->DelChild(ElT->GetIDI()->ID_File);
			SaveEl(tPar);
//			ModifElTab(&TmpAnchor,ElT);
//			ModifElTab(&TmpAnchor,ElT->pParent);
		}
*/		if (ElT->pParent)
		{
			if (ElT->pParent->AddChild(ElT->GetIDI()->ID_File,1)!=-1)
			{
				ElT->GetIDI()->ParentFile=ElT->pParent->GetIDI();
				SaveEl(ElT);
			}
		}
	}
	return RC;
};	// Ïðîâåðèòü íà 

El_Tab*	IDTable::FindLastET	(StrOwner* Owner, FindStruct* FndStr, int AcsMode, El_Tab* ElTParent)
{
	El_Tab*		tElT=NULL;
	ID_Elem64*	tID=NULL;
//	FindStrList8	FSL8;
	MyLock(__FILE__,__LINE__,&FlSim,this);
//	FndStr->ReInit();
//_m100:
	if (ElTParent==NULL)
	{
		char	ttt[128];
		sprintf(ttt,"FindLastET with ElTParent=NULL Owner=%s",Owner->pDec->Name_Mt);
		Srv->SetCriticalError(this,ttt,"Warning",TableName);
		MyUnLock(__FILE__,__LINE__,&FlSim,this);
		return NULL;
	}
	else
	{
		ElTParent->TestOwner(Owner);
_m004:	
		tElT=NULL;
		if (ElTParent->IDL64) tID=(ID_Elem64*)ElTParent->IDL64->End(FndStr->FSL8);
_m0041:	
		if (tID)
		{
//			FndStr->FSL8=FSL8;
//			MyUnLock(__FILE__,__LINE__,&Fl_RE,this);
			tElT=GetElTab(&TmpAnchor,tID->Vol);
//			MyLock(__FILE__,__LINE__,&Fl_RE,this);
			if (tElT!=NULL)
			{
				if (tElT->GetIDI()->ParentFile!=ElTParent->GetIDI())
				{
					ElTParent->DelChild(tID->Vol);
					UnLockEl(&TmpAnchor,tElT);
					goto _m004;
				}
				if (MoLockEl(&TmpAnchor,tElT,AcsMode)==-1)
				{
					Srv->SetCriticalError(this,"Error LockEl Read","Warning",TableName);
				};
				FndStr->ID=tElT->GetIDI()->ID_File;
				if (FndStr->IDPrm)
				{
					char*	tVol;
					switch(FndStr->Fl_Prm)
					{
					case 0:
						if (GetParam(&TmpAnchor,tElT,FndStr->IDPrm,tVol)==-1)
						{
							tID=(ID_Elem64*)ElTParent->IDL64->Perviose(FndStr->FSL8);
							UnLockEl(&TmpAnchor,tElT);
							goto	_m0041;
						};
						if (strcmp(tVol,FndStr->VolPrm)!=0)
						{
							tID=(ID_Elem64*)ElTParent->IDL64->Perviose(FndStr->FSL8);
							UnLockEl(&TmpAnchor,tElT);
							goto	_m0041;
						};
						break;
					case 1:
						if (GetParam(&TmpAnchor,tElT,FndStr->IDPrm,tVol)==-1)
						{
							tID=(ID_Elem64*)ElTParent->IDL64->Perviose(FndStr->FSL8);
							UnLockEl(&TmpAnchor,tElT);
							goto	_m0041;
						};
						break;
					case 2:
						if (GetParam(&TmpAnchor,tElT,FndStr->IDPrm,tVol)!=-1)
						{
							tID=(ID_Elem64*)ElTParent->IDL64->Perviose(FndStr->FSL8);
							UnLockEl(&TmpAnchor,tElT);
							goto	_m0041;
						};
						break;
					};
				}
			}
			else
			{
				ElTParent->DelChild(tID->Vol);
				goto _m004;
			}
		}
	};
//	if (!TestValid(tElT,ElTParent))
//	{
//		goto _m100;
//		DelElTab(&TmpAnchor,tElT); tElT=NULL;
//	};
//	TestValid(tElT,ElTParent);
	if (tElT!=NULL)
	{
//		FndStr->FSL8=FSL8;
		FndStr->ID=tElT->GetIDI()->ID_File;
	}
	else
	{
//		FndStr->FSL8.Active=NULL;
		FndStr->ID=0;
	}
	UnLockEl(&TmpAnchor,tElT);
	LockEl(Owner,tElT);
//	FndStr->FSL8.DestroyStr();
	MyUnLock(__FILE__,__LINE__,&FlSim,this);
	return tElT;
};

El_Tab*	IDTable::FindNextET	(StrOwner* Owner, FindStruct* FndStr, int AcsMode, El_Tab* ElTParent,int fl_repl)
{
	El_Tab*		tElT=NULL;
	ID_Elem64*	tID=NULL;
	MyLock(__FILE__,__LINE__,&FlSim,this);
//_m200:
	if (ElTParent==NULL)
	{
		char	ttt[128];
		sprintf(ttt,"FindNextET with ElTParent=NULL Owner=%s",Owner->pDec->Name_Mt);
		Srv->SetCriticalError(this,ttt,"Warning",TableName);
		MyUnLock(__FILE__,__LINE__,&FlSim,this);
		return NULL;
	}
	else
	{
_m006:
		tElT=NULL;
		if (ElTParent->IDL64) 
		{
//			if (FndStr->IDEl!=ElTParent->IDL64->ActiveEl)
//				tID=(ID_Elem64*)ElTParent->IDL64->Find((UCHAR*)&FndStr->ID,sizeof(INT64),1);
			tID=(ID_Elem64*)ElTParent->IDL64->Next(FndStr->FSL8);
		}
		if (tID)
		{
//			FndStr->FSL8=FSL8;//ElTParent->IDL64->ActiveEl;
//			MyUnLock(__FILE__,__LINE__,&Fl_RE,this);
			tElT=GetElTab(&TmpAnchor,tID->Vol,fl_repl);
//			MyLock(__FILE__,__LINE__,&Fl_RE,this);
			if (tElT!=NULL)
			{
				if (tElT->GetIDI()->ParentFile!=ElTParent->GetIDI())
				{
					ElTParent->DelChild(tID->Vol);
					UnLockEl(&TmpAnchor,tElT);
//						SaveEl(ElTParent);
					goto _m006;
				}
				if (MoLockEl(&TmpAnchor,tElT,AcsMode)==-1)
				{
					Srv->SetCriticalError(this,"Error LockEl Read","Warning",TableName);
				};
				FndStr->ID=tElT->GetIDI()->ID_File;
				if (FndStr->IDPrm)
				{
					char*	tVol;
					switch(FndStr->Fl_Prm)
					{
					case 0:
						if (GetParam(&TmpAnchor,tElT,FndStr->IDPrm,tVol)==-1)
						{
							UnLockEl(&TmpAnchor,tElT);
							goto	_m006;
						}
						if (strcmp(tVol,FndStr->VolPrm)!=0)
						{
							UnLockEl(&TmpAnchor,tElT);
							goto	_m006;
						}
						break;
					case 1:
						if (GetParam(&TmpAnchor,tElT,FndStr->IDPrm,tVol)==-1)
						{
							UnLockEl(&TmpAnchor,tElT);
							goto	_m006;
						}
						break;
					case 2:
						if (GetParam(&TmpAnchor,tElT,FndStr->IDPrm,tVol)!=-1)
						{
							UnLockEl(&TmpAnchor,tElT);
							goto	_m006;
						}
						break;
					};
				}
			}
			else
			{
//				tElT=GetElTab(&TmpAnchor,tID->Vol);
				ElTParent->DelChild(tID->Vol);
				goto _m006;
			}
		}
	};
//	if (!TestValid(tElT,ElTParent))
//	{
//		goto _m200;
//		DelElTab(&TmpAnchor,tElT); tElT=NULL;
//	};
//	TestValid(tElT,ElTParent);
	if (tID==NULL)
		tElT=NULL;
	if (tElT!=NULL)
	{
//		FndStr->IDEl=ElTParent->IDL64->ActiveEl;
		FndStr->ID=tElT->GetIDI()->ID_File;
	}
	else
	{
//		FndStr->FSL8.Active=NULL;
		FndStr->ID=0;
	}
	UnLockEl(&TmpAnchor,tElT);
	LockEl(Owner,tElT);
//	FndStr->FSL8.DestroyStr();
	MyUnLock(__FILE__,__LINE__,&FlSim,this);
	return tElT;
};

El_Tab*	IDTable::FindPervET(StrOwner* Owner, FindStruct* FndStr, int AcsMode, El_Tab* ElTParent)
{
	El_Tab*		tElT=NULL;
	ID_Elem64*	tID=NULL;
//_m300:
	MyLock(__FILE__,__LINE__,&FlSim,this);
	if (ElTParent==NULL)
	{
		char	ttt[128];
		sprintf(ttt,"FindPervET with ElTParent=NULL Owner=%s",Owner->pDec->Name_Mt);
		Srv->SetCriticalError(this,ttt,"Warning",TableName);
		MyUnLock(__FILE__,__LINE__,&FlSim,this);
		return NULL;
	}
	else
	{
_m008:
		tElT=NULL;
		if (ElTParent->IDL64) 
		{
// 			if (FndStr->IDEl!=ElTParent->IDL64->ActiveEl)
// 				tID=(ID_Elem64*)ElTParent->IDL64->Find((UCHAR*)&FndStr->ID,sizeof(INT64),1);
			tID=(ID_Elem64*)ElTParent->IDL64->Perviose(FndStr->FSL8);
		}
		if (tID)
		{
//			FndStr->IDEl=ElTParent->IDL64->ActiveEl;
//			MyUnLock(__FILE__,__LINE__,&Fl_RE,this);
			tElT=GetElTab(&TmpAnchor,tID->Vol);
//			MyLock(__FILE__,__LINE__,&Fl_RE,this);
			if (tElT!=NULL)
			{
				if (tElT->GetIDI()->ParentFile!=ElTParent->GetIDI())
				{
					ElTParent->DelChild(tID->Vol);
					UnLockEl(&TmpAnchor,tElT);
					goto _m008;
				}
				if (MoLockEl(&TmpAnchor,tElT,AcsMode)==-1)
				{
					Srv->SetCriticalError(this,"Error LockEl Read","Warning",TableName);
				};
				FndStr->ID=tElT->GetIDI()->ID_File;
				if (FndStr->IDPrm)
				{
					char*	tVol;
					switch(FndStr->Fl_Prm)
					{
					case 0:
						if (GetParam(&TmpAnchor,tElT,FndStr->IDPrm,tVol)==-1)
						{
							UnLockEl(&TmpAnchor,tElT);
							goto	_m008;
						}
						if (strcmp(tVol,FndStr->VolPrm)!=0)
						{
							UnLockEl(&TmpAnchor,tElT);
							goto	_m008;
						}
						break;
					case 1:
						if (GetParam(&TmpAnchor,tElT,FndStr->IDPrm,tVol)==-1)
						{
							UnLockEl(&TmpAnchor,tElT);
							goto	_m008;
						}
						break;
					case 2:
						if (GetParam(&TmpAnchor,tElT,FndStr->IDPrm,tVol)!=-1)
						{
							UnLockEl(&TmpAnchor,tElT);
							goto	_m008;
						}
						break;
					};
				}
			}
			else
			{
				ElTParent->DelChild(tID->Vol);
				goto _m008;
			}
		}
	};
//	if (!TestValid(tElT,ElTParent))
//	{
//		goto _m300;
//		DelElTab(&TmpAnchor,tElT); tElT=NULL;
//	};
//	TestValid(tElT,ElTParent);
	if (tElT!=NULL)
	{
//		FndStr->IDEl=ElTParent->IDL64->ActiveEl;
		FndStr->ID=tElT->GetIDI()->ID_File;
	}
	else
	{
//		FndStr->FSL8.Active=NULL;
		FndStr->ID=0;
	}
	UnLockEl(&TmpAnchor,tElT);
	LockEl(Owner,tElT);
//	FndStr->FSL8.DestroyStr();
	MyUnLock(__FILE__,__LINE__,&FlSim,this);
	return tElT;
};

El_Tab*	IDTable::FindNestedStr(StrOwner* Owner, int AcsMode, char* FClass, El_Tab* ElTParent)
{
	El_Tab*		tElT=NULL;
	char	FirstClass[512];
	MyLock(__FILE__,__LINE__,&FlSim,this);
	strcpy(FirstClass,FClass);
	char	Tmp;
//	va_list arglist;
	char* Str=0;
	int		Pointer=0;
	int		StrLen=strlen(FirstClass);
	El_Tab* pEl;
	El_Tab* pEl2=NULL;
	if (ElTParent)
	{
//		char	ttt[128];
//		sprintf(ttt,"FindNestedStr with ElTParent=NULL Owner=%s",Owner->pDec->Name_Mt);
//		Srv->SetCriticalError(this,ttt,"Warning",TableName);
//		return NULL;
		Pointer=-1;
		pEl=ElTParent;
		ElTParent->TestOwner(Owner);
	}
	else
	{
		Str=FirstClass;
		for (Pointer;Pointer<StrLen;Pointer++)
		{
			if (FirstClass[Pointer]=='\\')
				break;
		};
		Tmp=FirstClass[Pointer];
		FirstClass[Pointer]=0;
		pEl=Find(&TmpAnchor,AcsMode,Str,ElTbl);
		FirstClass[Pointer]=Tmp;
	};
	while ((pEl)&&(Pointer<StrLen))
	{
		Str=&FirstClass[++Pointer];
		for (Pointer;Pointer<StrLen;Pointer++)
		{
			if (FirstClass[Pointer]=='\\')
				break;
		};
		Tmp=FirstClass[Pointer];
		FirstClass[Pointer]=0;
		pEl2=pEl;
		pEl=Find(&TmpAnchor,AcsMode,Str,pEl2);
		if (!TestValid(pEl,pEl2))
		{
//			DelElTab(&TmpAnchor,pEl);
//			pEl=NULL;
		};
//		TestValid(pEl,pEl2);
		UnLockEl(&TmpAnchor,pEl2);
		if (!pEl)
			break;
		if (Pointer>=StrLen) break;
		FirstClass[Pointer]=Tmp;
	};
	tElT=pEl;
	UnLockEl(&TmpAnchor,tElT);
	LockEl(Owner,tElT);
	MyUnLock(__FILE__,__LINE__,&FlSim,this);
	return tElT;
};

El_Tab*	IDTable::FindNested	(StrOwner* Owner, int AcsMode, char* FirstClass,...)
{
	El_Tab*		tElT=NULL;
	va_list arglist;
	char* Str=0;
	El_Tab* pEl;
	El_Tab* pEl2;
	MyLock(__FILE__,__LINE__,&FlSim,this);
	va_start(arglist, FirstClass);
 	pEl=Find(&TmpAnchor,AcsMode,FirstClass,ElTbl);
 	while (pEl)
 	{
		Str=va_arg(arglist,char*);
 		if (Str==NULL) break;
 		pEl2=Find(&TmpAnchor,AcsMode,Str,pEl);
		if (!TestValid(pEl2,pEl))
		{
//			DelElTab(&TmpAnchor,pEl2); pEl2=NULL;
		};
//		TestValid(pEl2,pEl);
		UnLockEl(&TmpAnchor,pEl);
		pEl=pEl2;
		if (!pEl)
			break;
	};
	tElT=pEl;
//	UnLockEl(Owner,pEl);
	UnLockEl(&TmpAnchor,tElT);
	LockEl(Owner,tElT);
	va_end(arglist);
	MyUnLock(__FILE__,__LINE__,&FlSim,this);
	return tElT;
};

Parametr*	IDTable::FindFirstPrm	(StrOwner* Owner, FindStruct* FndStr, El_Tab* ElT, void* &Vol,size_t &tLen)
{
	Parametr* Prm=NULL;
	MyLock(__FILE__,__LINE__,&FlSim,this);
	if (ElT->GetIDI(Owner)->pPList->FindFirst(FndStr->IDPrm,Vol,tLen)==0)
	{
		Prm=Srv->GetParam(FndStr->IDPrm);
	};
	MyUnLock(__FILE__,__LINE__,&FlSim,this);
	return Prm;
};

Parametr*	IDTable::FindNextPrm		(StrOwner* Owner, FindStruct* FndStr, El_Tab* ElT, void* &Vol,size_t &tLen)
{
	Parametr* Prm=NULL;
	MyLock(__FILE__,__LINE__,&FlSim,this);
	if (GetParam(Owner,ElT,FndStr->IDPrm,(char*&)Vol)==0)
	{
		if (ElT->GetIDI()->pPList->FindNext(FndStr->IDPrm,Vol,tLen)==0)
		{
			Prm=Srv->GetParam(FndStr->IDPrm);
		};
	};
	MyUnLock(__FILE__,__LINE__,&FlSim,this);
	return Prm;
};

El_Tab*	IDTable::AttainNested(StrOwner* Owner,char *mFormat,char *FirstClass,...) //Ex:AttainNested("%d,%s,%d",StreamList,2000,LogChannel,12,-1);
{
	El_Tab* pEl,*pElOld;
	va_list arglist;
	int m_ch;
	MyLock(__FILE__,__LINE__,&FlSim,this);
	int	State=0;
	va_start(arglist, FirstClass);
	INT64 m_iVal;
	char *m_sVal=NULL;
//	if (Srv->System_On!=1) return NULL;
	if (Fl_Debug)
	{
		sprintf(LogBuf,"AttainNested(%s)=%s",Owner->pDec->Name_Mt,FirstClass);
	}
	pEl=Find(&TmpAnchor,AcsMd_Read,FirstClass,ElTbl);
//	if (pEl==0)	{Sleep(500);pEl=Find(&TmpAnchor,AcsMode,FirstClass);};
	if (pEl==0)
	{
		pEl=AddET(&TmpAnchor,FirstClass);
	};
	UnLockEl(&TmpAnchor,pEl);
	pElOld=pEl;
	while ((m_ch = *mFormat++) !=0) 
	{
		if (State==3) break;
		else if (m_ch=='%') State=1;
		else if (State==1)
		{
			switch (m_ch)
			{
			case 'd'://INT64
			case 'D'://INT64
				m_iVal=va_arg(arglist,INT64);
				if ((m_iVal!=-1) || (m_sVal==0))
				{
					if (Fl_Debug)
					{
						sprintf(LogBuf,"%s\\%I64d",LogBuf,m_iVal);
					}
					pEl=Find(&TmpAnchor,AcsMd_Read,m_iVal,pElOld);
//					if (pEl==0){Sleep(500);pEl=Find(&TmpAnchor,AcsMode,m_iVal,pElOld);};
					if (pEl==0)
					{
						pEl=AddET(&TmpAnchor,m_iVal,pElOld);
					};
					UnLockEl(&TmpAnchor,pEl);
					pElOld=pEl;
				}
				else State=3;
				State=0;
				break;
			case 's'://str
			case 'S'://str
				m_sVal=va_arg(arglist,char*);
				if ((((int)(m_sVal))!=-1) || (m_sVal==0))
				{
					if (Fl_Debug)
					{
						sprintf(LogBuf,"%s\\%s",LogBuf,m_sVal);
					}
					pEl=Find(&TmpAnchor,AcsMd_Read,m_sVal,pElOld);
//					if (pEl==0){Sleep(500);pEl=Find(&TmpAnchor,AcsMode,m_sVal,pElOld);};
					if (pEl==0)
					{
						pEl=AddET(&TmpAnchor,m_sVal,pElOld);
					};
					UnLockEl(&TmpAnchor,pEl);
					pElOld=pEl;
				}
				else
					State=3;					
				State=0;
				break;
			}
		};		
	};
	UnLockEl(&TmpAnchor,pEl);
	if (TestEl(pEl)==-1)
		pEl=NULL;
	LockEl(Owner,pEl);
	va_end(arglist);
	if (Fl_Debug)
	{
		Srv->OutLog(LogBuf,this,TableName);
	}
	MyUnLock(__FILE__,__LINE__,&FlSim,this);
	return pEl;
};
/*
El_Tab*	IDTable::AttainNestedI(StrOwner* Owner,char *mFormat,char *FirstClass,...) //Ex:AttainNested("%d,%s,%d",StreamList,2000,LogChannel,12,-1);
{
	El_Tab* pEl,*pElOld;
	va_list arglist;
	int m_ch;
	int	State=0;
	va_start(arglist, FirstClass);
	INT64 m_iVal;
	char *m_sVal;
	pEl=Find(Owner,AcsMd_Read,FirstClass);
	if (pEl==0)
	{
		pEl=AddETI(Owner,FirstClass);
	};
	UnLockEl(Owner,pEl);
	pElOld=pEl;
	while ((m_ch = *mFormat++) !=0) 
	{
		if (State==3) break;
		else if (m_ch=='%') State=1;
		else if (State==1)
		{
			switch (m_ch)
			{
			case 'i'://INT64
			case 'I'://INT64
				m_iVal=va_arg(arglist,int);
				if (m_iVal!=-1)
				{
					pEl=Find(Owner,AcsMd_Read,m_iVal,pElOld);
					if (pEl==0)
					{
						pEl=AddET(Owner,m_iVal,pElOld);
					};
					UnLockEl(Owner,pEl);
					pElOld=pEl;
				}
				else State=3;
				State=0;
				break;
			case 'd'://INT64
			case 'D'://INT64
				m_iVal=va_arg(arglist,INT64);
				if (m_iVal!=-1)
				{
					pEl=Find(Owner,AcsMd_Read,m_iVal,pElOld);
					if (pEl==0)
					{
						pEl=AddET(Owner,m_iVal,pElOld);
					};
					UnLockEl(Owner,pEl);
					pElOld=pEl;
				}
				else State=3;
				State=0;
				break;
			case 's'://str
			case 'S'://str
				m_sVal=va_arg(arglist,char*);
				if ((int)m_sVal!=-1)
				{
					pEl=Find(Owner,AcsMd_Read,m_sVal,pElOld);
					if (pEl==0)
					{
						pEl=AddETI(Owner,m_sVal,pElOld);
					};
					UnLockEl(Owner,pEl);
					pElOld=pEl;
				}
				else State=3;					
				State=0;
				break;
			}
		};		
	};
	LockEl(Owner,pEl);
	va_end(arglist);
	return pEl;
};
*/
El_Tab*	IDTable::AddET		(StrOwner* Owner,El_Tab* ElTParent)
{
	char*		tVol;
	MyLock(__FILE__,__LINE__,&FlSim,this);
	El_Tab*		tElT=NULL;
	tElT=NewElTab(&TmpAnchor,ElTParent);
//	UnLockEl(&TmpAnchor,tElT);
//	LockEl(Owner,tElT);
//	MyLock(__FILE__,__LINE__,&Fl_RE,this);
	if (Fl_Debug)
	{
		if (GetParam(&TmpAnchor,tElT,IDNameElTab,tVol)==0)
		{
			sprintf(LogBuf,"AdET ID=%I64x Name=%s Owner=%s",tElT->GetIDI()->ID_File,tVol,Owner->pDec->Name_Mt);
		}
		else
		{
			sprintf(LogBuf,"AddET ID=%I64x Owner=%s",tElT->GetIDI()->ID_File,Owner->pDec->Name_Mt);
		}
	}
	UnLockEl(&TmpAnchor,tElT);
	if (Fl_Debug)
	{
		Srv->OutLog(LogBuf,this,TableName);
	}
	LockEl(Owner,tElT);
//	MyUnLock(__FILE__,__LINE__,&Fl_Th,this);
//	MyUnLock(__FILE__,__LINE__,&Fl_RE,this);
	MyUnLock(__FILE__,__LINE__,&FlSim,this);
	return tElT;
};

El_Tab*	IDTable::AddET		(StrOwner* Owner,char* Vol, El_Tab* ElTParent)
{
	char*		tVol;
	MyLock(__FILE__,__LINE__,&FlSim,this);
	El_Tab*		tElT=NULL;
	tElT=Find(&TmpAnchor,AcsMd_Read,Vol,ElTParent);
	if (tElT==NULL)
	{
		tElT=NewElTab(&TmpAnchor,Vol,ElTParent);
		if (tElT)
		{
			if (Fl_Debug)
			{
				if (GetParam(&TmpAnchor,tElT,IDNameElTab,tVol)==0)
				{
					sprintf(LogBuf,"AddETNew ID=%I64x Name=%s Owner=%s",tElT->GetIDI()->ID_File,tVol,Owner->pDec->Name_Mt);
				}
				else
				{
					sprintf(LogBuf,"AddETNew ID=%I64x Owner=%s",tElT->GetIDI()->ID_File,Owner->pDec->Name_Mt);
				}
				Srv->OutLog(LogBuf,this,TableName);
			}
			//		UnLockEl(&TmpAnchor,tElT);
		}
	}
	else
	{
		if (Fl_Debug)
		{
			if (GetParam(&TmpAnchor,tElT,IDNameElTab,tVol)==0)
			{
				sprintf(LogBuf,"AddETOld ID=%I64x Name=%s Owner=%s",tElT->GetIDI()->ID_File,tVol,Owner->pDec->Name_Mt);
			}
			else
			{
				sprintf(LogBuf,"AddETOld ID=%I64x Owner=%s",tElT->GetIDI()->ID_File,Owner->pDec->Name_Mt);
			}
			Srv->OutLog(LogBuf,this,TableName);
		}
	}
	UnLockEl(&TmpAnchor,tElT);
	LockEl(Owner,tElT);
	MyUnLock(__FILE__,__LINE__,&FlSim,this);
	return tElT;
};

El_Tab*	IDTable::AddETI		(StrOwner* Owner,char* Vol, El_Tab* ElTParent)
{
	El_Tab*		tElT=NULL;
	tElT=Find(&TmpAnchor,AcsMd_Read,Vol,ElTParent);
	if (tElT==NULL)
	{
		tElT=NewElTab(&TmpAnchor,Vol,ElTParent);
	}
	UnLockEl(&TmpAnchor,tElT);
	LockEl(Owner,tElT);
	return tElT;
};
/*
El_Tab*	IDTable::AddET		(StrOwner* Owner,int Vol, El_Tab* ElTParent)
{
	El_Tab*		tElT=NULL;
	tElT=Find(Owner,AcsMd_Read,Vol,ElTParent);
	if (tElT==NULL)
	{
		tElT=NewElTab(Owner,ElTParent);
		if (tElT)
		{
			AddParam(Owner,tElT,IDINTElTab,Vol);
			ModifElTab(Owner,tElT);
		}
	}
	return tElT;
};
*/
El_Tab*	IDTable::AddET		(StrOwner* Owner,INT64 Vol, El_Tab* ElTParent)
{
	El_Tab*		tElT=NULL;
	char*		tVol;
	tElT=Find(&TmpAnchor,AcsMd_Read,Vol,ElTParent);
	if (tElT==NULL)
	{
		tElT=NewElTab(&TmpAnchor,Vol,ElTParent);
		if (Fl_Debug)
		{
		
			if (GetParam(&TmpAnchor,tElT,IDNameElTab,tVol)==0)
			{
				sprintf(LogBuf,"AddET ID=%I64x Name=%s Owner=%s",tElT->GetIDI()->ID_File,tVol,Owner->pDec->Name_Mt);
			}
			else
			{
				sprintf(LogBuf,"AddET ID=%I64x Owner=%s",tElT->GetIDI()->ID_File,Owner->pDec->Name_Mt);
			}

		}
	}
	UnLockEl(&TmpAnchor,tElT);
	LockEl(Owner,tElT);
//	UnLockEl(&TmpAnchor,tElT);
	if (Fl_Debug)
	{
		Srv->OutLog(LogBuf,this,TableName);
	}
	return tElT;
};

El_Tab*	IDTable::CopyET		(StrOwner* Owner, El_Tab* ETbl,El_Tab* ETblDest,int FlNoModif)	// Copy ýëåìåíò â åëåìåíò íàçíà÷åíèÿ
{
	El_Tab*		TmpET;
	El_Tab*		TmpET2;
	MyLock(__FILE__,__LINE__,&FlSim,this);
	TmpET=NewElTab(&TmpAnchor,ETblDest);
	{
		CopyIDI2Data(ETbl->GetIDI(Owner),TmpET,FlNoModif);
		if(ETbl->IDL64)
		{
			FindStrList8	FSL8;
			ID_Elem64*	tElT=(ID_Elem64*)ETbl->IDL64->Begin(FSL8);
			while (tElT)
			{
//				MyUnLock(__FILE__,__LINE__,&Fl_RE,this);
				TmpET2=GetElTab(&TmpAnchor,tElT->Vol);
//				MyLock(__FILE__,__LINE__,&Fl_RE,this);
				if ((TmpET2)&&(TmpET2->pParent==ETbl))
				{
					CopyET(&TmpAnchor,TmpET2,TmpET,FlNoModif);
				}
				UnLockEl(&TmpAnchor,TmpET2);
//				tElT=(ID_Elem64*)ETbl->IDL64->Find((UCHAR*)&tElT->Vol,sizeof(INT64),1);
				tElT=(ID_Elem64*)ETbl->IDL64->Next(FSL8);
			}

		}

	};
	TestEl(TmpET);
	UnLockEl(&TmpAnchor,TmpET);
	LockEl(Owner,TmpET);
	MyUnLock(__FILE__,__LINE__,&FlSim,this);
	return TmpET;
};

El_Tab*	IDTable::MoveET		(StrOwner* Owner, El_Tab* ETbl,El_Tab* ETblDest,int FlNoModif)	// Ïåðåìåñòèòü ýëåìåíò â åëåìåíò íàçíà÷åíèÿ
{
	char*		tVol;
	El_Tab*		TmpPar;
	ID_Elem64*	tElT=NULL;
	MyLock(__FILE__,__LINE__,&FlSim,this);
	if (ETblDest)
	{
		LockEl(&TmpAnchor,ETbl);
		LockEl(&TmpAnchor,ETblDest);
		if (GetParam(&TmpAnchor,ETbl,IDNameElTab,tVol)==0)
		{
			sprintf(LogBuf,"MoveET ID=%I64x Name=%s to ID=%I64x Owner=%s",ETbl->GetIDI()->ID_File,tVol,ETblDest->GetIDI()->ID_File,Owner->pDec->Name_Mt);
		}
		else
		{
			sprintf(LogBuf,"MoveET ID=%I64x to ID=%I64x Owner=%s",ETbl->GetIDI()->ID_File,ETblDest->GetIDI()->ID_File,Owner->pDec->Name_Mt);
		}
		TmpPar=ETbl->pParent;
		if (TmpPar)
		{
			TmpPar->DelChild(ETbl->GetIDI()->ID_File);
			ETbl->pParent=NULL;
			UpDataEl(TmpPar);
			SendEl(TmpPar);
			if (FlNoModif==0)
			{
				TmpPar->Fl_Query=0;
				ReplEl(TmpPar);
			}
		}
		ETblDest->AddChild(ETbl->GetIDI()->ID_File,1);
		ETbl->pParent=ETblDest;
		ETbl->GetIDI()->ID_ParentFile=ETblDest->GetIDI()->ID_File;
		ETbl->GetIDI()->ParentFile=ETblDest->GetIDI();
		
		LockEl(&TmpAnchor,ETbl);
		if (MoLockEl(&TmpAnchor,ETbl,AcsMd_Write)!=-1)
		{
			tElT=NULL;
			FindStrList8	FSL8;
			if (ETbl->IDL64) tElT=(ID_Elem64*)ETbl->IDL64->Begin(FSL8);
			int CntChild=0;
			ETbl->GetIDI()->Command=Command_MOVE;
			ETbl->GetIDI()->TypeIDI=TypeIDI_TABLE;
			ETbl->GetIDI()->StateBlock=StateBlock_BEGIN;
			ETbl->GetIDI()->NumPack=0;
			if (LastHost[0]==0)
				strcpy(LastHost,"*");
//			ETbl->GetIDI()->pPList->SetParam(IDHostName,&LastHost,strlen(LastHost)+1,this);
			ETbl->GetIDI()->AdrInfo->DestinAI.HostName[0]=0;
			Srv->GetRealTime(&ETbl->GetIDI()->TimeOper.FTime);
//			ETbl->GetIDI()->pPList->DelParam(IDDomainName);
			ETbl->GetIDI()->AdrInfo->DestinAI.DomainName[0]=0;
			while (tElT!=NULL)
			{
				ETbl->GetIDI()->Buf=(UCHAR*)Child;
				ETbl->GetIDI()->ID_Standart=ETbl->IDL64->NumElem;
				while ((tElT!=NULL)&&(CntChild<SizeChildBuf))
				{
					Child[CntChild++]=tElT->Vol;
					if (ETbl->IDL64)
					{
//						tElT=(ID_Elem64*)ETbl->IDL64->Find((UCHAR*)&tElT->Vol,sizeof(INT64),1);
						tElT=(ID_Elem64*)ETbl->IDL64->Next(FSL8);
					}
				}
				ETbl->GetIDI()->Cnt=CntChild*sizeof(INT64);
				if ((ETbl->GetIDI()->Cnt!=0)&&(tElT!=NULL)) 
				{
					ETbl->GetIDI()->NumPack++;
					if (FlNoModif==0)
					{
						ETbl->Fl_Query=0;
						Srv->ReplElTabl(ETbl);
					}
//						ODataIDI(3,ETbl->GetIDI(),1);
					Srv->SendElTabl(ETbl);
//					ODataIDI(2,ETbl->GetIDI(),1);
					ETbl->GetIDI()->StateBlock=0;
				}
				CntChild=0;
			}
			ETbl->GetIDI()->StateBlock|=StateBlock_END;
			Srv->GetRealTime(&ETbl->GetIDI()->TimeOper.FTime);
			ETbl->GetIDI()->NumPack++;
			ETbl->GetIDI()->ID_SeansFrom=ID;
			if (FlNoModif==0)
			{
				ETbl->Fl_Query=0;
				Srv->ReplElTabl(ETbl);
			}
//				ODataIDI(3,ETbl->GetIDI(),1);
			Srv->SendElTabl(ETbl);
//			ODataIDI(2,ETbl->GetIDI(),1);
			if (Fl_Debug)
			{
				if (GetParam(&TmpAnchor,ETbl,IDNameElTab,tVol)==0)
				{
					sprintf(LogBuf,"MoveEl to %s ID=%I64x Name=%s",LastHost,ETbl->GetIDI()->ID_File,tVol);
				}
				else
				{
					sprintf(LogBuf,"MoveEl to %s ID=%I64x",LastHost,ETbl->GetIDI()->ID_File);
				}
				Srv->OutLog(LogBuf,this,TableName);
			}
			UnLockEl(&TmpAnchor,ETbl);
		}
		else
		{
			Srv->SetCriticalError(this,"Error LockEl Write in MoveET","Error",TableName);
		};
		UpDataEl(ETblDest);
		SendEl(ETblDest);
		if (FlNoModif==0)
		{
			ETblDest->Fl_Query=0;
			ReplEl(ETblDest);
		}

		UpDataEl(ETbl);
		if (TestEl(ETbl)!=-1)
			UnLockEl(&TmpAnchor,ETbl);
		else
			ETbl=NULL;
//		SendEl(ETbl);
//		ReplEl(ETbl);
		UnLockEl(&TmpAnchor,ETblDest);
	}
	MyUnLock(__FILE__,__LINE__,&FlSim,this);
	return ETbl;
};

El_Tab*	IDTable::MoveETInternal		(StrOwner* Owner, El_Tab* ETbl,El_Tab* ETblDest)
{
	char*		tVol;
	El_Tab*		TmpPar;
	ID_Elem64*	tElT=NULL;
	MyLock(__FILE__,__LINE__,&FlSim,this);
	if (ETblDest)
	{
		LockEl(&TmpAnchor,ETbl);
		LockEl(&TmpAnchor,ETblDest);
		if (GetParam(&TmpAnchor,ETbl,IDNameElTab,tVol)==0)
		{
			sprintf(LogBuf,"MoveETInt ID=%I64x Name=%s to ID=%I64x Owner=%s",ETbl->GetIDI()->ID_File,tVol,ETblDest->GetIDI()->ID_File,Owner->pDec->Name_Mt);
		}
		else
		{
			sprintf(LogBuf,"MoveETInt ID=%I64x to ID=%I64x Owner=%s",ETbl->GetIDI()->ID_File,ETblDest->GetIDI()->ID_File,Owner->pDec->Name_Mt);
		}
		TmpPar=ETbl->pParent;
		if (TmpPar)
		{
			TmpPar->DelChild(ETbl->GetIDI()->ID_File);
			ETbl->pParent=NULL;
			UpDataEl(TmpPar);
			SendEl(TmpPar);
//			if (FlNoModif==0)	ReplEl(TmpPar);
		}
		ETblDest->AddChild(ETbl->GetIDI()->ID_File,1);
		ETbl->pParent=ETblDest;
		ETbl->GetIDI()->ID_ParentFile=ETblDest->GetIDI()->ID_File;
		ETbl->GetIDI()->ParentFile=ETblDest->GetIDI();

		LockEl(&TmpAnchor,ETbl);
		if (MoLockEl(&TmpAnchor,ETbl,AcsMd_Write)!=-1)
		{
			tElT=NULL;
			FindStrList8	FSL8;
			if (ETbl->IDL64) tElT=(ID_Elem64*)ETbl->IDL64->Begin(FSL8);
			int CntChild=0;
			ETbl->GetIDI()->Command=Command_DEL;
			ETbl->GetIDI()->TypeIDI=TypeIDI_TABLE;
			ETbl->GetIDI()->StateBlock=StateBlock_BEGIN;
			ETbl->GetIDI()->NumPack=0;
			if (LastHost[0]==0)
				strcpy(LastHost,"*");
			//			ETbl->GetIDI()->pPList->SetParam(IDHostName,&LastHost,strlen(LastHost)+1,this);
			ETbl->GetIDI()->AdrInfo->DestinAI.HostName[0]=0;
			Srv->GetRealTime(&ETbl->GetIDI()->TimeOper.FTime);
			//			ETbl->GetIDI()->pPList->DelParam(IDDomainName);
			ETbl->GetIDI()->AdrInfo->DestinAI.DomainName[0]=0;
			while (tElT!=NULL)
			{
				ETbl->GetIDI()->Buf=(UCHAR*)Child;
				ETbl->GetIDI()->ID_Standart=ETbl->IDL64->NumElem;
				while ((tElT!=NULL)&&(CntChild<SizeChildBuf))
				{
					Child[CntChild++]=tElT->Vol;
					if (ETbl->IDL64)
					{
						//						tElT=(ID_Elem64*)ETbl->IDL64->Find((UCHAR*)&tElT->Vol,sizeof(INT64),1);
						tElT=(ID_Elem64*)ETbl->IDL64->Next(FSL8);
					}
				}
				ETbl->GetIDI()->Cnt=CntChild*sizeof(INT64);
				if ((ETbl->GetIDI()->Cnt!=0)&&(tElT!=NULL)) 
				{
					ETbl->GetIDI()->NumPack++;
// 					if (FlNoModif==0)
// 						Srv->ReplElTabl(ETbl);
					//						ODataIDI(3,ETbl->GetIDI(),1);
					Srv->SendElTabl(ETbl);
					//					ODataIDI(2,ETbl->GetIDI(),1);
					ETbl->GetIDI()->StateBlock=0;
				}
				CntChild=0;
			}
			ETbl->GetIDI()->StateBlock|=StateBlock_END;
			Srv->GetRealTime(&ETbl->GetIDI()->TimeOper.FTime);
			ETbl->GetIDI()->NumPack++;
			ETbl->GetIDI()->ID_SeansFrom=ID;
// 			if (FlNoModif==0)
// 				Srv->ReplElTabl(ETbl);
			//				ODataIDI(3,ETbl->GetIDI(),1);
			Srv->SendElTabl(ETbl);
			//			ODataIDI(2,ETbl->GetIDI(),1);
// 			if (Fl_Debug)
// 			{
// 				if (GetParam(&TmpAnchor,ETbl,IDNameElTab,tVol)==0)
// 				{
// 					sprintf(LogBuf,"MoveEl to %s ID=%I64x Name=%s",LastHost,ETbl->GetIDI()->ID_File,tVol);
// 				}
// 				else
// 				{
// 					sprintf(LogBuf,"MoveEl to %s ID=%I64x",LastHost,ETbl->GetIDI()->ID_File);
// 				}
// 				Srv->OutLog(LogBuf,this,TableName);
// 			}
			UnLockEl(&TmpAnchor,ETbl);
		}
		else
		{
			Srv->SetCriticalError(this,"Error LockEl Write in MoveETInt","Error",TableName);
		};
		UpDataEl(ETblDest);
		SendEl(ETblDest);
// 		if (FlNoModif==0)	ReplEl(ETblDest);

		UpDataEl(ETbl);
//		TestEl(ETbl);
		//		SendEl(ETbl);
		//		ReplEl(ETbl);
		UnLockEl(&TmpAnchor,ETblDest);
		UnLockEl(&TmpAnchor,ETbl);
	}
	MyUnLock(__FILE__,__LINE__,&FlSim,this);
	return ETbl;
};

El_Tab*	IDTable::Find		(StrOwner* Owner, int AcsMode, char* Str, El_Tab* ElTParent)
{
	El_Tab*		itElT=NULL;
	FindStruct	IntFStr;
	char*		tBuf;
	MyLock(__FILE__,__LINE__,&FlSim,this);
	if (ElTParent==NULL) ElTParent=ElTbl;
	LockEl(&TmpAnchor,ElTParent);
	itElT=FindFirstET(&TmpAnchor,&IntFStr,AcsMode,ElTParent);
	while(itElT)
	{
		if (GetParam(&TmpAnchor,itElT,IDNameElTab,tBuf)!=-1)
		{
			if (strcmp(Str,tBuf)==NULL)
				break;
		}
		UnLockEl(&TmpAnchor,itElT);
		itElT=FindNextET(&TmpAnchor,&IntFStr,AcsMode,ElTParent);
	}
	if (!TestValid(itElT,ElTParent))
	{
//		DelElTab(&TmpAnchor,tElT); tElT=NULL;
	};
//	TestValid(tElT,ElTParent);
	UnLockEl(&TmpAnchor,itElT);
	UnLockEl(&TmpAnchor,ElTParent);
	LockEl(Owner,itElT);
	MyUnLock(__FILE__,__LINE__,&FlSim,this);
	return itElT;
};
/*
El_Tab*	IDTable::Find		(StrOwner* Owner, int AcsMode, int ID, El_Tab* ElTParent)
{
	El_Tab*		tElT=NULL;
	FindStruct	FStr;
	int			*tVol;
	tElT=FindFirstET(Owner,&FStr,AcsMode,ElTParent);
	while(tElT)
	{
		if (GetParam(Owner,tElT,IDINTElTab,tVol)!=-1)
		{
			if (ID==*tVol)
				break;
		}
		tElT=FindNextET(Owner,&FStr,AcsMode,ElTParent);
	}
	return tElT;
};
*/
El_Tab*	IDTable::Find		(StrOwner* Owner, int AcsMode, INT64 ID, El_Tab* ElTParent)
{
	El_Tab*		tElT=NULL;
	FindStruct	FStr;
//	INT64		*tVol;
	MyLock(__FILE__,__LINE__,&FlSim,this);
	if (ElTParent==NULL) ElTParent=ElTbl;
	tElT=FindFirstET(&TmpAnchor,&FStr,AcsMode,ElTParent);
	while(tElT)
	{
		if (ID==tElT->GetIDI()->ID_File)
			break;
		UnLockEl(&TmpAnchor,tElT);
		tElT=FindNextET(&TmpAnchor,&FStr,AcsMode,ElTParent);
	}
	if (!TestValid(tElT,ElTParent))
	{
//		DelElTab(&TmpAnchor,tElT); tElT=NULL;
	};
//	TestValid(tElT,ElTParent);
	UnLockEl(&TmpAnchor,tElT);
	LockEl(Owner,tElT);
	MyUnLock(__FILE__,__LINE__,&FlSim,this);
	return tElT;
};

El_Tab* IDTable::GetElParent(int NumStep, El_Tab* ElBase)
{
	El_Tab*		tElProm=ElBase;
	size_t i;
	MyLock(__FILE__,__LINE__,&FlSim,this);
	
	for (i=0;i<NumStep;i++)
	{
		if (tElProm)
			tElProm=tElProm->pParent;
	};
	MyUnLock(__FILE__,__LINE__,&FlSim,this);
	return tElProm;
};

El_Tab*	IDTable::FindParallel(StrOwner* Owner, int NumStepUp, int NumStepDown, El_Tab* ElBase,char* ElParName)
{
	El_Tab*		tElT=NULL;
	El_Tab*		tElProm;
	size_t i;
	MyLock(__FILE__,__LINE__,&FlSim,this);
	ElBase->TestOwner(Owner);
	tElProm=GetElParent(NumStepUp,ElBase);
	tElT=FindNestedStr(&TmpAnchor,AcsMd_Read,ElParName,tElProm);
	if (tElT)
	{
		for(i=NumStepUp-1;i>NumStepUp-NumStepDown;i--)
		{
			char	*tVol;
			tElProm=GetElParent(i-1,ElBase);
			GetParam(&TmpAnchor,tElProm,IDNameElTab,tVol);
			tElProm=FindNestedStr(&TmpAnchor,AcsMd_Read,(char*)tVol,tElT);
			UnLockEl(&TmpAnchor,tElT);
			tElT=tElProm;
			if (tElT==NULL) break;
		}
	}
	LockEl(Owner,tElT);
	MyUnLock(__FILE__,__LINE__,&FlSim,this);
	return tElT;
};

El_Tab*	IDTable::FindProfile(StrOwner* Owner, El_Tab* ETbl,INT64 IDNameProFile)
{
	El_Tab*		tElT=NULL;
	El_Tab*		tElT2=NULL;
	MyLock(__FILE__,__LINE__,&FlSim,this);
	int			rc=0;
	if (ETbl)
	{
		char	*tVol=NULL;
		tElT2=ETbl;
		while (tElT2)
		{
			rc=GetParam(&TmpAnchor,tElT2,IDNameProFile,tVol);
			if (rc!=-1)
			{
				break;
			}
			tElT2=tElT2->pParent;
		};

		if (rc!=-1)
		{
			if (IDNameProFile==IDProfileFilter)
				tElT=FindNested(&TmpAnchor,AcsMd_Read,N_Profile,N_Filter,tVol,NULL);
			else
			{
				if (IDNameProFile==IDProfileQBE)
					tElT=FindNested(&TmpAnchor,AcsMd_Read,N_Profile,N_QBE,tVol,NULL);
				else
				{
					if (IDNameProFile==IDProfileSchema)
						tElT=FindNested(&TmpAnchor,AcsMd_Read,N_Profile,N_Schema,tVol,NULL);
					else
					{
						if (IDNameProFile==IDProfileCS)
							tElT=FindNested(&TmpAnchor,AcsMd_Read,N_Profile,N_CS,tVol,NULL);
						else
						{
							// ...
						}
					}
				}
			}
			if (tElT)
			{
				UnLockEl(&TmpAnchor,tElT);
				LockEl(Owner,tElT);
			}
//
//			FindStruct	tFS;
//_met0:
//			tElT2=ETbl->pParent;
//			tFS.IDPrm=IDNameElTab;
//			strcpy(tFS.VolPrm,"Profile");
//			tElT=FindFirstET(Owner,&tFS,AcsMd_Read,tElT2);
//			if (tElT==NULL)
//			{
//				tElT2=tElT2->pParent;
//				if (tElT2)
//					goto _met0;
//			}
//			else
//			{
//
//			}
		}
	}
	MyUnLock(__FILE__,__LINE__,&FlSim,this);
	return tElT;
};

int	IDTable::DelParam	(StrOwner* Owner,El_Tab* ETbl,INT64 IDPrm,int FlNoModif)
{
	int	rc=-1;
//	El_Tab*	tElP;
//	MyLock(__FILE__,__LINE__,&Fl_RE,this);
	if (MoLockEl(Owner,ETbl,AcsMd_Write)!=-1)
	{
		Parametr*	tPrm=Srv->GetParam(IDPrm);
		if ((tPrm)&&(Srv->Fl_Prm!=0))
			rc=ETbl->GetIDI()->pPList->DelParam(IDPrm);
		ETbl->GetIDI(Owner)->ModifCnt++;
		MoLockEl(Owner,ETbl,AcsMd_Read);
		if (FlNoModif!=1)
			ModifElTab(Owner,ETbl,FlNoModif);
	}
	else
	{
		Srv->SetCriticalError(this,"Error LockEl Write in DelParam","Error",TableName);
	};
//	MyUnLock(__FILE__,__LINE__,&Fl_RE,this);
	return rc;
};


int	IDTable::AddParam	(StrOwner* Owner,El_Tab* ETbl,INT64 IDPrm,INT64 Vol,int FlNoModif)
{
	int	rc=-1;
	El_Tab*	tElP;
	El_Tab*	tElP2;
	El_Tab*	tElP3;
	MyLock(__FILE__,__LINE__,&FlSim,this);
	LockEl(&TmpAnchor,ETbl);
	if (MoLockEl(&TmpAnchor,ETbl,AcsMd_Write)!=-1)
	{
//		MyLock(__FILE__,__LINE__,&Fl_RE,this);
		Parametr*	tPrm=Srv->GetParam(IDPrm);
		if ((tPrm)&&(Srv->Fl_Prm!=0))
			if (tPrm->Regularity)
			{
/*				char	tStr[20];
				sprintf(tStr,"%I64d",Vol);
//				MyUnLock(__FILE__,__LINE__,&Fl_RE,this);
				tElP=Srv->IDT_Prm->GetElTab(&TmpAnchor,IDPrm,1);
//				MyLock(__FILE__,__LINE__,&Fl_RE,this);
				if (tElP)
				{
					tElP2=Srv->IDT_Prm->Find(&TmpAnchor,AcsMd_Read,"Value",tElP);
					if (tElP2==NULL)
					{
						tElP2=Srv->IDT_Prm->AddETI(&TmpAnchor,"Value",tElP);
					}
					if (tElP2)
					{
						tElP3=Srv->IDT_Prm->Find(&TmpAnchor,AcsMd_Read,tStr,tElP2);
						if (tElP3==NULL)
						{
							tElP3=Srv->IDT_Prm->AddETI(&TmpAnchor,tStr,tElP2);
						}
						Srv->IDT_Prm->UnLockEl(&TmpAnchor,tElP3);
						Srv->IDT_Prm->UnLockEl(&TmpAnchor,tElP2);
					}
					Srv->IDT_Prm->UnLockEl(&TmpAnchor,tElP);
				}
//				tElP=Srv->IDT_Prm->AttainNestedI(Owner,"%i%s",tStr2,"Value",tStr,-1);
//				UnLockEl(Owner,tElP);
*/			}
		rc=ETbl->GetIDI(Owner)->pPList->SetParam(IDPrm,&Vol,sizeof(INT64),Owner->pDec);
		ETbl->GetIDI()->ModifCnt++;
//		MyUnLock(__FILE__,__LINE__,&Fl_RE,this);
		if (FlNoModif!=1) ModifElTab(&TmpAnchor,ETbl,FlNoModif);
//		MoLockEl(&TmpAnchor,ETbl,AcsMd_Read);
	}
	else
	{
		Srv->SetCriticalError(this,"Error LockEl Write in AddParam","Error",TableName);
	};
	UnLockEl(&TmpAnchor,ETbl);
	MyUnLock(__FILE__,__LINE__,&FlSim,this);
	return rc;
};

int	IDTable::AddParam	(StrOwner* Owner,El_Tab* ETbl,INT64 IDPrm,int Vol,int FlNoModif)
{
	int	rc=-1;
	El_Tab*	tElP;
	El_Tab*	tElP2;
	El_Tab*	tElP3;
	MyLock(__FILE__,__LINE__,&FlSim,this);
	LockEl(&TmpAnchor,ETbl);
	if (MoLockEl(&TmpAnchor,ETbl,AcsMd_Write)!=-1)
	{
//		MyLock(__FILE__,__LINE__,&Fl_RE,this);
		Parametr*	tPrm=Srv->GetParam(IDPrm);
		if ((tPrm)&&(Srv->Fl_Prm!=0))
			if (tPrm->Regularity)
			{
/*				char	tStr[20];
				sprintf(tStr,"%d",Vol);
				tElP=Srv->IDT_Prm->GetElTab(&TmpAnchor,IDPrm,1);
				if (tElP)
				{
					tElP2=Srv->IDT_Prm->Find(&TmpAnchor,AcsMd_Read,"Value",tElP);
					if (tElP2==NULL)
					{
						tElP2=Srv->IDT_Prm->AddETI(&TmpAnchor,"Value",tElP);
					}
					if (tElP2)
					{
						tElP3=Srv->IDT_Prm->Find(&TmpAnchor,AcsMd_Read,tStr,tElP2);
						if (tElP3==NULL)
						{
							tElP3=Srv->IDT_Prm->AddETI(&TmpAnchor,tStr,tElP2);
						}
						Srv->IDT_Prm->UnLockEl(&TmpAnchor,tElP3);
						Srv->IDT_Prm->UnLockEl(&TmpAnchor,tElP2);
					}
					Srv->IDT_Prm->UnLockEl(&TmpAnchor,tElP);
				}
//				tElP=Srv->IDT_Prm->AttainNestedI(Owner,"%i%s",tStr2,"Value",tStr,-1);
//				UnLockEl(Owner,tElP);
*/			}
		rc=ETbl->GetIDI(Owner)->pPList->SetParam(IDPrm,&Vol,sizeof(int),Owner->pDec);
		ETbl->GetIDI()->ModifCnt++;
//		MyUnLock(__FILE__,__LINE__,&Fl_RE,this);
		if (FlNoModif!=1) ModifElTab(&TmpAnchor,ETbl,FlNoModif);
//		MoLockEl(&TmpAnchor,ETbl,AcsMd_Read);

	}
	else
	{
		Srv->SetCriticalError(this,"Error LockEl Write in AddParam","Error",TableName);
	};
	UnLockEl(&TmpAnchor,ETbl);
	MyUnLock(__FILE__,__LINE__,&FlSim,this);
	return rc;
};

int	IDTable::AddParam	(StrOwner* Owner,El_Tab* ETbl,INT64 IDPrm,char* Vol,int FlNoModif)
{
	int	rc=-1;
	El_Tab*	tElP;
	El_Tab*	tElP2;
	El_Tab*	tElP3;
	MyLock(__FILE__,__LINE__,&FlSim,this);
	LockEl(&TmpAnchor,ETbl);
	if (MoLockEl(&TmpAnchor,ETbl,AcsMd_Write)!=-1)
	{
		//		MyLock(__FILE__,__LINE__,&Fl_RE,this);
		Parametr*	tPrm=Srv->GetParam(IDPrm);
		if ((tPrm)&&(Srv->Fl_Prm!=0)&&(IDPrm>=LastPrm))
		{
			if (tPrm->Type==PrmType_STRING)
			{
/*				tElP=Srv->IDT_Prm->GetElTab(&TmpAnchor,IDPrm,1);
				if (tElP)
				{
					int tLen=strlen(Vol);
					int	*tMSize;
					rc=Srv->IDT_Prm->GetParam(&TmpAnchor,tElP,IDMaxSize,tMSize);
					if (rc!=-1)
					{
						if (*tMSize<tLen)
						{
							Srv->IDT_Prm->AddParam(&TmpAnchor,tElP,IDMaxSize,tLen);
						}
					}
					else
					{
						Srv->IDT_Prm->AddParam(&TmpAnchor,tElP,IDMaxSize,tLen);
					}
					Srv->IDT_Prm->UnLockEl(&TmpAnchor,tElP);
				}
*/			}
			if (tPrm->Regularity)
			{
/*				tElP=Srv->IDT_Prm->GetElTab(&TmpAnchor,IDPrm);
				if (tElP)
				{
					tElP2=Srv->IDT_Prm->Find(&TmpAnchor,AcsMd_Read,"Value",tElP);
					if (tElP2==NULL)
					{
						tElP2=Srv->IDT_Prm->AddETI(&TmpAnchor,"Value",tElP);
					}
					if (tElP2)
					{
						tElP3=Srv->IDT_Prm->Find(&TmpAnchor,AcsMd_Read,Vol,tElP2);
						if (tElP3==NULL)
						{
							tElP3=Srv->IDT_Prm->AddETI(&TmpAnchor,Vol,tElP2);
						}
						Srv->IDT_Prm->UnLockEl(&TmpAnchor,tElP3);
						Srv->IDT_Prm->UnLockEl(&TmpAnchor,tElP2);
					}
					Srv->IDT_Prm->UnLockEl(&TmpAnchor,tElP);
				}
//				char	tStr2[20];
//				sprintf(tStr2,"%I64d",IDPrm);
//				tElP=Srv->IDT_Prm->AttainNestedI(this,"%i%s",tStr2,"Value",Vol,-1);
*/			}
		}
		rc=ETbl->GetIDI(Owner)->pPList->SetParam(IDPrm,(void*)Vol,strlen(Vol)+1,Owner->pDec);
		if (FlNoModif==2)
		ETbl->GetIDI()->ModifCnt+=2;
		else
		ETbl->GetIDI()->ModifCnt++;
//		MoLockEl(&TmpAnchor,ETbl,AcsMd_Read);
//		MyUnLock(__FILE__,__LINE__,&Fl_RE,this);
		if (FlNoModif!=1) ModifElTab(&TmpAnchor,ETbl,FlNoModif);
	}
	else
	{
		Srv->SetCriticalError(this,"Error LockEl Write in AddParam","Error",TableName);
	};
	UnLockEl(&TmpAnchor,ETbl);
	MyUnLock(__FILE__,__LINE__,&FlSim,this);
	return rc;
};

int	IDTable::AddParam	(StrOwner* Owner,El_Tab* ETbl,INT64 IDPrm,void* Vol,int Size,int FlNoModif)
{
	int	rc=-1;
//	El_Tab*	tElP;
	MyLock(__FILE__,__LINE__,&FlSim,this);
	LockEl(&TmpAnchor,ETbl);
	if (MoLockEl(&TmpAnchor,ETbl,AcsMd_Write)!=-1)
	{
//		MyLock(__FILE__,__LINE__,&Fl_RE,this);
		Parametr*	tPrm=Srv->GetParam(IDPrm);
		rc=ETbl->GetIDI(Owner)->pPList->SetParam(IDPrm,Vol,Size,Owner->pDec);
		ETbl->GetIDI()->ModifCnt++;
//		MyUnLock(__FILE__,__LINE__,&Fl_RE,this);
		if (FlNoModif!=1) ModifElTab(&TmpAnchor,ETbl,FlNoModif);
//		MoLockEl(Owner,ETbl,AcsMd_Read);
	}
	else
	{
		Srv->SetCriticalError(this,"Error LockEl Write in AddParam","Error",TableName);
	};
	UnLockEl(&TmpAnchor,ETbl);
	MyUnLock(__FILE__,__LINE__,&FlSim,this);
	return rc;
};

int	IDTable::AddParam	(StrOwner* Owner,El_Tab* ETbl,INT64 IDPrm,float Vol,int FlNoModif)
{
	int	rc=-1;
	El_Tab*	tElP;
	El_Tab*	tElP2;
	El_Tab*	tElP3;
	MyLock(__FILE__,__LINE__,&FlSim,this);
	LockEl(&TmpAnchor,ETbl);
	if (MoLockEl(&TmpAnchor,ETbl,AcsMd_Write)!=-1)
	{
//		MyLock(__FILE__,__LINE__,&Fl_RE,this);
		Parametr*	tPrm=Srv->GetParam(IDPrm);
		if ((tPrm)&&(Srv->Fl_Prm!=0))
		{
			if (tPrm->Regularity)
			{
/*				char	tStr[40];
				sprintf(tStr,"%f",Vol);
				tElP=Srv->IDT_Prm->GetElTab(&TmpAnchor,IDPrm,1);
				if (tElP)
				{
					tElP2=Srv->IDT_Prm->Find(&TmpAnchor,AcsMd_Read,"Value",tElP);
					if (tElP2==NULL)
					{
						tElP2=Srv->IDT_Prm->AddETI(&TmpAnchor,"Value",tElP);
					}
					if (tElP2)
					{
						tElP3=Srv->IDT_Prm->Find(&TmpAnchor,AcsMd_Read,tStr,tElP2);
						if (tElP3==NULL)
						{
							tElP3=Srv->IDT_Prm->AddETI(&TmpAnchor,tStr,tElP2);
						}
						Srv->IDT_Prm->UnLockEl(&TmpAnchor,tElP3);
						Srv->IDT_Prm->UnLockEl(&TmpAnchor,tElP2);
					}
					Srv->IDT_Prm->UnLockEl(&TmpAnchor,tElP);
				}
//				char	tStr2[20];
//				sprintf(tStr2,"%I64d",IDPrm);
//				tElP=Srv->IDT_Prm->AttainNestedI(this,"%i%f",tStr2,"Value",tStr,-1);
//				UnLockEl(&TmpAnchor,tElP);
*/			}
			rc=ETbl->GetIDI(Owner)->pPList->SetParam(IDPrm,(void*)&Vol,sizeof(float),Owner->pDec);
			ETbl->GetIDI()->ModifCnt++;
//			MyUnLock(__FILE__,__LINE__,&Fl_RE,this);
			if (FlNoModif!=1) ModifElTab(&TmpAnchor,ETbl,FlNoModif);
//			MoLockEl(&TmpAnchor,ETbl,AcsMd_Read);
		}
	}
	else
	{
		Srv->SetCriticalError(this,"Error LockEl Write in AddParam","Error",TableName);
	};
	UnLockEl(&TmpAnchor,ETbl);
	MyUnLock(__FILE__,__LINE__,&FlSim,this);
	return rc;
};

int	IDTable::AddParam	(StrOwner* Owner,El_Tab* ETbl,INT64 IDPrm,double Vol,int FlNoModif)
{
	int	rc=-1;
	El_Tab*	tElP;
	El_Tab*	tElP2;
	El_Tab*	tElP3;
	MyLock(__FILE__,__LINE__,&FlSim,this);
	LockEl(&TmpAnchor,ETbl);
	if (MoLockEl(&TmpAnchor,ETbl,AcsMd_Write)!=-1)
	{
//		MyLock(__FILE__,__LINE__,&Fl_RE,this);
		Parametr*	tPrm=Srv->GetParam(IDPrm);
		if ((tPrm)&&(Srv->Fl_Prm!=0))
			if (tPrm->Regularity)
			{
/*				char	tStr[80];
				sprintf(tStr,"%e",Vol);
				tElP=Srv->IDT_Prm->GetElTab(&TmpAnchor,IDPrm,1);
				if (tElP)
				{
					tElP2=Srv->IDT_Prm->Find(&TmpAnchor,AcsMd_Read,"Value",tElP);
					if (tElP2==NULL)
					{
						tElP2=Srv->IDT_Prm->AddETI(&TmpAnchor,"Value",tElP);
					}
					if (tElP2)
					{
						tElP3=Srv->IDT_Prm->Find(&TmpAnchor,AcsMd_Read,tStr,tElP2);
						if (tElP3==NULL)
						{
							tElP3=Srv->IDT_Prm->AddETI(&TmpAnchor,tStr,tElP2);
						}
						Srv->IDT_Prm->UnLockEl(&TmpAnchor,tElP3);
						Srv->IDT_Prm->UnLockEl(&TmpAnchor,tElP2);
					}
					Srv->IDT_Prm->UnLockEl(&TmpAnchor,tElP);
				}
*/
//				char	tStr2[20];
//				sprintf(tStr2,"%I64d",IDPrm);
//				tElP=Srv->IDT_Prm->AttainNestedI(this,"%i%s",tStr2,"Value",tStr,-1);
//				UnLockEl(&TmpAnchor,tElP);
			}
			rc=ETbl->GetIDI(Owner)->pPList->SetParam(IDPrm,(void*)&Vol,sizeof(double),Owner->pDec);
			ETbl->GetIDI()->ModifCnt++;
//			MyUnLock(__FILE__,__LINE__,&Fl_RE,this);
			if (FlNoModif!=1) ModifElTab(&TmpAnchor,ETbl,FlNoModif);
//			MoLockEl(&TmpAnchor,ETbl,AcsMd_Read);
	}
	else
	{
		Srv->SetCriticalError(this,"Error LockEl Write in AddParam","Error",TableName);
	};
	UnLockEl(&TmpAnchor,ETbl);
	MyUnLock(__FILE__,__LINE__,&FlSim,this);
	return rc;
};
/*
int	IDTable::AddParam	(StrOwner* Owner,El_Tab* ETbl,INT64 IDPrm,char Vol,int FlNoModif)
{
	int	rc=-1;
	El_Tab*	tElP;
	El_Tab*	tElP2;
	El_Tab*	tElP3;
	if (MoLockEl(Owner,ETbl,AcsMd_Write)!=-1)
	{
		Parametr*	tPrm=Srv->GetParam(IDPrm);
		if ((tPrm)&&(Srv->Fl_Prm!=0))
			if (tPrm->Regularity)
			{
				tElP=Srv->IDT_Prm->GetElTab(Owner,IDPrm);
				if (tElP)
				{
					tElP2=Srv->IDT_Prm->Find(Owner,AcsMd_Read,"Value",tElP);
					if (tElP2==NULL)
					{
						tElP2=Srv->IDT_Prm->AddETI(Owner,"Value",tElP);
					}
					if (tElP2)
					{
						tElP3=Srv->IDT_Prm->Find(Owner,AcsMd_Read,Vol,tElP2);
						if (tElP3==NULL)
						{
							tElP3=Srv->IDT_Prm->AddETI(Owner,Vol,tElP2);
						}
						Srv->IDT_Prm->UnLockEl(Owner,tElP3);
						Srv->IDT_Prm->UnLockEl(Owner,tElP2);
					}
					Srv->IDT_Prm->UnLockEl(&TmpAnchor,tElP);
				}
//				char	tStr2[20];
//				sprintf(tStr2,"%I64d",IDPrm);
//				tElP=Srv->IDT_Prm->AttainNestedI(this,"%i%c",tStr2,"Value",Vol,-1);
//				UnLockEl(&TmpAnchor,tElP);
			}
			rc=ETbl->GetIDI()->pPList->SetParam(IDPrm,(void*)&Vol,1,Owner->pDec);
			ETbl->GetIDI()->ModifCnt++;
			if (FlNoModif!=1) ModifElTab(Owner,ETbl,FlNoModif);
//			if (FlNoModif==0) ModifElTab(Owner,ETbl);
			MoLockEl(Owner,ETbl,AcsMd_Read);
	}
	return rc;
};
*/

int	IDTable::AddParamI	(StrOwner* Owner,El_Tab* ETbl,INT64 IDPrm,char* Vol,int FlNoModif)
{
	int	rc=-1;
//	El_Tab*	tElP;
	if (MoLockEl(Owner,ETbl,AcsMd_Write)!=-1)
	{
		//	Parametr*	tPrm=Srv->GetParam(IDPrm);
		//	if ((tPrm)&&(Srv->Fl_Prm!=0))
		//		if (tPrm->Regularity)
		//		{
		//			tElP=Srv->IDT_Prm->AttainNested(Owner,AcsMd_Read,"%s%s%s",tPrm->Name,"Value",Vol,-1);
		//		}
		AddParam(Owner,ETbl,IDPrm,Vol,1);
		//	rc=ETbl->GetIDI()->pPList->SetParam(IDPrm,(void*)Vol,strlen(Vol)+1,Owner->pDec);
		ETbl->GetIDI(Owner)->ModifCnt++;
		if (FlNoModif!=1) ModifElTab(Owner,ETbl,FlNoModif);
//		if (FlNoModif==0) ModifElTab(Owner,ETbl);
		MoLockEl(Owner,ETbl,AcsMd_Read);
	}
	else
	{
		Srv->SetCriticalError(this,"Error LockEl Write in AddParamI","Error",TableName);
	};
	return rc;
};

int	IDTable::GetParam	(StrOwner* Owner,El_Tab* ETbl,INT64 IDPrm,float* &Vol)
{
	int	rc=-1;
	Vol=NULL;
	if (ETbl) rc=ETbl->GetIDI(Owner)->pPList->GetParam(IDPrm,(void*&)Vol);
	return rc;
};
int	IDTable::GetParam	(StrOwner* Owner,El_Tab* ETbl,INT64 IDPrm,double* &Vol)
{
	int	rc=-1;
	Vol=NULL;
	if (ETbl) rc=ETbl->GetIDI(Owner)->pPList->GetParam(IDPrm,(void*&)Vol);
	return rc;
};
/*
int	IDTable::GetParam	(StrOwner* Owner,El_Tab* ETbl,INT64 IDPrm,char &Vol)
{
	int	rc=-1;
	Vol=NULL;
	if (ETbl) rc=ETbl->GetIDI()->pPList->GetParam(IDPrm,(void*&)Vol);
	return rc;
};
*/
int	IDTable::GetParam	(StrOwner* Owner,El_Tab* ETbl,INT64 IDPrm,void* &Vol)
{
	int	rc=-1;
	Vol=NULL;
	if (ETbl) rc=ETbl->GetIDI(Owner)->pPList->GetParam(IDPrm,(void*&)Vol);
	return rc;
};

int	IDTable::GetParam	(StrOwner* Owner,El_Tab* ETbl,INT64 IDPrm,int* &Vol)
{
	int	rc=-1;
	Vol=NULL;
	if (ETbl) rc=ETbl->GetIDI(Owner)->pPList->GetParam(IDPrm,(void*&)Vol);
	return rc;
};

int	IDTable::GetParam	(StrOwner* Owner,El_Tab* ETbl,INT64 IDPrm,INT64* &Vol)
{
	int	rc=-1;
	Vol=NULL;
	if (ETbl) rc=ETbl->GetIDI(Owner)->pPList->GetParam(IDPrm,(void*&)Vol);
	return rc;
};

int	IDTable::GetParam	(StrOwner* Owner,El_Tab* ETbl,INT64 IDPrm,char* &Vol)
{
	int	rc=-1;
	Vol=NULL;
	if (ETbl) rc=ETbl->GetIDI(Owner)->pPList->GetParam(IDPrm,(void*&)Vol);
	return rc;
};

int	IDTable::DelFromQueryList(INT64	ID)
{
	ID_Elem64*	tEl=NULL;
	int	rc=0;
	if (IDL_Query)
	{
		tEl=(ID_Elem64*)IDL_Query->Find((UCHAR*)&ID,sizeof(INT64));
		if (tEl)
		{
			if (Fl_Debug)
			{
				sprintf(LogBuf,"DelQueryList ID=%I64x",ID); 
				Srv->OutLog(LogBuf,this,TableName);
			}
			IDL_Query->Del(tEl);
		};
	};
	return rc;
}

int		IDTable::DataIDI(int N_Pin, IDInfo* tIDITr)
{
	int	OCnt=0;
	void*	tVol;
	char*	tV;
	INT64	tID;
	//	INT64	tID2;
	int	rc=0;
	El_Tab*		tElT;
	El_Tab*		tElTn;
	El_Tab*		tElTt;
	El_Tab*		tElT2;
	//	El_Tab*		tElT3;
	ID_Elem64*	tEl;
	LastHost[0]=0;
	FileTime	RealT;
	char		NameEl[128];

	if (tIDITr->CustomerID)
	{
		switch (ID)
		{
		case TabNeuronLocalCustomer:
			if ((tIDITr->CustomerID&0xffff0000)!=(CustomerID&0xffff0000))
			{
				return rc;
			}
			break;
		case TabNeuronLocalSite:
		case TabParametr:
			if (tIDITr->CustomerID!=CustomerID)
			{
				return rc;
			}
			break;
		case TabNeuronGlobal:
			break;
		};
	}
	else
	{
		return rc;
	};
	if (Fl_Stop!=1)
	{
		switch(N_Pin)
		{
		case 0:	// From FSrv
// 			if (BLDel.Find(tIDITr->ID_File))
// 			{
// 				El_Tab	*tElTbl;
// 				tElTbl=new El_Tab(&TmpAnchor,tIDITr->ID_File,this);
// 				tElTbl->GetIDI(&TmpAnchor)->ID_SeansFrom=ID;
// 				DelEl(tElTbl);
// 				delete tElTbl;
// 				break;
// 			}
			switch (tIDITr->TypeIDI)
			{
			case TypeIDI_TABLE:
				AddIDI(tIDITr);
				if (tIDITr->StateBlock&StateBlock_END)
				{
					Fl_Long=0;
				}
				else
				{
					Fl_Long=1;
				}
				break;
			case TypeIDI_ANSWER:
				if (tIDITr->StateBlock&StateBlock_ERROR)
				{
					CntActQuery--;
					if ((tIDITr->ID_File==ID)&&(ElTbl==0))
					{	// íåîáõîäèìî ñîçäàòü êîðåíü òàáëèöû
						ElTbl=(El_Tab*)IDL->NewElem();
						if (ElTbl==NULL)
							ElTbl=new El_Tab(&TmpAnchor,ID,this);
						else
							ElTbl->Init(&TmpAnchor,ID,this);
						//							ElTbl->GetIDI()->ID_File=ID;
						Srv->GetRealTime(&ElTbl->GetIDI(&TmpAnchor)->TimeOper.FTime);
						if ((ElTbl->GetIDI()->ID_File==ElTbl->GetIDI()->ID_ParentFile)||((ElTbl->GetIDI()->ParentFile)&&(ElTbl->GetIDI()->ID_File==ElTbl->GetIDI()->ParentFile->ID_File)))
						{
							{size_t iasm=1;}
						}
						else
							IDL->Add(ElTbl);
						UpDataEl2(ElTbl);
						UnLockEl(&Anchor,ElTbl);
					}
				}
				break;
			}
			break;
		case 2:	// From Replicator
			LastHost[0]=0;
			if (Srv->Fl_Repl)
			{
//				if (tIDITr->ID_ParentFile==1)	//0x252800005ee8e306
//					tIDITr->ID_ParentFile=1;
//				if (tIDITr->ID_File==0x182800000adb038c)	//0x252800005ee8e306
//					tIDITr->ID_File=0x182800000adb038c;
// 				if (tIDITr->pPList->GetParam(IDMyHostName,(void*&)tV)!=-1)
				if ((tIDITr->AdrInfo)&&(tIDITr->AdrInfo->NativeAI.HostName[0]))
				{
					if (strcmp(tIDITr->AdrInfo->NativeAI.HostName,Srv->ShortHostName)==0)
					{
						break;
					}
					else
					{
						strcpy(LastHost,tIDITr->AdrInfo->NativeAI.HostName);
					}
				};
				DelFromQueryList(tIDITr->ID_File);
				if (IDL_Query)
				{
					tEl=(ID_Elem64*)IDL_Query->Find((UCHAR*)&tIDITr->ID_File,sizeof(INT64));
					if (tEl)
					{
						if (Fl_Debug)
						{
							sprintf(LogBuf,"DelQueryList ID=%I64x",tIDITr->ID_File); 
							Srv->OutLog(LogBuf,this,TableName);
						};
						IDL_Query->Del(tEl);
					};
				};
				switch (tIDITr->TypeIDI)
				{
				case TypeIDI_TABLE:
					switch (tIDITr->Command)
					{
					case Command_REPAIR:
						if ((tIDITr->pPList->GetParam(IDNameElTab,(void*&)tV)!=0)&&(tIDITr->ID_File>10))
							break;
						BLDel.DelElemID(tIDITr->ID_File);
						break;
					case Command_MOVE:
						if ((tIDITr->pPList->GetParam(IDNameElTab,(void*&)tV)!=0)&&(tIDITr->ID_File>10))
							break;
/*						if (BLDel.Find(tIDITr->ID_File))
						{
							El_Tab	*tElTbl;
							tElTbl=(El_Tab*)IDL->Find((UCHAR*)&tIDITr->ID_File,sizeof(INT64));
							if (tElTbl)
							{
								if (tElTbl->Fl_Del==0)
									DelElTab(&TmpAnchor,tElTbl);
								DelEl(tElTbl);
							}
							else
							{
								tElTbl=new El_Tab(&TmpAnchor,tIDITr->ID_File,this);
								{
									if (Fl_Debug)
									{
										sprintf(LogBuf,"ReDelEl ID=%I64x",tElTbl->GetIDI()->ID_File);
										Srv->OutLog(LogBuf,this,"IDTableDel");
									};
									tElTbl->GetIDI()->Command=Command_DEL;
									tElTbl->GetIDI()->TypeIDI=TypeIDI_TABLE;
									tElTbl->GetIDI()->StateBlock=StateBlock_BEGIN|StateBlock_END;
									Srv->GetRealTime(&tElTbl->GetIDI()->TimeOper.FTime);
									tElTbl->GetIDI()->TimeClose.FTime=tElTbl->GetIDI()->TimeOper.FTime;
									tElTbl->GetIDI()->NumPack=1;
									strcpy(LastHost,"");
									tElTbl->GetIDI()->ID_SeansFrom=ID;
//									tElTbl->GetIDI()->pPList->SetParam(IDHostName,&LastHost,strlen(LastHost)+1,this);
									strcpy(tElTbl->GetIDI()->AdrInfo->DestinAI.HostName,LastHost);
//									MyUnLock(__FILE__,__LINE__,&Fl_RE,this);
									Srv->ReplElTabl(tElTbl);
//									ODataIDI(3,tElTbl->GetIDI());
//									MyLock(__FILE__,__LINE__,&Fl_RE,this);
								}
								delete tElTbl;
							}
							break;
						};
*/						rc=0;
						if (Fl_Debug)
						{
// 							if (tIDITr->pPList->GetParam(IDMyHostName,(void*&)tV)!=-1)
// 							{
// 								sprintf(LogBuf,"Comm_Move ID=%I64x From=%s",tIDITr->ID_File,tV); 
// 							}
// 							else
// 							{
// 								sprintf(LogBuf,"Comm_Move ID=%I64x",tIDITr->ID_File); 
// 							}
// 							Srv->OutLog(LogBuf,this,TableName);
						};
						tElT=(El_Tab*)IDL->Find((UCHAR*)&tIDITr->ID_File,sizeof(INT64));
						if (tElT==NULL)
						{
							tElT=LoadEl(&TmpAnchor,tIDITr->ID_File);
						}
						if (tElT!=NULL)
						{
							tElT2=(El_Tab*)IDL->Find((UCHAR*)&tIDITr->ID_ParentFile,sizeof(INT64));
							if (tElT2==NULL)
							{
								tElT2=LoadEl(&TmpAnchor,tIDITr->ID_ParentFile);
							}
							if (tElT2)
							{
								tElT=MoveET(&TmpAnchor,tElT,tElT2,1);
							}
							else
							{
								if (tIDITr->ID_ParentFile)
									QueryEl(tIDITr->ID_ParentFile);	// Çàïðîñèòü íåäîñòàþùèé ýëåìåíò
								QueryEl(tIDITr->ID_File);	// Çàïðîñèòü íåäîñòàþùèé ýëåìåíò
							}
/*							LockEl(&Anchor,tElT);
							MoLockEl(&Anchor,tElT,AcsMd_Write);
							if (tElT->pParent==NULL)
							{
								if (tIDITr->ID_File==ID)	// Òîëüêî êîðåíü!
								{
									if(tIDITr->TimeLastWr.Time>tElT->GetIDI()->TimeLastWr.Time)
									{
										if (GetParam(&Anchor,tElT,IDNameElTab,tV)==0)
										{
											sprintf(LogBuf,"Repl->CopyEl Name=%s",tV); 
										}
										else
										{
											sprintf(LogBuf,"Repl->CopyEl ID=%I64d",tIDITr->ID_File);
										}
										Srv->OutLog(LogBuf,this,TableName);
										rc=CopyIDI2El(tIDITr,tElT,0);
										if (rc!=-1)
										{
											if (tIDITr->Cnt!=0)
											{
												int		tCntP=tIDITr->Cnt/sizeof(INT64);
												INT64*	tBufP=(INT64*)tIDITr->Buf;
												for (size_t i=0;i<tCntP;i++) // Çàïðîñ îñòàëüíûõ ýëåìåíòîâ òàáëèöû
												{
													if (tElT->AddChild(tBufP[i],1)==0)
													{
														tElTt=GetElTab(&Anchor,tBufP[i]);
														UnLockEl(&Anchor,tElTt);
													}
												}
											}
											SaveEl(tElT);
											if (TestEl(tElT)!=-1) SendEl(tElT);
											else	tElT=0;

										}
										UnLockEl(&Anchor,tElT);
									}
									else
									{
										if(tIDITr->TimeLastWr.Time<tElT->GetIDI()->TimeLastWr.Time)
										{
											ReplEl(tElT);
										}
										else
										{
											if (tIDITr->Cnt!=0)
											{
												int		tCntP=tIDITr->Cnt/sizeof(INT64);
												INT64*	tBufP=(INT64*)tIDITr->Buf;
												for (size_t i=0;i<tCntP;i++) // Çàïðîñ îñòàëüíûõ ýëåìåíòîâ òàáëèöû
												{
													if (tElT->AddChild(tBufP[i],1)==0)
													{
														tElTt=GetElTab(&Anchor,tBufP[i]);
														UnLockEl(&Anchor,tElTt);
													}
												}
											}
										}
									}
								}
							}
							else
							{
								if(tIDITr->TimeLastWr.Time>tElT->GetIDI()->TimeLastWr.Time)
								{
									if (GetParam(&Anchor,tElT,IDNameElTab,tV)==0)
									{
										sprintf(LogBuf,"Repl->CopyEl Name=%s",tV);
									}
									else
									{
										sprintf(LogBuf,"Repl->CopyEl ID=%I64d",tIDITr->ID_File);
									}
									Srv->OutLog(LogBuf,this,TableName);
									rc=CopyIDI2El(tIDITr,tElT,0);
									if (rc!=-1)
									{
										if (tIDITr->Cnt!=0)
										{
											int		tCntP=tIDITr->Cnt/sizeof(INT64);
											INT64*	tBufP=(INT64*)tIDITr->Buf;
											for (size_t i=0;i<tCntP;i++) // Çàïðîñ îñòàëüíûõ ýëåìåíòîâ òàáëèöû
											{
												if (tElT->AddChild(tBufP[i],1)==0)
												{
													tElTt=GetElTab(&Anchor,tBufP[i]);
													UnLockEl(&Anchor,tElTt);
												}
											}
										}
										SaveEl(tElT);
//										UpDataEl(tElT);
										if (tElT->OwnerList)
										{
											if (tElT->OwnerList->NumElem)
											{
												tElT->GetIDI()->Command=Command_MODIFY;
												tElT->GetIDI()->StateBlock=StateBlock_BEGIN|StateBlock_END|StateBlock_EMPTY;
												tElT->GetIDI()->Cnt=0;
												ODataFF(2,(UCHAR*)tElT->GetIDI(),sizeof(IDInfo));
											}
										}
//										if (TestEl(tElT)!=-1) SendEl(tElT);
										if (TestEl(tElT)==-1) tElT=0;
									}
									UnLockEl(&Anchor,tElT);
								}
								else
								{
									if(tIDITr->TimeLastWr.Time<tElT->GetIDI()->TimeLastWr.Time)
									{
										ReplEl(tElT);
									}
									else
									{
										if (tIDITr->Cnt!=0)
										{
											int		tCntP=tIDITr->Cnt/sizeof(INT64);
											INT64*	tBufP=(INT64*)tIDITr->Buf;
											for (size_t i=0;i<tCntP;i++) // Çàïðîñ îñòàëüíûõ ýëåìåíòîâ òàáëèöû
											{
												if (tElT->AddChild(tBufP[i],1)==0)
												{
													tElTt=GetElTab(&Anchor,tBufP[i]);
													UnLockEl(&Anchor,tElTt);
												}
											}
//											if (TestEl(tElT)==-1) tElT=0;
//											else	SaveEl(tElT);
//											if (TestEl(tElT)!=-1) SendEl(tElT);
										}
									}
								}
							}
*/			
							UnLockEl(&TmpAnchor,tElT);
							rc=0;
						}
						else
						{	
 							tElT=(El_Tab*)IDL->Find((UCHAR*)&tIDITr->ID_ParentFile,sizeof(INT64));
 							if (tElT)
 							{	// Äîáàâëÿåì 
//								MyUnLock(__FILE__,__LINE__,&Fl_RE,this);
								tElT2=CopyIDI2Parent(&TmpAnchor,tIDITr,tElT);
//								MyLock(__FILE__,__LINE__,&Fl_RE,this);
								if (TestEl(tElT2)==-1)
									tElT2=0;
								if (tElT2)
									if (tIDITr->Cnt!=0)
									{
										int		tCntP=tIDITr->Cnt/sizeof(INT64);
										INT64*	tBufP=(INT64*)tIDITr->Buf;
										for (size_t i=0;i<tCntP;i++) // Çàïðîñ îñòàëüíûõ ýëåìåíòîâ òàáëèöû
										{
											if (tBufP[i])
											{
												if (tElT2->AddChild(tBufP[i],1)==0)
												{
													tElTt=GetElTab(&TmpAnchor,tBufP[i]);
													UnLockEl(&TmpAnchor,tElTt);
												}
											}
										}
										SaveEl(tElT2);
									}
								UnLockEl(&TmpAnchor,tElT2);
								SendEl(tElT);
								UnLockEl(&TmpAnchor,tElT);
							}
							else
							{
								QueryEl(tIDITr->ID_ParentFile);	// Çàïðîñèòü íåäîñòàþùèé ýëåìåíò
//								QueryEl(tIDITr->ID_File);			// Çàïðîñèòü ïîâòîð ýëåìåíòà
							}
						}
						break;
					case Command_DEL:
//						break;
						if (Fl_Debug)
						{
// 							if (tIDITr->pPList->GetParam(IDMyHostName,(void*&)tV)!=-1)
// 							{
// 								sprintf(LogBuf,"Comm_Del ID=%I64x From=%s",tIDITr->ID_File,tV); 
// 							}
// 							else
// 							{
// 								sprintf(LogBuf,"Comm_Del ID=%I64x",tIDITr->ID_File);
// 							}
// 							Srv->OutLog(LogBuf,this,TableName);
						};
//						if (!BLDel.Find(tIDITr->ID_File))
//						{
							tElT=GetElTab(&TmpAnchor,tIDITr->ID_File,1);
							if (tElT)
							{
								if (Fl_Debug)
								{
									if (GetParam(&TmpAnchor,tElT,IDNameElTab,tV)==0)
									{
										sprintf(LogBuf,"Repl->DelEl Name=%s",tV); Srv->OutLog(LogBuf,this,TableName);
									}
								}
								if (tElT->Fl_Del==0)
								{
//									MyUnLock(__FILE__,__LINE__,&Fl_RE,this);
									DelElTab(&TmpAnchor,tElT,1);	// !!!!!!!!!!!!!!!!!!!!!!!!
//									DelElTabRO(&TmpAnchor,tElT,1);	// !!!!!!!!!!!!!!!!!!!!!!!!
//									MyLock(__FILE__,__LINE__,&Fl_RE,this);
								}
								BLDel.Add(tIDITr->ID_File);
							}
//						}
//						else
//						{
//							BLDel.Add(tIDITr->ID_File);
//						}
						break;
//					case Command_ADD_CHILD:
					case Command_MODIFY:
					case Command_SAVE:
						tV=NULL;
						tIDITr->pPList->GetParam(IDNameElTab,(void*&)tV);
						if ((tV==0)&&(tIDITr->ID_File>10))
							break;
						if (tV==0)
							strcpy(NameEl,"Root");
						else
							strcpy(NameEl,(char*)tV);
						Srv->GetRealTime(&RealT.FTime);
// 						if (tIDITr->TimeLastWr.Time>RealT.Time)
// 						{
// 							tIDITr->TimeLastWr.Time=RealT.Time;
// 						}
// 						if (tIDITr->ID_File==0x182800000adb038c)	//0x252800005ee8e306
// 							tIDITr->ID_File=0x182800000adb038c;
						if (tIDITr->pPList->GetParam(IDNoDelete,tVol)!=0)
						{
/*							if(BLDel.Find(tIDITr->ID_File))
							{
								El_Tab	*tElTbl;
								tElTbl=(El_Tab*)IDL->Find((UCHAR*)&tIDITr->ID_File,sizeof(INT64));
								if (tElTbl)
								{
									if (tElTbl->Fl_Del==0)
										DelElTab(&TmpAnchor,tElTbl);
									DelEl(tElTbl);
								}
								else
								{
									tElTbl=new El_Tab(&TmpAnchor,tIDITr->ID_File,this);
									{
										if (Fl_Debug)
										{
											sprintf(LogBuf,"ReDelEl ID=%I64x",tElTbl->GetIDI()->ID_File);
											Srv->OutLog(LogBuf,this,"IDTableDel");
										}
										tElTbl->GetIDI()->Command=Command_DEL;
										tElTbl->GetIDI()->TypeIDI=TypeIDI_TABLE;
										tElTbl->GetIDI()->StateBlock=StateBlock_BEGIN|StateBlock_END;
										Srv->GetRealTime(&tElTbl->GetIDI()->TimeOper.FTime);
										tElTbl->GetIDI()->TimeClose.FTime=tElTbl->GetIDI()->TimeOper.FTime;
										tElTbl->GetIDI()->NumPack=1;
										strcpy(LastHost,"");
										tElTbl->GetIDI()->ID_SeansFrom=ID;
//										tElTbl->GetIDI()->pPList->SetParam(IDHostName,&LastHost,strlen(LastHost)+1,this);
										strcpy(tElTbl->GetIDI()->AdrInfo->DestinAI.HostName,LastHost);
//										MyUnLock(__FILE__,__LINE__,&Fl_RE,this);
										Srv->ReplElTabl(tElTbl);
//										ODataIDI(3,tElTbl->GetIDI());
//										MyLock(__FILE__,__LINE__,&Fl_RE,this);
									}
									delete tElTbl;
								}
								break;
							}
*/						}
						else
						{
							BLDel.DelElemID(tIDITr->ID_File);
							tIDITr->pPList->DelParam(IDNoDelete);
						};
						rc=0;
//						if (Fl_Debug)
//						{
// 							if (tIDITr->pPList->GetParam(IDMyHostName,(void*&)tV)!=-1)
// 							{
// 								sprintf(LogBuf,"Comm_Mod Name=%s ID=%I64x From=%s",NameEl,tIDITr->ID_File,tV); 
// 							}
// 							else
// 							{
// 								sprintf(LogBuf,"Comm_Mod Name=%s ID=%I64x",NameEl,tIDITr->ID_File); 
// 							}
// 							Srv->OutLog(LogBuf,this,TableName);
//						};
						tElT=(El_Tab*)IDL->Find((UCHAR*)&tIDITr->ID_File,sizeof(INT64));
						if (tElT==NULL)
						{
							if (tIDITr->ID_File==tIDITr->ID_ParentFile)
							{
								char	ttBuf[80];
								sprintf(ttBuf,"Repl ID==Parent=%I64x",tIDITr->ID_File);
								Srv->SetCriticalError(this,ttBuf,"Error",TableName);
//								MyUnLock(__FILE__,__LINE__,&Fl_RE,this);
								return rc;
							}
							tElT=LoadEl(&TmpAnchor,tIDITr->ID_File);
						}
						if (tElT!=NULL)
						{
// 							if (tElT->Fl_Del==1)
// 							{
// 								DelEl(tElT);
// 								return rc;
// 							}
							// Был ли элемент удален?
							if (tIDITr->TimeBegin.Time!=tElT->GetIDI()->TimeBegin.Time)
							{	// Element was delete or renew
								if (tIDITr->TimeBegin.Time>tElT->GetIDI()->TimeBegin.Time)
								{// tIDITr - New
									El_Tab*	TElTPar=NULL;
									if(tIDITr->ID_ParentFile)
										TElTPar=LoadEl(&TmpAnchor,tIDITr->ID_ParentFile);
									else
									{
										rc=CopyIDI2El(tIDITr,tElT,0);
										if (rc!=-1)
										{
											if (CompareChild(tIDITr,tElT))
												SaveEl(tElT);
// 											if (tIDITr->Cnt!=0)
// 											{
// 												int		tCntP=tIDITr->Cnt/sizeof(INT64);
// 												INT64*	tBufP=(INT64*)tIDITr->Buf;
// 												for (size_t i=0;i<tCntP;i++) // Çàïðîñ îñòàëüíûõ ýëåìåíòîâ òàáëèöû
// 												{
// 													if (tBufP[i])
// 													{
// 														if (tElT->AddChild(tBufP[i],1)==0)
// 														{
// 															tElTt=GetElTab(&TmpAnchor,tBufP[i]);
// 															UnLockEl(&TmpAnchor,tElTt);
// 														}
// 													}
// 												}
// 											}
// 											SaveEl(tElT);
											if (TestEl(tElT)!=-1) 
												SendEl(tElT);
											else
												tElT=0;

										}
									}
									if (TElTPar)
									{
										tElTn=MoveETInternal(&TmpAnchor,tElT,TElTPar);
										tElT=0;
										rc=CopyIDI2El(tIDITr,tElTn,0);
										if (rc!=-1)
										{
											if (CompareChild(tIDITr,tElTn))
												SaveEl(tElTn);
// 											if (tIDITr->Cnt!=0)
// 											{
// 												int		tCntP=tIDITr->Cnt/sizeof(INT64);
// 												INT64*	tBufP=(INT64*)tIDITr->Buf;
// 												for (size_t i=0;i<tCntP;i++) // Çàïðîñ îñòàëüíûõ ýëåìåíòîâ òàáëèöû
// 												{
// 													if (tBufP[i])
// 													{
// 														if (tElTn->AddChild(tBufP[i],1)==0)
// 														{
// 															tElTt=GetElTab(&TmpAnchor,tBufP[i]);
// 															UnLockEl(&TmpAnchor,tElTt);
// 														}
// 													}
// 												}
// 											}
											if (TestEl(tElTn)!=-1) 
												SendEl(tElTn);
// 											else
// 												tElT=0;

										}
										UnLockEl(&TmpAnchor,tElTn);
									}
									else
									{
										if (tIDITr->ID_ParentFile)
											QueryEl(tIDITr->ID_ParentFile);	// Çàïðîñèòü íåäîñòàþùèé ýëåìåíò
									}
								}
								else
								{//	tElT - New
									tElT->Fl_Query=0;
									ReplEl(tElT);
								}
								break;
							};
							// Проверка на ошибку
							if (tIDITr->ID_File==tIDITr->ID_ParentFile)
							{
								char	ttBuf[80];
								sprintf(ttBuf,"Repl Del ID==Parent=%I64x",tIDITr->ID_File);
								Srv->SetCriticalError(this,ttBuf,"Error",TableName);
//								DelElTab(&TmpAnchor,tElT);
//								MyUnLock(__FILE__,__LINE__,&Fl_RE,this);
								return rc;
							}
							LockEl(&TmpAnchor,tElT);
							// У элемента нет парента?
							if (tElT->pParent==NULL)
							{
								if (tIDITr->ID_File==ID)	// Òîëüêî êîðåíü!
								{
//									if(tIDITr->TimeLastWr.Time>tElT->GetIDI()->TimeLastWr.Time)
									if (tElT->IDL64==NULL)
										tElT->GetIDI()->ID_Standart=0;
									else
										tElT->GetIDI()->ID_Standart=tElT->IDL64->NumElem;
									if(TestNewest(tIDITr,tElT->GetIDI()))
									{
										if (Fl_Debug)
										{
											if (GetParam(&TmpAnchor,tElT,IDNameElTab,tV)==0)
											{
												sprintf(LogBuf,"Repl->CopyEl Name=%s",tV); 
											}
											else
											{
												sprintf(LogBuf,"Repl->CopyEl ID=%I64d",tIDITr->ID_File);
											}
											Srv->OutLog(LogBuf,this,TableName);
										}
										rc=CopyIDI2El(tIDITr,tElT,0);
										if (rc!=-1)
										{
											if (CompareChild(tIDITr,tElT))
												SaveEl(tElT);
// 											if (tIDITr->Cnt!=0)
// 											{
// 												int		tCntP=tIDITr->Cnt/sizeof(INT64);
// 												INT64*	tBufP=(INT64*)tIDITr->Buf;
// 												for (size_t i=0;i<tCntP;i++) // Çàïðîñ îñòàëüíûõ ýëåìåíòîâ òàáëèöû
// 												{
// 													if (tBufP[i])
// 													{
// 														if (tElT->AddChild(tBufP[i],1)==0)
// 														{
// 															tElTt=GetElTab(&TmpAnchor,tBufP[i]);
// 															UnLockEl(&TmpAnchor,tElTt);
// 														}
// 													}
// 												}
// 											}
											if (TestEl(tElT)!=-1) 
												SendEl(tElT);
											else
												tElT=0;

										}
										UnLockEl(&TmpAnchor,tElT);
									}
									else
									{
										CompareChild(tIDITr,tElT);
// 										int		tCntP=tIDITr->Cnt/sizeof(INT64);
// //										if(tIDITr->TimeLastWr.Time<tElT->GetIDI()->TimeLastWr.Time)
// 										if((tIDITr->ModifCnt!=tElT->GetIDI()->ModifCnt)||(tIDITr->ID_Standart<tElT->GetIDI()->ID_Standart))
// 										{
// 											if (tIDITr->Cnt!=0)
// 											{
// 												tCntP=tIDITr->Cnt/sizeof(INT64);
// 												INT64*	tBufP=(INT64*)tIDITr->Buf;
// 												for (size_t i=0;i<tCntP;i++) // Çàïðîñ îñòàëüíûõ ýëåìåíòîâ òàáëèöû
// 												{
// 													if (tBufP[i])
// 													{
// 														if (tElT->AddChild(tBufP[i],1)==0)
// 														{
// 															tElTt=GetElTab(&TmpAnchor,tBufP[i]);
// 															UnLockEl(&TmpAnchor,tElTt);
// 														}
// 													}
// 												}
// 											}
// 											ReplEl(tElT);
// 										}
// 										else
// 										{
// 											if (tIDITr->Cnt!=0)
// 											{
// 												
// 												INT64*	tBufP=(INT64*)tIDITr->Buf;
// 												for (size_t i=0;i<tCntP;i++) // Çàïðîñ îñòàëüíûõ ýëåìåíòîâ òàáëèöû
// 												{
// 													if (tBufP[i])
// 													{
// 														if (tElT->AddChild(tBufP[i],1)==0)
// 														{
// 															tElTt=GetElTab(&TmpAnchor,tBufP[i]);
// 															UnLockEl(&TmpAnchor,tElTt);
// 														}
// 
// 													}
// 												}
// 											}
// 										}
									}
								}
							}
							else
							{
//								if(tIDITr->TimeLastWr.Time>tElT->GetIDI()->TimeLastWr.Time)
								// Уелемента есть парент!
								if (tElT->IDL64==NULL)
									tElT->GetIDI()->ID_Standart=0;
								else
									tElT->GetIDI()->ID_Standart=tElT->IDL64->NumElem;
								if(TestNewest(tIDITr,tElT->GetIDI()))
								{
									if (Fl_Debug)
									{
										if (GetParam(&TmpAnchor,tElT,IDNameElTab,tV)==0)
										{
											sprintf(LogBuf,"Repl->CopyEl Name=%s",tV);
										}
										else
										{
											sprintf(LogBuf,"Repl->CopyEl ID=%I64d",tIDITr->ID_File);
										}
										Srv->OutLog(LogBuf,this,TableName);
									}
									rc=CopyIDI2El(tIDITr,tElT,0);
									if (rc!=-1)
									{
//										if (CompareChild(tIDITr,tElT))
										if (CompareChild(tIDITr,tElT))
											SaveEl(tElT);
// 										if (tIDITr->Cnt!=0)
// 										{
// 											int		tCntP=tIDITr->Cnt/sizeof(INT64);
// 											INT64*	tBufP=(INT64*)tIDITr->Buf;
// 											for (size_t i=0;i<tCntP;i++) // Çàïðîñ îñòàëüíûõ ýëåìåíòîâ òàáëèöû
// 											{
// 												if (tBufP[i])
// 												{
// 													if (tBufP[i])
// 													{
// 														if (tElT->AddChild(tBufP[i],1)==0)
// 														{
// 															tElTt=GetElTab(&TmpAnchor,tBufP[i]);
// 															UnLockEl(&TmpAnchor,tElTt);
// 														}
// 													}
// 												}
// 											}
// 										}
//										UpDataEl(tElT);
										if (tElT->OwnerList)
										{
											if (tElT->OwnerList->NumElem)
											{
												tElT->GetIDI()->Command=Command_MODIFY;
												tElT->GetIDI()->StateBlock=StateBlock_BEGIN|StateBlock_END|StateBlock_EMPTY;
												tElT->GetIDI()->Cnt=0;
//												MyUnLock(__FILE__,__LINE__,&Fl_RE,this);
												Srv->SendElTabl(tElT);
//												ODataIDI(2,tElT->GetIDI());
//												MyLock(__FILE__,__LINE__,&Fl_RE,this);
											}
										}
//										if (TestEl(tElT)!=-1) SendEl(tElT);
										if (TestEl(tElT)==-1) tElT=0;
									}
									UnLockEl(&TmpAnchor,tElT);
								}
								else
								{
									CompareChild(tIDITr,tElT);
								}
							}
							UnLockEl(&TmpAnchor,tElT);
							rc=0;
						}
						else
						{	
 							tElT=(El_Tab*)IDL->Find((UCHAR*)&tIDITr->ID_ParentFile,sizeof(INT64));
 							if (tElT)
 							{	// Äîáàâëÿåì 
//								MyUnLock(__FILE__,__LINE__,&Fl_RE,this);
								tElT2=CopyIDI2Parent(&TmpAnchor,tIDITr,tElT);
//								MyLock(__FILE__,__LINE__,&Fl_RE,this);
								if (TestEl(tElT2)==-1)
									tElT2=0;
								if (tElT2)
									CompareChild(tIDITr,tElT2);
// 
// 									if (tIDITr->Cnt!=0)
// 									{
// 										int		tCntP=tIDITr->Cnt/sizeof(INT64);
// 										INT64*	tBufP=(INT64*)tIDITr->Buf;
// 										for (size_t i=0;i<tCntP;i++) // Çàïðîñ îñòàëüíûõ ýëåìåíòîâ òàáëèöû
// 										{
// 											if (tBufP[i])
// 											{
// 												if (tElT2->AddChild(tBufP[i],1)==0)
// 												{
// 													tElTt=GetElTab(&TmpAnchor,tBufP[i]);
// 													UnLockEl(&TmpAnchor,tElTt);
// 												}
// 											}
// 										}
// 										SaveEl(tElT2);
// 									}
								UnLockEl(&TmpAnchor,tElT2);
								UnLockEl(&TmpAnchor,tElT);
								tElT->Fl_Query=0;
								ReplEl(tElT);
								SendEl(tElT,1);
							}
							else
							{
								QueryEl(tIDITr->ID_ParentFile);	// Çàïðîñèòü íåäîñòàþùèé ýëåìåíò
//								QueryEl(tIDITr->ID_File);			// Çàïðîñèòü ïîâòîð ýëåìåíòà
							}
						}
						break;
					}
					break;
				case TypeIDI_COMMAND:
					switch(tIDITr->Command)
					{
					case Command_C_GetIDI:
						if(tIDITr->pPList->GetParam(IDID,tVol)==-1)
							break;
						tID=*(INT64*)tVol;
// 						if (tID==0x172800019b059b81)
// 							tID=0x172800019b059b81;
						//if (tID==1)
						//	{size_t iasm=1;}
/*						if (BLDel.Find(tID))
						{
							El_Tab	*tElTbl;
							tElTbl=(El_Tab*)IDL->Find((UCHAR*)&tID,sizeof(INT64));
							if (tElTbl)
							{
								if (tElTbl->Fl_Del==0)
									DelElTab(&TmpAnchor,tElTbl);
								DelEl(tElTbl);
							}
							else
							{
								tElTbl=new El_Tab(&TmpAnchor,tID,this);
								{
//									if (tElTbl->GetIDI()->ID_File==0x1d2d0000519d47b1)
//									{
//										while(true) {Beep(1000,1000); Sleep(1000); }
//									}
									if (Fl_Debug)
									{
										sprintf(LogBuf,"ReDelEl ID=%I64x",tID);
										Srv->OutLog(LogBuf,this,"IDTableDel");
									}
									tElTbl->GetIDI()->Command=Command_DEL;
									tElTbl->GetIDI()->TypeIDI=TypeIDI_TABLE;
									tElTbl->GetIDI()->StateBlock=StateBlock_BEGIN|StateBlock_END;
									Srv->GetRealTime(&tElTbl->GetIDI()->TimeOper.FTime);
									tElTbl->GetIDI()->TimeClose.FTime=tElTbl->GetIDI()->TimeOper.FTime;
									tElTbl->GetIDI()->NumPack=1;
									strcpy(LastHost,"");
									tElTbl->GetIDI()->ID_SeansFrom=ID;
//									tElTbl->GetIDI()->pPList->SetParam(IDHostName,&LastHost,strlen(LastHost)+1,this);
									strcpy(tElTbl->GetIDI()->AdrInfo->DestinAI.HostName,LastHost);
//									MyUnLock(__FILE__,__LINE__,&Fl_RE,this);
									Srv->ReplElTabl(tElTbl);
//									ODataIDI(3,tElTbl->GetIDI());
//									MyLock(__FILE__,__LINE__,&Fl_RE,this);
								}
								delete tElTbl;
							}
							break;
						};
*/						
						if ((Srv->SInt->NumLANSrv==Srv->SInt->MyLANNum)||(Srv->SInt->NumLANSSrv==Srv->SInt->MyLANNum)||(strcmp(tIDITr->AdrInfo->DestinAI.HostName,Srv->ShortHostName)==0))
						{
							if (Fl_Debug)
							{
// 								if (tIDITr->pPList->GetParam(IDMyHostName,(void*&)tV)!=-1)
// 								{
// 									sprintf(LogBuf,"Comm_GetIDI ID=%I64x From=%s",tID,tV); 
// 								}
// 								else
// 								{
// 									sprintf(LogBuf,"Comm_GetIDI ID=%I64x",tID); 
// 								}
// 								Srv->OutLog(LogBuf,this,TableName);
							}
							tElT=GetElTab(&TmpAnchor,tID,1);
							if (tElT)
							{
								tElT->Fl_Query=0;
//								void*	tPrm;
//								size_t	tCnt;
								if (tIDITr->AdrInfo->NativeAI.HostName[0]!=0)
								{
//									tElT->GetIDI()->pPList->SetParam(IDHostName,tPrm,tCnt,this);
									strcpy(tElT->GetIDI()->AdrInfo->DestinAI.HostName,tIDITr->AdrInfo->NativeAI.HostName);
								}
								else
								{
//									tElT->GetIDI()->pPList->SetParam(IDHostName,"*",2,this);
									tElT->GetIDI()->AdrInfo->DestinAI.HostName[0]=0;
								}
								tElT->GetIDI()->ID_SeansFrom=ID;
								ReplEl(tElT);
//								tElT->GetIDI()->pPList->DelParam(IDHostName);	
								tElT->GetIDI()->AdrInfo->DestinAI.HostName[0]=0;
								UnLockEl(&TmpAnchor,tElT);
							}
							else
							{	// Re Query in other
								QueryEl(tID);
							}
						}
						break;

					case Command_C_GetIDIFromSrv:
						if(tIDITr->pPList->GetParam(IDID,tVol)==-1)
							break;
						tID=*(INT64*)tVol;
/*						if (BLDel.Find(tID))
						{
							El_Tab	*tElTbl;
							tElTbl=(El_Tab*)IDL->Find((UCHAR*)&tID,sizeof(INT64));
							if (tElTbl)
							{
								if (tElTbl->Fl_Del==0)
									DelElTab(&TmpAnchor,tElTbl);
								DelEl(tElTbl);
							}
							else
							{
								tElTbl=new El_Tab(&TmpAnchor,tID,this);
								{
//									if (tElTbl->GetIDI()->ID_File==0x1d2d0000519d47b1)
//									{
//										while(true) {Beep(1000,1000); Sleep(1000); }
//									}
									if (Fl_Debug)
									{
										sprintf(LogBuf,"ReDelEl ID=%I64x",tID);
										Srv->OutLog(LogBuf,this,"IDTableDel");
									}
									tElTbl->GetIDI()->Command=Command_DEL;
									tElTbl->GetIDI()->TypeIDI=TypeIDI_TABLE;
									tElTbl->GetIDI()->StateBlock=StateBlock_BEGIN|StateBlock_END;
									Srv->GetRealTime(&tElTbl->GetIDI()->TimeOper.FTime);
									tElTbl->GetIDI()->TimeClose.FTime=tElTbl->GetIDI()->TimeOper.FTime;
									tElTbl->GetIDI()->NumPack=1;
									strcpy(LastHost,"");
									tElTbl->GetIDI()->ID_SeansFrom=ID;
//									tElTbl->GetIDI()->pPList->SetParam(IDHostName,&LastHost,strlen(LastHost)+1,this);
									strcpy(tElTbl->GetIDI()->AdrInfo->DestinAI.HostName,LastHost);
//									MyUnLock(__FILE__,__LINE__,&Fl_RE,this);
									Srv->ReplElTabl(tElTbl);
//									ODataIDI(3,tElTbl->GetIDI());
//									MyLock(__FILE__,__LINE__,&Fl_RE,this);
								}
								delete tElTbl;
							}
							break;
						};
*/				
						if (Fl_Debug)
						{
// 							if (tIDITr->pPList->GetParam(IDMyHostName,(void*&)tV)!=-1)
// 							{
// 								sprintf(LogBuf,"Comm_GetIDI ID=%I64x From=%s",tID,tV); 
// 							}
// 							else
// 							{
// 								sprintf(LogBuf,"Comm_GetIDI ID=%I64x",tID); 
// 							}
// 							Srv->OutLog(LogBuf,this,TableName);
						}
						if (Srv->SInt->NumLANSrv!=Srv->SInt->MyLANNum)
						{
							El_Tab	*tElT3;
							tElT3=(El_Tab*)IDL->Find((UCHAR*)&tID,sizeof(INT64));
							tElT=GetElTab(&TmpAnchor,tID,1);
							if (tElT)
							{
//								void*	tPrm;
//								size_t	tCnt;
								tElT->Fl_Query=0;
								if (tIDITr->AdrInfo->DestinAI.HostName[0]!=0)
								{
//									tElT->GetIDI()->pPList->SetParam(IDHostName,tPrm,tCnt,this);
									strcpy(tElT->GetIDI()->AdrInfo->DestinAI.HostName,tIDITr->AdrInfo->DestinAI.HostName);
								}
								else	
								{
//									char	BBB[10];strcpy(BBB,"");
//									tElT->GetIDI()->pPList->SetParam(IDHostName,BBB,2,this);
									tElT->GetIDI()->AdrInfo->DestinAI.HostName[0]=0;
								}
								tElT->GetIDI()->ID_SeansFrom=ID;
								if (tElT3==NULL) ReplEl(tElT);
//								tElT->GetIDI()->pPList->DelParam(IDHostName);	
								tElT->GetIDI()->AdrInfo->DestinAI.HostName[0]=0;
								UnLockEl(&TmpAnchor,tElT);
							}
							else
							{	// Re Query in other
								QueryEl(tID);
							}
						}
						break;
					}
					break;
				default:
					Srv->SetCriticalError(this,"Unknown Type IDInfo.StateFile","Warning","MReplicator");
					break;
				}
			}
			else
			{
				Srv->SetCriticalError(this,"Wrong Data Size","Warning","MReplicator");
			}
			strcpy(LastHost,"*");
			break;
		}
	}
	return OCnt;
};

bool	IDTable::CompareChild(IDInfo* IDI1,El_Tab* tElTab)
{
	bool	rc=false;
	IDInfo* IDI2;
	El_Tab*		tElTt;
	IDI2=tElTab->GetIDI();
	if	(IDI1->ID_Standart!=IDI2->ID_Standart)
	{
		rc=true;
		if(IDI1->ID_Standart>IDI2->ID_Standart)
		{
			if (IDI1->Cnt!=0)
			{
				int		tCntP=IDI1->Cnt/sizeof(INT64);
				INT64*	tBufP=(INT64*)IDI1->Buf;
				for (size_t i=0;i<tCntP;i++) // Çàïðîñ îñòàëüíûõ ýëåìåíòîâ òàáëèöû
				{
					if (tBufP[i])
					{
						if (tElTab->AddChild(tBufP[i],1)==0)
						{
							tElTt=GetElTab(&TmpAnchor,tBufP[i]);
							if (tElTt)
							{
								if (tElTt->pParent->GetIDI()->ID_File!=tElTab->GetIDI()->ID_File)
								{
									QueryEl(tElTt->GetIDI()->ID_File);
									//								tElT=MoveET(&TmpAnchor,tElTt,tElTab);
								}
								UnLockEl(&TmpAnchor,tElTt);
							}
							else
								QueryEl(tBufP[i]);
						}
					}
				}
			}
			//											ReplEl(tElT);
		}
		else
		{	// Алгоритм "обратной тяги"
			ID_List8	TempList;
			if (IDI1->Cnt!=0)
			{
				int		tCntP=IDI1->Cnt/sizeof(INT64);
				INT64*	tBufP=(INT64*)IDI1->Buf;
				for (size_t i=0;i<tCntP;i++) // Çàïðîñ îñòàëüíûõ ýëåìåíòîâ òàáëèöû
				{
					if (tBufP[i])
					{
						ID_Elem64*		ElK=(ID_Elem64*)TempList.Find((UCHAR*)&tBufP[i],sizeof(INT64));
						if (ElK==NULL)
						{
							ElK=new ID_Elem64(tBufP[i]);
							if (TempList.Add(ElK)==-1)
								delete ElK;
						}
					}
				}
			}
			ID_Elem64*	tEl64;
			FindStrList8	FSL82;
			tEl64=(ID_Elem64*)tElTab->IDL64->Begin(FSL82);
			while (tEl64)
			{
				ID_Elem* TmpEl=TempList.Find((UCHAR*)&tEl64->Vol,sizeof(INT64));
				if (TmpEl==NULL)
				{
					El_Tab* TTTEl=GetElTab(&TmpAnchor,tEl64->Vol);
					if (TTTEl)
					{
						TTTEl->Fl_Query=0;
						ReplEl(TTTEl);
						UnLockEl(&TmpAnchor,TTTEl);
					}
				}
				tEl64=(ID_Elem64*)tElTab->IDL64->Next(FSL82);
			}
			//											Find(tElem->Buff,tElem->Len,1);
			FSL82.DestroyStr();
			TempList.DelAllElem();
			tElTab->Fl_Query=0;
			ReplEl(tElTab);
		}
	}
	return rc;
};
int	IDTable::IdleFunc(int NumberTr)
{
// 	IDInfo*		tIDITr;
// 	void*	tVol;
// 	char*	tV;
// 	INT64	tID;
// 	//	INT64	tID2;
// 	int	rc=0;
// 	El_Tab*		tElT;
// 	El_Tab*		tElTt;
// 	El_Tab*		tElT2;
// 	//	El_Tab*		tElT3;
// 	ID_Elem64*	tEl;
// 	LastHost[0]=0;
// 	FileTime	RealT;
// 	char		NameEl[128];
	int rc=ChildIdleFunc(NumberTr);
	if ((Fl_RR!=1)&&(Srv->System_On)&&(Srv->Fl_ConnectSrv))
	{
		if ((IDL_Test)&&(IDL_Test->NumElem))
		{
			Fl_RR=1;
			int CntMax=0;
			INT64	tID;
			while (IDL_Test->NumElem!=0)
			{
				tID=IDL_Test->GetID();
				if (tID)
				{
					ReplElI(tID);
				};
				if (CntMax++>20)
					break;
			};
			Fl_RR=0;
		}
	}
	int Cnt_Try=0;
	while (GetNumQueryList())
	{
		ID_Elem64*	tEl=(ID_Elem64*)IDL_Query->Next(FSL_8);
		if (tEl==NULL)
			tEl=(ID_Elem64*)IDL_Query->Begin(FSL_8);
		if (tEl)
		{
			QueryElI(tEl->Vol);
			DelFromQueryList(tEl->Vol);
		};
		if (Cnt_Try++>20)
			break;
	}
	FSL_8.DestroyStr();
	//		BLDel.DelOnTime(1*24*60*60,this);
	if ((Srv->System_On)&&(ID!=TabParametr))
//	if (FALSE)
	{
		if (BDL_Cnt++>100)
		{
			ElTblDel=FindNested(&TmpAnchor,AcsMd_Read,"DelList",NULL);
// 			if (ElTblDel==NULL)
// 			{
// 				ElTblDel=FindNested(&TmpAnchor,AcsMd_Read,"DelList",NULL);
// 				// 			if ((ElTbl)&&(ID!=TabParametr))
// 				// 				ElTblDel=AddETI(&TmpAnchor,"DelList",ElTbl);
// 			}
			if ((ElTblDel))
			{
// 				El_Tab*		tElRoot=FindNextET(&TmpAnchor,&FStr_Root,AcsMd_Read,ElTbl);
// 				if (tElRoot==NULL)
// 					tElRoot=FindFirstET(&TmpAnchor,&FStr_Root,AcsMd_Read,ElTbl);
// 				if (tElRoot)
// 					ReplEl(tElRoot);
// 				UnLockEl(&TmpAnchor,tElRoot);
				char		ttBf[80];
				FindStruct	tFStr;
				El_Tab*		tElDel;
				El_Tab*		tElDel2;
				El_Tab*		tElT;
				El_Tab*		ElTblForNew2;
				ElTblForNew=FindNested(&TmpAnchor,AcsMd_Read,N_Domain,Srv->MainDomainName,N_Host,Srv->ShortHostName,"DelList",NULL);
				if (ElTblForNew==NULL)
				{
//					ElTblForNew=FindNested(&TmpAnchor,AcsMd_Read,N_Domain,Srv->MainDomainName,N_Host,Srv->ShortHostName,"DelList",NULL);
					if (ID!=TabParametr)
						ElTblForNew=AttainNested(&TmpAnchor,"%s%s%s%s%s",N_Domain,Srv->MainDomainName,N_Host,Srv->ShortHostName,"DelList",-1);
				}
				if (ElTblForNew)
				{
					if ((ElTblForNew->IDL64==NULL)||(ElTblForNew->IDL64->NumElem<1000))
					{
						tElDel=FindFirstET(&Anchor,&tFStr,AcsMd_Read,ElTblDel);
						if (tElDel)
						{
							while (((tElDel)&&(tElDel->IDL64)&&(tElDel->IDL64->NumElem)))
							{
								tElDel2=FindFirstET(&Anchor,&tFStr,AcsMd_Read,tElDel);
								UnLockEl(&Anchor,tElDel);
								if (tElDel==tElDel2)
									break;
								tElDel=tElDel2;
							}
							FileTime	tempTime;
							Srv->GetRealTime(&tempTime.FTime);
							//&&(tempTime.Time-tElDel->GetIDI()->TimeBegin.Time>KtimeH))
							if (tElDel)
							{
								//							tElDel->Init(&TmpAnchor,tElDel->GetIDI()->ID_File,this);
								// 							tElDel->GetIDI()->pPList->DelAllElem();
								// 							sprintf(ttBf,"%I64x",tElDel->GetIDI()->ID_File);
								// 							AddParam(&Anchor,tElDel,IDNameElTab,ttBf,1);
								// 							Srv->GetRealTime(&tElDel->GetIDI()->TimeBegin.FTime);
								tElT=MoveET(&TmpAnchor,tElDel,ElTblForNew);
								tElT->GetIDI()->pPList->DelAllElem();
								sprintf(ttBf,"%I64x",tElT->GetIDI()->ID_File);
								AddParam(&TmpAnchor,tElT,IDNameElTab,ttBf,1);
								Srv->GetRealTime(&tElT->GetIDI()->TimeBegin.FTime);
								ModifElTab(&TmpAnchor,tElT);

								if (((ElTblDel->IDL64)&&(ElTblDel->IDL64->NumElem==0))||((ElTblForNew)&&(ElTblForNew->IDL64->NumElem==999)))
								{
									ModifElTab(&TmpAnchor,ElTblForNew);
								}
								//							ModifElTab(&TmpAnchor,ElTblForNew);

								UnLockEl(&Anchor,tElT);
							}
						}
						else
						{
							UnLockEl(&Anchor,ElTblDel);
							ElTblDel=NULL;
						}
					}
					else
					{
						ElTblForNew2=FindNested(&TmpAnchor,AcsMd_Read,N_Domain,Srv->MainDomainName,N_Host,Srv->ShortHostName,"ForNewList",NULL);
						if (ElTblForNew2)
						{
							if ((ElTblForNew2->IDL64==NULL)||(ElTblForNew2->IDL64->NumElem==0))
							{
								DelElTab(&TmpAnchor,ElTblForNew2);
								AddParam(&TmpAnchor,ElTblForNew,IDNameElTab,"ForNewList");
							}
						}
						else
						{
							AddParam(&TmpAnchor,ElTblForNew,IDNameElTab,"ForNewList");
						}
					}		
				}
			}
			BDL_Cnt=0;
		}
	}
	if (Fl_Long)
	{
		//				MyUnLock(__FILE__,__LINE__,&Fl_RE,this);
		ODataFF(1,NULL,1);
		//				MyLock(__FILE__,__LINE__,&Fl_RE,this);
	}
	//if ((ID!=2)&&(Srv->Timer(DelTimer,100)))
	if ((ID!=TabParametr)&&(Srv->Timer(DelTimer,500)) && (IDL))
	{
		El_Tab*	tElScan=NULL;
		char	ttBufD[40];
		sprintf(ttBufD,"NumQueryElTab%I64d",ID);
		Srv->Telemetr(this,ttBufD,(double)GetNumQueryList(),(void*)this);
		sprintf(ttBufD,"NumDelElTab%I64d",ID);
		Srv->Telemetr(this,ttBufD,(double)BLDel.NumElem,(void*)this);
		sprintf(ttBufD,"NumActElTab%I64d",ID);
		Srv->Telemetr(this,ttBufD,(double)IDL->NumElem,(void*)this);
		FindStrList8	FSL8;
		IDL->Find((UCHAR*)&Scan_Index,sizeof(INT64),FSL8);
		int		TryCnt=0;
		tElScan=(El_Tab*)IDL->Next(FSL8);
		if (tElScan==NULL)
		{
			tElScan=(El_Tab*)IDL->Begin(FSL8);
		};
_mmm1:
		if (tElScan)
		{
			FileTime	OperTime;
			INT64	DeltaT;
			Scan_Index=tElScan->GetIDI()->ID_File;
			tElScan->PreSave();
			if (ID==1)
			{
				ID=1;
			}
			if ((tElScan->OwnerList==NULL)||(tElScan->OwnerList->NumElem==0))
			{
//				if (IDL->NumElem>25000-150*Srv->MemUsage)
//				if (IDL->NumElem>15000-150*Srv->MemUsage)
				INT64		TimeInt;
				TimeInt=100*(INT64)KtimeM/(Srv->MemUsage*Srv->MemUsage);
//				if (Srv->MemUsage>70)
				{
					Srv->GetRealTime(&OperTime.FTime);
					DeltaT=OperTime.Time-tElScan->GetIDI()->TimeOper.Time;
					if (DeltaT>TimeInt)		// KtimeH
					{
						if (ElTbl!=tElScan)
						{
							if (tElScan->IDL64)
							{
								if (tElScan->IDL64->NumElem==0)
								{
									tElScan->Fl_Del=1;
									IDL->Del(tElScan);
								}
								else
								{
									int FlDel=0;
									ID_Elem64*	tEl64;
									FindStrList8	FSL82;
									tEl64=(ID_Elem64*)tElScan->IDL64->Begin(FSL82);
									while (tEl64)
									{
										FlDel|=(int)IDL->Find((UCHAR*)&tEl64->Vol,sizeof(INT64)/*,1*/);
										if (FlDel)
											break;
										tEl64=(ID_Elem64*)tElScan->IDL64->Next(FSL82);
									}
									//											Find(tElem->Buff,tElem->Len,1);
									FSL82.DestroyStr();
									if (FlDel==0)
									{
										//											tElScan->PreSave();
										tElScan->Fl_Del=1;
										IDL->Del(tElScan);
									}
								};
							}
							else
							{
								//									tElScan->PreSave();
								tElScan->Fl_Del=1;
								IDL->Del(tElScan);
							}
						}

					}
					tElScan=(El_Tab*)IDL->Next(FSL8);
					TryCnt++;
//					TryCnt+=1;
				} 
/*				else
				{
					Srv->GetRealTime(&OperTime.FTime);
					DeltaT=OperTime.Time-tElScan->GetIDI()->TimeOper.Time;
					if (DeltaT>(10*(INT64)KtimeM))		// KtimeH
					{
						if (ElTbl!=tElScan)
						{
							if (tElScan->IDL64)
							{
								if (tElScan->IDL64->NumElem==0)
								{
									tElScan->Fl_Del=1;
									IDL->Del(tElScan);
								}
								else
								{
									size_t FlDel=0;
									FindStrList8	FSL82;
									ID_Elem64*	tEl64;
									tEl64=(ID_Elem64*)tElScan->IDL64->Begin(FSL82);
									while (tEl64)
									{
										FlDel|=(size_t)IDL->Find((UCHAR*)&tEl64->Vol,sizeof(INT64));
										if (FlDel)
											break;
										tEl64=(ID_Elem64*)tElScan->IDL64->Next(FSL82);
									}
									FSL82.DestroyStr();
									if (FlDel==0)
									{
										//											tElScan->PreSave();
										tElScan->Fl_Del=1;
										IDL->Del(tElScan);
									}
								};
							}
							else
							{
								//									tElScan->PreSave();
								tElScan->Fl_Del=1;
								IDL->Del(tElScan);
							}
						}
					}
					tElScan=(El_Tab*)IDL->Next(FSL8);
					TryCnt+=5;
				}
*/
			}
			else
			{
				Srv->GetRealTime(&OperTime.FTime);
				DeltaT=OperTime.Time-tElScan->GetIDI()->TimeOper.Time;
//				if (DeltaT>(10*(INT64)KtimeM))		// KtimeH
				if (DeltaT>(5*(INT64)KtimeM))		// KtimeH
				{
					El_Owner*	tDec;
					//							MyLock(__FILE__,__LINE__,&tElScan->Fl_RE,this);
					FindStrList8	tFSL8;
					tDec=(El_Owner*)tElScan->OwnerList->Begin(tFSL8);
					if (tDec)
					{
						if (IsBadReadPtr(tDec->Own.pDec,sizeof(Decoder)))
						{
							tElScan->DelOwner(&tDec->Own);	
						}
						else
						{
							if (IsBadReadPtr(tDec->Own.pDec->Name_Mt,1))
							{
								tElScan->DelOwner(&tDec->Own);	
							}
							else
							{
								if(strcmp(tDec->Own.pDec->Name_Mt,TableName)==0)
								{
									UnLockEl(&TmpAnchor,tElScan,1);
									if (tElScan->OwnerList->NumElem==0)
										goto	_mmm1;
								}
								if(strcmp(tDec->Own.pDec->Name_Mt,tDec->NameMt)!=0)
								{
									tElScan->DelOwner(&tDec->Own);	
								}
							}
						}
					}
					//							MyUnLock(__FILE__,__LINE__,&tElScan->Fl_RE,this);
				}
//				UnLockEl(&TmpAnchor,tElScan);
				tElScan=(El_Tab*)IDL->Next(FSL8);
				TryCnt+=10;
			}
			if (tElScan)
			{
				if (TryCnt<100)
					goto	_mmm1;
			}
		}
		if (pFSrv) pFSrv->DataFFEx(1,(uchar*)&Fl_RR,1);
		Srv->Timer(DelTimer,500);
	}
	return rc;
};

int	IDTable::DataFF(int N_Pin,UCHAR* pmas,size_t Cnt)
{
	int	OCnt=0;
	IDInfo*		tIDITr;
//	void*	tVol;
//	char*	tV;
//	INT64	tID;
//	INT64	tID2;
	int	rc=0;
//	El_Tab*		tElT;
//	El_Tab*		tElTt;
//	El_Tab*		tElT2;
//	El_Tab*		tElT3;
//	ID_Elem64*	tEl;
	LastHost[0]=0;
//	FileTime	RealT;
//	char		NameEl[128];
//	MyLock(__FILE__,__LINE__,&Fl_RE,this);
	if (Fl_Stop!=1)
	{
		switch(N_Pin)
		{
		case 0:
			if (Cnt==sizeof(IDInfo))
			{
				tIDITr=(IDInfo*)pmas;
				DataIDI(N_Pin,tIDITr);
			}
			break;
		case 1:
			break;
		case 2:	// Âõîä îò ðåïëèêàòîðà!
			LastHost[0]=0;
			if ((Cnt==sizeof(IDInfo))&&(Srv->Fl_Repl))
			{
				tIDITr=(IDInfo*)pmas;
				DataIDI(N_Pin,tIDITr);
			}
			else
			{
				Srv->SetCriticalError(this,"Wrong Data Size","Warning","MReplicator");
			}
			strcpy(LastHost,"*");
			break;
		}
	};
//	MyUnLock(__FILE__,__LINE__,&Fl_RE,this);
	return OCnt;
};

int	IDTable::QueryElI(INT64 tID)
{
	int				rc=0;
	IDInfo*			QIDI;
	QIDI=Srv->GetNewIDI(this);
	QIDI->AdrInfo=&AdrI;
	QIDI->TypeIDI=TypeIDI_COMMAND;
	QIDI->Command=Command_C_GetIDI;
	QIDI->StateBlock=StateBlock_BEGIN|StateBlock_END|StateBlock_EMPTY;
	if ((QIDI->SeansFrom==NULL)&&(ElTbl))
		QIDI->SeansFrom=ElTbl->GetIDI();
	QIDI->ID_File=Srv->GetUID();
	Srv->GetRealTime(&QIDI->TimeBegin.FTime);
	QIDI->TimeOper.FTime=QIDI->TimeBegin.FTime;
	QIDI->TimeLastWr.FTime=QIDI->TimeBegin.FTime;
	QIDI->TimeClose.FTime=QIDI->TimeBegin.FTime;
	QIDI->pPList->SetParam(IDID,&tID,sizeof(INT64),this);
	QIDI->TypeIDI=TypeIDI_COMMAND;
//	if (Srv->SInt->NumLANSrv!=Srv->SInt->MyLANNum)
	{
		QIDI->Command=Command_C_GetIDI;
		if (Srv->SInt->NumLANSrv)
		{
			El_CliInt*	Cli=(El_CliInt*)Srv->SInt->CliLAN.Find((UCHAR*)&Srv->SInt->NumLANSrv,sizeof(ULONG));
			if (Cli)
			{
				strcpy(LastHost,Cli->Cli.ShortHost);
				if (strcmp(LastHost,Srv->ShortHostName)==0)
					strcpy(LastHost,"*");
			}
			else
			{
				strcpy(LastHost,"*");
			}
		}
		else
		{
			strcpy(LastHost,"*");
		}

// 		if ((Srv->SInt->NumLANSrv==0)||(Srv->SInt->CliLAN[Srv->SInt->NumLANSrv]==NULL))
// 			strcpy(LastHost,"*");
// 		else
// 			strcpy(LastHost,Srv->SInt->CliLAN[Srv->SInt->NumLANSrv]->ShortHost);
//		QIDI->pPList->SetParam(IDHostName,&LastHost,strlen(LastHost)+1,this);
		strcpy(QIDI->AdrInfo->DestinAI.HostName,LastHost);
	}
// 	else
// 	{
// 		QIDI->Command=Command_C_GetIDIFromSrv;
// 		if (LastHost[0]==0)
// 		{
// 			strcpy(LastHost,"*");
// 		}
// //		QIDI->pPList->SetParam(IDHostName,&LastHost,strlen(LastHost)+1,this);
// 		strcpy(QIDI->AdrInfo->DestinAI.HostName,LastHost);
// 	}
	QIDI->StateBlock=StateBlock_BEGIN|StateBlock_END|StateBlock_EMPTY;
	QIDI->NumPack=1;
	QIDI->ID_SeansFrom=ID;
	QIDI->CustomerID=CustomerID;
	ODataIDI(3,QIDI);
	if (Fl_Debug)
	{
		sprintf(LogBuf,"QueryEl to Host=%s ID=%I64x",LastHost,tID); Srv->OutLog(LogBuf,this,TableName);
	}
	if (QIDI) Srv->DeleteIDI(QIDI); QIDI=NULL;
	return	rc;
}

int	IDTable::QueryEl(INT64 tID)
{
	int rc=0;
	if (BLDel.Find(tID))
	{
		if (Fl_Debug)
		{
			sprintf(LogBuf,"Skip->QueryEl DelEl ID=%I64x",tID); Srv->OutLog(LogBuf,this,TableName);
		}
	}
	else
	{
		ID_Elem64*	tEl=(ID_Elem64*)IDL_Query->Find((UCHAR*)&tID,sizeof(INT64));
		if (tEl==NULL)
		{
			if (Srv->Fl_Repl==1)
			{
				tEl=(ID_Elem64*)IDL_Query->NewElem();
				if (tEl)
					tEl->Init(tID);
				else
					tEl=new ID_Elem64(tID);
//				if ((int)tEl>0x0ff00000)
//					{size_t iasm=1;}
				if (tEl)
					rc=(int)IDL_Query->Add(tEl);
				if (rc==-1)
				{
					delete tEl;
//					tEl=(ID_Elem64*)IDL_Query->Find((UCHAR*)&tID,sizeof(INT64));
				}
			}
		}
		else
		{
			if (Fl_Debug)
			{
				sprintf(LogBuf,"Skip->QueryEl ID=%I64x",tID); Srv->OutLog(LogBuf,this,TableName);
			}
//			if (tFT.Time-tEl->FT.Time>5*KtimeS)
//			{
//				DelFromQueryList(tID);
//				//			IDL_Query->Del(tEl);
//				goto _mm22;
//			}
			rc=-1;
		}

	}
	return rc;
};
/*
int	IDTable::ReplEl(El_Tab* ElT)
{
	int rc=0;
	char*			tVol;
	ID_Elem64*		tElT=0;
//	El_Tab*			ElT;
	MyLock(__FILE__,__LINE__,&Fl_RE,this);
//	ElT=GetElTab(&Anchor,tID);
	if ((ElT)&&((ElT->GetIDI()->ID_SeansFrom!=0)))	//||(ElT->GetIDI()->ID_SeansFrom!=TabParametr)
	{
		if (!BLDel.Find(ElT->GetIDI()->ID_File))
		{
			if (MoLockEl(&Anchor,ElT,AcsMd_Write)!=-1)
			{
				if (ElT->IDL64)
				{
					tElT=(ID_Elem64*)ElT->IDL64->Begin();
					ElT->GetIDI()->ID_Standart=ElT->IDL64->NumElem;
				}
				else
				{
					ElT->GetIDI()->ID_Standart=0;
				}
				CntChild=0;
				ElT->GetIDI()->Command=Command_MODIFY;
				ElT->GetIDI()->TypeIDI=TypeIDI_TABLE;
				ElT->GetIDI()->StateBlock=StateBlock_BEGIN;
				ElT->GetIDI()->NumPack=0;
				if (LastHost[0]==0)
					strcpy(LastHost,"*");
				ElT->GetIDI()->pPList->SetParam(IDHostName,&LastHost,strlen(LastHost)+1,this);
				Srv->GetRealTime(&ElT->GetIDI()->TimeOper.FTime);
				ElT->GetIDI()->pPList->DelParam(IDDomainName);
				while (tElT!=NULL)
				{
					El_Tab*	tEl;
					ElT->GetIDI()->Buf=(UCHAR*)Child;
					while ((tElT!=NULL)&&(CntChild<SizeChildBuf))
					{
						tEl=GetElTab(&TmpAnchor,tElT->Vol,1);
						if (tEl)
						{
							if (tEl->pParent!=ElT)
							{
								ElT->DelChild(tElT->Vol);
							}
							else
							{
								Child[CntChild++]=tElT->Vol;
							}
							UnLockEl(&TmpAnchor,tEl);
						}
						else
						{
							ElT->DelChild(tElT->Vol);
						};

						if (ElT->IDL64)
						{
//								tElT=(ID_Elem64*)ElT->IDL64->Find((UCHAR*)&tElT->Vol,sizeof(INT64),1);
//								tElT=(ID_Elem64*)ElT->IDL64->Next();
							tElT=(ID_Elem64*)ElT->IDL64->Next(tElT);
						}
					}
					ElT->GetIDI()->Cnt=CntChild*sizeof(INT64);
					if ((ElT->GetIDI()->Cnt!=0)&&(tElT!=NULL)) 
					{
						ElT->GetIDI()->NumPack++;
//						ODataFF(3,(UCHAR*)ElT->GetIDI(),sizeof(IDInfo));
						BufODataFF(3,ElT->GetIDI());
						ElT->GetIDI()->StateBlock=0;
					}
					CntChild=0;
				}
				ElT->GetIDI()->StateBlock|=StateBlock_END;
				Srv->GetRealTime(&ElT->GetIDI()->TimeOper.FTime);
				ElT->GetIDI()->NumPack++;
				ElT->GetIDI()->ID_SeansFrom=ID;
//				ODataFF(3,(UCHAR*)ElT->GetIDI(),sizeof(IDInfo));
				BufODataFF(3,ElT->GetIDI());
				if (Fl_Debug)
				{
					if (GetParam(&TmpAnchor,ElT,IDNameElTab,tVol)==0)
					{
						sprintf(LogBuf,"ReplEl to %s ID=%I64x Name=%s",LastHost,ElT->GetIDI()->ID_File,tVol);
					}
					else
					{
						sprintf(LogBuf,"ReplEl to %s ID=%I64x",LastHost,ElT->GetIDI()->ID_File);
					}
					Srv->OutLog(LogBuf,this,TableName);
				}
			}
			else
			{
				Srv->SetCriticalError(this,"Error LockEl Write in ReplEl","Error",TableName);
			};
			UnLockEl(&Anchor,ElT);
		}
		else
		{
			DelEl(ElT);
		}
	}
	MyUnLock(__FILE__,__LINE__,&Fl_RE,this);
	return rc;
};
*/
/*
int	IDTable::ReplEl(El_Tab* ElT)
{
	int rc=0;
// 
// 	MyLock(__FILE__,__LINE__,&Fl_RE4,this);
	INT64	tID=ElT->GetIDI()->ID_File;
	if (BLDel.Find(tID))
	{
		if (Fl_Debug)
		{
			sprintf(LogBuf,"Skip->ReplEl DelEl ID=%I64x",tID); Srv->OutLog(LogBuf,this,TableName);
		}
	}
	else
	{
		IDL_Repl->AddID(tID);
	}
// 	MyUnLock(__FILE__,__LINE__,&Fl_RE4,this);
	return rc;
};
*/
// 
// int	IDTable::TestEl(El_Tab* ElT)
// {
// 	int rc=0;
// 	// 
// 	if (ElT)
// 	{
// 		if (ElT->Fl_Test==0)
// 		{
// 			INT64	tID=ElT->GetIDI()->ID_File;
// 			ElT->Fl_Test=1;
// 			IDL_Test->AddID(tID);
// 		}
// 	}
// 	return rc;
// };
// 
//int	IDTable::TestElI(INT64 ID)
int	IDTable::TestEl(El_Tab* ElT)
{
	int rc=0;
	char*	tNameEl;
	El_Tab* tElT2;
	//	El_Tab* tElT3;
	char*	tNameEl2;
	ID_Elem64*	tID;
	//	INT64		IDName;
	//	void*		pVol;
	//	int			pCnt;
	Decoder*	ParentMt=NULL;
	char*		tVol;
//	El_Tab* ElT=GetElTab(&TmpAnchor,ID);
	if ((ElT)&&(ElT->Fl_Test==0))
	{
		ElT->Fl_Test=1;
		try
		{
			if (Fl_Debug)
			{
				if (GetParam(&TmpAnchor,ElT,IDNameElTab,tVol)==0)
					sprintf(LogBuf,"TestEl ID=%I64x Name=%s",ElT->GetIDI()->ID_File,tVol);
				else
					sprintf(LogBuf,"TestEl ID=%I64x",ElT->GetIDI()->ID_File);
				Srv->OutLog(LogBuf,this,TableName);
			}
			if(ElT->pParent)
			{
				if (GetParam(&TmpAnchor,ElT,IDNameElTab,tNameEl)!=-1)
				{
					FindStrList8	FSL8;
					if (ElT->pParent->IDL64)
						tID=(ID_Elem64*)ElT->pParent->IDL64->Begin(FSL8);
					else
						tID=NULL;
					INT64	ttID;
					while(tID)
					{
						ttID=ElT->GetIDI(&TmpAnchor)->ID_File;
						if (tID->Vol!=ttID)	// Ñåáÿ íå ñìîòðèì
						{
							tElT2=(El_Tab*)IDL->Find((UCHAR*)&tID->Vol,sizeof(INT64));
							//						tElT2=GetElTab(&TmpAnchor,tID->Vol);
							if ((tElT2)&&(tElT2->Fl_Del==0))
							{
								//							tElT2=GetElTab(&TmpAnchor,tID->Vol);
								LockEl(&TmpAnchor,tElT2);
								if (tElT2==NULL)
								{
									tElT2=(El_Tab*)IDL->Find((UCHAR*)&tID->Vol,sizeof(INT64));
								}
								if ((tElT2->GetIDI(&TmpAnchor)->ID_File==tElT2->GetIDI()->ID_ParentFile)||((tElT2->GetIDI()->ParentFile)&&(tElT2->GetIDI()->ID_File==tElT2->GetIDI()->ParentFile->ID_File)))
								{
//									if (tElT2->Fl_Del==0)
										DelElTab(&TmpAnchor,tElT2);
//									else
//										DelEl(tElT2);
								}
								if (tElT2->pParent!=ElT->pParent)
								{
									ElT->pParent->DelChild(tID->Vol);
								}
								else
								{
									if (GetParam(&TmpAnchor,tElT2,IDNameElTab,tNameEl2)!=-1)
									{
										if (strcmp(tNameEl,tNameEl2)==0)
										{	// Íàéäåíû îäèíàêîâûå!
											//										if (ElT->GetIDI()->TimeLastWr.Time>tElT2->GetIDI()->TimeLastWr.Time)
											//										if (ElT->GetIDI()->ID_File<tElT2->GetIDI()->ID_File)
											//										if (TestNewest(ElT->GetIDI(),tElT2->GetIDI()))
// 											if ((strcmp(tNameEl,"DelList")==0)||(ID==TabParametr))
// 											{
// 												if (ElT->GetIDI()->TimeBegin.Time>tElT2->GetIDI()->TimeBegin.Time)
// 													Combine(ElT,tElT2);
// 												else
// 												{
// 													Combine(tElT2,ElT);
// 													rc=-1;
// 												}
// 											}
// 											else
// 											{
 												if (ElT->GetIDI()->TimeBegin.Time<=tElT2->GetIDI()->TimeBegin.Time)
 													Combine(ElT,tElT2);
 												else
 												{
 													Combine(tElT2,ElT);
													tElT2->Fl_Query=0;
													ReplEl(tElT2);
 													rc=-1;
 												}
// 											}
										}
									}
								}
								UnLockEl(&TmpAnchor,tElT2);
							}
							else
							{
								//							ElT->pParent->DelChild(tID->Vol);
							}
						}
						if (rc==-1)
							break;
						else
						{
//							ElT->pParent->IDL64->Find((UCHAR*)&tID->Vol,sizeof(INT64),1);
							tID=(ID_Elem64*)ElT->pParent->IDL64->Next(FSL8);
						}
					}
				}
			}
		}
		catch (...)
		{
			rc=-1;	
		}
		UnLockEl(&TmpAnchor,ElT);
	}
	return rc;
};

int	IDTable::ReplEl(El_Tab* ElT)
{
	int rc=0;
	if (ID==TabParametr) return rc;
	if (ElT)
	{
		INT64	tID;
		if (ElT->Fl_Query==0)
		{
			ElT->Fl_Query=1;
			Srv->GetRealTime(&ElT->QueryTime.FTime);
			tID=ElT->GetIDI()->ID_File;
			IDL_Test->AddID(tID);
		}
		else
		{
 			FileTime	TmpTime;
 			Srv->GetRealTime(&TmpTime.FTime);
			if ((Srv->System_On==0)||(Srv->Fl_ConnectSrv==0))		// !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!11
			{
				tID=ElT->GetIDI()->ID_File;
				IDL_Test->AddID(tID);
				ElT->Fl_Query=0;
			}
 			if (TmpTime.Time-ElT->QueryTime.Time>Ktime5M)
 			{
				Srv->GetRealTime(&ElT->QueryTime.FTime);
 				tID=ElT->GetIDI()->ID_File;
 				IDL_Test->AddID(tID);
 			}
		}
	}
	return	rc;
};

int	IDTable::ReplElI(INT64 tID)
{
	int rc=0;
	char*			tVol;
	ID_Elem64*		tElT=0;
	El_Tab*			ElT;
	int		FlCntCh=0;
	if (ID==TabParametr) return rc;
	ElT=GetElTab(&Anchor,tID,1);
	if ((ElT)&&((ElT->GetIDI()->ID_SeansFrom!=0)))
	{
//		if (!BLDel.Find(ElT->GetIDI()->ID_File))
		{
			if (MoLockEl(&Anchor,ElT,AcsMd_Write)!=-1)
			{
				FindStrList8	FSL8;
				if (ElT->IDL64)
				{
					tElT=(ID_Elem64*)ElT->IDL64->Begin(FSL8);
					ElT->GetIDI()->ID_Standart=ElT->IDL64->NumElem;
				}
				else
				{
					ElT->GetIDI()->ID_Standart=0;
				}
				int CntChild=0;
				ElT->GetIDI()->Command=Command_MODIFY;
				ElT->GetIDI()->TypeIDI=TypeIDI_TABLE;
				ElT->GetIDI()->StateBlock=StateBlock_BEGIN;
				ElT->GetIDI()->NumPack=0;
//				if (LastHost[0]==0)
				strcpy(LastHost,"");
//				ElT->GetIDI()->pPList->SetParam(IDHostName,&LastHost,strlen(LastHost)+1,this);
				strcpy(ElT->GetIDI()->AdrInfo->DestinAI.HostName,LastHost);
				Srv->GetRealTime(&ElT->GetIDI()->TimeOper.FTime);
//				ElT->GetIDI()->pPList->DelParam(IDDomainName);
				ElT->GetIDI()->AdrInfo->DestinAI.DomainName[0]=0;
				ElT->GetIDI()->CustomerID=CustomerID;
				while (tElT!=NULL)
				{
					El_Tab*	tEl;
					ElT->GetIDI()->Buf=(UCHAR*)Child;
					while ((tElT!=NULL)&&(CntChild<SizeChildBuf))
					{
						tEl=GetElTab(&TmpAnchor,tElT->Vol,1);
						if (tEl)
						{
							if (tEl->pParent!=ElT)
							{
								ElT=GetElTab(&Anchor,tID,1);
								ElT->DelChild(tElT->Vol);
								FlCntCh++;
							}
							else
							{
								Child[CntChild++]=tElT->Vol;
							}
							UnLockEl(&TmpAnchor,tEl);
						}
						else
						{
//							tEl=GetElTab(&TmpAnchor,tElT->Vol,1);
							ElT->DelChild(tElT->Vol);	// !!!!!!!!!!!!!!!!!!!!!!!!!!!!
							FlCntCh++;
						};

						if (ElT->IDL64)
						{
							tElT=(ID_Elem64*)ElT->IDL64->Next(FSL8);
						}
					}
					ElT->GetIDI()->Cnt=CntChild*sizeof(INT64);
					if ((ElT->GetIDI()->Cnt!=0)&&(tElT!=NULL)) 
					{
						ElT->GetIDI()->NumPack++;
						Srv->ReplElTabl(ElT);
//						ODataIDI(3,ElT->GetIDI());
						ElT->GetIDI()->StateBlock=0;
					}
					CntChild=0;
				}
				ElT->GetIDI()->StateBlock|=StateBlock_END;
				Srv->GetRealTime(&ElT->GetIDI()->TimeOper.FTime);
				ElT->GetIDI()->NumPack++;
				ElT->GetIDI()->ID_SeansFrom=ID;
				Srv->ReplElTabl(ElT);
//				ODataIDI(3,ElT->GetIDI());
//				if (Fl_Debug)
				{
					if (GetParam(&TmpAnchor,ElT,IDNameElTab,tVol)==0)
					{
						sprintf(LogBuf,"ReplEl to %s ID=%I64x Name=%s",LastHost,ElT->GetIDI()->ID_File,tVol);
					}
					else
					{
						sprintf(LogBuf,"ReplEl to %s ID=%I64x",LastHost,ElT->GetIDI()->ID_File);
					}
					Srv->OutLog(LogBuf,this,TableName);
				}
			}
			else
			{
				Srv->SetCriticalError(this,"Error LockEl Write in ReplEl","Error",TableName);
			};
		}
// 		else
// 		{
// 			DelEl(ElT);
// 		}
	}
	if (FlCntCh)
		ElT->Fl_NeedWR=1;	//SaveEl(ElT);
	UnLockEl(&Anchor,ElT);
	return rc;
};

int	IDTable::AddIDI(IDInfo* InIDITr)
{
	int rc=-1;
	IDInfo* IDITr=InIDITr;
	InIDITr->CustomerID=CustomerID;

	if (FlagRE>100)
	{
		Srv->SetCriticalError(this,"AddIDI->FlagRE","Error",TableName);
		return rc;
	}
	FlagRE++;
//	El_Tab*		tElTt;
	El_Tab*		tElT2;
	El_Tab*		tElT=(El_Tab*)IDL->Find((UCHAR*)&IDITr->ID_File,sizeof(INT64));
	Srv->GetRealTime(&IDITr->TimeOper.FTime);
	if (IDITr->TimeBegin.Time==0)
		IDITr->TimeBegin.Time=IDITr->TimeOper.Time;
	if (tElT!=NULL)
	{

		if (IDITr->ID_File==IDITr->ID_ParentFile)
		{
			char	ttBuf[80];
			sprintf(ttBuf,"AddIDI ID==Parent=%I64x",IDITr->ID_File);
			Srv->SetCriticalError(this,ttBuf,"Error",TableName);
			return rc;
		}

		LockEl(&TmpAnchor,tElT);
//		MoLockEl(&TmpAnchor,tElT,AcsMd_Write);
		CntActQuery--;
		if (tElT->pParent==NULL)
		{
			if (IDITr->ID_File==ID)	// Òîëüêî êîðåíü!
			{
				rc=CopyIDI2El(IDITr,tElT,1);
				if (rc!=-1)
				{
					if (IDITr->Cnt!=0)
					{
						int		tCntP=IDITr->Cnt/sizeof(INT64);
						INT64*	tBufP=(INT64*)IDITr->Buf;
						for (size_t i=0;i<tCntP;i++) // Çàïðîñ îñòàëüíûõ ýëåìåíòîâ òàáëèöû
						{
							if (tBufP[i])
							{
								if (tElT->AddChild(tBufP[i])!=-1)
								{
	//								tElTt=GetElTab(&TmpAnchor,tBufP[i]);
	//								UnLockEl(&TmpAnchor,tElTt);
								}
							}
						}
					}
					if (IDITr->StateBlock&StateBlock_END)
					{
						Fl_Ready=1;
					}
	//				UpDataEl(tElT);	// ?????????????????????????
				}
			}
		}
		else
		{
			rc=CopyIDI2El(IDITr,tElT,1);
			if (rc!=-1)
			{
				if (IDITr->Cnt!=0)
				{
					int		tCntP=IDITr->Cnt/sizeof(INT64);
					INT64*	tBufP=(INT64*)IDITr->Buf;
					for (size_t i=0;i<tCntP;i++) // Çàïðîñ îñòàëüíûõ ýëåìåíòîâ òàáëèöû
					{
//						tElT->AddChild(tBufP[i]);
						if (tBufP[i])
						{
							if (tElT->AddChild(tBufP[i])!=-1)
							{
	//							tElTt=GetElTab(&TmpAnchor,tBufP[i]);
	//							UnLockEl(&TmpAnchor,tElTt);
							}
						}
					}
				}
				if (IDITr->StateBlock&StateBlock_END)
				{
					Fl_Ready=1;
				}
			}
		}
		UnLockEl(&TmpAnchor,tElT);
		if (TestEl(tElT)==-1) tElT=NULL;
		rc=0;
	}
	else
	{	// Ëèáî ÷óæîé ëèáî êîðåíü
		if (IDITr->ID_File==ID)	// Òîëüêî êîðåíü!
		{
			CntActQuery--;
			if (IDITr->StateBlock&StateBlock_ERROR)
			{
				Fl_Ready=1;
				rc=-1;
			}
			else
			{
				tElT=ElTbl=AddElTab(&TmpAnchor,NULL,ID);
//				MoLockEl(&TmpAnchor,tElT,AcsMd_Write);
				rc=CopyIDI2El(IDITr,tElT,1);
				if (rc!=-1)
				{
					if (IDITr->Cnt!=0)
					{
						int		tCntP=IDITr->Cnt/sizeof(INT64);
						INT64*	tBufP=(INT64*)IDITr->Buf;
						for (size_t i=0;i<tCntP;i++) // Çàïðîñ îñòàëüíûõ ýëåìåíòîâ òàáëèöû
						{
							if (tBufP[i])
							{
	//							tElT->AddChild(tBufP[i]);
								if (tElT->AddChild(tBufP[i])!=-1)
								{
	//								tElTt=GetElTab(&TmpAnchor,tBufP[i]);
	//								UnLockEl(&TmpAnchor,tElTt);
								}
							}
	/*						QIDI->ID_File=Srv->GetUID();
							Srv->GetRealTime(&QIDI->TimeBegin.FTime);
							QIDI->TimeOper.FTime=QIDI->TimeBegin.FTime;
							QIDI->TimeLastWr.FTime=QIDI->TimeBegin.FTime;
							QIDI->TimeClose.FTime=QIDI->TimeBegin.FTime;
							QIDI->pPList->SetParam(Srv->GetIDParam(PrmID),&tBufP[i],sizeof(INT64),this);
							if (IDL->Find((UCHAR*)&tBufP[i],sizeof(INT64))==NULL) 
								tElT=AddElTab(&TmpAnchor,ElTbl,tBufP[i]);
							CntActQuery++;
							ODataFF(0,(UCHAR*)QIDI,sizeof(IDInfo));
	*/					}
					}
					if (IDITr->StateBlock&StateBlock_END)
					{
						Fl_Ready=1;
					}
				}
				UnLockEl(&TmpAnchor,tElT);
			}
		} 
		else 
		{
			CntActQuery--;
			if (IDITr->ID_File==IDITr->ID_ParentFile)
			{
				char	ttBuf[80];
				sprintf(ttBuf,"AddIDI ID==Parent=%I64x",IDITr->ID_File);
				Srv->SetCriticalError(this,ttBuf,"Error",TableName);
				return rc;
			}
			if (IDITr->ID_SeansFrom==ID)	// Íàø êîðåíü!
			{
				tElT2=(El_Tab*)IDL->Find((UCHAR*)&IDITr->ID_ParentFile,sizeof(INT64));
/*				if (tElT2==NULL)
				{
//					if (IDITr->ID_ParentFile==IDITr->ID_File)
//					{
//						rc=-1;
//					}
//					else
// !!!!!!!!!!!!!!!!!!!!!! Åñëè íåò íè åëåì íè åãî ðîäèòåëÿ òî çàãðóçêà íå óäàëàñü!!!
					if (tElT2)
						while (tElT2->GetIDI()->ID_File!=IDITr->ID_ParentFile)
						{
							tElT2->Fl_Del=1;
							IDL->Del(tElT2);
							tElT2=LoadEl(IDITr->ID_ParentFile);
							if (tElT2==NULL)
								break;
						}

				}
*/				if (tElT2)
				{
					LockEl(&TmpAnchor,tElT2);
					if (tElT2->GetIDI()->ID_File!=IDITr->ID_ParentFile)
						rc=-1;
					tElT=AddElTab(&TmpAnchor,tElT2,IDITr->ID_File);
//					MoLockEl(&TmpAnchor,tElT,AcsMd_Write);
					rc=CopyIDI2El(IDITr,tElT,1);
					if (rc!=-1)
					{
						if (IDITr->Cnt!=0)
						{
							int		tCntP=IDITr->Cnt/sizeof(INT64);
							INT64*	tBufP=(INT64*)IDITr->Buf;
							for (size_t i=0;i<tCntP;i++) // Çàïðîñ îñòàëüíûõ ýëåìåíòîâ òàáëèöû
							{
								if (tBufP[i])
								{
	//								tElT->AddChild(tBufP[i]);
									if (tElT->AddChild(tBufP[i])!=-1)
									{
	//									tElTt=GetElTab(&TmpAnchor,tBufP[i]);
	//									if (tElTt==NULL) tElT->DelChild(tBufP[i]);
	//									UnLockEl(&TmpAnchor,tElTt);
									}
								}
							}
						}
						if (IDITr->StateBlock&StateBlock_END)
						{
							Fl_Ready=1;
						}
					}
					UnLockEl(&TmpAnchor,tElT2);
					UnLockEl(&TmpAnchor,tElT);
				}
				else
				{
					El_Tab*		ttElT=NULL;
					if (QueryEl(IDITr->ID_ParentFile)!=-1)
						ttElT=LoadEl(&TmpAnchor,IDITr->ID_ParentFile);
					if (IDL->Find((UCHAR*)&IDITr->ID_ParentFile,sizeof(INT64))!=0)
					{
						ttElT=LoadEl(&TmpAnchor,IDITr->ID_File);
					}
					if (ttElT)
						UnLockEl(&TmpAnchor,ttElT);
				}
			}
		}
		if (Srv->MyNum==Srv->SInt->NumLANSrv)
		{
			if (TestEl(tElT)==-1)
				tElT=NULL;
		}
//		TestEl(tElT);
	}
	FlagRE--;
	return rc;
};

El_Tab*		IDTable::AddElTab(StrOwner* Owner,El_Tab* ElTParent,INT64 tID)	// ElTParent = NULL main Table Elem
{
	El_Tab*		tElT=NULL;
	MyLock(__FILE__,__LINE__,&FlSim,this);
	if (ElTParent==NULL)
	{
		if (ElTbl!=NULL)
		{
			ElTbl->Fl_Del=1;
			IDL->Del(ElTbl);
			ElTbl->GetIDI()->pPList->DelAllElem();
		}
		ElTbl=(El_Tab*)IDL->NewElem();
		if (ID!=NULL)
		{
			if (ElTbl==NULL)
				ElTbl=new El_Tab(Owner,ID,this);
			else
				ElTbl->Init(Owner,ID,this);
		}
		else
		{
			if (ElTbl==NULL)
				ElTbl=new El_Tab(Owner,tID,this);
			else
				ElTbl->Init(Owner,tID,this);
		}
		if ((ElTbl->GetIDI()->ID_File==ElTbl->GetIDI()->ID_ParentFile)||((ElTbl->GetIDI()->ParentFile)&&(ElTbl->GetIDI()->ID_File==ElTbl->GetIDI()->ParentFile->ID_File)))
		{
			{size_t iasm=1;}
		}
		else
			IDL->Add(ElTbl);
		tElT=ElTbl;
		tElT->GetIDI()->CustomerID=CustomerID;
		tElT->GetIDI()->SeansFrom=ElTbl->GetIDI();
		tElT->GetIDI()->ID_SeansFrom=ElTbl->GetIDI()->ID_File;
	}
	else
	{
		ElTParent->TestOwner(Owner);
		tElT=(El_Tab*)IDL->NewElem();
		if (tElT)
			tElT->Init(Owner,tID,this);
		else
			tElT=new El_Tab(Owner,tID,this);
		tElT->pParent=ElTParent;
		tElT->GetIDI()->ParentFile=ElTParent->GetIDI();
		tElT->GetIDI()->SeansFrom=ElTbl->GetIDI();
		tElT->GetIDI()->ID_SeansFrom=ElTbl->GetIDI()->ID_File;
		if ((tElT->GetIDI()->ID_File==tElT->GetIDI()->ID_ParentFile)||((tElT->GetIDI()->ParentFile)&&(tElT->GetIDI()->ID_File==tElT->GetIDI()->ParentFile->ID_File)))
		{
			{size_t iasm=1;}
		}
		else
			IDL->Add(tElT);
		tElT->GetIDI()->CustomerID=CustomerID;
	}
//	UnLockEl(&TmpAnchor,tElT);
	LockEl(Owner,tElT);
//	ModifElTab(Owner,ElTParent);
	MyUnLock(__FILE__,__LINE__,&FlSim,this);
	return tElT;
}

El_Tab*	IDTable::CopyIDI2Parent(StrOwner* Owner,IDInfo* IDITr,El_Tab* ElTParent)
{
	El_Tab*		tElT=NULL;
	El_Tab*		tElTt=NULL;
	char*	tV=NULL;
	int		rc=0;
	if (ElTParent==NULL)
	{
		if (ElTbl==NULL)
		{	// Åñëè åëåìåíò óæå åñòü òî íåîáõîäèìî åãî ïåðåîïðåäåëèòü (èëè óäàëèòü !!!)
			return tElT;	// Error
		}
		tElT=(El_Tab*)IDL->Find((UCHAR*)&ElTbl->GetIDI()->ID_File,sizeof(INT64));
		if (tElT==NULL)
		{
			return tElT;	// Error
		}
		LockEl(&Anchor,ElTbl);
//		MoLockEl(&TmpAnchor,ElTbl,AcsMd_Write);
		tElT=(El_Tab*)IDL->NewElem();
		if (tElT==NULL)
			tElT=new El_Tab(&TmpAnchor,IDITr->ID_File,this);
		else
			tElT->Init(&TmpAnchor,IDITr->ID_File,this);
		tElT->pParent=ElTbl;
		tElT->GetIDI()->ParentFile=ElTbl->GetIDI();
		tElT->GetIDI()->SeansFrom=ElTbl->GetIDI();
		ElTbl->AddChild(tElT->GetIDI()->ID_File,1);
		LockEl(&TmpAnchor,tElT);
//		MoLockEl(&TmpAnchor,tElT,AcsMd_Write);
		if ((tElT->GetIDI()->ID_File==tElT->GetIDI()->ID_ParentFile)||((tElT->GetIDI()->ParentFile)&&(tElT->GetIDI()->ID_File==tElT->GetIDI()->ParentFile->ID_File)))
		{
			{size_t iasm=1;}
		}
		else
			IDL->Add(tElT);
		//		AddParam(&TmpAnchor,tElT,IDHostName,Srv->HostName);
		//		AddParam(&TmpAnchor,tElT,IDMtName,this->Name_Mt);
		if (tElT)
		{
//			MoLockEl(&TmpAnchor,tElT,AcsMd_Write);
			if (Fl_Debug)
			{
				if (GetParam(&TmpAnchor,tElT,IDNameElTab,tV)==0)
				{
					sprintf(LogBuf,"Repl->CopyEl Name=%s",tV); Srv->OutLog(LogBuf,this,TableName);
				}
			}
			rc=CopyIDI2El(IDITr,tElT,0);
			if (rc!=-1)
			{
				if (IDITr->Cnt!=0)
				{
					int		tCntP=IDITr->Cnt/sizeof(INT64);
					INT64*	tBufP=(INT64*)IDITr->Buf;
					for (size_t i=0;i<tCntP;i++) // Çàïðîñ îñòàëüíûõ ýëåìåíòîâ òàáëèöû
					{
						if (tBufP[i])
						{
							if (tElT->AddChild(tBufP[i])!=-1)
							{
								tElTt=GetElTab(&TmpAnchor,tBufP[i]);
								UnLockEl(&TmpAnchor,tElTt);
							}
						}
					}
				}
				SaveEl(tElT);
			}
			if (!TestValid(tElT,ElTParent))
			{
//				DelElTab(&TmpAnchor,tElT); tElT=NULL;
			};
			UnLockEl(&TmpAnchor,tElT);
		}
		SendEl(ElTbl);
		ElTbl->Fl_Query=0;
		ReplEl(ElTbl);
		UnLockEl(&Anchor,ElTbl);
	}
	else
	{
		tElT=(El_Tab*)IDL->Find((UCHAR*)&ElTParent->GetIDI()->ID_File,sizeof(INT64));
		if (tElT==NULL)
		{
			return tElT;	// Error
		}
		LockEl(&TmpAnchor,ElTParent);
//		MoLockEl(&TmpAnchor,ElTParent,AcsMd_Write);
		tElT=(El_Tab*)IDL->NewElem();
		if (tElT==NULL)
			tElT=new El_Tab(&TmpAnchor,IDITr->ID_File,this);
		else
			tElT->Init(&TmpAnchor,IDITr->ID_File,this);
		tElT->pParent=ElTParent;
		tElT->GetIDI()->ParentFile=ElTParent->GetIDI();
		tElT->GetIDI()->SeansFrom=ElTbl->GetIDI();
		ElTParent->AddChild(tElT->GetIDI()->ID_File,1);
		LockEl(&TmpAnchor,tElT);
//		MoLockEl(&TmpAnchor,tElT,AcsMd_Write);
		if ((tElT->GetIDI()->ID_File==tElT->GetIDI()->ID_ParentFile)||((tElT->GetIDI()->ParentFile)&&(tElT->GetIDI()->ID_File==tElT->GetIDI()->ParentFile->ID_File)))
		{
		size_t iasm=1;
		}
		else
			IDL->Add(tElT);
		//		AddParam(&TmpAnchor,tElT,IDHostName,Srv->HostName);
		//		AddParam(&TmpAnchor,tElT,IDMtName,this->Name_Mt);
		if (tElT)
		{
//			MoLockEl(&TmpAnchor,tElT,AcsMd_Write);
			if (Fl_Debug)
			{
				if (GetParam(&TmpAnchor,tElT,IDNameElTab,tV)==0)
				{
					sprintf(LogBuf,"Repl->CopyEl Name=%s",tV); Srv->OutLog(LogBuf,this,TableName);
				}
			}
			rc=CopyIDI2El(IDITr,tElT,0);
			if (rc!=-1)
			{
				if (IDITr->Cnt!=0)
				{
					int		tCntP=IDITr->Cnt/sizeof(INT64);
					INT64*	tBufP=(INT64*)IDITr->Buf;
					for (size_t i=0;i<tCntP;i++) // Çàïðîñ îñòàëüíûõ ýëåìåíòîâ òàáëèöû
					{
						if (tBufP[i])
						{
							if (tElT->AddChild(tBufP[i])!=-1)
							{
								tElTt=GetElTab(&TmpAnchor,tBufP[i]);
								UnLockEl(&TmpAnchor,tElTt);
							}
						}
					}
				}
				SaveEl(tElT);
			}
			if (!TestValid(tElT,ElTParent))
			{
//				DelElTab(&TmpAnchor,tElT); tElT=NULL;
			};
			UnLockEl(&TmpAnchor,tElT);
		}
		SendEl(ElTParent);
		ElTParent->Fl_Query=0;
		ReplEl(ElTParent);
		UnLockEl(&TmpAnchor,ElTParent);
		LockEl(Owner,ElTParent);
	}
	return	tElT;
};
El_Tab*	IDTable::CopyIDI2ParentC(StrOwner* Owner,IDInfo* IDITr,El_Tab* ElTParent)
{
	El_Tab*		tElT=NULL;
	El_Tab*		tElTt=NULL;
	char*	tV=NULL;
	int		rc=0;
	if (ElTParent==NULL)
	{
		if (ElTbl==NULL)
		{	// Åñëè åëåìåíò óæå åñòü òî íåîáõîäèìî åãî ïåðåîïðåäåëèòü (èëè óäàëèòü !!!)
			return tElT;	// Error
		}
		tElT=(El_Tab*)IDL->Find((UCHAR*)&ElTbl->GetIDI()->ID_File,sizeof(INT64));
		if (tElT==NULL)
		{
			return tElT;	// Error
		}
		LockEl(&Anchor,ElTbl);
//		MoLockEl(&TmpAnchor,ElTbl,AcsMd_Write);
		
		tElT=(El_Tab*)IDL->NewElem();
		if (tElT==NULL)
			tElT=new El_Tab(&TmpAnchor,IDITr->ID_File,this);
		else
			tElT->Init(&TmpAnchor,IDITr->ID_File,this);
		tElT->pParent=ElTbl;
		tElT->GetIDI()->ParentFile=ElTbl->GetIDI();
		tElT->GetIDI()->SeansFrom=ElTbl->GetIDI();
		ElTbl->AddChild(tElT->GetIDI()->ID_File,1);
		LockEl(&TmpAnchor,tElT);
//		MoLockEl(&TmpAnchor,tElT,AcsMd_Write);
		if ((tElT->GetIDI()->ID_File==tElT->GetIDI()->ID_ParentFile)||((tElT->GetIDI()->ParentFile)&&(tElT->GetIDI()->ID_File==tElT->GetIDI()->ParentFile->ID_File)))
		{
		size_t iasm=1;
	}
		else
			IDL->Add(tElT);
//		SendEl(ElTbl);
//		ReplEl(ElTbl);
		UnLockEl(&Anchor,ElTbl);
	}
	else
	{
		tElT=(El_Tab*)IDL->Find((UCHAR*)&ElTParent->GetIDI()->ID_File,sizeof(INT64));
		if (tElT==NULL)
		{
			return tElT;	// Error
		}
		LockEl(&TmpAnchor,ElTParent);
//		MoLockEl(&TmpAnchor,ElTParent,AcsMd_Write);
		
		tElT=(El_Tab*)IDL->NewElem();
		if (tElT==NULL)
			tElT=new El_Tab(&TmpAnchor,IDITr->ID_File,this);
		else
			tElT->Init(&TmpAnchor,IDITr->ID_File,this);
		tElT->pParent=ElTParent;
		tElT->GetIDI()->ParentFile=ElTParent->GetIDI();
		tElT->GetIDI()->SeansFrom=ElTbl->GetIDI();
		ElTParent->AddChild(tElT->GetIDI()->ID_File,1);
		LockEl(&TmpAnchor,tElT);
//		MoLockEl(&TmpAnchor,tElT,AcsMd_Write);
		if ((tElT->GetIDI()->ID_File==tElT->GetIDI()->ID_ParentFile)||((tElT->GetIDI()->ParentFile)&&(tElT->GetIDI()->ID_File==tElT->GetIDI()->ParentFile->ID_File)))
		{
			{size_t iasm=1;}
		}
		else
			IDL->Add(tElT);
//		SendEl(ElTParent);
//		ReplEl(ElTParent);
		UnLockEl(&TmpAnchor,ElTParent);
	}
	if (tElT)
	{
//		MoLockEl(&TmpAnchor,tElT,AcsMd_Write);
		if (Fl_Debug)
		{
			if (GetParam(&TmpAnchor,tElT,IDNameElTab,tV)==0)
			{
				sprintf(LogBuf,"Repl->CopyEl Name=%s",tV); Srv->OutLog(LogBuf,this,TableName);
			}
		}
		rc=CopyIDI2El(IDITr,tElT,0);
		if (rc!=-1)
		{
			if (IDITr->Cnt!=0)
			{
				int		tCntP=IDITr->Cnt/sizeof(INT64);
				INT64*	tBufP=(INT64*)IDITr->Buf;
				for (size_t i=0;i<tCntP;i++) // Çàïðîñ îñòàëüíûõ ýëåìåíòîâ òàáëèöû
				{
					if (tBufP[i])
					{
						if (tElT->AddChild(tBufP[i])!=-1)
						{
							tElTt=GetElTab(&TmpAnchor,tBufP[i]);
							UnLockEl(&TmpAnchor,tElTt);
						}
					}
				}
			}
			SaveEl(tElT);
		}
// 		if (!TestValid(tElT,ElTParent))
// 		{
// 			DelElTab(&TmpAnchor,tElT); tElT=NULL;
// 		};
//		TestValid(tElT,ElTParent);
		UnLockEl(&TmpAnchor,tElT);
//		SendEl(tElT);
	}
	return	tElT;
};
int	IDTable::CopyIDI2El(IDInfo* IDITr,El_Tab* tElT,int NoRepl)
{
	int			rc=0;
	El_Tab*		tET;
	INT64		IDName;
	void*		pVol;
	char*		tVol=NULL;
	size_t		pCnt;
	El_Tab*		tElTt=NULL;
	Decoder*	ParentMt=NULL;
	if (tElT->Fl_Del!=0)
	{
		Srv->SetCriticalError(this,"Copy Fl_Del=1","Warning");
	}
	if (tElT)
//	if ((tElT)&&(tElT->Fl_Del==0))
	{
		if (tElT->Fl_Test)
			tElT->Fl_Test=0;
// 		if (tElT->Fl_Query==0)
// 		{
// //			QueryEl(tElT->IDI->ID_File);
// //			tElT->Fl_Query=1;
// //			Srv->GetRealTime(&tElT->QueryTime.FTime);
// 		}
// 		else
// 		{
// //			FileTime	TmpTime;
// //			Srv->GetRealTime(&TmpTime.FTime);
// //			if (TmpTime.Time-tElT->QueryTime.Time>KtimeM)
// //			{
// //				tElT->Fl_Query=0;
// //			}
// 		};
		LockEl(&TmpAnchor,tElT);
//		MoLockEl(&TmpAnchor,tElT,AcsMd_Write);
		if (Fl_Debug)
		{
			if (GetParam(&TmpAnchor,tElT,IDNameElTab,tVol)==0)
			{
				sprintf(LogBuf,"CopyEl ID=%I64x Name=%s",tElT->GetIDI()->ID_File,tVol);
			}
			else
			{
				sprintf(LogBuf,"CopyEl ID=%I64x",tElT->GetIDI()->ID_File);
			}
			Srv->OutLog(LogBuf,this,TableName);
		}
		if (tElT->GetIDI()->ID_File==IDITr->ID_File)
		{
			if (IDITr->ID_ParentFile)
			{
				tET=(El_Tab*)IDL->Find((UCHAR*)&IDITr->ID_ParentFile,sizeof(INT64));
				if (tET)
				{
					if (tElT->GetIDI()->ParentFile!=NULL)
					{
						if (tElT->GetIDI()->ParentFile->ID_File!=tET->GetIDI()->ID_File)
						{
#ifdef	_DEBUG
// 							char	tB[128];
// 							if (tVol)
// 								sprintf(tB,"Copy Err! ID=%I64x Name=%s ParID=%I64x ParID2=%I64x",tElT->GetIDI()->ID_File,tVol,tElT->GetIDI()->ParentFile->ID_File,tET->GetIDI()->ID_File);
// 							else
// 								sprintf(tB,"Copy Err! ID=%I64x ParID=%I64x ParID2=%I64x",tElT->GetIDI()->ID_File,tElT->GetIDI()->ParentFile->ID_File,tET->GetIDI()->ID_File);
// 							Srv->SetCriticalError(this,tB,"Error","MReplicator");
#endif
							try
							{
								tElT->pParent->DelChild(IDITr->ID_File);
							}
							catch (...)
							{
								
							}
							tElT->GetIDI()->ParentFile=tET->GetIDI();
							tElT->GetIDI()->ID_ParentFile=tET->GetIDI()->ID_File;
							tElT->pParent=tET;
							if (tET->AddChild(IDITr->ID_File)!=-1)
								SaveEl(tET);
	//						ReplEl(tET);ReplEl(tElT);
	//						DelEl(tElT);
	//						rc=-1;
						}
						else
						{
							if (tET->AddChild(IDITr->ID_File)!=-1)
								SaveEl(tET);
						}
					}
					else
					{
						Srv->SetCriticalError(this,"Error Parent","Error","MReplicator");
					}
				}
				else
				{
					tET=GetElTab(&TmpAnchor,IDITr->ID_ParentFile);
					if (tET)
					{
						if (tElT->GetIDI()->ParentFile!=NULL)
						{
							if (tElT->GetIDI()->ParentFile->ID_File!=tET->GetIDI()->ID_File)
							{
#ifdef _DEBUG 
// 								char	tB[128];
// 								if (tVol)
// 									sprintf(tB,"Copy Err! ID=%I64x Name=%s ParID=%I64x ParID2=%I64x",tElT->GetIDI()->ID_File,tVol,tElT->GetIDI()->ParentFile->ID_File,tET->GetIDI()->ID_File);
// 								else
// 									sprintf(tB,"Copy Err! ID=%I64x ParID=%I64x ParID2=%I64x",tElT->GetIDI()->ID_File,tElT->GetIDI()->ParentFile->ID_File,tET->GetIDI()->ID_File);
// 								Srv->SetCriticalError(this,tB,"Error","MReplicator");
#endif								
								tElT->GetIDI()->ParentFile=tET->GetIDI();
								tElT->pParent=tET;
								//							ReplEl(tET);ReplEl(tElT);
								//							DelEl(tElT);
								//							rc=-1;
							}
						}
						else
						{
							Srv->SetCriticalError(this,"Error Parent","Error","MReplicator");
						}
						UnLockEl(&TmpAnchor,tET);
					}
					else
					{
//						ReplEl(tElT);
						QueryEl(IDITr->ID_ParentFile);
//						QueryEl(IDITr->ID_File);
						rc=-1;
					}
				};
			}
//			else
//			{
//				{size_t iasm=1;}
//			};
			IDInfo*	ttElT=tElT->GetIDI();
			strcpy(ttElT->ParentMtName,IDITr->ParentMtName);ttElT->ParentMtExt=IDITr->ParentMtExt;
			ttElT->ID_Standart=IDITr->ID_Standart;
			ttElT->StateProc=IDITr->StateProc;
			ttElT->Command=IDITr->Command;
			ttElT->StateBlock=IDITr->StateBlock;
			ttElT->TypeIDI=IDITr->TypeIDI;
			ttElT->Priority=IDITr->Priority;
			FileTime	tTm;
			Srv->GetRealTime(&tTm.FTime);tTm.Time+=KtimeM;
			if (IDITr->TimeBegin.Time>tTm.Time)
			{
				IDITr->TimeBegin.Time=tTm.Time;
				IDITr->TimeClose.Time=tTm.Time;
			}
			if (IDITr->TimeLastWr.Time>tTm.Time)
			{
				IDITr->TimeLastWr.Time=tTm.Time;
				IDITr->TimeClose.Time=tTm.Time;
			}
			ttElT->ModifCnt=IDITr->ModifCnt;
			ttElT->CustomerID=IDITr->CustomerID;
			ttElT->TimeBegin.Time=IDITr->TimeBegin.Time;
			ttElT->TimeLastWr.Time=IDITr->TimeLastWr.Time;
			ttElT->TimeClose.Time=IDITr->TimeClose.Time;
			ttElT->TimeOper.Time=IDITr->TimeOper.Time;
			ttElT->NumPack=IDITr->NumPack;
			ttElT->pPList->DelAllElem();
			VolPrm*	tpVol;
			if(IDITr->pPList->FindFirst(IDName,tpVol,pVol,pCnt,ParentMt)!=-1)
			{
				do
				{
					ttElT->pPList->SetParam(IDName,pVol,pCnt,ParentMt,&tpVol->dV.TimeModification);
				}
				while(IDITr->pPList->FindNext(IDName,tpVol,pVol,pCnt,ParentMt)!=-1);
			};
			ttElT->CustomerID=IDITr->CustomerID;
			if (rc!=-1)
			{	
			}
			else
			{
				ReplEl(tElT);		// ?????????????????????????????
			};
			if (IDITr->Cnt!=0)
			{
				int		tCntP=IDITr->Cnt/sizeof(INT64);
				INT64*	tBufP=(INT64*)IDITr->Buf;
				for (size_t i=0;i<tCntP;i++) // Çàïðîñ îñòàëüíûõ ýëåìåíòîâ òàáëèöû
				{
					if (tBufP[i])
					{
						if (tElT->AddChild(tBufP[i])!=-1)
						{
							if (NoRepl==0)
							{
								tElTt=GetElTab(&TmpAnchor,tBufP[i]);
								UnLockEl(&TmpAnchor,tElTt);
							}
						}
					}
				}
			}
			if (NoRepl==0)
				SaveEl(tElT);
	
		}
		else
			rc=-1;
		if (!TestValid(tElT,tElT->pParent))
		{
//			DelElTab(&TmpAnchor,tElT); tElT=NULL;
		};
//		TestValid(tElT,tElT->pParent);
		if (tElT)
		{
			tElT->Fl_Query=0;
			UnLockEl(&TmpAnchor,tElT);
		}
	}
//	Srv->GetRealTime(&tElT->GetIDI()->TimeOper.FTime);
	return rc;
};

int	IDTable::CopyIDI2Data(IDInfo* IDITr,El_Tab* tElT,int NoRepl)
{
	int			rc=0;
//	El_Tab*		tET;
	INT64		IDName;
	void*		pVol;
	char*		tVol;
	size_t		pCnt;
	El_Tab*		tElTt=NULL;
	Decoder*	ParentMt=NULL;
//	MyLock(__FILE__,__LINE__,&Fl_RE,this);
	if (tElT->Fl_Del!=0)
	{
		Srv->SetCriticalError(this,"Copy Fl_Del=1","Warning");
	}
	if (tElT)
//	if ((tElT)&&(tElT->Fl_Del==0))
	{
		LockEl(&TmpAnchor,tElT);
//		MoLockEl(&TmpAnchor,tElT,AcsMd_Write);
		if (Fl_Debug)
		{
			if (GetParam(&TmpAnchor,tElT,IDNameElTab,tVol)==0)
			{
				sprintf(LogBuf,"CopyElData ID=%I64x Name=%s",tElT->GetIDI()->ID_File,tVol);
			}
			else
			{
				sprintf(LogBuf,"CopyElData ID=%I64x",tElT->GetIDI()->ID_File);
			}
			Srv->OutLog(LogBuf,this,TableName);
		}
//		if (tElT->GetIDI()->ID_File==IDITr->ID_File)
		{
//			else
//			{
//				{size_t iasm=1;}
//			};
//			if (rc!=-1)
			{	
//				tElT->GetIDI()->ParentMt=IDITr->ParentMt;
				strcpy(tElT->GetIDI()->ParentMtName,IDITr->ParentMtName);tElT->GetIDI()->ParentMtExt=IDITr->ParentMtExt;
				tElT->GetIDI()->ID_Standart=IDITr->ID_Standart;
				tElT->GetIDI()->StateProc=IDITr->StateProc;
				tElT->GetIDI()->Command=IDITr->Command;
				tElT->GetIDI()->StateBlock=IDITr->StateBlock;
				tElT->GetIDI()->TypeIDI=IDITr->TypeIDI;
				tElT->GetIDI()->Priority=IDITr->Priority;
				FileTime	tTm;
				Srv->GetRealTime(&tTm.FTime);tTm.Time+=KtimeM;
				if (IDITr->TimeBegin.Time>tTm.Time)
				{
					IDITr->TimeBegin.Time=tTm.Time;
					IDITr->TimeClose.Time=tTm.Time;
				}
				if (IDITr->TimeLastWr.Time>tTm.Time)
				{
					IDITr->TimeLastWr.Time=tTm.Time;
					IDITr->TimeClose.Time=tTm.Time;
				}
				tElT->GetIDI()->ModifCnt=IDITr->ModifCnt;
				tElT->GetIDI()->CustomerID=IDITr->CustomerID;
				tElT->GetIDI()->TimeBegin.Time=IDITr->TimeBegin.Time;
				tElT->GetIDI()->TimeLastWr.Time=IDITr->TimeLastWr.Time;
				tElT->GetIDI()->TimeClose.Time=IDITr->TimeClose.Time;
				tElT->GetIDI()->TimeOper.Time=IDITr->TimeOper.Time;
				tElT->GetIDI()->NumPack=IDITr->NumPack;
				tElT->GetIDI()->pPList->DelAllElem();
				if(IDITr->pPList->FindFirst(IDName,pVol,pCnt,ParentMt)!=-1)
				{
					do
					{
						tElT->GetIDI()->pPList->SetParam(IDName,pVol,pCnt,ParentMt);
					}
					while(IDITr->pPList->FindNext(IDName,pVol,pCnt,ParentMt)!=-1);
				};
			}
			SaveEl(tElT);

		}
//		else
//			rc=-1;
		if (!TestValid(tElT,tElT->pParent))
		{
//			DelElTab(&TmpAnchor,tElT); tElT=NULL;
		};
//		TestValid(tElT,tElT->pParent);
		if (tElT)
		{
			tElT->Fl_Query=0;
			UnLockEl(&TmpAnchor,tElT);
		}
// 		if (tElT->Fl_Query==0)
// 		{
// //			QueryEl(tElT->IDI->ID_File);
// //			tElT->Fl_Query=1;
// //			Srv->GetRealTime(&tElT->QueryTime.FTime);
// 		}
// 		else
// 		{
// //			FileTime	TmpTime;
// //			Srv->GetRealTime(&TmpTime.FTime);
// //			if (TmpTime.Time-tElT->QueryTime.Time>KtimeM)
// //			{
// //				tElT->Fl_Query=0;
// //			}
// 		};
	}
//	Srv->GetRealTime(&tElT->GetIDI()->TimeOper.FTime);
//	MyUnLock(__FILE__,__LINE__,&Fl_RE,this);
	return rc;
};

int	IDTable::UpDataEl(El_Tab* ElT,int Fl_Mod)
{
	int rc=0;
	char*			tVol;
	ID_Elem64*		tElT=0;
	if (ElT)
	{
		if ((ElT->GetIDI()->ID_File==ElT->GetIDI()->ID_ParentFile)||((ElT->GetIDI()->ParentFile)&&(ElT->GetIDI()->ID_File==ElT->GetIDI()->ParentFile->ID_File)))
		{
			char	ttBuf[80];
			sprintf(ttBuf,"SaveIDI ID==Parent=%I64x",ElT->GetIDI()->ID_File);
			Srv->SetCriticalError(this,ttBuf,"Error",TableName);
			return -1;
		}
		if (!BLDel.Find(ElT->GetIDI()->ID_File))
		{
			LockEl(&TmpAnchor,ElT);
			if (MoLockEl(&TmpAnchor,ElT,AcsMd_Write)!=-1)
			{
				FindStrList8	FSL8;
				if (ElT->IDL64)
				{
					tElT=(ID_Elem64*)ElT->IDL64->Begin(FSL8);
					ElT->GetIDI()->ID_Standart=ElT->IDL64->NumElem;
				}
				else
				{
					ElT->GetIDI()->ID_Standart=0;
				};
				int CntChild=0;
				ElT->GetIDI()->Command=Command_MODIFY;
				ElT->GetIDI()->TypeIDI=TypeIDI_TABLE;
				ElT->GetIDI()->StateBlock|=StateBlock_BEGIN;
				if (Fl_Mod==0)
					Srv->GetRealTime(&ElT->GetIDI()->TimeLastWr.FTime);
				ElT->GetIDI()->TimeOper.FTime=ElT->GetIDI()->TimeLastWr.FTime;

				while (tElT!=NULL)
				{
					
					ElT->GetIDI()->Buf=(UCHAR*)Child;
					while ((tElT!=NULL)&&(CntChild<SizeChildBuf))
					{
						Child[CntChild++]=tElT->Vol;
						if (ElT->IDL64)
						{
//							tElT=(ID_Elem64*)ElT->IDL64->Find((UCHAR*)&tElT->Vol,sizeof(INT64),1);
							tElT=(ID_Elem64*)ElT->IDL64->Next(FSL8);
						}
					}
					ElT->GetIDI()->Cnt=CntChild*sizeof(INT64);
					if ((ElT->GetIDI()->Cnt!=0)&&(tElT!=NULL)) 
						pFSrv->DataIDIEx(0,ElT->GetIDI());
					CntChild=0;
				}
				//	ElT->GetIDI()->Cnt=0;
				ElT->GetIDI()->StateBlock|=StateBlock_END;
				if (Fl_Mod==0)
					Srv->GetRealTime(&ElT->GetIDI()->TimeOper.FTime);
				ElT->GetIDI()->TimeClose.FTime=ElT->GetIDI()->TimeOper.FTime;
				pFSrv->DataIDIEx(0,ElT->GetIDI());
				if (Fl_Debug)
				{
					if (GetParam(&TmpAnchor,ElT,IDNameElTab,tVol)==0)
					{
						sprintf(LogBuf,"UpDataEl ID=%I64x Name=%s",ElT->GetIDI()->ID_File,tVol);
					}
					else
					{
						sprintf(LogBuf,"UpDataEl ID=%I64x",ElT->GetIDI()->ID_File);
					}
					Srv->OutLog(LogBuf,this,TableName);
				}
				UnLockEl(&TmpAnchor,ElT);

			}
			else
			{
				Srv->SetCriticalError(this,"Error LockEl Write in UpDataEl","Error",TableName);
			};
		}
	}
	return rc;
};

int	IDTable::UpDataEl2(El_Tab* ElT)
{
	int rc=0;
	char*			tVol=NULL;
	ID_Elem64*		tElT=0;
	if (ElT)
	{
		if ((ElT->GetIDI()->ID_File==ElT->GetIDI()->ID_ParentFile)||((ElT->GetIDI()->ParentFile)&&(ElT->GetIDI()->ID_File==ElT->GetIDI()->ParentFile->ID_File)))
		{
			char	ttBuf[80];
			sprintf(ttBuf,"SaveIDI ID==Parent=%I64x",ElT->GetIDI()->ID_File);
			Srv->SetCriticalError(this,ttBuf,"Error",TableName);
			return -1;
		}
		if (!BLDel.Find(ElT->GetIDI()->ID_File))
		{
			LockEl(&TmpAnchor,ElT);
			if (MoLockEl(&TmpAnchor,ElT,AcsMd_Write)!=-1)
			{
				FindStrList8	FSL8;
				if (ElT->IDL64)
				{
					tElT=(ID_Elem64*)ElT->IDL64->Begin(FSL8);
					ElT->GetIDI()->ID_Standart=ElT->IDL64->NumElem;
				}
				else
				{
					ElT->GetIDI()->ID_Standart=0;
				};
				int CntChild=0;
				ElT->GetIDI()->Command=Command_MODIFY;
				ElT->GetIDI()->TypeIDI=TypeIDI_TABLE;
				ElT->GetIDI()->StateBlock=StateBlock_BEGIN;
				if (ElT->GetIDI()->TimeBegin.Time==0)
					Srv->GetRealTime(&ElT->GetIDI()->TimeBegin.FTime);
				ElT->GetIDI()->TimeOper.FTime=ElT->GetIDI()->TimeBegin.FTime;
				ElT->GetIDI()->TimeLastWr.FTime=ElT->GetIDI()->TimeBegin.FTime;
//				ElT->GetIDI()->ModifCnt++;
				while (tElT!=NULL)
				{
					ElT->GetIDI()->Buf=(UCHAR*)Child;
					while ((tElT!=NULL)&&(CntChild<SizeChildBuf))
					{
						Child[CntChild++]=tElT->Vol;
						if (ElT->IDL64)
						{
//							tElT=(ID_Elem64*)ElT->IDL64->Find((UCHAR*)&tElT->Vol,sizeof(INT64),1);
							tElT=(ID_Elem64*)ElT->IDL64->Next(FSL8);
						}
					}
					ElT->GetIDI()->Cnt=CntChild*sizeof(INT64);
					if ((ElT->GetIDI()->Cnt!=0)&&(tElT!=NULL)) 
						pFSrv->DataIDIEx(0,ElT->GetIDI());
					CntChild=0;
				}
				//	ElT->GetIDI()->Cnt=0;
				ElT->GetIDI()->StateBlock|=StateBlock_END;
				if (ElT->GetIDI()->TimeBegin.Time==0)
					Srv->GetRealTime(&ElT->GetIDI()->TimeBegin.FTime);
				ElT->GetIDI()->TimeOper.FTime=ElT->GetIDI()->TimeBegin.FTime;
				ElT->GetIDI()->TimeLastWr.FTime=ElT->GetIDI()->TimeBegin.FTime;
				ElT->GetIDI()->TimeClose.FTime=ElT->GetIDI()->TimeBegin.FTime;
				pFSrv->DataIDIEx(0,ElT->GetIDI());
				if (Fl_Debug)
				{
					if (GetParam(&TmpAnchor,ElT,IDNameElTab,tVol)==0)
					{
						sprintf(LogBuf,"UpDataEl ID=%I64x Name=%s",ElT->GetIDI()->ID_File,tVol);
					}
					else
					{
						sprintf(LogBuf,"UpDataEl ID=%I64x",ElT->GetIDI()->ID_File);
					}
					Srv->OutLog(LogBuf,this,TableName);
				}
				UnLockEl(&TmpAnchor,ElT);

			}
			else
			{
				Srv->SetCriticalError(this,"Error LockEl Write in UpDataEl2","Error",TableName);
			};
		}
	}
	return rc;
};

 int	IDTable::SaveEl(El_Tab* ElT)
{
	int rc=0;
	char*			tVol=NULL;
	ID_Elem64*		tElT=0;
	MyLock(__FILE__,__LINE__,&FlSim,this);
	if ((pFSrv)&&(ElT)&&(ElT->Fl_Del==0)&&(ID!=TabParametr))
	{
		LockEl(&TmpAnchor,ElT);
		if (MoLockEl(&TmpAnchor,ElT,AcsMd_Write)!=-1)
		{
			if ((ElT->GetIDI()->ID_File==ElT->GetIDI()->ID_ParentFile)||((ElT->GetIDI()->ParentFile)&&(ElT->GetIDI()->ID_File==ElT->GetIDI()->ParentFile->ID_File)))
			{
				char	ttBuf[80];
				sprintf(ttBuf,"SaveIDI ID==Parent=%I64x",ElT->GetIDI()->ID_File);
				Srv->SetCriticalError(this,ttBuf,"Error",TableName);
				MyUnLock(__FILE__,__LINE__,&FlSim,this);
				return -1;
			}
			FindStrList8	FSL8;

			if (ElT->IDL64)
			{
				tElT=(ID_Elem64*)ElT->IDL64->Begin(FSL8);
				ElT->GetIDI()->ID_Standart=ElT->IDL64->NumElem;
			}
			else
			{
				ElT->GetIDI()->ID_Standart=0;
			}
			int CntChild=0;
			ElT->GetIDI()->Command=Command_MODIFY;
			ElT->GetIDI()->TypeIDI=TypeIDI_TABLE;
			ElT->GetIDI()->StateBlock=StateBlock_BEGIN;
			Srv->GetRealTime(&ElT->GetIDI()->TimeOper.FTime);
			ElT->GetIDI()->NumPack=0;
			while (tElT!=NULL)
			{
				ElT->GetIDI()->Buf=(UCHAR*)Child;
				while ((tElT!=NULL)&&(CntChild<SizeChildBuf))
				{
					Child[CntChild++]=tElT->Vol;
					if (ElT->IDL64)
					{
//						tElT=(ID_Elem64*)ElT->IDL64->Find((UCHAR*)&tElT->Vol,sizeof(INT64),1);
						tElT=(ID_Elem64*)ElT->IDL64->Next(FSL8);
					}
				}
				ElT->GetIDI()->Cnt=CntChild*sizeof(INT64);
				if ((ElT->GetIDI()->Cnt!=0)&&(tElT!=NULL)) 
				{
					ElT->GetIDI()->NumPack++;
					pFSrv->DataIDIEx(0,ElT->GetIDI());
					ElT->GetIDI()->StateBlock=0;
				}
				CntChild=0;
			}
			//	ElT->GetIDI()->Cnt=0;
			ElT->GetIDI()->StateBlock|=StateBlock_END;
			Srv->GetRealTime(&ElT->GetIDI()->TimeOper.FTime);
			ElT->GetIDI()->NumPack++;
			pFSrv->DataIDIEx(0,ElT->GetIDI());
			if (Fl_Debug)
			{
				if (GetParam(&TmpAnchor,ElT,IDNameElTab,tVol)==0)
				{
					sprintf(LogBuf,"SaveEl ID=%I64x Name=%s",ElT->GetIDI()->ID_File,tVol);
				}
				else
				{
					sprintf(LogBuf,"SaveEl ID=%I64x",ElT->GetIDI()->ID_File);
				}
				Srv->OutLog(LogBuf,this,TableName);
			}
			UnLockEl(&TmpAnchor,ElT);

		}
		else
		{
			Srv->SetCriticalError(this,"Error LockEl Write in SaveEl","Error",TableName);
		};
	}
	MyUnLock(__FILE__,__LINE__,&FlSim,this);
	return rc;
};
/*
int	IDTable::RepairEl(El_Tab* ElT)
{
	int rc=0;
	char*			tVol;
	ID_Elem64*		tElT=0;
	if ((ElT)&&((ElT->GetIDI()->ID_SeansFrom!=0)))
	{
		BLDel.DelElemID(ElT->GetIDI()->ID_File);
//		if (!BLDel.Find(ElT->GetIDI()->ID_File))
		//			if (IDL_Req)
		//			{
		LockEl(&TmpAnchor,ElT);
		if (MoLockEl(&TmpAnchor,ElT,AcsMd_Write)!=-1)
		{
			FindStrList8	FSL8;
			if (ElT->IDL64)
				tElT=(ID_Elem64*)ElT->IDL64->Begin(FSL8);
			else
				ElT->GetIDI()->ID_Standart=0;
			int CntChild=0;
			ElT->GetIDI()->Command=Command_REPAIR;
			ElT->GetIDI()->TypeIDI=TypeIDI_TABLE;
			ElT->GetIDI()->StateBlock=StateBlock_BEGIN;
			ElT->GetIDI()->NumPack=0;
			if (LastHost[0]==0)
				strcpy(LastHost,"*");
//			ElT->GetIDI()->pPList->SetParam(IDHostName,&LastHost,strlen(LastHost)+1,this);
			strcpy(ElT->GetIDI()->AdrInfo->DestinAI.HostName,LastHost);
			Srv->GetRealTime(&ElT->GetIDI()->TimeOper.FTime);
//			ElT->GetIDI()->pPList->DelParam(IDDomainName);
			ElT->GetIDI()->AdrInfo->DestinAI.DomainName[0]=0;
			while (tElT!=NULL)
			{
				El_Tab*	tEl;
				ElT->GetIDI()->ID_Standart=ElT->IDL64->NumElem;
				ElT->GetIDI()->Buf=(UCHAR*)Child;
				while ((tElT!=NULL)&&(CntChild<SizeChildBuf))
				{
					tEl=GetElTab(&TmpAnchor,tElT->Vol,1);
					if (tEl)
					{
						if (tEl->pParent!=ElT)
						{
							ElT->DelChild(tElT->Vol);
						}
						else
						{
							Child[CntChild++]=tElT->Vol;
						}
						UnLockEl(&TmpAnchor,tEl);
					}
					else
					{
						ElT->DelChild(tElT->Vol);
					};
					
					if (ElT->IDL64)
					{
//						tElT=(ID_Elem64*)ElT->IDL64->Find((UCHAR*)&tElT->Vol,sizeof(INT64),1);
						tElT=(ID_Elem64*)ElT->IDL64->Next(FSL8);
					}
				}
				ElT->GetIDI()->Cnt=CntChild*sizeof(INT64);
				if ((ElT->GetIDI()->Cnt!=0)&&(tElT!=NULL)) 
				{
					ElT->GetIDI()->NumPack++;
					Srv->ReplElTabl(ElT);
//					ODataIDI(3,ElT->GetIDI());
					ElT->GetIDI()->StateBlock=0;
				}
				CntChild=0;
			}
			ElT->GetIDI()->StateBlock|=StateBlock_END;
			Srv->GetRealTime(&ElT->GetIDI()->TimeOper.FTime);
			ElT->GetIDI()->NumPack++;
			ElT->GetIDI()->ID_SeansFrom=ID;
			Srv->ReplElTabl(ElT);
//			ODataIDI(3,ElT->GetIDI());
			if (Fl_Debug)
			{
				if (GetParam(&TmpAnchor,ElT,IDNameElTab,tVol)==0)
				{
					sprintf(LogBuf,"RepairEl to %s ID=%I64x Name=%s",LastHost,ElT->GetIDI()->ID_File,tVol);
				}
				else
				{
					sprintf(LogBuf,"RepairEl to %s ID=%I64x",LastHost,ElT->GetIDI()->ID_File);
				}
				Srv->OutLog(LogBuf,this,TableName);
			}

		}
		else
		{
			Srv->SetCriticalError(this,"Error LockEl Write in ReplEl","Error",TableName);
		};
		UnLockEl(&TmpAnchor,ElT);
		//			};
	}
	return rc;
}
*/
/*
int	IDTable::ReplElCombine(El_Tab* ElT,INT64 ID2)
{
	int rc=0;
	char*			tVol;
	ID_Elem64*		tElT=0;
	if ((ElT)&&((ElT->GetIDI()->ID_SeansFrom==0)||(ElT->GetIDI()->ID_SeansFrom!=TabParametr)))
	{
		if (!BLDel.Find(ElT->GetIDI()->ID_File))
		{
//			if (IDL_Req)
//			{
//				ID_Elem64*	tEl=(ID_Elem64*)IDL_Req->Find((UCHAR*)&ElT->GetIDI()->ID_File,sizeof(INT64));
//				if (tEl)
//				{
//					FileTime	tFT;
//					Srv->GetRealTime(&tFT.FTime);
//					if (tFT.Time-tEl->FT.Time>Ktime)
//						IDL_Req->Del(tEl);
//					else
//						return rc;
//				}
//				else
//				{
//					tEl=(ID_Elem64*)IDL_Req->NewElem();
//					if (tEl)
//						tEl->Init(ElT->GetIDI()->ID_File);
//					else
//						tEl=new ID_Elem64(ElT->GetIDI()->ID_File);
//					if (tEl)
//						IDL_Req->Add(tEl);
//				}
			LockEl(&Anchor,ElT);
				if (MoLockEl(&Anchor,ElT,AcsMd_Write)!=-1)
				{
					if (ElT->IDL64) tElT=(ID_Elem64*)ElT->IDL64->Begin();
					CntChild=0;
					ElT->GetIDI()->Command=Command_REPLICATION;
					ElT->GetIDI()->TypeIDI=TypeIDI_TABLE;
					ElT->GetIDI()->StateBlock=StateBlock_BEGIN;
					ElT->GetIDI()->NumPack=0;
					ElT->GetIDI()->pPList->SetParam(IDID,&ID2,sizeof(INT64),this);
					if (LastHost[0]==0)
						strcpy(LastHost,"*");
					ElT->GetIDI()->pPList->SetParam(IDHostName,&LastHost,strlen(LastHost)+1,this);
					Srv->GetRealTime(&ElT->GetIDI()->TimeOper.FTime);
					while (tElT!=NULL)
					{
						ElT->GetIDI()->Buf=(UCHAR*)Child;
						while ((tElT!=NULL)&&(CntChild<SizeChildBuf))
						{
							Child[CntChild++]=tElT->Vol;
							if (ElT->IDL64) tElT=(ID_Elem64*)ElT->IDL64->Next();
						}
						ElT->GetIDI()->Cnt=CntChild*sizeof(INT64);
						if ((ElT->GetIDI()->Cnt!=0)&&(tElT!=NULL)) 
						{
							ElT->GetIDI()->NumPack++;
							ODataFF(3,(UCHAR*)ElT->GetIDI(),sizeof(IDInfo));
							ElT->GetIDI()->StateBlock=0;
						}
						CntChild=0;
					}
					ElT->GetIDI()->StateBlock|=StateBlock_END;
					Srv->GetRealTime(&ElT->GetIDI()->TimeOper.FTime);
					ElT->GetIDI()->NumPack++;
					ElT->GetIDI()->ID_SeansFrom=ID;
					ODataFF(3,(UCHAR*)ElT->GetIDI(),sizeof(IDInfo));
					if (GetParam(&Anchor,ElT,IDNameElTab,tVol)==0)
					{
						sprintf(LogBuf,"CombineEl to %s ID=%I64x with ID=%I64x Name=%s",LastHost,ElT->GetIDI()->ID_File,ID2,tVol);
					}
					else
					{
						sprintf(LogBuf,"CombineEl to %s ID=%I64x with ID=%I64x",LastHost,ElT->GetIDI()->ID_File,ID2);
					}
					Srv->OutLog(LogBuf,this,TableName);
					UnLockEl(&Anchor,ElT);

				}
			};
		}
	}
	return rc;
};
*/

IDTable::~IDTable()
{
	char			tTB[256];
	sprintf(tTB,"%s\\DelList%I64d.BDL",Srv->MainPath,ID);
	SetFileAttributesA(tTB,GetFileAttributesA(tTB)&~FILE_ATTRIBUTE_READONLY);
	int FH=_open(tTB,O_CREAT|O_BINARY|O_RDWR);
	if (FH==-1)				
		FH=_creat(tTB,0);
	if (FH!=-1)
	{
//		BLDel.SaveEl();
//		BLDel.SaveToFile(FH,0);
		close(FH);
	}
	if (IDL->NumElem)
	{
		char*			tVol;
		El_Tab*			TeLt;
		FindStrList8	FSL8;
		TeLt=(El_Tab*)IDL->Begin(FSL8);
		while (TeLt)
		{
			TeLt->PreSave();
			if ((TeLt->OwnerList)&&(TeLt->OwnerList->NumElem))
			{
				MyLock(__FILE__,__LINE__,&TeLt->Fl_RE,this);
				FindStrList8	FSL82;
				El_Owner*	tDec=(El_Owner*)TeLt->OwnerList->Begin(FSL82);
				char*	Owner=NULL;
				if (tDec)
				{
					Owner=tDec->NameMt;
				}
				if (GetParam(&Anchor,TeLt,IDNameElTab,tVol)==0)
					sprintf(LogBuf,"Element=%s not UnLocked ID=%I64x Owner=%s",tVol,TeLt->GetIDI()->ID_File,Owner);
				else
					sprintf(LogBuf,"Element not UnLocked ID=%I64x Owner=%s",TeLt->GetIDI()->ID_File,Owner);
				Srv->OutLog(LogBuf,this,"IDTableErr");
				MyUnLock(__FILE__,__LINE__,&TeLt->Fl_RE,this);
			}
			TeLt=(El_Tab*)IDL->Next(FSL8);
		}
	}
	FStr_Root.ReInit();
	if (ElTbl)
	{
		ElTbl->Fl_Del=1;
		IDL->Del(ElTbl); ElTbl=NULL;
	}
	if (IDL) delete IDL; IDL=NULL;
	if (IDL_Req) delete IDL_Req; IDL_Req=NULL;
	if (IDL_Query) delete IDL_Query; IDL_Query=NULL;
	if (IDL_Test) delete IDL_Test; IDL_Test=NULL;
	if (pFSrv)
	{
		Srv->CloseUnit(pFSrv);
		Srv->DeleteDecoder(pFSrv);pFSrv=NULL;
	}

};
/*
		El_Owner::El_Owner(Decoder*	Obj): ID_Elem((UCHAR*)&PObj,sizeof(Decoder*))
{
	PObj=Obj;
	strcpy(NameMt,Obj->Name_Mt);
	Counter=0;
};

void	El_Owner::Init(Decoder*	Obj)
{
	PObj=Obj;
	strcpy(NameMt,Obj->Name_Mt);
	Counter=0;
}
*/
El_Owner::El_Owner(StrOwner*	Ow): ID_Elem((UCHAR*)&Own,sizeof(StrOwner))
{
	memcpy(&Own,Ow,sizeof(StrOwner));
	strcpy(NameMt,Own.pDec->Name_Mt);
	Counter=0;
};

void	El_Owner::Init(StrOwner*	Ow)
{
	memcpy(&Own,Ow,sizeof(StrOwner));
	strcpy(NameMt,Own.pDec->Name_Mt);
	Counter=0;
}

MethElem::MethElem(DLLCntrl	*DLLL,int ModID,char* MtName):ID_Elem((UCHAR*)Name,strlen(MtName))
{
	int	i;
	ModIndex=ModID;
	Srv=GetServices();
	memset(Name,0,sizeof(Name));
	strcpy(Name,MtName);
	pDec=(Decoder*)DLLL->pMod[ModIndex].FindNewDecod(Name,Srv);
	if (pDec!=NULL)
	{
		pDec->GetInfoMethod(&IMt);
		IMt.Name=ABuf.AddStr(IMt.Name);
		IMt.Auth=ABuf.AddStr(IMt.Auth);
		IMt.Rem=ABuf.AddStr(IMt.Rem);
		IMt.Ver=ABuf.AddStr(IMt.Ver);
		IMt.Type=ABuf.AddStr(IMt.Type);
		for (i=0;i<IMt.NPinI;i++)
		{
			pDec->GetInfoPinIn(i,&IPinI[i]);
			IPinI[i].Name=ABuf.AddStr(IPinI[i].Name);
			IPinI[i].Rem=ABuf.AddStr(IPinI[i].Rem);
		};
		for (i=0;i<IMt.NPinO;i++)
		{
			pDec->GetInfoPinOut(i,&IPinO[i]);
			IPinO[i].Name=ABuf.AddStr(IPinO[i].Name);
			IPinO[i].Rem=ABuf.AddStr(IPinO[i].Rem);
		};
		for (i=0;i<IMt.NParam;i++)
		{
			pDec->GetInfoParam(i,&IPrm[i]);
			IPrm[i].Name=ABuf.AddStr(IPrm[i].Name);
			IPrm[i].RemPrm=ABuf.AddStr(IPrm[i].RemPrm);
			for (int j=0;j<50;j++)
			{
				IPrm[i].Vol[j]=ABuf.AddStr(IPrm[i].Vol[j]);
				if (IPrm[i].Vol[j][0]==0) break;
				IPrm[i].Rem[j]=ABuf.AddStr(IPrm[i].Rem[j]);
			}
		};
//		if (strcmp(pDec->Type_Mt,"Unit")==0)
//			Srv->CloseUnit(pDec);
		DLLL->pMod[ModIndex].DeleteDecod((LPVOID)pDec); pDec=NULL;
	}
};

DLLCntrl::DLLCntrl(char* FPath)
{
size_t			done;	//,i;
HANDLE			FindHandle;
char			*NameMt;
WIN32_FIND_DATAA ffblk;
char			FullSrName[512];	
char			FullDstName[512];	
// ----------------
	CntMeth=0;
	MethElem*			tMeth;	
	Srv=GetServices();

	MethInf=new ID_List8();
	Anchor.Init(Srv,this,0);
#ifdef _WIN64
	sprintf(FullPath,"%s64",FPath);
#else
	strcpy(FullPath,FPath); 
#endif
// Ïðîâåðÿåì íåîáõîäèìîñòü îáíîâèòü DLL
	char lpBuffer[512];
	GetCurrentDirectoryA(512,lpBuffer);
	sprintf(FullName,"%s\\New",FullPath);
	if (SetCurrentDirectoryA(FullName))
	{
// Ïðåäâàðèòåëüíî äåëàåì BackUp
		sprintf(FullSrName,"%s\\BackUp3",FullPath);
		strcat(FullSrName,"\\*.*");
		FindHandle = FindFirstFileA(FullSrName,&ffblk);
		done=(size_t)FindHandle;
		while (done !=-1)
		{
			sprintf(FullDstName,"%s\\BackUp3\\%s",FullPath,ffblk.cFileName);
			DeleteFileA(FullDstName);
			if (FindNextFileA(FindHandle,&ffblk)) done=0;
			else done=-1;
		}
		FindClose(FindHandle);
		sprintf(FullSrName,"%s\\BackUp3",FullPath);
		RemoveDirectoryA(FullSrName);
		sprintf(FullSrName,"%s\\BackUp2",FullPath);
		sprintf(FullDstName,"%s\\BackUp3",FullPath);
		MoveFileA(FullSrName,FullDstName);
		sprintf(FullSrName,"%s\\BackUp1",FullPath);
		sprintf(FullDstName,"%s\\BackUp2",FullPath);
		MoveFileA(FullSrName,FullDstName);

		sprintf(FullSrName,"%s\\*.dll",FullPath);
		sprintf(FullDstName,"%s\\BackUp1",FullPath);
		CreateDirectoryA(FullDstName,NULL);
		FindHandle = FindFirstFileA(FullSrName,&ffblk);
		done=(int)FindHandle;
		while (done !=-1)
		{
			sprintf(FullSrName,"%s\\%s",FullPath,ffblk.cFileName);
			sprintf(FullDstName,"%s\\BackUp1\\%s",FullPath,ffblk.cFileName);
			CopyFileA(FullSrName,FullDstName,FALSE);
			if (FindNextFileA(FindHandle,&ffblk)) done=0;
			else done=-1;
		}
		FindClose(FindHandle);
// ----------------------------
		strcat(FullName,"\\*.dll");
		FindHandle = FindFirstFileA(FullName,&ffblk);
		done=(int)FindHandle;
		while (done !=-1)
		{
			sprintf(FullSrName,"%s\\New\\%s",FullPath,ffblk.cFileName);
			sprintf(FullDstName,"%s\\%s",FullPath,ffblk.cFileName);
			MoveFileExA(FullSrName,FullDstName,MOVEFILE_REPLACE_EXISTING);
			if (FindNextFileA(FindHandle,&ffblk)) done=0;
			else done=-1;
		}
		FindClose(FindHandle);
		SetCurrentDirectoryA(lpBuffer);
		sprintf(FullName,"%s\\New",FullPath);
		RemoveDirectoryA(FullName);
	};
// ----------------------------------------------
	tModSize=1000;
	rModSize=0;
	pMod=(Module*)malloc(tModSize*sizeof(Module));

	pMod[rModSize].ModuleHandle=(HINSTANCE)-1;
	pMod[rModSize].FindNewDecod=&FindNewDecoderMtIntDec;
	pMod[rModSize].FindFirst=&FindFirstIntDec;
	pMod[rModSize].FindNext=&FindNextIntDec;
	pMod[rModSize].DeleteDecod=&DeleteDecod;
	strcpy(pMod[rModSize].Name,"InternalDecoder");
	pMod[rModSize].Cnt=0;
	NameMt=pMod[rModSize].FindFirst();
	while(NameMt[0]!=0)
	{
		tMeth=new MethElem(this,rModSize,NameMt);
		MethInf->Add(tMeth);
		NameMt=pMod[rModSize].FindNext();
	};
	rModSize++;if (rModSize>=tModSize) {tModSize+=100;pMod=(Module*)realloc((void*)pMod,tModSize*sizeof(Module));};
// Ïîèñê DLL ôàéëîâ
   strcpy(FullName,FullPath);
   strcat(FullName,"\\*.dll");
   FindHandle = FindFirstFileA(FullName,&ffblk);
	done=(int)FindHandle;
   while (done !=-1)
    {
    strcpy_s(pMod[rModSize].Name,ffblk.cFileName);
//    for (i=0;i<255;i++) {if (pMod[rModSize].Name[i]=='.') break;};
//    pMod[rModSize].Name[i]=0x0;
	strcpy_s(FullMod,FullPath);
	strcat_s(FullMod,"\\");
	strcat_s(FullMod,pMod[rModSize].Name);
	HANDLE	FlMod=CreateFileA(FullMod,GENERIC_READ,FILE_SHARE_READ,NULL,OPEN_EXISTING,FILE_ATTRIBUTE_NORMAL,NULL);  
	GetFileTime(FlMod,&pMod[rModSize].CrTime.FTime,&pMod[rModSize].AcTime.FTime,&pMod[rModSize].WrTime.FTime);
	CloseHandle(FlMod);
	pMod[rModSize].ModuleHandle = LoadLibraryA(FullMod);
    if (pMod[rModSize].ModuleHandle!=NULL)
    {

		pMod[rModSize].FindNewDecod=(LPVOID(*)(char*,Services*)) GetProcAddress(pMod[rModSize].ModuleHandle,"FindNewDecoderMt");
		pMod[rModSize].DeleteDecod=(void(*)(LPVOID)) GetProcAddress(pMod[rModSize].ModuleHandle,"DeleteDecod");
		pMod[rModSize].FindFirst=(char*(*)()) GetProcAddress(pMod[rModSize].ModuleHandle,"FindFirst");
		if (pMod[rModSize].FindFirst==NULL) pMod[rModSize].FindFirst=(char*(*)()) GetProcAddress(pMod[rModSize].ModuleHandle,"FindFirst_");
		pMod[rModSize].FindNext=(char*(*)()) GetProcAddress(pMod[rModSize].ModuleHandle,"FindNext");
		if (pMod[rModSize].FindNext==NULL) pMod[rModSize].FindNext=(char*(*)()) GetProcAddress(pMod[rModSize].ModuleHandle,"FindNext_");
		if ((pMod[rModSize].FindNewDecod==NULL)||(pMod[rModSize].FindFirst==NULL)||(pMod[rModSize].FindNext==NULL)) 
			{FreeLibrary(pMod[rModSize].ModuleHandle);pMod[rModSize].ModuleHandle=(HINSTANCE)-1;goto _nn01;}	// Íå íàøà DLL
		pMod[rModSize].Cnt=0;
		// Âû÷èòûâàåì ñîäåðæèìîå ìîäóëÿ
		NameMt=pMod[rModSize].FindFirst();
		while(NameMt[0]!=0)
		{
			tMeth=new MethElem(this,rModSize,NameMt);
			if (MethInf->Add(tMeth)==-1)
			{
				delete tMeth;
			};
			NameMt=pMod[rModSize].FindNext();
		};

//		tMeth=(MethElem*)MethInf->Find((UCHAR*)"FSrv",strlen("FSrv"));

		// ----------------------------
		FreeLibrary(pMod[rModSize].ModuleHandle);pMod[rModSize].ModuleHandle=(HINSTANCE)-1;
		rModSize++;if (rModSize>=tModSize) {tModSize+=10;pMod=(Module*)realloc((void*)pMod,tModSize*sizeof(Module));};
_nn01:;
    }
	else
	{
		pMod[rModSize].ModuleHandle=(HINSTANCE)-1;
		rModSize++;if (rModSize>=tModSize) {tModSize+=10;pMod=(Module*)realloc((void*)pMod,tModSize*sizeof(Module));};
		done=GetLastError();
 	};

    if (FindNextFileA(FindHandle,&ffblk)) done=0;
	else done=-1;
   };
	FindClose(FindHandle);
	rModSize--;
	tMeth=0;ActModule=0;
};

DLLCntrl::~DLLCntrl()
{
int	i;
	if (CntMeth)
	{
		Beep(1000,1000);	//!!!!!!!
	}
	MethInf->DelAllElem();
	for (i=1;i<rModSize-1;i++)
	{
	  if (pMod[i].Cnt!=0)
	  {
		  {size_t iasm=1;}
	  }
//	  {
		if (pMod[i].ModuleHandle!=(HINSTANCE)-1)
		{
			FreeLibrary(pMod[i].ModuleHandle);pMod[rModSize].ModuleHandle=(HINSTANCE)-1;
		}
//	  }
	}
	free (pMod);
	delete	MethInf;
}


void	DLLCntrl::Init()
{
	char*	tNameMt;
	int		i;
	FindStrList8	FSL8;
	tNameMt=FirstMeth(FSL8);
	while (tNameMt!=NULL)
	{
		Decoder*	tDec;
		tDec=Srv->NewDecod(tNameMt,Srv);
		if (tDec)
		{
		 	tDec->Initialization();
			Srv->DeleteDecoder(tDec);
		}
		tNameMt=NextMeth(FSL8);
	}
	while (Srv->IDT_NeuronGlobal->GetNumAllList())
	{
		Srv->CalcTable();
		Srv->IDT_NeuronGlobal->IDL_Test->DelAllEl();
	};
	while (Srv->IDT_NeuronLocalCustomer->GetNumAllList())
	{
		Srv->CalcTable();
		Srv->IDT_NeuronLocalCustomer->IDL_Test->DelAllEl();
	};
	while (Srv->IDT_NeuronLocalSite->GetNumAllList())
	{
		Srv->CalcTable();
		Srv->IDT_NeuronLocalSite->IDL_Test->DelAllEl();
	};
	if (Srv->NetCommLAN)
		Srv->NetCommLAN->StopEx();
	Srv->IDT_NeuronGlobal->DelAllTables();
	Srv->IDT_NeuronLocalCustomer->DelAllTables();
	Srv->IDT_NeuronLocalSite->DelAllTables();
// 	Srv->IDT_NeuronLocalSite->IDL_Query->DelAllElem();
// 	Srv->IDT_NeuronLocalSite->IDL_Test->DelAllEl();
// 	Srv->IDT_NeuronLocalSite->IDL->DelAllElem();Srv->IDT_NeuronLocalSite->SetZRoot();	// !!!!!!!!!!!!!!!!!!!!!!!!!!!
	Srv->IDT_NeuronGlobal->GetElTab(&Srv->Anchor,TabNeuronGlobal);
	Srv->IDT_NeuronLocalCustomer->GetElTab(&Srv->Anchor,TabNeuronLocalCustomer);
	Srv->IDT_NeuronLocalSite->GetElTab(&Srv->Anchor,TabNeuronLocalSite);
//	Srv->IDT_Prm->DelAllTables();
// 	Srv->IDT_Prm->IDL_Query->DelAllElem();
// 	Srv->IDT_Prm->IDL->DelAllElem();Srv->IDT_Prm->SetZRoot();	// !!!!!!!!!!!!!!!!!!!!!!!!!!!
//	Srv->IDT_Prm->GetElTab(&Srv->Anchor,TabParametr);
	for (i=1;i<rModSize-1;i++)
	{
		if (pMod[i].Cnt==0)
			if (pMod[i].ModuleHandle!=(HINSTANCE)-1)
			{
				FreeLibrary(pMod[i].ModuleHandle);pMod[i].ModuleHandle=(HINSTANCE)-1;
			}
	};

};


//Decoder*	DLLCntrl::NewDecod(char* Name)
//{
//	Decoder*	PDec=NULL;
//	if (SelectMeth(Name)!=0)
//	{
//		PDec=NewDecod();
//	}
//	return	PDec;
//};

Decoder*	DLLCntrl::NewDecod(char* Name,Decoder* p_Dec)
{
	char		ttB[128];
	Decoder*	PDec=NULL;
	MethElem*	tMeth;
	int			ModIndex;		 
	char		NameMt[40];//memset(NameMt,0,sizeof(NameMt));
	strcpy(NameMt,Name);
	tMeth=(MethElem*)MethInf->Find((UCHAR*)NameMt,strlen(NameMt));
	if (tMeth!=0)
	{
		MyLock(__FILE__,__LINE__,&Fl_RE,NULL);
		ModIndex=tMeth->ModIndex;
		if ((ModIndex!=0)&&(pMod[ModIndex].Cnt==0))
		{
			strcpy(FullMod,FullPath);
			strcat(FullMod,"\\");
			strcat(FullMod,pMod[ModIndex].Name);
			if (pMod[ModIndex].ModuleHandle==(HINSTANCE)-1)
			{
				HANDLE	FlMod=CreateFileA(FullMod,GENERIC_READ,FILE_SHARE_READ,NULL,OPEN_EXISTING,FILE_ATTRIBUTE_NORMAL,NULL);  
				GetFileTime(FlMod,&pMod[ModIndex].CrTime.FTime,&pMod[ModIndex].AcTime.FTime,&pMod[ModIndex].WrTime.FTime);
				CloseHandle(FlMod);
				pMod[ModIndex].ModuleHandle = LoadLibraryA(FullMod);
				if (pMod[ModIndex].ModuleHandle!=NULL)
				{
					pMod[ModIndex].FindNewDecod=(LPVOID(*)(char*,Services*)) GetProcAddress(pMod[ModIndex].ModuleHandle,"FindNewDecoderMt");
					pMod[ModIndex].DeleteDecod=(void(*)(LPVOID)) GetProcAddress(pMod[ModIndex].ModuleHandle,"DeleteDecod");
					pMod[ModIndex].FindFirst=(char*(*)()) GetProcAddress(pMod[ModIndex].ModuleHandle,"FindFirst");
					if (pMod[ModIndex].FindFirst==NULL) pMod[ModIndex].FindFirst=(char*(*)()) GetProcAddress(pMod[ModIndex].ModuleHandle,"FindFirst_");
					pMod[ModIndex].FindNext=(char*(*)()) GetProcAddress(pMod[ModIndex].ModuleHandle,"FindNext");
					if (pMod[ModIndex].FindNext==NULL) pMod[ModIndex].FindNext=(char*(*)()) GetProcAddress(pMod[ModIndex].ModuleHandle,"FindNext_");
					if ((pMod[ModIndex].FindNewDecod==NULL)||(pMod[ModIndex].FindFirst==NULL)||(pMod[ModIndex].FindNext==NULL)) 
						{
//							while (TRUE)
								Beep(1000,1000); 
						}	// Íå íàøà DLL
				}
				else
				{	// DLL îòñóòñòâóåò
					pMod[ModIndex].ModuleHandle =	(HINSTANCE)-1;
					// Flanker
// 					while (TRUE)
// 						Beep(1000,1000); 
					return	NULL;
				}
			}
		}
//		if (pMod[ModIndex].Cnt!=-1)
		if (pMod[ModIndex].FindNewDecod)
			PDec=(Decoder*)pMod[ModIndex].FindNewDecod(tMeth->Name,Srv);
		if	(PDec)
		{
			PDec->ParentMt=p_Dec;
			p_Dec->AddChildMt(PDec);
			strcpy(PDec->UInfo.DomainName,p_Dec->UInfo.DomainName);
			PDec->UInfo.TractID=p_Dec->UInfo.TractID;
			pMod[ModIndex].Cnt++;
			CntMeth++;
			sprintf(ttB,"New:%s Module:%s",Name,pMod[ModIndex].Name);
			Srv->OutLog(ttB,NULL,"DLLCntr");
		}
		MyUnLock(__FILE__,__LINE__,&Fl_RE,NULL);
	}
//	if	(PDec)
//		PDec->Initialization();

	if ((PDec)&&(strcmp(PDec->Type_Mt,"Unit")==0))
	{
		El_Tab*	ETbl=NULL;
		if (Srv->System_On==1)
		{
			Srv->InitUnit(PDec);
			ETbl=Srv->IDT_NeuronLocalSite->FindNested(&Anchor,AcsMd_Read,N_Domain,Srv->MainDomainName,N_Host,Srv->ShortHostName,N_Unit,PDec->Name_Mt,NULL);
// 			if (ETbl==NULL)
// 				ETbl=Srv->IDT_NeuronLocalSite->AttainNested(&Anchor,"%s%s%s%s%s",N_Domain,Srv->MainDomainName,N_Host,Srv->ShortHostName,N_Unit,PDec->Name_Mt,-1);
			if (ETbl!=NULL)
			{
//				Srv->IDT_NeuronLocalSite->AddParam(&Anchor,ETbl,IDState,VolumeProcessed[IDVolProc_Off]);
				Srv->IDT_NeuronLocalSite->UnLockEl(&Anchor,ETbl);
			}
		}
		else
			Srv->InitUnit(PDec);

	}
	return	PDec;
};

MethElem	*DLLCntrl::FindMeth(char* Name1)
{
	char	NameMt[40];//memset(NameMt,0,sizeof(NameMt));
	strcpy(NameMt,Name1);
	return (MethElem*)MethInf->Find((UCHAR*)NameMt,strlen(NameMt));
};

void		DLLCntrl::DeleteDecoder(Decoder* pDec)
{
	MethElem*	tMeth;			
	char		ttB[128];
	char		NameMt[40];
	int			ModIndex;
	if (pDec)
	{
		pDec->StopEx();
		if (pDec->ParentMt)
			pDec->ParentMt->DelChildMt(pDec);
		if (strcmp(pDec->Type_Mt,"Unit")==0)
			Srv->CloseUnit(pDec);
	//	memset(NameMt,0,sizeof(NameMt));
		strcpy(NameMt,pDec->Name_Mt);
		tMeth=(MethElem*)MethInf->Find((UCHAR*)NameMt,strlen(NameMt));
//		MyLock(__FILE__,__LINE__,&Fl_RE,NULL);
		if (tMeth)
		{
			ModIndex = tMeth->ModIndex;
			pMod[ModIndex].DeleteDecod((LPVOID)pDec);
			CntMeth--;
	//		MyUnLock(__FILE__,__LINE__,&Fl_RE,NULL);
			sprintf(ttB,"Del:%s Module:%s",NameMt,pMod[ModIndex].Name);
			Srv->OutLog(ttB,NULL,"DLLCntr");
			if (pMod[ModIndex].Cnt>0)
			{
				--pMod[ModIndex].Cnt;
				if (pMod[ModIndex].Cnt==0)
				{
		//			FreeLibrary(pMod[ModIndex].ModuleHandle); pMod[ModIndex].ModuleHandle=(HINSTANCE)-1;
				}
			}
			else
			{
				{size_t iasm=1;}
			};
		}
	}
};

char*		DLLCntrl::FirstModule()
{
	char*	pBuf=NULL;
	ActModule=0;
	pBuf=pMod[ActModule].Name;
	return	pBuf;
};

char*		DLLCntrl::NextModule()
{
	char*	pBuf=NULL;
	if (ActModule<rModSize)
	{
		ActModule++;
		pBuf=pMod[ActModule].Name;
	}
	return	pBuf;
};

Module*		DLLCntrl::FindModule(char* Name)
{
Module*	rpMod=NULL;
Module*	tpMod=pMod;
int		rMS=rModSize;
	tpMod++;
	while(rMS)
	{
		if (strcmp(tpMod->Name,Name)==0)
		{
			rpMod=tpMod;
			break;
		}
		rMS--;tpMod++;
	}
	return	rpMod;
};

char*		DLLCntrl::FirstMeth(FindStrList8	&FSL8)
{
	char*	pBuf=NULL;
	MethElem* tDec=(MethElem*)MethInf->Begin(FSL8);
	if (tDec!=NULL)
		pBuf=tDec->Name;
	return	pBuf;
};

char*		DLLCntrl::NextMeth(FindStrList8	&FSL8)
{
	char*	pBuf=NULL;
	MethElem* tDec=(MethElem*)MethInf->Next(FSL8);
	if (tDec!=NULL)
		pBuf=tDec->Name;
	return	pBuf;
};

char*		DLLCntrl::GetModuleName(char* NameMt)
{
	char*	pBuf=NULL;
	MethElem*	tMeth;
	char	Name[40];
	int		ModIndex;
//	memset(Name,0,sizeof(Name));
	strcpy(Name,NameMt);

	tMeth=(MethElem*)MethInf->Find((UCHAR*)Name,strlen(Name));
	if (tMeth!=NULL) 
	{
		ModIndex=tMeth->ModIndex;
		pBuf=pMod[ModIndex].Name;
	}
	return	pBuf;
};
/*
char*		DLLCntrl::GetMethName()
{
	char*	pBuf=NULL;
	if (tMeth!=0) pBuf=tMeth->Name;
	return	pBuf;
};
*/
int			DLLCntrl::GetInfoMethod(InfoMt* InfM,char* NameMt)
{
	int	rc=0;
	MethElem*	tMeth=NULL;
	char	Name[40];
//	memset(Name,0,sizeof(Name));
	strcpy(Name,NameMt);
	tMeth=(MethElem*)MethInf->Find((UCHAR*)Name,strlen(Name));
	if (tMeth!=NULL)
		memcpy(InfM,&tMeth->IMt,sizeof(InfoMt));
	else
		rc=-1;
	return rc;
};

int			DLLCntrl::GetInfoPinIn(int Num,InfoPin* InfP,char* NameMt)
{
	int	rc=0;
	MethElem*	tMeth;
	char	Name[40];
//	memset(Name,0,sizeof(Name));
	strcpy(Name,NameMt);

	tMeth=(MethElem*)MethInf->Find((UCHAR*)Name,strlen(Name));
	if (tMeth!=NULL)
	{
		if (Num<tMeth->IMt.NPinI)
			memcpy(InfP,&tMeth->IPinI[Num],sizeof(InfoPin));
		else rc=-1;
	}
	else rc=-1;
	return rc;
};

int			DLLCntrl::GetInfoPinOut(int Num,InfoPin* InfP,char* NameMt)
{
	int	rc=0;
	MethElem*	tMeth;
	char	Name[40];
//	memset(Name,0,sizeof(Name));
	strcpy(Name,NameMt);

	tMeth=(MethElem*)MethInf->Find((UCHAR*)Name,strlen(Name));
	if (tMeth!=NULL)
	{
		if (Num<tMeth->IMt.NPinO)
			memcpy(InfP,&tMeth->IPinO[Num],sizeof(InfoPin));
		else rc=-1;
	}
	else rc=-1;
	return rc;
};

int			DLLCntrl::GetInfoParam(int Num,InfoPrm* InfP,char* NameMt)
{
	int	rc=0;
	MethElem*	tMeth;
	char	Name[40];
//	memset(Name,0,sizeof(Name));
	strcpy(Name,NameMt);
	tMeth=(MethElem*)MethInf->Find((UCHAR*)Name,strlen(Name));
	if (tMeth!=NULL) memcpy(InfP,&tMeth->IPrm[Num],sizeof(InfoPrm));    else rc=-1;
	return rc;
};



AddBuf::AddBuf()
{
	Buf=(size_t*)malloc(sizeof(size_t)); Size=0;
};

AddBuf::~AddBuf() 
{
	if (Buf!=NULL)
	{
		for (size_t i=0;i<Size;i++)
		{
			free((void*)Buf[i]);
		}
		free(Buf);
	}
};

char* AddBuf::AddStr(char* Str)	// Êîïèðóåò â áóôåð è âîçâðàùàåò óêàçàòåëü íà íåãî 
{
	char*	Poi;
	if(Size!=0)	Buf=(size_t*)realloc(Buf,(Size+1)*sizeof(size_t));
	Buf[Size]=(size_t)malloc(strlen(Str)+1);
	Poi=(char*)Buf[Size];
	Size++;
	strcpy (Poi,Str);
	return Poi;
};

static char	*ListNameMt[]={
	"Invertor",
	"SubTr",
	"ConcBuf",
//	"MReplicator",
//	"IDI_MReplicator",
	"MultThrdMt",
	"NullMt",
	""};

int	plCnt=0;

LPVOID	FindNewDecoderMtIntDec(char* Name,Services* Srv)
{
	LPVOID RC=0;
	SetServices(Srv);
	if (strcmp(Name,"Invertor")==0) { RC=(LPVOID) new Invertor(); goto _mm1; };
	if (strcmp(Name,"ConcBuf")==0) { RC=(LPVOID) new ConcBuf(); goto _mm1; };
//	if (strcmp(Name,"MReplicator")==0) { RC=(int) new MReplicator(); goto _mm1; };
//	if (strcmp(Name,"IDI_MReplicator")==0) { RC=(int) new IDI_MReplicator(); goto _mm1; };
	if (strcmp(Name,"MultThrdMt")==0) { RC=(LPVOID) new MultThrdMt(); goto _mm1; };
	if (strcmp(Name,"SubTr")==0) { RC=(LPVOID) new SubTr(GetServices()); goto _mm1; };
	if (strcmp(Name,"NullMt")==0) { RC=(LPVOID) new NullMt(); goto _mm1; };
_mm1:
	return RC;
};
// Íåèçìåííàÿ ÷àñòü
char*	FindFirstIntDec()
{
	plCnt=0;
	return ListNameMt[plCnt];
};

char*	FindNextIntDec()
{
	if (ListNameMt[plCnt]!=0) plCnt++;
	return ListNameMt[plCnt];
};

Pack_Dec::Pack_Dec(Services *pSrv,UCHAR Host):ConstrDecF(Host)
			{
				Srv=pSrv;
//				Service2=Srv;
//				DLLCtl=DLLC;
//				ModuleHandle=HModule;
//				Fun=FFun;
			};

void	Pack_Dec::DelDecod(Decoder* PDec)
{
	if (PDec)
	{
		if (strcmp(PDec->Type_Mt,"Unit")==0)
			Srv->CloseUnit(PDec);
	if (strcmp(PDec->Name_Mt,"SubTr")==0)
	{
		DelChildMt(PDec);
		delete	PDec;
	}
	else
		Srv->DeleteDecoder(PDec);
	Srv->DelMt4Tlm(PDec);
	}
};
void	Pack_Dec::NewDecod(int	N_Mt)
{
if (strcmp(Str.Mt[N_Mt].Name,"SubTr")==0)
{
	Str.Mt[N_Mt].PDec=new SubTr(Srv/*DLLCtl,TPUDec*/);
	Str.Mt[N_Mt].PDec->ParentMt=this;
	AddChildMt(Str.Mt[N_Mt].PDec);
	strcpy(Str.Mt[N_Mt].PDec->UInfo.DomainName,UInfo.DomainName);
	Str.Mt[N_Mt].PDec->UInfo.TractID=UInfo.TractID;
	Str.Mt[N_Mt].PDec->UInfo.NumChan=UInfo.NumChan;
	goto _Ex_001;
}
// Ïîèñê â DLL
//int j;
	if ((Str.Mt[N_Mt].PDec=Srv->NewDecod(Str.Mt[N_Mt].Name,this))!=0) goto _Ex_001;
ErrorConstr|=1;

_Ex_001:
	
// Óñòàíîâèòü âñå èçìåíèâøèåñÿ ïàðàìåòðû
	if (ErrorConstr==0)
	{
//		Str.Mt[N_Mt].PDec->FlRSim=&FlSim;
		strcpy(Str.Mt[N_Mt].PDec->UInfo.DomainName,UInfo.DomainName);
		Str.Mt[N_Mt].PDec->UInfo.TractID=UInfo.TractID;
		Str.Mt[N_Mt].PDec->UInfo.NumChan=UInfo.NumChan;
		for (size_t i=0;i<Str.Max_Pr;i++)
		  if (Str.Pr[i].ID_Mt==N_Mt)
		   Str.Mt[N_Mt].PDec->ParamEx(Str.Pr[i].Name,Str.Pr[i].Vol);
		if (strcmp(Str.Mt[N_Mt].PDec->Type_Mt,"Unit")==0)
			Srv->InitUnit(Str.Mt[N_Mt].PDec);
	}
};


SubTr::SubTr(Decoder *pDec)	//DLLCntrl* DLLC,UDec* UD)
{
// Ýòîò ôðàãìåíò äîëæåí áûòü âñòàâëåí âî âñå ïîñëåäóþùèå îáúåêòû
		 Name_Mt=SubTr_NameMt;Rem_Mt=SubTr_RemMt;Ver_=SubTr_Version;Auth_=SubTr_Author;
		 PinName_I=SubTr_PinNameI;PinName_O=SubTr_PinNameO;Prm_=SubTr_Prm;Vol_=SubTr_Vol;
// -------------------------------------------------------------

		 Num_PinI=0;while (PinName_I[2*Num_PinI][0]!=0x0) Num_PinI++;
		 Num_PinO=0;while (PinName_O[2*Num_PinO][0]!=0x0) Num_PinO++;
		 Num_Prm=0;while (Prm_[2*Num_Prm][0]!=0x0) Num_Prm ++;
// -------------------------------------------------------------
//ModuleHandle=HModule;
//Fun=FFun;
if (pDec)
{
	Srv=pDec->Srv;
	strcpy(UInfo.DomainName,pDec->UInfo.DomainName);
	UInfo.TractID=pDec->UInfo.TractID;
}
//if (Srv!=NULL)
//	DLLCtl=pSrv->DLLL;
//TPUDec=pSrv->Tsk;
ModeName[0]=0;

//rdNIDI=0;
Fl_Start=0;
};

int SubTr::IdleFunc(int NumberTr)			// Idle function  if rc!=0 recall function (?) 
{
	int	rc=ChildIdleFunc(NumberTr);
	DataFF(6,(uchar*)&rc,1);
	return rc;
}

void	SubTr::Start()
{
	Fl_Start=1;
	if (ErrorConstr==0)
	 for (size_t i2=0;i2<Str.Max_Mt;i2++)
	   if (Str.Mt[i2].PDec!=NULL)
		{
			Str.Mt[i2].State=0;
			Str.Mt[i2].PDec->StartEx();
		};
};

void	SubTr::Stop()
{ 
	Fl_Start=0;
	if (ErrorConstr==0)
	 for (size_t i=0;i<Str.Max_LP;i++)
	   if (Str.LP[i].ID_Mt1==-1)
		  StepStop(Str.LP[i].ID_Mt2);
	//   else break;
	Act_N_Mt=-1;
};


void	SubTr::Param(char* Prm,char* Vol)
{
int	i;
	if (Srv)
	{
		if (strcmp(Prm,"NameTrackt")==0)
		{
			if (strcmp(ModeName,Vol)!=0)
			{
				strcpy(ModeName,Vol);
				// Ïîèñê â ñòðóêòóðå TPUDec óêàçàííîãî ðåæèìà
				for (i=0;i<Srv->Tsk->Max_Md;i++)
				 if (strcmp(Srv->Tsk->Md[i].Name,Vol)==0)
				 {
				   ReStruct(Srv->Tsk,i,UInfo.TractID);
				   break;
				 };
				if (Fl_Start==1)
					Start();
			}
		}
		else
		{
			if (ErrorConstr==0)
			 for (size_t i3=0;i3<Str.Max_Mt;i3++)
			   if (Str.Mt[i3].PDec!=NULL)
				Str.Mt[i3].PDec->ParamEx(Prm,Vol);
		}
	}
};


int SubTr::DataEx(int N_Pin,UCHAR* pmas,size_t Cnt)
{
	int	rc;
	MyLock(__FILE__,__LINE__,&FlSim,this);
	rc=Data(N_Pin,pmas,Cnt);
	MyUnLock(__FILE__,__LINE__,&FlSim,this);
	return rc;
};

int SubTr::DataIDIEx(			// Äàííûå äëÿ äåêîäèðîâàíèÿ
	int N_Pin,		// Íîìåð âõîäíîé íîæêè
	IDInfo* pIDI)		// Äëèíà äàííûõ
{
	int	rc;
	MyLock(__FILE__,__LINE__,&FlSim,this);
	rc=DataIDI(N_Pin,pIDI);
	MyUnLock(__FILE__,__LINE__,&FlSim,this);
	return rc;
}

int SubTr::DataFFEx(int N_Pin,UCHAR* pmas,size_t Cnt)
{
	int	rc;
	MyLock(__FILE__,__LINE__,&FlSim,this);
	rc=DataFF(N_Pin,pmas,Cnt);
	MyUnLock(__FILE__,__LINE__,&FlSim,this);
	return rc;
};


int SubTr::OutDataEx(int N_Pin,UCHAR* &pmas,size_t &Cnt)
{
	int	rc;
	MyLock(__FILE__,__LINE__,&FlSim,this);
	rc=OutData(N_Pin,pmas,Cnt);
	MyUnLock(__FILE__,__LINE__,&FlSim,this);
	return rc;
};



	ConcBuf::ConcBuf()
{
// Ýòîò ôðàãìåíò äîëæåí áûòü âñòàâëåí âî âñå ïîñëåäóþùèå îáúåêòû
// ConcBuf_ äîëæåí áûòü çàìåíåí íà èìÿ îáúåêòà
		Name_Mt=ConcBuf_NameMt;Rem_Mt=ConcBuf_RemMt;Ver_=ConcBuf_Version;Auth_=ConcBuf_Author;
		PinName_I=ConcBuf_PinNameI;PinName_O=ConcBuf_PinNameO;Prm_=ConcBuf_Prm;Vol_=ConcBuf_Vol;

		Num_PinI=0;while (PinName_I[2*Num_PinI][0]!=0x0) Num_PinI++;
		Num_PinO=0;while (PinName_O[2*Num_PinO][0]!=0x0) Num_PinO++;
		Num_Prm=0;while (Prm_[2*Num_Prm][0]!=0x0)Num_Prm ++;
// -------------------------------------------------------------
		Fl_Stop=1;Fl_Data=0;
		RSize=0;
		BSize=2048;
		OBuf=(UCHAR*)malloc(BSize);
};
	ConcBuf::~ConcBuf()
{
		free(OBuf);
};

void	ConcBuf::Param(char* Prm,char* Vol)
{
};

void	ConcBuf::Start()
{
	Fl_Stop=0;
};

void	ConcBuf::Stop()
{
	Fl_Stop=1;
};

int	ConcBuf::Data(int N_Pin,UCHAR* pmas,size_t Cnt)
{
	size_t	OCnt=0;
	if (Fl_Stop!=1)
	{
		Fl_Data=1;
		if ((RSize+Cnt)>=BSize)
		{
			BSize=((RSize+Cnt)/2048+1)*2048;
			OBuf=(UCHAR*)realloc((void*)OBuf,BSize);
		}
		memcpy(OBuf+RSize,pmas,Cnt);
		RSize+=Cnt;
		OCnt=RSize;
	};
return OCnt;
};

int	ConcBuf::OutData(int N_Pin,UCHAR* &pmas,size_t &Cnt)
{
	Cnt=0;
	if (Fl_Data!=0)
	{
		pmas=OBuf;
		Cnt=RSize;
		RSize=0;
		Fl_Data=0;
	}
return Cnt;
};

void	NData(int NPin,UCHAR* pmas,size_t Cnt,LPVOID Poi){};

ConstrDec::ConstrDec()
{
strcpy(Name_Mt,"ConstrDec");
Srv=GetServices();CE1=0;CE2=0;CE3=0;
Num_PinI=1;NumStep=0;
Num_PinO=10;
TPinO=0;
TPUDec=0;
OData=&NData;
//Srv=0;
for (size_t i=0;i<65536;i++)
	TabID[i]=0;
CntID=0;
Tthis=(LPVOID)this;
Act_N_Md=-1;
Act_N_Mt=-1;
ID_Host=0;
//FlagRestruct=0;
memset(&Str,0x0,sizeof(WUDec));

NC1=GetNClk();NC2=GetNClk();
nClk=0.0;nFClk=0.0;
//Chron=new Chronometry (this);
m_pItem.iItem=0;
m_pItem.iSubItem=0;
m_pItem.mask=LVIF_IMAGE;
kImage=0;
Path[0]=0;
PathOut[0]=0;
};
void	ConstrDec::NewDecod(int	N_Mt)
{
	ErrorConstr|=1;
};

ConstrDec::ConstrDec(UCHAR Host)
{
strcpy(Name_Mt,"ConstrDec");
Srv=GetServices();CE1=0;CE2=0;CE3=0;
Num_PinI=1;NumStep=0;
Num_PinO=10;
TPinO=0;
TPUDec=0;
OData=&NData;
//Srv=0;
for (size_t i=0;i<65536;i++)
	TabID[i]=0;
CntID=0;
Tthis=(LPVOID)this;
Act_N_Md=-1;
Act_N_Mt=-1;
ID_Host=Host;
//FlagRestruct=0;
memset(&Str,0x0,sizeof(WUDec));
NC1=GetNClk();NC2=GetNClk();
nClk=0.0;nFClk=0.0;
RecurseFl=0;
//Chron=new Chronometry (this);
m_pItem.iItem=0;
m_pItem.iSubItem=0;
m_pItem.mask=LVIF_IMAGE;
kImage=0;
Path[0]=0;
PathOut[0]=0;
};

void	ConstrDec::DelDecod(Decoder* PDec)
{
	if (PDec)
	{
		if (strcmp(PDec->Type_Mt,"Unit")==0)
			Srv->CloseUnit(PDec);
	};
	if (strcmp(PDec->Name_Mt,"SubTr")==0)
		delete	PDec;
	else
		Srv->DeleteDecoder(PDec);
	Srv->DelMt4Tlm(PDec);
};

ConstrDec::~ConstrDec()
{
//FlagRestruct=1;
// Destruct Old Md[Mt]
Stop();
 for (size_t i1=0;i1<Str.Max_Mt;i1++)
   if (Str.Mt[i1].PDec!=NULL)
    DelDecod(Str.Mt[i1].PDec);
//delete Chron;
};

void	ConstrDec::Start()
{

//while (FlagRestruct==1)
//	DosSleep(100);
if (ErrorConstr==0)
 for (size_t i2=0;i2<Str.Max_Mt;i2++)
   if (Str.Mt[i2].PDec!=NULL)
    {
//    if (Tthis!=(LPVOID)this)  DosBeep(1000,20000);
		Str.Mt[i2].State=0;
		Str.Mt[i2].PDec->StartEx();
    };
};

void	ConstrDec::Param(char* Prm,char* Vol)
{
if (strcmp(Prm,"Path")==0)
{
	strcpy(Path,Vol);
}
if (strcmp(Prm,"PathOut")==0)
{
	strcpy(PathOut,Vol);
}
if (ErrorConstr==0)
 for (size_t i3=0;i3<Str.Max_Mt;i3++)
   if (Str.Mt[i3].PDec!=NULL)
    Str.Mt[i3].PDec->ParamEx(Prm,Vol);
};

void	ConstrDec::DirectParam(char* FullPrm,char* Vol) // Èçìåíèòü ïàðàìåòð
{
char    *NameMd=0;
char    *NameMt=0;
char    *ExtMt=0;
char    *PrmMt=0;
int 	i=0;
if (ErrorConstr!=0) return;

if (FullPrm[0]==':')
  {
  NameMd=FullPrm+1;i++;
  while ((FullPrm[i]!=0)&&(FullPrm[i]!=':'))
      {
      if (FullPrm[i]=='\\')	// Èìÿ ìåòîäà
	{
	FullPrm[i]=0;
	NameMt=FullPrm+i+1;
	};
      if (FullPrm[i]=='@')	// Ðàñøèðåíèå
	{
	FullPrm[i]=0;
	ExtMt=FullPrm+i+1;
	};
      if (FullPrm[i]=='#')	// Ïàðàìåòð
	{
	FullPrm[i]=0;
	PrmMt=FullPrm+i+1;
	};
      i++;
      };
  }
else
  {
  NameMt=FullPrm;
  while ((FullPrm[i]!=0)&&(FullPrm[i]!=':'))
      {
      if (FullPrm[i]=='@')	// Ðàñøèðåíèå
	{
	FullPrm[i]=0;
	ExtMt=FullPrm+i+1;
	};
      if (FullPrm[i]=='#')	// Ïàðàìåòð
	{
	FullPrm[i]=0;
	PrmMt=FullPrm+i+1;
	};
      i++;
      };
  };
// Ðàçáîð
FullPrm[i]=0;
if (NameMd!=0) if (strcmp(Str.Md.Name,NameMd)!=0) return;
if (PrmMt==0)
  {
  for (size_t i3=0;i3<Str.Max_Mt;i3++)
   if (Str.Mt[i3].PDec!=NULL)
    if (strcmp(Str.Mt[i3].PDec->Name_Mt,"SubTr")==0) // äîëæíî ñîâïàñòü ñ èìåíåì â PackDec
     {
     if (ExtMt==0)
      {FullPrm[i]=':';((ConstrDec*)Str.Mt[i3].PDec)->DirectParam(FullPrm+i,Vol);}
     else if (Str.Mt[i3].PDec->Ext_Mt==atoi(ExtMt))
	   {FullPrm[i]=':';((ConstrDec*)Str.Mt[i3].PDec)->DirectParam(FullPrm+i,Vol);};
     };
  }
else
  {
  for (size_t i3=0;i3<Str.Max_Mt;i3++)
   if (Str.Mt[i3].PDec!=NULL)
    if (strcmp(Str.Mt[i3].PDec->Name_Mt,NameMt)==0)
     {
     if (ExtMt==0) Str.Mt[i3].PDec->ParamEx(PrmMt,Vol);
     else if (Str.Mt[i3].PDec->Ext_Mt==atoi(ExtMt))
	      Str.Mt[i3].PDec->ParamEx(PrmMt,Vol);
     };
  };
};

void	ConstrDec::Stop()
{
/*
for (size_t i4=0;i4<Str.Max_Mt;i4++)
   if (Str.Mt[i4].PDec!=NULL)
    {
//    if (Tthis!=(LPVOID)this)  DosBeep(1000,20000);
    Str.Mt[i4].PDec->Stop();
    };
*/
//while (FlagRestruct==1)
//	DosSleep(100);
if (ErrorConstr==0)
 for (size_t i=0;i<Str.Max_LP;i++)
   if (Str.LP[i].ID_Mt1==-1)
      StepStop(Str.LP[i].ID_Mt2);
//   else break;
Act_N_Mt=-1;
return;
};

void	ConstrDec::StepStop(int ID_Mt)
{
Act_N_Mt=ID_Mt;
if (Str.Mt[ID_Mt].State!=0) return;
StepCalcStop(ID_Mt);
for (size_t i5=0;i5<Str.Mt[ID_Mt].N_PO;i5++)
  for (size_t j=0;j<Str.Max_LP;j++)
    if (Str.LP[j].ID_Mt1==ID_Mt)
     if ((Str.LP[j].PO==i5)&&(Str.LP[j].ID_Mt2!=-1))
       StepStop(Str.LP[j].ID_Mt2);
return;
};

void	ConstrDec::StepCalcStop(int ID_Mt)
{
UCHAR	*tmas;
size_t	tcnt;
int	tlen=1;
Act_N_Mt=-1;
//Srv->Idle();	// !!!!!!!!!!!!!!!!!?????????????????
Act_N_Mt=ID_Mt;
/*
for (size_t i6=0;i6<Str.Mt[ID_Mt].N_PO;i6++)
  while (Str.Mt[ID_Mt].PDec->OutData(i6,tmas,tcnt)!=0)
    for (int j=0;j<Str.Max_LP;j++)
      if (Str.LP[j].ID_Mt1==ID_Mt)
       if ((Str.LP[j].PO==i6)&&(Str.LP[j].ID_Mt2!=-1))
	 StepCalc(Str.LP[j].PI,Str.LP[j].ID_Mt2,tmas,tcnt);
       else if (Str.LP[j].ID_Mt2==-1)
	 OData(Str.LP[j].PI,tmas,tcnt,ExtPoi,IDOut);
*/
	Str.Mt[ID_Mt].State=1;
	Str.Mt[ID_Mt].PDec->StopEx();
	while(tlen!=0)
	{
		tlen=0;
		for (int i13=0;i13<Str.Mt[ID_Mt].N_PO;i13++)
			if (Str.Mt[ID_Mt].PDec->OutDataEx(i13,tmas,tcnt)!=0)
			{
				tlen|=1;
				for (int j=0;j<Str.Max_LP;j++)
					if (Str.LP[j].ID_Mt1==ID_Mt)
						if (Str.LP[j].PO==i13)
						{
							if(Str.LP[j].ID_Mt2!=-1) StepCalc(Str.LP[j].PI,Str.LP[j].ID_Mt2,tmas,tcnt);
							else
							{	
//								if (Str.LP[j].PI==19)
//								{
//									CfgProc((LocalCfg*)tmas,tcnt);
//								}
//								else
									OData(Str.LP[j].PI,tmas,tcnt,ExtPoi);		// !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!! tNstr
							}
						};
			};
	};

return;
};

//
void	ConstrDec::ReStruct(UDec* PUDec,int	N_Md,int TractID)
{
ErrorConstr=0;
TPUDec=PUDec;
//FlagRestruct=1;

// Ðåêîíñòóèðóåì âñåãäà !!!
//if (N_Md==Act_N_Md)
//	ErrorConstr=ErrorConstr*2-ErrorConstr;
strcpy(Str.Md.Name,PUDec->Md[N_Md].Name);

 {
// Destruct Old Md[Mt]
 for (size_t i7=0;i7<Str.Max_Mt;i7++)
  if (Str.Mt[i7].ID_Md==Act_N_Md)
   if (Str.Mt[i7].PDec!=NULL)
    {
		DelIDMt(Str.Mt[i7].PDec->ID_Meth);
		DelDecod(Str.Mt[i7].PDec);Str.Mt[i7].PDec=NULL;
    };
// Copy from UDec
Str.Max_Mt=0;Str.Max_LP=0;Str.Max_Pr=0;
//for (i=0;i<PUDec.Max_Mt;i++)
//  if (PUDec.Mt[i].ID_Md==N_Md)
//    {
//      strcpy (Str.Mt[i].Name,PUDec.Mt[i].Name);
//      Str.Mt[i].PDec=NULL;Str.Mt[i].ID_Md=PUDec.Mt[i].ID_Md;
//      Str.Mt[i].N_PO=PUDec.Mt[i].N_PO;
//    };
for (size_t i8=0;i8<PUDec->Max_LP;i8++)
  if (PUDec->LP[i8].ID_Mt2==-1)
    {
      if (PUDec->Mt[PUDec->LP[i8].ID_Mt1].ID_Md==N_Md)
       { Str.LP[Str.Max_LP].PI=PUDec->LP[i8].PI;
	 Str.LP[Str.Max_LP].ID_Mt1=PUDec->LP[i8].ID_Mt1;
	 Str.LP[Str.Max_LP].PO=PUDec->LP[i8].PO;
	 Str.LP[Str.Max_LP++].ID_Mt2=PUDec->LP[i8].ID_Mt2;
       };
    }
  else
    {
      if (PUDec->Mt[PUDec->LP[i8].ID_Mt2].ID_Md==N_Md)
       { Str.LP[Str.Max_LP].PI=PUDec->LP[i8].PI;
	 Str.LP[Str.Max_LP].ID_Mt1=PUDec->LP[i8].ID_Mt1;
	 Str.LP[Str.Max_LP].PO=PUDec->LP[i8].PO;
	 Str.LP[Str.Max_LP++].ID_Mt2=PUDec->LP[i8].ID_Mt2;
       };
    };
for (size_t i9=0;i9<PUDec->Max_Pr;i9++)
  if (PUDec->Mt[PUDec->Pr[i9].ID_Mt].ID_Md==N_Md)
    { strcpy (Str.Pr[Str.Max_Pr].Name,PUDec->Pr[i9].Name);
      Str.Pr[Str.Max_Pr].ID_Mt=PUDec->Pr[i9].ID_Mt;
      strcpy (Str.Pr[Str.Max_Pr++].Vol,PUDec->Pr[i9].Vol);
     };

for (size_t i10=0;i10<PUDec->Max_Mt;i10++)
  if (PUDec->Mt[i10].ID_Md==N_Md)
    {
      strcpy (Str.Mt[Str.Max_Mt].Name,PUDec->Mt[i10].Name);
      Str.Mt[Str.Max_Mt].Ext=PUDec->Mt[i10].Ext;
      Str.Mt[Str.Max_Mt].PDec=NULL;
      Str.Mt[Str.Max_Mt].ID_Md=PUDec->Mt[i10].ID_Md;
      Str.Mt[Str.Max_Mt].N_PO=PUDec->Mt[i10].N_PO;
	for (int j=0;j<Str.Max_LP;j++)
	  { if (Str.LP[j].ID_Mt1==i10) Str.LP[j].ID_Mt1=Str.Max_Mt;
	    if (Str.LP[j].ID_Mt2==i10) Str.LP[j].ID_Mt2=Str.Max_Mt;
	  };
	for (int j1=0;j1<Str.Max_Pr;j1++)
	  if (Str.Pr[j1].ID_Mt==i10) Str.Pr[j1].ID_Mt=Str.Max_Mt;
      Str.Max_Mt++;
    };

// Construct New Md[Mt]
 for (int i11=0;i11<Str.Max_Mt;i11++)
   {
	   if (Str.Mt[i11].PDec!=NULL) {DelIDMt(Str.Mt[i11].PDec->ID_Meth);DelDecod(Str.Mt[i11].PDec);Str.Mt[i11].PDec=NULL;}
	   NewDecod(i11);
	   if (Str.Mt[i11].PDec!=0)     // Îøèáêà ñîçäàíèÿ
	   {
			Str.Mt[i11].N_PO=Str.Mt[i11].PDec->Num_PinO;
			Str.Mt[i11].PDec->ID_Meth=NewIDMt();
			Str.Mt[i11].PDec->ID_Host=ID_Host;
			Str.Mt[i11].PDec->Ext_Mt=Str.Mt[i11].Ext;
	   }
   };
// Ïðèñâîåíèå ðàñøèðåíèé
//for (i10=0;i10<Str.Max_Mt;i10++)
//  {
//  char*	TName;
//  i11=0;
//  if (Str.Mt[i10].PDec->Ext_Mt[0]==0)
//   { sprintf(Str.Mt[i10].PDec->Ext_Mt,"%03d",i11++);
//   TName=Str.Mt[i10].PDec->Name_Mt;
//   }
//  else continue;
//  for (i9=i10+1;i9<Str.Max_Mt;i9++)
//    {
//    if (Str.Mt[i9].PDec->Ext_Mt[0]==0)
//    if (strcmp(Str.Mt[i9].PDec->Name_Mt,TName)==0)
//      {
//      sprintf(Str.Mt[i9].PDec->Ext_Mt,"%03d",i11++);
//      }
//    };
//  };
// ----------------------
if (ErrorConstr!=0)
 for (size_t i8=0;i8<Str.Max_Mt;i8++)
  if (Str.Mt[i8].ID_Md==Act_N_Md)
   if (Str.Mt[i8].PDec!=NULL)
    {
		DelIDMt(Str.Mt[i8].PDec->ID_Meth);
		DelDecod(Str.Mt[i8].PDec);Str.Mt[i8].PDec=NULL;
    };

 Act_N_Md=N_Md;
 };
//FlagRestruct=0;
};

int ConstrDec::Data(int N_Pin,UCHAR* pmas,size_t Cnt)
{
	NumStep=0;
	NC1=GetNClk();
	NFClk=NC1-NC2;
	nFClk=nFClk*0.999+NFClk*0.001;
//	Gluc.Step();

	for (size_t i12=0;i12<Str.Max_Mt;i12++)
	{
		Str.Mt[i12].NC_Idle=0;
		Str.Mt[i12].NC_Data=0;
		Str.Mt[i12].NC_OData=0;
	};

	Act_N_Mt=-2;
//	if (N_Pin==6)
//		Srv->Idle();	// !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
	Act_N_Mt=-1;
	NC2=GetNClk();
	NClk=NC2-NC1;
	FileTime tFT=Srv->GetDeltaTime(NClk);
	INT64 tTmP=tFT.Time/Ktime;
	if (tTmP) Srv->Telemetr(NULL,"IdleTime,ms",(double)tTmP);

	if ((ErrorConstr==0)&&(Srv->Fl_NetLock!=0))
		for (size_t i13=0;i13<Str.Max_LP;i13++)
		   if (Str.LP[i13].ID_Mt1==-1)
		     if (Str.LP[i13].PO==N_Pin)
				StepCalc(Str.LP[i13].PI,Str.LP[i13].ID_Mt2,pmas,Cnt);
	Act_N_Mt=-1;

	for (size_t i14=0;i14<Str.Max_Mt;i14++)
	{
		Str.Mt[i14].nC_Idle=0.999*Str.Mt[i14].nC_Idle+0.001*Str.Mt[i14].NC_Idle;
		Str.Mt[i14].nC_Data=0.999*Str.Mt[i14].nC_Data+0.001*Str.Mt[i14].NC_Data;
		Str.Mt[i14].nC_OData=0.999*Str.Mt[i14].nC_OData+0.001*Str.Mt[i14].NC_OData;
	};

	NC2=GetNClk();
	NClk=NC2-NC1;
	FileTime tFT2=Srv->GetDeltaTime(NClk);
	tTmP=tFT2.Time/Ktime;
	if (tTmP) Srv->Telemetr(NULL,"CalcTime,ms",(double)tTmP);
	if (tFT2.Time>8*KtimeS)
	{
		char	tBf[128];
		sprintf(tBf,"CD=%d Idle T=%d,mS AData T=%d,mS NStep=%d",Ext_Mt,(int)(tFT.Time/Ktime),(int)(tFT2.Time/Ktime),NumStep);
		CE3=Srv->SetCriticalError(NULL,tBf,"Warning",Str.Md.Name,CE3);
	}
	nClk=nClk*0.999+NClk*0.001;
return 0;
};

//int ConstrDec::OutData(int N_Pin,UCHAR* &pmas,size_t &Cnt)
//{
//
//for (TPinO=TPinO;TPinO<Num_PinO;TPinO++)
//  for (size_t i99=0;i99<Str.Max_LP;i99++)
//    if ((Str.LP[i99].ID_Mt2==-1)&&(Str.LP[i99].PI==TPinO))
//      if (Str.Mt[Str.LP[i99].ID_Mt1].PDec->OutData(Str.LP[i99].PO,pmas,Cnt)!=0)
//	return Cnt;
//TPinO=0;
//return 0;
//};

//int ConstrDec::DataStop(UCHAR* pmas,size_t Cnt)
//{
//for (size_t i=0;i<Str.Max_LP;i++)
//   if (Str.LP[i].ID_Mt1==-1)
//      StepCalcStop(Str.LP[i].PI,Str.LP[i].ID_Mt2,pmas,Cnt);
//   else break;
//return 0;
//};

void	ConstrDec::StepCalc(int PinI,int ID_Mt,UCHAR* pmas,size_t Cnt)
{
	UCHAR		*tmas;
//	StrChron	*StrCh;
	size_t	tcnt;
	int	tlen=1;
	Act_N_Mt=-1;//  tlen=tlen/0;
	// !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
	INT64	nc0=GetNClk();NumStep++;

	//Chron->Measuring(0,0);
//	Srv->Idle();
	//StrCh=Chron->Measuring(1,0);
	//Srv->Telemetr(this,"Idle_t_max,s",StrCh->t_max,0);
	//Srv->Telemetr(this,"Idle_T_max,s",StrCh->T_max,0);
	//Srv->Telemetr(this,"Idle_t_average,s",StrCh->t_average,0);
	//Srv->Telemetr(this,"Idle_T_average,s",StrCh->T_average,0);
	//Srv->Telemetr(this,"Idle_Porocity,%",StrCh->porosity,0);
	INT64	tcn0;
	FileTime tFT;
	INT64	nc1=GetNClk(); 

	Str.Mt[ID_Mt].NC_Idle+=nc1-nc0;

	Act_N_Mt=ID_Mt;//  tlen=tlen/0;

	if (PinI<0)
	{	// Îñîáûå íîæêè
		switch (PinI)
		{
		case -1:
			Str.Mt[ID_Mt].PDec->StopEx();
			goto	_tt01;
	//		break;
		case -2:
			Str.Mt[ID_Mt].PDec->StartEx();
			break;
		case -3:
			if (Cnt>40)
				Str.Mt[ID_Mt].PDec->ParamEx((char*)pmas,(char*)pmas+40);
			break;
		}
	}
	else
	{
		Str.Mt[ID_Mt].CntData++;
		Str.Mt[ID_Mt].LenData+=Cnt;
		if (Str.Mt[ID_Mt].PDec->DataEx(PinI,pmas,Cnt)!=0)
		{
			tcn0=GetNClk()-nc1;
			tFT=Srv->GetDeltaTime(tcn0);
			if (tFT.Time>8*KtimeS)
			{
				char	tBf[128];
				sprintf(tBf,"Mt=%s Data Time=%d,mS",Str.Mt[ID_Mt].PDec->Name_Mt,(int)(tFT.Time/Ktime));
				CE1=Srv->SetCriticalError(NULL,tBf,"Warning","StepCalc",CE1);
			}
			Str.Mt[ID_Mt].NC_Data+=tcn0;
	_tt01:	while(tlen!=0)
			{
				tlen=0;
				for (int i13=0;i13<Str.Mt[ID_Mt].N_PO;i13++)
				{
					nc1=GetNClk();
					Str.Mt[ID_Mt].CntOData++;
					if (Str.Mt[ID_Mt].PDec->OutDataEx(i13,tmas,tcnt)!=0)
					{
						Str.Mt[ID_Mt].LenOData+=tcnt;
						tcn0=GetNClk()-nc1;
						tFT=Srv->GetDeltaTime(tcn0);
						if (tFT.Time>8*KtimeS)
						{
							char	tBf[128];
							sprintf(tBf,"Mt=%s OData Time=%d,mS",Str.Mt[ID_Mt].PDec->Name_Mt,(int)(tFT.Time/Ktime));
							CE2=Srv->SetCriticalError(NULL,tBf,"Warning","StepCalc",CE2);
						}
						Str.Mt[ID_Mt].NC_OData+=tcn0;
						tlen|=1;
						for (int j=0;j<Str.Max_LP;j++)
							if (Str.LP[j].ID_Mt1==ID_Mt)
								if (Str.LP[j].PO==i13)
								{
									if(Str.LP[j].ID_Mt2!=-1) StepCalc(Str.LP[j].PI,Str.LP[j].ID_Mt2,tmas,tcnt);
									else
									{	
											OData(Str.LP[j].PI,tmas,tcnt,ExtPoi);		// !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!! tNstr
									}
								};
					}
					else
					{
						tcn0=GetNClk()-nc1;
						tFT=Srv->GetDeltaTime(tcn0);
						if (tFT.Time>8*KtimeS)
						{
							char	tBf[128];
							sprintf(tBf,"Mt=%s OData Time=%d,mS",Str.Mt[ID_Mt].PDec->Name_Mt,(int)(tFT.Time/Ktime));
							CE2=Srv->SetCriticalError(NULL,tBf,"Warning","StepCalc",CE2);
						}
						Str.Mt[ID_Mt].NC_OData+=tcn0;
					};
				};
			};
		}
		else
		{
			tcn0=GetNClk()-nc1;
			tFT=Srv->GetDeltaTime(tcn0);
			if (tFT.Time>8*KtimeS)
			{
				char	tBf[128];
				sprintf(tBf,"Mt=%s Data Time=%d,mS",Str.Mt[ID_Mt].PDec->Name_Mt,(int)(tFT.Time/Ktime));
				CE1=Srv->SetCriticalError(NULL,tBf,"Warning","StepCalc",CE1);
			}
			Str.Mt[ID_Mt].NC_Data+=tcn0;
		};
	}
};

//void	ConstrDec::StepCalcStop(int PinI,int ID_Mt,UCHAR* pmas,size_t Cnt)
//{
//UCHAR	*tmas;
//int	tcnt;
//Str.Mt[ID_Mt].PDec->Data(PinI,pmas,Cnt);
//for (size_t i=0;i<Str.Mt[ID_Mt].N_PO;i++)
//  {
//  while (Str.Mt[ID_Mt].PDec->OutData(i,tmas,tcnt)!=0)
//    for (int j=0;j<Str.Max_LP;j++)
//      if (Str.LP[j].ID_Mt1==ID_Mt)
//       if (Str.LP[j].PO==i)
//	 StepCalcStop(Str.LP[j].PI,Str.LP[j].ID_Mt2,tmas,tcnt);
// Çàïàñíîé ïîðîõîä ñ ïóñòûìè äàííûìè
//  for (int j=0;j<Str.Max_LP;j++)
//    if (Str.LP[j].ID_Mt1==ID_Mt)
//     if (Str.LP[j].PO==i)
//      StepCalcStop(Str.LP[j].PI,Str.LP[j].ID_Mt2,tmas,tcnt);
//  };
//};

int	ConstrDec::LoadFromCfg(char* Name)
{
FILE	*HFile;
char	Line[80];
int	N_Mt=0;
int	N_Pr=0;
int	N_LP=0;
int	i;
// Destruct Old Md[Mt]
 for (i=0;i<Str.Max_Mt;i++)
   if (Str.Mt[i].PDec!=NULL)
    {
		DelIDMt(Str.Mt[i].PDec->ID_Meth);
		DelDecod(Str.Mt[i].PDec);
		Str.Mt[i].PDec=NULL;
    }
HFile=fopen(Name,"rt");
while (fscanf(HFile,"%s",Line)!=EOF)
 {
 if (strcmp(Line,"NameMode")==0)
  {
  fscanf(HFile,"%s",Line);strcpy(Str.Md.Name,Line);
  };
 if (strcmp(Line,"Method")==0)
  {
  fscanf(HFile,"%s",Line);N_Mt=atoi(Line);
  fscanf(HFile,"%s",Line);Str.Mt[N_Mt].N_PO=atoi(Line);
  fscanf(HFile,"%s",Line);strcpy(Str.Mt[N_Mt].Name,Line);
  Str.Mt[N_Mt].ID_Md=0;
  };
 if (strcmp(Line,"Param")==0)
  {
  fscanf(HFile,"%s",Line);strcpy(Str.Pr[N_Pr].Name,Line);Str.Pr[N_Pr].ID_Mt=N_Mt;
  fscanf(HFile,"%s",Line);strcpy(Str.Pr[N_Pr++].Vol,Line);
  };
 if (strcmp(Line,"Link")==0)
  {
  fscanf(HFile,"%s",Line);Str.LP[N_LP].PO=atoi(Line);
  fscanf(HFile,"%s",Line);Str.LP[N_LP].ID_Mt1=atoi(Line);
  fscanf(HFile,"%s",Line);Str.LP[N_LP].PI=atoi(Line);
  fscanf(HFile,"%s",Line);Str.LP[N_LP++].ID_Mt2=atoi(Line);
  };
 };
Str.Max_Mt=++N_Mt;
Str.Max_LP=N_LP;
Str.Max_Pr=N_Pr;
fclose (HFile);

// Construct New Md[Mt]
 for (i=0;i<Str.Max_Mt;i++)
   {
   if (Str.Mt[i].PDec!=NULL) {DelIDMt(Str.Mt[i].PDec->ID_Meth);DelDecod (Str.Mt[i].PDec);Str.Mt[i].PDec=NULL;}
	   NewDecod(i);
	   if (Str.Mt[i].PDec!=NULL)
	   {
		   Str.Mt[i].PDec->ID_Meth=NewIDMt();
	   }
   };
// ’»ÿþûýø²¼ ÿ°þòõ°ú³ ýð ýþ°üðû¼ýþõ ±þ÷ôðýøõ ²°ðú²ð
return 0;
};

//-----------------------

ConstrDecF::ConstrDecF()
{
/*
strcpy(Name_Mt,"ConstrDec");
Srv=GetServices();CE1=0;CE2=0;CE3=0;
Num_PinI=1;NumStep=0;
Num_PinO=10;
TPinO=0;
TPUDec=0;
OData=&NData;
Srv=0;
for (size_t i=0;i<65536;i++)
	TabID[i]=0;
CntID=0;
Tthis=(LPVOID)this;
Act_N_Md=-1;
Act_N_Mt=-1;
ID_Host=0;
//FlagRestruct=0;
memset(&Str,0x0,sizeof(WUDec));

NC1=GetNClk();NC2=GetNClk();
nClk=0.0;nFClk=0.0;
//Chron=new Chronometry (this);
*/
	CntRun=0;Fl_Error=0;
};

ConstrDecF::ConstrDecF(UCHAR Host)
{
/*
strcpy(Name_Mt,"ConstrDec");
Srv=GetServices();CE1=0;CE2=0;CE3=0;
Num_PinI=1;NumStep=0;
Num_PinO=10;
TPinO=0;
TPUDec=0;
OData=&NData;
Srv=0;
for (size_t i=0;i<65536;i++)
	TabID[i]=0;
CntID=0;
Tthis=(LPVOID)this;
Act_N_Md=-1;
Act_N_Mt=-1;
ID_Host=Host;
//FlagRestruct=0;
memset(&Str,0x0,sizeof(WUDec));
NC1=GetNClk();NC2=GetNClk();
nClk=0.0;nFClk=0.0;
RecurseFl=0;
//Chron=new Chronometry (this);
*/
	CntRun=0;Fl_Error=0;
};

void	ConstrDecF::Stop()
{
if (ErrorConstr==0)
	{
	for (size_t i=0;i<Str.Max_LP;i++)
		if (Str.LP[i].ID_Mt1==-1)
			if (Str.LP[i].ID_Mt2>=0)
			{
				Str.Mt[Str.LP[i].ID_Mt2].PDec->StopEx();
				int Fl_Pin=1;
				while (Fl_Pin!=0)
				{
					Fl_Pin=0;
					for (int Pin=0;Pin<Str.Mt[Str.LP[i].ID_Mt2].PDec->Num_PinO;Pin++)
					{
						if (Str.Mt[Str.LP[i].ID_Mt2].PDec->OutDataEx(Pin,tBuf,tCnt)!=0)
						{
							Fl_Pin|=1;
							Str.Mt[Str.LP[i].ID_Mt2].PDec->ODataFFEx(Pin,tBuf,tCnt);
						}
					}
				}
			};
	 for (size_t i2=0;i2<Str.Max_Mt;i2++)
	   if (Str.Mt[i2].PDec!=NULL)
		{
			Str.Mt[i2].PDec->StopEx();
		};
	}
Act_N_Mt=-1;
return;
};

void	ConstrDecF::ReStruct(UDec* PUDec,int N_Md,int TractID)
{
ErrorConstr=0;
TPUDec=PUDec;
TN_Md=N_Md;
// Ðåêîíñòóèðóåì âñåãäà !!!
strcpy(Str.Md.Name,PUDec->Md[N_Md].Name);
 {
// Destruct Old Md[Mt]
 for (size_t i7=0;i7<Str.Max_Mt;i7++)
  if (Str.Mt[i7].ID_Md==Act_N_Md)
   if (Str.Mt[i7].PDec!=NULL)
    {
		DelIDMt(Str.Mt[i7].PDec->ID_Meth);
		DelDecod(Str.Mt[i7].PDec);Str.Mt[i7].PDec=NULL;
    };
// Copy from UDec
Str.Max_Mt=0;Str.Max_LP=0;Str.Max_Pr=0;
for (size_t i8=0;i8<PUDec->Max_LP;i8++)
  if (PUDec->LP[i8].ID_Mt2==-1)
    {
      if (PUDec->Mt[PUDec->LP[i8].ID_Mt1].ID_Md==N_Md)
       { Str.LP[Str.Max_LP].PI=PUDec->LP[i8].PI;
	 Str.LP[Str.Max_LP].ID_Mt1=PUDec->LP[i8].ID_Mt1;
	 Str.LP[Str.Max_LP].PO=PUDec->LP[i8].PO;
	 Str.LP[Str.Max_LP++].ID_Mt2=PUDec->LP[i8].ID_Mt2;
       };
    }
  else
    {
      if (PUDec->Mt[PUDec->LP[i8].ID_Mt2].ID_Md==N_Md)
       { Str.LP[Str.Max_LP].PI=PUDec->LP[i8].PI;
	 Str.LP[Str.Max_LP].ID_Mt1=PUDec->LP[i8].ID_Mt1;
	 Str.LP[Str.Max_LP].PO=PUDec->LP[i8].PO;
	 Str.LP[Str.Max_LP++].ID_Mt2=PUDec->LP[i8].ID_Mt2;
       };
    };
for (size_t i9=0;i9<PUDec->Max_Pr;i9++)
  if (PUDec->Mt[PUDec->Pr[i9].ID_Mt].ID_Md==N_Md)
    { strcpy (Str.Pr[Str.Max_Pr].Name,PUDec->Pr[i9].Name);
      Str.Pr[Str.Max_Pr].ID_Mt=PUDec->Pr[i9].ID_Mt;
      strcpy (Str.Pr[Str.Max_Pr++].Vol,PUDec->Pr[i9].Vol);
     };

for (size_t i10=0;i10<PUDec->Max_Mt;i10++)
  if (PUDec->Mt[i10].ID_Md==N_Md)
    {
      strcpy (Str.Mt[Str.Max_Mt].Name,PUDec->Mt[i10].Name);
      Str.Mt[Str.Max_Mt].Ext=PUDec->Mt[i10].Ext;
      Str.Mt[Str.Max_Mt].PDec=NULL;
      Str.Mt[Str.Max_Mt].ID_Md=PUDec->Mt[i10].ID_Md;
      Str.Mt[Str.Max_Mt].N_PO=PUDec->Mt[i10].N_PO;
	for (int j=0;j<Str.Max_LP;j++)
	  { if (Str.LP[j].ID_Mt1==i10) Str.LP[j].ID_Mt1=Str.Max_Mt;
	    if (Str.LP[j].ID_Mt2==i10) Str.LP[j].ID_Mt2=Str.Max_Mt;
	  };
	for (int j1=0;j1<Str.Max_Pr;j1++)
	  if (Str.Pr[j1].ID_Mt==i10) Str.Pr[j1].ID_Mt=Str.Max_Mt;
      Str.Max_Mt++;
    };

// Construct New Md[Mt]
 for (int i11=0;i11<Str.Max_Mt;i11++)
   {
	   if (Str.Mt[i11].PDec!=NULL) {DelIDMt(Str.Mt[i11].PDec->ID_Meth);DelDecod(Str.Mt[i11].PDec);Str.Mt[i11].PDec=NULL;}
	   NewDecod(i11);
	   if (Str.Mt[i11].PDec!=0)     // Îøèáêà ñîçäàíèÿ
	   {
			Str.Mt[i11].N_PO=Str.Mt[i11].PDec->Num_PinO;
			Str.Mt[i11].PDec->ID_Meth=NewIDMt();
			Str.Mt[i11].PDec->ID_Host=ID_Host;
			Str.Mt[i11].PDec->Ext_Mt=Str.Mt[i11].Ext;
	   }
   };
// ----------------------
// Inicialization Forward Call Function
if (ErrorConstr==0)
	for (size_t i13=0;i13<Str.Max_LP;i13++)
	{
		if (Str.LP[i13].ID_Mt1!=-1)
		{	// Âõîäû îáðàáàòûâàþòñÿ â ôóíêöèè DataFF
			if (Str.LP[i13].ID_Mt2==-1)
			{
				Str.Mt[Str.LP[i13].ID_Mt1].PDec->AddDecLink(this,Str.LP[i13].PO,1000+Str.LP[i13].PI);	// 1000 !
			}
			else
			{
				Str.Mt[Str.LP[i13].ID_Mt1].PDec->AddDecLink(Str.Mt[Str.LP[i13].ID_Mt2].PDec,Str.LP[i13].PO,Str.LP[i13].PI);
			}
		};
	};
// ----------------------
if (ErrorConstr!=0)
 for (size_t i8=0;i8<Str.Max_Mt;i8++)
  if (Str.Mt[i8].ID_Md==Act_N_Md)
   if (Str.Mt[i8].PDec!=NULL)
    {
		DelIDMt(Str.Mt[i8].PDec->ID_Meth);
		DelDecod(Str.Mt[i8].PDec);Str.Mt[i8].PDec=NULL;
    };

 Act_N_Md=N_Md;
 };
//FlagRestruct=0;
};


int		ConstrDecF::DataFF(			// Äàííûå äëÿ äåêîäèðîâàíèÿ
					int N_Pin,		// Íîìåð âõîäíîé íîæêè
					UCHAR* pmas,	// Óêàçàòåëü íà áóôåð ñ äàííûìè
					size_t Cnt)		// Äëèíà äàííûõ
{
	int	rc=0;
	if (ErrorConstr!=0) return -1;
	char	vBuf[128];

//	Gluc.Step();
	if (N_Pin<1000)
	{	// Îáñëóæèâàíèå âõîäíûõ íîæåê
		NumStep=0;
		NC1=GetNClk();
		NFClk=NC1-NC2;
		nFClk=nFClk*0.999+NFClk*0.001;

		for (size_t i12=0;i12<Str.Max_Mt;i12++)
		{
			Str.Mt[i12].NC_Idle=0;
			Str.Mt[i12].NC_Data=0;
			Str.Mt[i12].NC_OData=0;
			if (Str.Mt[i12].PDec)
			{
				Str.Mt[i12].PDec->NC1=0;
				Str.Mt[i12].PDec->NC2=0;
				Str.Mt[i12].PDec->NCI=0;
				Str.Mt[i12].PDec->NCD=0;
				Str.Mt[i12].PDec->NCO=0;
			}
		};

		Act_N_Mt=-2;
//		try
//		{
//		if (N_Pin==6)
//			Srv->Idle();
//		}
//		catch(...)
//		{
//			Srv->Idle();
//		}
		Act_N_Mt=-1;
		NC2=GetNClk();
		NClk=NC2-NC1;
		FileTime tFT=Srv->GetDeltaTime(NClk);
		INT64 tTmP=tFT.Time/Ktime;
		if (tTmP) Srv->Telemetr(NULL,"IdleTime,ms",(double)tTmP);
		int Fl_Pin;
		{
			if (N_Pin<0)
			{
//				Srv->AddDebugLine(":");
				switch (N_Pin)
				{
				case -1:
					Srv->AddDebugLevel(this,"Sp");
					Stop();	// ODataFF
					Srv->SubDebugLevel(this);
					break;
				case -2:
					Srv->AddDebugLevel(this,"St");
					Start();
					Srv->SubDebugLevel(this);
					break;
				case -3:
					Srv->AddDebugLevel(this,"Pr");
					if (Cnt>40)
						Param((char*)pmas,(char*)pmas+40);
					Srv->SubDebugLevel(this);
					break;
				}
			}
			for (size_t i13=0;i13<Str.Max_LP;i13++)
			   if (Str.LP[i13].ID_Mt1==-1)
				 if (Str.LP[i13].PO==N_Pin)
				 {
//					Srv->AddDebugLine("->");
					if (Str.LP[i13].PI<0)
					{	// Îñîáûå íîæêè
						switch (Str.LP[i13].PI)
						{
						case -1:
							sprintf(vBuf," >%s:Sp",Str.Mt[Str.LP[i13].ID_Mt2].PDec->Name_Mt);
							Srv->AddDebugLevel(this,vBuf);
							Str.Mt[Str.LP[i13].ID_Mt2].PDec->StopEx();	// ODataFF
							Srv->SubDebugLevel(this);
							Fl_Pin=1;
							while (Fl_Pin!=0)
							{
								Fl_Pin=0;
								for (int Pin=0;Pin<Str.Mt[Str.LP[i13].ID_Mt2].PDec->Num_PinO;Pin++)
								{
									if (Str.Mt[Str.LP[i13].ID_Mt2].PDec->OutDataEx(Pin,tBuf,tCnt)!=0)
									{
										Fl_Pin|=1;
										Str.Mt[Str.LP[i13].ID_Mt2].PDec->ODataFFEx(Pin,tBuf,tCnt);
									}
									else
									{

									}
								}
							}
							break;
						case -2:
							sprintf(vBuf," >%s:St",Str.Mt[Str.LP[i13].ID_Mt2].PDec->Name_Mt);
							Srv->AddDebugLevel(this,vBuf);
							Str.Mt[Str.LP[i13].ID_Mt2].PDec->StartEx();
							Srv->SubDebugLevel(this);
							break;
						case -3:
							sprintf(vBuf," >%s:Pr",Str.Mt[Str.LP[i13].ID_Mt2].PDec->Name_Mt);
							Srv->AddDebugLevel(this,vBuf);
							if (Cnt>40)
								Str.Mt[Str.LP[i13].ID_Mt2].PDec->ParamEx((char*)pmas,(char*)pmas+40);
							Srv->SubDebugLevel(this);
							break;
						}
					}
					else
					{
						Str.Mt[Str.LP[i13].ID_Mt2].PDec->NC1=GetNClk();		// Èçìåðåíèå ïðîäîëæèòåëüíîñòè ðàáîòû ìåòîäà
						sprintf(vBuf," >%s:D%d",Str.Mt[Str.LP[i13].ID_Mt2].PDec->Name_Mt,N_Pin);
						Srv->AddDebugLevel(this,vBuf);
						Str.Mt[Str.LP[i13].ID_Mt2].PDec->DataFFEx(Str.LP[i13].PI,pmas,Cnt);
						Srv->SubDebugLevel(this);
						Str.Mt[Str.LP[i13].ID_Mt2].PDec->NC2=GetNClk();
						Str.Mt[Str.LP[i13].ID_Mt2].PDec->NCD+=Str.Mt[Str.LP[i13].ID_Mt2].PDec->NC2-Str.Mt[Str.LP[i13].ID_Mt2].PDec->NC1;
					}
				 }
		}
//		size_t i;
//		for (i=0;i<Str.Max_Mt;i++)
//		{
//			Str.Mt[i].PDec->DataIdle();
//		};
		Act_N_Mt=-1;

		for (size_t i14=0;i14<Str.Max_Mt;i14++)
		{
			if (Str.Mt[i14].PDec)
			{
				Str.Mt[i14].NC_Idle=Str.Mt[i14].PDec->NCI;
				Str.Mt[i14].NC_Data=Str.Mt[i14].PDec->NCD;
				Str.Mt[i14].NC_OData=Str.Mt[i14].PDec->NCO;
				Str.Mt[i14].nC_Idle=0.999*Str.Mt[i14].nC_Idle+0.001*Str.Mt[i14].NC_Idle;
				Str.Mt[i14].nC_Data=0.999*Str.Mt[i14].nC_Data+0.001*Str.Mt[i14].NC_Data;
				Str.Mt[i14].nC_OData=0.999*Str.Mt[i14].nC_OData+0.001*Str.Mt[i14].NC_OData;
			}
		};

		NC2=GetNClk();
		NClk=NC2-NC1;
		FileTime tFT2=Srv->GetDeltaTime(NClk);
		tTmP=tFT2.Time/Ktime;
		if (tTmP) Srv->Telemetr(NULL,"CalcTime,ms",(double)tTmP);
		if (tFT2.Time>8*KtimeS)
		{
			char	tBf[128];
			sprintf(tBf,"CD=%d Idle T=%d,mS AData T=%d,mS NStep=%d",Ext_Mt,(int)(tFT.Time/Ktime),(int)(tFT2.Time/Ktime),NumStep);
			CE3=Srv->SetCriticalError(NULL,tBf,"Warning",Str.Md.Name,CE3);
		}
		nClk=nClk*0.999+NClk*0.001;
	}
	else
	{	// Îáñëóæèâàíèå âûõîäíûõ íîæåê
		N_Pin=N_Pin-1000;
		if (NumLink>0)
		{
			for (size_t i=0;i<NumLink;i++)
			{
				if (TabDecLink[i].PinO==N_Pin)
				{
					TabDecLink[i].pObj->DataFFEx(TabDecLink[i].PinI,pmas,Cnt);
				}
			}
		}
		else
		{
			OData(N_Pin,pmas,Cnt,ExtPoi);
		}
	}
	return rc;
};

int		ConstrDecF::DataIDI(			// Äàííûå äëÿ äåêîäèðîâàíèÿ
	int N_Pin,		// Íîìåð âõîäíîé íîæêè
	IDInfo* pIDI)		// Äëèíà äàííûõ
{
	int	rc=0;
	if (ErrorConstr!=0) return -1;
	char	vBuf[128];

	//	Gluc.Step();
	if (N_Pin<1000)
	{	// Îáñëóæèâàíèå âõîäíûõ íîæåê
		NumStep=0;
		NC1=GetNClk();
		NFClk=NC1-NC2;
		nFClk=nFClk*0.999+NFClk*0.001;

		for (size_t i12=0;i12<Str.Max_Mt;i12++)
		{
			Str.Mt[i12].NC_Idle=0;
			Str.Mt[i12].NC_Data=0;
			Str.Mt[i12].NC_OData=0;
			if (Str.Mt[i12].PDec)
			{
				Str.Mt[i12].PDec->NC1=0;
				Str.Mt[i12].PDec->NC2=0;
				Str.Mt[i12].PDec->NCI=0;
				Str.Mt[i12].PDec->NCD=0;
				Str.Mt[i12].PDec->NCO=0;
			}
		};

		Act_N_Mt=-2;
		Act_N_Mt=-1;
		NC2=GetNClk();
		NClk=NC2-NC1;
		FileTime tFT=Srv->GetDeltaTime(NClk);
		INT64 tTmP=tFT.Time/Ktime;
		if (tTmP) Srv->Telemetr(NULL,"IdleTime,ms",(double)tTmP);
		int Fl_Pin;
		{
			if (N_Pin<0)
			{
				//				Srv->AddDebugLine(":");
				switch (N_Pin)
				{
				case -1:
					Srv->AddDebugLevel(this,"Sp");
					Stop();	// ODataFF
					Srv->SubDebugLevel(this);
					break;
				case -2:
					Srv->AddDebugLevel(this,"St");
					Start();
					Srv->SubDebugLevel(this);
					break;
				case -3:
// 					Srv->AddDebugLevel(this,"Pr");
// 					if (Cnt>40)
// 						Param((char*)pmas,(char*)pmas+40);
// 					Srv->SubDebugLevel(this);
					break;
				}
			}
			for (size_t i13=0;i13<Str.Max_LP;i13++)
				if (Str.LP[i13].ID_Mt1==-1)
					if (Str.LP[i13].PO==N_Pin)
					{
						//					Srv->AddDebugLine("->");
						if (Str.LP[i13].PI<0)
						{	// Îñîáûå íîæêè
							switch (Str.LP[i13].PI)
							{
							case -1:
								sprintf(vBuf," >%s:Sp",Str.Mt[Str.LP[i13].ID_Mt2].PDec->Name_Mt);
								Srv->AddDebugLevel(this,vBuf);
								Str.Mt[Str.LP[i13].ID_Mt2].PDec->StopEx();	// ODataFF
								Srv->SubDebugLevel(this);
								Fl_Pin=1;
								while (Fl_Pin!=0)
								{
									Fl_Pin=0;
									for (int Pin=0;Pin<Str.Mt[Str.LP[i13].ID_Mt2].PDec->Num_PinO;Pin++)
									{
										if (Str.Mt[Str.LP[i13].ID_Mt2].PDec->OutDataEx(Pin,tBuf,tCnt)!=0)
										{
											Fl_Pin|=1;
											Str.Mt[Str.LP[i13].ID_Mt2].PDec->ODataFFEx(Pin,tBuf,tCnt);
										}
										else
										{

										}
									}
								}
								break;
							case -2:
								sprintf(vBuf," >%s:St",Str.Mt[Str.LP[i13].ID_Mt2].PDec->Name_Mt);
								Srv->AddDebugLevel(this,vBuf);
								Str.Mt[Str.LP[i13].ID_Mt2].PDec->StartEx();
								Srv->SubDebugLevel(this);
								break;
							case -3:
// 								sprintf(vBuf," >%s:Pr",Str.Mt[Str.LP[i13].ID_Mt2].PDec->Name_Mt);
// 								Srv->AddDebugLevel(this,vBuf);
// 								if (Cnt>40)
// 									Str.Mt[Str.LP[i13].ID_Mt2].PDec->ParamEx((char*)pmas,(char*)pmas+40);
// 								Srv->SubDebugLevel(this);
								break;
							}
						}
						else
						{
							Str.Mt[Str.LP[i13].ID_Mt2].PDec->NC1=GetNClk();		// Èçìåðåíèå ïðîäîëæèòåëüíîñòè ðàáîòû ìåòîäà
							sprintf(vBuf," >%s:D%d",Str.Mt[Str.LP[i13].ID_Mt2].PDec->Name_Mt,N_Pin);
							Srv->AddDebugLevel(this,vBuf);
							Str.Mt[Str.LP[i13].ID_Mt2].PDec->DataIDIEx(Str.LP[i13].PI,pIDI);
							Srv->SubDebugLevel(this);
							Str.Mt[Str.LP[i13].ID_Mt2].PDec->NC2=GetNClk();
							Str.Mt[Str.LP[i13].ID_Mt2].PDec->NCD+=Str.Mt[Str.LP[i13].ID_Mt2].PDec->NC2-Str.Mt[Str.LP[i13].ID_Mt2].PDec->NC1;
						}
					}
		}

		Act_N_Mt=-1;

		for (size_t i14=0;i14<Str.Max_Mt;i14++)
		{
			if (Str.Mt[i14].PDec)
			{
				Str.Mt[i14].NC_Idle=Str.Mt[i14].PDec->NCI;
				Str.Mt[i14].NC_Data=Str.Mt[i14].PDec->NCD;
				Str.Mt[i14].NC_OData=Str.Mt[i14].PDec->NCO;
				Str.Mt[i14].nC_Idle=0.999*Str.Mt[i14].nC_Idle+0.001*Str.Mt[i14].NC_Idle;
				Str.Mt[i14].nC_Data=0.999*Str.Mt[i14].nC_Data+0.001*Str.Mt[i14].NC_Data;
				Str.Mt[i14].nC_OData=0.999*Str.Mt[i14].nC_OData+0.001*Str.Mt[i14].NC_OData;
			}
		};

		NC2=GetNClk();
		NClk=NC2-NC1;
		FileTime tFT2=Srv->GetDeltaTime(NClk);
		tTmP=tFT2.Time/Ktime;
		if (tTmP) Srv->Telemetr(NULL,"CalcTime,ms",(double)tTmP);
		if (tFT2.Time>8*KtimeS)
		{
			char	tBf[128];
			sprintf(tBf,"CD=%d Idle T=%d,mS AData T=%d,mS NStep=%d",Ext_Mt,(int)(tFT.Time/Ktime),(int)(tFT2.Time/Ktime),NumStep);
			CE3=Srv->SetCriticalError(NULL,tBf,"Warning",Str.Md.Name,CE3);
		}
		nClk=nClk*0.999+NClk*0.001;
	}
	else
	{	// Îáñëóæèâàíèå âûõîäíûõ íîæåê
		N_Pin=N_Pin-1000;
		if (NumLink>0)
		{
			for (size_t i=0;i<NumLink;i++)
			{
				if (TabDecLink[i].PinO==N_Pin)
				{
					TabDecLink[i].pObj->DataIDIEx(TabDecLink[i].PinI,pIDI);
				}
			}
		}
		else
		{
			OData(N_Pin,(UCHAR*)pIDI,sizeof(IDInfo),ExtPoi);
		}
	}
	return rc;
};



/*
// Âàðèàíò-1
fd_set	Srd,Swr,Sex;
struct timeval timeout;
int optval, optlen=sizeof(int);

Srd.fd_array[0]=Handle; Srd.fd_count=1;
Swr.fd_array[0]=Handle; Swr.fd_count=0;
Sex.fd_array[0]=Handle; Sex.fd_count=0;
timeout.tv_sec=0;		// s
timeout.tv_usec=10000;	// mks
rc=select(Handle,&Srd,&Swr,&Sex,&timeout);
rc=getsockopt(Handle,SOL_SOCKET,SO_ERROR,(char*)&optval,&optlen);
if (optval) 
{
	Flag=0; // disconnect
}

// Âàðèàíò-2
int rc1,rc2; char buf[8];
rc1=recv(Handle,buf,0, 0);
rc2=WSAGetLastErrorTrace();
if( (rc1<0) || (rc2==WSAECONNRESET) 
	)
{
	Flag=0;
}
*/

/*
Zero indicates that the connection has been gracefully closed. 
SOCKET_ERROR indicates failure. To get a specific error value, call WSAGetLastError
*/

// --------------------------------------------------------------------------------

//#define TraceEnable

// --------------------------------------------------------------------------------

char *WSAE_ERROR[]={
	"WSAEWOULDBLOCK",          // (WSABASEERR+35)
	"WSAEINPROGRESS",          // (WSABASEERR+36)
	"WSAEALREADY",             // (WSABASEERR+37)
	"WSAENOTSOCK",             // (WSABASEERR+38)
	"WSAEDESTADDRREQ",         // (WSABASEERR+39)
	"WSAEMSGSIZE",             // (WSABASEERR+40)
	"WSAEPROTOTYPE",           // (WSABASEERR+41)
	"WSAENOPROTOOPT",          // (WSABASEERR+42)
	"WSAEPROTONOSUPPORT",      // (WSABASEERR+43)
	"WSAESOCKTNOSUPPORT",      // (WSABASEERR+44)
	"WSAEOPNOTSUPP",           // (WSABASEERR+45)
	"WSAEPFNOSUPPORT",         // (WSABASEERR+46)
	"WSAEAFNOSUPPORT",         // (WSABASEERR+47)
	"WSAEADDRINUSE",           // (WSABASEERR+48)
	"WSAEADDRNOTAVAIL",        // (WSABASEERR+49)
	"WSAENETDOWN",             // (WSABASEERR+50)
	"WSAENETUNREACH",          // (WSABASEERR+51)
	"WSAENETRESET",            // (WSABASEERR+52)
	"WSAECONNABORTED",         // (WSABASEERR+53)
	"WSAECONNRESET",           // (WSABASEERR+54)
	"WSAENOBUFS",              // (WSABASEERR+55)
	"WSAEISCONN",              // (WSABASEERR+56)
	"WSAENOTCONN",             // (WSABASEERR+57)
	"WSAESHUTDOWN",            // (WSABASEERR+58)
	"WSAETOOMANYREFS",         // (WSABASEERR+59)
	"WSAETIMEDOUT",            // (WSABASEERR+60)
	"WSAECONNREFUSED",         // (WSABASEERR+61)
	"WSAELOOP",                // (WSABASEERR+62)
	"WSAENAMETOOLONG",         // (WSABASEERR+63)
	"WSAEHOSTDOWN",            // (WSABASEERR+64)
	"WSAEHOSTUNREACH",         // (WSABASEERR+65)
	"WSAENOTEMPTY",            // (WSABASEERR+66)
	"WSAEPROCLIM",             // (WSABASEERR+67)
	"WSAEUSERS",               // (WSABASEERR+68)
	"WSAEDQUOT",               // (WSABASEERR+69)
	"WSAESTALE",               // (WSABASEERR+70)
	"WSAEREMOTE",              // (WSABASEERR+71)

	"WSAEUNKNOWN"              // (WSABASEERR+72)
};

int WSAGetLastErrorTrace()
{
	int rc=WSAGetLastError();
#ifndef TraceEnable
	return rc;
#endif
	size_t index=rc-WSAEWOULDBLOCK;
	if( (index<0) || (index>37) ) index=37;
	int WriteToLog(char *);
	if(index) WriteToLog(WSAE_ERROR[index]);
	return rc;
}

int WSAGetLastErrorTrace1(int q)
{
	int rc=WSAGetLastError();
#ifndef TraceEnable
	return rc;
#endif
	size_t index=rc-WSAEWOULDBLOCK;
	if( (index<0) || (index>37) ) index=37;
	char s[256]; int WriteToLog(char *);
	sprintf(s,"this=0x%08x, %s",q,WSAE_ERROR[index]);
	if(index) WriteToLog(s);
	return rc;
}

// --------------------------------------------------------------------------------

int WriteToLog(char *mess)
{

	return 0; // forever

#ifndef TraceEnable
	return 0;
#endif

	char s[256];
	SYSTEMTIME time;
	GetLocalTime(&time);

	sprintf(s,"\\Debug.log");
	int h=open(s,O_BINARY|O_CREAT|O_WRONLY,S_IWRITE);
	if(h!=-1)
	{
		lseek(h,0,SEEK_END);

		sprintf(s,"%04ld-%02ld-%02ld %02ld:%02ld:%02ld   %s\r\n",
			time.wYear,time.wMonth,time.wDay,
			time.wHour,time.wMinute,time.wSecond,
			mess);
		write(h,s,(UINT)strlen(s));

		close(h);
	}
	return 0;
}


int ReceiveDisconnect(int h,int q)
{
	return 0;
	int rc1,rc2; 
	char buf[8],mess[256];
	rc1=recv(h,buf,0, 0);
	if(rc1==0) 
	{
		sprintf(mess,"this=0x%08x, handle=%d, SOCKET gracefully closed",q,h);
		WriteToLog(mess); return 1; // gracefully closed
	}
	if(rc1==SOCKET_ERROR)
	{
		rc2=WSAGetLastError();//WSAGetLastErrorTrace1(q);
		if(rc2==WSAECONNRESET) 
		{
			sprintf(mess,"this=0x%08x, handle=%d, signal <RST>",q,h);
			WriteToLog(mess); return 1; // <RST>
		}
	}
	return 0;
}

// ---------------------------------------------------------------------
int	Accept1(LPVOID);

int	CliFun(LPVOID s)
{
	int	rc=1;
	if (((ClientSock*)s)->Flag==0)
	{
		((ClientSock*)s)->DisConnect();
		Sleep(100);
		((ClientSock*)s)->Connect();
		Sleep(100);
		rc=0;
	}
	else
	{
		if ((((ClientSock*)s)->Idle()|((ClientSock*)s)->SendFromBuf())==0)
			rc=0;
	}	
	return rc;
}

// ---------------------------------------------------------------------
int ClientSock::Trace(char *mess,int prm,char *str)
{
	char message[1024];
	
	if( (prm==-1) && (str==NULL) )
	{
		sprintf(message,"this=0x%08p, handle=%d, %s",
			(LPVOID)this,Handle,mess);
	}
	else if ( (prm!=-1) && (str==NULL) )
	{
		sprintf(message,"this=0x%08p, handle=%d, %s %d",
			(LPVOID)this,Handle,mess,prm);
	}
	else if( (prm!=-1) && (str!=NULL) )
	{
		sprintf(message,"this=0x%08p, handle=%d, %s (%s:%d)",
			(LPVOID)this,Handle,mess,str,prm);
	}
	else if( (prm==-1) && (str!=NULL) )
	{
		sprintf(message,"this=0x%08p, handle=%d, %s (%s)",
			(LPVOID)this,Handle,mess,str);
	}

	//return WriteToLog(message);

#ifdef TraceEnable
	if(pService)
		pService->OutLog(message,NULL,"lls");
#endif

	return 0;
}

// ---------------------------------------------------------------------
ClientSock::ClientSock()
{
	pService=GetServices();
	hCE=0;PriorLev=0;PriorState=0;GlobalCnt=0;Fl_GCntIn=0;
	TimeOutWRB=0;//Fl_DB=0;
	SMode=0;
	TimeOut=1000;
	HeaderBuf.Syn=Syn1;
	LenOBuf=10000;
	OutBuf=(char*)malloc(LenOBuf);
//	SizeAllocMem+=LenOBuf;
	MdTr=0;tSm=0;
	Flag=0;
	SmBuf=0;
	Mode=0;
	readFunction=NULL;
	CallBackEvent=NULL;
	Handle=-1;
//	CB_Wr=CB_Rd=0;
//	CB_Len=100000;
//	CBuf=(UCHAR*)malloc(CB_Len);
	SizeBufCl=MaxPack;
	BufCl=(UCHAR*)malloc(SizeBufCl);
//	SizeAllocMem+=SizeBufCl;
	rcvbufsz=SockBufSizeRecv;
	sndbufsz=SockBufSizeSend;
	rcvbuf_fill=0; 
	sndbuf_fill=0;
	PackCnt=0;PackCntIn=0;
	TimeDisConn.Time=0;TimeConn.Time=0;
};

int		DmpPrcClientSock(void* Obj,UCHAR* tBuf,size_t Cnt)
{
	int	rc;
	rc=((ClientSock*)Obj)->SendNotBuf((char*)tBuf,Cnt);
	return rc;
};

ClientSock::ClientSock(char *ip, int port, 
		int	(*_CallBackEvent)(ClientSock* ,int), 
		int (*_readFunction)(ClientSock*,UCHAR*,int),
		Services *pSrv,Decoder *pDec)
{
	// Init
	hCE=0;TimeDisConn.Time=0;TimeConn.Time=0;PriorLev=0;PriorState=0;GlobalCnt=0;Fl_GCntIn=0;
	pService=pSrv;
	pDecoder=pDec;
	Dmp.SetPrm(pDecoder,(void*)this,&DmpPrcClientSock);
	Dmp0.SetPrm(pDecoder,(void*)this,&DmpPrcClientSock);
	Dmp1.SetPrm(pDecoder,(void*)this,&DmpPrcClientSock);
	TimeOutWRB=0;//Fl_DB=0;
	SMode=0;
	TimeOut=1000;
	HeaderBuf.Syn=Syn1;
	LenOBuf=10000;
	if (pDecoder)
		OutBuf=(char*)pDecoder->malloc(LenOBuf);
	else
		OutBuf=(char*)malloc(LenOBuf);
	MdTr=0;tSm=0;
//	CB_Wr=CB_Rd=0;
//	CB_Len=100000;
//	if (pDecoder)
//		CBuf=(UCHAR*)pDecoder->malloc(CB_Len);
//	else
//		CBuf=(UCHAR*)malloc(CB_Len);

	SizeBufCl=MaxPack;
	if (pDecoder)
		BufCl=(UCHAR*)pDecoder->malloc(SizeBufCl);
	else
		BufCl=(UCHAR*)malloc(SizeBufCl);

	Flag=0;
	Mode=0;
	SmBuf=0;

	strcpy(HostIP,ip); PortTCP=port;
	readFunction=_readFunction;
	CallBackEvent=_CallBackEvent;
	rcvbufsz=SockBufSizeRecv;
	sndbufsz=SockBufSizeSend;
	PackCnt=0;PackCntIn=0;

	Handle=-1;
//	SockIdle=NULL;
	
	server.sin_family = AF_INET;
	server.sin_port   = htons(PortTCP);
	server.sin_addr.s_addr = inet_addr(HostIP);

	gethostname(MyNetName,64);
	TestNetName(MyNetName);
	// Create thread "Connect"
	ClThread=new ClassThread("SockCliFun",CliFun,(LPVOID)this,THREAD_PRIORITY_HIGHEST);

}
ClientSock::~ClientSock()
{
	// Destroy thread "Connect"
	TimeOut=0;
	delete ClThread; ClThread=NULL;
	if (pDecoder)
	{
		if (BufCl) pDecoder->free(BufCl);
//		pDecoder->free(CBuf);
	}
	else
	{
		if (BufCl) free(BufCl);
//		free(CBuf);
	}
	DisConnect();
	if (pDecoder)
	{
		if (OutBuf) pDecoder->free(OutBuf);
	}
	else
	{
		if (OutBuf) free(OutBuf);
	}
}
int	ClientSock::GetFill()
{
	int FillTmp=0;
	//if(pSockData!=NULL)
	{
		if(0)
		{
			UCHAR pLocalBuf[0x1000];
			FillTmp=Receive(pLocalBuf,rcvbufsz,MSG_PEEK); 
			if(FillTmp) rcvbuf_fill=FillTmp;
		}
		else
		{
			if(Handle>0)
			{
				unsigned long SocketDataLen;

				SocketDataLen=0;
				ioctlsocket(Handle,FIONREAD,&SocketDataLen);
				if(SocketDataLen) rcvbuf_fill=SocketDataLen;

				//SocketDataLen=0;
				//ioctlsocket(Handle,,&SocketDataLen);
				//if(SocketDataLen) sndbuf_fill=SocketDataLen;
				/*
				int WSAIoctl(
					SOCKET s,
					DWORD dwIoControlCode,
					LPVOID lpvInBuffer,
					DWORD cbInBuffer,
					LPVOID lpvOutBuffer,
					DWORD cbOutBuffer,
					LPDWORD lpcbBytesReturned,
					LPWSAOVERLAPPED lpOverlapped,
					LPWSAOVERLAPPED_COMPLETION_ROUTINE lpCompletionRoutine
					);
				*/


// 				typedef DWORD WSAEVENT, FAR * LPWSAEVENT;
// 
// 				typedef struct _WSAOVERLAPPED {
// 					DWORD    Internal;
// 					DWORD    InternalHigh;
// 					DWORD    Offset;
// 					DWORD    OffsetHigh;
// 					WSAEVENT hEvent;
// 				} WSAOVERLAPPED, FAR * LPWSAOVERLAPPED;
// 
// 				typedef
// 					void
// 					(CALLBACK * LPWSAOVERLAPPED_COMPLETION_ROUTINE)(
// 					DWORD dwError,
// 					DWORD cbTransferred,
// 					LPWSAOVERLAPPED lpOverlapped,
// 					DWORD dwFlags
// 					);
// 
// 				SOCKET s;
// 				DWORD dwIoControlCode;
// 				LPVOID lpvInBuffer;
// 				DWORD cbInBuffer;
// 				LPVOID lpvOutBuffer;
// 				DWORD cbOutBuffer;
// 				LPDWORD lpcbBytesReturned;
// 				LPWSAOVERLAPPED lpOverlapped;
// 				LPWSAOVERLAPPED_COMPLETION_ROUTINE lpCompletionRoutine;
// 
// 				int WSAIoctl(
// 				SOCKET s,
// 				DWORD dwIoControlCode,
// 				LPVOID lpvInBuffer,
// 				DWORD cbInBuffer,
// 				LPVOID lpvOutBuffer,
// 				DWORD cbOutBuffer,
// 				LPDWORD lpcbBytesReturned,
// 				LPWSAOVERLAPPED lpOverlapped,
// 				LPWSAOVERLAPPED_COMPLETION_ROUTINE lpCompletionRoutine
// 				);
// 
// 				int rc=WSAIoctl(
// 					s,
// 					dwIoControlCode,
// 					lpvInBuffer,
// 					cbInBuffer,
// 					lpvOutBuffer,
// 					cbOutBuffer,
// 					lpcbBytesReturned,
// 					lpOverlapped,
// 					lpCompletionRoutine
// 					);

			}
		}
// 		//////////////////////////////////////////////////////////////////////////
// 		//  [1/18/2007 San] ???
// 		char Sss[256];
// 		sprintf(Sss,"SizeBufSock%d",Handle);
// 		if (pDecoder)
// 			pDecoder->Srv->Telemetr(pDecoder,Sss,(double)rcvbuf_fill);
// 		else
// 		{
// 			Services *pSrv=GetServices();
// 			if (pSrv)
// 				pSrv->Telemetr(pSrv,Sss,(double)rcvbuf_fill);
// 		}
// 		//////////////////////////////////////////////////////////////////////////

	}
	return FillTmp;
	return 0;
}

int ClientSock::Receive(PUCHAR p,int c,int f)
{
	//if(!IsConnect()) return 0;
	
	int rc,err;
	//rc=recv(Handle,(PCHAR)p,c,f);
	rc=1;
	if(rc<0)
	{
		err=WSAGetLastErrorTrace();
		if(err==WSAEWOULDBLOCK)
		{

		}
		else // other errors
		{
			// Disconnect(); 
		}
		return 0;
	}
	else if(rc==0)
	{
		// socket gracefully closed
		// Disconnect();
		return 0;
	}
	else 
	{
		// âåòêà àëãîðèòìà ïðè ñ÷èòûâàíèè áëîêà äàííûõ
		// åñëè MSG_PEEK âîçâðàùàåò íåâåðíûå äàííûå
		// òðåáóåòñÿ óòî÷íåíèå ïàðàìåòðà rc
		if(f==MSG_PEEK)
		{
			unsigned long SocketDataLen;
			ioctlsocket(Handle,FIONREAD,&SocketDataLen);
			//WSAIoctl
			return SocketDataLen;
		}
		else
		{
		size_t iasm=1;
	}
		return rc;
	}
}



int	ClientSock::Connect()
{
//HOSTENT *he;
//in_addr	ina;
//	int	dblock;
	int	namelen=sizeof(client);
	int optval, optlen=sizeof(int);
//	CB_Rd=CB_Wr;

//	PFIXED_INFO	PI;
//	long		BLen;
//	GetNetworkParams (&PI,&BLen);
// 	SQLDMO_LPBSTR	SQLL;
//	GetNetName(&SQLL);
//	NetIOPermssion	NetIO;
	
	/*if(Flag==0)
	{
		int err;
		struct hostent *hp;
		struct	sockaddr_in host;
		strcpy(HostName,HostIP);

		memset(&host,0,sizeof(host));
		host.sin_family = AF_INET;
		int FirstChar=HostName[0];
		if(('0'<=FirstChar)&&(FirstChar<='9'))
		{
			strcpy(HostIP,HostName);
			int addr=inet_addr(HostIP);
			hp=gethostbyaddr((PCHAR)&addr,4,AF_INET);
			if ( hp == NULL ) { err=WSAGetLastError(); return -1; }
			else 
			{
				host.sin_addr = *( struct in_addr * )hp->h_addr;
				strcpy(HostName,hp->h_name); // update HostName
			}
		}
		else
		{
			hp = gethostbyname( HostName );
			if ( hp == NULL ) { err=WSAGetLastError(); return -1; }
			else 
			{
				host.sin_addr = *( struct in_addr * )hp->h_addr;
				strcpy(HostIP,inet_ntoa(host.sin_addr)); // HostIP
			}
		}
		host.sin_port = htons( PortTCP );

		Trace("try connect to",PortTCP,HostName);
		
		Handle=socket(AF_INET, SOCK_STREAM, 0); if(Handle<0) return 0;
		DWORD dontblock=1; int rc=ioctlsocket(Handle, FIONBIO, &dontblock);
		setsockopt(Handle,SOL_SOCKET,SO_RCVBUF,(char*)&rcvbufsz,sizeof(rcvbufsz));
		setsockopt(Handle,SOL_SOCKET,SO_SNDBUF,(char*)&sndbufsz,sizeof(sndbufsz));
		rc=connect(Handle, (struct sockaddr *)&host, sizeof(host));
		if(rc<0)
		{ 
			err=WSAGetLastError();
			if (err==WSAEWOULDBLOCK)
			{
				for(size_t i=0;i<10;i++)
				{
					UCHAR pLocalBuf[256];
					rc=recv(Handle,(PCHAR)pLocalBuf,0,0);
					if(rc<0)
					{
						err=WSAGetLastErrorTrace();
						//err=WSAGetLastErrorTrace1(int(this));
						if(err==WSAENOTCONN)
						{
							Sleep(100); continue;
						}
					}
					break;
				}
			}
			else
			{
				closesocket(Handle); Handle=-1; return -1;
			}
		}
		if( (err==WSAENOTCONN) || (err==WSAENOTSOCK)) 
		{
			closesocket(Handle); Handle=-1; return -1;
		} 
		else
		{
			Flag=1;
			if (CallBackEvent!=0)
				(*CallBackEvent)(this,CBE_Connect);
			Trace("connect",PortTCP,HostName);
		}
	}

	return 0;*/

	if(Flag==0)
	{
		int err=0;
		struct hostent *hp;
		struct	sockaddr_in host;

//		CB_Rd=CB_Wr=0;

		//strcpy(HostIP,HostName);
		//strcpy(HostName,HostIP);
		int FirstChar=HostIP[0];

		memset(&host,0,sizeof(host));
		host.sin_family = AF_INET;
		if(('0'<=FirstChar)&&(FirstChar<='9'))
		{
			int addr=inet_addr(HostIP);
			if(addr==INADDR_NONE) return -1;
			host.sin_addr=*( struct in_addr * )(&addr);

			IPadr=addr;
			if (pService)
			{
				if (!pService->TestHost(IPadr))
					return -1;
			}
			// ñåðâèñ äëÿ îïðåäåëåíèÿ host name
			if(*HostName==0)
			{
				if (pService)
				{
					char*		tpCh;
					tpCh=pService->GetHostByAddr(HostIP);
					if (tpCh!=NULL)
					{
						strcpy(HostName,tpCh); // update HostName
						TestNetName(HostName);
					}
					else
					{
						strcpy(HostName,HostIP); // update HostName
					}
				}
				else
				{
					hp=gethostbyaddr((PCHAR)&addr,4,AF_INET);
					if ( hp == NULL )
					{ err=WSAGetLastError(); /*return -1;*/ }
					else
						strcpy(HostName,hp->h_name); // update HostName
					TestNetName(HostName);
				}
					
			}
		}
		else
		{
			// ñåðâèñ äëÿ îïðåäåëåíèÿ IP address
			strcpy(HostName,HostIP);
//			hp = gethostbyname(HostIP);
//			if ( hp == NULL ) { err=WSAGetLastError(); return -1; }
//			else 
//			{
			host.sin_addr = GetServices()->IPAddrLAN;
//				host.sin_addr = *( struct in_addr * )hp->h_addr;
			strcpy(HostIP,inet_ntoa(host.sin_addr)); // HostIP
//			}
		}

		host.sin_port = htons( PortTCP );

		Trace("try connect to",PortTCP,HostName);
		
		Handle=socket(AF_INET, SOCK_STREAM, 0);
		if(Handle==-1) 
			return 0;
		DWORD dontblock=1; int rc=ioctlsocket(Handle, FIONBIO, &dontblock);
		setsockopt(Handle,SOL_SOCKET,SO_RCVBUF,(char*)&rcvbufsz,sizeof(rcvbufsz));
		setsockopt(Handle,SOL_SOCKET,SO_SNDBUF,(char*)&sndbufsz,sizeof(sndbufsz));
		optval=1; rc=setsockopt(Handle,IPPROTO_TCP,TCP_NODELAY,(char*)&optval,optlen);
		rc=connect(Handle, (struct sockaddr *)&host, sizeof(host)); Sleep(1);
		if(rc<0)
		{ 
			err=WSAGetLastError();
			if (err==WSAEWOULDBLOCK)
			{
				UCHAR pLocalBuf[256];
				for(size_t i=0;i<20;i++)
				{
					rc=recv(Handle,(PCHAR)pLocalBuf,0,0);
					if(rc<0)
					{
						err=WSAGetLastErrorTrace();
						//err=WSAGetLastErrorTrace1(int(this));
						if(err==WSAENOTCONN)
						{
							Sleep(10); continue;
						}
					}
					break;
				}
			}
			else
			{
				closesocket(Handle); Handle=-1; return -1;
			}
		}
		if( (err==WSAENOTCONN) || (err==WSAENOTSOCK)) 
		{ 

			closesocket(Handle); Handle=-1; return -1;
		} 
		else
		{
			Dmp.Clear();
			Dmp0.Clear();
			Dmp1.Clear();
			Dmp.SetPrm(pDecoder,(void*)this,&DmpPrcClientSock);
			Dmp0.SetPrm(pDecoder,(void*)this,&DmpPrcClientSock);
			Dmp1.SetPrm(pDecoder,(void*)this,&DmpPrcClientSock);
			Flag=1;
			if (pService) 
				pService->GetRealTime(&TimeConn.FTime);
			else
				GetSystemTimeAsFileTime(&TimeConn.FTime);
			TimeDisConn.Time=0;
			if (CallBackEvent!=0)
				(*CallBackEvent)(this,CBE_Connect);
			Trace("connect",PortTCP,HostName);
		}
	}

	return 0;
/*



	
	if(Flag==0)
	{
	  // Connect
	  Trace("try connect to",PortTCP,HostIP);
	  Handle = socket(AF_INET, SOCK_STREAM, 0);
      if (Handle < 0)
      { 
		Handle=-1; return -1; 
	  }
	if (SMode==1)
	{	// Áðàòü àðåññ èç òàáëèöû
		if (++(*(3+(UCHAR*)&server.sin_addr.s_addr))==0)
			*(3+(UCHAR*)&server.sin_addr.s_addr)=1;
	}
	getpeername(Handle,(sockaddr*)&server,&namelen);

	IPadr=htonl(*(long*)&server.sin_addr);
	strcpy(HostIP,inet_ntoa(server.sin_addr));

	dblock=inet_addr(HostIP);
	memcpy((struct in_addr*)&ina, &dblock, 4);

//	he=gethostbyaddr((const char*)&dblock,4,AF_INET);
//	if (he)
//		strcpy(HostName, he->h_name);
//	else
//		*HostName=0;

	  DWORD dontblock=1; int rc=ioctlsocket(Handle, FIONBIO, &dontblock);

	  setsockopt(Handle,SOL_SOCKET,SO_RCVBUF,(char*)&rcvbufsz,sizeof(rcvbufsz));
	  setsockopt(Handle,SOL_SOCKET,SO_SNDBUF,(char*)&rcvbufsz,sizeof(sndbufsz));
      
	  rc=connect(Handle, (struct sockaddr *)&server, sizeof(server));

	  if(rc<0)
      { 
		rc=WSAGetLastErrorTrace1((int)this);

	    if (rc==WSAEWOULDBLOCK)
		{
	      Srd.fd_array[0]=Handle; Srd.fd_count=1;
	      Swr.fd_array[0]=Handle; Swr.fd_count=0;
	      Sex.fd_array[0]=Handle; Sex.fd_count=0;
	      struct timeval timeout;
	      timeout.tv_sec=1; timeout.tv_usec=0;
	      rc=select(Handle,&Srd,&Swr,&Sex,&timeout);

		  rc=getsockopt(Handle,SOL_SOCKET,SO_ERROR,(char*)&optval,&optlen);
	      if (optval)
		  {
			  closesocket(Handle); Handle=-1; return -1;
		  }
		  optval=1; rc=setsockopt(Handle,IPPROTO_TCP,TCP_NODELAY,(char*)&optval,optlen);
// 		  closesocket(Handle);
//	      return -1;
	    }
	    else
	    {
		  closesocket(Handle);Handle=-1;
	      return -1;
	    }
      }
	he=gethostbyaddr((const char*)&dblock,4,AF_INET);
	if (he)
		strcpy(HostName, he->h_name);
	else
		strcpy(HostName,HostIP);
//		*HostName=0;
//	if (HostName[0]==0)		// !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
//	{
//		closesocket(Handle);Handle=-1;
//		return -1;
//	};


	Flag=1;
	  if (CallBackEvent!=0)
			(*CallBackEvent)(this,CBE_Connect);
	  Trace("connect",PortTCP,HostIP);
	  return 0;
	}
	else
	{
	  return -1;
	}
*/
}
int ClientSock::DisConnect(char *mess,int rc)
{
//	Flag=0;
//	Dmp.Clear();
	SmBuf=0;
	tSm=0;
//	Fl_Calc=0;
//	CB_Rd=CB_Wr=0;
	// DisConnect -----------------
	if(mess==NULL)
	{
		Trace("disconnect",PortTCP,HostName);
	}
	else
	{
		char s[256];
		sprintf(s,"disconnect %s rc=%d",mess,rc);
		Trace(s,PortTCP,HostName);
	}
	// ----------------------------

	if (Handle!=-1)
	{
		if (pService) 
			pService->GetRealTime(&TimeDisConn.FTime);
		else
			GetSystemTimeAsFileTime(&TimeDisConn.FTime);
		if (CallBackEvent!=0)
			(*CallBackEvent)(this,CBE_DisCon);
		closesocket(Handle);
		Handle=-1;
		Flag=0;

	}

	return 0;
}

int	ClientSock::WaitConnect(int time)
{
size_t i;DWORD q=(DWORD)time/100;
	for(i=0;i<100;i++)
	{
	  if(Flag==1) return 0;
	  else Sleep(q);
	}
	return -1;

}
int	ClientSock::SendBuf(char *wbuf,int wlen,int Priority)
{
	int	rc=0;
	if (Flag==0) 
	{
		Dmp.Clear();
		Dmp0.Clear();
		Dmp1.Clear();
		if (pService)
		{
			FileTime tTime;
			pService->GetRealTime(&tTime.FTime);
			if (tTime.Time-TimeDisConn.Time>KtimeS)
			{
				if (hCE==NULL)
				{
					hCE=pService->SetCriticalError(pDecoder,"CS_DataLost","Warning","SendBuf",hCE);
					pService->OutLog("CS_DataLost",pDecoder,"DataLost");
				}
			}
		};
		return 0;
	}
	if (wlen==0) return rc;
	switch(Mode)
	{
	case 0:
		SendNotBuf(wbuf,wlen);
		break;
	case 1:
		SendCircBuf(wbuf,wlen);
		break;
	case 2:
		if (Priority<=PriorLev)
			rc=Dmp0.Data((UCHAR*)wbuf,wlen);
		else
			rc=Dmp1.Data((UCHAR*)wbuf,wlen);
		if (rc==-1)
		{
			DisConnect();
			if(pService)
				pService->OutLog("ClientSock(SendCircBuf) TimeOut Disconnect!!!",pDecoder,"CliSock");

		}
		break;
	case 3:
		HeaderBuf.Len=wlen;
		SendCircBuf((char*)&HeaderBuf,sizeof(HeaderBuf));
		SendCircBuf(wbuf,wlen);
		break;
	default:
		if(pService)
			pService->SetCriticalError(pDecoder,"Mode>2","Warning","ClientSock");
	};
  return rc;
};

int	ClientSock::SendCircBuf(char* wbuf,int wlen)
{
	int	rc=0;
	rc=Dmp.Data((UCHAR*)wbuf,wlen);
	if (rc==-1)
	{
		DisConnect();
		if(pService)
			pService->OutLog("ClientSock(SendCircBuf) TimeOut Disconnect!!!",pDecoder,"CliSock");

	}
	return rc;
};


int	ClientSock::SizeCB(int Size)
{
/*	for (size_t i=0;i<100;i++)
	{
		if (CB_Wr==CB_Rd) break;
		Sleep(10);
	}
//	free(CBuf);
	if (Size!=CB_Len)
	{
		if (pDecoder)
			CBuf=(UCHAR*)pDecoder->realloc(CBuf,Size);
		else
			CBuf=(UCHAR*)realloc(CBuf,Size);
		CB_Len=Size;
	}
	return CB_Len;
*/
	return Size;
};

int ClientSock::SendFromBuf()
{
	int	rc=0;
	if (Flag!=0)
	{
		FileTime	TTime;
		if (pService)
			pService->GetRealTime(&TTime.FTime);
		else
			GetSystemTimeAsFileTime(&TTime.FTime);
		if (TTime.Time-TimeConn.Time>KtimeS)
		{
			rc=1;
			while (rc)
			{
				rc=Dmp.OData();
				if (rc>0)
				{
					PriorState=2;
				}
				if ((rc==0)&&(PriorState<2))
				{
					rc=Dmp1.OData();
					if (rc>0)
					{
						PriorState=1;
					}
				}
				if ((rc==0)&&(PriorState<1))
				{
					rc=Dmp0.OData();
				}
				if (rc==0)
					PriorState=0;
			}
		}
	}
	return rc;
};

int	ClientSock::SendNotBuf(char *Wbuf,int Wlen)
{
	int	rc=0;
	int	TimeO=0;
	int			tlen,wlen;
	char		ttBuf[128];
	char*		tbuf;
	char*		wbuf;
	switch(Mode)
	{
	case 2:
		{
			SockHeader* Header=(SockHeader*)Wbuf;
			Header->CntPack=GlobalCnt++;
/*			SockEl=(Sock_El*)IDL.NewElem();
			if (SockEl==NULL)
				SockEl=new Sock_El((UCHAR*)Wbuf,Wlen);
			else
				SockEl->Init((UCHAR*)Wbuf,Wlen);
			rc=IDL.Add(SockEl);
			if (rc==-1)
			{
				ID_Elem* tIDEl=IDL.Find((UCHAR*)&Header->CntPack,sizeof(int));
				IDL.Del(tIDEl);
				rc=IDL.Add(SockEl);
			}
*/		}
		break;
	}
	if (Flag==0)
	{
		FileTime tTime;
		pService->GetRealTime(&tTime.FTime);
		if (tTime.Time-TimeDisConn.Time>KtimeS)
		{
			hCE=pService->SetCriticalError(pDecoder,"CS_DataLost","Warning","SendNotBuf",hCE);
		}
		/*CB_Rd=CB_Wr=0;*/ return 0;
	}
	tbuf=Wbuf;
	tlen=Wlen;
	do
	{
		if (tlen>MaxTrPack)
		{
			wlen=MaxTrPack;
			tlen-=MaxTrPack;
			wbuf=tbuf;
			tbuf+=MaxTrPack;
		}
		else
		{
			wlen=tlen;
			wbuf=tbuf;
			tlen=0;
		}
_p12:
		rc=send(Handle, wbuf, wlen, 0);
		if (wlen!=rc)
		{
			if (rc!=-1)
			{
				pService->SetCriticalError(pDecoder,"rc=Send=-1","Error","SockCli");
			}
			rc=WSAGetLastErrorTrace1(rc/*(int)this*/);
			if(
				(rc==WSAECONNRESET) ||
				(rc==WSAECONNABORTED)
				)
			{ 
				DisConnect("SendNotBuf RESET or ABORTED"); //Flag=0; 
			}
			if(rc==WSAEWOULDBLOCK)
			{
	//			if(ReceiveDisconnect(Handle,(int)this)==0)
				{
//					Sleep(1);
					if (TimeO++<TimeOut) goto _p12;
					Sleep(10);
					if (TimeO++<TimeOut+TimeOut) goto _p12;
					if(pService)
						pService->OutLog("ClientSock(SendNotBuf) TimeOut !",pDecoder,"CliSock");

				};
			};
			if(pService)
				pService->OutLog("ClientSock(SendNotBuf) Disconnect!",pDecoder,"CliSock");
			DisConnect(); //Flag=0;
			break;
		}
		else
		{
			PackCnt+=wlen;TimeO=0;
		}
	} while (tlen>0);
	if (rc!=-1) rc=Wlen;
	if (pService)
	{
		if (!pService->TestHost(IPadr))
		{
			sprintf(ttBuf,"DisConectSocket because Host=%p Error",pDecoder/*,*((UCHAR*)(&IPadr)+3)*/);
			pService->OutLog(ttBuf,pDecoder,"CliSock");
			DisConnect();
		}
	}
	return rc;
}

int ClientSock::Idle()
{

	UCHAR*	tBufCl;
	int	tLen,tCnt;
	int	rc=0;	//,rc2,i;
	int	cnt=0;
	long	rB=0;

	if(Flag==0) {/*CB_Rd=CB_Wr=0;*/return 0; }
	
	Srd.fd_array[0]=Handle; Srd.fd_count=1;
	Swr.fd_array[0]=Handle; Swr.fd_count=0;
	Sex.fd_array[0]=Handle; Sex.fd_count=0;
	struct timeval timeout;
	timeout.tv_sec=0; timeout.tv_usec=25;
	select(Handle,&Srd,&Swr,&Sex,&timeout);
	
	if ((SizeBufCl-SmBuf)==0)
	{
		MdTr=1;
		SizeBufCl+=MaxPack;
		if (pDecoder)
			tBufCl=(UCHAR*)pDecoder->realloc(BufCl,SizeBufCl);
		else
			tBufCl=(UCHAR*)realloc(BufCl,SizeBufCl);
//		SmBuf=tSm+tLen;
		if (tBufCl!=NULL)
		{
			BufCl=tBufCl;
		}
		else
		{	// Error Alloc 
			if(pService)
			{
				char	tB[128];
				sprintf(tB,"CS(Idle) P:%d Host:%s Error realloc!!!",PortTCP,HostName);
				pService->SetCriticalError(NULL,tB,"Error");
				pService->OutLog(tB,pDecoder,"CliSock");
			}
			DisConnect();
			return -1;
		};
	}
	
	GetFill();
	
	rc=recv(Handle, (char*)BufCl+SmBuf, SizeBufCl-SmBuf, 0);
	
	switch(rc)
	{
	  case -1:
				//rc=WSAGetLastErrorTrace1((int)this);
				rc=WSAGetLastError();
				if (rc==WSAEWOULDBLOCK) return 0;
				DisConnect(); //Flag=0;
				break;
	  case 0:
				DisConnect(); //Flag=0;
				break;
	  default:
			PackCntIn+=rc;
//			if(pService)
//			{
//				pService->Telemetr(pDecoder,"CliSockPackCntIn",(double)PackCntIn,this);
//			}
			switch(Mode)
			{ 
			case 0:
				if (rc>0) if (readFunction)
				(*readFunction)(this,(UCHAR*)BufCl, rc);
				break;
			case 1:
				if (rc>0) if (readFunction)
				(*readFunction)(this,(UCHAR*)BufCl, rc);
				break;
			case 2:
				if (rc>0) if (readFunction)
				{
					if (SmBuf==0) tSm=0;
					tLen=rc+SmBuf-tSm;
//					tLen=rc+SmBuf;
//					if (SmBuf==0) tSm=0;
					while(tLen!=0)
					{
						// Ðàçáîð áóôåðà äàííûõ
//						if (MdTr==0)
//						{
							if (tLen<sizeof(SockHeader)) 
							{	// Ìàëîâàòî äëÿ àíàëèçà
								SmBuf=tSm+tLen;
								break;
							};
							if (*(int*)(BufCl+tSm)==Syn1)
							{
								tCnt=*(int*)(BufCl+tSm+4);
								if ((tCnt+sizeof(SockHeader))<=tLen)
								{	// ïàêåò ïåðåäàí ïîëíîñòüþ ok!!!
									if (Fl_GCntIn==0)
									{
										GlobalCntIn=((SockHeader*)(BufCl+tSm))->CntPack+1;
										Fl_GCntIn=1;
									}
									else
									{
										if (GlobalCntIn==((SockHeader*)(BufCl+tSm))->CntPack)
										{
											GlobalCntIn++;
										}
										else
										{
//											pService->SetCriticalError(pDecoder,"PackCntWrong","Error","SockCli");
											GlobalCntIn=((SockHeader*)(BufCl+tSm))->CntPack;
										};
									}
									(*readFunction)(this,(UCHAR*)BufCl+tSm+sizeof(SockHeader),tCnt);
									tSm+=(tCnt+sizeof(SockHeader));
									tLen-=(tCnt+sizeof(SockHeader));
									SmBuf=0;
								}
								else
								{	// ïàêåò ïåðåäàí íå ïîëíîñòüþ - çàðàçà !!!
									if ((tSm+tLen+sizeof(SockHeader))>SizeBufCl)
									{	// Íå âëåç â áóôåð !!!
										MdTr=100;
										SizeBufCl+=10*MaxPack;
										if (pDecoder)
											tBufCl=(UCHAR*)pDecoder->realloc(BufCl,SizeBufCl);
										else
											tBufCl=(UCHAR*)realloc(BufCl,SizeBufCl);
										if (tBufCl!=NULL)
										{
											BufCl=tBufCl;
										}
										else
										{	// Error Alloc 
											if(pService)
											{
												char	tB[128];
												sprintf(tB,"CS(Idle) P:%d Host:%s Error realloc!!!",PortTCP,HostName);
												pService->SetCriticalError(NULL,tB,"Error");
												pService->OutLog(tB,pDecoder,"CliSock");
											}
											DisConnect();
											return -1;
										};
										SmBuf=tSm+tLen;
										break;
									}
									else
									{	// Êîïèðóåì äàííûå è æäåì !!!
										if ((tSm+tLen+sizeof(SockHeader))>(SizeBufCl-MaxPack)) 
											MdTr=100;
										memcpy(BufCl,BufCl+tSm,tLen);
										tSm=0;
										SmBuf=tLen;
										break;
									};
								};
							}
							else
							{
//								Fl_Calc=0;
								if(pService)
								{
									char	tB[128];
									sprintf(tB,"CS(Idle) P:%d Host:%s Syn1 Error!!!",PortTCP,HostName);
									pService->SetCriticalError(NULL,tB,"Error");
									pService->OutLog(tB,pDecoder,"CliSock");
								}
								else
								{
									Beep(1000,100);
									Beep(3000,100);
								}
								DisConnect();
								break;
							}
//						}
//						else
//						{
//							MdTr=0;
//						}
					}
					if (tLen==0)
					{
						if 	(SizeBufCl>MaxPack)
						{
							if (MdTr==0)
							{
								SizeBufCl-=MaxPack;
								if (pDecoder)
									tBufCl=(UCHAR*)pDecoder->realloc(BufCl,SizeBufCl);
								else
									tBufCl=(UCHAR*)realloc(BufCl,SizeBufCl);
								if (tBufCl!=NULL)
								{
									BufCl=tBufCl;
								}
								else
								{	// Error Alloc 
									if(pService)
									{
										char	tB[128];
										sprintf(tB,"CS(Idle) P:%d Host:%s Error realloc!!!",PortTCP,HostName);
										pService->SetCriticalError(NULL,tB,"Error");
										pService->OutLog(tB,pDecoder,"CliSock");
									}
									DisConnect();
									return -1;
								};
							}
							else MdTr--;
						}
					}
				}
				break;
			};
			//GetFill();
	}
	return rc;
// ------------
}
// ---------------------------------------------------------------------
ClientList::ClientList(int q)
{
	n=q; if(n>256) n=256;
	//list=(pItemClientList*)malloc(n*sizeof(pItemClientList));
	for(size_t i=0;i<n;i++)
	{
		//list[i]=new ItemClientList();
		list[i].f=0;
		list[i].obj=0;
		list[i].id=0;
	}
}
ClientList::~ClientList()
{
	for(size_t i=0;i<n;i++)
	{
		if(list[i].obj) 
		{
			list[i].f=0;
			delete list[i].obj;
			list[i].obj=0;
			//delete list[i];
		}
	}
	//free(list);
}
int ClientList::AddID(int q, ServerClientSock* obj, PVOID ptr)
{
	for(size_t i=0;i<n;i++)
	{
		if(!list[i].f)
		{
			//list[i].id=htons(q);

			SOCKADDR_IN* pSA=(SOCKADDR_IN*)ptr;
			int addr=htonl(pSA->sin_addr.S_un.S_addr);
			int port=htons(pSA->sin_port); obj->port=port;
			list[i].id=((addr&0xffff)<<16)|port;

			list[i].obj=obj;
			obj->NumInList=i;	// Êëèåíò ñåðâåðà ñîêåòîâ äîëæåí çíàòü ñâîé íîìåð â ñïèñêå
			list[i].f=1;

			//list[i].pObjPtr=(int)obj;
			list[i].pObjPtr=obj;
			list[i].SockHandle=obj->Handle;

			list[i].port=-1; strcpy(list[i].ip,"undefined");

			if(ptr)
			{
				strcpy(list[i].ip,inet_ntoa(pSA->sin_addr));
				list[i].port=htons(pSA->sin_port);
			}

			return 0;
		}
	}
	return 1;
}
void ClientList::DelID(int q)
{
	for(size_t i=0;i<n;i++)
	{
		if(list[i].f)
		{
			if(list[i].id==q)
			{
				list[i].f=0;
				delete list[i].obj;
				list[i].obj=NULL;
				break;
			}
		}
	}
}

int	FunNul(LPVOID s)
{
	return ((ClientSock*)s)->SendFromBuf();
}


ClientSockEx::ClientSockEx(char *ip, int port,Services *pSrv,Decoder* pDec)
{
	if (pSrv==NULL)
		pSrv=GetServices();
	if (pDec!=NULL)
	{
		free(OutBuf);
		free(BufCl);
		OutBuf=(char*)pDec->malloc(LenOBuf);
		BufCl=(UCHAR*)pDec->malloc(SizeBufCl);
	}
	int	tIP;
	Fl_Calc=0;//Fl_DB=0;
	TimeOut=1000;
	Div=0;
	strcpy(HostIP,ip); PortTCP=port; *HostName=0;
	pService=pSrv; pDecoder=pDec;
	if (pService) pService->GetRealTime(&RdTime.FTime);
	Dmp.SetPrm(pDecoder,(void*)this,&DmpPrcClientSock);
	Dmp0.SetPrm(pDecoder,(void*)this,&DmpPrcClientSock);
	Dmp1.SetPrm(pDecoder,(void*)this,&DmpPrcClientSock);
	server.sin_family = AF_INET;
	server.sin_port   = htons(PortTCP);
	server.sin_addr.s_addr = inet_addr(HostIP);
	tIP=server.sin_addr.s_addr;
	if (*((UCHAR*)(&tIP)+3)==0)
	{
		SMode=1;
	}
	else
		SMode=0;
	gethostname(MyNetName,64);
	TestNetName(MyNetName);

	rc=rc2=Fl_RS=tlen=0;

	ClThread=new ClassThread("ClientSockExFunNul",FunNul,(LPVOID)this,THREAD_PRIORITY_HIGHEST);
	
};

int ClientSockEx::ReInit(char *ip, int port,char *host)
{
	Trace("reinit",port,ip);

	if( (strcmp(HostIP,ip)==0) && (PortTCP==port) ) return 0;

	delete ClThread; ClThread=NULL;
	DisConnect();

	strcpy(HostIP,ip); PortTCP=port;
	if( (host!=NULL) && (*host) ) 
	{
		strcpy(HostName,host);	TestNetName(HostName);
	}
	else 
		strcpy(HostName,HostIP);
//		*HostName=0;
	server.sin_family = AF_INET;
	server.sin_port   = htons(PortTCP);
	server.sin_addr.s_addr = inet_addr(HostIP);

	ClThread=new ClassThread("ClientSockExFunNul",FunNul,(LPVOID)this,THREAD_PRIORITY_HIGHEST);
	
	return 0;
}

int	ClientSockEx::ReadSock(UCHAR* &Buf, size_t &Cnt)
{
	Cnt=0;
	UCHAR*	tBufCl;
	int		cnt=0,rc=0;
	long	rB=0;
	FileTime	tTime;
	tTime.Time=0;
//	ULONG	sockcnt;
	if(Flag==0) {/*CB_Rd=CB_Wr=0;*/ return 0; }

	if (pService) pService->GetRealTime(&tTime.FTime);
	if (tTime.Time-RdTime.Time>2*KtimeS)
	{
		RdTime.Time=tTime.Time;
 		return 0;
	}
	if (Fl_Calc==0)
	{
/*		if (Div++>100)
		{
			Div=0;
		}
		else
		{
			rc=ioctlsocket(Handle,FIONREAD,&sockcnt);
			if (rc==0)
			{
				if (sockcnt==0) 
					return 0;
			};
			Div=0;
		};
*/		Srd.fd_array[0]=Handle; Srd.fd_count=1;
		Swr.fd_array[0]=Handle; Swr.fd_count=0;
		Sex.fd_array[0]=Handle; Sex.fd_count=0;
		struct timeval timeout;
//		timeout.tv_sec=0; timeout.tv_usec=25;
		timeout.tv_sec=0; timeout.tv_usec=0;
		select(Handle,&Srd,&Swr,&Sex,&timeout);
		if ((SizeBufCl-SmBuf)==0)
		{
			SizeBufCl+=MaxPack;
			if (pDecoder)
				tBufCl=(UCHAR*)pDecoder->realloc(BufCl,SizeBufCl);
			else
				tBufCl=(UCHAR*)realloc(BufCl,SizeBufCl);
			if (tBufCl!=NULL)
			{
				BufCl=tBufCl;
			}
			else
			{	// Error Alloc 
				if(pService)
				{
					char	tB[128];
					sprintf(tB,"CSE(Idle) P:%d Host:%s Error realloc!!!",PortTCP,HostName);
					pService->SetCriticalError(NULL,tB,"Error");
					pService->OutLog(tB,pDecoder,"CliSock");
				}
				DisConnect();
				return -1;
			};
			MdTr=1;
		};
		GetFill();
		rc=recv(Handle,(char*)BufCl+SmBuf, SizeBufCl-SmBuf, 0);
		switch(rc)
		{
		  case -1:
					//rc=WSAGetLastErrorTrace1((int)this);
					rc=WSAGetLastError();
					if (rc==WSAEWOULDBLOCK)
					{
						if (pService) pService->GetRealTime(&RdTime.FTime);
						return 0;
					}
					DisConnect(); //Flag=0;
					break;
		  case 0:
					DisConnect(); //Flag=0;
					tLen=0;//Flag=0; 
					Fl_Calc=0;
					break;
		  default:
// ------------
			if (rc>0)
			{
				PackCntIn+=rc;
//				if(pService)
//				{
//					pService->Telemetr(pDecoder,"CliSockPackCntIn",(double)PackCntIn,this);
//				}
				Fl_Calc=1;
				if (SmBuf==0) tSm=0;
				tLen=rc+SmBuf-tSm;
//				tLen=rc+SmBuf;
//				if (SmBuf==0) tSm=0;
			}
			else
			{
				if (pService) pService->GetRealTime(&RdTime.FTime);
			};	
			break;
		}
		//GetFill();
	};
	switch(Mode)
	{
	case 0:
		Buf=(UCHAR*)BufCl;
		Cnt=rc;
		Fl_Calc=0;SmBuf=0;
	break;
	case 1:
		Buf=(UCHAR*)BufCl;
		Cnt=rc;
		Fl_Calc=0;SmBuf=0;
		break;
	case 2:
		if (Fl_Calc==1)
		{
			if(tLen!=0)
			{
				// Ðàçáîð áóôåðà äàííûõ
//				if (MdTr==0)
//				{
_FindSin:
				if (tLen<sizeof(SockHeader)) 
					{	// Ìàëîâàòî äëÿ àíàëèçà
						SmBuf=tSm+tLen;
						Fl_Calc=0;
					}
					else
					if (*(uchar*)(BufCl+tSm)==Syn1)
					{
						tCnt=*(USHORT*)(BufCl+tSm+2);
						if ((tCnt+sizeof(SockHeader))<=tLen)
						{	// ïàêåò ïåðåäàí ïîëíîñòüþ
							if (Fl_GCntIn==0)
							{
								GlobalCntIn=((SockHeader*)(BufCl+tSm))->CntPack+1;
								Fl_GCntIn=1;
							}
							else
							{
								if (GlobalCntIn==((SockHeader*)(BufCl+tSm))->CntPack)
								{
									GlobalCntIn++;
								}
								else
								{
//									pService->SetCriticalError(pDecoder,"PackCntWrong","Error","SockCliEx");
									GlobalCntIn=((SockHeader*)(BufCl+tSm))->CntPack;
								};
							}
//							(*readFunction)(this,(UCHAR*)bufcl+tSm+sizeof(SockHeader),tCnt);
							Buf=(UCHAR*)BufCl+tSm+sizeof(SockHeader);
							Cnt=tCnt;
							tSm+=(tCnt+sizeof(SockHeader));
							tLen-=(tCnt+sizeof(SockHeader));
							SmBuf=0;
						}
						else
						{	// ïàêåò ïåðåäàí íå ïîëíîñòüþ - çàðàçà !!!
							if ((tSm+tLen+sizeof(SockHeader))>SizeBufCl)
							{	// Íå âëåç â áóôåð !!!
								SizeBufCl+=10*MaxPack;
								if (pDecoder)
									tBufCl=(UCHAR*)pDecoder->realloc(BufCl,SizeBufCl);
								else
									tBufCl=(UCHAR*)realloc(BufCl,SizeBufCl);
								if (tBufCl!=NULL)
								{
									BufCl=tBufCl;
								}
								else
								{	// Error Alloc 
									if(pService)
									{
										char	tB[128];
										sprintf(tB,"CSE(Idle) P:%d Host:%s Error realloc!!!",PortTCP,HostName);
										pService->SetCriticalError(NULL,tB,"Error");
										pService->OutLog(tB,pDecoder,"CliSock");
									}
									DisConnect();
									return -1;
								};
								SmBuf=tSm+tLen;
								MdTr=100;
							}
							else
							{	// Êîïèðóåì äàííûå è æäåì !!!
								if ((tSm+tLen+sizeof(SockHeader))>(SizeBufCl-MaxPack)) 
									MdTr=100;
								memcpy(BufCl,BufCl+tSm,tLen);
								tSm=0;
								SmBuf=tLen;
								Fl_Calc=0;
							};
						};
					}
					else
					{	
						tLen--;tSm++;
						goto	_FindSin;

						Fl_Calc=0;
						if(pService)
						{
							char	tB[128];
							int Smm=0;
							sprintf(tB,"CSEx(RSock) P:%d Host:%s Syn1 Error!!!",PortTCP,HostName);
							pService->SetCriticalError(NULL,tB,"Error");
							pService->OutLog(tB,pDecoder,"CliSock");

							
//  [10/13/2006 San] Temp ???

							Smm+=sprintf(tB,"ErrPack Len(%d):",tLen);
							for (size_t iii=0;iii<tLen;iii++)
							{
								if (Smm>120)
									break;
								Smm+=sprintf(tB+Smm,"%02X ",*(char*)(BufCl+tSm+iii));
							}
							pService->OutLog(tB,pDecoder,"CliSock");
//  [10/13/2006 San] Temp ??? End
						}
						else
						{
							Beep(1000,100);
							Beep(3000,100);
						}
						DisConnect();
						break;
					}
//				}
//				else
//				{
//					MdTr=0;
//				}
			}
			else
			{
				if 	(SizeBufCl>MaxPack)
				{
					if (MdTr==0)
					{
						SizeBufCl-=MaxPack;
						if (pDecoder)
							tBufCl=(UCHAR*)pDecoder->realloc(BufCl,SizeBufCl);
						else
							tBufCl=(UCHAR*)realloc(BufCl,SizeBufCl);
						if (tBufCl!=NULL)
						{
							BufCl=tBufCl;
						}
						else
						{	// Error Alloc 
							if(pService)
							{
								char	tB[128];
								sprintf(tB,"CSE(Idle) P:%d Host:%s Error realloc!!!",PortTCP,HostName);
								pService->SetCriticalError(NULL,tB,"Error");
								pService->OutLog(tB,pDecoder,"CliSock");
							}
							DisConnect();
							return -1;
						};
					}
					else MdTr--;
				};
				Fl_Calc=0;
			}
		}
		break;
	}
// ------------
	return	Cnt;
};

/*
ClientSockEx::~ClientSockEx()
{
	free(BufCl);
	free(CBuf);
	DisConnect();
	free(OutBuf);
};
*/

ServerClientSock* ClientList::GetID(int q)
{
	for(size_t i=0;i<n;i++)
	{
		if(list[i].f)
		{
			if(list[i].id==q)
			{
				return list[i].obj;
			}
		}
	}
	return NULL;
}
ServerClientSock* ClientList::Get(int i)
{
	if(list[i].f)
		return list[i].obj;
	else
		return NULL;
}
void ClientList::Del(int i)
{
	if(list[i].f)
	{
		list[i].f=0;
		delete list[i].obj;
		list[i].obj=NULL;
	}
}

int ClientList::Count()
{
	int count=0;
	for(size_t i=0;i<n;i++)
	{
		if(list[i].f) count++;
	}
	return count;
}

ItemClientList* ClientList::GetFirst()
{
	CurrentItem=0;
	return GetNext();
}
ItemClientList* ClientList::GetNext()
{
	for(size_t i=CurrentItem;i<n;i++)
	{
		CurrentItem=i+1;

		if(list[i].f)
		{
			return &list[i];
		}
	}
	return 0;
}

// ---------------------------------------------------------------------
ServerSock::ServerSock(int port,
		int (*_CallBackEvent)(ServerClientSock *csock,int Event),
		int	NType,
		int (*_readFunction)(ServerClientSock *csock,PUCHAR buf, size_t cnt),
		int	NumCli, 
		Services *pSrv,Decoder* pDec):

		portTCP(port), 
		CallBackEvent(_CallBackEvent), 
		readFunction(_readFunction),
		nClient(NumCli),
		pService(pSrv),
		pDecoder(pDec)
{
	NetType=NType;
	NumCliSrv=-1;
	CurrentClient=-1;

	CBSize=10000;
	char SemaphoreDescriptor[256];
	sprintf(SemaphoreDescriptor,"Neuron_0x%08p",(LPVOID)this);
	hmtx=OpenSemaphoreA(SEMAPHORE_ALL_ACCESS	,TRUE,
		SemaphoreDescriptor
		//"SRVSOCKSEM"
		);
	if (hmtx==NULL) 
		hmtx=CreateSemaphoreA(NULL,1,1,SemaphoreDescriptor);
	else
	{
		while (TRUE)
		{
			Beep(3000,200);
			Beep(1000,200);
			Beep(2000,200);
			Sleep(1);
		}
	}

	int     rc=0;
	in_addr	ina;
	DWORD dontblock=1;
	Mode=0;
	int	optlen=sizeof(int);

	List=new ClientList(nClient);

	gethostname(namehost, 19);
	TestNetName(namehost);
//	HostEnt=gethostbyname(namehost);
//	memcpy((struct in_addr*)&ina, HostEnt->h_addr, 4);
	switch(NetType)
	{
// 	case NT_Def:
// 		ina=GetServices()->IPAddrLAN;
// 		break;
	case NT_PAN:
		ina=GetServices()->IPAddrPAN;
		break;
	case NT_LAN:
		ina=GetServices()->IPAddrLAN;
		break;
	case NT_WAN:
		ina=GetServices()->IPAddrWAN;
		break;

	}
	memcpy(&IPadr, &ina, 4);
	strcpy(IPhost, inet_ntoa(ina));
	
	server.sin_family = AF_INET;
	server.sin_port   = htons(portTCP);
	switch(NetType)
	{
// 	case NT_Def:
// 		server.sin_addr.s_addr = *(ulong*)&pSrv->IPAddrLAN;//INADDR_ANY;
// 		break;
	case NT_PAN:
		server.sin_addr.s_addr = *(ulong*)&pSrv->IPAddrPAN;//INADDR_ANY;
		break;
	case NT_LAN:
		server.sin_addr.s_addr = *(ulong*)&pSrv->IPAddrLAN;//INADDR_ANY;
		break;
	case NT_WAN:
		server.sin_addr.s_addr = *(ulong*)&pSrv->IPAddrWAN;//INADDR_ANY;
		break;
	}

    if ((sHandle = socket(AF_INET, SOCK_STREAM, 0)) ==-1) SockExit();
	if (sHandle!=-1)
	{
	    rc=ioctlsocket(sHandle, FIONBIO, &dontblock);
	    rc=getsockopt(sHandle,SOL_SOCKET,SO_REUSEADDR, (char*)&dontblock, (int*)&optlen);
	    dontblock=1; rc=setsockopt(sHandle,SOL_SOCKET,SO_REUSEADDR, (char*)&dontblock, optlen);
	    if (bind(sHandle, (struct sockaddr *)&server, sizeof(server)) < 0) SockExit();
	    if (listen(sHandle, 1) != 0) SockExit();
	}
	ClThread=new ClassThread("ServerSockAccept1",Accept1,(LPVOID)this);
	
}

int	ServerSockEx::ReInit(int Port)
{
	delete ClThread; ClThread=NULL;

	delete List;

	if (sHandle!=-1) 
	{
		closesocket(sHandle);
		sHandle=-1;
	}

	portTCP=Port; // ----------------------

	NumCliSrv=-1;
	CurrentClient=-1;
//	CBSize=10000;

	int     rc=0;
	in_addr	ina;
	DWORD dontblock=1;
	// Mode=0;
	int	optlen=sizeof(int);

	List=new ClientList(nClient);

	gethostname(namehost, 19);
	TestNetName(namehost);
//	HostEnt=gethostbyname(namehost);
//	memcpy((struct in_addr*)&ina, HostEnt->h_addr, 4);
	switch(NetType)
	{
// 	case NT_Def:
// 		ina=GetServices()->IPAddrLAN;
// 		break;
	case NT_PAN:
		ina=GetServices()->IPAddrPAN;
		break;
	case NT_LAN:
		ina=GetServices()->IPAddrLAN;
		break;
	case NT_WAN:
		ina=GetServices()->IPAddrWAN;
		break;
	}
	memcpy(&IPadr, &ina, 4);
	strcpy(IPhost, inet_ntoa(ina));
	
	server.sin_family = AF_INET;
	server.sin_port   = htons(portTCP);
	switch(NetType)
	{
// 	case NT_Def:
// 		server.sin_addr.s_addr = *(ulong*)&pService->IPAddrLAN;//INADDR_ANY;
// 		break;
	case NT_PAN:
		server.sin_addr.s_addr = *(ulong*)&pService->IPAddrPAN;//INADDR_ANY;
		break;
	case NT_LAN:
		server.sin_addr.s_addr = *(ulong*)&pService->IPAddrLAN;//INADDR_ANY;
		break;
	case NT_WAN:
		server.sin_addr.s_addr = *(ulong*)&pService->IPAddrWAN;//INADDR_ANY;
		break;
	}
//	server.sin_addr.s_addr = *(ulong*)&pService->IPAddrLAN;//INADDR_ANY;

    if ((sHandle = socket(AF_INET, SOCK_STREAM, 0)) ==-1) SockExit();
	if(sHandle!=-1)
	{
		rc=ioctlsocket(sHandle, FIONBIO, &dontblock);
		rc=getsockopt(sHandle,SOL_SOCKET,SO_REUSEADDR, (char*)&dontblock, (int*)&optlen);
		dontblock=1; rc=setsockopt(sHandle,SOL_SOCKET,SO_REUSEADDR, (char*)&dontblock, optlen);
		if (bind(sHandle, (struct sockaddr *)&server, sizeof(server)) < 0) SockExit();
		if (listen(sHandle, 1) != 0) SockExit();
	}
	ClThread=new ClassThread("ServerSockExAccept1",Accept1,(LPVOID)this);
	
	return 0;
}

ServerSock::~ServerSock()
{
	delete ClThread; ClThread=NULL;
	CloseHandle(hmtx);

	delete List;

	if (sHandle!=-1) 
	{
		closesocket(sHandle);
		sHandle=-1;
	}
}

int	Accept1(LPVOID s)
{
	((ServerSock*)s)->Accept();
	Sleep(100);
	((ServerSock*)s)->TestClient();
	Sleep(100);
	return 0;
}

void ServerSock::DisConnect(int NumCli)
{
	if (NumCli >= List->n)
		return;
	WaitForSingleObject(
		hmtx,   // handle of semaphore
		INFINITE);          // zero-second time-out interval
	if (NumCli == -1)
	{
		for (size_t i = 0; i < List->n; i++)
		{
			if ((List->list[i].f != 0) && (List->list[i].obj->Flag != 0))
			{
				List->list[i].obj->DisConnect();
			}
		}
	}
	else
	{
		if ((List->list[NumCli].f != 0) && (List->list[NumCli].obj->Flag != 0))
		{
			if (NumCli >= List->n)
				return;
			List->list[NumCli].obj->DisConnect();;
		}
	}
	ReleaseSemaphore(hmtx, 1, NULL);
};

int	ServerSock::SendBuf(int NumCli,UCHAR* buf,size_t cnt)	// áëîêè ëþáîãî ðàçìåðà
{
	int	RetC=0;
	if (NumCli>=List->n)
		return 0;
	WaitForSingleObject( 
	        hmtx,   // handle of semaphore
	        INFINITE);          // zero-second time-out interval
	if (NumCli==-1)
	{
		for(size_t i=0;i<List->n;i++)
		{
			if ((List->list[i].f!=0)&&(List->list[i].obj->Flag!=0))
			{
				List->list[i].PackOut++;
				List->list[i].CountOut+=cnt;
				RetC=List->list[i].obj->SendBuf((char*)buf,cnt);
			}
		}
	}
	else
	{
		if ((List->list[NumCli].f!=0)&&(List->list[NumCli].obj->Flag!=0))
		{
			if (NumCli>=List->n)
				return -1;
			List->list[NumCli].PackOut++;
			List->list[NumCli].CountOut+=cnt;
			RetC=List->list[NumCli].obj->SendBuf((char*)buf,cnt);
		}
	}
	ReleaseSemaphore(hmtx,1,NULL); 
	return RetC;
};

void		ServerSock::TimeOut(int TOut)
{
	for (size_t i=0;i<List->n;i++)
	{
		if ((List->list[i].f!=0)&&(List->list[i].obj!=0))
		{
			(List->list[i].obj)->TimeOut=TOut;
		}
	}
};

/*

struct sockaddr_in {
        short   sin_family;
        u_short sin_port;
        struct  in_addr sin_addr;
        char    sin_zero[8];
};

*/

int	ServerSock::Accept()
{
int	rc=0;
SOCKET	handle;                // socket connected to client
int	namelen;               // length of client name
ServerClientSock *csock;

	namelen = sizeof(client);
	if ((handle = accept(sHandle, (struct sockaddr *)&client, &namelen)) == -1)
	{
	   //rc=WSAGetLastErrorTrace1((int)this);
	   rc=WSAGetLastError();
	   if (rc!=WSAEWOULDBLOCK)
	   {
		SockExit();
	   }
	   else return 0;
	}
	csock=new ServerClientSock(handle, this,pService,pDecoder);
	csock->SetMode(Mode);
	csock->SizeCB(CBSize);
	if(List->AddID(client.sin_port,csock,&client))
		delete csock;
	else
	{
//		Dmp.Clear();
//		Dmp.SetPrm(pDecoder);
		csock->TimeDisConn.Time=0;
		if (CallBackEvent!=0)
			(*CallBackEvent)(csock,CBE_Connect);
	}
    return 0;
};

int	ServerSock::SizeCB(int Size)
{
	int	rc=0;
	ServerClientSock *csock;
	CBSize=Size;
	for(int i=0;i<nClient;i++)
	{
		csock=(ServerClientSock*)List->Get(i);
		if(csock!=NULL) rc=csock->SizeCB(Size);
	}
	return rc;
};

int	ServerSock::TestClient()
{
	ServerClientSock *csock;
	for(int i=0;i<nClient;i++)
	{
		csock=(ServerClientSock*)List->Get(i);
		if(csock!=NULL)
			if (!csock->Flag)
			{
				
				WaitForSingleObject( 
			        hmtx,   // handle of semaphore
			        INFINITE);          // zero-second time-out interval*/
				if (pService) 
					pService->GetRealTime(&csock->TimeDisConn.FTime);
				else
					GetSystemTimeAsFileTime(&csock->TimeDisConn.FTime);

				if (CallBackEvent!=0)
					(*CallBackEvent)(csock,CBE_DisCon);
				List->Del(i);
				ReleaseSemaphore(hmtx,1,NULL); 
			}
	}
    return 0;
};


int	ServerSock::SockExit()
{
	if (sHandle!=-1)
		closesocket(sHandle);
	sHandle=-1;
	return 0;
};

int	ServerSockEx::Accept()
{
int	rc=0;
SOCKET	handle;                    // socket connected to client
int	namelen;               // length of client name
int		wlen=0;
//char	wbuf[1];
ServerClientSock *csock;

	namelen = sizeof(client);
	if ((handle = accept(sHandle, (struct sockaddr *)&client, &namelen)) == -1)
	{
	   //rc=WSAGetLastErrorTrace1((int)this);
	   rc=WSAGetLastError();
	   if (rc!=WSAEWOULDBLOCK)
	   {
		SockExit();
	   }
	   else 
		   return 0;
	}
	csock=(ServerClientSock*) new ServerClientSockEx(handle, this, pService,pDecoder);
	csock->SetMode(Mode);
	csock->SizeCB(CBSize);
	if(List->AddID(client.sin_port,csock,&client))
		delete csock;
	else
	{
//		Dmp.Clear();
//		Dmp.SetPrm(pDecoder);
		csock->TimeDisConn.Time=0;

		if (CallBackEvent!=0)
			(*CallBackEvent)(csock,CBE_Connect);
	}
    return 0;
};

int	ServerSockEx::ReadSock(int NumCli,UCHAR* &buf,size_t &cnt)	// áëîêè ëþáîãî ðàçìåðà
{
	int	RetC=0;size_t i;
	cnt=0;
	if (NumCli>=List->n)
		return 0;
	if (NumCli==-1)
	{
		if (LastNumCli>=List->n)
			LastNumCli=0;
		for(i=LastNumCli;i<List->n;i++)
		{
			if ((List->list[i].f!=0)&&(List->list[i].obj))
			{
				CurrentClient=i; // !

				RetC=((ServerClientSockEx*)(List->list[i].obj))->ReadSock(buf,cnt);
				if (RetC!=0) 
				{
					List->list[i].PackIn++;
					List->list[i].CountIn+=cnt;
					
					LastNumCli=i+1;
					break;
				}
			}
		}
		if (i>=List->n)
		{
			for(i=0;i<LastNumCli;i++)
			{
				if ((List->list[NumCli].f!=0)&&(List->list[NumCli].obj))
				{
					CurrentClient=i; // !
					RetC=((ServerClientSockEx*)(List->list[i].obj))->ReadSock(buf,cnt);
					if (RetC!=0) 
					{
						List->list[i].PackIn++;
						List->list[i].CountIn+=cnt;
						
						LastNumCli=i+1;
						break;
					}
				}
			}
		}
	}
	else
	{
		if ((List->list[NumCli].f!=0)&&(List->list[NumCli].obj))
		{
			if (NumCli>=List->n) return -1;
			RetC=((ServerClientSockEx*)(List->list[NumCli].obj))->ReadSock(buf,cnt);
			if(RetC)
			{
				List->list[NumCli].PackIn++;
				List->list[NumCli].CountIn+=cnt;
			}
		}
	}
//	if(RetC)
//		{size_t iasm=1;}
	return RetC;
};


// ---------------------------------------------------------------------
int	ServerClientFun(LPVOID);

// ---------------------------------------------------------------------

int ServerClientSock::Trace(char *mess,int prm,char *str)
{
	char message[1024];
	
	if( (prm==-1) && (str==NULL) )
	{
		sprintf(message,"this=0x%08p, handle=%d, %s",
			(LPVOID)this,Handle,mess);
	}
	else if ( (prm!=-1) && (str==NULL) )
	{
		sprintf(message,"this=0x%08p, handle=%d, %s %d",
			(LPVOID)this,Handle,mess,prm);
	}
	else if( (prm!=-1) && (str!=NULL) )
	{
		sprintf(message,"this=0x%08p, handle=%d, %s (%s:%d)",
			(LPVOID)this,Handle,mess,str,prm);
	}
	else if( (prm==-1) && (str!=NULL) )
	{
		sprintf(message,"this=0x%08p, handle=%d, %s (%s)",
			(LPVOID)this,Handle,mess,str);
	}

	//return WriteToLog(message);

#ifdef TraceEnable
	if(pService) 
		pService->OutLog(message,NULL,"lls");
#endif

	return 0;
}

int		DmpPrcServerClientSock(void* Obj,UCHAR* tBuf,size_t Cnt)
{
	int	rc;
	rc=((ServerClientSock*)Obj)->SendNotBuf((char*)tBuf,Cnt);
	return rc;
};

// ---------------------------------------------------------------------
ServerClientSock::ServerClientSock(SOCKET handle, ServerSock *ssock,Services *pSrv,Decoder* pDec)
{
int	dontblock, rc=0;
int	namelen=sizeof(client);
int	optlen=sizeof(int);
HOSTENT *he;
in_addr	ina;
	hCE=0;
	pService=pSrv;
	pDecoder=pDec;
	TimeConn.Time=0;TimeDisConn.Time=0;PriorLev=0;PriorState=0;GlobalCnt=0;Fl_GCntIn=0;
	Dmp.Clear();
	Dmp0.Clear();
	Dmp1.Clear();
	Dmp.SetPrm(pDecoder,(void*)this,&DmpPrcServerClientSock);
	Dmp0.SetPrm(pDecoder,(void*)this,&DmpPrcServerClientSock);
	Dmp1.SetPrm(pDecoder,(void*)this,&DmpPrcServerClientSock);
	TimeOutWRB=0;
	TimeOut=1000;
	HeaderBuf.Syn=Syn1;
	LenOBuf=10000;
	if (pDecoder)
		OutBuf=(char*)pDecoder->malloc(LenOBuf);
	else
		OutBuf=(char*)malloc(LenOBuf);
	Flag=1;
	SmBuf=0;
	tSm=0;
	MdTr=0;
	PackCnt=0;PackCntIn=0;
	Mode=0;
	servSock=ssock;
	Handle=handle;
	NumInList=-1;

/*	CB_Wr=CB_Rd=0;
	CB_Len=100000;
	if(pDecoder)
		CBuf=(UCHAR*)pDecoder->malloc(CB_Len);
	else
		CBuf=(UCHAR*)malloc(CB_Len);
*/
	SizeBufCl=MaxPack;
	if (pDecoder)
		BufCl=(UCHAR*)pDecoder->malloc(SizeBufCl);
	else
		BufCl=(UCHAR*)malloc(SizeBufCl);

	getpeername(handle,(sockaddr*)&client,&namelen);

	IPadr=*(ULONG*)&client.sin_addr;
	if (pService)
	{
		if (!pService->TestHost(IPadr))		// 
//			return -1;
			{
		size_t iasm=1;
	}
	}
	strcpy(IPhost,inet_ntoa(client.sin_addr));

	dontblock=inet_addr(IPhost);
	memcpy((struct in_addr*)&ina, &dontblock, 4);
	if (pService)
	{
		char*		tpCh;
		tpCh=pService->GetHostByAddr(IPhost);
		if (tpCh!=NULL)
		{
			strcpy(namehost,tpCh); // update HostName
			TestNetName(namehost);
		}
		else
		{
			strcpy(namehost,IPhost); // update HostName
		}
	}
	else
	{
		he=gethostbyaddr((const char*)&dontblock,4,AF_INET);
		if (he)
		{	
			strcpy(namehost, he->h_name);
			TestNetName(namehost);
		}

		else
			strcpy(namehost, IPhost);
	}

//		*namehost=0;
	
	readFunction=ssock->readFunction;

	rcvbufsz=SockBufSizeRecv;
	sndbufsz=SockBufSizeSend;

	rc=setsockopt(Handle,SOL_SOCKET,SO_RCVBUF,(char*)&rcvbufsz,sizeof(rcvbufsz));
	rc=setsockopt(Handle,SOL_SOCKET,SO_SNDBUF,(char*)&rcvbufsz,sizeof(sndbufsz));

	ClThread=new ClassThread("ServerClientSockFun",ServerClientFun,(LPVOID)this,THREAD_PRIORITY_HIGHEST);

	if (pService) 
		pService->GetRealTime(&TimeConn.FTime);
	else
		GetSystemTimeAsFileTime(&TimeConn.FTime);
	Trace("connect",-1,namehost);
}

ServerClientSock::~ServerClientSock()
{
	DisConnect("destr");

	int	rc=0;
	TimeOut=0;
	delete ClThread; ClThread=NULL;
	if (Handle!=-1)
	{
	   rc=closesocket(Handle);
	   Handle=-1;
//	   rc=rc*2-rc;
	}
	if (pDecoder)
	{
		if (BufCl) pDecoder->free(BufCl);
//		pDecoder->free(CBuf);
		if (OutBuf) pDecoder->free(OutBuf);
	}
	else
	{
		if (BufCl) free(BufCl);
//		free(CBuf);
		if (OutBuf) free(OutBuf);
	}
}

int ServerClientSock::DisConnect(char *mess,int rc)
{
	Flag=0;
//	Dmp.Clear();
	SmBuf=0;
	tSm=0;
//	Fl_Calc=0;
//	CB_Rd=CB_Wr=0;
	if(mess==NULL)
	{
		Trace("disconnect",-1,namehost);
	}
	else
	{
		char s[256];
		sprintf(s,"disconnect %s rc=%d",mess,rc);
		Trace(s,-1,namehost);
	}
	return 0;
}
int ServerClientSock::SendFromBuf()
{
	int	rc=0;
	if (Flag!=0)
	{
		FileTime	TTime;
		if (pService)
			pService->GetRealTime(&TTime.FTime);
		else
			GetSystemTimeAsFileTime(&TTime.FTime);
		if (TTime.Time-TimeConn.Time>KtimeS)
		{
			rc=Dmp.OData();
			if (rc>0)
			{
				PriorState=2;
			}
			if ((rc==0)&&(PriorState<2))
			{
				rc=Dmp1.OData();
				if (rc>0)
				{
					PriorState=1;
				}
			}
			if ((rc==0)&&(PriorState<1))
			{
				rc=Dmp0.OData();
			}
			if (rc==0)
				PriorState=0;
		}
	}
//	if (rc)
//		SendNotBuf((char*)tB,tCnt);
/*
	int	rc=0;
	int	tCB_Wr=CB_Wr;
	if (tCB_Wr!=CB_Rd)
	{
		
		if (tCB_Wr<CB_Rd)
		{
			SendNotBuf((char*)(CBuf+CB_Rd),CB_Len-CB_Rd);
			rc=CB_Len-CB_Rd;
			CB_Rd=0;
		}
		else
		{
			SendNotBuf((char*)(CBuf+CB_Rd),tCB_Wr-CB_Rd);
			rc=tCB_Wr-CB_Rd;
			CB_Rd=tCB_Wr;
		};
	};
*/	return rc;
};


int	ServerClientSock::SizeCB(int Size)
{
/*	for (size_t i=0;i<100;i++)
	{
		if (CB_Wr==CB_Rd) break;
		Sleep(10);
	}
	if (pDecoder)
	{
		CBuf=(UCHAR*)pDecoder->realloc(CBuf,Size);
	}
	else
	{
		CBuf=(UCHAR*)realloc(CBuf,Size);
	}
	CB_Len=Size;
	return CB_Len;
*/
	return Size;
};


int ServerClientSock::Idle()
{
	int	tLen,tCnt;
	int	rc=0;	//,rc2,i;
	int	cnt=0;
	long	rB=0;
	UCHAR*	tBufCl;
	int	trc,tsmt,tsm;

	if(Flag==0) {/*CB_Rd=CB_Wr=0; */return 0; }
	
	Srd.fd_array[0]=Handle; Srd.fd_count=1;
	Swr.fd_array[0]=Handle; Swr.fd_count=0;
	Sex.fd_array[0]=Handle; Sex.fd_count=0;
	struct timeval timeout;
	timeout.tv_sec=0; timeout.tv_usec=250;
	select(Handle,&Srd,&Swr,&Sex,&timeout);

	if ((SizeBufCl-SmBuf)==0)
	{
		SizeBufCl+=MaxPack;
		if (pDecoder)
			tBufCl=(UCHAR*)pDecoder->realloc(BufCl,SizeBufCl);
		else
			tBufCl=(UCHAR*)realloc(BufCl,SizeBufCl);
		if (tBufCl!=NULL)
		{
			BufCl=tBufCl;
		}
		else
		{	// Error Alloc 
			if(pService)
			{
				char	tB[128];
				sprintf(tB,"SCSock(Idle) P:%d Host:%s Error Realloc!!!",servSock->portTCP,namehost);
				pService->SetCriticalError(NULL,tB,"Error");
				pService->OutLog(tB,pDecoder,"CliSock");
			}
			DisConnect();
			return -1;
		};
		MdTr=1;
	};

	rc=recv(Handle,(char*)BufCl+SmBuf,SizeBufCl-SmBuf, 0);
	
	switch(rc)
	{
	  case -1:
				//rc=WSAGetLastErrorTrace1((int)this); 
				rc=WSAGetLastError();
				if (rc==WSAEWOULDBLOCK)
					return 0;
				DisConnect("recv() -1",rc); //Flag=0;
				break;
	  case 0:
				DisConnect("recv() 0"); //Flag=0;
				break;
	  default:
			PackCntIn+=rc;
//			if(pService)
//			{
//				pService->Telemetr(pDecoder,"SCliSockPackCntIn",(double)PackCntIn,this);
//			}
			trc=rc;tsm=SmBuf;tsmt=tSm;
			switch(Mode)
			{
			case 0:
				if (rc>0) if (readFunction)
				(*readFunction)(this,(UCHAR*)BufCl, rc);
				break;
			case 1:
				if (rc>0) if (readFunction)
				(*readFunction)(this,(UCHAR*)BufCl, rc);
				break;
			case 2:
				if (rc>0) if (readFunction)
				{
					if (SmBuf==0) tSm=0;
					tLen=rc+SmBuf-tSm;
					while(tLen!=0)
					{
						// Ðàçáîð áóôåðà äàííûõ
//						if (MdTr==0)
//						{
							if (tLen<sizeof(SockHeader)) 
							{	// Ìàëîâàòî äëÿ àíàëèçà
								SmBuf=tSm+tLen;
								break;
							};
							if (*(int*)(BufCl+tSm)==Syn1)
							{
								tCnt=*(int*)(BufCl+tSm+4);
								if ((tCnt+sizeof(SockHeader))<=tLen)
								{	// ïàêåò ïåðåäàí ïîëíîñòüþ ok!!!
									if (Fl_GCntIn==0)
									{
										GlobalCntIn=((SockHeader*)(BufCl+tSm))->CntPack+1;
										Fl_GCntIn=1;
									}
									else
									{
										if (GlobalCntIn==((SockHeader*)(BufCl+tSm))->CntPack)
										{
											GlobalCntIn++;
										}
										else
										{
//											pService->SetCriticalError(pDecoder,"PackCntWrong","Error","SrvSockCli");
											GlobalCntIn=((SockHeader*)(BufCl+tSm))->CntPack;
										};
									}
									(*readFunction)(this,(UCHAR*)BufCl+tSm+sizeof(SockHeader),tCnt);
									tSm+=(tCnt+sizeof(SockHeader));
									tLen-=(tCnt+sizeof(SockHeader));
									SmBuf=0;
								}
								else
								{	// ïàêåò ïåðåäàí íå ïîëíîñòüþ - çàðàçà !!!
									if ((tSm+tLen+sizeof(SockHeader))>SizeBufCl)
									{	// Íå âëåç â áóôåð !!!
										MdTr=100;
										SizeBufCl+=10*MaxPack;
										if (pDecoder)
											tBufCl=(UCHAR*)pDecoder->realloc(BufCl,SizeBufCl);
										else
											tBufCl=(UCHAR*)realloc(BufCl,SizeBufCl);
										if (tBufCl!=NULL)
										{
											BufCl=tBufCl;
										}
										else
										{	// Error Alloc 
											if(pService)
											{
												char	tB[128];
												sprintf(tB,"SCSock(Idle) P:%d Host:%s Error Realloc!!!",servSock->portTCP,namehost);
												pService->SetCriticalError(NULL,tB,"Error");
												pService->OutLog(tB,pDecoder,"CliSock");
											}
											DisConnect();
											return -1;
										};
										SmBuf=tSm+tLen;
										break;
									}
									else
									{	// Êîïèðóåì äàííûå è æäåì !!!
										if ((tSm+tLen+sizeof(SockHeader))>(SizeBufCl-MaxPack)) 
											MdTr=100;
										memcpy(BufCl,BufCl+tSm,tLen);
										tSm=0;
										SmBuf=tLen;
										break;
									};
								};
							}
							else
							{
//								Fl_Calc=0;
//								if(pService)
//									pService->OutLog("ServerClientSock(Idle) Syn1 Error!!!",pDecoder,"SrvCliSock");
								if(pService)
								{
									char	tB[128];
									sprintf(tB,"SCSock(Idle) P:%d Host:%s Syn1 Error!!!",servSock->portTCP,namehost);
									pService->SetCriticalError(NULL,tB,"Error");
									pService->OutLog(tB,pDecoder,"SrvCliSock");
								}
								else
								{
									Beep(1000,100);
									Beep(3000,100);
								}
								DisConnect();
								break;
							}
//						}
//						else
//						{
//							MdTr=0;
//						}
					}
					if (tLen==0)
					{
						if 	(SizeBufCl>MaxPack)
						{
							if (MdTr==0)
							{
								SizeBufCl-=MaxPack;
								if (pDecoder)
									tBufCl=(UCHAR*)pDecoder->realloc(BufCl,SizeBufCl);
								else
									tBufCl=(UCHAR*)realloc(BufCl,SizeBufCl);
								if (tBufCl!=NULL)
								{
									BufCl=tBufCl;
								}
								else
								{	// Error Alloc 
									if(pService)
									{
										char	tB[128];
										sprintf(tB,"SCSock(Idle) P:%d Host:%s Error Realloc!!!",servSock->portTCP,namehost);
										pService->SetCriticalError(NULL,tB,"Error");
										pService->OutLog(tB,pDecoder,"CliSock");
									}
									DisConnect();
									return -1;
								};
							}
							else MdTr--;
						}
					}

				}
				break;
			};
	}
	return rc;
};

int	ServerClientFun(LPVOID csock)
{
	if (((ServerClientSock*)csock)->Flag!=0)
	{
		return (((ServerClientSock *)csock)->Idle()|((ServerClientSock *)csock)->SendFromBuf());
	}	
	return 0;
};

int	ServerClientNulFun(LPVOID csock)
{
	if (((ServerClientSock *)csock)->Flag!=0)
	{
		return ((ServerClientSock *)csock)->SendFromBuf();
	}
	return 0;
};

int	ServerClientNulFunExR(LPVOID csock)
{
	if (((ServerClientSockEx *)csock)->Flag!=0)
	{
		// Read
		UCHAR	*tBuf;
		size_t		tCnt=0;
		((ServerClientSockEx *)csock)->ReadSockI(tBuf,tCnt);
		if (tCnt)
		{
			((ServerClientSockEx *)csock)->DmpR.Data(tBuf,tCnt,0);
		}
		return tCnt;
	}
	return 0;
};

int	ServerClientNulFunEx(LPVOID csock)
{
	if (((ServerClientSockEx *)csock)->Flag!=0)
	{
		// Send
		return ((ServerClientSockEx *)csock)->SendFromBuf();
	}
	return 0;
};

int	ServerClientSock::SendBuf(char *wbuf,int wlen,int Priority)
{
	int	rc=0;
	if (Flag==0)
	{
		Dmp.Clear();
		Dmp0.Clear();
		Dmp1.Clear();
		if (pService)
		{
			FileTime tTime;
			pService->GetRealTime(&tTime.FTime);
			if (tTime.Time-TimeDisConn.Time>KtimeS)
			{
				hCE=pService->SetCriticalError(pDecoder,"SCS_DataLost","Warning","SendBuf",hCE);
				pService->OutLog("SCS_DataLost",pDecoder,"DataLost");
			}
		}
		return 0;
	}
	if (wlen==0)
		return rc;
	switch(Mode)
	{
	case 0:
		SendNotBuf(wbuf,wlen);
		break;
	case 1:
		SendCircBuf(wbuf,wlen);
		break;
	case 2:
		if (Priority<=PriorLev)
			rc=Dmp0.Data((UCHAR*)wbuf,wlen);
		else
			rc=Dmp1.Data((UCHAR*)wbuf,wlen);
		if (rc==-1)
		{
			DisConnect();
			if(pService)
				pService->OutLog("ClientSock(SendCircBuf) TimeOut Disconnect!!!",pDecoder,"CliSock");

		}
		break;
	case 3:
		HeaderBuf.Len=wlen;
		SendCircBuf((char*)&HeaderBuf,sizeof(HeaderBuf));
		SendCircBuf(wbuf,wlen);
		break;
	default:
		if(pService)
			pService->SetCriticalError(pDecoder,"Mode>2","Warning","ServerClientSock");
	};
	return rc;
};

int	ServerClientSock::SendCircBuf(char* wbuf,int wlen)
{
	int	rc=0;
	rc=Dmp.Data((UCHAR*)wbuf,wlen);
	if (rc==-1)
	{
		DisConnect();
		if(pService)
			pService->OutLog("SrvClientSock(SendCircBuf) TimeOut Disconnect!!!",pDecoder,"ServerClientSock");

	}
/*	int	FreeSize;
	int	rc=0;
	int	tCB_Rd;
	char	ttB[256];
	double	tLoad;
	while (wlen!=0)
	{
		tCB_Rd=CB_Rd;
		if (CB_Wr<tCB_Rd)
		{
			FreeSize=tCB_Rd-CB_Wr-1;
			tLoad=100.*(1.-(double)FreeSize/(double)(CB_Len-1));
			if((pService)&&(tLoad>1.))
			{
				pService->Telemetr(pDecoder,"SrvCliSockCircBuf%",tLoad,this);
			}
			if (FreeSize>wlen)
			{
				TimeOutWRB=0;
				memcpy(CBuf+CB_Wr,wbuf,wlen);
				CB_Wr+=wlen;
				rc+=wlen;
				wlen=0;
			}
			else
			{
				if (FreeSize!=0)
				{
					TimeOutWRB=0;
					memcpy(CBuf+CB_Wr,wbuf,tCB_Rd-CB_Wr-1);
					wbuf+=tCB_Rd-CB_Wr-1;
					rc+=tCB_Rd-CB_Wr-1;
					wlen-=tCB_Rd-CB_Wr-1;
					CB_Wr+=tCB_Rd-CB_Wr-1;
					continue;
				}
				if(pService)
				{
					sprintf(ttB,"SrvClient Sock Buf OverFlow Host:%s Size=%d Real=%d!!!",namehost,CB_Len,FreeSize);
					pService->OutLog(ttB,pDecoder,"CliSock");
				}
			
				if (TimeOutWRB++>TimeOut)
				{
					if(pService)
						pService->OutLog("SrvClientSock(SendCircBuf) TimeOut Disconnect!!!",pDecoder,"ServerClientSock");
					DisConnect("SendCircBuf"); //Flag=0;
					wlen=0;TimeOutWRB=0;
				}
				else
				{
					Sleep(10);
					continue;
				}
//				FreeSize--;
//				if (FreeSize<=0) 
//					{Sleep(10);continue;};
//				memcpy(CBuf+CB_Wr,wbuf,FreeSize);
//				wbuf+=FreeSize;
//				CB_Wr+=FreeSize;
//				rc+=FreeSize;
//				wlen-=FreeSize;
			}
		}
		else
		{
			FreeSize=CB_Len-(CB_Wr-tCB_Rd)-1;
			tLoad=100.*(1.-(double)FreeSize/(double)(CB_Len-1));
			if((pService)&&(tLoad>1.))
			{
				pService->Telemetr(pDecoder,"SrvCliSockCircBuf%",tLoad,this);
			}

//			if (FreeSize==0)
//				{Sleep(10);continue;};
			if (FreeSize>wlen)
			{
				TimeOutWRB=0;
				if ((CB_Len-CB_Wr)>wlen)
				{
					memcpy(CBuf+CB_Wr,wbuf,wlen);
					CB_Wr+=wlen;
					rc+=wlen;
					wlen=0;
				}
				else
				{
					memcpy(CBuf+CB_Wr,wbuf,CB_Len-CB_Wr);
					wbuf+=(CB_Len-CB_Wr);
					rc+=(CB_Len-CB_Wr);
					wlen-=(CB_Len-CB_Wr);
					CB_Wr=0;
				}
			}
			else
			{
				if (FreeSize!=0)
				{
					if (FreeSize>CB_Len-CB_Wr)
					{
						TimeOutWRB=0;
						memcpy(CBuf+CB_Wr,wbuf,CB_Len-CB_Wr);
						wbuf+=(CB_Len-CB_Wr);
						rc+=(CB_Len-CB_Wr);
						wlen-=(CB_Len-CB_Wr);
						CB_Wr=0;continue;
					}
					else
					{
						TimeOutWRB=0;
						memcpy(CBuf+CB_Wr,wbuf,FreeSize);
						wbuf+=(FreeSize);
						rc+=(FreeSize);
						wlen-=(FreeSize);
						CB_Wr+=FreeSize;continue;
					}
				}
				sprintf(ttB,"SrvClient Sock Buf OverFlow Host:%s Size=%d Real=%d!!!",namehost,CB_Len,FreeSize);
				if(pService)
					pService->OutLog(ttB,pDecoder,"CliSock");
				if (TimeOutWRB++>TimeOut)
				{
					if(pService)
						pService->OutLog("SrvClientSock(SendCircBuf) TimeOut Disconnect!!!",pDecoder,"ServerClientSock");
					DisConnect("SendCircBuf"); //Flag=0;
					wlen=0;
				}
				else
				{
					Sleep(10);continue;
				}
//				if (tCB_Rd==0) {Sleep(10);continue;};
//				memcpy(CBuf+CB_Wr,wbuf,CB_Len-CB_Wr);
//				wbuf+=(CB_Len-CB_Wr);
//				rc+=(CB_Len-CB_Wr);
//				wlen-=(CB_Len-CB_Wr);
//				CB_Wr=0;
			}
		}
	};
*/	return rc;
};


int	ServerClientSock::SendNotBuf(char *Wbuf,int Wlen)
{
	int	rc=0;
	int	TimeO=0;
	int			tlen,wlen;
	char		ttBuf[128];
	char*		tbuf;
	char*		wbuf;
	switch(Mode)
	{
	case 2:
		{
			SockHeader* Header=(SockHeader*)Wbuf;
			Header->CntPack=GlobalCnt++;

/*			SockEl=(Sock_El*)IDL.NewElem();
			if (SockEl==NULL)
				SockEl=new Sock_El((UCHAR*)Wbuf,Wlen);
			else
				SockEl->Init((UCHAR*)Wbuf,Wlen);
			rc=IDL.Add(SockEl);
			if (rc==-1)
			{
				ID_Elem* tIDEl=IDL.Find((UCHAR*)&Header->Cnt,sizeof(int));
				IDL.Del(tIDEl);
				rc=IDL.Add(SockEl);
			}
*/
		}
		break;
	}
	if (Flag==0)
	{/*CB_Rd=CB_Wr=0;*/ 
		FileTime tTime;
		pService->GetRealTime(&tTime.FTime);
		if (tTime.Time-TimeDisConn.Time>KtimeS)
		{
			hCE=pService->SetCriticalError(pDecoder,"SCS_DataLost","Warning","SendNotBuf",hCE);
		}
		return 0;
	}
	tbuf=Wbuf;
	tlen=Wlen;
	do
	{
		if (tlen>MaxTrPack)
		{
			wlen=MaxTrPack;
			tlen-=MaxTrPack;
			wbuf=tbuf;
			tbuf+=MaxTrPack;
		}
		else
		{
			wlen=tlen;
			wbuf=tbuf;
			tlen=0;
		}
_p11:
		rc=send(Handle, wbuf, wlen, 0);
		if (wlen!=rc)
		{
			if (rc!=-1)
			{
				pService->SetCriticalError(pDecoder,"rc=Send=-1","Error","SrvSockCli");
			}
			rc=WSAGetLastError();
			if(
				(rc==WSAECONNRESET) ||
				(rc==WSAECONNABORTED)
				)
			{ 
				DisConnect("SendNotBuf RESET or ABORTED"); //Flag=0; 
			}
			if (rc==WSAEWOULDBLOCK)		//((rc==WSAEWOULDBLOCK)||(rc==WSAEFAULT))
			{
	//			if(ReceiveDisconnect(Handle,(int)this)==0)
				{
//					Sleep(1);
					if (TimeO++<TimeOut) goto _p11;
					Sleep(10);
					if (TimeO++<TimeOut+TimeOut) goto _p11;
					if(pService)
						pService->OutLog("ServerClientSock(SendNotBuf) TimeOut !",pDecoder,"ServerClientSock");

				};
			};
			if(pService)
				pService->OutLog("ServerClientSock(SendNotBuf) Disconnect!",pDecoder,"ServerClientSock");
			DisConnect(); //Flag=0;
			break;
		}
		else
		{
			PackCnt+=wlen;TimeO=0;
		}
	} while (tlen>0);
	if (rc!=-1) rc=Wlen;
	if(pService)
		if (!pService->TestHost(IPadr))
		{
			sprintf(ttBuf,"DisConectSocket because Host=%d Error",*((UCHAR*)(&IPadr)+3));
			pService->OutLog(ttBuf,pDecoder,"ServerClientSock");
			DisConnect();
		}
	return rc;
/*	int	rc;
	int	TimeO=0;

	while(1)
	{
		rc=send(Handle, wbuf, wlen, 0);
		if (wlen!=rc)
		{
			if (rc!=-1)	while (TRUE)
			{	// Âñòàâèòü òàéìàóò
				Beep(3000,200);
				Beep(1000,200);
				Beep(2000,200);
			}

			rc=WSAGetLastError();
			if(
				(rc==WSAECONNRESET) ||
				(rc==WSAECONNABORTED)
				)
			{ 
				rc=0; DisConnect("SendNotBuf RESET or ABORTED"); //Flag=0; 
			}
			if (rc==WSAEWOULDBLOCK)
			{
				// íåîõîäèìî óòî÷íåíèå íà ñëó÷àé disconnect-à îò óäàëåííîãî êëèåíòà
//				if(0)
//				if(ReceiveDisconnect(Handle,(int)this)) 
//				{
//					// òàêè disconnect
//					rc=0; DisConnect(); //Flag=0;
//				}
//				else
				{
					// ýòî íå disconnect, îòðàáàòûâàåì áëîêèðîâêó
					if(++TimeO<TimeOut/10)
					{
						Trace("SendNotBuf Tmeout",TimeO);
						Sleep(1);
						continue;
					}
					// ïðèíóäèòåëüíûé ðàçðûâ ñîåäèíåíèÿ
					else
						DisConnect("SendNotBuf Disconnect Timeout"); //Flag=0;
				}
			}
		}
		else
		{
			PackCnt+=rc;
//			if(pService)
//			{
//				pService->Telemetr(pDecoder,"SCliSockPackCnt",(double)PackCnt,this);
//			}
		}
		break;
	}
	return rc;
*/
};

int	ServerClientSock::SockExit()
{
	if (Handle!=-1)
		closesocket(Handle);
	Handle=-1;
	return 0;
}

ServerClientSockEx::ServerClientSockEx(SOCKET handle, ServerSock *ssock,Services *pSrv,Decoder *pDec)
{
int	dontblock, rc=0;
int	namelen=sizeof(client);
int	optlen=sizeof(int);
HOSTENT *he;
in_addr	ina;
	Fl_Lock=0;
	pService=pSrv;
	pDecoder=pDec;
	if (pService) pService->GetRealTime(&RdTime.FTime);
	Dmp.Clear();
	Dmp0.Clear();
	Dmp1.Clear();
	DmpR.Clear();
	Dmp.SetPrm(pDecoder,(void*)this,&DmpPrcServerClientSock);
	Dmp0.SetPrm(pDecoder,(void*)this,&DmpPrcServerClientSock);
	Dmp1.SetPrm(pDecoder,(void*)this,&DmpPrcServerClientSock);
	DmpR.SetPrm(pDecoder,(void*)this,&DmpPrcServerClientSock);
	Div=0;
	TimeOut=1000;
	Fl_Calc=0;
	HeaderBuf.Syn=Syn1;
	LenOBuf=10000;
	if (pDecoder)
		OutBuf=(char*)pDecoder->malloc(LenOBuf);
	else
		OutBuf=(char*)malloc(LenOBuf);
	Flag=1;
	SmBuf=0;
	tSm=0;
	MdTr=0;
	Mode=0;

	servSock=ssock;
	Handle=handle;
	NumInList=-1;

	SizeBufCl=MaxPack;
	if (pDecoder)
		BufCl=(UCHAR*)pDecoder->malloc(SizeBufCl);
	else
		BufCl=(UCHAR*)malloc(SizeBufCl);
	*(ULONG*)&client.sin_addr=1;
	getpeername(handle,(sockaddr*)&client,&namelen);

	IPadr=*(ULONG*)&client.sin_addr;
	strcpy(IPhost,inet_ntoa(client.sin_addr));

	if (pService)
	{
		char	*tpCh;
		tpCh=pService->GetHostByAddr(IPhost);
		if (tpCh!=NULL)
		{
			strcpy(namehost,tpCh);
			TestNetName(namehost);
		}
		else
		{
			strcpy(namehost, IPhost);
		}
	}
	else
	{
		dontblock=inet_addr(IPhost);
		memcpy((struct in_addr*)&ina, &dontblock, 4);
		he=gethostbyaddr((const char*)&dontblock,4,AF_INET);
		if (he)
		{
			strcpy(namehost, he->h_name);
			TestNetName(namehost);
		}
		
		else
			strcpy(namehost, IPhost);
	}
	
	readFunction=ssock->readFunction;

	rcvbufsz=SockBufSizeRecv;
	sndbufsz=SockBufSizeSend;

	rc=setsockopt(Handle,SOL_SOCKET,SO_RCVBUF,(char*)&rcvbufsz,sizeof(rcvbufsz));
	rc=setsockopt(Handle,SOL_SOCKET,SO_SNDBUF,(char*)&rcvbufsz,sizeof(sndbufsz));

	ClThread=new ClassThread("ServerClientNulFunEx",ServerClientNulFunEx,(LPVOID)this,THREAD_PRIORITY_HIGHEST);
	ClThread2=new ClassThread("ServerClientNulFunEx",ServerClientNulFunExR,(LPVOID)this,THREAD_PRIORITY_HIGHEST);
	
	Trace("connect",-1,namehost);
}

	ServerClientSockEx::~ServerClientSockEx()
{
	if (Fl_Lock)
		DmpR.UnLockOData();
	delete ClThread2;
};

int	ServerClientSockEx::ReadSock(UCHAR* &Buf, size_t &Cnt)
{
	UCHAR	*tBuf;
	size_t		tCnt=0;
	Cnt=0;
	if (Fl_Lock)
	{
		DmpR.UnLockOData();Fl_Lock=0;
	}
	if (DmpR.LockOData(tBuf,tCnt))
	{
		Buf=tBuf+sizeof(SockHeader);
		Cnt=tCnt-sizeof(SockHeader);
		Fl_Lock=1;
	}
	else
	{
		DmpR.UnLockOData();Fl_Lock=0;
	}
	return Cnt;
};

int	ServerClientSockEx::ReadSockI(UCHAR* &Buf, size_t &Cnt)
{
	Cnt=0;
	int		cnt=0,rc=0;
	long	rB=0;
	UCHAR*	tBufCl=NULL;
	FileTime	tTime;
	tTime.Time=0;
//	ULONG	sockcnt;
	if(Flag==0) {/*CB_Rd=CB_Wr=0; */return 0; }
	if (pService) pService->GetRealTime(&tTime.FTime);
	if (tTime.Time-RdTime.Time>2*KtimeS)
	{
		RdTime.Time=tTime.Time;
		return 0;
	}
	if (Fl_Calc==0)
	{
/*		if (Div++>100)
		{
			Div=0;
		}
		else
		{
			rc=ioctlsocket(Handle,FIONREAD,&sockcnt);
			if (rc==0)
			{
				if (sockcnt==0) 
					return 0;
			};
			Div=0;
		};
*/
		Srd.fd_array[0]=Handle; Srd.fd_count=1;
		Swr.fd_array[0]=Handle; Swr.fd_count=0;
		Sex.fd_array[0]=Handle; Sex.fd_count=0;
		struct timeval timeout;
//		timeout.tv_sec=0; timeout.tv_usec=25;
		timeout.tv_sec=0; timeout.tv_usec=0;
		select(Handle,&Srd,&Swr,&Sex,&timeout);

		if ((SizeBufCl-SmBuf)==0)
		{
			SizeBufCl+=MaxPack;
			if (pDecoder)
				tBufCl=(UCHAR*)pDecoder->realloc(BufCl,SizeBufCl);
			else
				tBufCl=(UCHAR*)realloc(BufCl,SizeBufCl);
			if (tBufCl!=NULL)
			{
				BufCl=tBufCl;
			}
			else
			{	// Error Alloc 
				if(pService)
				{
					char	tB[128];
					sprintf(tB,"SCSE(Idle) P:%d Host:%s Error Realloc!!!",servSock->portTCP,namehost);
					pService->SetCriticalError(NULL,tB,"Error");
					pService->OutLog(tB,pDecoder,"CliSock");
				}
				DisConnect();
				return -1;
			};
			MdTr=1;
		}
		rc=recv(Handle, (char*)BufCl+SmBuf, SizeBufCl-SmBuf, 0);
		
		switch(rc)
		{
		  case -1:
					//rc=WSAGetLastErrorTrace1((int)this);
					rc=WSAGetLastError();
					if (rc==WSAEWOULDBLOCK)
					{
						if (pService) pService->GetRealTime(&RdTime.FTime);
						return 0;
					}
					DisConnect("recv() -1",rc); //Flag=0;
					break;
		  case 0:
					tLen=0;
					Fl_Calc=0;
					DisConnect("recv() 0"); //Flag=0;
					break;
		  default:
// ------------
			if (rc>0)
			{
				PackCntIn+=rc;
//				if(pService)
//				{	char	tBuf[256];
//					pService->Telemetr(pDecoder,"SCliSockPackCntIn",(double)PackCntIn,this);
////					sprintf(tBuf,"SockEx recv cnt=%I64d",PackCntIn);
////					pService->OutLog(tBuf,pDecoder,"TestSock");
//				}
				Fl_Calc=1;
				if (SmBuf==0) tSm=0;
				tLen=rc+SmBuf-tSm;
//				tLen=rc+SmBuf;
//				if (SmBuf==0) tSm=0;
			}
			else
			{
				if (pService) pService->GetRealTime(&RdTime.FTime);
			};	
			break;
		}
	};
	switch(Mode)
	{
	case 0:
		Buf=(UCHAR*)BufCl;
		Cnt=rc;
		Fl_Calc=0;SmBuf=0;
		break;
	case 1:
		Buf=(UCHAR*)BufCl;
		Cnt=rc;
		Fl_Calc=0;SmBuf=0;
		break;
	case 2:
		if (Fl_Calc==1)
		{
			if(tLen!=0)
			{
				// Ðàçáîð áóôåðà äàííûõ
//				if (MdTr==0)
//				{
					if (tLen<sizeof(SockHeader)) 
					{	// Ìàëîâàòî äëÿ àíàëèçà
						SmBuf=tSm+tLen;
						Fl_Calc=0;
					}
					else
					if (((SockHeader*)(BufCl+tSm))->Syn==Syn1)
					{
						tCnt=((SockHeader*)(BufCl+tSm))->Len;
						if ((tCnt+sizeof(SockHeader))<=tLen)
						{	// ïàêåò ïåðåäàí ïîëíîñòüþ
							if (Fl_GCntIn==0)
							{
								GlobalCntIn=((SockHeader*)(BufCl+tSm))->CntPack+1;
								Fl_GCntIn=1;
							}
							else
							{
								if (GlobalCntIn==((SockHeader*)(BufCl+tSm))->CntPack)
								{
									GlobalCntIn++;
								}
								else
								{
//									pService->SetCriticalError(pDecoder,"PackCntWrong","Error","SrvSockCliEx");
									GlobalCntIn=((SockHeader*)(BufCl+tSm))->CntPack;
								};
							}
//							(*readFunction)(this,(UCHAR*)bufcl+tSm+sizeof(SockHeader),tCnt);
							Buf=(UCHAR*)BufCl+tSm+sizeof(SockHeader);
							Cnt=tCnt;
							tSm+=(tCnt+sizeof(SockHeader));
							tLen-=(tCnt+sizeof(SockHeader));
							SmBuf=0;
						}
						else
						{	// ïàêåò ïåðåäàí íå ïîëíîñòüþ - çàðàçà !!!
							if ((tSm+tLen+sizeof(SockHeader))>SizeBufCl)
							{	// Íå âëåç â áóôåð !!!
								SizeBufCl+=10*MaxPack;
								if (pDecoder)
									tBufCl=(UCHAR*)pDecoder->realloc(BufCl,SizeBufCl);
								else
									tBufCl=(UCHAR*)realloc(BufCl,SizeBufCl);
								if (tBufCl!=NULL)
								{
									BufCl=tBufCl;
								}
								else
								{	// Error Alloc 
									if(pService)
									{
										char	tB[128];
										sprintf(tB,"SCSE(Idle) P:%d Host:%s Error Realloc!!!",servSock->portTCP,namehost);
										pService->SetCriticalError(NULL,tB,"Error");
										pService->OutLog(tB,pDecoder,"CliSock");
									}
									DisConnect();
									return -1;
								};
								SmBuf=tSm+tLen;
								MdTr=100;
							}
							else
							{	// Êîïèðóåì äàííûå è æäåì !!!
								if ((tSm+tLen+sizeof(SockHeader))>(SizeBufCl-MaxPack)) 
									MdTr=100;
								memcpy(BufCl,BufCl+tSm,tLen);
								tSm=0;
								SmBuf=tLen;
								Fl_Calc=0;
							};
						};
					}
					else
					{
						Fl_Calc=0;
						if(pService)
						{
							char	tB[128];
							sprintf(tB,"SCSEx(RSock) P:%d Host:%s Syn1 Error!!!",servSock->portTCP,namehost);
							pService->SetCriticalError(NULL,tB,"Error");
							pService->OutLog(tB,pDecoder,"SrvCliSock");
						}
						else
						{
							Beep(1000,100);
							Beep(3000,100);
						}
						DisConnect();
						break;
					}
//				}
//				else
//				{
//					MdTr=0;
//				}
			}
			else 
			{
				if 	(SizeBufCl>MaxPack)
				{
					if (MdTr==0)
					{
						SizeBufCl-=MaxPack;
						if (pDecoder)
							tBufCl=(UCHAR*)pDecoder->realloc(BufCl,SizeBufCl);
						else
							tBufCl=(UCHAR*)realloc(BufCl,SizeBufCl);
						if (tBufCl!=NULL)
						{
							BufCl=tBufCl;
						}
						else
						{	// Error Alloc 
							if(pService)
							{
								char	tB[128];
								sprintf(tB,"SCSE(Idle) P:%d Host:%s Error Realloc!!!",servSock->portTCP,namehost);
								pService->SetCriticalError(NULL,tB,"Error");
								pService->OutLog(tB,pDecoder,"CliSock");
							}
							DisConnect();
							return -1;
						};
					}
					else
						MdTr--;
				};
				Fl_Calc=0;
			}
		}
		break;
	}
// ------------
	return	Cnt;
};


NullMt::NullMt()
{
	// Этот фрагмент должен быть вставлен во все последующие объекты
	// NullMt_ должен быть заменен на имя объекта
	Name_Mt=NullMt_NameMt;Rem_Mt=NullMt_RemMt;Ver_=NullMt_Version;Auth_=NullMt_Author;
	PinName_I=NullMt_PinNameI;PinName_O=NullMt_PinNameO;Prm_=NullMt_Prm;Vol_=NullMt_Vol;
	Type_Mt=NullMt_Type;

	Num_PinI=0;while (PinName_I[2*Num_PinI][0]!=0x0) Num_PinI++;
	Num_PinO=0;while (PinName_O[2*Num_PinO][0]!=0x0) Num_PinO++;
	Num_Prm=0;while (Prm_[2*Num_Prm][0]!=0x0)Num_Prm ++;
	// -------------------------------------------------------------
	Fl_Stop=1;Fl_Data=0;
};
NullMt::~NullMt()
{
};

void	NullMt::Param(char* Prm,char* Vol)
{
};

void	NullMt::Start()
{
	Fl_Stop=0;
};

void	NullMt::Stop()
{
	Fl_Stop=1;
};

int	NullMt::DataFF(int N_Pin,UCHAR* pmas,int Cnt)
{
	int	OCnt=0;
	//	if (Fl_Stop!=1)	{};
	return OCnt;
};



	Invertor::Invertor()
{
// Ýòîò ôðàãìåíò äîëæåí áûòü âñòàâëåí âî âñå ïîñëåäóþùèå îáúåêòû
// Invertor_ äîëæåí áèòü çàìåíåí íà èìÿ îáúåêòà
		 Name_Mt=Invertor_NameMt;Rem_Mt=Invertor_RemMt;Ver_=Invertor_Version;Auth_=Invertor_Author;
		 PinName_I=Invertor_PinNameI;PinName_O=Invertor_PinNameO;Prm_=Invertor_Prm;Vol_=Invertor_Vol;

		 Num_PinI=0;while (PinName_I[2*Num_PinI][0]!=0x0) Num_PinI++;
		 Num_PinO=0;while (PinName_O[2*Num_PinO][0]!=0x0) Num_PinO++;
		 Num_Prm=0;while (Prm_[2*Num_Prm][0]!=0x0)Num_Prm ++;
// -------------------------------------------------------------
		 Flag=0;
		 ElUDP=NULL;
		 CntTimer=0;
};
	Invertor::~Invertor()
{
};

void	Invertor::Param(char* Prm,char* Vol)
{
};

void	Invertor::Start()
{
	CntTimer=0;
	ElUDP=Srv->OpenUDPCilent(NT_LAN,888,this);
};

void	Invertor::Stop()
{
	if (ElUDP)
		Srv->CloseUDPCilent(NT_LAN,888,this);
};

int Invertor::IdleFunc(int NumberTr)			// Idle function  if rc!=0 recall function (?)  NumberTr=-1 System NumberTr+0x8000 - DinTr
{
	if (ElUDP)
	{
		if (Srv->Timer(CntTimer,1000))
		{
//			ElUDP->Send((UCHAR*)&CntTimer,sizeof(INT64));
//			ElUDP->Recive();
		}
	}

	int	rc=ChildIdleFunc(NumberTr);
	return rc;
}

int	Invertor::Data(int N_Pin,UCHAR* pmas,size_t Cnt)
{
	int	OCnt=0;
	tBuf=pmas;
	tCnt=Cnt;
	Flag=1;
	Sm=0;
	OCnt=Cnt;
//	Sleep(1000);
return OCnt;
};

int	Invertor::OutData(int N_Pin,UCHAR* &pmas,size_t &Cnt)
{
	int	i;
	Cnt=0;
	if (Flag==1)
	{
		if (tCnt>SizeInvBuf)
		{
			for (i=0;i<SizeInvBuf;i++)
				BufOut[i]=~*tBuf++;
			Sm+=SizeInvBuf;tCnt-=SizeInvBuf;
			pmas=BufOut;Cnt=SizeInvBuf;
		}
		else
		{
			for (i=0;i<tCnt;i++)
				BufOut[i]=~*tBuf++;
			pmas=BufOut;Cnt=tCnt;
			Flag=0;
		}
	}
return (int)Cnt;
};



/*
unsigned short calc_crc16t[]={
   0x0000, 0x1021, 0x2042, 0x3063, 0x4084, 0x50a5, 0x60c6, 0x70e7,
   0x8108, 0x9129, 0xa14a, 0xb16b, 0xc18c, 0xd1ad, 0xe1ce, 0xf1ef,
   0x1231, 0x0210, 0x3273, 0x2252, 0x52b5, 0x4294, 0x72f7, 0x62d6,
   0x9339, 0x8318, 0xb37b, 0xa35a, 0xd3bd, 0xc39c, 0xf3ff, 0xe3de,
   0x2462, 0x3443, 0x0420, 0x1401, 0x64e6, 0x74c7, 0x44a4, 0x5485,
   0xa56a, 0xb54b, 0x8528, 0x9509, 0xe5ee, 0xf5cf, 0xc5ac, 0xd58d,
   0x3653, 0x2672, 0x1611, 0x0630, 0x76d7, 0x66f6, 0x5695, 0x46b4,
   0xb75b, 0xa77a, 0x9719, 0x8738, 0xf7df, 0xe7fe, 0xd79d, 0xc7bc,
   0x48c4, 0x58e5, 0x6886, 0x78a7, 0x0840, 0x1861, 0x2802, 0x3823,
   0xc9cc, 0xd9ed, 0xe98e, 0xf9af, 0x8948, 0x9969, 0xa90a, 0xb92b,
   0x5af5, 0x4ad4, 0x7ab7, 0x6a96, 0x1a71, 0x0a50, 0x3a33, 0x2a12,
   0xdbfd, 0xcbdc, 0xfbbf, 0xeb9e, 0x9b79, 0x8b58, 0xbb3b, 0xab1a,
   0x6ca6, 0x7c87, 0x4ce4, 0x5cc5, 0x2c22, 0x3c03, 0x0c60, 0x1c41,
   0xedae, 0xfd8f, 0xcdec, 0xddcd, 0xad2a, 0xbd0b, 0x8d68, 0x9d49,
   0x7e97, 0x6eb6, 0x5ed5, 0x4ef4, 0x3e13, 0x2e32, 0x1e51, 0x0e70,
   0xff9f, 0xefbe, 0xdfdd, 0xcffc, 0xbf1b, 0xaf3a, 0x9f59, 0x8f78,
   0x9188, 0x81a9, 0xb1ca, 0xa1eb, 0xd10c, 0xc12d, 0xf14e, 0xe16f,
   0x1080, 0x00a1, 0x30c2, 0x20e3, 0x5004, 0x4025, 0x7046, 0x6067,
   0x83b9, 0x9398, 0xa3fb, 0xb3da, 0xc33d, 0xd31c, 0xe37f, 0xf35e,
   0x02b1, 0x1290, 0x22f3, 0x32d2, 0x4235, 0x5214, 0x6277, 0x7256,
   0xb5ea, 0xa5cb, 0x95a8, 0x8589, 0xf56e, 0xe54f, 0xd52c, 0xc50d,
   0x34e2, 0x24c3, 0x14a0, 0x0481, 0x7466, 0x6447, 0x5424, 0x4405,
   0xa7db, 0xb7fa, 0x8799, 0x97b8, 0xe75f, 0xf77e, 0xc71d, 0xd73c,
   0x26d3, 0x36f2, 0x0691, 0x16b0, 0x6657, 0x7676, 0x4615, 0x5634,
   0xd94c, 0xc96d, 0xf90e, 0xe92f, 0x99c8, 0x89e9, 0xb98a, 0xa9ab,
   0x5844, 0x4865, 0x7806, 0x6827, 0x18c0, 0x08e1, 0x3882, 0x28a3,
   0xcb7d, 0xdb5c, 0xeb3f, 0xfb1e, 0x8bf9, 0x9bd8, 0xabbb, 0xbb9a,
   0x4a75, 0x5a54, 0x6a37, 0x7a16, 0x0af1, 0x1ad0, 0x2ab3, 0x3a92,
   0xfd2e, 0xed0f, 0xdd6c, 0xcd4d, 0xbdaa, 0xad8b, 0x9de8, 0x8dc9,
   0x7c26, 0x6c07, 0x5c64, 0x4c45, 0x3ca2, 0x2c83, 0x1ce0, 0x0cc1,
   0xef1f, 0xff3e, 0xcf5d, 0xdf7c, 0xaf9b, 0xbfba, 0x8fd9, 0x9ff8,
   0x6e17, 0x7e36, 0x4e55, 0x5e74, 0x2e93, 0x3eb2, 0x0ed1, 0x1ef0
};//*/

unsigned short calc_crc16_4t[]={
0x0000, 0x1021, 0x2042, 0x3063, 0x4084, 0x50A5, 0x60C6, 0x70E7,
0x8108, 0x9129, 0xA14A, 0xB16B, 0xC18C, 0xD1AD, 0xE1CE, 0xF1EF};

unsigned short calc_crc16t[]={
	  0x0000,0x1189,0x2312,0x329b,
	  0x4624,0x57ad,0x6536,0x74bf,
	  0x8c48,0x9dc1,0xaf5a,0xbed3,
	  0xca6c,0xdbe5,0xe97e,0xf8f7,
	  0x1081,0x0108,0x3393,0x221a,
	  0x56a5,0x472c,0x75b7,0x643e,
	  0x9cc9,0x8d40,0xbfdb,0xae52,
	  0xdaed,0xcb64,0xf9ff,0xe876,
	  0x2102,0x308b,0x0210,0x1399,
	  0x6726,0x76af,0x4434,0x55bd,
	  0xad4a,0xbcc3,0x8e58,0x9fd1,
	  0xeb6e,0xfae7,0xc87c,0xd9f5,
	  0x3183,0x200a,0x1291,0x0318,
	  0x77a7,0x662e,0x54b5,0x453c,
	  0xbdcb,0xac42,0x9ed9,0x8f50,
	  0xfbef,0xea66,0xd8fd,0xc974,
	  0x4204,0x538d,0x6116,0x709f,
	  0x0420,0x15a9,0x2732,0x36bb,
	  0xce4c,0xdfc5,0xed5e,0xfcd7,
	  0x8868,0x99e1,0xab7a,0xbaf3,
	  0x5285,0x430c,0x7197,0x601e,
	  0x14a1,0x0528,0x37b3,0x263a,
	  0xdecd,0xcf44,0xfddf,0xec56,
	  0x98e9,0x8960,0xbbfb,0xaa72,
	  0x6306,0x728f,0x4014,0x519d,
	  0x2522,0x34ab,0x0630,0x17b9,
	  0xef4e,0xfec7,0xcc5c,0xddd5,
	  0xa96a,0xb8e3,0x8a78,0x9bf1,
	  0x7387,0x620e,0x5095,0x411c,
	  0x35a3,0x242a,0x16b1,0x0738,
	  0xffcf,0xee46,0xdcdd,0xcd54,
	  0xb9eb,0xa862,0x9af9,0x8b70,
	  0x8408,0x9581,0xa71a,0xb693,
	  0xc22c,0xd3a5,0xe13e,0xf0b7,
	  0x0840,0x19c9,0x2b52,0x3adb,
	  0x4e64,0x5fed,0x6d76,0x7cff,
	  0x9489,0x8500,0xb79b,0xa612,
	  0xd2ad,0xc324,0xf1bf,0xe036,
	  0x18c1,0x0948,0x3bd3,0x2a5a,
	  0x5ee5,0x4f6c,0x7df7,0x6c7e,
	  0xa50a,0xb483,0x8618,0x9791,
	  0xe32e,0xf2a7,0xc03c,0xd1b5,
	  0x2942,0x38cb,0x0a50,0x1bd9,
	  0x6f66,0x7eef,0x4c74,0x5dfd,
	  0xb58b,0xa402,0x9699,0x8710,
	  0xf3af,0xe226,0xd0bd,0xc134,
	  0x39c3,0x284a,0x1ad1,0x0b58,
	  0x7fe7,0x6e6e,0x5cf5,0x4d7c,
	  0xc60c,0xd785,0xe51e,0xf497,
	  0x8028,0x91a1,0xa33a,0xb2b3,
	  0x4a44,0x5bcd,0x6956,0x78df,
	  0x0c60,0x1de9,0x2f72,0x3efb,
	  0xd68d,0xc704,0xf59f,0xe416,
	  0x90a9,0x8120,0xb3bb,0xa232,
	  0x5ac5,0x4b4c,0x79d7,0x685e,
	  0x1ce1,0x0d68,0x3ff3,0x2e7a,
	  0xe70e,0xf687,0xc41c,0xd595,
	  0xa12a,0xb0a3,0x8238,0x93b1,
	  0x6b46,0x7acf,0x4854,0x59dd,
	  0x2d62,0x3ceb,0x0e70,0x1ff9,
	  0xf78f,0xe606,0xd49d,0xc514,
	  0xb1ab,0xa022,0x92b9,0x8330,
	  0x7bc7,0x6a4e,0x58d5,0x495c,
	  0x3de3,0x2c6a,0x1ef1,0x0f78};//*/

unsigned short calc_crc16(unsigned char *Buf, unsigned int len)
{
register unsigned short i,CRC,bx;
register unsigned char *buf;

	buf=Buf;
	CRC = 0xffff;
	for(i=0;i<len;i++)
	{
		bx = buf[i] & 0xff;
		bx ^= CRC&0xff;
		CRC = ((CRC>>8) + (bx&0xff00)) ^ calc_crc16t[bx];
	}
	return (CRC ^=0xffff ^ 0xf47);
}

unsigned short calc_crc16_4(unsigned char *Buf, unsigned int len2)
{
register unsigned short i,CRC;
register unsigned char *buf;

	buf=Buf;
	CRC = 0xffff;
	for(i=0;i<len2;i++)
	{
		CRC=calc_crc16_4t[(CRC^((buf[i>1]>>((i&1)*4))&0xf))&0xF]^(CRC>>4);
//		bx = (buf[i>1]>>((i&1)*4)) & 0xf;
//		bx ^= CRC&0xf;
//		CRC = ((CRC>>4) + (bx&0xfff0)) ^ calc_crc16_4t[bx];
	}
	return (CRC ^=0xffff ^ 0xf47);
}

//#define INCL_DOSDATETIME
//#include <os2.h>

//#include <dir.h>
//#include <alloc.h>
//#include <io.H>
//#include <STDLIB.H> 
//#include <fcntl.h>
//#include <sys/types.h>
//#include <sys/stat.h>
//#include	<file.h>

char* CalcNameData(char* tbuf,int num)
{
SYSTEMTIME	TTime;
//char		tch;
GetSystemTime (&TTime);
//if (num>=10)
// tch=num+0x37;
//if (num<10)	sprintf(tbuf,"%x%02d%02d%02d%i",TDate.da_mon,TDate.da_day,TTime.ti_hour,TTime.ti_min,num);
//else 		sprintf(tbuf,"%x%02d%02d%02d%c",TDate.da_mon,TDate.da_day,TTime.ti_hour,TTime.ti_min,tch);
sprintf(tbuf,"%02d%02d%02d%02d%02x",TTime.wDay,TTime.wHour,TTime.wMinute, TTime.wSecond,num);
return (tbuf);
};

char* CalcNameDataLocal(char* tbuf,int num)
{
SYSTEMTIME	TTime;
//char		tch;
GetLocalTime (&TTime);
//if (num>=10)
// tch=num+0x37;
//if (num<10)	sprintf(tbuf,"%x%02d%02d%02d%i",TDate.da_mon,TDate.da_day,TTime.ti_hour,TTime.ti_min,num);
//else 		sprintf(tbuf,"%x%02d%02d%02d%c",TDate.da_mon,TDate.da_day,TTime.ti_hour,TTime.ti_min,tch);
sprintf(tbuf,"%02d%02d%02d%02x",TTime.wDay,TTime.wHour,TTime.wMinute,num);
return (tbuf);
};

int GenNameFile(char* cat,char* ext)
{
char	t_buf[80]="";
char	tn[9];
int	num=0;
//int	numext=0;
int	hand=0;
//int	i;
while (hand!=-1)
{
strcpy(t_buf,cat);
//i=0;
CalcPath(t_buf);
//while (t_buf[i]!=0x0) i++;
//if ((t_buf[--i]!='\\')&&(i!=0)) strcat(t_buf,"\\");
strcat(t_buf,CalcNameData(tn,num));
strcat(t_buf,ext);
hand=_open(t_buf,O_CREAT|O_BINARY|O_RDWR);
//
close(hand);
num++;
}
return _creat(t_buf,0);
};

int GenNameLocal(char* cat,char* ext)
{
char	t_buf[128]="";
char	tn[9];
int	num;
int	hand;
//int	numext=0;

hand=-1;
num=0;

while (hand==-1)
{
	int	err=0;
strcpy(t_buf,cat);
CalcPath(t_buf);
strcat(t_buf,CalcNameDataLocal(tn,num));
strcat(t_buf,ext);
_fmode=O_BINARY;
//hand=_creat(t_buf,0);
hand=_open(t_buf,O_CREAT|O_EXCL|O_BINARY|O_RDWR);
if (hand!=-1)
 SetFileAttributesA(t_buf,FILE_ATTRIBUTE_HIDDEN);
else
{
//	{size_t iasm=1;}

#if _MSC_VER > 1000
	err=_doserrno; 
//	err=*_errno(); 
#endif // _MSC_VER > 1000
	if ((err==17)||(err==80))
	{
	}
	else
		return hand;
//*/
}
num++;
}
strcpy(cat,t_buf);
return hand;
;
};

int GenName(char* cat,char* ext)
{
	char	t_buf[128]="";
	char	tn[9];
	int	num;
	int	hand;
	//int	numext=0;
	
	hand=-1;
	num=0;
	
	while (hand==-1)
	{
		int	err;
		strcpy(t_buf,cat);
		CalcPath(t_buf);
		strcat(t_buf,CalcNameData(tn,num));
		strcat(t_buf,ext);
		_fmode=O_BINARY;
		//hand=_creat(t_buf,0);
		hand=_open(t_buf,O_CREAT|O_EXCL|O_BINARY|O_RDWR);
		if (hand!=-1)
			SetFileAttributesA(t_buf,FILE_ATTRIBUTE_HIDDEN);
		else
		{
			
#if _MSC_VER > 1000
			err=_doserrno; 
			//	err=*_errno(); 
#endif // _MSC_VER > 1000
			if ((err==17)||(err==80))
			{
			}
			else
				return hand;
			
			//*/
		}
		num++;
	}
	strcpy(cat,t_buf);
	return hand;
	;
};

HANDLE GenName_H(char* cat,char* ext)
{
	char	t_buf[128]="";
	char	tn[9];
	int	num;
	HANDLE	hFile;
	
	hFile=INVALID_HANDLE_VALUE;
	num=0;
	
	while(hFile==INVALID_HANDLE_VALUE)
	{
		int	err;
		strcpy(t_buf,cat);
		CalcPath(t_buf);
		strcat(t_buf,CalcNameData(tn,num));
		strcat(t_buf,ext);
		_fmode=O_BINARY;
		//hand=_creat(t_buf,0);
		hFile=CreateFileA(t_buf,GENERIC_READ|GENERIC_WRITE,FILE_SHARE_READ,NULL,CREATE_ALWAYS,FILE_ATTRIBUTE_NORMAL,NULL);
//		hand=_open(t_buf,O_CREAT|O_EXCL|O_BINARY|O_RDWR);
//		if (hFile!=-1)
//			SetFileAttributes(t_buf,FILE_ATTRIBUTE_HIDDEN);
		if(hFile==INVALID_HANDLE_VALUE)
		{
			#if _MSC_VER > 1000
			err=_doserrno; 
			//	err=*_errno(); 
			#endif // _MSC_VER > 1000
			if ((err==17)||(err==80))
			{
			}
			else
				return hFile;
			
		}
		num++;
	}
	strcpy(cat,t_buf);

	return hFile;
};

void TestNetName(char* Name)
{
	size_t i=0;
	while (Name[i]!=0)
	{
		if (Name[i]=='.')
		{
//			Name[i]=0;
			break;
		}
		i++;
	}
};

int TestPath(char* path)
{
	int		i=0;
	while (path[i]!=0x0)
	{
		if (path[i]==0x20) strcpy(&path[i],&path[i+1]);
		if (
			(path[i]==0x5c)||
			((path[i]>=0x2d)&&(path[i]<=0x3a))||
			((path[i]>=0x41)&&(path[i]<=0x5a))||
			((path[i]>=0x61)&&(path[i]<=0x7a))
//			||((i>1)&&(path[i]==':'))
			)
			i++;
		else
		{
			path[i]=0x0;
			return 0;
		};
	};
return 0;
};

int CalcNetPath(char* path)		//
{
	int		i=0;
	int		j=0;
//	CalcPath(path);
	while (path[i]!=0x0)
	{
		if (path[j]==':') j++;
		path[i++]=path[j++];
	}
	return	0;
};

int CalcPath(char* path)
{
int		i=0;
int		j;
while (path[i]!=0x0)
{
	if (path[i]=='/') path[i]='\\';
	if ((i>1)&&(path[i]==':')) path[i]='\\';
	while ((path[i]=='\\')&&(path[i+1]=='\\'))
	{
		j=i+1;
		while (path[j]!=0x0)
		{
			path[j-1]=path[j];
			j++;
		}
		path[j-1]=0x0;
	};
	i++;
};
 if ((i!=0)&&(path[i-1]!='\\')) strcat(path,"\\");
return 0;
};

int MakeDir(char* path)				//
{
	size_t i=0,j=0;

	while(path[i]!=0x0) 
		i++;
	if((i!=0) && (path[i-1]=='\\')) 
		path[i-1]=0;
	if(CreateDirectoryA(path,0))
	{ 
		if(path[i-1]==0) 
			path[i-1]='\\'; 
		return 0;
	}
	while(path[j]!=0x0)
	{
		j++;
		if(path[j]=='\\')
		{
			path[j]=0;
			CreateDirectoryA(path,0);
			path[j]='\\';
		}
	}
	if(CreateDirectoryA(path,0))
	{ 
		if(path[i-1]==0) path[i-1]='\\'; 
		return 0;
	}
	if(path[i-1]==0) 
		path[i-1]='\\';

	return 1;
};

// Îáðàáîòêà ôàéëîâ ñ áóôåðèçàöèåé
//---------------------------------
StrimFile::StrimFile()
{
};

int	StrimFile::Lseek(int Sm,int Fl)
{
if ((Sm==0)&&(Fl==SEEK_CUR)) {return PoiBuf+Poi;};
//if((Sm==PoiBuf)&&(Fl==SEEK_SET))
// {
// Poi=0;
// }
//else
// {
 PoiR=lseek(FH,Sm,Fl);
 Poi=0;PoiBuf=PoiR;
 LenBuf=read(FH,Buf1,SizeOfBuf);
 PoiR+=LenBuf;
// };
return PoiBuf;
};

int	StrimFile::Open(char* Name)
{
Poi=0;PoiR=0;PoiBuf=0;
FH=_open(Name,O_CREAT|O_BINARY|O_RDWR);
LenBuf=read(FH,Buf1,SizeOfBuf);
PoiR+=LenBuf;
return FH;
};

int	StrimFile::Creat(char* Name)
{
Poi=0;PoiR=0;PoiBuf=0;
FH=_creat(Name,0);
LenBuf=0;
return FH;
};

int	StrimFile::Read(UCHAR* Buf,int Len)
{
int LocPoi=0,OldLen=Len;
if ((LenBuf-Poi)>=Len)
 {
 memcpy(Buf,Buf1+Poi,Len);
 Poi+=Len;Len=0;if (Poi==LenBuf) ReadBuf();
 }
else
{
	memcpy(Buf+LocPoi,Buf1+Poi,LenBuf-Poi);LocPoi+=LenBuf-Poi;
	while (((Len-=LenBuf-Poi)>0)&(LenBuf!=0))
	{
		Poi=LenBuf;
		ReadBuf();
		if ((LenBuf-Poi)>=Len)
		{
			memcpy(Buf+LocPoi,Buf1+Poi,Len);
			Poi+=Len;if (Poi==LenBuf) ReadBuf();Len=0;
		}
		else
		{
			memcpy(Buf+LocPoi,Buf1+Poi,LenBuf-Poi);LocPoi+=LenBuf-Poi;
		}
	};
}
if (Len<0) Len=0;
return OldLen-Len;
};

int	StrimFile::Write(UCHAR* Buf,int Len)
{
if (Len<SizeOfBuf-Poi)
 {
 memcpy(Buf1+Poi,Buf,Len);Poi+=Len;
 }
else
 {
// Íåîáõîäèìà êîíñòðóêöèÿ while()
 int Temp=SizeOfBuf-Poi;
 memcpy(Buf1+Poi,Buf,Temp);Len-=Temp;
 LenBuf=SizeOfBuf;
 WriteBuf();
 memcpy(Buf1,Buf+Temp,Len);Poi=Len;
 }
return 0;
};

int	StrimFile::WriteChar(char Ch)
{
Buf1[Poi]=Ch;
LenBuf=++Poi;
if (Poi>=SizeOfBuf)
 {
 WriteBuf();
 };
return SizeOfBuf-LenBuf;
};

int	StrimFile::WriteShort(short Vol)
{
*(short*)(&Buf1[Poi])=Vol;
Poi+=2;LenBuf=Poi;
if (Poi>=SizeOfBuf) WriteBuf();
return 0;
};

int	StrimFile::WriteBuf()
{
if (PoiR==PoiBuf)
 {
 write(FH,Buf1,LenBuf);
 PoiR=PoiBuf+LenBuf;
 PoiBuf=PoiR;
 LenBuf=0;
 }
else
 {
 lseek(FH,PoiBuf,SEEK_SET);
 write(FH,Buf1,LenBuf);
 PoiR=PoiBuf+LenBuf;
 PoiBuf=PoiR;
 LenBuf=0;
 }
Poi=0;
return 0;
};

int	StrimFile::ReadBuf()
{
if (PoiR==(PoiBuf+Poi))
 {
 PoiBuf=PoiR;
 LenBuf=read(FH,Buf1,SizeOfBuf);
 PoiR+=LenBuf;
 }
else
 {
 PoiBuf+=Poi;lseek(FH,PoiBuf,SEEK_SET);
 PoiR=PoiBuf;
 LenBuf=read(FH,Buf1,SizeOfBuf);
 PoiR+=LenBuf;
 }
Poi=0;
return LenBuf;
};

int	StrimFile::ReadChar(char &Ch)
{
Ch=Buf1[Poi];
if (++Poi>=LenBuf)
 {
 Poi=LenBuf;
 ReadBuf();
 };
return LenBuf-Poi;
};


int	StrimFile::Close()
{
close(FH);
return 0;
};


void	DeleteDecod(LPVOID pDec)
{
	if (strcmp(((Decoder*)pDec)->Type_Mt,"Unit")==0)
		GetServices()->CloseUnit((Decoder*)pDec);
	delete (Decoder*)pDec;
};

CMainCfg::CMainCfg(Task* Ts)
{
	Tsk=Ts;
	MData.fErrView=0;MData.fTabCfg=0;MData.fThIDV=0;
	LoadCfg();
	if (MData.Fl_Start==1)
		MData.Fl_AbnStart=1;
	else
		MData.Fl_AbnStart=0;
	MData.Fl_Start=1;
	SaveCfg();
}

CMainCfg::~CMainCfg()
{
	MData.tVersion=mVersion;
	MData.Fl_Start=0;
	MData.Fl_AbnStart=0;
	SaveCfg();
}

void	CMainCfg::MakeDirectory()
{
//	int RC;
	MakeDir(MData.PathDLL[MData.Active]);
//	if (RC!=0)
//		strcpy(MData.PathDLL,"");
	MakeDir(MData.PathCFG[MData.Active]);
//	if (RC!=0)
//		strcpy(MData.PathCFG,"");
	MakeDir(MData.PathLOG);

//	if (MData.PathOut[0])
//		MakeDir(MData.PathOut);

	MakeDir(MData.PathBD);
};

void	CMainCfg::LoadCfg()
{
int	FH,RLen,CRC;
	FH=open(NameCfgFile,O_BINARY);
	if (FH!=-1)
	{
		memset (&MData,0,sizeof(MainData));
		RLen=read(FH,&MData,sizeof(MainData));
		close(FH);
		if (RLen==0/*<sizeof(MainData)*/)
		{
			getcwd(MData.PathDLL[MData.Active],255);
			strcpy(MData.PathCFG[MData.Active],MData.PathDLL[MData.Active]);
			strcpy(MData.PathLOG,MData.PathDLL[MData.Active]);
			strcpy(MData.PathBD,MData.PathDLL[MData.Active]);
//			strcpy(MData.PathTask,MData.PathDLL[MData.Active]);
			strcat(MData.PathDLL[MData.Active],"\\DLL");
			strcat(MData.PathCFG[MData.Active],"\\CFG");
			strcat(MData.Description[MData.Active],"Default");
			strcat(MData.PathLOG,"\\LOG");
			strcat(MData.PathBD,"\\BD");
//			strcat(MData.PathTask,"\\Task");
//			MData.SizeSB=8*1024*1024;
			MData.Number=1;
			MData.Repl=1;
			MData.NumDayLog=3;
			MData.NumDayCE=3;
			MData.TimeLocGlob=0;
			MData.fErrView=0;
			MData.fTabCfg=0;
			MData.fThIDV=0;
			MData.Fl_RunSI=0;
			MData.Fl_PAN=0;
			MData.Fl_LAN=1;
			MData.Fl_WAN=0;
			MData.Fl_RunSICli=1;
			MData.IP_MaskPAN=MData.IP_MaskLAN=MData.IP_MaskWAN=0xffffff;
			MData.IP_PAN=0x32a8c0;
			MData.IP_LAN=0x64a8c0;		// 192.168.100.0
			MData.IP_WAN=0x7f;
			MData.Fl_Cripto=1;
			strcpy(MData.PathOut,"\\FileOut");
//			MData.Fl_WorkGroup=0;
//			strcpy(MData.WorkGroup,"Default");
			MData.Fl_NetCompress=0;
			MData.Fl_SendUpD=1;
			MData.Fl_RecUpD=1;
			MData.CRC16MCfg=calc_crc16((UCHAR*)&MData+2,sizeof(MainData)-2);
			MData.Lock=MData.CRC16SrvCfg^MData.CRC16FCli^MData.CRC16ShCli^MData.CRC16NetCli;
		}
		else
		{
			if (RLen==sizeof(MainData))
			{
				CRC=calc_crc16((UCHAR*)&MData+2,sizeof(MainData)-2);
				if (MData.CRC16MCfg!=CRC)
				{
					MData.Lock=0;
				};
			}
			else
			{
//				MData.Fl_RunSI=0;
//				MData.Fl_PAN=0;
//				MData.Fl_LAN=1;
//				MData.Fl_WAN=0;
//				MData.IP_MaskPAN=MData.IP_MaskLAN=MData.IP_MaskWAN=0xffffff;
//				MData.IP_PAN=0x32a8c0;
//				MData.IP_LAN=0x2da8c0;
//				MData.IP_WAN=0x7f;
//				MData.Fl_Cripto=1;
//				MData.Fl_RunSICli=1;
//				MData.Fl_SendUpD=1;
//				MData.Fl_RecUpD=1;
//				MData.Repl=1;
				MData.CRC16MCfg=calc_crc16((UCHAR*)&MData+2,sizeof(MainData)-2);
			}
		}
	}
	else
	{
		memset (&MData,0,sizeof(MainData));
		getcwd(MData.PathDLL[MData.Active],255);
		strcpy(MData.PathCFG[MData.Active],MData.PathDLL[MData.Active]);
		strcpy(MData.PathLOG,MData.PathDLL[MData.Active]);
		strcpy(MData.PathBD,MData.PathDLL[MData.Active]);
		strcat(MData.PathDLL[MData.Active],"\\DLL");
		strcat(MData.PathCFG[MData.Active],"\\CFG");
		strcat(MData.Description[MData.Active],"Default");
		strcat(MData.PathLOG,"\\LOG");
		strcat(MData.PathBD,"\\BD");
//		MData.SizeSB=8*1024*1024;
		MData.Number=1;
		MData.Repl=1;
		MData.TimeInt=60000;
		MData.NumDayLog=3;
		MData.NumDayCE=10;
		MData.TimeLocGlob=0;
		MData.fErrView=0;
		MData.fTabCfg=0;
		MData.fThIDV=0;
		MData.Fl_RunSI=0;
		MData.Fl_PAN=0;
		MData.Fl_LAN=1;
		MData.Fl_WAN=0;
		MData.IP_MaskPAN=MData.IP_MaskLAN=MData.IP_MaskWAN=0xffffff;
		MData.IP_PAN=0x32a8c0;
		MData.IP_LAN = 0x64a8c0;		// 192.168.100.0
//		MData.IP_LAN=0x2Da8c0;
		MData.IP_WAN=0x7f;
		MData.Fl_Cripto=1;
		MData.Fl_RunSICli=1;
		strcpy(MData.PathOut,"\\FileOut");
//		MData.Fl_WorkGroup=0;
//		strcpy(MData.WorkGroup,"Default");
		MData.Fl_NetCompress=0;
		MData.Fl_SendUpD=1;
		MData.Fl_RecUpD=1;
		MData.CRC16MCfg=calc_crc16((UCHAR*)&MData+2,sizeof(MainData)-2);
		MData.Lock=MData.CRC16SrvCfg^MData.CRC16FCli^MData.CRC16ShCli^MData.CRC16NetCli;
	};
	getcwd(MData.Path,255);
	MData.PathCFG[MData.Active][255]=0x0;
	MData.PathDLL[MData.Active][255]=0x0;
	MData.PathOut[255]=0x0;
	MData.PathBD[255]=0x0;
	MakeDirectory();
	MData.NumError=0;
	MData.CritError=0;
//	if (MData.SizeSB<1024*1024)
//		MData.SizeSB=1024*1024;
	if (MData.NumDayCE<=0) MData.NumDayCE=30;
	if (MData.PathBD[0]==0)
	{
		getcwd(MData.PathBD,255);
		strcat(MData.PathBD,"\\BD");
	}
//	CL=Service->GetInfo();
//	CL->GetNestedMember();


	// !!! 08.07.2008
//	MData.fErrView=0;
	// !!!
};

void	CMainCfg::SaveCfg()
{
int	FH,CRC;
char	tBuf[256];
sprintf(tBuf,"%s\\%s",MData.Path,NameCfgFile);

SetFileAttributesA(tBuf,GetFileAttributesA(tBuf)&~FILE_ATTRIBUTE_READONLY);
FH=open(tBuf,O_CREAT|O_BINARY|O_RDWR);
//FH=creat(NameCfgFile,O_BINARY);
  if (FH!=-1)
  {
	USHORT	tLock=MData.CRC16SrvCfg^MData.CRC16FCli^MData.CRC16ShCli^MData.CRC16NetCli;
	MData.CRC16SrvCfg=calc_crc16((UCHAR*)&Tsk,sizeof(Tsk));
	if (MData.Lock==tLock)
		MData.Lock=MData.CRC16SrvCfg^MData.CRC16FCli^MData.CRC16ShCli^MData.CRC16NetCli;
	CRC=calc_crc16((UCHAR*)&MData+2,sizeof(MainData)-2);
	MData.CRC16MCfg=CRC;
	write(FH,&MData,sizeof(MainData));
	close(FH);
  }
};

//---------------------------------

void _CopyParameter(IDInfo* pIDInfo0,IDInfo* pIDInfo1,Decoder* pDec,El_Tab* _pElTb)
{
	void	*Prm,*Prm2;
	size_t		Len,Len2;
	Prm2 = NULL;
	char	BufProfile[512];
	if(pIDInfo1->pPList->GetParam(IDSource,Prm,Len)!=-1)
	{
		pIDInfo0->pPList->SetParam(IDSource,Prm,Len,pDec);
// 		if(_pElTb)
// 		{
// 			if((_pElTb->GetIDI()->pPList->GetParam(IDNameElTab,Prm2,Len2)!=-1)&&(*(UCHAR*)Prm2))
// 			{
// 				sprintf(BufProfile,"%s\\%s",(char*)Prm,(char*)Prm2);
// 				pIDInfo0->pPList->SetParam(IDSource,BufProfile,strlen(BufProfile),pDec);
// 
// 			}
// 			else
// 			{
//				pIDInfo0->pPList->SetParam(IDSource,Prm,Len,pDec);
//			}
//		}
	}
	else
	{
		if(_pElTb)
		{
			El_Tab* tmp_pElTb=_pElTb;
			char	tmpBufProfile[512];tmpBufProfile[0]=0;
			char	tmpBufProfile2[512];tmpBufProfile2[0]=0;
			while(tmp_pElTb)
			{
				tmp_pElTb->GetIDI()->pPList->GetParam(IDNameElTab,Prm2,Len2);
				if((tmp_pElTb->GetIDI()->pPList->GetParam(IDProtocol,Prm,Len)!=-1)&&(strcmp((char*)Prm,N_FindSatellite)==0))
				{
					break;
				}
				strcpy(tmpBufProfile2,tmpBufProfile);
				strcpy(tmpBufProfile,(char*)Prm2);
				tmp_pElTb=tmp_pElTb->pParent;
			}
			if(tmpBufProfile[0]&&tmpBufProfile2[0]&&((char*)Prm2)[0])
			{
				sprintf(BufProfile,"%s\\%s\\%s",(char*)Prm2,tmpBufProfile,tmpBufProfile2);
				pIDInfo0->pPList->SetParam(IDSource,BufProfile,strlen(BufProfile)+1,pDec);
			}
			else
				pIDInfo0->pPList->SetParam(IDSource,"None",strlen("None")+1,pDec);
		}
	}
	int pParam;
	for(pParam=0;CopyParameter[pParam]!=0;pParam++)
	{
		if(pIDInfo1->pPList->GetParam(CopyParameter[pParam],Prm,Len)!=-1)
		{
			pIDInfo0->pPList->SetParam(CopyParameter[pParam],Prm,Len,pDec);
		}
	}
	for(pParam=0;CopyParameter2[pParam]!=0;pParam++)
	{
		if(pIDInfo1->pPList->GetParam(CopyParameter2[pParam],Prm,Len)!=-1)
		{
			pIDInfo0->pPList->SetParam(CopyParameter2[pParam],Prm,Len,pDec);
		}
		else
			pIDInfo0->pPList->DelParam(CopyParameter2[pParam]);
	}
	if(_pElTb)
	{
		for(pParam=0;CopyParameter[pParam]!=0;pParam++)
		{			
			if(_pElTb->GetIDI()->pPList->GetParam(CopyParameter[pParam],Prm,Len)!=-1)
			{
				pIDInfo0->pPList->SetParam(CopyParameter[pParam],Prm,Len,pDec);
			}
		}
		for(pParam=0;CopyParameter2[pParam]!=0;pParam++)
		{			
			if(_pElTb->GetIDI()->pPList->GetParam(CopyParameter2[pParam],Prm,Len)!=-1)
			{
				pIDInfo0->pPList->SetParam(CopyParameter2[pParam],Prm,Len,pDec);
			}
		}
	}
	if(pIDInfo1->pPList->GetParam(IDIDEntryPointNext,Prm,Len)!=-1)
	{
		pIDInfo0->pPList->SetParam(IDIDEntryPoint,Prm,Len,pDec);
	}
	else
	{
		if(pIDInfo1->pPList->GetParam(IDIDEntryPoint,Prm,Len)!=-1)
		{
			pIDInfo0->pPList->SetParam(IDIDEntryPoint,Prm,Len,pDec);
		}
	}
}

int		mStrCmpHost(char* Host1,char* Host2)
{
	int Ret=-1;
	char *p1=Host1,*p2=Host2;
	//Var1	
	size_t Len1=strlen(p1),Len2=strlen(p2);
	size_t Max=(Len1>Len2)?Len2:Len1;
	if (strnicmp(p1,p2,Max)==0)
	{
		p1+=Max;p2+=Max;
		if ((((*p1=='.') && (*p2==0)) || ((*p1==0) && (*p2=='.'))) || ((*p1==0) && (*p2==0)))
			Ret=0;
	}
	//-Var2
	// 	if ((*p1==*p2) && (*p1))
	// 	{
	// 		while (*p1==*p2)
	// 		{
	// 			p1++;p2++;
	// 			if (!*p1)
	// 				break;
	// 		}
	// 		if ((((*p1=='.') && (*p2==0)) || ((*p1==0) && (*p2=='.'))) || ((*p1==0) && (*p2==0)))
	// 			Ret=0;
	// 	}
	return Ret;
};

int MTFun(LPVOID	Prm)
{
	return	((MultThrdMt*)Prm)->Idle();
};


	MultThrdMt::MultThrdMt()
{
// Ýòîò ôðàãìåíò äîëæåí áûòü âñòàâëåí âî âñå ïîñëåäóþùèå îáúåêòû
// MultThrdMt_ äîëæåí áûòü çàìåíåí íà èìÿ îáúåêòà
		Name_Mt=MultThrdMt_NameMt;Rem_Mt=MultThrdMt_RemMt;Ver_=MultThrdMt_Version;Auth_=MultThrdMt_Author;
		PinName_I=MultThrdMt_PinNameI;PinName_O=MultThrdMt_PinNameO;Prm_=MultThrdMt_Prm;Vol_=MultThrdMt_Vol;
		Type_Mt=MultThrdMt_Type;

		Num_PinI=0;while (PinName_I[2*Num_PinI][0]!=0x0) Num_PinI++;
		Num_PinO=0;while (PinName_O[2*Num_PinO][0]!=0x0) Num_PinO++;
		Num_Prm=0;while (Prm_[2*Num_Prm][0]!=0x0)Num_Prm ++;
// -------------------------------------------------------------
		Fl_Stop=1;Fl_Data=0;
		TimerTest=0;
		CntMemClIn=CntMemClOut=0;
		strcpy(NameMtBuf,"FFT");PMt=NULL;
		CThread=new	ClassThread("MultThrdMt",MTFun,(LPVOID)this,THREAD_PRIORITY_TIME_CRITICAL);	//THREAD_PRIORITY_TIME_CRITICAL		//THREAD_PRIORITY_HIGHEST
		sprintf(TelemPrm,"Loads,%s,%%:",NameMtBuf);
};
	MultThrdMt::~MultThrdMt()
{
	delete CThread; CThread=NULL;
	if (PMt)
	{
		PMt->StopEx();
		Srv->DeleteDecoder(PMt);PMt=NULL;
	}
};

void	MultThrdMt::Param(char* Prm,char* Vol)
{
	if (strcmp(Prm,"NameMt")==0)	
	{
		strcpy(NameMtBuf,Vol);
		{
			if (PMt)
			{
				PMt->StopEx();
				Srv->DeleteDecoder(PMt);PMt=NULL;
			}
			PMt=Srv->NewDecod(NameMtBuf,this);
			if (PMt)
			{
				PMt->Ext_Mt=0;
				PMt->Srv=Srv;
				PMt->AddDecLink(this,0,1000);
				PMt->AddDecLink(this,1,1001);
				PMt->AddDecLink(this,2,1002);
				PMt->AddDecLink(this,3,1003);
				PMt->AddDecLink(this,4,1004);
				PMt->AddDecLink(this,5,1005);
				PMt->AddDecLink(this,6,1006);
				PMt->AddDecLink(this,7,1007);
				PMt->AddDecLink(this,8,1008);
				PMt->AddDecLink(this,9,1009);
			}
			sprintf(TelemPrm,"Loads,%s,%%:",NameMtBuf);
		}
	}
	else
	{
		if (PMt)
		{
			PMt->ParamEx(Prm,Vol);
		}
	}
};

void	MultThrdMt::Start()
{
	DPSIn.SetPrm(this);
	DPSOut.SetPrm(this);
	DPSIn.Clear();
	DPSOut.Clear();
	if (PMt==NULL)
	{
		PMt=GetServices()->NewDecod(NameMtBuf,this);
		if (PMt)
		{
			PMt->Ext_Mt=0;
			PMt->Srv=Srv;
			PMt->AddDecLink(this,0,1000);
			PMt->AddDecLink(this,1,1001);
			PMt->AddDecLink(this,2,1002);
			PMt->AddDecLink(this,3,1003);
			PMt->AddDecLink(this,4,1004);
			PMt->AddDecLink(this,5,1005);
			PMt->AddDecLink(this,6,1006);
			PMt->AddDecLink(this,7,1007);
			PMt->AddDecLink(this,8,1008);
			PMt->AddDecLink(this,9,1009);
		}
	}
	if ( PMt) PMt->StartEx();
	Fl_Stop=0;
	TimerTest=0;
};

void	MultThrdMt::Stop()
{
	Fl_Stop=1;
	if ( PMt) PMt->StopEx();
//	Sleep(10);
	DPSIn.Clear();
	DPSOut.Clear();
};

int	MultThrdMt::DataFF(int N_Pin,UCHAR* pmas,size_t Cnt)
{
	int		OCnt=0;
	uchar*	tBuf;
	size_t	tCnt;
	if (Fl_Stop!=1)
	{
		if (N_Pin<1000)
		{
			DPSIn.Data(pmas,Cnt,N_Pin);
			while (DPSOut.LockOData(tBuf,tCnt))
			{
				if (!IsBadReadPtr(tBuf,tCnt))
					ODataFF(((SockHeader*)tBuf)->CntPack,tBuf+sizeof(SockHeader),tCnt-sizeof(SockHeader));
				else
				{
		size_t iasm=1;
	}

				DPSOut.UnLockOData();
			};	
//			DPSOut.UnLockOData();		//
		}
		else
		{
//			Fl_Data=1;
			DPSOut.Data(pmas,Cnt,N_Pin-1000);
		}
		Srv->Telemetr(this,TelemPrm,CThread->Loads,this);
	};
	return OCnt;
};

int		MultThrdMt::Idle(void)
{
	int rc=0;
	uchar*	tBuf;
	size_t		tCnt;
	if ((Fl_Stop==0)&&(PMt))
	{
		while (DPSIn.LockOData(tBuf,tCnt))
		{
	//		Fl_Data=0;
			if (!IsBadReadPtr(tBuf,tCnt))
				PMt->DataFFEx(((SockHeader*)tBuf)->CntPack,tBuf+sizeof(SockHeader),tCnt-sizeof(SockHeader));
			else
			{
			size_t iasm=1;
			}
			rc+=1;
			DPSIn.UnLockOData();
			if (rc>100)
			{
				rc=0; break;
			}
		};	
	}
	if (Srv->Timer(TimerTest,1000))
	{
		if (CntMemClIn<DPSIn.NumCM)
		{
			FileTime	OperTime;
			Srv->GetRealTime(&OperTime.FTime);
			if (OperTime.Time-TimeOutIn.Time>KtimeS*20)
			{// Error TimeOut
				Srv->GetRealTime(&TimeOutIn.FTime);
				DPSIn.Clear();
			}
		}
		else
		{
			Srv->GetRealTime(&TimeOutIn.FTime);
		}
		CntMemClIn=DPSIn.NumCM;
		if (CntMemClOut<DPSOut.NumCM)
		{
			FileTime	OperTime;
			Srv->GetRealTime(&OperTime.FTime);
			if (OperTime.Time-TimeOutOut.Time>KtimeS*20)
			{// Error TimeOut
				Srv->GetRealTime(&TimeOutOut.FTime);
				DPSOut.Clear();
			}
		}
		else
		{
			Srv->GetRealTime(&TimeOutOut.FTime);
		}
		CntMemClOut=DPSOut.NumCM;
	};
//	DPSIn.UnLockOData();
	return	rc;
};





bool sfQBE_Test(IDInfo* pInpIDI,uchar *pBuffQBE,int CntBuffQBE,Services* pSrv)
{
	bool Ret=FALSE;
	uchar *pSendBuff=pBuffQBE;

	try
	{pInpIDI->pPList->TestValid();}
	catch (...)
	{return Ret;}
	if ((CntBuffQBE%sizeof(StrSimpleQBE))==4) {pSendBuff+=4;CntBuffQBE-=4;}
	if ((CntBuffQBE%sizeof(StrSimpleQBE))==0)
	{
		size_t ii=CntBuffQBE/sizeof(StrSimpleQBE);
		if (ii) 
		{
			Parametr* pNewParam;StrSimpleQBE * pStrSimpleQBE;//int SizeVol=0;
// 			union	{int iVol;INT64 i64Vol;float fVol;double dVol;short shVol;char cVol;char* sVol;};
// 			union	{int iVol2;INT64 i64Vol2;float fVol2;double dVol2;short shVol2;char cVol2;char* sVol2;};
			while (ii)
			{				
					
				pStrSimpleQBE=(StrSimpleQBE *)pSendBuff;
				pNewParam=GetTablParametrID(pStrSimpleQBE->iIDParametr);
				if (!pNewParam)
				{
					Ret=FALSE;
					break;
				}
				void* VolIDI;
				switch(pNewParam->UID)
				{
				case IDTimeBegin:VolIDI=&pInpIDI->TimeBegin.Time;break;
				case IDTimeLastWr:VolIDI=&pInpIDI->TimeLastWr.Time;break;
				case IDTimeClose:VolIDI=&pInpIDI->TimeClose.Time;break;
				case IDTimeRdAcces:VolIDI=&pInpIDI->TimeOper.Time;break;

				case IDCommand:VolIDI=&pInpIDI->Command;break;				
				case IDID:VolIDI=&pInpIDI->ID_File;break;
				case IDIDFileParent:
					VolIDI=&pInpIDI->ID_ParentFile;
					if ((pInpIDI->ID_ParentFile==0) && (pInpIDI->ParentFile))
					{
						try{VolIDI=&pInpIDI->ParentFile->ID_File;}
						catch (...)	{VolIDI=&pInpIDI->ID_ParentFile;}		
					}
					break;
				case IDIDFileSnsFr:
					VolIDI=&pInpIDI->ID_SeansFrom;
					if ((pInpIDI->ID_SeansFrom==0) && (pInpIDI->SeansFrom))
					{
						try{VolIDI=&pInpIDI->SeansFrom->ID_File;}
						catch (...)	{VolIDI=&pInpIDI->ID_SeansFrom;}		
					}
					break;
				case IDIDFileSnsTo:
					VolIDI=&pInpIDI->ID_SeansTo;
					if ((pInpIDI->ID_SeansTo==0) && (pInpIDI->SeansTo))
					{
						try{VolIDI=&pInpIDI->SeansTo->ID_File;}
						catch (...)	{VolIDI=&pInpIDI->ID_SeansTo;}		
					}
					break;
				//IDStandart ????
				case IDNumPack:VolIDI=&pInpIDI->NumPack;break;
				case IDMtName: VolIDI=&pInpIDI->ParentMtName;	break;
//				case IDMtName:if (pInpIDI->ParentMt)VolIDI=&pInpIDI->ParentMt->Name_Mt;	break;
				case IDPriority:VolIDI=&pInpIDI->Priority;break;
				case IDStateBlock:VolIDI=&pInpIDI->StateBlock;break;
				case IDStateProc:VolIDI=&pInpIDI->StateProc;break;
				case IDTypeIDI:VolIDI=&pInpIDI->TypeIDI;break;
				default:
					if (pInpIDI->pPList->GetParam(pNewParam->UID,VolIDI)==-1)
					{Ret=FALSE;VolIDI=0;}
					break;
				}
				if (VolIDI)
				{
					switch(pStrSimpleQBE->TypeArifmetic)
					{
					case Equil:
						switch(pNewParam->Type)
						{
						case PrmType_INT:if (*(int*)pStrSimpleQBE->ValueParam==*(int*)VolIDI) Ret=TRUE;else Ret=FALSE;break;
						case PrmType_INT64:if (*(INT64*)pStrSimpleQBE->ValueParam==*(INT64*)VolIDI) Ret=TRUE;else Ret=FALSE;break;
						case PrmType_STRING:if (strcmp((char*)pStrSimpleQBE->ValueParam,(char*)VolIDI)==0) Ret=TRUE;else Ret=FALSE;break;
						case PrmType_FLOAT:if (*(float*)pStrSimpleQBE->ValueParam==*(float*)VolIDI) Ret=TRUE;else Ret=FALSE;break;
						case PrmType_DOUBLE:if (*(double*)pStrSimpleQBE->ValueParam==*(double*)VolIDI) Ret=TRUE;else Ret=FALSE;break;
						case PrmType_CHAR:if (*(char*)pStrSimpleQBE->ValueParam==*(char*)VolIDI) Ret=TRUE;else Ret=FALSE;break;
						case PrmType_SHORT:if (*(short*)pStrSimpleQBE->ValueParam==*(short*)VolIDI) Ret=TRUE;else Ret=FALSE;break;						
						default:Ret=FALSE;//?????
							break;
						};						
						break;
					case TypeGetAny:Ret=TRUE;break;
					case GreaterThan:
						switch(pNewParam->Type)
						{
						case PrmType_INT:if (*(int*)pStrSimpleQBE->ValueParam<*(int*)VolIDI) Ret=TRUE;else Ret=FALSE;break;
						case PrmType_INT64:if (*(INT64*)pStrSimpleQBE->ValueParam<*(INT64*)VolIDI) Ret=TRUE;else Ret=FALSE;break;
						case PrmType_STRING:if (strcmp((char*)pStrSimpleQBE->ValueParam,(char*)VolIDI)>0) Ret=TRUE;else Ret=FALSE;break;
						case PrmType_FLOAT:if (*(float*)pStrSimpleQBE->ValueParam<*(float*)VolIDI) Ret=TRUE;else Ret=FALSE;break;
						case PrmType_DOUBLE:if (*(double*)pStrSimpleQBE->ValueParam<*(double*)VolIDI) Ret=TRUE;else Ret=FALSE;break;
						case PrmType_CHAR:if (*(char*)pStrSimpleQBE->ValueParam<*(char*)VolIDI) Ret=TRUE;else Ret=FALSE;break;
						case PrmType_SHORT:if (*(short*)pStrSimpleQBE->ValueParam<*(short*)VolIDI) Ret=TRUE;else Ret=FALSE;break;						
						default:Ret=FALSE;//?????
							break;
						};
						break;
					case LessThan:
						switch(pNewParam->Type)
						{
						case PrmType_INT:if (*(int*)pStrSimpleQBE->ValueParam>*(int*)VolIDI) Ret=TRUE;else Ret=FALSE;break;
						case PrmType_INT64:if (*(INT64*)pStrSimpleQBE->ValueParam>*(INT64*)VolIDI) Ret=TRUE;else Ret=FALSE;break;
						case PrmType_STRING:if (strcmp((char*)pStrSimpleQBE->ValueParam,(char*)VolIDI)<0) Ret=TRUE;else Ret=FALSE;break;
						case PrmType_FLOAT:if (*(float*)pStrSimpleQBE->ValueParam>*(float*)VolIDI) Ret=TRUE;else Ret=FALSE;break;
						case PrmType_DOUBLE:if (*(double*)pStrSimpleQBE->ValueParam>*(double*)VolIDI) Ret=TRUE;else Ret=FALSE;break;
						case PrmType_CHAR:if (*(char*)pStrSimpleQBE->ValueParam>*(char*)VolIDI) Ret=TRUE;else Ret=FALSE;break;
						case PrmType_SHORT:if (*(short*)pStrSimpleQBE->ValueParam>*(short*)VolIDI) Ret=TRUE;else Ret=FALSE;break;						
						default:Ret=FALSE;//?????
							break;
						};	
						break;
					case EqualToOrGreaterThan:
						switch(pNewParam->Type)
						{
						case PrmType_INT:if (*(int*)pStrSimpleQBE->ValueParam<=*(int*)VolIDI) Ret=TRUE;else Ret=FALSE;break;
						case PrmType_INT64:if (*(INT64*)pStrSimpleQBE->ValueParam<=*(INT64*)VolIDI) Ret=TRUE;else Ret=FALSE;break;
						case PrmType_STRING:if (strcmp((char*)pStrSimpleQBE->ValueParam,(char*)VolIDI)>=0) Ret=TRUE;else Ret=FALSE;break;
						case PrmType_FLOAT:if (*(float*)pStrSimpleQBE->ValueParam<=*(float*)VolIDI) Ret=TRUE;else Ret=FALSE;break;
						case PrmType_DOUBLE:if (*(double*)pStrSimpleQBE->ValueParam<=*(double*)VolIDI) Ret=TRUE;else Ret=FALSE;break;
						case PrmType_CHAR:if (*(char*)pStrSimpleQBE->ValueParam<=*(char*)VolIDI) Ret=TRUE;else Ret=FALSE;break;
						case PrmType_SHORT:if (*(short*)pStrSimpleQBE->ValueParam<=*(short*)VolIDI) Ret=TRUE;else Ret=FALSE;break;						
						default:Ret=FALSE;//?????
							break;
						};
						break;
					case EqualToOrLessThan:
						switch(pNewParam->Type)
						{
						case PrmType_INT:if (*(int*)pStrSimpleQBE->ValueParam>=*(int*)VolIDI) Ret=TRUE;else Ret=FALSE;break;
						case PrmType_INT64:if (*(INT64*)pStrSimpleQBE->ValueParam>=*(INT64*)VolIDI) Ret=TRUE;else Ret=FALSE;break;
						case PrmType_STRING:if (strcmp((char*)pStrSimpleQBE->ValueParam,(char*)VolIDI)<=0) Ret=TRUE;else Ret=FALSE;break;
						case PrmType_FLOAT:if (*(float*)pStrSimpleQBE->ValueParam>=*(float*)VolIDI) Ret=TRUE;else Ret=FALSE;break;
						case PrmType_DOUBLE:if (*(double*)pStrSimpleQBE->ValueParam>=*(double*)VolIDI) Ret=TRUE;else Ret=FALSE;break;
						case PrmType_CHAR:if (*(char*)pStrSimpleQBE->ValueParam>=*(char*)VolIDI) Ret=TRUE;else Ret=FALSE;break;
						case PrmType_SHORT:if (*(short*)pStrSimpleQBE->ValueParam>=*(short*)VolIDI) Ret=TRUE;else Ret=FALSE;break;						
						default:Ret=FALSE;//?????
							break;
						};					
						break;
					case Unequal:
						switch(pNewParam->Type)
						{
						case PrmType_INT:if (*(int*)pStrSimpleQBE->ValueParam!=*(int*)VolIDI) Ret=TRUE;else Ret=FALSE;break;
						case PrmType_INT64:if (*(INT64*)pStrSimpleQBE->ValueParam!=*(INT64*)VolIDI) Ret=TRUE;else Ret=FALSE;break;
						case PrmType_STRING:if (strcmp((char*)pStrSimpleQBE->ValueParam,(char*)VolIDI)!=0) Ret=TRUE;else Ret=FALSE;break;
						case PrmType_FLOAT:if (*(float*)pStrSimpleQBE->ValueParam!=*(float*)VolIDI) Ret=TRUE;else Ret=FALSE;break;
						case PrmType_DOUBLE:if (*(double*)pStrSimpleQBE->ValueParam!=*(double*)VolIDI) Ret=TRUE;else Ret=FALSE;break;
						case PrmType_CHAR:if (*(char*)pStrSimpleQBE->ValueParam!=*(char*)VolIDI) Ret=TRUE;else Ret=FALSE;break;
						case PrmType_SHORT:if (*(short*)pStrSimpleQBE->ValueParam!=*(short*)VolIDI) Ret=TRUE;else Ret=FALSE;break;						
						default:Ret=FALSE;//?????
							break;
						};	
						break;
					case WithoutPrm:
						Ret=FALSE;
						break;
					default:
						break;
					}
				}
				else
				{
					if (pStrSimpleQBE->TypeArifmetic==WithoutPrm)
						Ret=TRUE;
// 					else 
// 						break;
				}
				if ((pStrSimpleQBE->TypeLogical==LogicalAND) && (Ret==FALSE))
				{break;}
				else if ((pStrSimpleQBE->TypeLogical==LogicalOR) && (Ret==TRUE))
				{break;}
				if (pStrSimpleQBE->TypeLogical==0)
				{break;}
				ii--;pSendBuff+=sizeof(StrSimpleQBE);
			}
		};
	}	
	return Ret;
};
/*
IDInfoDamper::IDInfoDamper()
{
	Srv=GetServices();
	IDL=new	ID_List8();
	IDLCnt=new	ID_List8();
	SzIDID.NumIDI=0;
	SzIDID.MemSize=0;
	OrderNumber=0;
};

IDInfoDamper::~IDInfoDamper()
{
	delete	IDL; IDL=NULL;
	delete	IDLCnt; IDLCnt=NULL;
};

int			IDInfoDamper::WriteIDI(IDInfo* IDI)
{
	int	rc=-1;
	MyLock(__FILE__,__LINE__,&Fl_RE,NULL);
	IDInfo*	tIDI=FindIDI(IDI->ID_File);
	if (tIDI==NULL)
	{
		tIDI=Srv->GetNewIDI(Srv);
		if (tIDI)
		{
			Srv->CopyIDInfo(IDI,tIDI);
			if (IDL->Add(tIDI)==-1)
			{
				Srv->DeleteIDI(tIDI);	tIDI=NULL;
				Srv->SetCriticalError(Srv,"IDInfoDamper Error","Critical","IDInfoDamper");
			}
			else
			{
				tIDI->OrderNumber=ByteReverse(OrderNumber++);
				El_OrderNum*	tEl= new El_OrderNum(tIDI->OrderNumber,IDI->ID_File);
				if (IDLCnt->Add(tEl)==-1)
				{
					delete tEl; tEl=NULL;
					Srv->SetCriticalError(Srv,"IDInfoDamper Error","Critical","IDInfoDamper");
				}
			}
			SzIDID.NumIDI=IDL->NumElem;
		}
	}
	else
	{
		Srv->CopyIDInfo(IDI,tIDI);
	}

	MyUnLock(__FILE__,__LINE__,&Fl_RE,NULL);
	return rc;
};

IDInfo*		IDInfoDamper::ReadIDI()
{
	IDInfo*	tIDI=NULL;
	MyLock(__FILE__,__LINE__,&Fl_RE,NULL);
	El_OrderNum*	tEl=(El_OrderNum*)IDLCnt->Begin();
	if (tEl)
		tIDI=FindIDI(tEl->ID);	// ?
	MyUnLock(__FILE__,__LINE__,&Fl_RE,NULL);
	return tIDI;
};

IDInfo*		IDInfoDamper::ReadAndRemoveIDI()
{
	IDInfo*	tIDI=NULL;
	MyLock(__FILE__,__LINE__,&Fl_RE,NULL);
	El_OrderNum*	tEl=(El_OrderNum*)IDLCnt->Begin();
	if (tEl)
	{
		tIDI=FindIDI(tEl->ID);	// ?
		ID_Elem*	tEl=IDLCnt->Find((UCHAR*)&tIDI->OrderNumber,sizeof(INT64));
		IDLCnt->Del(tEl);
		if (IDL->Remove(tIDI)==-1)
		{
			{int iasm=1;}
		}
		SzIDID.NumIDI=IDL->NumElem;
	}
	MyUnLock(__FILE__,__LINE__,&Fl_RE,NULL);
	return tIDI;
};

int			IDInfoDamper::DelIDI(IDInfo* IDI)
{
	int	rc=-1;
	MyLock(__FILE__,__LINE__,&Fl_RE,NULL);
	ID_Elem*	tEl=IDLCnt->Find((UCHAR*)&IDI->OrderNumber,sizeof(INT64));
	IDLCnt->Del(tEl);
	if (IDL->Remove(IDI)==-1)
	{
		IDInfo*	tIDI=FindIDI(IDI);
		if (tIDI)
		{
			IDL->Remove(tIDI);
			Srv->DeleteIDI(tIDI); tIDI=NULL;
		}
	}
	else
	{
		
		Srv->DeleteIDI(IDI); IDI=NULL;
	}
	SzIDID.NumIDI=IDL->NumElem;
	MyUnLock(__FILE__,__LINE__,&Fl_RE,NULL);
	return rc;
};
IDInfo*		IDInfoDamper::FindIDI(IDInfo* IDI)
{
	IDInfo*	tIDI;
	MyLock(__FILE__,__LINE__,&Fl_RE,NULL);
	tIDI=(IDInfo*)IDL->Find((UCHAR*)&IDI->ID_File,sizeof(INT64));
	MyUnLock(__FILE__,__LINE__,&Fl_RE,NULL);
	return tIDI;
};

IDInfo*		IDInfoDamper::FindIDI(INT64 ID)
{
	IDInfo*	tIDI;
	MyLock(__FILE__,__LINE__,&Fl_RE,NULL);
	tIDI=(IDInfo*)IDL->Find((UCHAR*)&ID,sizeof(INT64));
	MyUnLock(__FILE__,__LINE__,&Fl_RE,NULL);
	return tIDI;
};

StrSzIDID*	IDInfoDamper::GetSize()
{
	return	&SzIDID;
};
*/

IDInfoConverter::IDInfoConverter()
{
	Buf=NULL;
	Srv=GetServices();
//	tIDI_InfoConverter=Srv->GetNewIDI(Srv);
//	tIDI_InfoConverter=new IDInfo();
	tIDI_InfoConverter=NULL;
	SizeOB=1024;
	OutBuf=(UCHAR*)malloc(SizeOB);
	SizePrm=1024;
	PrmBuf=(UCHAR*)malloc(SizePrm);
};

IDInfoConverter::~IDInfoConverter()
{
	if (tIDI_InfoConverter)
		delete tIDI_InfoConverter;
//	GetServices()->DeleteIDI(tIDI_InfoConverter);
	free(OutBuf);
	free(PrmBuf);
};

uchar*	IDInfoConverter::IDInfo2Buf(IDInfo* IDI,size_t	&Cnt)
{
	BL.DelAll();
	BL.AddElem((uchar*)IDI,sizeof(IDIStat));	//	IDInfo
	BL.AddElem(IDI->Buf,IDI->Cnt);				//	Buffer
	if (IDI->AdrInfo)
		BL.AddElem((UCHAR*)IDI->AdrInfo,sizeof(AddressInfo));				//	Buffer
	else
		BL.AddElem(NULL,0);				//	Buffer
	uchar*	tB;
	size_t		tC;
	PList2Buf(IDI,tB,tC);
	BL.AddElem(tB,tC);				//	Buffer
	Buf=BL.GetDataBuf(Cnt);
	return	Buf;
};

IDInfo*	IDInfoConverter::Buf2IDInfo(uchar* Buf,size_t	Cnt)
{
	uchar*	tBuf;
	size_t		tCnt;
	tIDI_InfoConverter=Srv->GetNewIDI(Srv,1);
//	tIDI_InfoConverter=new IDInfo(Srv);
	BL.SetDataBuf(Buf,Cnt);
	BL.FindFirst(tBuf,tCnt,&BF);	// IDInfo
	if (tCnt==sizeof(IDIStat))
	{
		memcpy((UCHAR*)tIDI_InfoConverter+sizeof(ID_Elem),tBuf+sizeof(ID_Elem),tCnt-sizeof(ID_Elem));
	}
	BL.FindNext(tBuf,tCnt,&BF);		// Data
	if (tCnt!=0)
	{
		if (SizeOB<tCnt)
		{
			free(OutBuf);
			SizeOB=tCnt;
			OutBuf=(UCHAR*)malloc(SizeOB);
		}
		memcpy(OutBuf,tBuf,tCnt);
		tIDI_InfoConverter->Buf=OutBuf;
		tIDI_InfoConverter->Cnt=tCnt;
	}
	else
	{
		tIDI_InfoConverter->Cnt=0;
	}
	BL.FindNext(tBuf,tCnt,&BF);		// Address
	if (tCnt!=0)
	{
//		tIDI_InfoConverter->AdrInfo=&AdrI;
		if (tCnt==sizeof(AddressInfo))
			memcpy(tIDI_InfoConverter->AdrInfo,tBuf,tCnt);
		else
		{
			delete tIDI_InfoConverter->AdrInfo;
			tIDI_InfoConverter->AdrInfo=0;
		}
	}
	else
	{
		delete tIDI_InfoConverter->AdrInfo;
		tIDI_InfoConverter->AdrInfo=0;
	}
	BL.FindNext(tBuf,tCnt,&BF);		// Param
	if (tCnt!=0)
	{
		Buf2PList(tBuf,tCnt,tIDI_InfoConverter);
	}
	tIDI_InfoConverter->ParentFile=NULL;
	tIDI_InfoConverter->SeansFrom=NULL;
	tIDI_InfoConverter->SeansTo=NULL;
	return	tIDI_InfoConverter;
};

void	IDInfoConverter::ReleaseIDInfo(IDInfo* tIDI)
{
//	delete tIDI;
	Srv->DeleteIDI(tIDI,1);
	tIDI_InfoConverter=NULL;
};

int	IDInfoConverter::PList2Buf(IDInfo* IDI,uchar* &Buf,size_t	&Cnt)
{
	int	rc=0;
	INT64		tID;
	void*		tVol;
	size_t		tC;
	PL.DelAll();
	IDI->pPList->FindFirst(tID,tVol,tC);
	while(tC)
	{
		if (SizePrm<tC+sizeof(PrmHeader))
		{
			free(PrmBuf);
			SizePrm=tC+sizeof(PrmHeader);
			PrmBuf=(UCHAR*)malloc(SizePrm);
		}
		((PrmHeader*)PrmBuf)->ID=tID;
		((PrmHeader*)PrmBuf)->Cnt=tC;
		memcpy(PrmBuf+sizeof(PrmHeader),tVol,tC);
		PL.AddElem(PrmBuf,tC+sizeof(PrmHeader));	//	Parametr
		IDI->pPList->FindNext(tID,tVol,tC);
	}
	Buf=PL.GetDataBuf(Cnt);
	return	rc;
};

int	IDInfoConverter::Buf2PList(uchar* Buf,size_t Cnt,IDInfo* IDI)
{
	int	rc=0;
	uchar*	tBuf;
	size_t		tCnt;
	IDI->pPList->DelAllElem();
	PL.SetDataBuf(Buf,Cnt);
	PL.FindFirst(tBuf,tCnt,&BF);	// IDInfo
	while (tCnt!=0)
	{
		IDI->pPList->SetParam(((PrmHeader*)tBuf)->ID,(char*)(tBuf+sizeof(PrmHeader)),((PrmHeader*)tBuf)->Cnt,Srv);
		PL.FindNext(tBuf,tCnt,&BF);		// Data
	}
	return	rc;
};



//-------------------------------------------
static char	EndFile[]="\n\n[]\n;_";
static char	R_RS[]=";_\n";
static char	R_Pl[]=";_\n";
static char	R_Fr[]=";_\n";
static char	R_Ad[]=";_\n";
static char	R_Se[]=";_\n";
static char	R_TC[]=";_\n";
//-------------------------------------------
static char	R_Md[]=";_\n";
static char	R_En[]=";_\n";
static char	R_Sp[]=";_\n";
static char	R_UD[]=";_\n";
static char	R_Mt[]=";_\n";
static char	R_Pr[]=";_\n";
//-------------------------------------------
static char	R_LP[]=";_\n";
static char	R_LPO[]=";_\n";
//static char	R_ML[]=";_\n";
static char	R_Un[]=";_\n";
//-------------------------------------------
static char	R_BM[]=";_\n";
static char	R_BP[]=";_\n";
static char	R_BI[]=";_\n";
static char	R_BO[]=";_\n";
//-------------------------------------------

//*********************************************************
DataImEx::DataImEx(){};
//*********************************************************
DataImEx::~DataImEx(){};
//*********************************************************
void	DataImEx::ExportTsk(Task* Tsk,char *PathName)
{
ExpTsk(Tsk,PathName,NULL);
return;
};
//*********************************************************
void	DataImEx::ImportTsk(Task* Tsk,char *PathName)
{
ImpTsk(Tsk,PathName,NULL);
return;
};
//*********************************************************
void	DataImEx::ImportMode(Task* Tsk,char *PathName)
{
//---------------------------------------------------------
char	NameMd[256];
//---------------------------------------------------------
FILE 	*HFileRD=fopen(PathName,"rt");
if(HFileRD==NULL) return;
if(ReadCFG(HFileRD,NameMd)==-1){fclose(HFileRD);return;};
if(strcmp(NameMd,"[Md]")==0)
  {if(ReadCFG(HFileRD,NameMd)==-1){fclose(HFileRD);return;};
  fclose(HFileRD);
  //Èìÿ ìåòîäà ïðî÷èòàíî
  ExpTsk(Tsk,"importmd.tmp",NameMd);
  ImpTsk(Tsk,"importmd.tmp",PathName);
  remove("importmd.tmp");
  };
return;
};
//*********************************************************
void	DataImEx::ExportMode(Task* Tsk,int NomMd,char *PathName)
{
FILE *HFileSD=fopen(PathName,"wt");
//------------------------------------------5-----------------------------------
// Ðåæèì
i1=NomMd;
fprintf(HFileSD,"\n\n;***************5***************\n");
fprintf(HFileSD,R_Md);
fprintf(HFileSD,"[Md]\t");
fprintf(HFileSD,"%s\n",Tsk->Md[i1].Name);
fprintf(HFileSD,"MethObn\t%d\n",Tsk->Md[i1].MethObn);
fprintf(HFileSD,"TimeProb\t%d\n",Tsk->Md[i1].TimeProb);
//
fprintf(HFileSD,"X_Md\t%d\n",Tsk->Md[i1].X);
fprintf(HFileSD,"Y_Md\t%d\n",Tsk->Md[i1].Y);
//

// Ýíåðãåòè÷. êðèòåðèé
if(Tsk->Md[i1].ID_En<Tsk->Max_En)
if(Tsk->En[Tsk->Md[i1].ID_En].Name[0]!=0)
{
fprintf(HFileSD,R_En);
fprintf(HFileSD,"[Md|En]\t");
fprintf(HFileSD,"%s\n",Tsk->En[Tsk->Md[i1].ID_En].Name);
};
// Ñïåêòðàëüíûé êðèòåðèé
if(Tsk->Md[i1].ID_Sp<Tsk->Max_Sp)
if(Tsk->Sp[Tsk->Md[i1].ID_Sp].Name[0]!=0)
{
fprintf(HFileSD,R_Sp);
fprintf(HFileSD,"[Md|Sp]\t");
fprintf(HFileSD,"%s\n",Tsk->Sp[(Tsk->Md[i1].ID_Sp)].Name);
};
// Óíèâåðñàëüíûé äåìîäóëÿòîð
if(Tsk->Md[i1].ID_UD<Tsk->Max_UD)
if(Tsk->UD[Tsk->Md[i1].ID_UD].Name[0]!=0)
{
fprintf(HFileSD,R_UD);
fprintf(HFileSD,"[Md|UD]\t");
fprintf(HFileSD,"%s\n",Tsk->UD[Tsk->Md[i1].ID_UD].Name);
};
for(i2=0;i2<Tsk->Max_Mt;i2++)
   if(Tsk->Mt[i2].ID_Md==i1)
	{// Ìåòîä âõîäÿùèé â äàííûé ðåæèì
	DublM=0;
	for(size_t i99=0;i99<i2;i99++)
	   if(Tsk->Mt[i99].ID_Md==i1)//Ïðèíàäëåæèò ðåæèìó
	   if(strcmp(Tsk->Mt[i2].Name,Tsk->Mt[i99].Name)==0)DublM++;
	fprintf(HFileSD,";===============5===============\n");
	fprintf(HFileSD,R_Mt);
	fprintf(HFileSD,"[Md|Mt]\t");
	fprintf(HFileSD,"%d|%s\n",DublM,Tsk->Mt[i2].Name);
//	printf("%d|%s\n",DublM,Tsk->Mt[i2].Name);
	fprintf(HFileSD,"N_PO\t%d\n",Tsk->Mt[i2].N_PO);
	fprintf(HFileSD,"N_PI\t%d\n",Tsk->Mt[i2].N_PI);
    fprintf(HFileSD,"X_Mt\t%d\n",Tsk->Mt[i2].X);
    fprintf(HFileSD,"Y_Mt\t%d\n",Tsk->Mt[i2].Y);
	////	int	N_PO;
	for(i3=0;i3<Tsk->Max_Pr;i3++)
	   if(Tsk->Pr[i3].ID_Mt==i2)
		{// Ïàðàìåòðû ìåòîäà
		fprintf(HFileSD,R_Pr);
		fprintf(HFileSD,"[Md|Mt|Pr]\t");
		fprintf(HFileSD,"%s\n",Tsk->Pr[i3].Name);
//		fprintf(HFileSD,"ID_Mt\t%d\n",Tsk->Pr[i3].ID_Mt);
		if(Tsk->Pr[i3].Vol[0]==0)
			fprintf(HFileSD,"Vol\t#\n");
			else
			fprintf(HFileSD,"Vol\t%s\n",EncStr(Tsk->Pr[i3].Vol));
		};
	for(i3=0;i3<Tsk->Max_LP;i3++)
	   if(Tsk->LP[i3].ID_Mt2==i2)
		{// Ëèíêè ìåòîäà
//		fprintf(HFileSD,";---------------5---------------\n");
		fprintf(HFileSD,R_LP);
		fprintf(HFileSD,"[Md|Mt|LP]\n");
		fprintf(HFileSD,"PO\t%d\n",Tsk->LP[i3].PO);
		if(Tsk->LP[i3].ID_Mt1!=-1)
		{
		DublM0=0;
		for(size_t i99=0;i99<Tsk->LP[i3].ID_Mt1;i99++)
		   if(Tsk->Mt[i99].ID_Md==i1)//Ïðèíàäëåæèò ðåæèìó
		   if(strcmp(Tsk->Mt[Tsk->LP[i3].ID_Mt1].Name,Tsk->Mt[i99].Name)==0)DublM0++;
			fprintf(HFileSD,"Mt1\t%d|%s\n",DublM0,Tsk->Mt[Tsk->LP[i3].ID_Mt1].Name);
			}
			else fprintf(HFileSD,"Mt1\tInput\n");
		fprintf(HFileSD,"PI\t%d\n",Tsk->LP[i3].PI);
		if(Tsk->LP[i3].ID_Mt2!=-1)
			fprintf(HFileSD,"Mt2\t%d|%s\n",DublM,Tsk->Mt[i2].Name);
			else fprintf(HFileSD,"Mt1\t#\n");
		};
	// Âûõîäíûå íîãè
	for(i3=0;i3<Tsk->Max_LP;i3++)
	   if(Tsk->LP[i3].ID_Mt1==i2)
	   if(Tsk->LP[i3].ID_Mt2==-1)
		{
		fprintf(HFileSD,R_LPO);
		fprintf(HFileSD,"[Md|Mt|LP]\n");
		fprintf(HFileSD,"PO\t%d\n",Tsk->LP[i3].PO);
		if(Tsk->LP[i3].ID_Mt1!=-1)
			{
			DublM0=0;
			for(size_t i99=0;i99<Tsk->LP[i3].ID_Mt1;i99++)
			   if(Tsk->Mt[i99].ID_Md==i1)//Ïðèíàäëåæèò ðåæèìó
			   if(strcmp(Tsk->Mt[Tsk->LP[i3].ID_Mt1].Name,Tsk->Mt[i99].Name)==0)DublM0++;
			fprintf(HFileSD,"Mt1\t%d|%s\n",DublM0,Tsk->Mt[Tsk->LP[i3].ID_Mt1].Name);
			}
			else fprintf(HFileSD,"Mt1\tInput\n");
		fprintf(HFileSD,"PI\t%d\n",Tsk->LP[i3].PI);
		fprintf(HFileSD,"Mt2\tOutput\n");
		};
	};
fprintf(HFileSD,EndFile);
fclose(HFileSD);
return;
};








//*********************************************************
void	DataImEx::ExpTsk(Task* Tsk,char *PathName,char *DelMode)
{
//Task*   Tsk=Tsk0;

//FILE *HFileSD=fopen("Data000.tsk","wt");
FILE *HFileSD=fopen(PathName,"wt");
if (HFileSD==NULL) return;
//------------------------------------------1-----------------------------------
// Áàçîâûå ìåòîäû
for(i1=0;i1<Tsk->Max_BMt;i1++)
	{// Áàçîâûé ìåòîä
	fprintf(HFileSD,"\n\n;***************1***************\n");
	fprintf(HFileSD,R_BM);
	fprintf(HFileSD,"[BM]\t");
	fprintf(HFileSD,"%s\n",Tsk->BMt[i1].Name);
	fprintf(HFileSD,"N_PI\t%d\n",Tsk->BMt[i1].N_PI);
	fprintf(HFileSD,"N_PO\t%d\n",Tsk->BMt[i1].N_PO);
	for(i2=0;i2<Tsk->Max_BIn;i2++)
	   if(Tsk->BIn[i2].ID_BMt==i1)
		{// Âõîäíûå íîæêè áàçîâîãî ìåòîäà
//		fprintf(HFileSD,"\n;===============1===============\n");
		fprintf(HFileSD,R_BI);
		fprintf(HFileSD,"[BM|BI]\t");
		fprintf(HFileSD,"%s\n",Tsk->BIn[i2].Name);
//		fprintf(HFileSD,"Rem\t{%s}\n",Tsk->BIn[i2].Rem);
		};
	for(i2=0;i2<Tsk->Max_BOut;i2++)
	   if(Tsk->BOut[i2].ID_BMt==i1)
		{// Âûõîäíûå íîæêè áàçîâîãî ìåòîäà
//		fprintf(HFileSD,"\n;===============1===============\n");
		fprintf(HFileSD,R_BO);
		fprintf(HFileSD,"[BM|BO]\t");
		fprintf(HFileSD,"%s\n",Tsk->BOut[i2].Name);
//		fprintf(HFileSD,"Rem\t{%s}\n",Tsk->BOut[i2].Rem);
		};
	for(i2=0;i2<Tsk->Max_BPr;i2++)
	   if(Tsk->BPr[i2].ID_BMt==i1)
		{// Ïàðàìåòðû áàçîâîãî ìåòîäà
//		fprintf(HFileSD,"\n;===============1===============\n");
		fprintf(HFileSD,R_BP);
		fprintf(HFileSD,"[BM|BP]\t");
		fprintf(HFileSD,"%s\n",Tsk->BPr[i2].Name);
		if(Tsk->BPr[i2].Vol[0]==0)
			fprintf(HFileSD,"Vol\t#\n");
			else
			fprintf(HFileSD,"Vol\t%s\n",EncStr(Tsk->BPr[i2].Vol));
		};
	};//*Áàçîâûå ìåòîäû

//
//
//
//------------------------------------------2-----------------------------------
// Ýíåðãåòè÷. êðèòåðèè
for(i1=0;i1<Tsk->Max_En;i1++)
	{fprintf(HFileSD,"\n\n;***************2***************\n");
	fprintf(HFileSD,R_En);
	fprintf(HFileSD,"[En]\t");
	fprintf(HFileSD,"%s\n",Tsk->En[i1].Name);
	fprintf(HFileSD,"Ep\t%d\n",Tsk->En[i1].Ep);
	};
//
//
//
//------------------------------------------3-----------------------------------
// Ñïåêòð. êðèòåðèè
for(i1=0;i1<Tsk->Max_Sp;i1++)
	{fprintf(HFileSD,"\n\n;***************3***************\n");
	fprintf(HFileSD,R_Sp);
	fprintf(HFileSD,"[Sp]\t");
	fprintf(HFileSD,"%s\n",Tsk->Sp[i1].Name);
	fprintf(HFileSD,"Eps\t%d\n",Tsk->Sp[i1].Eps);
	////int	Eps;		// ýíåðãèòè÷åñêèé ïîðîã óñåêàíèÿ ñïåêòðà
	fprintf(HFileSD,"dEps\t%d\n",Tsk->Sp[i1].dEps);
	////int	dEps;		// ïîðîã ÑÊÎ
	fprintf(HFileSD,"Kosr\t%d\n",Tsk->Sp[i1].Kosr);
	////int	Kosr;		// îñðåäíåíèå ñïåêòðà
	fprintf(HFileSD,"Porog\t%d\n",Tsk->Sp[i1].Porog);
	////int	Porog;		// ïîðîã
	fprintf(HFileSD,"Ep\t%d\n",Tsk->Sp[i1].Ep);
	////int	Ep;		// ýíåðãèòè÷åñêèé ïîðîã ñðàáàòûâàíèÿ òðåâîãè
	fprintf(HFileSD,"Spe\n");
	for(size_t i2=0;i2<8;i2++)
	   {for(size_t i3=0;i3<16;i3++)fprintf(HFileSD,"%d\t",(int)Tsk->Sp[i1].Spe[i2*16+i3]);
	   fprintf(HFileSD,"\n");};
	////UCHAR	Spe[128];
	};
//
//
//
//------------------------------------------4-----------------------------------
// Óíèâåðñàëüíûé äåìîäóëÿòîð
for(i1=0;i1<Tsk->Max_UD;i1++)
	{fprintf(HFileSD,"\n\n;***************4***************\n");
	// Óíèâåðñàëüíûé äåìîäóëÿòîð
	fprintf(HFileSD,R_UD);
	fprintf(HFileSD,"[UD]\t");
	fprintf(HFileSD,"%s\n",Tsk->UD[i1].Name);
	fprintf(HFileSD,"Mod\t%d\n",Tsk->UD[i1].Mod);
	////int	Mod;		// Òèï äåìîäóëÿòîðà
	fprintf(HFileSD,"Star\t%d\n",Tsk->UD[i1].Star);
	////int	Star;		// Âèä ñîçâåçäèÿ
	fprintf(HFileSD,"TypeTrnsm\t%d\n",Tsk->UD[i1].TypeTrnsm);
	////int	TypeTrnsm;     	// Òèï ïåðåäà÷è  1-íåïð 2-áëî÷í 3-ïàê
	fprintf(HFileSD,"Pilot\t%d\n",Tsk->UD[i1].Pilot);
	////int	Pilot;		//
	fprintf(HFileSD,"SynchP\n");
	for(size_t i2=0;i2<3;i2++)
	   {for(size_t i3=0;i3<10;i3++)fprintf(HFileSD,"%d\t",(int)Tsk->UD[i1].SynchP[i2*10+i3]);
	   fprintf(HFileSD,"\n");};
	////char	SynchP[30];	//
	fprintf(HFileSD,"Synch\t%d\n",Tsk->UD[i1].Synch);
	////int	Synch;		// Âèä òàêòîâîé ñèíõðîíèçàöèè
	fprintf(HFileSD,"SS_Inf\t%d\n",Tsk->UD[i1].SS_Inf);
	////int	SS_Inf;		// Êîëè÷åñòâî èíô áèò
	fprintf(HFileSD,"SS_Stop\t%d\n",Tsk->UD[i1].SS_Stop);
	////int	SS_Stop;	// Êîëè÷åñòâî ñòîïîâ
	fprintf(HFileSD,"Bl_T\t%d\n",Tsk->UD[i1].Bl_T);
	////int	Bl_T;		// ïåðèîä áëî÷íîé ïåðåäà÷è
	fprintf(HFileSD,"Bl_D\t%d\n",Tsk->UD[i1].Bl_D);
	////int	Bl_D;		// äëèíà èíôîðìàöèîííîé ÷àñòè
	fprintf(HFileSD,"De_Fn\t%f\n",Tsk->UD[i1].De_Fn);
	////float  De_Fn;
	fprintf(HFileSD,"De_dFn\t%f\n",Tsk->UD[i1].De_dFn);
	////float  De_dFn;
	fprintf(HFileSD,"De_Fb\t%f\n",Tsk->UD[i1].De_Fb);
	////float  De_Fb;
	fprintf(HFileSD,"De_dFb\t%f\n",Tsk->UD[i1].De_dFb);
	////float  De_dFb;
	fprintf(HFileSD,"De_Fr\t%f\n",Tsk->UD[i1].De_Fr);
	////float  De_Fr;
	fprintf(HFileSD,"De_dFr\t%f\n",Tsk->UD[i1].De_dFr);
	////float  De_dFr;
	fprintf(HFileSD,"De_Fch\t%f\n",Tsk->UD[i1].De_Fch);
	////float  De_Fch;
	fprintf(HFileSD,"De_dFch\t%f\n",Tsk->UD[i1].De_dFch);
	////float  De_dFch;
	fprintf(HFileSD,"De_Fp\t%f\n",Tsk->UD[i1].De_Fp);
	////float  De_Fp;
	fprintf(HFileSD,"De_dFp\t%f\n",Tsk->UD[i1].De_dFp);
	////float  De_dFp;
	fprintf(HFileSD,"De_Invert\t%d\n",Tsk->UD[i1].De_Invert);
	////int	De_Invert;
	fprintf(HFileSD,"De_Kaea\t%f\n",Tsk->UD[i1].De_Kaea);
	////float	De_Kaea;
	fprintf(HFileSD,"De_Kaen\t%f\n",Tsk->UD[i1].De_Kaen);
	////float	De_Kaen;
	fprintf(HFileSD,"De_Kaee\t%f\n",Tsk->UD[i1].De_Kaee);
	////float	De_Kaee;
	fprintf(HFileSD,"De_K1\t%f\n",Tsk->UD[i1].De_K1);
	////float	De_K1;
	fprintf(HFileSD,"De_K2\t%f\n",Tsk->UD[i1].De_K2);
	////float	De_K2;
	fprintf(HFileSD,"De_K21\t%f\n",Tsk->UD[i1].De_K21);
	////float	De_K21;
	fprintf(HFileSD,"De_K22\t%f\n",Tsk->UD[i1].De_K22);
	////float	De_K22;
	fprintf(HFileSD,"De_porog\t%f\n",Tsk->UD[i1].De_porog);
	////float	De_porog;
	fprintf(HFileSD,"De_porog_1\t%f\n",Tsk->UD[i1].De_porog_1);
	////float	De_porog_1;
	fprintf(HFileSD,"De_CAgc\t%e\n",Tsk->UD[i1].De_CAgc);
	////float	De_CAgc;
	fprintf(HFileSD,"De_CAgc2\t%e\n",Tsk->UD[i1].De_CAgc2);
	////float	De_CAgc2;
	fprintf(HFileSD,"De_CAgcMin\t%e\n",Tsk->UD[i1].De_CAgcMin);
	////float	De_CAgcMin;
	fprintf(HFileSD,"De_CAgcMax\t%e\n",Tsk->UD[i1].De_CAgcMax);
	////float	De_CAgcMax;
	fprintf(HFileSD,"De_Enp\t%f\n",Tsk->UD[i1].De_Enp);
	////float	De_Enp;
	fprintf(HFileSD,"De_Enp1\t%f\n",Tsk->UD[i1].De_Enp1);
	////float	De_Enp1;
	fprintf(HFileSD,"De_Enp2\t%f\n",Tsk->UD[i1].De_Enp2);
	////float	De_Enp2;
	fprintf(HFileSD,"De_Enp3\t%f\n",Tsk->UD[i1].De_Enp3);
	////float	De_Enp3;
	fprintf(HFileSD,"De_Ka1\t%d\n",Tsk->UD[i1].De_Ka1);
	////int	De_Ka1;
	fprintf(HFileSD,"De_Ka2\t%d\n",Tsk->UD[i1].De_Ka2);
	////int	De_Ka2;
	fprintf(HFileSD,"De_Ke\t%f\n",Tsk->UD[i1].De_Ke);
	////float	De_Ke;
	fprintf(HFileSD,"De_Km\t%f\n",Tsk->UD[i1].De_Km);
	////float	De_Km;
	fprintf(HFileSD,"De_Ksta\t%f\n",Tsk->UD[i1].De_Ksta);
	////float	De_Ksta;
	fprintf(HFileSD,"De_Kstn\t%f\n",Tsk->UD[i1].De_Kstn);
	////float	De_Kstn;
	//
	fprintf(HFileSD,"De_PersP\t%d\n",Tsk->UD[i1].De_PersP);
	////int	De_PersP;
	fprintf(HFileSD,"De_F1\t%f\n",Tsk->UD[i1].De_F1);
	////float	De_F1;
	fprintf(HFileSD,"De_F2\t%f\n",Tsk->UD[i1].De_F2);
	////float	De_F2;
	fprintf(HFileSD,"De_F3\t%f\n",Tsk->UD[i1].De_F3);
	////float	De_F3;
	fprintf(HFileSD,"De_F4\t%f\n",Tsk->UD[i1].De_F4);
	////float	De_F4;
	fprintf(HFileSD,"De_dF\t%f\n",Tsk->UD[i1].De_dF);
	////float	De_dF;
	};
//
//
//
//------------------------------------------5-----------------------------------
// Ðåæèì
for(i1=0;i1<Tsk->Max_Md;i1++)
	{// Ðåæèì
	//Ïðîïóñê íåæåëàòåëüíîãî ðåæèìà
	if(DelMode!=NULL)
	if(strcmp(Tsk->Md[i1].Name,DelMode)==0)continue;
	//
	fprintf(HFileSD,"\n\n;***************5***************\n");
	fprintf(HFileSD,R_Md);
	fprintf(HFileSD,"[Md]\t");
	fprintf(HFileSD,"%s\n",Tsk->Md[i1].Name);
	fprintf(HFileSD,"MethObn\t%d\n",Tsk->Md[i1].MethObn);
	fprintf(HFileSD,"TimeProb\t%d\n",Tsk->Md[i1].TimeProb);
    //
	fprintf(HFileSD,"X_Md\t%d\n",Tsk->Md[i1].X);
    fprintf(HFileSD,"Y_Md\t%d\n",Tsk->Md[i1].Y);
	//
//	int	MethObn;	// ìåäîä îáíàðóæåíèÿ
//	int	TimeProb;	// âðåìÿ ïðîáû äåìîäóëÿöèè
	// Ýíåðãåòè÷. êðèòåðèé
	if(Tsk->Md[i1].ID_En<Tsk->Max_En)
	if(Tsk->En[Tsk->Md[i1].ID_En].Name[0]!=0)
	{
	fprintf(HFileSD,R_En);
	fprintf(HFileSD,"[Md|En]\t");
	fprintf(HFileSD,"%s\n",Tsk->En[Tsk->Md[i1].ID_En].Name);
	};
	// Ñïåêòðàëüíûé êðèòåðèé
	if(Tsk->Md[i1].ID_Sp<Tsk->Max_Sp)
	if(Tsk->Sp[Tsk->Md[i1].ID_Sp].Name[0]!=0)
	{
	fprintf(HFileSD,R_Sp);
	fprintf(HFileSD,"[Md|Sp]\t");
	fprintf(HFileSD,"%s\n",Tsk->Sp[(Tsk->Md[i1].ID_Sp)].Name);
	};
	// Óíèâåðñàëüíûé äåìîäóëÿòîð
	if(Tsk->Md[i1].ID_UD<Tsk->Max_UD)
	if(Tsk->UD[Tsk->Md[i1].ID_UD].Name[0]!=0)
	{
	fprintf(HFileSD,R_UD);
	fprintf(HFileSD,"[Md|UD]\t");
	fprintf(HFileSD,"%s\n",Tsk->UD[Tsk->Md[i1].ID_UD].Name);
	};

	for(i2=0;i2<Tsk->Max_Mt;i2++)
	   if(Tsk->Mt[i2].ID_Md==i1)
		{// Ìåòîä âõîäÿùèé â äàííûé ðåæèì
		DublM=0;
		for(size_t i99=0;i99<i2;i99++)
		   if(Tsk->Mt[i99].ID_Md==i1)//Ïðèíàäëåæèò ðåæèìó
		   if(strcmp(Tsk->Mt[i2].Name,Tsk->Mt[i99].Name)==0)DublM++;
		fprintf(HFileSD,";===============5===============\n");
		fprintf(HFileSD,R_Mt);
		fprintf(HFileSD,"[Md|Mt]\t");
		fprintf(HFileSD,"%d|%s\n",DublM,Tsk->Mt[i2].Name);
//		printf("%d|%s\n",DublM,Tsk->Mt[i2].Name);
		fprintf(HFileSD,"N_PO\t%d\n",Tsk->Mt[i2].N_PO);
		fprintf(HFileSD,"N_PI\t%d\n",Tsk->Mt[i2].N_PI);
        fprintf(HFileSD,"X_Mt\t%d\n",Tsk->Mt[i2].X);
        fprintf(HFileSD,"Y_Mt\t%d\n",Tsk->Mt[i2].Y);
		////	int	N_PO;
		for(i3=0;i3<Tsk->Max_Pr;i3++)
		   if(Tsk->Pr[i3].ID_Mt==i2)
			{// Ïàðàìåòðû ìåòîäà
			fprintf(HFileSD,R_Pr);
			fprintf(HFileSD,"[Md|Mt|Pr]\t");
			fprintf(HFileSD,"%s\n",Tsk->Pr[i3].Name);
//			fprintf(HFileSD,"ID_Mt\t%d\n",Tsk->Pr[i3].ID_Mt);
			if(Tsk->Pr[i3].Vol[0]==0)
				fprintf(HFileSD,"Vol\t#\n");
				else
				fprintf(HFileSD,"Vol\t%s\n",EncStr(Tsk->Pr[i3].Vol));
			};
		for(i3=0;i3<Tsk->Max_LP;i3++)
		   if(Tsk->LP[i3].ID_Mt2==i2)
			{// Ëèíêè ìåòîäà
//			fprintf(HFileSD,";---------------5---------------\n");
			fprintf(HFileSD,R_LP);
			fprintf(HFileSD,"[Md|Mt|LP]\n");
			fprintf(HFileSD,"PO\t%d\n",Tsk->LP[i3].PO);
			if(Tsk->LP[i3].ID_Mt1!=-1)
				{
				DublM0=0;
				for(size_t i99=0;i99<Tsk->LP[i3].ID_Mt1;i99++)
				   if(Tsk->Mt[i99].ID_Md==i1)//Ïðèíàäëåæèò ðåæèìó
				   if(strcmp(Tsk->Mt[Tsk->LP[i3].ID_Mt1].Name,Tsk->Mt[i99].Name)==0)DublM0++;
				fprintf(HFileSD,"Mt1\t%d|%s\n",DublM0,Tsk->Mt[Tsk->LP[i3].ID_Mt1].Name);
				}
				else fprintf(HFileSD,"Mt1\tInput\n");
			fprintf(HFileSD,"PI\t%d\n",Tsk->LP[i3].PI);
			if(Tsk->LP[i3].ID_Mt2!=-1)
				fprintf(HFileSD,"Mt2\t%d|%s\n",DublM,Tsk->Mt[i2].Name);
				else fprintf(HFileSD,"Mt1\t#\n");
			};

//************************************************************************************************

		// Âûõîäíûå íîãè
		for(i3=0;i3<Tsk->Max_LP;i3++)
		   if(Tsk->LP[i3].ID_Mt1==i2)
		   if(Tsk->LP[i3].ID_Mt2==-1)
			{
			fprintf(HFileSD,R_LPO);
			fprintf(HFileSD,"[Md|Mt|LP]\n");
			fprintf(HFileSD,"PO\t%d\n",Tsk->LP[i3].PO);
			if(Tsk->LP[i3].ID_Mt1!=-1)
				{
				DublM0=0;
				for(size_t i99=0;i99<Tsk->LP[i3].ID_Mt1;i99++)
				   if(Tsk->Mt[i99].ID_Md==i1)//Ïðèíàäëåæèò ðåæèìó
				   if(strcmp(Tsk->Mt[Tsk->LP[i3].ID_Mt1].Name,Tsk->Mt[i99].Name)==0)DublM0++;
				fprintf(HFileSD,"Mt1\t%d|%s\n",DublM0,Tsk->Mt[Tsk->LP[i3].ID_Mt1].Name);
				}
				else fprintf(HFileSD,"Mt1\tInput\n");
			fprintf(HFileSD,"PI\t%d\n",Tsk->LP[i3].PI);
			fprintf(HFileSD,"Mt2\tOutput\n");
			};

//************************************************************************************************

		};
	};//*Ðåæèì
//
//
//
//------------------------------------------6-----------------------------------
// Óñòðîéñòâî ñâÿçè
for(i1=0;i1<Tsk->Max_Un;i1++)
	{// Óñòðîéñòâî ñâÿçè
	fprintf(HFileSD,"\n\n;***************6***************\n");
	fprintf(HFileSD,R_Un);
	fprintf(HFileSD,"[Un]\t");
	fprintf(HFileSD,"%s\n",Tsk->Un[i1].Name);
	fprintf(HFileSD,"TypeConnect\t%d\n",Tsk->Un[(Tsk->TC[i3].ID_Un)].TypeConnect);
	for(size_t i2=0;i2<Tsk->Max_MdL;i2++)
		{if(Tsk->MdL[i2].ID_Un==i1)
			{//Ðåæèì
			fprintf(HFileSD,R_Md);
			fprintf(HFileSD,"[Un|Md]\t");
			fprintf(HFileSD,"%s\n",Tsk->Md[Tsk->MdL[i2].ID_Md].Name);
			};
		};
	};
//
//
//
//------------------------------------------7-----------------------------------
// Òàáëèöà ðàäèîñåòåé
for(i1=0;i1<Tsk->Max_RS;i1++)
	{// Òàáëèöà ðàäèîñåòåé
	fprintf(HFileSD,"\n\n;***************7***************\n");
	fprintf(HFileSD,R_RS);
	fprintf(HFileSD,"[RS]\t");
//	printf("%s\n",Tsk->RS[i1].Name);
	fprintf(HFileSD,"%s\n",Tsk->RS[i1].Name);
	fprintf(HFileSD,"Fl\t%d\n",Tsk->RS[i1].Fl);


	for(i2=0;i2<Tsk->Max_Ad;i2++)
	   if(Tsk->Ad[i2].ID_RS==i1)
		{// Àäðåññ âõîäÿùèé â äàííóþ ðàäèîñåòü
		fprintf(HFileSD,";===============7===============\n");
		fprintf(HFileSD,R_Ad);
		fprintf(HFileSD,"[RS|Ad]\t");
		fprintf(HFileSD,"%s|%s\n",Tsk->RS[i1].Name,Tsk->Ad[i2].Name);
		fprintf(HFileSD,"Azim\t%d\n",Tsk->Ad[i2].Azim);
		fprintf(HFileSD,"Fl\t%d\n",Tsk->Ad[i2].Fl);
		////	int	Azim;
		};

	for(i2=0;i2<Tsk->Max_Fr;i2++)
	   if(Tsk->Fr[i2].ID_RS==i1)
		{// ×àñòîòà âõîäÿùàÿ â äàííóþ ðàäèîñåòü
		fprintf(HFileSD,";===============7===============\n");
		fprintf(HFileSD,R_Fr);
		fprintf(HFileSD,"[RS|Fr]\t");
		fprintf(HFileSD,"%s|%d\n",Tsk->RS[i1].Name,Tsk->Fr[i2].F);
		fprintf(HFileSD,"Fl\t%d\n",Tsk->Fr[i2].Fl);

		};

	for(i2=0;i2<Tsk->Max_Pl;i2++)
	   if(Tsk->Pl[i2].ID_RS==i1)
		{// Ïóë âõîäÿùèé â äàííóþ ðàäèîñåòü
		fprintf(HFileSD,";===============7===============\n");
		fprintf(HFileSD,R_Pl);
		fprintf(HFileSD,"[RS|Pl]\t");
		fprintf(HFileSD,"%s|%s\n",Tsk->RS[i1].Name,Tsk->Pl[i2].Name);
		fprintf(HFileSD,"Fl\t%d\n",Tsk->Pl[i2].Fl);

		for(i3=0;i3<Tsk->Max_GrFr;i3++)
		   if(Tsk->GrFr[i3].ID_Pl==i2)
			{// ×àñòîòà âõîäÿùàÿ â äàííûé ïóë
//			fprintf(HFileSD,";---------------7---------------\n");
//			fprintf(HFileSD,R_Fr);
			fprintf(HFileSD,"[RS|Pl|Fr]\t");
			fprintf(HFileSD,"%d\n",Tsk->Fr[Tsk->GrFr[i3].ID_F].F);
			fprintf(HFileSD,"GrFr_Fl\t%d\n",Tsk->GrFr[i3].Fl);

			};
		};

	for(i2=0;i2<Tsk->Max_Se;i2++)
	   if(Tsk->Se[i2].ID_RS==i1)
		{// Ñåàíñû âõîäÿùèå â äàííóþ ðàäèîñåòü
		fprintf(HFileSD,";===============7===============\n");
		fprintf(HFileSD,R_Se);
		fprintf(HFileSD,"[RS|Se]\t");
		fprintf(HFileSD,"%s|%s\n",Tsk->RS[i1].Name,Tsk->Se[i2].Name);
		fprintf(HFileSD,"Fl\t%d\n",Tsk->Se[i2].Fl);
		fprintf(HFileSD,"ModeWork\t%d\n",Tsk->Se[i2].ModeWork);
		fprintf(HFileSD,";Prgn[240]\n");
		for(i3=0;i3<12;i3++)
		   {for(size_t i4=0;i4<20;i4++)fprintf(HFileSD,"%d\t",(int)Tsk->Se[i2].Prgn[i3*20+i4]);
		   fprintf(HFileSD,"\n");};
		fprintf(HFileSD,";Stat[240]\n");
		for(i3=0;i3<12;i3++)
		   {for(size_t i4=0;i4<20;i4++)fprintf(HFileSD,"%d\t",(int)Tsk->Se[i2].Stat[i3*20+i4]);
		   fprintf(HFileSD,"\n");};
		for(i3=0;i3<Tsk->Max_TC;i3++)
		   if(Tsk->TC[i3].ID_Se==i2)
			{// Òî÷êè ñâÿçè âõîäÿùèå â äàííûé ñåàíñ
			fprintf(HFileSD,";---------------7---------------\n");
			fprintf(HFileSD,R_TC);
			fprintf(HFileSD,"[RS|Se|TC]\t");
			fprintf(HFileSD,"%d\n",i3);
			fprintf(HFileSD,"Fl\t%d\n",Tsk->TC[i3].Fl);

			for(i4=0;i4<Tsk->Max_GrPl;i4++)
			   if(Tsk->GrPl[i4].ID_TC==i3)
				{// Ïóë âõîäÿùèé â äàííóþ ðàäèîñåòü
				fprintf(HFileSD,R_Pl);
				fprintf(HFileSD,"[RS|Se|TC|Pl]\t");
				fprintf(HFileSD,"%s\n",Tsk->Pl[Tsk->GrPl[i4].ID_Pl].Name);
				fprintf(HFileSD,"GrPl_Fl\t%d\n",Tsk->GrFr[i3].Fl);

				};
/*			// Ñåàíñ âõîäÿùèé â äàííóþ ðàäèîñåòü
			fprintf(HFileSD,R_Se);
			fprintf(HFileSD,"[RS|Se|TC|Se]\t");
			fprintf(HFileSD,"%s\n",Tsk->Se[(Tsk->TC[i3].ID_Se)].Name);*/
			// Àäðåññàò âõîäÿùèé â äàííóþ ðàäèîñåòü
			fprintf(HFileSD,R_Ad);
			fprintf(HFileSD,"[RS|Se|TC|Ad]\t");
			fprintf(HFileSD,"%s\n",Tsk->Ad[(Tsk->TC[i3].ID_Ad)].Name);
			// Þíèò âõîäÿùèé â äàííóþ ðàäèîñåòü
//			fprintf(HFileSD,";===============7===============\n");
			fprintf(HFileSD,R_Un);
			fprintf(HFileSD,"[RS|Se|TC|Un]\t");
			fprintf(HFileSD,"%s\n",Tsk->Un[(Tsk->TC[i3].ID_Un)].Name);
			};
		};
	};//Òàáëèöà ðàäèîñåòåé

fprintf(HFileSD,EndFile);
fclose(HFileSD);
return;
};





//*********************************************************
void		DataImEx::ImpTsk(Task* Tsk,char *PathName,char *NewMdName)
{
/*
int	Nom_RS=1;//Ïîðÿäêîâûé íîìåð ðàäèîñåòè
int	Nom_Ad=1;//Ïîðÿäêîâûé íîìåð Àäðåññàòà
int	Nom_Pl=1;//Ïîðÿäêîâûé íîìåð ïóëà ÷àñòîò
int	Nom_Fr=1;//Ïîðÿäêîâûé íîìåð ÷àñòîòû
int	Nom_Se=1;//Ïîðÿäêîâûé íîìåð ÷àñòîòû
int	Nom_TC=1;//Ïîðÿäêîâûé íîìåð òî÷êè ñâÿçè
int	Nom_Md=1;//Ïîðÿäêîâûé íîìåð ðåæèìà
int	Nom_En=1;//Ïîðÿäêîâûé íîìåð ýí.êðèòåðèÿ
int	Nom_Sp=1;//Ïîðÿäêîâûé íîìåð ñï.êðèòåðèÿ
int	Nom_UD=1;//Ïîðÿäêîâûé íîìåð óí.äåìîäóëÿòîðà
int	Nom_Mt=1;//Ïîðÿäêîâûé íîìåð Ìåòîäà
int	Nom_Pr=1;//Ïîðÿäêîâûé íîìåð ïàðàìåòðà
int	Nom_BMt=1;//Ïîðÿäêîâûé íîìåð á.ìåòîäà
int	Nom_BIn=1;//Ïîðÿäêîâûé íîìåð âõ.íîæêè á.ìåò.
int	Nom_BOut=1;//Ïîðÿäêîâûé íîìåð âûõ.íîæêè á.ìåò.
int	Nom_BPr=1;//Ïîðÿäêîâûé íîìåð ïàðàìåòðà á.ìåò.
int	Nom_Un=1;//Ïîðÿäêîâûé íîìåð óñòð.ñâÿçè
int	Nom_MdL=1;//
int	N_Pl=0;
//int	N_Se=0;
int	N_Ad=0;
//int	N_Un=0;
//int	N_UnMd=0;
int	N_Mt1_2=0;

int	Nom_GrFr=1;//
int	Nom_GrPl=1;//
*/

int	Nom_RS=0;//Ïîðÿäêîâûé íîìåð ðàäèîñåòè
int	Nom_Ad=0;//Ïîðÿäêîâûé íîìåð Àäðåññàòà
int	Nom_Pl=0;//Ïîðÿäêîâûé íîìåð ïóëà ÷àñòîò
int	Nom_Fr=0;//Ïîðÿäêîâûé íîìåð ÷àñòîòû
int	Nom_Se=0;//Ïîðÿäêîâûé íîìåð ÷àñòîòû
int	Nom_TC=0;//Ïîðÿäêîâûé íîìåð òî÷êè ñâÿçè
int	Nom_Md=0;//Ïîðÿäêîâûé íîìåð ðåæèìà
int	Nom_En=0;//Ïîðÿäêîâûé íîìåð ýí.êðèòåðèÿ
int	Nom_Sp=0;//Ïîðÿäêîâûé íîìåð ñï.êðèòåðèÿ
int	Nom_UD=0;//Ïîðÿäêîâûé íîìåð óí.äåìîäóëÿòîðà
int	Nom_Mt=0;//Ïîðÿäêîâûé íîìåð Ìåòîäà
int	Nom_Pr=0;//Ïîðÿäêîâûé íîìåð ïàðàìåòðà
int	Nom_BMt=0;//Ïîðÿäêîâûé íîìåð á.ìåòîäà
int	Nom_BIn=0;//Ïîðÿäêîâûé íîìåð âõ.íîæêè á.ìåò.
int	Nom_BOut=0;//Ïîðÿäêîâûé íîìåð âûõ.íîæêè á.ìåò.
int	Nom_BPr=0;//Ïîðÿäêîâûé íîìåð ïàðàìåòðà á.ìåò.
int	Nom_Un=0;//Ïîðÿäêîâûé íîìåð óñòð.ñâÿçè
int	Nom_MdL=0;//
int	N_Pl=0;
//int	N_Se=0;
int	N_Ad=0;
//int	N_Un=0;
//int	N_UnMd=0;
int	N_Mt1_2=0;

int	Nom_GrFr=0;//
int	Nom_GrPl=0;//
int	Fl_ReadMd=0;//
FILE 	*HFileRD_Md=NULL;//

OutName[0]=Name1[0];OutName[1]=Name1[1];
char*    PReadStr=ReadStr;




//FILE 	*HFileRD=fopen("Data000.tsk","rt");
FILE 	*HFileRD=fopen(PathName,"rt");
if(HFileRD==NULL) return;


if(NewMdName!=NULL)
	{HFileRD_Md=fopen(NewMdName,"rt");
	if(HFileRD!=NULL)Fl_ReadMd=1;else Fl_ReadMd=0;
	}else Fl_ReadMd=0;
//--------------------------------------------------------


if(ReadCFG(HFileRD,PReadStr)==-1) goto _EndCFGRead;
_Mt0:
//------------------------------------1---------------------------------
if(strcmp(ReadStr,"[BM]")==0)
	{
//	Tsk_BMeth tbm=Tsk->BMt[Nom_BMt];
	if(ReadCFG(HFileRD,PReadStr)==-1) goto _EndCFGRead;
	strcpy(Tsk->BMt[Nom_BMt].Name,PReadStr);//name
	if(ReadCFG(HFileRD,PReadStr)==-1) goto _EndCFGRead;
	if(strcmp(ReadStr,"N_PI")==0){fscanf(HFileRD,"%s",Temp_Ch);Tsk->BMt[Nom_BMt].N_PI=atoi(Temp_Ch);};
	if(ReadCFG(HFileRD,PReadStr)==-1) goto _EndCFGRead;
	if(strcmp(ReadStr,"N_PO")==0){fscanf(HFileRD,"%s",Temp_Ch);Tsk->BMt[Nom_BMt].N_PO=atoi(Temp_Ch);};
      //if(ReadCFG(HFileRD,PReadStr)==-1) goto _EndCFGRead;
      //	if(strcmp(ReadStr,"X_Mt")==0){fscanf(HFileRD,"%s",Temp_Ch);Tsk->BMt[Nom_BMt].X=atoi(Temp_Ch);};
      //  if(ReadCFG(HFileRD,PReadStr)==-1) goto _EndCFGRead;
      //	if(strcmp(ReadStr,"Y_Mt")==0){fscanf(HFileRD,"%s",Temp_Ch);Tsk->BMt[Nom_BMt].Y=atoi(Temp_Ch);};
	//
	if(ReadCFG(HFileRD,PReadStr)==-1) goto _EndCFGRead;
	_Mt3:
	if(strcmp(ReadStr,"[BM|BI]")==0)
		{
		Tsk->BIn[Nom_BIn].ID_BMt=Nom_BMt;
		if(ReadCFG(HFileRD,PReadStr)==-1) goto _EndCFGRead;
		strcpy(Tsk->BIn[Nom_BIn].Name,PReadStr);
		//
//		if(ReadCFG(HFileRD,PReadStr)==-1) goto _EndCFGRead;
//		if(strcmp(PReadStr,"Rem")==0)
//			{while(PReadStr[0]!='{')
//				fgets(PReadStr,2,HFileRD);
//			int	PoiChar1=0;
//			_Get_Rem:
//			fgets((char*)(PReadStr+PoiChar1),2,HFileRD);
//			if(PReadStr[PoiChar1]=='}')
//				PReadStr[PoiChar1]=0;
//				else
//				goto _Get_Rem;
//			strcpy(Tsk->BIn[Nom_BIn].Rem,PReadStr);
//			};
		//
		if(ReadCFG(HFileRD,PReadStr)==-1) goto _EndCFGRead;
		Nom_BIn++;
		goto _Mt3;
		};
	if(strcmp(ReadStr,"[BM|BO]")==0)
		{
		Tsk->BOut[Nom_BOut].ID_BMt=Nom_BMt;
		if(ReadCFG(HFileRD,PReadStr)==-1) goto _EndCFGRead;
		strcpy(Tsk->BOut[Nom_BOut].Name,PReadStr);
		//
//		if(ReadCFG(HFileRD,PReadStr)==-1) goto _EndCFGRead;
//		if(strcmp(ReadStr,"Rem")==0)
//			{while(PReadStr[0]!='{')
//				fgets(PReadStr,2,HFileRD);
//			int	PoiChar1=0;
//			_Get_Rem1:
//			fgets((char*)(PReadStr+PoiChar1),2,HFileRD);
//			if(PReadStr[PoiChar1]=='}')
//				PReadStr[PoiChar1]=0;
//				else
//				goto _Get_Rem1;
//			strcpy(Tsk->BOut[Nom_BOut].Rem,PReadStr);
//			};
		//
		if(ReadCFG(HFileRD,PReadStr)==-1) goto _EndCFGRead;
		Nom_BOut++;
		goto _Mt3;
		};
	if(strcmp(ReadStr,"[BM|BP]")==0)
		{
		Tsk->BPr[Nom_BPr].ID_BMt=Nom_BMt;
		if(ReadCFG(HFileRD,PReadStr)==-1) goto _EndCFGRead;
		strcpy(Tsk->BPr[Nom_BPr].Name,PReadStr);
		if(ReadCFG(HFileRD,PReadStr)==-1) goto _EndCFGRead;
		if(strcmp(ReadStr,"Vol")==0)
			{//fscanf(HFileRD,"%s",PReadStr);
			ReadCFG(HFileRD,PReadStr);
			if(strcmp(PReadStr,"#")==0)
				Tsk->BPr[Nom_BPr].Vol[0]=0;
				else strcpy(Tsk->BPr[Nom_BPr].Vol,PReadStr);
			};

//		fscanf(HFileRD,"%s",Tsk->BPr[Nom_BPr].Vol);
//		if(Tsk->BPr[Nom_BPr].Vol[0]=='#')Tsk->BPr[Nom_BPr].Vol[0]=0;
		//
		if(ReadCFG(HFileRD,PReadStr)==-1) goto _EndCFGRead;
		Nom_BPr++;
		goto _Mt3;
		};
	Nom_BMt++;
	goto _Mt0;
	};
//
//
//
//------------------------------2---------------------------------------
//Ýíåðãåòè÷. êðèòåðèé
if(strcmp(ReadStr,"[En]")==0)
	{//Ýíåðãåòè÷. êðèòåðèé
	if(ReadCFG(HFileRD,PReadStr)==-1) goto _EndCFGRead;
	strcpy(Tsk->En[Nom_En].Name,PReadStr);
	if(ReadCFG(HFileRD,PReadStr)==-1) goto _EndCFGRead;
	if(strcmp(ReadStr,"Ep")==0){fscanf(HFileRD,"%s",Temp_Ch);Tsk->En[Nom_En].Ep=atoi(Temp_Ch);};
	if(ReadCFG(HFileRD,PReadStr)==-1) goto _EndCFGRead;
	Nom_En++;
	goto _Mt0;
	};
//
//
//
//------------------------------3---------------------------------------
//Ñïåêòðàëüíûé êðèòåðèé
if(strcmp(ReadStr,"[Sp]")==0)
	{
	if(ReadCFG(HFileRD,PReadStr)==-1) goto _EndCFGRead;
	strcpy(Tsk->Sp[Nom_Sp].Name,PReadStr);
	if(ReadCFG(HFileRD,PReadStr)==-1) goto _EndCFGRead;
	while(ReadStr[0]!='[')
		{//Eps
		if(strcmp(ReadStr,"Eps")==0){fscanf(HFileRD,"%s",Temp_Ch);Tsk->Sp[Nom_Sp].Eps=atoi(Temp_Ch);};
		//dEps
		if(strcmp(ReadStr,"dEps")==0){fscanf(HFileRD,"%s",Temp_Ch);Tsk->Sp[Nom_Sp].dEps=atoi(Temp_Ch);};
		//Kosr
		if(strcmp(ReadStr,"Kosr")==0){fscanf(HFileRD,"%s",Temp_Ch);Tsk->Sp[Nom_Sp].Kosr=atoi(Temp_Ch);};
		//Porog
		if(strcmp(ReadStr,"Porog")==0){fscanf(HFileRD,"%s",Temp_Ch);Tsk->Sp[Nom_Sp].Porog=atoi(Temp_Ch);};
		//Ep
		if(strcmp(ReadStr,"Ep")==0){fscanf(HFileRD,"%s",Temp_Ch);Tsk->Sp[Nom_Sp].Ep=atoi(Temp_Ch);};
		//Spe[128]
		if(strcmp(ReadStr,"Spe")==0)
			for(size_t i99=0;i99<128;i99++){fscanf(HFileRD,"%s",Temp_Ch);Tsk->Sp[Nom_Sp].Spe[i99]=atoi(Temp_Ch);};
		if(ReadCFG(HFileRD,PReadStr)==-1) goto _EndCFGRead;
		};
	Nom_Sp++;
	goto _Mt0;
	};
//
//
//
//----------------------------------------------------
//Óíèâåðñàëüíûé äåìîäóëÿòîð
if(strcmp(ReadStr,"[UD]")==0)
	{
	//
	if(ReadCFG(HFileRD,PReadStr)==-1) goto _EndCFGRead;
	strcpy(Tsk->UD[Nom_UD].Name,PReadStr);
	if(ReadCFG(HFileRD,PReadStr)==-1) goto _EndCFGRead;
	while(ReadStr[0]!='[')
		{//Mod
		if(strcmp(ReadStr,"Mod")==0){fscanf(HFileRD,"%s",Temp_Ch);Tsk->UD[Nom_UD].Mod=atoi(Temp_Ch);goto _WhileCont_UD;};
		//Star
		if(strcmp(ReadStr,"Star")==0){fscanf(HFileRD,"%s",Temp_Ch);Tsk->UD[Nom_UD].Star=atoi(Temp_Ch);goto _WhileCont_UD;};
		//TypeTrnsm
		if(strcmp(ReadStr,"TypeTrnsm")==0){fscanf(HFileRD,"%s",Temp_Ch);Tsk->UD[Nom_UD].TypeTrnsm=atoi(Temp_Ch);goto _WhileCont_UD;};
		//Pilot
		if(strcmp(ReadStr,"Pilot")==0){fscanf(HFileRD,"%s",Temp_Ch);Tsk->UD[Nom_UD].Pilot=atoi(Temp_Ch);goto _WhileCont_UD;};
		//SynchP[30];
		if(strcmp(ReadStr,"SynchP")==0)
		for(k0=0;k0<30;k0++)
		   {fscanf(HFileRD,"%s",Temp_Ch);Tsk->UD[Nom_UD].SynchP[k0]=atoi(Temp_Ch);goto _WhileCont_UD;};
		//Synch
		if(strcmp(ReadStr,"Synch")==0){fscanf(HFileRD,"%s",Temp_Ch);Tsk->UD[Nom_UD].Synch=atoi(Temp_Ch);goto _WhileCont_UD;};
		//SS_Inf
		if(strcmp(ReadStr,"SS_Inf")==0){fscanf(HFileRD,"%s",Temp_Ch);Tsk->UD[Nom_UD].SS_Inf=atoi(Temp_Ch);goto _WhileCont_UD;};
		//SS_Stop
		if(strcmp(ReadStr,"SS_Stop")==0){fscanf(HFileRD,"%s",Temp_Ch);Tsk->UD[Nom_UD].SS_Stop=atoi(Temp_Ch);goto _WhileCont_UD;};
		//Bl_T
		if(strcmp(ReadStr,"Bl_T")==0){fscanf(HFileRD,"%s",Temp_Ch);Tsk->UD[Nom_UD].Bl_T=atoi(Temp_Ch);goto _WhileCont_UD;};
		//Bl_D
		if(strcmp(ReadStr,"Bl_D")==0){fscanf(HFileRD,"%s",Temp_Ch);Tsk->UD[Nom_UD].Bl_D=atoi(Temp_Ch);goto _WhileCont_UD;};
		//f De_Fn
		if(strcmp(ReadStr,"De_Fn")==0){fscanf(HFileRD,"%s",Temp_Ch);Tsk->UD[Nom_UD].De_Fn=atof(Temp_Ch);goto _WhileCont_UD;};
		//f De_dFn
		if(strcmp(ReadStr,"De_dFn")==0){fscanf(HFileRD,"%s",Temp_Ch);Tsk->UD[Nom_UD].De_dFn=atof(Temp_Ch);goto _WhileCont_UD;};
		//f De_Fb
		if(strcmp(ReadStr,"De_Fb")==0){fscanf(HFileRD,"%s",Temp_Ch);Tsk->UD[Nom_UD].De_Fb=atof(Temp_Ch);goto _WhileCont_UD;};
		//f De_dFb
		if(strcmp(ReadStr,"De_dFb")==0){fscanf(HFileRD,"%s",Temp_Ch);Tsk->UD[Nom_UD].De_dFb=atof(Temp_Ch);goto _WhileCont_UD;};
		//f De_Fr
		if(strcmp(ReadStr,"De_Fr")==0){fscanf(HFileRD,"%s",Temp_Ch);Tsk->UD[Nom_UD].De_Fr=atof(Temp_Ch);goto _WhileCont_UD;};
		//f De_dFr
		if(strcmp(ReadStr,"De_dFr")==0){fscanf(HFileRD,"%s",Temp_Ch);Tsk->UD[Nom_UD].De_dFr=atof(Temp_Ch);goto _WhileCont_UD;};
		//f De_Fch
		if(strcmp(ReadStr,"De_Fch")==0){fscanf(HFileRD,"%s",Temp_Ch);Tsk->UD[Nom_UD].De_Fch=atof(Temp_Ch);goto _WhileCont_UD;};
		//f De_dFch
		if(strcmp(ReadStr,"De_dFch")==0){fscanf(HFileRD,"%s",Temp_Ch);Tsk->UD[Nom_UD].De_dFch=atof(Temp_Ch);goto _WhileCont_UD;};
		//f De_Fp
		if(strcmp(ReadStr,"De_Fp")==0){fscanf(HFileRD,"%s",Temp_Ch);Tsk->UD[Nom_UD].De_Fp=atof(Temp_Ch);goto _WhileCont_UD;};
		//f De_dFp
		if(strcmp(ReadStr,"De_dFp")==0){fscanf(HFileRD,"%s",Temp_Ch);Tsk->UD[Nom_UD].De_dFp=atof(Temp_Ch);goto _WhileCont_UD;};
		//De_Invert
		if(strcmp(ReadStr,"De_Invert")==0){fscanf(HFileRD,"%s",Temp_Ch);Tsk->UD[Nom_UD].De_Invert=atoi(Temp_Ch);goto _WhileCont_UD;};
		//f De_Kaea
		if(strcmp(ReadStr,"De_Kaea")==0){fscanf(HFileRD,"%s",Temp_Ch);Tsk->UD[Nom_UD].De_Kaea=atof(Temp_Ch);goto _WhileCont_UD;};
		//f De_Kaen
		if(strcmp(ReadStr,"De_Kaen")==0){fscanf(HFileRD,"%s",Temp_Ch);Tsk->UD[Nom_UD].De_Kaen=atof(Temp_Ch);goto _WhileCont_UD;};
		//f De_Kaee
		if(strcmp(ReadStr,"De_Kaee")==0){fscanf(HFileRD,"%s",Temp_Ch);Tsk->UD[Nom_UD].De_Kaee=atof(Temp_Ch);goto _WhileCont_UD;};
		//f De_K1
		if(strcmp(ReadStr,"De_K1")==0){fscanf(HFileRD,"%s",Temp_Ch);Tsk->UD[Nom_UD].De_K1=atof(Temp_Ch);goto _WhileCont_UD;};
		//f De_K2
		if(strcmp(ReadStr,"De_K2")==0){fscanf(HFileRD,"%s",Temp_Ch);Tsk->UD[Nom_UD].De_K2=atof(Temp_Ch);goto _WhileCont_UD;};
		//f De_K21
		if(strcmp(ReadStr,"De_K21")==0){fscanf(HFileRD,"%s",Temp_Ch);Tsk->UD[Nom_UD].De_K21=atof(Temp_Ch);goto _WhileCont_UD;};
		//f De_K22
		if(strcmp(ReadStr,"De_K22")==0){fscanf(HFileRD,"%s",Temp_Ch);Tsk->UD[Nom_UD].De_K22=atof(Temp_Ch);goto _WhileCont_UD;};
		//f De_porog
		if(strcmp(ReadStr,"De_porog")==0){fscanf(HFileRD,"%s",Temp_Ch);Tsk->UD[Nom_UD].De_porog=atof(Temp_Ch);goto _WhileCont_UD;};
		//f De_porog_1
		if(strcmp(ReadStr,"De_porog_1")==0){fscanf(HFileRD,"%s",Temp_Ch);Tsk->UD[Nom_UD].De_porog_1=atof(Temp_Ch);goto _WhileCont_UD;};
		//f De_CAgc
		if(strcmp(ReadStr,"De_CAgc")==0){fscanf(HFileRD,"%s",Temp_Ch);Tsk->UD[Nom_UD].De_CAgc=atof(Temp_Ch);goto _WhileCont_UD;};
		//f De_CAgc2
		if(strcmp(ReadStr,"De_CAgc2")==0){fscanf(HFileRD,"%s",Temp_Ch);Tsk->UD[Nom_UD].De_CAgc2=atof(Temp_Ch);goto _WhileCont_UD;};
		//f De_CAgcMin
		if(strcmp(ReadStr,"De_CAgcMin")==0){fscanf(HFileRD,"%s",Temp_Ch);Tsk->UD[Nom_UD].De_CAgcMin=atof(Temp_Ch);goto _WhileCont_UD;};
		//f De_CAgcMax
		if(strcmp(ReadStr,"De_CAgcMax")==0){fscanf(HFileRD,"%s",Temp_Ch);Tsk->UD[Nom_UD].De_CAgcMax=atof(Temp_Ch);goto _WhileCont_UD;};
		//f De_Enp
		if(strcmp(ReadStr,"De_Enp")==0){fscanf(HFileRD,"%s",Temp_Ch);Tsk->UD[Nom_UD].De_Enp=atof(Temp_Ch);goto _WhileCont_UD;};
		//f De_Enp1
		if(strcmp(ReadStr,"De_Enp1")==0){fscanf(HFileRD,"%s",Temp_Ch);Tsk->UD[Nom_UD].De_Enp1=atof(Temp_Ch);goto _WhileCont_UD;};
		//f De_Enp2
		if(strcmp(ReadStr,"De_Enp2")==0){fscanf(HFileRD,"%s",Temp_Ch);Tsk->UD[Nom_UD].De_Enp2=atof(Temp_Ch);goto _WhileCont_UD;};
		//f De_Enp3
		if(strcmp(ReadStr,"De_Enp3")==0){fscanf(HFileRD,"%s",Temp_Ch);Tsk->UD[Nom_UD].De_Enp3=atof(Temp_Ch);goto _WhileCont_UD;};
		//De_Ka1
		if(strcmp(ReadStr,"De_Ka1")==0){fscanf(HFileRD,"%s",Temp_Ch);Tsk->UD[Nom_UD].De_Ka1=atoi(Temp_Ch);goto _WhileCont_UD;};
		//De_Ka2
		if(strcmp(ReadStr,"De_Ka2")==0){fscanf(HFileRD,"%s",Temp_Ch);Tsk->UD[Nom_UD].De_Ka2=atoi(Temp_Ch);goto _WhileCont_UD;};
		//f De_Ke
		if(strcmp(ReadStr,"De_Ke")==0){fscanf(HFileRD,"%s",Temp_Ch);Tsk->UD[Nom_UD].De_Ke=atof(Temp_Ch);goto _WhileCont_UD;};
		//f De_Km
		if(strcmp(ReadStr,"De_Km")==0){fscanf(HFileRD,"%s",Temp_Ch);Tsk->UD[Nom_UD].De_Km=atof(Temp_Ch);goto _WhileCont_UD;};
		//f De_Ksta
		if(strcmp(ReadStr,"De_Ksta")==0){fscanf(HFileRD,"%s",Temp_Ch);Tsk->UD[Nom_UD].De_Ksta=atof(Temp_Ch);goto _WhileCont_UD;};
		//f De_Kstn
		if(strcmp(ReadStr,"De_Kstn")==0){fscanf(HFileRD,"%s",Temp_Ch);Tsk->UD[Nom_UD].De_Kstn=atof(Temp_Ch);};
		//
		//De_PersP;
		if(strcmp(ReadStr,"De_PersP")==0){fscanf(HFileRD,"%s",Temp_Ch);Tsk->UD[Nom_UD].De_PersP=atoi(Temp_Ch);};
		//f De_F1;
		if(strcmp(ReadStr,"De_F1")==0){fscanf(HFileRD,"%s",Temp_Ch);Tsk->UD[Nom_UD].De_F1=atof(Temp_Ch);};
		//f De_F2;
		if(strcmp(ReadStr,"De_F2")==0){fscanf(HFileRD,"%s",Temp_Ch);Tsk->UD[Nom_UD].De_F2=atof(Temp_Ch);};
		//f De_F3;
		if(strcmp(ReadStr,"De_F3")==0){fscanf(HFileRD,"%s",Temp_Ch);Tsk->UD[Nom_UD].De_F3=atof(Temp_Ch);};
		//f De_F4;
		if(strcmp(ReadStr,"De_F4")==0){fscanf(HFileRD,"%s",Temp_Ch);Tsk->UD[Nom_UD].De_F4=atof(Temp_Ch);};
		//f De_dF;
		if(strcmp(ReadStr,"De_dF")==0){fscanf(HFileRD,"%s",Temp_Ch);Tsk->UD[Nom_UD].De_dF=atof(Temp_Ch);};
		//
		//Íîâûå äàííûå
_WhileCont_UD:	if(ReadCFG(HFileRD,PReadStr)==-1) goto _EndCFGRead;
		};
	Nom_UD++;
	goto _Mt0;
	};
//
//
//
//------------------------------5---------------------------------------
//------------------------------5---------------------------------------
//Ðåæèì
if(strcmp(ReadStr,"[Md]")==0)
	{
	if(ReadCFG(HFileRD,PReadStr)==-1) goto _EndCFGRead;
	strcpy(Tsk->Md[Nom_Md].Name,PReadStr);
	if(ReadCFG(HFileRD,PReadStr)==-1) goto _EndCFGRead;
	if(strcmp(ReadStr,"MethObn")==0){fscanf(HFileRD,"%s",Temp_Ch);Tsk->Md[Nom_Md].MethObn=atoi(Temp_Ch);};
	if(ReadCFG(HFileRD,PReadStr)==-1) goto _EndCFGRead;
	if(strcmp(ReadStr,"TimeProb")==0){fscanf(HFileRD,"%s",Temp_Ch);Tsk->Md[Nom_Md].TimeProb=atoi(Temp_Ch);};
	//
	if(ReadCFG(HFileRD,PReadStr)==-1) goto _EndCFGRead;
   	if(strcmp(ReadStr,"X_Md")==0)
    {
		fscanf(HFileRD,"%s",Temp_Ch);Tsk->Md[Nom_Md].X=atoi(Temp_Ch);
        if(ReadCFG(HFileRD,PReadStr)==-1) goto _EndCFGRead;
	    if(strcmp(ReadStr,"Y_Md")==0){fscanf(HFileRD,"%s",Temp_Ch);Tsk->Md[Nom_Md].Y=atoi(Temp_Ch);}
        if(ReadCFG(HFileRD,PReadStr)==-1) goto _EndCFGRead;
    }
	_Mt2:
	if(strcmp(ReadStr,"[Md|En]")==0)
		{
		if(ReadCFG(HFileRD,PReadStr)==-1) goto _EndCFGRead;
		for(int i99=0;i99<Nom_En;i99++)
			if(strcmp(Tsk->En[i99].Name,PReadStr)==0)Tsk->Md[Nom_Md].ID_En=i99;
		if(ReadCFG(HFileRD,PReadStr)==-1) goto _EndCFGRead;
		goto	_Mt2;
		};
	if(strcmp(ReadStr,"[Md|Sp]")==0)
		{
		if(ReadCFG(HFileRD,PReadStr)==-1) goto _EndCFGRead;
		for(int i99=0;i99<Nom_Sp;i99++)
			if(strcmp(Tsk->Sp[i99].Name,PReadStr)==0)Tsk->Md[Nom_Md].ID_Sp=i99;
		if(ReadCFG(HFileRD,PReadStr)==-1) goto _EndCFGRead;
		goto	_Mt2;
		};
	if(strcmp(ReadStr,"[Md|UD]")==0)
		{
		if(ReadCFG(HFileRD,PReadStr)==-1) goto _EndCFGRead;
		for(int i99=0;i99<Nom_UD;i99++)
			if(strcmp(Tsk->UD[i99].Name,PReadStr)==0)Tsk->Md[Nom_Md].ID_UD=i99;
		if(ReadCFG(HFileRD,PReadStr)==-1) goto _EndCFGRead;
		goto	_Mt2;
		};
	//Ìåòîä
	if(strcmp(ReadStr,"[Md|Mt]")==0)
		{//Ìåòîä
		if(ReadCFG(HFileRD,PReadStr)==-1) goto _EndCFGRead;
		strcpy(Tsk->Mt[Nom_Mt].Name,ReadName(PReadStr)[1]);//name
		DublMt[Nom_Mt]=atoi(ReadName(PReadStr)[0]);
		Tsk->Mt[Nom_Mt].Ext=DublMt[Nom_Mt];
		//
		Tsk->Mt[Nom_Mt].ID_Md=Nom_Md;//
		if(ReadCFG(HFileRD,PReadStr)==-1) goto _EndCFGRead;
		if(strcmp(ReadStr,"N_PO")==0){fscanf(HFileRD,"%s",Temp_Ch);Tsk->Mt[Nom_Mt].N_PO=atoi(Temp_Ch);};
		if(ReadCFG(HFileRD,PReadStr)==-1) goto _EndCFGRead;
		if(strcmp(ReadStr,"N_PI")==0){fscanf(HFileRD,"%s",Temp_Ch);Tsk->Mt[Nom_Mt].N_PI=atoi(Temp_Ch);};
        if(ReadCFG(HFileRD,PReadStr)==-1) goto _EndCFGRead;
	   	if(strcmp(ReadStr,"X_Mt")==0)
        {
			fscanf(HFileRD,"%s",Temp_Ch);Tsk->Mt[Nom_Mt].X=atoi(Temp_Ch);
            if(ReadCFG(HFileRD,PReadStr)==-1) goto _EndCFGRead;
	     	if(strcmp(ReadStr,"Y_Mt")==0){fscanf(HFileRD,"%s",Temp_Ch);Tsk->Mt[Nom_Mt].Y=atoi(Temp_Ch);}
            if(ReadCFG(HFileRD,PReadStr)==-1) goto _EndCFGRead;
        }
                //if(ReadCFG(HFileRD,PReadStr)==-1) goto _EndCFGRead;
		_Mt8:
		if(strcmp(ReadStr,"[Md|Mt|Pr]")==0)
			{
			Tsk->Pr[Nom_Pr].ID_Mt=Nom_Mt;
			if(ReadCFG(HFileRD,PReadStr)==-1) goto _EndCFGRead;
			strcpy(Tsk->Pr[Nom_Pr].Name,PReadStr);
			if(ReadCFG(HFileRD,PReadStr)==-1) goto _EndCFGRead;
			if(strcmp(PReadStr,"Vol")==0)
				{//fscanf(HFileRD,"%s",PReadStr);
				ReadCFG(HFileRD,PReadStr);
				if(strcmp(PReadStr,"#")==0)
					Tsk->Pr[Nom_Pr].Vol[0]=0;
					else strcpy(Tsk->Pr[Nom_Pr].Vol,PReadStr);

				};
			//
			if(ReadCFG(HFileRD,PReadStr)==-1) goto _EndCFGRead;
			Nom_Pr++;
			goto _Mt8;
			};
		if(strcmp(ReadStr,"[Md|Mt|LP]")==0)
			{
			//Çàïîëíåíèå ñòðóêòóðû  Mt1_2[xx]
			Mt1_2[N_Mt1_2].ID_MD=Nom_Md;
			if(ReadCFG(HFileRD,PReadStr)==-1) goto _EndCFGRead;
			while(ReadStr[0]!='[')
				{if(strcmp(ReadStr,"PO")==0){fscanf(HFileRD,"%s",Temp_Ch);Mt1_2[N_Mt1_2].PO=atoi(Temp_Ch);};
				if(strcmp(ReadStr,"Mt1")==0) fscanf(HFileRD,"%s",Mt1_2[N_Mt1_2].Mt1);
				if(strcmp(ReadStr,"PI")==0) {fscanf(HFileRD,"%s",Temp_Ch);Mt1_2[N_Mt1_2].PI=atoi(Temp_Ch);};
				if(strcmp(ReadStr,"Mt2")==0) fscanf(HFileRD,"%s",Mt1_2[N_Mt1_2].Mt2);
				if(ReadCFG(HFileRD,PReadStr)==-1) goto _EndCFGRead;
				};
			N_Mt1_2++;
			goto _Mt8;
			};
		Nom_Mt++;
		goto _Mt2;
		};
	//
	Nom_Md++;
	goto _Mt0;
	};
//
//×òåíèå íîâîãî ðåæèìà èç ôàéëà NewMdName
if(Fl_ReadMd==1)
  {Fl_ReadMd=0;
  strcpy(ReadStrTemp,ReadStr);
//******************************
//******************************HFileRD_Md
if(ReadCFG(HFileRD_Md,PReadStr)==-1){fclose(HFileRD_Md);goto _ContMdRead;};
if(strcmp(ReadStr,"[Md]")==0)
	{
	if(ReadCFG(HFileRD_Md,PReadStr)==-1){fclose(HFileRD_Md);goto _ContMdRead;};
	strcpy(Tsk->Md[Nom_Md].Name,PReadStr);
	if(ReadCFG(HFileRD_Md,PReadStr)==-1){fclose(HFileRD_Md);goto _ContMdRead;};
	if(strcmp(ReadStr,"MethObn")==0){fscanf(HFileRD_Md,"%s",Temp_Ch);Tsk->Md[Nom_Md].MethObn=atoi(Temp_Ch);};
	if(ReadCFG(HFileRD_Md,PReadStr)==-1){fclose(HFileRD_Md);goto _ContMdRead;};
	if(strcmp(ReadStr,"TimeProb")==0){fscanf(HFileRD_Md,"%s",Temp_Ch);Tsk->Md[Nom_Md].TimeProb=atoi(Temp_Ch);};
	//
	if(ReadCFG(HFileRD_Md,PReadStr)==-1){fclose(HFileRD_Md);goto _ContMdRead;};
	if(strcmp(ReadStr,"X_Md")==0)
    {
		fscanf(HFileRD_Md,"%s",Temp_Ch);Tsk->Md[Nom_Md].X=atoi(Temp_Ch);
        if(ReadCFG(HFileRD_Md,PReadStr)==-1) goto _EndCFGRead;
	    if(strcmp(ReadStr,"Y_Md")==0){fscanf(HFileRD_Md,"%s",Temp_Ch);Tsk->Md[Nom_Md].Y=atoi(Temp_Ch);}
        if(ReadCFG(HFileRD_Md,PReadStr)==-1) goto _EndCFGRead;
    }
	_Mt2_Md:
	if(strcmp(ReadStr,"[Md|En]")==0)
		{
		if(ReadCFG(HFileRD_Md,PReadStr)==-1){fclose(HFileRD_Md);goto _ContMdRead;};
		for(int i99=0;i99<Nom_En;i99++)
			if(strcmp(Tsk->En[i99].Name,PReadStr)==0)Tsk->Md[Nom_Md].ID_En=i99;
		if(ReadCFG(HFileRD_Md,PReadStr)==-1){fclose(HFileRD_Md);goto _ContMdRead;};
		goto	_Mt2_Md;
		};
	if(strcmp(ReadStr,"[Md|Sp]")==0)
		{
		if(ReadCFG(HFileRD_Md,PReadStr)==-1){fclose(HFileRD_Md);goto _ContMdRead;};
		for(int i99=0;i99<Nom_Sp;i99++)
			if(strcmp(Tsk->Sp[i99].Name,PReadStr)==0)Tsk->Md[Nom_Md].ID_Sp=i99;
		if(ReadCFG(HFileRD_Md,PReadStr)==-1){fclose(HFileRD_Md);goto _ContMdRead;};
		goto	_Mt2_Md;
		};
	if(strcmp(ReadStr,"[Md|UD]")==0)
		{
		if(ReadCFG(HFileRD_Md,PReadStr)==-1){fclose(HFileRD_Md);goto _ContMdRead;};
		for(int i99=0;i99<Nom_UD;i99++)
			if(strcmp(Tsk->UD[i99].Name,PReadStr)==0)Tsk->Md[Nom_Md].ID_UD=i99;
		if(ReadCFG(HFileRD_Md,PReadStr)==-1){fclose(HFileRD_Md);goto _ContMdRead;};
		goto	_Mt2_Md;
		};
	//Ìåòîä
	if(strcmp(ReadStr,"[Md|Mt]")==0)
		{//Ìåòîä
		if(ReadCFG(HFileRD_Md,PReadStr)==-1){fclose(HFileRD_Md);goto _ContMdRead;};
		strcpy(Tsk->Mt[Nom_Mt].Name,ReadName(PReadStr)[1]);//name
		DublMt[Nom_Mt]=atoi(ReadName(PReadStr)[0]);
		Tsk->Mt[Nom_Mt].Ext=DublMt[Nom_Mt];
		//
		Tsk->Mt[Nom_Mt].ID_Md=Nom_Md;//
		if(ReadCFG(HFileRD_Md,PReadStr)==-1){fclose(HFileRD_Md);goto _ContMdRead;};
		if(strcmp(ReadStr,"N_PO")==0){fscanf(HFileRD_Md,"%s",Temp_Ch);Tsk->Mt[Nom_Mt].N_PO=atoi(Temp_Ch);};
		if(ReadCFG(HFileRD_Md,PReadStr)==-1){fclose(HFileRD_Md);goto _ContMdRead;};
		if(strcmp(ReadStr,"N_PI")==0){fscanf(HFileRD_Md,"%s",Temp_Ch);Tsk->Mt[Nom_Mt].N_PI=atoi(Temp_Ch);};
		//
		if(ReadCFG(HFileRD_Md,PReadStr)==-1){fclose(HFileRD_Md);goto _ContMdRead;};
        if(strcmp(ReadStr,"X_Mt")==0)
        {
			fscanf(HFileRD_Md,"%s",Temp_Ch);Tsk->Mt[Nom_Mt].X=atoi(Temp_Ch);
            if(ReadCFG(HFileRD_Md,PReadStr)==-1){fclose(HFileRD_Md);goto _ContMdRead;};
	     	if(strcmp(ReadStr,"Y_Mt")==0){fscanf(HFileRD_Md,"%s",Temp_Ch);Tsk->Mt[Nom_Mt].Y=atoi(Temp_Ch);}
            if(ReadCFG(HFileRD_Md,PReadStr)==-1){fclose(HFileRD_Md);goto _ContMdRead;};
        }   
		_Mt8_Md:
		if(strcmp(ReadStr,"[Md|Mt|Pr]")==0)
			{
			Tsk->Pr[Nom_Pr].ID_Mt=Nom_Mt;
			if(ReadCFG(HFileRD_Md,PReadStr)==-1){fclose(HFileRD_Md);goto _ContMdRead;};
			strcpy(Tsk->Pr[Nom_Pr].Name,PReadStr);
			if(ReadCFG(HFileRD_Md,PReadStr)==-1){fclose(HFileRD_Md);goto _ContMdRead;};
			if(strcmp(PReadStr,"Vol")==0)
				{//fscanf(HFileRD_Md,"%s",PReadStr);
				ReadCFG(HFileRD_Md,PReadStr);
				if(strcmp(PReadStr,"#")==0)
					Tsk->Pr[Nom_Pr].Vol[0]=0;
					else strcpy(Tsk->Pr[Nom_Pr].Vol,PReadStr);

				};
			//
			if(ReadCFG(HFileRD_Md,PReadStr)==-1){fclose(HFileRD_Md);goto _ContMdRead;};
			Nom_Pr++;
			goto _Mt8_Md;
			};
		if(strcmp(ReadStr,"[Md|Mt|LP]")==0)
			{
			//Çàïîëíåíèå ñòðóêòóðû  Mt1_2[xx]
			Mt1_2[N_Mt1_2].ID_MD=Nom_Md;
			if(ReadCFG(HFileRD_Md,PReadStr)==-1){fclose(HFileRD_Md);goto _ContMdRead;};
			while(ReadStr[0]!='[')
				{if(strcmp(ReadStr,"PO")==0){fscanf(HFileRD_Md,"%s",Temp_Ch);Mt1_2[N_Mt1_2].PO=atoi(Temp_Ch);};
				if(strcmp(ReadStr,"Mt1")==0) fscanf(HFileRD_Md,"%s",Mt1_2[N_Mt1_2].Mt1);
				if(strcmp(ReadStr,"PI")==0) {fscanf(HFileRD_Md,"%s",Temp_Ch);Mt1_2[N_Mt1_2].PI=atoi(Temp_Ch);};
				if(strcmp(ReadStr,"Mt2")==0) fscanf(HFileRD_Md,"%s",Mt1_2[N_Mt1_2].Mt2);
				if(ReadCFG(HFileRD_Md,PReadStr)==-1){fclose(HFileRD_Md);goto _ContMdRead;};
				};
			N_Mt1_2++;
			goto _Mt8_Md;
			};
		Nom_Mt++;
		goto _Mt2_Md;
		};
	//
	Nom_Md++;
	};
//******************************
//******************************
fclose(HFileRD_Md);
_ContMdRead:
  //âîñòàíîâëåíèå çíà÷åíèÿ PReadStr
  strcpy(ReadStr,ReadStrTemp);
  //
  };
//
//
//------------------------------6---------------------------------------
//Þíèò
if(strcmp(ReadStr,"[Un]")==0)
	{//Þíèò
	if(ReadCFG(HFileRD,PReadStr)==-1) goto _EndCFGRead;
	strcpy(Tsk->Un[Nom_Un].Name,PReadStr);
	if(ReadCFG(HFileRD,PReadStr)==-1) goto _EndCFGRead;
	if(strcmp(ReadStr,"TypeConnect")==0){fscanf(HFileRD,"%s",Temp_Ch);Tsk->Un[Nom_Un].TypeConnect=atoi(Temp_Ch);};
	if(ReadCFG(HFileRD,PReadStr)==-1) goto _EndCFGRead;
	_Mt9:
	if(strcmp(ReadStr,"[Un|Md]")==0)
		{//
		if(ReadCFG(HFileRD,PReadStr)==-1) goto _EndCFGRead;
		for(int i99=0;i99<Nom_Md;i99++)
			{if(strcmp(Tsk->Md[i99].Name,ReadStr)==0)
				{
				Tsk->MdL[Nom_MdL].ID_Un=Nom_Un;
				Tsk->MdL[Nom_MdL].ID_Md=i99;
				}
			};
		if(ReadCFG(HFileRD,PReadStr)==-1) goto _EndCFGRead;
		Nom_MdL++;
		goto _Mt9;
		};
	Nom_Un++;
	goto _Mt0;
	};
//
//
//
//------------------------------7---------------------------------------
if(strcmp(ReadStr,"[RS]")==0)
	{//Ðàäèîñåòü
	if(ReadCFG(HFileRD,PReadStr)==-1) goto _EndCFGRead;
	strcpy(Tsk->RS[Nom_RS].Name,PReadStr);
	//
	if(ReadCFG(HFileRD,PReadStr)==-1) goto _EndCFGRead;
	if(strcmp(ReadStr,"Fl")==0)
		{fscanf(HFileRD,"%s",Temp_Ch);Tsk->RS[Nom_RS].Fl=atoi(Temp_Ch);};
	//
	if(ReadCFG(HFileRD,PReadStr)==-1) goto _EndCFGRead;
	_Mt1:
	if(strcmp(ReadStr,"[RS|Ad]")==0)
		{//Àäðåññàò
		if(ReadCFG(HFileRD,PReadStr)==-1) goto _EndCFGRead;
		strcpy(Tsk->Ad[Nom_Ad].Name,ReadName(PReadStr)[1]);//Èìÿ
		Tsk->Ad[Nom_Ad].ID_RS=Nom_RS;//Ññûëêà íà ðàäèîñåòü
		if(ReadCFG(HFileRD,PReadStr)==-1) goto _EndCFGRead;
		if(strcmp(ReadStr,"Azim")==0)
			{fscanf(HFileRD,"%s",Temp_Ch);Tsk->Ad[Nom_Ad].Azim=atoi(Temp_Ch);};
		if(ReadCFG(HFileRD,PReadStr)==-1) goto _EndCFGRead;
		if(strcmp(ReadStr,"Fl")==0)
			{fscanf(HFileRD,"%s",Temp_Ch);Tsk->Ad[Nom_Ad].Fl=atoi(Temp_Ch);};
		//
		if(ReadCFG(HFileRD,PReadStr)==-1) goto _EndCFGRead;
		Nom_Ad++;
		goto _Mt1;
		};
//
	if(strcmp(ReadStr,"[RS|Fr]")==0)
		{//Àäðåññàò
		if(ReadCFG(HFileRD,PReadStr)==-1) goto _EndCFGRead;
		Tsk->Fr[Nom_Fr].F=atoi(ReadName(PReadStr)[1]);
		Tsk->Fr[Nom_Fr].ID_RS=Nom_RS;
		//
		if(ReadCFG(HFileRD,PReadStr)==-1) goto _EndCFGRead;
		if(strcmp(ReadStr,"Fl")==0)
			{fscanf(HFileRD,"%s",Temp_Ch);Tsk->Fr[Nom_Fr].Fl=atoi(Temp_Ch);};

		if(ReadCFG(HFileRD,PReadStr)==-1) goto _EndCFGRead;
		Nom_Fr++;
		goto _Mt1;
		};
//
	if(strcmp(ReadStr,"[RS|Pl]")==0)
		{//Ïóë ÷àñòîò
		if(ReadCFG(HFileRD,PReadStr)==-1) goto _EndCFGRead;
		strcpy(Tsk->Pl[Nom_Pl].Name,ReadName(PReadStr)[1]);
		Tsk->Pl[Nom_Pl].ID_RS=Nom_RS;//Ññûëêà íà ðàäèîñåòü

		if(ReadCFG(HFileRD,PReadStr)==-1) goto _EndCFGRead;
		if(strcmp(ReadStr,"Fl")==0)
			{fscanf(HFileRD,"%s",Temp_Ch);Tsk->Pl[Nom_Pl].Fl=atoi(Temp_Ch);};
		//
		if(ReadCFG(HFileRD,PReadStr)==-1) goto _EndCFGRead;
		_Mt4:
		if(strcmp(ReadStr,"[RS|Pl|Fr]")==0)
			{//×àñòîòà
			Tsk->GrFr[Nom_GrFr].ID_Pl=Nom_Pl;
			if(ReadCFG(HFileRD,PReadStr)==-1) goto _EndCFGRead;
			Temp=atoi(ReadName(PReadStr)[1]);
			for(int i99=0;i99<Nom_Fr;i99++)
			   if(Nom_RS==(Tsk->Fr[i99].ID_RS))
			   if(Temp==(Tsk->Fr[i99].F))Tsk->GrFr[Nom_GrFr].ID_F=i99;
			//
			if(ReadCFG(HFileRD,PReadStr)==-1) goto _EndCFGRead;
			if(strcmp(ReadStr,"GrFr_Fl")==0)
				{fscanf(HFileRD,"%s",Temp_Ch);Tsk->GrFr[Nom_GrFr].Fl=atoi(Temp_Ch);};
			//
			if(ReadCFG(HFileRD,PReadStr)==-1) goto _EndCFGRead;
			Nom_GrFr++;
			goto _Mt4;
			};
		Nom_Pl++;
		goto _Mt1;
		};
	if(strcmp(ReadStr,"[RS|Se]")==0)
		{//Ñåàíñ
		if(ReadCFG(HFileRD,PReadStr)==-1) goto _EndCFGRead;
		strcpy(Tsk->Se[Nom_Se].Name,ReadName(PReadStr)[1]);//Èìÿ
		Tsk->Se[Nom_Se].ID_RS=Nom_RS;//Ññûëêà íà ðàäèîñåòü
		//
		if(ReadCFG(HFileRD,PReadStr)==-1) goto _EndCFGRead;
		if(strcmp(ReadStr,"Fl")==0)
			{fscanf(HFileRD,"%s",Temp_Ch);Tsk->Se[Nom_Se].Fl=atoi(Temp_Ch);};
		if(ReadCFG(HFileRD,PReadStr)==-1) goto _EndCFGRead;
		if(strcmp(ReadStr,"ModeWork")==0)
			{fscanf(HFileRD,"%s",Temp_Ch);Tsk->Se[Nom_Se].ModeWork=atoi(Temp_Ch);};


		//UCHAR	Prgn[240];
                size_t i99;
		for(i99=0;i99<240;i99++)
			{ReadCFG(HFileRD,PReadStr);Tsk->Se[Nom_Se].Prgn[i99]=atoi(PReadStr);};
		//UCHAR	Stat[240];
		for(i99=0;i99<240;i99++)
			{ReadCFG(HFileRD,PReadStr);Tsk->Se[Nom_Se].Stat[i99]=atoi(PReadStr);};
		//
		if(ReadCFG(HFileRD,PReadStr)==-1) goto _EndCFGRead;
		_Mt5:
		if(strcmp(ReadStr,"[RS|Se|TC]")==0)
			{//Òî÷êà ñâÿçè
			//
			Tsk->TC[Nom_TC].ID_Se=Nom_Se;
			if(ReadCFG(HFileRD,PReadStr)==-1) goto _EndCFGRead;
			if(ReadCFG(HFileRD,PReadStr)==-1) goto _EndCFGRead;
			if(strcmp(ReadStr,"Fl")==0)
				{fscanf(HFileRD,"%s",Temp_Ch);Tsk->TC[Nom_TC].Fl=atoi(Temp_Ch);};
			//
			if(ReadCFG(HFileRD,PReadStr)==-1) goto _EndCFGRead;
			_Mt6:
			if(strcmp(ReadStr,"[RS|Se|TC|Pl]")==0)
				{//Ïóë òî÷êè ñâÿçè
				TC_Pl[N_Pl].F=Nom_TC;
				if(ReadCFG(HFileRD,PReadStr)==-1) goto _EndCFGRead;
				strcpy(TC_Pl[N_Pl].Name,PReadStr);//Èìÿ
				if(ReadCFG(HFileRD,PReadStr)==-1) goto _EndCFGRead;
				if(strcmp(ReadStr,"GrPl_Fl")==0)
					{fscanf(HFileRD,"%s",Temp_Ch);TC_Pl[N_Pl].Fl=atoi(Temp_Ch);};
				TC_Pl[N_Pl].ID_RS=Nom_RS;
				//
				if(ReadCFG(HFileRD,PReadStr)==-1) goto _EndCFGRead;
				N_Pl++;
				goto _Mt6;
				};
/*			if(strcmp(ReadStr,"[RS|Se|TC|Se]")==0)
				{//Ñåàíñ òî÷êè ñâÿçè
				TC_Se[N_Se].F=Nom_TC;
				if(ReadCFG(HFileRD,PReadStr)==-1) goto _EndCFGRead;
				strcpy(TC_Se[N_Se].Name,PReadStr);//Èìÿ
				//
				if(ReadCFG(HFileRD,PReadStr)==-1) goto _EndCFGRead;
				N_Se++;
				goto _Mt6;
				};*/
			if(strcmp(ReadStr,"[RS|Se|TC|Ad]")==0)
				{//Àäðåññàò òî÷êè ñâÿçè
				TC_Ad[N_Ad].F=Nom_TC;
				if(ReadCFG(HFileRD,PReadStr)==-1) goto _EndCFGRead;
				strcpy(TC_Ad[N_Ad].Name,PReadStr);//Èìÿ
				TC_Ad[N_Ad].ID_RS=Nom_RS;
				//
				if(ReadCFG(HFileRD,PReadStr)==-1) goto _EndCFGRead;
				N_Ad++;
				goto _Mt6;
				};
			if(strcmp(ReadStr,"[RS|Se|TC|Un]")==0)
				{//Óñòðîéñòâî ñâÿçè
				if(ReadCFG(HFileRD,PReadStr)==-1) goto _EndCFGRead;
				for( i99=0;i99<Nom_Un;i99++)
					if(strcmp(ReadStr,Tsk->Un[i99].Name)==0)Tsk->TC[Nom_TC].ID_Un=i99;
				if(ReadCFG(HFileRD,PReadStr)==-1) goto _EndCFGRead;
				goto _Mt6;
				};
			//
			Nom_TC++;
			goto _Mt5;
			};
		//
		Nom_Se++;
		goto _Mt1;
		};
	//
	Nom_RS++;
	goto _Mt0;
	};


if(strcmp(ReadStr,"[]")==0) goto _EndCFGRead;
if(ReadCFG(HFileRD,PReadStr)==-1) goto _EndCFGRead;
goto _Mt0;


_EndCFGRead:
fclose(HFileRD);

/*
//+++++++++++++++
//
// TC->Pl  ..->Ad  ..->Md
for(size_t i99=0;i99<N_Pl;i99++)
	{
	for(size_t i_Pl=1;i_Pl<Nom_Pl;i_Pl++)
		if(strcmp(TC_Pl[i99].Name,Tsk->Pl[i_Pl].Name)==0)
			{//Èìÿ ñîâï.
			Tsk->GrPl[Nom_GrPl].ID_TC=TC_Pl[i99].F;
			Tsk->GrPl[Nom_GrPl].ID_Pl=i_Pl;
			Tsk->GrPl[Nom_GrPl].Fl=TC_Pl[i99].Fl;
			Nom_GrPl++;
			break;
			};
       };
//
for(i99=0;i99<N_Ad;i99++)
	{
	for(size_t i_Ad=1;i_Ad<Nom_Ad;i_Ad++)
		if(strcmp(TC_Ad[i99].Name,Tsk->Ad[i_Ad].Name)==0)
			{//Èìÿ ñîâï.
			Tsk->TC[i99+1].ID_Ad=i_Ad;
			break;
			};
	};
//
//  Mt1--Mt2
for(int	i_M12=0;i_M12<N_Mt1_2;i_M12++)
	{
	Tsk->LP[i_M12+1].PO=Mt1_2[i_M12].PO;
	Tsk->LP[i_M12+1].PI=Mt1_2[i_M12].PI;
	if(strcmp(Mt1_2[i_M12].Mt1,"Input")==0)
		Tsk->LP[i_M12+1].ID_Mt1=-1;
		else
		for(size_t i99=1;i99<Nom_Mt;i99++)//Mt1
			if(Mt1_2[i_M12].ID_MD==Tsk->Mt[i99].ID_Md)
			if(strcmp(ReadName(Mt1_2[i_M12].Mt1)[1],Tsk->Mt[i99].Name)==0)
			if(atoi(ReadName(Mt1_2[i_M12].Mt1)[0])==DublMt[i99])
				Tsk->LP[i_M12+1].ID_Mt1=i99;
	for(size_t i99=1;i99<Nom_Mt;i99++)
		if(Mt1_2[i_M12].ID_MD==Tsk->Mt[i99].ID_Md)
		if(strcmp(ReadName(Mt1_2[i_M12].Mt2)[1],Tsk->Mt[i99].Name)==0)
		if(atoi(ReadName(Mt1_2[i_M12].Mt2)[0])==DublMt[i99])
			Tsk->LP[i_M12+1].ID_Mt2=i99;
	};


//
//
*/


// TC->Pl  ..->Ad  ..->Md
int i99;
for( i99=0;i99<N_Pl;i99++)
	{
	for(int i_Pl=0;i_Pl<Nom_Pl;i_Pl++)
		if(TC_Pl[i99].ID_RS==Tsk->Pl[i_Pl].ID_RS)
		if(strcmp(TC_Pl[i99].Name,Tsk->Pl[i_Pl].Name)==0)
			{//Èìÿ ñîâï.
			Tsk->GrPl[Nom_GrPl].ID_TC=TC_Pl[i99].F;
			Tsk->GrPl[Nom_GrPl].ID_Pl=i_Pl;
			Tsk->GrPl[Nom_GrPl].Fl=TC_Pl[i99].Fl;
			Nom_GrPl++;
			break;
			};
       };
//
for(i99=0;i99<N_Ad;i99++)
	{
	for(int i_Ad=0;i_Ad<Nom_Ad;i_Ad++)
		if(TC_Ad[i99].ID_RS==Tsk->Ad[i_Ad].ID_RS)
		if(strcmp(TC_Ad[i99].Name,Tsk->Ad[i_Ad].Name)==0)
			{//Èìÿ ñîâï.
			Tsk->TC[i99].ID_Ad=i_Ad;
			break;
			};
	};
//
//  Mt1--Mt2
for(int	i_M12=0;i_M12<N_Mt1_2;i_M12++)
	{
	Tsk->LP[i_M12].PO=Mt1_2[i_M12].PO;
	Tsk->LP[i_M12].PI=Mt1_2[i_M12].PI;
	if(strcmp(Mt1_2[i_M12].Mt1,"Input")==0)
		Tsk->LP[i_M12].ID_Mt1=-1;
		else
		for( i99=0;i99<Nom_Mt;i99++)//Mt1
			if(Mt1_2[i_M12].ID_MD==Tsk->Mt[i99].ID_Md)
			if(strcmp(ReadName(Mt1_2[i_M12].Mt1)[1],Tsk->Mt[i99].Name)==0)
			if(atoi(ReadName(Mt1_2[i_M12].Mt1)[0])==DublMt[i99])
				Tsk->LP[i_M12].ID_Mt1=i99;

	if(strcmp(Mt1_2[i_M12].Mt2,"Output")==0)
		Tsk->LP[i_M12].ID_Mt2=-1;
		else
		for( i99=0;i99<Nom_Mt;i99++)
			if(Mt1_2[i_M12].ID_MD==Tsk->Mt[i99].ID_Md)
			if(strcmp(ReadName(Mt1_2[i_M12].Mt2)[1],Tsk->Mt[i99].Name)==0)
			if(atoi(ReadName(Mt1_2[i_M12].Mt2)[0])==DublMt[i99])
				Tsk->LP[i_M12].ID_Mt2=i99;
	};


Tsk->Max_RS=Nom_RS;//
Tsk->Max_Ad=Nom_Ad;//
Tsk->Max_TC=Nom_TC;//
Tsk->Max_Un=Nom_Un;//
Tsk->Max_UD=Nom_UD;//
Tsk->Max_Pl=Nom_Pl;//
Tsk->Max_Fr=Nom_Fr;//
Tsk->Max_Se=Nom_Se;//
Tsk->Max_MdL=Nom_MdL;//
Tsk->Max_En=Nom_En;//
Tsk->Max_Sp=Nom_Sp;//

Tsk->Max_BMt=Nom_BMt;//
Tsk->Max_BPr=Nom_BPr;//
Tsk->Max_BIn=Nom_BIn;//
Tsk->Max_BOut=Nom_BOut;//

Tsk->Max_Md=Nom_Md;//
Tsk->Max_Mt=Nom_Mt;//
//
//Tsk->Max_LP=N_Mt1_2+1;//
//
Tsk->Max_LP=N_Mt1_2;//
Tsk->Max_Pr=Nom_Pr;//

Tsk->Max_GrFr=Nom_GrFr;//
Tsk->Max_GrPl=Nom_GrPl;//

return ;
};





//*********************************************************
char*	DataImEx::EncStr(char* pStr)
{
//	char	BufEncStr[1024];
	int pIn=0,pEnc=0;
	while(pStr[pIn]!=0)
	{
		if(((pStr[pIn]>0x20)&&(pStr[pIn]<0x7e)&&(pStr[pIn]!=0x3b))||(pStr[pIn]>0xbf))
		{	//Êîïèðóåì ñèìâîë
			BufEncStr[pEnc++]=pStr[pIn++];
		}
		else
		{	//Çàìåíÿåì íà ñëóæåáíóþ ïîñëåäîâàòåëüíîñòü
			pEnc+=sprintf(BufEncStr+pEnc,"~%02x",(UCHAR)pStr[pIn++]);
		}
	}
	BufEncStr[pEnc]=0;
	return BufEncStr;
};


//*********************************************************
int	DataImEx::ReadCFG(FILE* FileCFG,char* Strng)
{
	int 	ChGE1;
	char	Temp[1024],TMP_Vol[3];
_ReadCFG:
	if(fscanf(FileCFG,"%s",Temp)==1)
	{
		if(Temp[0]==';')
		{
_GetEndStr:
			ChGE1=fgetc(FileCFG);
			if((ChGE1!=0xa)&(ChGE1!=-1)) goto _GetEndStr;
			goto _ReadCFG;
		};
		//*****************************
		//Temp->Strng
		//*****************************
		size_t i=0,j=0;
		while (Temp[i]!=0)
		{	//ïðîâåðêà íà ñëóæåáíûé ñèìâîë ~(0x7e)
			if(Temp[i]==0x7e)
			{	//
				i++;
				TMP_Vol[0]=Temp[i++];TMP_Vol[1]=Temp[i++];TMP_Vol[2]=0;
				Strng[j++]=strtoul(TMP_Vol,NULL,16);
			}
			else
			{
				Strng[j++]=Temp[i++];
			}
		}
		Strng[j++]=0;
		//*****************************
		return 1;
	}
	else return -1;
};


ListID_FiFo::ListID_FiFo()
{
	CntInSide=0;
};
ListID_FiFo::~ListID_FiFo()
{
	Dmp.Clear();
};

void	ListID_FiFo::DelAllEl()
{
	DelAllElem();
	Dmp.Clear();
	CntInSide++;
}

int		ListID_FiFo::AddID(INT64	ID)
{
	ID_Elem*	tEl;
	int	rc=-1;
	MyLock(__FILE__,__LINE__,&Fl_RE,NULL);
	tEl=Find((uchar*)&ID,sizeof(ID));
	if (tEl==NULL)
	{
		tEl=NewElem();
		if (tEl)
		{
			((ID_Elem64*)tEl)->Init(ID);
		}
		else
		{
			tEl=new ID_Elem64(ID);
		}
		rc=Add(tEl);
		if (rc==-1)
		{
			delete tEl;
		}
		else
		{
			CntInSide++;
			Dmp.Data((uchar*)&ID,sizeof(ID));
		}
	}
	MyUnLock(__FILE__,__LINE__,&Fl_RE,NULL);
	return rc;
};

INT64		ListID_FiFo::GetID()
{
	INT64		tID=0;
	uchar*		tBuf;
	size_t			tCnt;
	int			rc=0;
	ID_Elem*	tEl;
	MyLock(__FILE__,__LINE__,&Fl_RE,NULL);
	rc=Dmp.LockOData(tBuf,tCnt);
	if (rc)
	{
		tID=*(INT64*)(tBuf+sizeof(SockHeader));
		tEl=Find((uchar*)&tID,sizeof(tID));
		if (tEl)
		{
			Del(tEl);
			CntInSide--;
		}
		else
		{
			{size_t iasm=1;}
		};
		Dmp.UnLockOData();
	};
	MyUnLock(__FILE__,__LINE__,&Fl_RE,NULL);
	return tID;
};







//*********************************************************
char**	DataImEx::ReadName(char* InName)
{
int	PosCHIn=0;
int	PosCHOut=0;

while(InName[PosCHIn]!=0)
     if(InName[PosCHIn]=='|')
		{OutName[1][PosCHOut++]=0;
		for(size_t i99=0;i99<PosCHOut;i99++)OutName[0][i99]=OutName[1][i99];
//		strcpy(OutName[0],OutName[1]);
		PosCHOut=0;PosCHIn++;}
		else
		OutName[1][PosCHOut++]=(char)InName[PosCHIn++];
OutName[1][PosCHOut]=0;
return OutName;
};


void	AddressInfo::Init()
{

	NCStrategic=0;NetType=NT_LAN;NetCommand=NC_Default;
	DestinAI.Init();// Self
	SourceAI.Init();// My
	NativeAI.Init();
	TranspAI.Init();
#ifndef _NOSRV
	Services* pSrv=GetServices();
	if (pSrv)	 ID=pSrv->GetUID();	// Ïî óìîë÷àíèþ íå èñïîëüçóåòñÿ, à ðàáîòàåò òîëüêî â Neuron !!! (Ýòî èìåííî _ñåðâèñíàÿ_ âîçìîæíîñòü !!!)
	else ID=0;
#else
	ID=0;
#endif
};

INT64	SrtHex2INT64(char* Buf)
{
	INT64	i64Vol;
	if ((Buf[0]=='0') && (Buf[1]=='x')) Buf+=2;
	int LenStr=strlen(Buf);
	uint	Vol1=strtoul(Buf+strlen(Buf)-8,NULL, 16 );
	int		TCh=Buf[LenStr-8];Buf[LenStr-8]=0;
	INT64	Vol2=strtoul( Buf, NULL, 16 );
	Buf[LenStr-8]=TCh;
	i64Vol=Vol1|(Vol2<<32);
	return i64Vol;
};



