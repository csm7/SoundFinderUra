
// SoundServerDlg.cpp: файл реализации
//


#include "pch.h"
#include "framework.h"
#include "SoundServer.h"
#include "SoundServerDlg.h"
#include "afxdialogex.h"

extern DevIntegrator* DInt;

//#include "WebSocket.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

char	*WSBuffer;         // 
int		LenWSBuffer;

//extern	SockSrv	*SrvSockLAN;
//extern	SockSrv	*SrvSockWAN;
//extern	SockSrv	*SrvSockWEBL;
//extern	SockSrv	*SrvSockWEBW;
extern	SockCli	*CSock;

const	double	pi = 3.14159265358979323846;//3.1415926536;
static float	Pi = 3.1415926536f;
static float	Pi2 = 2 * 3.1415926536f;
static float	Pi_2 = 0.5f * 3.1415926536f;
static float	Pi_4 = 0.25f * 3.1415926536f;
static float	_Pi = 1.f / 3.1415926536f;
static float	_Pi_2 =2.f / 3.1415926536f;

/*
int	CSoundServerDlg::Boost_Idle(void)
{
	WebSocketMain(inet_ntoa(*(in_addr*)&MCfg->MData.IP_WAN), UCfg->StCfg.IP_WEBSockPort);
	return 0;
};

//------------------------------------------------------------------------------
// End Boost Objects

int Boost_Calc(LPVOID poi)
{
	((CSoundServerDlg*)poi)->Boost_Idle();
	return 0;
};
*/
/*
int SrvSockWEBL_Calc(LPVOID poi)
{
	((CSoundServerDlg*)poi)->SrvSockWEBL_Idle();
	return 0;
};

int SrvSockWEBW_Calc(LPVOID poi)
{
	((CSoundServerDlg*)poi)->SrvSockWEBW_Idle();
	return 0;
};
*/

int SrvSockLAN_Calc(LPVOID poi)
{
	((CSoundServerDlg*)poi)->SrvSockLAN_Idle();
	return 0;
};


/*
int SrvSockWAN_Calc(LPVOID poi)
{
	((CSoundServerDlg*)poi)->SrvSockWAN_Idle();
	return 0;
};
*/
IPElem::IPElem()
{
	CliIP[0] = 0;
	CliName[0] = 0;
	NumCli = -1; Fl_Connect = -1;
};

IPElem::~IPElem()
{
	NumCli = -1; Fl_Connect = 0;
};

void	IPElem::InitIPEl(int NCli)
{
	NumCli = NCli;
	CliIP[0]=0;
	CliName[0]=0;
	Fl_Connect = 0;
	Init((UCHAR*)&NumCli, (size_t)sizeof(int));
};


StElem::StElem()
{
	NumCli = -1;
	Item = -1; Fl_Active = 0;
};

void	StElem::InitStEl(AudioLocatorSystemInfo* ASInfo)
{
	memcpy(&ALSInfo, ASInfo, sizeof(AudioLocatorSystemInfo));
	Init((UCHAR*)&ALSInfo.StationID, (size_t)sizeof(UINT64));
};

StElem::~StElem()
{
};


// Диалоговое окно CAboutDlg используется для описания сведений о приложении

class CAboutDlg : public CDialogEx
{
public:
	CAboutDlg();

// Данные диалогового окна
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_ABOUTBOX };
#endif

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // поддержка DDX/DDV

// Реализация
protected:
	DECLARE_MESSAGE_MAP()
};

CAboutDlg::CAboutDlg() : CDialogEx(IDD_ABOUTBOX)
{
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialogEx)
END_MESSAGE_MAP()


// Диалоговое окно CSoundServerDlg

CUserCfg::CUserCfg(CMainCfg* MCfg)
{
	sprintf(PathNameUserCfg, "%s\\StationCtl.cfg", MCfg->MData.Path);
	Load();
}

CUserCfg::~CUserCfg()
{
	Save();
}

int CUserCfg::Load()
{
	int		rc = 0;
	int		n, m, i;
	//	UCHAR	i;
	_sopen_s(&HF, PathNameUserCfg, _O_RDWR | _O_BINARY, _SH_DENYNO, _S_IREAD | _S_IWRITE);
	if (HF != -1)
	{
		LenF = _read(HF, &StCfg, sizeof(StCfg));
		if (LenF != sizeof(StCfg))
		{

		};
		_close(HF);
	}
	else
	{
//		strcpy(StCfg.IP_Address, "192.168.100.30");
//		strcpy(StCfg.IP_Port, "4080");
		Save();
	};
	return rc;
};

int CUserCfg::Save()
{
	int rc = 0;
	_sopen_s(&HF, PathNameUserCfg, _O_CREAT | _O_RDWR | _O_BINARY, _SH_DENYNO, _S_IREAD | _S_IWRITE);
	if (HF != -1)
	{
		LenF = _write(HF, &StCfg, sizeof(StCfg));
		_close(HF);
	}
	return rc;
};

CSoundServerDlg::CSoundServerDlg(CWnd* pParent /*=nullptr*/)
	: CDialogEx(IDD_SOUNDSERVER_DIALOG, pParent)
{
	MaxItem = 0;
	HtmlStr[0]=0;
	WSBuffer = HtmlStr;
	Fl_Ready = 0;
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
	Fl_UpDataAppLAN = Fl_UpDataSmStLAN = Fl_UpDataFWLAN = 0;
	ActiveDevEl = 0;
	AppUpDateHendleLAN = SmStUpDateHendleLAN = FWUpDateHendleLAN = 0;
	Fl_UpDataAppWAN = Fl_UpDataSmStWAN = Fl_UpDataFWWAN = 0;
//	ActiveStElWAN = 0;
	AppUpDateHendleWAN = SmStUpDateHendleWAN = FWUpDateHendleWAN = 0;


	sprintf(PathNameLANStationCfg, "%s\\LANStation.cfg", MCfg->MData.Path);
	sprintf(PathNameWANStationCfg, "%s\\WLANStation.cfg", MCfg->MData.Path);
//	flTest = 0;
	WEBLNumPack=0;
	WEBWNumPack=0;
	Fl_UPD = 0; ModeWork=0;
	Terminator_State = 0;
}

CSoundServerDlg::~CSoundServerDlg()
{

	int		TmpHF;



	UCfg->Save();
	MCfg->SaveCfg();
//	MyUnLock(__FILE__, __LINE__, &Boost_Thread->Fl_Th, NULL);
//	delete	Boost_Thread;
//	delete	SrvSockWEBL_Thread;
//	delete	SrvSockWEBW_Thread;
//	delete	SrvSockWAN_Thread;
	delete	SrvSockLAN_Thread;
	delete	UCfg;
/*
	UINT64	TmpID;
	StElem		*StElF;
	FindStrList8	FSTR;

	unlink(PathNameLANStationCfg);
	_sopen_s(&TmpHF, PathNameLANStationCfg, _O_CREAT | _O_RDWR | _O_BINARY, _SH_DENYNO, _S_IREAD | _S_IWRITE);
	if (TmpHF != -1)
	{
		StElF = (StElem*)LANStList.Begin(FSTR);
		while (StElF)
		{
			_write(TmpHF, &StElF->ALSInfo.StationID, sizeof(UINT64));
			StElF = (StElem*)LANStList.Next(FSTR);
		}
		_close(TmpHF);
	}
	unlink(PathNameWANStationCfg);
	_sopen_s(&TmpHF, PathNameWANStationCfg, _O_CREAT | _O_RDWR | _O_BINARY, _SH_DENYNO, _S_IREAD | _S_IWRITE);
	if (TmpHF != -1)
	{
		StElF = (StElem*)WANStList.Begin(FSTR);
		while (StElF)
		{
			_write(TmpHF, &StElF->ALSInfo.StationID, sizeof(UINT64));
			StElF = (StElem*)WANStList.Next(FSTR);
		}
		_close(TmpHF);
	}
*/
}

void CSoundServerDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CSoundServerDlg, CDialogEx)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_NOTIFY(LVN_ITEMCHANGED, IDC_LIST1, &CSoundServerDlg::OnLvnItemchangedList1)
	ON_EN_CHANGE(IDC_MFCEDITBROWSE1, &CSoundServerDlg::OnEnChangeMfceditbrowse1)
	ON_EN_CHANGE(IDC_MFCEDITBROWSE2, &CSoundServerDlg::OnEnChangeMfceditbrowse2)
	ON_EN_CHANGE(IDC_MFCEDITBROWSE3, &CSoundServerDlg::OnEnChangeMfceditbrowse3)
	ON_EN_CHANGE(IDC_EDIT2, &CSoundServerDlg::OnEnChangeEdit2)
	ON_EN_CHANGE(IDC_EDIT1, &CSoundServerDlg::OnEnChangeEdit1)
	ON_BN_CLICKED(IDCANCEL, &CSoundServerDlg::OnBnClickedCancel)
	ON_WM_TIMER()
	ON_EN_CHANGE(IDC_EDIT3, &CSoundServerDlg::OnEnChangeEdit3)
	ON_EN_CHANGE(IDC_EDIT4, &CSoundServerDlg::OnEnChangeEdit4)
	ON_BN_CLICKED(IDC_BUTTON1, &CSoundServerDlg::OnBnClickedButton1)
	ON_BN_CLICKED(IDC_BUTTON2, &CSoundServerDlg::OnBnClickedButton2)
	ON_EN_CHANGE(IDC_EDIT5, &CSoundServerDlg::OnEnChangeEdit5)
	ON_BN_CLICKED(IDC_BUTTON3, &CSoundServerDlg::OnBnClickedButton3)
	ON_EN_CHANGE(IDC_EDIT6, &CSoundServerDlg::OnEnChangeEdit6)
//	ON_NOTIFY(LVN_ITEMCHANGED, IDC_LIST2, &CSoundServerDlg::OnLvnItemchangedList2)
	ON_EN_CHANGE(IDC_EDIT7, &CSoundServerDlg::OnEnChangeEdit7)
	ON_BN_CLICKED(IDC_BUTTON6, &CSoundServerDlg::OnBnClickedButton6)
	ON_BN_CLICKED(IDC_BUTTON4, &CSoundServerDlg::OnBnClickedButton4)
	ON_BN_CLICKED(IDC_BUTTON5, &CSoundServerDlg::OnBnClickedButton5)
	ON_EN_CHANGE(IDC_EDIT8, &CSoundServerDlg::OnEnChangeEdit8)
	ON_BN_CLICKED(IDC_CHECK1, &CSoundServerDlg::OnBnClickedCheck1)
	ON_BN_CLICKED(IDC_CHECK2, &CSoundServerDlg::OnBnClickedCheck2)
	ON_EN_CHANGE(IDC_EDIT11, &CSoundServerDlg::OnEnChangeEdit11)
	ON_NOTIFY(LVN_ITEMACTIVATE, IDC_LIST1, &CSoundServerDlg::OnItemactivateList1)
	ON_BN_CLICKED(IDC_BUTTON8, &CSoundServerDlg::OnBnClickedButton8)
	ON_BN_CLICKED(IDC_CHECK5, &CSoundServerDlg::OnBnClickedCheck5)
	ON_BN_CLICKED(IDC_CHECK4, &CSoundServerDlg::OnBnClickedCheck4)
	ON_BN_CLICKED(IDC_BUTTON7, &CSoundServerDlg::OnBnClickedButton7)
	ON_BN_CLICKED(IDC_BUTTON9, &CSoundServerDlg::OnBnClickedButton9)
	ON_BN_CLICKED(IDC_BUTTON10, &CSoundServerDlg::OnBnClickedButton10)
	ON_BN_CLICKED(IDC_CHECK3, &CSoundServerDlg::OnBnClickedCheck3)
	ON_BN_CLICKED(IDC_BUTTON11, &CSoundServerDlg::OnBnClickedButton11)
	ON_BN_CLICKED(IDC_BUTTON12, &CSoundServerDlg::OnBnClickedButton12)
	ON_BN_CLICKED(IDC_BUTTON13, &CSoundServerDlg::OnBnClickedButton13)
	ON_BN_CLICKED(IDC_BUTTON14, &CSoundServerDlg::OnBnClickedButton14)
	ON_BN_CLICKED(IDC_BUTTON15, &CSoundServerDlg::OnBnClickedButton15)
	ON_BN_CLICKED(IDC_BUTTON17, &CSoundServerDlg::OnBnClickedButton17)
	ON_BN_CLICKED(IDC_BUTTON18, &CSoundServerDlg::OnBnClickedButton18)
	ON_BN_CLICKED(IDC_CHECK9, &CSoundServerDlg::OnBnClickedCheck9)
	ON_BN_CLICKED(IDC_CHECK10, &CSoundServerDlg::OnBnClickedCheck10)
	ON_BN_CLICKED(IDC_BUTTON19, &CSoundServerDlg::OnBnClickedButton19)
	ON_BN_CLICKED(IDC_CHECK6, &CSoundServerDlg::OnBnClickedCheck6)
	ON_BN_CLICKED(IDC_CHECK7, &CSoundServerDlg::OnBnClickedCheck7)
	ON_BN_CLICKED(IDC_CHECK8, &CSoundServerDlg::OnBnClickedCheck8)
	ON_BN_CLICKED(IDC_BUTTON20, &CSoundServerDlg::OnBnClickedButton20)
	ON_CBN_SELCHANGE(IDC_COMBO1, &CSoundServerDlg::OnCbnSelchangeCombo1)
	ON_BN_CLICKED(IDC_CHECK11, &CSoundServerDlg::OnBnClickedCheck11)
	ON_BN_CLICKED(IDC_CHECK12, &CSoundServerDlg::OnBnClickedCheck12)
	ON_BN_CLICKED(IDC_BUTTON21, &CSoundServerDlg::OnBnClickedButton21)
	ON_BN_CLICKED(IDC_BUTTON22, &CSoundServerDlg::OnBnClickedButton22)
	ON_BN_CLICKED(IDC_BUTTON16, &CSoundServerDlg::OnBnClickedButton16)
END_MESSAGE_MAP()


// Обработчики сообщений CSoundServerDlg

BOOL CSoundServerDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// Добавление пункта "О программе..." в системное меню.

	// IDM_ABOUTBOX должен быть в пределах системной команды.
	ASSERT((IDM_ABOUTBOX & 0xFFF0) == IDM_ABOUTBOX);
	ASSERT(IDM_ABOUTBOX < 0xF000);

	UCfg = new CUserCfg(MCfg);
	DataCnt = 0;

	CMenu* pSysMenu = GetSystemMenu(FALSE);
	if (pSysMenu != nullptr)
	{
		BOOL bNameValid;
		CString strAboutMenu;
		bNameValid = strAboutMenu.LoadString(IDS_ABOUTBOX);
		ASSERT(bNameValid);
		if (!strAboutMenu.IsEmpty())
		{
			pSysMenu->AppendMenu(MF_SEPARATOR);
			pSysMenu->AppendMenu(MF_STRING, IDM_ABOUTBOX, strAboutMenu);
		}
	}

	// Задает значок для этого диалогового окна.  Среда делает это автоматически,
	//  если главное окно приложения не является диалоговым
	SetIcon(m_hIcon, TRUE);			// Крупный значок
	SetIcon(m_hIcon, FALSE);		// Мелкий значок

	// TODO: добавьте дополнительную инициализацию
	CWnd		*win;
	CString		s;
	char		SendBuf[1024];
	CRect		rect;
	CComboBox*	pCombo;

	pCombo = (CComboBox*)GetDlgItem(IDC_COMBO1);
	s.Format(_T("AP"));
	pCombo->InsertString(0, s);
	s.Format(_T("APSTA"));
	pCombo->InsertString(1, s);
	s.Format(_T("STA"));
	pCombo->InsertString(2, s);
	pCombo->SetCurSel(0);
	pCombo = (CComboBox*)GetDlgItem(IDC_COMBO2);
	s.Format(_T("Normal"));
	pCombo->InsertString(0, s);
	s.Format(_T("Extended"));
	pCombo->InsertString(1, s);
	s.Format(_T("Debug"));
	pCombo->InsertString(2, s);
	pCombo->SetCurSel(0);
	pCombo = (CComboBox*)GetDlgItem(IDC_COMBO3);
	s.Format(_T("CAN_5KBPS"));
	pCombo->InsertString(0, s);
	s.Format(_T("CAN_10KBPS"));
	pCombo->InsertString(1, s);
	s.Format(_T("CAN_20KBPS"));
	pCombo->InsertString(2, s);
	s.Format(_T("CAN_31K25BPS"));
	pCombo->InsertString(3, s);
	s.Format(_T("CAN_33KBPS"));
	pCombo->InsertString(4, s);
	s.Format(_T("CAN_40KBPS"));
	pCombo->InsertString(5, s);
	s.Format(_T("CAN_50KBPS"));
	pCombo->InsertString(6, s);
	s.Format(_T("CAN_80KBPS"));
	pCombo->InsertString(7, s);
	s.Format(_T("CAN_83KBPS"));
	pCombo->InsertString(8, s);
	s.Format(_T("CAN_95KBPS"));
	pCombo->InsertString(9, s);
	s.Format(_T("CAN_100KBPS"));
	pCombo->InsertString(10, s);
	s.Format(_T("CAN_125KBPS"));
	pCombo->InsertString(11, s);
	s.Format(_T("CAN_200KBPS"));
	pCombo->InsertString(12, s);
	s.Format(_T("CAN_250KBPS"));
	pCombo->InsertString(13, s);
	s.Format(_T("CAN_500KBPS"));
	pCombo->InsertString(14, s);
	s.Format(_T("CAN_1000KBPS"));
	pCombo->InsertString(15, s);
	pCombo->SetCurSel(10);


	GetClientRect(rect);

/*
	in_addr InAddr;
	InAddr.S_un.S_un_b.s_b1 = 192;
	InAddr.S_un.S_un_b.s_b2 = 168;
	InAddr.S_un.S_un_b.s_b3 = 100;
	InAddr.S_un.S_un_b.s_b4 = 79;
	set_address(inet_ntoa(InAddr), "5111", &AddrToStepper, "udp");
*/
	bzero(&AddrToStepper, sizeof(AddrToStepper));

	AddrToStepper.sin_family = AF_INET;
	AddrToStepper.sin_addr.S_un.S_addr = UCfg->StCfg.IP_Stepper;
	/*
		AddrToStepper.sin_addr.S_un.S_un_b.s_b1 = 192;
		AddrToStepper.sin_addr.S_un.S_un_b.s_b2 = 168;
		AddrToStepper.sin_addr.S_un.S_un_b.s_b3 = 100;
		AddrToStepper.sin_addr.S_un.S_un_b.s_b4 = 79;
	*/
	AddrToStepper.sin_port = 0xf713;

	m_pMySV0 = new GraficM(GRM_STAR);
	RectTxt = m_pMySV0->CreateGrafic(IDC_STATIC0, this);
	m_pMySV0->SetDataMode(0); // 0 - Mono 1 - Complex
	//	m_pMySV0->SetShkala(2048,2048,2048);
	//	m_pMySV0->SetDelta(128,128,0);
	m_pMySV0->InitData();

	m_pMySV1 = new GraficM(GRM_SPECTR);
	RectTxt = m_pMySV1->CreateGrafic(IDC_STATIC1, this);
	m_pMySV1->SetDataMode(0); // 0 - Mono 1 - Complex
	m_pMySV1->SetParamX(0, 1024);
	m_pMySV1->SetParamY(0, 100);
	m_pMySV1->InitData();


	m_ListUDP = (CListCtrl*)GetDlgItem(IDC_LIST1);
	m_ListUDP->InsertColumn(0, _T("DevID"), LVCFMT_LEFT, 120);
	m_ListUDP->InsertColumn(1, _T("Atimuth"), LVCFMT_LEFT, 70);
	m_ListUDP->InsertColumn(2, _T("Elevation"), LVCFMT_LEFT, 70);
	m_ListUDP->InsertColumn(3, _T("Power"), LVCFMT_LEFT, 90);
	m_ListUDP->InsertColumn(4, _T("Compass"), LVCFMT_LEFT, 70);
	m_ListUDP->InsertColumn(5, _T("Ev_Atimuth"), LVCFMT_LEFT, 70);
	m_ListUDP->InsertColumn(6, _T("Ev_Elevation"), LVCFMT_LEFT, 70);
	m_ListUDP->InsertColumn(7, _T("Ev_Time"), LVCFMT_LEFT, 70);
	m_ListUDP->InsertColumn(8, _T("UTC"), LVCFMT_LEFT, 80);
	m_ListUDP->InsertColumn(9, _T("Lat"), LVCFMT_LEFT, 80);
	m_ListUDP->InsertColumn(10, _T("Lng"), LVCFMT_LEFT, 80);
	m_ListUDP->InsertColumn(11, _T("Alt"), LVCFMT_LEFT, 80);
	m_ListUDP->InsertColumn(12, _T("GPSStatus"), LVCFMT_LEFT, 70);
	m_ListUDP->InsertColumn(13, _T("DataCnt"), LVCFMT_LEFT, 70);
	//	m_List->Create(LVS_ICON | LVS_REPORT | WS_VISIBLE, rect, this, 100);
