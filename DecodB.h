//#ifndef _Neuron
//#define _Neuron
// --------------------------------
//#define		_NOSRV
//#if _MSC_VER > 1000
#pragma once
#pragma warning( error : 4700 )//uninitialized local variable 
//#endif // _MSC_VER > 1000
//////////////////////////////////////////////////////////////////////////
#define mVersion	212
#define mData		__DATE__
#define mType		"Beta"
//#define mType		"PreRelease"
//#define mType		"Release"
#define UDPSm		1
#define UDPSmN		10

#define Syn1		0x5a
//#define Syn1		0x0fcc55aa
#define Syn2		0x0fccaa55


#include <iostream>
#include <string>
#include <vector>
#include <stdio.h>
//-----------------------------------------------------------------------------
#include "json_writer.h"
#include "json_reader.h"
#include "pt.h"

//#include <Psapi.h>
/*
//#pragma comment(lib, "Psapi.lib" )
//#ifdef _WIN64
//#pragma comment(lib, "hasp_windows_x64_86256.lib" )
//#else
//#pragma comment(lib, "hasp_windows_86256.lib" )
//#endif
*/

#define	UDPPortPAN	"498"	//	"487"	//	"497"
#define	UDPPortLAN	"496"	//	"490"	//	"498"
#define	UDPPortWAN	"494"	//	"489"	//	"497"
#define TCPPort		"801"	//	"791"	//	"801"
// --------------------------------

// !!!!!!!  Â ïðîåêò íåîáõîäèìî äîáàâèòü WS2_32.LIB  !!!!!!
//#if _WIN64
//
//#else
//#pragma comment( lib, "Ws2_32.lib" )
//#pragma comment( lib, "N:\\CppLib\\hasp_windows_86256.lib" )
//#pragma comment( lib, "Shell32.lib" )
//#endif

#pragma warning(disable:4996)


typedef unsigned long ULONG;
typedef ULONG *PULONG;
typedef unsigned short USHORT;
typedef USHORT *PUSHORT;
typedef unsigned char UCHAR;
typedef UCHAR *PUCHAR;
typedef char *PSZ;
typedef signed char         INT8, *PINT8;
typedef signed short        INT16, *PINT16;
typedef signed int          INT32, *PINT32;
typedef signed __int64      INT64, *PINT64;
typedef unsigned char       UINT8, *PUINT8;
typedef unsigned short      UINT16, *PUINT16;
typedef unsigned int        UINT32, *PUINT32;
typedef unsigned __int64    UINT64, *PUINT64;

#define	MaxLenPack				522

/* exact-width signed integer types */
typedef   signed          char int8_t;
typedef   signed short     int int16_t;
typedef   signed           int int32_t;
//typedef   signed INT64		int64_t;

/* exact-width unsigned integer types */
typedef unsigned          char uint8_t;
typedef unsigned short     int uint16_t;
typedef unsigned           int uint32_t;
//typedef unsigned       INT64 uint64_t;

/* 7.18.1.2 */

/* smallest type of at least n bits */
/* minimum-width signed integer types */
typedef   signed          char int_least8_t;
typedef   signed short     int int_least16_t;
typedef   signed           int int_least32_t;
//typedef   signed       __INT64 int_least64_t;

/* minimum-width unsigned integer types */
typedef unsigned          char uint_least8_t;
typedef unsigned short     int uint_least16_t;
typedef unsigned           int uint_least32_t;
//typedef unsigned       __INT64 uint_leas

#include	"CommonCAN_V6.h"


#define	MaxID		30


#define Num_RNet	30
#define Num_Address	1000
#define Num_Connect	10000
#define Num_Unit	30
#define Num_UDem	300
#define Num_Spectr	300
#define Num_Seans	Num_Connect/2
#define Num_Pool	500
#define Num_Freq	1000
#define Num_LinkMode	1000
#define Num_Energy	10
#define Num_GroopFr     3000
#define Num_GroopPl     1000
#define Num_BMeth	10000		// !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!1
#define Num_BParam	2000
#define Num_BIn		1000
#define Num_BOut	1000

INT64	SrtHex2INT64(char* Buf);
// {
// 	INT64	i64Vol;
// 	if ((Buf[0]=='0') && (Buf[1]=='x')) Buf+=2;
// 	int LenStr=strlen(Buf);
// 	uint	Vol1=strtoul(Buf+strlen(Buf)-8,NULL, 16 );
// 	int		TCh=Buf[LenStr-8];Buf[LenStr-8]=0;
// 	INT64	Vol2=strtoul( Buf, NULL, 16 );
// 	Buf[LenStr-8]=TCh;
// 	i64Vol=Vol1|(Vol2<<32);
// 	return i64Vol;
// }


enum TypeDevice
{
TD_Undefine=0,
TD_XStar,
TD_Lite3,
};

struct	Abonent
{
	int	ID;
	char	Name[160];
};

struct	StrData
{
	int			PoiWr;
	int			Flag;
	//Services	*Srv;
	char		Fl_StateApp;		//	-1= Neuron ERROR state; 0 - No Run; 1 - Start (Wait); 2 - Work; 3 - Restart 4 - Close (Send Close to SST)
	char		Fl_StateSmartSt;	//	0 - No Run; 1 - Start (Wait); 2 - Work; 3 - Exit  4 - Ready to Run
	char		CntRun;
	char		Fl_StateR2;
//	Abonent		Ab[MaxID];
//	int			PoiRd[MaxID];
	int			SizeSBuf;
	UCHAR		Data[1];
	//UCHAR		Data[SizeShBuf];
};

struct	Dbg_Cnt  
{
	INT64	Cnt_El_Tab;
	INT64	Cnt_IDI_Info;
	INT64	Cnt_pList;
	INT64	Cnt_IDList8;
	INT64	MemSize;
	int		AllName;
	Dbg_Cnt();
	~Dbg_Cnt();
#ifdef _DEBUG
	char	NameMas[256][265];
	INT64	C_pList[256];
	void	AddPList(char* Name);
	void	DelPList(char* Name);
#endif
};
// -------------------------------
inline INT64 ByteReverse(INT64 value) //(-){return bitReverse((UINT64)value);}
{
	value = ((value & 0xFF00FF00FF00FF00) >>  8) | ((value & 0x00FF00FF00FF00FF) <<  8);//(+)__aullshr==Does a unsigned Long Shift Right
	value = ((value & 0xFFFF0000FFFF0000) >> 16) | ((value & 0x0000FFFF0000FFFF) << 16);//(+)__aullshr==Does a unsigned Long Shift Right
	// return  ((value << 32)                    |  (value >> 32));
	//(-) value = ((value << 32)                 |  (value >> 32));//(-)__allshr==Does a signed Long Shift Right
	value = ((value << 32)                       | ((value & 0xFFFFFFFFFFFFFFFF) >> 32));//(+)__aullshr
	return  value; 
}
//
//#include "StdAfx.h"
//
//
//#include	<winbase.h>
//#include	<Windows.h>

#include	<errno.h>
#include	<winsock.h>
#include	<stdlib.h>
#include	<io.h>
#include	<fcntl.h>
#include	<sys/types.h>
#include	<sys/stat.h> 
#include	<direct.h>

//#include	<afxtempl.h>
//#include	<hasp_api.h>

#define MaxAddCntVol	20	//Ìàêñèìàëüíîå êîëè÷åñòâî ýëåìåíòîâ â ñïðàâî÷íèêå çíà÷åíèé ïàðàìåòðîâ äëÿ ClassList

#define isvalidsock(s)	( ( s ) >= 0 )
#define bzero(sap,size_of_sap) memset(sap,0,size_of_sap)

char* GetInternetStandardAddress(struct sockaddr host);
char* CreateBroadcastPANAddress(char* host);
char* CreateBroadcastLANAddress(char* host);
char* CreateBroadcastWANAddress(char* host);
static void set_address( char *hname, char *sname,
struct sockaddr_in *sap, char *protocol );

static	char	TimeBuf[256];
static	INT64	Fcpu;
static	Dbg_Cnt	DBGC;

SOCKET tcp_server( char *hname, char *sname );
SOCKET tcp_client( char *hname, char *sname );
SOCKET udp_server( char *hname, char *sname );
SOCKET udp_client( char *hname, char *sname,struct sockaddr_in *sap );


typedef const char* tStr;
int Win32LogErr(tStr aSourse,tStr aMessage=NULL,tStr aComputer=NULL);
int Win32LogWrn(tStr aSourse,tStr aMessage=NULL,tStr aComputer=NULL);
int Win32LogInf(tStr aSourse,tStr aMessage=NULL,tStr aComputer=NULL);
////////////////////////////////////////////////////////////////////

int readn( SOCKET fd, char *bp, size_t len);
int readvrec( SOCKET fd, char *bp, size_t len );
int readline( SOCKET fd, char *bufptr, size_t len );

size_t	_mSize(void* tB);

struct	Task;

#define	Ktime		10000			// 1 ms	
#define	Ktime100	1000000			// 100 ms	
#define	KtimeS		10000000		// 1 s	
#define	KtimeMh		300000000		// 1/2 Min	
#define	KtimeM		600000000		// 1 Min	
#define	Ktime5M		3000000000		// 5 Min	
#define	KtimeH		36000000000		// 1 Hor	
#define	KtimeD		864000000000	// 1 Day	
#define	KtimeY		315360000000000	// 1 Yaer	
union	FileTime
{
	INT64		Time;
	FILETIME	FTime;
};
unsigned short calc_crc16(unsigned char *Buf, unsigned int len);
unsigned short calc_crc16_4(unsigned char *Buf, unsigned int len2);

static	INT64		SmLocalZone=0;

#define	SinSize		32768
static	double		KSin,KSin_1;
static	bool		Fl_Sin=TRUE;
static	double		TabSin[SinSize];
static	double		TabCos[SinSize];

#define NumTF	30

#define NT_ANY	0
#define NT_PAN	1
#define NT_LAN	2
#define NT_WAN	3

#define NC_Default	0
#define NC_Cli		1
#define NC_Cli2Srv	2
#define NC_Srv2Srv	4
#define NC_Srv2Cli	8

static		double		tMF[10*NumTF];
static		double		tMFf[10*NumTF];

double		Sin(double Arg);
double		Cos(double Arg);
double		MedFlt(double tMas[],int NTF);
double		AddFlt(double tMas[],int NTF);
float		MedFltf(float tMas[],int NTF);

char*		cTime(FileTime*);
char*		cTimeLocal(FileTime*);
char*		cTimeAll(FileTime* FlTime);

#include	<string.h>
#include	<malloc.h>
#include	<io.h>
//#include	<fcntl.h>
//#include	<P:\File.h>
#include	<TIME.H>
#include	<fcntl.h>
#include	<stdio.h>
//#include	<uarray.h>
//#include	<Win32Log.h>

typedef unsigned char uchar;
typedef unsigned int uint;
typedef unsigned long ulong;
typedef unsigned short ushort;
typedef unsigned char UCHAR;
typedef unsigned int UINT;
typedef unsigned long ULONG;
typedef unsigned short USHORT;


INT64	GetNClk();	// Âîçâðàùàåò ñ÷åò÷èê òàêòîâ ïðîöåññîðà (äëÿ îöåíêè ïðîèçâîäèòåëüíîñòè)

typedef struct tagTHREADNAME_INFO
{
	DWORD dwType; // must be 0x1000
	LPCSTR szName; // pointer to name (in user addr space)
	DWORD dwThreadID; // thread ID (-1=caller thread)
	DWORD dwFlags; // reserved for future use, must be zero
} THREADNAME_INFO;

void SetThreadName( DWORD dwThreadID, LPCSTR szThreadName);

#define Num_Mode	100
#define Num_Meth	1000
#define Num_LinkPin	10000
#define Num_Param	5000
#define Num_NamePin	5000
#define Num_Volume	5000

//const hasp_feature_t feature = 1;
//hasp_handle_t handle = HASP_INVALID_HANDLE_VALUE;
//hasp_status_t status;

enum UserStatusList
{
	US_User,
	US_DomainAdministrator,
	US_SiteAdministrator,
	US_CustomerAdministrator,
	US_CustomerSuperVisor,
	US_GlobalSuperVisor,
};

enum UserPositionList
{
	UP_Chief,
	UP_DeputyChief,
	UP_Manager,
	UP_ChiefSpesialist,
	UP_Spesialist,
	UP_Employee,
	UP_Worker,

	MaxPosition
};


static char	*PositionTable[MaxPosition+1]=
{
	"Chief",
	"DeputyChief",
	"Manager",
	"ChiefSpesialist",
	"Specialist",
	"Employee",
	"Worker",
	""
};

enum MsgStatusList
{
	MS_Closed,
	MS_Delivery,
	MS_Wait,
	MS_Reject,
	MS_Delayed,
	MS_InProgress,
	MS_Discussed,
	MS_HaveQuestions,
	MS_Completed,
	MS_Tested,
	MS_Attention,
	MS_Reminder,

	MaxMsgStatus
};

static char	*MsgStatusTable[MaxMsgStatus+1]=
{
	"Closed",
	"Delivery",
	"Wait",
	"Reject",
	"Delayed",
	"InProgress",
	"Discussed",
	"HaveQuestions",
	"Completed",
	"Tested",
	"Attention",
	"Reminder"
	""
};

static	unsigned char vendor_code[] =
"9whECiOVm/qxF048bFB/UOdOouOaBUNfpk4FhQsxuz1Tj8DIQHf5kVaUSVEbRozLx9t+Kg9S2b952s7Y"
"85poFziVVVxWw9EYXozvp0qoQQ5AHU+qGC6kJ9Y/DlZkTaAQhaiNAmqvVK29TsG7vxfhd4w4d4+VA8Rr"
"j/Xm3cRxxG2QPrFCGhBcpQoTRBUZ3Uw7eKkIbcM6HcDF3yBYJffxaZaDxcK2iFUQs1KSkmmFs6PZ+Jfs"
"wz0zX08+Xq5bh0Wpdi60/fDVq3RX3freBgmB3avP8wneLo/hWPw4yErist4Ob53crBz8MAGU8f7ep9cc"
"0L+k7AiSSommTnZvm4OmybjRR8ThOWKuYRpy3sV6qrmXh5Lh25kTVOqXgC91txxCGkqp1+4dHOXQTekP"
"tXI1k+oqvNegk19bsJnsCTsodiwP5k1KG6nD5vzopHHBXG1kk5R5pTe0yMwa/i/UoerTuooTKCUV95gI"
"GXujHrP/MSe1jcwIEat93Is/sl5UR0LO9/iEgnr9ZaCjVRiy26J2kO1I4rl9/QX50uIwCvoXf7FOlIOe"
"9cFmdG5FbvQpaR7rWS2zwr4mROPQO2dP/gGYqB5ViRiwxBaW1IOq1JR+AUlglvyrYS6ICbG50YOEiAF6"
"1YixIvFxQ9UE0yl2M+J3dw0T22dcP6DPWMkiIZAmbmL2iOzvRAyKNRMteEO/GyI1nuGF0FauJphXPmmv"
"bEw5xLzHZFY5xdfoZdEw2nv32wXP+Jg9F+0OuDJjjRmGPDNCyLDBdB5lzNYZE/HCKRJuxOfmSEDNWRK/"
"/rIyDTT2OLBkmGHDEbxBPKV8S1tRUOBQkcyxXIfAZC4sJSNqgJU5+or2DYOHXQgi5XpH1rsTSVfA5rH6"
"xT+4zBOZCWsTPJIXgfF5AibZothlyOqZ62N23d2QNn+wQIOtj+8nz8l6CXk=";

struct LicenseCond
{
	int		CntRole_DomCtr;		// Max number of Role DomCtr
	int		CntRole_Fsrv;		// Max number of Role Fsrv
	int		CntRole_DBas;		// Max number of Role DBas
	int		CntRole_MDBas;		// Max number of Role MDBas
	int		CntRole_SPrc;		// Max number of Role SPrc
	int		CntRole_WSOp;		// Max number of Role WSOp
	int		CntRole_WSAd;		// Max number of Role WSAd
	int		CntRole_SSSrv;		// Max number of Role SSSrv
	int		CntRole_PPSrv;		// Max number of Role PostProcSrv
	int		CntRole_WSTelemetry;// Max number of Role Telemetry
	int		CntRole_WSAnalitic;	// Max number of Role AnalitycWS
// -------------------

	int		CustomerID;			// Customer ID => short CustID, short SiteID;
// -------------------
	int		NumCh_Fsrv;		// Max number of channel Fsrv
	int		NumCh_DBas;		// Max number of channel DBas
	int		NumCh_SPrc;		// Max number of channel SPrc
	int		NumCh_WSAd;
	int		NumCh_OWS;
	int		NumCh_AnWS;
	int		NumCh_PPSrv;
	int		NumCh_ExtAPI;	// Max number of External channel
// -------------------
	int		NumCh_SRM2500;	// 
	int		NumCh_QRM2150;	// 
	int		NumCh_QRM2500;	// 
	int		NumCh_RCV_4;	// 
// ------------------
	int		CntHost;			// Max number of Hosts
// ------------------
	short	Sum;
	short	Sum2;
	short	Sum3;
	short	Sum4;
	int		NeuronMode;			// Neuron.exe mode
	int		ProtectionMode;		// Protection Mode
	int		OffsetPermittedHost;// Offset in memory when was determine Permitted Host
	int		SizePermittedHost;	// Size of memory when was determine Permitted host
	int		OffsetStatistic;	// Offset in memory when stored runs statistic
	int		SizeStatistic;		// Size of memory
	int		OffsetSysInfo;		// System information
	int		SizeSysInfo;		// Size of system information
	int		OffsetUnitInfo;		// System information
	int		SizeUnitInfo;		// Size of system information
	short	Sum5;
	short	Sum6;				// User
	short	Sum7;				// Domain
// -------------------
	int		OffsetDomainInfo;		// User information
	int		SizeDomainInfo;		// Size of User information
	int		OffsetUserInfo;		// User information
	int		SizeUserInfo;		// Size of User information
	int		CntRole_Rez[6+16-1-4-3-2-2];	// Rezerve
	int		Rez[2];
};

struct LC_DomainInfo
{
	char	MainDomain[128];
	char	AllowedDomain[10][128];
	int		NumAllowedDomain;
};

struct LC_UserInfo
{
	char	UserLogin[128];
	char	UserPassword[128];
	int		UserStatus;
};

struct LC_PrmHost
{
	char	Host[80];
};

struct LC_RunStat
{
	FileTime	FirstRunTime;
	FileTime	StartRunTime;
	FileTime	LastRunTime;
	FileTime	TotallRunTime;
	INT64		RunCnt;
	FileTime	TimeExpiry;
	INT64		Rez[100];
};

struct LC_SysInfo
{
	char	LicenseRemark[256];
};

struct LC_UnitInfo
{
	int		CustomerID;			// Customer ID => short CustID, short SiteID;
	// ------------ Processing Server  ----------
	char	PrcSrv_ECC;		// ECC processing function
	char	PrcSrv_E1;			// E1(G.704) streams processing function + VOX processing
	char	PrcSrv_T1;			// T1 streams processing  function + VOX processing
	char	PrcSrv_IP;			// IP processing function
	char	PrcSrv_VoIP;		// VoIP processing (H.323, SIP) function
	char	PrcSrv_V_F;			// Voice/FAX(Data) detector in E1/T1 streams
	char	PrcSrv_Abis;		// 1 - A.bis VOX processing function  2 - A.bis full processing function
	char	PrcSrv_VSAT_H_PES;	// VSAT Hughes PES Outbound/Inbound processing function
	char	PrcSrv_VSAT_H_TES;	// VSAT Hughes TES (ICC, OCC, SCPC) processing function
	char	PrcSrv_VSAT_G_DW_IP;// VSAT Gilat DialWay@IP Outbound processing function (modified Viterby,Gilat Framer)
	char	PrcSrv_VSAT_G_FW;	// VSAT Gilat FaraWay
	char	PrcSrv_VSAT_iDirect;// VSAT iDirect NetModem/NetModem2/iNfiniti Series Outbound/Inbound processing function
	char	PrcSrv_VSAT_iDirEv;	// VSAT iDirect Evolution Series Outbound/Inbound processing function
	char	PrcSrv_DVB;			// DVB-S/S2 MPEG2
	char	PrcSrv_DVB_GSE;		// DVB-S2 GSE(SIS,MIS)
	char	PrcSrv_Edmac;		// EDMAC-1008, EDMAC-2928
	char	PrcSrv_IDR;			// IDR (IESS-308)
	char	PrcSrv_IBS;			// IBS (IESS-309), IBSn, IBSpro
	char	PrcSrv_DCME_Cl;		// DCME classifier
	char	PrcSrv_Alcatel3612;	// Alcatel-3612/DTX-600IP equipment processing
	char	PrcSrv_TiburonSpeed;// 0=2.8 - 1=5.6 MHz
	char	PrcSrv_DTX240;		// DTX-240D/E/F/T
	char	PrcSrv_DTX360;		// DTX-360A/B
	char	PrcSrv_DTX600;		// DTX-600 
	char	PrcSrv_DX3000;		// DX-3000
	char	PrcSrv_DX7000;		// DX-7000A
	char	PrcSrv_NCM501;		// NCM-501
	char	PrcSrv_TC2000;		// TC-2000
	char	PrcSrv_Celtic3G;	// Celtic3G
	char	PrcSrv_PCME;		// PCME
	char	PrcSrv_MUXNewbridge;// MUX Newbridge
	char	PrcSrv_MUXV10;		// MUX V.10
	char	PrcSrv_MUXV16;		// MUX V.16
	char	PrcSrv_MUXT1876;	// MUX T1876
	char	PrcSrv_Analysis;	// Signal analysis function
	char	PrcSrv_FR;			// FrameRelay processing function
	char	PrcSrv_ADPCM;		// ADPCM
	char	PrcSrv_SS5;			// --------------------------------------------
	char	PrcSrv_1VF;			// --------------------------------------------
	char	PrcSrv_R2;			// --------------------------------------------
	char	PrcSrv_tVol[50];	//????
	// ------------ Signaling System Server ----------
	char	SSSrv_SS7;			// ITU SS7 ISUP processing
	char	SSSrv_SS7_CIC_Map;	// SS7 automatic CIC mapping
	char	SSSrv_SS7_SCCP;		// SS7 SCCP MAP processing 
	char	SSSrv_A;			// --------------------------------------------
	char	SSSrv_Ater;			// --------------------------------------------
	char	SSSrv_Abis;			// A.bis processing
	char	SSSrv_V51;			// --------------------------------------------
	char	SSSrv_EDSS;			// --------------------------------------------
	char	SSSrv_tVol[10];		//????
	// ------------ Neuron ---------------
	char	Prog_Feature;		// уточнить
	char	Prog_Feature2;		// Used All customer Tables
	char	Prog_WSAAutoStart;
	char	Prog_tVol[10];		//????
	// ------------ Administrator Workstation  ---------------
	char	WSA_RT_Video;		// Realtime video functions (MPEG2)
	char	WSA_tVol[30];		//????
	// ------------ Operator Workstation ---------------
	char	OWS_tVol[30];		//????
	// ------------ Analytic Workstation  ---------------
	char	ANWS_IPStat;		// IP statistic functions
	char	ANWS_Telemetry;		// Graphic functions control loading of Neuron system resources
	char	ANWS_Map;			// Maps Visualization functions
	char	ANWS_I2;			// I2 Analyst’s Notebook support functions
	char	ANWS_I2Full;		// Full integration of i2 Analyst’s Notebook functions
	char	ANWS_tVol[20];		//????
	// ------------ Post processing  ---------------
	char	PPrc_Decod;			// Automatic decoding vocoder data to WAV format
	char	PPrc_Export;		// Automatic meta-data export
	char	PPrc_Scheduler;		// Scheduler processing
	char	PPrc_tVol[10];		//????
	// ----------
	char	tVol[256-59-160];	//????
};
// 
// struct LC_TimeInfo
// {
// 	FileTime	TimeExpiry;
// };

class	ListID_FiFo;
class	DLLCntrl;
class	Decoder;
class	ID_List8;
//class	ClassList;
struct	IDInfo;
class	Damper;
class	DamperPS;



#define	NameCfgFile	"Neuron.ini"

void	DeleteDecod(LPVOID pDec);

#define	B_Alternate	0x1
#define	B_Begin		0x2
#define	B_End		0x4
#define	B_NoSendBuf	0x8
#define	B_Set		0x10

struct  StrOwner
{
	Decoder*	pDec;
	void*		pObj;
	int			Prm;
	int			Type;
				StrOwner();
void			Init(Decoder* pD,void* pO,int Pr,int Tp=0);
};


struct SimClient
{
	int			Status;	// Äëÿ óêàçàíèÿ àêòèâíîé êîíôèãóðàöèè
	int			Start;	// Ñòàðòîâàòü èëè íåò
	int			State;
	int			OnWork;	// Â ðàáîòå
	char		PathOut[512];
	char		DomainName[512];
	char		ModeName[40];
	int			ModeNum;
	char		CliName[40];
	int			Fl_ReConstr;	// Ôëàã íåîáõîäèìîñòè ïðåçàïóñêà
	int			Num_ReConstr;	// êîëè÷åñòâî ïîïûòîê ïåðåçàïóñêà
	int			Cnt_ReConstr;	// ñ÷åò÷èê ïåðåçàïóñêîâ
};

struct SimCTask
{
	SimClient	Cli[MaxID];
};


#define	NumChanDemod	128

struct	DemStatOut
{
	double	Fn;		// Íåñóùàÿ ÷àñòîòà îñíîâíàÿ
	double	Fb;		// Áîäîâàÿ ÷àñòîòà
	double	Fr;		// Ðàçíîñ ÷àñòîò â êíàëå
	double	Fch;	// Ðàçíîñ ìåæäó êàíàëàìè
	float	S_N;	// Ñèãíàë-øóì
	float	Ens;	// Ýíåðãèÿ ñèãíàëà (Ñðåäíÿÿ) (Ìåäëåííîå ÀÐÓ)
	int		Mode;	// Ðåæèì äåìîäóëÿòîðà
	// ×Ì	- 0	Ïðîñòûå ×Ì
	// ×Ì-N - 1	Ìíîãîïîçèöèîííûå ×Ì
	// ÔÌ	- 2	Ïðîñòûå ôàçîâûå
	// N-×Ì	- 3	Ìíîãîêàíàëüíûå ×Ì
	// N-ÔÌ	- 4	Ìíîãîïîçèöèîííûå ÔÌ
	// ...
	int		DCD;	// Äàííûå îáíàðóæåíû
	double	Fd;		// ×àñòîòà äèñêðåòèçàöèè
	float	Enm;	// Ýíåðãèÿ ñèãíàëà (Ìãíîâåííàÿ) (Áûñòðîå ÀÐÓ)
	float	PorN;	// íèæíèé ïîðîã äëÿ Ñèãíàë-øóìà, äÁ
	float	PorS;	// ñðåäíèé ïîðîã äëÿ Ñèãíàë-øóìà, äÁ
	float	PorW;	// âåðõíèé ïîðîã äëÿ Ñèãíàë-øóìà, äÁ
	int		N_Chan;	// Êîëè÷åñòâî êàíàëîâ èëè ïîçèöèé
	double	Fp[NumChanDemod];	// ×àñòîòíûå ïîçèöèè îòíîñèòåëüíî íåñóùåé ÷àñòîòû (îò 0 äî N_Chan-1) (îíè æå áóäóò èñïîëüçîâàíû äëÿ îòîáðàæåíèÿ ìàðêåðîâ)
	int		Fl_Pilot;	// Ôëàã èñïîëüçîâàíèÿ ÷àñòîòû ïèëîòà
	float	AmStat[20];
	int		Fl_Lock;
	float	StStat[20];
	int		N_Cnt;
	float	sErr;	// 
	DemStatOut()
	{
		int i;
		Fn=Fb=Fr=Fch=Fd=Enm=0.0;
		S_N=Ens=0.0;
		Mode=DCD=0;
		PorN=5;PorS=7;PorW=10;
		N_Chan=1;Fl_Pilot=0;
		sErr=0;Fl_Lock=0;N_Cnt=0;
		for (i=0;i<20;i++) 
		{
			AmStat[i]=0;StStat[i]=0;
		};
	};
};

struct	DemStatOutEvent
{
	INT64		UID;
	DemStatOut	DSO;
};

struct	MainData
{
	USHORT	CRC16MCfg;
	long	X,Y;
	long	X1,Y1;
	char	PathDLL[10][256];
	char	PathCFG[10][256];
	char	Description[10][40];
	int		TimeOut;
	int		Active;
	int		Number;
	UCHAR	ID_Host;
	USHORT	Lock;
	USHORT	CRC16SrvCfg;
	USHORT	CRC16FCli;
	USHORT	CRC16ShCli;
	USHORT	CRC16NetCli;
	char	SKey[64];
	char	PathLOG[256];
	int		NumError;
	int		CritError;
	int		ProtectMode;
//	int		SizeSB;
	char	Hiden;
	char	ConfExit;
	char	FlInteg;
	char	FlSetTime;
	int     NumDayCE;
	char	Rezerv[256-12];
	int		LANType;
//	Insert hear and will correct number of Rezerved char
	int		Fl_RunSICli;
	char	Path[256];
	int		Repl;
	int		Fl_Hide;
	WINDOWPLACEMENT WndPlc;
	int		Fl_SISrv;
	int		TimeInt;
	int		NumDayLog;
	int		TimeLocGlob;
	int		tVersion;
	int		fErrView;
	char	PathOut[256];
	int		fTabCfg;
	char	PathBD[256];
	int		fThIDV;
//	int		Fl_WorkGroup;
//	char	WorkGroup[256];
	int		Fl_Start;
	int		Fl_AbnStart;
	int		Fl_NetCompress;
	char	Fl_PAN;
	char	Fl_LAN;
	char	Fl_WAN;
	char	Fl_Cripto;
	int		IP_MaskPAN;
	int		IP_MaskLAN;
	int		IP_MaskWAN;
	int		IP_PAN;
	int		IP_LAN;
	int		IP_WAN;
	int		IP_WANIS1;
	int		IP_WANIS2;
	int		Fl_RunSI;
	int		Fl_SendUpD;
	int		Fl_RecUpD;
	int		Fl_RunSmSt;
	int		CustomerID;	// if 0 - not define
	char	LastLoginName[128];
	int		Flag_AutoLogon;
	int		Flag_APIActivity;
	char	IP_Address[16];
	char	IP_Port[16];
};

class CMainCfg  
{
public:
	MainData	MData;
//	ClassList*	CL;
	Task*		Tsk;
	CMainCfg(Task* Ts);
	virtual ~CMainCfg();
	void	LoadCfg();
	void	SaveCfg();
	void	MakeDirectory();
};


struct SockHeader
{
	uchar		Syn;
	uchar		CntPack;
	USHORT		Len;
//	uint		CntDebug;
	SockHeader()	{CntPack=0;/*CntDebug=0;*/};
};

struct DmpHeader
{
	uint		CntDebug;
	SockHeader	SockHead;
	DmpHeader()	{CntDebug=0;};
};

//#include <CRC16.h>

struct UDPHeader
{
	USHORT		CRC;
	USHORT		NumCli;	
	int			Comm;
	uint		Len;
	uint		CntPack;
	UDPHeader()	{CntPack=0;};
};

inline void	SetCRCUDP(UCHAR* Buf)
{
	((UDPHeader*)Buf)->CRC=calc_crc16(Buf+sizeof(USHORT),sizeof(UDPHeader)+((UDPHeader*)Buf)->Len-sizeof(USHORT));
};

inline BOOL	CalcCRCUDP(UCHAR* Buf)
{
	return ((UDPHeader*)Buf)->CRC==calc_crc16(Buf+sizeof(USHORT),sizeof(UDPHeader)+((UDPHeader*)Buf)->Len-sizeof(USHORT));
};

inline bool mValidStr(char* pStr) //ïðîâåðêà íà ïðàâèëüíîñòü ñòðîêè åñëè TRUE - ïðàâèëüíàÿ
{
	bool Rez=FALSE;
	if (pStr!=0) 
		if ((*(unsigned char *)pStr)!=0)
	{	
		Rez=TRUE;
		while ((*(unsigned char *)pStr)!=0)
		{
			if ((*(unsigned char *)pStr)<0x20)
			{
				Rez=FALSE;
				break;			
			}
			++pStr;
		};		
	};
	return Rez;
};

struct StrChron
{
	INT64	t0;			// t0 in CPU tacts
	INT64	t1;			// t1 in CPU tacts
	INT64	T;			// T in CPU tacts
	INT64	t;			// t in CPU tacts
	INT64	T_m;		// max T in CPU tacts
	INT64	t_m;		// max t in CPU tacts
	double	T_max;		// max T in sec
	double	t_max;		// max t in sec
	double	T_average;	// average T in sec
	double	t_average;	// average t in sec
	double	porosity;	// in %
};

class		Services;
static	Services*	PoiServices=0;	// Äëÿ ðàáîòû áèïåðà :)
void		SetServices(Services* Srv);
Services*	GetServices();

struct	FlagStruct	
{
	CRITICAL_SECTION CrSection;
	char*		FileName;
	int			NumLine;
	int			CntIn;
	Decoder*	PDec;
	FlagStruct()
	{
		InitializeCriticalSection(&CrSection);
		CntIn=0;FileName=NULL;CntIn=0;PDec=NULL;
	};
	~FlagStruct()
	{
		DeleteCriticalSection(&CrSection);
	}
};


void MyLock(char* FileName,int NLine,FlagStruct* AdrFlag,Decoder* Dec);
BOOL MyTryLock(char* FileName,int NLine,FlagStruct*	StrFlag,Decoder* Dec);
void MyUnLock(char* FileName,int NLine,FlagStruct* AdrFlag,Decoder* Dec);

class Chronometry
{
public:
	StrChron	StrCh[10];
	Decoder*	PDec;
	Services*	Srv;
		Chronometry(Decoder*	PD)
		{
			PDec=PD;Srv=GetServices();
			memset(StrCh,0,10*sizeof (StrChron));
		};
virtual	StrChron* Measuring(int command,uint NumIntervals);	// command 0 - begin 1 - end, NumIntervals 0,1,2,...,9
};

struct BandleFind
{
	int			PoiEl;
	int			PoiRd;
};

class BandleList
{
private:
	UCHAR*		DataBuf;
	int			SizeBuf;
	int			PoiEl;
	int			PoiRd;
	int			PoiWr;
public:
	int			NumElem;
Services*		Srv;
//HANDLE			hmtx;
char			SemName[80];
FlagStruct		Fl_MT;
				BandleList(int Size=NULL); // Ïðåäïîëàãàåìûé ðàçìåð
				~BandleList();
virtual	void	AddElem(UCHAR* Buf,size_t Cnt);
virtual	int		FindFirst(UCHAR* &Buf,size_t &Cnt);
virtual	int		FindNext(UCHAR* &Buf,size_t &Cnt);
virtual	int		FindFirst(UCHAR* &Buf,size_t &Cnt,BandleFind* BFind);
virtual	int		FindNext(UCHAR* &Buf,size_t &Cnt,BandleFind* BFind);
virtual	UCHAR*	GetDataBuf(size_t &Cnt);
virtual	void	SetDataBuf(UCHAR* Buf,size_t Cnt);
virtual	int		LoadFromFile(int FH,int Offset);
virtual	int		SaveToFile(int FH,int Offset);
virtual	int		DelElem(UCHAR* &Buf,size_t &Cnt);
virtual void	DelAll(void);	
virtual	int		DelLast(UCHAR* &Buf,size_t &Cnt);
};

struct	HShMem
{
int	PoiWr;
int	Len;
int	Flag;
};

struct	PrmHeader
{
	INT64	ID;
	int		Cnt;
};

struct	PrmDec
{
	char	Prm[40];
	char	Vol[128];
	void	Set(char *P,char *V)
	{
		strcpy(Prm,P);
		if (V) strcpy(Vol,V);
	};
};


struct	Tsk_Meth
{
	char	Name[40];
	int	Ext;
	Decoder	*PDec;
	int	ID_Md;
	int	N_PO;
	int	N_PI;
	int	State;
	INT64	NC_Data;
	INT64	NC_OData;
	double	nC_Data;
	double	nC_OData;
	int     X,Y;
	INT64	NC_Idle;	// !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
	double	nC_Idle;
	int		LenData;
	int		LenOData;
	int		CntData;
	int		CntOData;
};

struct	Tsk_NamePin
{
	char	Name[40];
	int	ID_Mt;
	int	N_PO;	// Íîìåð íîæêè  èëè -1 åñëè íå âûõîä
	int	N_PI;   // Íîìåð íîæêè  èëè -1 åñëè íå âõîä
};

struct	Tsk_Param
{
	char	Name[40];
	int	ID_Mt;
	char	Vol[255];
};

struct	Tsk_Volume
{
	char	Vol[255];
	int	ID_Pr;
};


struct	Tsk_Mode
{
	char	Name[40];
	int	MethObn;	// ìåäîä îáíàðóæåíèÿ
				// 1-ñïåêòðàëüíûé
				// 2-ýíåðãèòè÷åñêèé
				// 4-ïðîáíàÿ äåìîäóëÿöèÿ
	int	ID_Sp;		// ýòàëîííûé ñïåêòð
	int	ID_En;		// ýíåðãèòè÷åñêèå êðèòåðèè
	int	ID_UD;		// âèä ìîäóëÿöèè
	int	TimeProb;	// âðåìÿ ïðîáû äåìîäóëÿöèè
//	int	ID_LP;
        int     X;
        int     Y;  
};
struct	Tsk_LinkPin
{
	int	PI;
	int	ID_Mt1; // Äëÿ óêàçàíèÿ Èñòî÷íèêà ID_Mt1=-1
	int	PO;
	int	ID_Mt2; // Äëÿ Âûõîäà ID_Mt2=-1
};

struct	UDec
{
public:
	Tsk_Mode		Md[Num_Mode];
	Tsk_Meth		Mt[Num_Meth];
	Tsk_LinkPin		LP[Num_LinkPin];
	Tsk_Param		Pr[Num_Param];
	int		Max_Md;
	int		Max_Mt;
	int		Max_LP;
	int		Max_Pr;
};

struct	WUDec	// Ñòðóêòóðà õðàíÿùàÿ ìåäîäû òðàêòà èõ ñâÿçè è ïàðàìåòðû
{
public:
	Tsk_Mode		Md;
	Tsk_Meth		Mt[Num_Meth];
	Tsk_LinkPin		LP[Num_LinkPin];
	Tsk_Param		Pr[Num_Param];
	int		Max_Mt;
	int		Max_LP;
	int		Max_Pr;
};

struct	I64
{
	int	lo;
	int	hi;
};

union	NumClk
{
	INT64	NClk;
	I64		DWNClk;
};



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
	char descriptor[80];// (in/out) ìíåìîíè÷åñêîå íàçâàíèå ñåðâåðà (íà÷èí. ñ _) èëè êëèåíòà
	LPVOID handle;			// (out) ïðåäïîëàãàåòñÿ äëÿ ðàçïîçíàâàíèÿ îáúåêòà NetService
	PUCHAR buf;			// (in/out) óêàçàòåëü íà áóôåð
	char param[64];		// (in) èìÿ ïàðàìåòðà
	char vol[64];		// (in) çíà÷åíèå ïàðàìåòðà
	int pin;			// (in/out) íîìåð âûâîäà
	int command;		// (in) see NetSockCommand
	int data;			// (serivce) èñïîëüçóåòñÿ äëÿ ïåðåäà÷è íîìåðà êëèåíòà
	int	numinlist;
	int	NetType;		// (in) NT_PAN,LAN,WAN 
	size_t cnt;			// (in/out) äëèíà äàííûõ â áóôåðå
	//char host[256];		// host name server
};
typedef PackNetObj *pPackNetObj;

