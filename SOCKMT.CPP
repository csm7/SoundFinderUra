#include "pch.h"
//#include "stdafx.h"

#include "SockMt.h"
#include <io.h>
#include <fcntl.h>
#include <direct.h>
#include <sys/stat.h>

static inline void MyLock(FlagStruct*	StrFlag,Decoder* Dec)
{
	EnterCriticalSection(&StrFlag->CrSection);
	StrFlag->CntIn++;
	if (StrFlag->CntIn>1)
		{int iasm=1;}
	StrFlag->PDec=Dec;
};

static inline void MyUnLock(FlagStruct* StrFlag,Decoder* Dec)
{
	StrFlag->PDec=NULL;
	StrFlag->CntIn--;
	if (StrFlag->CntIn<0)
		{int iasm=1;}
	LeaveCriticalSection(&StrFlag->CrSection);
};



//#ifdef _DEBUG
////#include <afx.h>
//#define new DEBUG_NEW
//#undef THIS_FILE
//static char THIS_FILE[] = __FILE__;
//#endif

// int fCallBack(ClientSock* S,int Event)
// {
// 	return ((ElSockCliEx*)S)->SetEvent(Event);
// };

/*
int		Damper2Proc(void* Obj,UCHAR* tBuf,int Cnt)
{
	return 0;
};

Damper2::Damper2(int mSize)
{
	maxSize=mSize;
	Srv=NULL;
	PDec=NULL;
	for (int i=0;i<2;i++)
	{
		if (Size[i]!=0)
		{
			if (PDec)
				PDec->free (Buf[i]);
			else
				free (Buf[i]);
			Size[i]=0;
		}
		Rd[i]=Wr[i]=0;
	}
	ActWr=ActRd=Locked=0;
	sSize=sPeak=100.;
	sprintf(SemName,"SDmp%d",(int)this);
	hmtx=OpenSemaphore(SEMAPHORE_ALL_ACCESS	,TRUE,SemName);
	if (hmtx==NULL)
		hmtx=CreateSemaphore(NULL,1,1,SemName);
	pObj=NULL;
	DmpProc=&Damper2Proc;
};

void	Damper2::SetPrm(Decoder* PD,void* Obj,int (*DmpPrc)(void*,UCHAR*,int))
{
	PDec=PD;
	if (PDec)
		Srv=PDec->Srv;
	pObj=Obj;
	DmpProc=DmpPrc;
};

int		Damper2::FreeSize(int NumB)
{
	int	rc;
	int	tRd=Rd[NumB];
	int	tWr=Wr[NumB];
	if (tWr>=tRd)
	{
		rc=Size[NumB]-tWr+tRd;
	}
	else
	{
		rc=tRd-tWr;
	};
	return rc;
};

void	Damper2::Clear(void)
{
//	WaitForSingleObject(hmtx,INFINITE);
	DmpProc=&DamperProc;
	for (int i=0;i<2;i++)
	{
		if (Size[i]!=0)
		{
			if (PDec)
				PDec->free (Buf[i]);
			else
				free (Buf[i]);
			Size[i]=0;
		}
		Rd[i]=Wr[i]=0;
	}
	ActWr=ActRd=Locked=0;
	sSize=sPeak=100.;
	if (Srv)
	{
		char	tB[64];
		sprintf(tB,"pObj=%08X Clear",this);
		Srv->OutLog(tB,PDec,"Damper");
	}
//	ReleaseSemaphore(hmtx,1,NULL); 
};

int		Damper2::Data(UCHAR* tBuf,int Cnt)
{
	UCHAR	*tB;
	int		rc=0;
	int		tFSize;
	int		tLocked;
	if (Cnt==0) return rc;
	tFSize=FreeSize(ActWr);
	if (tFSize<=Cnt)
	{	// Нехватает !!!
		if (Size[ActWr]==0)
		{
			Size[ActWr]=Cnt*10;
			if (PDec)
				Buf[ActWr]=(UCHAR*)PDec->malloc(Size[ActWr]);
			else
				Buf[ActWr]=(UCHAR*)malloc(Size[ActWr]);
		}
		else
		{
			WaitForSingleObject(hmtx,INFINITE);
			if (Locked==0)
				Locked=2;
			tLocked=Locked;
			if (tLocked==1)
			{	//	tLocked=1;
				if (ActWr==ActRd)
				{
					int tFSize2=FreeSize(ActRd);
					int tActWr=(ActWr++)&1;


				}
				else
				{
					int tFSize2=FreeSize(ActRd);
					if (tFSize+tFSize2<=Cnt)
					{
						Size[ActWr]=Size[ActWr]*2+Cnt*10;
						if (PDec)
							tB=(UCHAR*)PDec->realloc((void*)Buf[ActWr],Size[ActWr]);
						else
							tB=(UCHAR*)realloc((void*)Buf[ActWr],Size[ActWr]);
						if (tB)
						{
							Buf[ActWr]=tB;
						}
						else
						{
							if (Srv)
							{
								char	tB[64];
								sprintf(tB,"pObj=%08X Error Allocated",this);
								Srv->OutLog(tB,PDec,"Damper2");
							}
							rc=-1;
						}
					}
					else
					{	// Есть место
						memcpy(Buf[ActWr]+Wr[ActWr],tBuf,Size[ActWr]-Wr[ActWr]);
						Cnt-=Size[ActWr]-Wr[ActWr];
						tBuf+=Size[ActWr]-Wr[ActWr];
						Wr[ActWr]=0;ActWr=(ActWr++)&1;
						Wr[ActWr]=0;
					}

				}
				ReleaseSemaphore(hmtx,1,NULL);
			}
			else
			{	//	tLocked=2;
				ReleaseSemaphore(hmtx,1,NULL);
				if (Wr[ActWr]>=Rd[ActWr])
				{
					Size[ActWr]=Size[ActWr]*2+Cnt*10;
					if (PDec)
						tB=(UCHAR*)PDec->realloc((void*)Buf[ActWr],Size[ActWr]);
					else
						tB=(UCHAR*)realloc((void*)Buf[ActWr],Size[ActWr]);
					if (tB)
					{
						Buf[ActWr]=tB;
					}
					else
					{
						if (Srv)
						{
							char	tB[64];
							sprintf(tB,"pObj=%08X Error Allocated",this);
							Srv->OutLog(tB,PDec,"Damper2");
						}
						rc=-1;
					}
				}
				else
				{
					int	oSize=Size[ActWr];
					Size[ActWr]=oSize*2+Cnt*10;
					if (PDec)
						tB=(UCHAR*)PDec->realloc((void*)Buf[ActWr],Size[ActWr]);
					else
						tB=(UCHAR*)realloc((void*)Buf[ActWr],Size[ActWr]);
					if (tB)
					{
						Buf[ActWr]=tB;
						memcpy(Buf[ActWr]+oSize+Cnt*10+Rd[ActWr],Buf[ActWr]+Rd[ActWr],oSize-Rd[ActWr]);
						Rd[ActWr]+=oSize+Cnt*10;
					}
					else
					{
						if (Srv)
						{
							char	tB[64];
							sprintf(tB,"pObj=%08X Error Allocated",this);
							Srv->OutLog(tB,PDec,"Damper2");
						}
						rc=-1;
					}
				}
				Locked=0;
			}
			memcpy(Buf[ActWr]+Wr[ActWr],tBuf,Cnt);
			Wr[ActWr]+=Cnt;
			if (Wr[ActWr]>=Size[ActWr])
			{
				Wr[ActWr]=0;ActWr=(ActWr++)&1;
				Wr[ActWr]=0;
			}
		};

	}
	else
	{
		int	tRd=Rd[ActWr];
		if (Wr[ActWr]>=tRd)
		{
			if (Cnt>Size[ActWr]-Wr[ActWr])
			{
				memcpy(Buf[ActWr]+Wr[ActWr],tBuf,Size[ActWr]-Wr[ActWr]);
				memcpy(Buf[ActWr],tBuf+Size[ActWr]-Wr[ActWr],Cnt-Size[ActWr]+Wr[ActWr]);
				Wr[ActWr]=Cnt-Size[ActWr]+Wr[ActWr];
			}
			else
			{
				memcpy(Buf[ActWr]+Wr[ActWr],tBuf,Cnt);
				Wr[ActWr]+=Cnt;
				if (Wr[ActWr]>=Size[ActWr])
				{
					Wr[ActWr]=0;ActWr=(ActWr++)&1;
					Wr[ActWr]=0;
				}
			}
		}
		else
		{
			memcpy(Buf[ActWr]+Wr[ActWr],tBuf,Cnt);
			Wr[ActWr]+=Cnt;
			if (Wr[ActWr]>=Size[ActWr])
			{
				Wr[ActWr]=0;ActWr=(ActWr++)&1;
				Wr[ActWr]=0;
			}
		}
		rc=Cnt;
	}
	return rc;
};

int		Damper2::ReSize(int Cnt)
{
	UCHAR	*tB;
	int	rc=0;
	int	fSize=FreeSize();
	if (fSize<=Cnt+16)
	{
		if (Size==0)
		{
			Size=Cnt*10;
			if (PDec)
				Buf=(UCHAR*)PDec->malloc(Size);
			else
				Buf=(UCHAR*)malloc(Size);
		}
		else
		{
			if (Wr>=Rd)
			{
				Size=Size*2+Cnt*10;
				if (PDec)
					tB=(UCHAR*)PDec->realloc((void*)Buf,Size);
				else
					tB=(UCHAR*)realloc((void*)Buf,Size);
				if (tB)
				{
					Buf=tB;
				}
				else
				{
					if (Srv)
					{
						char	tB[64];
						sprintf(tB,"pObj=%08X Error Allocated",this);
						Srv->OutLog(tB,PDec,"Damper2");
					}
					rc=-1;
				}
			}
			else
			{
				int	oSize=Size;
				Size=oSize*2+Cnt*10;
				if (PDec)
					tB=(UCHAR*)PDec->realloc((void*)Buf,Size);
				else
					tB=(UCHAR*)realloc((void*)Buf,Size);
				if (tB)
				{
					Buf=tB;
					memcpy(Buf+oSize+Cnt*10+Rd,Buf+Rd,oSize-Rd);
					Rd+=oSize+Cnt*10;
				}
				else
				{
					if (Srv)
					{
						char	tB[64];
						sprintf(tB,"pObj=%08X Error Allocated",this);
						Srv->OutLog(tB,PDec,"Damper2");
					}
					rc=-1;
				}
			}
		}
		if (Srv)
		{
			char	tB[64];
			sprintf(tB,"pObj=%08X AddSize=%d",this,Size);
			Srv->OutLog(tB,PDec,"Damper2");
		}
		sSize=100.;
		sPeak=100.;
	}
	else
	{
		double	Used=100*(double)(Size-fSize)/(double)Size;
		if (sPeak<Used)
			sPeak=Used;
		else
			sPeak=0.9999*sPeak+0.0001*Used;
		sSize=0.9999*sSize+0.0001*Used;
		if (sPeak<50.)
		{
			if (Wr==Rd)
			{
				Wr=Rd=0;sSize=100.;sPeak=100.;
				Size-=Size/10;
				if (Size!=0)
				{
					if (PDec)
						PDec->free (Buf);
					else
						free (Buf);
					if (PDec)
						Buf=(UCHAR*)PDec->malloc(Size);
					else
						Buf=(UCHAR*)malloc(Size);
					if (Buf==NULL)
					{
						Size=0;
						if (Srv)
						{
							char	tB[64];
							sprintf(tB,"pObj=%08X Error Allocated",this);
							Srv->OutLog(tB,PDec,"Damper2");
						}
						rc=-1;
					}
				}
				else
				{
					if (PDec)
						PDec->free (Buf);
					else
						free (Buf);
				}
				sPeak=100.;
			}
			if (Srv)
			{
				char	tB[64];
				sprintf(tB,"pObj=%08X ReSize=%d",this,Size);
				Srv->OutLog(tB,PDec,"Damper2");
			}
		}
	}
	return rc;
};

int		Damper2::OData()
{
	int	rc=0;
	int	tWr=Wr;
	UCHAR	*tBuf;
	int		Cnt;
	if (tWr!=Rd)
	{
		rc=WaitForSingleObject(hmtx,INFINITE);
		if (tWr>Rd)
		{
			tBuf=Buf+Rd;
			Cnt=tWr-Rd;
			DmpProc(pObj,tBuf,Cnt);
			Rd+=Cnt;if (Rd>=Size) Rd=0;
		}
		else
		{
			tBuf=Buf+Rd;
			Cnt=Size-Rd;
			DmpProc(pObj,tBuf,Cnt);
			Rd=0;
		}
		rc=Cnt;
		ReleaseSemaphore(hmtx,1,NULL); 
	}
	return rc;
};

Damper2::~Damper2()
{
	if (Size!=0)
	{
		if (PDec)
			PDec->free (Buf);
		else
			free (Buf);
	}
	CloseHandle(hmtx);
};
*/

	NetAdapterOut::NetAdapterOut()
{
// Этот фрагмент должен быть вставлен во все последующие объекты
// NetAdapterOut_ должен бить заменен на имя объекта
		 Name_Mt=NetAdapterOut_NameMt;Rem_Mt=NetAdapterOut_RemMt;Ver_=NetAdapterOut_Version;Auth_=NetAdapterOut_Author;
		 PinName_I=NetAdapterOut_PinNameI;PinName_O=NetAdapterOut_PinNameO;Prm_=NetAdapterOut_Prm;Vol_=NetAdapterOut_Vol;

		 Num_PinI=0;while (PinName_I[2*Num_PinI][0]!=0x0) Num_PinI++;
		 Num_PinO=0;while (PinName_O[2*Num_PinO][0]!=0x0) Num_PinO++;
		 Num_Prm=0;while (Prm_[2*Num_Prm][0]!=0x0)Num_Prm ++;
// -------------------------------------------------------------
	Fl_Out=0;
}
	NetAdapterOut::~NetAdapterOut()
{
}