/*	m_ListLAN->InsertColumn(1, _T("Station IP"), LVCFMT_LEFT, 90);
	m_ListLAN->InsertColumn(2, _T("Station Name"), LVCFMT_LEFT, 100);
	m_ListLAN->InsertColumn(3, _T("Ver App"), LVCFMT_LEFT, 40);
	m_ListLAN->InsertColumn(4, _T("Ver SSt"), LVCFMT_LEFT, 40);
	m_ListLAN->InsertColumn(5, _T("Ver FW"), LVCFMT_LEFT, 40);
	m_ListLAN->InsertColumn(6, _T("UpLoad"), LVCFMT_LEFT, 50);
	m_ListLAN->InsertColumn(7, _T("CPULoad"), LVCFMT_LEFT, 55);
	m_ListLAN->InsertColumn(9, _T("Active"), LVCFMT_LEFT, 60);
	m_ListLAN->InsertColumn(10, _T("Microphone"), LVCFMT_LEFT, 60);
	m_ListLAN->InsertColumn(11, _T("Strem"), LVCFMT_LEFT, 40);
	m_ListLAN->InsertColumn(12, _T("Calc"), LVCFMT_LEFT, 40);
	m_ListLAN->InsertColumn(13, _T("Session"), LVCFMT_LEFT, 40);
	m_ListLAN->InsertColumn(17, _T("BegTime"), LVCFMT_LEFT, 60);
	m_ListLAN->InsertColumn(18, _T("OperTime"), LVCFMT_LEFT, 60);
	m_ListLAN->InsertColumn(19, _T("NetCli"), LVCFMT_LEFT, 60);
	m_ListLAN->InsertColumn(20, _T("TypeCli"), LVCFMT_LEFT, 60);
*/
/*
	m_ListWAN = (CListCtrl*)GetDlgItem(IDC_LIST2);
	//	m_List->Create(LVS_ICON | LVS_REPORT | WS_VISIBLE, rect, this, 100);
	m_ListWAN->InsertColumn(0, _T("Station ID"), LVCFMT_LEFT, 60);
	m_ListWAN->InsertColumn(1, _T("Station IP"), LVCFMT_LEFT, 90);
	m_ListWAN->InsertColumn(2, _T("Station Name"), LVCFMT_LEFT, 100);
	m_ListWAN->InsertColumn(3, _T("Ver App"), LVCFMT_LEFT, 40);
	m_ListWAN->InsertColumn(4, _T("Ver SSt"), LVCFMT_LEFT, 40);
	m_ListWAN->InsertColumn(5, _T("Ver FW"), LVCFMT_LEFT, 40);
	m_ListWAN->InsertColumn(6, _T("UpLoad"), LVCFMT_LEFT, 50);
	m_ListWAN->InsertColumn(7, _T("CPULoad"), LVCFMT_LEFT, 55);
	m_ListWAN->InsertColumn(8, _T("Status"), LVCFMT_LEFT, 40);
	m_ListWAN->InsertColumn(9, _T("Active"), LVCFMT_LEFT, 60);
	m_ListWAN->InsertColumn(10, _T("Microphone"), LVCFMT_LEFT, 60);
	m_ListWAN->InsertColumn(11, _T("Strem"), LVCFMT_LEFT, 40);
	m_ListWAN->InsertColumn(12, _T("Calc"), LVCFMT_LEFT, 40);
	m_ListWAN->InsertColumn(13, _T("Session"), LVCFMT_LEFT, 40);
	m_ListWAN->InsertColumn(14, _T("Atimuth"), LVCFMT_LEFT, 50);
	m_ListWAN->InsertColumn(15, _T("Elevation"), LVCFMT_LEFT, 50);
	m_ListWAN->InsertColumn(16, _T("Power"), LVCFMT_LEFT, 60);
	m_ListWAN->InsertColumn(17, _T("BegTime"), LVCFMT_LEFT, 60);
	m_ListWAN->InsertColumn(18, _T("OperTime"), LVCFMT_LEFT, 60);
	m_ListWAN->InsertColumn(19, _T("NetCli"), LVCFMT_LEFT, 60);
	m_ListWAN->InsertColumn(20, _T("TypeCli"), LVCFMT_LEFT, 60);
*/
	int		TmpHF;
	int		LenF;
	UINT64	TmpID;
	int		TmpItem = 0;
	AudioLocatorSystemInfo TmpASInfo;

	memset(&TmpASInfo, 0, sizeof(AudioLocatorSystemInfo));
/*
	_sopen_s(&TmpHF, PathNameLANStationCfg, _O_RDWR | _O_BINARY, _SH_DENYNO, _S_IREAD | _S_IWRITE);
	if (TmpHF != -1)
	{
		do
		{
			LenF = _read(TmpHF, &TmpID, sizeof(UINT64));
			if (LenF == sizeof(UINT64))
			{
				StElem*	StEl;
				StEl = new StElem();
				TmpASInfo.StationID = TmpID;
				StEl->InitStEl(&TmpASInfo);
				LANStList.Add(StEl);
				s.Format(_T("%I64d"), TmpID);
				m_ListLAN->InsertItem(TmpItem, s);
				s.Format(_T("Inactive"));
				m_ListLAN->SetItemText(TmpItem, 9, s);
				StEl->Item = TmpItem;
				TmpItem++;
			}
		} while (LenF == sizeof(UINT64));
		_close(TmpHF);
	}
*/
	TmpItem = 0;
/*
	_sopen_s(&TmpHF, PathNameWANStationCfg, _O_RDWR | _O_BINARY, _SH_DENYNO, _S_IREAD | _S_IWRITE);
	if (TmpHF != -1)
	{
		do
		{
			LenF = _read(TmpHF, &TmpID, sizeof(UINT64));
			if (LenF == sizeof(UINT64))
			{
				StElem*	StEl;
				StEl = new StElem();
				TmpASInfo.StationID = TmpID;
				StEl->InitStEl(&TmpASInfo);
				WANStList.Add(StEl);
				s.Format(_T("%I64d"), TmpID);
				m_ListWAN->InsertItem(TmpItem, s);
				s.Format(_T("Inactive"));
				m_ListWAN->SetItemText(TmpItem, 9, s);
				StEl->Item = TmpItem;
				TmpItem++;
			}
		} while (LenF == sizeof(UINT64));
		_close(TmpHF);
	}
*/
//	s.Format(_T("%d.%d.%d.%d"), Srv->IPAddrLAN.S_un.S_un_b.s_b1, Srv->IPAddrLAN.S_un.S_un_b.s_b2, Srv->IPAddrLAN.S_un.S_un_b.s_b3, Srv->IPAddrLAN.S_un.S_un_b.s_b4);
	win = GetDlgItem(IDC_EDIT1);
	strcpy(SendBuf, inet_ntoa(*(in_addr*)&MCfg->MData.IP_LAN));
	s.Format(_T("%S"), SendBuf);
	win->SetWindowText(s);
	
	s.Format(_T("%S"), UCfg->StCfg.IP_Port);
	win = GetDlgItem(IDC_EDIT2);
	win->SetWindowText(s);

	s.Format(_T("%.02f"), UCfg->StCfg.Poligon);
	win = GetDlgItem(IDC_EDIT11);
	win->SetWindowText(s);

	if (UCfg->StCfg.Speed < 1)UCfg->StCfg.Speed = 1;
	if (UCfg->StCfg.Speed > 1000)UCfg->StCfg.Speed = 1000;

	s.Format(_T("%d"), UCfg->StCfg.Speed);
	win = GetDlgItem(IDC_EDIT5);
	win->SetWindowText(s);

	if (UCfg->StCfg.Step < 1)UCfg->StCfg.Step = 1;
	if (UCfg->StCfg.Step > 1000)UCfg->StCfg.Step = 1000;

	s.Format(_T("%d"), UCfg->StCfg.Step);
	win = GetDlgItem(IDC_EDIT6);
	win->SetWindowText(s);

	win = GetDlgItem(IDC_EDIT7);
	strcpy(SendBuf, inet_ntoa(*(in_addr*)&UCfg->StCfg.IP_Stepper));
	s.Format(_T("%S"), SendBuf);
	win->SetWindowText(s);


//	win = GetDlgItem(IDC_EDIT6);
//	strcpy(SendBuf, inet_ntoa(*(in_addr*)&MCfg->MData.IP_WAN));
//	s.Format(_T("%S"), SendBuf);
//	win->SetWindowText(s);


	s.Format(_T("%S"), UCfg->StCfg.PathNameMainApp);
	win = GetDlgItem(IDC_MFCEDITBROWSE1);
	if (s.GetLength() > 126)
	{
		s.Format(_T(""));
	};
	win->SetWindowText(s);
	s.Format(_T("%S"), UCfg->StCfg.PathNameSmStarter);
	win = GetDlgItem(IDC_MFCEDITBROWSE2);
	if (s.GetLength() > 126)
	{
		s.Format(_T(""));
	};
	win->SetWindowText(s);
	s.Format(_T("%S"), UCfg->StCfg.PathNameFirmWare);
	win = GetDlgItem(IDC_MFCEDITBROWSE3);
	if (s.GetLength() > 126)
	{
		s.Format(_T(""));
	};
	win->SetWindowText(s);


//	win = GetDlgItem(IDC_EDIT5);
////	s.Format(_T("0x%016I64x"), UCfg->StCfg.StationID);
//	s.Format(_T("%I64d"), UCfg->StCfg.StationIDLAN);
//	win->SetWindowText(s);

//	win = GetDlgItem(IDC_EDIT7);
//	s.Format(_T("%I64d"), UCfg->StCfg.StationIDWAN);
//	win->SetWindowText(s);


	CButton*	pBox;
	pBox = (CButton*)GetDlgItem(IDC_CHECK1);
	pBox->SetCheck(UCfg->StCfg.Fl_UsedLANList);
	pBox = (CButton*)GetDlgItem(IDC_CHECK2);
	pBox->SetCheck(UCfg->StCfg.Fl_UseGeoData);
	pBox = (CButton*)GetDlgItem(IDC_CHECK3);
	pBox->SetCheck(UCfg->StCfg.Fl_Triangulation);
	pBox = (CButton*)GetDlgItem(IDC_CHECK4);
	pBox->SetCheck(UCfg->StCfg.Fl_En_UD);
	pBox = (CButton*)GetDlgItem(IDC_CHECK5);
	pBox->SetCheck(UCfg->StCfg.Fl_En_LR);

	if (DInt->ElUDPStepper)
	{
		sprintf(SendBuf, "E1=%d\n\r", UCfg->StCfg.Fl_En_UD);
		DInt->ElUDPStepper->SendTo((UCHAR*)SendBuf, strlen(SendBuf), (sockaddr*)&AddrToStepper);
		sprintf(SendBuf, "E2=%d\n\r", UCfg->StCfg.Fl_En_LR);
		DInt->ElUDPStepper->SendTo((UCHAR*)SendBuf, strlen(SendBuf), (sockaddr*)&AddrToStepper);
	}

	StNum = 0;
	SetTimer(1, 100, NULL);
	SetTimer(2, 10000, NULL);
	SetTimer(3, 300, NULL);

//	SrvSockLAN->Param("ServerPort", "4082" );	//UCfg->StCfg.IP_Port
//	SrvSockLAN->Start();
/*
	SrvSockWAN->Param("ServerPort", UCfg->StCfg.IP_Port);
	SrvSockWAN->Start();

	SrvSockWEBL->Param("ServerPort", UCfg->StCfg.IP_WEBPort);
	SrvSockWEBL->Start();
	SrvSockWEBW->Param("ServerPort", UCfg->StCfg.IP_WEBPort);
	SrvSockWEBW->Start();
*/
	Fl_Ready = 1;
	SrvSockLAN_Thread = new ClassThread("SrvSockLAN_Calc", SrvSockLAN_Calc, (LPVOID)this);
//	SrvSockWAN_Thread = new ClassThread("SrvSockWAN_Calc", SrvSockWAN_Calc, (LPVOID)this);
//	SrvSockWEBL_Thread = new ClassThread("SrvSockWEBL_Calc", SrvSockWEBL_Calc, (LPVOID)this);
//	SrvSockWEBW_Thread = new ClassThread("SrvSockWEBW_Calc", SrvSockWEBW_Calc, (LPVOID)this);
//	Boost_Thread = new ClassThread("Boost_Calc", Boost_Calc, (LPVOID)this);
	
//	ElUDPHUB = Srv->OpenUDPCilent(NT_PAN, 5001, NULL);

	return TRUE;  // возврат значения TRUE, если фокус не передан элементу управления
}

void CSoundServerDlg::OnSysCommand(UINT nID, LPARAM lParam)
{
	if ((nID & 0xFFF0) == IDM_ABOUTBOX)
	{
		CAboutDlg dlgAbout;
		dlgAbout.DoModal();
	}
	else
	{
		CDialogEx::OnSysCommand(nID, lParam);
	}
}

// При добавлении кнопки свертывания в диалоговое окно нужно воспользоваться приведенным ниже кодом,
//  чтобы нарисовать значок.  Для приложений MFC, использующих модель документов или представлений,
//  это автоматически выполняется рабочей областью.

void CSoundServerDlg::OnPaint()
{
	if (IsIconic())
	{
		CPaintDC dc(this); // контекст устройства для рисования

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// Выравнивание значка по центру клиентского прямоугольника
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// Нарисуйте значок
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialogEx::OnPaint();
	}
}

int CSoundServerDlg::LANSendPacket(uint NumCli, AudioStationPack* APack)
{
	int rc = 0;
	IPElem	*IPEl = (IPElem*)LANCliList.Find((UCHAR*)&NumCli, sizeof(int));
	if (IPEl)
	{
//		SrvSockLAN->Data(0, (UCHAR*)&NumCli, 4);
//		SrvSockLAN->Data(1, (UCHAR*)APack, sizeof(PackHeader) + APack->PHeader.DataLen);
	}
	return rc;
}