class	PackNetObjArray
{
public:
	int	idx,N;
	pPackNetObj Array;

							PackNetObjArray(int q);
							~PackNetObjArray();
	virtual int				Reset();
	virtual pPackNetObj		GetPack();
	virtual void			PostPack(char* Descriptor,int Command);
	virtual void			PostPack(char* Descriptor,int Command,char* Param, char* Vol);
	virtual void			PostPack(char* Descriptor,int Command,UCHAR* pBuf, size_t Cnt);
	virtual void			PostPack(char* Descriptor,int Command,int pin,UCHAR* pBuf, size_t Cnt);
	virtual int				GetBuf(PUCHAR &p,int &c);
};
typedef PackNetObjArray *pPackNetObjArray;



void ThreadFun(LPVOID poi);

class	ClassThread
{
private:
	DWORD		IDThread;
	HANDLE		hThrd; 
	LPVOID		pObj;	
//	int			CntV;
	INT64		tTime;
	int			(*pFun)(LPVOID);
	int			Fl_Stop;
	Services	*Srv;
public:
	int			On;
	FlagStruct	Fl_Th;
	char		NameThread[255];
	float		Loads;
	double		Load_t;
	double		Load_T;
					ClassThread(char*	Name,size_t Priority=NULL);	// THREAD_PRIORITY_HIGHEST
/*
THREAD_PRIORITY_ABOVE_NORMAL Indicates 1 point above normal priority for the priority class. 
THREAD_PRIORITY_BELOW_NORMAL Indicates 1 point below normal priority for the priority class. 
THREAD_PRIORITY_HIGHEST Indicates 2 points above normal priority for the priority class. 
THREAD_PRIORITY_IDLE Indicates a base priority level of 1 for IDLE_PRIORITY_CLASS, BELOW_NORMAL_PRIORITY_CLASS, NORMAL_PRIORITY_CLASS, ABOVE_NORMAL_PRIORITY_CLASS, or HIGH_PRIORITY_CLASS processes, and a base priority level of 16 for REALTIME_PRIORITY_CLASS processes. 
THREAD_PRIORITY_LOWEST Indicates 2 points below normal priority for the priority class. 
THREAD_PRIORITY_NORMAL Indicates normal priority for the priority class. 
THREAD_PRIORITY_TIME_CRITICAL 
*/
					ClassThread(char*	Name,int (*Fun)(LPVOID) ,LPVOID Prm,size_t Priority=NULL);// Fun - óêàçàòåëü íà ïîëüçûâàòåëüñêóþ ôóíêöèþ Prm - óêàçàòåëü íà îáúåêò (áåç òèïà)	
	virtual	int		Calc();
	virtual	void	Stop();
	virtual	void	Start();
	virtual	void	ModOn(int Vol);
	virtual	int		GetOn();
					~ClassThread();
};

class	Decoder;
class	Savelog;
//class	ClassList;
//class	ID_El_CL;

class ID_Elem
{
public:
	INT64		Len;
	UCHAR		*Buff;
#ifndef _WIN64
	INT				Z1;
#endif
				ID_Elem()
				{
					Buff=NULL;Len=0;
#ifndef _WIN64
					Z1=0;
#endif
				}; 
				ID_Elem(UCHAR*	BuffIn,size_t	Cnt) 
				{
					Init(BuffIn,Cnt);
				};
virtual	void	Init(UCHAR*	BuffIn,size_t	Cnt) // Èñïîëüçóåòñÿ ïðè ïîâòîðíîì èñïîëüçîâàíèè ýëåìåíòà
				{
					Buff=BuffIn;
					Len=Cnt;
#ifndef _WIN64
					Z1=0;
#endif
				};		
virtual			~ID_Elem()
				{
				}; 
};

class ID_ElemM
{
public:
	ID_Elem*	Elem;
	ID_ElemM*	Perviose;
	ID_ElemM*	Next;
};

class ID_ElemAdrList	:	public	ID_Elem
{
public:
	LPVOID			Vol;
	ID_List8		*List;
	ID_ElemAdrList(LPVOID	tVol);
	~ID_ElemAdrList();
	virtual	void	Init(LPVOID	tVol);
};

class ID_ElemPTR	:	public	ID_Elem
{
public:
	LPVOID			Vol;
				ID_ElemPTR(LPVOID	tVol) 
				{
					Init(tVol);
				};
virtual	void	Init(LPVOID	tVol)
				{
					Vol=tVol;
					Buff=(UCHAR*)&Vol;
					Len=sizeof(LPVOID);
				}
};
class ID_Elem32	:	public	ID_Elem
{
public:
	int			Vol;
				ID_Elem32(int	tVol) 
				{
					Init(tVol);
				};
virtual	void	Init(int	tVol)
				{
					Vol=tVol;
					Buff=(UCHAR*)&Vol;
					Len=sizeof(int);
				}
};


class ID_Elem64	:	public	ID_Elem
{
public:
	INT64		Vol;
	FileTime	FT;
				ID_Elem64(){}; 
				ID_Elem64(INT64	tVol) 
				{
					Vol=tVol;
					Init(Vol);
				};
virtual	void	Init(INT64	tVol);
};

struct ShortAdressI
{
	INT64	UnitID;
	char	HostName[40];
	char	UnitName[40];
	char	DomainName[40];
	char	UnitMode[40];
	void Init(){UnitID=0;HostName[0]=0;UnitName[0]=0;DomainName[0]=0;UnitMode[0]=0;};
	ShortAdressI()
	{Init();}

};

class Services;
struct AddressInfo
{
	INT64	ID;
	int		NCStrategic;	// D0=0-All 1-Alternate D1=1-Begin D2=1 End D3=1-No Send Buf D4=1-Set
	short	NetType;		// NT_PAN, NT_LAN, NT_WAN
	short	NetCommand;
	ShortAdressI	DestinAI;// Self
	ShortAdressI	SourceAI;// My
	ShortAdressI	NativeAI;
	ShortAdressI	TranspAI;// As rule it is own DomainController or next DomainController

	void	Init();
	AddressInfo()
	{Init();};
// 	void	Init()
// 	{
// 
// 		NCStrategic=0;NetType=NT_LAN;NetCommand=NC_Default;
// 		DestinAI.Init();// Self
// 		SourceAI.Init();// My
// 		NativeAI.Init();
// 		TranspAI.Init();
// #ifndef _NOSRV
// 		Services* pSrv=GetServices();
// 		if (pSrv)	 ID=pSrv->GetUID();	// Ïî óìîë÷àíèþ íå èñïîëüçóåòñÿ, à ðàáîòàåò òîëüêî â Neuron !!! (Ýòî èìåííî _ñåðâèñíàÿ_ âîçìîæíîñòü !!!)
// 		else ID=0;
// #else
// 		ID=0;
// #endif
// 	};
};


class ID_ElemAdrStr	:	public	ID_Elem
{
public:
	INT64		Vol;
	FileTime	FT;
	AddressInfo	AdrI;
	ID_ElemAdrStr(){}; 
	ID_ElemAdrStr(INT64	tVol);
	virtual	void	Init(INT64	tVol);
};

class ID_ElemString	:	public	ID_Elem
{
public:
				char		Buf[64];
				ID_ElemString(){}; 
				ID_ElemString(char* tBuf) 
				{
					Init(tBuf);
				};
				virtual	void	Init(char* tBuf)
				{
					strcpy(Buf,tBuf);
					Buff=(UCHAR*)Buf;
					Len=1+strlen(tBuf);
				}
};

class ID_ElemChar	:	public	ID_Elem
{
public:
	ID_Elem64	Elem64;
	char		Buf[64];
				ID_ElemChar(){}; 
				ID_ElemChar(char* tBuf,INT64	tVol) 
				{
					Init(tBuf,tVol);
				};
				virtual	void	Init(char* tBuf,INT64	tVol)
				{
					Elem64.Init(tVol);
					strcpy(Buf,tBuf);
					Buff=(UCHAR*)Buf;
					Len=1+strlen(tBuf);
				}
};

class El_OrderNum	:	public	ID_Elem
{
public:
	INT64	OrdNum;
	INT64	ID;
	El_OrderNum(INT64	Num,INT64	tID)
	{
		OrdNum=Num;
		ID=tID;
		Init((UCHAR*)&OrdNum,sizeof(INT64));
	};
};

class ID_ElemIDI	:	public	ID_Elem
{
public:
	IDInfo*		IDI;
				ID_ElemIDI(Decoder* Mt=NULL); 
				~ID_ElemIDI(); 
virtual	void	Init(Decoder* Mt);

};

class ID_ElemIDI2	:	public	ID_Elem
{
public:
	IDInfo*		IDI;
	IDInfo*		tPIDI;
				ID_ElemIDI2(IDInfo* pIDI,Decoder* Mt=NULL); 
				~ID_ElemIDI2(); 
				virtual	void	Init(IDInfo* pIDI,Decoder* Mt);
};

#define SizeDPool		64
#define SizeDPoolEl		64
//#define SizeDPool		1
//#define SizeDPoolEl	1

struct	FindStrList8
{
//	ID_ElemM	*Active;
//	ID_ElemM	*Perv;
//	ID_ElemM	*Next;
	ID_List8*	IDL;
	uchar		ID_FSL;
	FindStrList8()
	{
		IDL=NULL;ID_FSL=255;
//		Active=Perv=Next=NULL;
	};
void	DestroyStr(void);
	~FindStrList8();
};

struct  RootSturct
{
	ID_ElemM*	ActiveEl[256];
	ID_ElemM*	ActiveElP[256];
	ID_ElemM*	ActiveElN[256];
	size_t		PoiFSL[256];
	char		Fl_PoiFSL[256];
	int			MaxPoi;
	RootSturct()
	{
		MaxPoi=0;
	};
};

class ID_List8
{
public:
	int			NumElem;
	int			Level;
protected:
	// --------------------
	ID_List8*	Lev[257];
	ID_ElemM	ElemM[257];
	ID_List8*	*DelPool;
	ID_Elem*	*DelPoolEl;
	ID_List8*	PoiRoot;
	int			FlBeg,FlEnd;
	int			NumDelPool;
	int			NumDelPoolEl;
	int			Fl_DL;
	SHORT		tPElem;
	int			CntDelInsert;
	// --------------------
	ID_ElemM*	BeginEl;
	ID_ElemM*	EndEl;
//public:
	FlagStruct	FlReEn;
//protected:
//	ID_ElemM*	ActiveNEl;
//	ID_ElemM*	ActivePEl;
	int			Fl_Del;
public:
	INT64		CriErr;
//	char		LastFunc[256];
//	HANDLE		hmtx;
	RootSturct	*RStr;
	ID_List8();
	ID_List8(int	Lev,ID_List8*	PRoot);
	~ID_List8();
	virtual	int			Add(ID_Elem* IDE);
	virtual	int			Del(ID_Elem* IDE);
	virtual	int			Remove(ID_Elem* IDE);
	virtual	ID_Elem*    SearchSIGN(UCHAR* Buff,size_t Cnt);
	virtual	ID_Elem*	Find(UCHAR* Buff,size_t Cnt,FindStrList8	&FSL8);	// Cond!=0 ªþ ÿõ¨õÿ¨ø¸òðøòðõª¸  ºúð÷ðªõû¹ ÿþø¸úð 
	virtual	ID_Elem*	Find(UCHAR* Buff,size_t Cnt);	// Cond!=0 ªþ ÿõ¨õÿ¨ø¸òðøòðõª¸  ºúð÷ðªõû¹ ÿþø¸úð 
	virtual	ID_Elem*	FindNext(UCHAR* Buff,size_t Cnt,FindStrList8	&FSL8);	// Cond!=0 ªþ ÿõ¨õÿ¨ø¸òðøòðõª¸  ºúð÷ðªõû¹ ÿþø¸úð 
	virtual	ID_Elem*	FindPerviose(UCHAR* Buff,size_t Cnt,FindStrList8	&FSL8);	// Cond!=0 ªþ ÿõ¨õÿ¨ø¸òðøòðõª¸  ºúð÷ðªõû¹ ÿþø¸úð 

	virtual	ID_Elem*	Begin(FindStrList8& FSL8);
	virtual	ID_Elem*	End(FindStrList8& FSL8);
	virtual	ID_Elem*	Next(FindStrList8& FSL8);
//	virtual	ID_Elem*	Next(ID_Elem* tElem);	// ñïîèñêîì îò ïðåäèäóùåãî ýëåìåíòà
	virtual	ID_Elem*	Perviose(FindStrList8& FSL8);
//	virtual	ID_Elem*	GetActive();

	virtual	int			CalcObjSize();	// ¨õúþüõýôºõª¸  ø¸ÿþû¹÷þòðª¹ ¨õôúþ !!!
	virtual	void		DelAllElem();
	virtual	void		RemoveAllPoiElem();
	virtual	ID_Elem*	NewElem();	// ¦þûº¢øª¹ ºúð÷ðªõû¹ ýð ¤ûõüõýª ø÷ ¸ÿø¸úð ÿþôóþªþòûõýýv¿ ú ºôðûõýø¦
protected:
	virtual	ID_ElemM*	SearchSIGNI(UCHAR* Buff,size_t Cnt);
	virtual	ID_ElemM*	FindI(UCHAR* Buff,size_t Cnt,FindStrList8	&FSL8);	// Cond!=0 ªþ ÿõ¨õÿ¨ø¸òðøòðõª¸  ºúð÷ðªõû¹ ÿþø¸úð 
	virtual	ID_ElemM*	FindI(UCHAR* Buff,size_t Cnt);	// Cond!=0 ªþ ÿõ¨õÿ¨ø¸òðøòðõª¸  ºúð÷ðªõû¹ ÿþø¸úð 
	virtual	ID_ElemM*	RemoveI(ID_Elem* IDE);
	virtual	ID_ElemM*	DelI(ID_Elem* IDE);
	virtual	ID_ElemM*	AddI(ID_Elem* IDE);
	virtual	ID_ElemM*	NextI();
	virtual	ID_ElemM*	PervioseI();
	virtual	void		Init(int Lv,ID_List8*	PRoot);
	virtual	ID_List8*	NewLev(int	Lev);
	virtual	void		DelLev(ID_List8* pLev);
	virtual	void		DelRLev(ID_List8* pLev);
	//virtual	void		mDelLev(ID_List8* pLev);
	//virtual	void		mDelRLev(ID_List8* pLev);
	virtual	void		DelElem(ID_ElemM *IDE)	// ¦þûþöøª¹ ¤ûõüõýª ò ¸ÿø¸þú ÿþôóþªþòûõýýv¿ ú ºôðûõýø¦
	{
		DelLink(IDE);
		if(PoiRoot->Fl_DL==0)
		{
			delete IDE->Elem;
		}
		else
		{
			//							int	i;
			if (PoiRoot->DelPoolEl==0)
			{
				delete IDE->Elem;
			}
			else
			{
				if (PoiRoot->NumDelPoolEl>=SizeDPoolEl)
				{
					delete PoiRoot->DelPoolEl[CntDelInsert];
					PoiRoot->DelPoolEl[CntDelInsert]=IDE->Elem;
					if (++CntDelInsert>=SizeDPoolEl)
						CntDelInsert=0;
//					delete IDE;
				}
				else
					PoiRoot->DelPoolEl[PoiRoot->NumDelPoolEl++]=IDE->Elem;
			}
		};
	};	
	virtual	void	DelLink(ID_ElemM* IDE)
	{
		if (PoiRoot->RStr)
		{
			int	i;
			for (i=0;i<PoiRoot->RStr->MaxPoi;i++)
			{
				if (PoiRoot->RStr->ActiveElN[i]==IDE)
				{
					PoiRoot->RStr->ActiveElN[i]=NULL;
				}
				if (PoiRoot->RStr->ActiveElP[i]==IDE)
				{
					PoiRoot->RStr->ActiveElP[i]=NULL;
				}
				if (PoiRoot->RStr->ActiveEl[i]==IDE)
				{
					PoiRoot->RStr->ActiveElN[i]=IDE->Next;
					PoiRoot->RStr->ActiveElP[i]=IDE->Perviose;
					PoiRoot->RStr->ActiveEl[i]=NULL;
				}
			}
		}
		if(IDE->Perviose)
		{
			IDE->Perviose->Next=IDE->Next;
			if (IDE->Next)
			{
				IDE->Next->Perviose=IDE->Perviose;
			}
			else
			{
				PoiRoot->EndEl=IDE->Perviose;
			};
		}
		else
		{
			if (IDE->Next)
			{
				IDE->Next->Perviose=IDE->Perviose;
				if (IDE->Next->Perviose==0)
				{
					PoiRoot->BeginEl=IDE->Next;
				}
			}
			else
			{
				PoiRoot->EndEl=PoiRoot->BeginEl=NULL;
			};
		}
// 		if (PoiRoot->ActiveEl==IDE)
// 		{
// 			PoiRoot->ActiveNEl=IDE->Next;
// 			PoiRoot->ActivePEl=IDE->Perviose;
// 			PoiRoot->ActiveEl=NULL;
// 		};
		IDE->Next=IDE->Perviose=0;

	};
public:
	virtual	bool TestValid();
	virtual	int Compare(ID_ElemM*	tEl1,ID_ElemM*	tEl2);
	virtual	uchar AddFSL(FindStrList8* FSL);
	virtual	bool DelFSL(uchar	ID_SFL);
};
// ------------------------------
typedef ID_List8 *pID_List8;
//---------------------------------

/*
class ID_List8
{
public:
	// --------------------
	ID_List8*	Lev[256];
	ID_Elem*	Elem[256];
	ID_List8*	*DelPool;
	ID_Elem*	*DelPoolEl;
	//	ID_List8*	DelPool[SizeDPool];
	//	ID_Elem*	DelPoolEl[SizeDPoolEl];
	ID_List8*	PoiRoot;
	int			Level;
	int			NumElem;
	int			FlBeg,FlEnd;
	int			NumDelPool;
	int			NumDelPoolEl;
	int			Fl_DL;
	SHORT		tPElem;
	int			CntDelInsert;
	// --------------------
	int			FlReEn;
	ID_List8();
	ID_List8(int	Lev,ID_List8*	PRoot);
	~ID_List8();
	virtual	int			Add(ID_Elem* IDE);
	virtual	int			Del(ID_Elem* IDE);
	virtual	int			Remove(ID_Elem* IDE);
	virtual	ID_Elem*    SearchSIGN(UCHAR* Buff,size_t Cnt);
	virtual	ID_Elem*	Find(UCHAR* Buff,int Cnt,int Cond=0);	// Cond!=0 ªþ ÿõ¨õÿ¨ø¸òðøòðõª¸  ºúð÷ðªõû¹ ÿþø¸úð 
	virtual	ID_Elem*	Begin();
	virtual	ID_Elem*	End();
	virtual	ID_Elem*	Next();
	virtual	ID_Elem*	Perviose();
	virtual	ID_Elem*	GetActive();
	virtual	int			CalcObjSize();	// ¨õúþüõýôºõª¸  ø¸ÿþû¹÷þòðª¹ ¨õôúþ !!!
	virtual	void		BeforeBegin();
	virtual	void		BeforeEnd();
	virtual	void		DelAllElem();
	virtual	void		RemoveAllPoiElem();

	virtual	ID_Elem*	NewElem()	// ¦þûº¢øª¹ ºúð÷ðªõû¹ ýð ¤ûõüõýª ø÷ ¸ÿø¸úð ÿþôóþªþòûõýýv¿ ú ºôðûõýø¦
	{
		if (PoiRoot->Fl_DL==0)
		{
			DelPool=(ID_List8**)malloc(SizeDPool*sizeof(ID_List8*));
			DelPoolEl=(ID_Elem**)malloc(SizeDPoolEl*sizeof(ID_Elem*));
		}
		PoiRoot->Fl_DL|=1;
		if (PoiRoot->NumDelPoolEl>0)
			return	PoiRoot->DelPoolEl[--PoiRoot->NumDelPoolEl];
		else
			return NULL; 
	};
protected:
	virtual	void		Init(int Lv,ID_List8*	PRoot);
	virtual	ID_List8*	NewLev(int	Lev);
	virtual	void		DelLev(ID_List8* pLev);
	virtual	void		DelRLev(ID_List8* pLev);
	//virtual	void		mDelLev(ID_List8* pLev);
	//virtual	void		mDelRLev(ID_List8* pLev);
	virtual	void		DelElem(ID_Elem* IDE)	// ¦þûþöøª¹ ¤ûõüõýª ò ¸ÿø¸þú ÿþôóþªþòûõýýv¿ ú ºôðûõýø¦
	{
		if(PoiRoot->Fl_DL==0)
			delete IDE;
		else
		{
			//							int	i;
			if (PoiRoot->DelPoolEl==0)
			{
				delete IDE;
			}
			else
			{
				if (PoiRoot->NumDelPoolEl>=SizeDPoolEl)
				{
					delete PoiRoot->DelPoolEl[CntDelInsert];
					PoiRoot->DelPoolEl[CntDelInsert]=IDE;
					if (++CntDelInsert>=SizeDPoolEl)
						CntDelInsert=0;
//					delete IDE;
				}
				else
					PoiRoot->DelPoolEl[PoiRoot->NumDelPoolEl++]=IDE;
			}
		};
	};	
};
// ------------------------------
typedef ID_List8 *pID_List8;
//---------------------------------
*/

template <class TT> class ID_List64:public ID_List8
{
public:
	ID_List64():ID_List8()
	{
		NumElem=0;tPElem=0;Level=0;FlBeg=0;FlEnd=0;
		for (int i=0;i<256;i++){Lev[i]=NULL;ElemM[i].Elem=NULL;}
//		for (int i=0;i<256;i++){Lev[i]=NULL;Elem[i]=NULL;}
	};
	
	TT* operator [](INT64 Position)
	{
		return (TT*)Find((uchar*)&Position,sizeof(INT64));
	}

	TT*	TBegin(FindStrList8& FSL8)
	{
		return(TT*)Begin(FSL8);	
	};
	
	TT*	TNext(FindStrList8& FSL8)
	{
		return(TT*)Next(FSL8);	
	};
	TT*	TStepPerviose(FindStrList8& FSL8)
	{
		return(TT*)Perviose(FSL8);	
	};
	/*
	TT& operator >=(ID_List64<TT>* p)
	{
		return (TT*)Next((uchar*)&Position,sizeof(INT64));
	}*/
};

//---------------------------------

struct		UDPCli
{
	int					fl;	// 0 êëèåíò îòñóòñòâóåò
	struct sockaddr		Adr;
	FileTime/*long*/	lwTime;
	double				CPUUsage;
	FileTime			crTime;
	char				IPAddr[32];
	char				Host[256];
	char				ShortHost[64];
	char				MainDomainName[16];	// !!!!!!!!!!!!!!!!!!
	INT64				UID_CritMsg;
	double				MemUsage;
	ID_List8*			DomainList;
	short				Ver;
	char				Command;
	char				CPUMode;
public:
	UDPCli()
	{
		fl=0;
		IPAddr[0]=0;
		Host[0]=0;
		ShortHost[0]=0;
		CPUUsage=0;
		UID_CritMsg=0;
		fl=0;
		IPAddr[0]=0;
		Host[0]=0;
		ShortHost[0]=0;
		MainDomainName[0]=0;
		CPUUsage=0;
		UID_CritMsg=0;
		DomainList=NULL;
	};
	bool				TestDomName(char* DomName)
	{
		if (DomainList)
			return(DomainList->Find((UCHAR*)DomName,strlen(DomName)+1)!=NULL);	
		else
			return FALSE;
	};
	~UDPCli()
	{
		if (DomainList)
		{
			delete DomainList; DomainList=NULL;
		}
	}
};

#define		SICom_Connect			0+UDPSmN*10
#define		SICom_ClientWait		1+UDPSmN*10
#define		SICom_ServerWait		2+UDPSmN*10
#define		SICom_DisConnect		3+UDPSmN*10
#define		SICom_Error				4+UDPSmN*10
#define		SICom_TimeSyn			6+UDPSmN*10
#define		SICom_Hardware			7+UDPSm*10
#define		SICom_ClientActive		8+UDPSmN*10
#define		SICom_ServerActive		9+UDPSmN*10
#define		SICom_Standalone		10+UDPSmN*10
#define		SICom_SerDecLock		11+UDPSmN*10
#define		SICom_CPUUsage			12+UDPSmN*10
#define		SICom_SuperVisorActive	13+UDPSmN*10
// ...
#define		SICom_Packet			15+UDPSmN*10	// Ïîñëåäíÿÿ

class		Decoder;

#pragma pack(push,1)

struct UDPPack
{
	uchar				Command;
	char				Rez;
	short				Ver;
	double				CPUUsage;
	FileTime			FTime;
	double				MemUsage;
	char				HostName[40-sizeof(ULONG)];
	ULONG				WANADR;								// 23.04.2010
	char				MainDomainName[40];					// 02.09.2010
};

struct UDPPack2
{
	uchar				Command;
	float				SrvCPUUsage;
	float				CPUUsage[MaxID];
	float				VirtCPUUsage[MaxID];
};

#pragma pack(pop)

class El_CliInt : public ID_Elem
{
public:
	UDPCli	Cli;
	ULONG	Addr;
	INT64	TimeAdd;
	float	CPULoadsSrv;
	float	CPULoadsBuff[MaxID];
	float	VirtCPULoadsBuff[MaxID];
	El_CliInt(ULONG Adr) 
	{
		Cli.fl=0;Cli.UID_CritMsg=0;
		TimeAdd=0;
		Addr=Adr;
		for (int i=0;i<MaxID;i++)
		{
			CPULoadsBuff[i]=0;
		}
		Init((uchar*)&Addr,sizeof(ULONG));
	};
protected:
private:
};

//---------------------------------

#include	"CommonCom.h"

#define MaxSPPack 2048

#pragma pack(push,1)
// Ñòðóêòóðà îïèñûâàþùàÿ ïàðàìåòðû Þíèòîâ ñèñòåìû
struct	UnitInfo
{
	INT64		PersID;			// Íàçíà÷àåìûé èíòåãðàòîðîì èäåíòèôèêàòîð ÷åðåç GetUID() ïðè ðåãèñòðàöèè
	Decoder*	pObj;			// Óêàçàòåëü íà ìåòîä ñåðâåðà äåêîäèðîâàíèÿ (äëÿ îáðàòíîé ñâÿçêè ñ îáúåêòîì)
#ifndef	_WIN64
	int			z1;
#endif
	int			NumChan;		// Channel Number 
	int			TractID;		// Èäåíòèôèêàòîð òðàêòà ñåðâåðà äåêîäèðîâàíèÿ
	char		Host[80];		// Èìÿ õîñòà
	uint		IP;				// IP õîñòà
	char		UnitName[40];	// Ïîñòîÿííîå èìÿ ñåìåéñòâà óñòðîéñòâ
	char		Mode[40];		// Ðåæèì ðàáîòûE
	char		State[40];		// Ñîñòîÿíèå
	char		DomainName[40];		// Ñîñòîÿíèå
	FileTime	CrTime;
	FileTime	ModTime;
// ---------- New parametrs ----------
//	float		LoadsCPU;
//	float		LoadsRAM;
	INT64		OwnerID;
	INT64		DeviceEnPoiID;
// -----------------------------------
				UnitInfo()
				{
					PersID=-1;
					pObj=NULL;
					NumChan=0;
					TractID=-1;
					Host[0]=0;
					IP=0;
					UnitName[0]=0;
					Mode[0]=0;
					State[0]=0;
					DomainName[0]=0;
					CrTime.Time=0;
					ModTime.Time=0;

					OwnerID=-1;DeviceEnPoiID=-1;
//					LoadsCPU=LoadsRAM=0.0;
				};
};


struct iri_SeansBegin			// íà÷àëî ñåàíñà
{
	FILETIME time;		// âðåìÿ âîçíèêíîâåíèÿ ñîáûòèÿ
	INT64 ID_Seans;		// èäåíòèôèêàòîð ñåàíñà
	INT64 ID_LogCh;		// èäåíòèôèêàòîð ëîãè÷åñêîãî êàíàëà
	int id_reason;			// îñíîâàíèå äëÿ íà÷àëà
};

struct iri_ContentType		// îïðåäåëåíèå âèäà íàãðóçêè
{
	FILETIME time;		// âðåìÿ âîçíèêíîâåíèÿ ñîáûòèÿ
	INT64 ID_Seans;		// èäåíòèôèêàòîð ñåàíñà
	INT64 ID_LogCh;		// èäåíòèôèêàòîð ëîãè÷åñêîãî êàíàëà
	int id_type;			// data/fax/voice
};

#define SEANS_TYPE_UNKNOWN	0
#define SEANS_TYPE_VOICE	1
#define SEANS_TYPE_FAX 		2
#define SEANS_TYPE_DATA		3
#define SEANS_TYPE_ISDN		4

struct iri_SeansEnd			// çàâåðøåíèå ñåàíñà
{
	FILETIME time;		// âðåìÿ âîçíèêíîâåíèÿ ñîáûòèÿ
	INT64 ID_Seans;		// èäåíòèôèêàòîð ñåàíñà
	INT64 ID_LogCh;		// èäåíòèôèêàòîð ëîãè÷åñêîãî êàíàëà
	INT64 ID_File;		// èäåíòèôèêàòîð ôàéëà
	char UnitID[40];		// UnitID ôàéëîâîãî ñåðâåðà
	int ReturnCode;		// êîä çàâåðøåíèÿ
};
// -------------------------------------------------------------------- 

// Ñîáûòèÿ, ïðèâÿçàííûå ê ïîòîêó
struct iri_StreamEvent		// âêëþ÷åíèå/âûêëþ÷åíèå ïîòîêà
{
	FILETIME time;		// âðåìÿ âîçíèêíîâåíèÿ ñîáûòèÿ
	int Event;			// âêëþ÷åíèå/âûêëþ÷åíèå
	INT64 ID_Stream;		// èäåíòèôèêàòîð ïîòîêà
	int InputDevice;		// íîìåð âõîäíîãî óñòðîéñòâà
	int InputNum;			// íîìåð ôèçè÷åñêîãî âõîäà
};

// Ñîáûòèÿ, ïðèâÿçàííûå ê ëîãè÷åñêîìó êàíàëó
struct iri_LogChEvent			// âêëþ÷åíèå/âûêëþ÷åíèå ëîãè÷åñêîãî êàíàëà
{
	FILETIME time;		// âðåìÿ âîçíèêíîâåíèÿ ñîáûòèÿ
	int Event;			// âêëþ÷åíèå/âûêëþ÷åíèå
	INT64 ID_LogCh;		// èäåíòèôèêàòîð ëîãè÷åñêîãî êàíàëà
	INT64 ID_Stream;		// èäåíòèôèêàòîð ïîòîêà
	int TimeSlot;			// íîìåð âðåìåííîãî èíòåðâàëà
};

struct iri_SimpleEvent		// çàíÿòèå êàíàëà
{
	FILETIME time;		// âðåìÿ âîçíèêíîâåíèÿ ñîáûòèÿ
	INT64 ID_LogCh;		// èäåíòèôèêàòîð ëîãè÷åñêîãî êàíàëà
	int param;			// îïèñûâàåòñÿ â êàæäîì êîíêðåòíîì ñëó÷àå
};

struct iri_SendPhoneNumb		// ïåðåäà÷à íîìåðíîé èíôîðìàöèè
{
	FILETIME time;		// âðåìÿ âîçíèêíîâåíèÿ ñîáûòèÿ
	INT64 ID_LogCh;		// èäåíòèôèêàòîð ëîãè÷åñêîãî êàíàëà
	int type;			// cnr/dnr
	char phone[32];		// phone number
};


#define PHONETYPE_UNKNOWN		0
#define PHONETYPE_CNR 			1
#define PHONETYPE_DNR 			2

//#define	STS_SeansCalledN	0x00030011//Âûçûâ-ìûé íîìåð 0x00030011
//#define	STS_SeansCallingN	0x00030012//Âûçûâ-ùèé íîìåð 0x00030012
//#define	STS_SeansConnectN	0x00030013//Ïðèñîåä-íûé íîìåð 0x00030013
//#define	STS_SeansRedirctN	0x00030014//Ïåðåíàïðàâë-íûé íîìåð 0x00030014
//#define	STS_SeansOriginN	0x00030015//Ïåðåíàïðàâë-íûé íîìåð 0x00030014

struct iri_DTMFdigit			// îïðåäåëåíèå òîíà DTMF
{
	FILETIME time;		// âðåìÿ âîçíèêíîâåíèÿ ñîáûòèÿ
	INT64 ID_LogCh;		// èäåíòèôèêàòîð ëîãè÷åñêîãî êàíàëà
	int cipher;			// öèôðà òåëåôîííîãî íîìåðà
};

struct iri_Criterion			// ñðàáàòûâàíèå êðèòåðèÿ îòáîðà
{
	FILETIME time;		// âðåìÿ âîçíèêíîâåíèÿ ñîáûòèÿ
	INT64 ID_LogCh;		// èäåíòèôèêàòîð ëîãè÷åñêîãî êàíàëà
	int id_obj;			// èäåíòèôèêàòîð îáúåêòà
};
// -------------------------------------------------------------------- 

// Ñîáûòèÿ ïåðåäà÷è ïàðàìåòðîâ ëîãè÷åñêîãî êàíàëà è ñåàíñà

struct iri_Param			// ïåðåäà÷à ïàðàìåòðà ëîãè÷åñêîãî êàíàëà ñåàíñà
{
	FILETIME time;		// âðåìÿ âîçíèêíîâåíèÿ ñîáûòèÿ
	INT64 id;			// èäåíòèôèêàòîð ëîãè÷åñêîãî êàíàëà èëè ñåàíñà
	PrmDec param;		// ïàðàìåòð
};

// -------------------------------------------------------------------- 
// --------       Ñîáûòèÿ/êîìàíäû TOI-èíòåðôåéñà
// -------------------------------------------------------------------- 

// Ñîáûòèÿ/êîìàíäû ìîäèôèêàöèè òàáëèöû íàáëþäåíèÿ
struct iri_toi_Object
{
	int id_obj;			// èäåíòèôèêàòîð îáúåêòà
	char phone[32]; 		// òåëåôîííûé íîìåð
	int prior_voice;		// ïðèîðèòåò voice
	int prior_fax;			// ïðèîðèòåò fax
	int prior_data;			// ïðèîðèòåò modem
	int prior_isdn;			// ïðèîðèòåò ISDN
};

// -------------------------------------------------------------------- 
// --------       Ñîáûòèÿ/êîìàíäû RT-èíòåðôåéñà
// -------------------------------------------------------------------- 
// Ñîáûòèÿ/êîìàíäû RT-íàáëþäåíèÿ
struct rt_AudioCtrlReq		// çàïðîñ àóäèîêîíòðîëÿ
{
	INT64 ID_LogCh;		// èäåíòèôèêàòîð ëîãè÷åñêîãî êàíàëà
	int fmt;				// ôîðìàò âûäà÷è àóäèîäàííûõ
	int ch;				// êîëè÷åñòâî êàíàëîâ 1-mono, 2-stereo
};
struct rt_AudioCtrlData		// ïåðåäà÷à àóäèîêîíòðîëÿ
{
	INT64 ID_LogCh;		// èäåíòèôèêàòîð ëîãè÷åñêîãî êàíàëà
	
	int fmt;			// ôîðìàò àóäèîäàííûõ
	int ch;				// êîëè÷åñòâî êàíàëîâ 1-mono, 2-stereo
	int len;			// ðàçìåð áëîêà àóäèîêîíòðîëÿ (áàéò)
	UCHAR data;			// áëîê àóäèîêîíòðîëÿ (1 áàéò äàííûõ)
};



#define RT_FMT_PCM	1 // PCM 16 bit
#define RT_FMT_PCMA	6 // PCM 8 bit A-law companding

// ïðè ïåðåñûëêå äàííûõ â ðåæèìå ñòåðåî îòñ÷åòû ëåâîãî è ïðàâîãî êàíàëîâ 
// âûñòðàèâàþòñÿ ïîî÷åðåäíî




// -------------------------------------------------------------------- 
// --------       Ñòðóêòóðû èíòåðôåéñà SrvTlf,SOP,Consol
// (êàíàë ïåðåäà÷è ïàðàìåòðîâ, ïðåäóïðåæäåíèé è îøèáîê)
// -------------------------------------------------------------------- 

//	Çíà÷åíèÿ ïàðàìåòðà Protocol
//#define STSProtocol_Ukn		0
//#define STSProtocol_SS7ISUP	1
//#define STSProtocol_EDSS		2
//#define STSProtocol_SS5		3
//#define STSProtocol_1Vf		4
//#define STSProtocol_Energy	5

struct StrIDStream	//Äëÿ êîìàíä STS_BegLogStream,STS_EndLogStream
{
	INT64 IDStream;		//Èäåíòèôèêàòîð ïîòîêà ñ êîíôèãóðàöèè(PrmIDStream)
	INT64 IDFileStream;	//Íàçíà÷àåìûé óíèêàë.ID èñòî÷íèêîì
	INT64 TimeEvent;	//Âðåìÿ âîçíèêíîâåíèÿ ñîáûòèÿ
};

struct StrIDLogChannel//Äëÿ êîìàíä STS_BegLogChannel,STS_EndLogChannel
{
	INT64 IDFileStream;	//Íàçíà÷àåìûé óíèêàë.ID ïîòîêà èñòî÷íèêîì
	INT64 IDLogChannel;	//Èäåíòèôèêàòîð êàíàëà ñ êîíôèãóðàöèè(PrmIDLogChannel)
	INT64 IDFileLogCh;	//Íàçíà÷àåìûé óíèêàë.ID êàíàëà èñòî÷íèêîì
	INT64 TimeEvent;	//Âðåìÿ âîçíèêíîâåíèÿ ñîáûòèÿ
	INT64 NumLogChannel;//Íîìåð ëîãè÷åñêîãî êàíàëà
};
// 
// struct  StrIDLogChannelSS7 : public StrIDLogChannel
// {
// 	INT64				IDLogChannelSS7;
// };

struct  StrIDLogChannelOtherSS : public StrIDLogChannel
{
	INT64				IDLogChannelSS7;
	INT64				TypeSS;
};


struct StrQueryNewSns//Äëÿ êîìàíä STS_QueryNewSeans
{	
	INT64 IDFileLogCh;	//Íàçíà÷àåìûé óíèêàë.ID êàíàëà èñòî÷íèêîì
	INT64 Protocol;		//Ïðîòîêîë
	INT64 TimeEvent;	//Âðåìÿ âîçíèêíîâåíèÿ ñîáûòèÿ
};

struct OpenNewSns//Äëÿ êîìàíä STS_OpenNewSeans
{
	INT64 IDFileLogCh;	//Íàçíà÷àåìûé óíèêàë.ID êàíàëà èñòî÷íèêîì
	INT64 IDSeansFrom;	//Íàçíà÷àåìûé óíèêàë.ID ñåàíñà ýòîãî êàíàëà ST
	INT64 IDSeansTo;	//Íàçíà÷àåìûé óíèêàë.ID ñåàíñà äëÿ îáðàòíîãî êàíàëà ST
	INT64 Protocol;		//Ïðîòîêîë
	INT64 TimeEvent;	//Âðåìÿ âîçíèêíîâåíèÿ ñîáûòèÿ
};

