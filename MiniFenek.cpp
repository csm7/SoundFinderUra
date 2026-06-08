#include "pch.h"

#include "MiniFenek.h"

#include "UdpListener.hpp"
//UdpListener listener(5111);
//bool listener_first_start = true;
//UdpListener::ReceivedPacket packet;

const unsigned short CRC16_Table[] =
{
  0x0000, 0x1021, 0x2042, 0x3063, 0x4084, 0x50A5, 0x60C6, 0x70E7,
  0x8108, 0x9129, 0xA14A, 0xB16B, 0xC18C, 0xD1AD, 0xE1CE, 0xF1EF,
  0x1231, 0x0210, 0x3273, 0x2252, 0x52B5, 0x4294, 0x72F7, 0x62D6,
  0x9339, 0x8318, 0xB37B, 0xA35A, 0xD3BD, 0xC39C, 0xF3FF, 0xE3DE,
  0x2462, 0x3443, 0x0420, 0x1401, 0x64E6, 0x74C7, 0x44A4, 0x5485,
  0xA56A, 0xB54B, 0x8528, 0x9509, 0xE5EE, 0xF5CF, 0xC5AC, 0xD58D,
  0x3653, 0x2672, 0x1611, 0x0630, 0x76D7, 0x66F6, 0x5695, 0x46B4,
  0xB75B, 0xA77A, 0x9719, 0x8738, 0xF7DF, 0xE7FE, 0xD79D, 0xC7BC,
  0x48C4, 0x58E5, 0x6886, 0x78A7, 0x0840, 0x1861, 0x2802, 0x3823,
  0xC9CC, 0xD9ED, 0xE98E, 0xF9AF, 0x8948, 0x9969, 0xA90A, 0xB92B,
  0x5AF5, 0x4AD4, 0x7AB7, 0x6A96, 0x1A71, 0x0A50, 0x3A33, 0x2A12,
  0xDBFD, 0xCBDC, 0xFBBF, 0xEB9E, 0x9B79, 0x8B58, 0xBB3B, 0xAB1A,
  0x6CA6, 0x7C87, 0x4CE4, 0x5CC5, 0x2C22, 0x3C03, 0x0C60, 0x1C41,
  0xEDAE, 0xFD8F, 0xCDEC, 0xDDCD, 0xAD2A, 0xBD0B, 0x8D68, 0x9D49,
  0x7E97, 0x6EB6, 0x5ED5, 0x4EF4, 0x3E13, 0x2E32, 0x1E51, 0x0E70,
  0xFF9F, 0xEFBE, 0xDFDD, 0xCFFC, 0xBF1B, 0xAF3A, 0x9F59, 0x8F78,
  0x9188, 0x81A9, 0xB1CA, 0xA1EB, 0xD10C, 0xC12D, 0xF14E, 0xE16F,
  0x1080, 0x00A1, 0x30C2, 0x20E3, 0x5004, 0x4025, 0x7046, 0x6067,
  0x83B9, 0x9398, 0xA3FB, 0xB3DA, 0xC33D, 0xD31C, 0xE37F, 0xF35E,
  0x02B1, 0x1290, 0x22F3, 0x32D2, 0x4235, 0x5214, 0x6277, 0x7256,
  0xB5EA, 0xA5CB, 0x95A8, 0x8589, 0xF56E, 0xE54F, 0xD52C, 0xC50D,
  0x34E2, 0x24C3, 0x14A0, 0x0481, 0x7466, 0x6447, 0x5424, 0x4405,
  0xA7DB, 0xB7FA, 0x8799, 0x97B8, 0xE75F, 0xF77E, 0xC71D, 0xD73C,
  0x26D3, 0x36F2, 0x0691, 0x16B0, 0x6657, 0x7676, 0x4615, 0x5634,
  0xD94C, 0xC96D, 0xF90E, 0xE92F, 0x99C8, 0x89E9, 0xB98A, 0xA9AB,
  0x5844, 0x4865, 0x7806, 0x6827, 0x18C0, 0x08E1, 0x3882, 0x28A3,
  0xCB7D, 0xDB5C, 0xEB3F, 0xFB1E, 0x8BF9, 0x9BD8, 0xABBB, 0xBB9A,
  0x4A75, 0x5A54, 0x6A37, 0x7A16, 0x0AF1, 0x1AD0, 0x2AB3, 0x3A92,
  0xFD2E, 0xED0F, 0xDD6C, 0xCD4D, 0xBDAA, 0xAD8B, 0x9DE8, 0x8DC9,
  0x7C26, 0x6C07, 0x5C64, 0x4C45, 0x3CA2, 0x2C83, 0x1CE0, 0x0CC1,
  0xEF1F, 0xFF3E, 0xCF5D, 0xDF7C, 0xAF9B, 0xBFBA, 0x8FD9, 0x9FF8,
  0x6E17, 0x7E36, 0x4E55, 0x5E74, 0x2E93, 0x3EB2, 0x0ED1, 0x1EF0
};

