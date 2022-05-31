#pragma once
constexpr int MAX_NETCONSOLE_INPUT_LEN = 4096;
constexpr int MSG_NOSIGNAL = 0;

// typedef struct netpacket_s netpacket_t;
// typedef struct __declspec(align(8)) netpacket_s
//{
//	DWORD         family_maybe;
//	sockaddr_in   sin;
//	WORD          sin_port;
//	char          gap16;
//	char          byte17;
//	DWORD         source;
//	double        received;
//	std::uint8_t* data;
//	std::uint64_t label;
//	BYTE          byte38;
//	std::uint64_t qword40;
//	std::uint64_t qword48;
//	BYTE          gap50[8];
//	std::uint64_t qword58;
//	std::uint64_t qword60;
//	std::uint64_t qword68;
//	int           less_than_12;
//	DWORD         wiresize;
//	BYTE          gap78[8];
//	struct netpacket_s* pNext;
//} netpacket_t;

enum class netadrtype_t
{
	NA_NULL = 0,
	NA_LOOPBACK,
	NA_IP,
};

enum class netadrversion_t
{
	NA_INVALID = -1,
	NA_V4 = 4,
	NA_V6 = 6,
};

class CNetAdr2
{
  public:
	CNetAdr2(void) {};
	CNetAdr2(const std::string& svInAdr);
	CNetAdr2(const std::string& svInAdr, const std::string& svInPort);
	~CNetAdr2(void);

	void SetIP(const std::string& svInAdr);
	void SetPort(const std::string& svInPort);
	void SetIPAndPort(std::string svInAdr);
	void SetIPAndPort(std::string svInAdr, std::string svInPort);
	void SetType(const netadrtype_t& type);
	void SetVersion(void);
	void SetFromSocket(const int& hSocket);
	bool SetFromSockadr(sockaddr_storage* s);

	std::string GetIP(bool bBaseOnly) const;
	std::string GetPort(void) const;
	std::string GetPort(std::string svInPort) const;
	std::string GetIPAndPort(void) const;
	netadrtype_t GetType(void) const;
	netadrversion_t GetVersion(void) const;
	std::string GetBase(void) const;
	std::string GetBase(const std::string& svInAdr) const;
	std::vector<std::string> GetParts(void) const;
	int GetSize(void) const;
	int GetFamily(void) const;

	void ToSockadr(sockaddr_storage* pSadr) const;
	void ToAdrinfo(addrinfo* pHint) const;

	bool IsValidPort(const std::string& svInPort) const;
	bool IsLocalhost(void) const;
	bool IsLoopback(void) const;
	bool IsReservedAdr(void) const;

	bool CompareAdr(const CNetAdr2& adr2, bool bBaseOnly) const;
	bool CompareClassBAdr(const CNetAdr2& adr2) const;
	bool CompareClassCAdr(const CNetAdr2& adr2) const;

	bool StringReplace(std::string& svStr, std::string svFrom, std::string svTo) const;

	void Clear(void);

  private:
	std::string m_svip;
	netadrtype_t m_type {};
	netadrversion_t m_version {};
	sockaddr_storage* m_sadr {};
};