/*
int CSoundServerDlg::WANSendPacket(uint NumCli, AudioStationPack* APack)
{
	int rc = 0;
	IPElem	*IPEl = (IPElem*)WANCliList.Find((UCHAR*)&NumCli, sizeof(int));
	if (IPEl)
	{
		SrvSockWAN->Data(0, (UCHAR*)&NumCli, 4);
		SrvSockWAN->Data(1, (UCHAR*)APack, sizeof(PackHeader) + APack->PHeader.DataLen);
	}
	return rc;
}
*/
#if 1
int CSoundServerDlg::LANParserPacket(IPElem* IPEl,UCHAR* Buf, uint Len)
{
	uint				rc = 0;
	CWnd				*win;
	CString				s;
	AudioStationPack*	APack=(AudioStationPack*)Buf;
	switch (APack->PHeader.Command)
	{
	case Sys_WDog:
		break;
	case St_Answ_OK:
		DataCnt += Len;
		if (DInt->WavTest)
			DInt->WavTest->WriteFloat(Buf+4, (Len-4)/4);	// 48000/16=3000
		Fl_WiatingAnswerLAN = 0;
		break;
	case St_Answ_Error:
		Fl_WiatingAnswerLAN = 2;
		break;
	case St_Answ_Absent:
		Fl_WiatingAnswerLAN = 3;
		break;
	case St_Answ_UnKnown:
		Fl_WiatingAnswerLAN = 4;
		break;
	default:
		ASPack.PHeader.Command = St_Get_SystemInfo;
		ASPack.PHeader.DataLen = 0;
//		LANSendPacket(IPEl->NumCli, &ASPack);

/*
	case St_Send_Request:
		ASPack.PHeader.Command = St_Get_SystemInfo;
		ASPack.PHeader.DataLen = 0;
		LANSendPacket(IPEl->NumCli,&ASPack);
		break;
	case St_Send_RealTimeInfo:
		if (APack->PHeader.DataLen <= sizeof(AudioLocatorRealTimeInfo))
		{
			CString		csText;
			StElem		*StElF;
			FindStrList8	FSTR;
			memcpy(&ALSInfo.RTInfo, APack->Data, sizeof(AudioLocatorRealTimeInfo));

			StElF = (StElem*)LANStList.Begin(FSTR);
			while (StElF)
			{
				
				if ((StElF->Fl_Active)&&(StElF->NumCli == IPEl->NumCli))
				{
					SYSTEMTIME	STime;
					FileTime	SrvTime;

					memcpy(&StElF->ALSInfo.RTInfo, &ALSInfo.RTInfo, sizeof(AudioLocatorRealTimeInfo));

					Srv->GetRealTime(&SrvTime.FTime);
					//
					// Сохранить в лог файле
					sprintf_s(LogBuf, "StationID=%d Session ID=%I64x Status=%d FiAzimuth=%0.1f FiElevation=%0.1f P_Delta=%0.1f DeltaTime=%I64d", StElF->ALSInfo.StationID, ALSInfo.RTInfo.SessionID, ALSInfo.RTInfo.Status, ALSInfo.RTInfo.FiAzimuth, ALSInfo.RTInfo.FiElevation, ALSInfo.RTInfo.P_Delta, SrvTime.Time - ALSInfo.RTInfo.SessionOperTime.Time);

//					float		CPULoad;				// 0 - 100.0 %
//					UCHAR		MicroPhoneStatus[8];	// 0 - Undeterm 1 - Ok 2-N Error
//					UCHAR		Status;					// Station Status
//					UCHAR		Fl_Stream;				//	
//					UCHAR		Fl_Calc;				//
//					UCHAR		Fl_Session;				//
//					INT64		SessionID;				//
//					FileTime	SessionBeginTime;		//
//					FileTime	SessionOperTime;		//
//					float		FiAzimuth;				//
//					float		FiElevation;			//
//					float		P_Delta;				//
//					int			FiAzimReady;
//					int			FiElevReady;

					Srv->OutLog(LogBuf, Srv, "RealTimeInfoLAN");
					// ---------------------


					csText.Format(_T("%0.2f"), StElF->ALSInfo.RTInfo.CPULoad);
					m_ListLAN->SetItemText(StElF->Item, 7, csText);
					csText.Format(_T("%d"), StElF->ALSInfo.RTInfo.Status);
					m_ListLAN->SetItemText(StElF->Item, 8, csText);

					csText.Format(_T("%d%d%d%d%d%d%d%d"), StElF->ALSInfo.RTInfo.MicroPhoneStatus[0], StElF->ALSInfo.RTInfo.MicroPhoneStatus[1], StElF->ALSInfo.RTInfo.MicroPhoneStatus[2], StElF->ALSInfo.RTInfo.MicroPhoneStatus[3], StElF->ALSInfo.RTInfo.MicroPhoneStatus[4], StElF->ALSInfo.RTInfo.MicroPhoneStatus[5], StElF->ALSInfo.RTInfo.MicroPhoneStatus[6], StElF->ALSInfo.RTInfo.MicroPhoneStatus[7]);
					m_ListLAN->SetItemText(StElF->Item, 10, csText);
					csText.Format(_T("%d"), StElF->ALSInfo.RTInfo.Fl_Stream);
					m_ListLAN->SetItemText(StElF->Item, 11, csText);
					csText.Format(_T("%d"), StElF->ALSInfo.RTInfo.Fl_Calc);
					m_ListLAN->SetItemText(StElF->Item, 12, csText);
					csText.Format(_T("%d"), StElF->ALSInfo.RTInfo.Fl_Session);
					m_ListLAN->SetItemText(StElF->Item, 13, csText);
					UCHAR	Res = 0;
					for (int i = 4; i < 8; i++)
					{
						Res = StElF->ALSInfo.RTInfo.MicroPhoneStatus[i];
					}
					if (Res==1)
					{
						csText.Format(_T("%0.1f"), 180 * StElF->ALSInfo.RTInfo.FiAzimuth / pi);
						m_ListLAN->SetItemText(StElF->Item, 14, csText);
						csText.Format(_T("%0.1f"), 180 * StElF->ALSInfo.RTInfo.FiElevation / pi);
						m_ListLAN->SetItemText(StElF->Item, 15, csText);
						csText.Format(_T("%0.1f"), StElF->ALSInfo.RTInfo.P_Delta);
						m_ListLAN->SetItemText(StElF->Item, 16, csText);
					}
					else
					{
						csText.Format(_T(""));
						m_ListLAN->SetItemText(StElF->Item, 14, csText);
						m_ListLAN->SetItemText(StElF->Item, 15, csText);
						m_ListLAN->SetItemText(StElF->Item, 16, csText);
					}
					if (StElF->ALSInfo.RTInfo.Fl_Session)
					{
						FileTime	tFTime;
						tFTime.Time = StElF->ALSInfo.RTInfo.SessionBeginTime.Time + 2 * KtimeH;
						FileTimeToSystemTime(&tFTime.FTime, &STime);
						csText.Format(_T("%02d:%02d.%02d"), STime.wHour, STime.wMinute, STime.wSecond);
						m_ListLAN->SetItemText(StElF->Item, 17, csText);
						tFTime.Time = StElF->ALSInfo.RTInfo.SessionOperTime.Time + 2 * KtimeH;
						FileTimeToSystemTime(&tFTime.FTime, &STime);
						csText.Format(_T("%02d:%02d.%02d"), STime.wHour, STime.wMinute, STime.wSecond);
						m_ListLAN->SetItemText(StElF->Item, 18, csText);
					}
					else
					{
						csText.Format(_T(""));
						m_ListLAN->SetItemText(StElF->Item, 17, csText);
						m_ListLAN->SetItemText(StElF->Item, 18, csText);
					}
					csText.Format(_T("%d"), StElF->NumCli);
					m_ListLAN->SetItemText(StElF->Item, 19, csText);
					break;
				}
				StElF = (StElem*)LANStList.Next(FSTR);
			}
		}
		break;
	case St_Send_SystemInfo:


		if (APack->PHeader.DataLen <= sizeof(AudioLocatorSystemInfo))
		{
			memcpy(&ALSInfo, APack->Data, sizeof(AudioLocatorSystemInfo));

			sprintf_s(LogBuf, "St_Send_SystemInfo NetCli=%d StID=%I64d", IPEl->NumCli, ALSInfo.StationID);
			Srv->OutLog(LogBuf, Srv, "SrvConnection");
//			strcpy(ALSInfo.CliIP, IPEl->CliIP);
//			strcpy(ALSInfo.CliName, IPEl->CliName);
			if (ALSInfo.StationID==0)
			{
				
				ASPack.PHeader.Command = St_Set_StationNewID;
				ASPack.PHeader.DataLen = sizeof(UINT64);
				*((UINT64*)ASPack.Data) = UCfg->StCfg.StationIDLAN++;
				LANSendPacket(IPEl->NumCli, &ASPack);
				win = GetDlgItem(IDC_EDIT5);
				s.Format(_T("%I64d"), UCfg->StCfg.StationIDLAN);
				win->SetWindowText(s);

			}
			else
			{
				if (ALSInfo.StationID > UCfg->StCfg.StationIDLAN)
				{
					ASPack.PHeader.Command = St_Set_StationNewID;
					ASPack.PHeader.DataLen = sizeof(UINT64);
					*((UINT64*)ASPack.Data) = UCfg->StCfg.StationIDLAN++;
					LANSendPacket(IPEl->NumCli, &ASPack);
					win = GetDlgItem(IDC_EDIT5);
					s.Format(_T("%I64d"), UCfg->StCfg.StationIDLAN);
					win->SetWindowText(s);
					//					UCfg->StCfg.StationID = ALSInfo.StationID + 1;
				}
			}
			if (ALSInfo.StationID != 0)
			{
				StElem		*StEl;
				StElem		*StElF;
				int			Item = 0;
				int			FlNew = 0;
				CString		csText;

				StEl = (StElem*)LANStList.Find((UCHAR*)&ALSInfo.StationID, sizeof(UINT64));
				if (StEl == NULL)
				{

					StEl = (StElem*)LANStList.NewElem();
					if (StEl == NULL)
						StEl = new	StElem();
					StEl->InitStEl(&ALSInfo);
					LANStList.Add(StEl);
					FlNew = 1;
					FindStrList8	FSTR;
					StElF = (StElem*)LANStList.Begin(FSTR);
					while (StElF)
					{
						StElF->Item = Item;
						Item++;
						StElF = (StElem*)LANStList.Next(FSTR);
					}
					sprintf_s(LogBuf, "New Item=%d NetCli=%d StID=%I64d", StEl->Item, IPEl->NumCli, ALSInfo.StationID);
					Srv->OutLog(LogBuf, Srv, "SrvConnection");
				}
				else
				{
					sprintf_s(LogBuf, "Old Item=%d NetCli=%d StID=%I64d", StEl->Item, IPEl->NumCli, ALSInfo.StationID);
					Srv->OutLog(LogBuf, Srv, "SrvConnection");

					if ((StEl->NumCli != IPEl->NumCli)&&(StEl->Fl_Active))
					{	// Дубляж идентификатора станции. Отключаем старую включаем новую.
						sprintf_s(LogBuf, "Duble Item=%d NumCli=%d NetCli=%d StID=%I64d", StEl->Item, StEl->NumCli, IPEl->NumCli, ALSInfo.StationID);
						Srv->OutLog(LogBuf, Srv, "SrvConnection");
//						StDisConnect(StEl->NumCli);
//						StEl->Fl_Connect = 0;
//						CliList.Del(StEl); IPEl = 0;
						StEl->NumCli = IPEl->NumCli;
						memcpy(&StEl->ALSInfo, &ALSInfo, sizeof(AudioLocatorSystemInfo));

//						ASPack.PHeader.Command = St_Set_StationNewID;
//						ASPack.PHeader.DataLen = sizeof(UINT64);
//						*((UINT64*)ASPack.Data) = UCfg->StCfg.StationID++;
//						SendPacket(IPEl->NumCli, &ASPack);
//						win = GetDlgItem(IDC_EDIT5);
//						s.Format(_T("%I64d"), UCfg->StCfg.StationID);
//						win->SetWindowText(s);
						break;
					}
					else
					{
						memcpy(&StEl->ALSInfo, &ALSInfo, sizeof(AudioLocatorSystemInfo));
					}

					// Проверить есть ли еще стации с таким идентификотором
//					FindStrList8	FSTR;
//					StElF = (StElem*)StList.Begin(FSTR);
//					while (StElF)
//					{
//						if (StElF != StEl)
//						{
//							if (StElF->ALSInfo.StationID == ALSInfo.StationID)
//							{
//								// Да есть!!!
//								ASPack.PHeader.Command = St_Set_StationNewID;
//								ASPack.PHeader.DataLen = sizeof(UINT64);
//								*((UINT64*)ASPack.Data) = UCfg->StCfg.StationID++;
//								SendPacket(StEl->NumCli, &ASPack);
//								ASPack.PHeader.Command = St_Send_Reset;
//								ASPack.PHeader.DataLen = 0;
//								SendPacket(StEl->NumCli, &ASPack);

//							}
//						}
//						StElF = (StElem*)StList.Next(FSTR);
//					}

				}
				StEl->Fl_Active = 1;
				Item = StEl->Item;
				StEl->NumCli = IPEl->NumCli;
				if (FlNew == 1)
				{
//					csText.Format(_T("0x%016I64x"), StEl->ALSInfo.StationID);
					csText.Format(_T("%I64d"), StEl->ALSInfo.StationID);
					m_ListLAN->InsertItem(Item, csText);
				}
				csText.Format(_T("%S"), StEl->ALSInfo.CliIP);
				m_ListLAN->SetItemText(Item, 1, csText);
				csText.Format(_T("%S"), StEl->ALSInfo.CliName);
				m_ListLAN->SetItemText(Item, 2, csText);
				csText.Format(_T("%d.%02d"), StEl->ALSInfo.VersionMainApp.Version, StEl->ALSInfo.VersionMainApp.Build);
				m_ListLAN->SetItemText(Item, 3, csText);
				csText.Format(_T("%d.%02d"), StEl->ALSInfo.VersionSmStarter.Version, StEl->ALSInfo.VersionSmStarter.Build);
				m_ListLAN->SetItemText(Item, 4, csText);
				csText.Format(_T("%d.%02d"), StEl->ALSInfo.VersionFirmWare.Version, StEl->ALSInfo.VersionFirmWare.Build);
				m_ListLAN->SetItemText(Item, 5, csText);
				csText.Format(_T("%0.2f"), StEl->ALSInfo.RTInfo.CPULoad);
				m_ListLAN->SetItemText(Item, 7, csText);
				switch (StEl->ALSInfo.RTInfo.Status)
				{
				}
				csText.Format(_T("%d"), StEl->ALSInfo.RTInfo.Status);
				m_ListLAN->SetItemText(Item, 8, csText);
				csText.Format(_T("Active"));
				m_ListLAN->SetItemText(Item, 9, csText);

				csText.Format(_T("%d"), StEl->NumCli);
				m_ListLAN->SetItemText(Item, 19, csText);
				switch (StEl->ALSInfo.TypeCli)
				{
				case 0:
					csText.Format(_T("UnDef"));
					break;
				case 1:
					csText.Format(_T("Station"));
					break;
				case 2:
					csText.Format(_T("ControlCli"));
					break;
				case 3:
					csText.Format(_T("UserCli"));
					break;
				default:
					csText.Format(_T("UnDef"));
				}
				m_ListLAN->SetItemText(Item, 20, csText);
			}
		}
		else
			ErrorLAN(10);
		break;
*/
	}
	return rc;
}
#endif
/*
int CSoundServerDlg::WANParserPacket(IPElem* IPEl, UCHAR* Buf, uint Len)
{
	uint				rc = 0;
	CWnd				*win;
	CString				s;
	AudioStationPack*	APack = (AudioStationPack*)Buf;
	switch (APack->PHeader.Command)
	{
	case Sys_WDog:
		break;
	case St_Answ_OK:
		Fl_WiatingAnswerWAN = 0;
		break;
	case St_Answ_Error:
		Fl_WiatingAnswerWAN = 2;
		break;
	case St_Answ_Absent:
		Fl_WiatingAnswerWAN = 3;
		break;
	case St_Answ_UnKnown:
		Fl_WiatingAnswerWAN = 4;
		break;
	case St_Send_Request:
		ASPack.PHeader.Command = St_Get_SystemInfo;
		ASPack.PHeader.DataLen = 0;
		WANSendPacket(IPEl->NumCli, &ASPack);
		break;
	case St_Send_RealTimeInfo:
		if (APack->PHeader.DataLen <= sizeof(AudioLocatorRealTimeInfo))
		{
			CString		csText;
			StElem		*StElF;
			FindStrList8	FSTR;
			memcpy(&ALSInfo.RTInfo, APack->Data, sizeof(AudioLocatorRealTimeInfo));

			StElF = (StElem*)WANStList.Begin(FSTR);
			while (StElF)
			{

				if ((StElF->Fl_Active) && (StElF->NumCli == IPEl->NumCli))
				{
					SYSTEMTIME	STime;
					memcpy(&StElF->ALSInfo.RTInfo, &ALSInfo.RTInfo, sizeof(AudioLocatorRealTimeInfo));
					csText.Format(_T("%0.2f"), StElF->ALSInfo.RTInfo.CPULoad);
					m_ListWAN->SetItemText(StElF->Item, 7, csText);
					csText.Format(_T("%d"), StElF->ALSInfo.RTInfo.Status);
					m_ListWAN->SetItemText(StElF->Item, 8, csText);

					csText.Format(_T("%d%d%d%d%d%d%d%d"), StElF->ALSInfo.RTInfo.MicroPhoneStatus[0], StElF->ALSInfo.RTInfo.MicroPhoneStatus[1], StElF->ALSInfo.RTInfo.MicroPhoneStatus[2], StElF->ALSInfo.RTInfo.MicroPhoneStatus[3], StElF->ALSInfo.RTInfo.MicroPhoneStatus[4], StElF->ALSInfo.RTInfo.MicroPhoneStatus[5], StElF->ALSInfo.RTInfo.MicroPhoneStatus[6], StElF->ALSInfo.RTInfo.MicroPhoneStatus[7]);
					m_ListWAN->SetItemText(StElF->Item, 10, csText);
					csText.Format(_T("%d"), StElF->ALSInfo.RTInfo.Fl_Stream);
					m_ListWAN->SetItemText(StElF->Item, 11, csText);
					csText.Format(_T("%d"), StElF->ALSInfo.RTInfo.Fl_Calc);
					m_ListWAN->SetItemText(StElF->Item, 12, csText);
					csText.Format(_T("%d"), StElF->ALSInfo.RTInfo.Fl_Session);
					m_ListWAN->SetItemText(StElF->Item, 13, csText);
					UCHAR	Res = 0;
					for (int i = 4; i < 8; i++)
					{
						Res = StElF->ALSInfo.RTInfo.MicroPhoneStatus[i];
					}
					if (Res == 1)
					{
						csText.Format(_T("%0.1f"), 180 * StElF->ALSInfo.RTInfo.FiAzimuth / pi);
						m_ListWAN->SetItemText(StElF->Item, 14, csText);
						csText.Format(_T("%0.1f"), 180 * StElF->ALSInfo.RTInfo.FiElevation / pi);
						m_ListWAN->SetItemText(StElF->Item, 15, csText);
						csText.Format(_T("%0.1f"), StElF->ALSInfo.RTInfo.P_Delta);
						m_ListWAN->SetItemText(StElF->Item, 16, csText);
					}
					else
					{
						csText.Format(_T(""));
						m_ListWAN->SetItemText(StElF->Item, 14, csText);
						m_ListWAN->SetItemText(StElF->Item, 15, csText);
						m_ListWAN->SetItemText(StElF->Item, 16, csText);
					}
					if (StElF->ALSInfo.RTInfo.Fl_Session)
					{
						FileTime	tFTime;
						tFTime.Time = StElF->ALSInfo.RTInfo.SessionBeginTime.Time + 2 * KtimeH;
						FileTimeToSystemTime(&tFTime.FTime, &STime);
						csText.Format(_T("%02d:%02d.%02d"), STime.wHour, STime.wMinute, STime.wSecond);
						m_ListWAN->SetItemText(StElF->Item, 17, csText);
						tFTime.Time = StElF->ALSInfo.RTInfo.SessionOperTime.Time + 2 * KtimeH;
						FileTimeToSystemTime(&tFTime.FTime, &STime);
						csText.Format(_T("%02d:%02d.%02d"), STime.wHour, STime.wMinute, STime.wSecond);
						m_ListWAN->SetItemText(StElF->Item, 18, csText);
					}
					else
					{
						csText.Format(_T(""));
						m_ListWAN->SetItemText(StElF->Item, 17, csText);
						m_ListWAN->SetItemText(StElF->Item, 18, csText);
					}
					csText.Format(_T("%d"), StElF->NumCli);
					m_ListWAN->SetItemText(StElF->Item, 19, csText);
					break;
				}
				StElF = (StElem*)WANStList.Next(FSTR);
			}
		}
		break;
	case St_Send_SystemInfo:


		if (APack->PHeader.DataLen <= sizeof(AudioLocatorSystemInfo))
		{
			memcpy(&ALSInfo, APack->Data, sizeof(AudioLocatorSystemInfo));

			sprintf_s(LogBuf, "St_Send_SystemInfo NetCli=%d StID=%I64d", IPEl->NumCli, ALSInfo.StationID);
			Srv->OutLog(LogBuf, Srv, "SrvConnection");
			//			strcpy(ALSInfo.CliIP, IPEl->CliIP);
			//			strcpy(ALSInfo.CliName, IPEl->CliName);
			if (ALSInfo.StationID == 0)
			{

				ASPack.PHeader.Command = St_Set_StationNewID;
				ASPack.PHeader.DataLen = sizeof(UINT64);
				*((UINT64*)ASPack.Data) = UCfg->StCfg.StationIDWAN++;
				WANSendPacket(IPEl->NumCli, &ASPack);
				win = GetDlgItem(IDC_EDIT5);
				s.Format(_T("%I64d"), UCfg->StCfg.StationIDWAN);
				win->SetWindowText(s);

			}
			else
			{
				if (ALSInfo.StationID > UCfg->StCfg.StationIDWAN)
				{
					ASPack.PHeader.Command = St_Set_StationNewID;
					ASPack.PHeader.DataLen = sizeof(UINT64);
					*((UINT64*)ASPack.Data) = UCfg->StCfg.StationIDWAN++;
					WANSendPacket(IPEl->NumCli, &ASPack);
					win = GetDlgItem(IDC_EDIT5);
					s.Format(_T("%I64d"), UCfg->StCfg.StationIDWAN);
					win->SetWindowText(s);
					//					UCfg->StCfg.StationID = ALSInfo.StationID + 1;
				}
			}
			if (ALSInfo.StationID != 0)
			{
				StElem		*StEl;
				StElem		*StElF;
				int			Item = 0;
				int			FlNew = 0;
				CString		csText;

				StEl = (StElem*)WANStList.Find((UCHAR*)&ALSInfo.StationID, sizeof(UINT64));
				if (StEl == NULL)
				{

					StEl = (StElem*)WANStList.NewElem();
					if (StEl == NULL)
						StEl = new	StElem();
					StEl->InitStEl(&ALSInfo);
					WANStList.Add(StEl);
					FlNew = 1;
					FindStrList8	FSTR;
					StElF = (StElem*)WANStList.Begin(FSTR);
					while (StElF)
					{
						StElF->Item = Item;
						Item++;
						StElF = (StElem*)WANStList.Next(FSTR);
					}
					sprintf_s(LogBuf, "New Item=%d NetCli=%d StID=%I64d", StEl->Item, IPEl->NumCli, ALSInfo.StationID);
					Srv->OutLog(LogBuf, Srv, "SrvConnection");
				}
				else
				{
					sprintf_s(LogBuf, "Old Item=%d NetCli=%d StID=%I64d", StEl->Item, IPEl->NumCli, ALSInfo.StationID);
					Srv->OutLog(LogBuf, Srv, "SrvConnection");

					if ((StEl->NumCli != IPEl->NumCli) && (StEl->Fl_Active))
					{	// Дубляж идентификатора станции. Отключаем старую включаем новую.
						sprintf_s(LogBuf, "Duble Item=%d NumCli=%d NetCli=%d StID=%I64d", StEl->Item, StEl->NumCli, IPEl->NumCli, ALSInfo.StationID);
						Srv->OutLog(LogBuf, Srv, "SrvConnection");
						//						StDisConnect(StEl->NumCli);
						//						StEl->Fl_Connect = 0;
						//						CliList.Del(StEl); IPEl = 0;
						StEl->NumCli = IPEl->NumCli;
						memcpy(&StEl->ALSInfo, &ALSInfo, sizeof(AudioLocatorSystemInfo));

						//						ASPack.PHeader.Command = St_Set_StationNewID;
						//						ASPack.PHeader.DataLen = sizeof(UINT64);
						//						*((UINT64*)ASPack.Data) = UCfg->StCfg.StationID++;
						//						SendPacket(IPEl->NumCli, &ASPack);
						//						win = GetDlgItem(IDC_EDIT5);
						//						s.Format(_T("%I64d"), UCfg->StCfg.StationID);
						//						win->SetWindowText(s);
						break;
					}
					else
					{
						memcpy(&StEl->ALSInfo, &ALSInfo, sizeof(AudioLocatorSystemInfo));
					}

				}
				StEl->Fl_Active = 1;
				Item = StEl->Item;
				StEl->NumCli = IPEl->NumCli;
				if (FlNew == 1)
				{
					//					csText.Format(_T("0x%016I64x"), StEl->ALSInfo.StationID);
					csText.Format(_T("%I64d"), StEl->ALSInfo.StationID);
					m_ListWAN->InsertItem(Item, csText);
				}
				csText.Format(_T("%S"), StEl->ALSInfo.CliIP);
				m_ListWAN->SetItemText(Item, 1, csText);
				csText.Format(_T("%S"), StEl->ALSInfo.CliName);
				m_ListWAN->SetItemText(Item, 2, csText);
				csText.Format(_T("%d.%02d"), StEl->ALSInfo.VersionMainApp.Version, StEl->ALSInfo.VersionMainApp.Build);
				m_ListWAN->SetItemText(Item, 3, csText);
				csText.Format(_T("%d.%02d"), StEl->ALSInfo.VersionSmStarter.Version, StEl->ALSInfo.VersionSmStarter.Build);
				m_ListWAN->SetItemText(Item, 4, csText);
				csText.Format(_T("%d.%02d"), StEl->ALSInfo.VersionFirmWare.Version, StEl->ALSInfo.VersionFirmWare.Build);
				m_ListWAN->SetItemText(Item, 5, csText);
				csText.Format(_T("%0.2f"), StEl->ALSInfo.RTInfo.CPULoad);
				m_ListWAN->SetItemText(Item, 7, csText);
				switch (StEl->ALSInfo.RTInfo.Status)
				{
				}
				csText.Format(_T("%d"), StEl->ALSInfo.RTInfo.Status);
				m_ListWAN->SetItemText(Item, 8, csText);
				csText.Format(_T("Active"));
				m_ListWAN->SetItemText(Item, 9, csText);

				csText.Format(_T("%d"), StEl->NumCli);
				m_ListWAN->SetItemText(Item, 19, csText);
				switch (StEl->ALSInfo.TypeCli)
				{
				case 0:
					csText.Format(_T("UnDef"));
					break;
				case 1:
					csText.Format(_T("Station"));
					break;
				case 2:
					csText.Format(_T("ControlCli"));
					break;
				case 3:
					csText.Format(_T("UserCli"));
					break;
				default:
					csText.Format(_T("UnDef"));
				}
				m_ListWAN->SetItemText(Item, 20, csText);
			}
		}
		else
			ErrorWAN(10);
		break;
	}
	return rc;
}
*/
#if 1
int CSoundServerDlg::LANStDisConnect(int	NumCli)
{
	int	rc = -1;
	StElem		*StElF;
	FindStrList8	FSTR;
	CString			csText;
	StElF = (StElem*)LANStList.Begin(FSTR);
	while (StElF)
	{
		if (StElF->NumCli == NumCli)
		{
/*
			if (ActiveStElLAN == StElF)
				ActiveStElLAN = 0;
			StElF->Fl_Active = 0;
//			m_List->DeleteItem(StElF->Item);
			csText.Format(_T(""));
			m_ListLAN->SetItemText(StElF->Item, 1, csText);
			m_ListLAN->SetItemText(StElF->Item, 2, csText);
			m_ListLAN->SetItemText(StElF->Item, 3, csText);
			m_ListLAN->SetItemText(StElF->Item, 4, csText);
			m_ListLAN->SetItemText(StElF->Item, 5, csText);
			m_ListLAN->SetItemText(StElF->Item, 6, csText);
			m_ListLAN->SetItemText(StElF->Item, 7, csText);
			m_ListLAN->SetItemText(StElF->Item, 8, csText);
			csText.Format(_T("Inactive"));
			m_ListLAN->SetItemText(StElF->Item, 9, csText);
			csText.Format(_T(""));
			m_ListLAN->SetItemText(StElF->Item, 10, csText);
			m_ListLAN->SetItemText(StElF->Item, 11, csText);
			m_ListLAN->SetItemText(StElF->Item, 12, csText);
			m_ListLAN->SetItemText(StElF->Item, 13, csText);
			m_ListLAN->SetItemText(StElF->Item, 14, csText);
			m_ListLAN->SetItemText(StElF->Item, 15, csText);
			m_ListLAN->SetItemText(StElF->Item, 16, csText);
			m_ListLAN->SetItemText(StElF->Item, 17, csText);
			m_ListLAN->SetItemText(StElF->Item, 18, csText);

			m_ListLAN->SetItemText(StElF->Item, 19, csText);
//			StList.Del(StElF);
			rc = 0;
			break;
*/
		}
		StElF = (StElem*)LANStList.Next(FSTR);
	}
	return rc;
}
#endif
/*
int CSoundServerDlg::WANStDisConnect(int	NumCli)
{
	int	rc = -1;
	StElem		*StElF;
	FindStrList8	FSTR;
	CString			csText;
	StElF = (StElem*)WANStList.Begin(FSTR);
	while (StElF)
	{
		if (StElF->NumCli == NumCli)
		{
			if (ActiveStElWAN == StElF)
				ActiveStElWAN = 0;
			StElF->Fl_Active = 0;
			//			m_List->DeleteItem(StElF->Item);
			csText.Format(_T(""));
			m_ListWAN->SetItemText(StElF->Item, 1, csText);
			m_ListWAN->SetItemText(StElF->Item, 2, csText);
			m_ListWAN->SetItemText(StElF->Item, 3, csText);
			m_ListWAN->SetItemText(StElF->Item, 4, csText);
			m_ListWAN->SetItemText(StElF->Item, 5, csText);
			m_ListWAN->SetItemText(StElF->Item, 6, csText);
			m_ListWAN->SetItemText(StElF->Item, 7, csText);
			m_ListWAN->SetItemText(StElF->Item, 8, csText);
			csText.Format(_T("Inactive"));
			m_ListWAN->SetItemText(StElF->Item, 9, csText);
			csText.Format(_T(""));
			m_ListWAN->SetItemText(StElF->Item, 10, csText);
			m_ListWAN->SetItemText(StElF->Item, 11, csText);
			m_ListWAN->SetItemText(StElF->Item, 12, csText);
			m_ListWAN->SetItemText(StElF->Item, 13, csText);
			m_ListWAN->SetItemText(StElF->Item, 14, csText);
			m_ListWAN->SetItemText(StElF->Item, 15, csText);
			m_ListWAN->SetItemText(StElF->Item, 16, csText);
			m_ListWAN->SetItemText(StElF->Item, 17, csText);
			m_ListWAN->SetItemText(StElF->Item, 18, csText);

			m_ListWAN->SetItemText(StElF->Item, 19, csText);
			//			StList.Del(StElF);
			rc = 0;
			break;
		}
		StElF = (StElem*)WANStList.Next(FSTR);
	}
	return rc;
}
*/

