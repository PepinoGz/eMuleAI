//This file is part of eMule AI
//Copyright (C)2026 eMule AI

#include "StdAfx.h"
#include "sourcesaver.h"
#include "PartFile.h"
#include "emule.h"
#include "emuledlg.h"
#include "OtherFunctions.h"
#include "DownloadQueue.h"
#include "updownclient.h"
#include "Preferences.h"
#include "Log.h"
#include "PartFileWriteThread.h"

#define RELOADTIME	3600000 //60 minutes	
#define RESAVETIME	 600000 //10 minutes

CSourceSaver::CSourceSaver(void)
{
	m_dwLastTimeLoaded = ::GetTickCount() - RELOADTIME;
	m_dwLastTimeSaved = ::GetTickCount() + (rand() * 30000 / RAND_MAX) - 15000 - RESAVETIME;
}

CSourceSaver::CSourceData::CSourceData(CUpDownClient* client, const CString& exp)
{
	nSrcExchangeVer = client->GetSourceExchange1Version(); // khaos::kmod+ Modified to Save Source Exchange Version
	sourceIP = client->GetIP();
	if (sourceIP.GetType() == CAddress::IPv4 && nSrcExchangeVer > 2 && IsLowID(client->GetUserIDHybrid())) // nSrcExchangeVer should be 3 or 4 except very old clients
		sourceID = client->GetUserIDHybrid();
	else 
		sourceID = 0; // IPv4 or IPv6 is saved and loaded instead of LowID for this case
	sourcePort = client->GetUserPort();
	serverip = client->GetServerIP();
	serverport = client->GetServerPort();
	partsavailable = client->GetAvailablePartCount();
	expiration = exp;
}

CSourceSaver::~CSourceSaver(void)
{
}

bool CSourceSaver::Process(CPartFile* file) // return false if sources not saved
{
	if ((int)(::GetTickCount() - m_dwLastTimeSaved) > RESAVETIME) {
		_tmakepath(szslsfilepath,NULL,(CString)file->GetTmpPath(), file->GetPartMetFileName(),_T(".txtsrc"));
		m_dwLastTimeSaved = ::GetTickCount() + (rand() * 30000 / RAND_MAX) - 15000;
		sources.RemoveAll();
		LoadSourcesFromFile(file);
		SaveSources(file, false);
		
		if ((int)(::GetTickCount() - m_dwLastTimeLoaded) > RELOADTIME) {
			m_dwLastTimeLoaded = ::GetTickCount() + (rand() * 30000 / RAND_MAX) - 15000;
			AddSourcesToDownload(file);
		}

		while (!sources.IsEmpty()) 
			delete sources.RemoveHead();
		
		return true;
	}
	return false;
}

void CSourceSaver::DeleteFile(CPartFile* file)
{
	TCHAR szslsfilepath[_MAX_PATH];
	_tmakepath(szslsfilepath,NULL,(CString)file->GetTmpPath(), file->GetPartMetFileName(),_T(".txtsrc"));
	if (_tremove(szslsfilepath) && errno != ENOENT)
		AddLogLine(true, _T("Failed to delete '%s'. You'll need to delete this file manually."), szslsfilepath);
}

void CSourceSaver::LoadSourcesFromFile(CPartFile*)
{
	CString strLine;
	CStdioFile f;
	if (!f.Open(szslsfilepath, CFile::modeRead | CFile::typeText))
		return;
	while(f.ReadString(strLine)) {
		if (strLine.GetAt(0) == '#')
			continue;
		int pos = strLine.Find(':');
		if (pos == -1)
			continue;
		CString strIP = strLine.Left(pos);
		strLine = strLine.Mid(pos+1);
		uint32 dwID = 0; // Only LowID's are set to nonzero integers. IPv4 or IPv6 is saved and loaded instead of LowID for this case
		if (strIP.Find('.') == -1 && strIP.Find(':') == -1) { // This is a LowID, not IP address. So we'll set its value.
			dwID = _ttoi(strIP);
			if (dwID == INADDR_NONE)
				continue;
		}
		pos = strLine.Find(',');
		if (pos == -1)
			continue;
		CString strPort = strLine.Left(pos);
		strLine = strLine.Mid(pos+1);
		uint16 wPort = (uint16)_tstoi(strPort);
		if (!wPort)
			continue;
		pos = strLine.Find(',');
		if (pos == -1)
			continue;
		CString strExpiration = strLine.Left(pos);
		if (IsExpired(strExpiration))
			continue;
		strLine = strLine.Mid(pos+1);
		pos = strLine.Find(',');
		if (pos == -1)
			continue;
		uint8 nSrcExchangeVer = (uint8)_tstoi(strLine.Left(pos));
		strLine = strLine.Mid(pos+1);
		pos = strLine.Find(':');
		if (pos == -1)
			continue;
		CString strserverip = strLine.Left(pos);
		strLine = strLine.Mid(pos+1);
		uint32 dwserverip = inet_addr(CT2CA(strserverip));
		if (dwserverip == INADDR_NONE) 
			continue;
		pos = strLine.Find(';');
		if (pos == -1 || strLine.GetLength() < 2)
			continue;
		CString strserverport = strLine.Left(pos);
		uint16 wserverport = (uint16)_tstoi(strserverport);
		if (!wserverport)
			continue;
		CSourceData* newsource = new CSourceData(CAddress(strIP, true), dwID, wPort, dwserverip, wserverport, strExpiration, nSrcExchangeVer);
		sources.AddTail(newsource);
	}
    f.Close();
}

