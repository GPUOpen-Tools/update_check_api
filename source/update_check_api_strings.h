//==============================================================================
/// Copyright (c) 2018-2020 Advanced Micro Devices, Inc. All rights reserved.
/// \author AMD Developer Tools Team
/// \file
/// \brief Constant strings used by the UpdateCheckApi.
//==============================================================================
#ifndef UPDATECHECKAPI_UPDATE_CHECK_API_STRINGS_H_
#define UPDATECHECKAPI_UPDATE_CHECK_API_STRINGS_H_

#define SCHEMA_VERSION_1_3 "1.3"
#define SCHEMA_VERSION_1_5 "1.5"
#define SCHEMA_VERSION_1_6 "1.6"
#define CURRENT_SCHEMA_VERSION SCHEMA_VERSION_1_6

// Parsing helpers
#define VERSION_DELIMITER "."

// Default string for undefined values.
const char* const kStringUndefined = "undefined";

// JSON file extension.
const char* const kStringJsonFileExtension = ".json";

// JSON tag for the SchemaVersion.
#define SCHEMAVERSION "SchemaVersion"

// JSON tags realted to schema 1.6
#define RELEASES "Releases"
#define RELEASEPLATFORMS "ReleasePlatforms"
#define RELEASETAGS "ReleaseTags"
#define RELEASETITLE "ReleaseTitle"
#define RELEASETYPE "ReleaseType"
#define DOWNLOADLINKS_PACKAGENAME "PackageName"

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
const char* const kStringPackageTypeZip     = "ZIP";
const char* const kStringPackageTypeMsi     = "MSI";
const char* const kStringPackageTypeTar     = "TAR";
const char* const kStringPackageTypeRpm     = "RPM";
const char* const kStringPackageTypeDebian  = "Debian";
const char* const kStringPackageTypeUnknown = "Unknown";

// JSON ReleaseType values from schema 1.5
const char* const kStringReleaseTypeGeneralAvailability = "GA";
const char* const kStringReleaseTypeBeta                = "Beta";
const char* const kStringReleaseTypeAlpha               = "Alpha";
const char* const kStringReleaseTypePatch               = "Patch";
const char* const kStringReleaseTypeDevelopment         = "Development";
const char* const kStringReleaseTypeUnknown             = "Unknown";

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
const char* const kStringPlatformTypeWindows = "Windows";
const char* const kStringPlatformTypeUbuntu  = "Ubuntu";
const char* const kStringPlatformTypeRhel    = "RHEL";
const char* const kStringPlatformTypeDarwin  = "Darwin";
const char* const kStringPlatformTypeUnknown = "Unknown";

// JSON PackageType values from schema 1.5
const char* const kStringPackageTypeWindowsZip  = "Windows_ZIP";
const char* const kStringPackageTypeWindowsMsi  = "Windows_MSI";
const char* const kStringPackageTypeLinuxTar    = "Linux_TAR";
const char* const kStringPackageTypeLinuxRpm    = "Linux_RPM";
const char* const kStringPackageTypeLinuxDebian = "Linux_Debian";

// Name of the downloader application.
#ifdef WIN32
const char* const kStringDownloaderApplication = "rtda.exe";
#elif defined(__linux__) || defined(__APPLE__)
const char* const kStringDownloaderApplication = "./rtda";
#endif  // __linux__ || __APPLE__

// Strings related to the Github Release API.
const char* const kStringHttpPrefix                      = "http";
const char* const kStringGithubReleasesLatest            = "/releases/latest";
const char* const kStringLatestJsonFilename              = "AMDToolsLatestRelease.json";
const char* const kStringTagAssets                       = "assets";
const char* const kStringTagAssetName                    = "name";
const char* const kStringTagMessage                      = "message";
const char* const kStringTagAssetBrowserDownloadUrl      = "browser_download_url";
const char* const kStringErrorMissingAssetsTags          = "The latest releases JSON is missing the assets element. ";
const char* const kStringErrorAssetNotFound              = "The required asset was not found in the assets list. ";
const char* const kStringErrorDownloadUrlNotFoundInAsset = "The download url was not found for the required asset. ";

