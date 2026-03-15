#pragma once

class CLanguageSelectDlg : public CDialog
{
public:
	CLanguageSelectDlg();
	enum { IDD = IDD_LANGUAGE_SELECT };

protected:
	CComboBox m_language;
	CStringArray m_langCodes;

	virtual void DoDataExchange(CDataExchange *pDX);
	virtual BOOL OnInitDialog();
	afx_msg void OnOK();
	DECLARE_MESSAGE_MAP()
};