struct StrCloseSns//Äëÿ êîìàíä STS_CloseSeans,STS_EndSeans
{
	INT64 IDFileLogCh;	//Íàçíà÷àåìûé óíèêàë.ID êàíàëà èñòî÷íèêîì
	INT64 IDSeans;	//Íàçíà÷àåìûé óíèêàë.ID ñåàíñà ST
	int	  StateClose;//Óñëîâèÿ (ñîñòîÿíèå) çàêðûòèÿ
	INT64 TimeEvent; //Âðåìÿ âîçíèêíîâåíèÿ ñîáûòèÿ
};



struct StrOpenSnsGSM//Äëÿ êîìàíä STS_OpenNewSeans
{
	INT64 IDFileLogCh;	//Íàçíà÷àåìûé óíèêàë.ID êàíàëà èñòî÷íèêîì
	ushort	TEI;		//0 to Period/16
	ushort	LogCh;		//0 to 16 
	ushort	TypeCh;		//0-FR, 1-HR
	short	OffsRealStr;//0 to (Period-1) - if present, else =-1

	INT64 IDSeansFrom;	//Íàçíà÷àåìûé óíèêàë.ID ñåàíñà ýòîãî êàíàëà ST
	INT64 IDSeansTo;	//Íàçíà÷àåìûé óíèêàë.ID ñåàíñà äëÿ îáðàòíîãî êàíàëà ST
	INT64 Protocol;		//STS_TypAbis - for Abis
	INT64 TimeEvent;	//Âðåìÿ âîçíèêíîâåíèÿ ñîáûòèÿ
	INT64 IDFileStream;
	int Handover;
};

struct StrCloseSnsGSM//Äëÿ êîìàíä STS_CloseSeans,STS_EndSeans
{
	//INT64 IDFileLogCh;	//Íàçíà÷àåìûé óíèêàë.ID êàíàëà èñòî÷íèêîì
	ushort	TEI;		//0 to Period/16
	ushort	LogCh;		//0 to 16
	int	  StateClose;//Óñëîâèÿ (ñîñòîÿíèå) çàêðûòèÿ
	INT64 IDSeans;	//Íàçíà÷àåìûé óíèêàë.ID ñåàíñà ST	
	INT64 TimeEvent; //Âðåìÿ âîçíèêíîâåíèÿ ñîáûòèÿ
	INT64 IDFileStream;
};


struct StrSetChInfGSM//Äëÿ êîìàíä STS_CloseSeans,STS_EndSeans
{	
	ushort	TEI;		//0 to Period/16
	ushort	LogCh;		//0 to 16
	int		OffsRealStr;
	INT64	IDFileStream;
};


struct StrDataSign//Äëÿ êîìàíä STS_SignalizPack
{
	INT64	IDFileLogCh;	//Íàçíà÷àåìûé èñòî÷íèêîì óíèêàë.ID êàíàëà ;
	INT64	Protocol;		//Ïðîòîêîë
	int		CntErrHDLC;		//Êîëè÷åñòâî îøèáî÷íûõ ïàêåòîâ HDLC
	int		CntPack;		//Êîëè÷åñòâî ïðàâèëüíûõ ïàêåòîâ HDLC â Buff
	INT64   TimeInit;       //Âðåìÿ âîçíèêíîâåíèÿ ñîáûòèÿ
	char	Buff[32*1024];		//Äàííûå: [Len0(int),Data0,...,[LenX(int),DataX]],0(int)
};

// struct StrVCli//Äëÿ êîìàíä STS_VConsConnCli,STS_VConsDiscCli
// {
// 	int	  NCliST;			//Íîìåð CliST âíóòðè ST
// 	INT64 TimeEvent;		//Âðåìÿ âîçíèêíîâåíèÿ ñîáûòèÿ
// };
// 
// struct StrVCliUI//Äëÿ êîìàíä STS_VSetUnitInfo
// {
// 	int		NCliST;			//Íîìåð CliST âíóòðè ST
// 	UnitInfo m_UICliST;		//UnitInfo CliST
// };

struct  StrVIDStream//Äëÿ êîìàíä STS_VBegLogStream,STS_VEndLogStream
{
	INT64 PersIDCli;
	StrIDStream m_Str;
	char NameSource[128];
};

struct StrTypeDataSns //Äëÿ êîìàíä STS_TypDataSeans
{
	INT64 IDFileLogCh;	//Íàçíà÷àåìûé óíèêàë.ID êàíàëà èñòî÷íèêîì
	INT64 IDSeans;	//Íàçíà÷àåìûé óíèêàë.ID ñåàíñà ST
//	INT64 IDSeansFrom;	//Íàçíà÷àåìûé óíèêàë.ID ñåàíñà ýòîãî êàíàëà ST
//	INT64 IDSeansTo;	//Íàçíà÷àåìûé óíèêàë.ID ñåàíñà äëÿ îáðàòíîãî êàíàëà ST
	int	  TypeData;	//Òèï äàííûõ
	INT64 TimeEvent; //Âðåìÿ âîçíèêíîâåíèÿ ñîáûòèÿ

};
//--Çíà÷åíèå TypeData
//#define	STS_TypDataVoice	0x0003001d
//#define	STS_TypDataISDN		0x0003001e
//#define	STS_TypDataModem	0x0003001f
//#define	STS_TypDataFax		0x00030020
//#define	STS_TypData			0x00030021


//<ID_File-INT64><SizBuff-int><Buff-áóôôåð ñ äàííûìè>
#define MaxSizSendData	8192
struct  StrDataIDFile
{
	INT64 IDFile;	//Íàçíà÷àåìûé óíèêàë.ID êàíàëà(ïîòîêà) èñòî÷íèêîì
	INT64 TimeEvent; //Âðåìÿ âîçíèêíîâåíèÿ ñîáûòèÿ
	int	  CntD;			//Ðàçìåð Buff
	char  Buff[MaxSizSendData];//Size ìîæåò áûòü áîëüøå
};


struct StrSendPhoneNumb		// ïåðåäà÷à íîìåðíîé èíôîðìàöèè íà VCon - STS_SendSeansNumb
{
	INT64	TimeEvent;			// âðåìÿ âîçíèêíîâåíèÿ ñîáûòèÿ
	INT64	IDFileLogCh;		//Íàçíà÷àåìûé óíèêàë.ID êàíàëà èñòî÷íèêîì
	INT64	IDSeans;			//Íàçíà÷àåìûé óíèêàë.ID ñåàíñà ST
	int		type;				// cnr/dnr STS_Seans<X>N
	char	phone[32];			// phone number
	int		FlForward;			// for Imitator R2D
};

//--	type	---
//#define	STS_SeansCalledN	0x00030011//Âûçûâ-ìûé íîìåð 0x00030011
//#define	STS_SeansCallingN	0x00030012//Âûçûâ-ùèé íîìåð 0x00030012
//#define	STS_SeansConnectN	0x00030013//Ïðèñîåä-íûé íîìåð 0x00030013
//#define	STS_SeansRedirctN	0x00030014//Ïåðåíàïðàâë-íûé íîìåð 0x00030014
//#define	STS_SeansOriginN	0x00030015//Ïåðåíàïðàâë-íûé íîìåð 0x00030014

struct  RecordOT
{
	INT64 Nobj;		//óíèê.íîìåð îáúåêòà äëÿ ýòîé òàáëèöû
	char MaskPhone[32]; //ìàñêà òåë.íîìåðà äëÿ îòáîðà â ôîðìàòàõ: 
	//1-"*"
	//2-"<âåñü íîìåð â öèôðàõ>"
	//3-"*<öèôðû>"
	//4-"<öèôðû>*"
};
struct StrSendPhoneOT //ïåðåäà÷à íîìåðíîé èíôîðìàöèè
{
	INT64	IDSeans;//Íàçíà÷àåìûé óíèêàë.ID ñåàíñà ST
	int	type;	// òèï íàìåðà STS_Seans<X>N (ïð.: STS_SeansCalledN)
	char	phone[32];			// phone number
};
struct StrSendNObjOT //ïåðåäà÷à íîìåðà îáúåêòà ñ ÎÒ äëÿ ñåàíñà
{
	INT64 IDSeans;//Íàçíà÷àåìûé óíèêàë.ID ñåàíñà ST
	INT64 Nobj;		//óíèê.íîìåð îáúåêòà â ÎÒ
};

struct StrObjAudData		// ïåðåäà÷à àóäèîäàííûõ â ôîðìàòå PCM16
{
	INT64 IDSeans;	// èäåíòèôèêàòîð ñåàíñà ST(èëè IDFile LCh)
	int ch;			// êîëè÷åñòâî êàíàëîâ 1-mono, 2-stereo
	int len;		// ðàçìåð áëîêà àóäèîêîíòðîëÿ (áàéò)
	UCHAR data[MaxSizSendData*4];	// áëîê àóäèîêîíòðîëÿ (1 áàéò äàííûõ)
};

//#define	STS_SendHostIP		0x00030053//Ïîññûë HostIP (ST->VCon)
////(int)NumCi,(int)IPCli
//#define	STS_SendHostName	0x00030054//Ïîññûë HostName (ST->VCon)
//(int)NumCi,(char[48])HostName
struct StrSendHostIP
{
	int NCli;
	char IP[16];
};

struct StrSendHostName
{
	int NCli;
	char HName[48];
};


// -------------------------------------------------------------------- 
// --------       Ñîáûòèÿ/êîìàíäû TLM-èíòåðôåéñà
// (êàíàë ïåðåäà÷è ïàðàìåòðîâ, ïðåäóïðåæäåíèé è îøèáîê)
// -------------------------------------------------------------------- 

#define ID_PARAM_STREAM_QUALITY		1
#define ID_PARAM_SLINK_QUALITY		2
#define ID_PARAM_TIP_UTILIZATION	3
#define ID_PARAM_FS_UTILIZATION		4

#define ID_ERROR_INPDEV	1	// Îøèáêà âõîäíîãî óñòðîéñòâà
#define ID_ERROR_FSFULL	2	// Ïåðåïîëíåíèå ôàéëîâîãî íàêîïèòåëÿ

// èäåíòèôèêàòîðû ïàðàìåòðîâ, ïðåäóïðåæäåíèé è îøèáîê ìîãóò áûòü äîïîëíåíû
// â õîäå ñîâìåñòíîé ðàáîòû ïî èíòåãðàöèè FE and BE

struct tlm_Event			// Êàíàë ïåðåäà÷è ïàðàìåòðîâ
{
	char	UnitName[80];	// Ïîñòîÿííîå èìÿ ñåìåéñòâà óñòðîéñòâ
	char	UnitID[40];		// Ïîñòîÿííûé èäåíòèôèêàòîð äëÿ êàæäîãî êîíêðåòíîãî óñòðîéñòâà
	int	ID_Param;		// èäåíòèôèêàòîð ïàðàìåòðà
	int	iValue;			// çíà÷åíèå Alarm, Error
	float	fValue;			// çíà÷åíèå Param
};

struct TabSysCli : public UnitInfo
{
	int		Fl;
//	char	IP[16];
	//	è äð.
};


struct	UpDateBuf
{
	int			Cli;
	TabSysCli	TSC;
};



/*
struct	CliUnitInfo : public UnitInfo	// Ñîáûòèå îò êëèåíòà ñèñòåìíîãî èíòåãðàòîðà
{
	int			Event;	// 0 - NewUnit  1 - UnitClose
	TabSysCli*	TSC;
};
*/

struct	SysPack
{
	int		N_Cli;
	int		Cnt;
	int		Type;
	char	Prm[MaxSPPack];
};


struct	StrGetFile
{
	char		TypeFile[9];
};

struct	StrAddFile
{
	INT64		IDSeans;
	char		NameFile[128];
	int			LenFile;
	StrGetFile	TFile;
	int			Band;
	INT64		IDParentFile;
};

struct	StrSendFile
{
	INT64		IDFile;
	StrAddFile	NameInfo;
};

struct	StrSendDupFile
{
	INT64		IDFile0;
	StrAddFile	NameInfo0;
	INT64		IDFile1;
	StrAddFile	NameInfo1;
};

struct	StrExcludeFile				// Äëÿ Èíìàðñàòà
{
	INT64		IDFile;
};

struct	 StrSendInf
{
	INT64		IDFile;
	INT64		FlInf;
};

struct	StrAddParam
{
	INT64		IDFile;
	PrmDec		PrmDecod;
};

struct	StrAddParamFull
{
	INT64		IDFile;
	char		NameParam[40];
	int			CntVol;
	char		Vol[1024];
};


struct	StrAddParamBandleList
{
	INT64		IDFile;
	//char		NameParam[40];
	int			CntBuf;
	uchar		Buff[1024];
};

struct	DataProc	: UnitInfo	// Ñòðóêòóðà îïèñûâàþùàÿ Ïðîöåññîðû îáðàáîòêè äàííûõ
{
	char	ID_WC[80];
	char	TypeFile[9];
	char	OutTypeFile[9];
	char	SrvUnitName[80];	
	char	SrvUnitID[40];	
};

struct	StrSetARCFN
{
	INT64 UID;
	int   ARCFN;
};
//  [8/23/2007 San] For MAO BD
//////////////////////////////////////////////////////////////////////////
enum ValueTypeArifmetic
{
	TypeGetAny=0,			//*(âñå èìåþùèå äàííûå ñ äàííûì IDPrm,à Value èãíîðèðóåòñÿ)
	Equil,					//=
	GreaterThan,			//>
	LessThan,				//<
	EqualToOrGreaterThan,	//>=
	EqualToOrLessThan,		//<=
	Unequal,				//!=
	WithoutPrm				//!*(äàííûå íå èìåþùèå äàííûé IDPrm)
};

enum ValueTypeLogical
{
	NoTypeLogical=0,		//* íåò óñëîâèé ??????
	LogicalAND,		//&&
	LogicalOR		//||
};


struct StrSimpleQBE 
{
	ushort	iIDParametr;			//ID ïàðàìåòðà äëÿ çàïðîñà 
	uchar	ValueParam[512];		//çíà÷åíèÿ ïàðàìåòðà â òèïå çàçíà÷åííîì äëÿ ïàðàìåòðà (â ñòðîêîâîì âèäå-???,åñëè íåò äàííûõ,òî âûäàòü âñå èìåþùèå äàííûå ñ äàííûì IDPrm)
	uchar	TypeArifmetic;			//0 - *(âñå èìåþùèå äàííûå ñ äàííûì IDPrm,à Value èãíîðèðóåòñÿ),1 - =,2 - >,3 - <,4 - >=,5 - <=, 6 - !=
	uchar	TypeLogical;			//åñëè áóäóò ñëåäóþùèå çàïðîñû òî ëîãè÷åñêèå óñëîâèÿ, 0 - íåò,1 - &&,2 - ||  
	ushort  Reserv;					//ðåçåðâ äëÿ áîëåå ñëîæíûõ çàïðîñîâ (äîïîëí.ñêîáêè, äîïîëí. óñëîâèÿ)
};

struct StrBDSimpleQBE
{
	int				Cnt;
	StrSimpleQBE	QBE;
};

struct StrSimpleCS 
{
	int		Weight;
	int		LenBuf;	
	uchar	Buf[512];
};

//////////////////////////////////////////////////////////////////////////



struct	PackForBD
{
	INT64 	IDSeans;					//Íàçíà÷àåìûé óíèêàë.ID ñåàíñà BDSrv
	INT64   IDSeansFr,IDSeansTo;		//Íàçíà÷àåìûé óíèêàë.ID ñåàíñà ST(ñîðò. Fr<To )	
	char 	NameSP[32];					//Èìÿ ÑÎÏà
	INT64   IDStream0,IDStream1;		//Èäåíòèôèêàòîð ïîòîêà (ñîðò. â IDStream0-Fr)
	INT64   IDLogCh0,IDLogCh1;			//Èäåíòèôèêàòîð êàíàëà 
	int 	Tslot0,Tslot1;				//òàéì-ñëîò
	int 	SlotSelect0,SlotSelect1;	//âûáîðêà èç òàéì-ñëîòà(20,21,22,23-äëÿ 2-áèò…) 
	
	char	CalledN[32];				//DNR,Âûçûâàåìûé íîìåð
	char	CallingN[32];				//ÑNR,Âûçûâàþùèé íîìåð
	char	ConnectN[32];				//Ñîåäèíåííûé íîìåð (ïðåäïîëîæèòåëüíî)
	char	RedirectionN[32];			//Íîìåð ïåðåàäðåñàöèè
	char	LocationN[32];				//location number (íîìåð ìåñòîïîëîæåíèÿ)
	char	OriginalN[32];				//original called number (ïåðâîíà÷àëüíûé âûçûâàåìûé íîìåð): 
	char	RedirectingN[32];			//Íîìåð ïåðåíàïðàâëåíèÿ
	
	char	NetPath[80];				//ñåòåâîé ïóòü
	INT64 	IDFile_PCM;					//Èäåíòèôèêàòîð ñîçäàííîãî ôàéëà PCM
	INT64 	IDFile_GSM;					//Èäåíòèôèêàòîð ñîçäàííîãî ôàéëà GSM
	
	char	TypeData[20];				//Òèï äàííûõ
	
	FILETIME	TimeBegin;				//âðåìÿ íà÷àëà ñåàíñà
	FILETIME	TimeEnd;				//âðåìÿ îêîí÷àíèÿ ñåàíñà
	
	int 	CauseEnd;					//ïðè÷èíà îêîí÷àíèÿ ñåàíñà	
	char 	Protocol[20];				//Òèï ñèãíàëèçàöèè.
	
	short	DPC,OPC;

	char	CallingN_1[32];				//ÑNR,Âûçûâàþùèé íîìåð_2
	char	RedirectionN_1[32];			//äîïîëí.íîìåð ïåðåàäðåñàöèè_2
	char	RedirectionN_2[32];			//äîïîëí.íîìåð ïåðåàäðåñàöèè_3
	char	RedirectionN_3[32];			//äîïîëí.íîìåð ïåðåàäðåñàöèè_4
	char	RedirectionN_4[32];			//äîïîëí.íîìåð ïåðåàäðåñàöèè_5 
	char	RedirectionN_5[32];			//äîïîëí.íîìåð ïåðåàäðåñàöèè_6 
	char	RedirectionN_6[32];			//äîïîëí.íîìåð ïåðåàäðåñàöèè_7 
	char	RedirectionN_7[32];			//äîïîëí.íîìåð ïåðåàäðåñàöèè_8 
	char	RedirectionN_8[32];			//äîïîëí.íîìåð ïåðåàäðåñàöèè_9 
	char	RedirectionN_9[32];			//äîïîëí.íîìåð ïåðåàäðåñàöèè_10 
	char	RedirectionN_10[32];		//äîïîëí.íîìåð ïåðåàäðåñàöèè_11 
	
	char	rez[32];

	PackForBD()
	{
		memset(this,0,sizeof(PackForBD));
		SlotSelect0=SlotSelect1=80;
		DPC=OPC=0;
	}
};

struct StrSendFRecFromRPU
{
	int ch;
	int frec;
	double fn;
};

//////////////Thuraya////////////////////////////////////////////////////////////
struct StrModeStream 
{
	INT64	IDFileStream;
	int		ModeStream;//0-Ukn,1-Static,2-Dynamic
	int		StateStream;//0-Off,1-On,2-Wait
	INT64	TimeEvent;
};


struct StrImmAssignPh
{
	INT64	IDFileStream;
	INT64	TimeEvent;	
	int		ARFCN;
};

struct StrImmAssignTS
{
	INT64	IDFileStream;
	INT64	TimeEvent;
	INT64	IDSeansFrom;//Íàçíà÷àåìûé óíèêàë.ID ñåàíñà ýòîãî êàíàëà ST
	INT64	IDSeansTo;	//Íàçíà÷àåìûé óíèêàë.ID ñåàíñà äëÿ îáðàòíîãî êàíàëà ST
	INT64	Protocol;	//Ïðîòîêîë

	int ARFCN;
	int TS;
	int KAB_Loc;
	int ChType;
	int	TimeOffs;
	int FreqOffs;
	int	GPS_Discr;
};

struct SendFd_Finist
{
	int CardNum;
	double Fd;
};

struct Data_THUR_Dec
{
	UCHAR	Data[64];
	UINT	LogChannel;
	UINT	FrameNum[4];
	UCHAR	DataType;
	UCHAR	FlagCiph;
	UCHAR	FlagCRC1;
	UCHAR	FlagCRC2;
};

#define Data_THUR_Dec_Undef		0
#define Data_THUR_Dec_BCCH		1
#define Data_THUR_Dec_PCH		2
#define Data_THUR_Dec_RACH		3
#define Data_THUR_Dec_GBCH		4
#define Data_THUR_Dec_FACCH3	5
#define Data_THUR_Dec_FACCH6	6
#define Data_THUR_Dec_FACCH9	7
#define Data_THUR_Dec_TCH3		8
#define Data_THUR_Dec_TCH6		9
#define Data_THUR_Dec_TCH9		10

struct SendDigitRec_Frec
{
	int   type;
	int   nChan;
	INT64 IDFile;
	int   Frec;
};
struct SendDigitRec_AGC
{
	int   type;
	int   nChan;
	INT64 IDFile;
	int   Agc;
};
struct SendDigitRec_Fd
{
	int   type;
	int   NTaps;
};
//////////////////////////////////////////////////////////////////////////
//#pragma pack(push,1)
// [10/18/2006 San] Pilot-K
struct HeadMessPK
{
	//	ushort	LenMess;
	INT64	NumModule;
	ushort	NumMess;
};

struct HeadParamPK
{
	//	ushort	LenMess;
	ushort	NumParam;
	ushort	Len;
};



#pragma pack(pop)


struct StructCtrlDomainCMD
{
	char	DomainName[40];
	uint	MyNum;

	void Init(char* name=NULL)
	{
		if(name==NULL)
			DomainName[0]=0;
		else
			sprintf_s(DomainName,"%s",name);
	}

	void Init(uint num,char* name=NULL)
	{
		MyNum=num;
		if(name==NULL)
			DomainName[0]=0;
		else
			sprintf_s(DomainName,"%s",name);
	}
};


class		Integrator
{
public:

	int					State,Fl_ReConnect,FatalError;
	ClassThread			*CThread,*CThreadPAN,*CThreadLAN; //,*CThreadWAN;
	struct sockaddr_in	peerPAN;
	struct sockaddr_in	peerLAN;
	//	struct sockaddr_in	peerWAN1;
	//	struct sockaddr_in	peerWAN2;
	struct sockaddr_in	peerc;
	SOCKET				scLAN;	//,scc;
	SOCKET				ssPAN,ssLAN;//,ssWAN;
	SOCKET				scPAN;//,scWAN1,scWAN2;
	int					dgramsz;
	UDPPack				UDPP;
	UDPPack2			UDPP2;
	int					dontblock;
	char				host[256];
	char				LanBuf[1024];		// !!!! > UDPPack | UDPPack2
	UDPPack				bufLAN;			// Âõîäíîé áóôåð
	UDPPack2			bufLAN2;			// Âõîäíîé áóôåð
	UDPPack				bufPAN;				// Âõîäíîé áóôåð
	struct sockaddr		SourcePAN;
	struct sockaddr		SourceLAN;
	//	struct sockaddr		SourceWAN;
	int					SizeSource;

	UDPCli				*CliPAN[256];		//-----------------------


	HOSTENT				*hoststr;
	long				INetAdrPAN;
	long				INetAdrLAN;
	//	long				INetAdrWAN;
	FileTime			laPANSvrTime;
	FileTime			laLANSvrTime;
	FileTime			laLANSSvrTime;
	FileTime			laWANSvrTime;
	uint				MyLANNum,NumLANSrv;
	uint				NumLANSSrv;
	UCHAR				MyPANNum,NumPANSrv;
	FileTime			RTime;
	Services			*Srv;
	int					Flag,FlagSyn;
	int					NumPANCli;
	int					NumLANCli;
	int					NumWANCli;
	int					FlStop;
	int					Status;
	// --------------------------------
	//	int				TmpCnt;
	//	char				EventBuf[40];
	INT64				TimeAdd[256];
	FlagStruct			Fl_RE;
	int					NumberHost;
	char				ListHost[256][256];
	StrOwner			Anchor;
	ID_List8			CliLAN;				//-----------------------
	ID_List8			CliWAN;
	char				PortL[8];
	char				PortL2[8];
	char				PortP[8];
	char				PortP2[8];
	char				PortW[8];
	char				PortW2[8];
	int					Fl_ReInit;
	int					SendCnt;
	Integrator(char* PortLAN,char* PortPAN,char* PortWAN);
	~Integrator();
	virtual	void		Idle();	// Çàïóñêàåòñÿ âíåøíèì èñòî÷íèêîì
	virtual	int			IdleLAN();	// Çàïóñêàåòñÿ âíåøíèì èñòî÷íèêîì
	virtual	int			IdlePAN();	// Çàïóñêàåòñÿ âíåøíèì èñòî÷íèêîì
	//virtual	int		IdleWAN();	// Çàïóñêàåòñÿ âíåøíèì èñòî÷íèêîì
	virtual	void		OnLine();
	virtual	char*		FindHost(char* HostName);	// return Addr
	virtual	UDPCli*		FindCliByHost(char* HostName);
	virtual	El_CliInt*	AddWANCli(UDPPack* Buf);
	virtual	void		ProcLANCommand(UINT	Num);
	virtual	void		DelHostFromDomain(char* HostName,char* DomainName);
	virtual	float		GetLoadsCPU(UnitInfo* UI);
	virtual	float		GetLoadsRAM(UnitInfo* UI);
	virtual	int			SelectUnit(UnitInfo* UI,float LoadVol=5.0);
	virtual	int			SetUnitLoad(UnitInfo* UI,float LoadVol);

protected:
	virtual	void		ReInit();
};


class ID_ElHostList	:	public	ID_Elem
{
public:
	int			Vol;
	int			State;
	int			Fl;
	UpDateBuf	TSC;	// Ïîêà íå áîëåå ñòà êëèåíòîâ
	ID_ElHostList(int	tVol) 
	{
		Init(tVol);
	};
	virtual	void	Init(int	tVol)
	{
		Vol=tVol;
		Buff=(UCHAR*)&Vol;
		Len=sizeof(int);
		State=0; Fl=0;
	}
};

class El_Unit :	public	ID_Elem
{
public:
	UnitInfo	UInf;
	POSITION	pRow;
				El_Unit(UnitInfo* UI);
				~El_Unit();
virtual	void	Init(UnitInfo*	UI);
};

class El_UnitName	:	public	ID_Elem
{
public:
	char		UnitName[40];
	El_Unit*	ElU;
	ID_List8*	DomenList;
	El_UnitName(El_Unit *tElU);
	~El_UnitName();
	virtual	void	Init(El_Unit *tElU);
	virtual	void	AddDomen(El_Unit *tElU);
	virtual	int		DelDomen(El_Unit *tElU);
};

class El_DomName	:	public	ID_Elem
{
public:
	char		DomName[40];
	El_Unit*	ElU;
	ID_List8*	UnitList;
	El_DomName(El_Unit *tElU);
	~El_DomName();
	virtual	void	Init(El_Unit *tElU);
	virtual	void	AddUIEl(El_Unit* tElU);
	virtual	int		DelUIEl(El_Unit *tElU);
};


struct LicUnitInfo 
{
	int NumLic;
	int	NumReal;
};

class	UnitMng
{
public:
		ID_List8	*UnitList;
		ID_List8	*UnitNameList;
		Services	*Srv;
		FlagStruct	Fl_RE;
					UnitMng();
					~UnitMng();
virtual	void		AddUnit(UnitInfo* UI);
virtual	void		DelUnit(UnitInfo* UI);
//virtual	El_Unit*	FindUnit(char* UnitName, int UnitID=0);
virtual	int			CalcUnitNumber(char* UnitName,char* Mode=NULL);
virtual	ID_List8*	FindUnit(char* UnitName);
virtual	ID_List8*	FindUnit(char* UnitName, char* DomainName);
virtual	El_Unit*	FindUnit(char* UnitName, char* DomainName,char*	Host, char* Mode,int Condition=0);	// 0-balance 1-CPU 2-RAM 3-First
virtual	El_Unit*	FindUnitChan(char* UnitName, char* DomainName,char*	Host, char* Mode,int NumChan ,int Condition=0);	// 0-balance 1-CPU 2-RAM 3-First
virtual	El_Unit*	FindUnitbyPersID(INT64 PersID);
virtual	bool		TestMode(char* IP,char* Mode,char* iUnitName);
virtual	int			UnitSelect(El_Unit* ElUnit);
virtual	int			UnitSetLoad(El_Unit* ElUnit,float Vol);
};

class	SysIntMng
{
public:
	TabSysCli	*TSysCli;
	int			RLen;
	SysIntMng()
	{
		RLen=0;TSysCli=NULL;
	};
	~SysIntMng()
	{
		if (TSysCli!=NULL)
		{
			free(TSysCli);RLen=0;TSysCli=NULL;
		}
	};
	virtual void	Clear(void)
	{
		if (TSysCli!=NULL)
		{
			free(TSysCli);RLen=0;TSysCli=NULL; 
		}
	};
};

#define	NumSICli	256
#define	NumSockCli	256

struct	ClientsList
{
	int			Fl;
	int			Mode;	//	0 - êëèåíò SI; 1 - êëèåíò; 2 - ñåðâåð
//	int			RecMode;	// 0 - UserCallBack  1 - GetPack
	Decoder*	PObj;
	int			FlCon;
	int			Fl_Data;	// 0 - No send Data  1 - Send Data
};

struct	LogParam
{
	int				Fl;
	char			KeyVol[84];
	char			NDay[40];
	int				TypeTime;
};

class	El_DllFile :	public ID_Elem
{
public:
char			DllName[40];
char			HostName[40];
FileTime		Cr;
FileTime		Wr;
FileTime		Ac;
				El_DllFile(char* DllName,char* HostName,FileTime* Cr,FileTime* Ac,FileTime* Wr);
};

class	El_Log :	public ID_Elem
{
public:
LogParam		LPrm;
Savelog			*Log;
//char			*tBuf;
//int			RSizeBuf;
Services*		Srv;
HANDLE			hmtxlog;

				El_Log(LogParam* LP,char* PathLog,Services* pSrv);
				~El_Log();
virtual	void	OutLog(char* Buf,Decoder *Poi);
virtual	void	Idle();
};

class Log_List : public ID_List8
{
public:
	El_Log*		tElL;
//	int			CntV;
				Log_List(){/*CntV=0;*/};
virtual	void	Idle()
				{
//					CntV++;
					FindStrList8	FSL8;
					tElL=(El_Log*)Begin(FSL8);
					while (tElL!=NULL)
					{
						tElL->Idle();
						tElL=(El_Log*)Next(FSL8);
					};
//					CntV--;
				};
				~Log_List()
				{
					DelAllElem();
				};				
};

struct	SrvCfg
{
	INT64	UID;	// Òåêóùèé óíèêàëüíûé ID
	INT64	PID;	// Îïåðåæàþùèé ID
	INT64	OldFcpu;
	INT64	IDMainTab;	// ID ãëàâíîé òàáëèöû
};

class	El_Event :	public ID_Elem
{
public:
int				Handle;
Decoder			*PObj;
				El_Event(Decoder*	Obj,int	Hnd): ID_Elem((UCHAR*)&Handle,sizeof(int))
				{
					PObj=Obj;
					Handle=Hnd;
				};
virtual	void	Init(Decoder*	Obj,int	Hnd)
				{
					PObj=Obj;
					Handle=Hnd;
				}
};
/*
class	El_Dec :	public ID_Elem
{
public:
Decoder			*PObj;
char			NameMt[80];
int				Counter;
				El_Dec(Decoder*	Obj);
virtual	void	Init(Decoder*	Obj);
};
*/
class	El_Owner :	public ID_Elem
{
public:
	StrOwner		Own;
	char			NameMt[80];
	int				Counter;
					El_Owner(StrOwner*	Ow);
virtual	void		Init(StrOwner*	Ow);
};

struct	TlmData
{
		Decoder*	PObj;
		PVOID		pObjO;
		char		Prm[40];
		char		NameMt[80];
		int			ExtMt;
void	Set(Decoder* PO,char* P,PVOID POO=NULL);
};


class PL_Elem	:	public	ID_Elem
{
public:
	void*		pVol;
	int			Size;
	Decoder*	ParentMt;
	long		LWrTime;	// Ïðîñòî ñ÷åò÷èê !!!!!!!!!!!!!!!!!!!!!!!!!!!!!

	PL_Elem(char* IDName,int Cnt,Decoder* ParMt)
	{
		LWrTime=0;//	time(NULL);
		ParentMt=ParMt;
		Len=(INT64)strlen(IDName)+1;
		Buff=(UCHAR*)malloc((size_t)Len);
		strcpy((char*)Buff,IDName);
		Size=(Cnt/256+1)*256;
		if (Size==0)
		{
			pVol=NULL;
		}
		else
		{
			pVol=malloc(Size);*(UCHAR*)pVol=0;
		};
	};

	virtual	void Init(char* IDName,int Cnt,Decoder* ParMt)
	{
		LWrTime=0;//	time(NULL);
		ParentMt=ParMt;
		size_t	tLen=strlen(IDName)+1;
		if (Len!=tLen)
		{
			if (Buff==NULL)
			{
				Buff=(UCHAR*)malloc(tLen);
			}
			else
			{
				free(Buff);
				Buff=(UCHAR*)malloc(tLen);
//				Buff=(UCHAR*)realloc(Buff,tLen);		// !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
			}
			Len=tLen;
		}
		strcpy((char*)Buff,IDName);
		ReAlloc(Cnt,ParMt);
	};

	virtual	void ReAlloc(int Len,Decoder* ParMt)
	{
		LWrTime++;//	=time(NULL);
		Size=(Len/256+1)*256;
		ParentMt=ParMt;
		if (Size==0)
		{
			if (pVol!=NULL) free(pVol); 
			pVol=NULL;
		}
		else
		{
			if (pVol!=NULL) free(pVol); 
			pVol=malloc(Size);		// !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
			*(UCHAR*)pVol=0;	
		};
	};

	virtual	void SetElem(void* Buf,int Cnt,Decoder* ParMt)
	{
		LWrTime++;//	=time(NULL);
		if (Cnt>Size)
		{
			ReAlloc(Cnt,ParMt);
		}
		ParentMt=ParMt;
		memcpy(pVol,Buf,Cnt);
	};

	~PL_Elem() 
	{
		if (pVol!=NULL)
		{
			free(pVol);pVol=NULL;
		};
		if (Buff!=NULL)
		{
			free(Buff); Buff=0;
		}
	};
};

/*
class	ParamList
{
public:
ID_List8			pList;
int					FlNew;
long				LWrTime;	// Ïðîñòî ñ÷åò÷èê èçìåíåíèé !!!!!!!!!!!!!!!!!!!!!!!!!!!!!
					ParamList();
virtual				~ParamList();
virtual	void*		AllocParam(char* IDName,int Cnt,Decoder* ParentMt);	// ParentMt=this
virtual	int			SetParam(char* IDName,void* Vol,int Cnt,Decoder* ParentMt);		// ParentMt=this	rc=-1 íåóäà÷à
virtual	int			GetParam(char* IDName,void* &Vol,size_t &Cnt,Decoder* &ParentMt);		// rc=-1 íåóäà÷à Cnt=ðàçìåð ñòðóêòóðû íà êîòîðóþ óêàçûâàåò Vol-óêàçàòåëü
virtual	int			GetParam(char* IDName,void* &Vol,Decoder* &ParentMt);				// rc=-1 íåóäà÷à
virtual	int			GetParam(char* IDName,void* &Vol,size_t &Cnt);		// rc=-1 íåóäà÷à Cnt=ðàçìåð ñòðóêòóðû íà êîòîðóþ óêàçûâàåò Vol-óêàçàòåëü
virtual	int			GetParam(char* IDName,void* &Vol);				// rc=-1 íåóäà÷à
virtual	int			DelParam(char* IDName);							// rc=-1 íåóäà÷à
virtual	int			FindFirst(char* &IDName,void* &Vol,size_t &Cnt,Decoder* &ParentMt);	// rc=-1 íåóäà÷à
virtual	int			FindNext(char* &IDName,void* &Vol,size_t &Cnt,Decoder* &ParentMt);	// rc=-1 íåóäà÷à
virtual	int			FindFirst(char* &IDName,void* &Vol,size_t &Cnt);	// rc=-1 íåóäà÷à
virtual	int			FindNext(char* &IDName,void* &Vol,size_t &Cnt);	// rc=-1 íåóäà÷à
virtual	int			FindFirst(char* &IDName,void* &Vol,size_t &Cnt,long &mLWrTime);	// rc=-1 íåóäà÷à
virtual	int			FindNext(char* &IDName,void* &Vol,size_t &Cnt,long &mLWrTime);	// rc=-1 íåóäà÷à
//virtual	int			FindStart();									// rc=-1 íåóäà÷à
virtual	PL_Elem*	Find(UCHAR* IDName);	// Ïîèñê ýëåìåíòà è
					// óñòàíàâêà óêàçàòåëÿ ïîèñêà íà ïîçèöèþ ñîîòâåòñòâóþùóþ óñëîâèþ ïîèñêà
					// íåçàâèñèìî îò ðåçóëüòàòîâ ïîèñêà
};

*/

class PLID_Elem	:	public	ID_Elem
{
public:
	INT64  UID;
	void*		pVol;
	int			Size;
	Decoder*	ParentMt;
	long		LWrTime;	// Ïðîñòî ñ÷åò÷èê !!!!!!!!!!!!!!!!!!!!!!!!!!!!!
	Parametr*	pPrm;
//	int			MaxLen;
	PLID_Elem(INT64 ID,int Cnt,Decoder* ParMt)
	{
		LWrTime=0;//	time(NULL);
		ParentMt=ParMt;
		Len=sizeof(INT64);
		UID=ID;
		pPrm=GetTablParametrID((int)ID);
//		Buff=(UCHAR*)malloc(Len);
//		strcpy((char*)Buff,(char*)&ID);
		Buff=(UCHAR*)&UID;
		Size=Cnt;
		if (Size==0)
		{
			pVol=NULL;
		}
		else
		{
			pVol=malloc(Size);*(UCHAR*)pVol=0;
		};
	};

	virtual	void Init(INT64 ID,int Cnt,Decoder* ParMt)
	{
		LWrTime=0;
		ParentMt=ParMt;
//		int	tLen=sizeof(INT64);

		UID=ID;
		pPrm=GetTablParametrID((int)ID);
//		if (Len!=tLen)
//		{
//			if (Buff==NULL)
//			{
//				Buff=(UCHAR*)malloc(tLen);
//			}
//			else
//			{
//				free(Buff);
//				Buff=(UCHAR*)malloc(tLen);
//			}
//			Len=tLen;
//		}
//		strcpy((char*)Buff,(char*)&ID);
		ReAlloc(Cnt,ParMt);
	};