int	CSoundServerDlg::ErrorLAN(int NumError)
{
	sprintf_s(LogBuf, "Error=%d NetCli=%d", NumError, SrvLANNumCli);
	Srv->OutLog(LogBuf, Srv, "SrvConnection");
	return	NumError;
};

/*
int	CSoundServerDlg::ErrorWAN(int NumError)
{
	sprintf_s(LogBuf, "Error=%d NetCli=%d", NumError, SrvWANNumCli);
	Srv->OutLog(LogBuf, Srv, "SrvConnection");
	return	NumError;
};
*/
/*
int CSoundServerDlg::SrvSockWEBL_Idle(void)
{
	int	rc = 0;
	UCHAR*	tBuf = NULL;
	size_t	Len;
	int		RC = 0;
	int		i;

	if (SrvSockWEBL->Data(2, tBuf, 1))
	{
		for (i = 0; i < SrvSockWEBL->Num_PinO; i++)
		{
			RC = SrvSockWEBL->OutData(i, tBuf, Len);
			if (RC)
			{
				switch (i)
				{
				case 0:	// NumCli
					SrvWEBLNumCli = *(int*)tBuf;
					break;
				case 1:	// Event
					break;
				case 2:
					break;
				case 3:
					break;
				case 4:	// Data
					if (Fl_Ready)
					{
						//						tBuf[Len-1] = 0;
						//						sprintf_s(LogBuf, "WEBCli=%d", SrvWEBNumCli);
						//						sprintf_s(LogBuf, "WEBCli=%d \r\n%s", SrvWEBNumCli, (char*)tBuf );
						//						Srv->OutLog(LogBuf, Srv, "WEBConnection");
						//						if (flTest==0)
						WEBLNumPack++;

						HtmlLen = PrepareResponse();

						if (HtmlLen)
						{
							//							flTest = 1;

							SrvSockWEBL->Data(0, (UCHAR*)&SrvWEBLNumCli, 4);
							sprintf_s(StrOut, "HTTP/1.1 200 OK\r\nAccess-Control-Allow-Origin : *\r\nAccess-Control-Allow-Credentials : true\r\nAccess-Control-Max-Age : 1\r\nAccess-Control-Allow-Methods : PUT,POST,GET,OPTIONS\r\nAccess-Control-Allow-Headers : *\r\nReferrerPolicy : unsafe-urln\r\nServer: Apache\r\nContent - Language : uk\r\nContent - Type : text / data; charset = utf - 8\r\nContent - Length: %d\r\n\r\n\%s\r\n", HtmlLen, HtmlStr);
							SrvSockWEBL->Data(1, (UCHAR*)StrOut, strlen(StrOut));
							SrvSockWEBL->SrvSock->DisConnect(SrvWEBLNumCli);
							//							Srv->OutLog(StrOut, Srv, "WEBConnection");
						}
						else
						{
							strcpy(HtmlStr, "");
							HtmlLen = strlen(HtmlStr);
							SrvSockWEBL->Data(0, (UCHAR*)&SrvWEBLNumCli, 4);
							sprintf_s(StrOut, "HTTP/1.1 200 OK\r\nAccess-Control-Allow-Origin : *\r\nAccess-Control-Allow-Credentials : true\r\nAccess-Control-Max-Age : 1\r\nAccess-Control-Allow-Methods : PUT,POST,GET,OPTIONS\r\nAccess-Control-Allow-Headers : *\r\nReferrerPolicy : unsafe-urln\r\nServer: Apache\r\nContent - Language : uk\r\nContent - Type : text / data; charset = utf - 8\r\nContent - Length: %d\r\n\r\n\%s\r\n", HtmlLen, HtmlStr);
							SrvSockWEBL->Data(1, (UCHAR*)StrOut, strlen(StrOut));
							SrvSockWEBL->SrvSock->DisConnect(SrvWEBLNumCli);
							//							Srv->OutLog(StrOut, Srv, "WEBConnection");
						}
					}
					break;

				}
			}
		}
	}
	return rc;
}

int CSoundServerDlg::SrvSockWEBW_Idle(void)
{
	int	rc = 0;
	UCHAR*	tBuf = NULL;
	size_t	Len;
	int		RC = 0;
	int		i;

	if (SrvSockWEBW->Data(2, tBuf, 1))
	{
		for (i = 0; i < SrvSockWEBW->Num_PinO; i++)
		{
			RC = SrvSockWEBW->OutData(i, tBuf, Len);
			if (RC)
			{
				switch (i)
				{
				case 0:	// NumCli
					SrvWEBWNumCli = *(int*)tBuf;
					break;
				case 1:	// Event
					break;
				case 2:
					break;
				case 3:
					break;
				case 4:	// Data
					if (Fl_Ready)
					{
						tBuf[Len-1] = 0;
						sprintf_s(LogBuf, "WEBCli=%d", SrvWEBWNumCli);
						sprintf_s(LogBuf, "WEBCli=%d \r\n%s", SrvWEBWNumCli, (char*)tBuf );
						Srv->OutLog(LogBuf, Srv, "WEBConnection");
//						if (flTest==0)
						WEBWNumPack++;

							HtmlLen=PrepareResponse();

						if (HtmlLen)
						{
//							flTest = 1;

							SrvSockWEBW->Data(0, (UCHAR*)&SrvWEBWNumCli, 4);
							sprintf_s(StrOut, "HTTP/1.1 200 OK\r\nAccess-Control-Allow-Origin : *\r\nAccess-Control-Allow-Credentials : true\r\nAccess-Control-Max-Age : 1\r\nAccess-Control-Allow-Methods : PUT,POST,GET,OPTIONS\r\nAccess-Control-Allow-Headers : *\r\nReferrerPolicy : unsafe-urln\r\nServer: Apache\r\nContent - Language : uk\r\nContent - Type : text / data; charset = utf - 8\r\nContent - Length: %d\r\n\r\n\%s\r\n", HtmlLen, HtmlStr);
							SrvSockWEBW->Data(1, (UCHAR*)StrOut, strlen(StrOut));
							SrvSockWEBW->SrvSock->DisConnect(SrvWEBWNumCli);
//							Srv->OutLog(StrOut, Srv, "WEBConnection");
						}
						else
						{
							strcpy(HtmlStr,"");
							HtmlLen = strlen(HtmlStr);
							SrvSockWEBW->Data(0, (UCHAR*)&SrvWEBWNumCli, 4);
							sprintf_s(StrOut, "HTTP/1.1 200 OK\r\nAccess-Control-Allow-Origin : *\r\nAccess-Control-Allow-Credentials : true\r\nAccess-Control-Max-Age : 1\r\nAccess-Control-Allow-Methods : PUT,POST,GET,OPTIONS\r\nAccess-Control-Allow-Headers : *\r\nReferrerPolicy : unsafe-urln\r\nServer: Apache\r\nContent - Language : uk\r\nContent - Type : text / data; charset = utf - 8\r\nContent - Length: %d\r\n\r\n\%s\r\n", HtmlLen, HtmlStr);
							SrvSockWEBW->Data(1, (UCHAR*)StrOut, strlen(StrOut));
							SrvSockWEBW->SrvSock->DisConnect(SrvWEBWNumCli);
//							Srv->OutLog(StrOut, Srv, "WEBConnection");
						}

					}
					break;

				}
			}
		}
	}
	return rc;
}
*/
/*
int CSoundServerDlg::PrepareResponse()
{
	int  rc = 0;
	js_AudioLocatorSystemInfo	js_ALSInfo;

	HtmlStr[0] = 0;
	StElem		*StElF;
	FindStrList8	FSTR;
	if (UCfg->StCfg.Fl_UsedLANList)
	{
		StElF = (StElem*)LANStList.Begin(FSTR);
		while (StElF)
		{
			if (StElF->Fl_Active)
			{
				memcpy(&js_ALSInfo.ASInfo, &StElF->ALSInfo, sizeof(AudioLocatorSystemInfo));

				Chordia::stringer sg;
				JSON::StringSink ssi(&sg);
				JSON::Writer writer(&ssi);
				js_ALSInfo.serialize(writer);

				strcat(HtmlStr, sg.c_str());
				strcat(HtmlStr, "\r\n");
			}
			StElF = (StElem*)LANStList.Next(FSTR);
		}
	}
	if (UCfg->StCfg.Fl_UsedWANList)
	{
		StElF = (StElem*)WANStList.Begin(FSTR);
		while (StElF)
		{
			if (StElF->Fl_Active)
			{
				memcpy(&js_ALSInfo.ASInfo, &StElF->ALSInfo, sizeof(AudioLocatorSystemInfo));

				Chordia::stringer sg;
				JSON::StringSink ssi(&sg);
				JSON::Writer writer(&ssi);
				js_ALSInfo.serialize(writer);

				strcat(HtmlStr, sg.c_str());
				strcat(HtmlStr, "\r\n");
			}
			StElF = (StElem*)WANStList.Next(FSTR);
		}
	}
	rc = strlen(HtmlStr);
	return rc;
	//	sprintf_s(HtmlStr, "<html>\r\n<body>\r\n%s</body>\r\n</html>", sg.c_str());
};
*/

int CSoundServerDlg::SrvSockLAN_Idle(void)
{
	int	rc = 0;
	UCHAR*	tBuf = NULL;
	size_t	Len;
	int		RC = 0;
	int		i;
/*
	if (SrvSockLAN->Data(2, tBuf, 1))
	{
		for (i = 0; i < SrvSockLAN->Num_PinO; i++)
		{
			RC = SrvSockLAN->OutData(i, tBuf, Len);
			if (RC)
			{
				switch (i)
				{
				case 0:	// NumCli
					SrvLANNumCli = *(int*)tBuf;
					IPElLAN = (IPElem*)LANCliList.Find((UCHAR*)&SrvLANNumCli, sizeof(int));
					if (IPElLAN ==0)
					{	// New
						sprintf_s(LogBuf, "New LAN NetCli=%d", SrvLANNumCli);
						Srv->OutLog(LogBuf, Srv, "SrvConnection");

						IPElLAN = (IPElem*)LANCliList.NewElem();
						if (IPElLAN == NULL)
							IPElLAN = new	IPElem();
						IPElLAN->InitIPEl(SrvLANNumCli);
						LANCliList.Add(IPElLAN);
					}
					else
					{
						if (IPElLAN->NumCli != SrvLANNumCli)
							ErrorLAN(0);
					}
					break;
				case 1:	// Event
					switch (((EventMsg*)tBuf)->Event)
					{
					case 0:
						sprintf_s(LogBuf, "Connect LAN NetCli=%d", SrvLANNumCli);
						Srv->OutLog(LogBuf, Srv, "SrvConnection");
						if (IPElLAN ==0)
						{	// Old - Error
							ErrorLAN(1);
						}
						else
						{
							IPElLAN->Fl_Connect = 1;
						}
						break;
					default:
						sprintf_s(LogBuf, "Disconnect LAN NetCli=%d", SrvLANNumCli);
						Srv->OutLog(LogBuf, Srv, "SrvConnection");
						if (IPElLAN)
						{	// Old 
							LANStDisConnect(IPElLAN->NumCli);
							IPElLAN->Fl_Connect = 0;
							LANCliList.Del(IPElLAN); IPElLAN = 0;
						}
						else
						{	// New - Error
							ErrorLAN(2);
						}
					}
					//					win = GetDlgItem(IDC_EDIT28);
					//					win->SetWindowText(s);
					break;
				case 2:
					sprintf_s(LogBuf, "LAN CliIP=%s NetCli=%d", (char*)tBuf, SrvLANNumCli);
					Srv->OutLog(LogBuf, Srv, "SrvConnection");

					if (IPElLAN)
					{
						strcpy(IPElLAN->CliIP, (char*)tBuf);
					}
					else
					{	// Error
						ErrorLAN(3);
					}
					break;
				case 3:
					sprintf_s(LogBuf, "LAN CliName=%s NetCli=%d", (char*)tBuf, SrvLANNumCli);
					Srv->OutLog(LogBuf, Srv, "SrvConnection");

					if (IPElLAN)
					{
						strcpy(IPElLAN->CliName, (char*)tBuf);
					}
					else
					{	// Error
						ErrorLAN(4);
					}
					break;
				case 4:	// Data
					if (IPElLAN)
					{
						if (Fl_Ready)
							LANParserPacket(IPElLAN, tBuf, Len);
					}
					else
					{  //Error
						ErrorLAN(5);
					}
					break;
				}
			}
		}
	}
*/
	return rc;
};
/*
int CSoundServerDlg::SrvSockWAN_Idle(void)
{
	int	rc = 0;
	UCHAR*	tBuf = NULL;
	size_t	Len;
	int		RC = 0;
	int		i;

	if (SrvSockWAN->Data(2, tBuf, 1))
	{
		for (i = 0; i < SrvSockWAN->Num_PinO; i++)
		{
			RC = SrvSockWAN->OutData(i, tBuf, Len);
			if (RC)
			{
				switch (i)
				{
				case 0:	// NumCli
					SrvWANNumCli = *(int*)tBuf;
					IPElWAN = (IPElem*)WANCliList.Find((UCHAR*)&SrvWANNumCli, sizeof(int));
					if (IPElWAN == 0)
					{	// New
						sprintf_s(LogBuf, "New WAN NetCli=%d", SrvWANNumCli);
						Srv->OutLog(LogBuf, Srv, "SrvConnection");

						IPElWAN = (IPElem*)WANCliList.NewElem();
						if (IPElWAN == NULL)
							IPElWAN = new	IPElem();
						IPElWAN->InitIPEl(SrvWANNumCli);
						WANCliList.Add(IPElWAN);
					}
					else
					{
						if (IPElWAN->NumCli != SrvWANNumCli)
							ErrorWAN(0);
					}
					break;
				case 1:	// Event
					switch (((EventMsg*)tBuf)->Event)
					{
					case 0:
						sprintf_s(LogBuf, "Connect WAN NetCli=%d", SrvWANNumCli);
						Srv->OutLog(LogBuf, Srv, "SrvConnection");
						if (IPElWAN == 0)
						{	// Old - Error
							ErrorWAN(1);
						}
						else
						{
							IPElWAN->Fl_Connect = 1;
						}
						break;
					default:
						sprintf_s(LogBuf, "Disconnect WAN NetCli=%d", SrvWANNumCli);
						Srv->OutLog(LogBuf, Srv, "SrvConnection");
						if (IPElWAN)
						{	// Old 
							WANStDisConnect(IPElWAN->NumCli);
							IPElWAN->Fl_Connect = 0;
							WANCliList.Del(IPElWAN); IPElWAN = 0;
						}
						else
						{	// New - Error
							ErrorWAN(2);
						}
					}
					//					win = GetDlgItem(IDC_EDIT28);
					//					win->SetWindowText(s);
					break;
				case 2:
					sprintf_s(LogBuf, "WAN CliIP=%s NetCli=%d", (char*)tBuf, SrvWANNumCli);
					Srv->OutLog(LogBuf, Srv, "SrvConnection");

					if (IPElWAN)
					{
						strcpy(IPElWAN->CliIP, (char*)tBuf);
					}
					else
					{	// Error
						ErrorWAN(3);
					}
					break;
				case 3:
					sprintf_s(LogBuf, "WAN CliName=%s NetCli=%d", (char*)tBuf, SrvWANNumCli);
					Srv->OutLog(LogBuf, Srv, "SrvConnection");

					if (IPElWAN)
					{
						strcpy(IPElWAN->CliName, (char*)tBuf);
					}
					else
					{	// Error
						ErrorWAN(4);
					}
					break;
				case 4:	// Data
					if (IPElWAN)
					{
						if (Fl_Ready)
							WANParserPacket(IPElWAN, tBuf, Len);
					}
					else
					{  //Error
						ErrorWAN(5);
					}
					break;
				}
			}
		}
	}
	return rc;
};
*/
// Система вызывает эту функцию для получения отображения курсора при перемещении
//  свернутого окна.
HCURSOR CSoundServerDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}


void CSoundServerDlg::OnLvnItemchangedList1(NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMLISTVIEW pNMLV = reinterpret_cast<LPNMLISTVIEW>(pNMHDR);
	// TODO: добавьте свой код обработчика уведомлений

	*pResult = 0;
}


void CSoundServerDlg::OnEnChangeMfceditbrowse1()
{
	// TODO:  Если это элемент управления RICHEDIT, то элемент управления не будет
	// send this notification unless you override the CDialogEx::OnInitDialog()
	// функция и вызов CRichEditCtrl().SetEventMask()
	// with the ENM_CHANGE flag ORed into the mask.

	// TODO:  Добавьте код элемента управления
	CWnd	*win;
	CString	s;
	size_t	len = 0;
	win = GetDlgItem(IDC_MFCEDITBROWSE1);
	win->GetWindowText(s);
	if (s.GetLength() > 126)
	{
		s.Format(_T(""));
	};
	wcstombs_s(&len, UCfg->StCfg.PathNameMainApp, 126, s, 126);

}


