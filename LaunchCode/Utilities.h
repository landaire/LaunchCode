#pragma once
//#define _NIB_ 1
#include "stdafx.h"
#include <vector>
#include "smc.h"

class Utilities
{

	struct KernelVersion
	{
		SHORT Unk, Unk2, FlashMajor, FlashMinor, XdkMajor, XdkMinor;
	};

private:
	// The following are set on the class initialization, as the addresses vary from kernel versions
	DWORD
		RsaPubCryptAddr,
		RsaAddress,
		ConVerify,
		KernelVersionStruct,
		LoadedImageName,
		XamAllocAddr,
		ServerToClientAddr,
		XamFindOrCreateInternalPassportAccountAddr,
		MapInternalDrivesAddr,
		MapDebugDriveAddr,
		XamUserOverrideUserInfoAddr,
		XamUserGetGamertagAddr,
		XamUserGetNameAddr,
		XamUserGetSigninInfoAddr,
		XamUserGetMembershipTypeAddr,
		XamUserGetMembershipTypeFromXUIDAddr;
	HANDLE KernelHandle, XamHandle, XbdmHandle;

	DWORD LoadImmediate; // li r3, 1
	DWORD nop;
	bool GameDirMounted, LauncherDataMounted;

public:
	struct Drive
	{
		std::string NeighborhoodName;
		std::string Path;
	};

	Utilities(void);
	~Utilities(void);

	DWORD (*MapInternalDrives)( void );
	DWORD (*MapDebugDrive)( const char* MountPoint, const char* DevicePath, bool ShowInNeighborhood );

	void PatchRsaPubCrypt				( void );
	void RemovePackageRsaChecks			( void );
	void RemoveConsoleSignatureChecks	( void );
	void SetLeds						( LEDState sTopLeft, LEDState sTopRight, LEDState sBottomLeft, LEDState sBottomRight );
	void RemoveProfileBreakpoints		( void );
	void RemoveXampHeapAllocExplicitBreakpoints( void );
	void EnableHiddenDrives				( void );
	void EnableApplicationStartup		( void );
	void AddExtraDrives					( const std::vector<Drive> &Drives );
	void PatchControllerDisconnect		( void );

	void SetGamertag					( const std::string &gamertag );
	void SetGold						( void );

	void MountGameDirectory				( void );
	void LaunchXshell					( void );
	void SetDefaultAccountPassword		( const std::string &password );
	DWORD (*XamUserOverrideUserInfo)	(LPCVOID structure, unsigned long index);
	//void OverwriteDevKitBackground		( std::string FileName );
	//void OverwriteTestKitBackground		( std::string FileName );
};