	virtual	void ReAlloc(int Len,Decoder* ParMt)
	{
		LWrTime++;
//		Size=Len;
		ParentMt=ParMt;
		if (Len==0)
		{
			if (pVol!=NULL) free(pVol); 
			pVol=NULL;
		}
		else
		{
			if(Len!=Size)
			{
				if(pVol!=NULL) free(pVol); 
				pVol=malloc(Len);
				Size=Len;
				*(UCHAR*)pVol=0;	
			}
		};
	};

	virtual	void SetElem(void* Buf,int Cnt,Decoder* ParMt)
	{
		if (pPrm)
		{
			if (pPrm->Size)
			{
				if (Cnt>pPrm->Size)
				{
					Cnt=pPrm->Size;
				}
			};
			LWrTime++;
			if (Cnt!=Size)
			{
				ReAlloc(Cnt,ParMt);
			}
			ParentMt=ParMt;
			memcpy(pVol,Buf,Cnt);
			if(pPrm->Type==PrmType_STRING)
			{
				((char*)pVol)[Cnt-1]=0;
			}
		}
	};

	~PLID_Elem() 
	{
		if (pVol!=NULL)
		{
			free(pVol);pVol=NULL;
		};
//		if (Buff!=NULL)
//		{
//			free(Buff); Buff=0;
//		}
	};
};

#define TypeOfParamListID	New
/*
#if	TypeOfParamListID != New	


class	ParamListID
{
public:
ID_List8			pList;
int					FlNew;
long				LWrTime;
FlagStruct			Fl_RE;
	// Ïðîñòî ñ÷åò÷èê èçìåíåíèé !!!!!!!!!!!!!!!!!!!!!!!!!!!!!
					ParamListID();
virtual				~ParamListID();
virtual	void*		AllocParam(INT64 ID,int Cnt,Decoder* ParentMt);	// ParentMt=this
virtual	int			SetParam(INT64 ID,void* Vol,int Cnt,Decoder* ParentMt);		// ParentMt=this	rc=-1 íåóäà÷à
virtual	int			SetParam(INT64 ID,char* Vol,Decoder* ParentMt) {return SetParam(ID,(void*)Vol,strlen(Vol)+1,ParentMt);};
virtual	int			GetParam(INT64 ID,void* &Vol,size_t &Cnt,Decoder* &ParentMt);		// rc=-1 íåóäà÷à Cnt=ðàçìåð ñòðóêòóðû íà êîòîðóþ óêàçûâàåò Vol-óêàçàòåëü
virtual	int			GetParam(INT64 ID,void* &Vol,Decoder* &ParentMt);				// rc=-1 íåóäà÷à
virtual	int			GetParam(INT64 ID,void* &Vol,size_t &Cnt);		// rc=-1 íåóäà÷à Cnt=ðàçìåð ñòðóêòóðû íà êîòîðóþ óêàçûâàåò Vol-óêàçàòåëü
virtual	int			GetParam(INT64 ID,void* &Vol);				// rc=-1 íåóäà÷à
virtual	int			DelParam(INT64 ID);							// rc=-1 íåóäà÷à
virtual	void		DelAllElem();
virtual	int			FindFirst(INT64 &ID,void* &Vol,size_t &Cnt,Decoder* &ParentMt);	// rc=-1 íåóäà÷à
virtual	int			FindNext(INT64 &ID,void* &Vol,size_t &Cnt,Decoder* &ParentMt);	// rc=-1 íåóäà÷à
virtual	int			FindFirst(INT64 &ID,void* &Vol,size_t &Cnt);	// rc=-1 íåóäà÷à
virtual	int			FindNext(INT64 &ID,void* &Vol,size_t &Cnt);	// rc=-1 íåóäà÷à
virtual	int			FindFirst(INT64 &ID,void* &Vol,size_t &Cnt,long &mLWrTime);	// rc=-1 íåóäà÷à
virtual	int			FindNext(INT64 &ID,void* &Vol,size_t &Cnt,long &mLWrTime);	// rc=-1 íåóäà÷à
//virtual	int			FindStart();									// rc=-1 íåóäà÷à
virtual	void		TestValid();
virtual	PLID_Elem*	Find(INT64 ID);	// Ïîèñê ýëåìåíòà è
					// óñòàíàâêà óêàçàòåëÿ ïîèñêà íà ïîçèöèþ ñîîòâåòñòâóþùóþ óñëîâèþ ïîèñêà
					// íåçàâèñèìî îò ðåçóëüòàòîâ ïîèñêà
};

//////////////////////////////////////////////////////////////////////////////////////////////////////////
#endif
*/
struct	DVol
{
	char*	tBf;
#ifndef _WIN64
	INT				Z0;
#endif
	size_t	tSize;
#ifndef _WIN64
	INT				Z1;
#endif
	FileTime	TimeModification;
};

struct	IVol
{
	INT64	VI64;
	INT64	Z0;
	FileTime	TimeModification;
};

union	VolPrm
{
	IVol	iV;
	DVol	dV;
};

#define NumDPrm	(FinishPrm-LastPrm+EndFirstPrm-FirstPrm)

#if	TypeOfParamListID == New	

class	ParamListID
{
public:
private:
	VolPrm	tV[NumDPrm];
	bool	Fl[NumDPrm];	// ?????
	int		FindPoi;
public:
	int					FlNew;
	long				LWrTime;
private:
	FlagStruct			Fl_RE;
	Services			*Srv;
public:
	int					NumElem;
	char				pName[256];	//!!!!!!!!!!!!!!!!!!!!!!!
public:
	// Ïðîñòî ñ÷åò÷èê èçìåíåíèé !!!!!!!!!!!!!!!!!!!!!!!!!!!!!
	ParamListID(char*	Name);
	virtual				~ParamListID();
	virtual	void*		AllocParam(INT64 ID,int Cnt,Decoder* ParentMt);	// ParentMt=this
	virtual	int			SetParam(INT64 ID,void* Vol,size_t Cnt,Decoder* ParentMt,FileTime* FTime=NULL);		// ParentMt=this	rc=-1 íåóäà÷à
	virtual	int			SetParam(INT64 ID,char* Vol,Decoder* ParentMt,FileTime* FTime=NULL) {return SetParam(ID,(void*)Vol,strlen(Vol)+1,ParentMt,FTime);};
	virtual	int			GetParam(INT64 ID,void* &Vol,size_t &Cnt,Decoder* &ParentMt);		// rc=-1 íåóäà÷à Cnt=ðàçìåð ñòðóêòóðû íà êîòîðóþ óêàçûâàåò Vol-óêàçàòåëü
	virtual	int			GetParam(INT64 ID,void* &Vol,Decoder* &ParentMt);				// rc=-1 íåóäà÷à
	virtual	int			GetParam(INT64 ID,void* &Vol,size_t &Cnt);		// rc=-1 íåóäà÷à Cnt=ðàçìåð ñòðóêòóðû íà êîòîðóþ óêàçûâàåò Vol-óêàçàòåëü
	virtual	int			GetParam(INT64 ID,void* &Vol);				// rc=-1 íåóäà÷à
// ---------------------
	virtual	int			GetParam(INT64 ID,VolPrm* &PrmVol,void* &Vol,size_t &Cnt,Decoder* &ParentMt);		// rc=-1 íåóäà÷à Cnt=ðàçìåð ñòðóêòóðû íà êîòîðóþ óêàçûâàåò Vol-óêàçàòåëü
	virtual	int			GetParam(INT64 ID,VolPrm* &PrmVol,void* &Vol,Decoder* &ParentMt);				// rc=-1 íåóäà÷à
	virtual	int			GetParam(INT64 ID,VolPrm* &PrmVol,void* &Vol,size_t &Cnt);		// rc=-1 íåóäà÷à Cnt=ðàçìåð ñòðóêòóðû íà êîòîðóþ óêàçûâàåò Vol-óêàçàòåëü
	virtual	int			GetParam(INT64 ID,VolPrm* &PrmVol,void* &Vol);				// rc=-1 íåóäà÷à
// ---------------------
	virtual	int			DelParam(INT64 ID);							// rc=-1 íåóäà÷à
	virtual	void		DelAllElem();
	virtual	int			FindFirst(INT64 &ID,void* &Vol,size_t &Cnt,Decoder* &ParentMt);	// rc=-1 íåóäà÷à
	virtual	int			FindNext(INT64 &ID,void* &Vol,size_t &Cnt,Decoder* &ParentMt);	// rc=-1 íåóäà÷à
	virtual	int			FindFirst(INT64 &ID,void* &Vol,size_t &Cnt);	// rc=-1 íåóäà÷à
	virtual	int			FindNext(INT64 &ID,void* &Vol,size_t &Cnt);	// rc=-1 íåóäà÷à
// ---------------------
	virtual	int			FindFirst(INT64 &ID,VolPrm* &PrmVol,void* &Vol,size_t &Cnt,Decoder* &ParentMt);	// rc=-1 íåóäà÷à
	virtual	int			FindNext(INT64 &ID,VolPrm* &PrmVol,void* &Vol,size_t &Cnt,Decoder* &ParentMt);	// rc=-1 íåóäà÷à
	virtual	int			FindFirst(INT64 &ID,VolPrm* &PrmVol,void* &Vol,size_t &Cnt);	// rc=-1 íåóäà÷à
	virtual	int			FindNext(INT64 &ID,VolPrm* &PrmVol,void* &Vol,size_t &Cnt);	// rc=-1 íåóäà÷à
//	virtual	int			FindNext(INT64 &ID,void* &Vol,size_t &Cnt,long &mLWrTime);	// rc=-1 íåóäà÷à
	//	virtual	int			FindFirst(INT64 &ID,void* &Vol,size_t &Cnt,long &mLWrTime);	// rc=-1 íåóäà÷à
	//virtual	int			FindStart();									// rc=-1 íåóäà÷à
	virtual	void		TestValid();
	virtual	VolPrm*		Find(INT64 ID);	// Ïîèñê ýëåìåíòà è
	// óñòàíàâêà óêàçàòåëÿ ïîèñêà íà ïîçèöèþ ñîîòâåòñòâóþùóþ óñëîâèþ ïîèñêà
	// íåçàâèñèìî îò ðåçóëüòàòîâ ïîèñêà
};

#endif
//////////////////////////////////////////////////////////////////////////////////////////////////////////

// Command
#define	Command_SAVE		0	//Äîïèñàòü äàííûå â ôàéë (ñîçäàòü íîâûé ôàéë)
#define	Command_MODIFY		1	//Ïåðåïèñàòü äàííûå â ôàéë (ñòàðûé ôàéë)
//#define	Command_ADD_CHILD	2	//Ïåðåïèñàòü äàííûå â ôàéë (ñòàðûé ôàéë)
//	#define	Command_CLOSE		0	// Çàìåíà íà  Save  Çàêðûòü ôàéë (êîíåö ôàéëà)		// !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
//	#define	Command_CLOSE_Err	2	//Çàêðûòü íåçàêðûòûé (íåïîëíûé, ïî òàéìàóòó , áåç ïðèçíàêîâ êîíöà, ...) ôàéë
//	#define	Command_CLOSE_Razr	3	//Çàêðûòü ôàéë ñ ïîòåðÿííûìè ïàêåòàìè (èëè íåâàëèäíûìè äàííûìè)
//	#define	Command_CLOSE_Begin	4	//Çàêðûòü ôàéë, òàê êàê áûëî îáíàðóæåíî íà÷àëî ñëåäóþùåãî
#define	Command_REPAIR		8	//Ïåðåìåùåíèå IDInfo ê äðóãîìó ðîäèòåëþ
#define	Command_MOVE		9	//Ïåðåìåùåíèå IDInfo ê äðóãîìó ðîäèòåëþ
#define	Command_DEL			10  //Óíè÷òîæèòü ðàííåå ñîõðàíÿåìûé ôàéë
#define	Command_REPLICATION	11	//Ïåðåïèñàòü äàííûå â ôàéë (ñòàðûé ôàéë)
// Äëÿ TypeIDI_COMMAND
#define	Command_C_GetIDI		100 // Ïðî÷èòàòü ïî èäåíòèôèêàòîðó int64
#define	Command_C_GetQBE		101 // Çàïðîñ â ôîðìàòå QBE (ñòðóêòóðà ïåðåäàåòñÿ â áóôåðå ñ äàííûìè)
#define	Command_C_GetFPrn		102 // Ïðî÷èòàòü ïî èäåíòèôèêàòîðó File Parent
#define	Command_SetStrCfgPoisk	103 // Êîìàíäà äëÿ ïåðåäà÷è êîíôèãóðàöèîííîé ñòðî÷êè äëÿ ïîèñêà
#define	Command_C_SendUnitInfo	104	// Ïîñëàòü îáíîâëåíèå UnitInfo
#define	Command_C_DelUnitInfo	105	// Óäàëèòü UnitInfo
#define	Command_C_GetUnitInfo	106 // 
#define	Command_C_GetIDIFromSrv	107
#define	Command_C_SendLCond		108	// Ïîñëàòü ëèöåíçèîííûõ óñëîâèé
#define	Command_C_GetLCond		109	// Çàïðîñ ëèöåíçèîííûõ óñëîâèé
#define	Command_C_SendLUinfo	110	// Ïîñëàòü ëèöåíçèîííûõ óñëîâèé

#define	Command_C_SendDllTime		111	// Ïîñëàòü âðåìÿ ñîçäàíèÿ Dll ôàéëîâ â ñòðóêòóðå Module
#define	Command_C_GetDllFile		112	// Çàïðîñèòü Dll ôàéë
#define	Command_C_SendDllFile		113	// Ïîñëàòü âðåìÿ ñîçäàíèÿ Dll ôàéëîâ â ñòðóêòóðå Module
#define	Command_C_UpDllData			114
#define	Command_C_SendMainFile		115	// Ïîñëàòü âðåìÿ ñîçäàíèÿ Dll ôàéëîâ â ñòðóêòóðå Module
#define	Command_C_GetMainFile		116	// Çàïðîñèòü Dll ôàéë
#define	Command_C_SendMainTime		117	// Ïîñëàòü âðåìÿ ñîçäàíèÿ Dll ôàéëîâ â ñòðóêòóðå Module
#define	Command_C_UpMainData		118

#define	Command_C_WANCli			200	// Èíôîðìàöèÿ î WANCli
#define	Command_C_UserLogIn			250

#define	Command_C_AddTract			300
#define	Command_C_DelTract			301
#define	Command_C_StopTr			302
#define	Command_C_StartTr			303
#define	Command_C_ReStartTr			304
#define	Command_C_DelHostFromDomain	305
#define	Command_C_ShutDownHost		306
#define	Command_C_Restart_Host		307
#define	Command_C_Restart_Neuron	308
#define	Command_C_Close_Neuron		309
#define	Command_C_OpenSmStarter		310
#define	Command_C_CloseSmStarter	311
#define	Command_C_GetMainCfg		312
#define	Command_C_MainCfg			313
#define	Command_C_SendMainCfg		314

#define	Command_C_GetStatisticQBE	401 
#define	Command_C_GetStatistic		402 

#define	Command_C_Test				1000 // Test  
#define	Command_C_Error				1001 // Test  

#define	Command_C_Switch_On_Spe		1002 // Test  
#define	Command_C_Switch_Off_Spe	1003 // Test  
#define	Command_C_Switch_On_Data	1004 // Test  
#define	Command_C_Switch_Off_Data	1005 // Test  
#define	Command_C_Switch_On_Star	1006 // Test  
#define	Command_C_Switch_Off_Star	1007 // Test  
#define	Command_C_Switch_On_Wav		1008 // Test  
#define	Command_C_Switch_Off_Wav	1009 // Test  
#define	Command_C_Spe				1010 // Test  
#define	Command_C_Data				1011 // Test  
#define	Command_C_Star				1012 // Test  
#define	Command_C_Wav				1013 // Test  
#define	Command_Analyze		        1014 // Command for ProtocolProc 

#define	Command_C_DelBD		        1015 // Command for delete BD 
#define	Command_C_Switch_On_ManCh	1016 // Test  
#define	Command_C_Switch_Off_ManCh	1017 // Test  

#define	Command_DeviceMan_On		1018
#define	Command_DeviceMan_Off		1019

#define	Command_Interception_On		1020// Command for set Interception_On for freq (for Answer - Ok)
#define	Command_Interception_Off	1021// Command for set Interception_Off for freq (for Answer - Ok)
#define	Command_Interception_Err	1022// Command for set Interception_Off when Answer=error )

#define	Command_Server_WD			1023
#define	Command_Server_Set			1024

#define	Command_Scanner_On			1025// Command for set Interception_On for freq (for Answer - Ok)
#define	Command_Scanner_Off			1026// Command for set Interception_On for freq (for Answer - Ok)

#define	Command_SetZeroFreq	        1027 // Command for SetZeroFreq
#define	Command_SetFunctType        1028 // Command for SetZeroFreq

#define	Command_Tuner_On			1029// Command for set Tuner_On for freq (for Answer - Ok)
#define	Command_Tuner_Off			1030// Command for set Tuner_On for freq (for Answer - Ok)
#define	Command_Tuner_Err			1031// Command for set Interception_Off when Answer=error )

#define	Command_C_Save_On			1032 // Test  
#define	Command_C_Save_Off			1033 // Test  

#define	Command_C_Start				1034 // 
#define	Command_C_Stop				1035 //  

#define	Command_Statistics			1036 //

#define	Command_C_Switch_On_Loupe	1037 // Test  
#define	Command_C_Switch_Off_Loupe	1038 // Test  
#define	Command_C_Loupe				1039 // Test  

#define	Command_FindModulation      1040 
#define	Command_C_NextFreq			1041	// Adonis com
#define	Command_ConfigChannel		1042	// Adonis com

#define	Command_C_SpeRF				1043 // Test
#define	Command_C_Ep				1044 // Test
#define	Command_C_SetUpFreq			1045	// Adonis com
#define	Command_C_ReleaseFreq		1046	// Adonis com
#define	Command_C_HoldFreq			1047	// Adonis com


#define	Command_C_Error_End			1048	// Test  
#define	Command_C_NextScan			1049	// new SSA com

#define	Command_C_Switch_On_DemData	1050 // Test  
#define	Command_C_Switch_Off_DemData	1051 // Test  

#define	Command_C_GetPolicyState	1052 // 
#define	Command_C_SetPolicyState	1053 // 
#define	Command_C_EndAnalyse		1054 // 

#define	Command_C_QueryStatistic	1055 // 
#define	Command_C_AnswerStatisticFromDem	1056 // 
#define	Command_C_AnswerStatistic	1058 // 
#define	Command_C_SetStatistic		1059

#define	Command_C_Blink_On			1060 //   
#define	Command_C_Blink_Off			1061 // 


#define	Command_C_SetExtTuner		1062 // OSPCH-M
#define	Command_C_StopExtTuner		1063 // OSPCH-M
#define	Command_C_StartWBR			1064 // OSPCH-M


#define	Command_C_StartSaveWav		1065 // 
#define	Command_C_Stop_SaveWav		1066 // 
// ...

// StateBlock
#define	StateBlock_BEGIN		1	// Ïðèçíàê ïåðâîãî ïàêåòà â ôàéëå
#define	StateBlock_END			2	// Ïðèçíàê êîíöà ôàéëà
#define	StateBlock_ERROR		4	// Ïðèçíàê òîãî, ÷òî â ôàéëå íàõîäÿòñÿ íåâåðíûå äàííûå
#define	StateBlock_EMPTY		8	// Ïðèçíàê òîãî, ÷òî â ôðàãìåíòå îòñóòñòâóþò äàííûå (îáû÷íî çàïîëíÿåòñÿ îïðåäåëåííîé êîìáèíàöèåé)
#define	StateBlock_UNIFORM		16	// Ïðèçíàê òîãî, ÷òî â ôðàãìåíòå ïåðåäàåòñÿ îäèíàêîâàÿ ïîñëåäîâàòåëüíîñòü äàííûõ 
#define	StateBlock_CRC16OK		32	// Ïðèçíàê òîãî, ÷òî â ïðîâåðêà CRC16 ïðîèçâåäåíà è äàííûå ïåðåäàþòñÿ áåç CRC16
#define	StateBlock_CRC32OK		64	// Ïðèçíàê òîãî, ÷òî â ïðîâåðêà CRC32 ïðîèçâåäåíà è äàííûå ïåðåäàþòñÿ áåç CRC32
#define	StateBlock_MODIF_PARAM	128	// Ïðèçíàê ìîäèôèêàöèè âíóòðåííèõ ïàðàìåòðîâ

#define	StateBlock_MainParent	256	// Ïåðâûé IDInfo (êîðíåâîé ýë-ò äëÿ âñåõ íèæåñòîÿùèõ IDInfo) //  [4/12/2011 San]
// ...

// TypeIDI	??????????????????????????????????????????????????????????
#define	TypeIDI_FILE			0		// Ïðèçíàê ôàéëà
#define	TypeIDI_SEANS			0x8000	// Ïðèçíàê ñåàíñà 
#define	TypeIDI_LOG				0x4000	// Ïðèçíàê Log äàííûõ 
#define	TypeIDI_COMMAND			0x2000	// Ïðèçíàê êîìàíäû
#define	TypeIDI_ANSWER			0x1000	// Ïðèçíàê îòâåòà íà çàïðîñ
#define	TypeIDI_TABLE			0x0800	// Ïðèçíàê òàáëèöû 
//	#define	StateFile_Command

// -------------------------------
#define			PrmType_Void	0
#define			PrmType_String	1
#define			PrmType_Int		2
#define			PrmType_Int64	3
#define			PrmType_Float	4
#define			PrmType_Double	5
#define			PrmType_Char	7
#define			PrmType_Short	8
#define			PrmType_Complex	9
#define			PrmType_ID		10	// index

#define			PrmType_XML		20
// -------------------------------
class	El_Telem :	public ID_Elem
{
public:
TlmData			TD;
double			mVol;
double			sVol;
double			dVol;
double			minVol;
double			maxVol;
FileTime		LWTime;
Services*		Srv;
INT64			tTCnt;
				El_Telem(TlmData *NPrm): ID_Elem((UCHAR*)&TD,sizeof(PVOID)+sizeof(Decoder*)+strlen(NPrm->Prm)+1)
				{
					Srv=GetServices();tTCnt=0;
					memcpy(&TD,NPrm,sizeof(TlmData));
					sVol=0;dVol=0;minVol=1e+100;maxVol=-1e+100;
					GetSystemTimeAsFileTime(&LWTime.FTime);
				};
virtual	void	Init(TlmData *NPrm);
virtual	void	SetVol(double Vol);
};


// Ñòðóêòóðà äëÿ ïîëó÷åíèÿ èíôîðìàöèè îò äåêîäåðà
// XXX_ = íàèìåíîâàíèå îáúåêòà_
static char	XXX_NameMt[]= "NameDecodMt";
static char	XXX_Version[]= "V1.0";
static char	XXX_Author[]= "SystemSoft (Y)";
static char	XXX_RemMt[]= "Remark";
static char	XXX_Type[]= "Decoder";
static char	*XXX_PinNameI[]=
{
//	"NamePinI1","Rem",
	""
};
static char	*XXX_PinNameO[]=
{
//	"NamePinO1","Rem",
	""
};

static char	*XXX_Prm[]=
{
//	"NamePrm","Rem",
	""
};

static char	*XXX_Vol[]=
{
//	"NamePrm","Vol1","Rem",
//	"NamePrm","Vol2","Rem",
//	"NamePrm2","Vol1","Rem",
//	"NamePrm2","Vol2","Rem",
	""
};
// ----------------------------------------------
// Ñëóæåáíûå ïàêåòû
struct ServicePack
{
	int	Type;
	int	Prm1;
	int	Prm2;
	int	Prm3;
};
// ----------------------------------------------

struct	InfoMt
{
char	*Name;
char	*Rem;
char	*Ver;
char	*Auth;
int		NPinI;
int		NPinO;
int		NParam;
char	*Type;
char	SourceType[8];
};

struct	InfoPin
{
char	*Name;
char	*Rem;
};

struct	InfoPrm
{
char	*Name;
char	*RemPrm;
char	*Vol[50];
char	*Rem[50];
};


	struct	DecLink
{
	Decoder*	pObj;
	int			PinO;
	int			PinI;
	char		MtName[64];
};


static	char	GlucStr[4]=
{
	'|',
	'\\',
	'-',
	'/'
};

class Glucator
{
public:
	Glucator()
	{
		Kdiv=10;Cnt=Cnt2=0;
	};
	void	Step()
	{
		if (Cnt++>Kdiv)
		{
			Cnt2++;Cnt=0;
		}
	};
	char	GetChar()
	{
		return GlucStr[Cnt2&3];
	}
protected:
	int	Cnt;
	int	Cnt2;
	int	Kdiv;
private:
};

class	DamperIDInfo;

enum NetObjectType
{
	ObjectClient=1,
	ObjectServer
};

class	NetObject : public ID_Elem
{
public:
	int TypeObj;
	char Descriptor[256];
	LPVOID handle;

	Services	*pSrv;

	int Param;				// äëÿ êëèåíòà - êîííåêò, äëÿ ñåðâåðà - êîëè÷. ïîäêëþ÷åíèé
	int PackCtrl;			// êîëè÷åñòâî óïðàâëÿþùèõ ïàêåòîâ
	int PackIn,PackOut;		// êîëè÷åñòâî ïàêåòîâ ïðèåì/ïåðåäà÷à (çà åäèíèöó âðåìåíè ò.å. ñî ñáðîñîì ïîñëå ÷òåíèÿ)
	int CountIn,CountOut;	// ñ÷åò÷èê ïåðåñûëàåìûõ äàííûõ ïðèåì/ïåðåäà÷à (çà åäèíèöó âðåìåíè ò.å. ñî ñáðîñîì ïîñëå ÷òåíèÿ)
	int pObjPtr, SockHandle;
	double	sRateDataIn,sRateDataOut,sRatePackIn,sRatePackOut;
	INT64 Click0,Click1,Click2;
	INT64 Click3,Click4,Click5;

	double	sLoadInT,sLoadOutT;
	double	sLoadInt,sLoadOutt;

	int FlagSelectProc;

	NetObject(int tObj, char *value,Services* ptr) :
	ID_Elem((PUCHAR)Descriptor,	strlen(value))
	{
		TypeObj=tObj;
		strcpy(Descriptor,value);
		handle=this;

		pSrv=ptr;
		if (pSrv==NULL)
			pSrv=GetServices();
		Param=0;
		PackCtrl=0;
		PackIn=0; PackOut=0;
		CountIn=0; CountOut=0;
		pObjPtr=0; SockHandle=0;

		sRateDataIn=sRateDataOut=sRatePackIn=sRatePackOut=0.;
		Click0=Click1=Click2=GetNClk(); 
		Click3=Click4=Click5=GetNClk();
		sLoadInT=sLoadOutT=sLoadInt=sLoadOutt=0.;

		FlagSelectProc=0;

	};	
	virtual	~NetObject(){};
	virtual int SetParam(char*,char*)	{ return 0; };
	virtual int Start()					{ return 0; };
	virtual int Stop()					{ return 0; };
	virtual int ObjIdle()				{ return 0; };
	virtual int CalcClick(int q)
	{ 
		switch(q)
		{
		case 0:
			Click0=Click2;
			Click1=GetNClk();
			break;
		case 1:
			Click2=GetNClk();
			sLoadOutT=0.999*sLoadOutT+0.001*(double)(Click2-Click0);
			sLoadOutt=0.999*sLoadOutt+0.001*(double)(Click2-Click1);
			break;
		case 2:
			Click3=Click5;
			Click4=GetNClk();
			break;
		case 3:
			Click5=GetNClk();
			sLoadInT=0.999*sLoadInT+0.001*(double)(Click5-Click3);
			sLoadInt=0.999*sLoadInt+0.001*(double)(Click5-Click4);
			//		sLoadIn=0;
			break;
		}
		return 0; 
	};
};
typedef NetObject *pNetObject;

 /* Ïðàðîäèòåëü */
class Decoder
{
public:
char	*Name_Mt;
char	*Rem_Mt;
char	*Ver_;
char	*Auth_;
char	**PinName_I;
char	**PinName_O;
char	**Prm_;
char	**Vol_;
char	*Type_Mt;

int	Num_PinI;		// Êîëè÷åñòâî âõîäíûõ íîæåê
int	Num_PinO;		// Êîëè÷åñòâî âûõîäíûõ íîæåê
int	Num_Prm;		// Êîëè÷åñòâî ïàðàìåòðîâ

int	Ext_Mt;			// Ðàñøèðåíèå èìåíè ìåòîäà äëÿ òî÷íîé àäðåñàöèè ïðè óñòàíîâêå ïàðàìåòðîâ
					// Èñïîëüçóåòñÿ êîíñòðóêòîðîì äåêîäåðîâ
USHORT	ID_Meth;	// Èäåíòèôèêàòîð ìåòîäà
UCHAR	ID_Host;    // Èäåíòèôèêàòîð õîñò-ìûøèíû
char	Rez;

//int			NStr;

Services	*Srv;
Decoder		*ParentMt;

//protected:

// Ïåðåìåííûå äëÿ âòîðîé ðåäàêöèè äåêîäåðîâ
DecLink			TabDecLink[512];
int				NumLink;
size_t			tCnt;
int				tNStrim;
UCHAR*			tBuf;
size_t			SizeAllocMem;
UnitInfo		UInfo;
INT64			NC1,NC2;
INT64			NCI,NCD,NCO;
StrOwner		Anchor;
FlagStruct		FlSim;
int				Fl_BufData;
int				Fl_ReCurse;
//int				Fl_ReCurse2;
DamperPS		*DmData;
DamperPS		*DmParam;
DamperIDInfo	*DmIDInfoUCB;	
DamperIDInfo	*DmIDInfoInput;	
ID_List8		ChildList;
FlagStruct		FlSimBuf;
public:
//-------------------------
	Decoder();
	virtual ~Decoder();
	// --------------------------------------------------------
	virtual	void ParamEx(		// Èçìåíèòü ïàðàìåòð
		char* Prm,	// Èäåíòèôèêàòîð ïàðàìåòðà
		char* Vol);	// Çíà÷åíèå ïàðàìåòðà
	virtual	void StartEx();		// Íà÷àëî ñåàíñà
	virtual	void StopEx();		// Êîíåö ñåàíñà
	virtual	int DataEx(		// Äàííûå äëÿ äåêîäèðîâàíèÿ
		int N_Pin,	// Íîìåð âõîäíîé íîæêè
		UCHAR* pmas,	// Óêàçàòåëü íà áóôåð ñ äàííûìè
		size_t Cnt);	// Äëèíà äàííûõ
	virtual	int OutDataEx(		// Âûõîäíûå äàííûå
		int N_Pin,	// Íîìåð âûõîäíîé íîæêè
		UCHAR* &pmas,	// Óêàçàòåëü íà áóôåð ñ äàííûìè
		size_t &Cnt);	// Óêàçàòåëü íà äëèíó äàííûõ
	virtual	int DataFFEx(			// Äàííûå äëÿ äåêîäèðîâàíèÿ
		int N_Pin,		// Íîìåð âõîäíîé íîæêè
		UCHAR* pmas,	// Óêàçàòåëü íà áóôåð ñ äàííûìè
		size_t Cnt);	// Äëèíà äàííûõ
	virtual	int ODataFFEx(				// Âûõîäíûå äàííûå
		int N_Pin,			// Íîìåð âûõîäíîé íîæêè
		UCHAR* pmas,		// Óêàçàòåëü íà áóôåð ñ äàííûìè
		size_t Cnt);			// Óêàçàòåëü íà äëèíó äàííûõ
	virtual	int ODataIDIEx(				// Âûõîäíûå äàííûå
		int N_Pin,			// Íîìåð âûõîäíîé íîæêè
		IDInfo* IDI,int FlagBuf=NULL);// 0- TryLock 1-Buf 2 NoBuf
	virtual	int DataIDIEx(			// Äàííûå äëÿ äåêîäèðîâàíèÿ
		int N_Pin,		// Íîìåð âõîäíîé íîæêè
		IDInfo* pIDI,
		int FlagBuf=NULL);		// Äëèíà äàííûõ
	virtual	int UserCallBackEx(	// Ôóíêöèÿ îáðàòíîãî âûçîâà
		int	Handle,		// Èäåíòèôèêàòîð ñîáûòèÿ
		int N_Pin,		// Íîìåð âõîäíîé íîæêè (ðàñøèðåíèå äëÿ îïðåäåëåíèÿ íåñêîëüêèõ ôóíêöèé)
		UCHAR* pmas,	// Óêàçàòåëü íà áóôåð ñ äàííûìè
		size_t Cnt,
		int FlagBuf=NULL);		// Äëèíà äàííûõ
	virtual	int UserCallBackIDIEx(	// Ôóíêöèÿ îáðàòíîãî âûçîâà
		int	Handle,		// Èäåíòèôèêàòîð ñîáûòèÿ
		IDInfo* IDI,
		StrOwner*	Own,
		int FlagBuf=NULL);		// Äëèíà äàííûõ
	// --------------------------------------------------------
protected:
		virtual	void Param(		// Èçìåíèòü ïàðàìåòð
			char* Prm,	// Èäåíòèôèêàòîð ïàðàìåòðà
			char* Vol);	// Çíà÷åíèå ïàðàìåòðà
		virtual	void Start();		// Íà÷àëî ñåàíñà
		virtual	void Stop();		// Êîíåö ñåàíñà
		virtual	int Data(		// Äàííûå äëÿ äåêîäèðîâàíèÿ
			int N_Pin,	// Íîìåð âõîäíîé íîæêè
			UCHAR* pmas,	// Óêàçàòåëü íà áóôåð ñ äàííûìè
			size_t Cnt);	// Äëèíà äàííûõ
		virtual	int OutData(		// Âûõîäíûå äàííûå
			int N_Pin,	// Íîìåð âûõîäíîé íîæêè
			UCHAR* &pmas,	// Óêàçàòåëü íà áóôåð ñ äàííûìè
			size_t &Cnt);	// Óêàçàòåëü íà äëèíó äàííûõ
		virtual	int DataFF(			// Äàííûå äëÿ äåêîäèðîâàíèÿ
			int N_Pin,		// Íîìåð âõîäíîé íîæêè
			UCHAR* pmas,	// Óêàçàòåëü íà áóôåð ñ äàííûìè
			size_t Cnt);		// Äëèíà äàííûõ
		virtual	int DataIDI(			// Äàííûå äëÿ äåêîäèðîâàíèÿ
			int N_Pin,		// Íîìåð âõîäíîé íîæêè
			IDInfo* pIDI);		// Äëèíà äàííûõ
		virtual	int UserCallBack(	// Ôóíêöèÿ îáðàòíîãî âûçîâà
			int	Handle,		// Èäåíòèôèêàòîð ñîáûòèÿ
			int N_Pin,		// Íîìåð âõîäíîé íîæêè (ðàñøèðåíèå äëÿ îïðåäåëåíèÿ íåñêîëüêèõ ôóíêöèé)
			UCHAR* pmas,	// Óêàçàòåëü íà áóôåð ñ äàííûìè
			size_t Cnt);		// Äëèíà äàííûõ
		virtual	int UserCallBackIDI(	// Ôóíêöèÿ îáðàòíîãî âûçîâà
			int	Handle,		// Èäåíòèôèêàòîð ñîáûòèÿ
			IDInfo* IDI,
			StrOwner*	Own);		// Äëèíà äàííûõ
		virtual	int ODataFF(				// Âûõîäíûå äàííûå
			int N_Pin,			// Íîìåð âûõîäíîé íîæêè
			UCHAR* pmas,		// Óêàçàòåëü íà áóôåð ñ äàííûìè
			size_t Cnt);			// Óêàçàòåëü íà äëèíó äàííûõ
		virtual	int ODataIDI(				// Âûõîäíûå äàííûå
			int N_Pin,			// Íîìåð âûõîäíîé íîæêè
			IDInfo* IDI,int NoBuf=0);	// 0- TryLock 1-Buf 2 NoBuf
		virtual	int		IntCalc();
		virtual	int		IntCalcPrm();
		virtual	int		IntCalcDataIDI();
		virtual	int		IntCalcUCB();
		virtual	int		IntCalcUCBIDI();
		virtual	int		BufUserCallBackEx(int	Handle,int N_Pin,UCHAR* pmas,size_t Cnt);
		virtual	int		BufUserCallBackIDIEx(int	Handle, IDInfo* IDI,	StrOwner	*Own);
		virtual	int		BufDataIDIEx(int N_Pin,IDInfo* pIDI);
public:
	// ---- Íîâûé ñòèëü íàïðèñàíèÿ ìåòîäîâ äåêîäèðîâàíèÿ --------------
/*	virtual	int BufDataFF(			// Äàííûå äëÿ äåêîäèðîâàíèÿ
					int N_Pin,		// Íîìåð âõîäíîé íîæêè
					UCHAR* pmas,	// Óêàçàòåëü íà áóôåð ñ äàííûìè
					size_t Cnt);		// Äëèíà äàííûõ
	virtual	int BufODataFF(				// Âûõîäíûå äàííûå
					int N_Pin,			// Íîìåð âûõîäíîé íîæêè
					UCHAR* pmas,		// Óêàçàòåëü íà áóôåð ñ äàííûìè
					size_t Cnt);			// Óêàçàòåëü íà äëèíó äàííûõ
	virtual	int BufDataFF(			// Äàííûå äëÿ äåêîäèðîâàíèÿ
					int N_Pin,		// Íîìåð âõîäíîé íîæêè
					IDInfo* IDI);	// Óêàçàòåëü íà áóôåð ñ äàííûìè
	virtual	int BufODataFF(				// Âûõîäíûå äàííûå
					int N_Pin,			// Íîìåð âûõîäíîé íîæêè
					IDInfo* IDI);		// Óêàçàòåëü íà áóôåð ñ äàííûìè
*/
//-----------------------------------------------------------------------
//	virtual	int DataIdle(void);			// Buffered data
	virtual	int IdleFunc(int NumberTr);	// Idle function  if rc!=0 recall function (?) 
	// --------------------------------------------------------
	virtual char* GetInfoStr(char* Str);	  // Óêàçàòåëü íà ñòîêó ñ èíôîðìàöèåé
	virtual void Initialization();		// Íà÷àëî ñåàíñà
// ---- Èíèöèàëèçàöèÿ äàííûõ î ìåòîäå äåêîäèðîâàíèÿ ----
	int		GetInfoMethod(InfoMt* InfM);
	int		GetInfoPinIn(int Num,InfoPin* InfP);
	int		GetInfoPinOut(int Num,InfoPin* InfP);
	int		GetInfoParam(int Num,InfoPrm* InfP);
	void*	malloc(size_t size);
	void*	realloc(void*tBuf,size_t size);
	void	free(void* tB);
	int		AddDecLink(Decoder* pDec,int PinO,int PinI);
	int		DelDecLink(Decoder* pDec);
	int		DelAllDecLink();
	int		AddChildMt(Decoder* pDec);
	int		DelChildMt(Decoder* pDec);
	void	InitDataCB(int (CALLBACK *pODataFFF)(int N_Pin,unsigned char *pData,size_t Cnt,Decoder* pDec,Decoder* pOwnDec),Decoder* pDec);
protected:
	virtual	int ChildIdleFunc(int NumberTr);
	Decoder*	pDecODataFFF;
	int (CALLBACK *pODataFFF)(int N_Pin,unsigned char *pData,size_t Cnt,Decoder* pDec,Decoder* pOwnDec);
};

