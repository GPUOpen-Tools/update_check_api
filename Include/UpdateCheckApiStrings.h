//==============================================================================
// Copyright (c) 2018-2019 Advanced Micro Devices, Inc. All rights reserved.
/// \author AMD Developer Tools Team
//
/// \file
//
/// \brief Constant strings used by the UpdateCheckApi.
//==============================================================================
#ifndef _UPDATECHECKAPI_STRINGS_H_
#define _UPDATECHECKAPI_STRINGS_H_

#define SCHEMA_VERSION_1_3 "1.3"
#define SCHEMA_VERSION_1_5 "1.5"
#define SCHEMA_VERSION_1_6 "1.6"
#define CURRENT_SCHEMA_VERSION SCHEMA_VERSION_1_6

// Parsing helpers
#define STR_VERSION_DELIMITER "."

// Default string for undefined values.
const char* const STR_UNDEFINED = "undefined";

// JSON file extension.
const char* const STR_JSON_FILE_EXTENSION = ".json";

// JSON tag for the SchemaVersion.
#define SCHEMAVERSION "SchemaVersion"

// JSON tags realted to schema 1.6
#define RELEASES "Releases"
#define RELEASEPLATFORMS "ReleasePlatforms"
#define RELEASETAGS "ReleaseTags"
#define RELEASETITLE "ReleaseTitle"
#define RELEASETYPE "ReleaseType"

// JSON tags related to schema 1.5
#define RELEASEVERSION "ReleaseVersion"
#define RELEASEVERSION_MAJOR "Major"
#define RELEASEVERSION_MINOR "Minor"
#define RELEASEVERSION_PATCH "Patch"
#define RELEASEVERSION_BUILD "Build"
#define RELEASEDESCRIPTION "ReleaseDescription"
#define INFOPAGELINKS "InfoPageLinks"
#define INFOPAGELINKS_URL "URL"
#define INFOPAGELINKS_DESCRIPTION "Description"
#define DOWNLOADLINKS "DownloadLinks"
#define DOWNLOADLINKS_URL "URL"
#define DOWNLOADLINKS_TARGETPLATFORMS "TargetPlatforms"
#define DOWNLOADLINKS_PACKAGETYPE "PackageType"
#define DOWNLOADLINKS_RELEASETYPE "ReleaseType"

// JSON PackageType values from schema 1.5
const char* const STR_PACKAGETYPE_ZIP = "ZIP";
const char* const STR_PACKAGETYPE_MSI = "MSI";
const char* const STR_PACKAGETYPE_TAR = "TAR";
const char* const STR_PACKAGETYPE_RPM = "RPM";
const char* const STR_PACKAGETYPE_DEBIAN = "Debian";
const char* const STR_PACKAGETYPE_UNKNOWN = "Unknown";

// JSON ReleaseType values from schema 1.5
const char* const STR_RELEASETYPE_GA = "GA";
const char* const STR_RELEASETYPE_BETA = "Beta";
const char* const STR_RELEASETYPE_ALPHA = "Alpha";
const char* const STR_RELEASETYPE_PATCH = "Patch";
const char* const STR_RELEASETYPE_DEVELOPMENT = "Development";
const char* const STR_RELEASETYPE_UNKNOWN = "Unknown";

// JSON tags related to schema 1.3
#define VERSIONSTRING "VersionString"
#define RELEASEDATE "ReleaseDate"
#define DESCRIPTION "Description"
#define INFOPAGEURL "InfoPageURL"
#define INFOPAGEURL_URL "URL"
#define INFOPAGEURL_DESCRIPTION "Description"
#define DOWNLOADURL "DownloadURL"
#define DOWNLOADURL_URL "URL"
#define DOWNLOADURL_TARGETINFO "TargetInfo"

// JSON PlatformType values from schema 1.3
const char* const STR_PLATFORMTYPE_WINDOWS = "Windows";
const char* const STR_PLATFORMTYPE_UBUNTU = "Ubuntu";
const char* const STR_PLATFORMTYPE_RHEL = "RHEL";
const char* const STR_PLATFORMTYPE_DARWIN = "Darwin";
const char* const STR_PLATFORMTYPE_UNKNOWN = "Unknown";

