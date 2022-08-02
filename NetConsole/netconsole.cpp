//=====================================================================================//
//
// Purpose: Lightweight netconsole client.
//
//=====================================================================================//

#include "pch.h"
#include "NetAdr2.h"
#include "socketcreator.h"
#include "sv_rcon.pb.h"
#include "cl_rcon.pb.h"
#include "net.h"
#include "netconsole.h"

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
CNetCon::CNetCon(void)
	: m_bInitialized(false), m_bNoColor(false), m_bQuitApplication(false), m_abPromptConnect(true), m_abConnEstablished(false)
{
	m_pNetAdr2 = new CNetAdr2("localhost", "37015");
	m_pSocket = new CSocketCreator();
}

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
CNetCon::~CNetCon(void)
{
	delete m_pNetAdr2;
	delete m_pSocket;
}

//-----------------------------------------------------------------------------
// Purpose: WSA and NETCON systems init
// Output : true on success, false otherwise
//-----------------------------------------------------------------------------
bool CNetCon::Init(void)
{
	WSAData wsaData {};
	int nError = ::WSAStartup(MAKEWORD(2, 2), &wsaData);

	if (nError != 0)
	{
		spdlog::error("Failed to start Winsock via WSAStartup: ({:s})", NET_ErrorString(WSAGetLastError()));
		return false;
	}

	this->TermSetup();

	std::thread tFrame(&CNetCon::RunFrame, this);
	tFrame.detach();

	return true;
}

//-----------------------------------------------------------------------------
// Purpose: WSA and NETCON systems shutdown
// Output : true on success, false otherwise
//-----------------------------------------------------------------------------
bool CNetCon::Shutdown(void)
{
	m_pSocket->CloseAllAcceptedSockets();
	m_abConnEstablished = false;

	int nError = ::WSACleanup();
	if (nError != 0)
	{
		spdlog::error("Failed to stop winsock via WSACleanup: ({:s})", NET_ErrorString(WSAGetLastError()));
		return false;
	}
	return true;
}

//-----------------------------------------------------------------------------
// Purpose: terminal setup
//-----------------------------------------------------------------------------
void CNetCon::TermSetup(void)
{
	DWORD dwMode = NULL;
	HANDLE hInput = GetStdHandle(STD_INPUT_HANDLE);
	HANDLE hOutput = GetStdHandle(STD_OUTPUT_HANDLE);

	CONSOLE_SCREEN_BUFFER_INFOEX sbInfoEx {};
	COLORREF storedBG = sbInfoEx.ColorTable[0];
	sbInfoEx.cbSize = sizeof(CONSOLE_SCREEN_BUFFER_INFOEX);

	GetConsoleScreenBufferInfoEx(hOutput, &sbInfoEx);
	sbInfoEx.ColorTable[0] = 0x0000;
	SetConsoleScreenBufferInfoEx(hOutput, &sbInfoEx);
}