class ClasterMem
{
public:
	int			Wr,Rd,Fl_Wr,Fl_Rd;
	UCHAR		*Buf;
	int			Size;
	ClasterMem*	PreviousCM;
	ClasterMem*	NextCM;
	Decoder*	Dec;
	Services	*Srv;
				ClasterMem(int SizeCM,Decoder* pDec);
				~ClasterMem();
virtual void	ReSize(int NewSize);
protected:
private:
};

int		DamperProc(void* Obj,UCHAR* tBuf,size_t Cnt);

class	Damper
{
protected:
	int			(*DmpProc)(void* Obj,UCHAR* tBuf,size_t Cnt);
	void*		pObj;
	int			BufMaxSize;
	int			RealSize;
	Decoder		*PDec;
	ClasterMem	*CM_Wr;
	ClasterMem	*CM_Rd;
	INT64		CrMSG;
	int			SizeCMax;
	int			SizeCMdl;
//	int			Fl_LockRd,Fl_LockWr;
	FileTime	TimeMemAlloc;
//	HANDLE			hmtx;
	char			SemName[80];
	int			Fl_Lock;
	int			SumSize;
	float		OptSize;
	int			tWr;
	FlagStruct	Fl_MT/*i,Fl_MTo*/;
// -------------------------------------------------------
public:
	int			NumCM,NumCMM;
	INT64		CntInside;
	Services	*Srv;
	int			Fl_Clean;

				Damper();
				Damper(int MaxSize);
virtual			~Damper();
virtual	void	Clear(); // Âñå ïî÷èñòèòü
virtual	void	SetStartSize(int Size); // Óñòàíîâèòü íà÷àëüíûé ðàçìåð áóôåðà
virtual	void	SetMaxSize(int MaxSize) {BufMaxSize=MaxSize;}; // Óñòàíîâèòü ìàêñèìàëüíûé ðàçìåð áóôåðà
virtual	void	SetPrm(Decoder* PD,void* Obj=NULL,int (*DmpPrc)(void*,UCHAR*,size_t)=&DamperProc);
virtual	int		Data(UCHAR* Buf,size_t Cnt);	// Çàïèñàòü äàííûå â áóôåð
virtual	int		OData(); // âûçîâ îñóùåñòâëÿåòñÿ èç OData()	CallBack function
virtual	int		LockOData(UCHAR* &Buf,size_t &Cnt); // Ïîäãîòîâèòü äàííûå äëÿ îáðàáîòêè (çàëî÷èâ ïðè ýòîì áóôåð)
virtual	void	UnLockOData(); // Îñâîáîäèòü äàííûå äëÿ ïåðåçàïèñè
protected:
virtual void	ReSize(int Act);	// Îïòèìèçàöèÿ
};
// -------------------
class	IDTable;
class	El_Tab;
class	IDInfoConverter;

struct	FindStruct
{
	INT64			ID;				// òåêóùèé ýëåìåíò
	INT64			IDPrm;			// èìÿ ïàðàìåòðà ïî êîòîðîìó âûïîëíÿåòñÿ ñîðòèðîâàêà (åñëè NamePrm[0]=0 òî ïîèñê èäåò ïî âñåì ýëåìåíòàì)
	char			VolPrm[8*1024];		// çíà÷åíèå ïàðàìåòðà ïî êîòîðîìó âûïîëíÿåòñÿ ñîðòèðîâàêà (åñëè VolPrm[0]=0 òî ïîèñê èäåò äëÿ âñåõ çíà÷åíèé ïàðàìåòðîâ)
	int				Fl_Prm;			// =0 Проверять значение параметра =1 Параметр существует (значение не имеет смысла) =2 параметр отсутствует вообще 
	int				Fl_revers;
	ID_List8*		IDL;			// ñïèñîê ñ ñîðòèðîâêîé (âñïîìîãàòåëüíûé)
	FindStrList8	FSL8;//	ID_ElemM*		IDEl;
	FindStruct()
	{
		IDL=new ID_List8();
		ID=NULL;
		IDPrm=0;
		Fl_Prm=0;
		VolPrm[0]=0;
		Fl_revers=0;
//		IDEl=NULL;
	}
	void ReInit()
	{
		ID=NULL;IDPrm=0;
		VolPrm[0]=0;
		Fl_revers=0;Fl_Prm=0;
		IDL->DelAllElem();
		FSL8.DestroyStr();
//		IDEl=NULL;
	};
	~FindStruct()
	{
		FSL8.DestroyStr();
		IDL->DelAllElem();
		delete IDL;
	}
	
};


class IDParam
{
	El_Tab*	Tab[65536];
	Services*	Srv;
	ID_List8*	IDL;
public:
	IDParam();
	~IDParam();
void		Init(Services* Sv) {Srv=Sv;};
El_Tab*		GetElTab(INT64 ID);
El_Tab*		AddET(INT64 ID);
El_Tab*		AddET(char* Vol,El_Tab* ElT=NULL);
El_Tab*		NewElTab(char* tVol, El_Tab* ElTParent);
int			AddParam(El_Tab *ElTab,int IDPrm,char* ElName);
int			AddParam(El_Tab *ElTab,int IDPrm,char El);
El_Tab*		Find(char* ElName,El_Tab *ElTab);
El_Tab*		FindFirstET(FindStruct *FStrtEl,El_Tab *ElTab);
El_Tab*		FindNextET(FindStruct *FStrtEl,El_Tab *ElTab);
int			GetParam(El_Tab* ETbl,INT64 IDPrm,char* &Vol);
El_Tab*		AttainNested(char *mFormat,char* FirstClass,...);
void		DelElem(El_Tab *tEl);
};

class	DamperPS : public Damper
{
protected:
	DmpHeader	HndlP;
	INT64		CntPackIn,CntPackOut;
	INT64		HndCE;
public:
				DamperPS();
				~DamperPS();
virtual	int		Data(UCHAR *Buf,size_t Cnt,int NumCli=-1);
virtual	int		OData();
virtual	int		LockOData(UCHAR* &Buf,size_t &Cnt);
virtual	void	UnLockOData(); // Îñâîáîäèòü äàííûå äëÿ ïåðåçàïèñè
virtual	void	Clear(); // Âñå ïî÷èñòèòü
};


class ListID_FiFo	:	public	ID_List8
{
public:
FlagStruct				Fl_RE;
//FlagStruct				Fl_REO;
int						CntInSide;
						ListID_FiFo();
						~ListID_FiFo();
	virtual	int			AddID(INT64	ID);	// -1 error (no added) 0 - ok
	virtual	INT64		GetID();
			void		DelAllEl();
protected:
	DamperPS	Dmp;
private:
};


class	DamperIDInfo : public DamperPS
{
public:
	Services	*Srv;

UCHAR*			TBuff;
int				SizeTBuff;
FlagStruct		Fl_RE;
int				CntInSidePackets;
				DamperIDInfo();
				~DamperIDInfo();
virtual	int		SendIDI(int	N_Pin,IDInfo*	IDI);
virtual	int		GetIDI(int	&N_Pin,IDInfo*	&IDI);
virtual	int		DestroyIDI(IDInfo*	IDI);
virtual	int		CopyPListID(ParamListID* pPListD,ParamListID* pPListS);
virtual	void	Clear();
};
/*
class IntTest
{
public:
	Services*	Srv;
	ID_List8	IList;
	int			NumberEl;
	int			Phase;
	IntTest();
	~IntTest();
	int		InternalTest();
	int		ReadTabl(El_Tab*	ElTParent);
	int		TestEl(El_Tab*	El);
private:
};
*/

#define MaxStackBufSize	1500
class	StackNumBuffer
{
public:
int		BufferDeep;
char*	BufferClaster;
int		NumFirstPack;
int		MaxNumPack;
		StackNumBuffer();
		~StackNumBuffer();
void	SetBufferDeep(int Deep);
int		Initialze(int	NumFrstPack);
int		AddPacket(int	Number, char* Buf, size_t Cnt);	//	-1 - îøèáêà 0 - ïàêåò ïðèíÿò, íî ïàêåòû íå âñå 1 -  ïàêåò ïðèíÿò è âñå ïàêåòû åñòü! 2 - ìåñòà áîëüøå íåò íåîáõîäèìî çàáèðàòü ïàêåòû
int		GetPacket(int	&Number, char* &Buf, size_t &Cnt);	//	-1 - îøèáêà 0 - âñå ïàêåòû âûäàíû 

protected:
private:
};

struct MemClaster 
{
	int		Fl;
	void*	Buf;
	int		Cnt;
};
#define SizeMC	256

struct CritErrStr
{
	char	ObjName[40];
	char	Msg[128];
	char	TypeMsg[40];
	char	MSystem[40];
	INT64	Handle;
};

// Ñòðóêòóðà äëÿ ïîëó÷åíèÿ èíôîðìàöèè îò äåêîäåðà
// Services_ = íàèìåíîâàíèå îáúåêòà_
static char	Services_NameMt[]= "Services";
static char	Services_Version[]= "V11.03";
static char	Services_Author[]= "SystemSoft (Y)";
static char	Services_RemMt[]= "Neuron_API";
static char	Services_Type[]= "Unit";
static char	*Services_PinNameI[]=
{
	"ThIDView","ThIDView",
	"IDI_FSrv","",
	"IDI_BD","Input from BD",
	"IDI_NCIn","Input to NetComm",
	"IDI_NCOut","Input from NetComm",
	""
};
static char	*Services_PinNameO[]=
{
//	"NamePinO1","Rem",
	""
};

static char	*Services_Prm[]=
{
//	"NamePrm","Rem",
	""
};

static char	*Services_Vol[]=
{
//	"NamePrm","Vol1","Rem",
//	"NamePrm","Vol2","Rem",
//	"NamePrm2","Vol1","Rem",
//	"NamePrm2","Vol2","Rem",
	""
};

struct ListIP
{
	char	IP[16][256][16];
	int CntIP[16];		// number of IP
	int MaxPage;
};

struct	Module
{
	HINSTANCE	ModuleHandle;	// Handle ìîäóëÿ
	char		Name[40];		// Èìÿ ìîäóëÿ
	int			Cnt;
	char*		(*FindFirst)();
	char*		(*FindNext)();
	LPVOID			(*FindNewDecod)(char*,Services*);
	void		(*DeleteDecod)(LPVOID);
	FileTime	CrTime;
	FileTime	AcTime;
	FileTime	WrTime;
};


struct	Str_0
{
int	F;
UCHAR	Fl;
int	ID_RS;
char	Name[40];
};

struct	Str_E
{
int	F;
int	ID_RS;
char	Name[40];
};

struct	Str_1
{
int	F;
char	Name[40];
};

struct	Str_2
{
char	Name1[40];
char	Name2[40];
};

struct	Str_3
{
int	PO;
char	Mt1[40];
int	PI;
char	Mt2[40];
int	ID_MD;
};

/*
#define Num_Mode	100
#define Num_Meth	1000
#define Num_LinkPin	10000
#define Num_Param	5000
#define Num_NamePin	5000
#define Num_Volume	5000
#define Num_RNet	30
#define Num_Address	1000
#define Num_Connect	10000
#define Num_Unit	30
#define Num_UDem	300
#define Num_Spectr	300
#define Num_Seans	Num_Connect/2
#define Num_Pool	500
#define Num_Freq	1000
#define Num_LinkMode	1000
#define Num_Energy	10
#define Num_GroopFr     3000
#define Num_GroopPl     1000
#define Num_BMeth	10000		// !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!1
#define Num_BParam	2000
#define Num_BIn		1000
#define Num_BOut	1000
*/
class DataImEx
{
public:

//Task	Tsk1,Tsk;
///TC_Se[1000]
Str_0   TC_Pl[Num_Pool];
Str_E   TC_Ad[Num_Address];
Str_1	TC_Un[Num_Unit];
Str_2	Un_Md[Num_Mode];
Str_3	Mt1_2[Num_LinkPin];
int	DublMt[1000];


char*	OutName[2];
char	Name1[2][40];
int     k0,i1,i2,i3,i4,DublM,DublM0,Temp;
char    ReadStr[255];
char    ReadStrTemp[255];
char	Temp_Ch[255];
char	BufEncStr[1024];


	DataImEx();
	~DataImEx();
void	ExportTsk(Task* Tsk,char *Name);
void	ImportTsk(Task* Tsk,char *Name);
void	ImportMode(Task* Tsk,char *Name);
void	ExportMode(Task* Tsk,int NomMd,char *Name);
//
void	ExpTsk(Task* Tsk,char *Name,char *NameDelMode);
void	ImpTsk(Task* Tsk,char *Name,char *NewMdPathName);
//
int	ReadCFG(FILE* FileCFG,char* Strng);
char*	EncStr(char* pStr);
char**	ReadName(char* InName);
};




void	CalcDec(int N_Pin,UCHAR* Buf,int Len,ULONG Poi,int ID);
void	CalcCli(int N_Pin,UCHAR* Buf,int Len,ULONG Poi);
/*
struct	PrmDec
{
char	Prm[256];
char	Vol[256];
};
*/

// ------------------
class	Sock_El	:	public	ID_Elem
{	
public:	
	UCHAR	Key[4];
	void	*Packet;
	size_t	LenPack;
	Sock_El(UCHAR* Buf,size_t Cnt)
	{
		Packet=NULL;
		Init (Buf,Cnt);
	}
	virtual	void	Init(UCHAR*	BuffIn,size_t	Cnt) // Èñïîëüçóåòñÿ ïðè ïîâòîðíîì èñïîëüçîâàíèè ýëåìåíòà
	{
		if (Packet==NULL)
			Packet=malloc(Cnt);
		else
		{
			free(Packet);
			Packet=malloc(Cnt);
		}
		memcpy(Packet,BuffIn,Cnt);
		LenPack=Cnt;
		//					Key[3]=*(BuffIn+8);
		//					Key[2]=*(BuffIn+9);
		//					Key[1]=*(BuffIn+10);
		//					Key[0]=*(BuffIn+11);
		Key[3]=*(BuffIn+11);
		Key[2]=*(BuffIn+10);
		Key[1]=*(BuffIn+9);
		Key[0]=*(BuffIn+8);
		Buff=Key;
		Len=sizeof(int);
	};
	~Sock_El()
	{
		if (Packet!=NULL)
			free(Packet);	
	}
};

class ClientSock
{
public:
	int		Flag;
	int		TimeOutWRB;
	int		SMode;
	int		TimeOut;
	SOCKET		Handle;
	int		Mode;
	int		ID_Cli;
	ULONG	IPadr;	// IP in long
	int		PortTCP;
	char	HostIP[32];		// IP
	char	HostName[32];	// Name
	struct	sockaddr_in client;	// client address information
	struct  sockaddr_in server;	// server address information
	//	char	Buf[CLIENT_SBUF]; // for function recv()
	//	CircBuf	*BufOut; // for SendData
	char	MyNetName[64];
	int	(*CallBackEvent)(ClientSock* S,int Event);
	int	(*readFunction)(ClientSock*,UCHAR*, int);
	//	CThreadSockCtrl	*SockCtrl;
	//	CThreadSockIdle	*SockIdle;
	fd_set	Srd,Swr,Sex;
	ClassThread	*ClThread;

	//	int		CB_Wr,CB_Rd,CB_Len;
	//	UCHAR*	CBuf;
	int		tSm;

	SockHeader		HeaderBuf;

	int		SizeBufCl;
	UCHAR	*BufCl;	/* buffer for sending and receiving data */
	//	char	bufcl[MaxPack];	/* buffer for sending and receiving data */

	int		SmBuf;

	char*	OutBuf;
	int		LenOBuf;
	int		MdTr;
	//	char	OutDBuf[MaxPack];

	int rcvbufsz;
	int sndbufsz;
	int rcvbuf_fill;
	int sndbuf_fill;
	Services	*pService;
	Decoder		*pDecoder;
	INT64		PackCnt,PackCntIn;
	Damper		Dmp;
	DamperPS	Dmp0,Dmp1;
	int			PriorState;
	float		PriorLev;
	int			GlobalCnt;
	UCHAR		GlobalCntIn;
	int			Fl_GCntIn;
	Sock_El*	SockEl;
	ID_List8	IDL;

	INT64		hCE;
	FileTime	TimeConn;
	FileTime	TimeDisConn;
	//	int			Fl_DB;

	ClientSock();
	ClientSock(char *ip, int port,
		int	(*_CallBackEvent)(ClientSock*,int) = NULL,
		int (*_readFunction)(ClientSock*,UCHAR*,int) = NULL,
		Services *pSrv=GetServices(),Decoder *pDec=NULL);
	virtual	~ClientSock();
	virtual	int	Connect();
	virtual	int DisConnect(char *mess=NULL,int rc=0);
	virtual	int	WaitConnect(int);
	virtual	int	SendBuf(char*,int,int Priority=0);
	virtual	int	SendCircBuf(char*,int);
	virtual	int	SendNotBuf(char*,int);
	virtual	int	SizeCB(int);
	//	int	ReadBuf(char*,int);
	//	char *GetNetName();

	//	void Ctrl();
	virtual	int Idle();
	virtual	int SendFromBuf();
	virtual	void SetMode(int Md){Mode=Md;};
	virtual	int	Trace(char*,int prm=-1,char *str=NULL);
	virtual	int GetFill();
	virtual	int Receive(PUCHAR p,int c,int f);
};

class	ClientSockEx : public ClientSock
{
public:
	int	Div;
	int tSm;
	int	rc2;
	int	Fl_RS;
	int	tlen;
	int	rc;
	int	tLen;
	int tCnt;
	int	Fl_Calc;
	FileTime	RdTime;
	ClientSockEx(char *ip, int port,Services *pSrv=GetServices(),Decoder *pDec=NULL);
	//virtual		~ClientSockEx();
	virtual	int	ReadSock(UCHAR* &Buf, size_t &Cnt);
	virtual	int	ReInit(char *ip, int port, char *host=NULL);
};


struct EventMsg
{
	int Event;
	int Number;
	//	ServerClientSock* csock;
	char	namehost[40];	// name host
	char	IPhost[20];	// IP string
	ULONG	IPadr;	// IP in long
	//	char	Host[40];
	//	char	IP[16];
	int		port;
	int		Handle;		// client socket
};
struct EventMsgCli
{
	int Event;
	//	ClientSock* csock;
	char	namehost[40];	// name host
	char	IPhost[20];	// IP string
	ULONG	IPadr;	// IP in long
	char	Host[40];
	char	IP[16];
};

int fCallBack(ClientSock* S,int Event);

class	ElSockCliEx	:	public NetObject, public ClientSockEx
{
public:
	int State;
	int Enable;
	int FlagEventArray[1024];
	int index_w, index_r, index_m;
	EventMsg tEMsg;

	char	IPAddr[80];
	int		Port;

	HANDLE	hThConn;
	DWORD	*TID_Conn;
	int		StartConn;
	int		WatchDogPack;
	int		tDiv,SetDiv;

	int DebugCount;

	ElSockCliEx(int tObj, char *value, Services *pSrv=NULL,Decoder* pDec=NULL) :
	NetObject(tObj, value,pSrv), 
		ClientSockEx("127.0.0.1",500,pSrv,pDec)
	{
		State=0;
		Enable=0;
		//FlagEvent=0;
		StartConn=0;

		Mode=0;
		TimeOut=10000;
		SizeCB(262144);
		strcpy(IPAddr,"127.0.0.1");Port=500;
		WatchDogPack=COM_WatchDog;
		tDiv=0;SetDiv=500;
		CallBackEvent=fCallBack;

		index_w=index_r=0; index_m=0x3ff;
		memset(FlagEventArray,0,sizeof(FlagEventArray));

		DebugCount=0;
	};	
	virtual	~ElSockCliEx()
	{
		while (StartConn==1)
			Sleep(10);
		if (StartConn==2)
		{
			CloseHandle(hThConn);
			StartConn=0;
		}
	};
	virtual	int SetParam(char* Prm,char* Vol);
	virtual	int	RdEvent(PUCHAR &,size_t &);
	virtual	int Start();
	virtual	int Stop();
	virtual	int	ObjIdle();
	virtual	int	GetHostName();
	virtual	int	SetEvent(int Event);
	virtual	int	Debug(char *q);
};
typedef ElSockCliEx *pElSockCliEx;

#define	MaxPack		32768
class ServerSock;

class ServerClientSock// : public UI_ELEMENT
{
public:
	int		TimeOutWRB;
	int		TimeOut;

	char	OutDBuf[MaxPack];

	int		Mode;
	int		Flag;
	ServerSock *servSock;
	int		NumInList;
	ULONG	IPadr;	// IP in long
	char	IPhost[20];	// IP string
	char	namehost[40];	// name host
	SOCKET	Handle;		// client socket
	struct	sockaddr_in client;	// client address information

	int	(*readFunction)(ServerClientSock *csock,PUCHAR buf, size_t Cnt);
	ClassThread	*ClThread;
	fd_set	Srd,Swr,Sex;

	int		SizeBufCl;
	UCHAR	*BufCl;	/* buffer for sending and receiving data */
	//	char	bufcl[MaxPack];	/* buffer for sending and receiving data */

	int		SmBuf;

	//	int		CB_Wr,CB_Rd,CB_Len;
	//	UCHAR*	CBuf;

	SockHeader		HeaderBuf;

	char*	OutBuf;
	int		LenOBuf;
	int		MdTr,tSm;

	int rcvbufsz;
	int sndbufsz;
	INT64		PackCnt,PackCntIn;

	Services	*pService;
	Decoder		*pDecoder;
	Damper		Dmp;
	DamperPS	Dmp0,Dmp1;
	int			PriorState;
	float		PriorLev;
	int			GlobalCnt;
	int			GlobalCntIn;
	int			Fl_GCntIn;
	Sock_El*	SockEl;
	ID_List8	IDL;

	INT64		hCE;
	FileTime	TimeConn;
	FileTime	TimeDisConn;

	int port;

	ServerClientSock(){TimeOutWRB=0;PackCnt=0;PackCntIn=0;Dmp.Clear();hCE=0;pService=GetServices();TimeConn.Time=0;TimeDisConn.Time=0;PriorLev=0;PriorState=0;GlobalCnt=0;Fl_GCntIn=0;};
	ServerClientSock(SOCKET handle, ServerSock *ssock,Services *pSrv=GetServices(),Decoder* pDec=NULL);
	virtual ~ServerClientSock();

	virtual	int		SizeCB(int Size);
	virtual int		SendFromBuf();
	virtual	int		SendCircBuf(char*,int);
	virtual	int		SendBuf(char *wbuf, int wlen,int Priority=0);
	virtual	int		SendNotBuf(char *wbuf, int wlen);
	virtual	int		SockExit();
	virtual	void	SetMode(int Md){Mode=Md;};
	virtual	int		Idle();

	virtual	int		DisConnect(char *mess=NULL,int rc=0);
	virtual	int		Trace(char*,int prm=-1,char *str=NULL);
};

class ItemClientList
{
public:
	int f,id;

	int PackIn;
	int PackOut;
	int CountIn;
	int CountOut;

	//int pObjPtr;
	ServerClientSock	*pObjPtr;
	SOCKET SockHandle;

	int port;
	char ip[256];

	ServerClientSock *obj;

	ItemClientList()
	{
		PackIn=0;
		PackOut=0;
		CountIn=0;
		CountOut=0;

		pObjPtr=0;
		SockHandle=0;
	};

	virtual ~ItemClientList(){};
protected:
private:
};
typedef ItemClientList *pItemClientList;
// ---------------------------------------------------------------------

class ClientList
{
public:
	int	n;
	ItemClientList list[NumSockCli];

	ClientList(int);
	virtual ~ClientList();
	virtual int  AddID(int,ServerClientSock* obj,PVOID PTR=NULL);
	virtual void DelID(int);
	virtual ServerClientSock* GetID(int);
	virtual ServerClientSock* Get(int);
	virtual void Del(int);
	virtual int Count();
	size_t CurrentItem;
	ItemClientList* GetFirst();
	ItemClientList* GetNext();
}; typedef ClientList *pClientList;


class ServerSock
{
public:
	int					CBSize;
	HANDLE				hmtx;
	void*				ID_Srv;
	//	int		On;
	int		Mode;
	int		nClient;
	int	short portTCP;
	unsigned long	IPadr;	// IP in long
	char	IPhost[20];		// IP string
	char	namehost[40];	// name host
	SOCKET	sHandle;            // server socket
	hostent	*HostEnt;
	struct	sockaddr_in client; // client address information
	struct  sockaddr_in server; // server address information

	int		NumCliSrv;
	size_t		CurrentClient;
	ClassThread	*ClThread;		
	//	HANDLE hThreadServer;
	//	DWORD *T_ID_SRV;
	int		NetType;
	int	(*CallBackEvent)(ServerClientSock *csock,int Event);
	int	(*readFunction)(ServerClientSock *csock, PUCHAR buf, size_t Cnt);
	Services	*pService;
	Decoder		*pDecoder;
	void DisConnect(int NumCli);
	int	SendBuf(int NumCli,UCHAR* buf,size_t Cnt);	// áëîêè ëþáîãî ðàçìåðà
	// äàëåå áüþòñÿ íà ïàêåòû ìåíüøå MaxPack ñ äîáàâëåíèåì ñèíõðî-êîìáèíàöèé â íà÷àëå è â êîíöå
	// â êîíöå äëÿ òî÷íîãî îïðåäåëåíèÿ êîíöà ïàêåòà
	int	SendBuf(PUCHAR buf,size_t cnt) { return SendBuf((int)CurrentClient,buf,cnt); };
	ClientList *List;

	ServerSock(){};
	ServerSock(int port,
		int (*_CallBackEvent)(ServerClientSock *csock,int Event),
		int	NetType,
		int (*_readFunction)(ServerClientSock *csock,PUCHAR buf, size_t len),
		int	NumCli = NumSockCli,
		Services *pSrv=GetServices(),Decoder* pDec=NULL);
	virtual ~ServerSock();

	virtual	int		SizeCB(int Size);
	virtual	int		Accept();
	virtual	int		TestClient();
	virtual	int		SockExit();
	virtual	void	SetMode(int Md){Mode=Md;};
	virtual	void	TimeOut(int TOut);
};


int	ElSockSrvEx_EventFun(ServerClientSock *csock,int Event);
class ServerSockEx : public ServerSock
{
public:
	int	LastNumCli;
	ServerSockEx(int port,
		int (*_CallBackEvent)(ServerClientSock *csock,int Event),
		//		int (*_readFunction)(ServerClientSock *csock,PUCHAR buf, int len),
		int	NetType,
		int	NumCli = NumSockCli,
		Services *pSrv=GetServices(),Decoder *pDec=NULL):
	ServerSock(port,_CallBackEvent,NetType,NULL,NumCli,pSrv,pDec)
	{
		LastNumCli=0;
	};

	virtual	int ReInit(int);
	virtual	int	Accept();
	virtual	int	ReadSock(int NumCli,UCHAR* &Buf, size_t &Cnt);
	int	ReadSock(PUCHAR &buf,size_t &cnt) { return ReadSock((int)CurrentClient,buf,cnt); };

};
// ---------------------------------------------------------------------

class	ElSockSrvEx	:	public NetObject, public ServerSockEx
{
public:
	int FlagEvent;

	int WrMsg,RdMsg;
	EventMsg EvMsg[1024];
	int	WatchDogPack;
	int	tDiv,SetDiv;


	ElSockSrvEx(int tObj, char *value,int NetType, Services *pSrv=NULL,Decoder* pDec=NULL) :
	NetObject(tObj, value,pSrv), 
		ServerSockEx(500,ElSockSrvEx_EventFun, NetType, NumSockCli, pSrv,pDec)
	{
		FlagEvent=0;

		Mode=0;
		TimeOut(10000);
		SizeCB(262144);
		ID_Srv=NULL;
		//	strcpy(IPAddr,"127.0.0.1");
		//	Port=500;
		WrMsg=RdMsg=0;
		WatchDogPack=COM_WatchDog;
		tDiv=0;SetDiv=500;
	};	
	virtual	~ElSockSrvEx(){};
	virtual	int SetParam(char* Prm,char* Vol);
	virtual	int WrEvent(ServerClientSock* csock,int Event);
	virtual	EventMsg* RdEvent();
	virtual	int Start();
	virtual	int Stop();
	virtual	int	ObjIdle();
};

int UDPFun(LPVOID	Prm);
//int UDPFunExt(LPVOID	Prm);
#define HandleUDP	0x10000
typedef ElSockSrvEx *pElSockSrvEx;

class ElUDPCli	:	public ID_Elem
{
public:
INT64				Vol;
int					NetType;
int					Port;
ID_List8			UserList;
Services*			Srv;
SOCKET				ss;
char				StrPort[8];
int					dontblock;
char				host[256];
ClassThread*		ElThr;
//ClassThread*		ElThrExt;
char				UDPFunName[64];
//----------------------------------------
struct sockaddr_in	peerUDP;
struct sockaddr		peerUDPIn;
struct sockaddr_in	peerUDPOut;
// ----------- Input buffer --------------
struct sockaddr		SourceUDP;
char				InBuf[2048];
// ---------------------------------------
int					SizeSource;
//----------------------------------------
DamperPS			Dmp;
					ElUDPCli(int NetType,int Port);
					~ElUDPCli();
virtual	void		Init(int NetType,int Port)
					{
						Vol=Port+((INT64)NetType<<16);
						Buff=(UCHAR*)&Vol;
						Len=sizeof(INT64);
					};
virtual	bool		AddUser(INT64 UniqUserID);
virtual	bool		DelUser(INT64 UniqUserID);
virtual int			Send(UCHAR* Buf,int Cnt);
virtual int			SendTo(UCHAR* Buf,int Cnt,struct sockaddr * Peer);
//virtual	int			Recive();
virtual	void		Idle();
//virtual	void		IdleExt();
protected:
private:
};

#define SizeMFBuf		10
#define MaxNumWANCli	4096
#define	SizeMainFiles	5

int SrvNetFun(LPVOID	Prm);
int SrvSysFun(LPVOID	Prm);

class	ServerDecod;

class	Services	:	public	Decoder
{
public:
	Integrator	*SInt;
	char		HostName[256],UserName[256],MainDomainName[256],WorkgroupName[256],ShortHostName[80];
	Savelog		*Log;
	char		PathCFG[512];
	Task		*Tsk;
	SrvCfg		Cfg;
	DLLCntrl*	DLLL;
	int			MyNum;
	Log_List	*IDL_Log;
	BandleList	*BL_Log;
	char*	    PathLog;
	char*	    PathBD;
//	Decoder*	NetMen;
	double		MemUsage;
	struct in_addr IPAddrPAN;
	struct in_addr IPAddrLAN;
	struct in_addr IPAddrWAN;
	struct in_addr IPAddrWANCli1;
	struct in_addr IPAddrWANCli2;
	Decoder*		FPurger;
	double			CPUUsage;
	double			CPUUsageS;
	double			CPUUsageSS;
	double			Fcpu;
	double			Fcpu_1;
	CMainCfg*		M_Cfg;
	StrData			*SData;
	ServerDecod*	Neuron;

	double			SmTime;
	ID_List8*		TraceLst;
	int				Fl_FSyn;
	int				Fl_Repl;
	int				TimeInt;
	int				On;
	int				Fl_NetLock;
//
	IDTable*		IDT_NeuronGlobal;
//	IDTable*		IDT_Prm;
	IDTable*		IDT_NeuronLocalCustomer;
	IDTable*		IDT_NeuronLocalSite;
//
//	Decoder*		UnitView;
	Decoder*		ThIDView;
//	Decoder*		BDBIDInfo;
	Decoder*		pMyErr;
	int				Fl_HardLock;
	int				Fl_Prm;
	int				System_On;
	int				Fl_SGUI;
	int				Fl_ReadyRepl;
	Decoder*		NetCommLAN;
	FileTime		TimeNetLock;
	int				Cli1,Cli2;
	FileTime		WANCli1FT;
	FileTime		WANCli2FT;
	int				WANSISrv;
	UnitMng*		UM;
	ListIP			IPList;
	int				TypeTime;
	HINSTANCE		ModuleVocAll;
//	Glucator		Gluc;
	char		MainPath[256];
	int			Fl_Stop;
protected:
	int				CntIdle;
//	Decoder*		SendMt;
	//	int				Fl_GIDI;
	//	IntTest			ITest;

//	FlagStruct		Fl_RE;		// CliList
	FlagStruct		Fl_RE1;		// CliList
	FlagStruct		Fl_RE2;		// GetNewIDI DeleteIDI
	//FlagStruct		Fl_RE3;		// Telemetr DelMt4Tlm Services->Idle
	FlagStruct		Fl_RE4;		//
	FlagStruct		Fl_RE5;		// OutLog
	FlagStruct		Fl_RE6;		// DataFF GetUID
	FlagStruct		Fl_RE7;		// CalcNetIP
	FlagStruct		Fl_RE8;		// NewDecod DeleteDecoder LoadLCfg SaveLCfg
//	FlagStruct		Fl_RE9;		// ReadSock CloseSock InitSock SendSock 
	FlagStruct		Fl_RE10;	// CloseTable 
	FlagStruct		Fl_RE11;	// GetSysPack 
	FlagStruct		Fl_RE12;	// Pack2Table	
	FlagStruct		Fl_RE13;	// GetUID

//	FlagStruct		Fl_MT;
//	FlagStruct		Fl_MTSI,Fl_MTSI2;
//	FlagStruct		Fl_MTSG;
//	FlagStruct		Fl_MTSID;
//	FlagStruct		Fl_Idle;
//	FlagStruct		hmtxMainThr;
//	FlagStruct		Fl_hmtx;

	int				Fl_ExtSin;
	FileTime		TimeLES;
	double			MedFltBuf[SizeMFBuf];
	int				IndexMF;
	int				NumIP;
	int				tFl_PAN,tFl_WAN,tFl_RunSI,tFl_RunSICli;
	
	int				ActCli;
	int				NumWANCli;
	bool			FlWANCli[MaxNumWANCli];
	IDInfoConverter	*IDICnv;
	MemClaster		FreeMC[SizeMC];
	MemClaster		AllocMC[SizeMC];
	int				MemClasterCnt;
	HANDLE			MapApp;
	ID_List8		*DllFileQwe;
	int				Fl_UpData;
	ID_List8		*MainFileQwe;
	int				Fl_UpData2;
	Module			pMod[SizeMainFiles];	// Òàáëèöà ñî ñòðóêòóðàìè Module for MainFiles
	int				CDiv3;
	char		NameUniCfg[256];
	UCHAR		*SysPackMem;
	INT64		RSizeSysPack;
//	HANDLE		hmtx;
	HANDLE		hmtxlog,PortD;
//	HANDLE		hmtxMainThr;
	HOSTENT		*hoststr; 
//	long		INetAdr;
	int			CountMode;
//	ClassList	*IDI_IS;
//	ID_El_CL	*ID_El;
	int			CDiv,CDiv2;
//	ParamList	PList;
	int			Net_Flag,Fl_SI;
//	Decoder*	SysInt;
//	PackNetObj	PNetObj;
	El_Log		*El_L;
	INT64		TimeConst,TS_CE;
	int			CntV;
	INT64       TimGUI;
	INT64		CntT,CntT2,CntT3,CntT4,CntT5,CntT6,CntT7,CntT8,CntTS4;
	double		TimeErr,sErr,difErrOld,FreqErr,DispErr,DispdifErr;
	double		Err[10*NumTF],difErr[10*NumTF];
	double		tErr,OldErr,OldDErr,sDErr,dFcpu,odFcpu;
	int			PoiNTF,PoiNTF2,KdifCnt;
	int			NDayLog;

	ID_List8*	EventFn;
	BandleList	*BL_Telemetr;
//	Chronometry	*Chron;
	int			Ver;
//	ID_El_CL	*ECl,*ECl2;
//	El_Tab		*ETbl,*ETbl2;
	int			NumEvent;
	int			FlReInit;
	int			CntTime;
	double		FcpuMax;
	double		FcpuMin;
	int			CntTK;
	int			NDayCE;
	char		namehost[40];
	Decoder		*tObj;
	INT64		NClickOld,RealTime,oNClk,oldNClk,CE1,CE2;
	int			FlSetT;
	int			FlReCur;
	FileTime	tTm,tTm2,TGID;
	ID_List8*	IDInfoCLst;
	// --------Test--------------
	double		VarErr,VardifErr,MVarErr;
	INT64		CntST;
	BandleList	*DbgBList;
	char		DbgLine[10240];
	ID_List8*	ActIDIList;
	ID_List8*	DelIDIList;
	int			Fl_TimeSyn;
public:
	INT64		SizeAllocMem;
protected:

	Decoder*	FSrv;
	ID_List8*	TableList;

//============================
//	Decoder*	Mt_IDIRepl;
	DamperPS	*DmpCE;
	DamperPS	*DmpDCE;
//	Decoder*	NetCommWAN;
	int			FlagEnter;
	int			Fl_StU;
	int			CntHost;
//============================
	Parametr	StatPrm;
//	IDTable*	IDT_ComSys;
	HINSTANCE	ModuleHandle;

