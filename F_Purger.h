#ifndef _F_Purger
#define _F_Purger
//**********************************************
#include "DecodB.h"
//**********************************************
// Структура для получения информации от декодера
// F_Purger_ = наименование объекта_
static char	F_Purger_NameMt[]= "F_Purger";
static char	F_Purger_Version[]= "V1.1";
static char	F_Purger_Author[]= "SystemSoft (S)";
static char	F_Purger_RemMt[]= "Подпрограмма удаления устаревших файлов";
static char	*F_Purger_PinNameI[]=
{
	""
};
static char	*F_Purger_PinNameO[]=
{
	"StatOut","Вывод статистики",
	""
};

static char	*F_Purger_Prm[]=
{
	"PathPurg",			"Полный путь для удаления устаревших файлов (прим. c:\\OutData\\Files)",
	"IgnoreTimeClk",	"Значение для прореживания частоты вызова TimeClk",
	"KolAktivExt",		"Кл-во активных записей (1-10)",
	"Ext0",				"Расширение для обрабатываемых файлов 0-го типа",
	"Ext1",				"Расширение для обрабатываемых файлов 1-го типа",
	"Ext2",				"Расширение для обрабатываемых файлов 2-го типа",
	"Ext3",				"Расширение для обрабатываемых файлов 3-го типа",
	"Ext4",				"Расширение для обрабатываемых файлов 4-го типа",
	"Ext5",				"Расширение для обрабатываемых файлов 5-го типа",
	"Ext6",				"Расширение для обрабатываемых файлов 6-го типа",
	"Ext7",				"Расширение для обрабатываемых файлов 7-го типа",
	"Ext8",				"Расширение для обрабатываемых файлов 8-го типа",
	"Ext9",				"Расширение для обрабатываемых файлов 9-го типа",
	"TimeExt0",			"Время жизни файлов 0-го типа в зависимости от размера своб. места на диске([>25%] [15%] [10%] [5%] [1%] [0%])",
	"TimeExt1",			"Время жизни файлов 1-го типа в зависимости от размера своб. места на диске([>25%] [15%] [10%] [5%] [1%] [0%])",
	"TimeExt2",			"Время жизни файлов 2-го типа в зависимости от размера своб. места на диске([>25%] [15%] [10%] [5%] [1%] [0%])",
	"TimeExt3",			"Время жизни файлов 3-го типа в зависимости от размера своб. места на диске([>25%] [15%] [10%] [5%] [1%] [0%])",
	"TimeExt4",			"Время жизни файлов 4-го типа в зависимости от размера своб. места на диске([>25%] [15%] [10%] [5%] [1%] [0%])",
	"TimeExt5",			"Время жизни файлов 5-го типа в зависимости от размера своб. места на диске([>25%] [15%] [10%] [5%] [1%] [0%])",
	"TimeExt6",			"Время жизни файлов 6-го типа в зависимости от размера своб. места на диске([>25%] [15%] [10%] [5%] [1%] [0%])",
	"TimeExt7",			"Время жизни файлов 7-го типа в зависимости от размера своб. места на диске([>25%] [15%] [10%] [5%] [1%] [0%])",
	"TimeExt8",			"Время жизни файлов 8-го типа в зависимости от размера своб. места на диске([>25%] [15%] [10%] [5%] [1%] [0%])",
	"TimeExt9",			"Время жизни файлов 9-го типа в зависимости от размера своб. места на диске([>25%] [15%] [10%] [5%] [1%] [0%])",
	"DeleteFolder",		"Delete Folder",
	""
};

