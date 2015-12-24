#pragma once

#include "pin.H"
#include "Config.h"
#include "Debug.h"
#include "TimeTracker.h"
#include <time.h>
#include <unordered_set>
namespace W{
	#include "windows.h"
	#include <tlhelp32.h>
	#include "Winternl.h"
	#include "winnt.h"
	#include  "Intsafe.h"
}

#define MAX_STACK_SIZE 0x100000    //Used to define the memory range of the stack
#define TEB_SIZE 0xfe0 	
#define KUSER_SHARED_DATA_ADDRESS 0x7ffe0000
#define KUSER_SHARED_DATA_SIZE 0x3e0 

typedef struct PEB {
	W::BYTE padding1[2];
	W::BYTE BeingDebugged ;
	W::BYTE padding2[73];
	W::PVOID ReadOnlySharedMemoryBase;
	W::BYTE padding3[8];
	W::PVOID AnsiCodePageData;
	W::BYTE padding4[52];
	W::PVOID ProcessHeaps;
	W::PVOID GdiSharedHandleTable;
	W::BYTE padding5[352];
	W::PVOID ActivationContextData;
	W::BYTE padding6[4];
	W::PVOID SystemDefaultActivationContextData;
	W::BYTE padding7[52];
	W::PVOID pContextData;
	W::BYTE padding8[4];

}PEB;

struct MemoryRange{
	ADDRINT StartAddress;
	ADDRINT EndAddress;
};

//This struct will track the library loaded
//at program startup
struct LibraryItem{
	ADDRINT StartAddress;
	ADDRINT EndAddress;
	string name;
};

//memorize the PE section information
struct Section {
 ADDRINT begin;
 ADDRINT end;
 string name;
};

struct HeapZone {
	ADDRINT begin;
	ADDRINT end;
	UINT32 size;
};

class ProcInfo
{
public:
	//singleton instance
	static ProcInfo* getInstance();
	//distruptor
	~ProcInfo(void);

	/* getter */
	ADDRINT getFirstINSaddress();
	ADDRINT getPrevIp();
	std::vector<Section> getSections();
	float getInitialEntropy();
	BOOL getPushadFlag();
	BOOL getPopadFlag();
	string getProcName();
	clock_t getStartTimer();
	std::unordered_set<ADDRINT> getJmpBlacklist();
	ADDRINT getPINVMStart();
	ADDRINT getPINVMEnd();
	

	/* setter */
	void addProcAddresses();
	void setFirstINSaddress(ADDRINT address);
	void setPrevIp(ADDRINT ip);
	void setInitialEntropy(float Entropy);
	void setPushadFlag(BOOL flag);
	void setPopadFlag(BOOL flag);
	void setProcName(string name);
	void setStartTimer(clock_t t);
	void setMainIMGAddress(ADDRINT startAddress,ADDRINT endAddr);
	
	/* debug */
	void PrintStartContext();
	void PrintCurrContext();
	void PrintSections();