void	NetAdapterOut::Param(char* Prm,char* Vol)
{
}

void	NetAdapterOut::Start()
{
}

void	NetAdapterOut::Stop()
{
}

int	NetAdapterOut::Data(int N_Pin,UCHAR* pmas,size_t Cnt)
{
	int	OCnt=Cnt;
	if (Cnt!=0)
	{
		Fl_Out=1;
		tPin=N_Pin|0x55aa0000;
		tBuf=pmas;
		tCnt=Cnt;
	}
return OCnt;
}

int	NetAdapterOut::OutData(int N_Pin,UCHAR* &pmas,size_t &Cnt)
{
	Cnt=0;
	switch (Fl_Out)
	{
	case 0:
		break;
	case 1:
		pmas=(UCHAR*)&tPin;
		Cnt=sizeof(tPin)+sizeof(tCnt);
		Fl_Out=2;
		break;
	case 2:
		pmas=tBuf;
		Cnt=tCnt;
		Fl_Out=0;
		break;
	}
return Cnt;
}


	NetAdapterIn::NetAdapterIn()
{
// Этот фрагмент должен быть вставлен во все последующие объекты
// NetAdapterIn_ должен бить заменен на имя объекта
		 Name_Mt=NetAdapterIn_NameMt;Rem_Mt=NetAdapterIn_RemMt;Ver_=NetAdapterIn_Version;Auth_=NetAdapterIn_Author;
		 PinName_I=NetAdapterIn_PinNameI;PinName_O=NetAdapterIn_PinNameO;Prm_=NetAdapterIn_Prm;Vol_=NetAdapterIn_Vol;

		 Num_PinI=0;while (PinName_I[2*Num_PinI][0]!=0x0) Num_PinI++;
		 Num_PinO=0;while (PinName_O[2*Num_PinO][0]!=0x0) Num_PinO++;
		 Num_Prm=0;while (Prm_[2*Num_Prm][0]!=0x0)Num_Prm ++;
// -------------------------------------------------------------
	Fl_Out=0;Sm=0;
	OutBuf=NULL;RealSize=0;
}
	NetAdapterIn::~NetAdapterIn()
{
	if (OutBuf!=NULL)
	{
		free(OutBuf);OutBuf=NULL;
	}
}

void	NetAdapterIn::Param(char* Prm,char* Vol)
{
}

void	NetAdapterIn::Start()
{
}

void	NetAdapterIn::Stop()
{
}

int	NetAdapterIn::Data(int N_Pin,UCHAR* pmas,size_t Cnt)
{
	int	OCnt=0;
	if (Cnt!=0)
	{
		switch(Fl_Out)
		{
		case 0:
			if ((Cnt==2*sizeof(int))&&(0x55aa==*(short*)(pmas+2)))
			{
				tPin=(*(int*)pmas)&0xffff;
				FullCnt=(*(int*)(pmas+4));
				Sm=0;
				Fl_Out=1;
				break;
			}
			else
			{
				Srv->OutLog("Error Syn!!! ",this);
			}
		case 1:
			if (RealSize==0)
			{
				RealSize=Cnt; OutBuf=(UCHAR*)malloc(RealSize);
			}
			else
			{
				if (RealSize<Sm+Cnt)
				{
					RealSize=Sm+Cnt;
					OutBuf=(UCHAR*)realloc((UCHAR*)OutBuf,RealSize);
				}
				else
				{
					if (RealSize/10>Sm+Cnt)
					{
						RealSize-=RealSize/10;
						OutBuf=(UCHAR*)realloc((UCHAR*)OutBuf,RealSize);
					}
				}
			}
			memcpy(OutBuf+Sm,pmas,Cnt);
			Sm+=Cnt;
			if (FullCnt<=Sm)
			{
				tBuf=OutBuf;
				tCnt=FullCnt;
				if (FullCnt==Sm)
				{
					Fl_Out=2;
				}
				else
				{
					memcpy(OutBuf,OutBuf+FullCnt,Sm-FullCnt);
					Sm-=FullCnt;
				}
				OCnt=Sm;
			}
			break;
		case 2:
			Fl_Out=0;
			break;
		}
	}
return OCnt;
}

int	NetAdapterIn::OutData(int N_Pin,UCHAR* &pmas,size_t &Cnt)
{
	Cnt=0;
	if (Fl_Out==2)
	{
		if (N_Pin==tPin)
		{
			pmas=tBuf;
			Cnt=tCnt;
			Fl_Out=0;
		}
	}
return Cnt;
}

int	EventFun(ServerClientSock *csock,int Event)
{
	SockSrv*	SSrv=(SockSrv*)csock->servSock->ID_Srv;
	SSrv->WrEvent(csock,Event);
	return 0;
}

	SockSrv::SockSrv()
{
// Этот фрагмент должен быть вставлен во все последующие объекты
// SockSrv_ должен бить заменен на имя объекта
		 Name_Mt=SockSrv_NameMt;Rem_Mt=SockSrv_RemMt;Ver_=SockSrv_Version;Auth_=SockSrv_Author;
		 PinName_I=SockSrv_PinNameI;PinName_O=SockSrv_PinNameO;Prm_=SockSrv_Prm;Vol_=SockSrv_Vol;

		 Num_PinI=0;while (PinName_I[2*Num_PinI][0]!=0x0) Num_PinI++;
		 Num_PinO=0;while (PinName_O[2*Num_PinO][0]!=0x0) Num_PinO++;
		 Num_Prm=0;while (Prm_[2*Num_Prm][0]!=0x0)Num_Prm ++;
// -------------------------------------------------------------
		TimeOut=15000;
		WrMsg=0;RdMsg=0;Fl_EvCon=0;Fl_Pack=0;
		CB_Size=10240;
		Act_Cli=-1;
		Mode=0;
		Fl_Data=0;
		Port=500;
		SrvSock=NULL;
		tDiv=0;SetDiv=500;
		Fl_Stop=1;
		WatchDogPack.PHeader.Command= Sys_WDog;
		WatchDogPack.PHeader.DataLen = 0;
//		WatchDogPack.PHeader.NetAddrFrom = 0xff;

		NetType=NT_LAN;
}
	SockSrv::~SockSrv()
{
	ServerClientSock	*SrvCliSock;	
	char	tBuf[256];
		if (SrvSock!=NULL)
		{
			for(int i=0;i<SrvSock->List->n;i++)
			{
				SrvCliSock=SrvSock->List->Get(i);
				if ((SrvCliSock->Flag!=0)&&(SrvCliSock!=0))
				{
					sprintf(tBuf,"Client DisConnect! Host=%s IP=%s",SrvCliSock->namehost,SrvCliSock->IPhost);
					if (Srv!=0) Srv->OutLog(tBuf,this);
				}
			};
			delete	SrvSock;
		}
}

void	SockSrv::Param(char* Prm,char* Vol)
{
	int	rc=0;
	if (strcmp(Prm,"NetType")==0) 
	{
		NetType=NT_LAN;
		if (strcmp(Vol,"PAN")==0) NetType=NT_PAN;
		if (strcmp(Vol,"LAN")==0) NetType=NT_LAN;
		if (strcmp(Vol,"WAN")==0) NetType=NT_WAN;
	}

	if (strcmp(Prm,"Mode")==0) 
	{
		if (strcmp(Vol,"NoBufNoPack")==0) Mode=0;
		if (strcmp(Vol,"BufNoPack")==0) Mode=1;
		if (strcmp(Vol,"BufPack")==0) Mode=2;
		if (SrvSock!=NULL)	SrvSock->SetMode(Mode);
	}
	if (strcmp(Prm,"ServerPort")==0) {rc=1; Port=atoi(Vol);}
	if (strcmp(Prm,"SizeTrBuf")==0) {CB_Size=atoi(Vol);if (SrvSock!=NULL) SrvSock->SizeCB(CB_Size);}
	if (strcmp(Prm,"BlockTimeOut")==0) {TimeOut=atoi(Vol);if (SrvSock!=NULL) SrvSock->TimeOut(TimeOut);}
	if (rc==1)
	{
		if (Fl_Stop==0)
		{
			if (SrvSock!=NULL)
				delete	SrvSock;
			SrvSock=new ServerSockEx(Port,EventFun,NetType,NumSockCli,Srv,this);
			SrvSock->ID_Srv=this;
			SrvSock->SizeCB(CB_Size);
			SrvSock->SetMode(Mode);
			SrvSock->TimeOut(TimeOut);
		}
	}
}

