structure of workspace

	- YourAnyFolderName
		|
		|- _Projects
		|
		|- EmbWizzard
		|
		|- gcc-arm-none-eabi
		|
		|- STM32Cube_FW_F7


where
	_Projects          <- repository here
	EmbWizzard         <- for advanced GUI
	gcc-arm-none-eabi  <- compiler gcc-arm-none-eabi-6-2017-q2-update-win32.exe
	STM32Cube_FW_F7    <- latest STM32Cube_FW_F7_V1.11.0.


 In dirName of cube lib "STM32Cube_FW_F7" delete version for EmbWizzard examples.
 If no need in this GUI - folders "EmbWizzard"+"gcc-arm-none-eabi" can be deleted.
 But in all projects path to HAL and BSP drivers is  ..STM32Cube_FW_F7\Drivers\..
 without version of 