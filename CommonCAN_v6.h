#ifndef _COMMONCAN_V6_H_
#define	_COMMONCAN_V6_H_

#define STANDARD_FORMAT  0
#define EXTENDED_FORMAT  1

#define PAGE_SIZE       	0x00000200
#define mPAGE_SIZE       	128

#define		SizePackBuf			4

#define	ToAll							0x1
#define	ToHUB							0x2
#define	ToExtSrv						0x3	// Outside the CAN
#define	ToTmpHost						0xff
#define	ToHost							250	// >250

#define	ALLCLI							0xff	// For many clients network


#define	TypeNetMask				0x7
#define	CANCliWarningMsk	0x8
#define	TimeMask					0xc0
#define	TimeDec						0x40

#define Syn1		0x5a	// Packet
#define Syn2		0xa5	// Ok
#define Syn3		0xaa	// Error


typedef	struct	CliListStr
{
	uint8_t	NetAddrFrom;	// NetAddrFrom - D7-D5 TypeNet D4-D0 NumCli
	uint8_t	Atribute;			// #define	TimeMask 0xc0 ...
} CliListStr;

typedef enum
{
	DATA_FRAME  =     0,
	REMOTE_FRAME,
} CAN_Type;

typedef enum
{
	IAM_CLIENT = 1,
	FIND_SERVER,
	SET_TMPID,
	CHNG_TMPID,
	REINIT_DEVICE,
	GET_FULLID,
	MY_FULLID0,
	MY_FULLID1,
	MY_FULLID2,
	DEV_ERROR,
	SEND_PACKHEADER,		// 11 0x0b
	SEND_PACKDATA,
	SEND_LASTPACKDATA,
	RELIASE_SERVER,
	MSG_OK,		//	15 0x0f
	MSG_ERROR,	//16
	PACK_OK,
	PACK_ERROR,	// 18
	FINDDEVICEBY_UUID,
	MYTID,	//20
	SEND_OUTPUTVOL,
	SEND_EVENTON,
	REQUEST_TIME,		// 23
	SET_TIME,
	GET_ACTIVESNS,	// 25
	SEND_SNS,
	GET_ACTIVEOUT,
	SEND_OUT,
	READY_ToBLUpData,
	READY_ToFWUpData,
	READY_BootLoader,
	GOTO_FWUpData,		// 32
	rPACKHEADER_READY,
	GET_FWInformation,		//	UpData
	POST_FWInformation,		//	UpData
	GET_BLInformation,		//	UpData
	POST_BLInformation,		//	UpData
	POST_FWCRC32,
	POST_FWSize,
	POST_BLCRC32,
	POST_BLSize,
	rSEND_SIMPLEHEADER,		// 42
	rSEND_SIMPLEDATA,
	rSEND_SIMPLELASTDATA,
	rSEND_SIMPLEDATA_OK,		// 45
	rSEND_SIMPLEDATA_ERROR,
	SEND_PING,
	ANSW_PING,
	REQUEST_FirmWare,
	REQUEST_BootLoader,		// 50
	ERROR_FWUpData,
	ERROR_BLUpData,	//52
	BOOTLOADER_STATE,
	SEND_WARNINGON,
	SEND_WARNINGOFF,
	SEND_EVENTOFF,	//56
	SEND_WARNINGRead,
	READY_StartFWUpData,
	READY_StartBLUpData,
	SET_REALTIMEACTIVESNS,
	RESET_REALTIMEACTIVESNS,
	GET_ROMID,
	SEND_ROMIDL,
	SEND_ROMIDH,
	I_SEE_YOU,

	SEND_Meta,
	SEND_EventMeta,
	SEND_Compass,
	SEND_Level,
	SEND_GPS_Lat,
	SEND_GPS_Lng,
	SEND_GPS_Alt,
	SEND_GPS_UTC,
	SEND_GPS_Stat,
	ADC_Stream_Control,
	SEND_ADC_Stream,
	SEND_EventTime,
	DO_AG_CALIBRATION,
	DO_Q_CALIBRATION,
	DO_T_CALIBRATION,
	SET_DEBUG,
	RESET_DEVICE,
	UPD_BEGIN,
	UPD_DATA,
	UPD_END,
	SET_MODE,
	REQUEST_VERSION,
	SEND_VERSION,
	SET_CANSPEED,
	Test_CMD,
	SEND_TrgType,
	Terminaor_Switch
} CAN_Cmd;

