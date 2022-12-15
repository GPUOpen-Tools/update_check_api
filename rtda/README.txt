#=================================================================
# Copyright 2018-2022 Advanced Micro Devices, Inc. All rights reserved.
#=================================================================
In order to build this project:

1) Download and install the Go tools from here: https://golang.org/doc/install
2) Update rtda.rc if there needs to be an updated version number.
2b) If on Windows, run generate_syso.bat. This will convert the rtda.rc file into a versioninfo.syso file that will automatically get embedded in the resulting executable so that the file properties in Windows are properly filled in.
3) Make any changes to the radeon_tools_download_assistant.go file.
4) cd into the rtda directory.
5) Compile it with the command: "go build"
   * If there are compiler errors, they will be printed.
   * If the compilation is successful, nothing will get printed.
6) You should find an updated rtda.exe in the local directory. Make sure to run it manually from the command line to ensure it works properly. If you get an "access is denied" error, there may be coding errors that are preventing it from running properly.

NOTE: The name of the compiled binary file is automatically selected from the project directory, not the source files, so in order to change the generated binary filename, the project directory name must also change.
