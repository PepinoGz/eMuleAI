//This file is part of eMule AI
//Copyright (C)2026 eMule AI

#pragma once
#include <atlcoll.h>
#include "resource.h"
#include "Preferences.h"
#include "eMuleAI/Address.h"
#include "eMuleAI/GeoLite2PP.hpp"

#define NO_FLAG 0
#define FLAG_WIDTH 20
#define FLAG_HEIGHT 14 

struct GeolocationData_Struct {
	CString			Country;
	CString			CountryCode;
	CString			City;
	WORD			FlagIndex;
};

typedef CTypedPtrArray<CPtrArray, GeolocationData_Struct*> CGeoLite2Array;

enum GeoLite2Mode {
	GL2_DISABLE = 0,
	GL2_COUNTRYCODE,
	GL2_COUNTRY,
	GL2_COUNTRYCITY
};

const uint16 CountryCodeFlagArraySize = 254;

struct CountryCodeFlag_Struct
{
	CString	strCountryCode;
	uint16	uResourceID;
};

class CGeoLite2
{
	public:
		CGeoLite2(void);
		~CGeoLite2(void);
		
		void	LoadGeoLite2();
		void	Reset(); // Reset GeoLite2 data holded by items.
		void	Redraw(); // Redraw windows holding GeoLite2 data.
		void	LoadFlags();
		void	UnloadFlags();
		const bool	IsGeoLite2Active() { return GeoLite2Loaded && thePrefs.GetGeoLite2Mode() != GL2_DISABLE; }
		const bool	ShowCountryFlag();
		const GeolocationData_Struct QueryGeolocationData(const CAddress& IP);
		const CString GetGeolocationData(const GeolocationData_Struct m_structServerCountry, bool bForceCountryCity =false) const;
		CImageList* GetFlagImageList() { return &FlagImageList; }
		IMAGELISTDRAWPARAMS GetFlagImageDrawParams(CDC* dc,int iIndex,POINT point) const;
		CString m_strGeoLite2CityFile;
		GeoLite2PP::DB* db;

	private:
		bool	m_bRunning;	//check is program current running, if it's under init or shutdown, set to false
		CImageList	FlagImageList;
		bool	GeoLite2Loaded;
		CRBMap<CString, uint16>	CountryCodeFlagMap;
		const CString GetFilePath();