//-----------------------------------------------------------------------------
// Purpose: gets input IP and port for initialization
//-----------------------------------------------------------------------------
void CNetCon::UserInput(void)
{
	std::string svInput;

	if (std::getline(std::cin, svInput))
	{
		if (strcmp(svInput.c_str(), "nquit") == 0)
		{
			m_bQuitApplication = true;
			return;
		}
		if (m_abConnEstablished)
		{
			if (strcmp(svInput.c_str(), "disconnect") == 0)
			{
				this->Disconnect();
				return;
			}

			std::vector<std::string> vSubStrings = StringSplit(svInput, ' ', 2);
			if (vSubStrings.size() > 1)
			{
				if (strcmp(vSubStrings[0].c_str(), "PASS") == 0) // Auth with RCON server.
				{
					std::string svSerialized = this->Serialize(vSubStrings[1], "", cl_rcon::request_t::SERVERDATA_REQUEST_AUTH);
					this->Send(svSerialized);
				}
				else if (strcmp(vSubStrings[0].c_str(), "SET") == 0) // Set value query.
				{
					if (vSubStrings.size() > 2)
					{
						std::string svSerialized =
							this->Serialize(vSubStrings[1], vSubStrings[2], cl_rcon::request_t::SERVERDATA_REQUEST_SETVALUE);
						this->Send(svSerialized);
					}
				}
				else // Execute command query.
				{
					std::string svSerialized = this->Serialize(svInput.c_str(), "", cl_rcon::request_t::SERVERDATA_REQUEST_EXECCOMMAND);
					this->Send(svSerialized);
				}
			}
			else if (!svInput.empty()) // Single arg command query.
			{
				std::string svSerialized = this->Serialize(svInput.c_str(), "", cl_rcon::request_t::SERVERDATA_REQUEST_EXECCOMMAND);
				this->Send(svSerialized);
			}
		}
		else // Setup connection from input.
		{
			if (!svInput.empty())
			{
				std::string::size_type nPos = svInput.find(' ');
				if (nPos > 0 && nPos < svInput.size() && nPos != svInput.size())
				{
					std::string svInPort = svInput.substr(nPos + 1);
					std::string svInAdr = svInput.erase(svInput.find(' '));

					if (!this->Connect(svInAdr, svInPort))
					{
						m_abPromptConnect = true;
						return;
					}
				}
			}
			else // Initialize as [127.0.0.1]:37015.
			{
				if (!this->Connect("", ""))
				{
					m_abPromptConnect = true;
					return;
				}
			}
		}
	}
}

//-----------------------------------------------------------------------------
// Purpose: client's main processing loop
//-----------------------------------------------------------------------------
void CNetCon::RunFrame(void)
{
	for (;;)
	{
		if (m_abConnEstablished)
		{
			std::this_thread::sleep_for(std::chrono::milliseconds(50));
			this->Recv();
		}
		else if (m_abPromptConnect)
		{
			std::cout << "Enter <IP> <PORT>: ";
			m_abPromptConnect = false;
		}
	}
}

//-----------------------------------------------------------------------------
// Purpose: checks if application should be terminated
// Output : true for termination, false otherwise
//-----------------------------------------------------------------------------
bool CNetCon::ShouldQuit(void) const
{
	return this->m_bQuitApplication;
}

//-----------------------------------------------------------------------------
// Purpose: connect to specified address and port
// Input  : *svInAdr -
//			*svInPort -
// Output : true if connection succeeds, false otherwise
//-----------------------------------------------------------------------------
bool CNetCon::Connect(const std::string& svInAdr, const std::string& svInPort)
{
	if (svInAdr.size() > 0 && svInPort.size() > 0)
	{
		// Default is [127.0.0.1]:37015
		m_pNetAdr2->SetIPAndPort(svInAdr, svInPort);
	}

	if (m_pSocket->ConnectSocket(*m_pNetAdr2, true) == SOCKET_ERROR)
	{
		spdlog::warn("Failed to connect. Error: (SOCKET_ERROR). Verify IP and PORT.");
		return false;
	}
	spdlog::info("Connected to: {:s}", m_pNetAdr2->GetIPAndPort());

	m_abConnEstablished = true;
	return true;
}

//-----------------------------------------------------------------------------
// Purpose: disconnect from current session
//-----------------------------------------------------------------------------
void CNetCon::Disconnect(void)
{
	m_pSocket->CloseAcceptedSocket(0);
	m_abPromptConnect = true;
	m_abConnEstablished = false;
}

//-----------------------------------------------------------------------------
// Purpose: send message
// Input  : *svMessage -
//-----------------------------------------------------------------------------
void CNetCon::Send(const std::string& svMessage) const
{
	std::ostringstream ssSendBuf;

	ssSendBuf << static_cast<uint8_t>(static_cast<int>(svMessage.size()) >> 24);
	ssSendBuf << static_cast<uint8_t>(static_cast<int>(svMessage.size()) >> 16);
	ssSendBuf << static_cast<uint8_t>(static_cast<int>(svMessage.size()) >> 8);
	ssSendBuf << static_cast<uint8_t>(static_cast<int>(svMessage.size()));
	ssSendBuf << svMessage;

	int nSendResult = ::send(
		m_pSocket->GetAcceptedSocketData(0)->m_hSocket, ssSendBuf.str().data(), static_cast<int>(ssSendBuf.str().size()), MSG_NOSIGNAL);
	if (nSendResult == SOCKET_ERROR)
	{
		spdlog::warn("Failed to send message: (SOCKET_ERROR)");
	}
}