#pragma pack(push, 1)

typedef struct CANMsgStr
{
  unsigned int   id;          			// TIDTo       // 29 bit identifier
  unsigned char  dat[8];           // Data field
  unsigned char  len;                // Length of data field in bytes
  unsigned char  format;             // 0 - STANDARD, 1- EXTENDED IDENTIFIER
  unsigned char  type;               // 0 - DATA FRAME, 1 - REMOTE FRAME
//	unsigned char  NetAddrFrom;				 //
} CANMsgStr;

#pragma pack(pop)

typedef struct WLMsgStr
{
	uint8_t		Syn;
	uint8_t		crc8;
	CANMsgStr	Msg;
}	WLMsgStr;

typedef struct
{
	uint16_t	len;
	uint8_t		crc8;
	uint8_t		idp;
} CAN_PackHeader;

typedef enum
{
	TN_UnDefine=0,
	TN_LAN,			// 1	0x20
	TN_WiFi,		// 2	0x40
	TN_GPRS,		// 3	0x60
	TN_WiLess,	// 4	0x80
	TN_CAN1,		// 5	0x90	
	TN_CAN2,		// 6	0xc0
	TN_HUB,			// 7	0xd0		 -????
//	TN_ALLEXT,
//	TN_ALLINT,
//	TN_ALL
} TypeNet;