		const CountryCodeFlag_Struct CountryCodeFlagArray[CountryCodeFlagArraySize] = {
			{ L"N/A",IDI_FLAG_NOFLAG },
			{ L"A1", IDI_FLAG_A1 },
			{ L"A2", IDI_FLAG_A2 },
			{ L"AD", IDI_FLAG_AD },
			{ L"AE", IDI_FLAG_AE },
			{ L"AF", IDI_FLAG_AF },
			{ L"AG", IDI_FLAG_AG },
			{ L"AI", IDI_FLAG_AI },
			{ L"AL", IDI_FLAG_AL },
			{ L"AM", IDI_FLAG_AM },
			{ L"AO", IDI_FLAG_AO },
			{ L"AQ", IDI_FLAG_AQ },
			{ L"AR", IDI_FLAG_AR },
			{ L"AS", IDI_FLAG_AS },
			{ L"AT", IDI_FLAG_AT },
			{ L"AU", IDI_FLAG_AU },
			{ L"AW", IDI_FLAG_AW },
			{ L"AX", IDI_FLAG_AX },
			{ L"AZ", IDI_FLAG_AZ },
			{ L"BA", IDI_FLAG_BA },
			{ L"BB", IDI_FLAG_BB },
			{ L"BD", IDI_FLAG_BD },
			{ L"BE", IDI_FLAG_BE },
			{ L"BF", IDI_FLAG_BF },
			{ L"BG", IDI_FLAG_BG },
			{ L"BH", IDI_FLAG_BH },
			{ L"BI", IDI_FLAG_BI },
			{ L"BJ", IDI_FLAG_BJ },
			{ L"BL", IDI_FLAG_BL },
			{ L"BM", IDI_FLAG_BM },
			{ L"BN", IDI_FLAG_BN },
			{ L"BO", IDI_FLAG_BO },
			{ L"BQ", IDI_FLAG_BQ },
			{ L"BR", IDI_FLAG_BR },
			{ L"BS", IDI_FLAG_BS },
			{ L"BT", IDI_FLAG_BT },
			{ L"BV", IDI_FLAG_BV },
			{ L"BW", IDI_FLAG_BW },
			{ L"BY", IDI_FLAG_BY },
			{ L"BZ", IDI_FLAG_BZ },
			{ L"CA", IDI_FLAG_CA },
			{ L"CC", IDI_FLAG_CC },
			{ L"CD", IDI_FLAG_CD },
			{ L"CF", IDI_FLAG_CF },
			{ L"CG", IDI_FLAG_CG },
			{ L"CH", IDI_FLAG_CH },
			{ L"CI", IDI_FLAG_CI },
			{ L"CK", IDI_FLAG_CK },
			{ L"CL", IDI_FLAG_CL },
			{ L"CM", IDI_FLAG_CM },
			{ L"CN", IDI_FLAG_CN },
			{ L"CO", IDI_FLAG_CO },
			{ L"CR", IDI_FLAG_CR },
			{ L"CU", IDI_FLAG_CU },
			{ L"CV", IDI_FLAG_CV },
			{ L"CW", IDI_FLAG_CW },
			{ L"CX", IDI_FLAG_CX },
			{ L"CY", IDI_FLAG_CY },
			{ L"CZ", IDI_FLAG_CZ },
			{ L"DE", IDI_FLAG_DE },
			{ L"DJ", IDI_FLAG_DJ },
			{ L"DK", IDI_FLAG_DK },
			{ L"DM", IDI_FLAG_DM },
			{ L"DO", IDI_FLAG_DO },
			{ L"DZ", IDI_FLAG_DZ },
			{ L"EC", IDI_FLAG_EC },
			{ L"EE", IDI_FLAG_EE },
			{ L"EG", IDI_FLAG_EG },
			{ L"EH", IDI_FLAG_EH },
			{ L"ER", IDI_FLAG_ER },
			{ L"ES", IDI_FLAG_ES },
			{ L"ET", IDI_FLAG_ET },
			{ L"EU", IDI_FLAG_EU },
			{ L"FI", IDI_FLAG_FI },
			{ L"FJ", IDI_FLAG_FJ },
			{ L"FK", IDI_FLAG_FK },
			{ L"FM", IDI_FLAG_FM },
			{ L"FO", IDI_FLAG_FO },
			{ L"FR", IDI_FLAG_FR },
			{ L"GA", IDI_FLAG_GA },
			{ L"GB", IDI_FLAG_GB },
			{ L"GD", IDI_FLAG_GD },
			{ L"GE", IDI_FLAG_GE },
			{ L"GF", IDI_FLAG_GF },
			{ L"GG", IDI_FLAG_GG },
			{ L"GH", IDI_FLAG_GH },
			{ L"GI", IDI_FLAG_GI },
			{ L"GL", IDI_FLAG_GL },
			{ L"GM", IDI_FLAG_GM },
			{ L"GN", IDI_FLAG_GN },
			{ L"GP", IDI_FLAG_GP },
			{ L"GQ", IDI_FLAG_GQ },
			{ L"GR", IDI_FLAG_GR },
			{ L"GS", IDI_FLAG_GS },
			{ L"GT", IDI_FLAG_GT },
			{ L"GU", IDI_FLAG_GU },
			{ L"GW", IDI_FLAG_GW },
			{ L"GY", IDI_FLAG_GY },
			{ L"HK", IDI_FLAG_HK },
			{ L"HM", IDI_FLAG_HM },
			{ L"HN", IDI_FLAG_HN },
			{ L"HR", IDI_FLAG_HR },
			{ L"HT", IDI_FLAG_HT },
			{ L"HU", IDI_FLAG_HU },
			{ L"ID", IDI_FLAG_ID },
			{ L"IE", IDI_FLAG_IE },
			{ L"IL", IDI_FLAG_IL },
			{ L"IM", IDI_FLAG_IM },
			{ L"IN", IDI_FLAG_IN },
			{ L"IO", IDI_FLAG_IO },
			{ L"IQ", IDI_FLAG_IQ },
			{ L"IR", IDI_FLAG_IR },
			{ L"IS", IDI_FLAG_IS },
			{ L"IT", IDI_FLAG_IT },
			{ L"JE", IDI_FLAG_JE },
			{ L"JM", IDI_FLAG_JM },
			{ L"JO", IDI_FLAG_JO },
			{ L"JP", IDI_FLAG_JP },
			{ L"KE", IDI_FLAG_KE },
			{ L"KG", IDI_FLAG_KG },
			{ L"KH", IDI_FLAG_KH },
			{ L"KI", IDI_FLAG_KI },
			{ L"KM", IDI_FLAG_KM },
			{ L"KN", IDI_FLAG_KN },
			{ L"KP", IDI_FLAG_KP },
			{ L"KR", IDI_FLAG_KR },
			{ L"KW", IDI_FLAG_KW },
			{ L"KY", IDI_FLAG_KY },
			{ L"KZ", IDI_FLAG_KZ },
			{ L"LA", IDI_FLAG_LA },
			{ L"LB", IDI_FLAG_LB },
			{ L"LC", IDI_FLAG_LC },
			{ L"LI", IDI_FLAG_LI },
			{ L"LK", IDI_FLAG_LK },
			{ L"LR", IDI_FLAG_LR },
			{ L"LS", IDI_FLAG_LS },
			{ L"LT", IDI_FLAG_LT },
			{ L"LU", IDI_FLAG_LU },
			{ L"LV", IDI_FLAG_LV },
			{ L"LY", IDI_FLAG_LY },
			{ L"MA", IDI_FLAG_MA },
			{ L"MC", IDI_FLAG_MC },
			{ L"MD", IDI_FLAG_MD },
			{ L"ME", IDI_FLAG_ME },
			{ L"MF", IDI_FLAG_MF },
			{ L"MG", IDI_FLAG_MG },
			{ L"MH", IDI_FLAG_MH },
			{ L"MK", IDI_FLAG_MK },
			{ L"ML", IDI_FLAG_ML },
			{ L"MM", IDI_FLAG_MM },
			{ L"MN", IDI_FLAG_MN },
			{ L"MO", IDI_FLAG_MO },
			{ L"MP", IDI_FLAG_MP },
			{ L"MQ", IDI_FLAG_MQ },
			{ L"MR", IDI_FLAG_MR },
			{ L"MS", IDI_FLAG_MS },
			{ L"MT", IDI_FLAG_MT },
			{ L"MU", IDI_FLAG_MU },
			{ L"MV", IDI_FLAG_MV },
			{ L"MW", IDI_FLAG_MW },
			{ L"MX", IDI_FLAG_MX },
			{ L"MY", IDI_FLAG_MY },
			{ L"MZ", IDI_FLAG_MZ },
			{ L"NA", IDI_FLAG_NA },
			{ L"NC", IDI_FLAG_NC },
			{ L"NE", IDI_FLAG_NE },
			{ L"NF", IDI_FLAG_NF },
			{ L"NG", IDI_FLAG_NG },
			{ L"NI", IDI_FLAG_NI },
			{ L"NL", IDI_FLAG_NL },
			{ L"NO", IDI_FLAG_NO },
			{ L"NP", IDI_FLAG_NP },
			{ L"NR", IDI_FLAG_NR },
			{ L"NU", IDI_FLAG_NU },
			{ L"NZ", IDI_FLAG_NZ },
			{ L"OM", IDI_FLAG_OM },
			{ L"PA", IDI_FLAG_PA },
			{ L"PE", IDI_FLAG_PE },
			{ L"PF", IDI_FLAG_PF },
			{ L"PG", IDI_FLAG_PG },
			{ L"PH", IDI_FLAG_PH },
			{ L"PK", IDI_FLAG_PK },
			{ L"PL", IDI_FLAG_PL },
			{ L"PM", IDI_FLAG_PM },
			{ L"PN", IDI_FLAG_PN },
			{ L"PR", IDI_FLAG_PR },
			{ L"PS", IDI_FLAG_PS },
			{ L"PT", IDI_FLAG_PT },
			{ L"PW", IDI_FLAG_PW },
			{ L"PY", IDI_FLAG_PY },
			{ L"QA", IDI_FLAG_QA },
			{ L"RE", IDI_FLAG_RE },
			{ L"RO", IDI_FLAG_RO },
			{ L"RS", IDI_FLAG_RS },
			{ L"RU", IDI_FLAG_RU },
			{ L"RW", IDI_FLAG_RW },
			{ L"SA", IDI_FLAG_SA },
			{ L"SB", IDI_FLAG_SB },
			{ L"SC", IDI_FLAG_SC },
			{ L"SD", IDI_FLAG_SD },
			{ L"SE", IDI_FLAG_SE },
			{ L"SG", IDI_FLAG_SG },
			{ L"SH", IDI_FLAG_SH },
			{ L"SI", IDI_FLAG_SI },
			{ L"SJ", IDI_FLAG_SJ },
			{ L"SK", IDI_FLAG_SK },
			{ L"SL", IDI_FLAG_SL },
			{ L"SM", IDI_FLAG_SM },
			{ L"SN", IDI_FLAG_SN },
			{ L"SO", IDI_FLAG_SO },
			{ L"SR", IDI_FLAG_SR },
			{ L"SS", IDI_FLAG_SS },
			{ L"ST", IDI_FLAG_ST },
			{ L"SV", IDI_FLAG_SV },
			{ L"SX", IDI_FLAG_SX },
			{ L"SY", IDI_FLAG_SY },
			{ L"SZ", IDI_FLAG_SZ },
			{ L"TC", IDI_FLAG_TC },
			{ L"TD", IDI_FLAG_TD },
			{ L"TF", IDI_FLAG_TF },
			{ L"TG", IDI_FLAG_TG },
			{ L"TH", IDI_FLAG_TH },
			{ L"TJ", IDI_FLAG_TJ },
			{ L"TK", IDI_FLAG_TK },
			{ L"TL", IDI_FLAG_TL },
			{ L"TM", IDI_FLAG_TM },
			{ L"TN", IDI_FLAG_TN },
			{ L"TO", IDI_FLAG_TO },
			{ L"TR", IDI_FLAG_TR },
			{ L"TT", IDI_FLAG_TT },
			{ L"TV", IDI_FLAG_TV },
			{ L"TW", IDI_FLAG_TW },
			{ L"TZ", IDI_FLAG_TZ },
			{ L"UA", IDI_FLAG_UA },
			{ L"UG", IDI_FLAG_UG },
			{ L"UM", IDI_FLAG_UM },
			{ L"US", IDI_FLAG_US },
			{ L"UY", IDI_FLAG_UY },
			{ L"UZ", IDI_FLAG_UZ },
			{ L"VA", IDI_FLAG_VA },
			{ L"VC", IDI_FLAG_VC },
			{ L"VE", IDI_FLAG_VE },
			{ L"VG", IDI_FLAG_VG },
			{ L"VI", IDI_FLAG_VI },
			{ L"VN", IDI_FLAG_VN },
			{ L"VU", IDI_FLAG_VU },
			{ L"WF", IDI_FLAG_WF },
			{ L"WS", IDI_FLAG_WS },
			{ L"XK", IDI_FLAG_XK },
			{ L"YE", IDI_FLAG_YE },
			{ L"YT", IDI_FLAG_YT },
			{ L"ZA", IDI_FLAG_ZA },
			{ L"ZM", IDI_FLAG_ZM },
			{ L"ZW", IDI_FLAG_ZW }
		};
};