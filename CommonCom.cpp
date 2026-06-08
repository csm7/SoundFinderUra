#include "pch.h"
//#include "stdafx.h"

#include "CommonCom.h"

static Parametr TablParametrs[]=
{	// !!!
	{IDNameElTab,		"NameElTab",		PrmType_STRING,	PrmIndex_Yes,	PrmRegularity_No,	0,	PrmStatus_All,	128,	0,"Element of tables names"},
	{IDPrmType,			"PrmType",			PrmType_CHAR,	PrmIndex_No,	PrmRegularity_Yes,	0,	PrmStatus_All,	0,	0,"Type of parameters"},
	{IDPrmIndex,		"PrmIndex",			PrmType_CHAR,	PrmIndex_No,	PrmRegularity_Yes,	0,	PrmStatus_All,	0,	0,"Index of parameters"},
	{IDPrmRegularity,	"PrmRegularity",	PrmType_CHAR,	PrmIndex_No,	PrmRegularity_Yes,	0,	PrmStatus_All,	0,	0,"Regularity of parameters"},
	{IDPrmStatus,		"PrmStatus",		PrmType_INT,	PrmIndex_No,	PrmRegularity_No,	0,	PrmStatus_All,	0,	0,"Status of parameters"},
	{IDPrmSize,			"PrmSize",			PrmType_INT,	PrmIndex_No,	PrmRegularity_No,	0,	PrmStatus_All,	0,	0,"Size of parameters"},
	{IDPrmView,			"PrmView",			PrmType_INT,	PrmIndex_No,	PrmRegularity_No,	0,	PrmStatus_All,	0,	0,"Type view of parameters"},
// --------------------------------------------------------------------------------------------------------------
	{IDProtocol,		"Protocol",			PrmType_STRING,	PrmIndex_Yes,	PrmRegularity_Yes,	0,	PrmStatus_All,	0,	0,"Protocol"},
	{IDFormat,			"Format",			PrmType_STRING,	PrmIndex_Yes,	PrmRegularity_Yes,	0,	PrmStatus_All,	0,	0,"Format"},
	{IDIPFrom,			"IPFrom",			PrmType_INT,	PrmIndex_Yes,	PrmRegularity_No,	0,	PrmStatus_All,	0,	ParametrView_IPAddr,	"IPForm address"},
	{IDIPTo,			"IPTo",				PrmType_INT,	PrmIndex_Yes,	PrmRegularity_No,	0,	PrmStatus_All,	0,	ParametrView_IPAddr,	"IPTo address"},
	{IDPortFrom,		"PortFrom",			PrmType_INT,	PrmIndex_Yes,	PrmRegularity_No,	0,	PrmStatus_All,	0,	ParametrView_UDec,		"PortForm address"},
	{IDPortTo,			"PortTo",			PrmType_INT,	PrmIndex_Yes,	PrmRegularity_No,	0,	PrmStatus_All,	0,	ParametrView_UDec,		"PortTo address"},
	{IDFileType,		"FileType",			PrmType_STRING,	PrmIndex_Yes,	PrmRegularity_Yes,	0,	PrmStatus_All,	0,	0,"File type"},
	{IDFileLen,			"FileLen",			PrmType_INT64,	PrmIndex_Yes,	PrmRegularity_No,	0,	PrmStatus_All,	0,	0,"File length"},
	{IDFileExt,			"FileExt",			PrmType_STRING,	PrmIndex_Yes,	PrmRegularity_Yes,	0,	PrmStatus_All,	0,	0,"File extension"},
	{IDFileName,		"FileName",			PrmType_STRING,	PrmIndex_Yes,	PrmRegularity_No,	0,	PrmStatus_All,	0,	0,"File name"},
	{IDContentType,		"ContentType",		PrmType_STRING,	PrmIndex_Yes,	PrmRegularity_Yes,	0,	PrmStatus_All,	0,	0,"Content Type"},
	{IDPayloadType,		"PayloadType",		PrmType_STRING,	PrmIndex_Yes,	PrmRegularity_Yes,	0,	PrmStatus_All,	0,	0,"Payload Type"},
	{IDFSrvName,		"FSrvName",			PrmType_STRING,	PrmIndex_No,	PrmRegularity_Yes,	0,	PrmStatus_All,	0,	0,"File Server Name"},
	{IDFlagCRC,			"FlagCRC",			PrmType_INT,	PrmIndex_Yes,	PrmRegularity_No,	0,	PrmStatus_All,	0,	0,	"FlagCRC"},
	{IDID,				"ID",				PrmType_INT64,	PrmIndex_Yes,	PrmRegularity_No,	0,	PrmStatus_All,	0,	ParametrView_Hex,		"Identificator (ID)"},
	{IDIDFileSnsFr,		"IDFileSnsFr",		PrmType_INT64,	PrmIndex_Yes,	PrmRegularity_No,	0,	PrmStatus_All,	0,	ParametrView_Hex,		"ID files SnsFr"},
	{IDIDFileSnsTo,		"IDFileSnsTo",		PrmType_INT64,	PrmIndex_Yes,	PrmRegularity_No,	0,	PrmStatus_All,	0,	ParametrView_Hex,		"ID files SnsTo"},
	{IDIDFileParent,	"IDFileParent",		PrmType_INT64,	PrmIndex_Yes,	PrmRegularity_No,	0,	PrmStatus_All,	0,	ParametrView_Hex,		"ID files Parent"},
	{IDMtName,			"MtName",			PrmType_STRING,	PrmIndex_No,	PrmRegularity_Yes,	0,	PrmStatus_All,	0,	0,""},
	{IDMtAuth,			"MtAuthor",			PrmType_STRING,	PrmIndex_No,	PrmRegularity_Yes,	0,	PrmStatus_All,	0,	0,""},
	{IDMtVer,			"MtVersion",		PrmType_STRING,	PrmIndex_No,	PrmRegularity_Yes,	0,	PrmStatus_All,	0,	0,""},
	{IDIDSession,		"IDSession",		PrmType_INT64,	PrmIndex_Yes,	PrmRegularity_No,	0,	PrmStatus_All,	0,	0,"ID Session"},
	{IDIDStream,		"IDStream",			PrmType_INT64,	PrmIndex_No,	PrmRegularity_No,	0,	PrmStatus_All,	0,	0,"ID Stream"},
	{IDNumberFrChan,	"NumberFrChan",		PrmType_INT64,	PrmIndex_No,	PrmRegularity_No,	0,	PrmStatus_All,	0,	0,"Use WSA for device"},
	{IDMode,			"Mode",				PrmType_STRING,	PrmIndex_No,	PrmRegularity_Yes,	0,	PrmStatus_All,	0,	0,"Mode"},
	{IDFd,				"Fd",				PrmType_DOUBLE,	PrmIndex_No,	PrmRegularity_No,	0,	PrmStatus_All,	0,	0,"Fd"},
	{IDNPhChan,			"NPhChan",			PrmType_INT,	PrmIndex_No,	PrmRegularity_No,	0,	PrmStatus_All,	0,	0,"Number physical channel"},
	{IDFrequencyRange,	"FrequencyRange",	PrmType_STRING,	PrmIndex_No,	PrmRegularity_No,	0,	PrmStatus_All,	0,	0,"IDIThurDem"},
	{IDIDGroop,			"IDGroop",			PrmType_INT64,	PrmIndex_No,	PrmRegularity_No,	0,	PrmStatus_All,	0,	0,"IDIThurDem"},
	{IDS_N,				"S/N",				PrmType_DOUBLE,	PrmIndex_No,	PrmRegularity_No,	0,	PrmStatus_All,	0,	0,"Signal/Noise"},
	{IDTDMA_FN,			"TDMA_FN",			PrmType_INT64,	PrmIndex_No,	PrmRegularity_No,	0,	PrmStatus_All,	0,	0,""},
	{IDTDMA_SFN,		"TDMA_SFN",			PrmType_INT64,	PrmIndex_No,	PrmRegularity_No,	0,	PrmStatus_All,	0,	0,""},
	{IDTDMA_MFN,		"TDMA_MFN",			PrmType_INT64,	PrmIndex_No,	PrmRegularity_No,	0,	PrmStatus_All,	0,	0,""},
	{IDIDLogChannel,	"IDLogChannel",		PrmType_INT64,	PrmIndex_No,	PrmRegularity_No,	0,	PrmStatus_All,	0,	0,"ID LogChannel"},
	{IDIDDemChannel,	"IDDemChannel",		PrmType_INT64,	PrmIndex_No,	PrmRegularity_No,	0,	PrmStatus_All,	0,	0,""},
	{IDFrequency,		"Frequency",		PrmType_INT64,	PrmIndex_Yes,	PrmRegularity_No,	0,	PrmStatus_All,	0,	0,"Frequency"},
	{_IDUnitID,			"UnitID",			PrmType_INT64,	PrmIndex_No,	PrmRegularity_No,	0,	PrmStatus_All,	0,	0,"UnitID"},
	{IDADC,				"ADC",				PrmType_DOUBLE,	PrmIndex_No,	PrmRegularity_No,	0,	PrmStatus_All,	0,	0,""},
	{IDProcTCPIP,		"ProcTCPIP",		PrmType_STRING,	PrmIndex_No,	PrmRegularity_No,	0,	PrmStatus_All,	0,	0,""},
	{IDProcUnknUDP,		"ProcUnknUDP",		PrmType_STRING,	PrmIndex_No,	PrmRegularity_No,	0,	PrmStatus_All,	0,	0,""},
	{IDIDParStream,		"IDParStream",		PrmType_INT64,	PrmIndex_No,	PrmRegularity_No,	0,	PrmStatus_All,	0,	0,"IDParStream"},
	{IDIODLCAddr,		"IODLCAddr",		PrmType_INT64,	PrmIndex_Yes,	PrmRegularity_No,	0,	PrmStatus_All,	0,	0,""},
	{IDIODLCSession,	"IODLCSession",		PrmType_INT64,	PrmIndex_Yes,	PrmRegularity_No,	0,	PrmStatus_All,	0,	0,""},
	{IDCallNumberFrom,	"CallNumberFrom",	PrmType_STRING,	PrmIndex_Yes,	PrmRegularity_No,	0,	PrmStatus_All,	0,	0,""},
	{IDCallNumberTo,	"CallNumberTo",		PrmType_STRING,	PrmIndex_Yes,	PrmRegularity_No,	0,	PrmStatus_All,	0,	0,""},
	{IDSIPProxyAuthFrom,"SIPProxyAuthFrom",	PrmType_STRING,	PrmIndex_No,	PrmRegularity_No,	0,	PrmStatus_All,	0,	0,"SIPProxyAuthFrom"},
	{IDSIPProxyAuthTo,	"SIPProxyAuthTo",	PrmType_STRING,	PrmIndex_No,	PrmRegularity_No,	0,	PrmStatus_All,	0,	0,"SIPProxyAuthTo"},
	{IDSIPFrom,			"SIPFrom",			PrmType_STRING,	PrmIndex_No,	PrmRegularity_No,	0,	PrmStatus_All,	0,	0,"SIPFrom"},
	{IDSIPTo,			"SIPTo",			PrmType_STRING,	PrmIndex_No,	PrmRegularity_No,	0,	PrmStatus_All,	0,	0,"SIPTo"},
	{IDHDLC_All,		"HDLC_All",			PrmType_INT64,	PrmIndex_No,	PrmRegularity_No,	0,	PrmStatus_All,	0,	0,""},
	{IDHDLC_ErrSize,	"HDLC_ErrSize",		PrmType_INT64,	PrmIndex_No,	PrmRegularity_No,	0,	PrmStatus_All,	0,	0,""},
	{IDHDLC_OthHDLC,	"HDLC_OthHDLC",		PrmType_INT64,	PrmIndex_No,	PrmRegularity_No,	0,	PrmStatus_All,	0,	0,""},
	{IDHDLC_ErrIP,		"HDLC_ErrIP",		PrmType_INT64,	PrmIndex_No,	PrmRegularity_No,	0,	PrmStatus_All,	0,	0,""},
	{IDCRC_OK,			"CRC_OK",			PrmType_INT64,	PrmIndex_No,	PrmRegularity_No,	0,	PrmStatus_All,	0,	0,"CRC_OK"},
	{IDCRC_ERR,			"CRC_ERR",			PrmType_INT64,	PrmIndex_No,	PrmRegularity_No,	0,	PrmStatus_All,	0,	0,"CRC_ERR"},
	{IDHDLC_TCP,		"HDLC_TCP",			PrmType_INT64,	PrmIndex_No,	PrmRegularity_No,	0,	PrmStatus_All,	0,	0,""},
	{IDHDLC_UDP,		"HDLC_UDP",			PrmType_INT64,	PrmIndex_No,	PrmRegularity_No,	0,	PrmStatus_All,	0,	0,""},
	{IDQuality,			"Quality",			PrmType_FLOAT,	PrmIndex_No,	PrmRegularity_No,	0,	PrmStatus_All,	0,	0,"Quality"},
	{IDFileInfo,		"FileInfo",			PrmType_USER,	PrmIndex_No,	PrmRegularity_No,	0,	PrmStatus_All,	0,	0,""},
	{IDDVB_PID,			"DVB_PID",			PrmType_INT64,	PrmIndex_Yes,	PrmRegularity_No,	0,	PrmStatus_All,	0,	0,"DVB_PID"},
	{IDService,			"Service",			PrmType_STRING,	PrmIndex_No,	PrmRegularity_No,	0,	PrmStatus_All,	0,	0,"Service (http,smtp,pop3,...)"},
	{IDUIDICLList,		"UIDICLList",		PrmType_INT64,	PrmIndex_No,	PrmRegularity_No,	0,	PrmStatus_All,	0,	0,""},
	{IDProfile,			"Profile",			PrmType_STRING,	PrmIndex_Yes,	PrmRegularity_Yes,	0,	PrmStatus_All,	0,	0,""},
	{IDIDProfile,		"IDProfile",		PrmType_INT64,	PrmIndex_No,	PrmRegularity_No,	0,	PrmStatus_All,	0,	0,""},
	{IDTypeMsg,			"TypeMsg",			PrmType_STRING,	PrmIndex_No,	PrmRegularity_Yes,	0,	PrmStatus_All,	0,	0,""},
	{IDOriginalNR,		"OriginalNR",		PrmType_STRING,	PrmIndex_No,	PrmRegularity_No,	0,	PrmStatus_All,	0,	0,"Original NR"},
	{IDCallingNR,		"CallingNR",		PrmType_STRING,	PrmIndex_Yes,	PrmRegularity_No,	0,	PrmStatus_All,	0,	0,"Calling NR"},
	{IDConnectNR,		"ConnectNR",		PrmType_STRING,	PrmIndex_No,	PrmRegularity_No,	0,	PrmStatus_All,	0,	0,"Connected NR"},
	{IDRedirctingNR,	"RedirectingNR",	PrmType_STRING,	PrmIndex_No,	PrmRegularity_No,	0,	PrmStatus_All,	0,	0,"Redirecting NR"},
	{IDLocationNR,		"LocationNR",		PrmType_STRING,	PrmIndex_No,	PrmRegularity_No,	0,	PrmStatus_All,	0,	0,"LocationNR"},
	{IDRedirctionNR,	"RedirectionNR",	PrmType_STRING,	PrmIndex_No,	PrmRegularity_No,	0,	PrmStatus_All,	0,	0,"Redirection NR"},
	{IDCalledNR,		"CalledNR", 		PrmType_STRING,	PrmIndex_Yes,	PrmRegularity_No,	0,	PrmStatus_All,	0,	0,"Called NR"},
	{IDTimeSlot,		"TimeSlot",			PrmType_INT,	PrmIndex_No,	PrmRegularity_No,	0,	PrmStatus_All,	0,	0,""},
	{IDSlotSelect,		"SlotSelect",		PrmType_INT,	PrmIndex_No,	PrmRegularity_No,	0,	PrmStatus_All,	0,	0,""},
	{IDCarrierBand,		"CarrierBand",		PrmType_INT64,	PrmIndex_No,	PrmRegularity_No,	0,	PrmStatus_All,	0,	0,""},
	{IDHeterodyneLBand,	"HeterodyneLBand",	PrmType_INT64,	PrmIndex_No,	PrmRegularity_No,	0,	PrmStatus_All,	0,	0,""},
	{IDCarrierLBand,	"CarrierLBand",		PrmType_INT64,	PrmIndex_No,	PrmRegularity_No,	0,	PrmStatus_All,	0,	0,""},
	{IDCarrierDR,		"CarrierDR",		PrmType_INT64,	PrmIndex_No,	PrmRegularity_No,	0,	PrmStatus_All,	0,	0,""},
	{IDCarrierSpan,		"CarrierSpan",		PrmType_INT64,	PrmIndex_No,	PrmRegularity_No,	0,	PrmStatus_All,	0,	0,""},
	{IDFrequencyLower,	"LowerFrequency",	PrmType_INT64,	PrmIndex_No,	PrmRegularity_No,	0,	PrmStatus_All,	0,	0,"LowerFrequency"},
	{IDFrequencyUpper,	"UpperFrequency",	PrmType_INT64,	PrmIndex_No,	PrmRegularity_No,	0,	PrmStatus_All,	0,	0,"UpperFrequency"},
	{IDDTXChan,			"DTXChan",			PrmType_INT,	PrmIndex_No,	PrmRegularity_No,	0,	PrmStatus_All,	0,	0,""},
	{IDRemark,			"Remark",			PrmType_STRING,	PrmIndex_No,	PrmRegularity_No,	0,	PrmStatus_All,	0,	0,"Remark"},
	{IDDPC,				"DPC",				PrmType_INT,	PrmIndex_No,	PrmRegularity_No,	0,	PrmStatus_All,	0,	0,"Destination PC"},
	{IDOPC,				"OPC",				PrmType_INT,	PrmIndex_No,	PrmRegularity_No,	0,	PrmStatus_All,	0,	0,"OPC"},
	{IDCIC,				"CIC",				PrmType_INT,	PrmIndex_No,	PrmRegularity_No,	0,	PrmStatus_All,	0,	0,"Channel Indetification Code"},
	{IDPriority,		"Priority",			PrmType_INT,	PrmIndex_No,	PrmRegularity_No,	0,	PrmStatus_All,	0,	0,"Priority"},
	{IDStreamList,		"StreamList",		PrmType_INT,	PrmIndex_No,	PrmRegularity_No,	0,	PrmStatus_All,	0,	0,""},
	{IDGroupList,		"GroupList",		PrmType_INT,	PrmIndex_No,	PrmRegularity_No,	0,	PrmStatus_All,	0,	0,""},
	{IDTypePacket,		"TypePacket",		PrmType_INT,	PrmIndex_No,	PrmRegularity_No,	0,	PrmStatus_All,	0,	0,"TypePacket"},
	{IDBufferiz,		"Buffered",			PrmType_STRING,	PrmIndex_No,	PrmRegularity_No,	0,	PrmStatus_All,	0,	0,"Bufferiz"},
	{IDModeSDec,		"ModeSDec",			PrmType_STRING,	PrmIndex_No,	PrmRegularity_No,	0,	PrmStatus_All,	0,	0,""},
	{IDNDemChan,		"NDemChan",			PrmType_INT,	PrmIndex_No,	PrmRegularity_No,	0,	PrmStatus_All,	0,	0,"NDemChan"},
	{IDTopUID,			"TopUID",			PrmType_INT64,	PrmIndex_No,	PrmRegularity_No,	0,	PrmStatus_All,	0,	0,""},
	{_IDHostName,		"HostName",			PrmType_STRING,	PrmIndex_No,	PrmRegularity_No,	0,	PrmStatus_All,	0,	0,""},
	{IDMonSystem,		"MonSystem",		PrmType_STRING,	PrmIndex_No,	PrmRegularity_No,	0,	PrmStatus_All,	0,	0,""},
	{IDRandomInf,		"RandomInf",		PrmType_INT,	PrmIndex_No,	PrmRegularity_No,	0,	PrmStatus_All,	0,	0,""},
	{_IDUnitName,		"UnitName",			PrmType_STRING,	PrmIndex_No,	PrmRegularity_Yes,	0,	PrmStatus_All,	0,	0,""},
	{IDTypeCli,			"TypeCli",			PrmType_STRING,	PrmIndex_No,	PrmRegularity_Yes,	0,	PrmStatus_All,	0,	0,"TypeCli"},
	{IDFullNameUp,		"PathFullNameUp",	PrmType_STRING,	PrmIndex_No,	PrmRegularity_No,	0,	PrmStatus_All,	0,	0,""},
	{IDFullNameDn,		"PathFullNameDown",	PrmType_STRING,	PrmIndex_No,	PrmRegularity_No,	0,	PrmStatus_All,	0,	0,""},
	{IDFullNameDnVoc,	"PathFullNameDownVoc",PrmType_STRING,PrmIndex_No,	PrmRegularity_No,	0,	PrmStatus_All,	0,	0,""},
	{IDFullNameUpVoc,	"PathFullNameUpVoc",PrmType_STRING,	PrmIndex_No,	PrmRegularity_No,	0,	PrmStatus_All,	0,	0,""},
	{IDSpotID,			"SpotID",			PrmType_INT,	PrmIndex_Yes,	PrmRegularity_No,	0,	PrmStatus_All,	0,	0,""},
	{IDSatLatitude,		"SatLatitude",		PrmType_DOUBLE,	PrmIndex_No,	PrmRegularity_No,	0,	PrmStatus_All,	0,	0,"SatLatitude"},
	{IDSatLongitude,	"SatLongitude",		PrmType_DOUBLE,	PrmIndex_No,	PrmRegularity_No,	0,	PrmStatus_All,	0,	0,"SatLongitude"},
	{IDRadius,			"Radius",			PrmType_DOUBLE,	PrmIndex_No,	PrmRegularity_No,	0,	PrmStatus_All,	0,	0,""},
	{IDBeamLatitude,	"BeamLatitude",		PrmType_DOUBLE,	PrmIndex_No,	PrmRegularity_No,	0,	PrmStatus_All,	0,	0,""},
	{IDBeamLongitude,	"BeamLongitude",	PrmType_DOUBLE,	PrmIndex_No,	PrmRegularity_No,	0,	PrmStatus_All,	0,	0,""},
	{IDSatID,			"SatID",			PrmType_INT,	PrmIndex_No,	PrmRegularity_No,	0,	PrmStatus_All,	0,	0,""},
	{IDSMS,				"SMS",				PrmType_STRING,	PrmIndex_No,	PrmRegularity_No,	0,	PrmStatus_All,	0,	0,"SMS"},
	{IDEstablishCause,	"EstablishCause",	PrmType_STRING,	PrmIndex_No,	PrmRegularity_No,	0,	PrmStatus_All,	0,	0,""},
	{IDLatitude,		"Latitude",			PrmType_DOUBLE,	PrmIndex_No,	PrmRegularity_No,	0,	PrmStatus_All,	0,	0,""},
	{IDLongitude,		"Longitude",		PrmType_DOUBLE,	PrmIndex_No,	PrmRegularity_No,	0,	PrmStatus_All,	0,	0,""},
	{IDIMSI,			"IMSI",				PrmType_STRING,	PrmIndex_No,	PrmRegularity_No,	0,	PrmStatus_All,	0,	0,"IMSI"},
	{IDTMSI,			"TMSI",				PrmType_STRING,	PrmIndex_No,	PrmRegularity_No,	0,	PrmStatus_All,	0,	0,"TMSI"},
	{IDIMEI,			"IMEI",				PrmType_STRING,	PrmIndex_No,	PrmRegularity_No,	0,	PrmStatus_All,	0,	0,"IMEI"},
	{IDIMEISV,			"IMEISV",			PrmType_STRING,	PrmIndex_No,	PrmRegularity_No,	0,	PrmStatus_All,	0,	0,"IMEISV"},
	{IDtempTMSI,		"tempTMSI",			PrmType_STRING,	PrmIndex_No,	PrmRegularity_No,	0,	PrmStatus_All,	0,	0,"tempTMSI"},	
	{IDNCSC_Les,		"NCSC_Les",			PrmType_INT,	PrmIndex_No,	PrmRegularity_No,	0,	PrmStatus_All,	0,	0,""},	
	{IDNCSC_Present,	"NCSC_Present",		PrmType_INT,	PrmIndex_No,	PrmRegularity_No,	0,	PrmStatus_All,	0,	0,""},	
	{IDNCSC_Mes,		"NCSC_Mes",			PrmType_INT,	PrmIndex_No,	PrmRegularity_No,	0,	PrmStatus_All,	0,	0,""},	
	{IDNCSC_Adress,		"NCSC_Adress",		PrmType_INT,	PrmIndex_No,	PrmRegularity_No,	0,	PrmStatus_All,	0,	0,""},	
	{IDNCSC_AdressType,	"NCSC_AdressType",	PrmType_INT,	PrmIndex_No,	PrmRegularity_No,	0,	PrmStatus_All,	0,	0,""},	
	{IDNCSC_SubAdress,	"NCSC_SubAdress",	PrmType_INT,	PrmIndex_No,	PrmRegularity_No,	0,	PrmStatus_All,	0,	0,""},	
	{IDUnit,			"Unit",				PrmType_STRING,	PrmIndex_No,	PrmRegularity_Yes,	0,	PrmStatus_All,	0,	0,""},	
	{IDNetSource,		"NetSource",		PrmType_STRING,	PrmIndex_No,	PrmRegularity_Yes,	0,	PrmStatus_All,	0,	0,"NetSource"},	
	{IDInput,			"Input",			PrmType_INT,	PrmIndex_No,	PrmRegularity_Yes,	0,	PrmStatus_All,	0,	0,"Input"},	
	{IDStream,			"Stream",			PrmType_INT,	PrmIndex_No,	PrmRegularity_No,	0,	PrmStatus_All,	0,	0,""},	
	{IDProcessed,		"Processed",		PrmType_STRING,	PrmIndex_No,	PrmRegularity_Yes,	0,	PrmStatus_All,	0,	0,"Processed"},	
	{IDIDEntryPoint,	"IDEntryPoint",		PrmType_INT64,	PrmIndex_No,	PrmRegularity_No,	0,	PrmStatus_All,	0,	0,"IDEntryPoint"},
	{IDTDMA_Sin,		"TDMA_Sin",			PrmType_INT64,	PrmIndex_No,	PrmRegularity_No,	0,	PrmStatus_All,	0,	0,""},
	{IDIDSeans,			"IDSeans",			PrmType_INT64,	PrmIndex_Yes,	PrmRegularity_No,	0,	PrmStatus_All,	0,	0,""},
	{IDElevation,		"Elevation",		PrmType_DOUBLE,	PrmIndex_No,	PrmRegularity_No,	0,	PrmStatus_All,	0,	0,"IDElevation"},
	{IDDeviceType,		"DeviceType",		PrmType_STRING,	PrmIndex_No,	PrmRegularity_Yes,	0,	PrmStatus_All,	0,	0,"DeviceType"},
	{IDString,			"String",			PrmType_STRING,	PrmIndex_No,	PrmRegularity_No,	0,	PrmStatus_All,	0,	0,"String"},
	{IDTimeBegin,		"StartTime",		PrmType_INT64,	PrmIndex_Yes,	PrmRegularity_No,	0,	PrmStatus_All,	0,	ParametrView_FTime,		"StartTime"},
	{IDTimeLastWr,		"ModifyTime",		PrmType_INT64,	PrmIndex_Yes,	PrmRegularity_No,	0,	PrmStatus_All,	0,	ParametrView_FTime,		"ModifyTime"},
	{IDTimeClose,		"FinishTime",		PrmType_INT64,	PrmIndex_Yes,	PrmRegularity_No,	0,	PrmStatus_All,	0,	ParametrView_FTime,		"FinishTime"},
	{IDCommand,			"Command",			PrmType_INT,	PrmIndex_No,	PrmRegularity_Yes,	0,	PrmStatus_All,	0,	0,"Command"},
	{IDStateBlock,		"BlockState",		PrmType_INT,	PrmIndex_No,	PrmRegularity_Yes,	0,	PrmStatus_All,	0,	ParametrView_Bin,"IDStateBlock"},
	{IDTypeIDI,			"TypeIDI",			PrmType_INT,	PrmIndex_Yes,	PrmRegularity_Yes,	0,	PrmStatus_All,	0,	0,"IDTypeIDI"},
	{IDNumPack,			"PackNumber",		PrmType_INT64,	PrmIndex_No,	PrmRegularity_No,	0,	PrmStatus_All,	0,	0,"IDNumPack"},
	{IDNumSyn,			"NumSyn",			PrmType_INT64,	PrmIndex_No,	PrmRegularity_No,	0,	PrmStatus_All,	0,	0,""},
	{IDNumSynAll,		"NumSynAll",		PrmType_INT64,	PrmIndex_No,	PrmRegularity_No,	0,	PrmStatus_All,	0,	0,"NumSynAll"},
	{IDDataTraffic,		"DataTraffic",		PrmType_FLOAT,	PrmIndex_No,	PrmRegularity_No,	0,	PrmStatus_All,	0,	0,""},
	{IDControlParam,	"ControlParam",		PrmType_STRING,	PrmIndex_No,	PrmRegularity_No,	0,	PrmStatus_All,	0,	0,"IDControlParam"},
	{IDLoading,			"Loading",			PrmType_FLOAT,	PrmIndex_No,	PrmRegularity_No,	0,	PrmStatus_All,	0,	0,""},
	{IDTrSpead,			"TrSpead",			PrmType_INT,	PrmIndex_No,	PrmRegularity_No,	0,	PrmStatus_All,	0,	0,""},
	{IDBufCnt,			"IDInfoBuffer",		PrmType_BLOB,	PrmIndex_No,	PrmRegularity_No,	0,	PrmStatus_All,	0,	0,"IDBufCnt"},
	{IDFile,			"File",				PrmType_STRING,	PrmIndex_No,	PrmRegularity_No,	0,	PrmStatus_All,	0,	0,""},
	{IDAGC,				"AGC",				PrmType_INT,	PrmIndex_No,	PrmRegularity_No,	0,	PrmStatus_All,	0,	0,""},
	{IDDeltaFc,			"deltaFc",			PrmType_FLOAT,	PrmIndex_No,	PrmRegularity_No,	0,	PrmStatus_All,	0,	0,"IDDeltaFc"},
	{IDNCSend,			"SendState",		PrmType_STRING,	PrmIndex_No,	PrmRegularity_Yes,	0,	PrmStatus_All,	0,	0,""},
	{IDThur_SeansID,	"Thur_SeansID",		PrmType_INT,	PrmIndex_No,	PrmRegularity_No,	0,	PrmStatus_All,	0,	0,""},
	{IDID_Standart,		"Standard ID",	 	PrmType_INT,	PrmIndex_No,	PrmRegularity_No,	0,	PrmStatus_All,	0,	0,""},
	{IDStateProc,		"StateProc",		PrmType_INT,	PrmIndex_Yes,	PrmRegularity_Yes,	0,	PrmStatus_All,	0,	0,"StateProc"},
	{_IDNCStrategic,	"NC_Strategic",		PrmType_STRING,	PrmIndex_No,	PrmRegularity_Yes,	0,	PrmStatus_All,	0,	0,""},	
	{IDNCProfile,		"NC_Profile",		PrmType_STRING,	PrmIndex_No,	PrmRegularity_No,	0,	PrmStatus_WrBD|PrmStatus_WrFS,	0,	0,""},	
	{IDProgress,		"Progress",			PrmType_FLOAT,	PrmIndex_No,	PrmRegularity_No,	0,	PrmStatus_All,	0,	0,"IDProgress"},	
	{IDCondition,		"Condition",		PrmType_INT,	PrmIndex_Yes,	PrmRegularity_Yes,	0,	PrmStatus_All,	0,	0,""},	
	{IDCloseReason,		"CloseReason",		PrmType_INT,	PrmIndex_Yes,	PrmRegularity_Yes,	0,	PrmStatus_All,	0,	0,"CloseReason"},	
	{IDWeight,			"Weight",			PrmType_INT,	PrmIndex_No,	PrmRegularity_No,	0,	PrmStatus_All,	0,	0,"Weight"},	
	{IDConfig,			"Config",			PrmType_STRING,	PrmIndex_Yes,	PrmRegularity_Yes,	0,	PrmStatus_All,	0,	0,"Config"},	
	{IDProcessingState,	"ProcessingState",	PrmType_INT,	PrmIndex_No,	PrmRegularity_Yes,	0,	PrmStatus_All,	0,	0,""},	
	{IDBuffQBE,			"BuffQBE",			PrmType_BLOB,	PrmIndex_No,	PrmRegularity_No,	0,	PrmStatus_All,	0,	0,"Buffer for QBE"},	
	{IDTimeRdAcces,		"ReadAccessTime",	PrmType_INT64,	PrmIndex_No,	PrmRegularity_No,	0,	PrmStatus_All,	0,	ParametrView_FTime,	"ReadAccessTime"},
	{IDOffsetBFile,		"OffsetBFile",		PrmType_INT,	PrmIndex_No,	PrmRegularity_No,	0,	PrmStatus_All,	0,	0,"Offset in Binary File"},
	{IDNCSendBuff,		"SendBuff",			PrmType_STRING,	PrmIndex_No,	PrmRegularity_Yes,	0,	PrmStatus_All,	0,	0,""},
	{IDNCSendTypeIDI,	"SendTypeIDI",		PrmType_STRING,	PrmIndex_No,	PrmRegularity_Yes,	0,	PrmStatus_All,	0,	0,""},
	{IDSeansNumFile,	"SeansNumFile",		PrmType_INT,	PrmIndex_No,	PrmRegularity_No,	0,	PrmStatus_All,	0,	0,""},
	{IDSeansNumFileAll,	"SeansNumFileAll",	PrmType_INT,	PrmIndex_No,	PrmRegularity_No,	0,	PrmStatus_All,	0,	0,""},
	{IDProfileCS,		N_ProfileCS,		PrmType_STRING,	PrmIndex_Yes,	PrmRegularity_Yes,	0,	PrmStatus_All,	0,	0,"ProfileCS"},
	{IDBuffCS,			"BuffCS",			PrmType_BLOB,	PrmIndex_No,	PrmRegularity_No,	0,	PrmStatus_All,	0,	0,"Buffer structur's for Context Search"},
	{IDProfileQBE,		N_ProfileQBE,		PrmType_STRING,	PrmIndex_Yes,	PrmRegularity_Yes,	0,	PrmStatus_All,	0,	0,"ProfileQBE"},
	{IDMaxQueryCnt,		"MaxQueryCnt",		PrmType_INT,	PrmIndex_No,	PrmRegularity_No,	0,	PrmStatus_All,	0,	0,"MaxQueryCnt"},
	{IDMailFrom,		"MailFrom",			PrmType_STRING,	PrmIndex_Yes,	PrmRegularity_No,	0,	PrmStatus_All,	0,	0,"MailFrom"},
	{IDMailTo,			"MailTo",			PrmType_STRING,	PrmIndex_Yes,	PrmRegularity_No,	0,	PrmStatus_All,	0,	0,"MailTo"},
	{IDMyHostName,		"MyHostName",		PrmType_STRING,	PrmIndex_No,	PrmRegularity_Yes,	0,	PrmStatus_All,	0,	0,""},
	{_IDMyUnitName,		"MyUnitHost",		PrmType_STRING,	PrmIndex_No,	PrmRegularity_Yes,	0,	PrmStatus_All,	0,	0,""},
	{IDSelection,		"Selection",		PrmType_STRING,	PrmIndex_No,	PrmRegularity_Yes,	0,	PrmStatus_All,	0,	0,"IDSelection"},	
	{IDOpenFile,		"OpenFile",			PrmType_INT,	PrmIndex_No,	PrmRegularity_No,	0,	PrmStatus_All,	0,	0,""},	
	{IDOpenFile_1s,		"OpenFile_1s",		PrmType_INT,	PrmIndex_No,	PrmRegularity_No,	0,	PrmStatus_All,	0,	0,""},	
	{IDOpenFileSF_1s,	"OpenFileSF_1s",	PrmType_INT,	PrmIndex_No,	PrmRegularity_No,	0,	PrmStatus_All,	0,	0,""},	
	{IDCloseFile_1s,	"CloseFile_1s",		PrmType_INT,	PrmIndex_No,	PrmRegularity_No,	0,	PrmStatus_All,	0,	0,""},	
	{IDCloseFileSF_1s,	"CloseFileSF_1s",	PrmType_INT,	PrmIndex_No,	PrmRegularity_No,	0,	PrmStatus_All,	0,	0,""},	
	{IDElapsedTime,		"ElapsedTime",		PrmType_INT64,	PrmIndex_No,	PrmRegularity_No,	0,	PrmStatus_All,	0,	0,"IDElapsedTime"},
	{IDMethodProcessing,"MethodProcessing",	PrmType_STRING,	PrmIndex_No,	PrmRegularity_No,	0,	PrmStatus_All,	0,	0,"IDMethodProcessing"},
	{IDMethodAnalyzing,	"MethodAnalyzing",	PrmType_STRING,	PrmIndex_No,	PrmRegularity_No,	0,	PrmStatus_All,	0,	0,"IDMethodAnalyzing"},
	{IDIDEntryPointNext,"IDEntryPointNext",	PrmType_INT64,	PrmIndex_No,	PrmRegularity_No,	0,	PrmStatus_All,	0,	0,"IDEntryPoint"},
	{IDNumberInputs,	"NumberInputs",		PrmType_INT64,	PrmIndex_No,	PrmRegularity_No,	0,	PrmStatus_All,	0,	0,"Number of device Inputs"},
	{IDNumberOutputs,	"NumberOutputs",	PrmType_INT64,	PrmIndex_No,	PrmRegularity_No,	0,	PrmStatus_All,	0,	0,"Number of device Outputs"},
	{IDOutput,			"Output",			PrmType_INT,	PrmIndex_No,	PrmRegularity_Yes,	0,	PrmStatus_All,	0,	0,"Output"},	
	{IDIPAdr,			"IPAdr",			PrmType_INT,	PrmIndex_Yes,	PrmRegularity_No,	0,	PrmStatus_All,	0,	ParametrView_IPAddr,	"IP address"},
	{IDDevice,			"Device",			PrmType_STRING,	PrmIndex_No,	PrmRegularity_Yes,	0,	PrmStatus_All,	0,	0,"Device Name (for link)"},
	{IDModeCRC,			"ModeCRC",			PrmType_STRING,	PrmIndex_No,	PrmRegularity_Yes,	0,	PrmStatus_All,	0,	0,"Mode CRC (None,CRC16,CRC32,...)"},
	{IDCoordinateX,		"Coordinate_X",		PrmType_INT,	PrmIndex_No,	PrmRegularity_No,	0,	PrmStatus_All,	0,	0,"Relative coordinate X"},
	{IDCoordinateY,		"Coordinate_Y",		PrmType_INT,	PrmIndex_No,	PrmRegularity_No,	0,	PrmStatus_All,	0,	0,"Relative coordinate Y"},
	{IDFunctionType,	"FunctionType",		PrmType_STRING,	PrmIndex_No,	PrmRegularity_Yes,	0,	PrmStatus_All,	0,	0,"Function type device(Source, Conductor, Consumer, e.t.)"},
	{IDPort,			"Port",				PrmType_STRING,	PrmIndex_No,	PrmRegularity_No,	0,	PrmStatus_All,	0,	0,"Function type device(Source, Conductor, Consumer, e.t.)"},
	{IDDeviceFrom,		"DeviceFrom",		PrmType_STRING,	PrmIndex_No,	PrmRegularity_Yes,	0,	PrmStatus_All,	0,	0,"Output device"},
	{IDDeviceTo,		"DeviceTo",			PrmType_STRING,	PrmIndex_No,	PrmRegularity_Yes,	0,	PrmStatus_All,	0,	0,"Input device"},
	{IDHost,			"Host",				PrmType_STRING,	PrmIndex_Yes,	PrmRegularity_Yes,	0,	PrmStatus_All,	0,	0,"Connected host name"},
	{IDIntInput,		"IntInput",			PrmType_STRING,	PrmIndex_Yes,	PrmRegularity_Yes,	0,	PrmStatus_All,	0,	0,"Link to internal input"},
	{IDExtInput,		"ExtInput",			PrmType_STRING,	PrmIndex_Yes,	PrmRegularity_Yes,	0,	PrmStatus_All,	0,	0,"Link to external input"},
	{IDExtOutput,		"ExtOutput",		PrmType_STRING,	PrmIndex_Yes,	PrmRegularity_Yes,	0,	PrmStatus_All,	0,	0,"Link to external output"},
	{IDEntryPointName,	"EntryPointName",	PrmType_STRING,	PrmIndex_Yes,	PrmRegularity_No,	0,	PrmStatus_All,	0,	0,"Name elements table for associating"},
	{IDEntryPointNameSel,"EntryPointNameSel",PrmType_STRING,PrmIndex_Yes,	PrmRegularity_No,	0,	PrmStatus_All,	0,	0,"Name selected elements table associating for BD"},
	{IDConvSpeed,		"ConvSpeed",		PrmType_STRING,	PrmIndex_No,	PrmRegularity_Yes,	0,	PrmStatus_All,	0,	0,"Conv. speed (1/2,2/3,3/4,...)"},
	{IDFrequencyHeterodyne,	"HeterodyneFrequency",	PrmType_INT64,	PrmIndex_No,	PrmRegularity_No,	0,	PrmStatus_All,	0,	0,"IDFrequencyHeterodyne"},
	{IDModulationType,	"ModulationType",	PrmType_STRING,	PrmIndex_No,	PrmRegularity_Yes,	0,	PrmStatus_All,	0,	0,"Modulation Type"},
	{IDSymbolRate,		"SymbolRate",		PrmType_DOUBLE,	PrmIndex_No,	PrmRegularity_No,	0,	PrmStatus_All,	0,	0,"SymbolRate"},
	{IDFrequencyRF,		"FrequencyRF",		PrmType_INT64,	PrmIndex_No,	PrmRegularity_No,	0,	PrmStatus_All,	0,	0,""},
	{IDFrequencyIF,		"FrequencyIF",		PrmType_INT64,	PrmIndex_No,	PrmRegularity_No,	0,	PrmStatus_All,	0,	0,"IDFrequencyIF"},
	{IDFrequencyLowerOut,"LowerFrequencyOut",PrmType_INT64,	PrmIndex_No,	PrmRegularity_No,	0,	PrmStatus_All,	0,	0,"Lower Frequency Out"},
	{IDFrequencyUpperOut,"UpperFrequencyOut",PrmType_INT64,	PrmIndex_No,	PrmRegularity_No,	0,	PrmStatus_All,	0,	0,"Upper Frequency Out"},
	{IDBandwidth,		"Bandwidth",		PrmType_INT64,	PrmIndex_Yes,	PrmRegularity_No,	0,	PrmStatus_All,	0,	0,"Bandwidth"},
	{IDBitRate,			"BitRate",			PrmType_INT64,	PrmIndex_Yes,	PrmRegularity_No,	0,	PrmStatus_All,	0,	0,"BitRate"},
	{IDLocked,			"Locked",			PrmType_STRING,	PrmIndex_Yes,	PrmRegularity_Yes,	0,	PrmStatus_All,	0,	0,"Locked"},
	{IDParameterSet,	"ParameterSet",		PrmType_STRING,	PrmIndex_Yes,	PrmRegularity_No,	0,	PrmStatus_All,	0,	0,"ParameterSet"},
	{IDBeam,			"Beam",				PrmType_STRING,	PrmIndex_Yes,	PrmRegularity_No,	0,	PrmStatus_All,	0,	0,""},
	{IDBearerType,		"Bearer_type",		PrmType_STRING,	PrmIndex_Yes,	PrmRegularity_Yes,	0,	PrmStatus_All,	0,	0,"Bearer_type"},
	{IDPolarization,	"Polarization",		PrmType_STRING,	PrmIndex_Yes,	PrmRegularity_Yes,	0,	PrmStatus_All,	0,	0,"Polarization"},
	{IDSampleTime,		"SampleTime",		PrmType_DOUBLE,	PrmIndex_No,	PrmRegularity_No,	0,	PrmStatus_All,	0,	0,"IDSampleTime"},
	{IDGainLevel,		"GainLevel",		PrmType_INT,	PrmIndex_No,	PrmRegularity_No,	0,	PrmStatus_All,	0,	0,"GainLevel"},
 	{IDPolynome1,		"Polynome1",		PrmType_STRING,	PrmIndex_No,	PrmRegularity_Yes,	0,	PrmStatus_All,	0,	0,""},
 	{IDPolynome2,		"Polynome2",		PrmType_STRING,	PrmIndex_No,	PrmRegularity_Yes,	0,	PrmStatus_All,	0,	0,""},
	{IDECCType,			"ECCType",			PrmType_STRING,	PrmIndex_Yes,	PrmRegularity_Yes,	0,	PrmStatus_All,	0,	0,"ECCType"},
	{IDZeroFrequency,	"ZeroFrequency",	PrmType_INT64,	PrmIndex_No,	PrmRegularity_No,	0,	PrmStatus_All,	0,	0,"ZeroFrequency"},
	{IDInvert,			"Invert",			PrmType_STRING,	PrmIndex_No,	PrmRegularity_No,	0,	PrmStatus_All,	0,	0,"Invert"},
	{IDNTaps,			"NTaps",			PrmType_INT,	PrmIndex_No,	PrmRegularity_Yes,	0,	PrmStatus_All,	0,	0,"Parameter for DigitalRec"},
	{IDVirtChan,		"VirtualChannel",	PrmType_INT,	PrmIndex_No,	PrmRegularity_Yes,	0,	PrmStatus_All,	0,	0,"Number virtual channeNDemChanl"},
	{IDFrequencyLowerIn,"LowerFrequencyIn",	PrmType_INT64,	PrmIndex_No,	PrmRegularity_No,	0,	PrmStatus_All,	0,	0,"IDFrequencyLowerIn"},
	{IDFrequencyUpperIn,"UpperFrequencyIn",	PrmType_INT64,	PrmIndex_No,	PrmRegularity_No,	0,	PrmStatus_All,	0,	0,"IDFrequencyUpperIn"},
	{IDEp,				"Ep",				PrmType_DOUBLE,	PrmIndex_No,	PrmRegularity_No,	0,	PrmStatus_All,	0,	0,"Ep"},
	{IDMAC_Address,		"MAC_Address",		PrmType_INT64,	PrmIndex_Yes,	PrmRegularity_No,	0,	PrmStatus_All,	0,	0,"MAC_Address"},
	{IDStep,			"Step",				PrmType_INT64,	PrmIndex_No,	PrmRegularity_No,	0,	PrmStatus_All,	0,	0,"Step"},
	{IDTimeout,			"Timeout",			PrmType_INT64,	PrmIndex_No,	PrmRegularity_No,	0,	PrmStatus_All,	0,	0,"Timeout"},
	{IDScramblerType,	"ScramblerType",	PrmType_STRING,	PrmIndex_No,	PrmRegularity_Yes,	0,	PrmStatus_All,	0,	0,"ScramblerType"},
	{IDProfileFilter,	N_ProfileFilter,	PrmType_STRING,	PrmIndex_Yes,	PrmRegularity_Yes,	0,	PrmStatus_All,	0,	0,"ProfileFilter"},
	{IDInterleaving,	"Interleaving",		PrmType_STRING,	PrmIndex_No,	PrmRegularity_Yes,	0,	PrmStatus_All,	0,	0,"Interleaving (example:for Reed-Solomon code)"},
	{IDIntCommand,		"IntCommand",		PrmType_STRING,	PrmIndex_No,	PrmRegularity_Yes,	0,	PrmStatus_All,	0,	0,"IntCommand"},
	{IDState,			"State",			PrmType_STRING,	PrmIndex_No,	PrmRegularity_Yes,	0,	PrmStatus_All,	0,	0,"State"},
	{IDPersonalID,		"PersonalID",		PrmType_INT64,	PrmIndex_No,	PrmRegularity_No,	0,	PrmStatus_All,	0,	0,"IDPersonalID"},
	{IDFc,				"Fc",				PrmType_DOUBLE,	PrmIndex_No,	PrmRegularity_No,	0,	PrmStatus_All,	0,	0,""},
	{IDSource,			"Source",			PrmType_STRING,	PrmIndex_Yes,	PrmRegularity_No,	0,	PrmStatus_All,	0,	0,"Source"},
	{IDParameter,		"Parameter",		PrmType_STRING,	PrmIndex_No,	PrmRegularity_Yes,	0,	PrmStatus_All,	0,	0,"Parameter"},
	{IDCriterion,		"Criterion",		PrmType_STRING,	PrmIndex_Yes,	PrmRegularity_No,	0,	PrmStatus_All,	0,	0,"Criterion"},
	{IDTimeoutProc,		"TimeoutProc",		PrmType_INT64,	PrmIndex_No,	PrmRegularity_No,	0,	PrmStatus_All,	0,	0,"Timeout demodulation or processing"},
	{IDTimeoutTuning,	"TimeoutTuning",	PrmType_INT64,	PrmIndex_No,	PrmRegularity_No,	0,	PrmStatus_All,	0,	0,"Timeout tuning"},
	{IDSpectrum,		"Spectrum",			PrmType_USER,	PrmIndex_No,	PrmRegularity_No,	0,	PrmStatus_All,	sizeof(Str_Spectrum),	0,"Spectrum sample"},
	{IDSignalLevel,		"SignalLevel",		PrmType_DOUBLE,	PrmIndex_No,	PrmRegularity_No,	0,	PrmStatus_All,	0,	0,"Signal level"},
	{ID_K11,			"K11",				PrmType_DOUBLE,	PrmIndex_No,	PrmRegularity_No,	0,	PrmStatus_All,	0,	0,"Coefficient K11"},
	{ID_K12,			"K12",				PrmType_DOUBLE,	PrmIndex_No,	PrmRegularity_No,	0,	PrmStatus_All,	0,	0,"Coefficient K12"},
	{ID_Kea1,			"Kea1",				PrmType_DOUBLE,	PrmIndex_No,	PrmRegularity_No,	0,	PrmStatus_All,	0,	0,"Coefficient Kea1"},
	{ID_Kea2,			"Kea2",				PrmType_DOUBLE,	PrmIndex_No,	PrmRegularity_No,	0,	PrmStatus_All,	0,	0,"Coefficient Kea2"},
	{ID_KuP1,			"KuP1",				PrmType_DOUBLE,	PrmIndex_No,	PrmRegularity_No,	0,	PrmStatus_All,	0,	0,"Coefficient KuP1"},
	{ID_KuP2,			"KuP2",				PrmType_DOUBLE,	PrmIndex_No,	PrmRegularity_No,	0,	PrmStatus_All,	0,	0,"Coefficient KuP2"},
	{IDLNB_Power,		"LNB_Power",		PrmType_STRING,	PrmIndex_No,	PrmRegularity_Yes,	0,	PrmStatus_All,	0,	0,"Power of LNB 13/18V"},
	{IDPeriod,			"Period",			PrmType_INT64,	PrmIndex_No,	PrmRegularity_No,	0,	PrmStatus_All,	0,	0,"Bit stream period"},
	{IDTransponder,		"Transponder",		PrmType_STRING,	PrmIndex_Yes,	PrmRegularity_Yes,	0,	PrmStatus_All,	0,	0,"Satellite transponder"},
	{IDSyn,				"Syn",				PrmType_INT64,	PrmIndex_No,	PrmRegularity_No,	0,	PrmStatus_All,	0,	0,"Synchronization word"},
	{IDSynLen,			"SynLen",			PrmType_INT64,	PrmIndex_No,	PrmRegularity_No,	0,	PrmStatus_All,	0,	0,"Synchronization length"},
	{IDPeriodPrime,		"PeriodPrime",		PrmType_STRING,	PrmIndex_No,	PrmRegularity_No,	0,	PrmStatus_All,	0,	0,"Prime decomposition"},
	{IDCountryNameFrom,	"CountryNameFrom",	PrmType_STRING,	PrmIndex_Yes,	PrmRegularity_Yes,	0,	PrmStatus_All,	0,	0,"Country Name From"},
	{IDCountryNameTo,	"CountryNameTo",	PrmType_STRING,	PrmIndex_Yes,	PrmRegularity_Yes,	0,	PrmStatus_All,	0,	0,"Country Name To"},
	{IDCountryCodeFrom,	"CountryCodeFrom",	PrmType_STRING,	PrmIndex_Yes,	PrmRegularity_Yes,	0,	PrmStatus_All,	0,	0,"Country Code From"},
	{IDCountryCodeTo,	"CountryCodeTo",	PrmType_STRING,	PrmIndex_Yes,	PrmRegularity_Yes,	0,	PrmStatus_All,	0,	0,"Country Code To"},
	{IDRole,			"Role",				PrmType_STRING,	PrmIndex_Yes,	PrmRegularity_Yes,	0,	PrmStatus_All,	0,	0,"Role"},
	{_IDNativeHostName,	"NativeHostName",	PrmType_STRING,	PrmIndex_No,	PrmRegularity_Yes,	0,	PrmStatus_All,	0,	0,""},
	{_IDNativeUnitName,	"NativeUnitHost",	PrmType_STRING,	PrmIndex_No,	PrmRegularity_Yes,	0,	PrmStatus_All,	0,	0,""},
	{IDInterception,	"Interception",		PrmType_STRING,	PrmIndex_No,	PrmRegularity_Yes,	0,	PrmStatus_All,	0,	0,"Interception mode"},
	{_IDMyPersonalID,	"MyPersonalID",		PrmType_INT64,	PrmIndex_No,	PrmRegularity_No,	0,	PrmStatus_All,	0,	0,""},
	{IDCreatingType,	"CreatingType",		PrmType_STRING,	PrmIndex_No,	PrmRegularity_Yes,	0,	PrmStatus_All,	0,	0,"Type of creating method"},
	{IDAttribute,		"Attribute",		PrmType_INT64,	PrmIndex_No,	PrmRegularity_No,	0,	PrmStatus_All,	0,	0,"Element Attribute"},
	{IDEPNameMerging,	"EntryPointNameMerging",	PrmType_STRING,	PrmIndex_Yes,	PrmRegularity_No,	0,	PrmStatus_All,	0,	0,"Merging Entry Point Name"},
	{IDEPNameDuplexing,	"EntryPointNameDuplexing",	PrmType_STRING,	PrmIndex_Yes,	PrmRegularity_No,	0,	PrmStatus_All,	0,	0,"Duplexing Entry Point Name"},
	{IDEnoise,			"Enoise",			PrmType_DOUBLE,	PrmIndex_No,	PrmRegularity_No,	0,	PrmStatus_All,	0,	0,"Noise energy"},
	{IDZeroFrequencyIn,	"ZeroFrequencyIn",	PrmType_INT64,	PrmIndex_No,	PrmRegularity_No,	0,	PrmStatus_All,	0,	0,"ZeroFrequencyIn"},
	{IDZeroFrequencyOut,"ZeroFrequencyOut",	PrmType_INT64,	PrmIndex_No,	PrmRegularity_No,	0,	PrmStatus_All,	0,	0,"ZeroFrequencyOut"},
	{IDAnalyzeModeName,	"AnalyseModeName",	PrmType_STRING,	PrmIndex_No,	PrmRegularity_Yes,	0,	PrmStatus_All,	0,	0,"Analyse Mode"},
	{_IDNativeDomainName,"NativeDomainName",	PrmType_STRING,	PrmIndex_No,	PrmRegularity_No,	0,	PrmStatus_All,	0,	0,"Native Domain Name"},
	{_IDDomainName,		"DomainName",		PrmType_STRING,	PrmIndex_No,	PrmRegularity_No,	0,	PrmStatus_All,	0,	0,"Domain Name"},
	{_IDUnitMode,		"UnitMode",			PrmType_STRING,	PrmIndex_No,	PrmRegularity_No,	0,	PrmStatus_All,	0,	0,"Unit mode"},
	{IDDBManName,		"DBManName",		PrmType_STRING,	PrmIndex_No,	PrmRegularity_No,	0,	PrmStatus_All,	0,	0,""},
	{IDMessageFrom,		"MessageFrom",		PrmType_STRING,	PrmIndex_Yes,	PrmRegularity_No,	0,	PrmStatus_All,	0,	0,"Message From"},
	{IDMessageTo,		"MessageTo",		PrmType_STRING,	PrmIndex_Yes,	PrmRegularity_No,	0,	PrmStatus_All,	0,	0,"Message To"},
	{IDMessage,			"Message",			PrmType_STRING,	PrmIndex_Yes,	PrmRegularity_No,	0,	PrmStatus_All,	0,	0,"Message"},
	{IDLocal,			"Local",			PrmType_INT,	PrmIndex_No,	PrmRegularity_Yes,	0,	PrmStatus_All,	0,	0,"Sign of local operation"},
	{IDPathStorage,		"PathStorage",		PrmType_STRING,	PrmIndex_No,	PrmRegularity_No,	0,	PrmStatus_All,	0,	0,"Path of Storage resource"},
	{IDOwner,			"Owner",			PrmType_STRING,	PrmIndex_No,	PrmRegularity_No,	0,	PrmStatus_All,	0,	0,"Owner of Domain"},
	{IDTuner,			"Tuner",			PrmType_STRING,	PrmIndex_No,	PrmRegularity_Yes,	0,	PrmStatus_All,	0,	0,"Tuner mode"},
	{IDScanner,			"Scanner",			PrmType_STRING,	PrmIndex_No,	PrmRegularity_Yes,	0,	PrmStatus_All,	0,	0,"Scanner mode"},
	{IDNetViewProcessed,"NetViewProcessed",	PrmType_STRING,	PrmIndex_No,	PrmRegularity_Yes,	0,	PrmStatus_All,	0,	0,"NetView Processed"},	
	{IDGainLevel1,		"GainLevel1",		PrmType_INT,	PrmIndex_No,	PrmRegularity_No,	0,	PrmStatus_All,	0,	0,"GainLevel1"},
	{IDGainLevel2,		"GainLevel2",		PrmType_INT,	PrmIndex_No,	PrmRegularity_No,	0,	PrmStatus_All,	0,	0,"GainLevel2"},
	{IDGainLevel3,		"GainLevel3",		PrmType_INT,	PrmIndex_No,	PrmRegularity_No,	0,	PrmStatus_All,	0,	0,"GainLevel3"},
	{IDRealHost,		"RealHost",			PrmType_STRING,	PrmIndex_No,	PrmRegularity_No,	0,	PrmStatus_All,	0,	0,"IDRealHost"},
	{IDMaxSize,			"MaxSize",			PrmType_INT,	PrmIndex_No,	PrmRegularity_No,	0,	PrmStatus_All,	0,	0,""},
	{IDTypeView,		"TypeView",			PrmType_STRING,	PrmIndex_No,	PrmRegularity_No,	0,	PrmStatus_All,	0,	0,"TypeView"},
	{IDContainerType,	"ContainerType",	PrmType_STRING,	PrmIndex_No,	PrmRegularity_Yes,	0,	PrmStatus_All,	0,	0,"ContainerType"},
	{IDRF_Amp,			"RF_Amplifier",		PrmType_INT,	PrmIndex_No,	PrmRegularity_No,	0,	PrmStatus_All,	0,	0,"RF_Amplifier"},
	{IDRF_Att,			"RF_Attenuator",	PrmType_INT,	PrmIndex_No,	PrmRegularity_No,	0,	PrmStatus_All,	0,	0,"RF_Attenuator"},	
	{IDCountError,		"CountError",		PrmType_INT,	PrmIndex_No,	PrmRegularity_No,	0,	PrmStatus_All,	0,	0,"Count error"},	
	{IDSaveMode,		"SaveMode",			PrmType_INT,	PrmIndex_No,	PrmRegularity_No,	0,	PrmStatus_All,	0,	0,"SaveMode 1=Energy 2=ManualSave 16-SS7"},	
	{IDProfileSchema,	N_ProfileSchema,	PrmType_STRING,	PrmIndex_No,	PrmRegularity_Yes,	0,	PrmStatus_All,	0,	0,""},
	{IDRepeat,			"Repeat",			PrmType_INT,	PrmIndex_No,	PrmRegularity_Yes,	0,	PrmStatus_All,	0,	0,"Flag repeat"},
	{IDReadSpeed,		"ReadSpeed",		PrmType_FLOAT,	PrmIndex_No,	PrmRegularity_No,	0,	PrmStatus_All,	0,	0,"IDReadSpeed"},
	{IDNoDelete,		"NoDelete",			PrmType_INT,	PrmIndex_No,	PrmRegularity_No,	0,	PrmStatus_All,	0,	0,"Системный параметр заперта удаления данного элемента"},

	{IDAzimuth,			"Azimuth",			PrmType_DOUBLE,	PrmIndex_No,	PrmRegularity_No,	0,	PrmStatus_All,	0,	0,""},
	{IDdAzimuth,		"dAzimuth",			PrmType_DOUBLE,	PrmIndex_No,	PrmRegularity_No,	0,	PrmStatus_All,	0,	0,""},
	{IDdElevation,		"dElevation",		PrmType_DOUBLE,	PrmIndex_No,	PrmRegularity_No,	0,	PrmStatus_All,	0,	0,""},
	{IDProcessingHost,	"ProcessingHost",	PrmType_STRING,	PrmIndex_No,	PrmRegularity_Yes,	0,	PrmStatus_All,	0,	0,"Host name where processing this element"},
	{_IDNativeUnitlID,	"NativeUnitlID",	PrmType_INT64,	PrmIndex_No,	PrmRegularity_No,	0,	PrmStatus_All,	0,	0,""},
	{IDMCC,				"MCC",				PrmType_STRING,	PrmIndex_Yes,	PrmRegularity_Yes,	0,	PrmStatus_All,	0,	0,"Mobile Country Code"},
	{IDMNC,				"MNC",				PrmType_STRING,	PrmIndex_No,	PrmRegularity_Yes,	0,	PrmStatus_All,	0,	0,"Mobile Network Code"},
	{IDMCC_MNC,			"MCC_MNC",			PrmType_STRING,	PrmIndex_Yes,	PrmRegularity_Yes,	0,	PrmStatus_All,	0,	0,"MCC / MNC tuple"},
	{IDNameOperator,	"NameOperator",		PrmType_STRING,	PrmIndex_Yes,	PrmRegularity_Yes,	0,	PrmStatus_All,	0,	0,"Name operator"},
	{IDFlagDeleteDB,	"FlagDeleteDB",		PrmType_STRING,	PrmIndex_No,	PrmRegularity_Yes,	0,	PrmStatus_All,	0,	0,""},
	{IDPilottone,		"Pilottone",		PrmType_STRING,	PrmIndex_No,	PrmRegularity_Yes,	0,	PrmStatus_All,	0,	0,"IDPilottone"},
	{IDNetType,			"NetType",			PrmType_STRING,	PrmIndex_No,	PrmRegularity_Yes,	0,	PrmStatus_All,	0,	0,"Type of Network PAN, LAN, WAN"},
	{IDNumberDaysStorage,"StorageTime",		PrmType_INT,	PrmIndex_No,	PrmRegularity_No,	0,	PrmStatus_All,	0,	0,"Number of days of storage for DBs or FileServers"},
	{IDBuffDiz,			"BuffDiz",			PrmType_BLOB,	PrmIndex_No,	PrmRegularity_No,	0,	PrmStatus_All,	0,	0,"Info for diz-file"},	
	{IDN_Pin,			"N_Pin",			PrmType_INT,	PrmIndex_No,	PrmRegularity_No,	0,	PrmStatus_All,	0,	0,"N_Pin"},	
	{IDNameDB,			"Name_DB",			PrmType_STRING,	PrmIndex_Yes,	PrmRegularity_No,	0,	PrmStatus_All,	0,	0,"Name DB"},	
	{IDNameDBDel,		"Name_DB_Del",		PrmType_STRING,	PrmIndex_Yes,	PrmRegularity_No,	0,	PrmStatus_All,	0,	0,"Name DB where record must be removed"},
	{IDInformationType,	"InformationType",	PrmType_STRING,	PrmIndex_Yes,	PrmRegularity_Yes,	0,	PrmStatus_All,	0,	0,"InformationType"},	
	{IDStrOwner,		"StrOwner",			PrmType_BLOB,	PrmIndex_No,	PrmRegularity_No,	0,	PrmStatus_All,	0,	0,""},	
	{IDDCMEType,		"DCME_Type",		PrmType_STRING,	PrmIndex_Yes,	PrmRegularity_Yes,	0,	PrmStatus_All,	0,	0,"Type DCME Stream"},
	{IDNameSatellite,	"NameSatellite",	PrmType_STRING,	PrmIndex_Yes,	PrmRegularity_No,	0,	PrmStatus_All,	0,	0,"Name satellite"},

	{IDFrame_Syn,		"FrameSyn",			PrmType_STRING,	PrmIndex_No,	PrmRegularity_No,	0,	PrmStatus_All,	0,	0,"IDFrame_Syn"},
	{IDFrame_Length,	"FrameLenght",		PrmType_INT,	PrmIndex_No,	PrmRegularity_No,	0,	PrmStatus_All,	0,	0,"Frame size"},	
	{IDFrame_Pause,		"FramePause",		PrmType_INT,	PrmIndex_No,	PrmRegularity_No,	0,	PrmStatus_All,	0,	0,"IDFrame_Pause"},	
	{IDSuperFrame_Length,"SuperFrame Lenght",PrmType_INT,	PrmIndex_No,	PrmRegularity_No,	0,	PrmStatus_All,	0,	0,"IDSuperFrame_Length"},	
	{IDSuperFrame_Pause,"SuperFrame Pause",	PrmType_INT,	PrmIndex_No,	PrmRegularity_No,	0,	PrmStatus_All,	0,	0,"IDSuperFrame_Pause"},	
	{IDSuperFrame_FrameNumber,"SuperFrame FrameNumber",PrmType_INT,	PrmIndex_No,	PrmRegularity_No,	0,	PrmStatus_All,	0,	0,"IDSuperFrame_FrameNumber"},	
	{IDAddressInfo,		"AddressInfo",		PrmType_BLOB,	PrmIndex_No,	PrmRegularity_No,	0,	PrmStatus_All,	0,	0,"Structure AddressInfo"},	
	{IDHardwareType,	"EquipmentType",	PrmType_STRING,	PrmIndex_No,	PrmRegularity_No,	0,	PrmStatus_All,	0,	0,"EquipmentType"},
	{IDErrorID,			"ErrorID",			PrmType_INT64,	PrmIndex_No,	PrmRegularity_Yes,	0,	PrmStatus_All,	0,	0,"ErrorID"},
	
	{IDReverse,		"Reverse",			PrmType_STRING,	PrmIndex_No,	PrmRegularity_No,	0,	PrmStatus_All,	0,	0,"Reverse data"},
	{IDBitPerSample,	"BitPerSample",		PrmType_INT,	PrmIndex_No,	PrmRegularity_No,	0,	PrmStatus_All,	0,	0,"Bit Per Sample"},
	{IDBufferingTime,	"BufferingTime",	PrmType_INT64,	PrmIndex_No,	PrmRegularity_No,	0,	PrmStatus_All,	0,	0,"Buffering time (ms)"},	

	{IDGroupName,		"GroupName",		PrmType_STRING,	PrmIndex_No,	PrmRegularity_No,	0,	PrmStatus_All,	0,	0,""},
	{IDUserLogin,		"UserLogin",		PrmType_STRING,	PrmIndex_No,	PrmRegularity_No,	0,	PrmStatus_All,	0,	0,""},
	{IDUserFirstName,	"UserFirstName",	PrmType_STRING,	PrmIndex_No,	PrmRegularity_No,	0,	PrmStatus_All,	0,	0,""},
	{IDUserLastName,	"UserLastName",		PrmType_STRING,	PrmIndex_No,	PrmRegularity_No,	0,	PrmStatus_All,	0,	0,""},
	{IDUserPassword,	"UserPassword",		PrmType_USER,	PrmIndex_No,	PrmRegularity_No,	0,	PrmStatus_All,	0,	0,""},
	{IDUserExpiryTime,	"UserExpiryTime",	PrmType_INT64,	PrmIndex_No,	PrmRegularity_No,	0,	PrmStatus_All,	0,	ParametrView_FTime,""},
	{IDUserPersID,		"UserPersID",		PrmType_INT64,	PrmIndex_No,	PrmRegularity_No,	0,	PrmStatus_All,	0,	0,""},	
	{IDTaskName,		"TaskName",			PrmType_STRING,	PrmIndex_No,	PrmRegularity_No,	0,	PrmStatus_All,	0,	0,""},
	{IDThemeName,		"ThemeName",		PrmType_STRING,	PrmIndex_No,	PrmRegularity_No,	0,	PrmStatus_All,	0,	0,""},
	{IDOriginatorID,		"OriginatorID",	PrmType_INT64,	PrmIndex_No,	PrmRegularity_No,	0,	PrmStatus_All,	0,	0,""},
	{IDImplementerID,		"ImplementerID",	PrmType_INT64,	PrmIndex_No,	PrmRegularity_No,	0,	PrmStatus_All,	0,	0,""},
	{IDResponsiblePersonID,	"ResponsiblePersonID",	PrmType_INT64,	PrmIndex_No,	PrmRegularity_No,	0,	PrmStatus_All,	0,	0,"ResponsiblePersonID"},
	{IDAuditorID,		"IDAuditorID",		PrmType_INT64,	PrmIndex_No,	PrmRegularity_No,	0,	PrmStatus_All,	0,	0,""},
	{IDDeadline,		"Deadline",			PrmType_INT64,	PrmIndex_No,	PrmRegularity_No,	0,	PrmStatus_All,	0,	ParametrView_FTime,"Last time, the execution of tasks"},
	{IDEstimatedTime,	"EstimatedTime",	PrmType_INT64,	PrmIndex_No,	PrmRegularity_No,	0,	PrmStatus_All,	0,	ParametrView_FTime,"That wants to spend the Implementer"},
	{IDActualTime,		"ActualTime",		PrmType_INT64,	PrmIndex_No,	PrmRegularity_No,	0,	PrmStatus_All,	0,	ParametrView_FTime,"Time actually spent on the task"},
	{IDHistoryFileID,	"HistoryFileID",	PrmType_INT64,	PrmIndex_No,	PrmRegularity_No,	0,	PrmStatus_All,	0,	0,"Identifier if History file"},
	{IDMsgAttribute,	"MsgAttribute",		PrmType_INT64,	PrmIndex_No,	PrmRegularity_No,	0,	PrmStatus_All,	0,	0,"Massage attribute"},
	{IDTaskID,			"TaskID",			PrmType_INT64,	PrmIndex_No,	PrmRegularity_No,	0,	PrmStatus_All,	0,	0,"Identifier of Task"},
	{IDOwnerTaskID,		"OwnerTaskID",		PrmType_INT64,	PrmIndex_No,	PrmRegularity_No,	0,	PrmStatus_All,	0,	0,""},
	{IDThemeID,			"ThemeID",			PrmType_INT64,	PrmIndex_No,	PrmRegularity_No,	0,	PrmStatus_All,	0,	0,""},
	{IDSiteID,			"SiteID",			PrmType_INT,	PrmIndex_No,	PrmRegularity_No,	0,	PrmStatus_All,	0,	0,""},
	{IDMessageType,		"MessageType",		PrmType_INT,	PrmIndex_No,	PrmRegularity_No,	0,	PrmStatus_All,	0,	0,"MessageType"},
	{IDMessageID,		"MessageID",		PrmType_INT64,	PrmIndex_No,	PrmRegularity_No,	0,	PrmStatus_All,	0,	0,"MessageID"},
	{IDRecipientID,		"RecipientID",		PrmType_INT64,	PrmIndex_No,	PrmRegularity_No,	0,	PrmStatus_All,	0,	0,""},
	{IDPosition,		"Position",			PrmType_INT,	PrmIndex_No,	PrmRegularity_No,	0,	PrmStatus_All,	0,	0,"Current position"},
	{IDUserStatus,		"UserStatus",		PrmType_INT,	PrmIndex_No,	PrmRegularity_No,	0,	PrmStatus_All,	0,	0,"User Status"},
	{IDRequiredTime,	"RequiredTime",		PrmType_INT64,	PrmIndex_No,	PrmRegularity_No,	0,	PrmStatus_All,	0,	ParametrView_FTime,"Time that requires the head"},
	
	
	{0}

};

