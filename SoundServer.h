
// SoundServer.h: главный файл заголовка для приложения PROJECT_NAME
//

#pragma once

#ifndef __AFXWIN_H__
	#error "include 'pch.h' before including this file for PCH"
#endif

#include "resource.h"		// основные символы
#include	"DecodB.h"
#include	"SockMt.h"


// CSoundServerApp:
// Сведения о реализации этого класса: SoundServer.cpp
//

class CSoundServerApp : public CWinApp
{
public:
	CSoundServerApp();

// Переопределение
public:
	virtual BOOL InitInstance();

// Реализация

	DECLARE_MESSAGE_MAP()
};

extern CSoundServerApp theApp;