// JSON PackageType values from schema 1.5
const char* const STR_PACKAGETYPE_WINDOWS_ZIP = "Windows_ZIP";
const char* const STR_PACKAGETYPE_WINDOWS_MSI = "Windows_MSI";
const char* const STR_PACKAGETYPE_LINUX_TAR = "Linux_TAR";
const char* const STR_PACKAGETYPE_LINUX_RPM = "Linux_RPM";
const char* const STR_PACKAGETYPE_LINUX_DEBIAN = "Linux_Debian";

// Name of the downloader application.
#ifdef WIN32
const char* const STR_DOWNLOADER_APPLICATION = "AMDToolsDownloader.exe";
#elif defined(__linux__) || defined(__APPLE__)
const char* const STR_DOWNLOADER_APPLICATION = "./AMDToolsDownloader";
#endif // __linux__ || __APPLE__

// Strings related to the Github Release API.
const char* const STR_HTTP_PREFIX = "http";
const char* const STR_GITHUB_RELEASES_LATEST = "/releases/latest";
const char* const STR_LATEST_JSON_FILENAME = "AMDToolsLatestRelease.json";
const char* const TAG_ASSETS = "assets";
const char* const TAG_ASSET_NAME = "name";
const char* const TAG_MESSAGE = "message";
const char* const TAG_ASSET_BROWSWERDOWNLOADURL = "browser_download_url";
const char* const STR_ERROR_MISSING_ASSETS_TAG = "The latest releases JSON is missing the assets element. ";
const char* const STR_ERROR_ASSET_NOT_FOUND = "The required asset was not found in the assets list. ";
const char* const STR_ERROR_DOWNLOAD_URL_NOT_FOUND_IN_ASSET = "The download url was not found for the required asset. ";

// High Level Error Messages.
const char* const STR_ERROR_URL_MUST_POINT_TO_A_JSON_FILE = "URL must point to a JSON file.";
const char* const STR_ERROR_UNABLE_TO_FIND_TEMP_DIRECTORY = "Unable to find temp directory.";
const char* const STR_ERROR_UNKNOWN_ERROR_OCCURRED = "An unknown error occurred: ";
const char* const STR_ERROR_FAILED_TO_LAUNCH_VERSION_FILE_DOWNLOADER = "Failed to launch the AMD Tools Downloader.";
const char* const STR_ERROR_FAILED_TO_LAUNCH_VERSION_FILE_DOWNLOADER_UNKNOWN_ERROR = "Failed to launch the AMD Tools Downloader due to an unknown error: ";
const char* const STR_ERROR_FAILED_TO_DOWNLOAD_VERSION_FILE = "Failed to download version file.";
const char* const STR_ERROR_FAILED_TO_LOAD_VERSION_FILE = "Failed to load version file.";
const char* const STR_ERROR_DOWNLOADED_AN_EMPTY_VERSION_FILE = "Downloaded an empty version file.";
const char* const STR_ERROR_FAILED_TO_PARSE_VERSION_FILE = "Failed to parse version file.";
const char* const STR_ERROR_UNSUPPORTED_SCHEMAVERION = "The schema version of the version file is not supported; latest supported version is " CURRENT_SCHEMA_VERSION ".";
const char* const STR_ERROR_UNABLE_TO_CONVERT_1_5_TO_1_6 = "Unable to convert from UpdateCheckApi Schema 1.5 to 1.6.";
const char* const STR_ERROR_UNABLE_TO_CONVERT_1_3_TO_1_6 = "Unable to convert from UpdateCheckApi Schema 1.3 to 1.6.";

// Error Messages related to missing or invalid JSON tags.
#define VERSION_FILE_IS_MISSING_THE "The version file is missing the "
#define ENTRY " entry. "

