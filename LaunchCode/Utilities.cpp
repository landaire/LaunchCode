#include "stdafx.h"
#include "Utilities.h"
#include "smc.h"
#include <fstream>
#include <xtl.h>
#include "clk.h"
#define VERSION 1.0f

Utilities::Utilities(void) : LoadImmediate(0x38600001), nop(0x60000000), GameDirMounted(false), LauncherDataMounted(false)
{
	ConsolePrint("LaunchCode", "Welcome to LaunchCode.  Made by CLK.  Version: %.2f", VERSION);
	// XboxHardwareInfo
	// Get all of our addresses and whatnot
	XexGetModuleHandle("xboxkrnl.exe", &KernelHandle);
	XexGetModuleHandle("xbdm.xex", &XbdmHandle);
	XexGetModuleHandle("xam.xex", &XamHandle);

	XexGetProcedureAddress(KernelHandle, 599, &ConVerify);
	XexGetProcedureAddress(KernelHandle, 600, &RsaAddress);
	XexGetProcedureAddress(KernelHandle, 344, &KernelVersionStruct);
	XexGetProcedureAddress(KernelHandle, 431, &LoadedImageName); // ExLoadedImageName
	XexGetProcedureAddress(KernelHandle, 365, &RsaPubCryptAddr); // XeCryptBnQwNeRsaPubCrypt
	
	XexGetProcedureAddress(XamHandle, 495, &ServerToClientAddr);
	XexGetProcedureAddress(XamHandle, 1253, &XamFindOrCreateInternalPassportAccountAddr);
	XexGetProcedureAddress(XamHandle, 490, &XamAllocAddr);
	XexGetProcedureAddress(XamHandle, 524, &XamUserGetGamertagAddr);
	XexGetProcedureAddress(XamHandle, 526, &XamUserGetNameAddr);
	XexGetProcedureAddress(XamHandle, 551, &XamUserGetSigninInfoAddr);
	XexGetProcedureAddress(XamHandle, 539, &XamUserGetMembershipTypeAddr);
	XexGetProcedureAddress(XamHandle, 535, &XamUserGetMembershipTypeFromXUIDAddr);
	XexGetProcedureAddress(XamHandle, 1205, &XamUserOverrideUserInfoAddr);

	// DmStartFileEventCapture ordinal, which isn't used.
	XexGetProcedureAddress(XbdmHandle, 152, &MapInternalDrivesAddr);
	MapInternalDrivesAddr -= 0x330;

	MapDebugDriveAddr = MapInternalDrivesAddr - 0x198;

	MapInternalDrives = (DWORD (__cdecl*)(void))MapInternalDrivesAddr;
	MapDebugDrive = (DWORD (__cdecl*)( const char*, const char*, bool )) MapDebugDriveAddr;
	

	// Mount the flash partition
	ConsolePrint("LaunchCode", "Mounting flash partition...");
	DWORD Result = MountPartition("\\Device\\Flash", "flash:");
}


Utilities::~Utilities(void)
{
	ConsolePrint("LaunchCode", "Cleaning up...");
	// Unmount the flash partition, yolo
	UnmountPartition("flash:");
	if (GameDirMounted)
		UnmountPartition("game:");
	if (LauncherDataMounted)
		UnmountPartition("LauncherData:");
	CloseHandle(XamHandle);
	CloseHandle(KernelHandle);
	CloseHandle(XbdmHandle);
}

void Utilities::EnableApplicationStartup( void )
{
	// Make sure that this executable is in dashboard.xbx
	FILE *f = fopen("flash:\\dashboard.xbx", "wb");
	if (f)
	{
		BYTE Buffer[0x100] = {0};
		sprintf((char*)&Buffer, (const char*)LoadedImageName);
		fwrite(Buffer, sizeof(char), 0x100, f);
		fclose(f);
	}
	else
		ConsolePrint("LaunchCode", "Couldn't open startup file...");
}

void Utilities::EnableHiddenDrives( void )
{
// #ifndef _NIB_
	// Make sure that we can see hidden partitions
	ConsolePrint("LaunchCode", "Mounting flash partition...");
	DWORD Result = MountPartition("\\Device\\Flash", "flash:");
	FILE *f = 0;
	f = fopen("flash:\\recint.ini", "wb");
	if (f)
	{
		ConsolePrint("LaunchCode", "Enabling hidden partitions...");
		char Buffer[] = {"[xbdm]\r\ndrivemap internal=1\r\n"};
		fwrite(Buffer, sizeof(char), strlen(Buffer), f);
		fclose(f);
		f = 0;
	}
	else
		ConsolePrint("LaunchCode", "Couldn't open partition file...");
// #else
// 	MapInternalDrives();
// #endif
}

