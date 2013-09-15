#pragma once
#include <xtl.h>
#include <xboxmath.h>
#include <vector>
using namespace std;

typedef struct _AnsiString {
       USHORT Length;
       USHORT MaximumLength;
       PCHAR Buffer;
   } AnsiString;

extern "C" 
{
	//-------------------------------------------------------------------------------------
	// Name: ObCreateSymbolicLink
	// Desc: Creates a symbolic link for a device path
	//-------------------------------------------------------------------------------------
	DWORD		 WINAPI	ObCreateSymbolicLink(   AnsiString* LinkName,
												AnsiString* DevicePath);

	//-------------------------------------------------------------------------------------
	// Name: ObDeleteSymbolicLink
	// Desc: Destroys a linked path
	//-------------------------------------------------------------------------------------
	DWORD		 WINAPI	ObDeleteSymbolicLink(   AnsiString* LinkName);


	//-------------------------------------------------------------------------------------
	// Name: XexGetModuleHandle
	// Desc: Gets a HANDLE for a module already loaded in memory
	//-------------------------------------------------------------------------------------
	DWORD XBOXAPI WINAPI XexGetModuleHandle(	 const char* ModuleName,
												 HANDLE* outHandle );

	//-------------------------------------------------------------------------------------
	// Name: XexGetProcdureAddress
	// Desc: Gets the address to a function within a module already loaded in memory
	//-------------------------------------------------------------------------------------
	DWORD XBOXAPI WINAPI XexGetProcedureAddress( HANDLE Module,	
												 DWORD ordinal,
												 DWORD* OutAddress );
}
//-------------------------------------------------------------------------------------
// Name:  MountPartition
// Desc:  Mounts a partition, making it ready for reading/writing
// Notes: DevicePath parameter should contain preceding backslashes (e.g. \\Device\\Flash)
// and Link should not have trailing backslashes (e.g. flash:)
//-------------------------------------------------------------------------------------
DWORD MountPartition(char* DevicePath, char* Link)
{
	char mountName[16];										
	sprintf_s( mountName,"\\??\\%s", Link );

	AnsiString deviceOriginal =								// Original device path
								{	strlen(DevicePath),		// Length of string
									strlen(DevicePath) + 1,	// Length of string + 1 (null terminator)
									DevicePath };			// String

	AnsiString deviceMounted =								// Mounted link
								{	strlen(mountName),
									strlen(mountName) + 1,
									mountName };

	return ObCreateSymbolicLink(	&deviceMounted,
									&deviceOriginal );
}

//-------------------------------------------------------------------------------------
// Name: UnmountPartition
// Desc: Unmounts a linked partition
//-------------------------------------------------------------------------------------
DWORD UnmountPartition(char* link)
{
	char mountName[16];
	sprintf_s( mountName,"\\??\\%s", link );
	
	AnsiString deviceMounted =								// Mounted link
								{	strlen(mountName),		// Length of string
									strlen(mountName) + 1,	// Length of string + 1 (null terminator)
									mountName };			// String

	return ObDeleteSymbolicLink(&deviceMounted);
}

//-------------------------------------------------------------------------------------
// Name: XamLoaderLaunchTitleEx
// Desc: Launches a new executable, exiting the current one
//-------------------------------------------------------------------------------------
DWORD (*XamLoaderLaunchTitleEx)( const char* XexPath,
							     const char* XexFolder,
							     const char* idk,
							     unsigned __int32 idklol );

//-------------------------------------------------------------------------------------
// Name: InitializeXamLoaderLaunchTitleEx
// Desc: Loads XamLoaderLauncTitleEx from memory, making it function ready to use
//-------------------------------------------------------------------------------------
void InitXamLoaderLaunchTitleEx( void )
{
	DWORD outPointer;										// Address of the function
	HANDLE XamHandle;										// HANDLE for XAM
	XexGetModuleHandle("xam.xex", &XamHandle);				// Get XAM's handle

	XexGetProcedureAddress(XamHandle, 0x1A5, &outPointer);	// Get the procedure address for XamLoaderLaunchTitleEx

	XamLoaderLaunchTitleEx = (DWORD (__cdecl *)(			// Setup XamLoaderLaunchTitleEx so when we call it, it will directly call the XAM function
												const char*,
												const char*,
												const char*,
												unsigned __int32))outPointer;
	return;
}

DWORD (*DmMountFdfxVolume)     ( const char* FilePath,
								 const char* LinkName );

//-------------------------------------------------------------------------------------
// Name: InitializeDmMountFdfxVolume
// Desc: Loads DmMountFdfxVolume from memory, making it function ready to use
//-------------------------------------------------------------------------------------
void InitDmMountFdfxVolume( void )
{
	DWORD outPointer;										// Address of the function
	HANDLE XbdmHandle;										// HANDLE for XBDM
	XexGetModuleHandle("xbdm.xex", &XbdmHandle);			// Get XBDM's handle

	XexGetProcedureAddress(XbdmHandle, 0x82, &outPointer);	// Get the procedure address for DmMountFdfxVolume

	DmMountFdfxVolume = (DWORD (__cdecl *)(					// Setup DmMountFdfxVolume so when we call it, it will directly call the XAM function
												char const *,
												char const *))outPointer;
	return;
}