const char* const STR_ERROR_VERSION_FILE_IS_MISSING_THE_SCHEMAVERSION_ENTRY = VERSION_FILE_IS_MISSING_THE SCHEMAVERSION ENTRY;
const char* const STR_ERROR_VERSION_FILE_IS_MISSING_THE_RELEASES_ENTRY = VERSION_FILE_IS_MISSING_THE RELEASES ENTRY;
const char* const STR_ERROR_VERSION_FILE_IS_MISSING_THE_VERSIONSTRING_ENTRY = VERSION_FILE_IS_MISSING_THE VERSIONSTRING ENTRY;
const char* const STR_ERROR_VERSION_FILE_IS_MISSING_THE_RELEASEDATE_ENTRY = VERSION_FILE_IS_MISSING_THE RELEASEDATE ENTRY;
const char* const STR_ERROR_VERSION_FILE_IS_MISSING_THE_DESCRIPTION_ENTRY = VERSION_FILE_IS_MISSING_THE DESCRIPTION ENTRY;
const char* const STR_ERROR_VERSION_FILE_IS_MISSING_THE_INFOPAGEURL_ENTRY = VERSION_FILE_IS_MISSING_THE INFOPAGEURL ENTRY;
const char* const STR_ERROR_VERSION_FILE_IS_MISSING_THE_DOWNLOADURL_ENTRY = VERSION_FILE_IS_MISSING_THE DOWNLOADURL ENTRY;
const char* const STR_ERROR_VERSION_FILE_IS_MISSING_THE_RELEASEVERSION_ENTRY = VERSION_FILE_IS_MISSING_THE RELEASEVERSION ENTRY;
const char* const STR_ERROR_VERSION_FILE_IS_MISSING_THE_RELEASEDESCRIPTION_ENTRY = VERSION_FILE_IS_MISSING_THE RELEASEDESCRIPTION ENTRY;
const char* const STR_ERROR_VERSION_FILE_IS_MISSING_THE_RELEASETAGS_ENTRY = VERSION_FILE_IS_MISSING_THE RELEASETAGS ENTRY;
const char* const STR_ERROR_VERSION_FILE_IS_MISSING_THE_RELEASETITLE_ENTRY = VERSION_FILE_IS_MISSING_THE RELEASETITLE ENTRY;
const char* const STR_ERROR_VERSION_FILE_IS_MISSING_THE_RELEASETYPE_ENTRY = VERSION_FILE_IS_MISSING_THE RELEASETYPE ENTRY;
const char* const STR_ERROR_VERSION_FILE_IS_MISSING_THE_RELEASEPLATFORMS_ENTRY = VERSION_FILE_IS_MISSING_THE RELEASEPLATFORMS ENTRY;
const char* const STR_ERROR_VERSION_FILE_IS_MISSING_THE_INFOPAGELINKS_ENTRY = VERSION_FILE_IS_MISSING_THE INFOPAGELINKS ENTRY;
const char* const STR_ERROR_VERSION_FILE_IS_MISSING_THE_DOWNLOADLINKS_ENTRY = VERSION_FILE_IS_MISSING_THE DOWNLOADLINKS ENTRY;

const char* const STR_ERROR_VERSION_FILE_IS_MISSING_THE_DOWNLOADLINKS_URL_ENTRY = VERSION_FILE_IS_MISSING_THE DOWNLOADLINKS_URL ENTRY;
const char* const STR_ERROR_VERSION_FILE_IS_MISSING_THE_DOWNLOADLINKS_TARGETPLATFORMS_ENTRY = VERSION_FILE_IS_MISSING_THE DOWNLOADLINKS_TARGETPLATFORMS ENTRY;
const char* const STR_ERROR_VERSION_FILE_IS_MISSING_THE_DOWNLOADLINKS_PACKAGETYPE_ENTRY = VERSION_FILE_IS_MISSING_THE DOWNLOADLINKS_PACKAGETYPE ENTRY;
const char* const STR_ERROR_VERSION_FILE_IS_MISSING_THE_DOWNLOADLINKS_RELEASETYPE_ENTRY = VERSION_FILE_IS_MISSING_THE DOWNLOADLINKS_RELEASETYPE ENTRY;