void Utilities::SetGold( void )
{
	ConsolePrint("LaunchCode", "Setting gold membership");
	DWORD li = LoadImmediate;
	li += 5;
	SetMemory((LPVOID)(XamUserGetMembershipTypeAddr + 0x34), &li, 4);
	SetMemory((LPVOID)(XamUserGetMembershipTypeFromXUIDAddr + 0x38), &li, 4);
}

void Utilities::SetGamertag( const std::string &gamertag )
{
	ConsolePrint("LaunchCode", "Setting gamertag...");
	if (gamertag.size() > 15)
		ConsolePrint("LaunchCode", "Not setting gamertag -- length is longer than 15 characters");
	// Get the address to some random error string which fits our purposes.
	DWORD FreeStringAddr = 0, AsciiAddress = 0;
	XexGetProcedureAddress(XamHandle, 508, &FreeStringAddr);
	FreeStringAddr = ((((DWORD)(*(USHORT*)(FreeStringAddr + 0xE4 + 2))) << 16) + *((USHORT*)(FreeStringAddr + 0xF0 + 2)));
	AsciiAddress = FreeStringAddr + (16 * 2);

	// The address to where we start patching
	DWORD Address = XamUserGetGamertagAddr + 0x48, XUGNAddr = XamUserGetNameAddr + 0x24;
	USHORT GamerTagHigh = FreeStringAddr >> 16;
	USHORT GamerTagLow = (USHORT)FreeStringAddr;

	USHORT AsciiHigh = AsciiAddress >> 16;
	USHORT AsciiLow = (USHORT)AsciiAddress;

	// Patch where r10 is loaded as the offset for the profile data
	DWORD r10lis = 0x3D40;
	r10lis <<= 16;
	r10lis += (USHORT)GamerTagHigh;
	SetMemory((LPVOID)Address, &r10lis, 4);
	SetMemory((LPVOID)XUGNAddr, &r10lis, 4);

	// Patch where r11 is the multiplier for the user index
	Address += 4;
	XUGNAddr += 4;
	DWORD r11lis = 0x3D600000;
	SetMemory((LPVOID)Address, &r11lis, 4);
	SetMemory((LPVOID)XUGNAddr, &r11lis, 4);

	// Patch where r10 is added to form the full address
	Address += 4;
	XUGNAddr += 4;
	DWORD r10addi = 0x394A0000 | GamerTagLow;
	SetMemory((LPVOID)Address, &r10addi, 4);
	SetMemory((LPVOID)XUGNAddr, &r10addi, 4);
	
	// Nop where r10 is r10 + 0x1C
	Address += 8;
	XUGNAddr += 8;
	SetMemory((LPVOID)Address, &nop, 4);
	SetMemory((LPVOID)XUGNAddr, &nop, 4);

	// Start patching XamGetCachedGamertagW
	DWORD XamGetCachedGamertagW = 0;
	XexGetProcedureAddress(XamHandle, 697, &XamGetCachedGamertagW);
	DWORD r4lis = 0x3C800000 | AsciiHigh, r4addi = 0x38840000 | AsciiLow;
	//SetMemory((LPVOID)(XamGetCachedGamertagW + 0x7C), &r4lis, 4);
	//SetMemory((LPVOID)(XamGetCachedGamertagW + 0x88), &r4addi, 4);
	r4lis = 0x3C800000 | GamerTagHigh;
	r4addi = 0x38840000 | GamerTagLow;
	SetMemory((LPVOID)(XamUserGetSigninInfoAddr + 0x258), &nop, 4);
	SetMemory((LPVOID)(XamUserGetSigninInfoAddr + 0x25C), &r4lis, 4);
	SetMemory((LPVOID)(XamUserGetSigninInfoAddr + 0x274), &r4addi, 4);

	// Signs user out when switching XEXs
	wchar_t *Gamertag = (wchar_t*)FreeStringAddr;
	wchar_t Buffer[16] = {0};
	mbstowcs(Buffer, gamertag.c_str(), 15);
	SetMemory(Gamertag, (LPCVOID)Buffer, 16 * 2);

	XamUserOverrideUserInfo = (DWORD (__cdecl*)(LPCVOID, unsigned long ))XamUserOverrideUserInfoAddr;
	DWORD ProfileStructureAddr = ((((DWORD)(*(USHORT*)(XamUserOverrideUserInfoAddr + 0x14 + 2))) << 16) + *((SHORT*)(XamUserOverrideUserInfoAddr + 0x1E)));
	BYTE ProfileStructure[0x2B0] = {0};
	GetMemory((LPVOID)ProfileStructureAddr, ProfileStructure, 0x2B0);
	SetMemory((LPVOID)(&ProfileStructure[0x1C]), Buffer, 16);
	SetMemory((LPVOID)ProfileStructureAddr, Buffer, 0x2B0);

	char AsciiBuffer[16] = {0};
	char* GamertagAscii = (char*)(FreeStringAddr + (16 * 2));
	sprintf(AsciiBuffer, gamertag.c_str());
	SetMemory(GamertagAscii, AsciiBuffer, 16);

	DWORD XamUserSetLoggedOnUsersAddr = 0;
	XexGetProcedureAddress(XamHandle, 1253, &XamUserSetLoggedOnUsersAddr);
	XamUserSetLoggedOnUsersAddr += 0x978;
	DWORD xuaddr = XamUserSetLoggedOnUsersAddr + 0x1E8;
	DWORD Instructions[6] =
	{
		0x7fc3f378,
		0x3c800000 | GamerTagHigh,
		0x38840000 | GamerTagLow,
		0x38a00020,
		0x60000000,
		0x60000000
	};
	// SetMemory((LPVOID)xuaddr, Instructions, 24);
}