	void		*VDlg;
//	hasp_handle_t	handle;
//	hasp_status_t	status;
	int				SizeNetMen;
//	uchar*			NetMenBuf;
protected:
	FindStrList8	FSL_8;
	FindStrList8	FSL85;
	LicenseCond		LCond;
	LicUnitInfo		tLUInfo;
	LC_RunStat		LRStat;
	LC_PrmHost		LPHost[50];
	LC_RunStat		NetLRStat;
	LC_PrmHost		NetLPHost[50];
public:
	LicenseCond		NetLCond;
	LC_SysInfo		LSinfo;
	LC_UnitInfo		LUinfo;
	LC_UserInfo		LUSinfo;
	LC_DomainInfo	LDinfo;
	LC_SysInfo		NetLSinfo;
	LC_UnitInfo		NetLUinfo;
//	CNeuron64Dlg	*dlg;
// -----------------------------------
protected:
	int				Fl_Recurce;
	ID_List8		*AddList;
	INT64			CntT9,CntNumDec;
// NetServices
	int				State;
	int				Counter;
	int				TimeClick; 
	pID_List8		ClientList;	
public:
	pID_List8		MonClientList;	
protected:
	// -----------------------------------
	int				StateOut;
	PUCHAR			SockBuf;
	size_t			SockCnt;
	int				StopProcessing;
	int				TelemetryCount;
	pNetObject		pCurrnetTelem;
//	FlagStruct		Fl_RE;	// !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!11
	int				CntCliPack;
	FindStrList8	CFSL8;
//	FindStrList8	IFSL8;
	ClassThread*	NetThread;
	ClassThread*	Sys_Thread;
	DamperPS		NetDmp;
//	INT64			tTime1,tTime2,tTime3,tTime4,tTimeInt;	//Поискнаиболее загруженной части Айдл-функции сервиса.
//	INT64			dtTime1,dtTime2,dtTime3,dtTimeAll;	//Поискнаиболее загруженной части Айдл-функции сервиса.
	ID_List8*		ActiveDecoderList;
// -----------
	Customer*		CustomerInfo;
	INT64			RCntGUIActivities;
	FileTime		LastUserActivitiesTime;
public:
	char			UserLogin[128];
	INT64			UserID;
	INT64			CntGUIActivities;
	El_Tab			*ElHeap;
	int				UserStatus;
	FlagStruct		FlSimCT;
	int				Fl_ConnectSrv;
					Services(HANDLE	MapApp, char* PathLog,char	*Path,char	*PathDll,char	*PathBD,int FlRI,CMainCfg* MCfg,int NDay=5,int FlLocGlob=0,int FlSTime=0,int NDayCE=30);
virtual				~Services();
virtual	int			DataFF(			// Äàííûå äëÿ äåêîäèðîâàíèÿ
					int N_Pin,		// Íîìåð âõîäíîé íîæêè
					UCHAR* pmas,	// Óêàçàòåëü íà áóôåð ñ äàííûìè
					size_t Cnt);		// Äëèíà äàííûõ
virtual	int			DataIDI(			// Äàííûå äëÿ äåêîäèðîâàíèÿ
					int N_Pin,		// Íîìåð âõîäíîé íîæêè
					IDInfo* pIDI);		// Äëèíà äàííûõ
virtual	int			UserCallBack(int	Handle,int N_Pin,UCHAR* pmas,size_t Cnt);
virtual	int			UserCallBackIDI(int Handle,IDInfo* idi,StrOwner *Own);
virtual	INT64		GetUID();
virtual	void		OutLog(char* pBuf, Decoder* PObj=NULL, char* Type=NULL);
virtual	int			SaveLCfg(char* UnitID, void* pCfg, int Size, Decoder* pObj); // Ñîõðàíèòü ëîêàëüíóþ äëÿ ìåòîäà êîíôèãóðàöèþ (-1 - îøèáêà)
																			// Èñïîëüçîâàòü â Stop()
virtual	int			LoadLCfg(char* UnitID, void* pCfg, int Size, Decoder* pObj); // Çàãðóçèòü ëîêàëüíóþ äëÿ ìåòîäà êîíôèãóðàöèþ (-1 - îøèáêà)
																			// Èñïîëüçîâàòü â Start()

virtual	char*		FirstMode();	// Ñïèñîê ñóùåñòâóþùèõ ðåæèìîâ ñåðâåðà   Ïîèñê ïåðâîãî
virtual	char*		NextMode();	//										 Ïîèñê ñëåäóþùåãî
virtual	char*		GetNameMode(int NumMode);	//	Ïîëó÷èòü èìÿ ïî íîìåðó
//virtual	ClassList*	GetInfo(){return IDI_IS;};
virtual	Task*		GetTask();
virtual void		SetTask(Task* tTsk) {Tsk=tTsk;};
virtual	void		Exit() { On=0;};
virtual	HANDLE		GetPortD(){ return PortD;};
//virtual	void		Idle();
virtual	int			IdleFunc(int NumberTr);
virtual	bool		Timer(INT64 &Cnt,INT64 Time)	// Time in mS Òàéìåð - Ïðîðåæèâàòåëü
					{
						INT64	tCnt=GetNClk();
						INT64	tTime=Time;
						bool	rc=(((tCnt-Cnt)>(tTime*TimeConst))||((tCnt-Cnt)<0));
						if (rc) // 
						{
							Cnt=tCnt;
						};
						return rc;
					};
virtual int			InitUnit(Decoder* PoiObj);
virtual int			CloseUnit(Decoder* PoiObj);
virtual	int			InitSock(Decoder* PoiObj,int Mode,short NetType, char* Port=NULL,char* IP=NULL);	// Èíèöèàëèçàöèÿ CallBack-ôóíêöèè äëÿ Ñîêåòà
						// -1 Íåìîãó ñîçäàòü SI
						// çàðåãèñòðèðîâàííûé èäåíòèôèêàòîð
						// Mode =0-Cli SI 1-Cli 2-Srv  (BufPack)
 						// Mode =4	      5-Cli 6-Srv  (BufNoPack)
						// Mode =8		  9-Cli 10-Srv (NoBufNoPack)
						// Mode|=16 åñëè áóäåì îïðàøèâàòü ñàìîñòîÿòåëüíî ÷åðåç ReadSock
						// NetType = NT_PAN, NT_LAN, NT_WAN
virtual	int			InitEventFn(Decoder* PoiObj);	// Èíèöèàëèçàöèÿ CallBack-ôóíêöèè äëÿ Ñîáûòèé
virtual	void		CloseEventFn(int Handle);	// Çàêðûòü CallBack-ôóíêöèþ Ñîáûòèÿ
virtual	void		SendEvent(int Event, UCHAR* Packet, size_t Cnt);
 		
virtual	int			CloseSock(int Handle);		// Çàêðûòü CallBack-ôóíêöèþ Ñîêåòà
virtual	int			SIPack(int Handle,int Type,UCHAR* Prm=NULL,size_t Cnt=0);	// Ïàêåò Ñèñòåìíîìó èíòåãðàòîðó
virtual	int			SIPack(int Handle,int N_Pin,int Type,UCHAR* Prm=NULL,size_t Cnt=0);	// Ïàêåò 
virtual	int			SendSock(int Handle,int N_Pin,UCHAR* Buf,size_t Cnt);	// Îòïðàâèòü ïàêåò Ñîêåòó
virtual	void		PrmSock(int Handle,char* Port,char* IP=NULL,int SizeBuf=0,int TimeOut=0);	// Óñòàíîâèòü ïîðò è àäðåññ
virtual	void		Telemetr(Decoder*,char* NamePrm,double Vol,PVOID pOO=NULL);
virtual	void		Telemetr(TlmData* NamePrm,double Vol);
virtual	void		DelMt4Tlm(Decoder*);
virtual	SysPack*	GetSysPack(size_t Size);
virtual	void		SendGetUnitInfo(void);
//virtual	TabSysCli*	FindUnit(char* Name,char* ID=NULL) { return SIM->FindUnit(Name,ID); };
//virtual	TabSysCli*	FindUnit(char* Name,char* MSrv,char* SSrv,ID_El_CL* El,int &StateFindSrv);
//virtual	TabSysCli*	GetUnit(int Num) { return SIM->GetUnit(Num); };
virtual	void		GetRealTime(FILETIME* RealTime); // 1c=100 ns * 10 000 000
virtual	char*		GetHostByAddr(char* Adr);
virtual	void		ReplCfgOn();
virtual	void		ReplCfgOff();

virtual	INT64		SetCriticalError(Decoder* PObj,char* Msg,char* TypeMsg,char* MSystem="Default",INT64 Handle=NULL);
virtual	void		DelCriticalError(INT64 Handle);



virtual	void		ExtSynTime(FileTime FTime,int Fl=0);
virtual	bool		TestHost(int NumHost);
virtual	bool		TestHost(char* Host);

protected:

virtual	void		SetCriticalErrorI(char* ObjName,char* Msg,char* TypeMsg,char* MSystem,INT64 Handle);
virtual	void		DelCriticalErrorI(INT64 Handle);

ClientsList			CliList[NumSICli];

virtual	int			LoadCfg();
//virtual	int			SaveCfgID();
virtual	int			SaveCfg();
public:
//virtual	int			ReadSock(int Handle,int &N_Pin,UCHAR* &Buf,size_t &Cnt);
virtual	FileTime	GetDeltaTime(INT64 NClk); // 1c=100 ns * 10 000 000
virtual	Decoder*	NewDecod(char* Name,Decoder* pDec);
virtual	void		DeleteDecoder(Decoder* pDec);
virtual void		ClearDebugLine()
					{
//						MyLock(__FILE__,__LINE__,&Fl_RE,this);
						DbgBList->DelAll();
//						MyUnLock(__FILE__,__LINE__,&Fl_RE,this);
					};
virtual void		AddDebugLevel(Decoder* pDec,char* tBuf);
virtual void		SubDebugLevel(Decoder* pDec);
virtual char*		GetDebugLine(Decoder* pDec);
virtual IDInfo*		GetNewIDI(Decoder* ParMt,int Fl_CreateAdrInfo=0);	// Ðåêîìåíäîâàíà äëÿ èñïîëüçîâàíèÿ
virtual int			DeleteIDI(IDInfo* IDI,int Fl_DeleteAdrInfo=0);		// -1 îøèáêà 0 îk
// ---------------------------------------------
virtual Parametr*	GetParam(INT64 ID);
virtual Parametr*	GetParam(char* Name);
virtual Parametr*	GetParamVol(INT64 ID,BandleList* CSL);
// ---------------------------------------------
protected:
virtual	IDTable*	OpenTable(Decoder* ParMt,INT64 ID,int CustomID=0);			// -> ReadyTable
virtual	int			CloseTable(Decoder* ParMt,IDTable* Tbl);					// -> Immediately closed
public:
virtual	void		CalcTable(void);

virtual	int			AddFSrvLink(Decoder* pDec,int PinI);
virtual	int			OpenThIDView(void);
virtual	int			CloseThIDView(void);

virtual Parametr*	GetParamVol(char* Name,BandleList* CSL);
virtual	int			SendElTabl(El_Tab* tElT);
virtual	int			ReplElTabl(El_Tab* tElT,int Mode=0);	// 0 - From Table to Service this Domain 1 - Form DomainContr to other DomainContr
virtual	int			NS_DataFF(int N_Pin,UCHAR* pmas,size_t Cnt,Decoder* pDec=0);
virtual	int			GetTelemetry();
virtual	pNetObject	GetTelemetryPtr(int first=0);
virtual	int			IdleNet();
virtual	int			IdleSys();
virtual	int			InternalCloseSock(int Handle);		// Çàêðûòü CallBack-ôóíêöèþ Ñîêåòà

void*				malloc(size_t size)
					{	
						char	ttb[128];
						void*	tB=::malloc(size);
						if (tB!=NULL)
						{
							SizeAllocMem+=_msize(tB);
//							sprintf(ttb,"%08X Alloc=%d(%d) All=%d Services",tB,_msize(tB),size,SizeAllocMem);
//							OutLog(ttb,NULL,"Memory");
						}
						else
						{
							sprintf(ttb,"Error Alloc=%d All=%I64d Services",size,SizeAllocMem);
							OutLog(ttb,NULL,"Memory");
						}

						return tB;
					}
void*				realloc(void*tBuf,size_t size)
					{	
						char	ttb[128];
						size_t	tSize=_msize(tBuf);
						void*	tB=::realloc(tBuf,size);
						if (tB!=NULL)
						{
							if (SizeAllocMem==0)
								SizeAllocMem=tSize;
							SizeAllocMem-=tSize;
							if (SizeAllocMem<0)
							{
								OutLog("ErrorReAllocMemSize",NULL,"Memory");
								SizeAllocMem=0;
								SetCriticalError(NULL,"ErrorReAllocMemSize","Error");
							}
							SizeAllocMem+=_msize(tB);
//							sprintf(ttb,"%08X->%08X ReAlloc=%d(%d) Old=%d All=%d Services",tBuf,tB,size,_msize(tB),tSize,SizeAllocMem);
//							OutLog(ttb,NULL,"Memory");
						}
						else
						{
							sprintf(ttb,"Error ReAlloc=%d All=%I64d Services",size,SizeAllocMem);
							OutLog(ttb,NULL,"Memory");
						}
						return tB;
					}
void				free(void* tB)
					{	
						if (tB!=NULL)
						{
//							char	ttb[128];
							if (SizeAllocMem==0)
								SizeAllocMem=_msize(tB);
							SizeAllocMem-=_msize(tB);
							if ((SizeAllocMem<0)&&(Srv)&&(Srv->IDL_Log))
							{
								char	ttBuf[128];
								sprintf(ttBuf,"ErrorFreeMemSize=%I64d",SizeAllocMem);
								OutLog(ttBuf,NULL,"Memory");
								SetCriticalError(NULL,ttBuf,"Error");
								SizeAllocMem=0;
							}
//							sprintf(ttb,"%08X Free=%d All=%d Services",tB,_msize(tB),SizeAllocMem);
//							OutLog(ttb,NULL,"Memory");
							::free(tB);
						}
					}
//virtual	int			PostIDI(IDInfo* IDI,Decoder* Obj,char* NetType="ANY");	// Îòïðàâèòü ïàêåò ÷åðåç NetComm  NetType=ANY, PAN, LAN, WAN
virtual	int			SendIDI(IDInfo* IDI,Decoder* Obj,int NetType=NT_ANY);	// Îòïðàâèòü ïàêåò ÷åðåç NetComm  NetType=ANY, PAN, LAN, WAN
virtual void		InitCommIDI(IDInfo* IDI,int typeidi=TypeIDI_COMMAND);
virtual	int			ModifUnit(Decoder* PoiObj);
virtual	void		Pack2Table(IDInfo* IDI);
virtual	void		Stop();
virtual void		Start();
virtual	void		CopyLic(LicenseCond* LCond);
virtual	void		CopyLicUnit(LC_UnitInfo* LU);
virtual	int			CalcNetIP(IDInfo* IDI);	// return active page of IP list
virtual	int			NLSpecialNetIP(IDInfo* IDI,char* pUnitHost,char* pUnitName,INT64 pUnitID);
virtual	void		NLAdd(int IndPage,char* IP);
virtual	void		NLDel(int IndPage,char* IP);
virtual	int			NLFind(int IndPage,char* IP);	// NumLine
virtual	void		NLAddbyUnitName(char* UnitName,char* IP);
virtual	void		NLDelbyUnitName(char* UnitName,char* IP);
virtual	int			NLReturnIndex(char* UnitName);
virtual	int			CopyIDInfo(IDInfo* IDISrs,IDInfo* IDIDis);
virtual	IDInfo*		CrIDICopy(IDInfo* IDI,Decoder* Obj);
virtual	void		DelIDICopy(IDInfo* IDIS,IDInfo* IDID);
virtual	int			RoleSwOn(char* RoleName);	// -1 - Error Switch On Role 0 - Ok Switch On 1 - Too many roles, my be later
virtual	int			RoleSwOff(char* RoleName);	// -1 - Error Switch Off Role 0 - Ok Switch Off
protected:
virtual	LicUnitInfo*	TestMaxNumRole(char* RoleName);	// Number of  Role limitation
public:
virtual	UnitInfo*	GetActiveDC(char* DomName=NULL);
virtual	LicenseCond*	GetLC() {return &LCond;};
virtual	LC_RunStat*	GetLRS() {return &LRStat;};
INT64				RndVol;
virtual	INT64		Randomize();
virtual	void*		MallocHuge(size_t size);
virtual	void		FreeHuge(void* tB);
virtual	void		ReInitUnit(char* Host);
virtual	Module*		FindModule(char* Name);
virtual	int			SendToLocal(IDInfo* tIDI);
virtual	bool		TestPointer(PVOID	pObj);
// ---------------------------------------------------
#define SizeUDPPort (IPPORT_DYNAMIC_MAX-IPPORT_DYNAMIC_MIN)
bool				UDPPortFl[IPPORT_DYNAMIC_MAX-IPPORT_DYNAMIC_MIN];
ID_List8*			ListUDPCli;
virtual	int			GetUDPPort(void);
virtual	bool		ReleaseUDPPort(int Port);
virtual ElUDPCli*	OpenUDPCilent(int NetType,int Port, Decoder* PObj);
virtual bool		CloseUDPCilent(int NetType,int Port, Decoder* PObj);
// ----------------- APCS Function -------------------
virtual bool		TestLoginName(char *Name);
virtual INT64		TestLoginPassword(char *Name,char *Password);	//LogIn Return UserID
virtual	void		GUIActivities();
virtual	void		Logoff();									// Может использовать только авторизованный пользователь
virtual El_Tab*		FindUsersGroupBegin(char *Name,FindStruct*	FStr);
virtual El_Tab*		FindUsersGroupNext(char *Name,FindStruct*	FStr);
virtual	El_Tab*		GetGroupElByUser(El_Tab* User_El);

virtual bool		AddNewUser(char *Name,char *Password);			//+
virtual bool		AddNewGroup(char *Group);						//+
virtual bool		AddUserInGroup(char *Name,char *Group);			//
virtual bool		AddGroupInGroup(char *Group,char *GroupIn);		//
virtual bool		DelUser(char *Name);							//
virtual bool		DelGroup(char *Group);							//
virtual bool		DelUserFromGroup(char *Name,char *Group);		//
virtual bool		DelGroupFromGroup(char *Group,char *GroupFrom);	//
virtual INT64		GetUserID(char *Name);
virtual INT64		GetWorkGroupID(char *Group);
virtual char*		GetUserNameByID(INT64	UserID);
virtual char*		GetWorkGroupNameByID(INT64	UserID);
virtual bool		TestGroupLevelDown(char *Group);			//	 Может использовать только авторизованный пользователь
virtual bool		TestUserLevelDown(char *Name);			//	 Может использовать только авторизованный пользователь
virtual bool		TestGroupLevelUp(char *Group);			//	 Может использовать только авторизованный пользователь
virtual bool		TestUserLevelUp(char *Name);			//	 Может использовать только авторизованный пользователь
virtual bool		GetActiveUser(INT64 UserID);
virtual bool		GetActiveUser(char *Name);
//	For Sasha functions
virtual bool		Is_IncomingMsg(El_Tab* tEl);
virtual bool		Is_SelectingMsg(El_Tab* tEl);
virtual bool		Is_ReadedMsg(El_Tab* tEl);
virtual bool		AddHistoryLine(El_Tab* tEl,char* sBuf,int Fl_UnRead=0);
virtual bool		UnSelectMsg(El_Tab* tEl);

virtual	void		StopSock(int Handle);		// 
virtual	void 		StartSock(int Handle);		// 
//---------------------
};

struct ReplPack
{
	int			Ver;			// Âåðñèÿ ïàêåòà
	int			NumHost;
	INT64		UID;
	FileTime	TimeModif;		// Âðåìÿ èçìåíåíèÿ
};

struct	ElemVol : public ReplPack
{
	INT64		UpUID;
	int			TypeKey;	// 0-String 1-int64 2-ClassName
	union
	{
	char		KeyVol[128];
	INT64		KeyVolI64;
	};
	char		Remark[128];
	char		UserName[80];
	int			NumPrm;
};

struct PrmPack : public ReplPack
{
//	int			Ver;		// Âåðñèÿ ïàêåòà
//	INT64		UID;
//	FileTime	Time;		// Âðåìÿ èçìåíåíèÿ
//	int			NumHost;
	PrmDec		PDec;
};

struct ID_IDTime 
{
	INT64		ID;
	FileTime	FT;
};



class	BLDelElem		:	public	BandleList
{
protected:

ID_List8		DelList;
protected:
public:
void			DelOnTime(INT64 Time,IDTable* pIDT);	// in second
void			Add(INT64 ID);
void			DelElemID(INT64 ID);
bool			Find(INT64 ID);
int				CopyEl();
int				SaveEl();
};

class	BLDelPrm		:	public	BandleList
{
public:
void		Add(PrmPack* PPack);
bool		Find(PrmPack* PPack);
void		DelOnTime(INT64 Time);	// in second
};

//static	char	NameCL[]={"ClassList"}; 

struct DelHost
{
	int		Ver;	
	int		MyNum;	
	bool	DelFl[256];
};

struct	DataBuf
{
	UCHAR	*Buf;
	int		Cnt;
};

/*
struct	SeansInfo	// Ñ ðàñ÷åòîì íà òî, ÷òî ýà ñòðóêòóðà áóäåò äîáàâëåíà â pList
{
INT64	ID_Seans;
USHORT	ID_Meth;	// èäåíòèôèêàòîð ìåòîäà äåêîäèðîâàíèÿ ñîçäàâøåãî ñåàíñà			// 
UCHAR	ID_Host;	// èäåíèôèêàòîð õîñòà íà êîòîðîì çàïóùåí ñåðâåð äåêîäèðîâàíèÿ	// 
UCHAR	ID_Ext;		// ðåçåðâ (îáû÷íî ðàâåí 0, íî äîïóñêàþòñÿ è äðóãèå çíà÷åíèÿ)	//  

long	TimeBeg;
long	Duration;
int		StateSeans;	// òåêóùåå ñîñòîÿíèå äàííûõ ôàéëå
};

// StateBlock
	#define	StateSeans_BEGIN		1	// Ïðèçíàê íà÷àëà ñåàíñà
	#define	StateSeans_END			2	// Ïðèçíàê êîíöà ñåàíñà
// ...
*/
// Ñòðóêòóðû îïèñûâàþùèå äàííûå ïðè ïåðåäà÷å äàííûõ â ïàêåòíîì ðåæèìå 
// îñíîâíûå ïîëîæåíèÿ òåõíîëîãèè:
// 1 - ñòðóêòóðà IDInfo èñïîëüçóåòñÿ êàê òðàíñïîðò äëÿ íåñåòåâîé êîíâååðíîé îáðàáîòêè äàííûõ  
// 2 - òåõíîëîãèÿ èñïîëüçóåò ìåòîä ïàêåòíîé ïåðåäà÷è äàííûõ äëÿ íåîãðàíè÷åííîãî (*) êîëè÷åñòâà ïîòîêîâ (ôàéë äàííûõ)
// 3 - òàêîé ôàéë âñåãäà èìååò èäåíòèôèêàòîð, íà÷àëî è êîíåö
// 4 - äîïîëíèòåëüíî èäåíèôèêàòîð ðîäèòåëÿ, ñåàíñà, ñòàíäàðòà, àïïàðàòóðû, ñïèñêà ñ ïàðàìåòðàìè 
// 5 - åñëè StateFile=StateFile_SEANS òî IDInfo îïèñûâàåò ñåàíñ
// 6 - ó÷àñòîê ñòðóêòóðû IDInfo ñ ID_File ïî ID_SeansTo âêëþ÷èòåëüíî ÿâëÿåòñÿ óíèêàëüíûì êëþ÷åì 
// 7 - åñëè SeansFrom=0 è SeansTo=0 òî ïåðåäàåòñÿ ïðîñòîé ïîòîê äàííûõ
// 8 - åñëè SeansFrom!=0 èëè SeansTo!=0 òî ñóùåñòâóåò ñåàíñ ñîäåðæàùèé äàííûé ôàéë è îí áóäåò óíè÷òîæåí òîëüêî ïîñëå ïåðåäà÷è âñåõ äàííûõ
struct IDIStat : ID_Elem
{
INT64		ID_File;		// èäåíòèôèêàòîð ôàéëà (ñåàíñà)
INT64		ID_ParentFile;	// èäåíòèôèêàòîð ðîäèòåëüñêîãî ôàéëà
INT64		ID_SeansFrom;	// èäåíòèôèêàòîð ñåàíñà îòïðàâèòåëÿ èëè ïðîñòî ñåàíñà
INT64		ID_SeansTo;		// èäåíòèôèêàòîð ñåàíñà ïîëó÷àòåëÿ
INT64		ID_Standart;	// èäåíòèôèêàòîð Ïðîòîêîëà (Protocol) èëè Ôîðìàòà (Format) äàííûõ
FileTime	TimeBegin;		// Âðåìÿ íà÷àëà ïîñòóïëåíèÿ äàííûõ
FileTime	TimeLastWr;		// Âðåìÿ ïîñëåäíåé ïåðåäà÷è äàííûõ
FileTime	TimeClose;		// Âðåìÿ çàêðûòèÿ ôàçû ïåðåäà÷è äàííûõ
FileTime	TimeOper;		// Âðåìÿ ñîçäàíèÿ äàííîãî áëîêà äàííûõ
INT64		NumPack;		// Íîìåð ïàêåòà (èíêðåìåíòèðóåòñÿ äëÿ êàæäîãî ñëåäóþùåãî ïàêåòà)
int			ParentMtExt;
int			StateProc;	    // ñîñòîÿíèå öèôðîâîãî àâòîìàòà äëÿ ðàáîòû ñèñòåìû îòáîðà è îáðàáîòêè â ðåæèìå êîíâååðà
int			Command;		// êîìàíäà î òîì, ÷òî òðåáóåòñÿ ñäåëàòü ñ äàííûì êóñêîì ôàéëà
int			StateBlock;		// òåêóùåå ñîñòîÿíèå äàííûõ ôàéëå
int			TypeIDI;		// 
int			Priority;		// ïðèîðèòåò
int			ModifCnt;
int			CustomerID;		// -- 2+2 byte --
char		ParentMtName[32];
//int			Rezerv1;		// Âûðàâíèâàíèå
//int			Rezerv0;		// Âûðàâíèâàíèå
//int			Rezerv;
//Decoder		*ParentMt;		// Óêàçàòåëü íà âëàäåëüöà IDInfo
};
/*
struct tIDIStat : ID_Elem
{
INT64		ID_File;		// èäåíòèôèêàòîð ôàéëà (ñåàíñà)
INT64		ID_ParentFile;	// èäåíòèôèêàòîð ðîäèòåëüñêîãî ôàéëà
INT64		ID_SeansFrom;	// èäåíòèôèêàòîð ñåàíñà îòïðàâèòåëÿ èëè ïðîñòî ñåàíñà
INT64		ID_SeansTo;		// èäåíòèôèêàòîð ñåàíñà ïîëó÷àòåëÿ
Decoder		*ParentMt;		// Óêàçàòåëü íà âëàäåëüöà IDInfo
INT64		ID_Standart;	// èäåíòèôèêàòîð Ïðîòîêîëà (Protocol) èëè Ôîðìàòà (Format) äàííûõ
int			StateProc;	    // èñïîëüçåòñÿ êàê íîìåð êàíàëà !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
int			Command;		// êîìàíäà î òîì, ÷òî òðåáóåòñÿ ñäåëàòü ñ äàííûì êóñêîì ôàéëà
int			StateBlock;		// òåêóùåå ñîñòîÿíèå äàííûõ ôàéëå
int			TypeIDI;		// òåêóùåå ñîñòîÿíèå äàííûõ ôàéëå
int			Priority;		// ïðèîðèòåò
FileTime	TimeBegin;		// Âðåìÿ íà÷àëà ïîñòóïëåíèÿ äàííûõ
FileTime	TimeLastWr;		// Âðåìÿ ïîñëåäíåé ïåðåäà÷è äàííûõ
FileTime	TimeClose;		// Âðåìÿ çàêðûòèÿ ôàçû ïåðåäà÷è äàííûõ
FileTime	TimeOper;		// Âðåìÿ ñîçäàíèÿ äàííîãî áëîêà äàííûõ
INT64		NumPack;		// Íîìåð ïàêåòà (èíêðåìåíòèðóåòñÿ äëÿ êàæäîãî ñëåäóþùåãî ïàêåòà)
};
*/
struct	IDInfo : IDIStat
{
INT64			Cnt;			// Ðàçìåð äàííûõ
INT64			OldCnt;			// Ðàçìåð çàäåðæàííûõ íà òàêò äàííûõ
INT64			OrderNumber;	// Äëÿ ïîñòðîåíèÿ ñëîæíûõ èíäåêñîâ
ParamListID		*pPList;		// Ñïèñîê ñ ïàðàìåòðàìè
#ifndef _WIN64
INT				Z0;
#endif
UCHAR			*Buf;			// Óêàçàòåëü íà áóôåð ñ äàííûìè
#ifndef _WIN64
INT				Z1;
#endif
UCHAR			*OldBuf;		// Óêàçàòåëü íà áóôåð ñ çàäåðæàííûìè íà òàêò äàííûìè
#ifndef _WIN64
INT				Z2;
#endif
IDInfo			*ParentFile;	// óêàçàòåëü íà IDInfo ParentFile
#ifndef _WIN64
INT				Z3;
#endif
IDInfo			*SeansFrom;		// óêàçàòåëü íà IDInfo SeansFrom
#ifndef _WIN64
INT				Z4;
#endif
IDInfo			*SeansTo;		// óêàçàòåëü íà IDInfo SeansTo
#ifndef _WIN64
INT				Z5;
#endif
AddressInfo		*AdrInfo;		// For sending IDInfo to other unit
#ifndef _WIN64
INT				Z6;
#endif

	IDInfo(Decoder* ParMt=NULL)
	{
		OrderNumber=0;CustomerID=0;
		DBGC.Cnt_IDI_Info++;
		ID_ParentFile=0;ID_SeansFrom=0;ID_SeansTo=0;AdrInfo=0;
//		ParentMt=ParMt;
		ParentFile=0;SeansFrom=0;SeansTo=0;
		ID_File=0;
		ID_Standart=-1;StateProc=0;
		Command=0;StateBlock=0;TypeIDI=0;Priority=0;
		Cnt=0;OldCnt=0;NumPack=0;
		if (ParMt!=NULL)
			ParMt->Srv->GetRealTime(&TimeBegin.FTime);
		else
			GetSystemTimeAsFileTime(&TimeBegin.FTime);
		TimeLastWr.Time=0;
		TimeClose.Time=0;
		TimeOper.Time=0;
		ModifCnt=0;
		CustomerID=0;

		if (ParMt)
			pPList=new ParamListID(ParMt->Name_Mt);
		else
			pPList=new ParamListID("NoName");

		Init((UCHAR*)&ID_File,sizeof(INT64));
		if (ParMt)
		{
			strcpy(ParentMtName,ParMt->Name_Mt);
			ParentMtExt=ParMt->Ext_Mt;
		}
		else
		{
			ParentMtName[0]=0;
			ParentMtExt=0;
		}
	}
	void	ReInit(Decoder* ParMt)
	{
//		ParentMt=ParMt;
		OrderNumber=0;CustomerID=0;
		ID_ParentFile=0;ID_SeansFrom=0;ID_SeansTo=0;/*AdrInfo=0;*/	// !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
		ParentFile=0;SeansFrom=0;SeansTo=0;
		ID_File=0;
		ID_Standart=-1;StateProc=0;
		Command=0;StateBlock=0;TypeIDI=0;Priority=0;
		Cnt=0;OldCnt=0;NumPack=0;
		if (ParMt!=NULL)
			ParMt->Srv->GetRealTime(&TimeBegin.FTime);
		else
			GetSystemTimeAsFileTime(&TimeBegin.FTime);
		TimeLastWr.Time=0;
		TimeClose.Time=0;
		TimeOper.Time=0;
		ModifCnt=0;
		CustomerID=0;
		//Rezerv=0;
		pPList->DelAllElem();
		pPList->FlNew=1;pPList->LWrTime=0;
		Init((UCHAR*)&ID_File,sizeof(INT64));
		if (ParMt)
		{
			strcpy(ParentMtName,ParMt->Name_Mt);
			ParentMtExt=ParMt->Ext_Mt;
		}
		else
		{
			ParentMtName[0]=0;
			ParentMtExt=0;
		}
	}
	virtual ~IDInfo()
	{
		if (pPList==0)
		{
			//	Èñêëþ÷èòåëüíàÿ ñèòóàöèÿ (äâîéíîå óäàëåíèå pPList)!!!
//			throw;
		}
		else
		{
			delete pPList;
			pPList=0;
		}
		--DBGC.Cnt_IDI_Info;
	}
};

typedef IDInfo *pIDInfo;

struct	FileInfo			// : public IDInfo
{
	char	Name[80];		// Ïðåäïîëàãàåìîå èìÿ ôàéëà
	char	Path[256];		// Ïóòü ê ôàéëó
	char	Ext[80];		// Ðàñøèðåíèå ôàéëà
	int		NumDir;			// Íîìåð äèðåêòîðèÿ
	char	CreateName[512];// Ôàêòè÷åñêè ñîçäàíîå èìÿ ôàéëà
	char	C_Type[80];		// Ñòðîêîâîå çíà÷åíèå ïðåäïîëàãàåìîãî òèïà ôàéëà
	char	Type[80];		// òèï ôàéëà
	char	SubType[80];	// Ñåðâèñ èëè ðàñøèðåíèå òèïà ôàéëà
	int		C_Len;			// ïðåäïîëàãàåìàÿ äëèíà ôàéëà
	int		R_Len;			// ðåàëüíàÿ äëèíà ôàéëà
	int		Ptr;			// òåêóùèé óêàçàòåëü äëèíû (ïî áëîêàì)
	int		Ofs;			// ñìåùåíèå â ïðåäåëàõ áëîêà
	int		rc;
//	int		Begin;
//	int		State;
};
// ---------------------------

struct	FullFileInfo : public FileInfo,IDInfo			// : public IDInfo   !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
{
};


struct FileAtrib
{
	char*	Name;
	int		Len;
};
// ------------------------
#define	MaxSizBufOut 65536*2
// ------------------
// Ñòðóêòóðà äëÿ ïîëó÷åíèÿ èíôîðìàöèè îò äåêîäåðà
// Savelog_ = íàèìåíîâàíèå îáúåêòà_
static char	Savelog_NameMt[]= "Savelog";
static char	Savelog_Version[]= "V1.0";
static char	Savelog_Author[]= "San";
static char	Savelog_RemMt[]= "";
static char	*Savelog_PinNameI[]=
{
	"DataIn","",	
	"TimeClick","TimeClick",	
	//"Ctrl","Óïðàâëåíèå",
	""
};
static char	*Savelog_PinNameO[]=
{
	//"Pkt","Èíôîðìàöèÿ î ñåàíñå + Àóäèîäàííûå",
	""
};

static char	*Savelog_Prm[]=
{
	"PathLog",	"",
	"NDay",		"",
	"TimeWr",	"",
	"AppendTime","",
	"TypeTime"	,"",
	"AppendLF",	"",
	"ExtLog",	"",
//	"NamePrm","Rem",
	""
};

static char	*Savelog_Vol[]=
{
	"PathLog",	"C:\\Log",	"",
	"NDay",		"10",		"",
	"TimeWr",	"5",		"",
	"AppendTime","Off",		"",
	"AppendTime","On",		"",
	"AppendLF",	"Off",		"",
	"AppendLF",	"On",		"",
	"AppendLF",	"OnEnd",	"",
	"ExtLog",	"log",		"",
	"ExtLog",	"txt",		"",
	"TypeTime",	"System",	"",
	"TypeTime",	"Local",	"",
	
//	"NamePrm","Vol1","Rem",
//	"NamePrm","Vol2","Rem",
//	"NamePrm2","Vol1","Rem",
//	"NamePrm2","Vol2","Rem",
	""
};
// ----------------------------------------------
class	Savelog : public	Decoder
{
public:
	int		TypeTime,Day;
	int		AppendTime,AppendLF;
	//int		Fl_Stop;//,Fl_Start;
	int		Fl_FindF;
	INT64	Pror;
//	static const	MaxSizBufOut;
	
//	SYSTEMTIME	TTime;
// 	struct	tm TLogBeg,TLogEnd;
//	time_t	beg,end;
//	struct _finddata_t c_file;
//	long	hFile;
	
	char	Path[128];//Path for log-files
	char	PathName[128];//,PathNameOld[128];
	char	NameLog[128];//Name File Log
	char	ExtLog[128];
// 	int		FileLog;//handle for log-file
	
	uchar	NDay;
	int		TimeWr;
//	INT64	TimeWr;
	int		pBufOut;
	char	BufOut[MaxSizBufOut];

	void	WriteFileLog();

		Savelog();
		~Savelog();
virtual	void	Param(			// Èçìåíèòü ïàðàìåòð
			char* Prm,	// Èäåíòèôèêàòîð ïàðàìåòðà
			char* Vol);	// Çíà÷åíèå ïàðàìåòðà
virtual void	Start();		// Íà÷àëî ñåàíñà
virtual void	Stop();			// Êîíåö ñåàíñà
virtual	int		Data(			// Äàííûå äëÿ äåêîäèðîâàíèÿ
				int N_Pin,	// Íîìåð âõîäíîé íîæêè
				UCHAR* pmas,	// Óêàçàòåëü íà áóôåð ñ äàííûìè
				size_t Cnt);	// Äëèíà äàííûõ
virtual	int		IdleFunc(int NumberTr);
//virtual	int	OutData(			// Âûõîäíûå äàííûå
//			int N_Pin,	// Íîìåð âûõîäíîé íîæêè
//			UCHAR* &pmas,	// Óêàçàòåëü íà áóôåð ñ äàííûìè
//			size_t &Cnt);	// Óêàçàòåëü íà äëèíó äàííûõ
//virtual	void	OutLog(char* Buf)
//				{
//					char	tBuf[512];
//				    long	t = time(NULL);
//					sprintf (tBuf,"\r\n%s %s",Buf,ctime( &t ));
//					Data(0,(UCHAR*)tBuf,strlen(tBuf));
//					Data(1,(UCHAR*)&hFile,1);	// TimeClick
//				};
};

//static const	Savelog::MaxSizBufOut=65536;


/*
struct	LocalCfg
{
	char	UniName[80];
	int		Command;
	int		SizeData;
	UCHAR	*Data;
};
*/

/////////////////////////////////////////////////////////////////////////////
/// Bit flags definition for field Fl                                     ///
/////////////////////////////////////////////////////////////////////////////
#define BitDisable ((unsigned char)(0x20))
#define BitHold    ((unsigned char)(0x40))
#define BitRemoved ((unsigned char)(0x80))

/////////////////////////////////////////////////////////////////////////////
/// Bit flags definition for Seans.Tp                                     ///
/////////////////////////////////////////////////////////////////////////////
#define BitWork    ((unsigned char)(0x10))
#define BitCall    ((unsigned char)(0x20))
#define BitAnswer  ((unsigned char)(0x40))
#define BitReserv  ((unsigned char)(0x80))

/////////////////////////////////////////////////////////////////////////////
/// Bit flags definition for Seans.ModeWork                               ///
/////////////////////////////////////////////////////////////////////////////
#define BitDiskWrite ((unsigned char)(0x01))
#define BitDemod     ((unsigned char)(0x02))
#define BitDecod     ((unsigned char)(0x04))
#define BitLog       ((unsigned char)(0x08))


struct	Tsk_BMeth
{
	char	Name[40];
	int	N_PO;
	int	N_PI;
};

struct	Tsk_BParam
{
	char	Name[40];
	int		ID_BMt;
	char	Vol[255];
};

struct	Tsk_BInput
{
	char	Name[40];
	int		ID_BMt;
};

struct	Tsk_BOutput
{
	char	Name[40];
	int		ID_BMt;
};



struct	Tsk_RNet
{
	UCHAR	Fl;
	char	Name[40];
};
struct	Tsk_Address
{
	UCHAR	Fl;
	int		ID_RS;
	int		Azim;
	char	Name[40];
};
struct	Tsk_Connect
{
	UCHAR	Fl;
	int	ID_Un;
//	int	ID_Pl;
	int	ID_Se;
	int	ID_Ad;
				// 0-255 (0.0-1.0) âåðîÿòíîñòü
};

struct	Tsk_Seans
{
	UCHAR	Fl;
	char	Name[40];
	int		ID_RS;
	UCHAR	ModeWork;
	UCHAR	Prgn[240];	// 6 Ìèíóòíûå èíòåðâàëû
	UCHAR	Stat[240];	// 6 Ìèíóòíûå èíòåðâàëû
};

struct Tsk_UnDem
{
	char	Name[40];
	int	Mod;		// Òèï äåìîäóëÿòîðà
	int	Star;		// Âèä ñîçâåçäèÿ
	int	TypeTrnsm;     	// Òèï ïåðåäà÷è  1-íåïð 2-áëî÷í 3-ïàê
	int	Pilot;		//
	char	SynchP[30];	//
	int	Synch;		// Âèä òàêòîâîé ñèíõðîíèçàöèè
	int	SS_Inf;		// Êîëè÷åñòâî èíô áèò
	int	SS_Stop;	// Êîëè÷åñòâî ñòîïîâ
	int	Bl_T;		// ïåðèîä áëî÷íîé ïåðåäà÷è
	int	Bl_D;		// äëèíà èíôîðìàöèîííîé ÷àñòè
//======================
	float  De_Fn;
	float  De_dFn;
	float  De_Fb;
	float  De_dFb;
	float  De_Fr;
	float  De_dFr;
	float  De_Fch;
	float  De_dFch;
	float  De_Fp;
	float  De_dFp;
	int	De_Invert;
	float	De_Kaea;
	float	De_Kaen;
	float	De_Kaee;
	float	De_K1;
	float	De_K2;
	float	De_K21;
	float	De_K22;
	float	De_porog;
	float	De_porog_1;
	float	De_CAgc;
	float	De_CAgc2;
	float	De_CAgcMin;
	float	De_CAgcMax;
	float	De_Enp;
	float	De_Enp1;
	float	De_Enp2;
	float	De_Enp3;
	int	De_Ka1;
	int	De_Ka2;
	float	De_Ke;
	float	De_Km;
	float	De_Ksta;
	float	De_Kstn;