static char	*F_Purger_Vol[]=
{
	"PathPurg",	"C:\\Out",		"Путь C:\\Out",
	"PathPurg", "SetPrmPath",	"Путь устанавливается по SetPrmPath",
	"PathPurg", "Path",			"Путь устанавливается по Path",
	"PathPurg", "PathBin",		"Путь устанавливается по PathBin",
	"PathPurg", "PathTlg",		"Путь устанавливается по PathTlg",
	"PathPurg", "PathTxt",		"Путь устанавливается по PathTxt",
	"PathPurg", "PathLog",		"Путь устанавливается по PathLog",
	"IgnoreTimeClk","20",	"Реакция на один из 20-ти вызовов TimeClk",
	"KolAktivExt","1",		"Кл-во активных записей (1-10)",
	"KolAktivExt","2",		"Кл-во активных записей (1-10)",
	"KolAktivExt","3",		"Кл-во активных записей (1-10)",
	"KolAktivExt","4",		"Кл-во активных записей (1-10)",
	"KolAktivExt","5",		"Кл-во активных записей (1-10)",
	"KolAktivExt","6",		"Кл-во активных записей (1-10)",
	"KolAktivExt","7",		"Кл-во активных записей (1-10)",
	"KolAktivExt","8",		"Кл-во активных записей (1-10)",
	"KolAktivExt","9",		"Кл-во активных записей (1-10)",
	"KolAktivExt","10",		"Кл-во активных записей (1-10)",
	"Ext0",	"*","Файлы с любым расширением",
	"Ext0",	"WAV","Файлы с любым расширением",
	"Ext0",	"BIN","Файлы с любым расширением",
	"Ext0",	"VOC","Файлы с любым расширением",
	"Ext0",	"DAT","Файлы с любым расширением",
	"Ext0",	"OUT","Файлы с любым расширением",
	"Ext0",	"TST","Файлы с любым расширением",
	"Ext1",	"","",
	"Ext2",	"","",
	"Ext3",	"","",
	"Ext4",	"","",
	"Ext5",	"","",
	"Ext6",	"", "",
	"Ext7",	"", "",
	"Ext8",	"", "",
	"Ext9",	"", "",
	"TimeExt0", "D30,D15,D5,D1,H12,H0", "Dxx-значение в днях  Hxx-значение в часах ([>25%] [15%] [10%] [5%] [1%] [0%])",
	"TimeExt1", "", "Dxx-значение в днях  Hxx-значение в часах ([>25%] [15%] [10%] [5%] [1%] [0%])",
	"TimeExt2", "", "Dxx-значение в днях  Hxx-значение в часах ([>25%] [15%] [10%] [5%] [1%] [0%])",
	"TimeExt3", "", "Dxx-значение в днях  Hxx-значение в часах ([>25%] [15%] [10%] [5%] [1%] [0%])",
	"TimeExt4", "", "Dxx-значение в днях  Hxx-значение в часах ([>25%] [15%] [10%] [5%] [1%] [0%])",
	"TimeExt5", "", "Dxx-значение в днях  Hxx-значение в часах ([>25%] [15%] [10%] [5%] [1%] [0%])",
	"TimeExt6", "", "Dxx-значение в днях  Hxx-значение в часах ([>25%] [15%] [10%] [5%] [1%] [0%])",
	"TimeExt7", "", "Dxx-значение в днях  Hxx-значение в часах ([>25%] [15%] [10%] [5%] [1%] [0%])",
	"TimeExt8", "", "Dxx-значение в днях  Hxx-значение в часах ([>25%] [15%] [10%] [5%] [1%] [0%])",
	"TimeExt9", "", "Dxx-значение в днях  Hxx-значение в часах ([>25%] [15%] [10%] [5%] [1%] [0%])",
	"DeleteFolder","No",		"Folder will n't delete",
	"DeleteFolder","Yes",		"Folder will delete",

	""
};
// -------------------------------------------------------------------------------------------------------------------------------------------------
class	F_Purger : public Decoder
{
public:
// --- Variables ------
	bool fDeleteFolder;
char		StatOut[20480];		//Статистика
char		StatOut0[20480];	//Статистика
int			KolStatOut;			//
int			KolStatOut0;		//

char		NamePurgFileEXT[10][256];
int			TimePurgFileEXT[10][6];
int			TimePurgFile[10];

float		OldFreeDiskSpace;
float		NewFreeDiskSpace;
float		RaznDiskSpaceOut;

UINT		KolAktivExt;

int			IgnoreTimeClk;		//Коеф пропуска вызовов TimeClk();
int			KolTimeClk;

char		PathPurg[2048];		//Путь для работы (включая поддиректории)

HANDLE		hmtx;
WCHAR		NameDmp[80];

//UINT		KoefFreeDiskSpace;

UCHAR		Fl_CalcDir;
//UCHAR		Fl_On;
//DWORD		*T_ID_RecObr;
//HANDLE		hThreadRecObr;

UCHAR		Fl_StartMt/*,Fl_DataMt*/;

INT64 Timer_0;
INT64 Timer_1;
ClassThread	*ClThread;

//------ Method -------
	F_Purger();
virtual ~F_Purger();
virtual void Start();
virtual	void Param(char* Prm,char* Vol);
virtual void Stop();
virtual	int DataFF(int N_Pin,UCHAR* pmas,size_t Cnt);
virtual int IdleFunc(int NumberTr);

//-------------------------------------
float 	GetFree_Space(char Disk);
int		PurgFile(char* FullPath,UINT	GlRecurs);
int		ReadParam(int Num,char* Vol);
int		CalcTimeL();
int		LinApr(float Fr1,float Fr2,int NTime1,int NTime2,float FrX);
void	UnixTimeToFileTime(time_t t, LPFILETIME pft);
};

#endif
