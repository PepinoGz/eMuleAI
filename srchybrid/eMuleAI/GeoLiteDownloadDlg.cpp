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

#include "stdafx.h"
#include "emule.h"
#include "GeoLiteDownloadDlg.h"
#include "OtherFunctions.h"
#include "emuledlg.h"
#include "Opcodes.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

// CGeoLiteDownloadDlg dialog

IMPLEMENT_DYNAMIC(CGeoLiteDownloadDlg, CDialog)

CGeoLiteDownloadDlg::CGeoLiteDownloadDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CGeoLiteDownloadDlg::IDD, pParent)
{
}

CGeoLiteDownloadDlg::~CGeoLiteDownloadDlg()
{
}

void CGeoLiteDownloadDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CGeoLiteDownloadDlg, CDialog)
	ON_BN_CLICKED(IDC_GEOLITECITY_DOWNLOAD, OnBnClickedDownload)
END_MESSAGE_MAP()

// CGeoLiteDownloadDlg message handlers

BOOL CGeoLiteDownloadDlg::OnInitDialog()
{
	CDialog::OnInitDialog();
	InitWindowStyles(this);

	SetWindowText(GetResString(_T("EMULE_AI_DL_GEOLITE_TITLE")));
	SetDlgItemText(IDC_GEOLITECITY_TEXT, GetResString(_T("EMULE_AI_DL_GEOLITE_TEXT")));
	SetDlgItemText(IDC_GEOLITECITY_DOWNLOAD, GetResString(_T("DOWNLOAD")));
	SetDlgItemText(IDCANCEL, GetResString(_T("MAIN_BTN_CANCEL")));

	return TRUE;  // return TRUE unless you set the focus to a control
	// EXCEPTION: OCX Property Pages should return FALSE
}

void CGeoLiteDownloadDlg::OnBnClickedDownload()
{
	BrowserOpen(kGeoLiteDownloadUrl, thePrefs.GetMuleDirectory(EMULE_EXECUTABLEDIR));
	EndDialog(IDOK);
}