void CSoundServerDlg::OnEnChangeMfceditbrowse2()
{
	// TODO:  Если это элемент управления RICHEDIT, то элемент управления не будет
	// send this notification unless you override the CDialogEx::OnInitDialog()
	// функция и вызов CRichEditCtrl().SetEventMask()
	// with the ENM_CHANGE flag ORed into the mask.

	// TODO:  Добавьте код элемента управления
	CWnd	*win;
	CString	s;
	size_t	len = 0;
	win = GetDlgItem(IDC_MFCEDITBROWSE2);
	win->GetWindowText(s);
	if (s.GetLength() > 126)
	{
		s.Format(_T(""));
	};
	wcstombs_s(&len, UCfg->StCfg.PathNameSmStarter, 126, s, 126);
}


void CSoundServerDlg::OnEnChangeMfceditbrowse3()
{
	// TODO:  Если это элемент управления RICHEDIT, то элемент управления не будет
	// send this notification unless you override the CDialogEx::OnInitDialog()
	// функция и вызов CRichEditCtrl().SetEventMask()
	// with the ENM_CHANGE flag ORed into the mask.

	// TODO:  Добавьте код элемента управления
	CWnd	*win;
	CString	s;
	size_t	len = 0;
	int		IndexV;
	char	TmpStr[512];
	win = GetDlgItem(IDC_MFCEDITBROWSE3);
	win->GetWindowText(s);
	if (s.GetLength() > 126)
	{
		s.Format(_T(""));
	};
	wcstombs_s(&len, UCfg->StCfg.PathNameFirmWare, 126, s, 126);
	for (IndexV = 0; IndexV < strlen(UCfg->StCfg.PathNameFirmWare); IndexV++)
	{
		strcpy(TmpStr, &UCfg->StCfg.PathNameFirmWare[IndexV]);
		TmpStr[5] = 0;
		if (strcmp(TmpStr, "4micV") == 0)
		{
			break;
		}
	}
	UCfg->StCfg.VersionFW.Version = atoi(UCfg->StCfg.PathNameFirmWare + IndexV+5);
	UCfg->StCfg.VersionFW.Build = atoi(UCfg->StCfg.PathNameFirmWare + IndexV+7);
}


void CSoundServerDlg::OnEnChangeEdit2()
{
	// TODO:  Если это элемент управления RICHEDIT, то элемент управления не будет
	// send this notification unless you override the CDialogEx::OnInitDialog()
	// функция и вызов CRichEditCtrl().SetEventMask()
	// with the ENM_CHANGE flag ORed into the mask.

	// TODO:  Добавьте код элемента управления
	CWnd		*win;
	CString		s;
	size_t	len;
	win = GetDlgItem(IDC_EDIT2);
	win->GetWindowText(s);
	if (s.GetLength() > 31)
	{
		s.Format(_T(""));
		win->SetWindowText(s);
	}
	wcstombs_s(&len, UCfg->StCfg.IP_Port, 32, s, 32);

}


void CSoundServerDlg::OnEnChangeEdit1()
{
	// TODO:  Если это элемент управления RICHEDIT, то элемент управления не будет
	// send this notification unless you override the CDialogEx::OnInitDialog()
	// функция и вызов CRichEditCtrl().SetEventMask()
	// with the ENM_CHANGE flag ORed into the mask.

	// TODO:  Добавьте код элемента управления
	CWnd		*win;
	char		tmpBuf[16];
	CString		s;
	size_t	len;
	win = GetDlgItem(IDC_EDIT1);
	win->GetWindowText(s);
	wcstombs_s(&len, tmpBuf, 16, s, 16);
	MCfg->MData.IP_LAN = inet_addr(tmpBuf);
}


void CSoundServerDlg::OnBnClickedCancel()
{
	// TODO: добавьте свой код обработчика уведомлений
	Fl_Ready = 0;
	SrvSockLAN_Thread->Stop();
//	SrvSockWAN_Thread->Stop();
//	SrvSockWEBL_Thread->Stop();
//	SrvSockWEBW_Thread->Stop();
	UCfg->Save();
	CDialogEx::OnCancel();
}