void Utilities::SetDefaultAccountPassword( const std::string &password )
{
	ConsolePrint("LaunchCode", "Setting default password...");
	if (XamFindOrCreateInternalPassportAccountAddr == 0)
	{
		ConsolePrint("LaunchCode", "Couldn't get password address; kernel may be unsupported...");
		return;
	}
	if (password.size() > 11)
	{
		ConsolePrint("LaunchCode", "Password length is greater than 11 characters");
		return;
	}
	DWORD LoadAddress = XamFindOrCreateInternalPassportAccountAddr + 0x10C;
	DWORD supersecret = ((DWORD)(*(USHORT*)(LoadAddress + 2)) << 16);
	supersecret |= (*((USHORT*)(LoadAddress + 10)));
	SetMemory((LPVOID)supersecret, (LPCVOID)password.c_str(), password.size() + 1);
}

// THIS METHOD IS HARDCODED FOR RECOVERY
void Utilities::PatchControllerDisconnect( void )
{
	ConsolePrint("LaunchCode", "Patching controller authentication...");
	int Branch = 0x4800008C;
	SetMemory((LPVOID)0x80127D04, (LPCVOID)&Branch, 4);
}

void Utilities::LaunchXshell( void )
{
	ConsolePrint("LaunchCode", "Patches complete.  Launching xshell...");
	// Initiate XamLoaderLaunchTitleEx so that we can launch xshell
	InitXamLoaderLaunchTitleEx();
	XamLoaderLaunchTitleEx("\\Device\\Flash\\xshell.xex", "\\Device\\Fash", NULL, NULL);
}

void Utilities::MountGameDirectory( void )
{
	std::string XexPath((const char*)LoadedImageName);
	XexPath = XexPath.substr(0, XexPath.find_last_of("\\"));
	GameDirMounted = !MountPartition(XexPath.c_str(), "g:");
}

void Utilities::PatchRsaPubCrypt( void )
{
	ConsolePrint("LaunchCode", "Patching XeCrypt RSA Public Crypt...");
	SetMemory((LPVOID)(RsaPubCryptAddr + 0x70), (LPCVOID)&nop, 4);
	SetMemory((LPVOID)(RsaPubCryptAddr + 0x74), (LPCVOID)&LoadImmediate, 4);
}

void Utilities::RemoveConsoleSignatureChecks( void )
{
	/* Patch XeKeysConsoleSignatureVerification
	 * 0x801267CC - li
	 * 0x801267B0 - li, nop
	 */
	ConsolePrint("LaunchCode", "Patching Console Signature Verification...");
	SetMemory((LPVOID)(ConVerify + 0x174), (LPCVOID*)&LoadImmediate, 4);
	SetMemory((LPVOID)(ConVerify + 0x158), (LPCVOID*)&LoadImmediate, 4);
	SetMemory((LPVOID)(ConVerify + 0x15C), (LPCVOID*)&nop, 4);
}