	/* helper */
	void insertSection(Section section);
	string getSectionNameByIp(ADDRINT ip);
	void insertHeapZone(HeapZone heap_zone);
	void deleteHeapZone(UINT32 index);
	void removeLastHeapZone();
	UINT32 searchHeapMap(ADDRINT ip);
	HeapZone *getHeapZoneByIndex(UINT32 index);
	float GetEntropy();
	void insertInJmpBlacklist(ADDRINT ip);
	BOOL isInsideJmpBlacklist(ADDRINT ip);
	BOOL isInsideMainIMG(ADDRINT address);
	BOOL isInterestingProcess(unsigned int pid);
	//PEB
	BOOL isPebAddress(ADDRINT addr);
	//TEB
	BOOL isTebAddress(ADDRINT addr);
	VOID addThreadTebAddress();
	//Stack
	BOOL isStackAddress(ADDRINT addr);
	VOID addThreadStackAddress(ADDRINT addr);
	//Memory Mapped Files
	BOOL isMappedFileAddress(ADDRINT addr);
	VOID addMappedFilesAddress(ADDRINT startAddr);
	//Library
	BOOL isLibraryInstruction(ADDRINT address);
	BOOL isKnownLibraryInstruction(ADDRINT address);
	VOID addLibrary(const string name,ADDRINT startAddr,ADDRINT endAddr);
	VOID addRtn(const string name,ADDRINT startAddr,ADDRINT endAddr);
	//Generic Address (pContexData, SharedMemory..)
	BOOL isGenericMemoryAddress(ADDRINT address);
	//Whitelist Memory
	BOOL isAddrInWhiteList(ADDRINT address);
	VOID enumerateWhiteListMemory();
	VOID enumerateCurrentMemory();
	VOID PrintCurrentMemorydAddr();
	VOID PrintWhiteListedAddr();
	VOID PrintDebugProcessAddr();
	VOID enumerateDebugProcessMemory();

	
	//Debug
	void printHeapList();
	void PrintAllMemory();
	

	
private:
	
	static ProcInfo* instance;
	ProcInfo::ProcInfo();
	ADDRINT first_instruction;
	ADDRINT prev_ip;
	

	std::vector<MemoryRange>  stacks;				//Set of Stack one for each thread
	MemoryRange mainImg;
	std::vector<MemoryRange> tebs;                                //Teb Base Address
	PEB *peb;
	std::vector<MemoryRange>  mappedFiles;
	std::vector<MemoryRange>  genericMemoryRanges;
	std::vector<MemoryRange>  whiteListMemory;
	std::vector<MemoryRange>  currentMemory;
	
	std::vector<Section> Sections;
	std::vector<HeapZone> HeapMap;
	std::unordered_set<ADDRINT> addr_jmp_blacklist;
	std::vector<LibraryItem> knownLibraries;			//vector of know library loaded
	std::vector<LibraryItem> unknownLibraries;			//vector of unknow library loaded	
	std::vector<LibraryItem> rtn_not_filtered;			//vector of routine not filtered ( as for example GetTickCount )
	float InitialEntropy;
	//track if we found a pushad followed by a popad
	//this is a common technique to restore the initial register status after the unpacking routine
	BOOL pushad_flag;
	BOOL popad_flag;
	string full_proc_name;
	string proc_name;
	clock_t start_timer;
	//processes to be monitored set 
	std::unordered_set<string> interresting_processes_name; 
	std::unordered_set<unsigned int> interresting_processes_pid;  

	void retrieveInterestingPidFromNames();
	
	//Enumerate Whitelisted Memory Helpers	
	//return the MemoryRange in which the address is mapped
	BOOL getMemoryRange(ADDRINT address, MemoryRange& range);
	VOID addWhitelistAddress(ADDRINT baseAddr,ADDRINT endAddress);
	VOID mergeMemoryAddresses();
	VOID mergeCurrentMemory();
	
	BOOL isKnownLibrary(const string name,ADDRINT startAddr,ADDRINT endAddr);
	VOID addProcessHeapsAddress();
	VOID addPebAddress();
	VOID addContextDataAddress();
	VOID addSharedMemoryAddress();
	VOID addCodePageDataAddress();
	VOID addKUserSharedDataAddress();


	//Enumerate current  Memory Helpers
	VOID addCurrentMemoryAddress(ADDRINT baseAddr,ADDRINT regionSize);

	//Enumerate Debug process Memory Helpers
	VOID addDebugProcessAddresses(ADDRINT baseAddr,ADDRINT regionSize);
	VOID enumerateProcessMemory(W::HANDLE hProc);


	
	//Library Helpers
	string libToString(LibraryItem lib);

	
	long long FindEx(W::HANDLE hProcess, W::LPVOID MemoryStart, W::DWORD MemorySize, W::LPVOID SearchPattern, W::DWORD PatternSize, W::LPBYTE WildCard);

};