unsigned short	CalcCRC16(unsigned char* Buf, int Len, unsigned short  OldResult)				// ïîäñ÷¸ò CRC16 ïîáàéòíî â crc_16 (CRC-16 CCITT Poly  : 0x1021)
{
	unsigned short	crc_16 = OldResult;
	while (Len--)
	{
		crc_16 = (crc_16 << 8) ^ CRC16_Table[(crc_16 >> 8) ^ *Buf++];
	}
	return	crc_16;
}
unsigned char	CalcCRC8(unsigned char* Buf, int Len)				// ïîäñ÷¸ò CRC16 ïîáàéòíî â crc_16 (CRC-16 CCITT Poly  : 0x1021)
{
	return 0xff & CalcCRC16(Buf, Len, 0xffff);
}


int	StrToBin(UCHAR* ReceivBuf, void* tBufF, uint8_t MaxLen)
{
	uint8_t* PoiI = (uint8_t*)ReceivBuf;
	uint8_t* PoiO = (uint8_t*)tBufF;
	uint8_t   TmpIndex = 0;
	if (*PoiI == 'D')
	{
		PoiI++;
		do
		{
			*PoiO = ((*PoiI - 0x30) << 4) + (*(PoiI + 1) - 0x30);
			PoiO++; TmpIndex++; PoiI = PoiI + 2;
			if (TmpIndex > MaxLen)
			{
				TmpIndex = 0;
				break;
			}
		} while (*PoiI > 0x0d);
	}
	return TmpIndex;
}

int	BinToStr(char* tSendBuf, char* BufF, int LenF)
{
	int i;
	int Index = 0;
	tSendBuf[Index++] = 'D';
	for (i = 0; i < LenF; i++)
	{
		tSendBuf[Index++] = ((BufF[i] & 0xf0) >> 4) + 0x30;
		tSendBuf[Index++] = (BufF[i] & 0x0f) + 0x30;
	}
	tSendBuf[Index++] = 0xd;
	tSendBuf[Index++] = 0xa;
	tSendBuf[Index] = 0;
	return Index;
};

int32_t Idle_Calc(void* poi)
{
	return 	((DevIntegrator*)poi)->IdleFunc();
}

#if 0

DevCAN::DevCAN(int8_t DevID, Services* Srv)
{
	this->Srv = Srv;
/*
	DElem = NULL;
	AXOut = AYOut = AZOut = GXOut = GYOut = GZOut = QXOut = QYOut = QZOut = 0;
	DevPort = 0;
	Fl_Event1 = 0;
*/
	DevInit(DevID);
};

void DevCAN::DevInit(uint8_t DevID)
{
	if (Fl_DevInit == 0)
	{
		int32_t i;
		Fl_Stop = 1;
		Dev_ID = DevID;
		Init((uchar_t*)&Dev_ID, sizeof(int8_t));
	}
}
void DevCAN::Del_ID_Elem(void)
{
	Stop();
/*
	disconnectFromServer();

	if (Fl_DevInit)
	{
		StopThreads();

		//      delete CPort_Thread;
		delete FFT1_Thread; FFT1_Thread = NULL;
		delete FFT5_Thread; FFT5_Thread = NULL;
		delete Syn1_5_Thread; Syn1_5_Thread = NULL;
		delete Event_Thread; Event_Thread = NULL;
		delete AKF1_Thread; AKF1_Thread = NULL;
		delete AKF5_Thread; AKF5_Thread = NULL;
		delete AKF1_5_Thread; AKF1_5_Thread = NULL;
		delete Wav_Thread; Wav_Thread = NULL;
		delete Calc1_Thread; Calc1_Thread = NULL;
		delete Calc5_Thread; Calc5_Thread = NULL;
		delete Calc1_5_Thread; Calc1_5_Thread = NULL;
		delete Analysis1_Thread; Analysis1_Thread = NULL;
		delete NetCom_Thread; NetCom_Thread = NULL;
		delete TestSoundSpeed_Thread; TestSoundSpeed_Thread = NULL;

		KFun1.clear();
		KFun5.clear();
		KFun15.clear();

		ISpe1Buf.clear();
		ISpe5Buf.clear();

		free(Spe_Si1);	free(Spe_Sq1); Spe_Si1 = Spe_Sq1 = NULL;
		free(Spe_Si2);	free(Spe_Sq2); Spe_Si2 = Spe_Sq2 = NULL;
		free(Spe_Si3);	free(Spe_Sq3); Spe_Si3 = Spe_Sq3 = NULL;
		free(Spe_Si4);	free(Spe_Sq4); Spe_Si4 = Spe_Sq4 = NULL;
		free(Spe_Si5);	free(Spe_Sq5); Spe_Si5 = Spe_Sq5 = NULL;
		free(Spe_Si6);	free(Spe_Sq6); Spe_Si6 = Spe_Sq6 = NULL;
		free(Spe_Si7);	free(Spe_Sq7); Spe_Si7 = Spe_Sq7 = NULL;
		free(Spe_Si8);	free(Spe_Sq8); Spe_Si8 = Spe_Sq8 = NULL;

		free(IBuf1); IBuf1 = NULL;
		free(IBuf2); IBuf2 = NULL;
		free(IBuf3); IBuf3 = NULL;
		free(IBuf4); IBuf4 = NULL;
		free(IBuf5); IBuf5 = NULL;
		free(IBuf6); IBuf6 = NULL;
		free(IBuf7); IBuf7 = NULL;
		free(IBuf8); IBuf8 = NULL;

		free(I_Buf1); I_Buf1 = NULL;
		free(I_Buf2); I_Buf2 = NULL;
		free(I_Buf3); I_Buf3 = NULL;
		free(I_Buf4); I_Buf4 = NULL;
		free(I_Buf5); I_Buf5 = NULL;
		free(I_Buf6); I_Buf6 = NULL;
		free(I_Buf7); I_Buf7 = NULL;
		free(I_Buf8); I_Buf8 = NULL;
		free(I_BufS); I_BufS = NULL;

		delete CSock; CSock = NULL;
		delete MirFiltr; MirFiltr = NULL;
		delete PDetector; PDetector = NULL;
		delete Cfg; Cfg = NULL;
	}
	// End DeInit
	Fl_DevInit = 0;
	Buff = NULL;
	Len = 0;
*/
};

