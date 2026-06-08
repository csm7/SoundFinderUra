#if !defined(AFX_MYSV_H__1CC50F45_9833_11D4_86A7_008048D6B425__INCLUDED_)
#define AFX_MYSV_H__1CC50F45_9833_11D4_86A7_008048D6B425__INCLUDED_

//#if _MSC_VER > 1000
#pragma once
//#endif 

//#include "DSPLib.h"
#include <Winuser.h>
#include <time.h>

//#include "S:\\VSDem\\resource.h"
//#include "S:\\SKL_WC\\resource.h"
//#include "resource.h"
// #include <MarkerG_N.h>

#define	GRM_STAR		0
#define	GRM_SPECTR		1
#define	GRM_VODOPAD		2
#define	GRM_VODOPAD2	7
#define	GRM_INDICATOR	3
#define	GRM_AZIMUT		4
#define	GRM_AZIMUTP		5
#define	GRM_SPECTREMP	6
#define	GRM_Line		7
#define	GRM_MULTYGR		8

#define	RG_REFRESH	0
#define	RG_INIT		1
#define	RG_PAINT	2
#define	RG_REPAINT	3
#define	RG_RESIZE	4
#define	RG_MARKER	5
#define RG_SHKALA	6
#define RG_MOUSEMOVESHKALA	7
#define RG_EMPTYWINDOW  	8

typedef unsigned char uchar;


struct XY
{
	float x,y;
};

struct PARAMETR
{
	float x,fr,fd;
	PARAMETR()
	{
		x=0.0;
		fr=0.0;
		fd=0.0;
	}
};

//#define	Zvezda	0
//#define	Spectr	1
//#define	Vodopad	2
//#define	Indicator 3
//#define	ParamDemStat	1


struct SPECTR
{
	float MaxData,MinData,HalfData,MaxX,MinX,MaxY,MinY;
	float ddeltaX,ddeltaY;
	float Fn;
	int	  CntAprox;
};

struct ZVEZDA
{
	float MaxX,MinX,MaxY,MinY;
	int   N;
};

struct VODOPAD
{
	float       MaxX,MinX,MaxY,MinY;
};


struct ColorIni
{
	COLORREF  MarkerFonColor;		//цвет маркера вне графика
	COLORREF  MarkerFonColor1;		//цвет маркера-разноса вне графика
	COLORREF  MarkerFonColor2;		//цвет маркера-разноса вне графика
    COLORREF  MarkerColor;			//цвет маркера на графике
	COLORREF  MarkerColor1;			//цвет маркера-разноса на графике	
	COLORREF  MarkerColor2;			//цвет маркера-разноса на графике	
	COLORREF  ColorFon;             //цвет фоновой области
	COLORREF  ColorPointGraf;       //цвет графика
	COLORREF  XOY;                  //цвет осей координат
	COLORREF  cEns;                 //цвет Энергии сигнала (Средняя)
	COLORREF  cEnm;                 //цвет Энергии сигнала (Мгновенная)
	COLORREF  MarkerFonColor_m1;
	COLORREF  MarkerFonColor_m2;
	COLORREF  DeadZone;
	COLORREF  ColorFonInv;          
	COLORREF  ColorPointGrafInv;   
	COLORREF  XOYInv;                 
};
// ----------------------------------------------------------------------------------------------------
#define SIMPLE	1
#define COMPLEX 2
// ------------------------------------------------------------
/*
class MarkGN:public ID_Elem
{
public:	
	INT64 IDFile;
	Services* pSrv;
	int X1,X2;
	double F1,F2;
	
	MarkGN(int x1,int x2):ID_Elem((UCHAR*)&IDFile,sizeof(INT64))
	{
		pSrv=GetServices();
		IDFile=pSrv->GetUID();
		X1=x1; X2=x2;
		F1=F2=0.0;
	}
	
	~MarkGN()
	{
	}
	
	BOOL GetAttachment(int x1,int x2)
	{
		BOOL RC=FALSE;
		
		if((x1>=X1) && (x2<=X2))
			return TRUE;
		
		if(((x1>=X1) && (x1<=X2)) || ((x2>=X1) && (x2<=X2)))
			return TRUE;
		
		return RC;
	}
	
	BOOL GetAttachment(int poz)
	{
		BOOL RC=FALSE;
		
		if((poz>=X1) && (poz<=X2))
			return TRUE;
		
		return RC;
	}
};
// ------------------------------------------------------------
class MarkerG_N
{
	FindStrList8 FSL8;
public:
	Services* pSrv;
	ID_List64<MarkGN> L_MarkGN;
	
	MarkerG_N();
	~MarkerG_N();
	void Param(char* Prm,char* Vol);
	void Start();
	void Stop();
	void Add(int x1,int x2,double a1,double a2 );
	int  GetAttachment(int poz);
	int  GetFirstSample(double* f1,double* f2);
	int  GetNextSample(double* f1,double* f2);
	void Reset();
	
protected:
	
};
*/
// ----------------------------------------------------------------------------------------------------
/////////////////////////////////////////////////////////////////////////////
// GraficM view
class GraficM : public CScrollView
{
public:
// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(GraficM)
	public:
	virtual BOOL Create(DWORD dwExStyle, DWORD dwStyle, const RECT& rect,CWnd* pParentWnd, UINT nID, CCreateContext* pContext = NULL);
	protected:
	virtual void OnInitialUpdate();     // first time after construct
	virtual void OnDraw(CDC* pDC);
	//}}AFX_VIRTUAL
// Implementation
public:
	GraficM(int regim);
	~GraficM();
protected:
//	FlagStruct	Fl_Idle;
//--подстилка------------------------------
//	CDC* m_pDC;
	CSize SizeScr;

