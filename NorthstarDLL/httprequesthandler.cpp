#include "pch.h"
#include "httprequesthandler.h"
#include "version.h"
#include "squirrel.h"
#include "tier0.h"


HttpRequestHandler* g_httpRequestHandler;

void HttpRequestHandler::StartHttpRequestHandler()
{
	if (IsRunning())
	{
		spdlog::warn("HttpRequestHandler::StartHttpRequestHandler was called while IsRunning() is true");
		return;
	}

	m_bIsHttpRequestHandlerRunning = true;
	spdlog::info("HttpRequestHandler started.");
}

void HttpRequestHandler::StopHttpRequestHandler()
{
	if (!IsRunning())
	{
		spdlog::warn("HttpRequestHandler::StopHttpRequestHandler was called while IsRunning() is false");
		return;
	}


	m_bIsHttpRequestHandlerRunning = false;
	spdlog::info("HttpRequestHandler stopped.");
}

bool HttpRequestHandler::IsDestinationHostAllowed(const std::string& host, std::string& outResolvedHost, std::string& outHostHeader)
{
	bool bAllowed = false;

	CURLU* url = curl_url();
	if (!url)
	{
		spdlog::error("Failed to call curl_url() for http request.");
		return false;
	}

	if (curl_url_set(url, CURLUPART_URL, host.c_str(), 0) != CURLUE_OK)
	{
		spdlog::error("Failed to parse destination URL for http request.");

		curl_url_cleanup(url);
		return false;
	}

	char* urlHostname = nullptr;
	if (curl_url_get(url, CURLUPART_HOST, &urlHostname, 0) != CURLUE_OK)
	{
		spdlog::error("Failed to parse hostname from destination URL for http request.");

		curl_url_cleanup(url);
		return false;
	}

	char* urlScheme = nullptr;
	if (curl_url_get(url, CURLUPART_SCHEME, &urlHostname, CURLU_DEFAULT_SCHEME))
	{
		spdlog::error("Failed to parse scheme from destination URL for http request.");

		curl_url_cleanup(url);
		curl_free(urlHostname);
		return false;
	}

	// Resolve the hostname into an address.
	addrinfo* result;
	if (getaddrinfo(urlHostname, urlScheme, nullptr, &result) != 0)
	{
		spdlog::error("Failed to resolve http request destination {} using getaddrinfo().", urlHostname);

		curl_url_cleanup(url);
		curl_free(urlHostname);
		curl_free(urlScheme);
		return false;
	}

	bool bFoundIPv6;
	sockaddr_in* sockaddr_ipv4 = nullptr;
	for (addrinfo* info = result; info; info = info->ai_next)
	{
		if (info->ai_family == AF_INET)
		{
			sockaddr_ipv4 = (sockaddr_in*)info->ai_addr;
			break;
		}

		bFoundIPv6 &= info->ai_family == AF_INET6;
	}

	if (sockaddr_ipv4 == nullptr)
	{
		if (bFoundIPv6)
		{
			spdlog::error("Only IPv4 destinations are supported for HTTP requests. To allow IPv6, launch the game using -allowlocalhttp.");
		}
		else
		{
			spdlog::error("Failed to resolve http request destination {} into a valid IPv4 address.", urlHostname);
		}
		
		goto CLEANUP;
	}

	// Fast checks for private ranges of IPv4.
	{
		auto addrBytes = sockaddr_ipv4->sin_addr.S_un.S_un_b;

		if (addrBytes.s_b1 == 10															// 10.0.0.0			- 10.255.255.255		(Class A Private)
			|| addrBytes.s_b1 == 172 && addrBytes.s_b2 >= 16 && addrBytes.s_b2 <= 31		// 172.16.0.0		- 172.31.255.255		(Class B Private)
			|| addrBytes.s_b1 == 192 && addrBytes.s_b2 == 168								// 192.168.0.0		- 192.168.255.255		(Class C Private)
			|| addrBytes.s_b1 == 192 && addrBytes.s_b2 == 0 && addrBytes.s_b3 == 0			// 192.0.0.0		- 192.0.0.255			(IETF Assignment)
			|| addrBytes.s_b1 == 192 && addrBytes.s_b2 == 0 && addrBytes.s_b3 == 2			// 192.0.2.0		- 192.0.2.255			(TEST-NET-1)
			|| addrBytes.s_b1 == 192 && addrBytes.s_b2 == 88 && addrBytes.s_b3 == 99		// 192.88.99.0		- 192.88.99.255			(IPv4-IPv6 Relay)
			|| addrBytes.s_b1 == 192 && addrBytes.s_b2 >= 18 && addrBytes.s_b2 <= 19		// 192.18.0.0		- 192.19.255.255		(Internet Benchmark)
			|| addrBytes.s_b1 == 192 && addrBytes.s_b2 == 51 && addrBytes.s_b3 == 100		// 192.51.100.0		- 192.51.100.255		(TEST-NET-2)
			|| addrBytes.s_b1 == 203 && addrBytes.s_b2 == 0 && addrBytes.s_b3 == 113		// 203.0.113.0		- 203.0.113.255			(TEST-NET-3)
			|| addrBytes.s_b1 == 169 && addrBytes.s_b2 == 254								// 169.254.00		- 169.254.255.255		(Link-local/APIPA) 
			|| addrBytes.s_b1 == 127														// 127.0.0.0		- 127.255.255.255		(Loopback)
			|| addrBytes.s_b1 == 0															// 0.0.0.0			- 0.255.255.255			(Current network)
			|| addrBytes.s_b1 == 100 && addrBytes.s_b2 >= 64 && addrBytes.s_b2 <= 127		// 100.64.0.0		- 100.127.255.255		(Shared address space)
			|| sockaddr_ipv4->sin_addr.S_un.S_addr == 0xFFFFFFFF							// 255.255.255.255							(Broadcast)
			|| addrBytes.s_b1 >= 224 && addrBytes.s_b2 <= 239								// 224.0.0.0		- 239.255.255.255		(Multicast)
			|| addrBytes.s_b1 == 233 && addrBytes.s_b2 == 252 && addrBytes.s_b3 == 0		// 233.252.0.0		- 233.252.0.255			(MCAST-TEST-NET)
			|| addrBytes.s_b1 > 240 && addrBytes.s_b4 <= 254)								// 240.0.0.0		- 255.255.255.254		(Future Use Class E)
		{
			goto CLEANUP;
		}
	}

	bAllowed = true;
	char resolvedStr[INET_ADDRSTRLEN];
	inet_ntop(AF_INET, &sockaddr_ipv4->sin_addr, resolvedStr, INET_ADDRSTRLEN);

	// Use CURL to change the host of the url while keeping anything like path.
	curl_url_set(url, CURLUPART_HOST, resolvedStr, 0);

	// Keep the request host name to be used as a Host: header.
	outHostHeader = urlHostname;

	curl_free(urlHostname);
	curl_url_get(url, CURLUPART_URL, &urlHostname, CURLU_DEFAULT_SCHEME);

	// Use the resolved address as the new request host.
	outResolvedHost = urlHostname;
	
	freeaddrinfo(result);

CLEANUP:

	curl_free(urlHostname);
	curl_free(urlScheme);
	curl_url_cleanup(url);
	return bAllowed;
}