DevCAN::~DevCAN()
{
	Del_ID_Elem();
}
#endif

DevElem::DevElem()
{
	Item = (int) -1;
	Ev_Azimuth = nan(0);
	TargetType = 0;
/*
	CntPack = 0;
	processState = ProcessState::Idle;
	PDevEl = nullptr;
	Fl_Active = 0;
	Fl_Mode = ProcessStartMode::Manual;
*/
}

void DevElem::Del_ID_Elem(void)
{
/*
	if (PDevEl)
	{
		//PDevEl->Stop();
		DevCAN* tPDevEl = PDevEl;
		PDevEl = nullptr;
		delete tPDevEl;
	}
*/
};

DevElem::~DevElem()
{
	Del_ID_Elem();
};

DevIntegrator::DevIntegrator(Services* pSrv)
{
	// Этот фрагмент должен быть вставлен во все последующие объекты
	// NullMt_ должен быть заменен на имя объекта
	Name_Mt = (char*) DevIntegrator_NameMt; Rem_Mt = (char*)DevIntegrator_RemMt; Ver_ = (char*)DevIntegrator_Version; Auth_ = (char*)DevIntegrator_Author;
	PinName_I = (char**)DevIntegrator_PinNameI; PinName_O = (char**)DevIntegrator_PinNameO; Prm_ = (char**)DevIntegrator_Prm; Vol_ = (char**)DevIntegrator_Vol;
	Type_Mt = (char*)DevIntegrator_Type;

	Num_PinI = 0; while (PinName_I[2 * Num_PinI][0] != 0x0) Num_PinI++;
	Num_PinO = 0; while (PinName_O[2 * Num_PinO][0] != 0x0) Num_PinO++;
	Num_Prm = 0; while (Prm_[2 * Num_Prm][0] != 0x0) Num_Prm++;
	// -------------------------------------------------------------
	Fl_Stop = 1; Fl_Data = 0;
	Srv = pSrv;
	CSock = NULL;
	Idel_Thread = new ClassThread("IdleFunc", Idle_Calc, (void*)this, THREAD_PRIORITY_NORMAL);     // THREAD_PRIORITY_TIME_CRITICAL
	Fl_UDPOld = Fl_UDPNew = 0;
	ElUDPHUBOld = NULL;
	ElUDPHUBNew = NULL;
	ElUDPStepper = NULL;
	Fl_DataStream = 0;
	Fl_SaveWav = 0;
	Fl_Debug = 0;
	PoiWrSB = 0;
	WavTest = NULL;

	for (int i = 0; i < 1024; i++)
	{
		SpeBuf[i] = 0.0;
	}

	CntCANError=0;
	CntCANOk=0;

	Wr_IndMsg = Rd_IndMsg = 0;
	Wr_OutMsg = Rd_OutMsg = 0;
	/*
		Fl_Stream = 0;
		ElUDPIntegrator = NULL;
		ElUDPCom = NULL;
		//	CntInPack = 0;
		//	CntOutPack = 0;
		UDPPack = NULL;
		ElUDPStream = NULL;
		ElUDPStream2 = NULL;
		// New
		activeElement = nullptr;
		Fl_InitDlg = 2;

		DmpUDP = new DamperPS();
		DmpUDP2 = new DamperPS();


	#ifdef _DEBUG	// Fake device to test client server connection without real devices
		DevElem* tDevEl = new DevElem();

		uchar_t	TmpCh;
		//memcpy(&tDevEl->Addr, pmas, sizeof(sockaddr));
		*(USHORT*)&tDevEl->Addr.sa_data[0] = 5001;
		TmpCh = tDevEl->Addr.sa_data[0];
		tDevEl->Addr.sa_data[0] = tDevEl->Addr.sa_data[1];
		tDevEl->Addr.sa_data[1] = TmpCh;

		tDevEl->IPAddr = 0;
		tDevEl->HW_ID = 0;
		tDevEl->Init((uchar_t*)&tDevEl->IPAddr, sizeof(ULONG));
		tDevEl->HW_Type = 0;
		tDevEl->Version = 169;
		strcpy(tDevEl->DevName, "FakeDev");
		DevList.Add(tDevEl);
	#endif // _DEBUG

		CMainCfg* M_Cfg = GetMainConfig();

		std::string purgerPath;
		SessionsPurger = std::make_unique<F_Purger>();
		purgerPath = std::string(M_Cfg->MData.Path) + "\\SessionWav\\";

		SessionsPurger->Param("PathPurg", purgerPath.c_str());
		SessionsPurger->Param("IgnoreTimeClk", "20000");					// 	"Dxx-значение в днях  Hxx-значение в часах ([>25%] [15%] [10%] [5%] [1%] [0%])",
		SessionsPurger->Param("TimeExt0", M_Cfg->MData.PurgerPrm.c_str());	//  "D7,D7,D7,D3,D1,H0"
		SessionsPurger->setPurgerFPS(M_Cfg->MData.purgerFPS);
		SessionsPurger->Start();

		EventsPurger = std::make_unique<F_Purger>();
		purgerPath = std::string(M_Cfg->MData.Path) + "\\EventWav\\";

		EventsPurger->Param("PathPurg", purgerPath.c_str());
		EventsPurger->Param("IgnoreTimeClk", "20000");					// 	"Dxx-значение в днях  Hxx-значение в часах ([>25%] [15%] [10%] [5%] [1%] [0%])",
		EventsPurger->Param("TimeExt0", M_Cfg->MData.PurgerPrm.c_str());	//  "D7,D7,D7,D3,D1,H0"
		EventsPurger->setPurgerFPS(M_Cfg->MData.purgerFPS);
		EventsPurger->Start();

		SSock = new SockSrv();
		SSock->Param("ServerPort", "4010");
		SSock->Param("NetType", "LAN");
		SSock->Param("Mode", "BufNoPack");
		SSock->Start();
*/
	};

	DevIntegrator::~DevIntegrator()
	{
/*
		if (SSock) {
			delete SSock;
		}
		SessionsPurger.reset(nullptr);
		EventsPurger.reset(nullptr);

		deviceProcessStarted.disconnect_all();
		deviceProcessDestroyed.disconnect_all();
		deviceUpdated.disconnect_all();
		deviceRemoved.disconnect_all();

		Idel_Thread->Stop();
		delete Idel_Thread;

		if (ElUDPStream)
			Srv->CloseUDPCilent(NT_PAN, 5003, this);
		if (ElUDPStream2)
			Srv->CloseUDPCilent(NT_PAN, 5009, this);

		if (ElUDPIntegrator)
			Srv->CloseUDPCilent(NT_PAN, 5002, this);
		if (ElUDPCom)
			Srv->CloseUDPCilent(NT_PAN, 5001, this);
		ElUDPIntegrator = NULL;
		ElUDPCom = NULL;
		ElUDPStream = NULL;
		ElUDPStream2 = NULL;
		delete DmpUDP;
		delete DmpUDP2;
*/
		if (ElUDPHUBOld)
			Srv->CloseUDPCilent(NT_LAN, UDPPortOld, this);
		ElUDPHUBOld = NULL;
		if (ElUDPHUBNew)
			Srv->CloseUDPCilent(NT_LAN, UDPPortNew, this);
		ElUDPHUBNew = NULL;
		if (ElUDPDebug)
			Srv->CloseUDPCilent(NT_LAN, UDPPortDebug, this);
		ElUDPDebug = NULL;

		
//		if (ElUDPStepper)
//			Srv->CloseUDPCilent(NT_LAN, UDPPortStepper, this);
		ElUDPStepper = NULL;

		Idel_Thread->Stop();
		delete Idel_Thread;

};