	int	De_PersP;
	float	De_F1;
	float	De_F2;
	float	De_F3;
	float	De_F4;
	float	De_dF;

};

struct	Tsk_Spectr
{
	char	Name[40];
	UCHAR	Spe[128];
	int	Eps;		// ýíåðãèòè÷åñêèé ïîðîã óñåêàíèÿ ñïåêòðà
	int	dEps;		// ïîðîã ÑÊÎ
	int	Kosr;		// îñðåäíåíèå ñïåêòðà
	int	Porog;		// ïîðîã
	int	Ep;		// ýíåðãèòè÷åñêèé ïîðîã ñðàáàòûâàíèÿ òðåâîãè
};

struct	Tsk_Unit
{
	char	Name[40];
//	int	NumKorr;	// 1-îäíî 2-äâóõ ñòîðîííÿÿ 0-íåîïðåäåëåííî
//	int	Connect;	// 1-ðàñïèñàíèå
//				// 2-çîíäèðîâàíèå
//				// 4-çàïðîñ êîîðäèíèð ñòàíöèè
	int	TypeConnect;	// 1-ãîëîñ
				// 2-êëþ÷
				// 4-ìîäóëèðîâàíûé ñèãíàë
				// 8-íåìîäóëèðîâàííûé ñèãíàë
//	int	UDemC;		// äåìîäóëÿòîð âûçîâà

//	int     ReConnect;	// îòâåò
//				// 1-ãîëîñîì
//				// 2-êëþ÷
//				// 3-ìîäóëèðîâàííûé îòâåò
//				// 4-íåìîäóëèðîâàííûé îòâåò
//				// 5-íà÷àëî ñåàíñà

//int	NumDem;		// êîëè÷åñòâî âîçìîæíûõ âèäîâ ìîäóëÿöèè
//int	Dem;		// îæèäàåìûé íîìåð èäåíòèôèêàòîðà âèäà ìîäóëÿöèè
//UniDem	UDemP[10];	// âèä ìîäóëÿöèè
};

struct	Tsk_LinkMode
{
	int	ID_Un;
	int	ID_Md;
};


struct	Tsk_Energy
{
	char	Name[40];
	int	Ep;
};

struct	Tsk_Pool
{
	UCHAR	Fl;
	char	Name[40];
	int	ID_RS;
};
struct	Tsk_Freq
{
	UCHAR	Fl;
	int	ID_RS;
	int	F;
//	UCHAR	Spe[128];
};

struct	Tsk_GroopFr
{
	UCHAR	Fl;
	UCHAR	Tp;
	int	ID_F;
	int	ID_Pl;
};

struct	Tsk_GroopPl
{
	UCHAR	Fl;
	UCHAR	Tp;
	int	ID_TC;
	int	ID_Pl;
};

struct Task : public UDec
{
// Âîçìîæíî ïîòðåáóåòñÿ ââåñòè ìàêñèìàëüíûé ðàçìåð ìàññèâîâ
//
public:
	char			Name[80];
	Tsk_RNet		RS[Num_RNet];
	Tsk_Address		Ad[Num_Address];
	Tsk_Connect		TC[Num_Connect];
	Tsk_Unit		Un[Num_Unit];
	Tsk_UnDem		UD[Num_UDem];
	Tsk_Pool		Pl[Num_Pool];
	Tsk_Freq		Fr[Num_Freq];
	Tsk_Seans		Se[Num_Seans];
	Tsk_LinkMode	MdL[Num_LinkMode];
	Tsk_Energy		En[Num_Energy];
	Tsk_Spectr		Sp[Num_Spectr];
// -----
	Tsk_BMeth		BMt[Num_BMeth];
	Tsk_BParam		BPr[Num_BMeth];
	Tsk_BInput		BIn[Num_BMeth];
	Tsk_BOutput		BOut[Num_BMeth];
//------
	Tsk_GroopFr		GrFr[Num_GroopFr];
	Tsk_GroopPl		GrPl[Num_GroopPl];
/*
	UArray<Tsk_RNet>		RS;//[Num_RNet];
	UArray<Tsk_Address>		Ad;//[Num_Address];
	UArray<Tsk_Connect>		TC;//[Num_Connect];
	UArray<Tsk_Unit>		Un;//[Num_Unit];
	UArray<Tsk_UnDem>		UD;//[Num_UDem];
	UArray<Tsk_Pool>		Pl;//[Num_Pool];
	UArray<Tsk_Freq>		Fr;//[Num_Freq];
	UArray<Tsk_Seans>		Se;//[Num_Seans];
	UArray<Tsk_LinkMode>	MdL;//[Num_LinkMode];
	UArray<Tsk_Energy>		En;//[Num_Energy];
	UArray<Tsk_Spectr>		Sp;//[Num_Spectr];
// -----
	UArray<Tsk_BMeth>		BMt;//[Num_BMeth];
	UArray<Tsk_BParam>		BPr;//[Num_BMeth];
	UArray<Tsk_BInput>		BIn;//[Num_BMeth];
	UArray<Tsk_BOutput>		BOut;//[Num_BMeth];
//------
	UArray<Tsk_GroopFr>		GrFr;//[Num_GroopFr];
	UArray<Tsk_GroopPl>		GrPl;//[Num_GroopPl];
*/
	int		Max_RS;
	int		Max_Ad;
	int		Max_TC;
	int		Max_Un;
	int		Max_UD;
	int		Max_Pl;
	int		Max_Fr;
	int		Max_Se;
	int		Max_MdL;
	int		Max_En;
	int		Max_Sp;
//------
	int		Max_BMt;
	int		Max_BPr;
	int		Max_BIn;
	int		Max_BOut;
	int		Max_GrFr;
	int		Max_GrPl;
};

class CTask
{
public:
Task		*Tsk;
			CTask(Task* Ts){Tsk=Ts;};
	void	DelMd(int ID_Md);		// Óäàëèòü ðåæèì (íîìåð ðåæìà àáñîëþòíûé)
	void	DelMdMt(int ID_Md,int ID_Mt);	// Óäàëèòü â äàííîì ðåæèìå ìåòîä (íîìåð ìåòîäà îòíîñèòåëüíûé) 
	void	DelMt(int ID_Mt);		// Óäàëèòü ìåòîä (íîìåð ìåòîäà àáñîëþòíûé)
	void	DelPr(int ID_Pr);		//
	void	DelLP(int ID_LP);		//
	void	CopyMd(int ID_Md1,int ID_Md2);
	void	CopyMt(int ID_Mt1,int ID_Mt2);
	void	CopyLP(int ID_LP1,int ID_LP2);
	void	CopyPr(int ID_Pr1,int ID_Pr2);
};



class AnalizStrToInt
{
protected:
	int	SizArr;	
public:
	int *ArrInt;
	int	Cnt;

	AnalizStrToInt()
	{
		SizArr=32;
		Cnt=0;ArrInt=new int[SizArr];
	};
	~AnalizStrToInt()
	{
		delete[]ArrInt;
	}
	int Do(char* Str) //Ôóíêöèÿ êîíâåðòèò ñòðîêó âèäà "2,4-16,18-31" â ìàñèâ ÷èñåë ArrInt êîë-ì Cnt
	{
		PCHAR Zap=NULL,Minusi=NULL,Pred=Str;
		int PrVol=0,Razn=0;
		int Fl=0;int ii;int *CArrInt;
		char *LStr=new char[(strlen(Str)/4+2)*4];Cnt=0;
		Zap=strchr(Pred,',');Minusi=strchr(Pred,'-');
		while ((Zap!=NULL) || (Minusi!=NULL))
		{
			if (Fl==1)
			{
				Fl=0;Razn=0;
				if ((Zap!=Pred) && ((Zap>0) && ((Zap<Minusi) || (Minusi==NULL))))
				{
					strncpy(LStr,Pred,Zap-Pred);LStr[Zap-Pred]=0;Pred=Zap;
				}
				else if ((Minusi!=Pred) && ((Minusi>0) && ((Zap>Minusi) || (Zap==NULL))))
				{
					strncpy(LStr,Pred,Minusi-Pred);LStr[Minusi-Pred]=0;Pred=Minusi; 
				};
				PrVol=atoi(LStr);
				Razn=PrVol-ArrInt[Cnt-1];
				if (Razn>0) 
				{
					if (SizArr<(Cnt+Razn+1))
					{CArrInt=new int[SizArr];memcpy(CArrInt,ArrInt,Cnt*sizeof(int));delete[]ArrInt;SizArr+=((Cnt+Razn+1)/32+1)*32;ArrInt=new int[SizArr];memcpy(ArrInt,CArrInt,Cnt*sizeof(int));delete[]CArrInt;};
					for(ii=Cnt;ii<Cnt+Razn;ii++) ArrInt[ii]=ArrInt[ii-1]+1;
					Cnt+=Razn;
				}
			}
			else if ((Zap!=Pred) && ((Zap>0) && ((Zap<Minusi) || (Minusi==NULL))))
			{
				strncpy(LStr,Pred,Zap-Pred);LStr[Zap-Pred]=0;
				Pred=Zap;
				if (SizArr<(Cnt+1)){CArrInt=new int[SizArr];memcpy(CArrInt,ArrInt,Cnt*sizeof(int));delete[]ArrInt;SizArr+=32;ArrInt=new int[SizArr];memcpy(ArrInt,CArrInt,Cnt*sizeof(int));delete[]CArrInt;};
				ArrInt[Cnt++]=atoi(LStr);				
			}
			else if ((Minusi!=Pred) && ((Minusi>0) && ((Zap>Minusi) || (Zap==NULL))))
			{
				strncpy(LStr,Pred,Minusi-Pred);LStr[Minusi-Pred]=0;
				Pred=Minusi;
				if (SizArr<(Cnt+1)){CArrInt=new int[SizArr];memcpy(CArrInt,ArrInt,Cnt*sizeof(int));delete[]ArrInt;SizArr+=32;ArrInt=new int[SizArr];memcpy(ArrInt,CArrInt,Cnt*sizeof(int));delete[]CArrInt;};
				ArrInt[Cnt++]=atoi(LStr);
				Fl=1;
			}
			Pred++;
			Zap=strchr(Pred,',');Minusi=strchr(Pred,'-');
		}
		strcpy(LStr,Pred);
		if (LStr[0]!=0)
		{
			if (Fl==1)
			{
				Razn=0;
				PrVol=atoi(LStr);		
				Razn=PrVol-ArrInt[Cnt-1];
				if (Razn>0) 
				{
					if (SizArr<(Cnt+Razn+1)){CArrInt=new int[SizArr];memcpy(CArrInt,ArrInt,Cnt*sizeof(int));delete[]ArrInt;SizArr+=((Cnt+Razn+1)/32+1)*32;ArrInt=new int[SizArr];memcpy(ArrInt,CArrInt,Cnt*sizeof(int));delete[]CArrInt;};
					for(ii=Cnt;ii<Cnt+Razn;ii++) ArrInt[ii]=ArrInt[ii-1]+1;
					Cnt+=Razn;
				}
			}
			else 
			{
				if (SizArr<(Cnt+1)){CArrInt=new int[SizArr];memcpy(CArrInt,ArrInt,Cnt*sizeof(int));delete[]ArrInt;SizArr+=32;ArrInt=new int[SizArr];memcpy(ArrInt,CArrInt,Cnt*sizeof(int));delete[]CArrInt;};
				ArrInt[Cnt++]=atoi(LStr);
			}
		};
		delete[]LStr;
		return Cnt;
	}
};

class	LIFO
{
public:
		void*		*DelPool;
		int			NumDelPool;
		int			SizeDelPool;
		int			NumIDInfo;

		LIFO(int Num)
		{
			NumIDInfo=Num;
			SizeDelPool=NumIDInfo;
			DelPool=(void**)malloc(SizeDelPool*sizeof(void*));
			NumDelPool=0;
		};

virtual	void*	GetElem(int Prm=NULL)
		{
			if (NumDelPool!=0)
				return	DelPool[--NumDelPool];
			else
				return NewElem(); 
		};

virtual	void	SaveElem(void* IDE)
		{
			DelPool[NumDelPool++]=IDE;
			if (NumDelPool>=SizeDelPool)
			{
				SizeDelPool+=NumIDInfo;
				DelPool=(void**)realloc(DelPool,SizeDelPool*sizeof(void*));
			}
		};
virtual	void*	NewElem(int Prm=NULL)
{
	return (void*) new(void*);
}

virtual	~LIFO()
		{
			int	i;
			if (DelPool!=NULL)
			{
				for (i=0;i<NumDelPool;i++)
				{
					delete DelPool[i];
				}
				//
				free(DelPool);DelPool=NULL;
			}
		}
};

#define		Kmemz	2
// -------------------

void IDI2FileName (IDInfo* IDI,char* Path,char* Name);

template<class T> class I64El:public ID_Elem
{
private:
	INT64	VolI64;//Óíèê.Çíà÷åíèå
	T   *pPar;

public:	
	I64El():ID_Elem((UCHAR*)&VolI64,8)	
	{VolI64=0;pPar=0;}
	I64El(INT64 lVolI64,T* lPar=0):ID_Elem((UCHAR*)&VolI64,8)	
	{VolI64=lVolI64;pPar=lPar;}

	//ïåðåêðûòèå =
//	I64El& operator=(int NewVol,T* lPar=0)
//	{VolI64=NewVol;pPar=lPar;return *this;};
	I64El& operator=(INT64 NewVol)
	{VolI64=NewVol;return *this;};
//	T*	operator=(I64El* Fnd) 	
//	{return (Fnd)?(Fnd->pPar):0;}
	
	
	INT64 GetVol()
	{return VolI64;};
	void SetPoiParent(T *lPoi)
	{pPar=lPoi;}
	T* GetPoiParent()
	{return pPar;}
};

template<class T> class I32El:public ID_Elem
{
private:
	int	VolI;//Óíèê.Çíà÷åíèå
	T   *pPar;
	
public:	
	I32El():ID_Elem((UCHAR*)&VolI,4)	
	{VolI=0;pPar=0;}
	I32El(int lVolI,T* lPar=0):ID_Elem((UCHAR*)&VolI,4)	
	{VolI=lVolI;pPar=lPar;}
	
	I32El&	operator=(int NewVol) 	{VolI=NewVol;return *this;}

//	T*	operator=(I32El* Fnd) 	
//	{return (Fnd)?(Fnd->pPar):0;}
	
	int		GetVol()	{return VolI;};
	void	SetPoiParent(T *lPoi)	{pPar=lPoi;}
	T*		GetPoiParent()			{return pPar;}
};

template<class T> class StrEl:public ID_Elem
{
private:
//	char	VolI[];//Óíèê.Çíà÷åíèå
//	int m_strLen;
	T   *pPar;
	
public:	
	StrEl():ID_Elem()	
	{
		Len=0;pPar=0;
	}
	StrEl(char* lVol,T* lPar=0):ID_Elem()	
	{
		Len=strlen(lVol)+1;
		if (Len>1)
		{Buff=new uchar[Len];strcpy((char*)Buff,lVol);}
		else {Len=0;Buff=0;}
		pPar=lPar;
	}
	~StrEl()
	{
		if (Buff)
		 delete []Buff;		
	}
	
	StrEl&	operator=(char *NewVol)
	{
		size_t lLen=strlen(NewVol)+1;
		if ((lLen>1) && (Len>0))
		{
			if (lLen!=Len)
			{Len=lLen;delete []Buff;Buff=new uchar[Len];}
			strcpy((char*)Buff,NewVol);		
		}
		else 
		{ 
			if (lLen>1) 
			{Len=lLen;Buff=new uchar[Len];strcpy((char*)Buff,NewVol);}
			else if ((lLen==1) && (Len>0))
			{Len=0;delete []Buff;Buff=0;}
		}		
		return *this;
	}

	char*	GetVol()	
	{return (char*)Buff;};
	void	SetPoiParent(T *lPoi)	{pPar=lPoi;}
	T*		GetPoiParent()			{return pPar;}
};

template <class TT> class iListI64:public ID_List8
{
public:
	iListI64():ID_List8()
	{
		NumElem=0;tPElem=0;Level=0;FlBeg=0;FlEnd=0;
		for (int i=0;i<256;i++){Lev[i]=NULL;ElemM[i].Elem=NULL;}
//		for (int i=0;i<256;i++){Lev[i]=NULL;Elem[i]=NULL;}
	};
	
	TT* operator [](INT64 Position)
	{
		I64El<TT>* pEl=(I64El<TT>*)Find((uchar*)&Position,8);
		return (pEl)?pEl->GetPoiParent():0;
	}

	int operator +(I64El<TT>* pAddEl)
	{return Add(pAddEl);}
	int operator -(I64El<TT>* pRemoveEl)
	{return Remove(pRemoveEl);}
	
	TT*	TBegin(FindStrList8& FSL8)
	{
		I64El<TT>* pEl=(I64El<TT>*)Begin(FSL8);
		return (pEl)?pEl->GetPoiParent():0;
	};	
	TT*	TNext(FindStrList8& FSL8)
	{
		I64El<TT>* pEl=(I64El<TT>*)Next(FSL8);
		return (pEl)?pEl->GetPoiParent():0;
	};

	I64El<TT>*	iBegin(FindStrList8& FSL8)
	{return(I64El<TT>*)Begin(FSL8);};	
	I64El<TT>*	iNext(FindStrList8& FSL8)
	{return (I64El<TT>*)Next(FSL8);};

	virtual void TDelAllElem()
	{
		FindStrList8	FSL8;
		I64El<TT>* pElem=iBegin(FSL8);
		while (pElem)
		{
			Remove(pElem);
			delete pElem->GetPoiParent();
			pElem=iNext(FSL8);
			if (!pElem)pElem=iBegin(FSL8);
		}
	}
};

template <class TT> class iListI32:public ID_List8
{
public:
	iListI32():ID_List8()
	{
		NumElem=0;tPElem=0;Level=0;FlBeg=0;FlEnd=0;
		for (int i=0;i<256;i++){Lev[i]=NULL;ElemM[i].Elem=NULL;}
//		for (int i=0;i<256;i++){Lev[i]=NULL;Elem[i]=NULL;}
	};
	
	TT* operator [](int Position)
	{
		I32El<TT>* pEl=(I32El<TT>*)Find((uchar*)&Position,4);
		return (pEl)?pEl->GetPoiParent():0;
	}
	int operator +(I32El<TT>* pAddEl)
	{return Add(pAddEl);}
	int operator -(I32El<TT>* pRemoveEl)
	{return Remove(pRemoveEl);}

	TT*	TBegin(FindStrList8& FSL8)
	{
		I32El<TT>* pEl=(I32El<TT>*)Begin(FSL8);
		return (pEl)?pEl->GetPoiParent():0;
	};
	TT*	TNext(FindStrList8& FSL8)
	{
		I32El<TT>* pEl=(I32El<TT>*)Next(FSL8);
		return (pEl)?pEl->GetPoiParent():0;
	};

	I32El<TT>*	iBegin(FindStrList8	*FSL8)
	{return(I32El<TT>*)Begin(FSL8);};	
	I32El<TT>*	iNext()
	{return (I32El<TT>*)Next(FSL8);};
};

template <class TT> class iListStr:public ID_List8
{

public:
	iListStr():ID_List8()
	{
		NumElem=0;tPElem=0;Level=0;FlBeg=0;FlEnd=0;
		for (int i=0;i<256;i++){Lev[i]=NULL;ElemM[i].Elem=NULL;}
//		for (int i=0;i<256;i++){Lev[i]=NULL;Elem[i]=NULL;}
	};
	
	TT* operator [](char *Position)
	{
		StrEl<TT>* pEl=(StrEl<TT>*)Find((uchar*)Position,strlen(Position)+1);
		return (pEl)?pEl->GetPoiParent():0;
	};

	int operator +(StrEl<TT>* pAddEl)
	{return Add(pAddEl);}
	int operator -(StrEl<TT>* pRemoveEl)
	{return Remove(pRemoveEl);}

	
	TT*	TBegin(FindStrList8& FSL8)
	{
		StrEl<TT>* pEl=(StrEl<TT>*)Begin(FSL8);
		return (pEl)?pEl->GetPoiParent():0;
	};
	TT*	TNext(FindStrList8& FSL8)
	{
		StrEl<TT>* pEl=(StrEl<TT>*)Next(FSL8);
		return (pEl)?pEl->GetPoiParent():0;
	};

	StrEl<TT>*	iBegin()
	{return(StrEl<TT>*)Begin(FSL8);};	
	StrEl<TT>*	iNext()
	{return (StrEl<TT>*)Next(FSL8);};

};

class El_TabList :	public ID_Elem
{
public:
	IDTable*	IDT;
	BandleList	BL;
				El_TabList(INT64 ID,Decoder* pDec);
				~El_TabList();
virtual void	AddLink(Decoder* ParMt);
virtual void	DelLink(Decoder* ParMt);
};

class	El_Tab :	public ID_Elem
{
	Services*	Srv;
	Decoder*	pMt;
// ------------------------

//	UCHAR*		TextBuf;
//	int			CntTB;
	IDInfo*		IDI;
public:
	ID_List8*	IDL64;
	El_Tab*		pParent;
	int			Fl_LockWR;
	ID_List8*	OwnerList;
	StrOwner*	LastOwner;
	char		LastOwnerName[64];
	IDTable*	IDTab;
	int			Fl_NeedWR;
	int			Fl_Del;
	int			Fl_Query;
	int			Fl_Test;
	FileTime	QueryTime;
	INT64		ID;
	FlagStruct	Fl_RE;
				El_Tab(StrOwner*	Owner,INT64 tID,IDTable*	IDT);
				~El_Tab();
//virtual	void	AddText(UCHAR* Txt);
virtual	int		AddChild(INT64 ID,int Flag=0);
virtual	void	DelChild(INT64 ID);
virtual	void	AddOwner(StrOwner*	Owner);
virtual	int		DelOwner(StrOwner*	Owner);	// RC!=-1
virtual	void	Init(StrOwner*	Owner,INT64 tID,IDTable*	IDT);
virtual	bool	TestModif(FileTime *fTime);
virtual	int		GetNumChild();
virtual	IDInfo*	GetIDI()
				{
					return IDI;
				};
virtual	IDInfo*	GetIDI(StrOwner*	Owner)
				{
					TestOwner(Owner);
					return IDI;
				};
virtual	bool	TestOwner(StrOwner*	Owner);
virtual	int		PreSave();

};

class	El_Prm :	public ID_Elem
{
public:
uchar			VolPrm[80];	
int				LenPrm;		
El_Tab			*ElTbl;
				El_Prm(El_Tab*	El,uchar *pVolPrm,int PLen): ID_Elem((UCHAR*)VolPrm,PLen)
				{
					LenPrm=PLen;
					memcpy(VolPrm,pVolPrm,PLen);
					ElTbl=El;
				};
virtual	void	Init(El_Tab*	El,uchar *pVolPrm,int PLen)
				{
					LenPrm=PLen;
					memcpy(VolPrm,pVolPrm,PLen);
					ElTbl=El;
				}
};

/*
class	El_Key	:	public ID_Elem
{
public:
	El_Tab*		ElT;
				El_Key (El_Tab* pElT);
};
*/
// Ñòðóêòóðà äëÿ ïîëó÷åíèÿ èíôîðìàöèè îò äåêîäåðà
// IDTable_ = íàèìåíîâàíèå îáúåêòà_
static char	IDTable_NameMt[]= "IDTable";
static char	IDTable_Version[]= "V1.0";
static char	IDTable_Author[]= "SystemSoft (Y)";
static char	IDTable_RemMt[]= "TableManager";
static char	*IDTable_PinNameI[]=
{
	"IDInfoFS","",
	"TimeClick","Time Click",
	"IDInfoRepl","",
	""
};
static char	*IDTable_PinNameO[]=
{
	"IDInfoOut","",
	"ExtTimeClick","External Time Click",
	"IDInfoOutUser","",
	"IDInfoOutRepl","",
	""
};

static char	*IDTable_Prm[]=
{
//	"Table_ID","Èäåíòèôèêàòîð òàáëèöû",
	""
};

static char	*IDTable_Vol[]=
{
//	"Table_ID","0","Íåîïðåäåëåíî",
	""
};
// ----------------------------------------------

#define			SizeChildBuf	256
#define			AcsMd_Read		0
#define			AcsMd_Write		1

class IDTable	:	public Decoder
{
protected:
//El_Tab*			tElTbl;	// Âðåìåííûé ýëåìåíò
int				Fl_Stop;

//Decoder*		Mt;				// Ìåòîä ñîçäàòåëü òàáëèöû
//IDInfo*			QIDI;		// Äëÿ ôîìèðîâàíèÿ çàïðîñà ê ôàéëîâîé ñèñòåìå (âðåìåííûé)
//INT64			QID;			// Ïàðàìåðò çàïðîñà ê ÁÄ - èäåíòèôèêàòîð îáúåêòà
El_Tab*			ElTbl;			// Ýëåìåíò ñîäåðæàùèé ïàðàìåòðû ñàìîé òàáëèöû (êîðíåâîé ýëåìåíò)
El_Tab*			ElTblDel;		// Ýëåìåíò ñîäåðæàùèé ïàðàìåòðû ñàìîé òàáëèöû (êîðíåâîé ýëåìåíò)
El_Tab*			ElTblForNew;	// Ýëåìåíò ñîäåðæàùèé ïàðàìåòðû ñàìîé òàáëèöû (êîðíåâîé ýëåìåíò)
int				Fl_Long;
// -------------------
INT64			Child[SizeChildBuf+16];
//INT64			tttt;
//int				CntChild;
int				Fl_Ready;
int				CntActQuery;
// -------------------
char			LogBuf[256];
BLDelElem		BLDel;		// 
int				BDL_Cnt;
ID_List8*		IDL_Req;	// Ñïèñîê óæå çàïðîøåíûõ ýëåìåíòîâ
INT64			Scan_Index;
INT64			DelTimer;
char			LastHost[256];
int				FlagRE;
//FlagStruct		Fl_RE;
//FlagStruct		Fl_RE4;		//
INT64			IDTHnd;
//FlagStruct		Fl_Th;
StrOwner		TmpAnchor;
public:
ID_List8*		IDL_Query;	// Ñïèñîê óæå çàïðîøåíûõ ýëåìåíòîâ
ID_List8*		IDL;	// Ñïèñîê âñå ýëåìåíòîâ (êîòîðûå çàãðóçèëà ñèñòåìà) ñ ñîðòèðîâîé ïî èäåíòèôèêàòîðó
int				Fl_Debug;
INT64			ID;		// Èäåíòèôèêàòîð òàáëèöû
ListID_FiFo*	IDL_Test;	// Ñïèñîê óæå çàïðîøåíûõ ýëåìåíòîâ
int				Fl_RR;
Decoder*		pFSrv;
int				CustomerID;
FindStrList8	FSL_8;
AddressInfo		AdrI;
int				Fl_AntyRec;
FindStruct		FStr_Root;
char			TableName[64];
				IDTable();
				IDTable(Decoder* Mt,INT64 tID);
				~IDTable();
protected:
virtual	void	Param(			// Èçìåíèòü ïàðàìåòð
				char* Prm,		// Èäåíòèôèêàòîð ïàðàìåòðà
				char* Vol);		// Çíà÷åíèå ïàðàìåòðà
virtual void	Start();		// Íà÷àëî ñåàíñà
virtual void	Stop();			// Êîíåö ñåàíñà
virtual	int		DataFF(			// Äàííûå äëÿ äåêîäèðîâàíèÿ
				int N_Pin,		// Íîìåð âõîäíîé íîæêè
				UCHAR* pmas,	// Óêàçàòåëü íà áóôåð ñ äàííûìè
				size_t Cnt);		// Äëèíà äàííûõ
virtual int		DataIDI(int N_Pin, IDInfo* pIDI);
// ---System Funtion------------------
protected:
El_Tab*			AddElTab(StrOwner* Owner,El_Tab* ElTParent,INT64 tID);	// ElTParent = NULL main Table Elem
//int				DelElTabStep	(StrOwner* Owner, El_Tab* ElT,int Lev,int Fl_NoR=0);				// Óäàëèòü åëåìåíò â òàáëèöå
int				AddIDI(IDInfo* IDITr);
int				CopyIDI2El(IDInfo* IDITr,El_Tab* tElT,int NoRepl);
El_Tab*			CopyIDI2Parent(StrOwner* Owner,IDInfo* IDITr,El_Tab* ElTParent);
El_Tab*			CopyIDI2ParentC(StrOwner* Owner,IDInfo* IDITr,El_Tab* ElTParent);
El_Tab*			LoadEl	(StrOwner* Owner,INT64 tID);	// Çàãðóçèòü äàííûå ñ ôàéë-ñåðâåðà
int				SendEl(El_Tab* ElT,int Fl_AddChild=0);	// Ïîñëàòü èçìåíåíèÿ â ïîëüçîâàòåëüñêîå ïðèëîæåíèå
int				TestEl(El_Tab* ElT);	// Ïðîâåðèòü íà äóáëèêàòû
//int				TestElI(INT64 ID);	// Ïðîâåðèòü íà äóáëèêàòû
public:
int				QueryEl(INT64 tID);		// Çàïðîñèòü åëåìåíò ïî ID ó Âñåõ (÷åðåç ðåïëèêàöèþ)
protected:
int				QueryElI(INT64 tID);	// Çàïðîñèòü åëåìåíò ïî ID ó Âñåõ (÷åðåç ðåïëèêàöèþ)
int				UpDataEl(El_Tab* ElT,int Fl_Mod=0);	// Ñîõðàíèòü ýëåìåíò íà ÔÑ ñ èçìåíåíèåì âðåìåíè (ñîîáùèòü î ìîäèôèêàöèè)
int				UpDataEl2(El_Tab* ElT);	// Ñîõðàíèòü ýëåìåíò íà ÔÑ ñ èçìåíåíèåì âðåìåíè (ñîîáùèòü î ìîäèôèêàöèè)
int				ReplEl(El_Tab* ElT);	// Ïîñëàòü âñåì Õîñòàì (íà ðåïëèêàöþ)
int				ReplElI(INT64 tID);	// Ïîñëàòü âñåì Õîñòàì (íà ðåïëèêàöþ)
//int				RepairEl(El_Tab* ElT);	// Ïîñëàòü âñåì Õîñòàì (íà ðåïëèêàöþ)
//int				DelEl(El_Tab* ElT,int Fl_NoR=0);		// Óäàëèòü åëåìåíò
int				AddParamI	(StrOwner* Owner,El_Tab* ETbl,INT64 IDPrm,char* Vol,int FlNoModif=0);
//El_Tab*			AttainNestedI(StrOwner* Owner,char *mFormat,char* FirstClass,...);//Ïî ââåäåííîé öåïî÷êå ïàðàìåòðîâ â çàäàííîì ôîðìàòå âûäàåò ýë-ò 
El_Tab*			AddETI		(StrOwner* Owner, char* Vol, El_Tab* ElTParent=NULL);	// Ñîçäàòü çàïèñü â òàáëèöå
El_Tab*			ModifElTabI	(StrOwner* Owner, El_Tab* ElT,int Fl_Mod=0);				// Ìîäèôèöèðîâàòü ïàðàìåòðû ýëåìåíòà òàáëèöû (Ýëåìåíò äîëæåí áûòü îòêðûò â ðåæèìå çàïèñè)
void			CreatePrmList	(StrOwner* Owner, FindStruct* FndStr,El_Tab* tEl);
El_Tab*			CreateNewEl	();	// Used DelList
El_Tab*			NewElTab	(StrOwner* Owner, char* tVol, El_Tab* ElTParent=NULL);	// Ñîçäàòü çàïèñü â òàáëèöå
El_Tab*			NewElTab	(StrOwner* Owner, El_Tab* ElTParent=NULL);				// Ñîçäàòü çàïèñü â òàáëèöå
El_Tab*			NewElTab	(StrOwner* Owner, INT64 tID, El_Tab* ElTParent=NULL);	// Ñîçäàòü çàïèñü â òàáëèöå
int				DelFromQueryList(INT64	ID);
// ---User Funtion--------------------
bool			TestValid(El_Tab* ElT,El_Tab* ParentElT);
bool			Combine(INT64 ID1,INT64 ID2);
bool			Combine(El_Tab* &ETbl1,El_Tab* &ETbl2,int Fl_NoRepl=0);
//int				ReplElCombine(El_Tab* ElT,INT64 ID2);
bool			TestNewest(IDInfo*,IDInfo*);
bool			CompareChild(IDInfo*,El_Tab*);	// true if it was detect difference
public:
El_Tab*			GetRoot(){return ElTbl;};
void			SetZRoot(){ElTbl=NULL;ElTblForNew=ElTblDel=NULL;};
int				GetNumQueryList()
				{
				if (IDL_Query)
					return IDL_Query->NumElem;
				return 0;
				};
int				GetNumAllList()
 				{
 					return IDL_Query->NumElem+IDL_Test->NumElem;
 				};
int				SaveEl(El_Tab* ElT);	// Ñîõðàíèòü ýëåìåíò íà ÔÑ
// -----------------------------------
// Äëÿ âñåõ åëåìåíòîâ îòêðûòûõ íà ÷òåíèå â ñëó÷àå ìîäèôèêàöèè åëåìåíòîâ äðóãèìè îáúåêòàìè âûçûâàåòñÿ ODataFF c IDInfo ìîäèôèöèðîâàííîãî åëåìåíòà 
// -----------------------------------
El_Tab*			AddET		(StrOwner* Owner, char* Vol, El_Tab* ElTParent=NULL);	// Ñîçäàòü çàïèñü â òàáëèöå
El_Tab*			AddET		(StrOwner* Owner, El_Tab* ElTParent=NULL);				// Ñîçäàòü çàïèñü â òàáëèöå
El_Tab*			AddET		(StrOwner* Owner, INT64 Vol, El_Tab* ElTParent=NULL);	// Ñîçäàòü çàïèñü â òàáëèöå
El_Tab*			AttainNested(StrOwner* Owner,char *mFormat,char* FirstClass,...);//Ïî ââåäåííîé öåïî÷êå ïàðàìåòðîâ â çàäàííîì ôîðìàòå âûäàåò ýë-ò
	//mFormat-óêàçûâàåò òèïû öåïî÷êè ïàðàìåòðîâ:%d-INT64,%s-char* ;FirstClass-èìÿ áàçîâîãî êëàññà; -1 - êîíåö öåïî÷êè 
	//åñëè êàêîãî-òî åëåìåíòà(èç öåïî÷êè) íåò,òî îí ñîçäàåòñÿ
	//Ex:AttainNested("%d,%s,%d",StreamList,2000,LogChannel,12,-1); - âûäàåò ýë-ò ïî öåïî÷êå StreamList->2000->LogChannel->12
int				LockEl		(StrOwner* Owner, El_Tab* ElT);	// -1 íåóäà÷à
protected:
int				MoLockEl	(StrOwner* Owner, El_Tab* ElT,int AcsMode);	// -1 íåóäà÷à
public:
int				UnLockEl	(StrOwner* Owner, El_Tab* ElT,int Fl_NoOTime=0);				// -1 íåóäà÷à
int				UnLockAllEl	(StrOwner* Owner);							// -1 íåóäà÷à
El_Tab*			GetElTab	(StrOwner* Owner, INT64 tID,int NoRepl=0);	// Çàãðóçèòü è äàòü óêàçàòåëü íà åëåìåíò òàáëèöû. Åñëè ýòî åëåìåíò íèæíåãî óðîâíÿ òî çàãðóæàþòñÿ âñå âûøåñòîÿùèå ýëåìåíòû 
El_Tab*			ModifElTab	(StrOwner* Owner, El_Tab* ElT,int Fl_Mod=0);				// Ìîäèôèöèðîâàòü ïàðàìåòðû ýëåìåíòà òàáëèöû (Ýëåìåíò äîëæåí áûòü îòêðûò â ðåæèìå çàïèñè)
int				DelElTab	(StrOwner* Owner, El_Tab* ElT,int Fl_NoR=0);				// Óäàëèòü åëåìåíò â òàáëèöå
//int				DelElTabRO	(StrOwner* Owner, El_Tab* ElT,int Fl_NoR=0);				// Óäàëèòü åëåìåíò â òàáëèöå
//int				DelElTabWDL	(StrOwner* Owner, El_Tab* ElT);				// Óäàëèòü åëåìåíò â òàáëèöå
// Ïîèñêîâûå ôóíêöèè
El_Tab*			FindFirstET	(StrOwner* Owner, FindStruct* FndStr, int AcsMode, El_Tab* ElTParent=NULL,int fl_repl=0);	// Íàéòè ïåðâûé ýëåìåíò â òàáëèöå 
El_Tab*			FindNextET	(StrOwner* Owner, FindStruct* FndStr, int AcsMode, El_Tab* ElTParent=NULL,int fl_repl=0);	// Íàéòè ñëåäóþùèé ýëåìåíò â òàáëèöå  
El_Tab*			FindPervET	(StrOwner* Owner, FindStruct* FndStr, int AcsMode, El_Tab* ElTParent=NULL);	// Íàéòè ïðåäûäóùèé ýëåìåíò â òàáëèöå  
El_Tab*			FindLastET	(StrOwner* Owner, FindStruct* FndStr, int AcsMode, El_Tab* ElTParent=NULL);	// Íàéòè ïåðâûé ýëåìåíò â òàáëèöå 
El_Tab*			FindNested	(StrOwner* Owner, int AcsMode, char* FirstClass,...);
El_Tab*			FindNestedStr	(StrOwner* Owner, int AcsMode, char* FirstClass, El_Tab* ElTParent=NULL);
El_Tab*			Find		(StrOwner* Owner, int AcsMode, char* Str, El_Tab* ElTParent);		// Íàéòè ïåðâûé ýëåìåíò â òàáëèöå 
El_Tab*			Find		(StrOwner* Owner, int AcsMode, INT64 ID, El_Tab* ElTParent);		// Íàéòè ïåðâûé ýëåìåíò â òàáëèöå 
El_Tab*			FindParallel(StrOwner* Owner,  int NumStepUp, int NumStepDown, El_Tab* ElBase,char* ElParName);		// Íàéòè ïåðâûé ýëåìåíò â òàáëèöå 
El_Tab*			FindProfile	(StrOwner* Owner, El_Tab* ETbl,INT64 IDNameProFile);	// Íàéòè áëèæàéøèé ïðîôèëü IDNameProFile=IDProfile,IDProfileFilter,IDProfileCS,IDProfileSchema
El_Tab*			GetElParent (int NumStep, El_Tab* ElBase);
El_Tab*			FindUnLockET	(StrOwner* Owner, FindStruct* FndStr, int AcsMode, El_Tab* ElTParent);
// Ðàáîòà ñ ïàðàìåòðàìè åëåìåíòîâ
Parametr*		FindFirstPrm	(StrOwner* Owner, FindStruct* FndStr, El_Tab* ElTParent, void* &Vol,size_t &tLen);
Parametr*		FindNextPrm		(StrOwner* Owner, FindStruct* FndStr, El_Tab* ElTParent, void* &Vol,size_t &tLen);
int				DelParam	(StrOwner* Owner,El_Tab* ETbl,INT64 IDPrm,int FlNoModif=0);
int				AddParam	(StrOwner* Owner,El_Tab* ETbl,INT64 IDPrm,char* Vol,int FlNoModif=0);
int				AddParam	(StrOwner* Owner,El_Tab* ETbl,INT64 IDPrm,int Vol,int FlNoModif=0);
int				AddParam	(StrOwner* Owner,El_Tab* ETbl,INT64 IDPrm,INT64 Vol,int FlNoModif=0);
int				AddParam	(StrOwner* Owner,El_Tab* ETbl,INT64 IDPrm,float Vol,int FlNoModif=0);
int				AddParam	(StrOwner* Owner,El_Tab* ETbl,INT64 IDPrm,double Vol,int FlNoModif=0);
//int				AddParam	(StrOwner* Owner,El_Tab* ETbl,INT64 IDPrm,char Vol,int FlNoModif=0);
int				AddParam	(StrOwner* Owner,El_Tab* ETbl,INT64 IDPrm,void* Vol,int Size,int FlNoModif=0);

//int				GetParam	(StrOwner* Owner,El_Tab* ETbl,INT64 IDPrm,char &Vol);
int				GetParam	(StrOwner* Owner,El_Tab* ETbl,INT64 IDPrm,char* &Vol);
int				GetParam	(StrOwner* Owner,El_Tab* ETbl,INT64 IDPrm,int* &Vol);
int				GetParam	(StrOwner* Owner,El_Tab* ETbl,INT64 IDPrm,INT64* &Vol);
int				GetParam	(StrOwner* Owner,El_Tab* ETbl,INT64 IDPrm,float* &Vol);
int				GetParam	(StrOwner* Owner,El_Tab* ETbl,INT64 IDPrm,double* &Vol);
int				GetParam	(StrOwner* Owner,El_Tab* ETbl,INT64 IDPrm,void* &Vol);
//-------------
/*
VolPrm*			GetParam	(StrOwner* Owner,El_Tab* ETbl,INT64 IDPrm,char* &Vol);
VolPrm*			GetParam	(StrOwner* Owner,El_Tab* ETbl,INT64 IDPrm,int* &Vol);
VolPrm*			GetParam	(StrOwner* Owner,El_Tab* ETbl,INT64 IDPrm,INT64* &Vol);
VolPrm*			GetParam	(StrOwner* Owner,El_Tab* ETbl,INT64 IDPrm,float* &Vol);
VolPrm*			GetParam	(StrOwner* Owner,El_Tab* ETbl,INT64 IDPrm,double* &Vol);
VolPrm*			GetParam	(StrOwner* Owner,El_Tab* ETbl,INT64 IDPrm,void* &Vol);
*/// Ôóíêöèè ïåðåìåùåíèÿ è êîïèðîâàíèÿ
El_Tab*			MoveET		(StrOwner* Owner, El_Tab* ETbl,El_Tab* ETblDest,int FlNoModif=0);	// Ïåðåìåñòèòü ýëåìåíò â åëåìåíò íàçíà÷åíèÿ
int				CopyIDI2Data(IDInfo* IDITr,El_Tab* tElT,int NoRepl);	//WSA
El_Tab*			CopyET		(StrOwner* Owner, El_Tab* ETbl,El_Tab* ETblDest,int FlNoModif=0);	// WSA
bool			TestOwnEl	(StrOwner* Owner,El_Tab* ETbl);
int				IdleFunc(int NumberTr);
void			DelAllTables();

protected:
El_Tab*			MoveETInternal		(StrOwner* Owner, El_Tab* ETbl,El_Tab* ETblDest);	// Ïåðåìåñòèòü ýëåìåíò â åëåìåíò íàçíà÷åíèÿ

//CRITICAL_SECTION lpCSection; 
// El_Tab==NULL êîãäà íåò èñêîìîãî èëè äîñòèãíóò ïðåäåë ïîñêà

// -----------------------------------
};