void Utilities::RemovePackageRsaChecks( void )
{
	/* Patch XeKeysVerifyRSASignature
	 * 0x801248A8 - li
	 */
	ConsolePrint("LaunchCode", "Patching Verify RSA Signature...");
	SetMemory((LPVOID)(RsaAddress + 0xB4), (LPCVOID*)&LoadImmediate, 4);
}

// some profiles trigger a twi for some reason, this removes that twi
void Utilities::RemoveProfileBreakpoints( void )
{
	// Get the kernel/xdk version
	KernelVersion *kv = (KernelVersion*)KernelVersionStruct;
	char FlashVersion[10] = {0};
	sprintf(FlashVersion, "%d.%d", kv->FlashMajor, kv->FlashMinor & 0x7FFF);

	/* Patch twis in TitleList::ConvertServerToClient to prevent console freezing when going to fucked up profiles */
	if (!memcmp(FlashVersion, "15574.0", strlen(FlashVersion)) || (*((DWORD*)ServerToClientAddr - 0x21B94) == 0x0FE00019))
	{
		ConsolePrint("LaunchCode", "Patching profile breakpoints...");
		SetMemory((LPVOID)(ServerToClientAddr - 0x21B94), (LPCVOID*)&nop, 4);
		SetMemory((LPVOID)(ServerToClientAddr - 0x21B80), (LPCVOID*)&nop, 4);
	}
	else
		ConsolePrint("LaunchCode", "Didn't patch profile breakpoints -- kernel %s is unsupported...", FlashVersion);
}


// Removes some breakpoints which may randomly be hit and can be pretty much ignored
// this is very hacky.
void Utilities::RemoveXampHeapAllocExplicitBreakpoints( void )
{
	ConsolePrint("LaunchCode", "Removing XampHeapAllocExplicit breakpoints...");
	DWORD XamAllocHeapAddr = (XamAllocAddr + 0x14) + *(USHORT*)(XamAllocAddr + 0x16);
	DWORD XamAllocHeapExplicitAddr = (XamAllocHeapAddr + 0x68) + *(USHORT*)(XamAllocHeapAddr + 0x6A) - 1;
	DWORD CXamInstrumentedHeapAloc = (XamAllocHeapExplicitAddr + 0xC4) + *(USHORT*)(XamAllocHeapExplicitAddr + 0xC6) - 1;
	DWORD CXamInstrumentedHeapPrintHeapLowWarning = (CXamInstrumentedHeapAloc + 0xAC) + *(USHORT*)(CXamInstrumentedHeapAloc + 0xAE) - 1;

	std::vector<DWORD> NopAddresses;
	if (*((DWORD*)(XamAllocHeapExplicitAddr + 0x20)) == 0x0FE00019 || *((DWORD*)(XamAllocHeapExplicitAddr + 0x20)) == 0x0FE00016)
	{
		NopAddresses.push_back(XamAllocHeapExplicitAddr + 0x20);
		NopAddresses.push_back(XamAllocHeapExplicitAddr + 0x80);
		NopAddresses.push_back(XamAllocHeapExplicitAddr + 0x134);
		NopAddresses.push_back(XamAllocHeapExplicitAddr + 0x160);
		NopAddresses.push_back(XamAllocHeapExplicitAddr + 0x16C);
	}
	else
		ConsolePrint("LaunchCode", "Not patching XamAllocHeapExplicit breakpoints -- invalid address...");
	if (*((DWORD*)(CXamInstrumentedHeapPrintHeapLowWarning + 0x6C)) == 0x0FE00019 || *((DWORD*)(CXamInstrumentedHeapPrintHeapLowWarning + 0x6C)) == 0x0FE00016)
	{
		NopAddresses.push_back(CXamInstrumentedHeapPrintHeapLowWarning + 0x6C);
	}
	else
		ConsolePrint("LaunchCode", "Not patching InstrumentedHeapPrintHeapLowWarning breakpoints -- invalid address...");

	for (int i = 0; i < NopAddresses.size(); i++)
		SetMemory((LPVOID)NopAddresses[i], &nop, 4);
}

void Utilities::SetLeds( LEDState sTopLeft, LEDState sTopRight, LEDState sBottomLeft, LEDState sBottomRight )
{
	ConsolePrint("LaunchCode", "Setting LEDs...");
	smc s;
	s.SetLEDS(sTopLeft, sTopRight, sBottomLeft, sBottomRight);
	s.SetPowerLED(POWER_LED_DEFAULT, false);
}


/* DriveNameConversionTable format: */