void	SockSrv::Start()
{
	if (Fl_Stop==1)
	{
		SrvSock=new ServerSockEx(Port,EventFun,NetType,NumSockCli,Srv,this);
		SrvSock->ID_Srv=this;
		SrvSock->SizeCB(CB_Size);
		SrvSock->SetMode(Mode);
		SrvSock->TimeOut(TimeOut);
	}
	WrMsg=0;RdMsg=0;
	Fl_Stop=0;
}

void	SockSrv::Stop()
{
	Fl_Stop=1;
	if (SrvSock!=NULL)
	{
		ServerClientSock	*SrvCliSock;	
		char	tBuf[256];
		for(int i=0;i<SrvSock->List->n;i++)
		{
			SrvCliSock=SrvSock->List->Get(i);
			if ((SrvCliSock!=0)&&(SrvCliSock->Flag!=0))
			{
				sprintf(tBuf,"Client DisConnect! Host=%s IP=%s",SrvCliSock->namehost,SrvCliSock->IPhost);
				if (Srv!=0) Srv->OutLog(tBuf,this);
			}
		};
		delete	SrvSock;SrvSock=NULL;
	}
}

void	SockSrv::WrEvent(ServerClientSock* csock,int Event)
{
	EvMsg[WrMsg].Event=Event;
	EvMsg[WrMsg].Handle=csock->Handle;
//	strcpy(EvMsg[WrMsg].Host,csock->namehost);
	EvMsg[WrMsg].IPadr=csock->IPadr;
	strcpy(EvMsg[WrMsg].IPhost,csock->IPhost);
	strcpy(EvMsg[WrMsg].namehost,csock->namehost);
	EvMsg[WrMsg].Number=csock->NumInList;
	EvMsg[WrMsg].port=csock->port;
/*
	EvMsg[WrMsg].Number=csock->NumInList;
//	EvMsg[WrMsg].csock=csock;
	EvMsg[WrMsg].Event=Event;
//	EvMsg[WrMsg].Host[0]=0;
//	EvMsg[WrMsg].IP[0]=0;
	strcpy(EvMsg[WrMsg].IP,csock->IPhost);
	strcpy(EvMsg[WrMsg].Host,csock->namehost);
*/
	if (++WrMsg>=100) WrMsg=0;
}

EventMsg*	SockSrv::RdEvent()
{
	EventMsg	*rc=0;
	if (WrMsg!=RdMsg)
	{
		rc=&EvMsg[RdMsg];
		if (++RdMsg>=100) RdMsg=0;
	}
	return rc;
}


int SockSrv::IdleFunc(int NumberTr)			// Idle function  if rc!=0 recall function (?)  NumberTr=-1 System NumberTr+0x8000 - DinTr
{
	int	rc=ChildIdleFunc(NumberTr);
	Data(2,(uchar*)&NumberTr,sizeof(NumberTr));
	return rc;
}

int	SockSrv::Data(int N_Pin,UCHAR* pmas,size_t Cnt)
{
	int	OCnt=0;
	if (Fl_Stop!=1)
	{
		switch(N_Pin)
		{
		case 0:
			Act_Cli=*(int*)pmas;
			if (Act_Cli>=NumSockCli)
				Act_Cli=-1;
			Fl_Data=1;OCnt=Cnt;tActCli=0;
			break;
		case 1:
			SrvSock->SendBuf(Act_Cli,pmas,Cnt);
			break;
		case 2:
			if (--tDiv<0)
			{
				tDiv=SetDiv;
				if (Mode==2)
					SrvSock->SendBuf(-1,(UCHAR*)&WatchDogPack,sizeof(PackHeader));
			}
			Fl_Data=1;OCnt=Cnt;tActCli=0;
			break;
		case 3: // Установить порт
			Param("ServerPort",(char*)pmas);
			break;
		}
	}
return OCnt;
}

int	SockSrv::OutData(int N_Pin,UCHAR* &pmas,size_t &Cnt)
{
	Cnt=0;
	if (Fl_Data==1)
	{
		switch(N_Pin)
		{
		case 0:
			tEMsg=RdEvent();
			if(tEMsg!=0)
			{
				pmas=(UCHAR*)&tEMsg->Number;
				Cnt=sizeof(tEMsg->Number);
			}
			else
			{
_mm1:			SrvSock->ReadSock(tActCli,tBuf,tCnt);
				if (tCnt==0)
				{
					if (++tActCli<NumSockCli)
					{
						goto _mm1;
					}
					else
					{
						Fl_Data=0;				
					}
				}
				else
				{
					pmas=(UCHAR*)&tActCli;Cnt=sizeof(tActCli);
					Fl_Pack=1;
				}
			}
			break;
		case 1:	// Событие
			if(tEMsg!=0)
			{
				char	tBuf[256];
				pmas=(UCHAR*)&tEMsg->Event;Cnt=sizeof(tEMsg->Event);
				if (tEMsg->Event==CBE_Connect) Fl_EvCon=1;
				if (Srv!=0)
					switch (tEMsg->Event)
					{
					case CBE_Connect:
						sprintf(tBuf,"Client Connect! Host=%s IP=%s",tEMsg->namehost,tEMsg->IPhost);
						Srv->OutLog(tBuf,this);
						break;
					case CBE_DisCon:
						sprintf(tBuf,"Client DisConnect! Host=%s IP=%s",tEMsg->namehost,tEMsg->IPhost);
						Srv->OutLog(tBuf,this);
						break;
					case CBE_Error:
						sprintf(tBuf,"Client Error! Host=%s IP=%s",tEMsg->namehost,tEMsg->IPhost);
						Srv->OutLog(tBuf,this);
						break;
					}
			}
			break;
		case 2: // IP
			if(Fl_EvCon==1)
			{
				pmas=(UCHAR*)&tEMsg->IPhost;	// IP string
				Cnt=sizeof(tEMsg->IPhost);
//				pmas=(UCHAR*)&tEMsg->csock->IPhost;	// IP string
//				Cnt=sizeof(tEMsg->csock->IPhost);
			}
			break;
		case 3: // Name
			if(Fl_EvCon==1)
			{
//				pmas=(UCHAR*)&tEMsg->csock->namehost;	// name host
//				Cnt=sizeof(tEMsg->csock->namehost);
				pmas=(UCHAR*)&tEMsg->namehost;	// name host
				Cnt=sizeof(tEMsg->namehost);
				Fl_EvCon=0;
			}
			break;
		case 4:
			if(Fl_Pack==1)
			{
				pmas=tBuf;Cnt=tCnt;
				Fl_Pack=0;
			}
			break;
		}
	}
return Cnt;
}

void	ConnectFunction(SockCli* SCli)
{
	
	SCli->CliSock->DisConnect();
	SCli->CliSock->Connect();
	if (SCli->CliSock->Flag==1)
		SCli->Fl_Conn=1;
	SCli->StartConn=2;
	ExitThread(0);
}

	SockCli::SockCli()
{
// Этот фрагмент должен быть вставлен во все последующие объекты
// SockCli_ должен бить заменен на имя объекта
		 Name_Mt=SockCli_NameMt;Rem_Mt=SockCli_RemMt;Ver_=SockCli_Version;Auth_=SockCli_Author;
		 PinName_I=SockCli_PinNameI;PinName_O=SockCli_PinNameO;Prm_=SockCli_Prm;Vol_=SockCli_Vol;

		 Num_PinI=0;while (PinName_I[2*Num_PinI][0]!=0x0) Num_PinI++;
		 Num_PinO=0;while (PinName_O[2*Num_PinO][0]!=0x0) Num_PinO++;
		 Num_Prm=0;while (Prm_[2*Num_Prm][0]!=0x0)Num_Prm ++;
// -------------------------------------------------------------
		TimeOut=15000;
		CB_Size=10240;Fl_Conn=0;
		strcpy(IPAddr,"127.0.0.1");
		Port=500;
		CliSock=new ClientSockEx(IPAddr,Port,Srv,this);
		CliSock->SizeCB(CB_Size);
		Mode=0;
		CliSock->SetMode(Mode);
		CliSock->TimeOut=TimeOut;
		StartConn=0;tDiv=0;SetDiv=500;
		Fl_Stop=1;//Fl_DB=0;
		WatchDogPack.PHeader.Command = Sys_WDog;
		WatchDogPack.PHeader.DataLen = 0;
//		WatchDogPack.PHeader.NetAddrFrom = 0xff;
		CE_Conn=NULL;
}
	SockCli::~SockCli()
{
		char	tBuf[256];
		while (StartConn==1)
			Sleep(10);
		if (StartConn==2)
		{
			StartConn=0;
			CloseHandle(hThConn);
		}
		if (CliSock->Flag!=0)
		{
			sprintf(tBuf," DisConnect! Host=%s IP=%s",CliSock->HostName,CliSock->HostIP);
			if (Srv!=0) Srv->OutLog(tBuf,this);
		}
		delete	CliSock;
		if (CE_Conn)
		{
			Srv->DelCriticalError(CE_Conn); CE_Conn=NULL;
		};
};

void	SockCli::Param(char* Prm,char* Vol)
{
	int	rc=0;
	if (strcmp(Prm,"Mode")==0) 
	{
		if (strcmp(Vol,"NoBufNoPack")==0) Mode=0;
		if (strcmp(Vol,"BufNoPack")==0) Mode=1;
		if (strcmp(Vol,"BufPack")==0) Mode=2;
		CliSock->SetMode(Mode);
	}
/*
	if (strcmp(Prm,"DataBuf")==0) 
	{
		if (strcmp(Vol,"On")==0)
		{
			Fl_DB=1;
		}
		else
		{
			Fl_DB=0;
		};
	};
*/
	if (strcmp(Prm,"ServerPort")==0) {rc=1; Port=atoi(Vol);}
	if (strcmp(Prm,"ServerIP")==0) {rc=1; strcpy(IPAddr,Vol);}
	if (strcmp(Prm,"SizeTrBuf")==0) {CB_Size=atoi(Vol);CliSock->SizeCB(CB_Size);}
	if (strcmp(Prm,"BlockTimeOut")==0) {TimeOut=atoi(Vol);CliSock->TimeOut=TimeOut;}
	if (rc==1)
	{
		while (StartConn==1)
			Sleep(10);
		if (StartConn==2)
		{
			StartConn=0;
			CloseHandle(hThConn);
		}
		StartConn=0;
		delete	CliSock;
		if (Fl_Conn!=0) 
			Fl_Conn=0;
		CliSock=new ClientSockEx(IPAddr,Port,Srv,this);
		CliSock->SizeCB(CB_Size);
		CliSock->SetMode(Mode);
		CliSock->TimeOut=TimeOut;
	}
}