// High Level Error Messages.
const char* const kStringErrorUrlMustPointToAJsonFile                         = "URL must point to a JSON file.";
const char* const kStringErrorUnableToFindTempDirectory                       = "Unable to find temp directory.";
const char* const kStringErrorUnknownErrorOccurred                            = "An unknown error occurred: ";
const char* const kStringErrorFailedToLaunchVersionFileDownloader             = "Failed to launch the Radeon Tools Download Assistant (rtda).";
const char* const kStringErrorFailedToLaunchVersionFileDownloaderUnknownError = "Failed to launch the Radeon Tools Download Assistant (rtda) due to an unknown error: ";
const char* const kStringErrorFailedToLoadLatestReleaseInformation            = "Failed to load latest release information.";
const char* const kStringErrorFailedToDownloadVersionFile                     = "Failed to download version file.";
const char* const kStringErrorFailedToLoadVersionFile                         = "Failed to load version file.";
const char* const kStringErrorDownloadedAnEmptyVersionFile                    = "Downloaded an empty version file.";
const char* const kStringFailedToParseVersionFile                             = "Failed to parse version file.";
const char* const kStringErrorUnsupportedSchemaVersion =
    "The schema version of the version file is not supported; latest supported version is " CURRENT_SCHEMA_VERSION ".";
const char* const kStringErrorUnableToConvert1_5To1_6 = "Unable to convert from UpdateCheckApi Schema 1.5 to 1.6.";
const char* const kStringErrorUnableToConvert1_3To1_6 = "Unable to convert from UpdateCheckApi Schema 1.3 to 1.6.";

// Error Messages related to missing or invalid JSON tags.
#define VERSION_FILE_IS_MISSING_THE "The version file is missing the "
#define ENTRY " entry. "

const char* const kStringErrorVersionFileIsMissingTheSchemaVersionEntry                = VERSION_FILE_IS_MISSING_THE SCHEMAVERSION ENTRY;
const char* const kStringErrorVersionFileIsMissingTheReleasesEntry                     = VERSION_FILE_IS_MISSING_THE RELEASES ENTRY;
const char* const kStringErrorVersionFileIsMissingTheVersionStringEntry                = VERSION_FILE_IS_MISSING_THE VERSIONSTRING ENTRY;
const char* const kStringErrorVersionFileIsMissingTheReleaseDateEntry                  = VERSION_FILE_IS_MISSING_THE RELEASEDATE ENTRY;
const char* const kStringErrorVersionFileIsMissingTheDescriptionEntry                  = VERSION_FILE_IS_MISSING_THE DESCRIPTION ENTRY;
const char* const kStringErrorVersionFileIsMissingTheInfoPageUrlEntry                  = VERSION_FILE_IS_MISSING_THE INFOPAGEURL ENTRY;
const char* const kStringErrorVersionFileIsMissingTheDownloadUrlEntry                  = VERSION_FILE_IS_MISSING_THE DOWNLOADURL ENTRY;
const char* const kStringErrorVersionFileIsMissingTheReleaseVersionEntry               = VERSION_FILE_IS_MISSING_THE RELEASEVERSION ENTRY;
const char* const kStringErrorVersionFileIsMissingTheReleaseDescriptionEntry           = VERSION_FILE_IS_MISSING_THE RELEASEDESCRIPTION ENTRY;
const char* const kStringErrorVersionFileIsMissingTheReleaseTagsEntry                  = VERSION_FILE_IS_MISSING_THE RELEASETAGS ENTRY;
const char* const kStringErrorVersionFileIsMissingTheReleaseTitleEntry                 = VERSION_FILE_IS_MISSING_THE RELEASETITLE ENTRY;
const char* const kStringErrorVersionFileIsMissingTheReleaseTypeEntry                  = VERSION_FILE_IS_MISSING_THE RELEASETYPE ENTRY;
const char* const kStringErrorVersionFileIsMissingTheReleasePlatformsEntry             = VERSION_FILE_IS_MISSING_THE RELEASEPLATFORMS ENTRY;
const char* const kStringErrorVersionFileIsMissingTheInfoPageLinksEntry                = VERSION_FILE_IS_MISSING_THE INFOPAGELINKS ENTRY;
const char* const kStringErrorVersionFileIsMissingTheDownloadLinksEntry                = VERSION_FILE_IS_MISSING_THE DOWNLOADLINKS ENTRY;
const char* const kStringErrorVersionFileIsMissingTheDownloadLinksUrlEntry             = VERSION_FILE_IS_MISSING_THE DOWNLOADLINKS_URL ENTRY;
const char* const kStringErrorVersionFileIsMissingTheDownloadLinksTargetPlatformsEntry = VERSION_FILE_IS_MISSING_THE DOWNLOADLINKS_TARGETPLATFORMS ENTRY;
const char* const kStringErrorVersionFileIsMissingTheDownloadLinksPackageTypeEntry     = VERSION_FILE_IS_MISSING_THE DOWNLOADLINKS_PACKAGETYPE ENTRY;
const char* const kStringErrorVersionFileIsMissingTheDownloadLinksReleaseTypeEntry     = VERSION_FILE_IS_MISSING_THE DOWNLOADLINKS_RELEASETYPE ENTRY;