void	DevIntegrator::Param(const char* Prm, const char* Vol)
{
};

void	DevIntegrator::Start()
{
	if (Fl_Stop != 0)
	{
		if (ElUDPHUBOld)
			Srv->CloseUDPCilent(NT_LAN, UDPPortOld, this);
		ElUDPHUBOld = Srv->OpenUDPCilent(NT_LAN, UDPPortOld, this);

		if (ElUDPHUBNew)
			Srv->CloseUDPCilent(NT_LAN, UDPPortNew, this);
		ElUDPHUBNew = Srv->OpenUDPCilent(NT_LAN, UDPPortNew, this);

		if (ElUDPDebug)
			Srv->CloseUDPCilent(NT_LAN, UDPPortDebug, this);
		ElUDPDebug = Srv->OpenUDPCilent(NT_LAN, UDPPortDebug, this);

//		if (ElUDPStepper)
//			Srv->CloseUDPCilent(NT_LAN, UDPPortStepper, this);
//		ElUDPStepper = Srv->OpenUDPCilent(NT_LAN, UDPPortStepper, this);

		Wr_IndMsg = Rd_IndMsg = 0;
		Fl_Stop = 0;
	}
		/*
				int32_t		TmpHF;
				int32_t		LenF;
				DevConfig	LoadDCfg;
				int32_t		TmpItem = 0;
				PathNameUDPStationCfg = GetMainConfig()->MData.Path + "\\UDPStation.cfg";
				_sopen_s(&TmpHF, PathNameUDPStationCfg.c_str(), _O_RDWR | _O_BINARY, _SH_DENYNO, _S_IREAD | _S_IWRITE);
				if (TmpHF != -1)
				{
					do
					{
						LenF = _read(TmpHF, &LoadDCfg, sizeof(DevConfig));
						if (LenF == sizeof(DevConfig))
						{
							DevElem* tDevEl;
							tDevEl = (DevElem*)DevList.NewElem();
							if (tDevEl == NULL)
								tDevEl = new DevElem();

							uchar_t	TmpCh;
							tDevEl->IPAddr = LoadDCfg.IPAddr;
							tDevEl->Init((uchar_t*)&tDevEl->IPAddr, sizeof(ULONG));
							tDevEl->HW_ID = LoadDCfg.HW_ID;
							tDevEl->Fl_Mode = LoadDCfg.Fl_Mode;
							tDevEl->Fl_Active = 1;
							DevList.Add(tDevEl);
							memcpy(&tDevEl->Addr.sa_data[2], &tDevEl->IPAddr, sizeof(ULONG));
							tDevEl->Addr.sa_family = AF_INET;
							*(USHORT*)&tDevEl->Addr.sa_data[0] = 5001;
							TmpCh = tDevEl->Addr.sa_data[0];
							tDevEl->Addr.sa_data[0] = tDevEl->Addr.sa_data[1];
							tDevEl->Addr.sa_data[1] = TmpCh;

							deviceUpdated(tDevEl);
						}
					} while (LenF == sizeof(DevConfig));
					_close(TmpHF);
				}
				if (ElUDPIntegrator)
					Srv->CloseUDPCilent(NT_PAN, 5002, this);
				if (ElUDPCom)
					Srv->CloseUDPCilent(NT_PAN, 5001, this);
				if (ElUDPStream)
					Srv->CloseUDPCilent(NT_PAN, 5003, this);
				if (ElUDPStream2)
					Srv->CloseUDPCilent(NT_PAN, 5009, this);
				ElUDPIntegrator = Srv->OpenUDPCilent(NT_PAN, 5002, this);
				ElUDPCom = Srv->OpenUDPCilent(NT_PAN, 5001, this);
				ElUDPStream = Srv->OpenUDPCilent(NT_PAN, 5003, this);
				ElUDPStream2 = Srv->OpenUDPCilent(NT_PAN, 5009, this);
			}
			Fl_Stop = 0;
			//	CntInPack = 0;
			//	CntOutPack = 0;
			//	CntError = 0;
			//	Fl_Stream = 0;
		};
*/


	/*
		DevList.DelAllElem();
		if (ElUDPIntegrator)
		{
			Srv->CloseUDPCilent(NT_PAN, 5002, this);
		}
		if (ElUDPCom)
		{
			Srv->CloseUDPCilent(NT_PAN, 5001, this);
		}
		if (ElUDPStream)
		{
			Srv->CloseUDPCilent(NT_PAN, 5003, this);
		}
		if (ElUDPStream2)
		{
			Srv->CloseUDPCilent(NT_PAN, 5009, this);
		}
		ElUDPIntegrator = nullptr;
		ElUDPCom = nullptr;
		ElUDPStream = nullptr;
		ElUDPStream2 = nullptr;
	*/
};

