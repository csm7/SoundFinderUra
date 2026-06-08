#ifndef _CommonCom
#define _CommonCom

#pragma once


#include <stdio.h>

//#define UINT64 unsigned INT64

#undef _ITERATOR_DEBUG_LEVEL
#define	FirstPrm	10000
#define LastPrm		20000

struct	Str_Spectrum
{
	float	Spe[128];
};

struct	SysError
{
	INT64	UID;			//
	char	Name[64];		//
};

SysError* GetSysErrorByID(UINT64 ID);
char* GetErrorTxtByID(UINT64 ID);

enum	StatUID		// Статические значения уникальных идентификаторторов
{
		NullID=0,
// В этом разделе должны быть перечислены идентификаторы таблиц
		TabNeuronGlobal,		//	Èäåíòèôèêàòîð ñèñòåìíîé òàáëèöè Ñåðâåðà äåêîäèðîâàíèÿ
		TabNeuronLocalCustomer,	//	Èäåíòèôèêàòîð òàáëèöû ñèñòåì ñâÿçè
		TabNeuronLocalSite,		//	Èäåíòèôèêàòîð òàáëèöû ñèñòåì ñâÿçè
//		TabNeuronLocalDomain,	//	Èäåíòèôèêàòîð òàáëèöû àáîíåíòîâ ñèñòåì ñâÿçè
		TabParametr,			//	Èäåíòèôèêàòîð ñèñòåìíîé òàáëèöè Ñåðâåðà äåêîäèðîâàíèÿ ñîäåðæàùåé îïèñàíèå ïàðàìåòðîâ èñïîëüçóåìûõ â IDInfo òåõîëîãèè

// В этом разделе должны быть перечислены параметры, которые используются в технологии IDInfo
// Новые параметры должны добавляться ТОЛЬКО В КОНЕЦ СПИСКА
	
// ==== Базовые параметры (значения которых меняться не могут) ===
		IDNameElTab=FirstPrm,		//	Текстовое наименование елемента таблицы
		IDPrmType,			//	Тип параметра
		IDPrmIndex,			//	Индексация параметра
		IDPrmRegularity,	//	Уникальность параметра
		IDPrmStatus,		//	Статус параметра
		IDPrmSize,			//	Размер параметра (если он ползовательский)
		IDPrmView,			//	Способ отображения параметра
		EndFirstPrm,
// -------------------------------------------------------------------------
		IDProtocol=LastPrm,	//	Параметр в котором указывается строковое значение обозначающее название протокола обработки данных
		IDFormat,			//	Параметр в котором указывается строковое значение обозначающее название формата представления данных
		IDIPFrom,			//	IP адрес отправителя в формате	INT	
		IDIPTo,				//	IP адрес получателя в формате	INT	
		IDPortFrom,			//	значание порта источника TCP сессии в формате INT
		IDPortTo,			//	значание порта приемника TCP сессии в формате INT
		IDFileType,			//	Тип файла
		IDFileLen,			//	длинны файла
		IDFileExt,			//	расширение файла
		IDFileName,			//	имя файла
		IDContentType,		//	тип содержимого файла
		IDPayloadType,		//	тип содержимого файла для медиаданных
		IDFSrvName,			//	имя файлового сервера на котором записан файл
		IDFlagCRC,			//	Признак наличия в пакете CRC
		IDID,				//
		IDIDFileSnsFr,		//
		IDIDFileSnsTo,		//
		IDIDFileParent,		//
		IDMtName,			//
		IDMtAuth,			//
		IDMtVer,			//
		IDIDSession,		//
		IDIDStream,			//
		IDNumberFrChan,		//
		IDMode,				//
		IDFd,				//
		IDNPhChan,			//
		IDFrequencyRange,	//
		IDIDGroop,			//
		IDS_N,				//
		IDTDMA_FN,			//
		IDTDMA_SFN,			//
		IDTDMA_MFN,			//
		IDIDLogChannel,		//
		IDIDDemChannel,		//
		IDFrequency,		//
		_IDUnitID,			//
		IDADC,				//
		IDProcTCPIP,		//
		IDProcUnknUDP,		//
		IDIDParStream,		//
		IDIODLCAddr,		//
		IDIODLCSession,		//
		IDCallNumberFrom,	//
		IDCallNumberTo,		//
		IDSIPProxyAuthFrom,	//
		IDSIPProxyAuthTo,	//
		IDSIPFrom,			//
		IDSIPTo,			//
		IDHDLC_All,			//
		IDHDLC_ErrSize,		//
		IDHDLC_OthHDLC,		//
		IDHDLC_ErrIP,		//
		IDCRC_OK,			//
		IDCRC_ERR,			//
		IDHDLC_TCP,			//
		IDHDLC_UDP,			//
		IDQuality,			//
		IDFileInfo,			//
		IDDVB_PID,			//
		IDService,			//
		IDUIDICLList,		//
		IDProfile,			//
		IDIDProfile,		//
		IDTypeMsg,			//
		IDOriginalNR,		//
		IDCallingNR,		//
		IDConnectNR,		//
		IDRedirctingNR,		//
		IDLocationNR,		//
		IDRedirctionNR,		//
		IDCalledNR,			//
		IDTimeSlot,			//
		IDSlotSelect,		//
		IDCarrierBand,      //
		IDHeterodyneLBand,  //
		IDCarrierLBand,		//
		IDCarrierDR,		//
		IDCarrierSpan,		//
		IDFrequencyLower,	//
		IDFrequencyUpper,	//
		IDDTXChan,			//
		IDRemark,			//
		IDDPC,				//
		IDOPC,				//
		IDCIC,				//
		IDPriority,			// IDInfo->Priority
		IDStreamList,		//
		IDGroupList,		//
		IDTypePacket,		//
		IDBufferiz,			//
		IDModeSDec,			//
		IDNDemChan,			//
		IDTopUID,			//
		_IDHostName,			//
		IDMonSystem,		//
		IDRandomInf,		// Thuraya parameter
		_IDUnitName,			// Параметр определяет имя юнита которому адресуется команда
		IDTypeCli,
		IDFullNameUp,		// Thuraya parameter
		IDFullNameDn,		// Thuraya parameter
		IDFullNameDnVoc,	// Thuraya parameter
		IDFullNameUpVoc,	// Thuraya parameter
		IDSpotID,			// Thuraya parameter
		IDSatLatitude,		// Thuraya parameter
		IDSatLongitude,		// Thuraya parameter
		IDRadius,			// Thuraya parameter
		IDBeamLatitude,		// Thuraya parameter
		IDBeamLongitude,	// Thuraya parameter
		IDSatID,			// Thuraya parameter
		IDSMS,				// Thuraya parameter
		IDEstablishCause,	// Thuraya parameter
		IDLatitude,			// Thuraya parameter
		IDLongitude,		// Thuraya parameter
		IDIMSI,				// Thuraya parameter
		IDTMSI,				// Thuraya parameter
		IDIMEI,				// Thuraya parameter
		IDIMEISV,			// Thuraya parameter
		IDtempTMSI,			// Thuraya parameter
		IDNCSC_Les,			//
		IDNCSC_Present,		//
		IDNCSC_Mes,			//
		IDNCSC_Adress,		//
		IDNCSC_AdressType,	//
		IDNCSC_SubAdress,	//
		IDUnit,				//
		IDNetSource,		//
		IDInput,			//
		IDStream,			//
		IDProcessed,		//
		IDIDEntryPoint,		// 
		IDTDMA_Sin,			//
		IDIDSeans,			//
		IDElevation,
		IDDeviceType,		// Device Type (Antenna, MUX, splitter, e.t.)
		IDString,			// Строка для IDI_Tester (для симуляции записи параметров)
		IDTimeBegin,		// IDInfo.TimeBegin= Время создания оъекта IDInfo с уникальным UID (связано с событием Begin)
		IDTimeLastWr,		// IDInfo.TimeLastWr= Время МОДИФИКАЦИИ параметров и Buf оъекта IDInfo = время доступа по записи.
		IDTimeClose,		// IDInfo.TimeClose= Время окончания (связано с событием End). TimeLastWr может > TimeClose.
		IDCommand,			// IDInfo.IDCommand= клоны статических полей структуры IDInfo 
		IDStateBlock,		//
		IDTypeIDI,		    //
		IDNumPack,			//
		IDNumSyn,			//
		IDNumSynAll,		//
		IDDataTraffic,		//
		IDControlParam,		//
		IDLoading,			//
 		IDTrSpead,
		IDBufCnt,			// *IDInfo.Buf содержимое длиной IDInfo.Cnt (c)Mao
		IDFile,
		IDAGC,
		IDDeltaFc,
		IDNCSend,			// Parameter in NetComm for set flag send     //"All"-передавать все данные,"Begin_End"-only Begin End,"End"-only End,
		IDThur_SeansID,
		IDID_Standart,		// *IDInfo.ID_Standart статический параметр
		IDStateProc,		// *IDInfo.ID_Hard статический параметр
		_IDNCStrategic,		// Parameter in NetComm for string name strategic  ((Alternate-поочередно(Default)),1-Set-набор,2-All(всем),3-Remote(удаленн),4-Nothing)
		IDNCProfile,		// Parameter in NetComm for string name profile 
		IDProgress,
		IDCondition,		// Аргумент цифрового автомата для обозначения этапов обработки и номера условий
		IDCloseReason,		// Причина закрытия файла или сеанса
		IDWeight,			// Весовой коэфф. при отборе
		IDConfig,
		IDProcessingState,	// Состояние обработки 0-Off(Запр. сост.) 1-Ok 2-Error 3-NoData
		IDBuffQBE,			// Параметр для хранения qbe
		IDTimeRdAcces,		// IDInfo.OperTime Оперативное время = время доступа на ЧТЕНИЕ к параметрам и Buf структуры IDInfo (OnlineTime).
		IDOffsetBFile,		// Смещение в байтах относительно начала файла
		IDNCSendBuff,		// Parameter in NetComm for set send content(data buff)     //"All"-передавать все данные,"Without_Data"-без данных буфера,"Without_Data_for_Sessions"-не передавать данные только у сеансов,
		IDNCSendTypeIDI,	// Parameter in NetComm for set send IDI filter TypeIDI     //"All"-передавать все данные,"FILE","SEANS","LOG","COMMAND","ANSWER","TABLE"- передавать IDI только выбранных типов
		IDSeansNumFile,		// Кол-во сохраненных файлов (после фильтрации)	
		IDSeansNumFileAll,	// Общее кол-во файлов в сеансе
		IDProfileCS,
		IDBuffCS,
		IDProfileQBE,
		IDMaxQueryCnt,		// Параметр устанавливающий размер порции данных удовлетворяющих запросу
		IDMailFrom,
		IDMailTo,
		IDMyHostName,		// Параметр определяет имя хоста отправителя команды
		_IDMyUnitName,		// Параметр определяет имя юнита отправителя команды
		IDSelection,
		IDOpenFile,
		IDOpenFile_1s,
		IDOpenFileSF_1s,
		IDCloseFile_1s,
		IDCloseFileSF_1s,
		IDElapsedTime,
		IDMethodProcessing, // Имя метода-обработчика
		IDMethodAnalyzing,  // Имя метода-анализатора
		IDIDEntryPointNext, // 
		IDNumberInputs,		// Number of device Inputs 
		IDNumberOutputs,	// Number of device Outputs
		IDOutput,
		IDIPAdr,			// IP adress devices
		IDDevice,
		IDModeCRC,
		IDCoordinateX,		//координатами (x) tCoordinate
		IDCoordinateY,		//координатами (y) tCoordinate
		IDFunctionType,		// Function type (Source, Conductor, Consumer, e.t.)
		IDPort,				// Number port
		IDDeviceFrom,		// Nаме Output Device 
		IDDeviceTo,			// Name Input Device
		IDHost,
		IDIntInput,
		IDExtInput,
		IDExtOutput,
		IDEntryPointName,
		IDEntryPointNameSel,
		IDConvSpeed,		
		IDFrequencyHeterodyne,	//
		IDModulationType,	
		IDSymbolRate,	
		IDFrequencyRF,	
		IDFrequencyIF,	
		IDFrequencyLowerOut,	//FrequencyLower on output
		IDFrequencyUpperOut,	//
		IDBandwidth,		//
		IDBitRate,
		IDLocked,
		IDParameterSet,
		IDBeam,
		IDBearerType,
		IDPolarization,		//
		IDSampleTime,		//
		IDGainLevel,		//
		IDPolynome1,		//
		IDPolynome2,		//
		IDECCType,			//
		IDZeroFrequency,	//
		IDInvert,			//
		IDNTaps,			// Param For DigitalRec 
		IDVirtChan,			// 
		IDFrequencyLowerIn,	//FrequencyLower on output
		IDFrequencyUpperIn,	//
		IDEp,				// Мгновенная спектральная енергия сигнала
		IDMAC_Address,			// MAC adress
		IDStep,				// Step
		IDTimeout,			// Timeout
		IDScramblerType,	// ScramblerType
		IDProfileFilter,	// ProfileSelection
		IDInterleaving,		// Interleaving
		IDIntCommand,
		IDState,
		IDPersonalID,
		IDFc,
		IDSource,
		IDParameter,
		IDCriterion,
		IDTimeoutProc,			// Timeout processing
		IDTimeoutTuning,		// Timeout tuning
		IDSpectrum,
		IDSignalLevel,
		ID_K11,
		ID_K12,
		ID_Kea1,
		ID_Kea2,
		ID_KuP1,
		ID_KuP2,
		IDLNB_Power,
		IDPeriod,
		IDTransponder,	
		IDSyn,	
		IDSynLen,	
		IDPeriodPrime,
		IDCountryNameFrom,	
		IDCountryNameTo,	
		IDCountryCodeFrom,	
		IDCountryCodeTo,	
		IDRole,
		_IDNativeHostName,		// Параметр определяет имя хоста отправителя команды
		_IDNativeUnitName,		// Параметр определяет имя юнита отправителя команды
		IDInterception,
		_IDMyPersonalID,
		IDCreatingType,	
		IDAttribute,
		IDEPNameMerging,
		IDEPNameDuplexing,
		IDEnoise,
		IDZeroFrequencyIn,
		IDZeroFrequencyOut,
		IDAnalyzeModeName,
		_IDNativeDomainName,	
		_IDDomainName,	
		_IDUnitMode,	
		IDDBManName,
		IDMessageFrom,
		IDMessageTo,
		IDMessage,
		IDLocal,
		IDPathStorage,
		IDOwner,
		IDTuner,
		IDScanner,
		IDNetViewProcessed,
		IDGainLevel1,
		IDGainLevel2,
		IDGainLevel3,
		IDRealHost,
		IDMaxSize,
		IDTypeView,
		IDContainerType,
		IDRF_Amp,
		IDRF_Att,
		IDCountError,
		IDSaveMode,
		IDProfileSchema,
		IDRepeat,
		IDReadSpeed,
		IDNoDelete,

		IDAzimuth,
		IDdAzimuth,
		IDdElevation,
		IDProcessingHost,
		_IDNativeUnitlID,

		IDMCC,
		IDMNC,
		IDMCC_MNC,
		IDNameOperator,

		IDFlagDeleteDB,

		IDPilottone,
		IDNetType,

		IDNumberDaysStorage,//number of days of storage
		IDBuffDiz,		//info for diz-file
		IDN_Pin,
		IDNameDB,
		IDNameDBDel,
		IDInformationType,
		IDStrOwner,
		IDDCMEType,
		IDNameSatellite,

		IDFrame_Syn,
		IDFrame_Length,
		IDFrame_Pause,
		IDSuperFrame_Length,
		IDSuperFrame_Pause,
		IDSuperFrame_FrameNumber,
		IDAddressInfo,
		IDHardwareType,
		IDErrorID,
	    IDReverse,
		IDBitPerSample,
		IDBufferingTime,
// Набор новых параметров по системе управления процессами
		IDGroupName,
		IDUserLogin,
		IDUserFirstName,
		IDUserLastName,
		IDUserPassword,
		IDUserExpiryTime,
		IDUserPersID,
		IDTaskName,
		IDThemeName,
		IDOriginatorID,			// Автор
		IDImplementerID,		// Исполнитель
		IDResponsiblePersonID,	// Ответственный
		IDAuditorID,			// Проверяющий или смотрящий
		IDDeadline,
		IDEstimatedTime,		// Предполагаемое
		IDActualTime,			// Фактическое
		IDHistoryFileID,
		IDMsgAttribute,
		IDTaskID,
		IDOwnerTaskID,
		IDThemeID,
		IDSiteID,
		IDMessageType,
		IDMessageID,
		IDRecipientID,			// Получатель
		IDPosition,
		IDUserStatus,
		IDRequiredTime,
// ---------------------------------------------------------

		//(!)               // Íîâûå ïàðàìåòðû äîáàâëÿéòå òîëüêî Â ÊÎÍÅÖÖÖ ÑÏÈÑÊÀ !!!
		FinishPrm,			// Èíäåêñ ïîñëåäíåãî ïàðàìåòðà
};

char*	GetMessageType(int ID_MsgType);

enum	MessageType	
{
	MT_SimplMsg=0,				//
	MT_Task,					//
	MT_Report,					//
	MT_Request,					//
	MT_Information,
	MT_Proposal,
	MT_Question,
	MT_Topic,

	MaxMessageType
};


static char	*MessageTypeTable[MaxMessageType+1]=
{
	"Simple",
	"Task",
	"Report",
	"Request",
	"Information",
	"Proposal",
	"Question",
	"Topic",
	""
};

// IDMsgAttribute
#define MA_Readed				1
#define MA_Archived				2
#define MA_Deleted				4
#define MA_Hidden				8
#define MA_Highlighted			16
#define MA_ResponsibleReaded	32
#define MA_OriginatorReaded		64


enum	ErrorID	
{
	NoErrors=0,
	ErrUnknown,					//
	ErrUnknownSoft,				//
	ErrUnknownNet,				//
	ErrUnknownHard,				//
	ErrOpenFile,				//
	ErrOpenSocket,				//
	ErrOpenShMem,				//
	ErrOpenPort,				//
	ErrTimeOut,					//
	ErrDomainCntl,				//
	ErrProcSrv,					//
	ErrFileSrv,					//
	ErrDB,						//
	ErrDBManager,				//
	ErrSSSrv,					//
	ErrChannel,					//
	ErrSetup,					//
	ErrParam,					//
	ErrDevUnavailable,			//
	ErrUnitUnavailable,			//
	ErrFuncionUnavailable,		//
	ErrDomainCntlUnavailable,	//
	ErrDomainState,				//
	ErrLock,					//
	ErrPolicy,					//


