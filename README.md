# UpdateCheckAPI
The UpdateCheckAPI repository provides utility source code for AMD Tools to check for new releases available through GitHub's Release API. The code also provides a Qt-based dialog that can be used to display the results of the check for updates.

## Usage:
Projects that want to utilize UpdateCheckAPI can do so using CMake's add_subdirectory(<path to UpdateCheckAPI>) command. UpdateCheckAPI contains a CMakeLists.txt which defines several other cached variables that identify the various files that are necessary to use UpdateCheckAPI:
* UPDATECHECKAPI_SRC (source files)
* UPDATECHECKAPI_INC (header files)
* UPDATECHECKAPI_INC_DIRS (additional include directories)
* UPDATECHECKAPI_LIBS (required libraries)
* UPDATECHECKAPI_LIB_DIRS (additional library directories)

Additional CMake variables are also defined to utilize the Qt widgets:
* UPDATECHECKAPI_QT_SRC (source files which reference Qt components)
* UPDATECHECKAPI_QT_INC (header files which reference Qt components)
* UPDATECHECKAPI_QT_UI (ui files for the new widgets)

Also, the UpdateCheckAPI utilizes an executable named rtda to download files from the internet. This needs to copied into the application's working directory. To simplify copying the executable, its platform-specific path is cached in the CMake variable:
* RTDA_PATH (Path to the platform-specific rtda executable)

## Release Notes:
Version 2.1.1
* Support an environment variable "RDTS_UPDATER_ASSUME_VERSION" for overriding the current version of the tool
* Tooltip painting updates
* Code formatting updates

Version 2.1.0
* An optional PackageName tag has been added to the DownloadLinks which allows for customizing the text that will appear in the UI in place of the package type.
* RTDA now has a --version cmd line option to report both the RTDA and UpdateCheckAPI version numbers.
* Add RTDA binaries to git lfs.

Version 2.0.1
* Fix the close button on Update Check Results dialog.

Version 2.0.0
* Updated all code and filenames to follow Google C++ Style Guide
* Updated to json.hpp version 3.9.1

Version 1.1.0
* Rename AMDToolsDownloader to rtda and remove logo.
* Fix compiler warning on Windows and ubuntu 20.04.

Version 1.0.1
* Fix crash caused by parse error on restricted networks.
* Add GUI option for hiding release tags.

Version 1.0
* Initial release

