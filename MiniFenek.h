#pragma once

#include	"DecodB.h"
#include	"wavlib.h"
#include	"SockMt.h"

#define		UDPPortOld			5101
#define		UDPPortNew			5112
#define		UDPPortDebug		5121

#define		UDPPortStepper		5110
#define		LenQueue	1024 

#pragma pack(push,1)

/*
class	DevCAN : public	ID_Elem, Decoder
{
public:
	DevCAN(int8_t DevID, Services* Srv);
	virtual ~DevCAN();
	virtual	void	DevInit(int32_t DevID);
	virtual	void	Del_ID_Elem(void);

};
*/

// Target Type Begin
#define	TT_Mechanical	0x01
#define	TT_2TMotor		0x02
#define	TT_FPVDrone		0x04
#define	TT_Helicopter	0x08
#define	TT_Aircraft		0x10
#define	TT_Rocket		0x20
// Target Type End


enum SessionType
{
	SNS_Begin = 1,
	SNS_Continue,
	SNS_End,
};

typedef struct
{
	int32_t		SID;
	SessionType	State;
} SessionStr;

typedef struct	DeviceInfo
{
	uint32_t	UUID;
	uint8_t		DevType;
}	DeviceInfo;


class DevElem : public ID_Elem
{
public:
	int					Item;
	uint8_t				HW_ID;
	uint8_t				HW_Type;
	float				Azimuth;
	float				UgolM;
	float				Compass;
	uint8_t				Level[4];
	float				Lat;
	float				Lng;
	float				Alt;
	float				UTC;
	uint8_t				GPS_State;
	float				Ev_Azimuth;
	float				Ev_UgolM;
	uint32_t			Ev_Time;
	uint16_t			Version;
	DeviceInfo	    	DInfo;
	SessionStr			SNSStr;
	uint8_t				TargetType;
/*
	short				Version;
	uint32_t			IPAddr;
	char				DevName[40];
	float				CPUUsage;
	float				MemUsage;
	int32_t					CntPack;
	char				Fl_Active;
	TimePoint			lastUdpPacketTime;
	sockaddr			Addr;
	ProcessStartMode	Fl_Mode;
	ProcessState		processState;
*/
//	DevCAN*				PDevEl;
	DevElem();
	~DevElem();
	virtual	void Del_ID_Elem(void);
};


static const char	DevIntegrator_NameMt[] = "DevIntegrator";
static const char	DevIntegrator_Version[] = "V1.0";
static const char	DevIntegrator_Author[] = "YuVlV";
static const char	DevIntegrator_RemMt[] = "Device Integrator";
static const char	DevIntegrator_Type[] = "Decoder";
static const char* DevIntegrator_PinNameI[] =
{
	""
};
static const char* DevIntegrator_PinNameO[] =
{
	""
};

static const char* DevIntegrator_Prm[] =
{
	""
};

static const char* DevIntegrator_Vol[] =
{
	""
};