	MaxErrorNumber
};
//	Attribute Value
#define		AttrEl_ReadOnly		1					//	Только для чтения, но параметры можно менять
#define		AttrEl_NoReplicate	2					//	Не реплицировать
#define		AttrEl_NoModify		4					//	Не параметры, кроме самого атрибута
#define		AttrEl_System		0x1000000000000000	//	Разрешить запись в базу данных

#define LimitLSRP10		32000.0
#define LimitUSRP10		9312000.0
#define LimitSRS1401    40000000.0

#define LimitSRTiburon  5600000.0	/*2800000.0*/ /*347000.0*//*256000.0*/
#define LimitSRTiburLic 2800000.0	/*2800000.0*/ /*347000.0*//*256000.0*/

#define LimitSR_PoiskDVB	32000.0
#define LimitSR_Poisk140	32000.0
#define LimitLSRUWBDem		5691.0
#define LimitUSRUWBDem		46620000.0
#define LimitLSRUDVB2_DVBS2	128000.0//256000.0
#define LimitLSRUDVB2_DVBS	16000.0//1000000.0
#define LimitUSRUDVB2		45000000.0

#define LimitLSR_UWBDS4		1.0//1000000.0
#define LimitUSR_UWBDS4		45000000.0

#define LimitSR_RCV4D_Low	9000.0
#define LimitSR_RCV4D_Upp	38100000.0

#define CPUU60			60
#define CPUU40			40
#define MEMU80			90
#define	RF_BandWidthDef	15000

#define LimitSRTiburV4  5600000.0


// Unit -----------------------------------------------------------------------------------
#define	N_ShareName			"AudioServer"
#define	N_DC				"DownConverter"
#define	N_WBR				"WideBandReceiver"
#define	N_DigitRec			"DigitalReceiver"
#define	N_CtrlMXL8x8		"CtrlMXL8x8"
#define	N_TlfSrv			"TlfSrv"
#define	N_TlfCon			"TlfCon"
#define	N_WSACfg			"WSACfg"
#define	N_Analytic			"AnalystMt"
#define	N_UnitDomainCtrl	"UnitDomainCtrl"
#define	N_ProcessingServer	"IDI_Protocol"
#define	N_UnitPoiskClient	"UnitPoiskCli"
#define	N_NetView			"NetView"
#define	N_UnitMainDB	    "UnitMainDB"
#define	N_UnitDB			"BDMan"
#define	N_UnitStrmSrv		"UnitStreamServer"
#define	N_UnitFileSrv		"IDI_FSrv"
#define	N_UnitTlfSrv		"TlfSrv"
#define	N_UnitPostProcSrv	"UnitPostProcSrv"
#define	N_UnitRole			"UnitRole"
#define	N_UnitChannel		"UnitChannel"
#define	N_UnitDomainCtrl	"UnitDomainCtrl"
#define	N_UnitTiburon		"UnitTiburon"
// Unit -----------------------------------------------------------------------------------
#define	N_SignalSimulator   "SignalSimulator"
#define	N_Host				"Host"
#define	N_Domain			"Domain"
#define	N_APCS				"APCS"
#define	N_WorkGroups		"WorkGroups"
#define	N_WorkGroupsIn		"WorkGroupsIn"
#define	N_Users				"Users"
#define	N_SMS				"SMS"
#define	N_Archive			"Archive"

#define	N_Implementers		"Implementers"
#define	N_Originator		"Originator"
#define	N_Recipients		"Recipients"
#define	N_RecipientGroups	"RecipientGroups"
#define	N_Responsible		"Responsible"
#define	N_Auditors			"Auditors"
#define	N_AuditorGroups		"AuditorGroups"

#define	N_Messages			"Messages"
#define	N_Childs			"Childs"
#define	N_Owners			"Owners"
#define	N_Attachments		"Attachments"
#define	N_History			"History"

#define N_EstimatedTime		"Estimated Time"
#define N_RequiredTime		"Required Time"
#define N_ActualTime		"Actual Time"

#define	N_Workgroup			"Workgroup"
#define	N_Processor			"Processor"
#define	N_Unit				"Unit"
#define	N_File				"File"
#define	N_NetComm			"NetComm"
#define	N_BDBrowse			"BDBrowse"
#define	N_OperWS			"OperWS"
#define	N_Policy			"Policy"

#define	N_Protocol			"Protocol"			
#define	N_Device			"Device"			
#define	N_CommSys			"CommunicationsSystem"
#define	N_CommSysSat		"SatelliteCommunication"// одна из CommunicationsSystems for descr Satellite
#define	N_CommRF			"RadioCommunication"// одна из CommunicationsSystems for descr Satellite
#define	N_ReceiveLoc		"ReceiveLocation"// одна из CommunicationsSystems for descr Satellite
#define	N_Satellite			"Satellite"
#define	N_SatelliteArch		"Satellite.Archive"
#define	N_SatelliteTrans	"Satellite.Transponder"
#define	N_SatelliteFrRange	"Satellite.FrequencyRange"
#define	N_FreqPool			"FrequencyPool"
#define	N_SystemSetting		"SystemSetting"
#define	N_MonitoringSystem	"MonitoringSystem"
#define	N_CriticalError		"CriticalError"
#define	N_Service			"Services"
#define	N_Server			"Server"
#define	N_Link				"Link"
#define	N_Channel			"Channel"
#define	N_Input				"Input"
#define	N_Output			"Output"
#define	N_Role				"Role"
#define	N_DeviceLink		"DeviceLink"
//-----------------------------------------------------------------------------------
#define	N_Scrambler			"Scrambler"
#define	N_Demodulation		"Modulation"
#define	N_ECC				"ECC"
#define	N_SCC				"SCC"
#define	N_Turbo				"Turbo"
#define	N_Inverter			"Inverter"
#define	N_DiffDecoder		"DiffDecoder"
#define	N_DCME				"DCME"


#define	P_TC2000			"TC2000"
#define	N_DVB				"DVB"
#define	N_DVB_S2			"DVB_S2"
#define	N_HDLC				"HDLC"
#define	N_G742				"G742"
#define	N_G742_DEMUX		"G742_DEMUX"
#define	N_G704				"IDIR_G704"
#define	P_G704				"G704"
#define	N_G704_DEMUX		"IDIR_G704_DEMUX"
#define	P_G704_DEMUX		"G704_DEMUX"
#define	N_VSAT_Outroute		"VSAT_Outroute"
#define	N_VSAT_Inroute		"VSAT_Inroute"
#define	N_IBS				"IDIR_IBS"
#define	P_IBS				"IBS"
#define	N_IBS_DEMUX			"IDIR_IBS_DEMUX"
#define	P_IBS_DEMUX			"IBS_DEMUX"
#define	N_IDR				"IDR"
#define	N_MUX_V16			"IDIR_MUX_V16"
#define	P_MUX_V16			"MUX_V16"
#define	N_MUX_160			"IDIR_MUX_160"
#define	P_MUX_160			"MUX_160"
#define	N_MUX_V10			"IDIR_MUX_V10"
#define	P_MUX_V10			"MUX_V10"
#define	N_MUX_T1876			"IDIR_MUX_T1876"
#define	P_MUX_T1876			"MUX_T1876"
#define	N_iDirect			"iDirect"
#define	N_iDirect_DEMUX		"iDirect_DEMUX"

#define	P_ALCATEL			"ALCATEL_3612"
#define	P_ALCATEL_DEMUX		"ALCATEL_3612_DEMUX"
#define	N_Alcatel			"Alcatel"
#define	N_Alcatel_DEMUX		"Alcatel_DEMUX"

#define	N_DTX240			"DTX240"
#define	N_DTX240T			"DTX-240T"
#define	N_DTX240DEF			"DTX-240DEF"
#define	N_DTX240_DEMUX		"DTX240_DEMUX"

#define P_G763				"G763"
#define P_G763_DEMUX		"G763_DEMUX"
#define P_G767				"G767"
#define P_G767_DEMUX		"G767_DEMUX"
#define P_G768				"G768"
#define P_G768_DEMUX		"G768_DEMUX"
#define P_CELTIC_DEMUX		"Celtic_DEMUX"
#define P_G765				"G765"
#define P_G765_DEMUX		"G765_DEMUX"

#define P_GSM_DEMUX			"GSM_DEMUX"

#define	P_Alcatel2			"Alcatel"
#define	N_Alcatel2			"IDIR_Alcatel"
#define	N_Framer32			"IDIR_Framer32"
#define	P_Framer32			"Framer32"
#define	N_Framer16			"IDIR_Framer16"
#define	P_Framer16			"Framer16"

#define	N_TES				"HNS_TES"
#define	N_TES_G728			"HNS_TES_G728"
#define	N_TES_G728_DEMUX	"HNS_TES_G728_DEMUX"
#define	N_RadioMPEG			"RadioMPEG"
#define	N_RadioMPEG_DEMUX	"RadioMPEG_DEMUX"
#define	N_Cisco				"Cisco"
#define	N_Ethernet			"Ethernet"
#define	N_Gilat				"FramerGilat"
#define	N_FrameRelay		"FrameRelay"
#define	N_FrameRelay_DEMUX	"FrameRelay_DEMUX"
#define	N_PPP				"PPP"
#define	N_SS7				"SS7"
#define	N_NetModem			"NetModem"
#define	N_NetModem_DEMUX	"NetModem_DEMUX"
#define	N_UnknownIP			"UnknownIP"
#define	N_IP				"IP"
#define	N_EDMAC				"Edmac"
#define	N_Vocoder			"Vocoder"
//#define	N_CliSSS			"CliSSS"
//#define	N_Energy			"Energy"
#define	N_Manual			"Manual"
#define	N_Automate			"Automate"
#define	N_Search			"Search"
//-----------------------------------------------------------------------------------
#define	N_SelPolarization	"SelectPolarization"
#define	N_SelAntenna		"SelectAntenna"
#define	N_SymbolRate		"SymbolRate"
#define	N_BitRate			"BitRate"
#define	N_CarrierDetector	"CarrierDetector"
#define	N_RFCarrierDetector	"RFCarrierDetector"
#define	N_FindSatellite	    "FindSatellite"
#define	N_SelLocation	    "SelectLocation"

#define	N_Relock			"Relock"
#define	N_Scanner			"Scanner"
#define	N_Rescanner			"Rescanner"
#define	N_Tuner				"Tuner"
#define	N_Analyzer			"Analyzer"
//#define	N_Spectrum			"Spectrum"

#define	N_Interception		"Interception"
#define	N_ScannerPool		"Pool_scanner"
#define	N_ScannerRange		"Range_scanner"

#define	N_Demodulator		"Demodulator"
//#define	N_S_Analyzer		"S_Analyzer"


#define	N_Profile			"Profile"
#define	N_ProfileSchema		"ProfileSchema"
#define	N_ProfileCS			"ProfileCS"
#define	N_ProfileQBE		"ProfileQBE"
#define	N_ProfileFilter		"ProfileFilter"
#define	N_Filter			"Filter"
#define	N_QBE				"QBE"
#define	N_CS				"CS"
#define	N_Default			"Default"
#define	N_Group				"Group"
#define	N_Schema			"Schema"
#define	N_ExtSrv			"ExternalServers"

// #define	N_PoiskCli		"PoiskClient"

#define SLAVE						0
#define N_Slave						"Slave"

#define MASTER						1
#define N_Master					"Master"
#define REMASTER					2

#define N_Storage					"Storage"
#define N_Standalone				"Standalone"

#define	N_DomainController			"DomainController"
#define	N_FileServer				"FileServer"
#define	N_DataBase					"DataBase"
//#define	N_MainDataBase			"MainDataBase"
#define	N_MainDataBase				"DataBaseManager"
#define	N_ProcServer				"ProcessingServer"
#define	N_OperatorWorkstation		"OperatorWorkstation"
#define	N_AdministratorWorkstation	"AdministratorWorkstation"
#define	N_SignalingSystemServer		"SignalingSystemServer"
#define	N_BDManager					"BDMan"
#define	N_StreamServer				"StreamServer"
#define	N_PostProcServer			"PostProcessingServer"
#define	N_AnalyticWorkstation		"AnalyticWorkstation"
#define	N_TelemetrViewer			"Telemetry"

#define	N_Continue					"Continuous"
#define	N_Packet					"Packet"
#define	N_Block						"Block"


#define	N_Before					"Before"
#define	N_After						"After"


//  [7/21/2003] San
//---- Список имен групп (подгрупп)
#define	NameGroupList	"GroupList"
#define	NameStreamList	"StreamList"		// группа идентификаторов потоков
#define	NameLogChannel	"LogChannel"		// подгруппа идентификаторов логических каналов потока
#define	NameDeamChannel	"DemChannel"		// подгруппа идентификаторов демодулируемых логических каналов
#define	NameNewStream	"NewStream"			// подгруппа идентификаторов логических подпотоков потока

#define	NameParamList	"ParamList"			// группа со списком используемых параметров

#define	NameNetSources	"NetSource"

//#define	NameMonitorSys	"MonitoringSystem"	// группа конфигурации систем мониторинга
#define	NameSubSystem	"SubSystem"			// подгруппа конфигурации подсистем мониторинга
#define	NameUnitID		"UnitID"			// подгруппа конфигурации конкретных Unit'ов подсистем мониторинга
#define	NameCrossMux	"CrossMux"			// 
#define	NameNetInputs	"PhInput"			// подгруппа сетевых входов для подсистемы NetIDICli,или  серверов  для NetCommTx
#define	NamePhInputs	"PhInput"			// 
#define	NameInputs		"PhInput"			// 
#define	NameProtocol	"Protocol"			// 
#define	NameFormat		"Format"			// 
#define	NameDemod		"Demodulator"		// группа демодуляторов
//#define	NameSatellites	"Satellite"		

#define	NameSysSetting	"SystemSetting"
//#define	NameMonitorSys2	"MonitoringSystem"
#define	NameNeuron		"Neuron"
#define	NameTables		"Table"

#define	NameAddParam	"AdditionalParameter"//дополнительный параметр для NetIDICli
#define	NameCanonizatNR	"RuleCanonization"	//группа с правилами канонизации телеф. номеров
#define	NameSetRuleCan	"SetRuleCanonization"//подгруппа для потока с набором правил канонизации телеф. номеров
#define	NameProfileList	"ProfileList"	     //группа Profile
#define	NameFrequencyRange "FrequencyRange"  //группа FrequencyRange
//	Имена UnitName
#define	STS_ConTlx		"ConTlx"
#define	STS_SrvTlx		"SrvTlx"
#define	STS_SrvTlf		"SrvTlf"
#define	STS_SrvSSO		"SrvSSO"
#define	STS_VConSrvTlf	"VConsolTlf"
#define	STS_VConSrvTlx	"VConsolTlx"
#define	STS_SOPTlf		"SopTlf"
#define	STS_SOPVOIP		"SopVOIP"
#define	IDI_SrvIDI		"SrvIDI"	//NetIDISrv
#define	IDI_CliIDI		"CliIDI"	//NetIDICli
#define	IDI_SOP			"SOP"	
#define	IDI_FileSrv		"FileServer"
#define	STS_SSO			"SSO"
#define	VisualConSKL	"@sVisualCon_SKL"
#define	STS_CliSAP		"CliSAP"

#define	STS_ModulePK	"Pilot-K"
#define	STS_ModMan		"ModuleMan"

#define	STS_StrumCli	"StrumCli"
#define	STS_StrumSrv	"StrumSrv"
#define	STS_PoiskCli	"PoiskConf"

#define	IDI_CommRx		"NetCommRx"
#define	IDI_CommTx		"NetCommTx"
#define	IDI_Comm		"NetComm"
 
#define	IDI_BDClient	"BDClient"
#define	IDI_BDServer	"BDServer"

#define	IDI_BDCliBE_M2	"BDCliBE_M2"
#define	STS_ObjCliBE	"ObjectCliBE"

// --- IDSaveMode Volue
#define	VolSM_FileServer					0x01
#define	VolSM_DataBase						0x02
#define	VolSM_MainDataBase					0x04
#define	VolSM_SignalingSystemServer			0x08
#define	VolSM_Energy						0x10



//--Значения #CContentType
#define	VolContentTypeAudioNotDefined	"AudioNotDefined"
#define	VolContentTypeAudioNotSpeech	"AudioNotSpeech"
#define	VolContentTypeAudioSpeech		"AudioSpeech"
#define	VolContentTypeISDN				"ISDN"
#define	VolContentTypeFax				"FAX"
//////////////////////////////////////////////////////////////////////////
//--Значения @pEncoding
#define	VolFormatMTK2					"MTK-2"			// тип кодирования MTK-2
#define	VolFormatOSNAZ					"OSNAZ"			// тип кодирования OSNAZ
//////////////////////////////////////////////////////////////////////////
//--Значения #PFormat
#define	VolFormatFloat			"Float"			// формат данных в потоке(канале) float
#define	VolFormatFloatIQ		"FloatIQ"		// формат данных в потоке(канале) float-Complex
#define	VolFormatPCMa			"PCMa"			// формат данных в потоке(канале) PCMa
#define	VolFormatPCMm			"PCMm"			// формат данных в потоке(канале) PCMm
#define	VolFormatADPCM			"ADPCM"			// формат данных в потоке(канале) ADPCM
#define	VolFormatGSM610			"GSM6.10"		// формат данных в потоке(канале) Gsm 6.10


#define	VolFormatTAG			"TAG"	// формат данных в потоке(канале) c 
#define	VolFormatOADPCM			"OADPCM"	// формат данных в потоке(канале) ADPCM
#define	VolFormatADPCM_OKI		"ADPCM_OKI"	// формат данных в потоке(канале) ADPCM
#define	VolFormatADPCM_G722		"ADPCM G.722"	// формат данных в потоке(канале) ADPCM
#define	VolFormatADPCM_G726		"ADPCM G.726"	// формат данных в потоке(канале) ADPCM
#define	VolFormatADPCM_G727		"ADPCM G.727"	// формат данных в потоке(канале) ADPCM
#define	VolFormatLDCELP_G728	"LDCELP G.728"	// формат данных в потоке(канале) LDCELP
#define	VolFormatCSACELP_G729	"CS-ACELP G.729"	// формат данных в потоке(канале) CS-ACELP
#define	VolFormatG723			"G.723"			// формат данных в потоке(канале) 