typedef enum
{
	CAN_Pack=1,		// CAN_pack
	Set_Time,
	Answ_OK,				// ????????????? ??????? ????????
	Sys_WDog,				// System WatchDog
	Export_Start,			// ?????? ???????? ????????????
	Send_TaskFunction,		// ???????? ?????? TaskFunction
	Send_TaskOutput,		// ???????? ?????? TaskOutput
	Send_TaskInput,			// ???????? ?????? TaskInput
	Send_TaskSensor,		// ???????? ?????? TaskSensor
	Send_TaskExecution,		// ???????? ?????? TaskExecution
	Send_TaskLinkToEvent,	// ???????? ?????? TaskLinkToEvent
	Send_TaskEvent,			// ???????? ?????? TaskEvent
	Send_TaskTimeCod,		// 
	Send_TaskUserScreen,		// 
	Send_TaskUserMenu,			// 
	Send_TaskWarning,			// 
	Send_TaskAlarm,			// 
	Send_NumElement,		// ???????? ???? ?????????? Num
	Export_Stop,			// ????? ???????? ???????????? (?????????? ????? ??????????)
	Get_Sensor,			// 20
	Send_Sensor,
	Get_Output,
	Send_Output,
	Send_FWUpData,
	Send_Reset,			// 25
	Send_SysSetting,
	Answ_Error,			// 27
	Get_CliList,
	Send_CliList,
	Answ_Absent,	// 30
	WiLess_CANMsg,
rezWiLess_Ok,				// 32
	WiLess_Pack,		// 33
	WiLess_EtherPack,	// 34
rezWiLess_HostPack,	// 35
	Set_LANIP,
// Работа с файлами всегда работаем только с отним файлом (пока)
	FS_ReSet,	// 
	FS_DIR,		// Прочитать каталог
	FS_CHDIR,	// Изменить каталог
	FS_Open,	// 40  Открыть файл
	FS_Read,	// Прочитать из файла
	FS_Seek,	// Установить указатель чтения
	FS_Close, // Закрыть файл
	FSA_Status, // Передат статус файловой операции
	FSA_DIR,	// Прередать прочитанный каталог
	FSA_ReadData,	// Прередать блок прочитанных данных
	FS_DelDir,		//
	FS_DelFile,	//
	Menu_GetInfo,	// -????????? ?????????? ? ?????????? ???? ??????????: ??????????, ????????????, ??? ? ???????? ??????? ?????? ????
	Menu_GetState,	// -????????? ????????? ?????? ???? ??????????
	Menu_SetState,	// -???????? ????????? ?????? ???? ??????????
	Menu_SendInfo,	// -???????? ?????????? ? ?????????? ???? ??????????: ??????????, ????????????, ??? ? ???????? ??????? ?????? ????
	Menu_SendState,	// -???????? ????????? ?????? ???? ??????????
	UserScr_GetInfo,	//
	UserScr_SendInfo,	//
	Get_UserSensor,
	FS_Create,
	FS_Write,
	FS_ReNameFile,	//
	FS_TestCRC,			//60
	Answ_UnKnown,
	Send_SysWarning,
	Get_SysWarning,
	Send_SiteID,
	Get_SiteID,	
	Set_LANMAC,
	Send_LANMAC,
	Get_LANMAC,	
	Get_Version,	//	69
	Send_Version,	//	70
	Get_TaskFunction,
	Get_TaskOutput,
	Get_TaskInput,
	Get_TaskSensor,
	Get_TaskExecution,
	Get_TaskLinkToEvent,
	Get_TaskEvent,
	Get_TaskTimeCod,
	Get_TaskScreen,
	Get_TaskMenu,	//80
	Get_TaskWarning,
	Get_TaskAlarm,
	Get_NumElement,	
	FS_Format,
	SYS_FactoryReSet,
	WiLess_Param,
	HOST_RequestToFWUpData,
	HOST_FWUpDataPack,
	HOST_FWUpDataCheck,
	Request_Identification,	//90
	Send_IdentificationID,
	Post_IdentificationID,
	Set_IdentificationID,
	
	Set_RealTimeVariable,		//94	до 16 переменных разных устройств
	ReSet_RealTimeVariable,	// 95
	HOST_RequestToBLUpData,
	HOST_BLUpDataPack,
	HOST_BLUpDataCheck,
	Set_LANPort,		// 99
	Set_TelemetryFlag,	// 100
	Set_SD_DiskFlag,
// New	
	Send_InputPackStret,
	Get_DevSystemSetting,
	Send_DevSystemSetting,
	Save_DevSystemSetting,
	Get_PreviousSMS,
	Get_NextSMS,
	Send_SMS,
	Del_ActiveSMS,
	Reset_1WireList,	// 110
	Get_1WireList,
	Send_1WireList,
	Set_WiFiFlag,
	Set_LANFlag,
	Set_WiLessFlag,
	Set_GSMFlag,
	Set_WiFiIP,
	Set_WiFiPort,
	WiLess_MsgInputPackStrHeader,
	WiLess_MsgInputPackStrData,			// 120
	WiLess_MsgInputPackStrEnd,
	Send_LongPacket,
	Msg_Pack,												// PackStr
	ERROR_MSG,
	DEVICE_INFO,
rez_GoToFWUpData,
RezReadyToFWUpData,
	RequestToFWUpData,	// 
	FWUpDataPack,
	FWUpDataCheck,				// 130
	Rez_ReadyToBLUpData,
	RequestToBLUpData,	// 
	BLUpDataPack,
	BLUpDataCheck,	// 
	Rez_GetFWInformation,
	Rez_PostFWInformation,
	Rez_GetBLInformation,
	Rez_PostBLInformation,
	Rez_ReadyBootLoader,
	Rez_SendNetToDevPacket,	// 140
	Rez_SendDevToNetPacket,	// 
	SendBLHeader,
	SendBLData,
	SendBLCheck,
	SendFWHeader,
	SendFWData,
	SendFWCheck,		// 147
// --- New ---	
	UnBindApp,						//	148
	BindApp,							//	149
	Get_ROMID,						//	150
	Set_ROMID,						//	151
	Send_ROMID,						//	152
	Set_SwitchOn,					//	153
	Set_SwitchOff,				//	154
	Set_TurnOffForAWhile,	//	155
	Get_Status,						//	156
	Send_Status,					//	157

} TypeCommand;


