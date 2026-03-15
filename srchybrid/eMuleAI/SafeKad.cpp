//This file is part of eMule AI
//Copyright (C)2026 eMule AI

#include "stdafx.h"
#include "SafeKad.h"
#include "Opcodes.h"
#include "Preferences.h"

using namespace Kademlia;

namespace Kademlia
{
	CSafeKad2	safeKad;
}

CSafeKad2::CSafeKad2() throw()
{
	m_tLastCleanup = time(nullptr);
}

CSafeKad2::~CSafeKad2() throw()
{
	for (std::map<NodeAddress, Tracked*>::const_iterator it = m_mapTrackedNodes.begin(); it != m_mapTrackedNodes.end(); ++it)
		delete it->second;
	for (std::map<uint32, Banned*>::const_iterator it = m_mapBannedIPs.begin(); it != m_mapBannedIPs.end(); ++it)
		delete it->second;
	for (std::map<NodeAddress, Problematic*>::const_iterator it = m_mapProblematicNodes.begin(); it != m_mapProblematicNodes.end(); ++it)
		delete it->second;
}

void CSafeKad2::TrackNode(uint32 const uIP, uint16 const uPort, const CUInt128& uID, bool const isIDVerified) throw()
{
	if (IsBanned(uIP))
		return;

	time_t const tCurrent = time(nullptr);
	Tracked* tracked;
	std::map<NodeAddress, Tracked*>::const_iterator it = m_mapTrackedNodes.find(NodeAddress(uIP, uPort));
	if (it == m_mapTrackedNodes.end())
	{
		Cleanup(); // Make space for a new node to track

		if (m_mapTrackedNodes.size() >= s_uMaxTrackedNodes) // Don't track more than 10000 nodes
			return;
		tracked = new (std::nothrow) Tracked;
		if (tracked == nullptr) // Should only happen if we run out of memory!
			return;
		tracked->m_uLastID = uID;
		tracked->m_tLastIDChange = tCurrent;
		tracked->m_isIDVerified = isIDVerified;
		tracked->m_uNode = NodeAddress(uIP, uPort);
	}
	else
		tracked = it->second;
	m_mapTrackedAge.erase(*tracked);
	if (uID != tracked->m_uLastID && !(tracked->m_isIDVerified && !isIDVerified)) {
		if (thePrefs.IsBanBadKadNodes() && tCurrent - tracked->m_tLastIDChange < s_tMinimumIDChangeInterval && isIDVerified) { // If change occurs more often than once an hour, ban this IP
			BanIP(uIP);
			m_mapTrackedNodes.erase(tracked->m_uNode);
			delete tracked;
			return;
		}
		tracked->m_uLastID = uID;
		tracked->m_tLastIDChange = tCurrent;
	}
	tracked->m_tLastReferenced = tCurrent;
	if (tracked->m_isIDVerified == false) // Only update verified status if not obfuscated before
		tracked->m_isIDVerified = isIDVerified;
	m_mapTrackedNodes[tracked->m_uNode] = tracked;
	m_mapTrackedAge[*tracked] = tracked;
}

void CSafeKad2::TrackProblematicNode(uint32 const uIP, uint16 const uPort, const CUInt128& /*uID*/) throw()
{
	if (IsBanned(uIP))
		return;

	time_t const tCurrent = time(nullptr);
	Problematic* problematic;
	std::map<NodeAddress, Problematic*>::const_iterator it = m_mapProblematicNodes.find(NodeAddress(uIP, uPort));
	if (it == m_mapProblematicNodes.end())
	{
		Cleanup(); // Make space for a new node to track

		if (m_mapProblematicNodes.size() >= s_uMaxProblematicNodes) // Don't track more than 10000 nodes
			return;
		problematic = new (std::nothrow) Problematic;
		if (problematic == nullptr) // Should only happen if we run out of memory!
			return;
		problematic->m_tFailed = tCurrent;
		problematic->m_uNode = NodeAddress(uIP, uPort);
	}
	else
		problematic = it->second;

	m_mapProblematicAge.erase(*problematic);
	problematic->m_tLastReferenced = tCurrent;
	m_mapProblematicNodes[problematic->m_uNode] = problematic;
	m_mapProblematicAge[*problematic] = problematic;
}