#define	VolFormatPCM			"PCM"			// формат данных в потоке(канале) PCM
#define	VolFormatG726			"G.726"			// формат данных в потоке(канале) G.726
#define	VolFormatG728			"G.728"			// формат данных в потоке(канале) G.728
#define	VolFormatG729			"G.729"			// формат данных в потоке(канале) G.729
#define	VolFormatUnknown		"Unknown"		// Неизвестный формат данных в потоке(канале)
#define	VolFormatData			"Data"			// 
#define	VolFormatMPEG			"MPEG"			// формат данных в потоке(канале) MPEG
#define	VolFormatVMPEG			"VideoMPEG"		// формат данных в потоке(канале) MPEG
#define	VolFormatRTP			"RTP"			// формат данных в потоке(канале) RTP
 
#define	VolFormatStereoPCMa		"StereoPCMa"	// формат данных в потоке(канале) 
#define	VolFormatStereoPCMm		"StereoPCMm"	// формат данных в потоке(канале) 
#define	VolFormatStereoPCMaD5	"StereoPCMaD5" // D5
#define	VolFormatExtBinary		"ExtBinary"			// бинарный поток в формате демодулятора "СИГМАТЕХ"
#define	VolFormatBinary			"Binary"			// бинарный поток в формате демодулятора
#define	VolFormatRCV4D			"RCV4D"			// бинарный поток в формате демодулятора
//////////////////////////////////////////////////////////////////////////
//--Значения #PProtocol
#define	VolProtocolAint			"A-Interface"	// формат данных в потоке(канале) PCMa
//#define	VolProtocolAter		"Ater-Interface"// формат данных в потоке(канале) 
#define	VolProtocolEDSS			"EDSS"			// протокол данных в потоке(канале) 
#define	VolProtocolSTS5			"STS-5"			// протокол данных в потоке(канале) 
#define	VolProtocolSTS7			"STS-7"			// протокол данных в потоке(канале) 
#define	VolProtocolR101			"R.101"			// протокол данных в потоке(канале) согласно рекомендации R.101
#define	VolProtocol_1VF			"1VF"			// протокол данных в потоке(канале) 
#define	VolProtocol_V52			"V52"			// протокол данных в потоке(канале) 
#define	VolProtocolH323			"H.323"			// протокол данных в потоке(канале) 
#define	VolProtocolSIP			"SIP"			// протокол данных в потоке(канале) 
#define	VolProtocolHDLC			"HDLC"			// протокол данных в потоке(канале) 
#define	VolProtocolPckHDLC		"PackHDLC"		// протокол данных в потоке(канале)
#define	VolProtocol_R2D			"R2D"			// протокол данных в потоке(канале)
#define VolProtocolRESPCM		"TRESPCM"		// протокол данных Thuraya после ресемплера
#define	VolProtocolPackTBCCH	"PackTBCCH"		// протокол данных Thuraya BCCH, после декодера
#define	VolProtocolPackTPCH		"PackTPCH"	    // протокол данных Thuraya PCH, после декодера
#define	VolProtocolPackTGBCH	"PackTGBCH"	    // протокол данных Thuraya GBCH, после декодера
#define	VolProtocolPackTTCH3	"PackTTCH3"	    // протокол данных Thuraya TCH, после декодера
#define	VolProtocolVocTTCH3		"VocTTCH3"	    // протокол данных Thuraya Voc, после декодера
#define	VolProtocolPackTTCH6	"PackTTCH6"	    // протокол данных Thuraya TCH, после декодера
#define	VolProtocolPackTTCH9	"PackTTCH9"	    // протокол данных Thuraya TCH, после декодера
#define	VolProtocolPackTRACH	"PackTRACH"	    // протокол данных Thuraya RACH, после декодера
#define VolProtocolSaveTCH3		"SaveTTCH3"

#define	VolProtocolTBCCH		"TBCCH"		// протокол данных Thuraya BCCH, после демодулятора
#define	VolProtocolTPCH			"TPCH"	    // протокол данных Thuraya PCH, после демодулятора
#define	VolProtocolTGBCH		"TGBCH"	    // протокол данных Thuraya GBCH, после демодулятора
#define	VolProtocolTTCH3		"TTCH3"	    // протокол данных Thuraya TCH3, после демодулятора
#define	VolProtocolTTCH6		"TTCH6"	    // протокол данных Thuraya TCH6, после демодулятора
#define	VolProtocolTTCH9		"TTCH9"	    // протокол данных Thuraya TCH9, после демодулятора
#define	VolProtocolTRACH		"TRACH"	    // протокол данных Thuraya RACH, после демодулятора
#define	VolProtocolTFCCH		"TFCCH"	    // протокол данных Thuraya FCCH, после демодулятора

#define	VolProtocolSCCP_CDMA	"SCCP_CDMA" // протокол данных 

#define	VolProtocolPackGBCCH	"PackGBCCH"		// протокол данных GSM BCCH, после декодера


//////////////////////////////////////////////////////////////////////////
#define	VolTypeDigitSignal		"DigitSignal"			// протокол данных в потоке(канале) 
#define	VolTypeBinaryData		"BinaryData"			// протокол данных в потоке(канале) 
//--Значения @cMode
#define	VolStatic		"Static"	// Указыват на работу в статическом режиме
#define	VolDynamic		"Dynamic"	// Указывает на работу в динамическом режиме
#define	VolOff			"Off"		// Выключено
//#define VolControl		"Control"	// Указыват на работу в режиме канала управления (Thuraya)
//#define VolTraffic		"Traffic"	// Указыват на работу в режиме канала передачи трафика (Thuraya)
//--Значения @pDemodulators
#define	VolDOF2400A		"DOF-2400/V.26a"	// Пеpедача DOF-2400/V.26A - альтернатива А
#define	VolDOF2400B		"DOF-2400/V.26b"	// Пеpедача DOF-2400/V.26B - альтернатива B
#define	VolM24120		"M-24/120"			// Пеpедача M-24/120
//--Значения @cState
#define VolStateWork	"Work"			//Vol=Work Close Off
#define VolStateClose	"Close"			//Vol=Work Close Off
#define VolStateOff		"Off"			//Vol=Work Close Off
#define VolStateOn		"On"			//Vol=On
#define VolDirectionRX		"RX"			//значение направления передачи(PrmDirection) на прием
#define VolDirectionTX		"TX"			//значение направления передачи(PrmDirection) на передачу
#define VolDirectionForw	"Forward"		//значение направления передачи(PrmDirection) на передачу
#define VolDirectionBack	"Backward"		//значение направления передачи(PrmDirection) на передачу


struct Customer
{

	USHORT	CID;
	USHORT	SID;
	char	CustomerRemark[255];
	char	Flag;
};

Customer* GetCustomerInfo(int  CustomerID);

struct	Parametr
{
	INT64	UID;			//	Уникальный идентификатор параметра
	char	Name[64];		//	Имя параметра
	char	Type;			//	Тип параметра
	char	Index;			//	Признак необходмости индексации параметра
	char	Regularity;		//	Признак уникальности параметра
	char	Rez;
	int		Status;			//	Атрибут определяющий возможные действия над параметром
	int		Size;			//	Размер параметра если 0 то нет ограничения
	int		View;			//	Способ отображения параметра
	char	Remark[128];	//  Parametrs comments
};

// Type
enum		Parametr_Type
{
			PrmType_SHORT=0,
			PrmType_INT,
			PrmType_INT64,
			PrmType_FLOAT,
			PrmType_DOUBLE,
			PrmType_CHAR,
			PrmType_STRING,
			PrmType_USER,
			PrmType_BLOB,	// "Бесконечная" структура. В базе хранится Заголовок = int32(0..2147483647) = размер последующих данных. (c)Mao
			PrmType_TAG,	// "Пустая" структура нулевой длиной. Обозначает какой-либо логический признак. PrmType_INDICATOR (c)Mao
			PrmType_GeoPos, // Географические координаты на земном шаре - структура tGeoPos размером sizeof(float)*3.
			PrmType_GeoDir, // Данные угломерного целеуказания - структура tDirPos размером sizeof(float)*2 (с позиции на поверхности земли).
};

enum		Parametr_Index
{
			PrmIndex_No=0,
			PrmIndex_Yes,
};

enum		Parametr_Unique
{
			PrmRegularity_No=0,
			PrmRegularity_Yes,
};

#define		PrmStatus_WrBD		1	//	Разрешить запись в базу данных
#define		PrmStatus_WrFS		2	//	Разрешить запись в файловый сервер
#define		PrmStatus_WrNC		4	//	Разрешить передачу по сетевому взаимодействию
#define		PrmStatus_All (PrmStatus_WrBD|PrmStatus_WrFS|PrmStatus_WrNC)//	Разрешить All
//(?)#define		PrmStatus_All (~0)//	Разрешить All
#define		PrmStatus_None (0)      //	Запретить базу данных, файловый сервер и сетевое взаимодействие

enum		Parametr_View
{
			ParametrView_Def=0,  // according to Parametr.Type
			ParametrView_Dec,    // signed number in radix 10
			ParametrView_Hex,    // unsigned number in radix 16
			ParametrView_Bin,    // unsigned number in radix 2
			ParametrView_Dump,   // Text as in FAR:F3
			ParametrView_Txt,    // Text with formatting: LR LF TAB ...
			ParametrView_IPAddr, // example 192.168.45.1
			ParametrView_FTime,  // FileTime in form [Control Panel].[Regional and Language Options]
			ParametrView_UDec,    // unsigned number in radix 10
			ParametrView_ElapsedTimeI64
};

struct tGeoPos          //  Картографическая и орбитальная 3D структура (для PrmType_GeoPos)
{
	double	Latitude;   //  Географическая ШИРОТА в градусах = [-90..+90]
	double	Longitude;  //  Географическая ДОЛГОТА в градусах = [-180..+180]
	double	Altitude;   //  ВЫСОТА над средним уровнем моря в метрах (от центра земного шара 6378135 метров)
};

struct tGeoDir          // Итоговые метрики угломерного целеуказания с поверхности земного шара (для PrmType_GeoDir)
{
	float	Azimuth;    //  Азимут в градусах = [0..359.(9)]
	float	Elevation;  //  Угол места в градусах = [-90..+90]
};
// 
// struct tCoordinate		
// {
// 	int X;
// 	int Y;
// };
                                                                                                 
struct StatFromFindFnFb
{
	int mode; // 1-Fn,2-Fb,4-TypeMod
	double fn,fb;
	int poz;
	int modulation; // 0-UNDEF,1-SIN,2-BPSK,4-QPSK,5-OQPSK,8-TPSK,9-QAM8,16-QAM16
	INT64 flstop;
	char str[128];
	Str_Spectrum spec0;
	int Fl_Pack;
	INT64 ID;
};


static int  CopyParameter[]=
{
	IDDVB_PID,
	IDMAC_Address,
	IDIODLCAddr,
	IDIODLCSession,
 	IDSource,
	IDIDGroop,

	IDIPFrom,	
	IDPortFrom,
	IDCountryCodeFrom,
	IDCountryNameFrom,
	IDIPTo,	
	IDPortTo,
	IDCountryCodeTo,
	IDCountryNameTo,
	IDNameOperator,
	IDOPC,
	IDDPC,
	IDEPNameMerging,
	IDIDFileSnsFr,
	IDIDFileSnsTo,
	0	
};
static int  CopyParameter2[]=
{
	IDProfileFilter,
	IDProfileQBE,
	IDProfileCS,
 	IDProfile,
	0	
};

enum		StateProc
{
	// Режимы создания и завершения
	NewActIDI=0,				// Созданое, но незавершенное IDI
	NewCloseIDI,			// Созданое, завершенное IDI
	SesionActNoFiles,		// Сессии активные без файлов (детей)
	SesionActWithFiles,		// Сессии активные содержащие файлы
	SesionCloseNoFiles,		// Сессии завершенные без файлов (детей)
	SesionCloseWithFiles,	// Сессии завершенные содержащие файлы
	SesionStreamAct,		// Сессии потоков активные
	SesionStreamClose,		// Сессии потоков закрытые
	SesionLogChAct,			// Сессии логических каналов активные
	SesionLogChClose,		// Сессии логических каналов закрытые
	FileAct,				// Файлы активные без файлов (детей)
	FileClose,				// Файлы завершенные без файлов (детей)
	// Режимы стадий обработки оператором

	// Режимы стадий обработки автоматом (роботом)
	NoProcessed=200,			// Не обработанный файл

	// Режимы стадий оперативной маршрутизации в процессе обработки данных
	BDV_Reading=300,
	Processing,
	BDV_Moved,
	BDV_Copy,
	
	DeleteIDI=1000		// Помечен как удаленный
};

static char* StateProcRem[]=
{
	"Созданое, но незавершенное IDI",
	"Созданое, завершенное IDI",
	"Сессии активные без файлов (детей)",
	"Сессии активные содержащие файлы",
	"Сессии завершенные без файлов (детей)",
	"Сессии завершенные содержащие файлы",
	"Сессии потоков активные",
	"Сессии потоков закрытые",
	"Сессии логических каналов активные",
	"Сессии логических каналов закрытые",
	"Файлы активные без файлов (детей)",
	"Файлы завершенные без файлов (детей)",

	// Режимы стадий оперативной маршрутизации в процессе обработки данных
	"Просмотренный",	//BDV_Reading
	"Обработанный",		//Processing
	"Перенесенная запись из одной БД в другую",		//BDV_Moved
	"Скопированная запис из одной БД в другую",		//BDV_Copy

	"Удаленный"
};

enum		VolCloseReason
{
	CloseNormal,
	DisconnectSourceNC,	//For NetComm when disconnect 

	CloseUnknown	
};


enum IDNameDeviceType
{
	IDDevTypName_Unknown=0,
	IDDevTypName_SatComAntenna,
	IDDevTypName_Splitter,
	IDDevTypName_MUX,
	IDDevTypName_DownConverter,
	IDDevTypName_Demodulator,
	IDDevTypName_Host,
	IDDevTypName_DigitalRec,
	IDDevTypName_WideBandRec,
	IDDevTypName_Feeder,
	IDDevTypName_MDCVL15,
	IDDevTypName_Poisk10,
	IDDevTypName_Poisk140,
	IDDevTypName_Poisk140D,
	IDDevTypName_PoiskDVB,
	IDDevTypName_VirtMUX,
	IDDevTypName_SkyStar3_S1401,
	IDDevTypName_SkyStar3_S3200,
	IDDevTypName_Container,
	IDDevTypName_Input,
	IDDevTypName_Output,
	IDDevTypName_Channel,
	IDDevTypName_SR2200,
	IDDevTypName_RFAntenna,
	IDDevTypName_WLBD15,
	IDDevTypName_DDC6654,
	IDDevTypName_SignalSimulator,

	IDDevTypName_ADCL2V1,
	IDDevTypName_MUX_moxa,

	IDDevTypName_UWBR,
	IDDevTypName_UWBD,
	IDDevTypName_SID32V1,
	IDDevTypName_UDVB2,
	IDDevTypName_ADP8CL,
	IDDevTypName_SatFeederInput,
	IDDevTypName_ModulatorL,
	IDDevTypName_MUX_moxa_8x16x8,
	IDDevTypName_MUX_QUINTECH,
	IDDevTypName_RCV4D,
	IDDevTypName_UWBDS4,
	IDDevTypName_ADCL2V2,

	IDDevTypName_Last
};

static char* NameDeviceType[]=
{
	"Unknown",
	"Satcom_antenna",
	"Splitter",
	"Signal Switch",
	"DownConverter",
	"Demodulator",
	"Host",
	"DigitalReceiver",
	"WideBandReceiver",
	"Feeder",
	"MDCVL-15",

	"Poisk-10",
	"Poisk-140",
	"Poisk-140D",
	"Poisk-DVB",

	"Virtual Signal Switch",

	"S-1401",
	"S2-3200",
	
	"Container",
	"Input",
	"Output",
	"Channel",

	"SR2200",

	"RF_Antenna",
	"WLBD-15",
	"DDC6654",
	"SignalSimulator",
	"ADCL2V1",
	"Signal Switch(MOXA)",
	"UWBR",
	"UWBD",
	"SID32V1",
	"UDVB2",
	"ADP8CL",
	"SatFeederInput",
	"ModulatorL",
	"Signal Switch(MOXA) 8x16x8",
	"Signal Switch QUINTECH SRM2150",
	"Multichannel Digital Receiver RCV4D",
	"UWBD-S4",
	"ADCL2V2",

	""
};


enum IDNameContainerType
{
	IDContainerTypName_Unknown=0,
	IDContainerTypName_SA,
	IDContainerTypName_DDC6654,
	IDContainerTypName_Adonis,
	IDContainerTypName_UniDem,
	IDContainerTypName_Last
};

static char* NameContainerType[]=
{
	"Undefine",
	"AnalysisStation",
	"Tiburon",
	"Adonis",
	"UniDem",
	""

};

enum IDVolumeProcessedName
{
	IDVolProc_On=0,
	IDVolProc_Off,
	IDVolProc_Error,
//	IDVolProc_SwitchOff,
//	IDVolProc_SwitchOn,
	IDVolProc_Analyze,
	IDVolProc_Ready,
	IDVolProc_Setup,
	IDVolProc_Remove,
	IDVolProc_Wait,

	IDVolProc_Last,
};
static char* VolumeProcessed[]=
{
	"On",
	"Off",
	"Error",
//	"SwitchOff",
//	"SwitchOn",
	"Analyze",
	"Ready",
	"Setup",
	"Remove",
	"Wait",
	""
};

enum IDVolumeBearerType
{
	enBearTypINTELSAT=0,
	enBearTypDVBS,
//	enBearTypDVBS2,
	enBearTypDVBDSNG,
	enBearTypDVBS2N,
	enBearTypDVBS2S,
	enBearTypTURBO1032_4K,
	enBearTypTURBO1032_16K,
	enBearTypTURBO,
	enBearTypHard_Dec,
	enBearTypSoft_Dec,
	enBearTypSCPC,
	enBearTypTDMA,
	enBearTypFTDMA,
	enBearTypNCS,
	enBearTypBEACON,
	enBearTypVSAT,
	enBearTypDVBS2,