void CSourceSaver::AddSourcesToDownload(CPartFile* file) 
{
	for (POSITION pos = sources.GetHeadPosition(); pos; sources.GetNext(pos)) {
		if (file->GetMaxSources() <= file->GetSourceCount())
			return;

		CSourceData* cur_src = sources.GetAt(pos);
		CUpDownClient* newclient; 

		if (!cur_src->sourceID || cur_src->nSrcExchangeVer >= 3) // Only LowID's are set to nonzero integers. IPv4 or IPv6 is saved and loaded instead of LowID for this case. nSrcExchangeVer should be >= 3 except very old clients.
			newclient = new CUpDownClient(file, cur_src->sourcePort, cur_src->sourceID ? cur_src->sourceID : cur_src->sourceIP.ToUInt32(true), cur_src->serverip, cur_src->serverport, false, cur_src->sourceIP);
		else
			newclient = new CUpDownClient(file, cur_src->sourcePort, cur_src->sourceID, cur_src->serverip, cur_src->serverport, true);

		newclient->SetSourceFrom(SF_SLS);
		theApp.downloadqueue->CheckAndAddSource(file, newclient, SF_SLS);
        
	}
}

void CSourceSaver::SaveSources(CPartFile* file, bool bForce)
{
	CSingleLock sSaveSourcesLock(&file->m_SaveSourcesLock, FALSE);
	if (sSaveSourcesLock.IsLocked()) // Source are being saved inside the thread. Don't make GUI thread to wait it, return here and try next time.
		return;
	sSaveSourcesLock.Lock(); // Lock is free, lets lock it and do the job.

	CSourceData* sourcedata;
	int m_iSaveLoadSourcesMaxSources = thePrefs.GetSaveLoadSourcesMaxSources();
	int m_iSaveLoadSourcesExpirationDays = thePrefs.GetSaveLoadSourcesExpirationDays();

	ASSERT2(file->srcstosave.IsEmpty());

	POSITION pos2, pos;
	CUpDownClient* cur_src;
	// Choose best sources for the file
	for (pos = file->srclist.GetHeadPosition(); pos != 0;) {
		cur_src = file->srclist.GetNext(pos);

		if (!cur_src->IsValidSource())
			continue;

		if (file->srcstosave.IsEmpty()) {
			sourcedata = new CSourceData(cur_src, CalcExpiration(m_iSaveLoadSourcesExpirationDays));
			file->srcstosave.AddHead(sourcedata);
			continue;
		}

		// Skip also Required Obfuscation, because we don't save the userhash (and we don't know if all settings are still valid on next restart)
		if (cur_src->RequiresCryptLayer() || thePrefs.IsCryptLayerRequired())
			continue;

		if ((UINT)file->srcstosave.GetCount() < m_iSaveLoadSourcesMaxSources || (cur_src->GetAvailablePartCount() > file->srcstosave.GetTail()->partsavailable) || (cur_src->GetSourceExchange1Version() > file->srcstosave.GetTail()->nSrcExchangeVer)) {
			if ((UINT)file->srcstosave.GetCount() == m_iSaveLoadSourcesMaxSources)
				delete file->srcstosave.RemoveTail();
			ASSERT((UINT)file->srcstosave.GetCount() < m_iSaveLoadSourcesMaxSources);
			bool bInserted = false;
			for (pos2 = file->srcstosave.GetTailPosition(); pos2 != 0; file->srcstosave.GetPrev(pos2)) {
				CSourceData* cur_srctosave = file->srcstosave.GetAt(pos2);
				if (file->GetAvailableSrcCount() > (m_iSaveLoadSourcesMaxSources * 2) && cur_srctosave->nSrcExchangeVer > cur_src->GetSourceExchange1Version())
					bInserted = true;
				else if (file->GetAvailableSrcCount() > (m_iSaveLoadSourcesMaxSources * 2) && cur_srctosave->nSrcExchangeVer == cur_src->GetSourceExchange1Version() && cur_srctosave->partsavailable > cur_src->GetAvailablePartCount())
					bInserted = true;
				else if (file->GetAvailableSrcCount() <= (m_iSaveLoadSourcesMaxSources * 2) && cur_srctosave->partsavailable > cur_src->GetAvailablePartCount())
					bInserted = true;

				if (bInserted) {
					sourcedata = new CSourceData(cur_src, CalcExpiration(m_iSaveLoadSourcesExpirationDays));
					file->srcstosave.InsertAfter(pos2, sourcedata);
					break;
				}
			}

			if (!bInserted) {
				sourcedata = new CSourceData(cur_src, CalcExpiration(m_iSaveLoadSourcesExpirationDays));
				file->srcstosave.AddHead(sourcedata);
			}
		}
	}

	// Add previously saved sources if found sources does not reach the limit
	for (pos = sources.GetHeadPosition(); pos; sources.GetNext(pos)) {
		CSourceData* cur_sourcedata = sources.GetAt(pos);

		if ((UINT)file->srcstosave.GetCount() == m_iSaveLoadSourcesMaxSources)
			break;
		ASSERT((UINT)file->srcstosave.GetCount() <= m_iSaveLoadSourcesMaxSources);

		bool bFound = false;
		for (pos2 = file->srcstosave.GetHeadPosition(); pos2; file->srcstosave.GetNext(pos2)) {
			if (file->srcstosave.GetAt(pos2)->Compare(cur_sourcedata)) {
				bFound = true;
				break;
			}
		}

		if (!bFound)
			file->srcstosave.AddTail(new CSourceData(cur_sourcedata));

	}
	sSaveSourcesLock.Unlock(); // We finished updating srcstosave. Now we can unlock the lock before proceeding interaction with the thread.

	// We'll submit a command to the thread only if there's no save sources met command in queue
	if (file && !file->m_bSaveSourcesInQueue && (bForce || thePrefs.GetCommitFiles() >= 2 || (thePrefs.GetCommitFiles() >= 1 && theApp.IsClosing()))) {
		CPartFileWriteThread* pThread = theApp.m_pPartFileWriteThread;
		if (pThread && pThread->IsRunning()) {
			CSingleLock sFlushListLock(&pThread->m_lockFlushList, TRUE);
			file->m_bSaveSourcesInQueue = true;
			pThread->m_FlushList.AddTail(ToWrite{ file, NULL, NULL, new SaveSourcesData{ thePrefs.GetSaveLoadSourcesMaxSources(), thePrefs.GetSaveLoadSourcesExpirationDays(), thePrefs.IsCryptLayerRequired() } });

			if (!pThread->m_FlushList.IsEmpty()) //let it sleep if nothing to do
				pThread->WakeUpCall();
		}
	}
}

CString CSourceSaver::CalcExpiration(int Days)
{
	CTime expiration = CTime::GetCurrentTime();
	CTimeSpan timediff(Days, 0, 0, 0);
	expiration += timediff;
	CString strExpiration;
	strExpiration.Format(_T("%02i%02i%02i%02i%02i"), (expiration.GetYear() % 100), expiration.GetMonth(), expiration.GetDay(), expiration.GetHour(), expiration.GetMinute());
	return strExpiration;
}

bool CSourceSaver::IsExpired(CString expirationdate)
{
	int year = _tstoi(expirationdate.Mid(0, 2)) + 2000;
	int month = _tstoi(expirationdate.Mid(2, 2));
	int day = _tstoi(expirationdate.Mid(4, 2));
	int hour = _tstoi(expirationdate.Mid(6, 2));
	int minute = _tstoi(expirationdate.Mid(8, 2));
	CTime expiration(year, month, day, hour, minute, 0);
	return (expiration < CTime::GetCurrentTime());
}
