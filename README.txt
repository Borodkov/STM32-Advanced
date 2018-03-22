structure of workspace

	- YourAnyFolderName
		|
		|- _Projects         (this repository)
		|
		|- EmbWizzard        (cool GUI, see youtube)
		|
		|- gcc-arm-none-eabi (compiler for EmbWizzard build system)
		|
		|- STM32Cube_FW_F7   (not included in repository, you must copy\paste your cube HAL lib here)


where
	_Projects          <- repository here
	EmbWizzard         <- for advanced GUI
	gcc-arm-none-eabi  <- compiler gcc-arm-none-eabi-6-2017-q2-update-win32.exe
	STM32Cube_FW_F7    <- latest STM32Cube_FW_F7_V1.11.0


 In dirName of cube lib remove version (this is for EmbWizzard examples and their build system with GCC):

	STM32Cube_FW_F7_V1.11.0   ->    STM32Cube_FW_F7

 If no need in this GUI - folders "EmbWizzard"+"gcc-arm-none-eabi" can be deleted.
 But in all projects path to HAL and BSP drivers is  ..STM32Cube_FW_F7\Drivers\..  without version.