	int	FlagInit;

	XY	   FnValue,LevelUp,LevelDn;

	int	  delta,delta1;
	int	  Delta;
	int   temp;
	int	  pushDown,pushUp;
	int	  pushDownX,pushUpX;
	int	  RpushDown,RpushUp;
	int	  pushDown1,pushUp1;
	int	  RpushDown1,RpushUp1;

//	struct DemStatOut *DemStat;
//	struct DemStatOut DemStat;

	static LPCTSTR m_plpszClassName;

//--график-------------------------------
	CBrush	BrushCur,	*pBrushOld;
	CPen	*pPenOld;
//	CDC		*cdc;
	CWnd	*Win;
	RECT	rect,rectOld;

	CDC		dc;

	CBitmap  bmp;
	CBitmap *oldbmp;

	XY		xy;

	struct ColorIni clr;

	int		tCnt;
	COLORREF *BrushCurCol;
	CPen	  PenCurFonColor,PenCurFonColor1,PenCurFonColor2;
	CPen	  PenCurColor,PenCurColor1,PenCurColor2;
	CPen	  PenCurColorFon,PenCurColorFonInv;
	CPen	  PenCurPointGraf,PenCurPointGrafInv;
	CPen	  PenCurXOY,PenCurXOYInv;
	CPen	  PenCurXOY_2;
	CPen	  PenCurcEns;
	CPen	  PenCurcEnm;
	CPen	  PenCurcDeadZone;
	CPen	  PenCurColorFon_2;

//	CPen	  QAZ;
	
	int       LineWidthO;           //толщина осевой линии 
	int       LineWidth;            //толщина линии графика

	int	    *Buf,*BufOld;
	int     cntnew,cntOld;
	char    *X,*Y;
	float		CntX,CntY,MaxX,MaxY,MinY,MinX,MaxZ,MinZ;
	int     CNT,CNTVA;
	int     flagcnt;
	int     nLine;
	int     size,sizeold;
	int		CntAprox;
	int     Start;
	int		deleteMarker,deleteMarker1,deleteMarker2;
	int		bitpixel;

	float*	InputBuf;
	float*	InputBufM[4];
	float	tempZv;
//	int		Sdvig;
	int		SizeBuf;
	int		State;

	int    pZeroX,pZeroY,pZeroZ;
	int    pZeroMX,pZeroMY,pZeroMZ;
	int    pZeromX,pZeromY,pZeromZ;
	float  DivY,DivX;
	float  pScaleX,pScaleY,pScaleZ;
	float  oldpScaleX,oldpScaleY;
	float  pStepX,pStepY;
	float   deltaX,deltaY,ddeltaX,ddeltaY;
//	int    marker,marker1,marker2;
	int	   marker[80];
	int	   CntMarker;
	int    DeltaX,DeltaY;
	float  Ens,Enm,EnsOld,EnmOld;
	PARAMETR	parametr;
//	HANDLE		hmtx;
	CRITICAL_SECTION hmtx1;

