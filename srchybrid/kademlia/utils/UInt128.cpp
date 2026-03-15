/*
This file is part of eMule AI

Copyright (C)2003 Barry Dunne (https://www.emule-project.net)
Copyright (C)2026 eMule AI

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either
version 2 of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
*/

// Note To Mods //
/*
Please do not change anything here and release it.
There is going to be a new forum created just for the Kademlia side of the client.
If you feel there is an error or a way to improve something, please
post it in the forum first and let us look at it. If it is a real improvement,
it will be added to the official client. Changing something without knowing
what all it does, can cause great harm to the network if released in mass form.
Any mod that changes anything within the Kademlia side will not be allowed to advertise
their client on the eMule forum.
*/

#include "stdafx.h"
#include "cryptopp/osrng.h"
#include "kademlia/utils/UInt128.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

#if (_MSC_FULL_VER > 13009037) && (defined(_M_IX86) || defined(_M_X64) || defined(_M_ARM64))
#pragma intrinsic(_byteswap_ulong)
#endif

using namespace Kademlia;
using namespace CryptoPP;

CUInt128::CUInt128(const CUInt128 &uValue, UINT uNumBits)
{
	CUInt128 mask(1UL);
	CUInt128 random;
	mask.ShiftLeft(128 - uNumBits);
	mask.Subtract(1UL);
	random.SetValueRandom();
	*this = (uValue & ~mask) | (random & mask);
}

CUInt128& CUInt128::SetValueBE(const byte *pbyValueBE)
{
#if (defined(_M_IX86) || defined(_M_AMD64) || defined(_M_ARM64)) && (_MSC_FULL_VER > 13009037)
	m_uData0 = _byteswap_ulong((reinterpret_cast<const ULONG*>(pbyValueBE))[0]);
	m_uData1 = _byteswap_ulong((reinterpret_cast<const ULONG*>(pbyValueBE))[1]);
	m_uData2 = _byteswap_ulong((reinterpret_cast<const ULONG*>(pbyValueBE))[2]);
	m_uData3 = _byteswap_ulong((reinterpret_cast<const ULONG*>(pbyValueBE))[3]);
#else
	SetValue(0ul);
	for (int iIndex = 0; iIndex < 16; ++iIndex)
		m_uData[iIndex / 4] |= ((ULONG)pbyValueBE[iIndex]) << (8 * (3 - (iIndex % 4)));
#endif
	return *this;
}

CUInt128& CUInt128::SetValueRandom()
{
	AutoSeededRandomPool rng;
	byte byRandomBytes[16];
	rng.GenerateBlock(byRandomBytes, 16);
	SetValueBE(byRandomBytes);
	return *this;
}

CUInt128& CUInt128::SetValueGUID()
{
	GUID guid;
	if (CoCreateGuid(&guid) == S_OK) {
		m_uData[0] = guid.Data1;
		m_uData[1] = ((ULONG)guid.Data2) << 16 | guid.Data3;
#if (_MSC_FULL_VER > 13009037) && (defined(_M_IX86) || defined(_M_X64) || defined(_M_ARM64))
		m_uData[2] = _byteswap_ulong(*(ULONG*)guid.Data4);
		m_uData[3] = _byteswap_ulong(*(ULONG*)&guid.Data4[4]);
#else
		m_uData[2] = ((ULONG)guid.Data4[0]) << 24 | ((ULONG)guid.Data4[1]) << 16 | ((ULONG)guid.Data4[2]) << 8 | ((ULONG)guid.Data4[3]);
		m_uData[3] = ((ULONG)guid.Data4[4]) << 24 | ((ULONG)guid.Data4[5]) << 16 | ((ULONG)guid.Data4[6]) << 8 | ((ULONG)guid.Data4[7]);
#endif
	} else
		SetValue(0ul);
	return *this;
}

UINT CUInt128::GetBitNumber(UINT uBit) const
{
	if (uBit > 127)
		return 0;
	int iLongNum = uBit / 32;
	int iShift = 31 - (uBit % 32);
	return ((m_uData[iLongNum] >> iShift) & 1);
}

CUInt128& CUInt128::SetBitNumber(UINT uBit, UINT uValue)
{
	int iLongNum = uBit / 32;
	int iShift = 31 - (uBit % 32);
	m_uData[iLongNum] |= (1 << iShift);
	if (uValue == 0)
		m_uData[iLongNum] ^= (1 << iShift);
	return *this;
}


CUInt128& CUInt128::ShiftLeft(UINT uBits)
{
	if ((uBits == 0) || (CompareTo(0) == 0))
		return *this;
	if (uBits > 127) {
		SetValue(0ul);
		return *this;
	}

	ULONG uResult[4] = {};
	int iIndexShift = (int)uBits / 32;
	__int64 iShifted = 0;
	for (int iIndex = 4; --iIndex >= iIndexShift;) {
		iShifted += ((__int64)m_uData[iIndex]) << (uBits % 32);
		uResult[iIndex - iIndexShift] = (ULONG)iShifted;
		iShifted = iShifted >> 32;
	}
	memcpy(m_uData, uResult, sizeof uResult);
	return *this;
}