// NOTE!!! 
//  Use this functon with great caution! 
//  Only use if you can with great certanity tell that the IP is used by a bad node.
//  Make sure you haven't been decived by another node to think this IP is bad!
void CSafeKad2::BanIP(uint32 const uIP) throw()
{
	time_t const tCurrent = time(nullptr);
	Cleanup(); // Make space for a new IP to ban

	Banned* banned;
	std::map<uint32, Banned*>::const_iterator it = m_mapBannedIPs.find(uIP);
	if (it == m_mapBannedIPs.end())
	{
		if (m_mapBannedIPs.size() >= s_uMaxBannedIPs) // Don't ban more than 1000 IPs (if we reach this we are probably in deep shit!)
			return;
		banned = new (std::nothrow) Banned;
		if (banned == nullptr) // Should only happen if we run out of memory!
			return;
		banned->m_uIP = uIP;
	}
	else
		banned = it->second;
	m_mapBannedAge.erase(*banned);
	banned->m_tBanned = tCurrent;
	banned->m_tLastReferenced = tCurrent;
	m_mapBannedIPs[banned->m_uIP] = banned;
	m_mapBannedAge[*banned] = banned;
}

bool CSafeKad2::IsBadNode(uint32 const uIP, uint16 const uPort, const CUInt128& uID, uint8 const uKadVersion, bool const isIDVerified, bool const onlyOneNodePerIP) throw()
{
	time_t const tCurrent = time(nullptr);
	if (tCurrent - m_tLastCleanup > MIN2S(10))
		Cleanup();

	std::map<NodeAddress, Tracked*>::iterator itTracked = m_mapTrackedNodes.find(NodeAddress(uIP, uPort));
	if (IsBanned(uIP)) {
		if (itTracked != m_mapTrackedNodes.end()) {
			// Cache pointer BEFORE erasing iterator (iterator becomes invalid after erase)
			Tracked* pTracked = itTracked->second; // Avoid use-after-erase
			m_mapTrackedNodes.erase(itTracked);
			if (pTracked != nullptr) {
				if (m_mapTrackedAge.find(*pTracked) != m_mapTrackedAge.end())
					m_mapTrackedAge.erase(*pTracked);
				delete pTracked;
			}
		}
		return true;
	}
	if (itTracked != m_mapTrackedNodes.end()) {
		if (itTracked->second != nullptr && m_mapTrackedAge.find(*itTracked->second) != m_mapTrackedAge.end())
			m_mapTrackedAge.erase(*itTracked->second);
		itTracked->second->m_tLastReferenced = tCurrent;
		m_mapTrackedAge[*itTracked->second] = itTracked->second;
		if (itTracked->second->m_uLastID != uID) {
			if ((itTracked->second->m_isIDVerified || uKadVersion < KADEMLIA_VERSION8_49b) && !isIDVerified) // Don't allow new ID's for older KAD version nodes
				return true;
			TrackNode(uIP, uPort, uID, isIDVerified); // Update ID and ban if appropriate
			return IsBanned(uIP);
		}
	} else {
		if (onlyOneNodePerIP) {
			if (m_mapTrackedNodes.lower_bound(NodeAddress(uIP, 0)) != m_mapTrackedNodes.lower_bound(NodeAddress(uIP, 0xFFFF))) // Is there already a tracked node at this IP?
				return true;
		}
		TrackNode(uIP, uPort, uID, isIDVerified); // Start tracking this node
	}
	return false;
}

bool CSafeKad2::IsBanned(uint32 const uIP) throw() {
	std::map<uint32, Banned*>::iterator itBanned = m_mapBannedIPs.find(uIP);
	if (itBanned != m_mapBannedIPs.end()) {
		Banned* pBanned = itBanned->second; // Cache pointer before iterator invalidation
		time_t const tCurrent = time(nullptr);
		m_mapBannedAge.erase(*pBanned);
		pBanned->m_tLastReferenced = tCurrent;
		if (tCurrent - pBanned->m_tBanned > s_tMaximumBanTime) { // Max ban time is 4 hours
			m_mapBannedIPs.erase(itBanned);
			delete pBanned;
		} else {
			m_mapBannedAge[*pBanned] = pBanned;
			return true;
		}
	}
	return false;
}