//-----------------------------------------------------------------------------
// Purpose: receive message
//-----------------------------------------------------------------------------
void CNetCon::Recv(void)
{
	static char szRecvBuf[1024];
	CConnectedNetConsoleData* pData = m_pSocket->GetAcceptedSocketData(0);

	{ //////////////////////////////////////////////
		int nPendingLen = ::recv(pData->m_hSocket, szRecvBuf, sizeof(char), MSG_PEEK);
		if (nPendingLen == SOCKET_ERROR && m_pSocket->IsSocketBlocking())
		{
			return;
		}
		if (nPendingLen <= 0 && m_abConnEstablished) // EOF or error.
		{
			this->Disconnect();
			spdlog::info("Server closed connection");
			return;
		}
	} //////////////////////////////////////////////

	u_long nReadLen; // Find out how much we have to read.
	::ioctlsocket(pData->m_hSocket, FIONREAD, &nReadLen);

	while (nReadLen > 0)
	{
		int nRecvLen = ::recv(pData->m_hSocket, szRecvBuf, MIN(sizeof(szRecvBuf), nReadLen), MSG_NOSIGNAL);
		if (nRecvLen == 0 && m_abConnEstablished) // Socket was closed.
		{
			this->Disconnect();
			spdlog::info("Server closed connection");
			break;
		}
		if (nRecvLen < 0 && !m_pSocket->IsSocketBlocking())
		{
			spdlog::error("RCON Cmd: recv error ({:s})", NET_ErrorString(WSAGetLastError()));
			break;
		}

		nReadLen -= nRecvLen; // Process what we've got.
		this->ProcessBuffer(szRecvBuf, nRecvLen, pData);
	}
}

//-----------------------------------------------------------------------------
// Purpose: parses input response buffer using length-prefix framing
// Input  : *pRecvBuf -
//			nRecvLen -
//			*pData -
//-----------------------------------------------------------------------------
void CNetCon::ProcessBuffer(const char* pRecvBuf, int nRecvLen, CConnectedNetConsoleData* pData)
{
	while (nRecvLen > 0)
	{
		if (pData->m_nPayloadLen)
		{
			if (pData->m_nPayloadRead < pData->m_nPayloadLen)
			{
				pData->m_RecvBuffer[pData->m_nPayloadRead++] = *pRecvBuf;

				pRecvBuf++;
				nRecvLen--;
			}
			if (pData->m_nPayloadRead == pData->m_nPayloadLen)
			{
				this->ProcessMessage(
					this->Deserialize(std::string(reinterpret_cast<char*>(pData->m_RecvBuffer.data()), pData->m_nPayloadLen)));

				pData->m_nPayloadLen = 0;
				pData->m_nPayloadRead = 0;
			}
		}
		else if (pData->m_nPayloadRead < sizeof(int)) // Read size field.
		{
			pData->m_RecvBuffer[pData->m_nPayloadRead++] = *pRecvBuf;

			pRecvBuf++;
			nRecvLen--;
		}
		else // Build prefix.
		{
			pData->m_nPayloadLen = static_cast<int>(
				pData->m_RecvBuffer[0] << 24 | pData->m_RecvBuffer[1] << 16 | pData->m_RecvBuffer[2] << 8 | pData->m_RecvBuffer[3]);
			pData->m_nPayloadRead = 0;

			if (pData->m_nPayloadLen < 0)
			{
				spdlog::error("RCON Cmd: sync error ({:d})", pData->m_nPayloadLen);
				this->Disconnect(); // Out of sync (irrecoverable).

				break;
			}
			else
			{
				pData->m_RecvBuffer.resize(pData->m_nPayloadLen);
			}
		}
	}
}