struct DriveName
{
	char* PathAddress; // 0x0 (pointer to a location in memory where the string is located)
	DWORD SomeValue; // 0x4 (no idea what this is.  either 0xE or 0x8)
	char NeighborhoodName[0x24]; // 0x8 (this is what shows as the name in neighborhood - 0x23 (not incl. null terminator) in length) 
	// Total size: 0x2C
};

void Utilities::AddExtraDrives( const std::vector<Drive> &Drives )
{
	//DebugBreak();
	ConsolePrint("LaunchCode", "Adding extra Neighborhood drives...");
	DWORD BaseAddr = MapInternalDrivesAddr + 0x1EC;
	DWORD DriveConversionTable = ((DWORD)*((USHORT*)(MapDebugDriveAddr + 0x86)) << 16) | *((USHORT*)(MapDebugDriveAddr + 0x8A)) + 0x630;
	DWORD DrivePathTable = ((DWORD)*((USHORT*)(MapInternalDrivesAddr + 0x1F2)) << 16) | *((USHORT*)(MapInternalDrivesAddr + 0x1F6));

	std::vector<char*> NoMemLeaks;
	// Nop out a branch that fails in MapDebugDrive (compares addresses to see if we're sneaking in more drives)
	//DmSetMemory((LPVOID)(MapDebugDriveAddr + 0xE4), 4, (LPCVOID)&nop, NULL);

	if (*((DWORD*)(BaseAddr)) == nop)
	{
		ConsolePrint("LaunchCode", "WARNING: LaunchCode already executed.  Extra drives will not be mounted");
		return;
	}

	char OldInstructions[0x60];
	GetMemory((LPCVOID)BaseAddr, OldInstructions, 0x60);

	// Starting from the base address, let's just make all devices show in Neighborhood
	for (int i = MapInternalDrivesAddr + 0x1C; i < MapInternalDrivesAddr + 0x2A4; i += 0x18)
	{
		USHORT Value = 1;
		DmSetMemory((LPVOID)(i + 2), 2, &Value, NULL);
	}

	// Nop out the DVD drive/transfer cable shit to make room for our strings (don't even try loading those ones)
	for (; BaseAddr < MapInternalDrivesAddr + 0x24C; BaseAddr += 0x4)
		SetMemory((LPVOID)(BaseAddr), (LPCVOID)&nop, 4);

	for (int i = 0; i < Drives.size(); i++, BaseAddr += 24)
	{
		// Check to make sure all of the paths are of proper length
		if (Drives[i].NeighborhoodName.size() > 0x23)
		{
			ConsolePrint("LaunchCode", "WARNING: Neighborhood name \"%s\" is longer than 35 characters, and will not be mounted!", Drives[i].NeighborhoodName.c_str());
			continue;
		}
		if (Drives[i].Path.size() > 0x3D)
		{
			ConsolePrint("LaunchCode", "WARNING: Device path \"%s\" is longer than 61 characters, and will not be mounted!", Drives[i].Path.c_str());
			continue;
		}
		if (!Drives[i].Path.compare(""))
		{
			continue;
		}

		char *MBuffer = new char[0x24];
		memset(MBuffer, 0, 0x24);
		char *PBuffer = new char[0x3E];
		memset(PBuffer, 0, 0x3E);

		DWORD PathPointer = (DrivePathTable + (i * 0x3E));

		NoMemLeaks.push_back(MBuffer);
		NoMemLeaks.push_back(PBuffer);

		sprintf(MBuffer, Drives[i].NeighborhoodName.c_str());
		sprintf(PBuffer, Drives[i].Path.c_str());
		USHORT MountAddrHigh = (((DWORD)MBuffer) & 0xFFFF0000) >> 16;
		USHORT MountAddrLow = ((DWORD)MBuffer) & 0x0000FFFF;
		USHORT PathAddrHigh = (((DWORD)PathPointer) & 0xFFFF0000) >> 16;
		USHORT PathAddrLow = ((DWORD)PathPointer) & 0x0000FFFF;
		USHORT One = 1;
		// Set r11 to load the path's high address
		SetMemory((LPVOID)(BaseAddr + 2), (LPCVOID)&PathAddrHigh, 2);
		// Set r10 to load the mount point's high address
		SetMemory((LPVOID)(BaseAddr + 6), (LPCVOID)&MountAddrHigh, 2);
		// Set r4 to r11 + path low
		SetMemory((LPVOID)(BaseAddr + 10), (LPCVOID)&PathAddrLow, 2);
		// Set r3 to r10 + mount low
		SetMemory((LPVOID)(BaseAddr + 14), (LPCVOID)&MountAddrLow, 2);
		// Set r5 to load 1 (to show in neighborhood)
		SetMemory((LPVOID)(BaseAddr + 18), (LPCVOID)&One, 2);

		//DebugBreak();

		DriveName d = {0};
		sprintf(d.NeighborhoodName, Drives[i].NeighborhoodName.c_str());
		// Set the path in the path table
		SetMemory((LPVOID)(DrivePathTable + (i * 0x3E)), (LPCVOID)PBuffer, 0x3E);
		SetMemory((LPVOID)(DriveConversionTable + i * 0x2C), (LPCVOID)&PathPointer, 0x4);
		// Set the shit to display in neighborhood
		SetMemory((LPVOID)((DriveConversionTable + i * 0x2C) + 0x8), (LPCVOID)d.NeighborhoodName, 0x24);
	}

	MapInternalDrives();

	// Copy back the old instructions
	SetMemory((LPVOID)(MapInternalDrivesAddr + 0x1EC), OldInstructions, 0x60);

	for (int i = 0; i < NoMemLeaks.size(); i++)
		delete NoMemLeaks[i];
}



