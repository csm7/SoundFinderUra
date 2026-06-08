
// SoundServerDlg.h: файл заголовка
//

#pragma once

extern	Services		*Srv;
extern	CMainCfg		*MCfg;

#include "Grafic.h"
#include "MiniFenek.h"

#define		UPDBlockSize	1024

enum CAN_SPEED {
	CAN_5KBPS,
	CAN_10KBPS,
	CAN_20KBPS,
	CAN_31K25BPS,
	CAN_33KBPS,
	CAN_40KBPS,
	CAN_50KBPS,
	CAN_80KBPS,
	CAN_83K3BPS,
	CAN_95KBPS,
	CAN_100KBPS,
	CAN_125KBPS,
	CAN_200KBPS,
	CAN_250KBPS,
	CAN_500KBPS,
	CAN_1000KBPS
};

typedef enum
{
	St_Answ_OK = 5,
	St_Answ_Error,
	St_Answ_Absent,
	St_Answ_UnKnown,
	St_Send_Request = 10,
	St_Get_SystemInfo,
	St_Send_SystemInfo,
	St_Send_BeginSession,
	St_Send_Session,
	St_Send_EndSession,
	St_Set_StationNewID,
	St_Send_Reset,
	St_Send_BeginUpLoadFile,
	St_Send_UpLoadFile,
	St_Send_EndUpLoadFile,
	St_Send_BeginDownLoadFile,
	St_Send_DownLoadFile,
	St_Send_EndDownLoadFile,
	St_Send_DeleteFile,
	St_Send_CreateDir,
	St_Send_RenameDir,
	St_Send_DeleteDir,
	St_Send_SoundSreamOn,
	St_Send_SoundSreamOff,
	St_Send_SaveWavOn,
	St_Send_SaveWavOff,
	St_Send_SessionStreamOn,
	St_Send_SessionStreamOff,
	St_Send_RealTimeInfo,
} Net_Command;

#pragma pack(push,1)

struct VersionStr
{
	USHORT	Version;
	USHORT	Build;
};

struct NetFileBeginStr
{
	UINT64	File_TID;
	char	File_Name[128];
	char	File_Path[256];
	uint	File_Type;		// ????
	uint	File_Size;		// если 0 то не знаем
};

struct NetFileData
{
	UINT64	File_TID;
	uint	File_BlockLen;
	uchar	File_Data[1024];
};

struct NetRenameDirStr
{
	char	File_PathOld[256];
	char	File_PathNew[256];
};

struct NetFileEnd
{
	UINT64	File_TID;
	short	File_CRC16;
};


struct AudioLocatorRealTimeInfo
{
	float		CPULoad;				// 0 - 100.0 %
	UCHAR		MicroPhoneStatus[8];	// 0 - Undeterm 1 - Ok 2-N Error
	UCHAR		Status;					// Station Status
	UCHAR		Fl_Stream;				//	
	UCHAR		Fl_Calc;				//
	UCHAR		Fl_Session;				//
	INT64		SessionID;				//
	FileTime	SessionBeginTime;		//
	FileTime	SessionOperTime;		//
	float		FiAzimuth;				//
	float		FiElevation;			//
	float		P_Delta;				//
	int			FiAzimReady;
	int			FiElevReady;
};

struct AudioLocatorSystemInfo
{
	INT64						StationID;
	VersionStr					VersionMainApp;
	VersionStr					VersionSmStarter;
	VersionStr					VersionFirmWare;
	char						CliIP[16];
	char						CliName[128];
	double						Latitude;
	double						Longitude;
	double						Altitude;
	int							TypeCli;		// 0-UnDef 1-Station 2-ControlCli 3-UserCli
// Place for new param

	AudioLocatorRealTimeInfo	RTInfo;
};

class js_AudioLocatorSystemInfo
{
public:
	js_AudioLocatorSystemInfo() {};

	AudioLocatorSystemInfo		ASInfo;