void	DevIntegrator::Stop()
{

	int32_t		TmpHF;
	if (Fl_Stop == 0)
	{
/*
		unlink(PathNameUDPStationCfg.c_str());
		_sopen_s(&TmpHF, PathNameUDPStationCfg.c_str(), _O_CREAT | _O_RDWR | _O_BINARY, _SH_DENYNO, _S_IREAD | _S_IWRITE);
		if (TmpHF != -1)
		{
			DevElem* tDevEl;
			FindStrList8	FSTR;
			DevConfig		LoadDCfg;
			tDevEl = (DevElem*)DevList.Begin(FSTR);
			while (tDevEl)
			{
				//			if (tDevEl->Fl_Active)
				{
					LoadDCfg.IPAddr = tDevEl->IPAddr;
					LoadDCfg.HW_ID = tDevEl->HW_ID;
					LoadDCfg.Fl_Mode = tDevEl->Fl_Mode;
					_write(TmpHF, &LoadDCfg, sizeof(LoadDCfg));
				}

				deviceProcessDestroyed(tDevEl->HW_ID);

				tDevEl = (DevElem*)DevList.Next(FSTR);
			}
			_close(TmpHF);
		}
*/
		Fl_Stop = 1;
	}
	DevList.DelAllElem();
	if (ElUDPHUBOld)
	{
		Srv->CloseUDPCilent(NT_LAN, UDPPortOld, this);
	}
	ElUDPHUBOld = nullptr;
	if (ElUDPHUBNew)
	{
		Srv->CloseUDPCilent(NT_LAN, UDPPortNew, this);
	}
	ElUDPHUBNew = nullptr;
	if (ElUDPDebug)
	{
		Srv->CloseUDPCilent(NT_LAN, UDPPortDebug, this);
	}
	ElUDPDebug = nullptr;

//	if (ElUDPStepper)
//		Srv->CloseUDPCilent(NT_LAN, UDPPortStepper, this);
//	ElUDPStepper = nullptr;

/*
	if (ElUDPIntegrator)
	{
		Srv->CloseUDPCilent(NT_PAN, 5002, this);
	}
	if (ElUDPCom)
	{
		Srv->CloseUDPCilent(NT_PAN, 5001, this);
	}
	if (ElUDPStream)
	{
		Srv->CloseUDPCilent(NT_PAN, 5003, this);
	}
	if (ElUDPStream2)
	{
		Srv->CloseUDPCilent(NT_PAN, 5009, this);
	}
	ElUDPIntegrator = nullptr;
	ElUDPCom = nullptr;
	ElUDPStream = nullptr;
	ElUDPStream2 = nullptr;
*/
};

