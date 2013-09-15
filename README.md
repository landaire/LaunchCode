LaunchCode (for Xbox 360 development kits)
========================================  
### By Lander Brandt

Some important notes about LaunchCode:

- Some functions are dependent on the kernel revision. For instance (or maybe the only instance? it's been a while since I've dug deep into the app), the PatchControllerDisconnect function has a hardcoded address simply because there are no functions close enough to it which are exported. What it patches is the branch at RgcSecAuthenticateDeviceAsyncStateMachine+704 to avoid the function branching into the code where the controller challenge is seen as failed.

- Some functions are quite hacky. See the SetGamertag function.

- This is not the complete app. Some features are removed because they certainly should not be in the hands of the general public (who can't be bothered to research stuff themselves).

Here is the original readme:


LaunchCode was created by CLK.


### What can this app do?  
- Remove console signature checks  
- Remove RSA signature checks
- Set RoL LEDs
- Remove profile breakpoints (freezing when viewing some profiles)  
 - Remove XampHeapAllocExplicit breakpoints (some bullshit)  
- Enable hidden partitions for Xbox 360 Neighborhood  
- Add additional, customizable partitions to Neighborhood  
- Set the default account password for PartnerNet accounts    
- Set the display gamertag    
- Make the console think you have a gold membership


#### -> Instructions/Limitations:    
- Account password length must not exceed 11 characters.    
- Each Neighborhood partition should match the following  
			 criteria, otherwise it will not be mounted:    
	- NeighborhoodName must not exceed 35 characters    
	- DevicePath must not exceed 61 characters    
	- DevicePath must be formatted as so:    
		- \\Device\\Harddisk0\\Partition1\\Devikit\     
		- The following are all valid basic paths:    
			       - \\Device    
			       - \\Device\\Harddisk0\\SystemPartition    
			       - \\Device\\Harddisk0\\Partition1    
			       - \\Device\\Flash    
			       - \\Device\\Mu0System    
			       - \\Device\\Mu1System    
			       - \\Device\\Mass0    
			       - \\Device\\Mass1    
			       - \\Device\\Mass2    
			       - \\Device\\BuiltInMuSfcSystem    
			       - \\Device\\Harddisk\\SystemExtPartition    
			       - \\Device\\Harddisk\\SystemAuxPartition    
			       - \\Device\\BuiltInMuUsb\\Storage    
			       - \\Device\\BuiltInMuUsb\\StorageSystem    
			       - \\Device\\BuiltInMuUsb\\SystemExtPartition    
			       - \\Device\\BuiltInMuUsb\\SystemAuxPartition    
			       - \\Device\\BuiltInMuMmc\\Storage    
			       - \\Device\\BuiltInMuMmc\\StorageSystem    
			       - \\Device\\BuiltInMuMmc\\SystemExtPartition    
			       - \\Device\\BuiltInMuMmc\\SystemAuxPartition    
			       - \\Device\\HdDvdRom    
			       - \\Device\\HdDvdStorage    
			       - \\Device\\HdDvdPlayer    
			       - \\Device\\TransferCable    
			       - \\Device\\Mass0PartitionFile\\Storage    
			       - \\Device\\Mass1PartitionFile\\Storage    
			       - \\Device\\Mass0PartitionFile\\StorageSystem    
			       - \\Device\\Mass1PartitionFile\\StorageSystem    
			       - \\Device\\Harddisk0\\Cache0    
			       - \\Device\\Harddisk0\\Cache1    
			       - \\Device\  etworkStorageCache    
			       - \\Device\\UpdateStorage    
			       - \\Device\\Harddisk0\\TitleURLCachePartition    
			       - \\Device\\DeviceAuth    
			       - \\Device\\BuiltInMuUsb\\ReservationPartition    
			       - \\Device\\LauncherData    


Greets to the following people:    
			 gabe_k, Xenon.7 (Xenomega), paul (British dude), rickshaw, skitzo, kiwi, dual    
			 yolo. 