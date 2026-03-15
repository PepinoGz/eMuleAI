//This file is part of eMule AI
//Copyright (C)2026 eMule AI
//
//This program is free software; you can redistribute it and/or
//modify it under the terms of the GNU General Public License
//as published by the Free Software Foundation; either
//version 2 of the License, or (at your option) any later version.
//
//This program is distributed in the hope that it will be useful,
//but WITHOUT ANY WARRANTY; without even the implied warranty of
//MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//GNU General Public License for more details.
//
//You should have received a copy of the GNU General Public License
//along with this program; if not, write to the Free Software
//Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.

#pragma once

static LPCTSTR const kGeoLiteDownloadUrl = _T("https://www.maxmind.com/en/accounts/current/geoip/downloads");

// CGeoLiteDownloadDlg dialog
class CGeoLiteDownloadDlg : public CDialog
{
	DECLARE_DYNAMIC(CGeoLiteDownloadDlg)

public:
	CGeoLiteDownloadDlg(CWnd* pParent = NULL);   // standard constructor
	virtual ~CGeoLiteDownloadDlg();

	enum { IDD = IDD_GEOLITE_DOWNLOAD };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	virtual BOOL OnInitDialog();

	DECLARE_MESSAGE_MAP()
	afx_msg void OnBnClickedDownload();
};