bool CSafeKad2::IsProblematic(uint32 const uIP, uint16 const uPort) throw()
{
	std::map<NodeAddress, Problematic*>::iterator itProblematic = m_mapProblematicNodes.find(NodeAddress(uIP, uPort));
	if (itProblematic != m_mapProblematicNodes.end()) {
		time_t const tCurrent = time(nullptr);
		Problematic* pProblematic = itProblematic->second; // Cache pointer before potential iterator invalidation
		m_mapProblematicAge.erase(*pProblematic);
		pProblematic->m_tLastReferenced = tCurrent;
		if (tCurrent - pProblematic->m_tFailed > s_tMaximumProblematicTime) { // Max time to ignore problematic nodes are 5 minutes
			m_mapProblematicNodes.erase(itProblematic);
			delete pProblematic; // Safe delete after erase
		} else {
			m_mapProblematicAge[*pProblematic] = pProblematic;
			return true;
		}
	}
	return IsBanned(uIP); // If an IP is banned it is problematic
}

void CSafeKad2::ShutdownCleanup() throw()
{
	// Release all cached nodes to avoid shutdown-time leak noise.
	for (std::map<NodeAddress, Tracked*>::const_iterator it = m_mapTrackedNodes.begin(); it != m_mapTrackedNodes.end(); ++it)
		delete it->second;
	for (std::map<uint32, Banned*>::const_iterator it = m_mapBannedIPs.begin(); it != m_mapBannedIPs.end(); ++it)
		delete it->second;
	for (std::map<NodeAddress, Problematic*>::const_iterator it = m_mapProblematicNodes.begin(); it != m_mapProblematicNodes.end(); ++it)
		delete it->second;

	m_mapTrackedNodes.clear();
	m_mapTrackedAge.clear();
	m_mapBannedIPs.clear();
	m_mapBannedAge.clear();
	m_mapProblematicNodes.clear();
	m_mapProblematicAge.clear();
	m_tLastCleanup = time(nullptr);
}

void CSafeKad2::Cleanup(time_t const tNodeMaxRefAge, time_t const tBanMaxRefAge, time_t const tProblematicMaxRefAge) throw()
{
	time_t const tCurrent = time(nullptr);
	// Clean out nodes that haven't been referenced for more than tNodeMaxRefAge
	for (std::map<AgeOf<NodeAddress>, Tracked*>::iterator it = m_mapTrackedAge.begin(); it != m_mapTrackedAge.end();) {
		Tracked* const tracked = (it++)->second;
		if (tCurrent - tracked->m_tLastReferenced > tNodeMaxRefAge)	{
			m_mapTrackedNodes.erase(tracked->m_uNode);
			m_mapTrackedAge.erase(*tracked);
			delete tracked;
		} else 
			break;
	}
	// Clean out IP's that haven't been referenced for more than tBanMaxRefAge
	for (std::map<AgeOf<uint32>, Banned*>::iterator it = m_mapBannedAge.begin(); it != m_mapBannedAge.end();) {
		Banned* const banned = (it++)->second;
		if (tCurrent - banned->m_tLastReferenced > tBanMaxRefAge) {
			m_mapBannedIPs.erase(banned->m_uIP);
			m_mapBannedAge.erase(*banned);
			delete banned;
		} else
			break;
	}
	// Clean out IP's that haven't been referenced for more than tProblematicMaxRefAge
	for (std::map<AgeOf<NodeAddress>, Problematic*>::iterator it = m_mapProblematicAge.begin(); it != m_mapProblematicAge.end();) {
		Problematic* const problematic = (it++)->second;
		if (tCurrent - problematic->m_tLastReferenced > tProblematicMaxRefAge) {
			m_mapProblematicNodes.erase(problematic->m_uNode);
			m_mapProblematicAge.erase(*problematic);
			delete problematic;
		} else
			break;
	}
}