void	SockCli::Start()
{
	Fl_Stop=0;
}

void	SockCli::Stop()
{
	Fl_Stop=1;
	while (StartConn==1)
		Sleep(10);
	if (StartConn==2)
	{
		StartConn=0;
		CloseHandle(hThConn);
	}
}

int	SockCli::Data(int N_Pin,UCHAR* pmas,size_t Cnt)
{
	int	OCnt=0;
	if (Fl_Stop!=1)
	switch(N_Pin)
	{
	case 0:
		CliSock->SendBuf((char*)pmas,Cnt);
		break;
	case 1:
		if (CliSock->Flag==0)
		{
			if (Fl_Conn==2)	// 0->1->2->3->0
			{
				Fl_Conn=3; OCnt=1;
			}

			if (StartConn==2)
			{
				StartConn=0;
				CloseHandle(hThConn);
			}
			if (StartConn==0)
			{
				StartConn=1;
				hThConn = CreateThread(NULL,					// pointer to security attributes
										0,						// initial thread stack size
										(LPTHREAD_START_ROUTINE)ConnectFunction,// pointer to thread function
										(void*)this,			// argument for new thread
										0,						// creation flags
										(LPDWORD)&TID_Conn);	// pointer to receive thread ID); 
			}
		}
		else	
		{
			if (--tDiv<0)
			{
				tDiv=SetDiv;
				if (Mode==2)
					CliSock->SendBuf((char*)&WatchDogPack,sizeof(PackHeader));
			}
			OCnt=Cnt;
		}
		break;
	case 2:	// Установить адрес
		Param("ServerIP",(char*)pmas);
		break;
	case 3: // Установить порт
		Param("ServerPort",(char*)pmas);
		break;
	default:;
	}
return OCnt;
}

int	SockCli::OutData(int N_Pin,UCHAR* &pmas,size_t &Cnt)
{
	Cnt=0;
	char	tBuf[256];
	switch(N_Pin)
	{
	case 0:
		switch(Fl_Conn)
		{
		case 1:
			// Connect
			sprintf(tBuf," Connect! Host=%s IP=%s",CliSock->HostName,CliSock->HostIP);
			if (Srv!=0) Srv->OutLog(tBuf,this);
			tEMsg.Event=CBE_Connect;
			pmas=(UCHAR*)&tEMsg.Event;
			Cnt=sizeof(tEMsg.Event);
			Fl_Conn=2;
			if (CE_Conn)
			{
				Srv->DelCriticalError(CE_Conn);CE_Conn=NULL;
			}
			break;
		case 3:
			// DisConnect
			sprintf(tBuf," DisConnect! Host=%s IP=%s",CliSock->HostName,CliSock->HostIP);
			if (Srv!=0) Srv->OutLog(tBuf,this);
			tEMsg.Event=CBE_DisCon;
			pmas=(UCHAR*)&tEMsg.Event;
			Cnt=sizeof(tEMsg.Event);
			Fl_Conn=0;
			CE_Conn=Srv->SetCriticalError(this,"DisConnect Socket!","Warning","SockCli",CE_Conn);
			break;
		}
		break;
	case 1:
		CliSock->ReadSock(pmas,Cnt);
		break;
	default:;
	}
return Cnt;
}

SockCliN::SockCliN()
{
// Этот фрагмент должен быть вставлен во все последующие объекты
// SockCliN_ должен бить заменен на имя объекта
		 Name_Mt=SockCliN_NameMt;Rem_Mt=SockCliN_RemMt;Ver_=SockCliN_Version;Auth_=SockCliN_Author;
		 PinName_I=SockCliN_PinNameI;PinName_O=SockCliN_PinNameO;Prm_=SockCliN_Prm;Vol_=SockCliN_Vol;

		 Num_PinI=0;while (PinName_I[2*Num_PinI][0]!=0x0) Num_PinI++;
		 Num_PinO=0;while (PinName_O[2*Num_PinO][0]!=0x0) Num_PinO++;
		 Num_Prm=0;while (Prm_[2*Num_Prm][0]!=0x0)Num_Prm ++;
// -------------------------------------------------------------
	State=0;
	Counter=0;
	TimeClick=0;
	pElement=NULL;
	nPinOut=-1;

	ClientList=new ID_List8();
}
SockCliN::~SockCliN()
{
	delete ClientList;
}

void SockCliN::Param(char* Prm,char* Vol)
{
}

void SockCliN::Start()
{
	State=1;
	FindStrList8	FSL8;
	pCurrnet=(pElSockCli)ClientList->Begin(FSL8);
	while(pCurrnet)
	{
		pCurrnet->Start();
		pCurrnet=(pElSockCli)ClientList->Next(FSL8);
	}
}

void SockCliN::Stop()
{
	State=2;
	FindStrList8	FSL8;
	pCurrnet=(pElSockCli)ClientList->Begin(FSL8);
	while(pCurrnet)
	{
		pCurrnet->Stop();
		pCurrnet=(pElSockCli)ClientList->Next(FSL8);
	}
}

int SockCliN::Data(int N_Pin,UCHAR* pmas,size_t Cnt)
{
	int rc,OCnt=0;
	if(State==1)
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
				
				pElement=(pElSockCli)ClientList->Find(
					(PUCHAR)pack->descriptor,
					strlen(pack->descriptor));
				
				if(pElement==NULL)
				{
					if(pack->command==NetSockNew)
					{
						pElement=new ElSockCli(pack->descriptor);
						pElement->Srv=Srv;
						if(pElement==NULL) return 0;
						ClientList->Add(pElement);
						Counter++;
					}
					else return 0;
				}
				
				switch(pack->command)
				{
				case NetSockDel:
						rc=ClientList->Del(pElement);
						if(rc!=-1) Counter--;
						pElement=NULL;
						break;
				case NetSockParam:
						pElement->Param(pack->param,pack->vol);
						break;
				case NetSockStart:
						pElement->Start();
						break;
				case NetSockStop:
						pElement->Stop();
						break;
				case NetSockData:
						OCnt=pElement->Data(
							pack->pin,pack->buf,pack->cnt);
						nPinOut=0;
						break;
				}
			}
			break;
		case 1:
			{
				pCurrnet=(pElSockCli)ClientList->Begin(FSL8);
				while(pCurrnet)
				{
					pCurrnet->Data(N_Pin,pmas,Cnt);
					pCurrnet=(pElSockCli)ClientList->Next(FSL8);
				}
				pElement=(pElSockCli)ClientList->Begin(FSL8);
				TimeClick=1; OCnt=TimeClick; nPinOut=0;
			}
			break;

		}
	}
	return OCnt;
}

int SockCliN::OutData(int N_Pin,UCHAR* &pmas,size_t &Cnt)
{
	Cnt=0;

	if(State!=0)
	{
		pack=&mypack;
		if(pElement==NULL) return 0;
		pack->command=NetSockOutData;
		int count=0; while(nPinOut>-1)
		{
			pack->pin=nPinOut; if(++nPinOut==2) nPinOut=0;
			pack->handle=pElement->handle;
			strcpy(pack->descriptor,pElement->Descriptor);
			pElement->OutData(pack->pin, pack->buf, pack->cnt);
			if(pack->cnt) break;

			count++; // Couner Cnt=0;

			// завершился круг опроса, посчитаем сколькр раз
			if(nPinOut==0) if(count>1) // 2 or 3
			{
				if(TimeClick) // будем ли искать остальных клиентов?
				{
					pElement=(pElSockCli)ClientList->Next(FSL8);
					if(pElement==NULL) { nPinOut=-1; TimeClick=0; break; }
					count=0; nPinOut=0;
				}
				else { nPinOut=-1; break; }
			}
		}
		pmas=(PUCHAR)pack; Cnt=sizeof(PackNetObj);
	}
	return Cnt;
}