// ----------------------------------------------
class	DevIntegrator : public	Decoder
{
public:
	int32_t			Fl_Stop;
	int32_t			Fl_Data;
	ElUDPCli*		ElUDPStepper;
	ElUDPCli*		ElUDPHUBOld;
	ElUDPCli*		ElUDPHUBNew;
	ElUDPCli*		ElUDPDebug;
//	Services*		Srv;
	ClassThread*	Idel_Thread;
	ID_List8		DevList;
	CANMsgStr		Msg;
	CANMsgStr		MsgInQueue[LenQueue];
	CANMsgStr		MsgOutQueue[LenQueue];
	int				Rd_IndMsg;
	int				Wr_IndMsg;
	int				Rd_OutMsg;
	int				Wr_OutMsg;
	DevElem*		activeElement;
	sockaddr		AddrFrom;
	CANMsgStr		MsgOut;
	uint8_t			Fl_DataStream;
	uint8_t			Fl_SaveWav;
	uint8_t			Fl_Debug;
	FileWAV_VeryFast* WavTest;
	SockCli*		CSock;
	int				Fl_CliConnect;
	uchar			SockBuf[10240];
	int				PoiWrSB;
	uint8_t			Fl_UDPOld;
	uint8_t			Fl_UDPNew;
	uint32_t		CntCANError;
	uint32_t		CntCANOk;
	float			SpeBuf[1024];
	/*
		//---------  Test ESP32  ----------
		SockSrv* SSock;
		int			Fl_Connect;
		int			SrvNumCli;
		char		SrvCliIP[256];
		char		SrvCliName[256];
		// --------------------------------

		TimePoint	LastIdelFTime;
		// -----------------------------------------
		VersionStr	VersionUDP;
		int32_t			UpDaterModeUDP;
		int32_t			Fl_UDPStream;
		int32_t			Fl_WiatingAnswer;	// 0-Ok 1-wait 2-Error 4-Unknown
		TimePoint	RealTimeUDP;
		// -----------------------------------------
		int32_t			Fl_InitDlg;
		ElUDPCli* ElUDPCom;
		ElUDPCli* ElUDPIntegrator;
		ElUDPCli* ElUDPStream;
		ElUDPCli* ElUDPStream2;
		PackStr			Pack;
		int32_t			Fl_Stream;
		UDPPackHW* UDPPack;
		int32_t			Fl_Print;
		DevElem* activeElement;
		DamperPS* DmpUDP;
		DamperPS* DmpUDP2;
		std::string		PathNameUDPStationCfg;
	*/


	DevIntegrator(Services* pSrv);
	~DevIntegrator();
	//	virtual	DevElem* SetActive(uint32_t itemID);	// From Dlg
	//	virtual	DevElem* GetActive();	// From Dlg
	//	virtual	void SetAction(int32_t Action, int32_t State = 0);	// From Dlg
	//	virtual	void SendPack(PackStr* pPack, sockaddr* PeerTo);

	virtual	void	Param(			// Изменить параметр
		const char* Prm,	// Идентификатор параметра
		const char* Vol);	// Значение параметра
	virtual void	Start();		// Начало сеанса
	virtual void	Stop();			// Конец сеанса
	virtual	int32_t	DataFF(			// Данные для декодирования
		int32_t N_Pin,	// Номер входной ножки
		UCHAR* pmas,	// Указатель на буфер с данными
		int32_t Cnt);	// Длина данных
	virtual	int32_t UserCallBackEx(	// Функция обратного вызова
		int32_t	Handle,				// Идентификатор события
		int32_t N_Pin,				// Номер входной ножки (расширение для определения нескольких функций)
		UCHAR* pmas,				// Указатель на буфер с данными
		size_t Cnt,
		int32_t FlagBuf = 0); // override;		// Длина данных
	virtual	int32_t	 IdleFunc();
	virtual	DevElem* SetActive(uint8_t itemID);	// From Dlg
	virtual	DevElem* GetActive();	// From Dlg
	virtual	void	SetAction(int8_t Action, int8_t State = 0);	// From Dlg

	virtual	void	AddToCANQueue(CANMsgStr*);
	virtual	void	ParserMsg(CANMsgStr*);
	virtual	void	SendToCANQueue(CANMsgStr*);
	virtual	void	SendMsg2CAN(uchar Command, uchar IDTo, uchar* Buf, int Len);
	virtual	void	ParserPacketTCP(uchar* tBuf, int Len);
	virtual int32_t listening_port_5111();

	//	void			RestartPurgers();

	//	F_Purger* GetSessionsPurger();
	//	F_Purger* GetEventsPurger();

private:
	//	void createDeviceProcess(DevElem* tDevEl);
	//	void destroyDeviceProcess(DevElem* tDevEl);

public:
	//	sigslot::signal<DevElem*> deviceProcessStarted;
	//	sigslot::signal<uint32_t> deviceProcessDestroyed;

	//	sigslot::signal<DevElem*> deviceUpdated;
	//	sigslot::signal<uint32_t> deviceRemoved;

private:
	//	std::unique_ptr<F_Purger>	SessionsPurger;
	//	std::unique_ptr<F_Purger>	EventsPurger;
};

