//This file is part of eMule AI
//Copyright (C)2026 eMule AI

#include "stdafx.h"
#include "Log.h"
#include "DesktopIntegration.h"

CDesktopIntegration	theDesktop;

CDesktopIntegration::CDesktopIntegration()
{
	m_pWineNT2UnixPath = (t_wine_get_unix_file_name) ::GetProcAddress(GetModuleHandle(_T("KERNEL32")), "wine_get_unix_file_name");
	m_pWineUnix2NTPath = (t_wine_get_dos_file_name) ::GetProcAddress(GetModuleHandle(_T("KERNEL32")), "wine_get_dos_file_name");
	
	m_eDesktop = WINDOWS_DESKTOP;
	if (m_pWineNT2UnixPath != NULL && m_pWineUnix2NTPath != NULL) {
		DebugLog(_T("Detected WINE!"));
		TCHAR	lpBuffer[512];
		if (GetEnvironmentVariable(_T("KDE_FULL_SESSION"), lpBuffer, _countof(lpBuffer)) != 0) {
			DebugLog(_T("Detected KDE!"));
			m_eDesktop = KDE_DESKTOP;
		} else if (GetEnvironmentVariable(_T("GNOME_DESKTOP_SESSION_ID"), lpBuffer, _countof(lpBuffer)) != 0) {
			DebugLog(_T("Detected GNOME!"));
			m_eDesktop = GNOME_DESKTOP;
		}
	}
}

CDesktopIntegration::~CDesktopIntegration()
{
}

BOOL CDesktopIntegration::ShellExecute(LPCTSTR lpOperation, LPCTSTR lpFile, LPCTSTR lpParameters)
{
	CString		strApp(lpFile);
	CString		strParameters(lpParameters);
	
	switch(_GetDesktopType())
	{
		case GNOME_DESKTOP:
		{
			lpOperation = _T("open");
			strApp.Format(_T("\"%s\""), _NativeToWindowsPath("/usr/bin/gnome-open"));
			strParameters.Format(_T("\"%s\""), _WindowsToNativePath(lpFile));
			break;
		}
		case KDE_DESKTOP:
		{
			lpOperation = _T("open");
			strApp.Format(_T("\"%s\""), _NativeToWindowsPath("/usr/bin/kfmclient"));
			strParameters.Format(_T("exec \"file:%s\""), _WindowsToNativePath(lpFile));
			break;
		}
		case WINDOWS_DESKTOP:
		default:
			break;
	}

	if (strParameters.IsEmpty()) {
		DebugLog(_T("ShellExecute: %s %s"), lpOperation, strApp);
		::ShellExecute(NULL, lpOperation, strApp.GetString(), NULL, NULL, SW_SHOW);
		//_SaferShellExecute(NULL, lpOperation, strApp.GetString(), NULL, NULL, SW_SHOW, SAFER_LEVELID_NORMALUSER);
	} else {
		DebugLog(_T("ShellExecute: %s %s %s"), lpOperation, strApp, strParameters);
		::ShellExecute(NULL, lpOperation, strApp.GetString(), strParameters.GetString(), NULL, SW_SHOW);
		//_SaferShellExecute(NULL, lpOperation, strApp.GetString(), strOptions.GetString(), NULL, SW_SHOW, SAFER_LEVELID_NORMALUSER);
	}

	return TRUE;
}

CString CDesktopIntegration::_NativeToWindowsPath(CStringA path)
{
	if (m_pWineUnix2NTPath != NULL)
		return CString(m_pWineUnix2NTPath(path.GetString()));
	else
		return CString(path);
}

CString CDesktopIntegration::_WindowsToNativePath(CStringW path)
{
	if (m_pWineNT2UnixPath != NULL)
		return CString(m_pWineNT2UnixPath(path));
	else
		return CString(path);
}
