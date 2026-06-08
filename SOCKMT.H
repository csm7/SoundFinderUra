#ifndef _SockMt
#define _SockMt
// ------------------
#include	"DecodB.h"

//#include	<TCPIP.h>
//#include	<SockLib.h>
// ------------------
// Структура для получения информации от декодера
// NetAdapterOut_ = наименование объекта_
static char	NetAdapterOut_NameMt[]= "NetAdapterOut";
static char	NetAdapterOut_Version[]= "V1.0";
static char	NetAdapterOut_Author[]= "SystemSoft(Y)";
static char	NetAdapterOut_RemMt[]= "Адаптер сетевого взаимодействия для передачи данных по 8 ножкам удаленному клиенту (серверу)";
static char	*NetAdapterOut_PinNameI[]=
{
	"PinI1","Вход 1",
	"PinI2","Вход 2",
	"PinI3","Вход 3",
	"PinI4","Вход 4",
	"PinI5","Вход 5",
	"PinI6","Вход 6",
	"PinI7","Вход 7",
	"PinI8","Вход 8",
	""
};
static char	*NetAdapterOut_PinNameO[]=
{
	"DataOut","Преобразованные данные",
	""
};

static char	*NetAdapterOut_Prm[]=
{
	""
};

static char	*NetAdapterOut_Vol[]=
{
	""
};
// ----------------------------------------------
class	NetAdapterOut : public	Decoder
{
public:
int		Fl_Out,tPin,tCnt;

UCHAR*	tBuf;
		NetAdapterOut();
		~NetAdapterOut();
virtual	void	Param(			// Изменить параметр
			char* Prm,	// Идентификатор параметра
			char* Vol);	// Значение параметра
virtual void	Start();		// Начало сеанса
virtual void	Stop();			// Конец сеанса
virtual	int	Data(			// Данные для декодирования
			int N_Pin,	// Номер входной ножки
			UCHAR* pmas,	// Указатель на буфер с данными
			size_t Cnt);	// Длина данных
virtual	int	OutData(			// Выходные данные
			int N_Pin,	// Номер выходной ножки
			UCHAR* &pmas,	// Указатель на буфер с данными
			size_t &Cnt);	// Указатель на длину данных

};

// Структура для получения информации от декодера
// NetAdapterIn_ = наименование объекта_
static char	NetAdapterIn_NameMt[]= "NetAdapterIn";
static char	NetAdapterIn_Version[]= "V1.0";
static char	NetAdapterIn_Author[]= "SystemSoft(Y)";
static char	NetAdapterIn_RemMt[]= "Адаптер сетевого взаимодействия для приема данных от удаленного клиента (сервера) и преобразование их в 8 выходов";
static char	*NetAdapterIn_PinNameI[]=
{
	"DataIn","Входные данные от сетевого клиента (сервера)",
	""
};
static char	*NetAdapterIn_PinNameO[]=
{
	"PinO1","Выход 1",
	"PinO2","Выход 2",
	"PinO3","Выход 3",
	"PinO4","Выход 4",
	"PinO5","Выход 5",
	"PinO6","Выход 6",
	"PinO7","Выход 7",
	"PinO8","Выход 8",
	""
};

static char	*NetAdapterIn_Prm[]=
{
	""
};

static char	*NetAdapterIn_Vol[]=
{
	""
};
// ----------------------------------------------
class	NetAdapterIn : public	Decoder
{
public:
int		Fl_Out,tPin,tCnt,Sm;
int		FullCnt;
UCHAR*	tBuf;
UCHAR*	OutBuf;
int		RealSize;
		NetAdapterIn();
		~NetAdapterIn();
virtual	void	Param(			// Изменить параметр
			char* Prm,	// Идентификатор параметра
			char* Vol);	// Значение параметра
virtual void	Start();		// Начало сеанса
virtual void	Stop();			// Конец сеанса
virtual	int	Data(			// Данные для декодирования
			int N_Pin,	// Номер входной ножки
			UCHAR* pmas,	// Указатель на буфер с данными
			size_t Cnt);	// Длина данных
virtual	int	OutData(			// Выходные данные
			int N_Pin,	// Номер выходной ножки
			UCHAR* &pmas,	// Указатель на буфер с данными
			size_t &Cnt);	// Указатель на длину данных

};