static	Customer	CustomerList[]=
{
	{0xffff,0x1,"Sigmatech, Developer site",0x1},		// Зарезервировано
	{0xffff,0x2,"Sigmatech, Sigma site",0x1},	// Зарезервировано
	{0xffff,0x3,"Sigmatech, Tester site",0x1},		// Зарезервировано
	{0xffff,0x4,"Sigmatech, Manager site",0x1},		// Зарезервировано
	{0xffff,0x5,"Sigmatech, HardWare site",0x1},	// Зарезервировано
	{0xffff,0x6,"Sigmatech, Demo site",0x1},		// Зарезервировано
// -------------------------------------------------------------------
	{0x0,0x1,"Foreign Intelligence of Ukraine, Kolos site",0x0},
	{0x1,0x1,"Security Service of Ukraine, Marhalevka site",0x1},
	{0x1,0x2,"Security Service of Ukraine, Kolos site",0x1},
	{0x2,0x1,"Border Service of Ukraine, Lutck site",0x1},
	{0x3,0x1,"Border Service of Ukraine, SS site",0x1},
	{0x4,0x1,"Unknown, site",0x0},
	{0x5,0x1,"CabSec India, Rajkot site",0x1},
	{0x5,0x1,"CabSec India, Kolcata site",0x1},
	{0x6,0x1,"BEL India, Kolcata site",0x1},
	{0x7,0x1,"Unknown, site",0x0},
	{0x8,0x1,"ICM Group Ukraine, Kiev site",0x1},
	{0x9,0x1,"Vastech SAR, Demo site",0x1},
	{0xa,0x1,"IBH Impex Germany, Desau site",0x1},
	{0xb,0x1,"Unknown, site",0x0},
	{0xc,0x1,"Unknown, site",0x0},
	{0xd,0x1,"Unknown, site",0x0},
	{0xe,0x1,"Unknown, site",0x0},
	{0xf,0x1,"Unknown, site",0x0},
	{0x10,0x1,"Unknown, site",0x0},
	{0x11,0x1,"ElVit Ukraine, Lviv site",0x1},
	{0x12,0x1,"Versiya Ukraine, Customer site",0x1},
	{0x13,0x1,"Vastech SAR Angel project, first site",0x1},
	{0x14,0x1,"Vastech SAR Plaas project, Pretoria site",0x1},
	{0x15,0x1,"Vastech SAR Echo project, Demo site",0x1},
	{0x16,0x1,"SI India, Bhathinda site",0x1},
	{0x16,0x2,"SI India, Delhi site",0x1},
	{0x17,0x1,"Demo key",0x1},
	{0x18,0x1,"Unknown, site",0x0},
	{0x19,0x1,"Unknown, site",0x0},
	{0x1a,0x1,"Unknown, site",0x0},
	{0x1b,0x1,"Unknown, site",0x0},
	{0x1c,0x1,"Unknown, site",0x0},
	{0x1d,0x1,"Unknown, site",0x0},
	{0x1e,0x1,"Unknown, site",0x0},
	{0x1f,0x1,"Unknown, site",0x0},
	{0x20,0x1,"Vastech SAR Tefer project, first site",0x1},
	{0,0,"",0}
};

