//This file is part of eMule AI
//Copyright (C)2026 eMule AI

#include "StdAfx.h"
#include "emule.h"
#include "emuleDlg.h"
#include "conchecker.h"
#include "Log.h"
#include <wininet.h>
#include "Preferences.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#endif

const UINT UWM_CONCHECKER = ::RegisterWindowMessage(_T("UWM_CONCHECKER_{C44CF9E8-06B0-4ce4-A422-53DAE6802A1E}"));

namespace
{
void DrainPendingConCheckerMessages()
{
	if (theApp.emuledlg == NULL)
		return;

	const HWND hWnd = theApp.emuledlg->GetSafeHwnd();
	if (!::IsWindow(hWnd))
		return;

	MSG msg;
	while (::PeekMessage(&msg, hWnd, UWM_CONCHECKER, UWM_CONCHECKER, PM_REMOVE)) {
	}
}
}

CConChecker::CConChecker(void)
{
	m_hThread = NULL;
	m_hKilled = ::CreateEvent(NULL, FALSE, FALSE, _T("thread_iskilled"));
	m_bRun = false;
}

CConChecker::~CConChecker(void)
{
	// Cooperative shutdown; avoid TerminateThread on invalid handle
	m_bRun = false;
	Stop();
	if (m_hKilled) {
		CloseHandle(m_hKilled);
		m_hKilled = NULL;
	}
}

const bool CConChecker::Start()
{
	if (IsActive())
		return false;

	theApp.SetConnectionState(CONSTATE_NULL);
	AddLogLine(false, _T("***"));

	if (thePrefs.GetConnectionChecker()) {
		AddLogLine(false, GetResString(_T("CONN_CHECK_ACTIVE")));
		AddLogLine(false, _T("***"));
	} else {
		AddLogLine(false, GetResString(_T("CONN_CHECK_PASSIVE")));
		AddLogLine(false, _T("***"));
		return false;
	}

	m_bRun = false;
	if (m_hKilled) // Ensure event is unsignaled before start
		::ResetEvent(m_hKilled); // Worker will wait on this
	m_hThread = ::CreateThread(NULL, NULL, (LPTHREAD_START_ROUTINE)ThreadProc, this, CREATE_SUSPENDED, 0);

	if (m_hThread == NULL)
		return false;

	::SetThreadPriority(m_hThread,THREAD_PRIORITY_NORMAL);
	::ResumeThread(m_hThread);
	m_bRun = true;
	return true;
}

const bool CConChecker::Stop()
{
	m_bRun=false;

	if (m_hThread == NULL)
		return true;

	// Signal worker to exit and wait for it
	if (m_hKilled)
		::SetEvent(m_hKilled);

	DWORD waitRes = ::WaitForSingleObject(m_hThread, 5000); // Wait up to 5 seconds

	DrainPendingConCheckerMessages();

	::CloseHandle(m_hThread);
	m_hThread = NULL;

	AddLogLine(false, GetResString(_T("CONN_CHECK_STOPPED")));
	return (waitRes == WAIT_OBJECT_0);
}

const bool CConChecker::IsActive()
{
	if (m_hThread == NULL)
		return false;

	DWORD dwExitCode = 0;		
	GetExitCodeThread(m_hThread,&dwExitCode);

	if (dwExitCode == STILL_ACTIVE)
		return true;

	return false;
}

UINT CConChecker::ThreadProc(LPVOID pProcParam)
{
	CConChecker* pThis = reinterpret_cast<CConChecker*>(pProcParam);
	const HANDLE hKilled = pThis != NULL ? pThis->m_hKilled : NULL;

	if (hKilled == NULL)
		return 0;

#pragma warning(disable:4127) // Disable compiler warning for constant usage in the loop
	while (::WaitForSingleObject(hKilled, 1000) == WAIT_TIMEOUT) {
		if (theApp.IsClosing())
			break;

		const HWND hWnd = theApp.emuledlg != NULL ? theApp.emuledlg->GetSafeHwnd() : NULL;
		if (!::IsWindow(hWnd))
			continue;

		const LPARAM lConnectionState = InternetCheckConnection(thePrefs.GetConnectionCheckerServer(), FLAG_ICC_FORCE_CONNECTION, 0) ? CONSTATE_ONLINE : CONSTATE_OFFLINE;
		if (theApp.IsClosing())
			break;

		::PostMessage(hWnd, UWM_CONCHECKER, (WPARAM)2, lConnectionState);
	}
#pragma warning(default:4127)

	return 0;
}