	enBearTyp_Last
};
static char* VolumeBearerType[]=
{
	"Intelsat",
	"DVB-S",	
//	"DVB-S2",
	"DVB-DSNG",
	"DVB-S2N",
	"DVB-S2S",
 	"TURBO-1032_4K",
 	"TURBO-1032_16K",
	"TURBO",
	"Hard_Dec",
	"Soft_Dec",
	"SCPC",
	"TDMA",
	"FTDMA",
	"NCS",
	"BEACON",
	"VSAT",
	"DVB-S2",
	""
};
enum IDVolumeNameUnit
{
	IDUnit_DR=0,
	IDUnit_WBR,
	IDUnit_MXL8x8,
	IDUnit_Antenna,
	IDUnit_Feeder,
	IDUnit_Spliter,
	IDUnit_DCRF,
	IDUnit_Dem,
	
	IDUnit_Last,
};
static char* VolumeNameUnit[]=
{
	"DigitalReceiver",
	"WideBandReceiver",
	"Signal Switch",
	"Satcom_antenna",
	"Feeder",
	"Splitter",
	"DownConverter",
	"MDCVL-15",
	"Poisk-10",
	"Poisk-140",
	"Poisk-140D",
	"Poisk-DVB",
	"Virtual Signal Switch",
	"S-1401",
	"S2-3200",
	"Container",
	"Input",
	"Output",
	"Channel",
	"SR2200",
	"RF_Antenna",
	"WLBD-15",
 	"DDC6654",
	"SignalSimulator",
	"ADCL2V1",
	"Signal Switch(MOXA)",
	"UWBR",
	"UWBD",
	"SID32V1",
	"UDVB2",
	"ADP8CL",
	"SatFeederInput",
	"ModulatorL",
	"Signal Switch(MOXA) 8x16x8",
	"Signal Switch QUINTECH SRM2150",
	"Multichannel Digital Receiver RCV4D",
	"UWBD-S4",
	"ADCL2V2",

	""
};
static char* VolumeNameDec[]= 
{
	"UnitDR",
	"UnitWBR",
	"UnitMXL8x8",
	"UnitAntenna",
	"UnitFeeder",
	"UnitSpliter",
	"UnitDCRF",
	"UnitMDCVL15",
	"UnitPoiskCtrl",
	"UnitPoiskCtrl",
	"UnitPoiskCtrl",
	"UnitPoiskCtrl",
	"UnitVirtMXL8x8",
	"UnitS1401",
	"UnitS1401",
	"UnitContainer",
	"UnitInput",
	"UnitOutput",
	"UnitChannel",
	"UnitSR2200",
	"UnitRFAntenna",
	"UnitWLBD15",
	"UnitTiburon",
	"UnitSignalSimulator",
	"UnitADCL2",
	"UnitMUX_MOXA",
	"UnitUWBR",
	"UnitUWBD",
	"UnitSID32",
	"UnitUDVB2",
	"UnitADPCL",
	"UnitSatFeederInput",
	"UnitModulatorL",
	"UnitMUX_MOXA",
	"UnitMUX_MOXA",
	"UnitRCV4D",
	"UnitUWBDS4",
	"UnitADCL2V2",

	""
};

static char* Parametr_LNB_Power[]=
{
	"POWER_OFF",
	"POL_VERT",
	"POL_HORZ",
};
enum IDVolumeLNB_POWER
{
	IDVolumePOWER_OFF=0,
	IDVolumePOL_VERT,
	IDVolumePOL_HORZ,

	IDVolumeLNB_Last,
};

enum IDVolumePolar
{
	IDVolumePolar_RHCP=0,
	IDVolumePolar_LHCP,
	IDVolumePolar_VP,
	IDVolumePolar_HP,

	IDVolumePolar_Last,
};
static char* VolumePolar[]=
{
	"RHCP",
	"LHCP",
	"VP",
	"HP",
	""
};


enum IDVolumeRole
{
	IDVolumeRole_DomCtr=0,
	IDVolumeRole_Fsrv,
	IDVolumeRole_DBas,
	IDVolumeRole_MDBas,
	IDVolumeRole_SPrc,
	IDVolumeRole_WSOp,
	IDVolumeRole_WSAd,
	IDVolumeRole_SSSrv,
	IDVolumeRole_PostPrcSrv,
	IDVolumeRole_AnalyticWS,
	IDVolumeRole_TelemetrViewer,
//	IDVolumeRole_StrmSrv,
		
	IDVolumeRole_Last,
};
static char* VolumeRole[]=
{
	N_DomainController,
	N_FileServer,
	N_DataBase,
	N_MainDataBase,
	N_ProcServer,
	N_OperatorWorkstation,
	N_AdministratorWorkstation,
	N_SignalingSystemServer,
	N_PostProcServer,
	N_AnalyticWorkstation,
	N_TelemetrViewer,

//	N_StreamServer,
	""
};

enum TypeContainer_Wizard
{
	IDVolumeContainer_TiburonNB=0,
	IDVolumeContainer_TiburonWB,
	IDVolumeContainer_UWBD140,
	IDVolumeContainer_DVB,

	IDVolumeContainer_Last,
};
static char* VolumeTypeContainer_Wizard[]=
{
	"Tiburon-NB",
	"Tiburon-WB",
	"UWBD-140",
	"UWBD-DVB",

	""
};


struct SetParamRole
{
	int ID_Paramet;
	char ValueDefault[40];
};

struct StrCtrlAntenna
{
	char NameSat[40];
	INT64 ZeroFreq;
	double AGC;
	int CntAGC;
	int Fl_Lock;
};

enum IDVolumePolicyType
{
	IDVolumePolicyTypeHost=0,
	IDVolumePolicyTypeRole,
	IDVolumePolicyTypeDevice,
		
	IDVolumePolicyTypeLast,
};
struct StatePolicy
{
	int     Type;
	char	Name[40];
	int		State;
	char	Remark[128];

	void Init(int type,char* name,int state,char* remark)
	{
		Type=type;
		if(Name)
			sprintf_s(Name,"%s",name);
		else
			Name[0]=0;
		State=state;
		if(remark)
			sprintf_s(Remark,"%s",remark);
		else
			Remark[0]=0;
	};
};

struct StrCMDInterception
{
	INT64 EPoint;
	INT64 EPoint_F;
	INT64 EPoint_D;
	INT64 IDProcSrv;
	INT64 Freq;
	INT64 FrequencyGeterodin;
	INT64 FrequencyLower,FrequencyUpper;
	INT64 ZeroFrequency;
	int   Fl_Invert,LogChan;
	int Processed;
	double SR,BW;
	char EpointNameSel[40];
	char HostName[40];
	char DomainName[40];
	char RegimI[20];
	char Chan[10];
	INT64 EPoint_PP;
	char NameMerging[80];
	int deltaL,deltaU;
	INT64 ID_SeansFrom,ID_SeansTo;
	
	void Init()
	{
		IDProcSrv=0;
		Freq=0;
		EPoint=EPoint_F=EPoint_D=EPoint_PP=-1;
		EpointNameSel[0]=HostName[0]=DomainName[0]=RegimI[0]=Chan[0]=0;
		Processed=IDVolProc_On;
		Fl_Invert=0;
		LogChan=-1;
		ZeroFrequency=FrequencyLower=FrequencyUpper=FrequencyGeterodin=0;
		SR=BW=0;
		NameMerging[0]=0;
		deltaL=deltaU=0;
		ID_SeansFrom=ID_SeansTo=-1;
	};
};

struct StatisticFromDemodulator
{
	INT64 EPoint;
	float deltaFc;
	double Energy;
	double SN;
	INT64 counter;
	float IdFc;
	INT64 counter_IdFc;

	float Improve_dFc;
	
	void Init()
	{
		EPoint=0;
		deltaFc=0.0;
		Energy=0.0;
		SN=0.0;
		counter=0;
		Improve_dFc=0;
		IdFc=0.0;
		counter_IdFc=0;
	};
};
// static SetParamRole ArrSetParamRole_DomCtr[]=
// {
// 	{IDTuner,"On"},{IDInterception,"On"},{IDScanner,"On"},0
// };

static SetParamRole ArrSetParamRole_Fsrv[]=
{
	{IDPathStorage,""},{IDRealHost,""},{IDNumberDaysStorage,"30"},{IDNameDB,"Main"},{IDNPhChan,"1"},0
};

static SetParamRole ArrSetParamRole_DBas[]=
{
	{IDPathStorage,""},{IDMode,"CDB"},{IDNumberDaysStorage,"30"},{IDNPhChan,"1"},0
};

static SetParamRole ArrSetParamRole_MDB[]=
{
	{IDPort,""},0
};

static SetParamRole ArrSetParamRole_SPrc[]=
{
	{IDNetViewProcessed,"On"},{IDNPhChan,"1"},0
};


static SetParamRole ArrSetParamRole_WSA[]=
{
	{IDTypeView,"SatteliteScheme"},{IDNPhChan,"1"},{IDControlParam,"SuperVisor"},0
};

static SetParamRole ArrSetParamRole_PostProc[]=
{
	{IDMode,""},{IDProfileQBE,""},{_IDNativeDomainName,""},{IDNPhChan,"1"},0
};

static SetParamRole ArrSetParamRole_AnalyticWS[]=
{
	//{IDMode,""},{IDProfileQBE,""},{_IDNativeDomainName,""},{IDNPhChan,"1"},
	0
};

static SetParamRole ArrSetParamRole_N_TelemetrViewer[]=
{
	//{IDMode,""},{IDProfileQBE,""},{_IDNativeDomainName,""},{IDNPhChan,"1"},
	0
};

static SetParamRole *ArrSetParamRole[IDVolumeRole_Last]=
{
	{0},//IDVolumeRole_DomCtr
	{ArrSetParamRole_Fsrv},//IDVolumeRole_Fsrv
	{ArrSetParamRole_DBas},//IDVolumeRole_DBas
	{ArrSetParamRole_MDB},// 	IDVolumeRole_MDBas,
	{ArrSetParamRole_SPrc},// 	IDVolumeRole_SPrc,
	{0},// 	IDVolumeRole_WSOp,
	{ArrSetParamRole_WSA},// 	IDVolumeRole_WSAd,
	{0},// 	IDVolumeRole_SSSrv,
	{ArrSetParamRole_PostProc},//IDVolumeRole_PostPrcSrv,
	{ArrSetParamRole_AnalyticWS},//IDVolumeRole_PostPrcSrv
	{0},// 	IDVolumeRole_SSSrv,
};




enum IDVolRoleChan
{
	IDVolRoleChan_Undef=0,
	IDVolRoleChan_AnalysStat,
	IDVolRoleChan_TiburonChannel,
	IDVolRoleChan_AdonisChannel,
	IDVolRoleChan_UWBDChannel,
	IDVolRoleChan_UDVB2Channel,
	IDVolRoleChan_UWBDS4Channel,
	IDVolRoleChan_Last
};
static char* VolRoleChan[]=
{
	"Undefine",
	"AnalysisStation",
	"TiburonChannel",
	"AdonisChannel",
	"UWBD_Channel",
	"UDVB2_Channel",
	"UWBDS4_Channel",
	""
};

enum IDVolStateDem
{
	IDVol_Lock=0,
	IDVol_Unlock,
	IDVol_Relock,
	IDVol_Scan,

	IDVolStateDem_Last
};
static char* VolStateDem[]=
{
	"Lock",
	"Unlock",
	"Relock",
	N_Scanner,
	""
};

enum IDVolControlParam
{
	IDVol_TCPIP=0,
	IDVol_RealPort,
	
	IDVolControlParam_Last
};
static char* VolControlParam[]=
{ 
	"TCPIP",
	"RealPort",
	""
};
//--Значения #PFormat

#define	VolFormatByteIQ		"ByteIQ"			// формат данных в потоке(канале) byte-Complex
#define	VolFormatShort		"Short"			// формат данных в потоке(канале) short
#define	VolFormatShortIQ		"ShortIQ"		// формат данных в потоке(канале) short-Complex
#define	VolFormatFloat			"Float"			// формат данных в потоке(канале) float
#define	VolFormatFloatIQ		"FloatIQ"		// формат данных в потоке(канале) float-Complex
#define	VolFormatByte		"Byte"		// формат данных в потоке(канале) short-Complex
#define	VolFormatPCMa			"PCMa"			// формат данных в потоке(канале) PCMa
#define	VolFormatPCMm			"PCMm"			// формат данных в потоке(канале) PCMm
#define	VolFormatADPCM			"ADPCM"			// формат данных в потоке(канале) ADPCM
#define	VolFormatGSM610			"GSM6.10"		// формат данных в потоке(канале) Gsm 6.10
#define	VolFormatStereoPCMa		"StereoPCMa"	// формат данных в потоке(канале) Gsm 6.10
#define	VolFormatStereoPCMm		"StereoPCMm"	// формат данных в потоке(канале) Gsm 6.10
#define	VolFormatStereoPCMaD5	"StereoPCMaD5"	// D5
#define	VolFormatGSM			"GSM"			// формат данных в потоке(канале) Frame Gsm (AMR,FR,HR,EFR,...)
#define	VolFormatABIS			"ABIS"			// формат данных в потоке(канале) ABIS
#define	VolFormatGSM_HR			"GSM_HR"			// формат данных в потоке(канале) Frame Gsm (AMR,FR,HR,EFR,...)
#define	VolFormatGSM_FR			"GSM_FR"			// формат данных в потоке(канале) Frame Gsm (AMR,FR,HR,EFR,...)

//#define	VolFormatABIS			"ABIS"			// формат данных в потоке(канале) ABIS

// ====================================
enum IDVolumeFormat
{
	FormatFloat=0,
	FormatFloatIQ,
	FormatPCMa,
	FormatPCMm,
	FormatADPCM,
	FormatGSM610,
	FormatStereoPCMa,
	FormatStereoPCMm,
	FormatStereoPCMaD5
};
// ====================================
// ====================================
// ====================================

#define		NumStPack	1000
#define		NumBDWPack	100
#define		NumSIPack	10

#define		HandleTable	100000				// хендл событий от таблиц (для использования в UserCallBack)
#define		HandleOther	100001				// !!!!!!!!!!!!!!!!!!!!!!!!