int32_t	DevIntegrator::DataFF(int32_t N_Pin, UCHAR* pmas, int32_t Cnt)
{
	int32_t	OCnt = 0;
	if (Fl_Stop != 1)
	{
	};
	return OCnt;
};

void	DevIntegrator::ParserMsg(CANMsgStr* pMsg)
{
	uint8_t	IDFrom= pMsg->dat[2];
	activeElement=SetActive(IDFrom);
	float	Vol = *(float*)&pMsg->dat[3];
	uint16_t	iVol1 = *(uint16_t*)&pMsg->dat[3];
	uint16_t	iVol2 = *(uint16_t*)&pMsg->dat[5];
	uint32_t	iVol32 = *(uint32_t*)&pMsg->dat[3];
	switch (pMsg->id)
	{
	case ToExtSrv:
	case ToHUB:
	case ToAll:
		switch (pMsg->dat[1])	// Command
		{
		case IAM_CLIENT:
			memcpy((void*)&activeElement->DInfo, &pMsg->dat[3], sizeof(DeviceInfo));
			break;
		case SEND_VERSION:
			activeElement->Version = iVol1;
			break;
		case SEND_Meta:
			activeElement->Azimuth = iVol1 / 10000.;
			activeElement->UgolM = iVol2 / 10000.;
			break;
		case SEND_EventMeta:
			activeElement->Ev_Azimuth = iVol1 / 10000.;
			activeElement->Ev_UgolM = iVol2 / 10000.;
			break;
		case SEND_EventTime:
			activeElement->Ev_Time=iVol32;
			break;
		case SEND_Compass:
			activeElement->Compass = Vol;
			break;
		case SEND_Level:
			activeElement->Level[0] = pMsg->dat[3];
			activeElement->Level[1] = pMsg->dat[4];
			activeElement->Level[2] = pMsg->dat[5];
			activeElement->Level[3] = pMsg->dat[6];
			break;
		case SEND_GPS_Lat:
			activeElement->Lat = Vol;
			break;
		case SEND_GPS_Lng:
			activeElement->Lng = Vol;
			break;
		case SEND_GPS_Alt:
			activeElement->Alt = Vol;
			break;
		case SEND_GPS_UTC:
			activeElement->UTC = Vol;
			break;
		case SEND_GPS_Stat:
			activeElement->GPS_State = pMsg->dat[3];
			break;
		case SEND_SNS:
			memcpy((void*)&activeElement->SNSStr, &pMsg->dat[3], 5);
			switch (activeElement->SNSStr.State)
			{
			case SNS_Begin:
				break;
			case SNS_End:
				break;
			}
			break;
		case SEND_TrgType:
			activeElement->TargetType = pMsg->dat[3];
			break;
		default:
			;
		}
		break;
	default:
		;
	}
}

void	DevIntegrator::SendMsg2CAN(uchar Command, uchar IDTo, uchar* Buf, int Len)
{
	MsgOut.id = IDTo;
	MsgOut.dat[1] = Command;
	MsgOut.dat[2] = ToExtSrv;	// From ExtSrv
	MsgOut.len = 3 + Len;
	if (Len)
		memcpy(&MsgOut.dat[3], Buf, Len);
	MsgOut.dat[0] = CalcCRC8(MsgOut.dat+1, MsgOut.len-1);
	MsgOut.format = 0;
	MsgOut.type = 0;
	SendToCANQueue(&MsgOut);
};

void	DevIntegrator::SendToCANQueue(CANMsgStr* Msg)
{
	memcpy(&MsgOutQueue[Wr_OutMsg], Msg, sizeof(CANMsgStr));
//	memset(&MsgOutQueue[Wr_OutMsg], 01, sizeof(CANMsgStr));
	Wr_OutMsg = (Wr_OutMsg + 1) % LenQueue;
};

void	DevIntegrator::AddToCANQueue(CANMsgStr* Msg)
{
	memcpy(&MsgInQueue[Wr_IndMsg],Msg,sizeof(CANMsgStr));
	Wr_IndMsg = (Wr_IndMsg + 1) % LenQueue;
};