const char* const kStringErrorInvalidVersionNumberProvided                    = "The version file contains an invalid " RELEASEVERSION " number. ";
const char* const kStringErrorVersionFileContainsAnEmptyReleasesList          = "The version file contains an empty " RELEASES " list. ";
const char* const kStringErrorVersionFileContainsAnEmptyInfoPageUrlList       = "The version file contains an empty " INFOPAGEURL " list. ";
const char* const kStringErrorVersionFileContainsAnIncompleteInfoPageUrlEntry = "The version file contains an incomplete " INFOPAGEURL " entry. ";
const char* const kStringErrorVersionFileContainsAnEmptyDownloadUrlList       = "The version file contains an empty " DOWNLOADURL " list. ";
const char* const kStringErrorVersionFileContainsAnIncompleteDownloadUrlEntry = "The version file contains an incomplete " DOWNLOADURL " entry. ";
const char* const kStringErrorVersionFileContainsAnInvalidDownloadUrlTargetInfoValue =
    "The version file contains an invalid " DOWNLOADURL_TARGETINFO " value. ";

const char* const kStringErrorVersionFileContainsAnEmptyDownloadLinksTargetPlatformsList =
    "The version file contains an empty " DOWNLOADLINKS_TARGETPLATFORMS " list. ";
const char* const kStringErrorVersionFileContainsAnInvalidDownloadLinksTargetPlatformValue =
    "The version file contains an invalid " DOWNLOADLINKS_TARGETPLATFORMS " value. ";
const char* const kStringErrorVersionFileContainsAnInvalidDownloadLinksReleaseTypeValue =
    "The version file contains an invalid " DOWNLOADLINKS_RELEASETYPE " value. ";
const char* const kStringErrorVersionFileContainsAnInvalidDownloadLinksPackageTypeValue =
    "The version file contains an invalid " DOWNLOADLINKS_PACKAGETYPE " value. ";

const char* const kStringErrorVersionFileContainsAnEmptyReleasePlatformsList    = "The version file contains an empty " RELEASEPLATFORMS " list. ";
const char* const kStringErrorVersionFileContainsAnInvalidReleasePlatformsValue = "The version file contains an invalid " RELEASEPLATFORMS " value. ";
const char* const kStringErrorVersionFileContainsAnInvalidReleaseTypeValue      = "The version file contains an invalid " RELEASETYPE " value. ";
const char* const kStringErrorVersionFileContainsAnEmptyInfoPageLinksList       = "The version file contains an empty " INFOPAGELINKS " list. ";
const char* const kStringErrorVersionFileContainsAnIncompleteInfoPageLinksEntry = "The version file contains an incomplete " INFOPAGELINKS " entry. ";
const char* const kStringErrorVersionFileContainsAnEmptyDownloadLinksList       = "The version file contains an empty " DOWNLOADLINKS " list. ";
const char* const kStringErrorVersionFileContainsAnIncompleteDownloadLinksEntry = "The version file contains an incomplete " DOWNLOADLINKS " entry. ";

#endif  // UPDATECHECKAPI_UPDATE_CHECK_API_STRINGS_H_
