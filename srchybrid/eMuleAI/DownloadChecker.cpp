//This file is part of eMule AI
//Copyright (C)2026 eMule AI

#include "StdAfx.h"
#include "emule.h"
#include "emuleDlg.h"
#include "DownloadChecker.h"
#include "Log.h"
#include "Preferences.h"
#include "KnownFile.h"
#include "PartFile.h"
#include "DownloadQueue.h"
#include "TransferDlg.h"
#include "SearchDlg.h"
#include "SearchList.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#endif

CDownloadChecker::CDownloadChecker(void)
{
	m_iDataSize = -1;
}

CDownloadChecker::~CDownloadChecker(void)
{
}

void CDownloadChecker::ReloadMap()
{
	if (theApp.IsClosing())
		return;

	CString m_strProcessedFileName;
	FileInfoType m_FileInfo;

	if (m_iDataSize == -1) { // Initializing map here instead of constructor since download list is not available there and init size effect performance
		m_iDataSize = theApp.emuledlg->transferwnd->GetDownloadList()->m_ListItems.size() + theApp.knownfiles->m_Files_map.GetCount() + 10000;
		m_DownloadCheckerMap.InitHashTable(m_iDataSize);
	} else
		m_DownloadCheckerMap.RemoveAll(); // Clean up first

	// Downloading files
	for (auto it = theApp.emuledlg->transferwnd->GetDownloadList()->m_ListItems.begin(); it != theApp.emuledlg->transferwnd->GetDownloadList()->m_ListItems.end(); ++it) {
		const CtrlItem_Struct* cur_item = it->second;
		if (cur_item->type != FILE_TYPE)
			continue;

		CPartFile* pFile = static_cast<CPartFile*>(cur_item->value);
		if (pFile)
			AddToMap(pFile->GetFileHash(), pFile->GetFileName(), pFile->GetFileSize());
	}

	// Known files
	for (const CKnownFilesMap::CPair* pair = theApp.knownfiles->m_Files_map.PGetFirstAssoc(); pair != NULL; pair = theApp.knownfiles->m_Files_map.PGetNextAssoc(pair)) {
		CKnownFile* pFile = pair->value;
		if (pFile)
			AddToMap(pFile->GetFileHash(), pFile->GetFileName(), pFile->GetFileSize());
	}

	// Duplicate files
	CSingleLock slDuplicatesLock(&theApp.knownfiles->m_csDuplicatesLock, TRUE);
	for (auto&& pFile : theApp.knownfiles->m_duplicateFileList) {
		if (pFile)
			AddToMap(pFile->GetFileHash(), pFile->GetFileName(), pFile->GetFileSize());
	}
}

void CDownloadChecker::AddToMap(const uchar* hash, const CString filename, const EMFileSize filesize)
{
	if (theApp.IsClosing() || filename.IsEmpty() || filesize == 0ull || !theApp.DownloadChecker) // Check if inputs are valid and DownloadChecker is initialized, this will bypass calls from knownfiles init)
		return;

	CString m_strProcessedFileName;
	FileInfoType m_FileInfo;
	m_strProcessedFileName = filename;

	if (thePrefs.GetDownloadCheckerIgnoreExtension())
		m_strProcessedFileName = RemoveFileExtension(m_strProcessedFileName);

	if (thePrefs.GetDownloadCheckerIgnoreTags())
		m_strProcessedFileName = RemoveTags(m_strProcessedFileName, thePrefs.GetDownloadCheckerDontIgnoreNumericTags());

	if (thePrefs.GetDownloadCheckerIgnoreNonAlphaNumeric())
		m_strProcessedFileName = RemoveNonAlphaNumeric(m_strProcessedFileName);

	if (thePrefs.GetDownloadCheckerCaseInsensitive())
		m_strProcessedFileName.MakeLower();

	if (m_DownloadCheckerMap.Lookup(m_strProcessedFileName, m_FileInfo) && m_FileInfo.uSize >= filesize)
		return; // Same key found but file size is bigger, so we can return here.

	m_FileInfo.strName = filename;
	m_FileInfo.uSize = filesize;
	md4cpy(m_FileInfo.ucHash, hash);
	m_DownloadCheckerMap[m_strProcessedFileName] = m_FileInfo;
}

