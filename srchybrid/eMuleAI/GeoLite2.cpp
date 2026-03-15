//This file is part of eMule AI
//Copyright (C)2026 eMule AI

#include "StdAfx.h"
#include <share.h>
#include "GeoLite2.h"
#include "emule.h"
#include "Preferences.h"
#include "otherfunctions.h"
#include "log.h"
#include "serverlist.h"
#include "TransferDlg.h"
#include "clientlist.h"
#include "emuledlg.h"
#include "serverwnd.h"
#include "serverlistctrl.h"
#include "kademliawnd.h"
#include "eMuleAI/GeoLite2PP.hpp"
#include <system_error>
#include <eMuleAI/json.hpp> 

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

CGeoLite2::CGeoLite2()
{
	m_bRunning = false;
	GeoLite2Loaded = false;
	db = NULL;

	LoadFlags();
	if(thePrefs.GetGeoLite2Mode() != GL2_DISABLE || thePrefs.GetGeoLite2ShowFlag())
		LoadGeoLite2();

	m_bRunning = true;
}

CGeoLite2::~CGeoLite2()
{
	// Deterministically release GeoLite2 DB
	if (db != NULL) {
		try {
			delete db;
		} catch (...) {
			// Be safe in shutdown
		}
		db = NULL;
	}
	UnloadFlags();
}

// Finds and returns the largest file with mmdb extention in the config directory.
const CString CGeoLite2::GetFilePath()
{
	CString m_strPath;
	ULONGLONG m_ulSize = 0;
	CFileFind current;
	BOOL bFound = current.FindFile(thePrefs.GetMuleDirectory(EMULE_CONFIGDIR) + _T("*.mmdb"));
	if (bFound) {
		do {
			bFound = current.FindNextFile();
			if (!current.IsDirectory() && current.GetLength() > 0 && (!m_ulSize || current.GetLength() > m_ulSize)) {
				m_strPath = current.GetFilePath();
				m_ulSize = current.GetLength();
			}
		} while (bFound);
	}

	return m_strPath;
}

void CGeoLite2::LoadGeoLite2()
{
	GeoLite2Loaded = false;
	m_strGeoLite2CityFile = GetFilePath();

	if (!m_strGeoLite2CityFile.IsEmpty()) {
		try {
			// Release any previous DB instance if reloading
			if (db != NULL) {
				try { delete db; } catch (...) { /* swallow */ }
				db = NULL;
			}
			db = new GeoLite2PP::DB((std::string)CT2CA(m_strGeoLite2CityFile.GetString()));
		} catch (const std::system_error& e) {
			CString errmsg(e.what());
			Log(_T("%i %s %s"), e.code().value(), errmsg, m_strGeoLite2CityFile);
		} catch (...) {
			Log(_T("Failed: %s"), m_strGeoLite2CityFile);
		}

		if (db != NULL) {
			GeoLite2Loaded = true;
			// Be defensive: metadata JSON may be missing/corrupted; do not throw here.
			CString m_strMsg1, m_strMsg2;
			try { // Parse metadata to log DB build time, but keep running on failure
				const std::string meta = db->get_metadata();
				nlohmann::json json_obj = nlohmann::json::parse(meta);
				auto it = json_obj.find("build_epoch");
				if (it != json_obj.end() && it->is_number_integer()) {
					time_t build_epoch = static_cast<time_t>(it->get<long long>());
					m_strMsg2.Format(GetResString(_T("GEOLITE2_LOADED2")), CString(epoch2str(build_epoch).c_str()));
				}
			} catch (const std::exception& ex) {
				// Log but do not propagate; keep GeoLite2 enabled.
				CString exMsg = UTF8To16(ex.what());
				AddDebugLogLine(false, _T("[GeoLite2] Failed to parse metadata JSON: %s"), (LPCTSTR)EscPercent(exMsg));
			} catch (...) {
				// Unknown error; keep running without build time info.
				AddDebugLogLine(false, _T("[GeoLite2] Unknown error while parsing metadata JSON"));
			}
			m_strMsg1.Format(GetResString(_T("GEOLITE2_LOADED")), m_strGeoLite2CityFile);
			Log(m_strMsg2.IsEmpty() ? m_strMsg1 : m_strMsg1 + _T(" ") + m_strMsg2);
		}
	}

	if(m_bRunning)
		Reset();
}

void CGeoLite2::Reset()
{
	theApp.serverlist->ResetGeoLite2();
	theApp.clientlist->ResetGeoLite2();
	theApp.emuledlg->kademliawnd->ResetGeoLite2();
}

void CGeoLite2::Redraw()
{
	theApp.emuledlg->serverwnd->RedrawWindow();
	theApp.emuledlg->kademliawnd->RedrawWindow();
	theApp.emuledlg->transferwnd->GetDownloadList()->RedrawWindow();
	theApp.emuledlg->transferwnd->GetClientList()->RedrawWindow();
	theApp.emuledlg->transferwnd->GetDownloadClientsList()->RedrawWindow();
	theApp.emuledlg->transferwnd->GetUploadList()->RedrawWindow();
	theApp.emuledlg->transferwnd->GetQueueList()->RedrawWindow();
}

