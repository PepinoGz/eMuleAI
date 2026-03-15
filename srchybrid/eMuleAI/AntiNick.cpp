//This file is part of eMule AI
//Copyright (C)2026 eMule AI

//This class is a changed version of WiZaRd's  AntiLeechClass v1 B3t4
//I (Stulle) removed everything that hasn't to do with AntiUserNameThief
//and renamed the whole class to AntiNick
//Big thanks to WiZaRd for his great work. Thank you.
//
//>>>>>>>>>>>>>>>>>>>>>>> AntiUserNameThief start <<<<<<<<<<<<<<<<<<<<<<<<<<//
//The whole feature is not about detecting clients with the same name but 
//clients using a UserNameThief.
//Though it is annoying, this is not harmful in any way to us!
//BUT clients using a UserNameThief will for 99% also use other bad "features" 
//like a very low sessionupload, fakeranks or even no ratio/0 UL
//
//That's why I want to detect (and ban) them! And the UserNameThief is very 
//easy to detect!
//
//The AntiUserNameThief sends a random string to each user, if the user will 
//adapt that string to his name and send it back, we can (safely) ban him
//
//
//Revision History:
//v2.3: Small fixes, changes and safety loops (don't know how to discribe :D)
//		Removed check for old tag (safe PC ressources) --> inspired by WiZaRd
//		All changes by Stulle
//
//v2.2: added a small fix to always get the mirroring clients
//		even shortly after the 1-update-per-day
//
//v2.1: use one string for all users saving lots of RAM
//		also only update once per day saving a few CPU cycles
//		and also fixed a bug causing garbage and eating up CPU (mea culpa)
//		I know that there will be a short period when we will not detect 
//		nickthieves (i.e. because of the 1-update-per-day) but sooner or later
//		they will run into it ;)
//
//v2:	send random string of random length
//		this should take care of leechers simply cutting the anti-tag 
//		if it has exactly 6 chars
//
//v1:	send random string to each user
//>>>>>>>>>>>>>>>>>>>>>>> AntiUserNameThief end <<<<<<<<<<<<<<<<<<<<<<<<<<<<//

#include "stdafx.h"
#include "AntiNick.h"
#include "Preferences.h" //for GetUserNick()

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#endif

CAntiNick theAntiNickClass;

//>>> AntiUserNameThief
#define MIN_LENGTH	4						//min chars to use
#define MAX_LENGTH	8						//max chars to use
#if (MIN_LENGTH > MAX_LENGTH)
#error MAX_LENGTH MUST BE GREATER THAN OR EQUAL TO MIN_LENGTH
#endif
#define MAX_ADD		(MAX_LENGTH-MIN_LENGTH) //max chars to add
#define MAX_VALID	(24*60*60*1000)			//1 day expiration
CString	CAntiNick::m_sAntiUserNameThiefTag; 
uint32	CAntiNick::m_uiAntiUserNameThiefCreateTimer = NULL;
//<<< AntiUserNameThief

//>>> Global functions
void	CAntiNick::Init()
{
	CreateAntiUserNameThiefTag();
}

void	CAntiNick::UnInit()
{
	// Release tag buffer before CRT leak snapshot.
	m_sAntiUserNameThiefTag.Empty();
	m_sAntiUserNameThiefTag.FreeExtra();
	m_uiAntiUserNameThiefCreateTimer = 0;
}
//<<< Global functions
//>>> AntiUserNameThief
void CAntiNick::CreateAntiUserNameThiefTag()
{
	// don't run this untill it's neccessary - Stulle
	if(::GetTickCount() < m_uiAntiUserNameThiefCreateTimer)
		return;

	CString nick;	
	uint8 maxchar = (uint8)(MIN_LENGTH+rand()%MAX_ADD); //BuGFiX: d'oh - stupid me!!
	for(uint8 i = 0; i < maxchar; ++i)
	{
		if(rand()%2)
			nick.AppendFormat(_T("%c"), _T('A')+rand()%25);
		else
			nick.AppendFormat(_T("%c"), _T('a')+rand()%25);			
	}
	m_sAntiUserNameThiefTag.Format(_T("[AI-%s]"), (LPCTSTR)nick);
	m_uiAntiUserNameThiefCreateTimer = ::GetTickCount()+MAX_VALID;
}

CString	CAntiNick::GetAntiUserNameThiefNick()
{
	if(m_sAntiUserNameThiefTag.IsEmpty()) // no tag¿ - Stulle
		CreateAntiUserNameThiefTag(); // create a tag! - Stulle

	if(::GetTickCount() > m_uiAntiUserNameThiefCreateTimer)
		CreateAntiUserNameThiefTag();

	return m_sAntiUserNameThiefTag;
}

bool CAntiNick::FindOurTagIn(const CString& tocomp)
{
	if(m_sAntiUserNameThiefTag.IsEmpty()) // no tag¿ - Stulle
		CreateAntiUserNameThiefTag(); // create a tag! - Stulle

	//is he mirroring our current tag?
	if(tocomp.Find(m_sAntiUserNameThiefTag) != -1) // search for tag, not only for entire name - Stulle
		return true;
	else
	//else he is a nice guy ;)
		return false;
}
//<<< AntiUserNameThief
bool CAntiNick::IsReservedAntiUserNameThiefTag(const CString& tag)
{
	if (tag.GetLength() <= 5)
		return false;
	if (tag[0] != _T('[') || tag[tag.GetLength() - 1] != _T(']'))
		return false;

	return tag.Mid(1, 3).CompareNoCase(_T("AI-")) == 0;
}

bool CAntiNick::ContainsReservedAntiUserNameThiefTag(const CString& text)
{
	if (text.IsEmpty())
		return false;

	CString lowerText(text);
	lowerText.MakeLower();
	return lowerText.Find(_T("[ai-")) != -1;
}