// -----------------------------------------------------------------------------------
// 
// void	ConnectFunctionN(ElSockCliEx* obj)
// {
// 	if(obj->Flag==1)
// 	{
// 		obj->DisConnect();
// 	}
// 	obj->Connect();
// 	if(obj->Flag==1)
// 	{
// 		if (obj->State!=2)
// 		{
// 			//obj->State=2;
// 			//obj->FlagEvent|=SockCliConnect;
// 			obj->SetEvent(0);
// 		}
// 	}
// 	else
// 	{
// 		obj->State=0;
// 	}
// 	obj->StartConn=2;
// 	obj->Debug("ExitThread");
// 	ExitThread(16);
// }
// 
// 
// int ElSockCliEx::SetParam(char* Prm,char* Vol)
// {
// 	int	rc=0;
// 	if (strcmp(Prm,"Mode")==0) 
// 	{
// 		if (strcmp(Vol,"NoBufNoPack")==0) Mode=0;
// 		if (strcmp(Vol,"BufNoPack")==0) Mode=1;
// 		if (strcmp(Vol,"BufPack")==0) Mode=2;
// 	}
// 	if (strcmp(Prm,"ServerIP")==0)
// 	{
// 		rc=1;
// 		strcpy(IPAddr,Vol);
// 		GetHostName();
// 	}
// 	if (strcmp(Prm,"ServerPort")==0)	{rc=1; Port=atoi(Vol);}
// 
// 	if (strcmp(Prm,"SizeTrBuf")==0)
// 	{SizeCB(atoi(Vol));}
// 	if (strcmp(Prm,"BlockTimeOut")==0)	{TimeOut=atoi(Vol);}
// 
// 	if (rc==1)
// 	{
// 		int fl=Enable;
// 		
// 		if(fl) Stop(); 
// 		ReInit(IPAddr,Port,HostName); 
// 		if(fl) Start(); 
// 	}
// 	return 0;
// }
// int ElSockCliEx::Start()
// {
// 	Enable=1;
// 	return 0;
// }
// int ElSockCliEx::Stop()
// {
// 	Enable=0;
// 	if(State==2)
// 	{
// 		State=3;
// 		DisConnect();
// 		//State=0;
// 		//FlagEvent|=SockCliDisConnect;
// 		SetEvent(1);
// 	}
// 	return 0;
// }
// int	ElSockCliEx::ObjIdle()
// {
// /*
// 	State:
// 	0 - DISCONNECT
// 	1 - попытка конекта
// 	2 - CONNECT
// 	3 - попытка дисконекта
// 
// */
// 	if(State==0)
// 	{
// 		if(StartConn==2)
// 		{
// 			if(hThConn)
// 			{
// 				Debug("CloseHandle");
// 				CloseHandle(hThConn);
// 				hThConn=0;
// 
// 				DebugCount=1;
// 			}
// 		}
// 	}
// 
// 	if(DebugCount)
// 	{
// 		if(++DebugCount<20) 
// 			return 0;
// 		DebugCount=0;
// 	}	
// 	
// 	if(Enable==1) 
// 	{
// 		if(State==0)
// 		{
// 			State=1;
// 			if (StartConn==2)
// 			{
// 				StartConn=0;
// 				if(hThConn)
// 				{
// 					CloseHandle(hThConn);
// 					hThConn=0;
// 				}
// 			}
// 			StartConn=1;
// 			Debug("CreateThread");
// 			hThConn = CreateThread(NULL,// pointer to security attributes
// 				0,						// initial thread stack size
// 				(LPTHREAD_START_ROUTINE)ConnectFunctionN,// pointer to thread function
// 				(void*)this,			// argument for new thread
// 				0,						// creation flags
// 				(LPDWORD)&TID_Conn);	// pointer to receive thread ID); 
// 		}
// 		if(State==2) if(Mode==2)
// 		{
// 			// ElSockCliEx	:	public NetObject, public ClientSockEx
// 			if (--tDiv<0)
// 			{
// 				tDiv=SetDiv;
// 				SendBuf((char*)&WatchDogPack,sizeof(int));
// 			}
// 		}
// 		// непреднамеренный разрыв сокета
// 		if(State==2) if(Flag==0)
// 		{
// 			State=3;
// 			DisConnect();
// 			//State=0;
// 			//FlagEvent|=SockCliDisConnect;
// 			SetEvent(1);
// 		}
// 	}
// 	return 0;
// }
// 
// int	ElSockCliEx::SetEvent(int Event)
// {
// 	int FlagEvent=0; State=0;
// 	
// 	switch(Event)
// 	{
// 	case 0:
// 		State=2;
// 		FlagEvent=SockCliConnect;
// 		break;
// 	case 1:
// 		State=0;
// 		FlagEvent=SockCliDisConnect;
// 		break;
// 	}
// 
// 	if(FlagEvent)
// 	{
// 		FlagEventArray[index_w]=FlagEvent;
// 		index_w=(index_w+1)&index_m;
// 	}
// 
// 	return FlagEvent;
// };
// 
// int	ElSockCliEx::RdEvent(PUCHAR &p, size_t &c)
// {
// 	c=0; if(index_w==index_r) return 0;
// 
// 	int FlagEvent=FlagEventArray[index_r];
// 	index_r=(index_r+1)&index_m;
// 
// 	switch(FlagEvent) {
// 	case SockCliConnect:
// 		{
// 			tEMsg.Event=CBE_Connect;
// 			p=(UCHAR*)&tEMsg.Event;
// 			c=sizeof(tEMsg.Event);
// 		}
// 		break;
// 	case SockCliDisConnect:
// 		{
// 			tEMsg.Event=CBE_DisCon;
// 			p=(UCHAR*)&tEMsg.Event;
// 			c=sizeof(tEMsg.Event);
// 		}
// 		break;
// 	default:;
// 	}
// 
// 	return c;
// }
// 
// int	ElSockCliEx::GetHostName()
// {
// 	int i; *HostName=0;
// 
// 	//////////////////////////////////////////////////////////////////////////
// 	HostName[0]=0;//????
// 	/////////////////////////////////////////////////////////////////////////]
// 	int FirstChar=HostIP[0];
// 	if(('0'<=FirstChar)&&(FirstChar<='9'))
// 	{
// 		if (pSrv->SInt)
// 		{				
// 			FindStrList8	FSL85;
// 			El_CliInt*	Cli=(El_CliInt*)pSrv->SInt->CliLAN.Begin(FSL85);
// 			while (Cli)
// 			{
// 				if (pSrv->SInt)
// 				{				
// 					if(pSrv->SInt->CliLAN[i])
// 					{
// 						if(strcmp(Cli->Cli.IPAddr,IPAddr)==0)
// 						{
// 							strcpy(HostName,Cli->Cli.Host);
// 							return 1;
// 						}
// 					}
// 				}	
// 				Cli=(El_CliInt*)pSrv->SInt->CliLAN.Next(FSL85);
// 			}
// 		}
// 	}
// 	else
// 	{
// 		// а тут пока тупое копирование
// 		strcpy(HostName,IPAddr);
// 	}
// 
// 	return 0;
// }
// 
// int ElSockCliEx::Debug (char *q)
// {
// 	return 0;
// 
// 	char s[1024];
// 	SYSTEMTIME time;
// 	GetLocalTime(&time);
// 
// 	sprintf(s,"%02ld/%02ld/%02ld %02ld:%02ld:%02ld.%03d  %s\r\n",
// 		time.wDay,time.wMonth,time.wYear,
// 		time.wHour,time.wMinute,time.wSecond,time.wMilliseconds,q);
// 	sprintf(s,"%02ld:%02ld:%02ld.%03d  %s\r\n",
// 		time.wHour,time.wMinute,time.wSecond,time.wMilliseconds,q);
// 
// 	int h=open("\\ElSockCliEx.log",O_BINARY|O_CREAT|O_WRONLY,S_IWRITE);
// 	if(h!=-1)
// 	{
// 		lseek(h,0,SEEK_END);
// 		write(h,s,strlen(s));
// 		close(h);
// 	}
// 
// 	return 0;
// }
// 
// -----------------------------------------------------------------------------------
// 
// int	ElSockSrvEx_EventFun(ServerClientSock *csock,int Event)
// {
// 	pElSockSrvEx ptr=(pElSockSrvEx)csock->servSock;
// 	ptr->WrEvent(csock,Event);
// 	return 0;
// }
// int ElSockSrvEx::WrEvent(ServerClientSock* csock,int Event)
// {
// 	int q=WrMsg-1;
// 	if(q<0) q=0x3ff;
// 	EventMsg* ptr=&EvMsg[q];
// 
// 	EvMsg[WrMsg].Event=Event;
// 	EvMsg[WrMsg].Handle=csock->Handle;
// 	strcpy(EvMsg[WrMsg].Host,csock->namehost);
// 	EvMsg[WrMsg].IPadr=csock->IPadr;
// 	strcpy(EvMsg[WrMsg].IPhost,csock->IPhost);
// 	strcpy(EvMsg[WrMsg].namehost,csock->namehost);
// 	EvMsg[WrMsg].Number=csock->NumInList;
// 	EvMsg[WrMsg].port=csock->port;
// 	WrMsg=(WrMsg+1)&0x3ff;
// 	return 0;
// }
// EventMsg* ElSockSrvEx::RdEvent()
// {
// 	EventMsg *pMess=0;
// 	if (WrMsg!=RdMsg)
// 	{
// 		pMess=&EvMsg[RdMsg];
// 		RdMsg=(RdMsg+1)&0x3ff;
// 	}
// 	return pMess;
// }
// int ElSockSrvEx::SetParam(char* Prm,char* Vol)
// {
// 	if (strcmp(Prm,"Mode")==0) 
// 	{
// 		if (strcmp(Vol,"NoBufNoPack")==0) Mode=0;
// 		if (strcmp(Vol,"BufNoPack")==0) Mode=1;
// 		if (strcmp(Vol,"BufPack")==0) Mode=2;
// 	}
// 	if (strcmp(Prm,"ServerPort")==0)	{ReInit(atoi(Vol));}
// 	if (strcmp(Prm,"SizeTrBuf")==0)	
// 	{SizeCB(atoi(Vol));}
// 	if (strcmp(Prm,"BlockTimeOut")==0)	{TimeOut(atoi(Vol));}
// 
// 	return 0;
// }
// int ElSockSrvEx::Start()
// {
// 	return 0;
// }
// int ElSockSrvEx::Stop()
// {
// 	return 0;
// }
// int	ElSockSrvEx::ObjIdle()
// {
// 	if(Mode==2)
// 	{
// 		// ElSockSrvEx	:	public NetObject, public ServerSockEx
// 		if (--tDiv<0)
// 		{
// 			tDiv=SetDiv;
// 			SendBuf(-1,(UCHAR*)&WatchDogPack,sizeof(int));
// 		}
// 	}
// 	
// 	return 0;
// }
// -----------------------------------------------------------------------------------

	NetService::NetService()
{
// Этот фрагмент должен быть вставлен во все последующие объекты
// NetService_ должен бить заменен на имя объекта
	Name_Mt=NetService_NameMt;Rem_Mt=NetService_RemMt;Ver_=NetService_Version;Auth_=NetService_Author;
	PinName_I=NetService_PinNameI;PinName_O=NetService_PinNameO;Prm_=NetService_Prm;Vol_=NetService_Vol;

	Num_PinI=0;while (PinName_I[2*Num_PinI][0]!=0x0) Num_PinI++;
	Num_PinO=0;while (PinName_O[2*Num_PinO][0]!=0x0) Num_PinO++;
	Num_Prm=0;while (Prm_[2*Num_Prm][0]!=0x0)Num_Prm ++;
// -------------------------------------------------------------
// 	CntCliPack=0;
// 	State=0;
// 	Counter=0;
// 	TimeClick=0;; 
// 	pElement=NULL;		// определяет поведение модуля по хв. воздействию
// 	pElementOut=NULL;	// после TimeClick производится опрос всех объектов списка
// 
// 	StateOut=0;
// 
// 	pMess=NULL;
// 	SockBuf=NULL; 
// 	SockCnt=0;
// 
// 	StopProcessing=0;
// 
// 	TelemetryCount=0;
// 
// 	ClientList=new ID_List8();
// 	MonClientList=new ID_List8();
}
	NetService::~NetService()
{
//	MyLock(__FILE__,__LINE__,&Fl_RE,this);
//	Stop();
// 	FSL8.DestroyStr();
// 	MonClientList->RemoveAllPoiElem();
// 	delete ClientList; ClientList=NULL;
// 	delete MonClientList; MonClientList=NULL;
//	MyUnLock(__FILE__,__LINE__,&Fl_RE,this);
}

void NetService::Param(char* Prm,char* Vol)
{
// 	if(!strcmp(Prm,"StopProcessing"))
// 	{
// 		if(!strcmp(Vol,"No")) StopProcessing=0;
// 		if(!strcmp(Vol,"Yes")) StopProcessing=1;
// 	}
// 	if(!strcmp(Prm,"SetElemForOutData"))
// 	{
// 		SetElemForOutData((pPackNetObj)Vol);
// 	}
// 	if(!strcmp(Prm,"SetElemProperties"))
// 	{
// 		SetElemProperties((pPackNetObj)Vol);
// 	};
}

void NetService::Start()
{
//	MyLock(__FILE__,__LINE__,&Fl_RE,this);
// 	State=1;
// 	if(StopProcessing)
// 	{
// 		FindStrList8	FSL81;
// 		pCurrnet=(pNetObject)ClientList->Begin(FSL81);
// 		while(pCurrnet)
// 		{
// 			pCurrnet->Start();
// 			pCurrnet=(pNetObject)ClientList->Next(FSL81);
// 		}
// 	};
// 	TelemetryCount=0;
//	MyUnLock(__FILE__,__LINE__,&Fl_RE,this);
}