	void serialize(JSON::Adapter& adapter)
	{
		// this pattern is required 
		JSON::Class root(adapter, "SystemInfo");
		JSON_E(adapter, ASInfo.StationID);
		JSON_E(adapter, ASInfo.Latitude);
		JSON_E(adapter, ASInfo.Longitude);
		JSON_E(adapter, ASInfo.Latitude);
		JSON_E(adapter, ASInfo.Altitude);
		JSON_E(adapter, ASInfo.RTInfo.SessionID);
		JSON_E(adapter, ASInfo.RTInfo.SessionBeginTime.Time);
		JSON_E(adapter, ASInfo.RTInfo.SessionOperTime.Time);
		JSON_E(adapter, ASInfo.RTInfo.Status);
//		JSON_E(adapter, ASInfo.RTInfo.MicroPhoneStatus[0]);
//		JSON_E(adapter, ASInfo.RTInfo.MicroPhoneStatus[1]);
//		JSON_E(adapter, ASInfo.RTInfo.MicroPhoneStatus[2]);
//		JSON_E(adapter, ASInfo.RTInfo.MicroPhoneStatus[3]);
//		JSON_E(adapter, ASInfo.RTInfo.MicroPhoneStatus[4]);
//		JSON_E(adapter, ASInfo.RTInfo.MicroPhoneStatus[5]);
//		JSON_E(adapter, ASInfo.RTInfo.MicroPhoneStatus[6]);
//		JSON_E(adapter, ASInfo.RTInfo.MicroPhoneStatus[7]);
		JSON_E(adapter, ASInfo.RTInfo.P_Delta);
		JSON_E(adapter, ASInfo.RTInfo.Fl_Session);
		JSON_E(adapter, ASInfo.RTInfo.FiAzimuth);
		JSON_E(adapter, ASInfo.RTInfo.FiElevation);
		JSON_E(adapter, ASInfo.RTInfo.FiAzimReady);
		JSON_E(adapter, ASInfo.RTInfo.FiElevReady);
	}
};

struct AudioStationPack
{
	PackHeader		PHeader;
	UCHAR			Data[MaxPack];
};

struct Station	// New version
{
	UCHAR					Fl_Activity;
	double					Latitude;
	double					Longitude;
	double					Altitude;
	float					AthimuthCorrection;
	float					X, Y;
};

struct StationConfig	// New version
{
	char					IP_Port[16];
	char					IP_Address[32];
	char					PathNameMainApp[128];
	char					PathNameSmStarter[128];
	char					PathNameFirmWare[128];
	UINT64					StationIDLAN;					// Для автоматического присоения уникального идентификатора
	UINT64					StationIDWAN;
	VersionStr				VersionApp;
	VersionStr				VersionSmSt;
	VersionStr				VersionFW;
	char					IP_WEBPort[16];
	int						Fl_UsedLANList;
	int						Fl_UsedWANList;
	char					IP_WEBSockPort[16];

	float					Poligon;
	Station					Dev[256];

	char					Fl_En_LR;
	char					Fl_En_UD;
	char					Fl_Triangulation;
	char					Fl_UseGeoData;
	int						Speed;
	int						Step;
	int						IP_Stepper;
};

class CUserCfg
{
public:
	int				HF;
	int				LenF;
	char			PathNameUserCfg[128];
	StationConfig	StCfg;
	CUserCfg(CMainCfg* MCfg);
	~CUserCfg();
	virtual int Load();
	virtual int Save();
};
class IPElem : public ID_Elem
{
public:
	int		NumCli;
	int		Fl_Connect = 1;
	char	CliIP[16];
	char	CliName[128];
	IPElem();
	~IPElem();
	virtual	void	InitIPEl(int NumCli);
};

class StElem : public ID_Elem
{
public:
	AudioLocatorSystemInfo	ALSInfo;
	int						Item;
	int						NumCli;
	int						Fl_Active;
	// --------------------------------------------------
	StElem();
	~StElem();
	virtual	void	InitStEl(AudioLocatorSystemInfo* ASInfo);
/*
	virtual void	InitDevEl(OUT_UDP_PACK* Pack);
	virtual void	SetActive();
	virtual void	StartWritePOF(char* NameFile, uint NumPage);
	virtual void	StartWriteM10(char* NameFile1, char* NameFile2, uint NumPage);
	//virtual void	StartWriteRPD(char* NameFile, uint NumPage);
	virtual void	StartReset(uint NumPage);
	virtual int		Idle();
	virtual void	StopThreadIdle();
	virtual void	StartWrReg(uint Addr, uint Data);
	virtual void	StartRdReg(uint Addr);
	//virtual bool	ReadyRdReg(uint &Data);	// folse - not ready; true - ready
*/
};