size_t HttpCurlWriteToStringBufferCallback(char* contents, size_t size, size_t nmemb, void* userp)
{
	((std::string*)userp)->append((char*)contents, size * nmemb);
	return size * nmemb;
}

template <ScriptContext context>
int HttpRequestHandler::MakeHttpRequest(const HttpRequest& requestParameters)
{
	if (!IsRunning())
	{
		spdlog::warn("HttpRequestHandler::MakeHttpRequest was called while IsRunning() is false");
		return -1;
	}

	bool bAllowLocalHttp = Tier0::CommandLine()->FindParm("-allowlocalhttp");
	std::string resolvedHostName, hostHeaderOverride;

	if (!bAllowLocalHttp)
	{
		if (!IsDestinationHostAllowed(requestParameters.baseUrl, resolvedHostName, hostHeaderOverride))
		{
			spdlog::warn("HttpRequestHandler::MakeHttpRequest attempted to make a request to localhost. This is only allowed on servers "
						 "running with -allowlocalhttp.");
			return -1;
		}
	}

	// This handle will be returned to Squirrel so it can wait for the response and assign a callback for it.
	int handle = ++m_iLastRequestHandle;

	std::thread requestThread(
		[this, handle, requestParameters, bAllowLocalHttp, resolvedHostName, hostHeaderOverride]()
		{
			CURL* curl = curl_easy_init();
			if (!curl)
			{
				spdlog::error("HttpRequestHandler::MakeHttpRequest failed to init libcurl for request.");
				g_pSquirrel<context>->createMessage(
					"NSHandleFailedHttpRequest", handle, static_cast<int>(CURLE_FAILED_INIT), curl_easy_strerror(CURLE_FAILED_INIT));				
				return;
			}

			CURLoption curlMethod = CURLOPT_HTTPGET;
			switch (requestParameters.method)
			{
			case HttpRequestMethod::GET:
				curlMethod = CURLOPT_HTTPGET;
				break;
			case HttpRequestMethod::POST:
				curlMethod = CURLOPT_HTTPPOST;
				break;
			}

			std::string queryUrl = requestParameters.baseUrl;

			// Only resolve to IPv4 if we don't allow private network requests.
			// TODO: Also use resolved host ip as destination, and set hostname using Host: header.
			if (!bAllowLocalHttp)
			{
				curl_easy_setopt(curl, CURLOPT_RESOLVE, CURL_IPRESOLVE_V4);
				queryUrl = resolvedHostName;
			}

			// Ensure we only allow HTTP or HTTPS.
			curl_easy_setopt(curl, CURLOPT_PROTOCOLS, CURLPROTO_HTTP | CURLPROTO_HTTPS);
			curl_easy_setopt(curl, curlMethod, 1L);

			// Allow redirects
			curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);
			curl_easy_setopt(curl, CURLOPT_MAXREDIRS, 3L);

			// GET requests, or POST requests with an empty body, can have query parameters.
			// Append them to the base url.
			if (requestParameters.method == HttpRequestMethod::GET || requestParameters.body.empty())
			{
				int idx = 0;
				for (const auto& kv : requestParameters.queryParameters)
				{
					if (idx == 0)
					{
						queryUrl.append(fmt::format("?{}={}", kv.first, kv.second));
					}
					else
					{
						queryUrl.append(fmt::format("&{}={}", kv.first, kv.second));
					}
				}
			}
			else
			{
				// Grab the body and set it as a POST field
				curl_easy_setopt(curl, CURLOPT_POSTFIELDS, requestParameters.body);
			}

			curl_easy_setopt(curl, CURLOPT_URL, queryUrl.c_str());

			// If we're using a GET method, setup the write function so it can write into the body.
			std::string bodyBuffer;
			std::string headerBuffer;

			if (requestParameters.method == HttpRequestMethod::GET)
			{
				curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, HttpCurlWriteToStringBufferCallback);
				curl_easy_setopt(curl, CURLOPT_WRITEDATA, &bodyBuffer);
			}

			curl_easy_setopt(curl, CURLOPT_HEADERFUNCTION, HttpCurlWriteToStringBufferCallback);
			curl_easy_setopt(curl, CURLOPT_HEADERDATA, &headerBuffer);

			// Add all the headers for the request.
			struct curl_slist* headers = nullptr;
			curl_slist_append(headers, fmt::format("Content-Type: {}", requestParameters.contentType).c_str());
			for (const auto& kv : requestParameters.headers)
			{
				curl_slist_append(headers, fmt::format("{}: {}", kv.first, kv.second).c_str());
			}

			// Append the Host header if we private network requests are forbidden.
			if (!bAllowLocalHttp)
			{
				curl_slist_append(headers, fmt::format("Host: {}", hostHeaderOverride).c_str());
			}

			curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
			curl_easy_setopt(curl, CURLOPT_HEADER, 1L);

			// Enforce the Northstar user agent.
			curl_easy_setopt(curl, CURLOPT_USERAGENT, &NSUserAgent);

			CURLcode result = curl_easy_perform(curl);
			if (IsRunning())
			{
				if (result == CURLE_OK)
				{
					// While the curl request is OK, it could return a non success code.
					// Squirrel side will handle firing the correct callback.
					long httpCode = 0;
					curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &httpCode);
					// TODO: Send response over to Squirrel, yay.
					g_pSquirrel<context>->createMessage("NSHandleSuccessfulHttpRequest", handle, static_cast<int>(httpCode), bodyBuffer, headerBuffer);				
				}
				else
				{
					// Pass CURL result code & error.
					spdlog::error("curl_easy_perform() failed with code {}, error: {}", static_cast<int>(result), curl_easy_strerror(result));
					g_pSquirrel<context>->createMessage("NSHandleFailedHttpRequest", handle, static_cast<int>(result), curl_easy_strerror(result));			
				}
			}

			curl_easy_cleanup(curl);
			curl_slist_free_all(headers);
		});

	requestThread.detach();
	return handle;
}