const char* const STR_ERROR_INVALID_VERSION_NUMBER_PROVIDED = "The version file contains an invalid " RELEASEVERSION " number. ";

const char* const STR_ERROR_VERSION_FILE_CONTAINS_AN_EMPTY_RELEASES_LIST = "The version file contains an empty " RELEASES " list. ";

const char* const STR_ERROR_VERSION_FILE_CONTAINS_AN_EMPTY_INFOPAGEURL_LIST = "The version file contains an empty " INFOPAGEURL " list. ";
const char* const STR_ERROR_VERSION_FILE_CONTAINS_AN_INCOMPLETE_INFOPAGEURL_ENTRY = "The version file contains an incomplete " INFOPAGEURL " entry. ";

const char* const STR_ERROR_VERSION_FILE_CONTAINS_AN_EMPTY_DOWNLOADURL_LIST = "The version file contains an empty " DOWNLOADURL " list. ";
const char* const STR_ERROR_VERSION_FILE_CONTAINS_AN_INCOMPLETE_DOWNLOADURL_ENTRY = "The version file contains an incomplete " DOWNLOADURL " entry. ";
const char* const STR_ERROR_VERSION_FILE_CONTAINS_AN_INVALID_DOWNLOADURL_TARGETINFO_VALUE = "The version file contains an invalid " DOWNLOADURL_TARGETINFO " value. ";

const char* const STR_ERROR_VERISON_FILE_CONTAINS_AN_EMPTY_DOWNLOADLINKS_TARGETPLATFORMS_LIST = "The version file contains an empty " DOWNLOADLINKS_TARGETPLATFORMS " list. ";
const char* const STR_ERROR_VERSION_FILE_CONTAINS_AN_INVALID_DOWNLOADLINKS_TARGETPLATFORM_VALUE = "The version file contains an invalid " DOWNLOADLINKS_TARGETPLATFORMS " value. ";
const char* const STR_ERROR_VERSION_FILE_CONTAINS_AN_INVALID_DOWNLOADLINKS_RELEASETYPE_VALUE = "The version file contains an invalid " DOWNLOADLINKS_RELEASETYPE " value. ";
const char* const STR_ERROR_VERSION_FILE_CONTAINS_AN_INVALID_DOWNLOADLINKS_PACKAGETYPE_VALUE = "The version file contains an invalid " DOWNLOADLINKS_PACKAGETYPE " value. ";

const char* const STR_ERROR_VERISON_FILE_CONTAINS_AN_EMPTY_RELEASEPLATFORMS_LIST = "The version file contains an empty " RELEASEPLATFORMS " list. ";
const char* const STR_ERROR_VERSION_FILE_CONTAINS_AN_INVALID_RELEASEPLATFORMS_VALUE = "The version file contains an invalid " RELEASEPLATFORMS " value. ";

const char* const STR_ERROR_VERSION_FILE_CONTAINS_AN_INVALID_RELEASETYPE_VALUE = "The version file contains an invalid " RELEASETYPE " value. ";

const char* const STR_ERROR_VERSION_FILE_CONTAINS_AN_EMPTY_INFOPAGELINKS_LIST = "The version file contains an empty " INFOPAGELINKS " list. ";
const char* const STR_ERROR_VERSION_FILE_CONTAINS_AN_INCOMPLETE_INFOPAGELINKS_ENTRY = "The version file contains an incomplete " INFOPAGELINKS " entry. ";

const char* const STR_ERROR_VERSION_FILE_CONTAINS_AN_EMPTY_DOWNLOADLINKS_LIST = "The version file contains an empty " DOWNLOADLINKS " list. ";
const char* const STR_ERROR_VERSION_FILE_CONTAINS_AN_INCOMPLETE_DOWNLOADLINKS_ENTRY = "The version file contains an incomplete " DOWNLOADLINKS " entry. ";

#endif //_UPDATECHECKAPI_STRINGS_H_