// Диалоговое окно CSoundServerDlg
class CSoundServerDlg : public CDialogEx
{
// Создание
public:
	uint32_t			DataCnt;

	uint16_t			MaxItem;
//	ElUDPCli*			ElUDPHUB;


	sockaddr_in			AddrToStepper;
	AudioStationPack		ASPack;
	AudioLocatorSystemInfo	ALSInfo;
	int						Fl_Ready;
	char	StrOut[2048*50];
	int		HtmlLen;
	char	HtmlStr[2048*50];

//	int		flTest;

	char					PathNameLANStationCfg[256];
	char					PathNameWANStationCfg[256];
	char					LogBuf[20480];
	CListCtrl		*m_ListUDP;
//	CListCtrl		*m_ListWAN;
	CUserCfg		*UCfg;
	uint			ActiveSt;
	uint			StNum;
	int				SrvLANNumCli;
	int				SrvWANNumCli;
	int				SrvWEBLNumCli;
	int				SrvWEBWNumCli;
	int				WEBLNumPack;
	int				WEBWNumPack;
	IPElem			*IPElLAN;
	IPElem			*IPElWAN;
	ID_List8		LANStList;				// Список станций по уникальному идентификатору
	ID_List8		LANCliList;			// Список клиентов сервера по номеру клиента
//	ID_List8		WANStList;				// Список станций по уникальному идентификатору
//	ID_List8		WANCliList;			// Список клиентов сервера по номеру клиента

	int				Fl_UpDataAppLAN;
	StElem			*UpDAppStElFLAN;
	int				Fl_UpDataSmStLAN;
	StElem			*UpDSmStStElFLAN;
	int				Fl_UpDataFWLAN;
//	StElem			*UpDFWStElFLAN;
//	StElem			*ActiveStElLAN;
	DevElem			*ActiveDevEl;
	uint			Fl_WiatingAnswerLAN;
	int				AppUpDateHendleLAN;
	int				SmStUpDateHendleLAN;
	int				FWUpDateHendleLAN;
	UINT64			AppFile_TIDLAN;
	UINT64			SmStFile_TIDLAN;
	UINT64			FWFile_TIDLAN;
	uint			WaitingCntLAN;
	uint			UpLTmpDataLenLAN;
	uint			UpLFile_SizeLAN;

	int				Fl_UpDataAppWAN;
	StElem			*UpDAppStElFWAN;
	int				Fl_UpDataSmStWAN;
	StElem			*UpDSmStStElFWAN;
	int				Fl_UpDataFWWAN;
	StElem			*UpDFWStElFWAN;
//	StElem			*ActiveStElWAN;
	uint			Fl_WiatingAnswerWAN;
	int				AppUpDateHendleWAN;
	int				SmStUpDateHendleWAN;
	int				FWUpDateHendleWAN;
	UINT64			AppFile_TIDWAN;
	UINT64			SmStFile_TIDWAN;
	UINT64			FWFile_TIDWAN;
	uint			WaitingCntWAN;
	uint			UpLTmpDataLenWAN;
	uint			UpLFile_SizeWAN;
	char			Fl_UPD;
	long			UpDFile_Size;
	uint			UpDTmpDataLen;
	uint			ModeWork;
	int				File_BlockLen;
	unsigned char	UPD_Data[5];
	unsigned char	File_Data[UPDBlockSize];
	CRect			RectTxt;
	GraficM*		m_pMySV0;
	GraficM*		m_pMySV1;
	uint8_t			Terminator_State;