void CSoundServerDlg::OnTimer(UINT_PTR nIDEvent)
{
	// TODO: добавьте свой код обработчика сообщений или вызов стандартного
	CWnd			*win;
	char			tmpBuf[16];
	CString			s;

	CString			csText;
	DevElem*		DElF;
	FindStrList8	FSTR;
	float			SizeSns;
	float			FiAzLog;
	float			DirectionX, DirectionY;
	float			Tmp_DistanceXY;
	float			TmpUg;

	m_pMySV1->PaintData(DInt->SpeBuf, 1024);
	m_pMySV1->FreshRePaint();

	m_pMySV0->FullReInit();
	if (UCfg->StCfg.Poligon < 1) UCfg->StCfg.Poligon = 1;
	m_pMySV0->SetParamX(-UCfg->StCfg.Poligon * 0.5, UCfg->StCfg.Poligon * 0.5);
	m_pMySV0->SetParamY(-UCfg->StCfg.Poligon * 0.25, UCfg->StCfg.Poligon * 0.25);
	SizeSns = UCfg->StCfg.Poligon / 500.;
//	if (SizeSns < 1)
//		SizeSns = 1;
	if (nIDEvent == 2)
	{
		SYSTEMTIME	sTime;
		GetSystemTime(&sTime);
		uint32_t Micros = 1000 * (sTime.wMilliseconds + sTime.wSecond*1000 + sTime.wMinute * 60000);
		DInt->SendMsg2CAN(SET_TIME, ToAll, (uchar*)&Micros, 4);
	}
	if (nIDEvent == 3)
	{
		switch (Fl_UPD)
		{
		case 1:
			if (FWUpDateHendleLAN > 0)
			{
				_close(FWUpDateHendleLAN); FWUpDateHendleLAN = 0;
			}
			_sopen_s(&FWUpDateHendleLAN, UCfg->StCfg.PathNameFirmWare, _O_RDONLY | _O_BINARY, _SH_DENYNO, _S_IREAD);
			if (FWUpDateHendleLAN != -1)
			{
				UpDTmpDataLen = 0;
				UpDFile_Size = filelength(FWUpDateHendleLAN);
				UPD_Data[0] = 0;
				if (ActiveDevEl)
					DInt->SendMsg2CAN(UPD_BEGIN, ActiveDevEl->HW_ID,(uchar*) & UpDFile_Size, sizeof(UpDFile_Size));
				else
					DInt->SendMsg2CAN(UPD_BEGIN, 1, (uchar*)&UpDFile_Size, sizeof(UpDFile_Size));
				Fl_UPD++;
			}
			else
				Fl_UPD = 100;
			break;
		case 2:
			File_BlockLen = read(FWUpDateHendleLAN, File_Data, UPDBlockSize);
			UpDTmpDataLen += File_BlockLen;

			win = GetDlgItem(IDC_BUTTON2);
			s.Format(_T("%0.2f"), 100.0 * UpDTmpDataLen / UpDFile_Size);
			win->SetWindowText(s);
			if (File_BlockLen == UPDBlockSize)
			{
				for (int i = 0; i < File_BlockLen; i += 4)
				{
					memcpy(&UPD_Data[1], &File_Data[i], 4);
					if (ActiveDevEl)
						DInt->SendMsg2CAN(UPD_DATA, ActiveDevEl->HW_ID, &UPD_Data[0], 5);
					else
						DInt->SendMsg2CAN(UPD_DATA, 1, &UPD_Data[0], 5);
					UPD_Data[0]++;
				}
			}
			else
			{
				int DifLen = File_BlockLen / 4;
				int TmpLen = DifLen * 4;
				DifLen = File_BlockLen - TmpLen;
				for (int i = 0; i < TmpLen; i += 4)
				{
					memcpy(&UPD_Data[1], &File_Data[i], 4);
					if (ActiveDevEl)
						DInt->SendMsg2CAN(UPD_DATA, ActiveDevEl->HW_ID, &UPD_Data[0], 5);
					else
						DInt->SendMsg2CAN(UPD_DATA, 1, &UPD_Data[0], 5);
					UPD_Data[0]++;
				}
				if (DifLen)
				{
					memcpy(&UPD_Data[1], &File_Data[TmpLen], DifLen);
					if (ActiveDevEl)
						DInt->SendMsg2CAN(UPD_DATA, ActiveDevEl->HW_ID, &UPD_Data[0], 1+ DifLen);
					else
						DInt->SendMsg2CAN(UPD_DATA, 1, &UPD_Data[0], 1 + DifLen);
					UPD_Data[0]++;
				}
				Fl_UPD++;
			}
			break;
		case 3:
			if (ActiveDevEl)
				DInt->SendMsg2CAN(UPD_END, ActiveDevEl->HW_ID, NULL,0);
			else
				DInt->SendMsg2CAN(UPD_END, 1, NULL, 0);
			Fl_UPD=100;
			break;
		case 100:
			win = GetDlgItem(IDC_BUTTON2);
			s.Format(_T("UpData"));
			win->SetWindowText(s);
			Fl_UPD = 0;
			break;
		}
	}

	if (nIDEvent == 1)
	{

		DElF = (DevElem*)DInt->DevList.Begin(FSTR);
		while (DElF)
		{
			//		if ((StElF->Fl_Active) && (StElF->NumCli == IPEl->NumCli))
			//		{
			csText.Format(_T("ID=%d-T%d-V%d.%d"), DElF->HW_ID, DElF->DInfo.DevType, DElF->Version>>8, DElF->Version&0xff);
			if (DElF->Item < 0)
			{
				DElF->Item = MaxItem;
				m_ListUDP->InsertItem(MaxItem, csText);
				MaxItem++;
			}
			if (UCfg->StCfg.Dev[DElF->HW_ID].AthimuthCorrection > 360) UCfg->StCfg.Dev[DElF->HW_ID].AthimuthCorrection = 0;
			if (UCfg->StCfg.Dev[DElF->HW_ID].AthimuthCorrection < -360) UCfg->StCfg.Dev[DElF->HW_ID].AthimuthCorrection = 0;
			m_ListUDP->SetItemText(DElF->Item, 0, csText);
			csText.Format(_T("%.02f"), DElF->Azimuth * 180. / pi + UCfg->StCfg.Dev[DElF->HW_ID].AthimuthCorrection);
			m_ListUDP->SetItemText(DElF->Item, 1, csText);
			csText.Format(_T("%.02f"), DElF->UgolM * 180. / pi);
			m_ListUDP->SetItemText(DElF->Item, 2, csText);
			switch (DElF->SNSStr.State)
			{
			case SNS_Begin:
				csText.Format(_T("S:%d,%d,%d,%d"), DElF->Level[0], DElF->Level[1], DElF->Level[2], DElF->Level[3]);
				break;
			case SNS_End:
			default:
				csText.Format(_T("%d,%d,%d,%d"), DElF->Level[0], DElF->Level[1], DElF->Level[2], DElF->Level[3]);
				break;
			}

			m_ListUDP->SetItemText(DElF->Item, 3, csText);
			csText.Format(_T("%.02f"), DElF->Compass * 180. / pi);
			m_ListUDP->SetItemText(DElF->Item, 4, csText);

			csText.Format(_T("%.02f"), DElF->Ev_Azimuth * 180. / pi);
			m_ListUDP->SetItemText(DElF->Item, 5, csText);
			csText.Format(_T("%.02f"), DElF->Ev_UgolM * 180. / pi);
			m_ListUDP->SetItemText(DElF->Item, 6, csText);

			int Min = DElF->Ev_Time / 60000000;
			int Sec = (DElF->Ev_Time - Min * 60000000) / 1000000;
			int mSec = (DElF->Ev_Time - Min * 60000000 - Sec *1000000)/1000 ;

			csText.Format(_T("%02d:%02d.%03d"),Min,Sec,mSec);
			m_ListUDP->SetItemText(DElF->Item, 7, csText);


			int32_t wHour = (int32_t)DElF->UTC / 10000;
			int32_t wMinute = (int32_t)DElF->UTC / 100 - 100 * wHour;
			int32_t wSecond = (int32_t)DElF->UTC - 10000 * wHour - 100 * wMinute;
			csText.Format(_T("%d:%d.%d"), wHour, wMinute, wSecond);
			m_ListUDP->SetItemText(DElF->Item, 8, csText);
			csText.Format(_T("%.06f"), DElF->Lat);
			m_ListUDP->SetItemText(DElF->Item, 9, csText);
			csText.Format(_T("%.06f"), DElF->Lng);
			m_ListUDP->SetItemText(DElF->Item, 10, csText);
			csText.Format(_T("%.02f"), DElF->Alt);
			m_ListUDP->SetItemText(DElF->Item, 11, csText);
			csText.Format(_T("%d"), DElF->GPS_State);
			m_ListUDP->SetItemText(DElF->Item, 12, csText);
			csText.Format(_T("%d"), DataCnt);
			m_ListUDP->SetItemText(DElF->Item, 13, csText);

			if (abs(UCfg->StCfg.Dev[DElF->HW_ID].X) > UCfg->StCfg.Poligon)
				UCfg->StCfg.Dev[DElF->HW_ID].X = 0;
			if (abs(UCfg->StCfg.Dev[DElF->HW_ID].Y) > UCfg->StCfg.Poligon)
				UCfg->StCfg.Dev[DElF->HW_ID].Y = 0;
	//		UCfg->StCfg.Dev[DElF->HW_ID].X = UCfg->StCfg.Dev[DElF->HW_ID].Y = 0;
			m_pMySV0->PrmPlotLine(UCfg->StCfg.Dev[DElF->HW_ID].X - SizeSns, UCfg->StCfg.Dev[DElF->HW_ID].Y - SizeSns, UCfg->StCfg.Dev[DElF->HW_ID].X + SizeSns, UCfg->StCfg.Dev[DElF->HW_ID].Y - SizeSns, RGB(0, 0, 255));
			m_pMySV0->PrmPlotLine(UCfg->StCfg.Dev[DElF->HW_ID].X + SizeSns, UCfg->StCfg.Dev[DElF->HW_ID].Y - SizeSns, UCfg->StCfg.Dev[DElF->HW_ID].X + SizeSns, UCfg->StCfg.Dev[DElF->HW_ID].Y + SizeSns, RGB(0, 0, 255));
			m_pMySV0->PrmPlotLine(UCfg->StCfg.Dev[DElF->HW_ID].X + SizeSns, UCfg->StCfg.Dev[DElF->HW_ID].Y + SizeSns, UCfg->StCfg.Dev[DElF->HW_ID].X - SizeSns, UCfg->StCfg.Dev[DElF->HW_ID].Y + SizeSns, RGB(0, 0, 255));
			m_pMySV0->PrmPlotLine(UCfg->StCfg.Dev[DElF->HW_ID].X - SizeSns, UCfg->StCfg.Dev[DElF->HW_ID].Y + SizeSns, UCfg->StCfg.Dev[DElF->HW_ID].X - SizeSns, UCfg->StCfg.Dev[DElF->HW_ID].Y - SizeSns, RGB(0, 0, 255));

			if (!isnan(DElF->Azimuth))
			{
	//			UCfg->StCfg.Dev[DElF->HW_ID].AthimuthCorrection = 0;
				FiAzLog = DElF->Azimuth* 180.* _Pi;
				FiAzLog += UCfg->StCfg.Dev[DElF->HW_ID].AthimuthCorrection;
				if (FiAzLog < 0)FiAzLog += 360;
				if (FiAzLog >= 360) FiAzLog -= 360;

				Tmp_DistanceXY = UCfg->StCfg.Poligon * 0.5 * 0.005 *((float)DElF->Level[0] + (float)DElF->Level[1] + (float)DElF->Level[2] + (float)DElF->Level[3]-115.);
				if (Tmp_DistanceXY < UCfg->StCfg.Poligon * 0.001) Tmp_DistanceXY = UCfg->StCfg.Poligon * 0.001;
				DirectionX = UCfg->StCfg.Dev[DElF->HW_ID].X + Tmp_DistanceXY * sin(Pi2 * FiAzLog / 360.);
				DirectionY = UCfg->StCfg.Dev[DElF->HW_ID].Y + Tmp_DistanceXY * cos(Pi2 * FiAzLog / 360.);
	//			PE[i].X = DirectionX;
	//			PE[i].Y = DirectionY;

				TmpUg=DElF->UgolM * _Pi_2;
	//			m_pMySV0->PrmPlotLine(UCfg->StCfg.Dev[DElF->HW_ID].X, UCfg->StCfg.Dev[DElF->HW_ID].Y, DirectionX, DirectionY, RGB(0, 255 * (1 - TmpUg), 255 * TmpUg));
				m_pMySV0->PrmPlotLine(UCfg->StCfg.Dev[DElF->HW_ID].X, UCfg->StCfg.Dev[DElF->HW_ID].Y, DirectionX, DirectionY, RGB(0, 0, 0));
			}

			if ((!isnan(DElF->Ev_Azimuth))&&(UCfg->StCfg.Fl_Triangulation))
			{
				//			UCfg->StCfg.Dev[DElF->HW_ID].AthimuthCorrection = 0;
				FiAzLog = DElF->Ev_Azimuth * 180. * _Pi;
				FiAzLog += UCfg->StCfg.Dev[DElF->HW_ID].AthimuthCorrection;
				if (FiAzLog < 0)FiAzLog += 360;
				if (FiAzLog >= 360) FiAzLog -= 360;

				Tmp_DistanceXY = UCfg->StCfg.Poligon;
				DirectionX = UCfg->StCfg.Dev[DElF->HW_ID].X + Tmp_DistanceXY * sin(Pi2 * FiAzLog / 360.);
				DirectionY = UCfg->StCfg.Dev[DElF->HW_ID].Y + Tmp_DistanceXY * cos(Pi2 * FiAzLog / 360.);
				//			PE[i].X = DirectionX;
				//			PE[i].Y = DirectionY;

				TmpUg = DElF->UgolM * _Pi_2;
				//			m_pMySV0->PrmPlotLine(UCfg->StCfg.Dev[DElF->HW_ID].X, UCfg->StCfg.Dev[DElF->HW_ID].Y, DirectionX, DirectionY, RGB(0, 255 * (1 - TmpUg), 255 * TmpUg));
				m_pMySV0->PrmPlotLine(UCfg->StCfg.Dev[DElF->HW_ID].X, UCfg->StCfg.Dev[DElF->HW_ID].Y, DirectionX, DirectionY, RGB(255, 0, 0));
			}


	//			m_ListUDP->InsertColumn(0, _T("DevID"), LVCFMT_LEFT, 60);
	//			m_ListUDP->InsertColumn(1, _T("Atimuth"), LVCFMT_LEFT, 50);
	//			m_ListUDP->InsertColumn(2, _T("Elevation"), LVCFMT_LEFT, 50);
	//			m_ListUDP->InsertColumn(3, _T("Power"), LVCFMT_LEFT, 60);
	//			m_ListUDP->InsertColumn(4, _T("Compass"), LVCFMT_LEFT, 60);
	//			m_ListUDP->InsertColumn(5, _T("UTC"), LVCFMT_LEFT, 60);
	//			m_ListUDP->InsertColumn(6, _T("Lat"), LVCFMT_LEFT, 60);
	//			m_ListUDP->InsertColumn(7, _T("Lng"), LVCFMT_LEFT, 60);
	//			m_ListUDP->InsertColumn(8, _T("Alt"), LVCFMT_LEFT, 60);
	//			m_ListUDP->InsertColumn(9, _T("GPSStatus"), LVCFMT_LEFT, 40);

	//		}
			DElF = (DevElem*)DInt->DevList.Next(FSTR);
		}
		m_pMySV0->FreshRePaint();
	}

#if 0
	win = GetDlgItem(IDC_EDIT9);
	s.Format(_T("%d"), WEBLNumPack);
	win->SetWindowText(s);
	win = GetDlgItem(IDC_EDIT10);
	s.Format(_T("%d"), WEBWNumPack);
	win->SetWindowText(s);

	LenWSBuffer=PrepareResponse();

	StElem		*StElF;
	FindStrList8	FSTR;
	if ((Fl_UpDataAppLAN == 0) && (Fl_UpDataSmStLAN ==0) && (Fl_UpDataFWLAN ==0))
	{
		StElF = (StElem*)LANStList.Begin(FSTR);
		while (StElF)
		{
			if (StElF->Fl_Active)
			{
				if ((StElF->ALSInfo.VersionMainApp.Version < UCfg->StCfg.VersionApp.Version) || ((StElF->ALSInfo.VersionMainApp.Version == UCfg->StCfg.VersionApp.Version) && (StElF->ALSInfo.VersionMainApp.Build < UCfg->StCfg.VersionApp.Build)))
				{
					Fl_UpDataAppLAN = 1;
					UpDAppStElFLAN = StElF;
					break;
				}
				if ((StElF->ALSInfo.VersionSmStarter.Version) && (StElF->ALSInfo.VersionSmStarter.Build))
				{
					if ((StElF->ALSInfo.VersionSmStarter.Version < UCfg->StCfg.VersionSmSt.Version) || ((StElF->ALSInfo.VersionSmStarter.Version == UCfg->StCfg.VersionSmSt.Version) && (StElF->ALSInfo.VersionSmStarter.Build < UCfg->StCfg.VersionSmSt.Build)))
					{
						Fl_UpDataSmStLAN = 1;
						UpDSmStStElFLAN = StElF;
						break;
					}
				}
				if ((StElF->ALSInfo.VersionFirmWare.Version < UCfg->StCfg.VersionFW.Version) || ((StElF->ALSInfo.VersionFirmWare.Version == UCfg->StCfg.VersionFW.Version) && (StElF->ALSInfo.VersionFirmWare.Build < UCfg->StCfg.VersionFW.Build)))
				{
					Fl_UpDataFWLAN = 1;
					UpDFWStElFLAN = StElF;
					break;
				}
			}
			StElF = (StElem*)LANStList.Next(FSTR);
		}
	}
	switch (Fl_UpDataAppLAN)
	{
	case 1:
		if (UpDAppStElFLAN)
		{
			if (AppUpDateHendleLAN > 0)
			{
				_close(AppUpDateHendleLAN); AppUpDateHendleLAN = 0;
			}
//			AppUpDateHendle = open(UCfg->StCfg.PathNameMainApp, O_BINARY | O_RDONLY, S_IREAD);
			_sopen_s(&AppUpDateHendleLAN, UCfg->StCfg.PathNameMainApp, _O_RDONLY | _O_BINARY, _SH_DENYNO, _S_IREAD);

			if (AppUpDateHendleLAN != -1)
			{
				ASPack.PHeader.Command = St_Send_BeginUpLoadFile;
				ASPack.PHeader.DataLen = sizeof(NetFileBeginStr);
				strcpy(((NetFileBeginStr*)&ASPack.Data)->File_Path, "\\Install.Tmp");
				strcpy(((NetFileBeginStr*)&ASPack.Data)->File_Name, "SoundLocator.exe");
				AppFile_TIDLAN = Srv->GetUID();
				((NetFileBeginStr*)&ASPack.Data)->File_TID = AppFile_TIDLAN;
				((NetFileBeginStr*)&ASPack.Data)->File_Type = 0;
				UpLFile_SizeLAN = filelength(AppUpDateHendleLAN); UpLTmpDataLenLAN = 0;
				((NetFileBeginStr*)&ASPack.Data)->File_Size = UpLFile_SizeLAN;
				LANSendPacket(UpDAppStElFLAN->NumCli, &ASPack);
				Fl_UpDataAppLAN++; WaitingCntLAN = 0; Fl_WiatingAnswerLAN = 1;
			}
			else
				Fl_UpDataAppLAN = 100;
		}
		else
			Fl_UpDataAppLAN = 0;
		break;
	case 2:
		switch (Fl_WiatingAnswerLAN)
		{
		case 0:
			Fl_UpDataAppLAN++;
			break;
		case 1:
			WaitingCntLAN++;
			if (WaitingCntLAN > 1000)
				Fl_UpDataAppLAN = 100; // Error!!!
			break;
		case 2:
			Fl_UpDataAppLAN =1;
			break;
		case 3:
		case 4:
			Fl_UpDataAppLAN = 100;
			break;
		}
		break;
	case 3:
		if ((UpDAppStElFLAN)&&(AppUpDateHendleLAN != -1))
		{
			CString	csText;
			ASPack.PHeader.Command = St_Send_UpLoadFile;
			((NetFileData*)&ASPack.Data)->File_TID = AppFile_TIDLAN;
			((NetFileData*)&ASPack.Data)->File_BlockLen = read(AppUpDateHendleLAN,((NetFileData*)&ASPack.Data)->File_Data, 10240);
			UpLTmpDataLenLAN += ((NetFileData*)&ASPack.Data)->File_BlockLen;
			ASPack.PHeader.DataLen = ((NetFileData*)&ASPack.Data)->File_BlockLen+12;	// Header
			if (ASPack.PHeader.DataLen)
				LANSendPacket(UpDAppStElFLAN->NumCli, &ASPack);
			csText.Format(_T("%0.2f"), 100.0*UpLTmpDataLenLAN /UpLFile_SizeLAN);
			m_ListLAN->SetItemText(UpDAppStElFLAN->Item, 6, csText);
			if (((NetFileData*)&ASPack.Data)->File_BlockLen == 10240)
			{
				Fl_UpDataAppLAN--; WaitingCntLAN = 0; Fl_WiatingAnswerLAN = 1;
			}
			else
			{
				Fl_UpDataAppLAN++; WaitingCntLAN = 0; Fl_WiatingAnswerLAN = 1;
			}
		}
		else
			Fl_UpDataAppLAN = 100;
		break;
	case 4:
		if ((UpDAppStElFLAN) && (AppUpDateHendleLAN != -1))
		{
			ASPack.PHeader.Command = St_Send_EndUpLoadFile;
			((NetFileEnd*)&ASPack.Data)->File_TID = AppFile_TIDLAN;
			((NetFileEnd*)&ASPack.Data)->File_CRC16 = 0;
			LANSendPacket(UpDAppStElFLAN->NumCli, &ASPack);
			Fl_UpDataAppLAN++;
			_close(AppUpDateHendleLAN); AppUpDateHendleLAN = 0;
		}
		else
			Fl_UpDataAppLAN = 0;
		break;
	case 5:
		if (UpDAppStElFLAN)
		{
			if ((Fl_UpDataSmStLAN == 0) && (Fl_UpDataFWLAN == 0))
			{
				ASPack.PHeader.Command = St_Send_RenameDir;
				ASPack.PHeader.DataLen = sizeof(NetRenameDirStr);
				strcpy(((NetRenameDirStr*)ASPack.Data)->File_PathOld, "\\Install.Tmp");
				strcpy(((NetRenameDirStr*)ASPack.Data)->File_PathNew, "\\Install");
				LANSendPacket(UpDAppStElFLAN->NumCli, &ASPack);
				UpDAppStElFLAN->ALSInfo.VersionMainApp.Version < UCfg->StCfg.VersionApp.Version;
				UpDAppStElFLAN->ALSInfo.VersionMainApp.Build < UCfg->StCfg.VersionApp.Build;
				ASPack.PHeader.Command = St_Send_Reset;
				ASPack.PHeader.DataLen = 0;
				LANSendPacket(UpDAppStElFLAN->NumCli, &ASPack);
			}
			Fl_UpDataAppLAN = 0;
		}
		else
			Fl_UpDataAppLAN = 0;
		break;
	case 100:
		if (AppUpDateHendleLAN > 0)
		{
			_close(AppUpDateHendleLAN); AppUpDateHendleLAN = 0;
		}
		Fl_UpDataAppLAN = 0;
		break;
	}

	switch (Fl_UpDataSmStLAN)
	{
	case 1:
		if (UpDSmStStElFLAN)
		{
			if (SmStUpDateHendleLAN > 0)
			{
				_close(SmStUpDateHendleLAN); SmStUpDateHendleLAN = 0;
			}

			_sopen_s(&SmStUpDateHendleLAN, UCfg->StCfg.PathNameSmStarter, _O_RDONLY | _O_BINARY, _SH_DENYNO, _S_IREAD);

			if (SmStUpDateHendleLAN != -1)
			{
				ASPack.PHeader.Command = St_Send_BeginUpLoadFile;
				ASPack.PHeader.DataLen = sizeof(NetFileBeginStr);
				strcpy(((NetFileBeginStr*)&ASPack.Data)->File_Path, "\\Install.Tmp");
				strcpy(((NetFileBeginStr*)&ASPack.Data)->File_Name, "SmartStarter.exe");
				SmStFile_TIDLAN = Srv->GetUID();
				((NetFileBeginStr*)&ASPack.Data)->File_TID = SmStFile_TIDLAN;
				((NetFileBeginStr*)&ASPack.Data)->File_Type = 0;
				UpLFile_SizeLAN = filelength(SmStUpDateHendleLAN); UpLTmpDataLenLAN = 0;
				((NetFileBeginStr*)&ASPack.Data)->File_Size = UpLFile_SizeLAN;
				LANSendPacket(UpDSmStStElFLAN->NumCli, &ASPack);
				Fl_UpDataSmStLAN++; WaitingCntLAN = 0; Fl_WiatingAnswerLAN = 1;
			}
			else
				Fl_UpDataSmStLAN = 100;
		}
		else
			Fl_UpDataSmStLAN = 0;
		break;
	case 2:
		switch (Fl_WiatingAnswerLAN)
		{
		case 0:
			Fl_UpDataSmStLAN++;
			break;
		case 1:
			WaitingCntLAN++;
			if (WaitingCntLAN > 1000)
				Fl_UpDataSmStLAN = 100; // Error!!!
			break;
		case 2:
			Fl_UpDataSmStLAN = 1;
			break;
		case 3:
		case 4:
			Fl_UpDataSmStLAN = 100;
			break;
		}
		break;
	case 3:
		if ((UpDSmStStElFLAN) && (SmStUpDateHendleLAN != -1))
		{
			CString	csText;
			ASPack.PHeader.Command = St_Send_UpLoadFile;
			((NetFileData*)&ASPack.Data)->File_TID = SmStFile_TIDLAN;
			((NetFileData*)&ASPack.Data)->File_BlockLen = read(SmStUpDateHendleLAN, ((NetFileData*)&ASPack.Data)->File_Data, 10240);
			UpLTmpDataLenLAN += ((NetFileData*)&ASPack.Data)->File_BlockLen;
			ASPack.PHeader.DataLen = ((NetFileData*)&ASPack.Data)->File_BlockLen + 12;	// Header
			if (ASPack.PHeader.DataLen)
				LANSendPacket(UpDSmStStElFLAN->NumCli, &ASPack);
			csText.Format(_T("%0.2f"), 100.0*UpLTmpDataLenLAN / UpLFile_SizeLAN);
			m_ListLAN->SetItemText(UpDSmStStElFLAN->Item, 6, csText);
			if (((NetFileData*)&ASPack.Data)->File_BlockLen == 10240)
			{
				Fl_UpDataSmStLAN--; WaitingCntLAN = 0; Fl_WiatingAnswerLAN = 1;
			}
			else
			{
				Fl_UpDataSmStLAN++; WaitingCntLAN = 0; Fl_WiatingAnswerLAN = 1;
			}
		}
		else
			Fl_UpDataSmStLAN = 100;
		break;
	case 4:
		if ((UpDSmStStElFLAN) && (SmStUpDateHendleLAN != -1))
		{
			ASPack.PHeader.Command = St_Send_EndUpLoadFile;
			((NetFileEnd*)&ASPack.Data)->File_TID = SmStFile_TIDLAN;
			((NetFileEnd*)&ASPack.Data)->File_CRC16 = 0;
			LANSendPacket(UpDSmStStElFLAN->NumCli, &ASPack);
			Fl_UpDataSmStLAN++;
			_close(SmStUpDateHendleLAN); SmStUpDateHendleLAN = 0;
		}
		else
			Fl_UpDataSmStLAN = 0;
		break;
	case 5:
		if (UpDSmStStElFLAN)
		{
			if ((Fl_UpDataAppLAN == 0) && (Fl_UpDataFWLAN == 0))
			{
				ASPack.PHeader.Command = St_Send_RenameDir;
				ASPack.PHeader.DataLen = sizeof(NetRenameDirStr);
				strcpy(((NetRenameDirStr*)ASPack.Data)->File_PathOld, "\\Install.Tmp");
				strcpy(((NetRenameDirStr*)ASPack.Data)->File_PathNew, "\\Install");
				LANSendPacket(UpDSmStStElFLAN->NumCli, &ASPack);
				UpDSmStStElFLAN->ALSInfo.VersionSmStarter.Version < UCfg->StCfg.VersionSmSt.Version;
				UpDSmStStElFLAN->ALSInfo.VersionSmStarter.Build < UCfg->StCfg.VersionSmSt.Build;
				ASPack.PHeader.Command = St_Send_Reset;
				ASPack.PHeader.DataLen = 0;
				LANSendPacket(UpDSmStStElFLAN->NumCli, &ASPack);
			}
			Fl_UpDataSmStLAN = 0;
		}
		else
			Fl_UpDataSmStLAN = 0;
		break;
	case 100:
		if (SmStUpDateHendleLAN > 0)
		{
			_close(SmStUpDateHendleLAN); SmStUpDateHendleLAN = 0;
		}
		Fl_UpDataSmStLAN = 0;
		break;
	}

	switch (Fl_UpDataFWLAN)
	{
	case 1:
		if (UpDFWStElFLAN)
		{
			if (FWUpDateHendleLAN > 0)
			{
				_close(FWUpDateHendleLAN); FWUpDateHendleLAN = 0;
			}

			_sopen_s(&FWUpDateHendleLAN, UCfg->StCfg.PathNameFirmWare, _O_RDONLY | _O_BINARY, _SH_DENYNO, _S_IREAD);

			if (FWUpDateHendleLAN != -1)
			{
				int i;
				ASPack.PHeader.Command = St_Send_BeginUpLoadFile;
				ASPack.PHeader.DataLen = sizeof(NetFileBeginStr);
				strcpy(((NetFileBeginStr*)&ASPack.Data)->File_Path, "\\FirmWare");
				for (i = strlen(UCfg->StCfg.PathNameFirmWare); i > 0; i--)
				{
					if (UCfg->StCfg.PathNameFirmWare[i]=='\\')
						break;
				}
				strcpy(((NetFileBeginStr*)&ASPack.Data)->File_Name, &UCfg->StCfg.PathNameFirmWare[i+1]);
				FWFile_TIDLAN = Srv->GetUID();
				((NetFileBeginStr*)&ASPack.Data)->File_TID = FWFile_TIDLAN;
				((NetFileBeginStr*)&ASPack.Data)->File_Type = 0;
				UpLFile_SizeLAN = filelength(FWUpDateHendleLAN); UpLTmpDataLenLAN = 0;
				((NetFileBeginStr*)&ASPack.Data)->File_Size = UpLFile_SizeLAN;
				LANSendPacket(UpDFWStElFLAN->NumCli, &ASPack);
				Fl_UpDataFWLAN++; WaitingCntLAN = 0; Fl_WiatingAnswerLAN = 1;
			}
			else
				Fl_UpDataFWLAN = 100;
		}
		else
			Fl_UpDataFWLAN = 0;
		break;
	case 2:
		switch (Fl_WiatingAnswerLAN)
		{
		case 0:
			Fl_UpDataFWLAN++;
			break;
		case 1:
			WaitingCntLAN++;
			if (WaitingCntLAN > 1000)
				Fl_UpDataFWLAN = 100; // Error!!!
			break;
		case 2:
			Fl_UpDataFWLAN = 1;
			break;
		case 3:
		case 4:
			Fl_UpDataFWLAN = 100;
			break;
		}
		break;
	case 3:
		if ((UpDFWStElFLAN) && (FWUpDateHendleLAN != -1))
		{
			CString	csText;
			ASPack.PHeader.Command = St_Send_UpLoadFile;
			((NetFileData*)&ASPack.Data)->File_TID = FWFile_TIDLAN;
			((NetFileData*)&ASPack.Data)->File_BlockLen = read(FWUpDateHendleLAN, ((NetFileData*)&ASPack.Data)->File_Data, 10240);
			UpLTmpDataLenLAN += ((NetFileData*)&ASPack.Data)->File_BlockLen;
			ASPack.PHeader.DataLen = ((NetFileData*)&ASPack.Data)->File_BlockLen + 12;	// Header
			if (ASPack.PHeader.DataLen)
				LANSendPacket(UpDFWStElFLAN->NumCli, &ASPack);
			csText.Format(_T("%0.2f"), 100.0*UpLTmpDataLenLAN / UpLFile_SizeLAN);
			m_ListLAN->SetItemText(UpDFWStElFLAN->Item, 6, csText);
			if (((NetFileData*)&ASPack.Data)->File_BlockLen == 10240)
			{
				Fl_UpDataFWLAN--; WaitingCntLAN = 0; Fl_WiatingAnswerLAN = 1;
			}
			else
			{
				Fl_UpDataFWLAN++; WaitingCntLAN = 0; Fl_WiatingAnswerLAN = 1;
			}
		}
		else
			Fl_UpDataFWLAN = 100;
		break;
	case 4:
		if ((UpDFWStElFLAN) && (FWUpDateHendleLAN != -1))
		{
			ASPack.PHeader.Command = St_Send_EndUpLoadFile;
			((NetFileEnd*)&ASPack.Data)->File_TID = FWFile_TIDLAN;
			((NetFileEnd*)&ASPack.Data)->File_CRC16 = 0;
			LANSendPacket(UpDFWStElFLAN->NumCli, &ASPack);
			Fl_UpDataFWLAN++;
			_close(FWUpDateHendleLAN); FWUpDateHendleLAN = 0;
		}
		else
			Fl_UpDataFWLAN = 0;
		break;
	case 5:
		if (UpDFWStElFLAN)
		{
			if ((Fl_UpDataSmStLAN == 0) && (Fl_UpDataAppLAN == 0))
			{
//				ASPack.PHeader.Command = St_Send_RenameDir;
//				ASPack.PHeader.DataLen = sizeof(NetRenameDirStr);
//				strcpy(((NetRenameDirStr*)ASPack.Data)->File_PathOld, "\\Install.Tmp");
//				strcpy(((NetRenameDirStr*)ASPack.Data)->File_PathNew, "\\Install");
//				SendPacket(UpDFWStElF->NumCli, &ASPack);

				UpDFWStElFLAN->ALSInfo.VersionFirmWare.Version < UCfg->StCfg.VersionFW.Version;
				UpDFWStElFLAN->ALSInfo.VersionFirmWare.Build < UCfg->StCfg.VersionFW.Build;
				ASPack.PHeader.Command = St_Send_Reset;
				ASPack.PHeader.DataLen = 0;
				LANSendPacket(UpDFWStElFLAN->NumCli, &ASPack);
			}
			Fl_UpDataFWLAN = 0;
		}
		else
			Fl_UpDataFWLAN = 0;
		break;
	case 100:
		if (FWUpDateHendleLAN > 0)
		{
			_close(FWUpDateHendleLAN); FWUpDateHendleLAN = 0;
		}
		Fl_UpDataFWLAN = 0;
		break;
	}
// WAN 
	if ((Fl_UpDataAppWAN == 0) && (Fl_UpDataSmStWAN == 0) && (Fl_UpDataFWWAN == 0))
	{
		StElF = (StElem*)WANStList.Begin(FSTR);
		while (StElF)
		{
			if (StElF->Fl_Active)
			{
				if ((StElF->ALSInfo.VersionMainApp.Version < UCfg->StCfg.VersionApp.Version) || ((StElF->ALSInfo.VersionMainApp.Version == UCfg->StCfg.VersionApp.Version) && (StElF->ALSInfo.VersionMainApp.Build < UCfg->StCfg.VersionApp.Build)))
				{
					Fl_UpDataAppWAN = 1;
					UpDAppStElFWAN = StElF;
					break;
				}
				if ((StElF->ALSInfo.VersionSmStarter.Version) && (StElF->ALSInfo.VersionSmStarter.Build))
				{
					if ((StElF->ALSInfo.VersionSmStarter.Version < UCfg->StCfg.VersionSmSt.Version) || ((StElF->ALSInfo.VersionSmStarter.Version == UCfg->StCfg.VersionSmSt.Version) && (StElF->ALSInfo.VersionSmStarter.Build < UCfg->StCfg.VersionSmSt.Build)))
					{
						Fl_UpDataSmStWAN = 1;
						UpDSmStStElFWAN = StElF;
						break;
					}
				}
				if ((StElF->ALSInfo.VersionFirmWare.Version < UCfg->StCfg.VersionFW.Version) || ((StElF->ALSInfo.VersionFirmWare.Version == UCfg->StCfg.VersionFW.Version) && (StElF->ALSInfo.VersionFirmWare.Build < UCfg->StCfg.VersionFW.Build)))
				{
					Fl_UpDataFWWAN = 1;
					UpDFWStElFWAN = StElF;
					break;
				}
			}
			StElF = (StElem*)WANStList.Next(FSTR);
		}
	}

#endif
/*
	switch (Fl_UpDataAppWAN)
	{
	case 1:
		if (UpDAppStElFWAN)
		{
			if (AppUpDateHendleWAN > 0)
			{
				_close(AppUpDateHendleWAN); AppUpDateHendleWAN = 0;
			}
			//			AppUpDateHendle = open(UCfg->StCfg.PathNameMainApp, O_BINARY | O_RDONLY, S_IREAD);
			_sopen_s(&AppUpDateHendleWAN, UCfg->StCfg.PathNameMainApp, _O_RDONLY | _O_BINARY, _SH_DENYNO, _S_IREAD);

			if (AppUpDateHendleWAN != -1)
			{
				ASPack.PHeader.Command = St_Send_BeginUpLoadFile;
				ASPack.PHeader.DataLen = sizeof(NetFileBeginStr);
				strcpy(((NetFileBeginStr*)&ASPack.Data)->File_Path, "\\Install.Tmp");
				strcpy(((NetFileBeginStr*)&ASPack.Data)->File_Name, "SoundLocator.exe");
				AppFile_TIDWAN = Srv->GetUID();
				((NetFileBeginStr*)&ASPack.Data)->File_TID = AppFile_TIDWAN;
				((NetFileBeginStr*)&ASPack.Data)->File_Type = 0;
				UpLFile_SizeWAN = filelength(AppUpDateHendleWAN); UpLTmpDataLenWAN = 0;
				((NetFileBeginStr*)&ASPack.Data)->File_Size = UpLFile_SizeWAN;
				WANSendPacket(UpDAppStElFWAN->NumCli, &ASPack);
				Fl_UpDataAppWAN++; WaitingCntWAN = 0; Fl_WiatingAnswerWAN = 1;
			}
			else
				Fl_UpDataAppWAN = 100;
		}
		else
			Fl_UpDataAppWAN = 0;
		break;
	case 2:
		switch (Fl_WiatingAnswerWAN)
		{
		case 0:
			Fl_UpDataAppWAN++;
			break;
		case 1:
			WaitingCntWAN++;
			if (WaitingCntWAN > 1000)
				Fl_UpDataAppWAN = 100; // Error!!!
			break;
		case 2:
			Fl_UpDataAppWAN = 1;
			break;
		case 3:
		case 4:
			Fl_UpDataAppWAN = 100;
			break;
		}
		break;
	case 3:
		if ((UpDAppStElFWAN) && (AppUpDateHendleWAN != -1))
		{
			CString	csText;
			ASPack.PHeader.Command = St_Send_UpLoadFile;
			((NetFileData*)&ASPack.Data)->File_TID = AppFile_TIDWAN;
			((NetFileData*)&ASPack.Data)->File_BlockLen = read(AppUpDateHendleWAN, ((NetFileData*)&ASPack.Data)->File_Data, 10240);
			UpLTmpDataLenWAN += ((NetFileData*)&ASPack.Data)->File_BlockLen;
			ASPack.PHeader.DataLen = ((NetFileData*)&ASPack.Data)->File_BlockLen + 12;	// Header
			if (ASPack.PHeader.DataLen)
				WANSendPacket(UpDAppStElFWAN->NumCli, &ASPack);
			csText.Format(_T("%0.2f"), 100.0*UpLTmpDataLenWAN / UpLFile_SizeWAN);
			m_ListWAN->SetItemText(UpDAppStElFWAN->Item, 6, csText);
			if (((NetFileData*)&ASPack.Data)->File_BlockLen == 10240)
			{
				Fl_UpDataAppWAN--; WaitingCntWAN = 0; Fl_WiatingAnswerWAN = 1;
			}
			else
			{
				Fl_UpDataAppWAN++; WaitingCntWAN = 0; Fl_WiatingAnswerWAN = 1;
			}
		}
		else
			Fl_UpDataAppWAN = 100;
		break;
	case 4:
		if ((UpDAppStElFWAN) && (AppUpDateHendleWAN != -1))
		{
			ASPack.PHeader.Command = St_Send_EndUpLoadFile;
			((NetFileEnd*)&ASPack.Data)->File_TID = AppFile_TIDWAN;
			((NetFileEnd*)&ASPack.Data)->File_CRC16 = 0;
			WANSendPacket(UpDAppStElFWAN->NumCli, &ASPack);
			Fl_UpDataAppWAN++;
			_close(AppUpDateHendleWAN); AppUpDateHendleWAN = 0;
		}
		else
			Fl_UpDataAppWAN = 0;
		break;
	case 5:
		if (UpDAppStElFWAN)
		{
			if ((Fl_UpDataSmStWAN == 0) && (Fl_UpDataFWWAN == 0))
			{
				ASPack.PHeader.Command = St_Send_RenameDir;
				ASPack.PHeader.DataLen = sizeof(NetRenameDirStr);
				strcpy(((NetRenameDirStr*)ASPack.Data)->File_PathOld, "\\Install.Tmp");
				strcpy(((NetRenameDirStr*)ASPack.Data)->File_PathNew, "\\Install");
				WANSendPacket(UpDAppStElFWAN->NumCli, &ASPack);
				UpDAppStElFWAN->ALSInfo.VersionMainApp.Version < UCfg->StCfg.VersionApp.Version;
				UpDAppStElFWAN->ALSInfo.VersionMainApp.Build < UCfg->StCfg.VersionApp.Build;
				ASPack.PHeader.Command = St_Send_Reset;
				ASPack.PHeader.DataLen = 0;
				WANSendPacket(UpDAppStElFWAN->NumCli, &ASPack);
			}
			Fl_UpDataAppWAN = 0;
		}
		else
			Fl_UpDataAppWAN = 0;
		break;
	case 100:
		if (AppUpDateHendleWAN > 0)
		{
			_close(AppUpDateHendleWAN); AppUpDateHendleWAN = 0;
		}
		Fl_UpDataAppWAN = 0;
		break;
	}
*/
/*
	switch (Fl_UpDataSmStWAN)
	{
	case 1:
		if (UpDSmStStElFWAN)
		{
			if (SmStUpDateHendleWAN > 0)
			{
				_close(SmStUpDateHendleWAN); SmStUpDateHendleWAN = 0;
			}

			_sopen_s(&SmStUpDateHendleWAN, UCfg->StCfg.PathNameSmStarter, _O_RDONLY | _O_BINARY, _SH_DENYNO, _S_IREAD);

			if (SmStUpDateHendleWAN != -1)
			{
				ASPack.PHeader.Command = St_Send_BeginUpLoadFile;
				ASPack.PHeader.DataLen = sizeof(NetFileBeginStr);
				strcpy(((NetFileBeginStr*)&ASPack.Data)->File_Path, "\\Install.Tmp");
				strcpy(((NetFileBeginStr*)&ASPack.Data)->File_Name, "SmartStarter.exe");
				SmStFile_TIDWAN = Srv->GetUID();
				((NetFileBeginStr*)&ASPack.Data)->File_TID = SmStFile_TIDWAN;
				((NetFileBeginStr*)&ASPack.Data)->File_Type = 0;
				UpLFile_SizeWAN = filelength(SmStUpDateHendleWAN); UpLTmpDataLenWAN = 0;
				((NetFileBeginStr*)&ASPack.Data)->File_Size = UpLFile_SizeWAN;
				WANSendPacket(UpDSmStStElFWAN->NumCli, &ASPack);
				Fl_UpDataSmStWAN++; WaitingCntWAN = 0; Fl_WiatingAnswerWAN = 1;
			}
			else
				Fl_UpDataSmStWAN = 100;
		}
		else
			Fl_UpDataSmStWAN = 0;
		break;
	case 2:
		switch (Fl_WiatingAnswerWAN)
		{
		case 0:
			Fl_UpDataSmStWAN++;
			break;
		case 1:
			WaitingCntWAN++;
			if (WaitingCntWAN > 1000)
				Fl_UpDataSmStWAN = 100; // Error!!!
			break;
		case 2:
			Fl_UpDataSmStWAN = 1;
			break;
		case 3:
		case 4:
			Fl_UpDataSmStWAN = 100;
			break;
		}
		break;
	case 3:
		if ((UpDSmStStElFWAN) && (SmStUpDateHendleWAN != -1))
		{
			CString	csText;
			ASPack.PHeader.Command = St_Send_UpLoadFile;
			((NetFileData*)&ASPack.Data)->File_TID = SmStFile_TIDWAN;
			((NetFileData*)&ASPack.Data)->File_BlockLen = read(SmStUpDateHendleWAN, ((NetFileData*)&ASPack.Data)->File_Data, 10240);
			UpLTmpDataLenWAN += ((NetFileData*)&ASPack.Data)->File_BlockLen;
			ASPack.PHeader.DataLen = ((NetFileData*)&ASPack.Data)->File_BlockLen + 12;	// Header
			if (ASPack.PHeader.DataLen)
				WANSendPacket(UpDSmStStElFWAN->NumCli, &ASPack);
			csText.Format(_T("%0.2f"), 100.0*UpLTmpDataLenWAN / UpLFile_SizeWAN);
			m_ListWAN->SetItemText(UpDSmStStElFWAN->Item, 6, csText);
			if (((NetFileData*)&ASPack.Data)->File_BlockLen == 10240)
			{
				Fl_UpDataSmStWAN--; WaitingCntWAN = 0; Fl_WiatingAnswerWAN = 1;
			}
			else
			{
				Fl_UpDataSmStWAN++; WaitingCntWAN = 0; Fl_WiatingAnswerWAN = 1;
			}
		}
		else
			Fl_UpDataSmStWAN = 100;
		break;
	case 4:
		if ((UpDSmStStElFWAN) && (SmStUpDateHendleWAN != -1))
		{
			ASPack.PHeader.Command = St_Send_EndUpLoadFile;
			((NetFileEnd*)&ASPack.Data)->File_TID = SmStFile_TIDWAN;
			((NetFileEnd*)&ASPack.Data)->File_CRC16 = 0;
			WANSendPacket(UpDSmStStElFWAN->NumCli, &ASPack);
			Fl_UpDataSmStWAN++;
			_close(SmStUpDateHendleWAN); SmStUpDateHendleWAN = 0;
		}
		else
			Fl_UpDataSmStWAN = 0;
		break;
	case 5:
		if (UpDSmStStElFWAN)
		{
			if ((Fl_UpDataAppWAN == 0) && (Fl_UpDataFWWAN == 0))
			{
				ASPack.PHeader.Command = St_Send_RenameDir;
				ASPack.PHeader.DataLen = sizeof(NetRenameDirStr);
				strcpy(((NetRenameDirStr*)ASPack.Data)->File_PathOld, "\\Install.Tmp");
				strcpy(((NetRenameDirStr*)ASPack.Data)->File_PathNew, "\\Install");
				WANSendPacket(UpDSmStStElFWAN->NumCli, &ASPack);
				UpDSmStStElFWAN->ALSInfo.VersionSmStarter.Version < UCfg->StCfg.VersionSmSt.Version;
				UpDSmStStElFWAN->ALSInfo.VersionSmStarter.Build < UCfg->StCfg.VersionSmSt.Build;
				ASPack.PHeader.Command = St_Send_Reset;
				ASPack.PHeader.DataLen = 0;
				WANSendPacket(UpDSmStStElFWAN->NumCli, &ASPack);
			}
			Fl_UpDataSmStWAN = 0;
		}
		else
			Fl_UpDataSmStWAN = 0;
		break;
	case 100:
		if (SmStUpDateHendleWAN > 0)
		{
			_close(SmStUpDateHendleWAN); SmStUpDateHendleWAN = 0;
		}
		Fl_UpDataSmStWAN = 0;
		break;
	}
*/
/*
	switch (Fl_UpDataFWWAN)
	{
	case 1:
		if (UpDFWStElFWAN)
		{
			if (FWUpDateHendleWAN > 0)
			{
				_close(FWUpDateHendleWAN); FWUpDateHendleWAN = 0;
			}

			_sopen_s(&FWUpDateHendleWAN, UCfg->StCfg.PathNameFirmWare, _O_RDONLY | _O_BINARY, _SH_DENYNO, _S_IREAD);

			if (FWUpDateHendleWAN != -1)
			{
				int i;
				ASPack.PHeader.Command = St_Send_BeginUpLoadFile;
				ASPack.PHeader.DataLen = sizeof(NetFileBeginStr);
				strcpy(((NetFileBeginStr*)&ASPack.Data)->File_Path, "\\FirmWare");
				for (i = strlen(UCfg->StCfg.PathNameFirmWare); i > 0; i--)
				{
					if (UCfg->StCfg.PathNameFirmWare[i] == '\\')
						break;
				}
				strcpy(((NetFileBeginStr*)&ASPack.Data)->File_Name, &UCfg->StCfg.PathNameFirmWare[i + 1]);
				FWFile_TIDWAN = Srv->GetUID();
				((NetFileBeginStr*)&ASPack.Data)->File_TID = FWFile_TIDWAN;
				((NetFileBeginStr*)&ASPack.Data)->File_Type = 0;
				UpLFile_SizeWAN = filelength(FWUpDateHendleWAN); UpLTmpDataLenWAN = 0;
				((NetFileBeginStr*)&ASPack.Data)->File_Size = UpLFile_SizeWAN;
				WANSendPacket(UpDFWStElFWAN->NumCli, &ASPack);
				Fl_UpDataFWWAN++; WaitingCntWAN = 0; Fl_WiatingAnswerWAN = 1;
			}
			else
				Fl_UpDataFWWAN = 100;
		}
		else
			Fl_UpDataFWWAN = 0;
		break;
	case 2:
		switch (Fl_WiatingAnswerWAN)
		{
		case 0:
			Fl_UpDataFWWAN++;
			break;
		case 1:
			WaitingCntWAN++;
			if (WaitingCntWAN > 1000)
				Fl_UpDataFWWAN = 100; // Error!!!
			break;
		case 2:
			Fl_UpDataFWWAN = 1;
			break;
		case 3:
		case 4:
			Fl_UpDataFWWAN = 100;
			break;
		}
		break;
	case 3:
		if ((UpDFWStElFWAN) && (FWUpDateHendleWAN != -1))
		{
			CString	csText;
			ASPack.PHeader.Command = St_Send_UpLoadFile;
			((NetFileData*)&ASPack.Data)->File_TID = FWFile_TIDWAN;
			((NetFileData*)&ASPack.Data)->File_BlockLen = read(FWUpDateHendleWAN, ((NetFileData*)&ASPack.Data)->File_Data, 10240);
			UpLTmpDataLenWAN += ((NetFileData*)&ASPack.Data)->File_BlockLen;
			ASPack.PHeader.DataLen = ((NetFileData*)&ASPack.Data)->File_BlockLen + 12;	// Header
			if (ASPack.PHeader.DataLen)
				WANSendPacket(UpDFWStElFWAN->NumCli, &ASPack);
			csText.Format(_T("%0.2f"), 100.0*UpLTmpDataLenWAN / UpLFile_SizeWAN);
			m_ListWAN->SetItemText(UpDFWStElFWAN->Item, 6, csText);
			if (((NetFileData*)&ASPack.Data)->File_BlockLen == 10240)
			{
				Fl_UpDataFWWAN--; WaitingCntWAN = 0; Fl_WiatingAnswerWAN = 1;
			}
			else
			{
				Fl_UpDataFWWAN++; WaitingCntWAN = 0; Fl_WiatingAnswerWAN = 1;
			}
		}
		else
			Fl_UpDataFWWAN = 100;
		break;
	case 4:
		if ((UpDFWStElFWAN) && (FWUpDateHendleWAN != -1))
		{
			ASPack.PHeader.Command = St_Send_EndUpLoadFile;
			((NetFileEnd*)&ASPack.Data)->File_TID = FWFile_TIDWAN;
			((NetFileEnd*)&ASPack.Data)->File_CRC16 = 0;
			WANSendPacket(UpDFWStElFWAN->NumCli, &ASPack);
			Fl_UpDataFWWAN++;
			_close(FWUpDateHendleWAN); FWUpDateHendleWAN = 0;
		}
		else
			Fl_UpDataFWWAN = 0;
		break;
	case 5:
		if (UpDFWStElFWAN)
		{
			if ((Fl_UpDataSmStWAN == 0) && (Fl_UpDataAppWAN == 0))
			{
				//				ASPack.PHeader.Command = St_Send_RenameDir;
				//				ASPack.PHeader.DataLen = sizeof(NetRenameDirStr);
				//				strcpy(((NetRenameDirStr*)ASPack.Data)->File_PathOld, "\\Install.Tmp");
				//				strcpy(((NetRenameDirStr*)ASPack.Data)->File_PathNew, "\\Install");
				//				SendPacket(UpDFWStElF->NumCli, &ASPack);

				UpDFWStElFWAN->ALSInfo.VersionFirmWare.Version < UCfg->StCfg.VersionFW.Version;
				UpDFWStElFWAN->ALSInfo.VersionFirmWare.Build < UCfg->StCfg.VersionFW.Build;
				ASPack.PHeader.Command = St_Send_Reset;
				ASPack.PHeader.DataLen = 0;
				WANSendPacket(UpDFWStElFWAN->NumCli, &ASPack);
			}
			Fl_UpDataFWWAN = 0;
		}
		else
			Fl_UpDataFWWAN = 0;
		break;
	case 100:
		if (FWUpDateHendleWAN > 0)
		{
			_close(FWUpDateHendleWAN); FWUpDateHendleWAN = 0;
		}
		Fl_UpDataFWWAN = 0;
		break;
	}
*/
	CDialogEx::OnTimer(nIDEvent);
}


