//This file is part of eMule AI
//Copyright (C)2026 eMule AI

#pragma once
#include "Preferences.h"

#define CONSTATE_NULL			0
#define CONSTATE_ONLINE			1
#define CONSTATE_OFFLINE		2

extern const UINT UWM_CONCHECKER;

class CConChecker
{
	public:
		CConChecker(void);
		~CConChecker(void);

		const bool Start();
		const bool Stop();
		const bool IsActive();

	private:
		static UINT ThreadProc(LPVOID pParam);

	protected:
		bool	m_bRun;
		HANDLE	m_hThread;
		HANDLE  m_hKilled;
};