// 
// //		Правила введения и использования параметров.
// //	# - признак параметра который необходимо занести в базу данных
// //	@ - признак параметра который является оперативно необходимым
// //  др. - пользывательский параметр 
// //	S(s) - Строковая переменная (уникальная)
// //	C(c) - неуникальная Строковая переменная (вероятность повторения которых высока)
// //	I(i) - Целочисленная переменная (INT)
// //	D(d) - Целочисленная переменная (INT64)
// //	F(f) - переменная с Плавающей запятой (FLOAT)
// //	E(e) - переменная с Плавающей запятой (DOUBLE)
// //	//L    - структура PList (?)
// //  V - Структура PrmDec  например @VIniPath "D:\Cfg\Ini.cfg"
// //	!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!! Del T	 - текстовая строка с параметрами
// //	Заглавная буква означает необходимость индексации данной переменной
// //	строчная обозначает ее запись без индексации (по индексированным переменным
// //	возможен быстрый поиск!). 
// //	Список зарегистрированных параметров
// 
// #define	PrmIPFrom		"#CIPFrom"		// IP Отправителя
// #define	PrmIPTo			"#CIPTo"		// IP Получателя
// #define	PrmIIPFrom		"#IIPFrom"		// IP Отправителя
// #define	PrmIIPTo		"#IIPTo"		// IP Получателя
// #define	PrmPortFrom		"#IPortFrom"	// Port Отправителя
// #define	PrmPortTo		"#IPortTo"		// Port Получателя
// #define	PrmService		"#CService"		// сервис
// #define	PrmFileType		"#CFileType"	// тип файла
// #define	PrmFileLen		"#IFileLen"		// длинны файла
// //#define	PrmFileExt		"#CFileExt"		// расширение файла
// #define	PrmFileExExt	"#CFileExExt"	// дополнительное расширение файла (определяется когда #CExtFile уже определено)
// #define	PrmContentType	"#CContentType"	// тип содержимого файла
// #define	PrmPayloadType	"#CPayloadType"	// тип содержимого файла для медиаданных
// // #define	PrmFSrvName		"#CFSrvName"	// имя файлового сервера на котором записан файл
// 
// #define	PrmFlagCRC		"@IFlagCRC"		// Признак наличия в пакете CRC
// #define	PrmCutOff		"@DCutOff"		// Признак обрезания конца файла
// 
// //	Идентификаторы параметров в системе обработки потоков
// #define	PrmIDStream		"#GStreamList"	// идентификатор потока, определяется системным администратором, присваивается входным устройством
// #define	PrmIDLogChannel	"#GLogChannel"	// идентификатор логического канала, определяется системным администратором, присваивается входным устройством
// #define	PrmIDDemChannel	"#GDemChannel"	// идентификатор демодуляционного канала
// #define	PrmNPhChan		"#INPhChan"		// номер физического канала
// #define	PrmIDGroop		"#PGroupList"   // идентификатор группы, определяется системным администратором, присваивается входным устройством
// #define	PrmNLogChan		"#INLogChan"	// номер логического канала
// #define	PrmNDemChan		"#INDemChan"	// номер логического канала
// #define	PrmDVB_PID		"#IDVB_PID"		// PID потока  DVB
// #define	PrmTypeStream	"#PTypeStream"  // тип потока, определяется разработчиком
// #define	PrmStreamList	"#PStreamList"  // идентификатор потока, определяется системным администратором, присваивается входным устройством
// #define	PrmProtocol		"#PProtocol"	// тип протокола в потоке, определяется разработчиком
// #define	PrmFrequencyRange	"#PFrequencyRange"	// тип частотного диапазона, определяется разработчиком
// #define	PrmFormat		"#PFormat"		// формат данных в потоке(канале), определяется разработчиком (PCMa,PCMb,ADPCM,..)
// #define	PrmEncoding		"@pEncoding"	// тип кодирования
// #define	PrmIDDuplStream	"@gStreamList"	// ID потока обратного(в дуплексе) для текущего, определяется системным администратором 
// #define	PrmProfileList	"@pProfileList"	// идентифекатор profile
// #define	PrmQuality		"@fQuality"		// 
// #define	PrmStrLoad		"@fLoading"		// 
// #define	PrmNumSyn		"@iNSyn"	// 
// #define	PrmNumSynAll	"@iNSynAll"	// 
// 
// #define	PrmTDMA_Sin		"@cTDMA_Sin"	// 
// #define	PrmTDMA_FN		"@iTDMA_FN"		// 
// #define	PrmTDMA_SFN		"@iTDMA_SFN"	// 
// #define	PrmTDMA_MFN		"@iTDMA_MFN"	// 
// #define	PrmTDMA_HFN		"@iTDMA_HFN"	// 
// #define PrmThur_SeansID "@iThur_SeansID"// Идентификатор сеаса связи двух половинок дуплекса в ССС Thuraya для дешифратора.
// #define PrmRandomInf	"#iRandomInf"   // Случайное значение от 0 до 31 для отличия назначаемых сеансов в ССС Thuraya.
// 
// #define	PrmFileInfo		"FileInfo"
// #define	PrmHttpInfo		"HttpInfo"
// #define	PrmPOP3Info		"POP3Info"
// #define	PrmSMTPInfo		"SMTPInfo"
// #define	PrmIODLCAddr	"#iODLCAddress"
// #define	PrmIODLCSess	"#iODLCSession"
// #define	PrmCallNumFrom	"#cCallNumberFrom"
// #define	PrmCallNumTo	"#cCallNumberTo"
// #define	PrmSIPFrom		"#cSIPFrom"
// #define	PrmSIPTo		"#cSIPTo"
// #define	PrmSIPFromPAuth	"#cSIPFromProxyAuth"
// #define	PrmSIPToPAuth	"#cSIPToProxyAuth"
// 
// #define	PrmHDLC_All		"@iHDLC_All"
// #define	PrmHDLC_ErrSize	"@iHDLC_ErrSize"
// #define	PrmHDLC_OthHDLC	"@iHDLC_OthHDLC"
// #define	PrmHDLC_ErrIP	"@iHDLC_ErrIP"
// #define	PrmCRC_OK		"@iCRC_OK"
// #define	PrmCRC_ERR		"@iCRC_ERR"
// #define	PrmHDLC_TCP		"@iHDLC_TCP"
// #define	PrmHDLC_UDP		"@iHDLC_UDP"
// 
// #define	PrmHTTP_CLen	"#iHTTP_ContentLenght"
// #define	PrmHTTP_CRange	"#cHTTP_ContentRange"
// #define	PrmHTTP_ETag	"#cHTTP_ETag"
// #define	PrmHTTP_Host	"#cHTTP_Host"
// 
// #define	PrmFrequency	"#IFrequency"
// 
// 
// 
// 
// #define	PrmDTXChan		"#IDataType"
// 
// #define	PrmADC			"@IADC"			// переполнение АЦП
// 
// #define	PrmIDParStream	"#DParentStream"// родитель-поток для логических каналов или child-потоков 
// 
// #define	PrmTypePacket	"@ITypePacket"	// передача пакетной информации с типом <TypePacket>(пример:STS_HDLC)
// 
// #define	PrmDPC			"#IDPC"			// адресная часть для СС7
// #define	PrmOPC			"#IOPC"			// адресная часть для СС7
// #define	PrmCIC			"#ICIC"			// адресная часть для СС7
// 
// #define	PrmMUX			"#iMUX"			// адресная часть для Telefony
// 
// #define	PrmIPHost		"@CIPHost"		// IP адрес host
// 
// #define	PrmProcessed	"@cProcessed"	// Обрабатывать поток (если On),или нет(если Off)
// #define	PrmTimeSlot		"@CTimeSlot"	// Номер(а) физического(их) канала(ов)
// #define	PrmSlotSelect	"@iSlotSelect"	// Выборка битов физического канала
// #define	PrmCBufferiz	"@CNoBuffering"	// Буферизация канала, если On - выключена, иначе включена
// 
// #define	PrmTopUID		"@dTopUID"		// 
// 
// //	Идентификаторы параметров для сеанса 
// #define	PrmCalledN		"#CCalledN"			// 1 - вызываемый телефонный номер [+ "_N" - где номер дополнительного номера]
// #define	PrmCallingN		"#CCallingN"		// 2 - вызывающий телефонный номер [+ "_N" - где номер дополнительного номера]
// #define	PrmConnectN		"#CConnectN"		// 3 - присоедененный телефонный номер [+ "_N" - где номер дополнительного номера]
// #define	PrmRedirctN		"#CRedirctingN"		// 4 - перенаправленный телефонный номер [+ "_N" - где номер дополнительного номера]
// #define	PrmLocationN	"#CLocationN"		// 5 - номер местоположения [+ "_N" - где номер дополнительного номера]
// #define	PrmOriginalN	"#COriginalN"		// 6 - телефонный номер [+ "_N" - где номер дополнительного номера]
// #define	PrmRedirctionN	"#CRedirctionN"		// 7 - номер переадресации [+ "_N" - где номер дополнительного номера]
// 
// 
// 
// 
// #define	PrmTypSTS		"#ITypSTS"			// тип сигнализации
// #define	PrmIDParentSns	"#DIDParentSean"	// IDFile санса на сервере телефонии 
// #define	PrmTypeSnsData	"#ITypeSeansData"	// вид нагрузки сеанса (STS_TypDataISDN,STS_TypDataModem,STS_TypDataFax)
// //////////////////////////////////////////////////////////////////////////
// #define	PrmMonSystems	"@sMonitoringSystem"
// #define	PrmHostName		"@sHostName"
// #define	PrmMethName		"@sMethName"
// #define	PrmTypeMsg		"@sTypeMsg"
// //////////////////////////////////////////////////////////////////////////
// 
// #define	PrmMode			"@cMode"		// режим обработки
// 
// #define	PrmModeSDec		"@pModeSDec"		// режим обработки
// #define	PrmMServer		"@cMainServer"		// 
// #define	PrmSServer		"@cSecondServer"	// 
// #define	PrmNetPath		"@sNetPath"			// Сетевой путь (например к FSrv) 
// 
// #define	PrmDemod		"@pDemodulator"	// режимы демдуляции
// 
// #define	PrmIDICli		"@cSendIDI"			// параметр подсистемы NetIDICli для отбора IDI по его параметрам
// // * - присылать все IDI
// // <NamePrm> - присылать все IDI c данным параметром
// // <NamePrm>,<VolPrm> - присылать все IDI c данным параметром и данным значениям
// #define	PrmDemS_N			"@fS/N,dB"
// #define	PrmDemFd			"@eFd,Hz"//"@fFd,Hz"
// #define	PrmDemFn			"@eFn,Hz"//"@fFn,Hz"
// #define	PrmDemFb			"@eFb,Hz"//"@fFb,Hz"
// 
// #define	PrmFrec			    "@iFrec"
// #define	PrmFreqRec		    "@iFreqRec"
// #define	PrmBaseFrec		    "@fBaseFrequency"
// #define	PrmStepFrec		    "@fStepFrequency"
// #define	PrmShiftFrec	    "@fShiftFrequency"
// 
// #define PrmState		"@cState"		//Vol=Work Close Off
// 
// #define PrmCommRxName		"@sNetCommRx"	//Имя сервера NetCommRx для группы клиентов одного NetCommТx
// 
// #define PrmNameMt_BD		"#cName_Mt"
// #define PrmAuth_BD			"#cAuth_"
// #define PrmVer_BD			"#cVer_"
// 
// #define PrmTimeBegin_BD		"#DTimeBegin"
// #define PrmTimeClose_BD	    "#DTimeClose"
// #define PrmTimeLastWr_BD	"#DTimeLastWr"
// 
// #define PrmIDFile			"@DIDFile"
// 
// #define PrmIDFileParent_BD	"#DID_ParentFile"
// #define PrmIDFileSnsFr_BD	"#DID_SeansFrom"
// #define PrmIDFileSnsTo_BD	"#DID_SeansTo"
// 
// #define PrmIDFileSnsFr		"@DIDFileSnsFr"
// #define PrmIDFileSnsTo		"@DIDFileSnsTo"
// #define PrmIDFileParent		"@DIDFileParent"
// 
// #define PrmNumberInList		"@iNumberInList"
// #define PrmNumberFrChan		"@iNumberFrChan"
// 
// #define PrmTimeBegin		"@DTimeBegin"
// #define PrmTimeEnd		    "@DTimeEnd"
// 
// #define PrmCodeState		"@cCodeState"	//код страны для правил канонизации (Пример(для Укр.):380)
// #define PrmCodeSity			"@cCodeSity"	//коды городов(операторов) для правил канонизации (Пример(для Укр.):31-39,41,43,44,46-48,50-57,61,62,64-67,69,97)
// #define PrmLenNRState		"@iLenNRState"	//длина номера страны для правил канонизации (Пример(для Укр.):12)если нет или равен 0 (то длина не важна)
// #define PrmRuleCanon		"@pRuleCanonization"//правило канонизации для потока
// #define PrmLenNRSubscriber	"@iLenNRSubscriber"	//длина номера абонента для правил канонизации (Пример(для Укр.):7)если нет или равен 0 (то длина не важна)
// #define PrmPrefix			"@cPrefix"		//возможная приставка к номеру абонента для правил канонизации (Пример(для Укр.):10,15,2)если нет или равен 0 (то длина не важна)
// #define PrmLenNRToPrefix	"@cLenNRToPrefix"//длина NR до приставки к номеру абонента для правил канонизации (Пример(для Укр.):10,15,2)если нет или равен 0 (то префикс не проверяется)
// 
// #define PrmDelFile			"@cDelFile"		//файл удален (для BDSrv) ="On"
// 
// 
// #define PrmNameMethod		"@CMethod"
// #define PrmNameSubtr		"@CSubTr"
// #define PrmNameSubtrIDI		"@CSubTrIDI"
// 
// #define PrmUIDICLList		"@DUIDICLList" // идентифекатор родительского ClassList
// 
// #define PrmDirection		"@cDirection"	//направление передачи
// 
// 
// #define PrmUnitName			"@cUnitName"	//UnitName
// #define PrmUnitID			"@cUnitID"		//UnitID
// 
// 
// //PilotK
// #define PrmTemperature		"@iTemperature"		//Temperature
// #define PrmCurrent			"@fCurrent"			//current
// #define PrmVoltage			"@fVoltage"			//voltage
// #define PrmThresholdVoltage	"@fThresholdVoltage"//PN порог по напряжению
// #define PrmThresholdCurrent "@fThresholdCurrent"//PT порог по току
// #define PrmAZA				"@cAutoQueryAON"		//
// #define PrmBLOK				"@cBlock"		//
// #define PrmK				"@cCross"		//
// #define PrmONN				"@cONN"			//
// #define PrmTAK				"@cTypeSubscrSet"			//
// #define VolTAKSP1			"SP1"			//
// #define VolTAKSP2			"SP2"			//
// #define VolTAKI				"Individual"	//
// #define PrmPKMode			"@cPKMode"		//
// #define VolPKModeVolt		"Voltage"		//
// #define VolPKModeCurr		"Current"		//
// #define VolPKModeCurrVolt	"Voltage&Current"	//
// #define PrmDZA				"@cTimeQueryAON"	//
// #define VolDZA100			"100"			//
// #define VolDZA180			"180"			//
// #define VolDZA260			"260"			//
// #define VolDZA340			"340"			//
// 
// #define PrmPriority			"#iPriority"
// 
// #define PrmPassive			"@cPassive"
// #define PrmActiv			"@cActiv"
// #define PrmFAX				"@cFAX"
// #define PrmModem			"@cModem"
// #define PrmAON				"@cfAON"
// #define PrmOON				"@cOON"
// #define PrmCallerID			"@cfCallerID"
// #define PrmDTMF				"@cfDTMF"
// #define PrmImpulse			"@cfImpulse"
// //#define PrmID				"ID"
// #define PrmCommand			"Command"
// 
// #define PrmVolAON			"AON"
// #define PrmVolCallerID		"CallerID"
// #define PrmVolDTMF			"DTMF"
// #define PrmType				"Type"
// #define PrmText				"Text"
// 
// #define PrmAutoTypeSubscrSet "AutoTypeSubscrSet"			//
// #define PrmAutoVoltage		"AutoThresholdVoltage"
// #define PrmCrossState		"CrossState"
// 
// #define PrmEmergency		"Emergency"	//Авария On Off
// #define PrmTRangeViolation	"TRangeViolation"	//NTP On Off
// #define PrmFuseDamage		"FuseDamage"	//PP On Off
// #define PrmEmergencySupply	"EmergencySupply"	//PNAP On Off
// #define PrmSignalingCableDamage	"SignalingCableDamage"	//OSC On Off
// #define PrmSpecSignalingCableDamage	"SpecSignalingCableDamage"	//OSCC On Off
// 
// #define PrmSignalingCableCntrl1	"SignalingCableCntrl_1"	//OSC On Off
// #define PrmSpecSignalingCableCntrl1	"SpecSignalingCableCntrl_1"	//OSCC On Off
// 
// #define PrmSignalingCableCntrl2	"SignalingCableCntrl_2"	//OSC On Off
// #define PrmSpecSignalingCableCntrl2	"SpecSignalingCableCntrl_2"	//OSCC On Off
// 
// #define PrmTempSensor		"TemperaturSensor"	// On Off
// #define PrmTempInterval		"TemperaturInterval"	// On Off
// #define PrmTempHiLimit		"TemperaturHiLimit"	// On Off
// #define PrmTempLoLimit		"TemperaturLoLimit"	// On Off
// #define PrmEventCounter		"EventCounter"	//EventCounter
// 
// #define PrmRelayOut1		"RelayOut1"	//RO1 On Off
// #define PrmSignalingCable1	"SignalingCable1"	//DC1 On Off
// #define PrmFuseDamage1		"FuseDamage1"	//PP On Off
// #define PrmEmergencySupply1	"EmergencySupply1"	//PNAP On Off
// #define PrmSpecSignalingCableDamage1 "SpecSignalingCableDamage1"	//OSCC On Off
// 
// #define PrmRelayOut2		"RelayOut2"	//RO1 On Off
// #define PrmSignalingCable2	"SignalingCable2"	//DC2 On Off
// #define PrmFuseDamage2		"FuseDamage2"	//PP On Off
// #define PrmEmergencySupply2	"EmergencySupply2"	//PNAP On Off
// #define PrmSpecSignalingCableDamage2 "SpecSignalingCableDamage2"	//OSCC On Off
// 
// #define PrmTopLimit			"TopLimit"		//VP On Off
// #define PrmLowerLimit		"LowerLimit"	//NP On Off
// 
// #define PrmTime				"Time"	//Day On Off
// 
// #define PrmTypeCli			"TypeCli"	//Day On Offpe
// #define PrmHandle			"Handle"
// 
// //for MS-19
// #define PrmRemoteSup		"RemoteSupply"
// #define PrmRS_Type			"RS_Type"
// #define PrmSLK				"SLK"
// #define PrmFTap				"ForcTap"
// #define PrmLineType			"LineType"//M0-M1
// #define PrmShCirc			"ShortCircuit"
// #define PrmBrokWire			"BrokenWire"
// 
// #define PrmAutoSLK			"AutoSLK"
// #define PrmAutoLineType		"AutoLineType"
// 
// #define PrmSatellites		"Satellite"
// #define PrmTransponders		"Transponder"
// #define PrmGetoradin		"Getoradin"
// #define NameCarriers		"Carrier"
// #define PrmHiFrequency		"HiFrequency"
// #define PrmLowFrequency		"LowFrequency"
// 
// 
// //////////////////////////////////////////////////////////////////////////
// //  [2/21/2007 San] Thur
// #define PrmSatLatitude		"#eSatLatitude"
// #define PrmSatLongitude		"#eSatLongitude"
// #define PrmRadius			"#eRadius"
// #define PrmBeamLatitude		"#eBeamLatitude"
// #define PrmBeamLongitude	"#eBeamLongitude"
// #define PrmSpotID			"#ISpotID"
// #define PrmSatID			"#iSatID"
// #define PrmSMS				"#CSMS"
// 
// #define PrmEstCause		"#CEstablishCause"
// #define PrmLatitude		"#eLatitude"
// #define PrmLongitude	"#eLongitude"
// 
// #define PrmIMSI			"#CIMSI"
// #define PrmTMSI			"#CTMSI"
// #define PrmIMEI			"#CIMEI"
// #define PrmIMEISV		"#CIMEISV"
// #define PrmtempTMSI		"#CtempTMSI"
// 
// #define PrmFullNameUp	"#cPathFullNameUp"
// #define PrmFullNameDn	"#cPathFullNameDown"
// #define PrmFullNameDnVoc	"#cPathFullNameDownVoc"
// #define PrmFullNameUpVoc	"#cPathFullNameUpVoc"
// 

// #define PrmDay				"Day"	//Day On Off
// #define PrmDayOfWeek		"DayOfWeek"	//DayOfWeek On Off
// #define PrmMonth			"Month"	//Month On Off
// #define PrmYear				"Year"	//Year On Off
// #define PrmHour				"Hour"	//Hour On Off
// #define PrmMinute			"Minute"	//Minute On Off
// #define PrmSecond			"Second"	//Second On Off

//!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!	#define	PrmNewStream	"#TNewStream"	// параметр с описанием способа формирования нового потока, определяется разработчиком
//!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!	#define	PrmNewLogChan	"#TNewLogChan"	// параметр с описанием способа формирования нового логического канала, определяется разработчиком
//
//ParamStream	List    определяется разработчиком, описывается
//			через конфигуратор
//	Значения TypeStream
//	Unknown
//	Undefine
//	G.704		Простой демультиплексор кадра G.704,
//			формирует логические каналы или другие потоки
//	TCP/IP          Сплошной поток TCP/IP
//	SMPRS           Компрессированный поток TCP/IP
//	PPP		Point to Point protocol
//	PackTCP/IP	Выделенный попакетно TCP/IP
//	X.25            Потокол X.25
//	X.75            Потокол X.75
//	PD		Смешанные типы протоколов передачи данных
//	DTX-240         Оборудование DTX-240

//************************************************************************
//************************************************************************
//  Значения ID_Standart из структуры IDInfo
//0x00000000 - 0x0000ffff

//0x00010000 - 0x0001ffff
#define	Standart_PackHDLC	0x00010000
#define	Standart_HDLC		0x00010001

#define	Standart_PackODLC	0x00010010
#define	Standart_ODLC		0x00010011

//0x00020000 - 0x0002ffff

//0x00030000 - 0x0003ffff

//0x00040000 - 0x0004ffff



#define StandartUndef	0xffffffff


//************************************************************************
//************************************************************************