void CGeoLite2::LoadFlags() {
	FlagImageList.DeleteImageList();
	FlagImageList.Create(FLAG_WIDTH, FLAG_HEIGHT, theApp.m_iDfltImageListColorFlags | ILC_MASK, 0, 1);
	FlagImageList.SetBkColor(CLR_NONE);

	HICON iconHandle = NULL;
	int iconIndex = -1;
	for (int i = 0; i < CountryCodeFlagArraySize; i++) {
		iconHandle = (HICON)::LoadImage(AfxGetResourceHandle(), MAKEINTRESOURCE(CountryCodeFlagArray[i].uResourceID), IMAGE_ICON, FLAG_WIDTH, FLAG_HEIGHT, LR_DEFAULTCOLOR);
		if(iconHandle) {
			iconIndex = FlagImageList.Add(iconHandle);
			if(iconIndex != -1)
				CountryCodeFlagMap.SetAt(CountryCodeFlagArray[i].strCountryCode, (uint16)iconIndex);
			::DestroyIcon(iconHandle);
		} else
			AddDebugLogLine(LOG_WARNING, _T("[GeoLite2] Invalid flag resource ID"), CountryCodeFlagArray[i].uResourceID);
	}

	AddLogLine(false, GetResString(_T("GEOLITE2_FLAGLOAD")));
}

void CGeoLite2::UnloadFlags() {
	//Clean out the map table
	CountryCodeFlagMap.RemoveAll();

	try {
		//destory all image
		if (FlagImageList && FlagImageList.m_hImageList) {
			FlagImageList.DeleteImageList();
			Log(GetResString(_T("GEOLITE2_FLAGUNLD")));
		}
	} catch (...) {
		if (thePrefs.GetVerbose())
			AddDebugLogLine(false, _T("[GeoLite2] CountryFlagImageList failed."));
		ASSERT(0);
	}
}


const struct GeolocationData_Struct CGeoLite2::QueryGeolocationData(const CAddress& IP)
{
	GeolocationData_Struct ppFound;
	// Set default values first.
	ppFound.Country = GetResString(_T("GEOLITE2_NA"));
	ppFound.CountryCode = GetResString(_T("GEOLITE2_NA"));
	ppFound.City = GetResString(_T("GEOLITE2_NA"));
	ppFound.FlagIndex = NO_FLAG;

	if (GeoLite2Loaded == false || !IP.IsPublicIP())
		return ppFound;

	try {
		GeoLite2PP::MStr m = db->get_all_fields(IP.ToString(), "en");
		for (const auto iter : m) {
			if (iter.first == "country")
				ppFound.Country = UTF8To16(iter.second.c_str());
			else if (iter.first == "country_iso_code") {
				ppFound.CountryCode = UTF8To16(iter.second.c_str());
				uint16 m_uTemp = -1;
				if (CountryCodeFlagMap.Lookup(ppFound.CountryCode, m_uTemp) && m_uTemp >= 0)
					ppFound.FlagIndex = m_uTemp;
			} else if (iter.first == "city")
				ppFound.City = UTF8To16(iter.second.c_str());
		}
	} catch (const std::length_error&) {
		// entry not found; keep defaults
	} catch (const std::exception& ex) {
		if (thePrefs.GetVerbose()) {
			CString exMsg = UTF8To16(ex.what());
			AddDebugLogLine(false, _T("[GeoLite2] std::exception while querying GeoLite2: %s"), (LPCTSTR)EscPercent(exMsg));
		}
		ASSERT(0);
	} catch (...) {
		if (thePrefs.GetVerbose())
			AddDebugLogLine(false, _T("[GeoLite2] Unknown exception while querying GeoLite2"));
		ASSERT(0);
	}

	return ppFound;
}

const CString CGeoLite2::GetGeolocationData(const GeolocationData_Struct m_structGeolocationData, bool bForceCountryCity) const
{
	if(GeoLite2Loaded) {
		if(bForceCountryCity)
			return m_structGeolocationData.Country;
		switch(thePrefs.GetGeoLite2Mode()) {
			case GL2_COUNTRYCODE:
				return m_structGeolocationData.CountryCode;
			case GL2_COUNTRY:
				return m_structGeolocationData.Country;
			case GL2_COUNTRYCITY:
			{
				if (m_structGeolocationData.Country == GetResString(_T("GEOLITE2_NA"))) // Show "N/A" instead of "N/A, N/A"
					return m_structGeolocationData.Country;
				CString m_strTemp;
				m_strTemp.Format(_T("%s, %s"), m_structGeolocationData.Country, m_structGeolocationData.City);
				return m_strTemp;
			}
		}
	} else if(bForceCountryCity)
		return GetResString(_T("DISABLED"));	

	return EMPTY;
}

const bool CGeoLite2::ShowCountryFlag() {
	return (thePrefs.GetGeoLite2ShowFlag() && GeoLite2Loaded); // User enabled flags and GeoLite 2 DB file have been loaded
}

IMAGELISTDRAWPARAMS CGeoLite2::GetFlagImageDrawParams(CDC* dc,int iIndex, POINT point) const
{
	IMAGELISTDRAWPARAMS imldp;
	imldp.cbSize   = sizeof(IMAGELISTDRAWPARAMS);
	imldp.himl     = FlagImageList.m_hImageList;
	imldp.i        = iIndex;
	imldp.hdcDst   = dc->m_hDC;
	imldp.x        = point.x;
	imldp.y        = point.y;
	imldp.cx       = FLAG_WIDTH;
	imldp.cy       = FLAG_HEIGHT;
	imldp.xBitmap  = 0;
	imldp.yBitmap  = 0;
	imldp.rgbBk    = CLR_DEFAULT;
	imldp.rgbFg    = CLR_DEFAULT;
	imldp.fStyle   = ILD_SCALE;
	imldp.dwRop    = SRCCOPY;
#if (_WIN32_WINNT >= 0x501)
	imldp.fState   = ILS_NORMAL;
	imldp.Frame    = 0;
	imldp.crEffect = CLR_DEFAULT;
#endif

	return imldp;
}