// Структура для получения информации от декодера
// SockCli_ = наименование объекта_
static char	SockCli_NameMt[]= "SockCli";
static char	SockCli_Version[]= "V1.0";
static char	SockCli_Author[]= "SystemSoft (Y) и Ко";
static char	SockCli_RemMt[]= "Клиент сокета TCP/IP";
static char	*SockCli_PinNameI[]=
{
	"PackTr","Пакет данных для передачи",
	"TimeClk","Тики таймера",
	"SetIP","Установить адрес",
	"SetPort","Установить порт",
	""
};
static char	*SockCli_PinNameO[]=
{
	"Event","Сообщения о состояниях клиента",
	"PackRes","Принимаемый пакет",
	""
};

static char	*SockCli_Prm[]=
{
	"Mode","Режим работы клиента",
	"ServerIP","IP адрес сервера",
	"ServerPort","Порт сервера",
	"SizeTrBuf","Размер передающего буфера",
	"BlockTimeOut","Таймаут ожидания в блокирующем режиме",
	"DataBuf","Режим буферизации данных при дисконекте",
	""
};

static char	*SockCli_Vol[]=
{
	"Mode","NoBufNoPack","Режим передачи данных без буферизации и без пакетной организации",
	"Mode","BufNoPack","Режим передачи данных с буферизацией и без пакетной организации",
	"Mode","BufPack","Режим передачи данных с буферизацией и пакетной организацией обмена",
	"ServerIP","127.0.0.1","IP адрес сервера",
	"ServerIP","192.168.44.22","IP адрес сервера",
	"ServerIP","192.1.1.73","IP адрес сервера",
	"ServerPort","500","Порт сервера",
	"SizeTrBuf","10240","~10k",
	"SizeTrBuf","102400","~100k",
	"SizeTrBuf","1024","~1k",
	"BlockTimeOut","15000","10~15 сек",
	"DataBuf","Off","Режим буферизации данных при дисконекте выключен",
	"DataBuf","On","Режим буферизации данных при дисконекте выключен",
	""
};
// ----------------------------------------------
// ----------------------------------------------
class	SockCli : public Decoder
{
public:
	int		Fl_Stop;
	int		TimeOut;
	int		CB_Size;
	int		Fl_Conn;
	int		Port;
	char	IPAddr[80];
	ClientSockEx*	CliSock;
	int		Mode;
	int		tDiv,SetDiv;
	EventMsg	tEMsg;

	HANDLE	hThConn;
	DWORD	*TID_Conn;
	int		StartConn;
	PackStr		WatchDogPack;
//	int		Fl_DB;
	INT64	CE_Conn;
// -----------------------
		SockCli();
		~SockCli();
virtual	void	Param(			// Изменить параметр
			char* Prm,	// Идентификатор параметра
			char* Vol);	// Значение параметра
virtual void	Start();		// Начало сеанса
virtual void	Stop();			// Конец сеанса
virtual	int	Data(			// Данные для декодирования
			int N_Pin,	// Номер входной ножки
			UCHAR* pmas,	// Указатель на буфер с данными
			size_t Cnt);	// Длина данных
virtual	int	OutData(			// Выходные данные
			int N_Pin,	// Номер выходной ножки
			UCHAR* &pmas,	// Указатель на буфер с данными
			size_t &Cnt);	// Указатель на длину данных

};

typedef SockCli *pSockCli;


// ---------------------------------------------------------------------------
// ---------------------------------------------------------------------------

class	ElSockCli : public ID_Elem, public SockCli
{
public:
	char Descriptor[256];
	LPVOID handle;

	ElSockCli(char *value) :
		ID_Elem((PUCHAR)Descriptor,
			strlen(value)), SockCli()
{
	strcpy(Descriptor,value);
	handle=this;
};	
virtual	~ElSockCli(){};
};
typedef ElSockCli *pElSockCli;

// ---------------------------------------------------------------------------
// ---------------------------------------------------------------------------

// Структура для получения информации от декодера
// SockCli_ = наименование объекта_
static char	SockCliN_NameMt[]= "SockCliN";
static char	SockCliN_Version[]= "V1.0";
static char	SockCliN_Author[]= "DY,Y & Ko";
static char	SockCliN_RemMt[]= "Сетевой интерфейс TCP/IP";
static char	*SockCliN_PinNameI[]=
{
	"PackNetObj","пакетный протокол сетевого интерфейса",
	"TimeClick","TimeClick",
	""
};
static char	*SockCliN_PinNameO[]=
{
	"PackNetObj","пакетный протокол сетевого интерфейса",
	""
};

static char	*SockCliN_Prm[]=
{
	""
};

static char	*SockCliN_Vol[]=
{
	""
};

class	SockCliN : public	Decoder
{
public:

	int	State;
	int Counter;
	pID_List8	ClientList;	
	pElSockCli	pElement;
	pElSockCli	pCurrnet;
	PackNetObj mypack;
	pPackNetObj pack;
	int nPinOut;
	int TimeClick;
	FindStrList8	FSL8;
			SockCliN();
			~SockCliN();
virtual	void Param(			// Изменить параметр
			char* Prm,	// Идентификатор параметра
			char* Vol);	// Значение параметра
virtual void Start();		// Начало сеанса
virtual void Stop();			// Конец сеанса
virtual	int	Data(			// Данные для декодирования
			int N_Pin,	// Номер входной ножки
			UCHAR* pmas,	// Указатель на буфер с данными
			size_t Cnt);	// Длина данных
virtual	int	OutData(			// Выходные данные
			int N_Pin,	// Номер выходной ножки
			UCHAR* &pmas,	// Указатель на буфер с данными
			size_t &Cnt);	// Указатель на длину данных

};

// ---------------------------------------------------------------------------
// int fCallBack(ClientSock* S,int Event);
// ---------------------------------------------------------------------------

// class	ElSockCliEx	:	public NetObject, public ClientSockEx
// {
// public:
// 	int State;
// 	int Enable;
// 	int FlagEventArray[1024];
// 	int index_w, index_r, index_m;
// 	EventMsg tEMsg;
// 
// 	char	IPAddr[80];
// 	int		Port;
// 
// 	HANDLE	hThConn;
// 	DWORD	*TID_Conn;
// 	int		StartConn;
// 	int		WatchDogPack;
// 	int		tDiv,SetDiv;
// 
// 	int DebugCount;
// 
// 	ElSockCliEx(int tObj, char *value, Services *pSrv=NULL,Decoder* pDec=NULL) :
// 		NetObject(tObj, value,pSrv), 
// 		ClientSockEx("127.0.0.1",500,pSrv,pDec)
// {
// 	State=0;
// 	Enable=0;
// 	//FlagEvent=0;
// 	StartConn=0;
// 
// 	Mode=0;
// 	TimeOut=10000;
// 	SizeCB(262144);
// 	strcpy(IPAddr,"127.0.0.1");Port=500;
// 	WatchDogPack=COM_WatchDog;
// 	tDiv=0;SetDiv=500;
// 	CallBackEvent=fCallBack;
// 
// 	index_w=index_r=0; index_m=0x3ff;
// 	memset(FlagEventArray,0,sizeof(FlagEventArray));
// 
// 	DebugCount=0;
// };	
// virtual	~ElSockCliEx()
// 		{
// 			while (StartConn==1)
// 				Sleep(10);
// 			if (StartConn==2)
// 			{
// 				StartConn=0;
// 				CloseHandle(hThConn);
// 			}
// 		};
// virtual	int SetParam(char* Prm,char* Vol);
// virtual	int	RdEvent(PUCHAR &,size_t &);
// virtual	int Start();
// virtual	int Stop();
// virtual	int	ObjIdle();
// virtual	int	GetHostName();
// virtual	int	SetEvent(int Event);
// virtual	int	Debug(char *q);
// };
// typedef ElSockCliEx *pElSockCliEx;
// 
// int	ElSockSrvEx_EventFun(ServerClientSock *csock,int Event);
// class	ElSockSrvEx	:	public NetObject, public ServerSockEx
// {
// public:
// 	int FlagEvent;
// 
// 	int WrMsg,RdMsg;
// 	EventMsg EvMsg[1024];
// 	int	WatchDogPack;
// 	int	tDiv,SetDiv;
// 
// 
// 	ElSockSrvEx(int tObj, char *value,int NetType, Services *pSrv=NULL,Decoder* pDec=NULL) :
// 		NetObject(tObj, value,pSrv), 
// 		ServerSockEx(500,ElSockSrvEx_EventFun, NetType, NumSockCli, pSrv,pDec)
// {
// 	FlagEvent=0;
// 
// 	Mode=0;
// 	TimeOut(10000);
// 	SizeCB(262144);
// 	ID_Srv=NULL;
// //	strcpy(IPAddr,"127.0.0.1");
// //	Port=500;
// 	WrMsg=RdMsg=0;
// 	WatchDogPack=COM_WatchDog;
// 	tDiv=0;SetDiv=500;
// };	
// virtual	~ElSockSrvEx(){};
// virtual	int SetParam(char* Prm,char* Vol);
// virtual	int WrEvent(ServerClientSock* csock,int Event);
// virtual	EventMsg* RdEvent();
// virtual	int Start();
// virtual	int Stop();
// virtual	int	ObjIdle();
// };
// typedef ElSockSrvEx *pElSockSrvEx;
// 
// ---------------------------------------------------------------------------