// Глобальные команды службы сетевого взаимодействия
// ---------------------------
#define	COM_Defaul		0x00000000
// ---------------------------
#define	COM_Start		0x00000001	// Старт всех систем
//	
#define	COM_Stop		0x00000002	// Стоп всех систем
//	
#define	COM_Data		0x00000003	// Передача блока данных
//	char* Buf
#define	COM_WatchDog	0x00000004	// Передача тестирующего пакета
//
#define COM_Ack			0x00000005  // Подтверждение принятого пакета
// USHORT	Cnt
// ---------------------------
//#define	NET_QueryPack	0x00000010	//
//
//#define	NET_UDPPack		0x00000011	// 
//
#define	NET_ReplElem		0x00000020	//
// int Version,int NumHost, int64 - UID Elem,int64 TimeM,int64 UpUID,int TypeKey,char Elem[128],char	Remark[128],char UserName[80];
#define	NET_ReplParam		0x00000021	// 
// int Version,int NumHost, int64 - UID Elem,int64 TimeM,PrmDec;
#define	NET_QueryReplElem	0x00000022	// 
// int Version,int NumHost, int64 - UID Elem 
#define	NET_DelElem			0x00000023	//
// int Version,int NumHost, int64 - UID Elem,int64 TimeM
#define	NET_DelParam		0x00000024	// 
// int Version,int NumHost, int64 - UID Elem,int64 TimeM,PrmDec;
#define	NET_QueryAllRepl	0x00000025	// Запрос всей репликации
// int Version
#define	NET_DelAll			0x00000026	// удалить все элементы списка
// int Version
// int MyNum - номер клиента посылающего комаду
#define	NET_DelAck			0x00000027	// удалить все элементы списка
// int Version
// int MyNum - номер клиента посылающего комаду
#define	NET_ReplEnd			0x00000028	//
// -------- System Event -------
#define	EV_Defaul			0x00001000	// 

#define	EV_StartServer		0x00001001	// 
#define	EV_StopServer		0x00001002	// 

#define	EV_AddUnit			0x00001003	// 
#define	EV_DelUnit			0x00001004	// 
#define	EV_ModifUnit		0x00001005	// 

#define	EV_StartCtrlDomainServer	0x00001006	// 
#define	EV_StopCtrlDomainServer		0x00001007	// 
#define	EV_CtrlDomainServer			0x00001008	// 

#define	EV_Last				0x00001fff	// 
// -----------------------------
#define	SYS_Defaul		0x00010000
//
#define	SYS_NewUnit		0x00010001	// зарегистрировано новое устройство
//	N_Cli
#define	SYS_UnitClose	0x00010002	// устройство отсоединено от системы
//	N_Cli
#define	SYS_GetTSC		0x00010003	// запрос таблицы TabSysCli и выдача списка Хостов
//	
#define	SYS_TSC			0x00010004	// таблица TabSysCli
//	N_Cli,TabSysCli[N_Cli]
//	N_Cli=-1 Признак конца таблицы
#define	SYS_UnitUpDate	0x00010005	// обновить параметры устройства
//	N_Cli,TabSysCli[N_Cli]
#define	SYS_UnitInfo	0x00010006	// параметры устойства
//	UnitInfo
#define	SYS_SetPersID	0x00010007	// определить устройству персональный ID
//  String
#define	SYS_GetUnitInfo	0x00010008	// запросить параметры устройства
//
#define	SYS_AccessAv	0x00010009	// запросить параметры устройства
//
#define	SYS_AccessDen	0x00010010	// запросить параметры устройства
//
#define	SYS_NewHost		0x00010020	// запросить параметры устройства
//	String HostName
#define	SYS_HostClose	0x00010021	// запросить параметры устройства
//	String HostName
#define	SYS_HostError	0x00010022	// запросить параметры устройства
//	String HostName
#define	SYS_IDInfo		0x00010023	// запросить параметры устройства
// Struct UDPPack
#define SYS_UDPPack		0x00010024	//
// ---------------------------
#define	INM_Defaul		0x00020000	// зарезервированно для системы Inmarsat
// Список пакетов в системе Inmarsat
#define	INM_StartWC		0x00020010	// Запустить рабочий канал		(общий)
//	+
#define	INM_StopWC		0x00020011	// Остановить рабочий канал		(общий)
//	+
#define	INM_PackNCS		0x00020012	// Пакет с NCS
//	+nsc_info_2
//#define	INM_SetModeWC	0x00020013	// Установить режим рабочего канала
//	-
#define	INM_DataWC		0x00020014	// Сообщить переменные рабочего канала
//	+WCData
#define	INM_GetDataWC	0x00020015	// Запрос переменных рабочего канала 
//	+
#define	INM_DemStatOut	0x00020017	// Структура с параметрами демодулятора			------------------
//	+DemStatOut
#define	INM_DemStar		0x00020018	// Сигнальное созвездие
//	+X1,Y1,X2,Y2,...
#define	INM_PackWAV		0x00020019	// Аудио данные
//	+(short*)Buf
#define	INM_SetScrembler	0x0002001a	// Установить скремблер в другом канале дуплекса
//	+Scrembler
#define	INM_StartSeans	0x0002001b	// Команда от сервера о начале сеанса		!!!
//	+SeInfo
//#define	INM_SynchTime	0x0002001c	// Синхронизация времени
//	+SystemTime
#define	INM_SetFreq		0x00020020	// Установить частоту в канале (по номеру)
//	+Freq
#define	INM_SetNumChan	0x00020021	// Установить частоту в канале (по номеру)
//	+NumChannal
#define	INM_OnDetect	0x00020022	// Включить автоопределение с буферизацией 
//	+
#define	INM_OnStM		0x00020023	// Включить стандарт M
//	+
#define	INM_OnStmM		0x00020024	// Включить стандарт mini-M
//	+
#define	INM_OnNCS		0x00020025	// Включить NCS
//	-
#define	INM_ModeDetect	0x00020026	// режим определен (данные читать из WCData)
//	+
#define	INM_EndSeans	0x00020027	// Событие о завершении сеанса
//	+SeInfo
#define	INM_WAVOn		0x00020028	// Включить передачу аудио данных
#define	INM_WAVOff		0x00020029	// Выключить передачу аудио данных
#define	INM_ResetWC		0x0002002a	// Сброс операции в рабочем канале
//	+
#define	INM_SeansContinue	0x0002002b	// Сообщение второй половине дуплекса о том что сеанс продолжается 
//	+	
#define	INM_DataDBW		0x0002002c	// Сообщить переменные DB-врайтера
//	+ DBWData
#define	INM_QueryMaxIDSe	0x0002002e	// Запрос максимального идентификатора сеанса
//	+
#define	INM_MaxIDSe		0x0002002f	// Максимальноый идентификатор сеанса
//	+	INT64 MaxIDSe
#define	INM_QueryDBMES	0x00020030	// Запрос передачи таблицы MES-ов
//	+
#define	INM_DBMES		0x00020031	// Передать строчку из таблицы MES-ов
//	+ DBMES
#define	INM_EndDBMES	0x00020032	// Сообщить о том, что таблица MES-ов передана полностью
//	+
#define	INM_SeansInfo	0x00020033	// Информация о сеансе
//	+ InmSeansInfo
#define	INM_DBStat		0x00020034	// Передать строчку из таблицы MES-ов
//	+ DBStat
#define	INM_SetStandart	0x00020035	//  Установить стандарт
//	+ Standart (uchar)
#define	INM_AddFile		0x00020036	// 
//	- StrAddFile
#define	INM_GetFile		0x00020037	// 
//	- StrGetFile
#define	INM_SendFile	0x00020038	// 
//	- StrSendFile
#define	INM_ExcludeFile	0x00020039	// 
//	- StrExcludeFile
#define	INM_AddParam	0x0002003A	// 
//	- StrAddParam
#define	INM_GetError	0x0002003B
//
#define	INM_GetDupFile	0x0002003C	// 
//	- StrGetFile
#define	INM_SendDupFile	0x0002003D	// 
//	- StrSendDupFile
#define	INM_OnStM4		0x0002003e	// Включить стандарт M4
//	+
#define	INM_OnStB		0x0002003f	// Включить стандарт B
//	+
#define	INM_QueryTlfNum	0x00020040	// Запрос передачи таблицы Телефонов
//	+
#define	INM_TlfNum		0x00020041	// Передать от БазыДанных строчку из таблицы Телефонов
//	+ TlfNum
#define	INM_EndTlfNum	0x00020042	// Сообщить о том, что таблица Телефонов передана полностью
//	+
#define	INM_PackNCSC	0x00020043	// Пакет с NCS С - стандарта
//	+ NCSCInfo
#define	INM_ChangePrior	0x00020044	// 
//	+ int dPrior
#define	INM_SendTlfNum	0x00020045	// Телефонный номер (после определения)
//  + String TlfNum
#define	INM_EndSeansC	0x00020046	// Событие о завершении сеанса
//	+ SeInfo
#define	INM_UsedChN		0x00020047
//	+ ScanInfo
#define	INM_Default		0x00020000	// 
// ------------------

#define	STS_Default	0x00030000	// зарезервированно для системы St-5 St-7 EDSS & др.
// Cтруктуры находятся в <MStrTlf.h>
//enum STS_DefaulEnum
//{
//	STS_xxxxx=STS_Default,    //0x00030000
#define	STS_Complex	0x00030001		//Сложный пакет 0x00030001
//-(STS_Complex,[Длина1,[STS_X1,[Data1]]...{ДлинаN,[STS_XN,[DataN]]}])
#define	STS_HDLC	0x00030002		//HDLC-Пакет возможно с ОКС7(от СОП к серверу)  0x00030002
//-(INT64)IDStream,(INT64)NLogCh,CntErrHDLC,CntPack=N,[Len1pack,Packet1,...[LenNpack,PacketN]]
//-(INT64)IDStream,(INT64)NLogCh,CntErrHDLC,CntPack=N,[Len1pack,Packet1,...[LenNpack,PacketN]]
#define	STS_GetUnitID	0x00030003	//Запрос на получения UnitID при присоединении СОП к серверу (от сервера к СОП) 0x00030003
#define	STS_SetUnitID	0x00030004	//Ответ на запрос на получения UnitID при присоединении СОП(от СОП к серверу) 0x00030004
//UnitID		//40 байт со стройкой UnitID
#define	STS_BegLogStream	0x00030005//Начало нового потока  0x00030005
//<IDStream-INT64><ID_File-INT64>
#define	STS_EndLogStream	0x00030006//Окончание нового потока 0x00030006
//<IDStream-INT64><ID_File-INT64>
#define	STS_BegLogChannel	0x00030007//Начало нового лог.канала 0x00030007
//<IDStream-INT64><IDLogChannel-INT64><ID_File-INT64> 0x00030008
#define	STS_EndLogChannel	0x00030008//Окончание нового лог.канала
//<IDStream-INT64><IDLogChannel-INT64><ID_File-INT64> 0x00030009
//STS_SetCnfSOP,		//Настройка СОП при присоединении (от сервера к СОП)
//-CnfSOP              
//-CntStreem=R-Количество потоков,[CnfStreem1,..,CnfStreemR]
//STS_SetColor,		//Установка цветовой конфигурации при присоединении СОП(от сервера к СОП)
//-ColorIni(структура с цветовой конфигурации)
#define	STS_SetLogPot		0x00030009//Включение І-лог. потока(по умолчанию)(от сервера к СОП) 0x0003000a
//-I(номер лог. потока)
#define	STS_ResetLogPot		0x0003000a//Выключение І-лог. потока(от сервера к СОП) 0x0003000b
//-I(номер лог. потока)
#define	STS_SeansData		0x0003000b//Данные о сеансе(от сервера к СОП)(от СОП к серверу) 0x0003000c
//S	//-(+4)IDStream,(+12)NLogChannel,(+20)ID_Sns,(+28)TypSTS,(+32)Len,(+36+X)|[STS_SeansNewConfirm|STS_SeansEndConfirm|
//s	//-(+4)IDFileStream,(+12)IDFileLogCh,(+20)ID_Sns,(+28)TypSTS,(+32)Len,(+36+X)[STS_SeansNew|STS_SeansEnd|STS_SeansIsISDN
//STS_SeansCalledN|STS_SeansCallingN|STS_SeansConnectN|STS_SeansRedirctN]
#define	STS_SeansNew		0x0003000c//Запрос на открытие сеанса(от сервера к СОП) 0x0003000d
#define	STS_SeansNewConfirm	0x0003000d//Подтверждение открытие сеанса(от СОП к серверу) 0x0003000e
#define	STS_SeansEnd		0x0003000e//Запрос на закрытие сеанса(от сервера к СОП) 0x0003000f
#define	STS_SeansEndConfirm	0x0003000f//Подтверждение закрытие сеанса(от СОП к серверу) 0x00030010	

#define	STS_SeansEndConfirm	0x0003000f//Подтверждение закрытие сеанса(от СОП к серверу) 0x00030010	


#define	STS_StopSOP		0x00030020//Остановить СОП 0x00030021
#define	STS_StartSOP	0x00030021//Запустить  СОП 0x00030022
#define	STS_SetRules	0x00030022//Установить правила отбора 0x00030023
#define	STS_GetRules	0x00030023//Запросить правила отбора	0x00030024

#define	STS_FullBegLogStream	0x00030024//Начало нового потока		0x00030025
//<IDStream-INT64><ID_File-INT64><UnitID_SOP-40 char>
//<CntLogChan=M-int>[<IDLogChannel1-INT64><ID_File1-INT64>...[<IDLogChannelM-INT64><ID_FileM-INT64>]]	
#define	STS_ConEndLogStream		0x00030025//Окончание нового потока	0x00030026
//<IDFile_Stream-INT64>
#define	STS_ConBegLogChannel	0x00030026//Начало нового лог.канала	0x00030027
//<IDFile_Stream-INT64><IDLogChannel-INT64><ID_File-INT64>
#define	STS_ConEndLogChannel	0x00030027//Окончание лог.канала		0x00030028
//<ID_File_LogChannel-INT64>
#define	STS_ConDiscUnitID		0x00030028//Отсоединение СОПа		0x00030029
//<UnitID_SOP-40 char>


#define	STS_OpenChSS		0x00030030//Открыть канал с сигнализацией 0x00030016
//(int)NStream-номер лог. потока,(int)NCh-номер канала,(int)TypSTS
#define	STS_SignalizPack	0x00030031
#define	STS_QueryNewSeans	0x00030032
#define	STS_OpenNewSeans	0x00030033
#define	STS_EndSeans		0x00030034
#define	STS_CloseSeans		0x00030035
#define	STS_ParamSeans		0x00030036

#define	STS_ParamElem		0x00030037//struct StrAddParamBandleList

#define	STS_VConsConnCli	0x00030040//Присоединение CliSt  и сообщение об этом VCon
//UnitInfo				//(int)NumCi,(INT64)TimeCr
#define	STS_VConsDiscCli	0x00030041//Отсоединение CliSt  и сообщение об этом VCon
//(INT64)PersID			//,(INT64)TimeCr	//(int)NumCi,(INT64)TimeCr
#define	STS_VSetUnitInfo	0x00030042//Идентификация CliST
//(int)NumCi,(UnitInfo CliST)UnitInfo
#define	STS_VBegLogStream	0x00030043//Начало нового потока на SP 0x00030005
//<IDStream-INT64><ID_File-INT64>
#define	STS_VEndLogStream	0x00030044//Окончание нового потока на SP 0x00030006
//<IDStream-INT64><ID_File-INT64>
#define	STS_GetDataIDFile	0x00030045//Запрос данных по IDFile (VCon->ST->SP)
//<ID_File-INT64>
#define	STS_SendDataIDFile	0x00030046//Поссыл данных по IDFile (SP->ST->VCon)
//<ID_File-INT64><SizBuff-int><Buff-буффер с данными>
#define	STS_EndDataIDFile	0x00030047//Завершение запроса данных по IDFile (VCon->ST->SP)
//<ID_File-INT64>
#define	STS_TypDataSeans	0x00030048//Установка типа данных для сеанса

#define	STS_SendSeansNumb	0x00030049//передача номерной информации на VCon
//struct StrSendPhoneNumb		

#define	STS_GetAudDataIDF	0x00030050//Запрос аудиоданных по IDFile стерео(VCon->ST)
//<ID_File-INT64>
#define	STS_SendAudDataIDF	0x00030051//Поссыл aудиоданных по IDFile (ST->VCon)
//struct	StrObjAudData
#define	STS_EndAudDataIDF	0x00030052//Окончание aудиоданных по IDFile (VCon->ST)
//<ID_File-INT64>

#define	STS_SendHostIP		0x00030053//Поссыл HostIP (ST->VCon)
//(int)NumCi,(int)IPCli
#define	STS_SendHostName	0x00030054//Поссыл HostName (ST->VCon)
//(int)NumCi,(char[48])HostName
#define	StructDemStatOut	0x00030055
#define	StructDemStarOut	0x00030056
#define	DemStatFd			0x00030057
//#define	DemStatFb			0x00030058
#define	StructParamDemStat	0x00030059



//-for Object Cli
#define	STS_ObjGetTable		0x00030060//Запросить объектовой таблицы (ОТ)	STS_ObjGetTable	FE->BE	NULL
#define	STS_ObjBegTable		0x00030061//Начало пересылки ОТ	STS_ObjBegTable	BE->FE	NULL
#define	STS_ObjEndTable		0x00030062//Окончание пересылки ОТ	STS_ObjEndTable	BE->FE	NULL
#define	STS_ObjPartTable	0x00030063//Часть ОТ	STS_ObjPartTable	BE->FE	<BoundPacket>*
#define	STS_ObjPhoneSns		0x00030064//Передача сеансовых тел. номеров	STS_ObjPhoneSns	FE->BE	struct StrSendPhoneOT
#define	STS_ObjDefinID		0x00030065//Передача определенных для сеанса Nobj (их может быть несколько)	STS_ObjDefinID	FE->BE	struct StrSendNObjOT
#define	STS_ObjGetAudSns	0x00030066//Запрос аудиоконтроля	STS_ObjGetAudSns	BE->FE	INT64 IDSeans;
#define	STS_ObjSetAudSns	0x00030067//Передача аудиоконтроля	STS_ObjGetAudSns	BE->FE	INT64 IDSeans;
#define	STS_ObjOpenSns		0x00030068//Получен сеанс	STS_ObjOpenSns	FE->BE	INT64 IDSeans;
#define	STS_ObjCloseSns		0x00030069//Окончание сеанса	STS_ObjCloseSns	FE->BE	INT64 IDSeans;
#define	STS_ObjEndAudSns	0x00030070//Окончание передачи аудиоконтроля	BE->FE или FE->BE(при окончание сеанса)	INT64 IDSeans;
#define	STS_ObjFindSns		0x00030071//Найден сеанс	STS_ObjFindSns	FE->BE	INT64 IDSeans;
//- for BDSrv
#define	STS_SendBDataSns	0x00030080//Передача данных сеанса	struct	PackForBD