void CDownloadChecker::RemoveFromMap(const uchar* hash, const CString filename, const EMFileSize filesize)
{
	if (theApp.IsClosing() || filename.IsEmpty() || filesize == 0ull || !theApp.DownloadChecker) // Check if inputs are valid and DownloadChecker is initialized, this will bypass calls from knownfiles init)
		return;

	uint m_iFoundCount = 0;

	if(theApp.downloadqueue->GetFileByID(hash))
		m_iFoundCount++;

	if (theApp.knownfiles->FindKnownFileByID(hash))
		m_iFoundCount++;

	m_iFoundCount += theApp.knownfiles->DuplicatesCount(hash);

	if (m_iFoundCount) // If extra file(s) found, we don't remove it from map
		return;

	CString m_strProcessedFileName;
	m_strProcessedFileName = filename;

	if (thePrefs.GetDownloadCheckerIgnoreExtension())
		m_strProcessedFileName = RemoveFileExtension(m_strProcessedFileName);

	if (thePrefs.GetDownloadCheckerIgnoreTags())
		m_strProcessedFileName = RemoveTags(m_strProcessedFileName, thePrefs.GetDownloadCheckerDontIgnoreNumericTags());

	if (thePrefs.GetDownloadCheckerIgnoreNonAlphaNumeric())
		m_strProcessedFileName = RemoveNonAlphaNumeric(m_strProcessedFileName);

	if (thePrefs.GetDownloadCheckerCaseInsensitive())
		m_strProcessedFileName.MakeLower();

	m_DownloadCheckerMap.RemoveKey(m_strProcessedFileName); // Remove entry from map.
}