// Структура для получения информации от декодера
// NetService_ = наименование объекта_
static char	NetService_NameMt[]= "NetService";
static char	NetService_Version[]= "V1.0";
static char	NetService_Author[]= "DY,Y & Ko";
static char	NetService_RemMt[]= "Сокет TCP/IP (клиент и сервер)";
static char	*NetService_PinNameI[]=
{
	"PackNetObj","пакетный протокол сетевого интерфейса",
//	"TimeClick","TimeClick",
	""
};
static char	*NetService_PinNameO[]=
{
	"PackNetObj","пакетный протокол сетевого интерфейса",
	""
};

static char	*NetService_Prm[]=
{
	"StopProcessing","обработка сигнала Sop",
	""
};

static char	*NetService_Vol[]=
{
	"StopProcessing","No","",
	"StopProcessing","Yes","",
	""
};

/*
	"Mode","Режим работы клиента",
	"ServerIP","IP адрес сервера",
	"ServerPort","Порт сервера",
	"SizeTrBuf","Размер передающего буфера",
	"BlockTimeOut","Таймаут ожидания в блокирующем режиме",
*/

class	NetService : public	Decoder
{
public:

// 	int				State;
// 	int				Counter;
// 	int				TimeClick; 
// 	pID_List8		ClientList;	
// 	pID_List8		MonClientList;	
// 	pNetObject		pElement;
// 	pNetObject		pCurrnet;
// 	pPackNetObj		pack;
// 	pNetObject		pElementOut;
// 	PackNetObj		mypack;
// 	// -----------------------------------
// 	int				StateOut;
// 	EventMsg		*pMess;
// 	PUCHAR			SockBuf;
// 	size_t			SockCnt;
// 	int				StopProcessing;
// 	FlagStruct		Fl_RE;
// 	int				CntCliPack;
// 	FindStrList8	FSL8;

		NetService();
		~NetService();
virtual	void	Param(			// Изменить параметр
			char* Prm,	// Идентификатор параметра
			char* Vol);	// Значение параметра
virtual void	Start();		// Начало сеанса
virtual void	Stop();			// Конец сеанса
virtual	int	DataFF(			// Данные для декодирования
			int N_Pin,	// Номер входной ножки
			UCHAR* pmas,	// Указатель на буфер с данными
			size_t Cnt);	// Длина данных
// virtual	int	OutData(			// Выходные данные
// 			int N_Pin,	// Номер выходной ножки
// 			UCHAR* &pmas,	// Указатель на буфер с данными
// 			size_t &Cnt);	// Указатель на длину данных
virtual	int	Debug(char *src,char *mess,char *prm=NULL,char *vol=NULL,int q=-255);
virtual	int IdleFunc(int NumberTr);		// Idle function  if rc!=0 recall function (?)  NumberTr=-1 System NumberTr+0x8000 - DinTr
	
	virtual int GetTelemetry();
	virtual pNetObject GetTelemetryPtr(int first=0);
	virtual int WriteToLog(char *mess);
	virtual int OutLog(int,char*);

	virtual int Debug2(char* q);
	virtual int SetElemProperties(pPackNetObj ptr);
	virtual int SetElemForOutData(pPackNetObj ptr);

};
typedef NetService *pNetService;


// ---------------------------------------------------------------------------

// Структура для получения информации от декодера
// SockCli_ = наименование объекта_
static char	TestSockCliN_NameMt[]= "TestSockCliN";
static char	TestSockCliN_Version[]= "V1.0";
static char	TestSockCliN_Author[]= "DY only";
static char	TestSockCliN_RemMt[]= "Тестирование сетевого интерфейса TCP/IP";
static char	*TestSockCliN_PinNameI[]=
{
	"PackNetObj","пакетный протокол сетевого интерфейса",
	"TimeClick","TimeClick",
	""
};
static char	*TestSockCliN_PinNameO[]=
{
	"PackNetObj","пакетный протокол сетевого интерфейса",
	""
};

static char	*TestSockCliN_Prm[]=
{
	""
};

static char	*TestSockCliN_Vol[]=
{
	""
};

class	TestSockCliN : public	Decoder
{
public:

	int	State;
	int Counter;
	PUCHAR buf;
	int cnt;

