# UpdateCheckAPI
The UpdateCheckAPI repository provides utility source code for AMD Tools to check for new releases available through GitHub's Release API. The code also provides a Qt-based dialog that can be used to display the results of the check for updates.

## Usage:
Projects that want to utilize UpdateCheckAPI can do so using CMake's `add_subdirectory(<path to UpdateCheckAPI>)` command. UpdateCheckAPI defines two CMake library targets:
* `update_check_api`, which contains the update-checking functionality
* `update_check_api_qt`, which contains the Qt-based widgets for checking for updates in a GUI application.

To use them, configure your project to link against one of the libraries using `target_link_libraries`.

`update_check_api_qt` depends on `update_check_api` and will provide it so only one `target_link_libraries` is required.

### Headless builds
Projects which do not depend on Qt can use the non-GUI components of UpdateCheckApi without adding a Qt dependency
by setting the `UPDATE_CHECK_API_HEADLESS` CMake cache option.

### RTDA
The UpdateCheckAPI utilizes an executable named rtda to download files from the internet. This needs to copied into the application's working directory. To simplify copying the executable, its platform-specific path is cached in the CMake variable:
* `RTDA_PATH` (Path to the platform-specific rtda executable)

## Release Notes:
Version 2.1.2
* Define a separate target for the update checker GUI

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

