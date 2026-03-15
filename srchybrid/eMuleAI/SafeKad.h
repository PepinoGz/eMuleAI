//This file is part of eMule AI
//Copyright (C)2026 eMule AI

#include "Kademlia/routing/Contact.h"
#include "Kademlia/routing/Maps.h"

namespace Kademlia
{
	class CSafeKad2
	{
	private:
		class NodeAddress
		{
		public:
			NodeAddress() {}
			NodeAddress(uint32 uIP, uint16 uPort) throw() : m_uIP(uIP), m_uPort(uPort) {}
			NodeAddress(const NodeAddress& uNode) throw() : m_uIP(uNode.m_uIP), m_uPort(uNode.m_uPort) {}
			bool operator< (const NodeAddress &value) const throw() {return (m_uIP < value.m_uIP || (m_uIP == value.m_uIP && m_uPort < value.m_uPort));}
			bool operator<= (const NodeAddress &value) const throw() {return (m_uIP < value.m_uIP || (m_uIP == value.m_uIP && m_uPort <= value.m_uPort));}
			bool operator> (const NodeAddress &value) const throw() {return (m_uIP > value.m_uIP || (m_uIP == value.m_uIP && m_uPort > value.m_uPort));}
			bool operator>= (const NodeAddress &value) const throw() {return (m_uIP > value.m_uIP || (m_uIP == value.m_uIP && m_uPort >= value.m_uPort));}
			bool operator== (const NodeAddress &value) const throw() {return (m_uIP == value.m_uIP && m_uPort == value.m_uPort);}
			bool operator!= (const NodeAddress &value) const throw() {return (m_uIP != value.m_uIP || m_uPort != value.m_uPort);}
		private:
			uint32 m_uIP;
			uint16 m_uPort;
		};
		template<class _T>
		class AgeOf
		{
		public:
			AgeOf(time_t tAge, _T key) throw() : m_tAge(tAge), m_key(key) {}
			bool operator< (const AgeOf &value) const throw() {return (m_tAge < value.m_tAge || (m_tAge == value.m_tAge && m_key < value.m_key));}
		private:
			time_t	m_tAge;
			_T		m_key;
		};
		struct Tracked
		{
			CUInt128	m_uLastID;
			time_t		m_tLastIDChange;
			time_t		m_tLastReferenced;
			unsigned	m_isIDVerified:1;
			NodeAddress m_uNode;
			operator AgeOf<NodeAddress>() const throw() {return AgeOf<NodeAddress>(m_tLastReferenced, m_uNode);}
		};
		struct Banned
		{
			time_t		m_tBanned;
			time_t		m_tLastReferenced;
			uint32		m_uIP;
			operator AgeOf<uint32>() const throw() {return AgeOf<uint32>(m_tLastReferenced, m_uIP);}
		};
		struct Problematic
		{
			time_t		m_tFailed;
			time_t		m_tLastReferenced;
			NodeAddress m_uNode;
			operator AgeOf<NodeAddress>() const throw() {return AgeOf<NodeAddress>(m_tLastReferenced, m_uNode);}
		};
	
	public:
		CSafeKad2() throw();
		~CSafeKad2() throw();
		void TrackNode(uint32 uIP, uint16 uPort, const CUInt128& uID, bool isIDVerified = false) throw();
		void TrackProblematicNode(uint32 uIP, uint16 uPort, const CUInt128& uID = CUInt128(0UL)) throw();
		void BanIP(uint32 uIP) throw();
		bool IsBadNode(uint32 uIP, uint16 uPort, const CUInt128& uID, uint8 kadVersion, bool isIDVerified = false, bool onlyOneNodePerIP = true) throw();
		bool IsBanned(uint32 uIP) throw();
		bool IsProblematic(uint32 uIP, uint16 uPort) throw();
		void ShutdownCleanup() throw();

	private:
		static const size_t s_uMaxTrackedNodes = 10000;
		static const size_t s_uMaxProblematicNodes = 10000;
		static const size_t s_uMaxBannedIPs = 1000;
		static const time_t s_tMinimumIDChangeInterval = 3600; // 1 hour
		static const time_t s_tMaximumBanTime = 4 * 3600; // 4 hours
		static const time_t s_tMaximumProblematicTime = 5 * 60; // 5 minutes

		void Cleanup(time_t tNodeMaxRefAge = 3600, time_t tBanMaxRefAge = 3600, time_t tProblematicMaxRefAge = 300) throw();
		std::map<NodeAddress, Tracked*> m_mapTrackedNodes;
		std::map<AgeOf<NodeAddress>, Tracked*> m_mapTrackedAge;
		std::map<NodeAddress, Problematic*> m_mapProblematicNodes;
		std::map<AgeOf<NodeAddress>, Problematic*> m_mapProblematicAge;
		std::map<uint32, Banned*> m_mapBannedIPs;
		std::map<AgeOf<uint32>, Banned*> m_mapBannedAge;
		time_t m_tLastCleanup;
	};

	extern CSafeKad2 safeKad;
}