const ushort iArrAnd[]=
{
	0x00,0x80,0xc0,0xe0,0xf0,0xf8,0xfc,0xfe,0xff
};
const uint iArrAnd2[]=
{
	0x00000000,0x00000001,0x00000003,0x00000007,0x0000000f,
	0x0000001f,0x0000003f,0x0000007f,0x000000ff,
	0x000001ff,0x000003ff,0x000007ff,0x00000fff,
	0x00001fff,0x00003fff,0x00007fff,0x0000ffff,
	0x0001ffff,0x0003ffff,0x0007ffff,0x000fffff,
	0x001fffff,0x003fffff,0x007fffff,0x00ffffff,
	0x01ffffff,0x03ffffff,0x07ffffff,0x0fffffff,
	0x1fffffff,0x3fffffff,0x7fffffff,0xffffffff
};
const uint iArrAnd3[]=
{
	0x00000000,0x00000001,0x00000002,0x00000004,0x00000008,
	0x00000010,0x00000020,0x00000040,0x00000080,
	0x00000100,0x00000200,0x00000400,0x00000800,
	0x00001000,0x00002000,0x00004000,0x00008000,
	0x00010000,0x00020000,0x00040000,0x00080000,
	0x00100000,0x00200000,0x00400000,0x00800000,
	0x01000000,0x02000000,0x04000000,0x08000000,
	0x10000000,0x20000000,0x40000000,0x80000000,0x00000000
};

const uint iArrAnd4[]=
{
	0xffffffff,0xfffffffe,0xfffffffc,0xfffffff8,
	0xfffffff0,0xffffffe0,0xffffffc0,0xffffff80,
	0xffffff00,0xfffffe00,0xfffffc00,0xfffff800,
	0xfffff000,0xffffe000,0xffffc000,0xffff8000,
	0xffff0000,0xfffe0000,0xfffc0000,0xfff80000,
	0xfff00000,0xffe00000,0xffc00000,0xff800000,
	0xff000000,0xfe000000,0xfc000000,0xf8000000,
	0xf0000000,0xe0000000,0xc0000000,0x80000000,0x00000000
};



int	GetInt(uchar* pBuf,int OffsBit,int CntBit,BOOL SignMinus=FALSE);


class	AddBuf
{
	int		Size;
public:
	size_t		*Buf;
	AddBuf();
	~AddBuf();
	virtual	char* AddStr(char* Str);	// Êîïèðóåò â áóôåð è âîçâðàùàåò óêàçàòåëü íà íåãî 
};


class	MethElem:public	ID_Elem 
{
public:
	char		Name[80];
	int			ModIndex;
	InfoMt		IMt;
	InfoPin		IPinI[100];
	InfoPin		IPinO[100];
	InfoPrm		IPrm[100];

	Decoder*	pDec;
	Services*	Srv;
	void*		StrInfo;
	AddBuf		ABuf;
	char		SourceType[8];
	MethElem(DLLCntrl	*DLLL,int ModID,char* MtName);
	~MethElem()
	{
	};
};

class	MethList	:	public	ID_List8
{
public:

};

class DLLCntrl  
{
public:
	Services*		Srv;
	int				CntMeth;
	StrOwner		Anchor;
	DLLCntrl(char* FullPath);
	FlagStruct		Fl_RE;
	virtual ~DLLCntrl();
// ---------------------
	virtual	char*		FirstModule();
	virtual	char*		NextModule();
	virtual	char*		FirstMeth(FindStrList8	&FSL8);
	virtual	char*		NextMeth(FindStrList8	&FSL8);
	virtual	char*		GetModuleName(char* NameMt);	// Íàéòè íàçâàíèå Ìîäóëÿ ïî èìåíè Ìåòîäà
// ---------------------
	virtual MethElem*	FindMeth(char*);	// Âûáðàòü Ìåòîä äëÿ äàëüíåéøèõ ìàíèïóëÿöèé
	virtual Module*		FindModule(char*);	// Âûáðàòü Ìåòîä äëÿ äàëüíåéøèõ ìàíèïóëÿöèé
	virtual Decoder*	NewDecod(char*,Decoder* ParentDec);	// Ñêîíñòðóèðîâàòü îáúåêò
	virtual int			GetInfoMethod(InfoMt* InfM,char* Meth);			// Àíàëîãè÷íî ìåòîäàì äåêîäåðà
	virtual int			GetInfoPinIn(int Num,InfoPin* InfP,char* Meth);	// Àíàëîãè÷íî ìåòîäàì äåêîäåðà
	virtual int			GetInfoPinOut(int Num,InfoPin* InfP,char* Meth);	// Àíàëîãè÷íî ìåòîäàì äåêîäåðà
	virtual int			GetInfoParam(int Num,InfoPrm* InfP,char* Meth);	// Àíàëîãè÷íî ìåòîäàì äåêîäåðà
	virtual	void		DeleteDecoder(Decoder*);
	virtual void		Init();
// ---------------------
	Module				*pMod;	// Òàáëèöà ñî ñòðóêòóðàìè Module
	int					rModSize;	// ðåàëüíîå êîëè÷åñòâî DLL - ôàéëîâ
protected:
	ID_List8*			MethInf;
	int					tModSize;	// òåêóùèé ðàçìåð òàáëèöû
	int					ActModule;	
// ---------------------
	char				FullPath[512];
	char				FullName[512];
	char				FullMod[512];

};


class	ConstrDec : public Decoder
//class	ConstrDec
{
public:

int		RecurseFl;
INT64	NClk;
double	nClk;

INT64	NC0,NC1,NC2;

INT64	NFClk;
double	nFClk;

LPVOID	Tthis;
int		Act_N_Md;
int		Act_N_Mt;
WUDec   Str;
int		TPinO;
int		IDOut;
LPVOID	ExtPoi;
int		ErrorConstr;
UDec*	TPUDec;
// -------------
char	TabID[65536];
USHORT	CntID;
UCHAR	ID_Host;

//Chronometry	*Chron;
Services	*Srv;
INT64		CE1,CE2,CE3;
int			NumStep;
//Glucator	Gluc;
LVITEM		m_pItem;
uint		kImage;
//CListCtrl*	pList;
// ---------------------
char		Path[512];
char		PathOut[512];

		ConstrDec();
		ConstrDec(UCHAR Host);
virtual		~ConstrDec();
virtual	void	Param(char* Prm,char* Vol);// Èçìåíèòü ïàðàìåòð
virtual void	Start();
virtual void	Stop();
virtual	int		Data(int N_Pin,UCHAR* pmas,size_t Cnt); // Äàííûå äëÿ äåêîäèðîâàíèÿ
// -------------

virtual	void	DirectParam(char* FullPrm,char* Vol);// Èçìåíèòü ïàðàìåòð
		// îáðàçåö
		// :NameMd\NameMt@ExtMt#Prm
		//	-||-	       :NameMd\ ...  - äëÿ âëîæåííûõ ðåæèìîâ

virtual	void	StepCalc(int PinI,int ID_Mt,UCHAR* pmas,size_t Cnt); // Äàííûå äëÿ äåêîäèðîâàíèÿ
virtual int		LoadFromCfg(char* Name);	// Íå ðàáîòàåò
virtual void	ReStruct(UDec* PUDec,int N_Md,int TractID);
virtual void	NewDecod(int N_Mt);
virtual void	DelDecod(Decoder* PDec);	// {delete PDec;};
virtual	void	StepStop(int ID_Mt); // Äàííûå äëÿ äåêîäèðîâàíèÿ
virtual	void	StepCalcStop(int ID_Mt); // Äàííûå äëÿ äåêîäèðîâàíèÿ
// --------------------------------------------
virtual	USHORT	NewIDMt()
						{
							while (TabID[CntID]!=0)
							{
								CntID++;
							}
							TabID[CntID]=1;
							return CntID++;
						};
virtual	void	DelIDMt(USHORT IDMt)
						{TabID[IDMt]=0;};
// --------------------------------------------
//void		CfgProc(LocalCfg* LocCfg,int Cnt,int tNstr);

void		(*OData)(int NPin,UCHAR* pmas,size_t Cnt,LPVOID Poi);

};

class	ConstrDecF : public ConstrDec
{
public:
int				TN_Md;
int				Fl_Error;
INT64			CntRun;

				ConstrDecF();
				ConstrDecF(UCHAR Host);
//virtual		~ConstrDecF();
//	List of same Function
//				Param(...);
//				Start();
virtual void	Stop();
//				NewDecod(); ???
//				DelDecod();
//				NewIDMt();
//				DelIDMt();
virtual	int		Data(int N_Pin,UCHAR* pmas,size_t Cnt)
				{
					if (Srv->Fl_NetLock!=0)
						return DataFF(N_Pin,pmas,Cnt);
					else
						return 0;
				}
virtual	int		DataFF(			// Äàííûå äëÿ äåêîäèðîâàíèÿ
					int N_Pin,		// Íîìåð âõîäíîé íîæêè
					UCHAR* pmas,	// Óêàçàòåëü íà áóôåð ñ äàííûìè
					size_t Cnt);		// Äëèíà äàííûõ
virtual	int		DataIDI(			// Äàííûå äëÿ äåêîäèðîâàíèÿ
					int N_Pin,		// Íîìåð âõîäíîé íîæêè
					IDInfo* pIDI);		// Äëèíà äàííûõ
virtual void	ReStruct(UDec* PUDec,int N_Md,int TractID);

};


LPVOID		FindNewDecoderMtIntDec(char* Name,Services* Srv);
char*	FindFirstIntDec();
char*	FindNextIntDec();
//int		FindNewDecoderMtIntDec(char* Name);
//char*	FindFirstIntDec();
//char*	FindNextIntDec();
//void	DeleteDecod(int);

class	Pack_Dec : public ConstrDecF
{
public:
//int			(**Fun)(char* Name);
//HINSTANCE	*ModuleHandle;
//Services	*pSrv;
//DLLCntrl*	DLLCtl;
		Pack_Dec(){};
		Pack_Dec(Services *Srv/*DLLCntrl* DLLC*/,UCHAR Host=NULL);
virtual	void	NewDecod(int	N_Mt);
virtual	void	DelDecod(Decoder* PDec);
};

// Ñòðóêòóðà äëÿ ïîëó÷åíèÿ èíôîðìàöèè îò äåêîäåðà
static char	SubTr_NameMt[]= "SubTr"; // !!! Èìÿ SubTr èñïîëüçóåòñÿ òàê æå â ìîäóëå Constr_d
static char	SubTr_RemMt[]= "";
static char	SubTr_Version[]= "V1.0";
static char	SubTr_Author[]= "SystemSoft (Y)";
static char	*SubTr_PinNameI[]={
	"DataIn","Inputs Data (for one channel mode)",						// 0
	"NameFile","File Name",												// 1
	"ExtFile","File Extension",											// 2
	"Path","Path",														// 3
	"NumChan","Channel Number",											// 4
	"MCh_Data","Inputs Data (MCh_Data format)",								// 5
	"TimeClick","Timer Click (for visual form and etc.)",				// 6
	"HostName","Clients Host Name for socket connected tracts (his number tell to NumChan)",
	"HostIP","Clients IP adress for socket connected tracts (his number tell to NumChan)",
	"SocketEvent","Sockets Events",				// 9
	"ThIDView","Input from ThIDView outputs",					// 10
	"IDI_FSrv","Input from IDI_FSrv outputs",					// 11
	"BDBInfo","Input from IDI_FSrv outputs",					// 12
	"NetComm","Input from NetComm outputs",						// 13
	"UserIn7","User inputs",									// 14
	"UserIn7","User inputs",									// 15
	"LoadCircBuff","Circle buffer loading,(% number in FLOAT formats)",
//	"LoadConfig","×òåíèå êîíôèãóðàöèîííîãî ôàéëà (ñòðóêòóðà LocCfg)",	// 17
	""};
static char	*SubTr_PinNameO[]={
	"DataOut","Output Data",								// 0
	"IDInfo","IDInfo Structure",							// 1
	"ThIDView","Output to ThIDView inputs",					// 2
	"IDI_FSrv","Output to IDI_FSrv inputs",					// 3
	"BDBInfo","Output to IDI_FSrv inputs",					// 4
	"NetComm","Output to NetComm inputs",					// 5
	"TimeClick","Timer Click",								// 6
	"NumChan","Channal number",								// 7
	"MCh_DataOut","Inputs Data (for multi-channel mode)",	// 8
	"GoodSeans","",		// 9
	"TrAnsv","",			// 10
	"LoopBack","Test Loop Back",							// 11
	"SetIP","",								// 12
	"SetPort","",							// 13
	"Data0","User inputs",									// 14
	"Data1","User inputs",									// 15
	"Data2","User inputs",									// 16
	"Data3","User inputs",									// 17
	"Data4","User inputs",									// 18
//	"Config","Çàïèñü è çàïðîñ êîíôèãóðàöèîííîãî ôàéëà (ñòðóêòóðà LocCfg)",	// 19
	""};

static char	*SubTr_Prm[]={
	"NameTrackt","Trackts Name",
	""};
	
static char	*SubTr_Vol[]={
	"NameTrackt","Default","Trackts Name",
	""};
// ----------------------------------------------
class	SubTr : public Pack_Dec
{
public:
// --- Variables ------
//int		(**Fun)(char* Name);
//HINSTANCE	*ModuleHandle;
//DLLCntrl*	DLLCtl;
//ConstrDec	*Trct;
//UCHAR*		PoiOut[100];
//int			LenBufOut[100];
//int			PoiBufOut[100];
//int			RealLenBufOut[100];
char		ModeName[80];
// ---- IDInfo --------
//IDInfo		**IDI;
//int			rNIDI,mNIDI;
//int			rdNIDI;
//UCHAR		*IDIData;
//int			rNIDID,mNIDID;
//------ Method -------
//Services	*pSrv;
int			Fl_Start;
		SubTr(Decoder *Dec);	//DLLCntrl* DLLC,UDec* UD);
virtual void Start();
virtual void Stop();
virtual	void Param(char* Prm,char* Vol);// Èçìåíèòü ïàðàìåòð
virtual	int	DataEx(int N_Pin,UCHAR* pmas,size_t Cnt);
virtual	int	DataFFEx(int N_Pin,UCHAR* pmas,size_t Cnt);
virtual	int	DataIDIEx(int N_Pin,IDInfo* pIDI);
virtual	int OutDataEx(int N_Pin,UCHAR* &pmas,size_t &Cnt);
virtual	int IdleFunc(int NumberTr);			// Idle function  if rc!=0 recall function (?) 
// ------- End --------
};

// Ñòðóêòóðà äëÿ ïîëó÷åíèÿ èíôîðìàöèè îò äåêîäåðà
// ConcBuf_ = íàèìåíîâàíèå îáúåêòà_
static char	ConcBuf_NameMt[]= "ConcBuf";
static char	ConcBuf_Version[]= "V1.0";
static char	ConcBuf_Author[]= "SystemSoft(Y)";
static char	ConcBuf_RemMt[]= "";
static char	*ConcBuf_PinNameI[]=
{
	"Input","",
	""
};
static char	*ConcBuf_PinNameO[]=
{
	"Output","",
	""
};

static char	*ConcBuf_Prm[]=
{
//	"NamePrm","Rem",
	""
};

static char	*ConcBuf_Vol[]=
{
//	"NamePrm","Vol1","Rem",
//	"NamePrm","Vol2","Rem",
//	"NamePrm2","Vol1","Rem",
//	"NamePrm2","Vol2","Rem",
	""
};
// ----------------------------------------------
class	ConcBuf : public	Decoder
{
public:
int		Fl_Stop;
int		Fl_Data;
UCHAR	*OBuf;
int		BSize;
int		RSize;
		ConcBuf();
		~ConcBuf();
virtual	void	Param(			// Èçìåíèòü ïàðàìåòð
			char* Prm,	// Èäåíòèôèêàòîð ïàðàìåòðà
			char* Vol);	// Çíà÷åíèå ïàðàìåòðà
virtual void	Start();		// Íà÷àëî ñåàíñà
virtual void	Stop();			// Êîíåö ñåàíñà
virtual	int	Data(			// Äàííûå äëÿ äåêîäèðîâàíèÿ
			int N_Pin,	// Íîìåð âõîäíîé íîæêè
			UCHAR* pmas,	// Óêàçàòåëü íà áóôåð ñ äàííûìè
			size_t Cnt);	// Äëèíà äàííûõ
virtual	int	OutData(			// Âûõîäíûå äàííûå
			int N_Pin,	// Íîìåð âûõîäíîé íîæêè
			UCHAR* &pmas,	// Óêàçàòåëü íà áóôåð ñ äàííûìè
			size_t &Cnt);	// Óêàçàòåëü íà äëèíó äàííûõ

};

#define	MaxTrPack	16*1024

#define	CBE_Connect	0
#define	CBE_DisCon	1
#define	CBE_Error	3
#define SockCliConnect 1
#define SockCliDisConnect 2

#define SockBufSizeRecv 0x1000000
#define SockBufSizeSend 0x1000000

// ---------------------------------------------------------------------

int ReceiveDisconnect(int,int);

// ---------------------------------------------------------------------
#define CLIENT_SBUF   0x8000


class	UDP_El	:	public	ID_Elem
{
public:	
				UCHAR	Key[4];
				void	*Packet;
				size_t		LenPack;
				UDP_El(UCHAR* Buf,size_t Cnt)
				{
					Packet=NULL;
					Init (Buf,Cnt);
				}
virtual	void	Init(UCHAR*	BuffIn,size_t	Cnt) // Èñïîëüçóåòñÿ ïðè ïîâòîðíîì èñïîëüçîâàíèè ýëåìåíòà
				{
					if (Packet==NULL)
						Packet=malloc(Cnt);
					else
					{
						free(Packet);
						Packet=malloc(Cnt);
					}
					memcpy(Packet,BuffIn,Cnt);
					LenPack=Cnt;
					Key[3]=*(BuffIn+15);
					Key[2]=*(BuffIn+14);
					Key[1]=*(BuffIn+13);
					Key[0]=*(BuffIn+12);
					Buff=Key;
					Len=sizeof(int);
				};
				~UDP_El()
				{
					if (Packet!=NULL)
						free(Packet);	
				}
};

// ---------------------------------------------------------------------
class ServerClientSock;
// ---------------------------------------------------------------------
/*
typedef struct _ItemClientList
{
	int f,id;
	ServerClientSock *obj;
}ItemClientList, *pItemClientList;
*/
// ---------------------------------------------------------------------


class ServerClientSockEx : public ServerClientSock
{
public:
// ----------------------------
		int	Div;
		int	rc;
		int	tLen;
		int tCnt;
		int	Fl_Calc;
		FileTime	RdTime;
		DamperPS	DmpR;
		ClassThread	*ClThread2;
		int	Fl_Lock;
// ----------------------------
	ServerClientSockEx(SOCKET handle, ServerSock *ssock,Services *pSrv=GetServices(),Decoder* pDec=NULL);
	virtual		~ServerClientSockEx();
	virtual	int ReadSockI(UCHAR* &Buf, size_t &Cnt);
	virtual	int ReadSock(UCHAR* &Buf, size_t &Cnt);
};

// ----------------------------------------------
// ---------------------------------------------------------------------



// ------------------
#define UDPReplCom_Pack		0
#define UDPReplCom_Req		1
#define UDPReplCom_Syn		2
#define UDPReplCom_Empty	3
// ------------------

struct ReqPack
{
	int	CntPack;
	int	NumCli;
};


static char	NullMt_NameMt[]= "NullMt";
static char	NullMt_Version[]= "V1.0";
static char	NullMt_Author[]= "Nemo";
static char	NullMt_RemMt[]= "Empty module";
static char	NullMt_Type[]= "Decoder";
static char	*NullMt_PinNameI[]=
{
	//	"NamePinI1","Rem",
	""
};
static char	*NullMt_PinNameO[]=
{
	//	"NamePinO1","Rem",
	""
};

static char	*NullMt_Prm[]=
{
	//	"NamePrm","Rem",
	""
};

static char	*NullMt_Vol[]=
{
	//	"NamePrm","Vol1","Rem",
	//	"NamePrm","Vol2","Rem",
	//	"NamePrm2","Vol1","Rem",
	//	"NamePrm2","Vol2","Rem",
	""
};
// ----------------------------------------------
class	NullMt : public	Decoder
{
public:
	int		Fl_Stop;
	int		Fl_Data;

	NullMt();
	~NullMt();
	virtual	void	Param(			// Изменить параметр
		char* Prm,	// Идентификатор параметра
		char* Vol);	// Значение параметра
	virtual void	Start();		// Начало сеанса
	virtual void	Stop();			// Конец сеанса
	virtual	int	DataFF(			// Данные для декодирования
		int N_Pin,	// Номер входной ножки
		UCHAR* pmas,	// Указатель на буфер с данными
		int Cnt);	// Длина данных
};

// ------------------
// Ñòðóêòóðà äëÿ ïîëó÷åíèÿ èíôîðìàöèè îò äåêîäåðà
// Invertor_ = íàèìåíîâàíèå îáúåêòà_
static char	Invertor_NameMt[]= "Invertor";
static char	Invertor_Version[]= "V1.0";
static char	Invertor_Author[]= "SystemSoft(Y)";
static char	Invertor_RemMt[]= "";
static char	*Invertor_PinNameI[]=
{
	"DataIn","",
	""
};
static char	*Invertor_PinNameO[]=
{
	"DataOut","",
	""
};

static char	*Invertor_Prm[]=
{
//	"NamePrm","Rem",
	""
};

static char	*Invertor_Vol[]=
{
//	"NamePrm","Vol1","Rem",
//	"NamePrm","Vol2","Rem",
//	"NamePrm2","Vol1","Rem",
//	"NamePrm2","Vol2","Rem",
	""
};
// ----------------------------------------------
#define	SizeInvBuf	32*1204
class	Invertor : public	Decoder
{
public:
UCHAR	*tBuf;
int		tCnt;
int		Flag,Sm;

UCHAR	BufOut[SizeInvBuf];
INT64	CntTimer;
ElUDPCli*	ElUDP;
		Invertor();
		~Invertor();
virtual	void	Param(			// Èçìåíèòü ïàðàìåòð
			char* Prm,	// Èäåíòèôèêàòîð ïàðàìåòðà
			char* Vol);	// Çíà÷åíèå ïàðàìåòðà
virtual void	Start();		// Íà÷àëî ñåàíñà
virtual void	Stop();			// Êîíåö ñåàíñà
virtual	int	Data(			// Äàííûå äëÿ äåêîäèðîâàíèÿ
			int N_Pin,	// Íîìåð âõîäíîé íîæêè
			UCHAR* pmas,	// Óêàçàòåëü íà áóôåð ñ äàííûìè
			size_t Cnt);	// Äëèíà äàííûõ
virtual	int	OutData(			// Âûõîäíûå äàííûå
			int N_Pin,	// Íîìåð âûõîäíîé íîæêè
			UCHAR* &pmas,	// Óêàçàòåëü íà áóôåð ñ äàííûìè
			size_t &Cnt);	// Óêàçàòåëü íà äëèíó äàííûõ
virtual	int	IdleFunc(int NumberTr);


};

int GenNameLocal(char* cat,char* ext);
int GenName(char* cat,char* ext);
HANDLE GenName_H(char* cat,char* ext);
int GenNameFile(char* cat,char* ext);

char*	CalcNameDataLocal(char* tbuf,int num);
char*	CalcNameData(char* tbuf,int num);
int		CalcPath(char* path);
int		CalcNetPath(char* path);
int		MakeDir(char* path);
int		TestPath(char* path);
void	TestNetName(char* path);

#define	SizeOfBuf 102400

class	StrimFile
{
int	FH;
int	Poi,PoiR,PoiBuf,LenBuf;
UCHAR	Buf1[SizeOfBuf];
public:
	StrimFile();
int	Open(char* Name);
int	Creat(char* Name);
int	Close();
int	Lseek(int Sm,int Flag);
int	Read(UCHAR* Buf,int Len);
int	Write(UCHAR* Buf,int Len);
int	ReadChar(char &Ch);
int	ReadBuf();
int	WriteBuf();
int	WriteChar(char Ch);
int	WriteShort(short Vol);

short	ReadShort();
int	ReadInt();
float	ReadFloat();
};

struct	RandomBuf
{
	UCHAR	R0;
	UCHAR	R1;
	UCHAR	R2;
	UCHAR	R3;
};

void	_CopyParameter(IDInfo* pIDInfo0,IDInfo* pIDInfo1,Decoder* pDec,El_Tab* _pElTb=NULL);
int		mStrCmpHost(char* Host1,char* Host2);


// ------------------
// Ñòðóêòóðà äëÿ ïîëó÷åíèÿ èíôîðìàöèè îò äåêîäåðà
// MultThrdMt_ = íàèìåíîâàíèå îáúåêòà_
static char	MultThrdMt_NameMt[]= "MultThrdMt";
static char	MultThrdMt_Version[]= "V1.0";
static char	MultThrdMt_Author[]= "SystemSoft(Y)";
static char	MultThrdMt_RemMt[]= "Multy thread manager";
static char	MultThrdMt_Type[]= "Decoder";
static char	*MultThrdMt_PinNameI[]=
{
	"DataIn0","",
	"DataIn1","",
	"DataIn2","",
	"DataIn3","",
	"DataIn4","",
	"DataIn5","",
	"DataIn6","",
	"DataIn7","",
	"DataIn8","",
	"DataIn9","",
	""
};
static char	*MultThrdMt_PinNameO[]=
{
	"DataOut0","",
	"DataOut1","",
	"DataOut2","",
	"DataOut3","",
	"DataOut4","",
	"DataOut5","",
	"DataOut6","",
	"DataOut7","",
	"DataOut8","",
	"DataOut9","",
	""
};

static char	*MultThrdMt_Prm[]=
{
	"NameMt","Method Name",
	"Mode","",
	"Fd","",
	"Fn","",
//	"dFn","äèàïàçîí èçìåíåíèÿ íåñóùåé ÷àñòîòû",
	"Fb","",
//	"dFb","äèàïàçîí èçìåíåíèÿ áîäîâîé ÷àñòîòû",
//	"Fr","×àñòîòà ðàçíîñà",
//	"dFr","äèàïàçîí èçìåíåíèÿ ÷àñòîòû ðàçíîñà",
//	"Fch","Ðàçíîñ ìåæäó êàíàëàìè",
//	"dFch","äèàïàçîí èçìåíåíèÿ ðàçíîñà ìåæäó êàíàëàìè",
//	"K11","Êîýôôèöèåíò K1 ÀÐÓ1 (áûñòðîå)",
//	"K12","Êîýôôèöèåíò K1 ÀÐÓ2 (ìåäëåííîå)",
//	"Ep","Ïîðîã ïðåäåëüíîãî ñîîòíîøåíèÿ Ñ/Ø äëÿ äàííîãî òèïà äåìîäóëÿòîðà (Äëÿ Ñ>Ø çíà÷åíèå îòðèöàòåëüíîå)",
//	"Kea1","Êîýôôèöèåíò àäàïòàöèè äî çàõâàòà ñèãíàëà",
//	"Kea2","Êîýôôèöèåíò àäàïòàöèè ïîñëå çàõâàòà ñèãíàëà",
//	"KuP1","Êîýôôèöèåíò óñèëåíèÿ îøèáêè ïîäñòðîåê äî çàõâàòà ñèãíàëà",
//	"KuP2","Êîýôôèöèåíò óñèëåíèÿ îøèáêè ïîäñòðîåê ïîñëå çàõâàòà ñèãíàëà",
	"TrMode","",
//	"Kerr","Êîýôôèöèåíò óñðåäíåíèÿ äëÿ âû÷èñëåíèÿ ñîîòíîøåíèÿ Ñ/Ø",
	"ModeInp","",
//	"TabCod","Âûáîð áèòîâîé êîäèðîâêè ñèãíàëüíîãî ñîçâåçäèÿ (äëÿ ðåæèìîâ êðîìå O-QPSK)",
	""
};

static char	*MultThrdMt_Vol[]=
{
	"NameMt","FFT","",
	"Mode","Off","",
	"Mode","BPSK","",
	"Mode","QPSK","",
	"Mode","TPSK","",
	"Mode","O-QPSK","",
	"Mode","MTOF","",
	"Mode","MTOFA","",
	"Mode","QAM8","",
	"Mode","QAM16","",
	"Mode","QAM32","",
	"Mode","QAM64","",
	"Mode","QAM128","",
	"Mode","QAM256","",
	"Mode","QAM512","",
	"Fd","22050","",
	"Fd","8000","",
	"Fd","11025","",
	"Fd","22100","",
	"Fn","1850","",
	"Fn","3100","",
	"Fn","3600","",
	"Fb","2400","",
	"Fb","4000","",
	"Fb","6000","",
	"TrMode",N_Continue,"",
	"TrMode","Packet","",
	"TrMode","Block","",
	"ModeInp","Simpl","S(t)",
	"ModeInp","Complex","I & Q",
	""
};

// ----------------------------------------------

int MTFun(ULONG	Prm);

class	MultThrdMt : public	Decoder
{
public:
int				Fl_Stop;
int				Fl_Data;
Decoder*		PMt;
DamperPS		DPSIn;
DamperPS		DPSOut;
char			NameMtBuf[256];
char			TelemPrm[256];
int				CntMemClIn,CntMemClOut;
ClassThread*	CThread;
INT64			TimerTest;	
FileTime		TimeOutIn;
FileTime		TimeOutOut;
		MultThrdMt();
		~MultThrdMt();
virtual	void	Param(			// Èçìåíèòü ïàðàìåòð
				char* Prm,	// Èäåíòèôèêàòîð ïàðàìåòðà
				char* Vol);	// Çíà÷åíèå ïàðàìåòðà
virtual void	Start();		// Íà÷àëî ñåàíñà
virtual void	Stop();			// Êîíåö ñåàíñà
virtual	int		DataFF(			// Äàííûå äëÿ äåêîäèðîâàíèÿ
				int N_Pin,	// Íîìåð âõîäíîé íîæêè
				UCHAR* pmas,	// Óêàçàòåëü íà áóôåð ñ äàííûìè
				size_t Cnt);	// Äëèíà äàííûõ
virtual	int		Idle(void);
};


bool sfQBE_Test(IDInfo* pInpIDI,uchar *pBuffQBE,int CntBuffQBE,Services* pSrv);

struct	StrSzIDID
{
	INT64	MemSize;
	INT64	NumIDI;
};
/*
class	IDInfoDamper
{
protected:
	ID_List8*			IDL;
	ID_List8*			IDLCnt;
	StrSzIDID			SzIDID;	
	FlagStruct			Fl_RE;
	Services*			Srv;
	INT64				OrderNumber;
public:
	IDInfoDamper();
	~IDInfoDamper();
	//---------------------
	virtual	int			WriteIDI(IDInfo* IDI);
	virtual	IDInfo*		ReadIDI();
	virtual	IDInfo*		ReadAndRemoveIDI();	// You can use Srv->DeleteIDI(IDI);
	virtual	int			DelIDI(IDInfo* IDI);
	virtual	IDInfo*		FindIDI(IDInfo* IDI);
	virtual	IDInfo*		FindIDI(INT64 ID);
	virtual	StrSzIDID*	GetSize();
	
};
*/
class	IDInfoConverter
{
protected:
	uchar*		Buf;
	IDInfo*		tIDI_InfoConverter;
	Services*	Srv;
	BandleList	BL;
	BandleFind	BF;
	UCHAR*		OutBuf;
	size_t		SizeOB;
	BandleList	PL;
	UCHAR*		PrmBuf;
	size_t		SizePrm;
	AddressInfo	AdrI;
public:
		IDInfoConverter();
		~IDInfoConverter();
virtual	uchar*	IDInfo2Buf(IDInfo* IDI,size_t	&Cnt);
virtual	IDInfo*	Buf2IDInfo(uchar* Buf,size_t	Cnt);
virtual	void	ReleaseIDInfo(IDInfo* tIDI);
virtual	int		PList2Buf(IDInfo* IDI,uchar* &Buf,size_t	&Cnt);
virtual	int		Buf2PList(uchar* Buf,size_t	Cnt,IDInfo* IDI);
};

struct  TempSrvDecPoi
{
ConstrDecF*		HandlePointer;
ClassThread*	ThreadPointer;
};

class	ServerDecod
{
public:
/*
char*				(*FindFirst[1000])();
char*				(*FindNext[1000])();
int					(*FindNewDecod[1000])(char*);
HINSTANCE			ModuleHandle[1000];
char				Name[1000][40];
*/

int		SpeedIn;
int		SpeedOut;

int		ClkWDog;

//Task		*Tsk[MaxID];
HANDLE		MapT[MaxID];
ClassThread	*SrvThread;
ClassThread	*SysThread;
ClassThread	*ClThread[MaxID];
//TID		T_ID;
//HANDLE	Map1;
//SharedNet	*SDec;

int			ID[MaxID];
//int			FlagCli[MaxID];
//char		DomainName[MaxID][256];
SimCTask	SimTsk;
char		TBuf[256];
int			TLen;
int			TmpIDTrct;
Services	*Srv;
Task		*TTsk;
int			Fl_Strat;
//private:
ConstrDecF	*Tract[MaxID];
double		TractUsage[MaxID];
//public:
FlagStruct		Fl_RE;
FlagStruct		Fl_REDin;
CListCtrl*		pList;
CListCtrl*		pList2;
INT64		CntRun[MaxID];
INT64		CntDinRun[MaxID];
INT64		CntTry[MaxID];
INT64		CntDinTry[MaxID];
LVITEM		m_pItem;
uint		kImage;

ConstrDecF	*DinTract[MaxID];
double		DinTractUsage[MaxID];
ClassThread	*DinThread[MaxID];
DamperPS	DinDmp;

		ServerDecod(Services	*Srv);
		~ServerDecod();
//virtual	void	WDog();
virtual	void		Calc(int N_Pin,UCHAR* Buf,int Len,int Poi);
virtual	void		ServicesIdle();
virtual	void		TractIdle(size_t ID);
virtual	void		ReStartTr(int Number=-1);
virtual	void		SaveCfg();
virtual	void		LoadCfg();
virtual	void		CloseTr();
//virtual	void	GetTract(int Num);
virtual	int			AddTract(char* DomainName,char* PathOut);	// -1 error
virtual	int			DelTract(char* DomainName);	// -1 error
virtual	void		RePackTr();
virtual	void		StopTr(char* DomainName);
virtual	void		StartTr(char* DomainName);
virtual	void		ReStartTr(char* DomainName);
virtual	void		SendData(int N_Pin,UCHAR* Buf,int Len);	// Not used
virtual ConstrDecF*	CreateDinTract(char* ModeName,char* DomainName,int NumChan=0);
virtual int			DeleteDinTract(ConstrDecF* HandlePointer);
virtual	void		DinTractIdle(size_t ID);
virtual	void		SystemIdle();
virtual	void		SetUsage(size_t ID,double Vol);	// 0 - 100 %
virtual	void		SetDifUsage(size_t ID,double Vol);	// Usage=Usage+Vol ()
protected:
virtual	void		SendToDestroy(ConstrDecF* HandlePointer,ClassThread* ThreadPointer);
};

//#endif

