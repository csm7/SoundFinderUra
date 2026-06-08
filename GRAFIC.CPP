#include "pch.h"
//#include "stdafx.h"
#include <stdlib.h> 
#include <math.h>
#include <stdio.h>
#include "grafic.h"
//#include <VSDemLib.h>
#include	<io.h>


#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

#define PROBEL	1
#define CROSS	MAKEINTRESOURCE(32515))
#define SIZE	MAKEINTRESOURCE(32645))
const	double	pi=3.1415926536;

const	double  pi10=pi/18.0;
const	double  pi45=pi/4.0;
const	double  pi90=pi/2.0;
const	double  pi180=180.0/pi;
#define IDC_HAND1                       5001
#define IDC_HAND2                       5002
/////////////////////////////////График//////////////////////////////////////
//---------------------------------------------------------------------------------------
LPCTSTR GraficM::m_plpszClassName=NULL;
GraficM::GraficM(int regim)
{
	cdc=NULL;
	FlVecAnayze=0;
	flPara=flParaold=0;
	DeltaAGC=1;
	InitializeCriticalSection (&hmtx1);
// 	memset(tNameMem,0,sizeof(tNameMem));
// 	sprintf(tNameMem,"Sem_Grafic_Show");
// 	hmtx=OpenSemaphore(SEMAPHORE_ALL_ACCESS	,TRUE,tNameMem);
// 	if(hmtx==NULL) hmtx=CreateSemaphore(NULL,1,1,tNameMem);
	
//--подстилка
	SizeScr.cx=SizeScr.cy=0;
	pushDown=pushUp=0;
	RpushDown=RpushUp=-1;
	pushDown1=pushUp1=0;
	RpushDown1=RpushUp1=-1;
	delta=delta1=0;
	Delta=0;
//	DemStat=NULL;
//--график
	tCnt=0;
	Regim=regim;
	cntOld=cntnew=0;
	Start=0;
	CntX=1024*16;
	InputBuf=(float*)malloc(CntX*sizeof(float));
	memset(InputBuf,0,CntX*sizeof(float));
	Buf=(int*)malloc(CntX*sizeof(float)); 
	BufOld=(int*)malloc(CntX*sizeof(float)); 
//	memset(Buf,0,4*CntX);
//	memset(BufOld,0,4*CntX);
//	Sdvig=0;
	Fl_tempZv=0;
	tempZv=0.0;
	SizeBuf=0;
	State=0;
	DeltaX=DeltaY=0;
	AGCM2=AGCM1=AGCM=0.0;

	Tochno=0;

	clr.cEnm=RGB(255,128,0);
	clr.cEns=RGB(0,255,0);
	memset(marker,-1,sizeof(marker));
	CntMarker=-1;
	WightPixel=2;
	Fl_Timer1=0;
	Flag_Fun_Show=0;
	MaxAGC=0.0;
	OldMaxAGC=0.0;
	Fl_MaxAGC=0;

	DataMode=0;// Mono
	FlagInit=0;
	Fl_LBDown=0;
	Mark_Lupa[0]=Mark_Lupa[1]=0;
	x_MouseMove=0;
	fl_drag=0;
	Fl_Minimize=0;

	pushDownX=pushUpX=0;

	offShkala=0;
	Fl_WSA=1;
	
	switch(Regim)
	{
	case GRM_VODOPAD:
	case GRM_VODOPAD2:
		clr.ColorPointGraf=RGB(0,0,0);
		clr.ColorFon=RGB(0,0,0);
		clr.MarkerColor=RGB(0,255,255);
		clr.XOY=RGB(0,0,0);
		clr.MarkerFonColor_m1=RGB(255,0,0);
		clr.MarkerFonColor_m2=RGB(0,0,255);
		LineWidthO=1;
	    MaxX=512; MinX=0;
		MaxY=50; MinY=0;
		nLine=0;
		PenCurXOY.DeleteObject();
		PenCurXOY.CreatePen(PS_DOT|PS_GEOMETRIC|PS_ENDCAP_ROUND,LineWidthO,clr.XOY);
		PenCurXOY_2.DeleteObject();
		PenCurXOY_2.CreatePen(PS_SOLID,2,clr.XOY);

		PenCurPointGraf.DeleteObject();
		PenCurPointGraf.CreatePen(PS_SOLID,LineWidthO,clr.ColorPointGraf);

		PenCurColorFon.DeleteObject();
		PenCurColorFon.CreatePen(PS_SOLID,LineWidthO,clr.ColorFon);

		PenCurColor.DeleteObject();
		PenCurColor.CreatePen(PS_SOLID,LineWidthO,clr.MarkerColor);

		PenCurColor1.DeleteObject();
		PenCurColor1.CreatePen(PS_SOLID,LineWidthO,clr.MarkerColor1);

		PenCurColor2.DeleteObject();
		PenCurColor2.CreatePen(PS_SOLID,LineWidthO,clr.MarkerColor2);

		PenCurFonColor.DeleteObject();
		PenCurFonColor.CreatePen(PS_SOLID,LineWidthO,clr.MarkerFonColor);

		PenCurFonColor1.DeleteObject();
		PenCurFonColor1.CreatePen(PS_SOLID,LineWidthO,clr.MarkerFonColor1);

		PenCurFonColor2.DeleteObject();
		PenCurFonColor2.CreatePen(PS_SOLID,LineWidthO,clr.MarkerFonColor2);

		PenCurcEns.DeleteObject();
		PenCurcEns.CreatePen(PS_SOLID,LineWidthO,clr.cEns);

		PenCurcEnm.DeleteObject();
		PenCurcEnm.CreatePen(PS_SOLID,LineWidthO,clr.cEnm);
		break;
	case GRM_MULTYGR:
	case GRM_SPECTREMP:
	case GRM_SPECTR:
//		clr.ColorPointGraf=RGB(255,255,255);
		clr.ColorPointGraf=RGB(120,120,120);
		clr.ColorPointGrafInv=RGB(192,192,192);
		clr.ColorFon=RGB(192,192,192);
		clr.ColorFonInv=RGB(120,120,120);
		clr.MarkerFonColor=RGB(0,255,0);
		clr.MarkerFonColor1=RGB(0,255,0);
		clr.MarkerFonColor2=RGB(0,255,0);
		clr.MarkerColor=RGB(255,255,255);
		clr.MarkerColor1=RGB(255,255,255);
		clr.MarkerColor2=RGB(255,255,255);
		clr.MarkerFonColor_m1=RGB(255,0,0);
		clr.MarkerFonColor_m2=RGB(0,0,255);
		clr.XOY=RGB(100,100,100);
		clr.XOYInv=RGB(100,100,100);
		LineWidthO=1;
		MaxX=512; 
		MaxY=100;
		MinX=0;   MinY=0;	
		ddeltaX=20; ddeltaY=10;
		CntAprox=0;
		DivY=1; DivX=1;
		oldpScaleX=pScaleX=oldpScaleY=pScaleY=-1;
		deleteMarker=deleteMarker1=deleteMarker2=0;
		razn=0;
		PenCurXOY.DeleteObject();
		PenCurXOY.CreatePen(PS_DOT|PS_GEOMETRIC|PS_ENDCAP_ROUND,LineWidthO,clr.XOY);
		PenCurXOYInv.DeleteObject();
		PenCurXOYInv.CreatePen(PS_DOT|PS_GEOMETRIC|PS_ENDCAP_ROUND,LineWidthO,clr.XOYInv);
		PenCurXOY_2.DeleteObject();
		PenCurXOY_2.CreatePen(PS_SOLID,2,clr.XOY);
		
		PenCurPointGraf.DeleteObject();
		PenCurPointGraf.CreatePen(PS_SOLID,LineWidthO,clr.ColorPointGraf);
		
		PenCurPointGrafInv.DeleteObject();
		PenCurPointGrafInv.CreatePen(PS_SOLID,LineWidthO,clr.ColorPointGrafInv);
		
		PenCurColorFon.DeleteObject();
		PenCurColorFon.CreatePen(PS_SOLID,LineWidthO,clr.ColorFon);
		
		PenCurColorFonInv.DeleteObject();
		PenCurColorFonInv.CreatePen(PS_SOLID,LineWidthO,clr.ColorFonInv);
		
		PenCurColor.DeleteObject();
		PenCurColor.CreatePen(PS_SOLID,LineWidthO,clr.MarkerColor);
		
		PenCurColor1.DeleteObject();
		PenCurColor1.CreatePen(PS_SOLID,LineWidthO,clr.MarkerColor1);
		
		PenCurColor2.DeleteObject();
		PenCurColor2.CreatePen(PS_SOLID,LineWidthO,clr.MarkerColor2);
		
		PenCurFonColor.DeleteObject();
		PenCurFonColor.CreatePen(PS_SOLID,LineWidthO,clr.MarkerFonColor);
		
		PenCurFonColor1.DeleteObject();
		PenCurFonColor1.CreatePen(PS_SOLID,LineWidthO,clr.MarkerFonColor1);
		
		PenCurFonColor2.DeleteObject();
		PenCurFonColor2.CreatePen(PS_SOLID,LineWidthO,clr.MarkerFonColor2);
		
		PenCurcEns.DeleteObject();
		PenCurcEns.CreatePen(PS_SOLID,LineWidthO,clr.cEns);
		
		PenCurcEnm.DeleteObject();
		PenCurcEnm.CreatePen(PS_SOLID,LineWidthO,clr.cEnm);
		break;
	case GRM_STAR:
		clr.ColorPointGraf=RGB(120,120,120);
		clr.ColorFon=RGB(192,192,192);
		clr.MarkerColor=RGB(0,255,255);
		clr.XOY=RGB(0,0,0);
		LineWidthO=1;
		CNT=512;
		flagcnt=1;
		MaxX=2; MaxY=2;
		MinY=0;	MinX=0;
		PenCurXOY.DeleteObject();
		PenCurXOY.CreatePen(PS_DOT|PS_GEOMETRIC|PS_ENDCAP_ROUND,LineWidthO,clr.XOY);
		PenCurXOY_2.DeleteObject();
		PenCurXOY_2.CreatePen(PS_SOLID,2,clr.XOY);

		PenCurPointGraf.DeleteObject();
		PenCurPointGraf.CreatePen(PS_SOLID,LineWidthO,clr.ColorPointGraf);

		PenCurColorFon.DeleteObject();
		PenCurColorFon.CreatePen(PS_SOLID,LineWidthO,clr.ColorFon);

		PenCurColorFon_2.DeleteObject();
		PenCurColorFon_2.CreatePen(PS_SOLID,2,clr.ColorFon);

		PenCurColor.DeleteObject();
		PenCurColor.CreatePen(PS_SOLID,LineWidthO,clr.MarkerColor);

		PenCurColor1.DeleteObject();
		PenCurColor1.CreatePen(PS_SOLID,LineWidthO,clr.MarkerColor1);

		PenCurColor2.DeleteObject();
		PenCurColor2.CreatePen(PS_SOLID,LineWidthO,clr.MarkerColor2);

		PenCurFonColor.DeleteObject();
		PenCurFonColor.CreatePen(PS_SOLID,LineWidthO,clr.MarkerFonColor);

		PenCurFonColor1.DeleteObject();
		PenCurFonColor1.CreatePen(PS_SOLID,LineWidthO,clr.MarkerFonColor1);

		PenCurFonColor2.DeleteObject();
		PenCurFonColor2.CreatePen(PS_SOLID,LineWidthO,clr.MarkerFonColor2);

		PenCurcEns.DeleteObject();
		PenCurcEns.CreatePen(PS_SOLID,LineWidthO,clr.cEns);

		PenCurcEnm.DeleteObject();
		PenCurcEnm.CreatePen(PS_SOLID,LineWidthO,clr.cEnm);
		break;
	case GRM_INDICATOR:
		clr.ColorFon=RGB(255,0,0);
		PenCurXOY.DeleteObject();
		PenCurXOY.CreatePen(PS_DOT|PS_GEOMETRIC|PS_ENDCAP_ROUND,LineWidthO,clr.XOY);
		PenCurXOY_2.DeleteObject();
		PenCurXOY_2.CreatePen(PS_SOLID,2,clr.XOY);

		PenCurPointGraf.DeleteObject();
		PenCurPointGraf.CreatePen(PS_SOLID,LineWidthO,clr.ColorPointGraf);

		PenCurColorFon.DeleteObject();
		PenCurColorFon.CreatePen(PS_SOLID,LineWidthO,clr.ColorFon);

		PenCurColor.DeleteObject();
		PenCurColor.CreatePen(PS_SOLID,LineWidthO,clr.MarkerColor);

		PenCurColor1.DeleteObject();
		PenCurColor1.CreatePen(PS_SOLID,LineWidthO,clr.MarkerColor1);

		PenCurColor2.DeleteObject();
		PenCurColor2.CreatePen(PS_SOLID,LineWidthO,clr.MarkerColor2);

		PenCurFonColor.DeleteObject();
		PenCurFonColor.CreatePen(PS_SOLID,LineWidthO,clr.MarkerFonColor);

		PenCurFonColor1.DeleteObject();
		PenCurFonColor1.CreatePen(PS_SOLID,LineWidthO,clr.MarkerFonColor1);

		PenCurFonColor2.DeleteObject();
		PenCurFonColor2.CreatePen(PS_SOLID,LineWidthO,clr.MarkerFonColor2);

		PenCurcEns.DeleteObject();
		PenCurcEns.CreatePen(PS_SOLID,LineWidthO,clr.cEns);

		PenCurcEnm.DeleteObject();
		PenCurcEnm.CreatePen(PS_SOLID,LineWidthO,clr.cEnm);
		break;
	case GRM_AZIMUT:
		clr.ColorPointGraf=RGB(120,120,120);
//		clr.ColorFon=RGB(192,192,192);
		clr.ColorFon=RGB(100,100,100);
		clr.MarkerColor=RGB(255,0,0);
		clr.XOY=RGB(200,200,200);
		clr.MarkerFonColor1=RGB(0,255,0);
		clr.DeadZone=RGB(192,192,192);
		LineWidthO=1;
		flagcnt=1;
		MaxX=2; MaxY=2;
		MinY=0;	MinX=0;
		OldAz=0;
		OldH1=0;OldH2=0;
		PenCurXOY.DeleteObject();
		PenCurXOY.CreatePen(PS_DOT|PS_GEOMETRIC|PS_ENDCAP_ROUND,LineWidthO,clr.XOY);
		PenCurXOY_2.DeleteObject();
		PenCurXOY_2.CreatePen(PS_SOLID,2,clr.XOY);

		PenCurPointGraf.DeleteObject();
		PenCurPointGraf.CreatePen(PS_SOLID,LineWidthO,clr.ColorPointGraf);

		PenCurColorFon.DeleteObject();
		PenCurColorFon.CreatePen(PS_SOLID,2,clr.ColorFon);

		PenCurColor.DeleteObject();
		PenCurColor.CreatePen(PS_SOLID,2,clr.MarkerColor);

		PenCurColor1.DeleteObject();
		PenCurColor1.CreatePen(PS_SOLID,LineWidthO,clr.MarkerColor1);

		PenCurColor2.DeleteObject();
		PenCurColor2.CreatePen(PS_SOLID,LineWidthO,clr.MarkerColor2);

		PenCurFonColor.DeleteObject();
		PenCurFonColor.CreatePen(PS_SOLID,LineWidthO,clr.MarkerFonColor);

		PenCurFonColor1.DeleteObject();
		PenCurFonColor1.CreatePen(PS_SOLID,LineWidthO,clr.MarkerFonColor1);

		PenCurFonColor2.DeleteObject();
		PenCurFonColor2.CreatePen(PS_SOLID,LineWidthO,clr.MarkerFonColor2);

		PenCurcEns.DeleteObject();
		PenCurcEns.CreatePen(PS_SOLID,LineWidthO,clr.cEns);

		PenCurcEnm.DeleteObject();
		PenCurcEnm.CreatePen(PS_SOLID,LineWidthO,clr.cEnm);
		
		PenCurcDeadZone.DeleteObject();
		PenCurcDeadZone.CreatePen(PS_SOLID,2,clr.DeadZone);
		break;
	case GRM_AZIMUTP:
		clr.ColorPointGraf=RGB(0,255,0);
		clr.ColorFon=RGB(0,100,0);
		clr.MarkerColor=RGB(255,0,0);
		clr.XOY=RGB(255,0,0);
		clr.MarkerFonColor1=RGB(0,255,0);
		LineWidthO=1;
		flagcnt=1;
		MaxX=2; MaxY=2;
		MinY=0;	MinX=0;
		OldAz=0;
		OldH1=0;OldH2=0;
		PenCurXOY.DeleteObject();
		PenCurXOY.CreatePen(PS_DOT|PS_GEOMETRIC|PS_ENDCAP_ROUND,LineWidthO,clr.XOY);
		PenCurXOY_2.DeleteObject();
		PenCurXOY_2.CreatePen(PS_SOLID,2,clr.XOY);

		PenCurPointGraf.DeleteObject();
		PenCurPointGraf.CreatePen(PS_SOLID,LineWidthO,clr.ColorPointGraf);

		PenCurColorFon.DeleteObject();
		PenCurColorFon.CreatePen(PS_SOLID,2,clr.ColorFon);

		PenCurColor.DeleteObject();
		PenCurColor.CreatePen(PS_SOLID,2,clr.MarkerColor);

		PenCurColor1.DeleteObject();
		PenCurColor1.CreatePen(PS_SOLID,LineWidthO,clr.MarkerColor1);

		PenCurColor2.DeleteObject();
		PenCurColor2.CreatePen(PS_SOLID,LineWidthO,clr.MarkerColor2);

		PenCurFonColor.DeleteObject();
		PenCurFonColor.CreatePen(PS_SOLID,LineWidthO,clr.MarkerFonColor);

		PenCurFonColor1.DeleteObject();
		PenCurFonColor1.CreatePen(PS_SOLID,LineWidthO,clr.MarkerFonColor1);

		PenCurFonColor2.DeleteObject();
		PenCurFonColor2.CreatePen(PS_SOLID,LineWidthO,clr.MarkerFonColor2);

		PenCurcEns.DeleteObject();
		PenCurcEns.CreatePen(PS_SOLID,LineWidthO,clr.cEns);

		PenCurcEnm.DeleteObject();
		PenCurcEnm.CreatePen(PS_SOLID,LineWidthO,clr.cEnm);
		break;
	}
	
/*
	PenCurXOY.DeleteObject();
	PenCurXOY.CreatePen(PS_DOT|PS_GEOMETRIC|PS_ENDCAP_ROUND,LineWidthO,clr.XOY);
	PenCurXOY_2.DeleteObject();
	PenCurXOY_2.CreatePen(PS_SOLID,2,clr.XOY);

	PenCurPointGraf.DeleteObject();
	PenCurPointGraf.CreatePen(PS_SOLID,LineWidthO,clr.ColorPointGraf);

	PenCurColorFon.DeleteObject();
	PenCurColorFon.CreatePen(PS_SOLID,LineWidthO,clr.ColorFon);

	PenCurColor.DeleteObject();
	PenCurColor.CreatePen(PS_SOLID,LineWidthO,clr.MarkerColor);

	PenCurColor1.DeleteObject();
	PenCurColor1.CreatePen(PS_SOLID,LineWidthO,clr.MarkerColor1);

	PenCurColor2.DeleteObject();
	PenCurColor2.CreatePen(PS_SOLID,LineWidthO,clr.MarkerColor2);

	PenCurFonColor.DeleteObject();
	PenCurFonColor.CreatePen(PS_SOLID,LineWidthO,clr.MarkerFonColor);

	PenCurFonColor1.DeleteObject();
	PenCurFonColor1.CreatePen(PS_SOLID,LineWidthO,clr.MarkerFonColor1);

	PenCurFonColor2.DeleteObject();
	PenCurFonColor2.CreatePen(PS_SOLID,LineWidthO,clr.MarkerFonColor2);

	PenCurcEns.DeleteObject();
//	PenCurcEns.CreatePen(PS_DOT|PS_INSIDEFRAME,LineWidthO,clr.cEns);
	PenCurcEns.CreatePen(PS_SOLID,LineWidthO,clr.cEns);

	PenCurcEnm.DeleteObject();
	PenCurcEnm.CreatePen(PS_SOLID,LineWidthO,clr.cEnm);
//	PenCurcEnm.CreatePen(PS_DOT|PS_INSIDEFRAME,LineWidthO,clr.cEnm);
*/
}
//---------------------------------------------------------------------------------------
BOOL GraficM::Create(DWORD dwExStyle, DWORD dwStyle, const RECT& rect,
	CWnd* pParentWnd, UINT nID, CCreateContext* pContext)
{
	// TODO: Add your specialized code here and/or call the base class
	return CWnd::CreateEx(dwExStyle, m_plpszClassName, _T(""), dwStyle,
		rect.left, rect.top, rect.right - rect.left, rect.bottom - rect.top,
		pParentWnd->GetSafeHwnd(), NULL, NULL );
}
//---------------------------------------------------------------------------------------
GraficM::~GraficM()
{
//	CloseHandle(hmtx);
	DeleteCriticalSection(&hmtx1);
	switch(Regim)
	{
	case GRM_VODOPAD:
	case GRM_VODOPAD2:
		free (InputBuf);
		free (BufOld);
		free (Buf);
		break;
	case GRM_MULTYGR:
	case GRM_SPECTREMP:
	case GRM_SPECTR:
		free (InputBuf);
		free (BufOld);
		free (Buf);
		break;
	case GRM_STAR:
		free (InputBuf);
		free (BufOld);
		free (Buf);
		break;
	case GRM_INDICATOR:
		free (InputBuf);
		free (BufOld);
		free (Buf);
		break;
	case GRM_AZIMUT:
	case GRM_AZIMUTP:
		free (InputBuf);
		free (BufOld);
		free (Buf);
		break;
	}
	Regim=-1;
	dc.SelectObject(oldbmp);
	dc.DeleteDC(); 
	bmp.DeleteObject();
	if(cdc)
	{
//		Win->ReleaseDC(cdc);
		cdc=NULL;
	}
}
//---------------------------------------------------------------------------------------
BEGIN_MESSAGE_MAP(GraficM, CScrollView)
	//{{AFX_MSG_MAP(GraficM)
	ON_WM_LBUTTONDOWN()
	ON_WM_LBUTTONUP()
	ON_WM_MOUSEMOVE()
	ON_WM_RBUTTONDOWN()
	ON_WM_RBUTTONUP()
	ON_WM_LBUTTONDBLCLK()
	ON_WM_RBUTTONDBLCLK()
	ON_WM_TIMER()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// GraficM drawing
