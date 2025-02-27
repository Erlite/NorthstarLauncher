#pragma once

void InitialiseCrashHandler();
void RemoveCrashHandler();

struct BacktraceModule
{
	std::string name;
	std::string relativeAddress;
	std::string address;
};

struct ExceptionLog
{
	std::string cause;
	HMODULE crashedModule;
	PEXCEPTION_RECORD exceptionRecord;
	PCONTEXT contextRecord;
	std::vector<BacktraceModule> trace;
	std::vector<std::string> registerDump;

	std::string runtimeInfo;

	int longestModuleNameLength;
	int longestRelativeAddressLength;
};