// Returns EDownloadCheckerResult
const UINT CDownloadChecker::CheckFile(const uchar* hash, const CString filename, const EMFileSize filesize, const bool bCalledByAddToDownload)
{
	CString cLogMsg;
	FileInfoType m_FileInfo;
	CString m_strProcessedFileName;

	if (bCalledByAddToDownload) {
		if (theApp.downloadqueue->IsFileExisting(hash, bCalledByAddToDownload))
			return EDownloadCheckerResult::Downloading;

		// Check if a file with the same hash and size exists
		if (thePrefs.GetDownloadCheckerRejectSameHash()) {
			CKnownFile* curFile = theApp.knownfiles->FindKnownFileByID(hash);
			if (curFile && (curFile->GetFileSize() == filesize) && !curFile->IsPartFile()) {
				cLogMsg.Format(GetResString(_T("DOWNLOAD_CHECK_REJECTED_MESSAGE")), GetResString(_T("DOWNLOAD_CHECK_REJECT_REASON_HASH")), filename, curFile->GetFileName());
				AddLogLine(true, (LPCTSTR)EscPercent(cLogMsg));
				return EDownloadCheckerResult::Known; // A file with same hash and size is found and user wants to reject it automatically without any further checks 
			}
		}

		// Check if the file has been canceled before
		if (thePrefs.GetDownloadCheckerRejectCanceled() && theApp.knownfiles->IsCancelledFileByID(hash)) {
			cLogMsg.Format(GetResString(_T("DOWNLOAD_CHECK_REJECTED_MESSAGE2")), GetResString(_T("DOWNLOAD_CHECK_REJECT_REASON_CANCELED")), filename);
			AddLogLine(true, (LPCTSTR)EscPercent(cLogMsg));
			return EDownloadCheckerResult::Cancelled; // A canceled file with same hash found and user wants to reject it automatically without any further checks
		}

		// Check if the file name is blacklisted
		if (thePrefs.GetDownloadCheckerRejectBlacklisted()) {
			// Check if the file name is manually blacklisted
			if (thePrefs.GetBlacklistManual() && theApp.searchlist->IsFilenameManualBlacklisted(CSKey(hash))) {
				cLogMsg.Format(GetResString(_T("DOWNLOAD_CHECK_REJECTED_MESSAGE2")), GetResString(_T("DOWNLOAD_CHECK_REJECT_REASON_MANUAL_BLACKLISTED")), filename);
				AddLogLine(true, (LPCTSTR)EscPercent(cLogMsg));
				return EDownloadCheckerResult::ManualBlacklisted; // File name is manually blacklisted and user wants to reject it automatically without any further checks
			}

			// Check if the file name is automatically blacklisted
			if (thePrefs.GetBlacklistAutomatic() && CSearchList::IsFilenameAutoBlacklisted(filename)) {
				cLogMsg.Format(GetResString(_T("DOWNLOAD_CHECK_REJECTED_MESSAGE2")), GetResString(_T("DOWNLOAD_CHECK_REJECT_REASON_AUTOMATIC_BLACKLISTED")), filename);
				AddLogLine(true, (LPCTSTR)EscPercent(cLogMsg));
				return EDownloadCheckerResult::AutomaticBlacklisted; // File name is automatically blacklisted and user wants to reject it automatically without any further checks
			}
		}
	}

	// Check if a file with the same name exists in the map
	m_strProcessedFileName = filename;

	if (m_strProcessedFileName.GetLength() < thePrefs.GetDownloadCheckerMinimumComparisonLength())
		return EDownloadCheckerResult::OK; // Stop comparison if file name size doesn't meet the condition

	if (thePrefs.GetDownloadCheckerIgnoreExtension())
		m_strProcessedFileName = RemoveFileExtension(m_strProcessedFileName);

	if (thePrefs.GetDownloadCheckerIgnoreTags())
		m_strProcessedFileName = RemoveTags(m_strProcessedFileName, thePrefs.GetDownloadCheckerDontIgnoreNumericTags());

	if (thePrefs.GetDownloadCheckerIgnoreNonAlphaNumeric())
		m_strProcessedFileName = RemoveNonAlphaNumeric(m_strProcessedFileName);

	if (thePrefs.GetDownloadCheckerCaseInsensitive())
		m_strProcessedFileName.MakeLower();

	if (!m_DownloadCheckerMap.Lookup(m_strProcessedFileName, m_FileInfo))
		return EDownloadCheckerResult::OK; // Key not found, stop here.

	if (thePrefs.GetDownloadChecker() == 2) { // if a file with the same name found, user wants to reject it automatically without any further checks
		if (bCalledByAddToDownload) {
			cLogMsg.Empty();
			cLogMsg.Format(GetResString(_T("DOWNLOAD_CHECK_REJECTED_MESSAGE")), GetResString(_T("DOWNLOAD_CHECK_REJECT_REASON_KNOWN")), filename, m_FileInfo.strName);
			AddLogLine(true, (LPCTSTR)EscPercent(cLogMsg));
		}
		return EDownloadCheckerResult::SimilarName;
	}

	if ((thePrefs.GetDownloadChecker() == 3) && (double(filesize) < (double(m_FileInfo.uSize)) * 0.01 * (100 + thePrefs.GetDownloadCheckerAcceptPercentage()))) { // Reject if new file size is smaller then old file + defined percentage
		if (bCalledByAddToDownload) {
			cLogMsg.Empty();
			cLogMsg.Format(GetResString(_T("DOWNLOAD_CHECK_REJECTED_MESSAGE3")), GetResString(_T("DOWNLOAD_CHECK_REJECT_REASON_KNOWN")), filesize, filename, m_FileInfo.uSize, m_FileInfo.strName);
			AddLogLine(true, (LPCTSTR)EscPercent(cLogMsg));
		}
		return EDownloadCheckerResult::SimilarName;
	}

	if (thePrefs.GetDownloadChecker() == 2 || thePrefs.GetDownloadChecker() == 3) // Since conditions are not met till now, we can return here.
		return EDownloadCheckerResult::OK;

	// Construct message
	if (bCalledByAddToDownload) {
		CString msg;
		CString sData;
		msg.Format(GetResString(_T("DOWNHISTORY_CHECK2")));
		msg += GetResString(_T("DOWNHISTORY_CHECK3"));
		msg += _T("\n\n") + GetResString(_T("DOWNHISTORY_CHECK6")) + _T("\n------------------------\n");
		msg += filename + _T("\n");
		sData.Format(GetResString(_T("DL_SIZE")) + _T(": %I64u\n"), filesize);
		msg += sData;
		sData.Format(GetResString(_T("ID2")) + _T(" %s\n"), EncodeBase16(hash, 16));
		msg += sData;
		msg += _T("\n") + GetResString(_T("DOWNHISTORY_CHECK7")) + _T("\n------------------------\n");
		msg += m_FileInfo.strName + _T("\n");
		sData.Format(GetResString(_T("DL_SIZE")) + _T(": %I64u\n"), m_FileInfo.uSize);
		msg += sData;
		sData.Format(GetResString(_T("ID2")) + _T(" %s\n"), EncodeBase16(m_FileInfo.ucHash, 16));
		msg += (sData + _T("\n\n"));

		if (CDarkMode::MessageBox(msg, MB_YESNO | MB_ICONQUESTION) == IDYES)
			return EDownloadCheckerResult::OK;
		else
			return EDownloadCheckerResult::SimilarName;
	}

	return EDownloadCheckerResult::OK; // This shouldn't be reached normally, just in case.
}
