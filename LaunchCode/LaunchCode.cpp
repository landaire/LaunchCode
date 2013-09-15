// ChangeLeds.cpp : Defines the entry point for the application.
//

#include "stdafx.h"
#include "Utilities.h"
#include "SimpleIni.h"
#include <fstream>
#include "smc.h"

struct NetworkSettings
{
	std::string AccountCreationPassword, Gamertag;
};

//-------------------------------------------------------------------------------------
// Name: main()
// Desc: The application's entry point
//-------------------------------------------------------------------------------------
void __cdecl main()
{
	CSimpleIniA ini(true, true, true);
	std::map<std::string, bool> Settings;

	Utilities u;
	// Try mounting the game directory; at console startup it's not mounted
	u.MountGameDirectory();
	if (ini.LoadFile("g:\\settings.ini") < 0)
	{
		ini.Reset();

		std::string IniText(
			" LaunchCode was created by CLK.\n\n"			\
			" What can this app do?\n"									\
			"    - Remove console signature checks\n"					\
			"    - Remove RSA signature checks\n"						\
			"    - Set RoL LEDs\n"										\
			"    - Remove profile breakpoints (freezing when viewing some profiles)\n"\
			"    - Remove XampHeapAllocExplicit breakpoints (some bullshit)\n"	\
			"    - Enable hidden partitions for Xbox 360 Neighborhood\n"		\
			"    - Add additional, customizable partitions to Neighborhood\n"	\
			"    - Set the default account password for PartnerNet accounts\n"	\
			"    - Set the display gamertag\n"							\
			"    - Make the console think you have a gold membership\n" \
			" -> Instructions/Limitations:\n"							\
			" - Account password length must not exceed 11 characters.\n"\
			" - Each Neighborhood partition should match the following"	\
			    "criteria, otherwise it will not be mounted:\n"			\
			" ~ NeighborhoodName must not exceed 35 characters\n"		\
			" ~ DevicePath must not exceed 61 characters\n"				\
			" ~ DevicePath must be formatted as so:\n"					\
			"  - \"\\Device\\Harddisk0\\Partition1\\Devikit\"\n"		\
			"    - The following are all valid basic paths:\n"			\
			"      - \\Device\n"										\
			"      - \\Device\\Harddisk0\\SystemPartition\n"			\
			"      - \\Device\\Harddisk0\\Partition1\n"					\
			"      - \\Device\\Flash\n"									\
			"      - \\Device\\Mu0System\n"								\
			"      - \\Device\\Mu1System\n"								\
			"      - \\Device\\Mass0\n"									\
			"      - \\Device\\Mass1\n"									\
			"      - \\Device\\Mass2\n"									\
			"      - \\Device\\BuiltInMuSfcSystem\n"					\
			"      - \\Device\\Harddisk\\SystemExtPartition\n"			\
			"      - \\Device\\Harddisk\\SystemAuxPartition\n"			\
			"      - \\Device\\BuiltInMuUsb\\Storage\n"					\
			"      - \\Device\\BuiltInMuUsb\\StorageSystem\n"			\
			"      - \\Device\\BuiltInMuUsb\\SystemExtPartition\n"		\
			"      - \\Device\\BuiltInMuUsb\\SystemAuxPartition\n"		\
			"      - \\Device\\BuiltInMuMmc\\Storage\n"					\
			"      - \\Device\\BuiltInMuMmc\\StorageSystem\n"			\
			"      - \\Device\\BuiltInMuMmc\\SystemExtPartition\n"		\
			"      - \\Device\\BuiltInMuMmc\\SystemAuxPartition\n"		\
			"      - \\Device\\HdDvdRom\n"								\
			"      - \\Device\\HdDvdStorage\n"							\
			"      - \\Device\\HdDvdPlayer\n"							\
			"      - \\Device\\TransferCable\n"							\
			"      - \\Device\\Mass0PartitionFile\\Storage\n"			\
			"      - \\Device\\Mass1PartitionFile\\Storage\n"			\
			"      - \\Device\\Mass0PartitionFile\\StorageSystem\n"		\
			"      - \\Device\\Mass1PartitionFile\\StorageSystem\n"		\
			"      - \\Device\\Harddisk0\\Cache0\n"						\
			"      - \\Device\\Harddisk0\\Cache1\n"						\
			"      - \\Device\\NetworkStorageCache\n"					\
			"      - \\Device\\UpdateStorage\n"							\
			"      - \\Device\\Harddisk0\\TitleURLCachePartition\n"		\
			"      - \\Device\\DeviceAuth\n"							\
			"      - \\Device\\BuiltInMuUsb\\ReservationPartition\n"	\
			"      - \\Device\\LauncherData\n"							\
			"\n\n\n\nGreets to the following people:\n"					\
			"gabe_k, Xenon.7 (Xenomega), paul (British dude), rickshaw, skitzo, kiwi, dual\n" \
			"yolo."
			);

		std::fstream readme("g:\\readme.txt", std::ios::trunc | std::ios::in | std::ios::out);
		readme << IniText;
		readme.close();

		ini.SetBoolValue("settings", "SetLeds", true);
		ini.SetBoolValue("settings", "PatchRsaPubCrypt", true);
		ini.SetBoolValue("settings", "RemoveConsoleSignatureChecks", true);
		ini.SetBoolValue("settings", "RemovePackageRsaChecks", true);
		ini.SetBoolValue("settings", "RemoveProfileBreakpoints", true);
		ini.SetBoolValue("settings", "RemoveXampHeapAllocExplicitBreakpoints", true);
		ini.SetBoolValue("settings", "SetAsStartupApplication", true);
		ini.SetBoolValue("settings", "EnableHiddenPartitions", true);
		ini.SetBoolValue("settings", "EnableExtraNeighborhoodDrives", true);

		ini.SetBoolValue("settings", "SetGamertag", true);
		ini.SetBoolValue("settings", "SetGoldMembership", true);
		ini.SetBoolValue("settings", "EnableXamWatsonOutput", true);

		ini.SetValue("Network", "AccountCreatePassword", "meth=friend");
		ini.SetValue("Network", "OnlineGamertag", "[test]DRUG DEALER"); // MODIFY THIS LINE TO YOUR DEFAULT GAMERTAG
		//ini.SetBoolValue("xshell", "WriteCustomBackground", false);
		//ini.SetValue("xshell", "DevKitBackground", "bg_image_blue.png");
		//ini.SetValue("xshell", "TestKitBackground", "bg_image_gray.png");

		ini.SetValue("LEDs", "TopLeft", "Inverted");
		ini.SetValue("LEDs", "TopRight", "Normal");
		ini.SetValue("LEDs", "BottomLeft", "Mix");
		ini.SetValue("LEDs", "BottomRight", "Off");
		
		ini.SetValue("NeighborhoodDrive1", "DevicePath", "");
		ini.SetValue("NeighborhoodDrive1", "NeighborhoodName", "");

		ini.SetValue("NeighborhoodDrive2", "DevicePath", "");
		ini.SetValue("NeighborhoodDrive2", "NeighborhoodName", "");

		ini.SetValue("NeighborhoodDrive3", "DevicePath", "");
		ini.SetValue("NeighborhoodDrive3", "NeighborhoodName", "");

		ini.SetValue("NeighborhoodDrive4", "DevicePath", "");
		ini.SetValue("NeighborhoodDrive4", "NeighborhoodName", "");
		
		ini.SaveFile("g:\\settings.ini");
		ini.LoadFile("g:\\settings.ini");
	}
	CSimpleIniA::TNamesDepend keys;
	ini.GetAllKeys("settings", keys);
	CSimpleIniA::TNamesDepend::iterator it;
	for (it = keys.begin(); it != keys.end(); it++)
	{
		const char *key = ((*it).pItem);
		bool Enabled = ini.GetBoolValue("settings", key, NULL);
		Settings[std::string(key)] = Enabled;
	}

	keys.clear();
	ini.GetAllKeys("Network", keys);
	NetworkSettings ns;
	for (it = keys.begin(); it != keys.end(); it++)
	{
		std::string key((*it).pItem);
		std::string value = ini.GetValue("Network", key.c_str(), NULL);
		if (key == "AccountCreatePassword")
			ns.AccountCreationPassword = value;
		else if (key == "OnlineGamertag")
			ns.Gamertag = value;
	}

	std::vector<Utilities::Drive> drl;
	char Section[20] = {0};
	for (int i = 1; i < 5; i++)
	{
		keys.clear();
		sprintf(Section, "NeighborhoodDrive%d", i);
		ini.GetAllKeys(Section, keys);
		Utilities::Drive d;
		for (it = keys.begin(); it != keys.end(); it++)
		{
			std::string key  = ((*it).pItem);
			std::string value = ini.GetValue(Section, key.c_str(), NULL);
			if (key == "NeighborhoodName")
				d.NeighborhoodName = value;
			else if(key == "DevicePath")
				d.Path = value;
		}
		if (!d.NeighborhoodName.size() || !d.Path.size())
			continue;
		else
			drl.push_back(d);
	}

	if (Settings[std::string("SetLeds")])
	{
		LEDState TL = LEDState::ORANGE, TR = LEDState::RED, BL = LEDState::ORANGE, BR = LEDState::RED;
		keys.clear();
		ini.GetAllKeys("LEDs", keys);
		for (it = keys.begin(); it != keys.end(); it++)
		{
			std::string key(((*it).pItem));
			std::string value(ini.GetValue("LEDs", key.c_str(), NULL));
			LEDState *CurrentLed = nullptr;
			if (key == "TopLeft")
				CurrentLed = &TL;
			else if (key == "TopRight")
				CurrentLed = &TR;
			else if (key == "BottomLeft")
				CurrentLed = &BL;
			else if (key == "BottomRight")
				CurrentLed = &BR;

			if (CurrentLed == nullptr)
				continue;

			if (value == "Inverted")
				*CurrentLed = LEDState::RED;
			else if (value == "Normal")
				*CurrentLed = LEDState::GREEN;
			else if (value == "Mix")
				*CurrentLed = LEDState::ORANGE;
			else if (value == "Off")
				*CurrentLed = LEDState::OFF;
		}
		u.SetLeds(TL, TR, BL, BR);
	}
	if (Settings[std::string("PatchRsaPubCrypt")])
		u.PatchRsaPubCrypt();
	if (Settings[std::string("RemoveConsoleSignatureChecks")])
		u.RemoveConsoleSignatureChecks();
	if (Settings[std::string("RemovePackageRsaChecks")])
		u.RemovePackageRsaChecks();
	if (Settings[std::string("RemoveProfileBreakpoints")])
		u.RemoveProfileBreakpoints();
	if (Settings[std::string("RemoveXampHeapAllocExplicitBreakpoints")])
		u.RemoveXampHeapAllocExplicitBreakpoints();
	if (Settings[std::string("EnableXamWatsonOutput")])
		u.EnableXamWatsonOutput();

	if (ns.AccountCreationPassword != "")
		u.SetDefaultAccountPassword(ns.AccountCreationPassword);
	if (Settings[std::string("SetGamertag")] && ns.Gamertag != "")
		u.SetGamertag(ns.Gamertag);
	if (Settings[std::string("SetGoldMembership")])
		u.SetGold();
	if (Settings[std::string("EnableExtraNeighborhoodDrives")])
		u.AddExtraDrives(drl);
	else if (Settings[std::string("EnableHiddenPartitions")])
		u.EnableHiddenDrives();
	if (Settings[std::string("SetAsStartupApplication")])
		u.EnableApplicationStartup();
	u.PatchControllerDisconnect();

	// Return to launcher
	u.LaunchXshell();
}