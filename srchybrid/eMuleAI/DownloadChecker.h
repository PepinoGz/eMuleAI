//This file is part of eMule AI
//Copyright (C)2026 eMule AI

#pragma once
#include "Preferences.h"
#include "KnownFileList.h"
#include "SharedFileList.h"
#include "SearchFile.h"

class CDownloadChecker
{
	public:
		CDownloadChecker(void);
		~CDownloadChecker(void);

		void ReloadMap();
		void AddToMap(const uchar* hash, const CString filename, const EMFileSize filesize);
		void RemoveFromMap(const uchar* hash, const CString filename, const EMFileSize filesize);
		const UINT CheckFile(const uchar* hash, const CString filename, const EMFileSize filesize, const bool bCalledByAddToDownload);

		enum EDownloadCheckerResult {
			OK = 0,
			Known,
			Downloading,
			Cancelled,
			SimilarName,
			ManualBlacklisted,
			AutomaticBlacklisted
		};

		struct FileInfoType
		{
			CString strName = EMPTY;
			EMFileSize uSize = 0ull;
			uchar ucHash[MDX_DIGEST_SIZE] = { 0 };
		};

		CMap<CString, LPCTSTR, FileInfoType, FileInfoType> m_DownloadCheckerMap;
	protected:
		int 	m_iDataSize;
};