void NetService::Stop()
{
// 	State=2;
// 	if(StopProcessing)
// 	{
// 		FindStrList8	FSL81;
// 		pCurrnet=(pNetObject)ClientList->Begin(FSL81);
// 		while(pCurrnet)
// 		{
// 			pCurrnet->Stop();
// 			pCurrnet=(pNetObject)ClientList->Next(FSL81);
// 		}
// 	};
}


int NetService::IdleFunc(int NumberTr)			// Idle function  if rc!=0 recall function (?)  NumberTr=-1 System NumberTr+0x8000 - DinTr
{
	int rc=0;
// 	pElSockSrvEx pServer;
// 	int	rc=ChildIdleFunc(NumberTr);
// 	pCurrnet=(pNetObject)ClientList->Begin(FSL8);
// 	while(pCurrnet)
// 	{
// 		pCurrnet->ObjIdle();
// 		if(pCurrnet->TypeObj==ObjectServer)
// 		{
// 			pServer=(pElSockSrvEx)pCurrnet;
// 			pServer->CurrentClient=0;
// 		}
// 		pCurrnet=(pNetObject)ClientList->Next(FSL8);
// 	}
// 	pElementOut=(pNetObject)ClientList->Begin(FSL8);
// 	TimeClick=1;
// 	GetTelemetry();
// 
// 	pElSockCliEx pClient;
// 	char s[256];
// 	if(State!=0)
// 	{
// 		pack=&mypack;
// 		if(pElementOut==NULL)
// 		{
// 			pElementOut=(pNetObject)ClientList->Begin(FSL8);
// 		}
// 		if (pElementOut==NULL)
// 		{
// 			return 0;
// 		}
// 		while(1)
// 		{
// 			if(TimeClick)
// 			{
// 				try
// 				{
// 					if(pElementOut->FlagSelectProc)
// 						goto NextClientJump;
// 				}
// 				catch (...) 
// 				{
// 					return 0;
// 				}
// 			}
// 
// 			pack->handle=pElementOut->handle;
// 			strcpy(pack->descriptor,pElementOut->Descriptor);
// 
// 			pack->command=NetSockOutData; pack->cnt=0;
// 
// 			if(pElementOut->TypeObj==ObjectClient)
// 			{
// 				pack->numinlist=-2;
// 				pClient=(pElSockCliEx)pElementOut;
// 				pack->pin=1; // read event
// 				pClient->RdEvent(pack->buf,pack->cnt);
// 				if(pack->cnt) 
// 				{
// 					if(*(int*)pack->buf==CBE_Connect)
// 					{
// 						pClient->Param=1;
// 						Debug(pack->descriptor,"Connect","HostName",pClient->HostName);
// 					}
// 					else if(*(int*)pack->buf==CBE_DisCon)
// 					{
// 						pClient->Param=0;
// 						Debug(pack->descriptor,"Disconnect","HostName",pClient->HostName);
// 						pElementOut=NULL;
// 					}
// 					break;
// 				}
// 				pack->pin=0; // read socket
// 				pClient->CalcClick(2);
// 				pClient->ReadSock(pack->buf, pack->cnt);
// 				pClient->CalcClick(3);
// 				if(pack->cnt)
// 				{
// 					pClient->PackIn++;
// 					pClient->CountIn+=pack->cnt;
// 					break;
// 				}
// 			}
// 
// 			else if(pElementOut->TypeObj==ObjectServer)
// 			{
// 				pServer=(pElSockSrvEx)pElementOut;
// 				// -------------------------------------------------------
// 
// 				pack->pin=0; // num client
// 				if(pack->pin==StateOut)
// 				{
// 					pMess=pServer->RdEvent();
// 					if(pMess)
// 					{
// 						if (pMess->Event!=1) pack->numinlist=pMess->Number;
// 						else pack->numinlist=-1;
// 						pack->buf=(UCHAR*)&pMess->Number;
// 						pack->cnt=sizeof(pMess->Number);
// 						StateOut=1;
// 						sprintf(s,"%d",pMess->Number); Debug2(s); //  [5/27/2005]
// 						break;
// 					}
// 					else
// 					{
// NextClient:
// 						if (CntCliPack++>5000)
// 						{
// 							CntCliPack=0;
// 							if (++pServer->CurrentClient>=NumSockCli)
// 								pServer->CurrentClient=0;
// 						}
// 						pServer->CalcClick(2);
// 						pServer->ReadSock(
// 							pServer->CurrentClient,
// 							SockBuf,SockCnt);
// 						pServer->CalcClick(3);
// 						if(SockCnt) 
// 						{
// 							pack->numinlist=pServer->CurrentClient;
// 							pack->buf=(UCHAR*)&pServer->CurrentClient;
// 							pack->cnt=sizeof(pServer->CurrentClient);
// 							pServer->PackIn++;
// 							pServer->CountIn+=SockCnt;
// 							StateOut=4;
// 							break;
// 						}
// 						else
// 						{
// 							CntCliPack=0;
// 							if (++pServer->CurrentClient<NumSockCli)
// 								goto NextClient;
// 						}
// 					}
// 				}
// 				// -------------------------------------------------------
// 				pack->pin=1; // event
// 				if(pack->pin==StateOut)
// 				{
// 					if(1) // паранойя
// 					{
// 						pack->handle=pElementOut->handle;
// 						strcpy(pack->descriptor,pElementOut->Descriptor);
// 
// 						pack->command=NetSockOutData; pack->cnt=0;
// 					}
// 
// 					pack->buf=(PUCHAR)&pMess->Event;
// 					pack->cnt=sizeof(pMess->Event);
// 					if (pMess->Event==CBE_Connect) 
// 					{
// 						pack->numinlist=pMess->Number;		
// 						StateOut=2;
// 						pServer->Param++;
// 						sprintf(s,"event connect"); Debug2(s); //  [5/27/2005]
// 					}
// 					else 
// 					{
// 						StateOut=0;
// 						Debug(pack->descriptor,"Disconnect");
// 						pServer->Param--;
// 						sprintf(s,"event disconnect"); Debug2(s); //  [5/27/2005]
// 					}
// 
// 					break;
// 				}
// 				// -------------------------------------------------------
// 				pack->pin=2; // host IP
// 				if(pack->pin==StateOut)
// 				{
// 					if(1) // паранойя
// 					{
// 						pack->handle=pElementOut->handle;
// 						strcpy(pack->descriptor,pElementOut->Descriptor);
// 						pack->command=NetSockOutData; pack->cnt=0;
// 					}
// 
// 					pack->numinlist=pMess->Number;		
// 					pack->buf=(PUCHAR)&pMess->IPhost;
// 					pack->cnt=sizeof(pMess->IPhost);
// 					StateOut=3; 
// 					sprintf(s,"ip-address %s",pMess->IPhost); Debug2(s); //  [5/27/2005]
// 					break;
// 				}
// 				// -------------------------------------------------------
// 				pack->pin=3; // host name
// 				if(pack->pin==StateOut)
// 				{
// 					pack->numinlist=pMess->Number;		
// 					pack->buf=(PUCHAR)&pMess->namehost;
// 					pack->cnt=sizeof(pMess->namehost);
// 					StateOut=0;
// 					Debug(pack->descriptor,"Connect",
// 						(PCHAR)&pMess->IPhost,
// 						(PCHAR)&pMess->namehost,
// 						pMess->Number);
// 					sprintf(s,"hostname %s",pMess->namehost); Debug2(s); //  [5/27/2005]
// 					sprintf(s,"%d - %d",pMess->port,pMess->Handle); Debug2(s); //  [5/27/2005]
// 					break;
// 				}
// 				// -------------------------------------------------------
// 				pack->pin=4; // read sock
// 				if(pack->pin==StateOut)
// 				{
// 					pack->numinlist=pMess->Number;		
// 					pack->buf=SockBuf;
// 					pack->cnt=SockCnt;
// 					StateOut=0; break;
// 
// 				}
// 				// -------------------------------------------------------
// 			} 
// 
// NextClientJump:
// 			if(TimeClick) // будем ли искать остальных клиентов?
// 			{
// 				pElementOut=(pNetObject)ClientList->Next(FSL8);
// 				if(pElementOut==NULL) {TimeClick=0; break; }
// 			}
// 			else break;
// 		}
// 
// 		if(pack->cnt)
// 		{
// 			ODataFF(0,(PUCHAR)pack,sizeof(PackNetObj));
// 		}
// 		else
// 		{
// //			pmas=(PUCHAR)pack; 
// 		}
// 	};
	return rc;
}


int NetService::DataFF(int N_Pin,UCHAR* pmas,size_t Cnt)
{
// 	pElSockCliEx pClient;
// 	pElSockSrvEx pServer;
// //	MyLock(__FILE__,__LINE__,&Fl_RE,this);
// 
// 	//INT64 localClick1,localClick2;
// 	//pNetObject pElementClick=NULL;
// 	//localClick1=GetNClk();
// 
 	int rc,OCnt=0;
// 	if(State) //if(State==1)
// 
// 	if(
// 		(StopProcessing==0) && (State) ||
// 		(StopProcessing==1) && (State==1)
// 		)
// 	{
// 		switch(N_Pin)
// 		{
// 		case 0:
// 			while(Cnt>0)
// 			{
// 				pack=(pPackNetObj)pmas;
// 				pmas+=sizeof(PackNetObj);
// 				Cnt-=sizeof(PackNetObj);
// 
// 				memcpy(&mypack,pack,sizeof(PackNetObj));
// 				pElement=(pNetObject)ClientList->Find(
// 					(PUCHAR)pack->descriptor,
// 					strlen(pack->descriptor));
// 				
// 				if(pElement==NULL)
// 				{
// 					if(pack->command==NetSockNew)
// 					{
// 						Debug(pack->descriptor,"create");
// 						if(pack->descriptor[0]!='_')
// 						{	// client
// 							pClient=new ElSockCliEx(ObjectClient,pack->descriptor,Srv,this);
// 							pElement=pClient; if(pElement==NULL) continue;
// 							//pElement->Start();
// 							ClientList->Add(pElement);MonClientList->Add(pElement); Counter++;
// 							continue; // !
// 						}
// 						else
// 						{	// server
// 							pServer=new ElSockSrvEx(ObjectServer,pack->descriptor,pack->NetType,Srv,this);
// 							pElement=pServer; if(pElement==NULL) continue;
// 							ClientList->Add(pElement);MonClientList->Add(pElement); Counter++;
// 							continue; // !
// 						}
// 					}
// 					else 
// 					{
// //						MyUnLock(__FILE__,__LINE__,&Fl_RE,this);
// 						return 0;
// 					}
// 				}
// 				if (pElement)
// 				{
// 					switch(pack->command)
// 					{
// 					case NetSockDel:
// 							Debug(pack->descriptor,"NetSockDel");
// 							MonClientList->Remove(pElement);
// 							rc=ClientList->Del(pElement);
// 							if(rc!=-1) Counter--;
// 							pElement=NULL;
// 							break;
// 					case NetSockParam:
// 							//if(!strcmp(pack->param,"ServerIP")) strcpy(pack->host,pack->vol);
// 							Debug(pack->descriptor,"NetSockParam",pack->param,pack->vol);
// 							pElement->SetParam(pack->param,pack->vol);
// 							pElement->PackCtrl++;
// 							break;
// 					case NetSockStart:
// 							Debug(pack->descriptor,"NetSockStart");
// 							pElement->Start();
// 							pElement->PackCtrl++;
// 							break;
// 					case NetSockStop:
// 							Debug(pack->descriptor,"NetSockStop");
// 							pElement->Stop();
// 							pElement->PackCtrl++;
// 							break;
// 					case NetSockData:
// 							pElement->PackOut++;
// 							pElement->CountOut+=pack->cnt;
// 							if(pElement->TypeObj==ObjectClient)
// 							{
// 								// without pack->pin
// 								pClient=(pElSockCliEx)pElement;
// 								pClient->CalcClick(0);
// 								pClient->SendBuf((PCHAR)pack->buf,pack->cnt);
// 								pClient->CalcClick(1);
// 							}
// 							else if(pElement->TypeObj==ObjectServer)
// 							{
// 								pServer=(pElSockSrvEx)pElement;
// 								switch(pack->pin)
// 								{
// 								case 0:	// NumCli
// 										pServer->CurrentClient=*(int*)(pack->buf);
// 										if(pServer->CurrentClient>=NumSockCli) 
// 											pServer->CurrentClient=-1;
// 										break;
// 								case 1:	// Transmit
// 										pServer->CalcClick(0);
// 										pServer->SendBuf(
// 											pServer->CurrentClient,
// 											pack->buf,pack->cnt);
// 										pServer->CalcClick(1);
// 										Srv->Telemetr(this,"Load_t/T%",100.*pServer->sLoadOutt/pServer->sLoadOutT);
// 										break;
// 								}
// 							}
// 							break;
// 					}
// 				}
// 			}
// 			break;
// 		}
// 	};
// //	MyUnLock(__FILE__,__LINE__,&Fl_RE,this);
 	return OCnt;
}