//  [11/11/2008 San] For new TlfSrv
#define	STS_QueryConn		0x00030081//запрос на присоединения клиента(консоль) к серверу
//(UnitInfo CliST)UnitInfo
#define	STS_AnswrConn		0x00030082//ответ на запрос о присоединения клиента(консоль) к серверу (от сервера)

#define	STS_GetFullData		0x00030083//запрос о получении всех данных 

#define	STS_LogChEnergyOn	0x00030084//
#define	STS_LogChEnergyOff	0x00030085//
#define	STS_LogChSS7		0x00030086//
#define	STS_LogStrSS7		0x00030087//
#define	STS_OpenSeansSS7	0x00030088//
#define	STS_ClosSeansSS7	0x00030089//
#define	STS_LogStrSS7LnkCh	0x00030090//


//  [12/29/2008 San]
#define	STS_BegIDI			0x00030091//
#define	STS_EndIDI			0x00030092//
#define	STS_SendIDI			0x00030093//
#define	STS_SaveBegIDI		0x00030094//
#define	STS_SaveEndIDI		0x00030095//
#define	STS_SaveRetIDI		0x00030096//
#define	STS_IsExistIDI		0x00030097//

#define	STS_LogStrSS7Close	0x00030098//


#define	STS_IsNotExistIDI	0x00030099//
#define	STS_IsYesExistIDI	0x0003009a//

#define	STS_SeansNewGSM		0x00031000//Запрос на открытие сеанса(от сервера к СОП) 0x0003000d
#define	STS_SeansEndGSM		0x00031001//
#define	STS_SetChInfGSM		0x00031002//



//#define	STS_LogChSave		0x00030099//StrSendInf


//  [3/30/2005] VSV IP-телефония
//Срочные командыSrv
#define	IPTLF_ComSetupF			0x00030100//* UCII	Setup			SOP->Srv
#define	IPTLF_ComSetupS			0x00030101//* UII	Setup			Srv->SOP
#define	IPTLF_ComCallProc		0x00030102//* UII	CallProc		SOP->Srv->SOP
#define	IPTLF_ComIUPD			0x00030103//* UI	IPPort update	SOP->Srv->SOP
//Команды с буферизацией
#define	IPTLF_ComRTPRelease		0x00030104//* U...	RTPRelease (p=1)
#define	IPTLF_ComRTPStop		0x00030105//s U...	RTP Stop Seans 
#define	IPTLF_ComRTPTimeOut		0x00030106//s U...	RTPTimeout (p=300)
#define	IPTLF_ComSeansActivity	0x00030107//s U...	RTPActivity (p=60)

#define	STS_SavBegIDI_hex		0x00030120//
#define	STS_SavEndIDI_hex		0x00030121//
#define	STS_SavRetIDI_hex		0x00030122//


#define	STS_ReQueryConn			0x00030130//перезапрос на присоединения клиента(консоль) к серверу

// 
// 
// #define	PrmDecodeConv	"@cDecConv"	//Сверточный код "1/2", "2/3", "3/4"
// #define	PrmDecodeMO2	"@cDecMO2"	//Декодирование Mo2 "On", "Off"
// #define	PrmDecodeInv	"@cDecInv"	//Инверсия выходного потока "On", "Off"
// 
// 

#define	STS_LogChSaveOn		0x00030150//INT64 IDFLogCh
#define	STS_LogChSaveOff		0x00030151//INT64 IDFLogCh

//Команда для Сервера ІКМ
//а)	пакетів зі службовою інформацією
#define	STS_ReqSaveCfg	0x00030201//запит на видачу збереженої інформації про конфігурацію;
#define	STS_ReqCurrCfg	0x00030202//запит на видачу поточної конфігурації  СОП та конфігурації  СС7;
#define	STS_ReqEnd		0x00030203//кінець виконання запиту;
#define	STS_ReqNextPack	0x00030204//передати наступний пакет;
#define	STS_ReqReptPack	0x00030205//повторити пакет (пакет не прийнято); 
#define	STS_ReqBeg		0x00030206//початок виконання запиту;
//б)	інформаційних пакетів:
#define	STS_PackOpenCfgSP	0x00030210//конфігурація СОП  (запуск Neuron);
#define	STS_PackClosCfgSP	0x00030211//конфігурація СОП (закриття Neuron);
#define	STS_PackChanCfgSP	0x00030212//конфігурація СОП  (зміна конфігурації);
#define	STS_PackReqCfgSP	0x00030213//конфігурація СОП  (за запитом);
#define	STS_PackOpenCfgSS7	0x00030214//конфігурація СС7 (запуск Neuron);
#define	STS_PackClosCfgSS7	0x00030215//конфігурація СС7 (закриття Neuron);
#define	STS_PackChanCfgSS7	0x00030216//конфігурація СС7 (зміна конфігурації);
#define	STS_PackReqCfgSS7	0x00030217//конфігурація СС7 (за запитом);


#define	STS_PackGSM			0x00030300//GSM Message


//  [10/17/2005] Thuraya for SrvTlf
#define STS_SetModeStream		0x00031010//Назначение режима static | dinamic
//SSO->ST struct StrModeStream
#define STS_CommPCH_ImmAssignPh	0x00031020//Комманда из сигнализ.PCH Immediate Assignment для занятия физ.канала
//ST->SSO struct StrImmAssignPh
#define STS_CommPCH_ImmAssignTS	0x00031021//Комманда из сигнализ.PCH Immediate Assignment для занятия лог.канала
//ST->SSO struct StrImmAssignTS


#define	DigitRecPCI_Start 0x00031028 // Команда Start
#define	DigitRecPCI_Stop  0x00031029 // Команда Stop
#define	DigitRecPCI_Frec  0x00031030 // Команда настройки частоты приемника
#define	DigitRecPCI_AGC   0x00031051 // Команда настройки AGC0
#define	DigitRecPCI_AGC0  0x00031031 // Команда настройки AGC0
#define	DigitRecPCI_AGC1  0x00031032 // Команда настройки AGC1
#define	DigitRecPCI_AGC2  0x00031033 // Команда настройки AGC2
#define	DigitRecPCI_Fd    0x00031052 // Команда частота дискретизации
#define	RecFrecForSklWC   0x00031034 // Команда настройки частоты приемника
#define	Cmd_Pozitiv_Stop  0x00031035 // Команда стоп для Pozitiv
#define	Cmd_Pozitiv_Start 0x00031036 // Команда старт для Pozitiv
#define	Finist_Fd         0x00031037 // Команда настройки частоты дискретизации для платы ввода Finist
#define	ChangeFRecFromRPU 0x00031038 // Команда передачи кода частоты при изменении частоты на РПУ
#define	ActiveWindowSKLWC 0x00031039 // Команда передачи номера активного канала
#define	NumChSignal0      0x00031040 // Команда передачи номера канала на который подается импульс нуля
#define	ConToSrvCmdPeleng 0x00031041 // Событие, присоединение к серверу передачи команд на пеленгование
#define	DConToSrvCmdPeleng 0x00031042 // Событие, отсоединение от сервера передачь команд на пеленгование
#define	LockOnLink        0x00031043 // Событие, блокировка канала связи, от сервера передачь команд на пеленгование
#define	LockOffLink       0x00031044 // Событие, снята блокировка канала связи, от сервера передачь команд на пеленгование
#define	EventPOstSMO1     0x00031045 // Событие, передача структуры POST_SMO1, на сервер передачь команд на пеленгование
#define	EventPOstSMO2     0x00031046 // Событие, передача структуры POST_SMO2, на сервер передачь команд на пеленгование
#define	EventHead         0x00031047 // Событие, передача структуры HEAD, на сервер передачь команд на пеленгование
#define	SetFrecFn         0x00031048 // Событие, передача Frec && Fn
#define	CMDFindBD         0x00031049 // Kомандa, поиска в БД
#define	CMDSKLSaveWave    0x00031050 
#define	SendParamFromPoisk 0x00031053
#define	SendFrecDownCtrl  0x00031054
#define	GetFrecDownCtrl   0x00031055
#define	PaintMarkerRaznos   0x00031056
#define	SendCommandFrecDownCtrl  0x00031057
#define	SendCommandLockDownCtrl  0x00031058
#define	SendCommandUnLockDownCtrl  0x00031059
#define	SendMonitorEvent	0x00031060



#define	CardDDC6654_SetDemodCh 0x00032000 // Команда SetDemodCh
#define	CardDDC6654_SetDeltaFc 0x00032001 // Команда SetDemodCh

#define	UWBD_Default				0x00033000 // 
#define	UWBD_GetGlobalDeviceStatus	0x00033001	
#define	UWBD_GlobalDeviceStatus		0x00033002	
#define	UWBD_GetDataCmd				0x00033004	// command to UWBD data from ADC (TCP)
#define	UWBD_StandartResponse		0x00033005	// response from UWBD (TCP)
#define	UWBD_ADCDataUDP				0x00033006	// ADC Data from UWBD (UDP)
#define	UWBD_FirmwareProg			0x00033007	// program firmware data to UWBD; struct - StrFirmwareProg; responce - UWBD_StandartResponse
#define	UWBD_IPAddrProg				0x00033008	// program IP addr to UWBD; struct - StrIPAddrProg; responce - ???
#define	UWBD_KeyProg				0x00033009	// program license key to UWBD; struct - StrKeyProg; responce - UWBD_StandartResponse
#define	UWBD_Reconfig				0x0003300a	// reconfiguration command to UWBD; struct - none; responce - UWBD_StandartResponse
#define	UWBD_SetConfigTunerMode		0x0003300b	// config  tuner mode (StrSetCfgTunnerCmd)
#define	UWBD_SetConfigDemodMode		0x0003300c	// config  tuner mode (StrSetCfgDemodCmd)
#define UWBD_DemodStat			 	0x0003300d // statistic  demodulator

struct StrKeyProg
{
	unsigned int	constantDeviceCode;			// 0x00033000

	unsigned int	ADC0_enable;				// ADC0_... all feateres:  bit 0:	0- disabled, 1- enabled 
	unsigned int	ADC0_send_samples;			// bit 0: 0- disabled, 1- enabled 
	unsigned int	ADC0_send_IQ;				// bit 0: 0- disabled, 1- enabled 
	unsigned int	ADC0_send_FFT;				// bit 0: 0- disabled, 1- enabled 
	unsigned int	ADC0_demod;					// demod all feateres:	bit 0: 0- disabled, 1- enabled 
	unsigned int	ADC0_demodChannelCntMax;	// 
	unsigned int	ADC0_demodBandMax;			// 0- not limited
	unsigned int	ADC0_demod_reserved[16];		// all bits 0	
	unsigned int	ADC0_decoder;				// decoder all feateres:	bit 0: 0- disabled, 1- enabled 
	unsigned int	ADC0_decoder_reserved[16];	// all bits 0		
	unsigned int 	ADC0_reservedField[24];		//
	
	unsigned int	ADC1_enable;				// ADC1_... all feateres:  bit 0:	0- disabled, 1- enabled 
	unsigned int	ADC1_send_samples;			// bit 0: 0- disabled, 1- enabled 
	unsigned int	ADC1_send_IQ;				// bit 0: 0- disabled, 1- enabled 
	unsigned int	ADC1_send_FFT;				// bit 0: 0- disabled, 1- enabled 
	unsigned int	ADC1_demod;					// demod all feateres:	bit 0: 0- disabled, 1- enabled 
	unsigned int	ADC1_demodChannelCntMax;	// 
	unsigned int	ADC1_demodBandMax;			// 0- not limited
	unsigned int	ADC1_demod_reserved[16];		// all bits 0	
	unsigned int	ADC1_decoder;				// decoder all feateres:	bit 0: 0- disabled, 1- enabled 
	unsigned int	ADC1_decoder_reserved[16];	// all bits 0		
	unsigned int 	ADC1_reservedField[24];		//
	
	unsigned int 	reserved[127];				//	
};

struct StrIPAddrProg
{
	unsigned int	cmd;		// UWBD_IPAddrProg
	unsigned int	newIP;
	unsigned char	destMAC[8];
};

struct StrFirmwareProg			//	firmware data to UWBD
{
	unsigned int dataType;			// 0 - FPGA config data, 1 - CPU program data
	unsigned int pageNum;  			// 0 - factory page, 1,2 ... n	- application page
	unsigned int startAddr;  		// destination start programming address
	unsigned int optBitStartAddr;  	// optional bits start programming address 0x20000
	unsigned int dataBlockNum;  	// programming data block number
	unsigned int dataBlockLen;  	// length of programming data block
	
	unsigned char data[1024];		//  programming data
};

struct HeaderFW_UWBD
{
	char VersCPU[8];
	char VersPLD[8];
	char Destination[128];
	char Remark[256];
	INT64 LenCPUFirmWare;					

	unsigned int pageNumCPU;  			// 0 - factory page, 1,2 ... n	- application page
	unsigned int startAddrCPU;  		// destination start programming address

	unsigned int pageNumFPGA;  			// 0 - factory page, 1,2 ... n	- application page
	unsigned int startAddrFPGA;  		// destination start programming address
	unsigned int optBitStartAddrFPGA;  	// optional bits start programming address 0x20000
};

struct StrStandartResponse
{
	unsigned int cmd;
	unsigned int response;  // 0 - OK
	unsigned int res[16];
};

#define UWBD_Data_Off 0
#define UWBD_Data_On  1
// TypeData
#define UWBD_Data_FFT 		0
#define UWBD_Data_ADC 		1
#define UWBD_Data_IQ   		2
#define UWBD_Data_TestCnt   3
#define UWBD_Data_DemBit    4
#define UWBD_Data_DemStat   5
#define UWBD_Data_DemStar   6
#define UWBD_Data_DebugInfo	7
#define UWBD_Data_Tuner		8

struct StrSetCfgDemodCmd
{
	unsigned int K_Decim;
	int deltaFc; // 0
	int SymbolRate;
	int TypeMod;
	short K_Table[5][128];
};

struct StrSetCfgTunnerCmd
{
	unsigned int K_Decim;
	short K_Table[256];
	unsigned int Len_K_Table;
	unsigned int Freq;
};

struct StrGetDataCmd
{
	unsigned int State;     // UWBD_Data_Off/UWBD_Data_On
	unsigned int TypeData;  // UWBD_Data_FFT/UWBD_Data_ADC/UWBD_Data_IQ/UWBD_Data_DemBit/UWBD_Data_DemStat/UWBD_Data_DemStar
	unsigned int sincIP;
	unsigned int sincPort;
	unsigned int SpeedData;
	unsigned int BufSize; // 0-0x03FFFFFF
	unsigned int Channel;
	unsigned int res[52];
};

struct StrGlobalDeviceStatus
{
	unsigned int	ADC_CLK_Freq;				// Частота тактового генератора в герцах
	unsigned char	MAC_ADDR_A[6];  			// MAC-Address
	unsigned char	MAC_ADDR_B[6];  			// MAC-Address

	unsigned short	workPort;  					// порт по которому работаем

	unsigned char	numClientsConnect;			// число присоединённых клиентов
	unsigned int	IP_Client_0;				// IP адрес 1-го клиента
	unsigned int	IP_Client_1;				// IP адрес 2-го клиента
	unsigned int	IP_Client_2;
	unsigned int	IP_Client_3;
	unsigned int	IP_Client_4;
	unsigned int	IP_Client_5;
	unsigned int	IP_Client_6;
	unsigned int	IP_Client_7;

	unsigned int	currPageAltera;
	unsigned char	versAltera[8];
	unsigned int	buildVersAltera;
	unsigned char	versCPU[8];
	unsigned int	currStateVersAltera;		// 0-factory 1 -app
	unsigned int	deviceErrState;				// 0
	unsigned int	globalState;				// 

	unsigned short	alteraCoreTemperature;
	unsigned short	boardTemperature;

	float	VCC0V9D_Voltage;
	float	VCC0V9D_Current;
	float	VCC1V0D0ETH_Voltage;
	float	VCC1V0D1ETH_Voltage;
	float	VCC1V1D_Voltage;
	float	VCC1V1D_Current;
	float	VCC1V5D_Voltage;
	float	VCC1V5D_Current;
	float	VCC1V8D_Current;
	float	VCC2V5D_Current;
	float	VCC2V5A_Current;
};

#pragma pack(push,1)
struct StrHeaderData
{
	short typecmd;
	short channel;
	int n_pack;     // # current block
	int cnt_pack;   // full cnt block
	short state_pack; // 1-begin 2-end
};

struct StrDemodStatistic
{
	unsigned int nChannel;
	unsigned int modulationType;
	unsigned int  tunerDiscrFreq;
	unsigned int  tunerDecimation;
	unsigned int  tunerNCO_Freq;
	unsigned int signalLock; // 0/1
	int    deltaFC_meas;
	unsigned int symolRate_meas;
	unsigned int signal_noiseValue;
	unsigned int resamplerDecimation;
	unsigned int signalLevel;
	int res[128];

};
#pragma pack(pop)

struct HEAD    // заголовок сообщения от "Парус-М"
{
	unsigned char Command;	        // идентификатор команды:
                                // 1 - "Сброс" КС (передается только структура HEAD)
                                // 2 - "Команда" (передается структура POST_SMO1)
                                // 3 - "Ориентирование" (передается структура POST_SMO2)
                                // 4 - работа по текущей команде завершена (передается только структура HEAD)
                                // 5 - "Станция работает" (передается структура POST_SMO3)
                                // 6 - "Позывной" (передается структура POST_SMO4)
	unsigned char NumPost;	        // номер поста 1-50 (обрабатывается при Command = 2, 3, 4)
	unsigned char NumCommand;       // номер команды 1-255 (обрабатывается при Command = 2, 3, 4)
	unsigned char Reserve;	        // резерв
};

struct MAIN_PART       // основная часть сообщения от "Парус-М"
{
	unsigned long Freq;	 	// частота ИРИ: 10000-319999 (с дискретностью 0.1 кГц)
	unsigned char SKP;	 	// код признака СКП = 1/0 (команда К1/К2)
	SYSTEMTIME Find;        	// дата и время обнаружения сигнала
	SYSTEMTIME Serv;	        // дата и время анализа сигнала
	SYSTEMTIME Res;	        // дата и время подачи команды (принятия решения)
	short int ParIRI[7];     	// ParIRI[0] - код вида модуляции Type_MD = 0…255
								// ParIRI[1] - код вида  передачи Type_TR =  0..255
                                // ParIRI[2] - функционал качества Quality: 50…100%
                                // ParIRI[3] - скорость манипуляции Vtlg = 1 …10 000 Бод
                                // ParIRI[4] - количество позиций ТЛГ-сигнала Ntlg = 2-32
                                // ParIRI[5] - сдвига частот FSK-сигнала ShiftFreq = 0-8000 Гц
                                // ParIRI[6] - ширина спектра сигнала BandWidth = 0-8000 Гц
};

