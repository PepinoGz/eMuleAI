//This file is part of eMule AI
//Copyright (C)2026 eMule AI

#pragma once
#include "eMuleAI/Address.h"

class CPartFile;
class CUpDownClient;

class CSourceSaver
{
public:
	CSourceSaver(void);
	~CSourceSaver(void);
	bool Process(CPartFile* file);
	void DeleteFile(CPartFile* file);
	CString CalcExpiration(int nDays);

	class CSourceData
	{
	public:

		CSourceData(CAddress cSourceIP, uint32 dwSourceID, uint16 wSourcePort, uint32 dwServerIP, uint16 wServerPort, const CString& strExpiration, uint8 uSXVer)
		{
			sourceIP = cSourceIP;
			sourceID = dwSourceID;
			sourcePort = wSourcePort;
			serverip = dwServerIP;
			serverport = wServerPort;
			expiration = strExpiration;
			nSrcExchangeVer = uSXVer;
		}

		CSourceData(CUpDownClient* client, const CString& strExpiration);

		CSourceData(CSourceData* pOld)
		{
			sourceIP = pOld->sourceIP;
			sourceID = pOld->sourceID; 
			sourcePort = pOld->sourcePort; 
			serverip = pOld->serverip;
			serverport = pOld->serverport;
			partsavailable = pOld->partsavailable;
			expiration = pOld->expiration;
			nSrcExchangeVer = pOld->nSrcExchangeVer;
		}

		bool Compare(CSourceData* tocompare) { return ((sourceIP == tocompare->sourceIP) && (sourcePort == tocompare->sourcePort)); }

		CAddress sourceIP;
		uint32	sourceID;
		uint16	sourcePort;
		uint32	serverip;
		uint16	serverport;
		uint32	partsavailable;
		CString	expiration;
		uint8	nSrcExchangeVer;
	};

	typedef CTypedPtrList<CPtrList, CSourceData*> Sources;
	Sources sources;
	TCHAR szslsfilepath[_MAX_PATH];

	void LoadSourcesFromFile(CPartFile* file);
	void SaveSources(CPartFile* file, bool bForce);
	void AddSourcesToDownload(CPartFile* file);

protected:
	uint32	m_dwLastTimeLoaded;
	uint32  m_dwLastTimeSaved;
	
	bool IsExpired(CString expirationdate);
};