typedef struct	DevSystemSetting
{
	uint8_t		tTID;
	uint8_t		DisplayMode;				// 0 - Allways Off  1 - UserScreen 2 - UserMenu 3 - IngeneeringScreen 4 - IngeneeringMenu 
	uint8_t		EngeneeringMode;		// 0 -NotUsed 1-OldVresionDevice
	uint8_t		FlagWiFiUsed;				// 0 - Off 1 - WiFiModem 2 - WiFiAccessPoint 3 - WireLess 4 - OM-310 5 - PLC KQ330
	char			WiFiNetworkName[16];
	char			WiFiPassWord[16];
	uint32_t	CANSpeed;	
	char			DebugMode;
	uint8_t		HomeSrvIP[4];	
	uint8_t		InitFlag;
	uint32_t	USARTSpeed;
// New elements
	float			TemperatureCorr;
// ------------------------------	Rez 32 
	uint8_t		FlagWarning;				// 0 - Not Used 1 - Only Critical 2 - All Warning
	uint8_t		WiLessCh;			// Number	1 - 127
	uint8_t		WiLessPw;			// Power 1-8
	uint8_t		TimeCodeType;	// Two types TimeCode mode 0 - new (Interval set) 1 - old (Event set)
	uint16_t	WiFiPort;
	uint8_t		WiFiRole;			// 0-Dev 1-HUB
	char			BufRez[32-4-2-1];
}	DevSystemSetting;


typedef struct	WLCli
{
uint8_t	 TID;					
} WLCli;

typedef struct	FWHeader
{
uint32_t	FWVersion;
uint32_t	Size;
uint32_t	FullCRC;
uint8_t		DeviceID;
}FWHeader;

typedef struct
{
	uint32_t	Address;
	uint16_t	Len;
	uint16_t	Num;
	uint8_t		Data[mPAGE_SIZE];	
} FWPack;

typedef struct
{
uint32_t	BLVersion;
uint32_t	Size;
uint32_t	FullCRC;
uint8_t		DeviceID;
} BLHeader;

typedef struct
{
	uint32_t	Address;
	uint16_t	Len;
	uint16_t	Num;
	uint8_t		Data[mPAGE_SIZE];	
} BLPack;

typedef struct	PackHeader
{
	uint16_t	Command;
	uint16_t	DataLen;
	//	uint8_t		Rez;	
	//	uint8_t		TIDTo;
	//	uint8_t		TIDFrom;
	//	uint8_t		CRC8;
}	PackHeader;

typedef struct	PackStr
{
	PackHeader	PHeader;
	uint8_t		Data[MaxLenPack];
}	PackStr;

typedef struct	InputPackStr
{
	uint8_t					Fl_Used;
//	uint16_t				Size;
//	uint16_t				CRC16;
	uint8_t					OwnerTID;
	uint8_t*				pBufInPack;
	uint16_t				Index;
	CAN_PackHeader	InpPHeader;	
	PackStr					Pack;
}	InputPackStr;
/*
typedef struct	InputPackStr
{
	CAN_PackHeader	InpPHeader;
	uint8_t*	pBufInPack;
	uint8_t		Flag;
	uint8_t		OwnerTID;
	uint8_t		Len;
	uint8_t		CntPack;
	uint8_t		Data[CANMaxLenPack];
}	InputPackStr;
*/
typedef	struct SockHeaderSmall
{
	uint8_t		Syn;
	uint8_t		CntPack;
	uint16_t	Len;
} SockHeaderSmall;

typedef	struct WiLessAnswStr
{
	SockHeaderSmall	SHeader;
	uint8_t					Answ;
} WiLessAnswStr;

/*
typedef struct	PackHeader
{
	uint8_t		ID_Command;	//	LANCommand
	uint8_t		TIDTo;
	uint16_t	DataLen;
	uint8_t		CRC8;
	uint8_t		NetAddrFrom;	//D7-D5-Type Net D4-D0-NumCli	!!!
	uint8_t		TIDFrom;	
	uint8_t		Rez;	
}	PackHeader;
//
typedef	struct HostPackStr
{
	PackHeader	PHeader;
	uint8_t			Data[MaxLenPack];
} HostPackStr;
*/
typedef	struct EtherPackStr
{
//	uint16_t		Len;
	SockHeaderSmall	SHeader;
	PackStr					Pack;
} EtherPackStr;

typedef	struct	WatchDogPack
{
	SockHeaderSmall	SHeader;
	PackHeader			PHeader;
} WatchDogPack;
#endif