template <ScriptContext context>
void HttpRequestHandler::RegisterSQFuncs()
{
	g_pSquirrel<context>->AddFuncRegistration
	(
		"int",
		"NS_InternalMakeHttpRequest",
		"int method, string baseUrl, table<string, string> headers, table<string, string> queryParams, string contentType, string body",
		"[Internal use only] Passes the HttpRequest struct fields to be reconstructed in native and used for an http request",
		SQ_InternalMakeHttpRequest<context>
	);
}

// int NS_InternalMakeHttpRequest(int method, string baseUrl, table<string, string> headers, table<string, string> queryParams, string contentType, string body)
template<ScriptContext context>
SQRESULT SQ_InternalMakeHttpRequest(HSquirrelVM* sqvm)
{
	if (!g_httpRequestHandler || !g_httpRequestHandler->IsRunning())
	{
		spdlog::warn("NS_InternalMakeHttpRequest called while the http request handler isn't running.");
		g_pSquirrel<context>->pushinteger(sqvm, -1);
		return SQRESULT_NOTNULL;
	}

	HttpRequest request;
	request.method = static_cast<HttpRequestMethod>(g_pSquirrel<context>->getinteger(sqvm, 1));
	request.baseUrl = g_pSquirrel<context>->getstring(sqvm, 2);

	SQTable* headerTable = sqvm->_stackOfCurrentFunction[3]._VAL.asTable;
	for (int idx = 0; idx < headerTable->_numOfNodes; ++idx)
	{
		tableNode* node = &headerTable->_nodes[idx];

		// TODO: Figure out why the first three nodes of this table are OT_NULL with a single value.
		// Then re-enable this if possible.
		//if (node->key._Type != OT_STRING || node->val._Type != OT_STRING)
		//{
		//	g_pSquirrel<context>->raiseerror(
		//		sqvm, fmt::format("Invalid header type or value (expected string, got {} = {})", SQTypeNameFromID(node->key._Type), SQTypeNameFromID(node->val._Type)).c_str());
		//	return SQRESULT_ERROR;
		//}

		if (node->key._Type == OT_STRING && node->val._Type == OT_STRING)
		{
			request.headers[node->key._VAL.asString->_val] = node->val._VAL.asString->_val;
		}
	}

	SQTable* queryTable = sqvm->_stackOfCurrentFunction[4]._VAL.asTable;
	for (int idx = 0; idx < queryTable->_numOfNodes; ++idx)
	{
		tableNode* node = &queryTable->_nodes[idx];
		// Same as above.
		//if (node->key._Type != OT_STRING || node->val._Type != OT_STRING)
		//{
		//	g_pSquirrel<context>->raiseerror(
		//		sqvm, fmt::format("Invalid query parameter type or value (expected string, got {} = {})", SQTypeNameFromID(node->key._Type), SQTypeNameFromID(node->val._Type)).c_str());
		//	return SQRESULT_ERROR;
		//}

		if (node->key._Type == OT_STRING && node->val._Type == OT_STRING)
		{
			request.queryParameters[node->key._VAL.asString->_val] = node->val._VAL.asString->_val;
		}
	}

	request.contentType = g_pSquirrel<context>->getstring(sqvm, 5);
	request.body = g_pSquirrel<context>->getstring(sqvm, 6);

	int handle = g_httpRequestHandler->MakeHttpRequest<context>(request);
	g_pSquirrel<context>->pushinteger(sqvm, handle);
	return SQRESULT_NOTNULL;
}

ON_DLL_LOAD_RELIESON("client.dll", HttpRequestHandler_ClientInit, ClientSquirrel, (CModule module))
{
	g_httpRequestHandler->RegisterSQFuncs<ScriptContext::CLIENT>();
}

ON_DLL_LOAD_RELIESON("server.dll", HttpRequestHandler_ServerInit, ServerSquirrel, (CModule module))
{
	g_httpRequestHandler->RegisterSQFuncs<ScriptContext::SERVER>();
}

ON_DLL_LOAD("engine.dll", HttpRequestHandler_Init, (CModule module))
{
	g_httpRequestHandler = new HttpRequestHandler;
	g_httpRequestHandler->StartHttpRequestHandler();
}