int NetService::SetElemProperties(pPackNetObj ptr)
{
// 	pElementOut=(pNetObject)ClientList->Find(
// 		(PUCHAR)ptr->descriptor,
// 		strlen(ptr->descriptor));
// 	
// 	if(pElementOut) pElementOut->FlagSelectProc=1;
	return 0;
}
int NetService::SetElemForOutData(pPackNetObj ptr)
{
// 	pElementOut=(pNetObject)ClientList->Find(
// 		(PUCHAR)ptr->descriptor,
// 		strlen(ptr->descriptor));
// 
// 	if(pElementOut->FlagSelectProc)
// 	{
// 		return (int)pElementOut; // может быть NULL
// 	}
// 	else
	{
		return 0;
	}
}
/*
int NetService::OutData(int N_Pin,UCHAR* &pmas,size_t &Cnt)
{
	pElSockCliEx pClient;
	pElSockSrvEx pServer;
	char s[256]; pmas=0;
	Cnt=0;
	if(State!=0)
	{
		pack=&mypack;
		if(pElementOut==NULL)
		{
			pElementOut=(pNetObject)ClientList->Begin(FSL8);
		}
		if (pElementOut==NULL)
		{
			return 0;
		}
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
					return 0;
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
						Debug(pack->descriptor,"Connect","HostName",pClient->HostName);
					}
					else if(*(int*)pack->buf==CBE_DisCon)
					{
						pClient->Param=0;
						Debug(pack->descriptor,"Disconnect","HostName",pClient->HostName);
						pElementOut=NULL;
					}
					break;
				}
				pack->pin=0; // read socket
				pClient->CalcClick(2);
				pClient->ReadSock(pack->buf, pack->cnt);
				pClient->CalcClick(3);
				if(pack->cnt)
				{
					pClient->PackIn++;
					pClient->CountIn+=pack->cnt;
					break;
				}
			}

			else if(pElementOut->TypeObj==ObjectServer)
			{
				pServer=(pElSockSrvEx)pElementOut;
				// -------------------------------------------------------
				
				pack->pin=0; // num client
				if(pack->pin==StateOut)
				{
					pMess=pServer->RdEvent();
					if(pMess)
					{
						if (pMess->Event!=1) pack->numinlist=pMess->Number;
							else pack->numinlist=-1;
						pack->buf=(UCHAR*)&pMess->Number;
						pack->cnt=sizeof(pMess->Number);
						StateOut=1;
						sprintf(s,"%d",pMess->Number); Debug2(s); //  [5/27/2005]
						break;
					}
					else
					{
					NextClient:
					if (CntCliPack++>5000)
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
							pack->buf=(UCHAR*)&pServer->CurrentClient;
							pack->cnt=sizeof(pServer->CurrentClient);
							pServer->PackIn++;
							pServer->CountIn+=SockCnt;
							StateOut=4;
							break;
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
					if(1) // паранойя
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
						sprintf(s,"event connect"); Debug2(s); //  [5/27/2005]
					}
					else 
					{
						StateOut=0;
						Debug(pack->descriptor,"Disconnect");
						pServer->Param--;
						sprintf(s,"event disconnect"); Debug2(s); //  [5/27/2005]
					}

					break;
				}
				// -------------------------------------------------------
				pack->pin=2; // host IP
				if(pack->pin==StateOut)
				{
					if(1) // паранойя
					{
						pack->handle=pElementOut->handle;
						strcpy(pack->descriptor,pElementOut->Descriptor);
						pack->command=NetSockOutData; pack->cnt=0;
					}

					pack->numinlist=pMess->Number;		
					pack->buf=(PUCHAR)&pMess->IPhost;
					pack->cnt=sizeof(pMess->IPhost);
					StateOut=3; 
					sprintf(s,"ip-address %s",pMess->IPhost); Debug2(s); //  [5/27/2005]
					break;
				}
				// -------------------------------------------------------
				pack->pin=3; // host name
				if(pack->pin==StateOut)
				{
					pack->numinlist=pMess->Number;		
					pack->buf=(PUCHAR)&pMess->namehost;
					pack->cnt=sizeof(pMess->namehost);
					StateOut=0;
					Debug(pack->descriptor,"Connect",
						(PCHAR)&pMess->IPhost,
						(PCHAR)&pMess->namehost,
						pMess->Number);
					sprintf(s,"hostname %s",pMess->namehost); Debug2(s); //  [5/27/2005]
					sprintf(s,"%d - %d",pMess->port,pMess->Handle); Debug2(s); //  [5/27/2005]
					break;
				}
				// -------------------------------------------------------
				pack->pin=4; // read sock
				if(pack->pin==StateOut)
				{
					pack->buf=SockBuf;
					pack->cnt=SockCnt;
					StateOut=0; break;

				}
				// -------------------------------------------------------
			} 
			
NextClientJump:
			if(TimeClick) // будем ли искать остальных клиентов?
			{
				pElementOut=(pNetObject)ClientList->Next(FSL8);
				if(pElementOut==NULL) {TimeClick=0; break; }
			}
			else break;
		} // while

		if(pack->cnt)
		{
			pmas=(PUCHAR)pack; 
			Cnt=sizeof(PackNetObj);
		}
		else
		{
			pmas=(PUCHAR)pack; 
		}
	};
	return Cnt;
}
*/
int	NetService::Debug(char *src,char *mess,char *prm,char *vol,int q)
{
//	MyLock(__FILE__,__LINE__,&Fl_RE,this);
	try
	{
	}
	catch (...) 
	{
		char s[256];
		if( (prm!=NULL)&&(vol!=NULL) )
		{
			if(q==-255)
			{
				sprintf(s,"%s: %s(%s,%s)",src,mess,prm,vol);
			}
			else
			{
				sprintf(s,"%s[%2d]: %s(%s,%s)",src,q,mess,prm,vol);
			}
			
		}
		else
		{
			sprintf(s,"%s: %s",src,mess);
		}
		Srv->OutLog(s,this,"trace");
	};
//	MyUnLock(__FILE__,__LINE__,&Fl_RE,this);
	return 0;
}

int	NetService::Debug2(char* q)
{
	return 0;
	
	char s[256];
	SYSTEMTIME time;
	GetLocalTime(&time);
	sprintf(s,"%04ld-%02ld-%02ld %02ld:%02ld:%02ld.%03d  %s\r\n",
		time.wYear,time.wMonth,time.wDay,
		time.wHour,time.wMinute,time.wSecond,
		time.wMilliseconds,q);

	int h=open("\\NetService.log",O_BINARY|O_CREAT|O_WRONLY,S_IWRITE);
	if(h!=-1)
	{
		lseek(h,0,SEEK_END);
		write(h,s,strlen(s));
		close(h);
	}

	return 0;
}

int NetService::GetTelemetry()
{
// 	if(++TelemetryCount==100)
// 	{
// 		TelemetryCount=0;
// 	
// 		char s[256];
// 		pNetObject pLocalElement;
// 		pLocalElement=GetTelemetryPtr(1);
// 		sprintf(s,"-------------------------------------------"); WriteToLog(s);
// 		while(1)
// 		{
// 			if(pLocalElement!=NULL)
// 			{
// 				sprintf(s,"%8s: %2d, %2d, %4d, %8dk, %4d, %8dk",
// 					pLocalElement->Descriptor,
// 					pLocalElement->PackCtrl,
// 					pLocalElement->Param,
// 
// 					pLocalElement->PackIn,
// 					pLocalElement->CountIn>>10,
// 
// 					pLocalElement->PackOut,
// 					pLocalElement->CountOut>>10
// 					);
// 				WriteToLog(s);
// 			}
// 			else break;
// 			pLocalElement=GetTelemetryPtr();
// 		}
// 		sprintf(s,""); WriteToLog(s);
// 	};
	return 0;
}
pNetObject NetService::GetTelemetryPtr(int first)
{
// 	if(first) return (pNetObject)ClientList->Begin(FSL8);
// 	else return (pNetObject)ClientList->Next(FSL8);
	return NULL;
}

// #define TraceEnable

int NetService::WriteToLog(char *mess)
{
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
		write(h,s,strlen(s));

		close(h);
	};
	return 0;
}

int NetService::OutLog(int q, char *mess)
{
	char s[256];
	sprintf(s,"%d - %s",q,mess);
	Srv->OutLog(s,this,"SI");
	return 0;
}


TestSockCliN::TestSockCliN()
{
// Этот фрагмент должен быть вставлен во все последующие объекты
// TestSockCliN_ должен бить заменен на имя объекта
		 Name_Mt=TestSockCliN_NameMt;Rem_Mt=TestSockCliN_RemMt;Ver_=TestSockCliN_Version;Auth_=TestSockCliN_Author;
		 PinName_I=TestSockCliN_PinNameI;PinName_O=TestSockCliN_PinNameO;Prm_=TestSockCliN_Prm;Vol_=TestSockCliN_Vol;

		 Num_PinI=0;while (PinName_I[2*Num_PinI][0]!=0x0) Num_PinI++;
		 Num_PinO=0;while (PinName_O[2*Num_PinO][0]!=0x0) Num_PinO++;
		 Num_Prm=0;while (Prm_[2*Num_Prm][0]!=0x0)Num_Prm ++;
// -------------------------------------------------------------
	State=0;
	Counter=0;

	pack=NULL;
	buf=NULL; cnt=0;

}
TestSockCliN::~TestSockCliN()
{
}