int32_t DevIntegrator::listening_port_5111()
{
/*
	if (listener_first_start)
	{
		listener.setBlocking(true);
		listener_first_start = false;
	}
	if (listener.receive(packet))
	{
		uint32_t LenPack = packet.data.size();
		UCHAR* UDPPack = (UCHAR*)(packet.data.data());
		//if (LenPack > sizeof(CANMsgStr))
		//	__debugbreak();
		//while (LenPack >= sizeof(CANMsgStr))
		{
			Fl_UDPOld = 0; Fl_UDPNew = 1;
			memcpy(&AddrFrom, &packet.sender, sizeof(sockaddr));
			if (((CANMsgStr*)UDPPack)->dat[0] == CalcCRC8(((CANMsgStr*)UDPPack)->dat + 1, ((CANMsgStr*)UDPPack)->len - 1))
			{
				AddToCANQueue((CANMsgStr*)UDPPack); CntCANOk++;
			}
			else CntCANError++;
			LenPack -= sizeof(CANMsgStr); 
			UDPPack += sizeof(CANMsgStr);
		}
	}
*/
	return 0;
}

int32_t DevIntegrator::UserCallBackEx(int32_t Handle, int32_t N_Pin, UCHAR* pmas, size_t Cnt, int32_t FlagBuf)
{
	int32_t rc = 0;
	UCHAR*		UDPPack;
//		DevElem* tDevEl;
		//	uchar_t* GPSBuf;
/*
			//	MyLock(__FILE__, __LINE__, &FlSim, this);
	if (Handle == (int32_t)ElUDPStepper)
	{
	}
	else
	{
		if (Handle == (int32_t)ElUDPHUB)
		{
		}
	}
*/

		uint32_t UDPIP = *(uint32_t*)&AddrFrom.sa_data[2];
		if (Cnt > sizeof(sockaddr))
		{
			rc = 1;
			uint32_t LenPack = Cnt - sizeof(sockaddr);
			switch (N_Pin)
			{
			case UDPPortOld:
				memcpy(&AddrFrom, pmas, sizeof(sockaddr));
				UDPPack = (pmas + sizeof(sockaddr));
				while (LenPack >= 35)
				{
					if (UDPPack[0] == 'D')
					{
						Fl_UDPNew = 0;
						Fl_UDPOld = 1;
						StrToBin(UDPPack, &Msg, 35);
						if (Msg.dat[0] == CalcCRC8(Msg.dat + 1, Msg.len - 1))
						{
							AddToCANQueue(&Msg);
							CntCANOk++;
						}
						else
						{
							CntCANError++;
						}
					};
					LenPack -= 35;
					UDPPack += 35;
				};
				break;
			case UDPPortNew:
				memcpy(&AddrFrom, pmas, sizeof(sockaddr));
				UDPPack = (pmas + sizeof(sockaddr));
				while (LenPack >= sizeof(CANMsgStr))
				{
					Fl_UDPOld = 0;
					Fl_UDPNew = 1;
					if (((CANMsgStr*)UDPPack)->dat[0] == CalcCRC8(((CANMsgStr*)UDPPack)->dat + 1, ((CANMsgStr*)UDPPack)->len - 1))
					{
						AddToCANQueue((CANMsgStr*)UDPPack);
						CntCANOk++;
					}
					else
					{
						CntCANError++;
					}
					LenPack -= sizeof(CANMsgStr);
					UDPPack += sizeof(CANMsgStr);
				};
				break;
			case UDPPortDebug:
				UDPPack = (pmas + sizeof(sockaddr));
				for (int i = 0; i < 1024; i++)
				{
					SpeBuf[i] = 20+(int8_t)UDPPack[i];
				}
				break;

			}
		}
	
	//	MyUnLock(__FILE__, __LINE__, &FlSim, this);
	return rc;
}

char	CharTest = 0x30;

void	DevIntegrator::ParserPacketTCP(uchar* tBuf, int Len)
{
	int	RC;
	if (PoiWrSB)
	{
		memcpy(&SockBuf[PoiWrSB], tBuf, Len);
		Len += PoiWrSB;
		tBuf = SockBuf;
	}
	while (Len >= 35)
	{
		if (tBuf[0] == 'D')
		{
			RC=StrToBin(tBuf, &Msg, 35);
			if (RC == sizeof(Msg))
				AddToCANQueue(&Msg);
			else
				break;
		}
		else
		{
			break;
		}
		Len -= 35;
		tBuf += 35;
	};
	if (Len < 35)
	{
		memcpy(SockBuf, tBuf, Len);
		PoiWrSB = Len;
	}
};