static SysError ErroList[]=
{
	{NoErrors,					""},
	{ErrUnknown,				"Unknown error"},
	{ErrUnknownSoft,			"Unknown software error"},
	{ErrUnknownNet,				"Unknown Network error"},
	{ErrUnknownHard,			"Unknown hardware error"},
	{ErrOpenFile,				"Open file error"},
	{ErrOpenSocket,				"Open Network socket error"},
	{ErrOpenShMem,				"Open shared memory error"},
	{ErrOpenPort,				"Open port error"},
	{ErrTimeOut,				"Timeout error"},
	{ErrDomainCntl,				"Domain Controller error"},
	{ErrProcSrv,				"Processing server error"},
	{ErrFileSrv,				"File Server error"},
	{ErrDB,						"DataBase error"},
	{ErrDBManager,				"DataBase Manager error"},
	{ErrSSSrv,					"Signaling system server error"},
	{ErrChannel,				"Channel error"},
	{ErrSetup,					"SetUp error"},
	{ErrParam,					"Parametr error"},
	{ErrDevUnavailable,			"Device Unavailable"},
	{ErrUnitUnavailable,		"Unit Unavailable"},
	{ErrFuncionUnavailable,		"Function Unavailable"},
	{ErrDomainCntlUnavailable,	"Domain controller Unavailable"},
	{ErrDomainState,			"Domain state error"},
	{ErrLock,					"Lock error"},
	{0}
};