void TestSockCliN::Param(char* Prm,char* Vol)
{
}

void TestSockCliN::Start()
{
	State=1;
}

void TestSockCliN::Stop()
{
	State=2;
}

/*
enum NetSockCommand
{
	NetSockNew,
	NetSockDel,
	NetSockParam,
	NetSockStart,
	NetSockStop,
	NetSockData,
	NetSockOutData
};
struct PackNetObj
{
	char descriptor[32];
	int command;
	PCHAR param,vol;
	int pin;
	PUCHAR buf;
	int cnt;
};
*/

int TestSockCliN::Data(int N_Pin,UCHAR* pmas,size_t Cnt)
{
	int h,OCnt=0;
	if(State==1)
	{
		switch(N_Pin)
		{
		case 0:
				pack=(pPackNetObj)pmas;

				char s[256];
				sprintf(s,"Debug.txt");
				h=open(s,O_BINARY|O_CREAT|O_WRONLY,S_IWRITE);
				if(h!=-1)
				{
					lseek(h,0,SEEK_END);
					sprintf(s,"%16s %4d %4d %4d\r\n",
						pack->descriptor,pack->pin,pack->buf[0],pack->cnt);
					write(h,s,strlen(s));
					close(h);
				}

				break;
		case 1:
				if(Counter==0)
				{
						currnet=0;
						pack=&mypack[currnet++];
						strcpy(pack->descriptor,"FileSrv1");
						pack->command=NetSockNew;

						pack=&mypack[currnet++];
						strcpy(pack->descriptor,"FileSrv1");
						pack->command=NetSockParam;
						strcpy(pack->param,"ServerIP");
						strcpy(pack->vol,"192.168.44.32");

						pack=&mypack[currnet++];
						strcpy(pack->descriptor,"FileSrv1");
						pack->command=NetSockParam;
						strcpy(pack->param,"ServerPort");
						strcpy(pack->vol,"499");

						pack=&mypack[currnet++];
						strcpy(pack->descriptor,"FileSrv1");
						pack->command=NetSockParam;
						strcpy(pack->param,"SizeTrBuf");
						strcpy(pack->vol,"262144");
						
						pack=&mypack[currnet++];
						strcpy(pack->descriptor,"FileSrv1");
						pack->command=NetSockParam;
						strcpy(pack->param,"Mode");
						strcpy(pack->vol,"BufPack");
						
						pack=&mypack[currnet++];
						strcpy(pack->descriptor,"FileSrv1");
						pack->command=NetSockStart;

						buf=(PUCHAR)mypack;
						cnt=currnet*sizeof(PackNetObj);
				}
				else if(Counter==1)
				{
						currnet=0;
						pack=&mypack[currnet++];
						strcpy(pack->descriptor,"FileSrv2");
						pack->command=NetSockNew;

						pack=&mypack[currnet++];
						strcpy(pack->descriptor,"FileSrv2");
						pack->command=NetSockParam;
						strcpy(pack->param,"ServerIP");
						strcpy(pack->vol,"192.168.44.32");

						pack=&mypack[currnet++];
						strcpy(pack->descriptor,"FileSrv2");
						pack->command=NetSockParam;
						strcpy(pack->param,"ServerPort");
						strcpy(pack->vol,"499");

						pack=&mypack[currnet++];
						strcpy(pack->descriptor,"FileSrv2");
						pack->command=NetSockParam;
						strcpy(pack->param,"SizeTrBuf");
						strcpy(pack->vol,"262144");
						
						pack=&mypack[currnet++];
						strcpy(pack->descriptor,"FileSrv2");
						pack->command=NetSockParam;
						strcpy(pack->param,"Mode");
						strcpy(pack->vol,"BufPack");
						
						pack=&mypack[currnet++];
						strcpy(pack->descriptor,"FileSrv2");
						pack->command=NetSockStart;

						buf=(PUCHAR)mypack;
						cnt=currnet*sizeof(PackNetObj);
				}
				else if(Counter==2)
				{
						currnet=0;
						pack=&mypack[currnet++];
						strcpy(pack->descriptor,"FileSrv3");
						pack->command=NetSockNew;

						pack=&mypack[currnet++];
						strcpy(pack->descriptor,"FileSrv3");
						pack->command=NetSockParam;
						strcpy(pack->param,"ServerIP");
						strcpy(pack->vol,"192.168.44.32");

						pack=&mypack[currnet++];
						strcpy(pack->descriptor,"FileSrv3");
						pack->command=NetSockParam;
						strcpy(pack->param,"ServerPort");
						strcpy(pack->vol,"499");

						pack=&mypack[currnet++];
						strcpy(pack->descriptor,"FileSrv3");
						pack->command=NetSockParam;
						strcpy(pack->param,"SizeTrBuf");
						strcpy(pack->vol,"262144");
						
						pack=&mypack[currnet++];
						strcpy(pack->descriptor,"FileSrv3");
						pack->command=NetSockParam;
						strcpy(pack->param,"Mode");
						strcpy(pack->vol,"BufPack");
						
						pack=&mypack[currnet++];
						strcpy(pack->descriptor,"FileSrv3");
						pack->command=NetSockStart;

						buf=(PUCHAR)mypack;
						cnt=currnet*sizeof(PackNetObj);
				}
				Counter++;
				OCnt=cnt;
				break;
		}
	}
	return OCnt;
}

int TestSockCliN::OutData(int N_Pin,UCHAR* &pmas,size_t &Cnt)
{
	Cnt=0;
	if(State!=0)
	{
		if(cnt)
		{
			pmas=buf;
			Cnt=cnt;

			cnt=0;
		}
	}
	return Cnt;
}


















/*

	SockSrv::SockSrv()
{
		TimeOut=15000;
		WrMsg=0;RdMsg=0;Fl_EvCon=0;Fl_Pack=0;
		CB_Size=10240;
		Act_Cli=-1;
		Mode=0;
		Fl_Data=0;
		Port=500;
		SrvSock=new ServerSockEx(Port,EventFun,100);
		SrvSock->SizeCB(CB_Size);
		SrvSock->ID_Srv=(int)this;
		SrvSock->SetMode(Mode);
		SrvSock->TimeOut(TimeOut);
		tDiv=0;SetDiv=10;
		Fl_Stop=1;
}
	SockSrv::~SockSrv()
{
		delete	SrvSock;
}

void	SockSrv::Param(char* Prm,char* Vol)
{
	int	rc=0;
	if (strcmp(Prm,"Mode")==0) 
	{
		if (strcmp(Vol,"NoBufNoPack")==0) Mode=0;
		if (strcmp(Vol,"BufNoPack")==0) Mode=1;
		if (strcmp(Vol,"BufPack")==0) Mode=2;
		SrvSock->SetMode(Mode);
	}
	if (strcmp(Prm,"ServerPort")==0) {rc=1; Port=atoi(Vol);}
	if (strcmp(Prm,"SizeTrBuf")==0) {CB_Size=atoi(Vol);SrvSock->SizeCB(CB_Size);}
	if (strcmp(Prm,"BlockTimeOut")==0) {TimeOut=atoi(Vol);SrvSock->TimeOut(TimeOut);}
	if (rc==1)
	{
		delete	SrvSock;
		SrvSock=new ServerSockEx(Port,EventFun,100);
		SrvSock->SizeCB(CB_Size);
		SrvSock->ID_Srv=(int)this;
		SrvSock->SetMode(Mode);
		SrvSock->TimeOut(TimeOut);
	}
}

void	SockSrv::WrEvent(ServerClientSock* csock,int Event)
{
	EvMsg[WrMsg].Number=csock->NumInList;
	EvMsg[WrMsg].csock=csock;
	EvMsg[WrMsg].Event=Event;
	if (++WrMsg>=100) WrMsg=0;
}

EventMsg*	SockSrv::RdEvent()
{
	EventMsg	*rc=0;
	if (WrMsg!=RdMsg)
	{
		rc=&EvMsg[RdMsg];
		if (++RdMsg>=100) RdMsg=0;
	}
	return rc;
}

int	SockSrv::Data(int N_Pin,UCHAR* pmas,int Cnt)
{
	int	OCnt=0;
	if (Fl_Stop!=1)
	switch(N_Pin)
	{
	case 0:
		Act_Cli=*(int*)pmas;
		if (Act_Cli>=100)
			Act_Cli=-1;
		Fl_Data=1;OCnt=Cnt;tActCli=0;
		break;
	case 1:
		SrvSock->SendBuf(Act_Cli,pmas,Cnt);
		break;
	case 2:
			Fl_Data=1;OCnt=Cnt;tActCli=0;
		break;
	case 3:
		Param("ServerPort",(char*)pmas);
		break;
	}
return OCnt;
}

int	SockSrv::OutData(int N_Pin,UCHAR* &pmas,int &Cnt)
{
	Cnt=0;
	if (Fl_Data==1)
	{
		switch(N_Pin)
		{
		case 0:
			tEMsg=RdEvent();
			if(tEMsg!=0)
			{
				pmas=(UCHAR*)&tEMsg->Number;
				Cnt=sizeof(tEMsg->Number);
			}
			else
			{
_mm1:			SrvSock->ReadSock(tActCli,tBuf,tCnt);
				if (tCnt==0)
				{
					if (++tActCli<100)
					{
						goto _mm1;
					}
					else
					{
						Fl_Data=0;				
					}
				}
				else
				{
					pmas=(UCHAR*)&tActCli;Cnt=sizeof(tActCli);
					Fl_Pack=1;
				}
			}
			break;
		case 1:	// Событие
			if(tEMsg!=0)
			{
				pmas=(UCHAR*)&tEMsg->Event;Cnt=sizeof(tEMsg->Event);
				if (tEMsg->Event==CBE_Connect) Fl_EvCon=1;
			}
			break;
		case 2: // IP
			if(Fl_EvCon==1)
			{
				pmas=(UCHAR*)&tEMsg->csock->IPhost;	// IP string
				Cnt=sizeof(tEMsg->csock->IPhost);
			}
			break;
		case 3: // Name
			if(Fl_EvCon==1)
			{
				pmas=(UCHAR*)&tEMsg->csock->namehost;	// name host
				Cnt=sizeof(tEMsg->csock->namehost);
				Fl_EvCon=0;
			}
			break;
		case 4:
			if(Fl_Pack==1)
			{
				pmas=tBuf;Cnt=tCnt;
				Fl_Pack=0;
			}
			break;
		}
	}
return Cnt;
}
*/