void CSoundServerDlg::OnEnChangeEdit3()
{
	// TODO:  Если это элемент управления RICHEDIT, то элемент управления не будет
	// send this notification unless you override the CDialogEx::OnInitDialog()
	// функция и вызов CRichEditCtrl().SetEventMask()
	// with the ENM_CHANGE flag ORed into the mask.

	// TODO:  Добавьте код элемента управления
	CWnd		*win;
	CString		s;
	size_t		len;
	char		TBuf[16];
	win = GetDlgItem(IDC_EDIT3);
	win->GetWindowText(s);
	wcstombs_s(&len, TBuf, 16, s, 16);
	UCfg->StCfg.Dev[ActiveDevEl->HW_ID].X = atof(TBuf);
}


void CSoundServerDlg::OnEnChangeEdit4()
{
	// TODO:  Если это элемент управления RICHEDIT, то элемент управления не будет
	// send this notification unless you override the CDialogEx::OnInitDialog()
	// функция и вызов CRichEditCtrl().SetEventMask()
	// with the ENM_CHANGE flag ORed into the mask.

	// TODO:  Добавьте код элемента управления
	CWnd		*win;
	CString		s;
	size_t		len;
	char		TBuf[16];
	win = GetDlgItem(IDC_EDIT4);
	win->GetWindowText(s);
	wcstombs_s(&len, TBuf, 16, s, 16);
	UCfg->StCfg.Dev[ActiveDevEl->HW_ID].Y = atof(TBuf);
}


void CSoundServerDlg::OnBnClickedButton1()
{
	// TODO: добавьте свой код обработчика уведомлений
/*
	if (ActiveStElLAN)
	{
		ASPack.PHeader.Command = St_Send_Reset;
		ASPack.PHeader.DataLen = 0;
		LANSendPacket(ActiveStElLAN->NumCli, &ASPack);
	}
*/
}


void CSoundServerDlg::OnBnClickedButton2()
{
	// TODO: Add your control notification handler code here
	CWnd		*win;
	CString		s;
//	if (ActiveDevEl)
	{
		if (Fl_UPD == 0)
			Fl_UPD = 1;
		else
		{
			Fl_UPD = 0;
			win = GetDlgItem(IDC_BUTTON2);
			s.Format(_T("UpDate"));
			win->SetWindowText(s);
		}
	}

/*
	if (ActiveStElLAN)
	{
		ASPack.PHeader.Command = St_Set_StationNewID;
		ASPack.PHeader.DataLen = sizeof(UINT64);
		*((UINT64*)ASPack.Data) = UCfg->StCfg.StationIDLAN++;
		LANSendPacket(ActiveStElLAN->NumCli, &ASPack);
		ASPack.PHeader.Command = St_Send_Reset;
		ASPack.PHeader.DataLen = 0;
		LANSendPacket(ActiveStElLAN->NumCli, &ASPack);

		win = GetDlgItem(IDC_EDIT5);
		//	s.Format(_T("0x%016I64x"), UCfg->StCfg.StationID);
		s.Format(_T("%I64d"), UCfg->StCfg.StationIDLAN);
		win->SetWindowText(s);

	}
*/
}


void CSoundServerDlg::OnEnChangeEdit5()
{
	// TODO:  If this is a RICHEDIT control, the control will not
	// send this notification unless you override the CDialogEx::OnInitDialog()
	// function and call CRichEditCtrl().SetEventMask()
	// with the ENM_CHANGE flag ORed into the mask.

	// TODO:  Add your control notification handler code here
	char		tBuf[128];
	CWnd		*win;
	CString		s;
	size_t		len;
	char		TBuf[32];
	win = GetDlgItem(IDC_EDIT5);
	win->GetWindowText(s);
	wcstombs_s(&len, TBuf, 32, s, 32);
	UCfg->StCfg.Speed = atoi(TBuf);
	sprintf(tBuf, "S1=%d\n\r", UCfg->StCfg.Speed);
	if (DInt->ElUDPStepper)
		DInt->ElUDPStepper->SendTo((UCHAR*)tBuf, strlen(tBuf), (sockaddr*)&AddrToStepper);

}


void CSoundServerDlg::OnBnClickedButton3()
{
	// TODO: Add your control notification handler code here
	CWnd		*win;
	CString		s;
/*
	if (ActiveStElLAN)
	{
		FindStrList8	FSTR;
		StElem*			StElF;
		int				Item = 0;

		if (ActiveStElLAN->Fl_Active)
		{
			ASPack.PHeader.Command = St_Set_StationNewID;
			ASPack.PHeader.DataLen = sizeof(UINT64);
			*((UINT64*)ASPack.Data) = 0;
			LANSendPacket(ActiveStElLAN->NumCli, &ASPack);
			ASPack.PHeader.Command = St_Send_Reset;
			ASPack.PHeader.DataLen = 0;
			LANSendPacket(ActiveStElLAN->NumCli, &ASPack);
		}
		StElF = (StElem*)LANStList.Begin(FSTR);
		while (StElF)
		{
			if (StElF->Item>ActiveStElLAN->Item)
				StElF->Item = Item;
			if (StElF->Item != ActiveStElLAN->Item)
				Item++;
			StElF = (StElem*)LANStList.Next(FSTR);
		}
		m_ListLAN->DeleteItem(ActiveStElLAN->Item);
		LANStList.Del(ActiveStElLAN); ActiveStElLAN = 0;
	}
*/
}