//---------------------------------------------------------------------------------------
void GraficM::OnInitialUpdate()
{
	CScrollView::OnInitialUpdate();

	//CSize sizeTotal;
	// TODO: calculate the total size of this view
	//sizeTotal.cx = sizeTotal.cy = 100;
//	MyLock(__FILE__,__LINE__,&Fl_Idle,NULL);
	SetScrollSizes(MM_TEXT,SizeScr);
//	MyUnLock(__FILE__,__LINE__,&Fl_Idle,NULL);
}
//---------------------------------------------------------------------------------------
void GraficM::OnDraw(CDC* pDC)
{
	// TODO: add draw code here
//	if(State!=RG_PAINT)
		Show();
}
//---------------------------------------------------------------------------------------
void GraficM::OnRButtonDown(UINT nFlags, CPoint point) 
{
	// TODO: Add your message handler code here and/or call default

	switch(Regim)
	{
	case GRM_SPECTR:
		SetCursor(LoadCursor(NULL,SIZE);
		if(nFlags&MK_SHIFT)
			RpushDown1=point.y;
// 		if(!(nFlags&MK_SHIFT))
// 			RpushDown=point.y;
// 		else
// 			RpushDown1=point.y;
		break;
	case GRM_STAR:
	case GRM_VODOPAD:
	case GRM_VODOPAD2:
		SetCursor(AfxGetApp()->LoadCursor(IDC_HAND2));
		break;
	}
	CScrollView::OnRButtonDown(nFlags, point);

}
//---------------------------------------------------------------------------------------
void GraficM::OnRButtonUp(UINT nFlags, CPoint point) 
{
	// TODO: Add your message handler code here and/or call default
	SetCursor(AfxGetApp()->LoadCursor(IDC_HAND2));

	switch(Regim)
	{
	case GRM_SPECTR:
		if(nFlags&MK_SHIFT)
		{
			if(nFlags&MK_RBUTTON)
			{	
				if(RpushDown1>RpushUp1) 
					delta1=delta1+(RpushDown1-RpushUp1);
				if(RpushDown1<RpushUp1) 
					delta1=delta1-(RpushUp1-RpushDown1);
				RpushUp1=RpushDown1=-1;
			}
			else
			{
				if(nFlags&MK_LBUTTON)
				{
					if(RpushDown>RpushUp) 
						delta=delta+(RpushDown-RpushUp);
					if(RpushDown<RpushUp) 
						delta=delta-(RpushUp-RpushDown);
					RpushUp=RpushDown=-1;
				}
			}
		}
// 		if(!(nFlags&MK_SHIFT))
// 		{
// 			if(RpushDown>RpushUp) 
// 				delta=delta+(RpushDown-RpushUp);
// 			if(RpushDown<RpushUp) 
// 				delta=delta-(RpushUp-RpushDown);
// 			RpushUp=RpushDown=-1;
// 		}
// 		else
// 		{
// 			if(RpushDown1>RpushUp1) 
// 				delta1=delta1+(RpushDown1-RpushUp1);
// 			if(RpushDown1<RpushUp1) 
// 				delta1=delta1-(RpushUp1-RpushDown1);
// 			RpushUp1=RpushDown1=-1;
// 		}
		break;
	case GRM_STAR:
	case GRM_VODOPAD:
	case GRM_VODOPAD2:
		break;
	}
	CScrollView::OnRButtonUp(nFlags, point);
}
//---------------------------------------------------------------------------------------
/*
void GraficM::OnMouseMove(UINT nFlags, CPoint point) 
{
	CString temp;
	double dx=0.0;
	double dy=0.0;
	double arctyx=0.0;
	int az;
//	int rc;
	int k;

	if((nFlags&MK_LBUTTON) && (nFlags&MK_CONTROL) && (pushDown>=0)) 
		SetCursor(AfxGetApp()->LoadCursor(IDC_HAND1));
// 	else
// 		SetCursor(AfxGetApp()->LoadCursor(IDC_HAND2));

	switch(Regim)
	{
	case GRM_SPECTR:
		x_MouseMove=point.x;
		if(nFlags&MK_SHIFT)
		{
//			SetCursor(AfxGetApp()->LoadCursor(IDC_HAND2));
			if(nFlags&MK_LBUTTON)
			{
				if(RpushDown>=0)
				{
					SetCursor(LoadCursor(NULL,SIZE);
					RpushUp=point.y;
					if(RpushDown>RpushUp)
					{
						EnterCriticalSection(&hmtx1);
//						EnterCriticalSection(&hmtx1);
						SetParamYIn((float)MinY+(RpushDown-RpushUp)+delta,-1);
						Shkala(48000,DeltaX);
//						Shkala(DemStat.Fd,DeltaX);
//						LeaveCriticalSection(&hmtx1);
						LeaveCriticalSection(&hmtx1);
					}
					if(RpushDown<RpushUp)
					{
						EnterCriticalSection(&hmtx1);
						SetParamYIn((float)MinY+delta-(RpushUp-RpushDown),-1);
						Shkala(48000,DeltaX);
//						Shkala(DemStat.Fd,DeltaX);
						LeaveCriticalSection(&hmtx1);
					}
				}
			}
			else
			{
				if(nFlags&MK_RBUTTON)
				{
					if(RpushDown1>=0)
					{
						SetCursor(LoadCursor(NULL,SIZE);
						RpushUp1=point.y;
						if(RpushDown1>RpushUp1)//яюфэшьрхь ттхЁї
						{
							EnterCriticalSection(&hmtx1);
							razn1=(float)MinY+(RpushDown1-RpushUp1)+delta1-MinY;
							k=MinY-(MaxY*razn1)/(pZeroY-rectOld.top);
							if(k<=0)
								DeltaAGC=pow((double)2,(double)-1*k/6);
							else
								DeltaAGC=1/pow((double)2,(double)k/6);
							LeaveCriticalSection(&hmtx1);
						}
						if(RpushDown1<RpushUp1)//юяєёърхь тэшч
						{
							EnterCriticalSection(&hmtx1);
							razn1=(float)MinY+delta1-(RpushUp1-RpushDown1)-MinY;
							k=MinY-(MaxY*razn1)/(pZeroY-rectOld.top);
							if(k<=0)
								DeltaAGC=pow((double)2,(double)-1*k/6);
							else
								DeltaAGC=1/pow((double)2,(double)k/6);
							LeaveCriticalSection(&hmtx1);
						}
					}
				}
			}
		}

		break;
	case GRM_STAR:
	case GRM_VODOPAD:
	case GRM_VODOPAD2:
		break;
	case GRM_AZIMUT:
		if(pushDown==1)
		{
			dx=point.x-pZeroX;
			dy=pZeroY-point.y;
			if(dx==0)
			{
				arctyx=90;
				if(((dx>=0) && (dy>=0)) || ((dx>0) && (dy<0)))
					az=90.0-arctyx;
				if(((dx<=0) && (dy<=0)) || ((dx<0) && (dy>0)))
					az=270.0-arctyx;
			}
			else
			{
				arctyx=dy/dx;
				arctyx=atan(arctyx);
				if(((dx>=0) && (dy>=0)) || ((dx>0) && (dy<0)))
					az=90.0-arctyx*pi180;
				if(((dx<=0) && (dy<=0)) || ((dx<0) && (dy>0)))
					az=270.0-arctyx*pi180;
			}
			Azimut=SetAzimut(az);
		}
		break;
	}
//	LeaveCriticalSection(&hmtx1); 

	CScrollView::OnMouseMove(nFlags, point);
}
*/
//---------------------------------------------------------------------------------------
void GraficM::GetM1M2(double fd,float &m1,float &m2,float &t_m,float &a1,float &a2,float &a)
{
	float temp=rectOld.right-pZeroX;
	if(DataMode==0)
		fd/=2.0;

	t_m=fd*(x_MouseMove-pZeroX)/temp;
	a=AGCM;

	if(Fl_LBDown==2)
	{
		m1=fd*(Mark_Lupa[0]-pZeroX)/temp;
		m2=fd*(Mark_Lupa[1]-pZeroX)/temp;
		a1=AGCM1;
		a2=AGCM2;
	}
	else
	{
		m1=m2=a1=a2=-1;
	}
}
//---------------------------------------------------------------------------------------
void GraficM::ActiveRegimM12(int state)
{
	int i;
//	MyLock(__FILE__,__LINE__,&Fl_Idle,NULL);
	switch(state)
	{
	case 0:
		Fl_LBDown=0;
		if(Mark_Lupa[0]!=-1)
		{
			for(i=pZeroY-1;i>=rectOld.top+1;i--)
			{
				if(((pZeroY-i)%2)!=0) 
				{
					if(i>=Buf[Mark_Lupa[0]]) 
						dc.SetPixel(Mark_Lupa[0],i,clr.ColorPointGraf);
					else 
						dc.SetPixel(Mark_Lupa[0],i,clr.ColorFon);
				}
				if((i==(int)((float)pZeroY-(float)((int)((pZeroY-i)/(ddeltaY*pScaleY)))*((float)ddeltaY*pScaleY))
					|| (Mark_Lupa[0]==pZeroX) || (Mark_Lupa[0]==rectOld.right-1)))
				{
					if(((pZeroX+Mark_Lupa[0])%2)==0)
						dc.SetPixel(Mark_Lupa[0],i,clr.XOY);
				}
			}
			Mark_Lupa[0]=0;
//			Mark_Lupa[0]=-1;
		}
		if(Mark_Lupa[1]!=-1)
		{
			for(i=pZeroY-1;i>=rectOld.top+1;i--)
			{
				if(((pZeroY-i)%2)!=0) 
				{
					if(i>=Buf[Mark_Lupa[1]]) 
						dc.SetPixel(Mark_Lupa[1],i,clr.ColorPointGraf);
					else 
						dc.SetPixel(Mark_Lupa[1],i,clr.ColorFon);
				}
				if((i==(int)((float)pZeroY-(float)((int)((pZeroY-i)/(ddeltaY*pScaleY)))*((float)ddeltaY*pScaleY))
					|| (Mark_Lupa[1]==pZeroX) || (Mark_Lupa[1]==rectOld.right-1)))
				{
					if(((pZeroX+Mark_Lupa[1])%2)==0)
						dc.SetPixel(Mark_Lupa[1],i,clr.XOY);
				}
			}
			Mark_Lupa[1]=0;
//			Mark_Lupa[1]=-1;
		}
		break;
	case 1:
		Mark_Lupa[0]=Mark_Lupa[1]=0;
		break;
	case -1:
		Mark_Lupa[0]=Mark_Lupa[1]=-1;
		break;
	}
//	MyUnLock(__FILE__,__LINE__,&Fl_Idle,NULL);
}
//---------------------------------------------------------------------------------------
double GraficM::GetDeltaLBDown(double fd,double d_fd)
{
	double rc=0.0;

	if(Fl_LBDown==2)
//		return rc=(d_fd/fd)*(fd*((double)(LBDown[1]-LBDown[0]))/((double)(rectOld.right-pZeroX)));
		return rc=fabs(fd*((double)(Mark_Lupa[1]-Mark_Lupa[0]))/((double)(rectOld.right-pZeroX)));
	else
		return rc=-1.0;

	return rc;
}
//---------------------------------------------------------------------------------------
void GraficM::DestroyMarker_Lupa()
{
//	MyLock(__FILE__,__LINE__,&Fl_Idle,NULL);
	for(int i=pZeroY-1;i>=rectOld.top+1;i--)
	{
		if(Mark_Lupa[0]!=-1)
		{
			if(((pZeroY-i)%2)!=0) 
			{
				if(i>=Buf[Mark_Lupa[0]]) 
					dc.SetPixel(Mark_Lupa[0],i,clr.ColorPointGraf);
				else 
					dc.SetPixel(Mark_Lupa[0],i,clr.ColorFon);
			}
		}
		if(Mark_Lupa[1]!=-1)
		{
			if(((pZeroY-i)%2)!=0) 
			{
				if(i>=Buf[Mark_Lupa[1]]) 
					dc.SetPixel(Mark_Lupa[1],i,clr.ColorPointGraf);
				else 
					dc.SetPixel(Mark_Lupa[1],i,clr.ColorFon);
			}
		}
	}
//	MyUnLock(__FILE__,__LINE__,&Fl_Idle,NULL);
}
//---------------------------------------------------------------------------------------
/*
void GraficM::OnLButtonDown(UINT nFlags, CPoint point) 
{
//	MyLock(__FILE__,__LINE__,&Fl_Idle,NULL);
	// TODO: Add your message handler code here and/or call default
	int i;
	CScrollView::OnLButtonDown(nFlags, point);
	return;
	pushDownX=point.x;
	switch(Regim)
	{
	case GRM_SPECTR:
		SetCursor(AfxGetApp()->LoadCursor(IDC_HAND1));
		if(nFlags&MK_SHIFT)
			RpushDown=point.y;
		else
		{
			if(!(nFlags&MK_CONTROL))
			{
				switch(Fl_LBDown)
				{
				case 2:
				case 0:
					if(Mark_Lupa[0]<0)
						break;
					Fl_LBDown=1;

					for(i=pZeroY-1;i>=rectOld.top+1;i--)
					{
						if(((pZeroY-i)%2)!=0) 
						{
							if(i>=Buf[Mark_Lupa[0]]) 
								dc.SetPixel(Mark_Lupa[0],i,clr.ColorPointGraf);
							else 
								dc.SetPixel(Mark_Lupa[0],i,clr.ColorFon);
						}
						if((i==(int)((float)pZeroY-(float)((int)((pZeroY-i)/(ddeltaY*pScaleY)))*((float)ddeltaY*pScaleY))
							|| (Mark_Lupa[0]==pZeroX) || (Mark_Lupa[0]==rectOld.right-1)))
						{
							if(((pZeroX+Mark_Lupa[0])%2)==0)
								dc.SetPixel(Mark_Lupa[0],i,clr.XOY);
						}

						if(((pZeroY-i)%2)!=0) 
						{
							if(i>=Buf[point.x]) 
								dc.SetPixel(point.x,i,clr.MarkerColor);
							else 
								dc.SetPixel(point.x,i,clr.MarkerFonColor_m1);
						}
					}
					Mark_Lupa[0]=point.x;
					break;
				case 1:
					if(Mark_Lupa[1]<0)
						break;
					Fl_LBDown=2;

					for(i=pZeroY-1;i>=rectOld.top+1;i--)
					{
						if(((pZeroY-i)%2)!=0) 
						{
							if(i>=Buf[Mark_Lupa[1]]) 
								dc.SetPixel(Mark_Lupa[1],i,clr.ColorPointGraf);
							else 
								dc.SetPixel(Mark_Lupa[1],i,clr.ColorFon);
						}
						if((i==(int)((float)pZeroY-(float)((int)((pZeroY-i)/(ddeltaY*pScaleY)))*((float)ddeltaY*pScaleY))
							|| (Mark_Lupa[1]==pZeroX) || (Mark_Lupa[1]==rectOld.right-1)))
						{
							if(((pZeroX+Mark_Lupa[1])%2)==0)
								dc.SetPixel(Mark_Lupa[1],i,clr.XOY);
						}

						if(((pZeroY-i)%2)!=0) 
						{
							if(i>=Buf[point.x]) 
								dc.SetPixel(point.x,i,clr.MarkerColor);
							else 
								dc.SetPixel(point.x,i,clr.MarkerFonColor_m2);
						}
					}
					Mark_Lupa[1]=point.x;
					break;
				}
//				pushDown=point.y;
			}
			else
				pushDown=point.y;
		}
		break;
	case GRM_STAR:
	case GRM_VODOPAD:
	case GRM_VODOPAD2:
		SetCursor(AfxGetApp()->LoadCursor(IDC_HAND2));
		break;
	case GRM_AZIMUT:
		pushDown=1;
		break;
	}
//	MyUnLock(__FILE__,__LINE__,&Fl_Idle,NULL);
}
//---------------------------------------------------------------------------------------
void GraficM::OnLButtonUp(UINT nFlags, CPoint point) 
{
//	MyLock(__FILE__,__LINE__,&Fl_Idle,NULL);
	int a;
	CScrollView::OnLButtonUp(nFlags, point);
	return;	
	// TODO: Add your message handler code here and/or call default
	SetCursor(AfxGetApp()->LoadCursor(IDC_HAND2));
	switch(Regim)
	{
	case GRM_SPECTR:
		if(nFlags&MK_SHIFT)
		{
 			if(RpushDown>RpushUp) 
 				delta=delta+(RpushDown-RpushUp);
 			if(RpushDown<RpushUp) 
 				delta=delta-(RpushUp-RpushDown);
 			RpushUp=RpushDown=-1;
		}
		else
		{
			if(nFlags&MK_CONTROL)
			{
				pushUp=point.y;
				if(pushDown>pushUp) // поднимаем вверх
				{
					a=((int)((float)(pushDown-pushUp))/(pScaleY));
					SetParamYIn((float)MinY+razn,(float)MaxY-a);
					EnterCriticalSection(&hmtx1);
					State=RG_REFRESH;
					Show();
					LeaveCriticalSection(&hmtx1);
				}
				if(pushUp>pushDown) // поднимаем вниз
				{
					a=((int)((float)(pushUp-pushDown))/(pScaleY));
					SetParamYIn((float)MinY+razn,(float)MaxY+a);
					EnterCriticalSection(&hmtx1);
					State=RG_REFRESH;
					Show();
					LeaveCriticalSection(&hmtx1);
				}
				pushUp=pushDown=-1;
			}
			else
			{
				if(Fl_WSA==1)
				{
					pushUpX=point.x;
					if((pushDownX!=0) && (pushUpX!=0))
					{
						if(pushDownX>pushUpX)
						{
							int temppush=pushDownX;
							pushDownX=pushUpX;
							pushUpX=temppush;
						}
						double a1=((double)(((float)(pushDownX-pZeroX))/pScaleX+(float)MinX))/((double)MaxX);
						double a2=((double)(((float)(pushUpX-pZeroX))/pScaleX+(float)MinX))/((double)MaxX);
//						Mark_gn.Add(pushDownX,pushUpX,a1,a2);
					}
					FreshRePaint();
				}
			}
		}
		break;
	case GRM_STAR:
	case GRM_VODOPAD:
	case GRM_VODOPAD2:
		break;
	case GRM_AZIMUT:
		pushDown=0;
		break;
	}
//	MyUnLock(__FILE__,__LINE__,&Fl_Idle,NULL);
}
*/
//---------------------------------------------------------------------------------------
void GraficM::SetFlWSA(int flag)
{
	Fl_WSA=flag;
}

void GraficM::PlotLine(float x0,float y0, float x,float y, COLORREF clr)
{
	float dx;
	float dy;
	float xt,yt;
	int i,j;

	if (fabs(x-x0)>fabs(y-y0))
	{
		if (x0>x)
		{
			for (i=x;i<x0;i++)
			{
				yt=y+(y0-y)*(i-x)/(x0-x);
				dc.SetPixel(i,yt,clr);
			}
		}
		else
		{
			for (i=x0;i<x;i++)
			{
				yt=y0+(y-y0)*(i-x0)/(x-x0);
				dc.SetPixel(i,yt,clr);
			}
		}
	}
	else
	{
		if (y0>y)
		{
			for (i=y;i<y0;i++)
			{
				xt=x+(x0-x)*(i-y)/(y0-y);
				dc.SetPixel(xt,i,clr);
			}
		}
		else
		{
			for (i=y0;i<y;i++)
			{
				xt=x0+(x-x0)*(i-y0)/(y-y0);
				dc.SetPixel(xt,i,clr);
			}
		}
	}
};


void GraficM::PlotPoint(float x,float y, COLORREF clr)
{
	dc.SetPixel(x,y,clr);
};

void GraficM::PrmPlotLine(float x0, float y0, float x, float y, COLORREF clr)
{
	float	Tmp_x0, Tmp_y0, Tmp_x, Tmp_y;
	Tmp_x0 = rectOld.right * (x0 - MinX) / (MaxX - MinX);
	Tmp_y0 = rectOld.bottom * (MaxY - y0) / (MaxY - MinY);
	Tmp_x = rectOld.right * (x - MinX) / (MaxX - MinX);
	Tmp_y = rectOld.bottom * (MaxY - y) / (MaxY - MinY);
	PlotLine(Tmp_x0, Tmp_y0, Tmp_x, Tmp_y,clr);
};
void GraficM::PrmPlotPoint(float x, float y, COLORREF clr)
{
	float	Tmp_x, Tmp_y;
	Tmp_x = rectOld.right * (x - MinX) / (MaxX - MinX);
	Tmp_y = rectOld.bottom * (MaxY - y) / (MaxY - MinY);
	PlotPoint(Tmp_x, Tmp_y, clr);
};

//---------------------------------------------------------------------------------------
/*
void GraficM::OnLButtonDblClk(UINT nFlags, CPoint point) 
{
//	MyLock(__FILE__,__LINE__,&Fl_Idle,NULL);
	// TODO: Add your message handler code here and/or call default
	
	CString temp;
	double dx=0.0;
	double dy=0.0;
	double arctyx=0.0;
	int az;

	SetCursor(AfxGetApp()->LoadCursor(IDC_HAND2));
	switch(Regim)
	{
	case GRM_SPECTR:
	case GRM_STAR:
	case GRM_VODOPAD:
	case GRM_VODOPAD2:
		break;
	case GRM_AZIMUT:
		dx=point.x-pZeroX;
		dy=pZeroY-point.y;
		if(dx==0)
		{
			arctyx=90;
			if(((dx>=0) && (dy>=0)) || ((dx>0) && (dy<0)))
				az=90.0-arctyx;
			if(((dx<=0) && (dy<=0)) || ((dx<0) && (dy>0)))
				az=270.0-arctyx;
		}
		else
		{
			arctyx=dy/dx;
			arctyx=atan(arctyx);
			if(((dx>=0) && (dy>=0)) || ((dx>0) && (dy<0)))
				az=90.0-arctyx*pi180;
			if(((dx<=0) && (dy<=0)) || ((dx<0) && (dy>0)))
				az=270.0-arctyx*pi180;
		}
			Azimut=SetAzimut(az);
		break;
	}
	CScrollView::OnLButtonDblClk(nFlags, point);
//	MyUnLock(__FILE__,__LINE__,&Fl_Idle,NULL);
}
*/
//---------------------------------------------------------------------------------------
void GraficM::OnRButtonDblClk(UINT nFlags, CPoint point) 
{
//	MyLock(__FILE__,__LINE__,&Fl_Idle,NULL);
	// TODO: Add your message handler code here and/or call default
	
	SetCursor(AfxGetApp()->LoadCursor(IDC_HAND2));
	CScrollView::OnRButtonDblClk(nFlags, point);
//	MyUnLock(__FILE__,__LINE__,&Fl_Idle,NULL);
}
//---------------------------------------------------------------------------------------
int GraficM::clrRefresh()
{
//	MyLock(__FILE__,__LINE__,&Fl_Idle,NULL);
//	CDC		*cdc;

	BrushCur.DeleteObject();
	BrushCur.CreateSolidBrush(clr.ColorFon);
	pBrushOld=dc.SelectObject(&BrushCur);
	pPenOld=dc.SelectObject(&PenCurPointGraf);
	dc.Rectangle(rectOld.left, rectOld.top, rectOld.right, rectOld.bottom);
	dc.SelectObject(&pPenOld);

	if(cdc)
	{
		Win->GetClientRect(&rect);
		cdc->StretchBlt(0,0,rect.right,rect.bottom,&dc,0,0,rectOld.right,rectOld.bottom,SRCCOPY);
	}
/*
	Win->GetClientRect(&rect);
	cdc=Win->GetDC();
	cdc->StretchBlt(0,0,rect.right,rect.bottom,&dc,0,0,rectOld.right,rectOld.bottom,SRCCOPY);
	Win->ReleaseDC(cdc);	
*/	
//	MyUnLock(__FILE__,__LINE__,&Fl_Idle,NULL);
	return 0;
}
//-------------------------------------------------------------
void GraficM::FullReInit()
{
//	MyLock(__FILE__,__LINE__,&Fl_Idle,NULL);
	Win=(CWnd*)(this);
	if(cdc)
	{
		dc.SelectObject(oldbmp);
		dc.DeleteDC(); 
		bmp.DeleteObject();
		
		if(cdc)
		{
			Win->ReleaseDC(cdc);	
			cdc=NULL;
		}
//		CDC		*cdc;
		dc.DeleteDC(); 
		Win->GetClientRect(&rect);
		cdc=Win->GetDC();
		dc.CreateCompatibleDC(cdc);

		bitpixel=0;
		bitpixel=cdc->GetDeviceCaps(BITSPIXEL);
		rectOld=rect;
		bmp.CreateBitmap(rect.right,rect.bottom,1,bitpixel,NULL);
		oldbmp=dc.SelectObject(&bmp);
		//очистить область вывода
		BrushCur.DeleteObject();
		BrushCur.CreateSolidBrush(clr.ColorFon);
		pBrushOld=dc.SelectObject(&BrushCur);
		pPenOld=dc.SelectObject(&PenCurPointGraf);
		dc.Rectangle(rectOld.left, rectOld.top, rectOld.right, rectOld.bottom);
		dc.SelectObject(&pPenOld);
		pixXOR=dc.GetROP2();
//		Win->ReleaseDC(cdc);
	}
//	MyUnLock(__FILE__,__LINE__,&Fl_Idle,NULL);
}
//-------------------------------------------------------------
int GraficM::Init(int reg)
{
//	MyLock(__FILE__,__LINE__,&Fl_Idle,NULL);
	int i,ii,j;
//	CDC		*cdc;
//	char str[80];
	CString	str;

	CFont fontText;
	COLORREF OldColorFon;
	int q;

	Win=(CWnd*)(this);
	if(!cdc)
	{
		dc.DeleteDC(); 
		if(cdc)
		{
			Win->ReleaseDC(cdc);	
			cdc=NULL;
		}
		Win->GetClientRect(&rect);
		cdc=Win->GetDC();
		dc.CreateCompatibleDC(cdc);
		bitpixel=0;
		bitpixel=cdc->GetDeviceCaps(BITSPIXEL);
		rectOld=rect;
		bmp.CreateBitmap(rect.right,rect.bottom,1,bitpixel,NULL);
		oldbmp=dc.SelectObject(&bmp);
		//очистить область вывода
		BrushCur.DeleteObject();
		BrushCur.CreateSolidBrush(clr.ColorFon);
		pBrushOld=dc.SelectObject(&BrushCur);
		pPenOld=dc.SelectObject(&PenCurPointGraf);
		dc.Rectangle(rectOld.left, rectOld.top, rectOld.right, rectOld.bottom);
		dc.SelectObject(&pPenOld);
		pixXOR=dc.GetROP2();
//		Win->ReleaseDC(cdc);
	}

	switch(reg)
	{
	case GRM_VODOPAD://Vodopad
	case GRM_VODOPAD2://Vodopad
	//-------------Система координат  - верхний левый угол------------
		MaxZ=rectOld.bottom; MinZ=0;
		pZeroX=0+20; pZeroY=0; pZeroZ=0;
		pScaleX=(float)(rectOld.right-pZeroX)/(MaxX-MinX);
		pScaleY=(float)255/(MaxY-MinY);
		pScaleZ=1;
		Win->GetClientRect(&rect);
//		SetTimer(1,5000,NULL);
		SetTimer(1,1000,NULL);
		break;
	case GRM_STAR://Zvezda
	//-------------Система координат  - центр------------
		pZeroX=rectOld.right/2;
		pZeroY=rectOld.bottom/2;
		pStepX=0; pStepY=0;
		//-----------------------масштаб---------------------
		pScaleX=(float)(rectOld.right-pZeroX)/(MaxX);
		pScaleY=(float)(pZeroY-rectOld.top)/(MaxY);
		//-----------------------

	   //прорисовка осей координат
		if(FlVecAnayze==0)
		{
			pPenOld=dc.SelectObject(&PenCurXOY);
			dc.MoveTo(pZeroX,rectOld.top);
			dc.LineTo(pZeroX,rectOld.bottom);
			dc.MoveTo(rectOld.left,pZeroY);
			dc.LineTo(rectOld.right,pZeroY);
			dc.SelectObject(pPenOld);
		}
		Win->GetClientRect(&rect);
		break;
	case GRM_AZIMUT://Azimut
	//-------------Система координат  - центр------------
		pZeroX=rectOld.right/2;
		pZeroY=rectOld.bottom/2;
		pZeroMX=0.875*rectOld.right;
		pZeromX=0.125*rectOld.right;
		pZeroMY=0.875*rectOld.bottom;
		pZeromY=0.125*rectOld.bottom;
		pStepX=0; pStepY=0;
		//-----------------------масштаб---------------------
		pScaleX=(float)(rectOld.right-pZeroX)/(MaxX);
		pScaleY=(float)(pZeroY-rectOld.top)/(MaxY);
		//-----------------------

		pPenOld=dc.SelectObject(&PenCurXOY_2);

	   //прорисовка осей координат
		dc.Ellipse(pZeromX,pZeroMX,pZeroMX,pZeromY);
		dc.SelectObject(pPenOld);
		pPenOld=dc.SelectObject(&PenCurXOY);

		for(i=0;i<36;i++)
		{
			dc.MoveTo(pZeroX+((pZeroMX-pZeromX+8)/2)*cos(i*pi10),pZeroY+((pZeroMX-pZeromX+8)/2)*sin(i*pi10));
			dc.LineTo(pZeroX+((pZeroMX-pZeromX)/2)*cos(i*pi10),pZeroY+((pZeroMX-pZeromX)/2)*sin(i*pi10));
		}

		for(i=0;i<8;i++)
		{
			dc.MoveTo(pZeroX+((pZeroMX-pZeromX)/2)*cos(i*pi45),pZeroY+((pZeroMX-pZeromX)/2)*sin(i*pi45));
			dc.LineTo(pZeroX+((pZeroMX-pZeromX+20)/2)*cos(i*pi45),pZeroY+((pZeroMX-pZeromX+20)/2)*sin(i*pi45));
		}

		fontText.CreateFontW(10,0,0,0,400,FALSE,FALSE,0,ANSI_CHARSET,OUT_DEFAULT_PRECIS,CLIP_DEFAULT_PRECIS,DEFAULT_QUALITY,DEFAULT_PITCH|FF_SWISS,_T("Arial"));
		dc.SelectObject(&fontText);
		for(i=0;i<4;i++)
		{
			OldColorFon=dc.SetBkColor(clr.ColorFon);
//			sprintf_s(str,"%d",i*90);
			str.Format(_T("%d"),i*90);
			if(i<2)
				dc.TextOutW(pZeroX+((pZeroMX-pZeromX)/2)*cos(i*pi90-pi90)+4,pZeroY+((pZeroMX-pZeromX)/2)*sin(i*pi90-pi90)-12,str);
			if(i==2)
				dc.TextOutW(pZeroX+((pZeroMX-pZeromX)/2)*cos(i*pi90-pi90)+4,pZeroY+((pZeroMX-pZeromX)/2)*sin(i*pi90-pi90)+4,str);
			if(i==3)
				dc.TextOutW(pZeroX+((pZeroMX-pZeromX)/2)*cos(i*pi90-pi90)-16,pZeroY+((pZeroMX-pZeromX)/2)*sin(i*pi90-pi90)-12,str);

		}
		fontText.DeleteObject();	

		dc.SelectObject(pPenOld);
		Win->GetClientRect(&rect);
		break;
	case GRM_MULTYGR:
	case GRM_SPECTREMP:
	//-------------Система координат  - нижний левый угол
		pZeroX=rectOld.left;
		pZeroY=rectOld.bottom-2;
		pStepX=(float)(MaxX-MinX)/ddeltaX;
		pStepY=((float)(MaxY-MinY))/ddeltaY;
		
		Enm=Ens=EnsOld=EnmOld=pZeroY;
		//-----------------------масштаб---------------------
		pPenOld=dc.SelectObject(&PenCurXOY);
		pScaleX=(float)(rectOld.right-pZeroX)/(MaxX-MinX);
		pScaleY=(float)(pZeroY-rectOld.top)/(MaxY-MinY);
		
		deltaY=(int)(ddeltaY*pScaleY);
		if(deltaY==0) deltaY=1;

		for(i=0;i<=(int)((MaxY-MinY-razn)/ddeltaY);i++)
		{
			q=0;
			for(j=pZeroX;j<=rectOld.right;j++)
			{
				if(q==2*PROBEL) q=0;
				if(q<=PROBEL-1) 
				{
					dc.SetPixel(j,pZeroY-i*ddeltaY*pScaleY,clr.XOY);
					q++;
				}
				else
				{
					dc.SetPixel(j,pZeroY-i*ddeltaY*pScaleY,clr.ColorFon);
					q++;
				}
			}
		}


		memset(setkaX,0,sizeof(setkaX));
		deltaX=(ddeltaX*pScaleX); 
		if(deltaX==0) 
			deltaX=1;
		//-----------------------

		for(i=0;i<=(int)((MaxX-MinX)/ddeltaX);i++)
		{
			q=0;
			for(j=pZeroY;j>=rectOld.top;j--)
			{
				if(q==2*PROBEL) q=0;
				if(q<=PROBEL-1) 
				{
					dc.SetPixel(pZeroX+i*ddeltaX*pScaleX,j,clr.XOY);
					q++;
				}
				else
				{
					dc.SetPixel(pZeroX+i*ddeltaX*pScaleX,j,clr.ColorFon);
					q++;
				}
			}
			setkaX[(int)(pZeroX+i*ddeltaX*pScaleX)]=1;
		}

		dc.SelectObject(pPenOld);
		Win->GetClientRect(&rect);
		break;
	case GRM_SPECTR://Spectr
	//-------------Система координат  - нижний левый угол
		pZeroX=rectOld.left+20;
		pZeroY=rectOld.bottom-2-20;
		pStepX=(float)(MaxX-MinX)/ddeltaX;
		pStepY=((float)(MaxY-MinY))/ddeltaY;
		
		Enm=Ens=EnsOld=EnmOld=pZeroY;
		//-----------------------масштаб---------------------
		pPenOld=dc.SelectObject(&PenCurXOY);
		pScaleX=(float)(rectOld.right-pZeroX)/(MaxX-MinX);
//		pScaleX=(float)(rectOld.right-pZeroX)/1024.0;
		pScaleY=(float)(pZeroY-rectOld.top)/(MaxY-MinY);
		
		deltaY=(int)(ddeltaY*pScaleY);
		if(deltaY==0) deltaY=1;

		for(i=0;i<=(int)((MaxY-MinY-razn)/ddeltaY);i++)
		{
			q=0;
			for(j=pZeroX;j<=rectOld.right;j++)
			{
				if(q==2*PROBEL) q=0;
				if(q<=PROBEL-1) 
				{
					dc.SetPixel(j,pZeroY-i*ddeltaY*pScaleY,clr.XOY);
					q++;
				}
				else
				{
					dc.SetPixel(j,pZeroY-i*ddeltaY*pScaleY,clr.ColorFon);
					q++;
				}
			}
		}


		memset(setkaX,0,sizeof(setkaX));
		if(Tochno==0)
		{
			deltaX=(ddeltaX*pScaleX); 
			if(deltaX==0) deltaX=1;
			//-----------------------

			for(i=0;i<=(int)((MaxX-MinX)/ddeltaX);i++)
			{
				q=0;
				for(j=pZeroY;j>=rectOld.top;j--)
				{
					if(q==2*PROBEL) q=0;
					if(q<=PROBEL-1) 
					{
						dc.SetPixel(pZeroX+i*ddeltaX*pScaleX,j,clr.XOY);
						q++;
					}
					else
					{
						dc.SetPixel(pZeroX+i*ddeltaX*pScaleX,j,clr.ColorFon);
						q++;
					}
				}
				setkaX[(int)(pZeroX+i*ddeltaX*pScaleX)]=1;
			}
		}
		else
		{
			float f=0.0; i=ii=0;
			int   Half;
			switch(DataMode)
			{
			case 0:
				while(f<=Fd)
				{
					i=(int)((2*f*(rectOld.right-rectOld.left-20))/(Fd))+20;
					setkaX[i]=1;
					q=0;
					for(j=pZeroY;j>=rectOld.top;j--)
					{
						if(q==2*PROBEL) q=0;
						if(q<=PROBEL-1)
						{
							dc.SetPixel(i,j,clr.XOY);
							q++;
						}
						else
						{
							dc.SetPixel(i,j,clr.ColorFon);
							q++;
						}
					}
					f+=ddeltaHz;
				}
				break;
			case 1:
				Half=(rectOld.right+pZeroX)/2;
				while(f<=Fd)
				{
					if(i==0) i=ii=Half;
					else 
					{
//						i=Half+(int)((f*(rectOld.right-rectOld.left-20))/(Fd))+20;
//						ii=Half-((int)((f*(rectOld.right-rectOld.left-20))/(Fd))+20);
						i=Half+(int)((f*(rectOld.right-rectOld.left-20))/(Fd));
						ii=Half-(int)((f*(rectOld.right-rectOld.left-20))/(Fd));
					}
					if((i>rectOld.right) || (ii<pZeroX)) 
					{
						i=rectOld.right;
						ii=pZeroX;
						f=Fd+1.0;
					}
					setkaX[i]=1;
					setkaX[ii]=1;
					q=0;
					for(j=pZeroY;j>=rectOld.top;j--)
					{
						if(q==2*PROBEL) q=0;
						if(q<=PROBEL-1)
						{
							dc.SetPixel(i,j,clr.XOY);
							dc.SetPixel(ii,j,clr.XOY);
							q++;
						}
						else
						{
							dc.SetPixel(i,j,clr.ColorFon);
							dc.SetPixel(ii,j,clr.ColorFon);
							q++;
						}
					}
					f+=ddeltaHz;
				}
				break;
			}
		}

		dc.SelectObject(pPenOld);
		Win->GetClientRect(&rect);
		break;
	case GRM_INDICATOR:
		dc.FillRect(&rectOld,&BrushCur);
		Win->GetClientRect(&rect);
		break;
	case GRM_AZIMUTP:
	//-------------Система координат  - центр------------
		pZeroX=rectOld.right/2;
		pZeroY=rectOld.bottom/2;
		pZeroMX=0.875*rectOld.right;
		pZeromX=0.125*rectOld.right;
		pZeroMY=0.875*rectOld.bottom;
		pZeromY=0.125*rectOld.bottom;
		pStepX=0; pStepY=0;
		//-----------------------масштаб---------------------
		pScaleX=(float)(rectOld.right-pZeroX)/(MaxX);
		pScaleY=(float)(pZeroY-rectOld.top)/(MaxY);
		//-----------------------

		pPenOld=dc.SelectObject(&PenCurXOY_2);

	   //прорисовка осей координат
		dc.Ellipse(pZeromX,pZeroMX,pZeroMX,pZeromY);
		dc.SelectObject(pPenOld);
		pPenOld=dc.SelectObject(&PenCurXOY);

		for(i=0;i<36;i++)
		{
			dc.MoveTo(pZeroX+((pZeroMX-pZeromX+8)/2)*cos(i*pi10),pZeroY+((pZeroMX-pZeromX+8)/2)*sin(i*pi10));
			dc.LineTo(pZeroX+((pZeroMX-pZeromX)/2)*cos(i*pi10),pZeroY+((pZeroMX-pZeromX)/2)*sin(i*pi10));
		}

		for(i=0;i<8;i++)
		{
			dc.MoveTo(pZeroX+((pZeroMX-pZeromX)/2)*cos(i*pi45),pZeroY+((pZeroMX-pZeromX)/2)*sin(i*pi45));
			dc.LineTo(pZeroX+((pZeroMX-pZeromX+20)/2)*cos(i*pi45),pZeroY+((pZeroMX-pZeromX+20)/2)*sin(i*pi45));
		}

		fontText.CreateFont(10,0,0,0,400,FALSE,FALSE,0,ANSI_CHARSET,OUT_DEFAULT_PRECIS,CLIP_DEFAULT_PRECIS,DEFAULT_QUALITY,DEFAULT_PITCH|FF_SWISS,_T("Arial"));
		dc.SelectObject(&fontText);
		for(i=0;i<4;i++)
		{
			OldColorFon=dc.SetBkColor(clr.ColorFon);
//			sprintf(str,"%d",i*90);
			str.Format(_T("%d"),i*90);
			if(i<2)
				dc.TextOutW(pZeroX+((pZeroMX-pZeromX)/2)*cos(i*pi90-pi90)+4,pZeroY+((pZeroMX-pZeromX)/2)*sin(i*pi90-pi90)-12,str);
			if(i==2)
				dc.TextOutW(pZeroX+((pZeroMX-pZeromX)/2)*cos(i*pi90-pi90)+4,pZeroY+((pZeroMX-pZeromX)/2)*sin(i*pi90-pi90)+4,str);
			if(i==3)
				dc.TextOutW(pZeroX+((pZeroMX-pZeromX)/2)*cos(i*pi90-pi90)-16,pZeroY+((pZeroMX-pZeromX)/2)*sin(i*pi90-pi90)-12,str);

		}
		fontText.DeleteObject();	

		dc.SelectObject(pPenOld);
		Win->GetClientRect(&rect);
		break;
	}
	
//	cdc=Win->GetDC();
	if(cdc)
		cdc->StretchBlt(0,0,rect.right,rect.bottom,&dc,0,0,rectOld.right,rectOld.bottom,SRCCOPY);
//	Win->ReleaseDC(cdc);
	FlagInit=1;

//	MyUnLock(__FILE__,__LINE__,&Fl_Idle,NULL);
	return 0;
}
//-------------------------------------------------------------
int GraficM::SetPenXORed()
{
	dc.SetROP2(R2_MASKPEN);

	return 0;
}
//-------------------------------------------------------------
int GraficM::DellPenXORed()
{
	dc.SetROP2(R2_COPYPEN);
	pPenOld=dc.SelectObject(&PenCurXOY);
	 
	return 0;
}
//-------------------------------------------------------------
int GraficM::InitGrafic(int reg)
{
	Init(reg);

	return 0;
}
//-------------------------------------------------------------
void GraficM::ChangeSize(int dY)
{
	int i;
//	MyLock(__FILE__,__LINE__,&Fl_Idle,NULL);
	for (i=0;i<CntX;i++) BufOld[i]=BufOld[i]+dY;
	for (i=0;i<CntX;i++) Buf[i]=Buf[i]+dY;
//	MyUnLock(__FILE__,__LINE__,&Fl_Idle,NULL);
}
//-------------------------------------------------------------
int GraficM::PositionMarker(int iPos)
{
//	MyLock(__FILE__,__LINE__,&Fl_Idle,NULL);
//	for(int i=0;i<=CntMarker;i++)
	for(int i=0;i<CntMarker;i++) 
		if(marker[i]==iPos) 
		{
//			MyUnLock(__FILE__,__LINE__,&Fl_Idle,NULL);
		return 0;}

	if((Mark_Lupa[0]==iPos) || (Mark_Lupa[1]==iPos))
	{
//		MyUnLock(__FILE__,__LINE__,&Fl_Idle,NULL);
	return 3;}

//	MyUnLock(__FILE__,__LINE__,&Fl_Idle,NULL);
	return -1;
}
//-------------------------------------------------------------
int	GraficM::FreshRePaint()
{
//	MyLock(__FILE__,__LINE__,&Fl_Idle,NULL);
	int	a,i,j;
// 	CDC		*cdc;

	switch(Regim)
	{
	case GRM_VODOPAD:
	case GRM_VODOPAD2:
		break;
	case GRM_MULTYGR:
	case GRM_SPECTREMP:
	case GRM_SPECTR:
		for (i=0;i<=rectOld.right;i++)
		{
			if(Buf[i]<=rectOld.top) Buf[i]=rectOld.top+1;
			if(Buf[i]>=pZeroY) Buf[i]=pZeroY;
		}

		for(i=pZeroX;i<=rectOld.right;i++)
		{
			int aa=0;//Mark_gn.GetAttachment(i);
			if(setkaX[i]==1)
			{
				a=-1; a=PositionMarker(i);

				switch(a)
				{
				case 0:
				case 3:
					for(j=pZeroY-1;j>=Buf[i];j--)
					{
						if(((pZeroY-j)%2)!=0) dc.SetPixel(i,j,clr.MarkerColor);
						else dc.SetPixel(i,j,clr.XOY);
					}
					for(j=rectOld.top+1;j<Buf[i];j++)
					{
						if(((pZeroY-j)%2)!=0) dc.SetPixel(i,j,clr.MarkerFonColor);
						else dc.SetPixel(i,j,clr.XOY);
					}
					break;
				case 1:
					for(j=pZeroY-1;j>=Buf[i];j--)
					{
						if(((pZeroY-j)%2)!=0) dc.SetPixel(i,j,clr.MarkerColor1);
						else dc.SetPixel(i,j,clr.XOY);
					}
					for(j=rectOld.top+1;j<Buf[i];j++)
					{
						if(((pZeroY-j)%2)!=0) dc.SetPixel(i,j,clr.MarkerFonColor1);
						else dc.SetPixel(i,j,clr.XOY);
					}
					break;
				case 2:
					for(j=pZeroY-1;j>=Buf[i];j--)
					{
						if(((pZeroY-j)%2)!=0) dc.SetPixel(i,j,clr.MarkerColor2);
						else dc.SetPixel(i,j,clr.XOY);
					}
					for(j=rectOld.top+1;j<Buf[i];j++)
					{
						if(((pZeroY-j)%2)!=0) dc.SetPixel(i,j,clr.MarkerFonColor2);
						else dc.SetPixel(i,j,clr.XOY);
					}
					break;
					break;
				default:
					pPenOld=dc.SelectObject(&PenCurXOY);
					dc.MoveTo(i,pZeroY); dc.LineTo(i,Buf[i]);
					break;
				}
			}
			else
			{
				a=-1; a=PositionMarker(i);

				switch(a)
				{
				case 0:
				case 3:
					for(j=pZeroY-1;j>=Buf[i];j--)
					{
						if(((pZeroY-j)%2)!=0) dc.SetPixel(i,j,clr.MarkerColor);
						else 
						{
							if(aa==-1)
								dc.SetPixel(i,j,clr.ColorPointGraf);
							else
								dc.SetPixel(i,j,clr.ColorPointGrafInv);
						}
					}
					for(j=rectOld.top+1;j<Buf[i];j++)
					{
						if(((pZeroY-j)%2)!=0) dc.SetPixel(i,j,clr.MarkerFonColor);
						else 
						{
							if(aa==-1)
								dc.SetPixel(i,j,clr.ColorFon);
							else
								dc.SetPixel(i,j,clr.ColorFonInv);
						}
					}
					break;
				case 1:
					for(j=pZeroY-1;j>=Buf[i];j--)
					{
						if(((pZeroY-j)%2)!=0) dc.SetPixel(i,j,clr.MarkerColor1);
						else 
						{
							if(aa==-1)
								dc.SetPixel(i,j,clr.ColorPointGraf);
							else
								dc.SetPixel(i,j,clr.ColorPointGrafInv);
						}
					}
					for(j=rectOld.top+1;j<Buf[i];j++)
					{
						if(((pZeroY-j)%2)!=0) dc.SetPixel(i,j,clr.MarkerFonColor1);
						else 
						{
							if(aa==-1)
								dc.SetPixel(i,j,clr.ColorFon);
							else
								dc.SetPixel(i,j,clr.ColorFonInv);
						}
					}
					break;
				case 2:
					for(j=pZeroY-1;j>=Buf[i];j--)
					{
						if(((pZeroY-j)%2)!=0) dc.SetPixel(i,j,clr.MarkerColor2);
						else 
						{
							if(aa==-1)
								dc.SetPixel(i,j,clr.ColorPointGraf);
							else
								dc.SetPixel(i,j,clr.ColorPointGrafInv);
						}
					}
					for(j=rectOld.top+1;j<Buf[i];j++)
					{
						if(((pZeroY-j)%2)!=0) dc.SetPixel(i,j,clr.MarkerFonColor2);
						else 
						{
							if(aa==-1)
								dc.SetPixel(i,j,clr.ColorFon);
							else
								dc.SetPixel(i,j,clr.ColorFonInv);
						}
					}
					break;
				default:
					if(aa==-1)
						pPenOld=dc.SelectObject(&PenCurPointGraf);
					else
						pPenOld=dc.SelectObject(&PenCurPointGrafInv);
					dc.MoveTo(i,pZeroY); dc.LineTo(i,Buf[i]);

					if(aa==-1)
						pPenOld=dc.SelectObject(&PenCurColorFon);
					else
						pPenOld=dc.SelectObject(&PenCurColorFonInv);
					dc.MoveTo(i,Buf[i]); dc.LineTo(i,rectOld.top);
					break;
				}
			}
			
			if(setkaX[i]==1)
			{
//				for (j=Buf[i];j<=pZeroY;j++)
				for (j=rectOld.top;j<=pZeroY;j++)
				{
					if(j<Buf[i])
					{
						if(((pZeroY-j)%2)==0)
							dc.SetPixel(i,j,clr.XOY);
						else
						{
							if(aa==-1)
								dc.SetPixel(i,j,clr.ColorFon);
							else
								dc.SetPixel(i,j,clr.ColorFonInv);
						}
					}
					else
					{
						if(((pZeroY-j)%2)==0) 
							dc.SetPixel(i,j,clr.XOY);
						else
						{
							if(aa==-1)
								dc.SetPixel(i,j,clr.ColorPointGraf);
							else
								dc.SetPixel(i,j,clr.ColorPointGrafInv);
						}
					}
				}
			}
			else
			{
//				for (j=Buf[i];j<=pZeroY;j++)
				for(j=rectOld.top;j<=pZeroY;j++)
				{
					if(((pZeroX+i)%2)==0)
						if((j==(int)((float)pZeroY-(float)((int)((pZeroY-j)/(ddeltaY*pScaleY)))*((float)ddeltaY*pScaleY))
							|| (i==pZeroX) || (i==rectOld.right-1)))
							
						{
							if(j>Buf[i])
								dc.SetPixel(i,j,clr.XOY);
							else
							{
								if(aa==-1)
									dc.SetPixel(i,j,clr.ColorPointGraf);
								else
									dc.SetPixel(i,j,clr.ColorPointGrafInv);
							}
						}
				}
			}
			dc.SelectObject(&pPenOld);
		}
		break;
	case GRM_STAR:
		break;
	case GRM_INDICATOR:
		break;
	}
	
	Win->GetClientRect(&rect);
//	cdc=Win->GetDC();
	if(cdc)
		cdc->StretchBlt(0,0,rect.right,rect.bottom,&dc,0,0,rectOld.right,rectOld.bottom,SRCCOPY);
//	Win->ReleaseDC(cdc);

//	MyUnLock(__FILE__,__LINE__,&Fl_Idle,NULL);
	return 0;
}
//-------------------------------------------------------------
int	GraficM::RePaint()
{
//	MyLock(__FILE__,__LINE__,&Fl_Idle,NULL);
// 	CDC		*cdc;

	EnterCriticalSection(&hmtx1);
	Win->GetClientRect(&rect);
//	cdc=Win->GetDC();
	if(cdc)
		cdc->StretchBlt(0,0,rect.right,rect.bottom,&dc,0,0,rectOld.right,rectOld.bottom,SRCCOPY);
//	Win->ReleaseDC(cdc);
	LeaveCriticalSection(&hmtx1); 

//	MyUnLock(__FILE__,__LINE__,&Fl_Idle,NULL);
	return 0;
};
//-------------------------------------------------------------
int	GraficM::SetDelta(float x,float y,double t)	//Устанавливает размер сетки, работает во всех режимах
{
//	MyLock(__FILE__,__LINE__,&Fl_Idle,NULL);
	Tochno=t;
	if(Tochno==0)
	{
		ddeltaX=x;
		ddeltaY=y;
	}
	else 
	{
		Fd=Tochno;
		ddeltaHz=x;
		ddeltaX=x;
		ddeltaY=y;
		Shkala(Fd,/*DeltaX*/100);
	}

//	MyUnLock(__FILE__,__LINE__,&Fl_Idle,NULL);
	return 0;
}
//-------------------------------------------------------------
struct ColorIni GraficM::GetColor()
{
	return clr;
}
//-------------------------------------------------------------
int GraficM::SetColor(struct ColorIni* c)//Устанавливает цветовую раскраску, работает во всех режимах
{
	clr.MarkerFonColor=c->MarkerFonColor;		//цвет маркера вне графика
	clr.MarkerFonColor1=c->MarkerFonColor1;		//цвет маркера-разноса вне графика
	clr.MarkerFonColor2=c->MarkerFonColor2;		//цвет маркера-разноса вне графика
    clr.MarkerColor=c->MarkerColor;			//цвет маркера на графике
	clr.MarkerColor1=c->MarkerColor1;			//цвет маркера-разноса на графике	
	clr.MarkerColor2=c->MarkerColor2;			//цвет маркера-разноса на графике	
	clr.ColorFon=c->ColorFon;             //цвет фоновой области
	clr.ColorPointGraf=c->ColorPointGraf;       //цвет графика
	clr.XOY=c->XOY;                 //цвет осей координат
	return 0;
}
//-------------------------------------------------------------
int GraficM::SetParamXIn(float min,float max)//Устанавливает пороговые значения по оси X, работает во всех режимах
{
	MaxX=max;
	MinX=min;
	return 0;
}
//-------------------------------------------------------------
int GraficM::SetParamX(float min,float max)//Устанавливает пороговые значения по оси X, работает во всех режимах
{
	SetParamXIn(min,max);
	return 0;
}
//-------------------------------------------------------------
int	GraficM::SetParamYIn(float min,float max)//Устанавливает пороговые значения по оси Y, работает во всех режимах
{
/*
	if(min!=-1.0)
		razn=(min-MinY);
 	if(max!=-1.0) 
 		MaxY=max;
*/
	MaxY = max;
	MinY = min;
	return 0;
}
//-------------------------------------------------------------
int	GraficM::SetParamY(float min,float max)//Устанавливает пороговые значения по оси Y, работает во всех режимах
{
	SetParamYIn(min,max);
	return 0;
}
//-------------------------------------------------------------
int GraficM::SetAprox(int x)//Устанавливает коэффициент апроксимации
{
	CntAprox=x;
	return 0;
}
//-------------------------------------------------------------
int GraficM::Coordinate(float ii,float jj)
{
	float Half=((float)rectOld.right+(float)pZeroX)/2.0;

	if(DataMode==0)	
//		return (int)((2*ii*(rect.right-rect.left-20))/(jj))+20;
		return (2.0*ii*((float)rect.right-(float)rect.left-20.0))/jj+20.0;
	else
	{
		if(ii>0) 
			return Half+(ii*((float)rect.right-(float)pZeroX))/jj;
		else 
			return Half+(ii*((float)rect.right-(float)pZeroX))/jj;
	}
	
	return 0;
}
//-------------------------------------------------------------
int GraficM::SetMarkerRaznos(double* m,double fd,float n)
{
//	MyLock(__FILE__,__LINE__,&Fl_Idle,NULL);
	if(FlagInit!=1) 
	{
//		MyUnLock(__FILE__,__LINE__,&Fl_Idle,NULL);
	return 0;}

	EnterCriticalSection(&hmtx1);
	DelMarker(m,fd,n);
	CntMarker=n;
	SetMarker(m,fd,CntMarker);
	LeaveCriticalSection(&hmtx1); 

//	MyUnLock(__FILE__,__LINE__,&Fl_Idle,NULL);
	return CntMarker;
}
//-------------------------------------------------------------
int GraficM::SetMarker(double* x,double y,int n)
{
//	MyLock(__FILE__,__LINE__,&Fl_Idle,NULL);
	int i,j;
// 	CDC	*cdc;
//	float HalfFq=y/2.0;
	float Half=(rectOld.right+pZeroX)/2;

	switch(Regim)
	{
	case GRM_VODOPAD2:
	case GRM_VODOPAD:
		for(j=0;j<n;j++) 
		{
			if(DataMode==0) marker[j]=((2*x[j]*(rectOld.right-pZeroX))/(y))+20;
			else 
			{
//				if(x[j]<=HalfFq) marker[j]=Half-((x[j]*(rectOld.right-pZeroX))/(y));
//				else marker[j]=Half+((x[j]*(rectOld.right-rectOld.left-20))/(y));
				if(x[j]>0) marker[j]=Half+((x[j]*(rectOld.right-pZeroX))/(y));
				else marker[j]=Half+((x[j]*(rectOld.right-rectOld.left-20))/(y));
			}
		}
		break;
	case GRM_SPECTR:
		for(j=0;j<n;j++)
		{
			if(DataMode==0) marker[j]=((2*x[j]*(rectOld.right-pZeroX))/(y))+20;
			else 
			{
//				if(x[j]<=HalfFq) marker[j]=Half-((x[j]*(rectOld.right-pZeroX))/(y));
//				else marker[j]=Half+((x[j]*(rectOld.right-pZeroX))/(y));
				if(x[j]>0) marker[j]=Half+((x[j]*(rectOld.right-pZeroX))/(y));
				else marker[j]=Half+((x[j]*(rectOld.right-pZeroX))/(y));
			}
//			marker[j]=((2*x[j]*(rectOld.right-rectOld.left-20))/(y))+20;
			if((marker[j]<pZeroX) || (marker[j]>rectOld.right-1)) marker[j]=-1;//mark=pZeroX; mark=rectOld.right-1;
			else
			{
				for(i=pZeroY-1;i>=rectOld.top+1;i--)
				{
					if(((pZeroY-i)%2)!=0) 
					{
						if(i>=Buf[marker[j]]) dc.SetPixel(marker[j],i,clr.MarkerColor);
						else dc.SetPixel(marker[j],i,clr.MarkerFonColor);
					}
				}
			}
		}
		break;
	}
	
	Win->GetClientRect(&rect);
//	cdc=Win->GetDC();
	if(cdc)
		cdc->StretchBlt(0,0,rect.right,rect.bottom,&dc,0,0,rectOld.right,rectOld.bottom,SRCCOPY);
//	Win->ReleaseDC(cdc);

//	MyUnLock(__FILE__,__LINE__,&Fl_Idle,NULL);
	return 0;
}
//-------------------------------------------------------------
struct XY GraficM::Value(float x,float y,int q)
{
//	float HalfFq=y/2.0;
	float Half=((float)(rectOld.right+pZeroX))/2.0;

	xy.y=MaxY-(q*MaxY)/(rect.bottom-2-20-rect.top);
	if(DataMode==0) 
		xy.x=((x-20.0)*y)/(2.0*(float)(rect.right-pZeroX));
//		((float)((x-20)*y))/((float)(2*(rect.right-pZeroX)));
	else 
	{
//		xy.x=(((float)x-(float)Half)*y)/((float)(rectOld.right-pZeroX));
		xy.x=((x-Half)*y)/((float)(rectOld.right-pZeroX));
	}
	return xy;
}
//-------------------------------------------------------------
void GraficM::SetAGC(double m)
{
	MaxAGC=m;
}
//-------------------------------------------------------------
int GraficM::DelMarker(double* m,double fd,float n)
{
//	MyLock(__FILE__,__LINE__,&Fl_Idle,NULL);
	int jj,jjj;
	int dell;
	int i;
// 	CDC		*cdc;
	int	a;
//	float HalfFq=fd/2.0;
	float Half=(rectOld.right+pZeroX)/2;

	switch(Regim)
	{
	case GRM_VODOPAD2:
	case GRM_VODOPAD:
		for(i=0;i<CntMarker;i++) if(marker[i]!=-1) marker[i]=-1;
		break;
	case GRM_SPECTR:
		for(i=0;i<CntMarker;i++)
		{
			dell=1;
			for(jjj=0;jjj<n;jjj++) 
			{
				if(DataMode==0) a=((2*m[jjj]*(rectOld.right-pZeroX))/(fd))+20;
				else 
				{
					if(m[jjj]>0) a=Half+((m[jjj]*(rectOld.right-pZeroX))/(fd));
					else a=Half+((m[jjj]*(rectOld.right-pZeroX))/(fd));
				}
				if(marker[i]==a) 
				{
					dell=0;
					break;
				}
			}
			if((marker[i]!=-1) && (dell==1))
			{
				for(jj=pZeroY-1;jj>=rectOld.top+1;jj--)
				{
					if(((pZeroY-jj)%2)!=0) 
					{
						if(jj>=Buf[marker[i]]) dc.SetPixel(marker[i],jj,clr.ColorPointGraf);
						else dc.SetPixel(marker[i],jj,clr.ColorFon);
					}
					if((jj==((float)pZeroY-(float)(((pZeroY-jj)/(ddeltaY*pScaleY)))*((float)ddeltaY*pScaleY))
						|| (marker[i]==pZeroX) || (marker[i]==rectOld.right-1)))
					{
						if(((pZeroX+marker[i])%2)==0) dc.SetPixel(marker[i],jj,clr.XOY);
					}
				}
				marker[i]=-1;
			}
		}
		break;
	}

	Win->GetClientRect(&rect);
//	cdc=Win->GetDC();
	if(cdc)
		cdc->StretchBlt(0,0,rect.right,rect.bottom,&dc,0,0,rectOld.right,rectOld.bottom,SRCCOPY);
//	Win->ReleaseDC(cdc);
//	MyUnLock(__FILE__,__LINE__,&Fl_Idle,NULL);
	
	return 0;
}
//-------------------------------------------------------------
void GraficM::SdvigShkala(int off)
{
	offShkala=off;
	Shkala(Fd,DeltaX);
}
//-------------------------------------------------------------
int GraficM::Shkala(float Fd,int deltaX)
{
//	MyLock(__FILE__,__LINE__,&Fl_Idle,NULL);
//	CString str,s,str1;
//	char str[128];
//	char str1[128];
	CString str;
	CString str1;
	COLORREF OldColorFon;
	CFont fontText;
	CFont* old_fontText;
	int	i,j;
	float f;
	int ff;
	CSize cs,cs1;
	int Half;

	DeltaX=deltaX;
//	j=MinY-(MaxY*razn)/(pZeroY-rectOld.top);
	j=MinY;
	fontText.CreateFontW(10,0,0,0,400,FALSE,FALSE,0,ANSI_CHARSET,OUT_DEFAULT_PRECIS,CLIP_DEFAULT_PRECIS,DEFAULT_QUALITY,DEFAULT_PITCH|FF_SWISS,_T("Arial"));
	old_fontText=dc.SelectObject(&fontText);
	for(i=pZeroY;i>=rectOld.top;i--)
	{
		if(i==((float)pZeroY-(float)(((pZeroY-i)/(ddeltaY*pScaleY)))*((float)ddeltaY*pScaleY)))
		{
//			str.Format("%.0f",(float)j);
//			str1.Format("       ");
//			sprintf(str,"%.0f",(float)j);
			str.Format(_T("%.0f"),(float)j);
//			sprintf(str1,"       ");
			str1.Format(_T("       "));
			j=j+ddeltaY;
			OldColorFon=dc.SetBkColor(clr.ColorFon);
			dc.TextOutW(1,i,str1,str1.GetLength());
			dc.TextOutW(1,i,str,str.GetLength());
		}
	}
	dc.SelectObject(&old_fontText);
	fontText.DeleteObject();	

	fontText.CreateFont(14,0,0,0,400,FALSE,FALSE,0,ANSI_CHARSET,OUT_DEFAULT_PRECIS,CLIP_DEFAULT_PRECIS,DEFAULT_QUALITY,DEFAULT_PITCH|FF_SWISS,_T("Arial"));
	old_fontText=dc.SelectObject(&fontText);
	j=0; f=0.0;
	switch(DataMode)
	{
	case 0:// Mono
		for(i=pZeroX;i<=rectOld.right;i++)
		{
			if(setkaX[i]==1)
			{
				if((j==DeltaX) || (j==0) || (DeltaX==0))
				{
					j=0;
					ff=f;
					if(ff>0)
						ff-=offShkala;
//					sprintf(str,"%d",ff);
					str.Format(_T("%d"),ff);
					cs=dc.GetTextExtent(str);
					dc.MoveTo(i,pZeroY+2); dc.LineTo(i,pZeroY);
					if((i+cs.cx)<=rectOld.right)
					{
						OldColorFon=dc.SetBkColor(clr.ColorFon); 
						dc.TextOut(i,pZeroY+2,str);
						pPenOld=dc.SelectObject(&PenCurXOY);
						f+=ddeltaHz*DeltaX;
					}
					else 
						i=rectOld.right+1;
				}
				j++;
			}
		}
		break;
	case 1:// Complex
		Half=(rectOld.right+pZeroX)/2;
		for(i=Half;i<=rectOld.right;i++)
		{
			if(setkaX[i]==1)
			{
				if((j==DeltaX) || (j==0) || (DeltaX==0))
				{
					j=0;
					ff=f;
//					str.Format("%d",ff);
//					str1.Format("%d",ff*(-1));
//					sprintf(str,"%d",ff);
					str.Format(_T("%d"),ff);
//					sprintf(str1,"%d",ff*(-1));
					str1.Format(_T("%d",ff*(-1)));
					cs=dc.GetTextExtent(str);
					cs1=dc.GetTextExtent(str1);
					dc.MoveTo(i,pZeroY+2); dc.LineTo(i,pZeroY);
					dc.MoveTo(Half-(i-Half),pZeroY+2); dc.LineTo(Half-(i-Half),pZeroY);
					if((i+cs.cx)<=rectOld.right)
					{
						OldColorFon=dc.SetBkColor(clr.ColorFon); 
						dc.TextOut(i,pZeroY+2,str,str.GetLength());	// strlen(str)
						dc.TextOut(Half-(i-Half),pZeroY+2,str1,str1.GetLength());
						pPenOld=dc.SelectObject(&PenCurXOY);
						f+=ddeltaHz*DeltaX;
					}
					else
					{
/*
						OldColorFon=dc.SetBkColor(clr.ColorFon); 
						dc.TextOut(Half-(i-Half),pZeroY+2,str1);
						pPenOld=dc.SelectObject(&PenCurXOY);
*/
						i=rectOld.right+1;
					}
				}
				j++;
			}
		}
		break;
	}
	dc.SelectObject(&old_fontText);
	fontText.DeleteObject();
//	MyUnLock(__FILE__,__LINE__,&Fl_Idle,NULL);
	return 0;
}
//-------------------------------------------------------------
int GraficM::SetSizePixel(int n)//Задает  колличество  точек  удержания, работает только в режиме Zvezda
{
	CNT=2*n;

	if((CNT%4)!=0)
		CNT=(CNT/4)*4;

	return 0;
}
//-------------------------------------------------------------
void GraficM::SetIndicator(COLORREF c)
{
//	CDC		*cdc;

	if(FlagInit!=1) return;
	if(Regim!=GRM_INDICATOR) return;
	clr.ColorFon=c;
	dc.FillSolidRect(&rectOld,clr.ColorFon);

//	cdc=Win->GetDC();
	if(cdc)
		cdc->StretchBlt(0,0,rect.right,rect.bottom,&dc,0,0,rectOld.right,rectOld.bottom,SRCCOPY);
//	Win->ReleaseDC(cdc);
}
//-------------------------------------------------------------
int	GraficM::Paint(float* buf,int cnt)
{
//	MyLock(__FILE__,__LINE__,&Fl_Idle,NULL);
	int	i=0,ii,j,kk,a;

	EnterCriticalSection(&hmtx1);

	tCnt=cnt;

		float old_Buf; int old_i;
		switch(Regim)
		{
		case GRM_VODOPAD2://---------------------------------------Vodopad-------------------------------------------
 			for(i=0;i<MinX;i++) 
 				Buf[i]=pZeroY;
			old_i=MinX;
			for(ii=MinX;ii<=MaxX;ii++)
			{
				if(buf[ii]<MinY) 
					buf[ii]=(float)MinY;
				if(buf[ii]>MaxY) 
					buf[ii]=(float)MaxY;

				i=/*pZeroX+*/(pScaleX*((float)(ii-MinX)));
				Buf[i]=(pScaleY*(buf[ii]-(float)MinY))+razn;
				if(Buf[i]<0) 
					Buf[i]=0;
				if((i-old_i)>1)
				{
					for(int iii=(old_i+1);iii<i;iii++)
						Buf[iii]=Buf[old_i];
				}
				old_Buf=Buf[i];
				old_i=i;
			}

			if(nLine>0)
			{
				dc.BitBlt(0,1,rectOld.right,rectOld.bottom-1,&dc,0,0,SRCCOPY);
			}

// 			if(Fl_Timer1==1) 
// 			{ 
// 				a=pZeroX;
// 				Fl_Timer1=0; 
// 				for(i=0;i<5;i++)
// 					dc.SetPixel(i,0,RGB(255,0,0));
// 			}
// 			else 
// 				a=0;
// 			for(i=a;i<=rectOld.right;i++)
// 			{
// 				kk=-1; kk=PositionMarker(i);
// 				switch(kk)
// 				{
// 				case 0:
// 					clr.ColorPointGraf=clr.MarkerColor;
// 					break;
// 				default:
// 					clr.ColorPointGraf=RGB(0+Buf[i],0+Buf[i],0+Buf[i]);
// 					break;
// 				}
// 				dc.SetPixel(i,0,clr.ColorPointGraf);
// 			}
			
			nLine++;
			if(nLine>rect.bottom-1) nLine=rect.bottom-1;
			break;
		case GRM_VODOPAD://---------------------------------------Vodopad-------------------------------------------
// 			for(i=0;i<=rectOld.right;i++) 
// 				Buf[i]=pZeroY;
 			for(i=0;i<MinX;i++) 
 				Buf[i]=pZeroY;
			old_i=MinX;
			for(ii=MinX;ii<=MaxX;ii++)
			{
				if(buf[ii]<MinY) 
					buf[ii]=(float)MinY;
				if(buf[ii]>MaxY) 
					buf[ii]=(float)MaxY;

				i=pZeroX+(pScaleX*((float)(ii-MinX)));
				Buf[i]=(pScaleY*(buf[ii]-(float)MinY))+razn;
				if(Buf[i]<0) 
					Buf[i]=0;
				if((i-old_i)>1)
				{
					for(int iii=(old_i+1);iii<i;iii++)
						Buf[iii]=Buf[old_i];
				}
				old_Buf=Buf[i]; old_i=i;

// 				if ((int)buf[ii]>MaxY) buf[ii]=(float)MaxY;
// 				else if ((int)buf[ii]<MinY) buf[ii]=(float)MinY;
// 				if (ii<MinX) ii=MinX;
// 				else if(ii>MaxX) ii=MaxX;
// 				i=pZeroX+(int)(pScaleX*(ii-MinX));
// 				Buf[i]=(int)(pScaleY*(buf[ii]-MinY))+razn;
// 				if(Buf[i]<0) Buf[i]=0;
			}

			if(nLine>0)
			{
				dc.BitBlt(0,1,rectOld.right,rectOld.bottom-1,&dc,0,0,SRCCOPY);
			}

			if(Fl_Timer1==1) 
			{ 
				a=pZeroX; Fl_Timer1=0; 
				for(i=0;i<5;i++) dc.SetPixel(i,0,RGB(255,0,0));
//				for(i=0;i<3;i++) dc.SetPixel(i,0,RGB(255,255,0));
			}
			else a=0;
			for(i=a;i<=rectOld.right;i++)
			{
//				if(i) Buf[i]=(Buf[i]+Buf[i-1])/2;
				kk=-1; kk=PositionMarker(i);
				switch(kk)
				{
				case 0:
					clr.ColorPointGraf=clr.MarkerColor;
					break;
				default:
					clr.ColorPointGraf=RGB(0+Buf[i],0+Buf[i],0+Buf[i]);
					break;
				}
//				clr.ColorPointGraf=RGB(0+Buf[i],0+Buf[i],0+Buf[i]);
				dc.SetPixel(i,0,clr.ColorPointGraf);
			}
			
	//		cdc->StretchBlt(0,0,rect.right,rect.bottom,&dc,0,0,rectOld.right,rectOld.bottom,SRCCOPY);
			nLine++;
			if(nLine>rect.bottom-1) nLine=rect.bottom-1;
			break;
		//--------------------------------
		case GRM_MULTYGR:
		case GRM_SPECTREMP:
		case GRM_SPECTR:
		{	
 			for(i=0;i<MinX;i++) 
 				Buf[i]=pZeroY;
			MaxAGC=0;
			//float old_Buf; 
			int /*old_i,*/old_i1;
			float max_Buf=0.0; 
			float tfloat;
			old_i=MinX; old_i1=-1;
			for(ii=MinX;ii<=MaxX;ii++)
			{
				i=pZeroX+(pScaleX*((float)(ii-MinX)));
				tfloat=pZeroY-(pScaleY*buf[ii])-razn;
				if(old_i1!=i)
				{
					if(max_Buf<tfloat)
	  					Buf[i]=tfloat;
					else
						Buf[i]=max_Buf;
					max_Buf=0.0; 
				}
				else
				{
					if(max_Buf>tfloat)
						max_Buf=tfloat;
				}
				old_i1=i;
				if((pScaleY*buf[ii])>80)
					pScaleY=pScaleY;
				if(Buf[i]<=rectOld.top) 
					Buf[i]=rectOld.top+1;
				if(Buf[i]>=pZeroY) 
					Buf[i]=pZeroY;
				if((ii>(MinX+5)) && (ii<(MaxX-5)))
				{
					if(MaxAGC<buf[ii])
							MaxAGC=buf[ii];
				}
				if((i-old_i)>1)
				{
					for(int iii=(old_i+1);iii<i;iii++)
						Buf[iii]=Buf[old_i];
				}
				old_Buf=Buf[i]; old_i=i;
			}
			AGCM1=buf[Mark_Lupa[0]];
			AGCM2=buf[Mark_Lupa[1]];
			AGCM=buf[x_MouseMove];
			
// 			if(CntAprox)
// 			{
// 				int lastnozero=pZeroY;
// 				for(i=pZeroX;i<=rectOld.right;i++) 
// 				{
// 					if(Buf[i]<pZeroY)
// 						lastnozero=Buf[i];
// 					else
// 						Buf[i]=lastnozero;
// 				}
// 			}
			for(i=pZeroX;i<=rectOld.right;i++)
			{
				int aa=0;
//				int aa=Mark_gn.GetAttachment(i);
				if(BufOld[i]!=Buf[i])
				{
					if(BufOld[i]>Buf[i])
					{
						a=-1; a=PositionMarker(i);
						switch(a)
						{
						case 0:
						case 3:
							for(j=Buf[i];j<=BufOld[i];j++)
							{
								if(((pZeroY-j)%2)!=0) 
									dc.SetPixel(i,j,clr.MarkerColor);
								else 
								{
									if(aa==-1)
										dc.SetPixel(i,j,clr.ColorPointGraf);
									else
										dc.SetPixel(i,j,clr.ColorPointGrafInv);
								}
							}
							break;
						case 1:
							for(j=Buf[i];j<=BufOld[i];j++)
							{
								if(((pZeroY-j)%2)!=0) 
									dc.SetPixel(i,j,clr.MarkerColor1);
								else 
								{
									if(aa==-1)
										dc.SetPixel(i,j,clr.ColorPointGraf);
									else
										dc.SetPixel(i,j,clr.ColorPointGrafInv);
								}
							}
							break;
						case 2:
							for(j=Buf[i];j<=BufOld[i];j++)
							{
								if(((pZeroY-j)%2)!=0) 
									dc.SetPixel(i,j,clr.MarkerColor2);
								else 
								{
									if(aa==-1)
										dc.SetPixel(i,j,clr.ColorPointGraf);
									else
										dc.SetPixel(i,j,clr.ColorPointGrafInv);
								}
							}
							break;
						default:
							if(aa==-1)
								pPenOld=dc.SelectObject(&PenCurPointGraf);
							else
								pPenOld=dc.SelectObject(&PenCurPointGrafInv);
							dc.MoveTo(i,BufOld[i]); dc.LineTo(i,Buf[i]);		
							dc.SelectObject(&pPenOld);
							break;
						}

						if(setkaX[i]==1) 
						{
							for (j=Buf[i];j<=BufOld[i];j++)
							{
								if(((pZeroY-j)%2)==0) dc.SetPixel(i,j,clr.XOY);
							}
						}
						else
						{
							for (j=Buf[i];j<=BufOld[i];j++)
							{
								if(((pZeroX+i)%2)==0)
								if((j==((float)pZeroY-(float)(((pZeroY-j)/(ddeltaY*pScaleY)))*((float)ddeltaY*pScaleY))
									|| (i==pZeroX) || (i==rectOld.right-1)))
								{
									dc.SetPixel(i,j,clr.XOY);
								}
							}
						}

					}
					else
					{
						a=-1; a=PositionMarker(i);
						
						switch(a)
						{
						case 0:
						case 3:
							for(j=BufOld[i];j<Buf[i];j++)
							{
								if(((pZeroY-j)%2)!=0) dc.SetPixel(i,j,clr.MarkerFonColor);
								else 
								{
									if(aa==-1)
										dc.SetPixel(i,j,clr.ColorFon);
									else
										dc.SetPixel(i,j,clr.ColorFonInv);
								}
							}
							break;
						case 1:
							for(j=BufOld[i];j<Buf[i];j++)
							{
								if(((pZeroY-j)%2)!=0) dc.SetPixel(i,j,clr.MarkerFonColor1);
								else 
								{
									if(aa==-1)
										dc.SetPixel(i,j,clr.ColorFon);
									else
										dc.SetPixel(i,j,clr.ColorFonInv);
								}
							}
							break;
						case 2:
							for(j=BufOld[i];j<Buf[i];j++)
							{
								if(((pZeroY-j)%2)!=0) dc.SetPixel(i,j,clr.MarkerFonColor2);
								else 
								{
									if(aa==-1)
										dc.SetPixel(i,j,clr.ColorFon);
									else
										dc.SetPixel(i,j,clr.ColorFonInv);
								}
							}
							break;
						default:
							if(aa==-1)
								pPenOld=dc.SelectObject(&PenCurColorFon);
							else
								pPenOld=dc.SelectObject(&PenCurColorFonInv);
							dc.MoveTo(i,BufOld[i]); dc.LineTo(i,Buf[i]);
							dc.SelectObject(&pPenOld);
							break;
						}

						if(setkaX[i]==1)
						{
							for (j=BufOld[i];j<=Buf[i];j++)
							{
								if(((pZeroY-j)%2)==0) dc.SetPixel(i,j,clr.XOY);
								else 
								{
									if(aa==-1)
										dc.SetPixel(i,j,clr.ColorFon);
									else
										dc.SetPixel(i,j,clr.ColorFonInv);
								}
							}
						}
						else
						{
							for (j=BufOld[i];j<=Buf[i];j++)
							{
								if((j==(int)((float)pZeroY-(float)((int)((pZeroY-j)/(ddeltaY*pScaleY)))*((float)ddeltaY*pScaleY))
									|| (i==pZeroX) || (i==rectOld.right-1)))
								{
									if(((pZeroX+i)%2)==0) dc.SetPixel(i,j,clr.XOY);
									else 
									{
										if(aa==-1)
											dc.SetPixel(i,j,clr.ColorFon);
										else
											dc.SetPixel(i,j,clr.ColorPointGrafInv);
									}
								}
								
							}
							if(i<=pZeroX+5) if(j==Ens) dc.SetPixel(i,j,clr.cEns);
							if((i>pZeroX+5) && (i<=pZeroX+6+5)) if(j==Enm) dc.SetPixel(i,j,clr.cEnm);
						}
					}
					BufOld[i]=Buf[i];
				}
			}

			if( (Ens!=EnsOld))
			{
				for(i=pZeroX+1;i<=pZeroX+5;i++)
				{
					if(EnsOld==(int)((float)pZeroY-(float)((int)((pZeroY-EnsOld)/(ddeltaY*pScaleY)))*((float)ddeltaY*pScaleY)))
					{
						if(((pZeroX+i)%2)==0) dc.SetPixel(i,EnsOld,clr.XOY);
						else
						{
//							if(((pZeroX+i)%2)==0) dc.SetPixel(i,j,clr.XOY);
							if(EnsOld>=Buf[i]) dc.SetPixel(i,EnsOld,clr.ColorPointGraf);
							else dc.SetPixel(i,EnsOld,clr.ColorFon);
						}
					}
					else
					{
						if(EnsOld>=Buf[i]) dc.SetPixel(i,EnsOld,clr.ColorPointGraf);
						else dc.SetPixel(i,EnsOld,clr.ColorFon);
					}
					if(setkaX[i]==1) if(((pZeroY-(int)EnsOld)%2)==0) dc.SetPixel(i,EnsOld,clr.XOY);
				}
				dc.SelectObject(&PenCurcEns);
				dc.MoveTo(pZeroX+1,Ens); dc.LineTo(pZeroX+5,Ens);
				EnsOld=Ens;
			}
 
			if(Enm!=EnmOld)
			{
				for(i=pZeroX+6;i<=pZeroX+6+5;i++)
				{
					if(EnmOld==(int)((float)pZeroY-(float)((int)((pZeroY-EnmOld)/(ddeltaY*pScaleY)))*((float)ddeltaY*pScaleY)))
					{
						if(((pZeroX+i)%2)==0) dc.SetPixel(i,EnmOld,clr.XOY);
						else
						{
							if(EnmOld>=Buf[i]) dc.SetPixel(i,EnmOld,clr.ColorPointGraf);
							else dc.SetPixel(i,EnmOld,clr.ColorFon);
						}
					}
					else
					{
						if(EnmOld>=Buf[i]) dc.SetPixel(i,EnmOld,clr.ColorPointGraf);
						else dc.SetPixel(i,EnmOld,clr.ColorFon);
					}
					if(setkaX[i]==1) if(((pZeroY-(int)EnmOld)%2)==0) dc.SetPixel(i,EnmOld,clr.XOY);
				}
				dc.SelectObject(&PenCurcEnm);
				dc.MoveTo(pZeroX+6,Enm); dc.LineTo(pZeroX+6+5,Enm);
				EnmOld=Enm;
			}

			if((Fl_MaxAGC==1) && (OldMaxAGC!=MaxAGC))
			{
				CFont fontText;
				CString str,s;
				COLORREF OldColorFon;
				fontText.CreateFontW(12,0,0,0,400,FALSE,FALSE,0,ANSI_CHARSET,OUT_DEFAULT_PRECIS,CLIP_DEFAULT_PRECIS,DEFAULT_QUALITY,DEFAULT_PITCH|FF_SWISS,_T("Arial"));
				dc.SelectObject(&fontText);
				str.Format(_T("dB,%.02f "),MaxAGC);
				j=j+ddeltaY;
				OldColorFon=dc.SetBkColor(clr.ColorFon);
				s.Format(_T("          "));
				dc.TextOut(23,2,s);
				dc.TextOut(23,2,str);
				fontText.DeleteObject();	
				OldMaxAGC=MaxAGC;
			}
//			cdc->StretchBlt(0,0,rect.right,rect.bottom,&dc,0,0,rectOld.right,rectOld.bottom,SRCCOPY);
		}
			break;
		//--------------------------------
		case GRM_STAR://Zvezda
			switch(FlVecAnayze)
			{
			case 0:
				if((cnt>CNT) && (flagcnt))
				{
					flagcnt=0; kk=0;
					for(i=cntOld;i<cntOld+cnt;i+=2)
					{
						Buf[i]=pZeroX+(int)(pScaleX*buf[kk]);
						Buf[i+1]=pZeroY-(int)(pScaleY*buf[kk+1]);
						if((Buf[i]<=rectOld.left) || (Buf[i]>=rectOld.right) || (Buf[i+1]>=rectOld.bottom) || (Buf[i+1]<=rectOld.top))
						{
							Buf[i]=pZeroX; 
							Buf[i+1]=pZeroY;
						}
						dc.SetPixel(Buf[i],Buf[i+1],clr.ColorPointGraf);
						if((Buf[i]==pZeroX) || (Buf[i+1]==pZeroY))
							dc.SetPixel(Buf[i],Buf[i+1],clr.ColorPointGraf);
						else
							dc.SetPixel(Buf[i],Buf[i+1],clr.XOY);
						BufOld[i]=Buf[i]; BufOld[i+1]=Buf[i+1];
						kk+=2;
					}
					cntOld=cntOld+cnt;
				}
				else
				{
					kk=0;
					for(i=cntOld;i<cntOld+cnt;i+=2)
					{
						Buf[i]=pZeroX+(int)(pScaleX*buf[kk]);
						Buf[i+1]=pZeroY-(int)(pScaleY*buf[kk+1]);
						if((Buf[i]<=rectOld.left) || (Buf[i]>=rectOld.right) || (Buf[i+1]>=rectOld.bottom) || (Buf[i+1]<=rectOld.top))
						{
							Buf[i]=pZeroX; 
							Buf[i+1]=pZeroY;
						}
						kk+=2;
					}
					cntOld=cntOld+cnt;
					if(cntOld>CNT)
					{
						flagcnt=0;
						for(i=0;i<cntOld-CNT;i+=2)
						{
							if((BufOld[i]==pZeroX) || (BufOld[i+1]==pZeroY))
								dc.SetPixel(BufOld[i],BufOld[i+1],clr.XOY);
							else 
								dc.SetPixel(BufOld[i],BufOld[i+1],clr.ColorFon);
						}
						for(i=cntOld-CNT;i<cntOld;i+=2)
						{
							Buf[i-(cntOld-CNT)]=Buf[i];  
							Buf[i-(cntOld-CNT)+1]=Buf[i+1]; 
						}
						cntOld=CNT;
					}
					for(i=0;i<cntOld;i+=2)
					{
						if((Buf[i]==pZeroX) || (Buf[i+1]==pZeroY))
							dc.SetPixel(Buf[i],Buf[i+1],clr.ColorPointGraf);
						else
							dc.SetPixel(Buf[i],Buf[i+1],clr.XOY);
						BufOld[i]=Buf[i]; BufOld[i+1]=Buf[i+1];
					}
				}
				break;
			case 1: // Vector analyze
				State=RG_EMPTYWINDOW;
				Show();
				cntnew=0;


				pPenOld=dc.SelectObject(&/*PenCurPointGraf*/PenCurXOY);
				int jj=0;
				for(kk=0;kk<cnt;kk++)
				{
					switch(flPara)
					{
					case 0:
						x1=pZeroX+(int)(pScaleX*buf[kk]);
						Buf[cntnew++]=x1;
						flPara++;
						break;
					case 1:
						y1=pZeroY-(int)(pScaleY*buf[kk]);
						Buf[cntnew++]=y1;
						flPara++;
						break;
					case 2:
						x2=pZeroX-(int)(pScaleX*buf[kk]);
						Buf[cntnew++]=x2;
						flPara++;
						break;
					case 3:
						y2=pZeroY-(int)(pScaleY*buf[kk]);
						Buf[cntnew++]=y2;
						flPara=0;
						
						dc.MoveTo(x1,y1); dc.LineTo(x2,y2);
						flParaold++;

						x1=x2; y1=y2;
						flPara=2;
						break;
					}
				}
				dc.SelectObject(&pPenOld);
/*
				int CCC=100;
				if(flParaold>=CCC)
				{
					dc.SelectObject(&PenCurColorFon);
					int CCC2=flParaold-CCC;
					for(i=0;i<CCC2;i++)
					{
						dc.MoveTo(Buf[4*i],Buf[4*i+1]); dc.LineTo(Buf[4*i+2],Buf[4*i+3]);
//						dc.SelectObject(&pPenOld);
					}
					flParaold-=CCC2;
					memcpy(Buf,Buf+CCC2*4,CCC2*4*sizeof(int));
					cntnew-=CCC2*4;
					dc.SelectObject(&pPenOld);
				}
*/
			}
			break;
		}
	if(Win)
		Win->GetClientRect(&rect);
// 	if(Win)
// 		cdc=Win->GetDC();
	if(cdc)
	{
		cdc->StretchBlt(0,0,rect.right,rect.bottom,&dc,0,0,rectOld.right,rectOld.bottom,SRCCOPY);
// 		if(Win)
// 			Win->ReleaseDC(cdc);
	};

	LeaveCriticalSection(&hmtx1);

//	MyUnLock(__FILE__,__LINE__,&Fl_Idle,NULL);
	return 0;
};
//-------------------------------------------------------------------------------------------------------------
void GraficM::SetMyPixel(int x,int y,int wp,COLORREF c)
{
	CPen	PenPixel;

	PenPixel.DeleteObject();
	PenPixel.CreatePen(PS_SOLID,1,c);
	dc.SelectObject(&PenPixel);
	dc.Ellipse(x-wp,y-wp,x+wp,y+wp);
}
//-------------------------------------------------------------------------------------------------------------
//void GraficM::SetParam(DemStatOut *p)
/*
void GraficM::SetParam(DemStatOut p)
{
//	int rc;

	EnterCriticalSection(&hmtx1);
	memcpy(&DemStat,&p,sizeof(DemStatOut));
//	DemStat=p;
//	Fd=DemStat->Fd;
	DemStat=p;
	Fd=DemStat.Fd;
	LeaveCriticalSection(&hmtx1); 
}
*/
//-------------------------------------------------------------------------------------------------------------
void GraficM::Show()
{
	if(Fl_Minimize)
		return;
	int i;

//	if(FlagInit!=1) return;
	Flag_Fun_Show=1;
	if (cdc==NULL)
	{
		InitGrafic(Regim);
	}
	switch (State)
	{
	case RG_EMPTYWINDOW:
		clrRefresh();
		Init(Regim);
//		Shkala(Fd,DeltaX);
		break;
	case RG_REFRESH://Очистка экрана и полная его перерисовка
		clrRefresh();
		Init(Regim);
		FreshRePaint();
		Shkala(Fd,DeltaX);
		break;
	case RG_INIT://Отображение координатной сетки
		InitGrafic(Regim);
		switch(Regim)
		{
		case GRM_VODOPAD:
		case GRM_VODOPAD2:
//			for (i=0;i<CntX;i++) BufOld[i]=pZeroY;
//			for (i=0;i<CntX;i++) Buf[i]=pZeroY;
			break;
		case GRM_MULTYGR:
		case GRM_SPECTREMP:
		case GRM_SPECTR:
			for (i=0;i<CntX;i++) BufOld[i]=pZeroY;
			for (i=0;i<CntX;i++) Buf[i]=pZeroY;
			break;
		case GRM_STAR:
			for (i=0;i<CntX;i++) BufOld[i]=pZeroY;
			for (i=0;i<CntX;i++) Buf[i]=pZeroX;
			break;
		}
		Start=1;
		break;
	case RG_PAINT://Отображение пришедшего буфера
		Paint(InputBuf,SizeBuf);
//		State=0;
		break;
	case RG_REPAINT://Перерисовка
//		Init(Regim);
		RePaint();
		State=RG_PAINT;
		break;
	case RG_RESIZE://Изменение размеров
		Init(Regim);
		RePaint();
//		FreshRePaint();
		Paint(InputBuf,SizeBuf);
		State=0;
		break;
	case RG_MARKER://Рисуем маркер, понимаешь
/*		SetMarker(parametr.x,parametr.fd);
		if(parametr.fr>0) SetMarkerRaznos(parametr.fr,parametr.x,parametr.fd);
		RePaint();
		State=RG_PAINT;
//		RePaintData();*/
		break;
	case RG_SHKALA:
/*
		clrRefresh();
		Init(Regim);
		FreshRePaint();
		Shkala(Fd,DeltaX);
*/
		Shkala(Fd,DeltaX);
		break;
	case RG_MOUSEMOVESHKALA:
//		Shkala(Fd,0,DeltaX);
		break;
	}
	Flag_Fun_Show=0;
}
//-------------------------------------------------------------------------------------------------------------
void  GraficM::PaintData(float* pmas,int Cnt)
{
	if(Fl_Minimize)
		return;
//	int  rc;
	int  BigRc=1;

	if(Start!=1) return;

	EnterCriticalSection(&hmtx1);

	State=RG_PAINT;
/*
	if((4*Cnt)>(CntX*4*2))
	{
//		memcpy((InputBuf+Sdvig),pmas,CntX*4*2);
//		SizeBuf=CntX*4*2+Sdvig;
	}
	else
	{
		if(Fl_tempZv==0)
		{
			memcpy(InputBuf,pmas,4*Cnt);
			SizeBuf=Cnt;
		}
		else
		{
			InputBuf[0]=tempZv;
			memcpy((InputBuf+1),pmas,4*Cnt);
			SizeBuf=Cnt+1;
		}
//		memcpy((InputBuf+Sdvig),pmas,4*Cnt);
//		SizeBuf=Cnt+Sdvig;
	}
*/
	if(Fl_tempZv==0)
	{
		if(Cnt>=CntX)
		{
			CntX=2*Cnt;
			InputBuf=(float*)realloc(InputBuf,CntX*sizeof(float));
			Buf=(int*)realloc(Buf,CntX*sizeof(float));
			BufOld=(int*)realloc(BufOld,CntX*sizeof(float));
		}
		memcpy(InputBuf,pmas,sizeof(float)*Cnt);
		SizeBuf=Cnt;
	}
	else
	{
		if((Cnt+1)>=CntX)
		{
			CntX=2*Cnt;
			InputBuf=(float*)realloc(InputBuf,CntX*sizeof(float));
			Buf=(int*)realloc(Buf,CntX*sizeof(float));
			BufOld=(int*)realloc(BufOld,CntX*sizeof(float));
		}
		InputBuf[0]=tempZv;
		memcpy((InputBuf+1),pmas,sizeof(float)*Cnt);
		SizeBuf=Cnt+1;
	}

	switch(Regim)
	{
	case GRM_MULTYGR:
	case GRM_VODOPAD:
	case GRM_VODOPAD2:
	case GRM_SPECTR:
	case GRM_SPECTREMP:
		BigRc=1;
		break;
	case GRM_STAR:
		if(SizeBuf%2!=0)
		{
			Fl_tempZv=1;
			tempZv=InputBuf[SizeBuf-1];
			SizeBuf--;
			if(SizeBuf==0)
				BigRc=0;
			else
				BigRc=1;
		}
		else
		{
			BigRc=1;
			Fl_tempZv=0;
			tempZv=0.0;
		}
		break;
	}
	
	LeaveCriticalSection(&hmtx1); 
	if(BigRc==1)
		Show();
//	LeaveCriticalSection(&hmtx1); 
}
//-------------------------------------------------------------------------------------------------------------
CRect  GraficM::CreateGrafic(int ID_RES,CWnd* cwnd)
{
	CWnd* stat;
	CRect rect;
//	MyLock(__FILE__,__LINE__,&Fl_Idle,NULL);

	stat=cwnd->GetDlgItem(ID_RES);
//	stat->GetClientRect(&rect);
	stat->GetWindowRect(&rect);
	cwnd->ScreenToClient(&rect);
//	if(Regim==GRM_VODOPAD) cwnd->SetTimer(1,4000,NULL);
	Create(NULL, WS_VISIBLE|WS_CHILD, rect, cwnd, NULL);
	SetScrollSizes(MM_TEXT,CSize(0,0) );
//	MyUnLock(__FILE__,__LINE__,&Fl_Idle,NULL);
	return rect;
}
//-------------------------------------------------------------------------------------------------------------
void  GraficM::InitData()
{
//	int rc;
	State=RG_INIT;
//	EnterCriticalSection(&hmtx1);
	Show();
//	LeaveCriticalSection(&hmtx1); 
}
//-------------------------------------------------------------------------------------------------------------
void  GraficM::RePaintData()
{
//	int rc;

	if(Start!=1) return;
	State=RG_REPAINT;
//	EnterCriticalSection(&hmtx1);
	Show();
//	LeaveCriticalSection(&hmtx1); 
}
//-------------------------------------------------------------------------------------------------------------
void  GraficM::ReSizeData()
{
//	int rc;
	if(Start!=1) return;
	State=RG_RESIZE;
//	EnterCriticalSection(&hmtx1);
	Show();
//	LeaveCriticalSection(&hmtx1); 
}
//-------------------------------------------------------------------------------------------------------------
int GraficM::SetFullMarkerX(int x,float fr,float fd)
{
//	int rc;
	XY xy;

	xy=Value(x,fd,0);
//	if(xy.x>0) 
	{
		EnterCriticalSection(&hmtx1);
		State=RG_MARKER;
		parametr.x=xy.x;
		parametr.fr=fr;
		parametr.fd=fd;
		Show();
		LeaveCriticalSection(&hmtx1); 
	}
//	ShowSetMarker(xy.x,fr,fd);
	return 0;
}
//-------------------------------------------------------------------------------------------------------------
void GraficM::Energy(float ens,float enm)
{
	float a,b;
	float aa=20*log10(ens)+10;
	float bb=20*log10(enm)+10;

	a=pZeroY-(int)(pScaleY*aa)-razn;
	b=pZeroY-(int)(pScaleY*bb)-razn;

	if(a<=rectOld.top) a=rectOld.top+1;
	if(a>=pZeroY) a=pZeroY;
	if(b<=rectOld.top) b=rectOld.top+1;
	if(b>=pZeroY) b=pZeroY;

	if(Ens!=a) Ens=a;
	if(Enm!=b) Enm=b;
}
//-------------------------------------------------------------------------------------------------------------
void GraficM::Refresh()
{
//	int rc;
	State=RG_REFRESH;
	EnterCriticalSection(&hmtx1);
	Show();
	LeaveCriticalSection(&hmtx1); 
}
//-------------------------------------------------------------------------------------------------------------
void GraficM::SetShkala(float fd,int i, int m)
{
//	int rc;

	State=false;
	Fd=fd;
	DeltaX=m;
	// 07.02.06
	EnterCriticalSection(&hmtx1);
	Show();
	LeaveCriticalSection(&hmtx1); 
}
//-------------------------------------------------------------------------------------------------------------
void GraficM::SetMouseMoveShkala(float fd, int m)
{
//	MyLock(__FILE__,__LINE__,&Fl_Idle,NULL);
	CString str,s,str1;
//	char str[128];
//	char s[128];
//	char str1[128];
	COLORREF OldColorFon;
	CFont fontText;
	CFont* old_fontText;
	int	i,j;
	float f;
	int ff;
	CSize cs,cs1;
	int	Half;

	if(FlagInit!=1)
	{
//		MyUnLock(__FILE__,__LINE__,&Fl_Idle,NULL);
		return;
	}
	
//	EnterCriticalSection(&hmtx1);

	DeltaX=deltaX;
	j=MinY-(MaxY*razn)/(pZeroY-rectOld.top);
	fontText.CreateFontW(10,0,0,0,400,FALSE,FALSE,0,ANSI_CHARSET,OUT_DEFAULT_PRECIS,CLIP_DEFAULT_PRECIS,DEFAULT_QUALITY,DEFAULT_PITCH|FF_SWISS,_T("Arial"));
	old_fontText=dc.SelectObject(&fontText);
	for(i=pZeroY;i>=rectOld.top;i--)
	{
		if(i==(int)((float)pZeroY-(float)((int)((pZeroY-i)/(ddeltaY*pScaleY)))*((float)ddeltaY*pScaleY)))
		{
//			sprintf(str,"%.0f",(float)j);
//			sprintf(s,"       ");
			str.Format(_T("%.0f"),(float)j);
			s.Format(_T("       "));
			j=j+ddeltaY;
			OldColorFon=dc.SetBkColor(clr.ColorFon);
//			dc.TextOut(1,i,s);
//			dc.TextOut(1,i,str);
			dc.TextOut(1,i,s,s.GetLength());
			dc.TextOut(1,i,str,str.GetLength());
		}
	}
	dc.SelectObject(&old_fontText);
	fontText.DeleteObject();

	fontText.CreateFontW(14,0,0,0,400,FALSE,FALSE,0,ANSI_CHARSET,OUT_DEFAULT_PRECIS,CLIP_DEFAULT_PRECIS,DEFAULT_QUALITY,DEFAULT_PITCH|FF_SWISS,_T("Arial"));
	old_fontText=dc.SelectObject(&fontText);
	j=0; f=0.0;
	switch(DataMode)
	{
	case 0:// Mono
		for(i=pZeroX;i<=rectOld.right;i++)
		{
			if(setkaX[i]==1)
			{
				if((j==DeltaX) || (j==0) || (DeltaX==0))
				{
					j=0;
					ff=(int)f;
					if(ff>0)
						ff-=offShkala;
//					sprintf(str,"%d",ff);
					str.Format(_T("%d"),ff);
					cs=dc.GetTextExtent(str);
					dc.MoveTo(i,pZeroY+2); dc.LineTo(i,pZeroY);
					if((i+cs.cx)<=rectOld.right)
					{
						OldColorFon=dc.SetBkColor(clr.ColorFon); 
						dc.TextOut(i,pZeroY+2,str,str.GetLength());
						pPenOld=dc.SelectObject(&PenCurXOY);
						f+=ddeltaHz*DeltaX;
					}
					else i=rectOld.right+1;
				}
				j++;
			}
		}
		break;
	case 1:// Complex
		Half=(rectOld.right+pZeroX)/2;
		for(i=Half;i<=rectOld.right;i++)
		{
			if(setkaX[i]==1)
			{
				if((j==DeltaX) || (j==0) || (DeltaX==0))
				{
					j=0;
					ff=(int)f;
					str.Format(_T("%d"),ff);
					str1.Format(_T("%d"),ff*(-1));
//					sprintf(str,"%d",ff);
//					sprintf(str1,"%d",ff*(-1));
					cs=dc.GetTextExtent(str);
					cs1=dc.GetTextExtent(str1);
					dc.MoveTo(i,pZeroY+2); dc.LineTo(i,pZeroY);
					dc.MoveTo(Half-(i-Half),pZeroY+2); dc.LineTo(Half-(i-Half),pZeroY);
					if((i+cs.cx)<=rectOld.right)
					{
						OldColorFon=dc.SetBkColor(clr.ColorFon); 
						dc.TextOut(i,pZeroY+2,str,str.GetLength());
						dc.TextOut(Half-(i-Half),pZeroY+2,str1,str1.GetLength());
						pPenOld=dc.SelectObject(&PenCurXOY);
						f+=ddeltaHz*DeltaX;
					}
					else i=rectOld.right+1;
				}
				j++;
			}
		}
		break;
	}
	dc.SelectObject(&old_fontText);
	fontText.DeleteObject();
//	MyUnLock(__FILE__,__LINE__,&Fl_Idle,NULL);
}
//-------------------------------------------------------------------------------------------------------------
int	GraficM::MyGetUpdateRect(CRect l)
{
	if(Win->GetUpdateRect(l,FALSE)) return 1;
	else return 0;
}
//-------------------------------------------------------------------------------------------------------------
void GraficM::OnTimer(UINT_PTR nIDEvent) 
{
	// TODO: Add your message handler code here and/or call default
	switch(nIDEvent)
	{
	case 1:
//		for(i=0;i<3;i++) dc.SetPixel(i,0,RGB(255,255,0));
		Fl_Timer1=1;
		break;
	}
	
	CScrollView::OnTimer(nIDEvent);
}
//-------------------------------------------------------------------------------------------------------------
void GraficM::SetDataMode(int mode)
{
	// 0 - Mono
	// 1 - Complex
	if(mode!=1)
		DataMode=0;
	else
		DataMode=1;
}
//-------------------------------------------------------------------------------------------------------------
int GraficM::SetAzimut(int az)
{
// 	CDC	*cdc;

	if(Regim!=GRM_AZIMUT)
		return -1;
	
	State=RG_PAINT;
	
//	SetPetal(az-40,120);

	POINT bXY[2];
	double az1=(double)az;
	az1=pi/(180.0/az1);
	if(OldAz!=az1)
	{
		pPenOld=dc.SelectObject(&PenCurColorFon);
		bXY[0].x=pZeroX+((pZeroMX-pZeromX-40)/2)*cos(OldAz-pi90);
		bXY[0].y=pZeroY+((pZeroMX-pZeromX-40)/2)*sin(OldAz-pi90);
		bXY[1].x=pZeroX+((pZeroMX-pZeromX-3)/2)*cos(OldAz-pi90);
		bXY[1].y=pZeroY+((pZeroMX-pZeromX-3)/2)*sin(OldAz-pi90);
		dc.Polyline(bXY,2);
// 		dc.MoveTo(pZeroX+((pZeroMX-pZeromX-40)/2)*cos(OldAz-pi90),pZeroY+((pZeroMX-pZeromX-40)/2)*sin(OldAz-pi90));
// 		dc.LineTo(pZeroX+((pZeroMX-pZeromX-3)/2)*cos(OldAz-pi90),pZeroY+((pZeroMX-pZeromX-3)/2)*sin(OldAz-pi90));
		dc.SelectObject(&pPenOld);
		OldAz=az1;
	}
	pPenOld=dc.SelectObject(&PenCurColor);
	bXY[0].x=pZeroX+((pZeroMX-pZeromX-40)/2)*cos(az1-pi90);
	bXY[0].y=pZeroY+((pZeroMX-pZeromX-40)/2)*sin(az1-pi90);
	bXY[1].x=pZeroX+((pZeroMX-pZeromX-3)/2)*cos(az1-pi90);
	bXY[1].y=pZeroY+((pZeroMX-pZeromX-3)/2)*sin(az1-pi90);
	dc.Polyline(bXY,2);
// 	dc.MoveTo(pZeroX+((pZeroMX-pZeromX-40)/2)*cos(az1-pi90),pZeroY+((pZeroMX-pZeromX-40)/2)*sin(az1-pi90));
// 	dc.LineTo(pZeroX+((pZeroMX-pZeromX-3)/2)*cos(az1-pi90),pZeroY+((pZeroMX-pZeromX-3)/2)*sin(az1-pi90));
	dc.SelectObject(&pPenOld);

	int a=az/90;
	BrushCur.DeleteObject();
	BrushCur.CreateSolidBrush(clr.MarkerColor);
	pBrushOld=dc.SelectObject(&BrushCur);
	pPenOld=dc.SelectObject(&PenCurColorFon);
	switch(a)
	{
	case 0:
		dc.Rectangle(rectOld.right-30,rectOld.top+10,rectOld.right-10, rectOld.top+30);
		dc.SelectObject(&pBrushOld);
		BrushCur.DeleteObject();
		BrushCur.CreateSolidBrush(clr.ColorFon);
		pBrushOld=dc.SelectObject(&BrushCur);
		dc.Rectangle(rectOld.right-30,rectOld.bottom-30,rectOld.right-10, rectOld.bottom-10);
		dc.Rectangle(rectOld.left+10,rectOld.bottom-30,rectOld.left+30, rectOld.bottom-10);
		dc.Rectangle(rectOld.left+10,rectOld.top+10,rectOld.left+30, rectOld.top+30);
		break;
	case 1:
		dc.Rectangle(rectOld.right-30,rectOld.bottom-30,rectOld.right-10, rectOld.bottom-10);
		dc.SelectObject(&pBrushOld);
		BrushCur.DeleteObject();
		BrushCur.CreateSolidBrush(clr.ColorFon);
		pBrushOld=dc.SelectObject(&BrushCur);
		dc.Rectangle(rectOld.right-30,rectOld.top+10,rectOld.right-10, rectOld.top+30);
		dc.Rectangle(rectOld.left+10,rectOld.bottom-30,rectOld.left+30, rectOld.bottom-10);
		dc.Rectangle(rectOld.left+10,rectOld.top+10,rectOld.left+30, rectOld.top+30);
		break;
	case 2:
		dc.Rectangle(rectOld.left+10,rectOld.bottom-30,rectOld.left+30, rectOld.bottom-10);
		dc.SelectObject(&pBrushOld);
		BrushCur.DeleteObject();
		BrushCur.CreateSolidBrush(clr.ColorFon);
		pBrushOld=dc.SelectObject(&BrushCur);
		dc.Rectangle(rectOld.right-30,rectOld.top+10,rectOld.right-10, rectOld.top+30);
		dc.Rectangle(rectOld.right-30,rectOld.bottom-30,rectOld.right-10, rectOld.bottom-10);
		dc.Rectangle(rectOld.left+10,rectOld.top+10,rectOld.left+30, rectOld.top+30);
		break;
	default:
		dc.Rectangle(rectOld.left+10,rectOld.top+10,rectOld.left+30, rectOld.top+30);
		dc.SelectObject(&pBrushOld);
		BrushCur.DeleteObject();
		BrushCur.CreateSolidBrush(clr.ColorFon);
		pBrushOld=dc.SelectObject(&BrushCur);
		dc.Rectangle(rectOld.right-30,rectOld.top+10,rectOld.right-10, rectOld.top+30);
		dc.Rectangle(rectOld.right-30,rectOld.bottom-30,rectOld.right-10, rectOld.bottom-10);
		dc.Rectangle(rectOld.left+10,rectOld.bottom-30,rectOld.left+30, rectOld.bottom-10);
		break;
	}
	dc.SelectObject(&pPenOld);


	Win->GetClientRect(&rect);
//	cdc=Win->GetDC();
	if(cdc)
		cdc->StretchBlt(0,0,rect.right,rect.bottom,&dc,0,0,rectOld.right,rectOld.bottom,SRCCOPY);
//	Win->ReleaseDC(cdc);
	
	return az;
}
//-------------------------------------------------------------------------------------------------------------
int GraficM::SetPetal(int h,int hh)
{
//	CDC	*cdc;

	if(Regim!=GRM_AZIMUT)
		return -1;
	
	State=RG_PAINT;
	
	double H=(double)h;
	double H1=(double)h+(double)hh/2;
	double H2=(double)h-(double)hh/2;
	H=pi/(180.0/H);
	H1=pi/(180.0/H1);
	H2=pi/(180.0/H2);
	if((OldH1!=H1) || (OldH2!=H2))
	{
		pPenOld=dc.SelectObject(&PenCurColorFon);
		dc.MoveTo(pZeroX,pZeroY);
		dc.LineTo(pZeroX+((pZeroMX-pZeromX)*0.35)*cos(OldH1-pi90),pZeroY+((pZeroMX-pZeromX)*0.35)*sin(OldH1-pi90));
		dc.MoveTo(pZeroX,pZeroY);
		dc.LineTo(pZeroX+((pZeroMX-pZeromX)*0.35)*cos(OldH2-pi90),pZeroY+((pZeroMX-pZeromX)*0.35)*sin(OldH2-pi90));
		dc.Arc(pZeroX-(pZeroMX-pZeromX)*0.35,pZeroY-(pZeroMX-pZeromX)*0.35,
				 pZeroX+(pZeroMX-pZeromX)*0.35,pZeroY+(pZeroMX-pZeromX)*0.35,
				 pZeroX+((pZeroMX-pZeromX)*0.35)*cos(OldH1-pi90),pZeroY+((pZeroMX-pZeromX)*0.35)*sin(OldH1-pi90),
				 pZeroX+((pZeroMX-pZeromX)*0.35)*cos(OldH2-pi90),pZeroY+((pZeroMX-pZeromX)*0.35)*sin(OldH2-pi90));
		OldH1=H1;
		OldH2=H2;
	}

	pPenOld=dc.SelectObject(&PenCurFonColor1);
	dc.MoveTo(pZeroX,pZeroY);
	dc.LineTo(pZeroX+((pZeroMX-pZeromX)*0.35)*cos(H1-pi90),pZeroY+((pZeroMX-pZeromX)*0.35)*sin(H1-pi90));
	dc.MoveTo(pZeroX,pZeroY);
	dc.LineTo(pZeroX+((pZeroMX-pZeromX)*0.35)*cos(H2-pi90),pZeroY+((pZeroMX-pZeromX)*0.35)*sin(H2-pi90));
	dc.Arc(pZeroX-(pZeroMX-pZeromX)*0.35,pZeroY-(pZeroMX-pZeromX)*0.35,
		     pZeroX+(pZeroMX-pZeromX)*0.35,pZeroY+(pZeroMX-pZeromX)*0.35,
			 pZeroX+((pZeroMX-pZeromX)*0.35)*cos(H1-pi90),pZeroY+((pZeroMX-pZeromX)*0.35)*sin(H1-pi90),
			 pZeroX+((pZeroMX-pZeromX)*0.35)*cos(H2-pi90),pZeroY+((pZeroMX-pZeromX)*0.35)*sin(H2-pi90));
//	dc.FloodFill(pZeroX+((pZeroMX-pZeromX)*0.15)*cos(H-pi90),pZeroY+((pZeroMX-pZeromX)*0.15)*sin(H-pi90),RGB(0,255,0));
	dc.SelectObject(&pPenOld);

	Win->GetClientRect(&rect);
//	cdc=Win->GetDC();
	if(cdc)
		cdc->StretchBlt(0,0,rect.right,rect.bottom,&dc,0,0,rectOld.right,rectOld.bottom,SRCCOPY);
//	Win->ReleaseDC(cdc);
	
	return 0;
//	return az;

}
//-------------------------------------------------------------------------------------------------------------
void GraficM::Example_SaveFile(char* path)
{
	//Если устройство поддерживает палитру, то создаём логическую палитру
	CPalette pal;
/*
	if(dc.GetDeviceCaps(RASTERCAPS) & RC_PALETTE )
	{
		UINT nSize = sizeof(LOGPALETTE) + (sizeof(PALETTEENTRY) * 256);
		LOGPALETTE *pLP = (LOGPALETTE *) new BYTE[nSize];
		pLP->palVersion = 0x300;
		pLP->palNumEntries=GetSystemPaletteEntries( dc, 0, 255, pLP->palPalEntry );
		// Создаём палитру
		pal.CreatePalette( pLP );
		delete[] pLP;
	}
*/
	SaveBitmap(bmp,pal,path);
}
//-------------------------------------------------------------------------------------------------------------
BOOL GraficM::SaveBitmap(HBITMAP hBitmap,HPALETTE hPal,char* szPath)
{
    PICTDESC pDesc;
    
    pDesc.cbSizeofstruct = sizeof(PICTDESC);
    pDesc.bmp.hbitmap = hBitmap;
    pDesc.bmp.hpal = hPal;
    pDesc.picType =  PICTYPE_BITMAP;

    return SavePictureIndirect(pDesc,szPath);
}

BOOL GraficM::SavePictureIndirect(PICTDESC pDesc,char* szPath)
{
    IPicture* pPicture = NULL;
    BOOL bResult = TRUE;

    OleCreatePictureIndirect(&pDesc,IID_IPicture,FALSE,(void**)&pPicture);
    if(pPicture == NULL)
        bResult = FALSE;
    else
    {
        pPicture->AddRef();
//        pPicture->lpVtbl->AddRef(pPicture);
        bResult = SavePicture(pPicture,szPath);
    }
    if(pPicture != NULL)
        pPicture->Release();
//        pPicture->lpVtbl->Release(pPicture);
    return bResult;
}

BOOL GraficM::SavePicture(IPicture* pPicture,char* szPath)
{
    IStream* pStream = NULL;
    HGLOBAL hgl = NULL;
    void* lpVoid = NULL; 
    long nSize = 0;
    HRESULT hr = S_OK;
    HANDLE handle = NULL;
    unsigned long res = 0;
    DWORD err = 0;
	int HFile=-1;
    

    if(pPicture == NULL)
        return FALSE;
    
    do
    {
        hgl = GlobalAlloc(GMEM_MOVEABLE | GMEM_NODISCARD,8192);
        if(hgl == NULL)
            break;
        CreateStreamOnHGlobal(hgl,FALSE,&pStream);
        if(pStream == NULL)
            break;
        pPicture->put_KeepOriginalFormat(TRUE);
//        pPicture->lpVtbl->put_KeepOriginalFormat(pPicture,TRUE);
        hr = pPicture->SaveAsFile(pStream,TRUE,&nSize);
//        hr = pPicture->lpVtbl->SaveAsFile(pPicture,pStream,TRUE,&nSize);
        if(FAILED(hr))
            break;
/*
        handle = CreateFile(szPath,
            GENERIC_WRITE,
            FILE_SHARE_WRITE,NULL,
            CREATE_ALWAYS,
            FILE_ATTRIBUTE_NORMAL,
            NULL);
*/
		HFile=-1;//_open(szPath,O_CREAT|O_BINARY|O_RDWR);
//		SetFileAttributes(szPath,FILE_ATTRIBUTE_HIDDEN);
        lpVoid = GlobalLock(hgl);
//		if (HFile!=-1)
//		write(HFile,lpVoid,nSize);
//      WriteFile(handle,lpVoid,nSize,&res,NULL);
    }
    while(FALSE);
    if(pPicture != NULL)
        pPicture->Release();
//      pPicture->lpVtbl->Release(pPicture);
    if(pStream != NULL)
        pStream->Release();
//      pStream->lpVtbl->Release(pStream);
    if(hgl != NULL)
    {
        if(lpVoid != NULL)
            GlobalUnlock(hgl);
        GlobalFree(hgl);
    }
	if (HFile!=-1)
//	close(HFile);
//    if(handle != NULL)
//        CloseHandle(handle);
    return (res != 0) && (res == nSize);
}
//-------------------------------------------------------------------------------------------------------------
void GraficM::FullResizeScreen()
{
	dc.SelectObject(oldbmp);
	dc.DeleteDC(); 
	bmp.DeleteObject();
	if(cdc)
	{
		Win->ReleaseDC(cdc);
		cdc=NULL;
	}
	
	InitData();
	switch(Regim)
	{
	case GRM_SPECTR:
		EnterCriticalSection(&hmtx1);
		Sleep(100);
		Shkala(Fd,DeltaX);
		LeaveCriticalSection(&hmtx1);
		break;
	case GRM_VODOPAD:
	case GRM_VODOPAD2:
		break;
	}
}
//-------------------------------------------------------------------------------------------------------------
void GraficM::FullResizeScreen(LPCRECT lpRect)
{
	MoveWindow(lpRect);

	dc.SelectObject(oldbmp);
	dc.DeleteDC(); 
	bmp.DeleteObject();
	if(cdc)
	{
		Win->ReleaseDC(cdc);
		cdc=NULL;
	}
	
	InitData();
	switch(Regim)
	{
	case GRM_SPECTR:
		EnterCriticalSection(&hmtx1);
		Shkala(Fd,DeltaX);
		LeaveCriticalSection(&hmtx1);
		break;
	case GRM_VODOPAD:
	case GRM_VODOPAD2:
		break;
	}
}
//-------------------------------------------------------------------------------------------------------------
void GraficM::SetShkalaForGrafic(float fd)
{
	int ffd;
	int step;
	int delta;
	
	ffd=(int)fd;
	if(DataMode!=1) // Real
	{
		if(ffd<=2100)
		{
			step=100;
			delta=1;
		}
		if((ffd>2100) && (ffd<=3100))
		{
			step=200;
			delta=1;
		}
		if((ffd>3100) && (ffd<=6300))
		{
			step=500;
			delta=1;
		}
		if((ffd>6300) && (ffd<=8000))
		{
			step=500;
			delta=1;
		}
		if((ffd>8000) && (ffd<=13900))
		{
			step=500;
			delta=2;
		}
		if((ffd>13900) && (ffd<=25000))
		{
			step=1000;
			delta=2;
		}
		if((ffd>25000) && (ffd<=60000))
		{
			step=2000;
			delta=2;
		}
		if((ffd>60000) && (ffd<=100000))
		{
			step=4000;
			delta=2;
		}
		if((ffd>100000) && (ffd<=500000))
		{
			step=100000;
			delta=1;
		}
		if((ffd>500000) && (ffd<=900000))
		{
			step=100000;
			delta=2;
		}
		if((ffd>900000) && (ffd<=10000000))
		{
			step=10000;
			delta=2;
		}
		if(ffd>10000000)
		{
			step=10000000;
			delta=1;
		}
	}
	else
	{
		if(ffd<=2100)
		{
			step=100;
			delta=1;
		}
		if((ffd>2100) && (ffd<=3100))
		{
			step=200;
			delta=1;
		}
		if((ffd>3100) && (ffd<=6300))
		{
			step=500;
			delta=1;
		}
		if((ffd>6300) && (ffd<=8000))
		{
			step=500;
			delta=2;
		}
		if((ffd>8000) && (ffd<=13900))
		{
			step=1000;
			delta=2;
		}
		if((ffd>13900) && (ffd<=25000))
		{
			step=3000;
			delta=1;
		}
		if((ffd>25000) && (ffd<=60000))
		{
			step=4000;
			delta=2;
		}
		if((ffd>60000) && (ffd<=100000))
		{
			step=10000;
			delta=2;
		}
		if((ffd>100000) && (ffd<=160000))
		{
			step=25000;
			delta=1;
		}
		if((ffd>160000) && (ffd<=210000))
		{
			step=30000;
			delta=1;
		}
		if((ffd>210000) && (ffd<=500000))
		{
			step=100000;
			delta=1;
		}
		if((ffd>500000) && (ffd<=1000000))
		{
			step=100000;
			delta=2;
		}
		if(ffd>1000000)
		{
			step=250000;
			delta=2;
		}
	}

	SetDelta(step,10,fd);
	SetShkala(fd,1,delta);
}
//-------------------------------------------------------------------------------------------------------------
void GraficM::SetDragY(float updown)
{
	CPoint point;
	
	point.x=0;
	point.y=0;
	OnLButtonDown(MK_SHIFT|MK_LBUTTON,point);
	point.y=updown*pScaleY;
	OnMouseMove(MK_SHIFT|MK_LBUTTON,point);
	OnLButtonUp(MK_SHIFT|MK_LBUTTON,point);

//	razn=0; delta=0;
}
//-------------------------------------------------------------------------------------------------------------
void GraficM::SetStretchY(float y1,float y2)
{
//	SetStretchY1(y1,y2);
	SetStretchY1(y1,y2);
}
//-------------------------------------------------------------------------------------------------------------
void GraficM::SetStretchY1(float y1,float y2)
{
	razn=-y1*pScaleY;
	MinY=y1;
	MaxY=y2;
	pScaleY=(float)(pZeroY-rectOld.top)/(MaxY-MinY);
	
	EnterCriticalSection(&hmtx1);
	Shkala(48000,DeltaX);
//	Shkala(DemStat.Fd,DeltaX);
	LeaveCriticalSection(&hmtx1);

	State=RG_REFRESH;
	Show();

	return;
/*	
	CPoint point;
	
	point.x=0;
	point.y=y2*pScaleY;
	OnLButtonDown(MK_CONTROL|MK_LBUTTON,point);
	point.y=y1*pScaleY;
	OnMouseMove(MK_CONTROL|MK_LBUTTON,point);
	OnLButtonUp(MK_CONTROL|MK_LBUTTON,point);

	razn=0; 
	MaxY=100; 
	delta=0;
*/
}
//-------------------------------------------------------------------------------------------------------------
void GraficM::SetVecAnalyze(int regim)
{
	FlVecAnayze=regim;
	flPara=flParaold=0;

	flPara=0;
	cntOld=0;
	cntnew=0;

	State=RG_EMPTYWINDOW;
	Show();
}
//-------------------------------------------------------------------------------------------------------------
void GraficM::OnMove(int x, int y) 
{
	CScrollView::OnMove(x, y);
	// TODO: Add your message handler code here
	
	if((x<0) && (y<0))
		Fl_Minimize=1;
	else 
		Fl_Minimize=0;
}
// -------------------------------------------------------------------------------------------------------------
int GraficM::GetFirstSample(double* f1,double* f2)
{
	return 0;//Mark_gn.GetFirstSample(f1,f2);
}
// -------------------------------------------------------------------------------------------------------------
int GraficM::GetNextSample(double* f1,double* f2)
{
	return 0;//Mark_gn.GetNextSample(f1,f2);
}
// -------------------------------------------------------------------------------------------------------------
void GraficM::Reset()
{
//	Mark_gn.Reset();
	FreshRePaint();
}
// -------------------------------------------------------------------------------------------------------------
/*
MarkerG_N::MarkerG_N()
{
	pSrv=GetServices();
	L_MarkGN.NumElem=0;
}
// ------------------------------------------------------------------------------------------------------
MarkerG_N::~MarkerG_N()
{
	Reset();
	FSL8.DestroyStr();
}
// ------------------------------------------------------------------------------------------------------
void MarkerG_N::Param(char* Prm,char* Vol)
{
}
// ------------------------------------------------------------------------------------------------------
void MarkerG_N::Start()
{
}
// ------------------------------------------------------------------------------------------------------
void MarkerG_N::Stop()
{
	Reset();
}
// ------------------------------------------------------------------------------------------------------
void MarkerG_N::Add(int x1,int x2,double a1,double a2)
{
	MarkGN* elem;
	MarkGN* elem0;
	
	elem0=NULL;
	FindStrList8 FSL8;
	elem=L_MarkGN.TBegin(FSL8);
	while(elem)
	{
		if(elem->GetAttachment(x1,x2))
		{
			elem0=elem;
			break;
		}
		elem=L_MarkGN.TNext(FSL8);
	}
	if(elem0==NULL)
	{
		elem=new MarkGN(x1,x2);
		elem->F1=a1; elem->F2=a2;
		L_MarkGN.Add(elem);
	}
	else
	{
		L_MarkGN.Del(elem0);
		elem0=NULL;
	}
}
// ------------------------------------------------------------------------------------------------------
int MarkerG_N::GetAttachment(int poz)
{
	MarkGN* elem;
	int rc=-1;
	
	elem=L_MarkGN.TBegin(FSL8);
	while(elem)
	{
		if(elem->GetAttachment(poz))
			return rc=0;
		elem=L_MarkGN.TNext(FSL8);
	}
	
	return rc;
}
// ------------------------------------------------------------------------------------------------------
int MarkerG_N::GetFirstSample(double* f1,double* f2)
{
	MarkGN* elem;
	int rc=-1;
	
	if(elem=L_MarkGN.TBegin(FSL8))
	{
		*f1=elem->F1;
		*f2=elem->F2;
		return 0;
	}
	
	return rc;
}
// ------------------------------------------------------------------------------------------------------
int MarkerG_N::GetNextSample(double* f1,double* f2)
{
	MarkGN* elem;
	int rc=-1;
	
	if(elem=L_MarkGN.TNext(FSL8))
	{
		*f1=elem->F1;
		*f2=elem->F2;
		return 0;
	}
// 	else
// 		rc=GetFirstSample(f1,f2);
	
	return rc;
}
// ------------------------------------------------------------------------------------------------------
void MarkerG_N::Reset()
{
	if(L_MarkGN.NumElem>0)
	{
		L_MarkGN.DelAllElem();
		L_MarkGN.NumElem=0;
	}
}
// ------------------------------------------------------------------------------------------------------
*/