//-----------------------------------------------------------------------------
// Purpose: processes received message
// Input  : *sv_response -
//-----------------------------------------------------------------------------
void CNetCon::ProcessMessage(const sv_rcon::response& sv_response) const
{
	switch (sv_response.responsetype())
	{
	case sv_rcon::response_t::SERVERDATA_RESPONSE_AUTH:
	case sv_rcon::response_t::SERVERDATA_RESPONSE_CONSOLE_LOG:
	{
		std::string svOut = sv_response.responsebuf();
		svOut.erase(std::remove(svOut.begin(), svOut.end(), '\n'), svOut.end());
		spdlog::info(svOut);
		break;
	}
	default:
	{
		break;
	}
	}
}

//-----------------------------------------------------------------------------
// Purpose: serializes input
// Input  : *svReqBuf -
//			*svReqVal -
//			request_t -
// Output : serialized results as string
//-----------------------------------------------------------------------------
std::string CNetCon::Serialize(const std::string& svReqBuf, const std::string& svReqVal, cl_rcon::request_t request_t) const
{
	cl_rcon::request cl_request;

	cl_request.set_requestid(-1);
	cl_request.set_requesttype(request_t);

	switch (request_t)
	{
	case cl_rcon::request_t::SERVERDATA_REQUEST_SETVALUE:
	case cl_rcon::request_t::SERVERDATA_REQUEST_AUTH:
	{
		cl_request.set_requestbuf(svReqBuf);
		cl_request.set_requestval(svReqVal);
		break;
	}
	case cl_rcon::request_t::SERVERDATA_REQUEST_EXECCOMMAND:
	{
		cl_request.set_requestbuf(svReqBuf);
		break;
	}
	}
	return cl_request.SerializeAsString();
}

//-----------------------------------------------------------------------------
// Purpose: de-serializes input
// Input  : *svBuf -
// Output : de-serialized object
//-----------------------------------------------------------------------------
sv_rcon::response CNetCon::Deserialize(const std::string& svBuf) const
{
	sv_rcon::response sv_response;
	sv_response.ParseFromArray(svBuf.data(), static_cast<int>(svBuf.size()));

	return sv_response;
}

//-----------------------------------------------------------------------------
// Purpose: entrypoint
// Input  : argc -
//			*argv -
//-----------------------------------------------------------------------------
int main(int argc, char* argv[])
{
	CNetCon* pNetCon = new CNetCon();
	std::cout << "Northstar TCP net console [Version " << NETCON_VERSION << "]" << std::endl;
	spdlog::default_logger()->set_pattern("[%H:%M:%S] [%l] %v");

	if (!pNetCon->Init())
	{
		return EXIT_FAILURE;
	}

	if (argc >= 3) // Get IP and Port from command line.
	{
		if (!pNetCon->Connect(argv[1], argv[2]))
		{
			return EXIT_FAILURE;
		}
	}

	while (!pNetCon->ShouldQuit())
	{
		pNetCon->UserInput();
	}

	if (!pNetCon->Shutdown())
	{
		return EXIT_FAILURE;
	}

	return ERROR_SUCCESS;
}

///////////////////////////////////////////////////////////////////////////////
// For splitting a string into substrings by delimiter.
std::vector<std::string>
StringSplit(std::string svInput, char cDelim, size_t nMax) // Move this to a dedicated string util file if one ever gets created.
{
	std::string svSubString;
	std::vector<std::string> vSubStrings;

	svInput = svInput + cDelim;

	for (size_t i = 0; i < svInput.size(); i++)
	{
		if (i != (svInput.size() - 1) && vSubStrings.size() >= nMax || svInput[i] != cDelim)
		{
			svSubString += svInput[i];
		}
		else
		{
			if (svSubString.size() != 0)
			{
				vSubStrings.push_back(svSubString);
			}
			svSubString.clear();
		}
	}
	return vSubStrings;
}