	float	Fd,ddeltaHz,Tochno;
	char	setkaX[32*1024];
	int	WightPixel;
	int	pixXOR;
	int	Fl_Timer1;
	int	DataMode;
	double OldAz;
	double OldH1;
	double OldH2;
	int F1,F2;
	int FlVecAnayze;
	int flPara,flParaold;
	int x1,y1,x2,y2;
	int Fl_Minimize;
//	MarkerG_N Mark_gn;
	int offShkala;
	CDC		*cdc;
	int Fl_WSA;
	
public:
	int		razn,razn1;
	int		Fl_MaxAGC;
	float	MaxAGC,OldMaxAGC;
	float   AGCM1,AGCM2,AGCM;
	int	Flag_Fun_Show;
	int Regim;//1 - Спектроанализатор; 0 - Фазовая развертка; 2 - Водопад:
	int	Fl_tempZv;
	int Azimut;
	double DeltaAGC;
//	int LBDown[2];
	int Fl_LBDown;
	int	Mark_Lupa[2];
	int x_MouseMove;
	int fl_drag;

//	void SetParam(DemStatOut* p);
//	void SetParam(DemStatOut p);
	int	  InitGrafic(int reg);
	int   FreshRePaint();
	struct ColorIni GetColor();
    int   SetColor(struct ColorIni* c);
	int	  SetDelta(float x,float y,double t);
	int   SetMarker(double* x,double y,int n);
	int	  SetMarkerRaznos(double* m,double fd,float n);
	int   SetAprox(int x);
	int   DelMarker(double* m,double fd,float n);
	int   SetSizePixel(int n);
	int	Coordinate(float ii,float jj);
	void  ChangeSize(int dY);
	void  SetShkala(float fd,int i, int m);
	void  PaintData(float* pmas,int Cnt);
	CRect CreateGrafic(int ID_RES,CWnd* cwnd);
	void  InitData();
	void  RePaintData();
	void  ReSizeData();
	int   SetFullMarkerX(int x,float fr,float fd);
	void  Energy(float ens,float enm);
	void  SetMouseMoveShkala(float fd, int m);
	int   SetParamY(float min,float max);
	int   SetParamX(float min,float max);
	int   Shkala(float Fd,int deltaX);
	struct XY Value(float x,float y,int q);
	int	  MyGetUpdateRect(CRect l);
	void  SetIndicator(COLORREF c);
	void  SetDataMode(int mode);
	int   SetAzimut(int az);
	int	  SetPetal(int h,int hh);
	double GetDeltaLBDown(double fd,double d_fd);
	void GetM1M2(double fd,float &m1,float &m2,float &t_m,float &a1,float &a2,float &a);
	void SetAGC(double m);
	void Example_SaveFile(char* path);
	void ActiveRegimM12(int state);
	void DestroyMarker_Lupa();
	void FullReInit();
	void FullResizeScreen();
	void FullResizeScreen(LPCRECT lpRect);
	void SetShkalaForGrafic(float fd);
	void SetDragY(float updown);
	void SetStretchY(float y1,float y2);
	void SetVecAnalyze(int regim);
	int  GetFirstSample(double* f1,double* f2);
	int  GetNextSample(double* f1,double* f2);
	void Reset();
	void SdvigShkala(int off);
	void SetFlWSA(int flag);
	void PlotPoint(float x,float y, COLORREF clr);
	void PlotLine(float x0,float y0, float x,float y, COLORREF clr);
	void PrmPlotPoint(float x, float y, COLORREF clr);
	void PrmPlotLine(float x0, float y0, float x, float y, COLORREF clr);

protected:
	void  Refresh();
	int	  Paint(float* Buf,int Cnt);
	int	  clrRefresh();
	int	  RePaint();
	int	  Init(int reg);
	void  Show();
	int   SetParamYIn(float min,float max);
	int   SetParamXIn(float min,float max);
	void  SetMyPixel(int x,int y,int wp,COLORREF c);
	int	  PositionMarker(int iPos);
	int   SetPenXORed();
	int   DellPenXORed();
	BOOL  SaveBitmap(HBITMAP hBitmap,HPALETTE hPal,char* szPath);
	BOOL  SavePictureIndirect(PICTDESC pDesc,char* szPath);
	BOOL  SavePicture(IPicture* pPicture,char* szPath);
	void  SetStretchY1(float y1,float y2);


	// Generated message map functions
	//{{AFX_MSG(GraficM)
//	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
//	afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
//	afx_msg void OnMouseMove(UINT nFlags, CPoint point);
	afx_msg void OnRButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnRButtonUp(UINT nFlags, CPoint point);
//	afx_msg void OnLButtonDblClk(UINT nFlags, CPoint point);
	afx_msg void OnRButtonDblClk(UINT nFlags, CPoint point);
	afx_msg void OnTimer(UINT_PTR nIDEvent);
	afx_msg void OnMove(int x, int y);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

// ----------------------------------------------------------------------------------------------------
//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_MYSV_H__1CC50F45_9833_11D4_86A7_008048D6B425__INCLUDED_)