struct POST_SMO1
{
	HEAD Head;
	MAIN_PART Main_Part;
};

struct DS_PART
{
	unsigned long Freq_Txt;    // 4 байта, текст (0..399999)
	unsigned char DS1A;		   // 1 байт, код дополнительного сообщения 1А, 0-9
	unsigned char DS1B;		   // 1 байт, код дополнительного сообщения 1Б, 0-9
	unsigned char DS2;		   // 1 байт, код дополнительного сообщения 2, 0-9			
	unsigned char DS3;		   // 1 байт, код дополнительного сообщения 3, 0-9
	unsigned char DS4;		   // 1 байт, код дополнительного сообщения 4, 0-9
};

struct POST_SMO2
{
	HEAD Head;
	DS_PART DS_Part;
};

struct HEAD1    
{
	unsigned char Command;	        // идентификатор команды:
                                // 1 - блокировка канала связи включена (передается структура HEAD1)
                                // 2 - блокировка канала связи отключена (передается структура HEAD1)
                                // 3 - канал связи занят (передается структура HEAD1)
	unsigned char NumPost;	        // номер поста 1-50
	unsigned char NumCommand;       // номер команды 1-255
	unsigned char Reserve;	        // резерв
};

struct SMO_POST1
{
	HEAD Head1;
	SYSTEMTIME System_Time;
};
struct SaveWave
{
	int ch;
	int cmd; // 1-start 0-stop
	char path[256];
};


struct NTaps_str
{
	int ntap;
	double Fd;
};
struct AGC_str
{
	int agc;
	int AGC;
};
static NTaps_str NTaps_All[]=
{
	{8192,9765.625},
	{6144,13020.833},
	{4096,19531.250},
	{3072,26041.667},
	{2048,39062.500},
	{1536,52083.333},
	{1024,78125.000},
	{768,104166.667},
	{512,156250.000},
	{384,208333.333},
	{256,312500.000},
	{192,416666.667},
	{128,625000.000},
	{96,833333.333},
	{48,1666666.666},
	{-1}
};
static AGC_str AGC_All[]=
{
	{0,-10},
	{3,-7},
	{6,-4},
	{9,-1},
	{12,2},
	{15,5},
	{18,8},
	{21,11},
	{24,14},
	{27,17},
	{30,20},
	{33,23},
	{36,27},
	{39,30},
	{42,33},
	{45,35},
	{-1}
};

//	STS_End
//};
//#define	TCP_Defaul	0x00040000	// зарезервированно для системы обработки ПД.
//#define	DTB_Defaul	0x00050000	// зарезервированно для СУБД.
// ---------------------------------------------
// Команды, запросы и ответы к файловому серверу
// ---------------------------------------------
//#define	FSV_GetParam		0x00060001	// Запрос параметров сервера
//
//#define	FSV_Param			0x00060002	// Передать параметры сервера
// FSrvParam
//#define	FSV_GetListFreeID	0x00060003	// Запрос списка своботных для использования идентификаторов
// NumID - количество запрашиваемых ID
// При отключении клиента все выделенные ему идентификаторы освобождаются
// IDFS не освобождаются при закрытии файла и может быть повторно использован клиентом
//#define	FSV_ListID			0x00060004	// Cписк своботных для использования идентификаторов
//	ID1,ID2,...,IDNumID	// каждый ID 4 байта (int)
//#define	FSV_FreeListID		0x00060005	// Освободить идентификаторы файлов
//	ID1,ID2,...,IDNumID	// каждый ID 4 байта (int)
//#define	FSV_FileMode		0x00060006	// Режим создания файла общий для одного клиента (по умолчанию 0)
//	(int) Mode	// 0 - бинарный (обычный) 1 - WAV-файл 
//#define	FSV_WAVParam		0x00060007	// Параметры WAV - файла
//	WAVParam ???
#define	FSV_IDInfo			0x00060008	// Передать структуру IDInfo	!!!  Основной режим передачи данных  !!!
//	IDInfo	(DBuf.Cnt - реальные, а pList & Dbuf.Buf - недействителны)
#define	FSV_DataIDI			0x00060009	// Передать блок с данными вслед за IDInfo
//	Buf[]
//#define	FSV_FileOpen		0x0006000a	// Открыть файл в одноканальном некоммутируемом режиме
//#define	FSV_Write			0x0006000b	// Передать блок с данными для записи
//#define	FSV_FileClose		0x0006000c	// Закрыть файл в одноканальном некоммутируемом режиме
//#define	FSV_FileOpenID		0x0006000d	// Открыть файл в многоканальном режиме с использованием IDFS (идентификатора файла
//	IDSF								// назначенного файл-сервером)
//#define	FSV_WriteID			0x0006000e	// Передать блок с данными для записи
//	IDFS,char* Buf
//#define	FSV_FileCloseID		0x0006000f	// Закрыть файл в одноканальном некоммутируемом режиме
//	IDFS
//#define	FSV_GetSharedFile	0x00060010	// Преместить файл в шаренную область диковой памяти
//	IDFS
//#define	FSV_SharedFile		0x00060011	// Файл в шаренной области готов
//	PathFile
//#define	FSV_FreeSharedFile	0x00060012	// Осбодить шаренный файл
//	IDFS
//#define	FSV_OpenForRead		0x00060013	// Открыть файл для чтения	(файл закрывается по завершению передачи)
//	IDFS
//#define	FSV_ReadIDInfo		0x00060014	// Передать структуру IDInfo (при чтении)
//	IDInfo
//#define	FSV_ReadData		0x00060015	// Данные
//	char* Buf
//#define	FSV_DeleteFile		0x00060016	// Убить файл (если нельзя сразу - поставить в очередь)
//	IDFS
//#define	FSV_EmptyDiskSpace	0x00060017	// Нехватает дисковой памяти
//#define	FSV_Error			0x00060018	// Сообщения об ошибках
#define	FSV_GetNetPath			0x00060019	// Запрос сетевого пути
//
#define	FSV_NetPath				0x0006001a	// Передать сетевой путь
//	Buf
#define	FSV_Defaul				0x00060000	//


#define	UPS_Defaul				0x00070000	//
//
#define	UPS_ResetComp			0x00070001	//Перезагрузить компьютер
// int NumComp
#define	UPS_PowerOn				0x00070002	//Включить компьютер
// int NumComp
#define	UPS_PowerOff			0x00070003	//Выключить компьютер
// int NumComp
#define	UPS_CurrentState		0x00070004	//Получить текущее состояние
//
#define	UPS_PowerOnAll			0x00070005	//Включить все компьютеры
//
#define	UPS_PowerOffAll			0x00070006	//Выключить все компьютеры
//
#define	UPS_CurrentData			0x00070007	//Получить текущие данные
//
#define UPS_UsbOn               0x00070008	//Включилось Usb
//
#define UPS_UsbOff              0x00070009	//Выключилось Usb
// 
#define UPS_Open	            0x00070010	//Вскрыли
// int NumSensor
#define UPS_Close               0x00070011	//Закрыли
// int NumSensor

// -------------------------------------------------------------------
#define	FE_Defaul				0x00100000	// Пакеты FrontEnd Systems
#define	IRI_On					0x00100100
#define	IRI_Off					0x00100101
#define	IRI_StreamOnOff			0x00100200
#define	IRI_SeansBegin			0x00100202
#define	IRI_ContentType			0x00100203
#define	IRI_SeansEnd			0x00100204
#define	IRI_ChOnOff				0x00100205
#define	IRI_ChRequest			0x00100206
#define	IRI_ChReqAck			0x00100207
#define	IRI_SendPhoneNumb		0x00100208
#define	IRI_PhoneNumbAck		0x00100209
#define	IRI_Connect				0x00100210
#define	IRI_ConnectAck			0x00100211
#define	IRI_ChRelease			0x00100212
#define	IRI_ChRelAck			0x00100213
#define	IRI_DTMFdigit			0x00100214
#define	IRI_Criterion			0x00100215
#define	IRI_LogChParam			0x00100216
#define	IRI_SeansParam			0x00100217
#define	TOI_EraseList			0x00100300
#define	TOI_AddItem				0x00100301
#define	TOI_DelItem				0x00100302
#define	TOI_ModifyItem			0x00100303
#define	TOI_Error				0x00100304
#define	RT_AudioCtrlData		0x00100400
#define	RT_AudioCtrlReq			0x00100401
#define	RT_AudioCtrlRel			0x00100402
#define	RT_Error				0x00100403
#define	TLM_Param				0x00100500
#define	TLM_Alarm				0x00100501
#define	TLM_Error				0x00100502


// 0x0007XXXX-0x000fXXXX - общий резерв

#define	TotRez1_Defaul	0x00070000
enum TotRez1_Enum
{
	TotRez1_xxxx=TotRez1_Defaul,

		TotRez1_Complex,//Сложный пакет, пока LenX!=0;
		//<TotRez1_Complex=4>[<Len0>[<Data0>]...[<LenM>[<DataM>]]]
		TotRez1_NetIDI,//Пакет с cамим IDInfo размером sizeof(IDInfo)
		TotRez1_NetIDIBuf,//Пакет с буфером IDInfo->Buf размера IDInfo->Сnt
		TotRez1_NetIDIParam,//Пакет с дополнительными параметрами IDInfo по парам <TotRez1_NetIDINameParam>,[<TotRez1_NetIDIVolParam>] разбор типа TotRez1_Complex
		TotRez1_NetIDINameParam,//Пакет с именем параметра для TotRez1_NetIDIParam
		TotRez1_NetIDIVolParam,//Пакет с значением параметра для TotRez1_NetIDIParam


		TotRez1_NetComplexEvent,//Пакет с cамим IDInfo размером sizeof(IDInfo) для события

		TotRez1_NetIDIOldBuf,//Пакет с буфером IDInfo->OldBuf размера IDInfo->OldСnt
		TotRez1_NetCli,		//<TotRez1_NetCli=4><NCli=4>
		TotRez1_NetCPUUsedRx,	//NetCommRx шлет клиентам свой CPUUsed =<double>
		TotRez1_ExitApp, //
		TotRez1_Watch,// Query
		TotRez1_Result,// Result
		TotRez1_Start,
		TotRez1_Stop,
		TotRez1_Param,//struct	PrmDec
		TotRez1_AddressInfo,
		TotRez1_NetIDIStrParam,//Пакет с значением параметра для TotRez1_NetIDIParam
		TotRez1_NetIDITimeStrParam,//Пакет с значением параметра для TotRez1_NetIDIParam
		
};

#define	TotRez_DataFromModule	0x00071000
//[<Data>]
#define	TotRez_DataToModule		0x00071001
//<IDF_Module=8,Data>

// Str2Ch32 Events
#define	CH32_Error				0x2a000001
// Str
struct	Str2Ch32_Error
{
	int			ID_Hard;
	char		ID_Srv[80];
	UINT		Error;
};

#define	Str2Ch32_DataError			1	//Плохие данные (невозможно найти синхру)
#define	Str2Ch32_MissData			2	//На входе нет данных длительное время



// 0x2a00XXXX-0x2fffXXXX - Events

#define	Event_CliIDI	0x2a000004	//Cобытия CliIDI
//<TypePack-INT>[<Data>]     -TypePack=Param_EventSendIDI|Param_EventDiscSrv

//
// 0x3000XXXX-0xffffXXXX - общий резерв






// Глобальные параметры для службы сетевого взаимодействия
// ---------------------------

#define	Param_EventSendIDI	0x2a000005	//TypePack cобытия CliIDI - передача в <Data> указателя на IDInfo
#define	Param_EventDiscSrv	0x2a000006	//TypePack cобытия CliIDI - отсоединение SrvIDI
#define	Param_Telegramma	0x2a000007	//TypePack -телеграмма

// Параметры системы телефонии
#define	STS_SeansCalledN	0x00030011//Вызыв-мый номер 
//(int)Len,(char)Digit[Len]
#define	STS_SeansCallingN	0x00030012//Вызыв-щий номер 
//(int)Len,(char)Digit[Len] 
#define	STS_SeansConnectN	0x00030013//Присоед-ный номер 
//(int)Len,(char)Digit[Len]
#define	STS_SeansRedirctN	0x00030014//номер переадресации(с обр.ст)
//(int)Len,(char)Digit[Len]
#define	STS_SeansOriginN	0x00030015//original called number (первоначальный вызываемый номер):
//(int)Len,(char)Digit[Len]
#define	STS_SeansLocationN	0x00030016//location number (номер местоположения)

#define	STS_SeansRedirctingN 0x00030017//Перенаправл-ный номер (с пр.ст)

#define	STS_SeansDampNumber 0x00030018//Перенаправл-ный номер (с пр.ст)

#define	STS_SeansIsISDN		0x00030020//Сеанс ISDN  0x00030015
#define	STS_OpenChOKC		0x00030021//Открыть канал с сигнализацией 0x00030016
//(int)NStream-номер лог. потока,(int)NCh-номер канала,(int)TypSTS
#define	STS_Typ1VF			0x00030022//Тип протокола - 1VF 0x00030017
#define	STS_TypEnergy		0x00030023//Тип протокола по умолчанию 0x00030018
#define	STS_TypEDSS			0x00030024//Тип протокола - EDSS 0x00030019
#define	STS_TypSS5			0x00030025//Тип протокола - SS N5 0x0003001a
#define	STS_TypSS7			0x00030026//Тип протокола - SS N7 0x0003001b
#define	STS_TypUKN			0x00030027//Тип протокола - unknown 0x0003001c
#define	STS_TypH323			0x00030028//Тип протокола - H.323
#define	STS_TypR2D			0x00030029//Тип протокола - R2D 0x00030017

#define	STS_TypSCCP_CDMA	0x00030037//Тип протокола - CDMA in SCCP
#define	STS_TypV52			0x00030038//Тип протокола - V52
#define	STS_TypAbis			0x00030039//Тип протокола - Abis

#define	STS_TypDataVoice	0x00030030
#define	STS_TypDataISDN		0x00030031
#define	STS_TypDataModem	0x00030032
#define	STS_TypDataFax		0x00030033
#define	STS_TypData			0x00030034

#define	STS_TypThurBCCH		0x00030040//Тип протокола - ThurBCCH
#define	STS_TypThurPCH		0x00030041//Тип протокола - ThurPCH	
#define	STS_TypThurGBCH		0x00030042//Тип протокола - ThurGBCH	

#define	STS_CommSetARCFN	0x00030043//команда установки ARCFN
// 
// // Inmarsat InmDec
// #define ParamNCSC_Les			"LesID"
// #define ParamNCSC_Mes			"MesID"
// #define ParamNCSC_Adress		"Adress"
// #define ParamNCSC_AdressType	"AdressType"
// #define ParamNCSC_Present		"Present"
// #define ParamNCSC_SubAdress		"SubAdress"

Parametr*	GetTablParametrID(UINT64 ID);
char*		GetTablName(int Number);
int			GetIDTablName(char* TablName);

/*Standard RIFF tag to identify RIFF file format in the WAVE header.*/
#define PJMEDIA_RIFF_TAG	('F'<<24|'F'<<16|'I'<<8|'R')
/*Standard WAVE tag to identify WAVE header.*/
#define PJMEDIA_WAVE_TAG	('E'<<24|'V'<<16|'A'<<8|'W')
/*Standard FMT tag to identify format chunks.*/
#define PJMEDIA_FMT_TAG		(' '<<24|'t'<<16|'m'<<8|'f')
/*fact tag */
#define PJMEDIA_FACT_TAG	('t'<<24|'c'<<16|'a'<<8|'f')
/*Standard DATA tag to identify data chunks.*/
#define PJMEDIA_DATA_TAG	('a'<<24|'t'<<16|'a'<<8|'d')
#pragma pack(push,1)

struct Riff_hdr /** This structure describes RIFF WAVE file header */
{
	unsigned int  riff;		/**< "RIFF" ASCII tag.		*/
	unsigned int  file_len;		/**< File length minus 8 bytes	*/
	//unsigned int  wave;		/**< "WAVE" ASCII tag.		*/
} /*riff_hdr*/;

/** This structure describes format chunks/header  */
struct Fmt_hdr
{
	unsigned int  wave;		/**< "WAVE" ASCII tag.		*/
	unsigned int  fmt;			/**< "fmt " ASCII tag.		*/
	unsigned int  len;			/**fmt length	*/
	unsigned short  fmt_tag;	/* Format code number		*/
	unsigned short  nchan;		/**< Number of channels (mono)*/
	unsigned int sample_rate;	/**< Sampling rate.	Uncompressed samples/second	*/
	unsigned int bytes_per_sec;	/**<  Compressed bytes/second	*/
	unsigned short  X_1;			/* Unknown format-specific 	*/
	unsigned int  X_2;			/* Unknown format-specific 	*/
} ;

/** The data header preceeds the actual data in the file. */
struct Data_hdr
{
	unsigned int data;		/**< "data" ASCII tag.		*/
	unsigned int len;		/**< Data length.		*/
} /*data_hdr*/;

struct TAG_HDR_STR
{
	Fmt_hdr		fmt_hdr;
	Data_hdr	data_hdr;
};

struct WAVE_HDR_STR
{
	
	Riff_hdr	riff_hdr;
	Fmt_hdr		fmt_hdr;
	Data_hdr	data_hdr;
};
#pragma pack(pop)

#define WAVE_FORMAT_ADPCM_G726   0x7701   // ADPCM G.726
#define WAVE_FORMAT_ADPCM_G727   0x7702   // ADPCM G.727
#define WAVE_FORMAT_LDCELP       0x7703   // LDCELP G.728
#define WAVE_FORMAT_ADPCM_OKI    0x7704   // ADPCM OKI
#define WAVE_FORMAT_OADPCM       0x7705   // OADPCM
#define WAVE_FORMAT_CSACELP      0x7706   // CS-ACELP
#define WAVE_FORMAT_G728		0x7703   // LDCELP G.728
#define WAVE_FORMAT_G729		0x7706   // CS-ACELP


#endif