void CSoundServerDlg::OnEnChangeEdit6()
{
	// TODO:  If this is a RICHEDIT control, the control will not
	// send this notification unless you override the CDialogEx::OnInitDialog()
	// function and call CRichEditCtrl().SetEventMask()
	// with the ENM_CHANGE flag ORed into the mask.

	// TODO:  Add your control notification handler code here
	CWnd* win;
	CString		s;
	size_t		len;
	char		TBuf[32];
	win = GetDlgItem(IDC_EDIT6);
	win->GetWindowText(s);
	wcstombs_s(&len, TBuf, 32, s, 32);
	UCfg->StCfg.Step = atoi(TBuf);

}


void CSoundServerDlg::OnLvnItemchangedList2(NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMLISTVIEW pNMLV = reinterpret_cast<LPNMLISTVIEW>(pNMHDR);
/*
	// TODO: Add your control notification handler code here
	CWnd			*win;
	CString			s;
	size_t			len;
	StElem			*StElF;
	FindStrList8	FSTR;
	ActiveSt = pNMLV->iItem;
	//	win = GetDlgItem(IDC_EDIT1);
	//	win->SetWindowText(s);
	StElF = (StElem*)WANStList.Begin(FSTR);
	while (StElF)
	{
		if (StElF->Item == ActiveSt)
		{
			ActiveStElWAN = StElF;
			break;
		}
		StElF = (StElem*)WANStList.Next(FSTR);
	}
*/
	*pResult = 0;
}


void CSoundServerDlg::OnEnChangeEdit7()
{
	// TODO:  If this is a RICHEDIT control, the control will not
	// send this notification unless you override the CDialogEx::OnInitDialog()
	// function and call CRichEditCtrl().SetEventMask()
	// with the ENM_CHANGE flag ORed into the mask.

	// TODO:  Add your control notification handler code here
	CWnd* win;
	char		tmpBuf[16];
	CString		s;
	size_t	len;
	win = GetDlgItem(IDC_EDIT7);
	win->GetWindowText(s);
	wcstombs_s(&len, tmpBuf, 16, s, 16);
	UCfg->StCfg.IP_Stepper = inet_addr(tmpBuf);
}


void CSoundServerDlg::OnBnClickedButton6()
{
	// TODO: Add your control notification handler code here
	CWnd		*win;
	CString		s;
/*
	if (ActiveStElWAN)
	{
		FindStrList8	FSTR;
		StElem*			StElF;
		int				Item = 0;

		if (ActiveStElWAN->Fl_Active)
		{
			ASPack.PHeader.Command = St_Set_StationNewID;
			ASPack.PHeader.DataLen = sizeof(UINT64);
			*((UINT64*)ASPack.Data) = 0;
			WANSendPacket(ActiveStElWAN->NumCli, &ASPack);
			ASPack.PHeader.Command = St_Send_Reset;
			ASPack.PHeader.DataLen = 0;
			WANSendPacket(ActiveStElWAN->NumCli, &ASPack);
		}
		StElF = (StElem*)WANStList.Begin(FSTR);
		while (StElF)
		{
			if (StElF->Item > ActiveStElWAN->Item)
				StElF->Item = Item;
			if (StElF->Item != ActiveStElWAN->Item)
				Item++;
			StElF = (StElem*)WANStList.Next(FSTR);
		}
		m_ListWAN->DeleteItem(ActiveStElWAN->Item);
		WANStList.Del(ActiveStElWAN); ActiveStElWAN = 0;
	}
*/
}


void CSoundServerDlg::OnBnClickedButton4()
{
	// TODO: Add your control notification handler code here
/*
	if (ActiveStElWAN)
	{
		ASPack.PHeader.Command = St_Send_Reset;
		ASPack.PHeader.DataLen = 0;
		WANSendPacket(ActiveStElWAN->NumCli, &ASPack);
	}
*/
}


void CSoundServerDlg::OnBnClickedButton5()
{
	// TODO: Add your control notification handler code here
	CWnd		*win;
	CString		s;
/*
	if (ActiveStElWAN)
	{
		ASPack.PHeader.Command = St_Set_StationNewID;
		ASPack.PHeader.DataLen = sizeof(UINT64);
		*((UINT64*)ASPack.Data) = UCfg->StCfg.StationIDWAN++;
		WANSendPacket(ActiveStElWAN->NumCli, &ASPack);
		ASPack.PHeader.Command = St_Send_Reset;
		ASPack.PHeader.DataLen = 0;
		WANSendPacket(ActiveStElWAN->NumCli, &ASPack);

		win = GetDlgItem(IDC_EDIT7);
		//	s.Format(_T("0x%016I64x"), UCfg->StCfg.StationID);
		s.Format(_T("%I64d"), UCfg->StCfg.StationIDWAN);
		win->SetWindowText(s);
	}
*/
}


void CSoundServerDlg::OnEnChangeEdit8()
{
	// TODO:  If this is a RICHEDIT control, the control will not
	// send this notification unless you override the CDialogEx::OnInitDialog()
	// function and call CRichEditCtrl().SetEventMask()
	// with the ENM_CHANGE flag ORed into the mask.

	// TODO:  Add your control notification handler code here
	CWnd* win;
	CString		s;
	size_t		len;
	char		TBuf[16];
	win = GetDlgItem(IDC_EDIT8);
	win->GetWindowText(s);
	wcstombs_s(&len, TBuf, 16, s, 16);
	if (ActiveDevEl)
		UCfg->StCfg.Dev[ActiveDevEl->HW_ID].AthimuthCorrection = atof(TBuf);
}


void CSoundServerDlg::OnBnClickedCheck1()
{
	// TODO: Add your control notification handler code here
	CButton*	pBox;
	pBox = (CButton*)GetDlgItem(IDC_CHECK1);
	UCfg->StCfg.Fl_UsedLANList = pBox->GetCheck();
}


void CSoundServerDlg::OnBnClickedCheck2()
{
	// TODO: Add your control notification handler code here
	CButton* pBox;
	pBox = (CButton*)GetDlgItem(IDC_CHECK2);
	UCfg->StCfg.Fl_UseGeoData = pBox->GetCheck();
}


void CSoundServerDlg::OnEnChangeEdit11()
{
	// TODO:  If this is a RICHEDIT control, the control will not
	// send this notification unless you override the CDialogEx::OnInitDialog()
	// function and call CRichEditCtrl().SetEventMask()
	// with the ENM_CHANGE flag ORed into the mask.

	// TODO:  Add your control notification handler code here

	CWnd* win;
	CString		s;
	size_t		len;
	char		TBuf[16];
	win = GetDlgItem(IDC_EDIT11);
	win->GetWindowText(s);
	wcstombs_s(&len, TBuf, 16, s, 16);
	UCfg->StCfg.Poligon = atof(TBuf);
}



void CSoundServerDlg::OnItemactivateList1(NMHDR* pNMHDR, LRESULT* pResult)
{
	LPNMITEMACTIVATE pNMIA = reinterpret_cast<LPNMITEMACTIVATE>(pNMHDR);
	// TODO: добавьте свой код обработчика уведомлений
	CWnd*			win;
	CString			s;
	size_t			len;
	DevElem*		DElF;
	FindStrList8	FSTR;

	ActiveSt = pNMIA->iItem;
	//	win = GetDlgItem(IDC_EDIT1);
	//	win->SetWindowText(s);
	DElF = (DevElem*)DInt->DevList.Begin(FSTR);

//	StElF = (StElem*)LANStList.Begin(FSTR);
	while (DElF)
	{
		if (DElF->Item == ActiveSt)
		{
			ActiveDevEl = DElF;
			// Отображение настроек для данной станции
			win = GetDlgItem(IDC_EDIT3);
			s.Format(_T("%.02f"), UCfg->StCfg.Dev[ActiveDevEl->HW_ID].X);
			win->SetWindowText(s);
			win = GetDlgItem(IDC_EDIT4);
			s.Format(_T("%.02f"), UCfg->StCfg.Dev[ActiveDevEl->HW_ID].Y);
			win->SetWindowText(s);
			s.Format(_T("%.02f"), UCfg->StCfg.Dev[ActiveDevEl->HW_ID].AthimuthCorrection);
			win = GetDlgItem(IDC_EDIT8);
			win->SetWindowText(s);
			win = GetDlgItem(IDC_EDIT13);
			s.Format(_T("%d"), ActiveDevEl->HW_ID);
			win->SetWindowText(s);
			break;
		}
		DElF = (DevElem*)DInt->DevList.Next(FSTR);
	}
	*pResult = 0;
}


void CSoundServerDlg::OnBnClickedButton8()
{
	// TODO: добавьте свой код обработчика уведомлений
	char		tBuf[128];
//	sprintf(tBuf, "W1=1\n\r");
//	DInt->ElUDPStepper->SendTo((UCHAR*)tBuf, strlen(tBuf), (sockaddr*)&AddrToStepper);
	sprintf(tBuf, "D1=-%d\n\r", UCfg->StCfg.Step);
	DInt->ElUDPStepper->SendTo((UCHAR*)tBuf, strlen(tBuf), (sockaddr*)&AddrToStepper);
}


void CSoundServerDlg::OnBnClickedCheck5()
{
	// TODO: добавьте свой код обработчика уведомлений
	char		tBuf[128];
	CButton* pBox;
	pBox = (CButton*)GetDlgItem(IDC_CHECK5);
	UCfg->StCfg.Fl_En_LR = pBox->GetCheck();
	sprintf(tBuf, "E2=%d\n\r", UCfg->StCfg.Fl_En_LR);
	DInt->ElUDPStepper->SendTo((UCHAR*)tBuf, strlen(tBuf), (sockaddr*)&AddrToStepper);

}


void CSoundServerDlg::OnBnClickedCheck4()
{
	// TODO: добавьте свой код обработчика уведомлений
	char		tBuf[128];
	CButton*	pBox;
	pBox = (CButton*)GetDlgItem(IDC_CHECK4);
	UCfg->StCfg.Fl_En_UD = pBox->GetCheck();
	sprintf(tBuf, "E1=%d\n\r", UCfg->StCfg.Fl_En_UD);
	DInt->ElUDPStepper->SendTo((UCHAR*)tBuf,strlen(tBuf),(sockaddr*) &AddrToStepper);
}


void CSoundServerDlg::OnBnClickedButton7()
{
	// TODO: добавьте свой код обработчика уведомлений
	char		tBuf[128];
//	sprintf(tBuf, "W1=0\n\r");
//	DInt->ElUDPStepper->SendTo((UCHAR*)tBuf, strlen(tBuf), (sockaddr*)&AddrToStepper);

	sprintf(tBuf, "D1=%d\n\r", UCfg->StCfg.Step);
	DInt->ElUDPStepper->SendTo((UCHAR*)tBuf, strlen(tBuf), (sockaddr*)&AddrToStepper);
}


void CSoundServerDlg::OnBnClickedButton9()
{
	// TODO: добавьте свой код обработчика уведомлений
	char		tBuf[128];
	sprintf(tBuf, "D2=%d\n\r", UCfg->StCfg.Step);
	DInt->ElUDPStepper->SendTo((UCHAR*)tBuf, strlen(tBuf), (sockaddr*)&AddrToStepper);
}


void CSoundServerDlg::OnBnClickedButton10()
{
	// TODO: добавьте свой код обработчика уведомлений
	char		tBuf[128];
	sprintf(tBuf, "D2=-%d\n\r", UCfg->StCfg.Step);
	DInt->ElUDPStepper->SendTo((UCHAR*)tBuf, strlen(tBuf), (sockaddr*)&AddrToStepper);
}


void CSoundServerDlg::OnBnClickedCheck3()
{
	// TODO: добавьте свой код обработчика уведомлений
	CButton* pBox;
	pBox = (CButton*)GetDlgItem(IDC_CHECK3);
	UCfg->StCfg.Fl_Triangulation = pBox->GetCheck();
	
}


void CSoundServerDlg::OnBnClickedButton11()
{
	// TODO: добавьте свой код обработчика уведомлений
	CWnd* win;
	CString		s;
	if (ActiveDevEl)
	{
		win = GetDlgItem(IDC_BUTTON11);

		DInt->Fl_DataStream ^= 1;

		DInt->SendMsg2CAN(ADC_Stream_Control, ActiveDevEl->HW_ID, &DInt->Fl_DataStream, sizeof(DInt->Fl_DataStream));
		if (DInt->Fl_DataStream)
			s.Format(_T("SwOff Stream"));
		else
			s.Format(_T("SwOn Stream"));
		win->SetWindowText(s);
	}
}


void CSoundServerDlg::OnBnClickedButton12()
{
	// TODO: добавьте свой код обработчика уведомлений
	CWnd*	win;
	CString	s;
	if (DInt->Fl_DataStream)
	{
		win = GetDlgItem(IDC_BUTTON12);
		DInt->Fl_SaveWav ^= 1;
		if (DInt->Fl_SaveWav)
			s.Format(_T("Stop SaveWave"));
		else
			s.Format(_T("Start SaveWave"));
		win->SetWindowText(s);
	}
}


void CSoundServerDlg::OnBnClickedButton13()
{
	// TODO: добавьте свой код обработчика уведомлений
	DInt->SendMsg2CAN(DO_AG_CALIBRATION,ActiveDevEl->HW_ID, NULL, 0);
}


void CSoundServerDlg::OnBnClickedButton14()
{
	// TODO: добавьте свой код обработчика уведомлений
	CWnd* win;
	CString		s;
	if (ActiveDevEl)
	{
		win = GetDlgItem(IDC_BUTTON14);

		DInt->Fl_Debug ^= 1;

		DInt->SendMsg2CAN(SET_DEBUG, ActiveDevEl->HW_ID, &DInt->Fl_Debug, sizeof(DInt->Fl_Debug));
		if (DInt->Fl_Debug)
			s.Format(_T("Debug Off"));
		else
			s.Format(_T("Debug On"));
		win->SetWindowText(s);
	}
}


void CSoundServerDlg::OnBnClickedButton15()
{
	// TODO: добавьте свой код обработчика уведомлений
	if (ActiveDevEl)
		DInt->SendMsg2CAN(RESET_DEVICE, ActiveDevEl->HW_ID, NULL, 0);
	else
		DInt->SendMsg2CAN(RESET_DEVICE, 1, NULL, 0);
}


void CSoundServerDlg::OnBnClickedButton17()
{
	// TODO: добавьте свой код обработчика уведомлений
	float	Vol;
	CWnd* win;
	CString		s;
	size_t		len;
	char		TBuf[16];
	win = GetDlgItem(IDC_EDIT12);
	win->GetWindowText(s);
	wcstombs_s(&len, TBuf, 16, s, 16);
	if (ActiveDevEl)
	{
		Vol = atof(TBuf);
		DInt->SendMsg2CAN(DO_Q_CALIBRATION, ActiveDevEl->HW_ID, (uchar*) & Vol, sizeof(float));
	}

}


void CSoundServerDlg::OnBnClickedButton18()
{
	// TODO: добавьте свой код обработчика уведомлений
	int	Vol;
	CWnd* win;
	CString		s;
	size_t		len;
	char		TBuf[16];
	win = GetDlgItem(IDC_EDIT13);
	win->GetWindowText(s);
	wcstombs_s(&len, TBuf, 16, s, 16);
	if (ActiveDevEl)
	{
		Vol = atoi(TBuf);
		if ((Vol>3)&&(ActiveDevEl->HW_ID!=Vol))
			DInt->SendMsg2CAN(CHNG_TMPID, ActiveDevEl->HW_ID, (uchar*)&Vol, 1);
	}
}


void CSoundServerDlg::OnBnClickedCheck9()
{
	// TODO: добавьте свой код обработчика уведомлений
	CButton* pBox;
	pBox = (CButton*)GetDlgItem(IDC_CHECK9);
	ModeWork &= 0xfeff;
	ModeWork |= pBox->GetCheck()<<8;

}


void CSoundServerDlg::OnBnClickedCheck10()
{
	// TODO: добавьте свой код обработчика уведомлений
	CButton* pBox;
	pBox = (CButton*)GetDlgItem(IDC_CHECK10);
	ModeWork &= 0xffef;
	ModeWork |= pBox->GetCheck()<<4;

}


void CSoundServerDlg::OnBnClickedButton19()
{
	// TODO: добавьте свой код обработчика уведомлений
	if (ActiveDevEl)
		DInt->SendMsg2CAN(SET_MODE, ActiveDevEl->HW_ID, (uchar*) & ModeWork, sizeof(ModeWork));
	else
		DInt->SendMsg2CAN(SET_MODE, ToAll, (uchar*)&ModeWork, sizeof(ModeWork));
}


void CSoundServerDlg::OnBnClickedCheck6()
{
	// TODO: добавьте свой код обработчика уведомлений
	
	CButton* pBox;
	pBox = (CButton*)GetDlgItem(IDC_CHECK6);
	ModeWork &= 0xfffe;
	ModeWork |= pBox->GetCheck()&0x1;
}


void CSoundServerDlg::OnBnClickedCheck7()
{
	// TODO: добавьте свой код обработчика уведомлений
	CButton* pBox;
	pBox = (CButton*)GetDlgItem(IDC_CHECK7);
	ModeWork &= 0xfdff;
	ModeWork |= pBox->GetCheck()<<9;

}


void CSoundServerDlg::OnBnClickedCheck8()
{
	// TODO: добавьте свой код обработчика уведомлений
	CButton* pBox;
	int		 ModeWiFi;
	int		 ChModeWiFi;
	pBox = (CButton*)GetDlgItem(IDC_CHECK8);
	CComboBox* pCombo;
	pCombo = (CComboBox*)GetDlgItem(IDC_COMBO1);
	ModeWiFi=1+pCombo->GetCurSel();
	ChModeWiFi = pBox->GetCheck();
	if (ChModeWiFi == 0)
	{
		ModeWork &= 0xfff3;
	}
	else
	{
		ModeWork |= ModeWiFi << 2;
	}
}


void CSoundServerDlg::OnBnClickedButton20()
{
	// TODO: добавьте свой код обработчика уведомлений
	
	if (ActiveDevEl)
		DInt->SendMsg2CAN(REQUEST_VERSION, ActiveDevEl->HW_ID, NULL, 0);
	else
		DInt->SendMsg2CAN(REQUEST_VERSION, 1, NULL, 0);
}


void CSoundServerDlg::OnCbnSelchangeCombo1()
{
	// TODO: добавьте свой код обработчика уведомлений
	CComboBox* pCombo;
	pCombo = (CComboBox*)GetDlgItem(IDC_COMBO1);
//	ModeWiFi=pCombo->GetCurSel();
}


void CSoundServerDlg::OnBnClickedCheck11()
{
	// TODO: добавьте свой код обработчика уведомлений
}


void CSoundServerDlg::OnBnClickedCheck12()
{
	// TODO: добавьте свой код обработчика уведомлений
}


void CSoundServerDlg::OnBnClickedButton21()
{
	// TODO: добавьте свой код обработчика уведомлений
	CComboBox* pCombo;
	pCombo = (CComboBox*)GetDlgItem(IDC_COMBO3);
	int	Vol;
	CWnd* win;
	CString		s;
	size_t		len;
	char		TBuf[16];
	char		CANSpeed;
	win = GetDlgItem(IDC_EDIT13);
	win->GetWindowText(s);
	wcstombs_s(&len, TBuf, 16, s, 16);
	if (ActiveDevEl)
	{
		CANSpeed=pCombo->GetCurSel();
		DInt->SendMsg2CAN(SET_CANSPEED, ActiveDevEl->HW_ID, (uchar*)&CANSpeed, sizeof(char));
	}
}


void CSoundServerDlg::OnBnClickedButton22()
{
	// TODO: добавьте свой код обработчика уведомлений
	if (ActiveDevEl)
		DInt->SendMsg2CAN(Test_CMD, ActiveDevEl->HW_ID, NULL, 0);

}


void CSoundServerDlg::OnBnClickedButton16()
{
	CWnd* win;
	CString		s;
	// TODO: добавьте свой код обработчика уведомлений
	if (ActiveDevEl)
		Terminator_State ^= 1;
		DInt->SendMsg2CAN(Terminaor_Switch, ActiveDevEl->HW_ID, &Terminator_State, 1);
		win = GetDlgItem(IDC_BUTTON16);
		if (Terminator_State)
			s.Format(_T("Terminator Off"));
		else
			s.Format(_T("Terminator On"));
		win->SetWindowText(s);
//	else
//		DInt->SendMsg2CAN(Terminaor_Switch, 1, NULL, 0);

}