/* It was decided that you simply cannot have write access to this file
 * and therefore the function was scrapped. I wanted to retain the code,
 * so here are its remnants in case I ever want it in the future
 */

//void Utilities::OverwriteDevKitBackground ( std::string FileName )
//{
//	DebugBreak();
//	ConsolePrint("LaunchCode", "Overwriting devkit background with file \"%s\"", FileName.c_str());
//	if (!LauncherDataMounted)
//	{
//		InitDmMountFdfxVolume();
//		AnsiString xlaunch = CreateAnsiString("\\Device\\Flash\\xlaunch.fdf");
//		AnsiString launcherdata = CreateAnsiString("\\Device\\LauncherData");
//		DmMountFdfxVolume(&xlaunch, &launcherdata, 0x1000000);
//		LauncherDataMounted = true;
//	}
//	if (MountPartition("\\Device\\LauncherData", "ld:"))
//		ConsolePrint("LaunchCode", "Could not mount launcher data...");
//	
//	BYTE *Buffer = nullptr;
//	std::string path("g:\\");
//	path += FileName;
//	BOOL Succeeded = CopyFileA("ld:\\images\\bg_image_blue.png", "g:\\bg_image_blue_backup.png", true);
//	std::fstream output("ld:\\images\\bg_image_gray.png", std::ios::out);
//	std::fstream input(path.c_str(), std::ios::in);
//
//	if (!output.good() || !input.good())
//	{
//		ConsolePrint("LaunchCode", "Could not open file...");
//		return;
//	}
//
//	output.seekg(0, std::ios::beg);
//	output << input.rdbuf();
//	output.close();
//	input.close();
//}
//
//void Utilities::OverwriteTestKitBackground ( std::string FileName )
//{
//	DebugBreak();
//	ConsolePrint("LaunchCode", "Overwriting testkit background with file \"%s\"", FileName.c_str());
//	if (!LauncherDataMounted)
//	{
//		InitDmMountFdfxVolume();
//		AnsiString xlaunch = CreateAnsiString("\\Device\\Flash\\xlaunch.fdf");
//		AnsiString launcherdata = CreateAnsiString("\\Device\\LauncherData");
//		DmMountFdfxVolume(&xlaunch, &launcherdata, 0x1000000);
//		LauncherDataMounted = true;
//	}
//	if (MountPartition("\\Device\\LauncherData", "ld:"))
//		ConsolePrint("LaunchCode", "Could not mount launcher data...");
//	
//	BYTE *Buffer = nullptr;
//	std::string path("g:\\");
//	path += FileName;
//	std::fstream output("ld:\\images\bg_image_gray.png", std::ios::in | std::ios::out | std::ios::trunc);
//	std::fstream input(path.c_str(), std::ios::in);;
//	std::fstream backup("g:\\bg_image_gray_backup.png", std::ios::out | std::ios::trunc);
//	if (!output.good() || !input.good() || !backup.good())
//	{
//		ConsolePrint("LaunchCode", "Could not open file...");
//		return;
//	}
//	//input.seekg(0, std::ios::end);
//	//DWORD Size = input.tellg();
//	//Buffer = new BYTE[Size];
//	//input.seekg(0, std::ios::beg);
//
//	//input.read((char*)&Buffer, Size);
//	backup << output.rdbuf();
//	output.seekg(0, std::ios::beg);
//	output << input.rdbuf();
//	output.close();
//	input.close();
//	backup.close();
//}