int32_t	 DevIntegrator::IdleFunc()
{
	int32_t rc = 0;
	FileTime	tFTime;
	UCHAR		TCPPackBuf[14000];
	UCHAR		UDPPackBufOld[1400];
	UCHAR		UDPPackBufNew[1400];
	int		SizeOutUDPBufOld = 0;
	int		SizeOutUDPBufNew = 0;
	int		SizeOutTCPBuf = 0;
	int		RLen;
	Srv->SData->CntRun++;
	if (Fl_Stop)
	{
		return 0;
	}
//	listening_port_5111(); //2025!

	Srv->GetRealTime(&tFTime.FTime);
	if (Rd_IndMsg != Wr_IndMsg)
	{
		ParserMsg(&MsgInQueue[Rd_IndMsg]);
		Rd_IndMsg = (Rd_IndMsg + 1) % LenQueue;
		rc = 1;
	}
	while (Rd_OutMsg != Wr_OutMsg)
	{
		if (Fl_UDPOld)
		{
			RLen = BinToStr((char*)&UDPPackBufOld[SizeOutUDPBufOld], (char*)&MsgOutQueue[Rd_OutMsg], sizeof(CANMsgStr));
			SizeOutUDPBufOld += RLen;
		}
		if (Fl_UDPNew)
		{
			memcpy((char*)&UDPPackBufNew[SizeOutUDPBufNew], (char*)&MsgOutQueue[Rd_OutMsg], sizeof(CANMsgStr));
			SizeOutUDPBufNew += sizeof(CANMsgStr);
		}

		Rd_OutMsg = (Rd_OutMsg + 1) % LenQueue;
		if (SizeOutUDPBufOld >= 1400 - 35)
			break;
		if (SizeOutUDPBufNew >= 1400 - 35)
			break;
		if (SizeOutTCPBuf >= 14000 - 35)
			break;
		rc = 1;
	}
	if (SizeOutTCPBuf)
	{
		if (Fl_CliConnect==1)
			CSock->Data(0,TCPPackBuf, SizeOutTCPBuf);
	}
	if (SizeOutUDPBufOld)
	{
		ElUDPHUBOld->SendTo(UDPPackBufOld, SizeOutUDPBufOld, &AddrFrom);
	}
	if (SizeOutUDPBufNew)
	{
		ElUDPHUBNew->SendTo(UDPPackBufNew, SizeOutUDPBufNew, &AddrFrom);
//		listener.sendTo(packet, UDPPackBufNew, SizeOutUDPBufNew);
	}

	if (Fl_SaveWav)
	{
		if (WavTest == NULL)
		{
			fmt_	fm;
			WavTest = new FileWAV_VeryFast(10 * 1024);
			WavTest->SetFormatMono();
			WavTest->SetFrequency(22050);
			WavTest->GetFmt(&fm);
			fm.fmt_format = 3;	// IEEE
			fm.fmt_bits = 32;	// 32IEEE
			WavTest->SetFmt(&fm);
			WavTest->ReCalcFmt();
			WavTest->Create4("TestWav", ".WAV", 1);
		}
	}
	else
	{
		if (WavTest)
		{
			WavTest->Close();
			delete WavTest; WavTest = NULL;
		}
	}

	//size_t	Len;
	//int		RC;
	//if (CSock)
	//{
	//	if (CSock->Data(1, tBuf, 1))
	//	{
	//		for (uint32_t i = 0; i < CSock->Num_PinO; i++)
	//		{
	//			RC = CSock->OutData(i, tBuf, Len);
	//			if (RC)
	//			{
	//				rc = 1;
	//				switch (i)
	//				{
	//				case 0:
	//					switch (((EventMsg*)tBuf)->Event)
	//					{
	//					case 0:
	//						Fl_CliConnect = 1;
	//						//						ASPack.PHeader.Command = St_Send_Request;
	//						//						ASPack.PHeader.DataLen = 0;
	//						//						CSock->Data(0, (uchar_t*)&ASPack, sizeof(PackHeader));
	//						break;
	//					default:
	//						Fl_CliConnect = 0;
	//					}

	//					break;
	//				case 1:
	//					if (Fl_CliConnect)
	//						ParserPacketTCP(tBuf, Len);
	//					break;
	//				}
	//			}
	//		}
	//	}
	//}

	return rc;
}

DevElem* DevIntegrator::SetActive(uint8_t itemID)
{
	FindStrList8	FSL;
	DevElem* tDevEl = NULL;
	activeElement = NULL;
	tDevEl = (DevElem*)DevList.Begin(FSL);
	while (tDevEl)
	{
		if (tDevEl->HW_ID == itemID)
		{
			activeElement = tDevEl;
			break;
		}
		else
		{
			// Error !!!
		}

		tDevEl = (DevElem*)DevList.Next(FSL);
	};
	if (activeElement == NULL)
	{
		tDevEl = new DevElem();
		tDevEl->HW_ID = itemID;
		tDevEl->Init((uint8_t*)&tDevEl->HW_ID, sizeof(uint8_t));
//		tDevEl->Item = (int) -1;
		DevList.Add(tDevEl);
		activeElement = tDevEl;
	}
	return activeElement;
}

DevElem* DevIntegrator::GetActive()
{
	return activeElement;
}
void DevIntegrator::SetAction(int8_t Action, int8_t State)
{
/*
	if (activeElement)
	{
		switch (Action)
		{
		case 1:	// Reset
			Pack.PHeader.Command = FC_Reset;
			Pack.PHeader.DataLen = 0;
			if (activeElement->PDevEl)
				activeElement->PDevEl->CntInPack++;
			SendPack(&Pack, &activeElement->Addr);
			break;
		case 3:	// On/Off Proc
			switch (State)
			{
			case 0:
				if ((activeElement->PDevEl) && (activeElement->processState == ProcessState::Started))
				{
					activeElement->processState = ProcessState::GoingToStop;
				}
				break;
			case 1:
				if ((activeElement->PDevEl == nullptr) && (activeElement->processState == ProcessState::Idle))
				{
					activeElement->processState = ProcessState::GoingToStart;
				}
				break;
			}
			break;
		}
	}
*/
};