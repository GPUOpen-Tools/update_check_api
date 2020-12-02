#=================================================================
# Copyright 2018-2020 Advanced Micro Devices, Inc. All rights reserved.
#=================================================================
In order to build this project:

1) Download and install the Go tools from here: https://golang.org/doc/install
2) Make any changes to the radeon_tools_download_assistant.go file.
3) cd into the rtda directory.
4) Compile it with the command: "go build"
   * If there are compiler errors, they will be printed.
   * If the compilation is successful, nothing will get printed.
5) You should find an updated rtda.exe in the local directory.

NOTE: The name of the compiled binary file is automatically selected from the project directory, not the source files, so in order to change the generated binary filename, the project directory name must also change.
