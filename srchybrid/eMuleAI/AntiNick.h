//This file is part of eMule AI
//Copyright (C)2026 eMule AI

#pragma once
class CUpDownClient;

class CAntiNick
{
//>>> Global functions
public:
	static	void	Init();
	static	void	UnInit();
//<<< Global functions
//>>> AntiUserNameThief
public:
	//this creates a new string or returns the current one
	static	CString	GetAntiUserNameThiefNick();

	//this returns if we have a UserNameThief
	static	bool	FindOurTagIn(const CString& tocomp);
	static	bool	IsReservedAntiUserNameThiefTag(const CString& tag);
	static	bool	ContainsReservedAntiUserNameThiefTag(const CString& text);

private:
	//this will be automatically called
	static	void	CreateAntiUserNameThiefTag();

	//the string
	static	CString m_sAntiUserNameThiefTag;

	//the update timer
	static	uint32	m_uiAntiUserNameThiefCreateTimer;

//<<< AntiUserNameThief
};

extern CAntiNick theAntiNickClass;