	ClassThread		*SrvSockLAN_Thread;
//	ClassThread		*SrvSockWAN_Thread;
//	ClassThread		*SrvSockWEBL_Thread;
//	ClassThread		*SrvSockWEBW_Thread;
//	ClassThread		*Boost_Thread;

	virtual	int	ErrorLAN(int NumError);
//	virtual	int	Boost_Idle(void);
	virtual	int	SrvSockLAN_Idle(void);
//	virtual	int	SrvSockWAN_Idle(void);
//	virtual	int	SrvSockWEBL_Idle(void);
//	virtual	int	SrvSockWEBW_Idle(void);
	virtual	int	LANParserPacket(IPElem* IPEl, UCHAR* Buf, uint Len);
	virtual	int LANSendPacket(uint NumCli, AudioStationPack* APack);
	virtual	int LANStDisConnect(int NumCli);
//	virtual	int	ErrorWAN(int NumError);
//	virtual	int	WANParserPacket(IPElem* IPEl, UCHAR* Buf, uint Len);
//	virtual	int WANSendPacket(uint NumCli, AudioStationPack* APack);
//	virtual	int WANStDisConnect(int NumCli);
//	virtual int PrepareResponse();

	CSoundServerDlg(CWnd* pParent = nullptr);	// стандартный конструктор
	~CSoundServerDlg();	// стандартный конструктор

// Данные диалогового окна
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_SOUNDSERVER_DIALOG };
#endif

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// поддержка DDX/DDV
	

// Реализация
protected:
	HICON m_hIcon;

	// Созданные функции схемы сообщений
	virtual BOOL OnInitDialog();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnLvnItemchangedList1(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnEnChangeMfceditbrowse1();
	afx_msg void OnEnChangeMfceditbrowse2();
	afx_msg void OnEnChangeMfceditbrowse3();
	afx_msg void OnEnChangeEdit2();
	afx_msg void OnEnChangeEdit1();
	afx_msg void OnBnClickedCancel();
	afx_msg void OnTimer(UINT_PTR nIDEvent);
	afx_msg void OnEnChangeEdit3();
	afx_msg void OnEnChangeEdit4();
	afx_msg void OnBnClickedButton1();
	afx_msg void OnBnClickedButton2();
	afx_msg void OnEnChangeEdit5();
	afx_msg void OnBnClickedButton3();
	afx_msg void OnEnChangeEdit6();
	afx_msg void OnLvnItemchangedList2(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnEnChangeEdit7();
	afx_msg void OnBnClickedButton6();
	afx_msg void OnBnClickedButton4();
	afx_msg void OnBnClickedButton5();
	afx_msg void OnEnChangeEdit8();
	afx_msg void OnBnClickedCheck1();
	afx_msg void OnBnClickedCheck2();
	afx_msg void OnEnChangeEdit11();
	afx_msg void OnItemactivateList1(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnBnClickedButton8();
	afx_msg void OnBnClickedCheck5();
	afx_msg void OnBnClickedCheck4();
	afx_msg void OnBnClickedButton7();
	afx_msg void OnBnClickedButton9();
	afx_msg void OnBnClickedButton10();
	afx_msg void OnBnClickedCheck3();
	afx_msg void OnBnClickedButton11();
	afx_msg void OnBnClickedButton12();
	afx_msg void OnBnClickedButton13();
	afx_msg void OnBnClickedButton14();
	afx_msg void OnBnClickedButton15();
	afx_msg void OnBnClickedButton17();
	afx_msg void OnBnClickedButton18();
	afx_msg void OnBnClickedCheck9();
	afx_msg void OnBnClickedCheck10();
	afx_msg void OnBnClickedButton19();
	afx_msg void OnBnClickedCheck6();
	afx_msg void OnBnClickedCheck7();
	afx_msg void OnBnClickedCheck8();
	afx_msg void OnBnClickedButton20();
	afx_msg void OnCbnSelchangeCombo1();
	afx_msg void OnBnClickedCheck11();
	afx_msg void OnBnClickedCheck12();
	afx_msg void OnBnClickedButton21();
	afx_msg void OnBnClickedButton22();
	afx_msg void OnBnClickedButton16();
};
