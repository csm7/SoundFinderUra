
// SoundServer.cpp: определяет поведение классов для приложения.
//

#include "pch.h"
#include "framework.h"
#include "SoundServer.h"
#include "SoundServerDlg.h"
#include "MiniFenek.h"


#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// CSoundServerApp

BEGIN_MESSAGE_MAP(CSoundServerApp, CWinApp)
	ON_COMMAND(ID_HELP, &CWinApp::OnHelp)
END_MESSAGE_MAP()


// Создание CSoundServerApp

CSoundServerApp::CSoundServerApp()
{
	// поддержка диспетчера перезагрузки
	m_dwRestartManagerSupportFlags = AFX_RESTART_MANAGER_SUPPORT_RESTART;

	// TODO: добавьте код создания,
	// Размещает весь важный код инициализации в InitInstance
}


// Единственный объект CSoundServerApp

CSoundServerApp theApp;
CSoundServerDlg	*Mdlg;
Task					*Tsk;
Services				*Srv;
ClassThread				*Srv_Thread;
CMainCfg				*MCfg;
SockCli					*CSock;
//SockSrv					*SrvSockLAN;
DevIntegrator			*DInt;
//SockSrv					*SrvSockWAN;
//SockSrv					*SrvSockWEBL;
//SockSrv					*SrvSockWEBW;

int Services_Calc(LPVOID poi)
{
	return 	((Services*)poi)->IdleFunc(-1);
};


// Инициализация CSoundServerApp

