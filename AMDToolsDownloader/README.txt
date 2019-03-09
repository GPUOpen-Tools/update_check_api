#=================================================================
# Copyright 2018-2019 Advanced Micro Devices, Inc. All rights reserved.
#=================================================================
In order to build this project:

1) Download and install the Go tools from here: https://golang.org/doc/install
2) Make any changes to the AMDToolsDownloader.go file.
3) cd into the AMDToolsDownloader directory.
4) Compile it with the command: "go build"
   * If there are compiler errors, they will be printed.
   * If the compilation is successful, nothing will get printed.
5) You should find an updated AMDToolsDownloader.exe in the local directory.

NOTE: Go naming conventions specify that the project directory (/AMDToolsDownloader) should actually be entirely in lowercase to avoid differences in capitalization across platforms. We do not make heavy use of Go, and prefer this capitalization for this project, so the exception has been made. Also, the name of the compiled binary file is automatically selected from the project directory, not the source files, so in order to change the generated binary filename, the project directory name must also change.