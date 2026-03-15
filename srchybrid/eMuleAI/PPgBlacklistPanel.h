//This file is part of eMule AI
//Copyright (C)2026 eMule AI

#pragma once
#include "preferences.h"

class CPPgBlacklistPanel : public CPropertyPage
{
	DECLARE_DYNAMIC(CPPgBlacklistPanel)

public:
	CPPgBlacklistPanel();
	virtual ~CPPgBlacklistPanel();
	virtual BOOL OnApply();
	virtual BOOL OnInitDialog();
	virtual BOOL OnKillActive();

	void Localize(void);
	void LocalizeCommonItems(void);
	void LoadSettings(void);
	void ReadBlacklistDefinitionsFromTextbox();

	enum { IDD = IDD_PPG_BLACKLIST_PANEL }; // Dialog Data

protected:
	virtual BOOL OnCommand(WPARAM wParam, LPARAM lParam);
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	virtual BOOL PreTranslateMessage(MSG* pMsg);

	DECLARE_MESSAGE_MAP()
	afx_msg void OnDestroy();
	afx_msg void OnHelp();
	afx_msg BOOL OnHelpInfo(HELPINFO* pHelpInfo);
	afx_msg void OnSettingsChange() { SetModified(); }
	afx_msg HBRUSH OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor);
};