BOOL CSoundServerApp::InitInstance()
{
	// InitCommonControlsEx() требуется для Windows XP, если манифест
	// приложения использует ComCtl32.dll версии 6 или более поздней версии для включения
	// стилей отображения.  В противном случае будет возникать сбой при создании любого окна.
	INITCOMMONCONTROLSEX InitCtrls;
	InitCtrls.dwSize = sizeof(InitCtrls);
	// Выберите этот параметр для включения всех общих классов управления, которые необходимо использовать
	// в вашем приложении.
	InitCtrls.dwICC = ICC_WIN95_CLASSES;
	InitCommonControlsEx(&InitCtrls);

	CWinApp::InitInstance();

	if (!AfxSocketInit())
	{
		AfxMessageBox(IDP_SOCKETS_INIT_FAILED);
		return FALSE;
	}


	AfxEnableControlContainer();

	// Создать диспетчер оболочки, в случае, если диалоговое окно содержит
	// представление дерева оболочки или какие-либо его элементы управления.
	CShellManager *pShellManager = new CShellManager;

	// Активация визуального диспетчера "Классический Windows" для включения элементов управления MFC
	CMFCVisualManager::SetDefaultManager(RUNTIME_CLASS(CMFCVisualManagerWindows));

	// Стандартная инициализация
	// Если эти возможности не используются и необходимо уменьшить размер
	// конечного исполняемого файла, необходимо удалить из следующих
	// конкретных процедур инициализации, которые не требуются
	// Измените раздел реестра, в котором хранятся параметры
	// TODO: следует изменить эту строку на что-нибудь подходящее,
	// например на название организации
	SetRegistryKey(_T("Локальные приложения, созданные с помощью мастера приложений"));

	int				RunFile;
	HANDLE			MapApp;
	char	CurrentDirectory[256];
	char	RunDirectory[256];
	char	ShareName[128];

//	GetCurrentDirectoryA(256, CurrentDirectory);
	strcpy(CurrentDirectory, "C:\\MiniSoundServer");
	SetCurrentDirectoryA(CurrentDirectory);

	//	sprintf(ShareName,"%s%d",N_ShareName,(int)(mVersion/100));
	sprintf(ShareName, "%s", N_ShareName);
	sprintf(RunDirectory, "%s\\Run", CurrentDirectory);
	// Проверка на повторный запуск
	MapApp = OpenFileMappingA(FILE_MAP_ALL_ACCESS, TRUE, ShareName);
	if (MapApp == NULL)
	{
		//		CloseHandle(MapApp); MapApp=NULL;  AfxMessageBox(IDP_NOSTART); return FALSE;
		MapApp = CreateFileMappingA((HANDLE)INVALID_HANDLE_VALUE, NULL, PAGE_READWRITE | SEC_COMMIT, 0, sizeof(StrData), ShareName);
	};

	//	MapApp=CreateFileMapping((HANDLE)0xFFFFFFFF,NULL,PAGE_READWRITE|SEC_COMMIT,0,sizeof(StrData),ShareName);
	int rc = GetLastError();
	if (MapApp == NULL) {/* AfxMessageBox(IDP_NOSTART)*/; return FALSE; };
	int rrc = _unlink(RunDirectory);

	RunFile = _open(RunDirectory, _O_RDWR | _O_CREAT | _O_EXCL | _O_BINARY, _S_IREAD | _S_IWRITE);
	if (RunFile == -1) {/*AfxMessageBox(IDP_NOSTART2)*/; CloseHandle(MapApp); MapApp = NULL;  return FALSE; };

	MCfg = new CMainCfg(Tsk);
	//	SetServices((Services*)NULL);
	MCfg->MData.Active = 0;
	//	strcpy(MCfg->MData.Path, CurrentDirectory);
	strcpy(MCfg->MData.Path, "C:\\MiniSoundServer");
	sprintf(MCfg->MData.PathDLL[MCfg->MData.Active], "%s\\Dll", MCfg->MData.Path);
	sprintf(MCfg->MData.PathCFG[MCfg->MData.Active], "%s\\Cfg", MCfg->MData.Path);
	sprintf(MCfg->MData.PathLOG, "%s\\Log", MCfg->MData.Path);
	sprintf(MCfg->MData.PathBD, "%s\\DB", MCfg->MData.Path);

	MCfg->LoadCfg();

	strcpy(MCfg->MData.Path, "C:\\MiniSoundServer");
	sprintf(MCfg->MData.PathDLL[MCfg->MData.Active], "%s\\Dll", MCfg->MData.Path);
	sprintf(MCfg->MData.PathCFG[MCfg->MData.Active], "%s\\Cfg", MCfg->MData.Path);
	sprintf(MCfg->MData.PathLOG, "%s\\Log", MCfg->MData.Path);
	sprintf(MCfg->MData.PathBD, "%s\\DB", MCfg->MData.Path);

	int		FlReInit = 0;
	if ((MCfg->MData.tVersion != mVersion) && (MCfg->MData.Fl_SISrv == 0))
	{
		FlReInit = 1;
	}


	SetServices((Services*)-1);

	Srv = new Services(MapApp, MCfg->MData.PathLOG, MCfg->MData.Path, MCfg->MData.PathDLL[MCfg->MData.Active], MCfg->MData.PathBD, FlReInit, MCfg, MCfg->MData.NumDayLog, MCfg->MData.TimeLocGlob, MCfg->MData.FlSetTime, MCfg->MData.NumDayCE);


	Srv_Thread = new ClassThread("Servises", Services_Calc, (LPVOID)Srv);


//	MCfg->MData.IP_LAN = 0x0064a8c0;	// 26.138.190.139 (Home-I9) 8bbe8a1a !!!!
//	MCfg->MData.IP_WAN = 0x00be8a1a;	// 26.138.190.139 (Home-I9) 8bbe8a1a
	DInt= new DevIntegrator(Srv);
	CSock = new SockCli();
	DInt->CSock = CSock;
//	SrvSockLAN = new SockSrv();
//	SrvSockWAN = new SockSrv();
//	SrvSockWEBL = new SockSrv();
//	SrvSockWEBW = new SockSrv();

	strcpy(MCfg->MData.IP_Address, "192.168.100.81");
	strcpy(MCfg->MData.IP_Port, "800");

	CSock->Param("ServerIP", "192.168.100.77");
	CSock->Param("ServerPort", "5100");
	CSock->Param("NetType", "LAN");
	CSock->Param("Mode", "NoBufNoPack");

	CSock->Start();

//	SrvSockLAN->Param("NetType", "LAN");
//	SrvSockLAN->Param("Mode", "BufPack");
//	SrvSockLAN->Param("Mode", "NoBufNoPack");
	/*
	SrvSockWAN->Param("NetType", "WAN");
	SrvSockWAN->Param("Mode", "BufPack");

	SrvSockWEBL->Param("NetType", "LAN");
	SrvSockWEBL->Param("Mode", "NoBufNoPack");

	SrvSockWEBW->Param("NetType", "WAN");
	SrvSockWEBW->Param("Mode", "NoBufNoPack");
*/
	DInt->Start();
	CSoundServerDlg *dlg;
	dlg = new CSoundServerDlg();
	m_pMainWnd = (CWnd*)dlg;
	INT_PTR nResponse = dlg->DoModal();

	MCfg ->SaveCfg();
	CSock->Stop();
//	SrvSockWEBL->Stop();
//	SrvSockWEBW->Stop();
//	SrvSockWAN->Stop();
//	SrvSockLAN->Stop();
  	DInt->Stop();
	delete DInt;

	delete	dlg;

	delete CSock;
//	delete SrvSockWEBL;
//	delete SrvSockWEBW;
//	delete SrvSockWAN;
//	delete SrvSockLAN;

	delete Srv_Thread;
	Srv->Stop();
	delete Srv;

	delete MCfg;

	if (nResponse == IDOK)
	{
		// TODO: Введите код для обработки закрытия диалогового окна
		//  с помощью кнопки "ОК"
	}
	else if (nResponse == IDCANCEL)
	{
		// TODO: Введите код для обработки закрытия диалогового окна
		//  с помощью кнопки "Отмена"
	}
	else if (nResponse == -1)
	{
		TRACE(traceAppMsg, 0, "Предупреждение. Не удалось создать диалоговое окно, поэтому работа приложения неожиданно завершена.\n");
		TRACE(traceAppMsg, 0, "Предупреждение. При использовании элементов управления MFC для диалогового окна невозможно #define _AFX_NO_MFC_CONTROLS_IN_DIALOGS.\n");
	}

	// Удалить диспетчер оболочки, созданный выше.
	if (pShellManager != nullptr)
	{
		delete pShellManager;
	}

#if !defined(_AFXDLL) && !defined(_AFX_NO_MFC_CONTROLS_IN_DIALOGS)
	ControlBarCleanUp();
#endif

	// Поскольку диалоговое окно закрыто, возвратите значение FALSE, чтобы можно было выйти из
	//  приложения вместо запуска генератора сообщений приложения.
	return FALSE;
}