static char	*TablName[80]=
{
	"TabNeuronGlobal",			//
	"TabNeuronLocalCustomer",	//
	"TabNeuronLocalSite",		//
//	"TabNeuronLocalDomain",		//
	"TabParametr",				//
	""
};
// 
// static char	*MessageTypeTable[MaxMessageType+1]=
// {
// 	"Simple",
// 	"Task",
// 	"Report",
// 	"Request",
// 	"Information",
// 	"Proposal",
// 	""
// };

char*	GetMessageType(int ID_MsgType)
{
	return MessageTypeTable[ID_MsgType];
};

Customer* GetCustomerInfo(int  CustomerID)
{
	Customer*	RC=NULL;
	USHORT	CID=(CustomerID>>16)&0xffff;
	USHORT	SID=CustomerID&0xffff;
	int sizeCL=1+sizeof(CustomerList)/sizeof(Customer);
	for (int i=0;i<sizeCL;i++)
	{
		if ((CustomerList[i].CID==CID)&&(CustomerList[i].SID==SID))
		{
			RC=&CustomerList[i];
			break;
		}
	};
	return RC;
};

SysError* GetSysErrorByID(UINT64 ID)
{
	if (ID>=MaxErrorNumber)
		return NULL;
	return &ErroList[ID];
};

char* GetErrorTxtByID(UINT64 ID)
{
	if (ID>=MaxErrorNumber)
		return NULL;
	return ErroList[ID].Name;
};


Parametr* GetTablParametrID(UINT64 ID)
{
	Parametr*	Prm=NULL;
	if ((ID<FinishPrm) && (ID>=FirstPrm))
	{
		if (ID<LastPrm)
			Prm=&TablParametrs[ID-FirstPrm];
		else
			Prm=&TablParametrs[ID-LastPrm+EndFirstPrm-FirstPrm];
	}
	return	Prm;
};

char*	GetTablName(int Number)
{
	if (Number==0) return 0;
	return TablName[Number-1];
};

int		GetIDTablName(char* TlName)
{
	int rc=-1;
	for (int i=0;i<10000;i++)
	{
		char*	TmpL=GetTablName(i+1);
		if (TmpL==0) break;
		if (strcmp(TlName,TmpL)==0)
		{
			rc=i+1; break;
		}
	};
	return rc;
};