	int currnet;
	PackNetObj mypack[1000];
	pPackNetObj pack;

// -----------------------
			TestSockCliN();
			~TestSockCliN();
virtual	void Param(			// Изменить параметр
			char* Prm,	// Идентификатор параметра
			char* Vol);	// Значение параметра
virtual void Start();		// Начало сеанса
virtual void Stop();			// Конец сеанса
virtual	int	Data(			// Данные для декодирования
			int N_Pin,	// Номер входной ножки
			UCHAR* pmas,	// Указатель на буфер с данными
			size_t Cnt);	// Длина данных
virtual	int	OutData(			// Выходные данные
			int N_Pin,	// Номер выходной ножки
			UCHAR* &pmas,	// Указатель на буфер с данными
			size_t &Cnt);	// Указатель на длину данных

};

// ---------------------------------------------------------------------------
// Структура для получения информации от декодера
// SockSrv_ = наименование объекта_
static char	SockSrv_NameMt[]= "SockSrv";
static char	SockSrv_Version[]= "V1.0";
static char	SockSrv_Author[]= "SystemSoft (Y) и Ко";
static char	SockSrv_RemMt[]= "Сервер сокетов TCP/IP";
static char	*SockSrv_PinNameI[]=
{
	"NumCli","Виртуальный номер клиента",
	"PackTr","Пакет данных для передачи",
	"TimerTick","Тики таймера",
	"SetPort","Установить порт",
	""
};
static char	*SockSrv_PinNameO[]=
{
	"NumCli","Виртуальный номер клиента",
	"Event","Сообщения о состояниях сервера",
	"HostIP","IP - адресс машины клиента",
	"HostName","Имя машины клиента",
	"PackRes","Принимаемый пакет",
	""
};

static char	*SockSrv_Prm[]=
{
	"NetType","Type of network",
	"Mode","Режим работы клиента",
	"ServerPort","Порт сервера",
	"SizeTrBuf","Размер передающего буфера",
	"BlockTimeOut","Таймаут ожидания в блокирующем режиме",
	""
};

static char	*SockSrv_Vol[]=
{
	"NetType","PAN","Personal aria network",
	"NetType","LAN","Local aria network",
	"NetType","WAN","World aria network",
	"Mode","NoBufNoPack","Режим передачи данных без буферизации и без пакетной организации",
	"Mode","BufNoPack","Режим передачи данных с буферизацией и без пакетной организации",
	"Mode","BufPack","Режим передачи данных с буферизацией и пакетной организацией обмена",
	"ServerPort","500","Порт сервера",
	"SizeTrBuf","10240","~10k",
	"SizeTrBuf","102400","~100k",
	"SizeTrBuf","1024","1k",
	"BlockTimeOut","15000","10~15 сек",
	""
};
// ----------------------------------------------
class	SockSrv : public	Decoder
{
public:
		int		Fl_Stop;
		UCHAR*	tBuf;
		size_t	tCnt;
		int		TimeOut;
		int		tActCli;
		int		Fl_Data;
		int		Act_Cli;
		int		CB_Size;
		ServerSockEx*	SrvSock;
		int		Port;
		int		Mode;
//		int		Fl_Event[100];
//		int		Fl_Connect[100];
//		int		Fl_DisConnect[100];
//		int		Fl_Error[100];
		EventMsg	EvMsg[100];
		int		WrMsg;
		int		RdMsg;
		EventMsg*	tEMsg;
		int		Fl_EvCon,Fl_Pack;
		int		tDiv,SetDiv;
		PackStr		WatchDogPack;
		int		NetType;
// ----------------------------------------------
		SockSrv();
		~SockSrv();
virtual	void	Param(			// Изменить параметр
			char* Prm,	// Идентификатор параметра
			char* Vol);	// Значение параметра
virtual void	Start();		// Начало сеанса
virtual void	Stop();			// Конец сеанса
virtual	int	Data(			// Данные для декодирования
			int N_Pin,	// Номер входной ножки
			UCHAR* pmas,	// Указатель на буфер с данными
			size_t Cnt);	// Длина данных
virtual	int	OutData(			// Выходные данные
			int N_Pin,	// Номер выходной ножки
			UCHAR* &pmas,	// Указатель на буфер с данными
			size_t &Cnt);	// Указатель на длину данных


virtual int IdleFunc(int NumberTr);			// Idle function  if rc!=0 recall function (?)  NumberTr=-1 System NumberTr+0x8000 - DinTr

// ----------------------
	void		WrEvent(ServerClientSock* csock,int Event);
	EventMsg*	RdEvent();
// ----------------------
};


#endif
// ------------------
