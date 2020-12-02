//==============================================================================
/// Copyright (c) 2018-2020 Advanced Micro Devices, Inc. All rights reserved.
/// \author AMD Developer Tools Team
/// \brief An API for checking for updates to an application.
//==============================================================================
#include "../Include/UpdateCheckApi.h"
#include "../Include/UpdateCheckApiStrings.h"
#include "../Include/UpdateCheckApiUtils.h"

#ifdef _WIN32
#pragma warning(push)
#pragma warning(disable : 4127)  // warning C4127: conditional expression is constant
#endif

#include "../Ext/json-3.2.0/json.hpp"

#ifdef _WIN32
#pragma warning(pop)
#endif

#include <assert.h>
#include <sstream>
#include <fstream>

#ifdef WIN32
#define updater_sscanf sscanf_s
#else // Linux
#define updater_sscanf std::sscanf
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#endif

#ifndef UNREFERENCED_PARAMETER
#define UNREFERENCED_PARAMETER(var) ((void)(var))
#endif

using namespace UpdateCheck;
using json = nlohmann::json;

const int NEWER = 1;
const int OLDER = -1;
const int EQUAL = 0;

VersionInfo UpdateCheck::GetApiVersionInfo()
{
    VersionInfo version;
    version.m_major = UPDATECHECKAPI_MAJOR;
    version.m_minor = UPDATECHECKAPI_MINOR;
    version.m_patch = UPDATECHECKAPI_PATCH;
    version.m_build = UPDATECHECKAPI_BUILD;

    return version;
}

/// Helper function to execute the Radeon Tools Download Assistant.
static bool ExecDownloader(const std::string& remoteURL, const std::string& localFile, std::string& errorMessage)
{
    bool wasLaunched = false;

    try
    {
        bool cancelSignal = false;
        std::string cmdOutput;

        // Setup the cmd line.
        std::string cmdLine = STR_DOWNLOADER_APPLICATION;
        cmdLine += " \"";
        cmdLine += remoteURL;
        cmdLine += "\" ";
        cmdLine += localFile;

        // Download the file.
        wasLaunched = UpdateCheckApiUtils::ExecAndGrabOutput(cmdLine.c_str(), cancelSignal, cmdOutput);

        if (!wasLaunched)
        {
            errorMessage.append(STR_ERROR_FAILED_TO_LAUNCH_VERSION_FILE_DOWNLOADER);
        }
    }
    catch(std::exception& e)
    {
        wasLaunched = false;
        errorMessage.append(STR_ERROR_FAILED_TO_LAUNCH_VERSION_FILE_DOWNLOADER_UNKNOWN_ERROR);
        errorMessage.append(e.what());
    }

    return wasLaunched;
}

/// Helper function to load a json file from disk.
/// \return true on success; jsonString will have the contents of the file at jsonFileUrl.
/// \return false on failure; jsonString will be unchanged.
static bool LoadJsonFile(const std::string& jsonFilePath, std::string& jsonString, std::string& errorMessage)
{
    bool isLoaded = true;
    jsonString.clear();

    // Open the file.
    std::ifstream readFile(jsonFilePath.c_str());
    if (!readFile.good())
    {
        isLoaded = false;
        errorMessage.append(STR_ERROR_FAILED_TO_LOAD_VERSION_FILE);
    }
    else
    {
        // Copy the file contents into the string.
        jsonString.assign((std::istreambuf_iterator<char>(readFile)),
            std::istreambuf_iterator<char>());

        readFile.close();

        // Consider it loaded if the JSON string is not empty.
        if (jsonString.empty())
        {
            isLoaded = false;
            errorMessage.append(STR_ERROR_DOWNLOADED_AN_EMPTY_VERSION_FILE);
        }
        else
        {
            isLoaded = true;
        }
    }

    return isLoaded;
}

/// Helper function to download JSON file.
/// \return true on success; jsonString will have the contents of the file at jsonFileUrl.
/// \return false on failure; jsonString will be unchanged.
static bool DownloadJsonFile(const std::string jsonFileUrl, std::string& jsonString, std::string& errorMessage)
{
    bool isLoaded = true;
    jsonString.clear();

    std::string localFile;
    if (!UpdateCheckApiUtils::GetTempDirectory(localFile))
    {
        isLoaded = false;
    }
    else
    {
        assert(localFile.size() != 0);
        localFile += "/";

        size_t posSlash = jsonFileUrl.find_last_of("/\\");
        size_t posEnd = jsonFileUrl.find_last_of("?");
        if (posEnd == std::string::npos)
        {
            posEnd = jsonFileUrl.size();
        }
        else
        {
            // Update the position to NOT include the character.
            posEnd = posEnd - 1;
        }

        if (posSlash != std::string::npos)
        {
            localFile += jsonFileUrl.substr(posSlash + 1, posEnd - posSlash);
        }
        else
        {
            localFile += jsonFileUrl.substr(0, posEnd);
        }

        // Attempt to delete the local file before downloading the new one.
        std::remove(localFile.c_str());

        // Download the JSON file.
        if (!ExecDownloader(jsonFileUrl, localFile, errorMessage))
        {
            isLoaded = false;
        }
        else
        {
            isLoaded = LoadJsonFile(localFile, jsonString, errorMessage);
        }
    }

    return isLoaded;
}

/// Given a JSON list of release assets, finds the named asset and returns the associated JSON element.
static bool FindAssetByName(json& assetList, const std::string& assetName, json& assetElement)
{
    bool wasAssetFound = false;

    // Check each asset for the desired asset name.
    for (json::iterator assetIter = assetList.begin(); assetIter != assetList.end(); ++assetIter)
    {
        auto assetNameElement = assetIter->find(TAG_ASSET_NAME);

        if (assetNameElement != assetIter->end())
        {
            wasAssetFound = (0 == assetNameElement->get<std::string>().compare(assetName));
            if (wasAssetFound)
            {
                assetElement = *assetIter;
                break;
            }
        }
    }

    return wasAssetFound;
}

/// Finds the asset based on its filename and returns the URL from which to download the asset.
static bool FindAssetDownloadUrl(json& latestRelease, const std::string& assetName, std::string& assetDownloadUrl, std::string& errorMessage)
{
    bool hasAssetDownloadUrl = false;

    auto assetsElement = latestRelease.find(TAG_ASSETS);

    if (assetsElement == latestRelease.end())
    {
        errorMessage.append(STR_ERROR_MISSING_ASSETS_TAG);
    }
    else
    {
        json& assetsList = *assetsElement;
        json assetElement;
        bool doesAssetExist = FindAssetByName(assetsList, assetName, assetElement);

        if (!doesAssetExist)
        {
            errorMessage.append(STR_ERROR_ASSET_NOT_FOUND);
        }
        else
        {
            auto assetUrl = assetElement.find(TAG_ASSET_BROWSWERDOWNLOADURL);
            if (assetUrl == assetElement.end())
            {
                errorMessage.append(STR_ERROR_DOWNLOAD_URL_NOT_FOUND_IN_ASSET);
            }
            else
            {
                assetDownloadUrl = assetUrl->get<std::string>().c_str();
                hasAssetDownloadUrl = true;
            }
        }
    }

    return hasAssetDownloadUrl;
}

/// Helper function to load JSON file from the latest release of a GitHub Repository.
/// \return true on success; jsonString will have the contents of the file at jsonFileUrl.
/// \return false on failure; jsonString will be unchanged.
static bool LoadJsonFromLatestRelease(const std::string jsonFileUrl, const std::string jsonFileName, std::string& jsonString, std::string& errorMessage)
{
    bool wasLoaded = false;

    // Build a path to a temporary file.
    std::string latestReleaseApiTempFile;
    if (!UpdateCheckApiUtils::GetTempDirectory(latestReleaseApiTempFile))
    {
        errorMessage.append(STR_ERROR_UNABLE_TO_FIND_TEMP_DIRECTORY);
    }
    else
    {
        latestReleaseApiTempFile += "/";
        latestReleaseApiTempFile += STR_LATEST_JSON_FILENAME;

        if (ExecDownloader(jsonFileUrl, latestReleaseApiTempFile, errorMessage))
        {
            try
            {
                std::string latestReleaseJson;
                if (LoadJsonFile(latestReleaseApiTempFile, latestReleaseJson, errorMessage))
                {
                    // Parsing the json can throw an exception if the string is
                    // not valid json. This can happen in networks that limit
                    // internet access, and result in downloading an html page.
                    json latestReleaseJsonDoc = json::parse(latestReleaseJson);

                    std::string versionFileUrl;
                    if (FindAssetDownloadUrl(latestReleaseJsonDoc, jsonFileName, versionFileUrl, errorMessage))
                    {
                        wasLoaded = DownloadJsonFile(versionFileUrl, jsonString, errorMessage);
                    }
                    else
                    {
                        // Failed to find the Asset, so check for a "message" tag which may indicate an error from the GitHub Release API.
                        auto messageElement = latestReleaseJsonDoc.find(TAG_MESSAGE);

                        if (messageElement != latestReleaseJsonDoc.end())
                        {
                            errorMessage.append(messageElement->get<std::string>().c_str());
                        }
                    }
                }
            }
            catch (...)
            {
                wasLoaded = false;
                errorMessage.append(STR_ERROR_FAILED_TO_LOAD_LATEST_RELEASE_INFORMATION);
            }
        }
    }

    return wasLoaded;
}

/// Convert VersionInfo to a string.
std::string VersionInfo::ToString() const
{
    std::stringstream versionString;
    versionString << m_major << STR_VERSION_DELIMITER << m_minor << STR_VERSION_DELIMITER << m_patch << STR_VERSION_DELIMITER << m_build;
    return versionString.str();
}

/// Compare 'this' version to the supplied version.
/// \return NEWER if 'this' version is newer than the otherVersion.
/// \return OLDER if 'this' version is older than the otherVersion.
/// \return EQUAL if 'this' version is equal to the otherVersion.
int VersionInfo::Compare(const VersionInfo& otherVersion) const
{
    int comparison = 0;

    // Compare Major version.
    if (m_major > otherVersion.m_major)
    {
        comparison = NEWER;
    }
    else if (m_major < otherVersion.m_major)
    {
        comparison = OLDER;
    }
    else
    {
        // Compare Minor version.
        if (m_minor > otherVersion.m_minor)
        {
            comparison = NEWER;
        }
        else if (m_minor < otherVersion.m_minor)
        {
            comparison = OLDER;
        }
        else
        {
            // Compare Patch version.
            if (m_patch > otherVersion.m_patch)
            {
                comparison = NEWER;
            }
            else if (m_patch < otherVersion.m_patch)
            {
                comparison = OLDER;
            }
            else
            {
                // Compare Build version.
                if (m_build > otherVersion.m_build)
                {
                    comparison = NEWER;
                }
                else if (m_build < otherVersion.m_build)
                {
                    comparison = OLDER;
                }
                else
                {
                    // Version numbers are the same.
                    comparison = EQUAL;
                }
            }
        }
    }

    return comparison;
}

/// Split the version string from Schema 1.3 into Major, Minor, Patch, and Build numbers.
static bool SplitVersionString_1_3(const std::string& version, VersionInfo& versionInfo, std::string& errorMessage)
{
    bool isValid = true;

    // If the version string is empty, it is invalid.
    if (version.empty())
    {
        isValid = false;
    }
    else
    {
        // Version format: "Major.Minor.Patch.Build".
        versionInfo.m_major = 0;
        versionInfo.m_minor = 0;
        versionInfo.m_patch = 0;
        versionInfo.m_build = 0;
        int numVersionParts = updater_sscanf(version.c_str(), "%u" STR_VERSION_DELIMITER "%u" STR_VERSION_DELIMITER "%u" STR_VERSION_DELIMITER "%u", &versionInfo.m_major, &versionInfo.m_minor, &versionInfo.m_patch, &versionInfo.m_build);

        if (numVersionParts == EOF ||
            numVersionParts <= 0 ||
            numVersionParts > 4)
        {
            isValid = false;
            errorMessage.append(STR_ERROR_INVALID_VERSION_NUMBER_PROVIDED);
        }
    }

    return isValid;
}

/// Split the version string from Schema 1.5 into Major, Minor, Patch, and Build numbers.
static bool SplitVersionString_1_5(json& jsonDoc, VersionInfo& versionInfo, std::string& errorMessage)
{
    bool isValid = true;

    // Attempt to parse the version string.
    auto jsonMajorVersion = jsonDoc.find(RELEASEVERSION_MAJOR);
    auto jsonMinorVersion = jsonDoc.find(RELEASEVERSION_MINOR);
    auto jsonPatchVersion = jsonDoc.find(RELEASEVERSION_PATCH);
    auto jsonBuildVersion = jsonDoc.find(RELEASEVERSION_BUILD);

    if (jsonMajorVersion == jsonDoc.end() &&
        jsonMinorVersion == jsonDoc.end() &&
        jsonPatchVersion == jsonDoc.end() &&
        jsonBuildVersion == jsonDoc.end())
    {
        isValid = false;
        errorMessage.append(STR_ERROR_INVALID_VERSION_NUMBER_PROVIDED);
    }
    else
    {
        if (jsonMajorVersion != jsonDoc.end())
        {
            versionInfo.m_major = jsonMajorVersion->get<uint32_t>();
        }

        if (jsonMinorVersion != jsonDoc.end())
        {
            versionInfo.m_minor = jsonMinorVersion->get<uint32_t>();
        }

        if (jsonPatchVersion != jsonDoc.end())
        {
            versionInfo.m_patch = jsonPatchVersion->get<uint32_t>();
        }

        if (jsonBuildVersion != jsonDoc.end())
        {
            versionInfo.m_build = jsonBuildVersion->get<uint32_t>();
        }
    }

    return isValid;
}

/// Translate the package string to the corresponding UpdatePackageType.
/// \return True if the packageString is recognized and a valid UpdatePackageType is set.
/// \return False if the packageString is not recognized.
static bool GetPackageType_1_3(const std::string& packageString, TargetPlatform& platform, PackageType& packageType)
{
    bool isKnownType = true;

    if (packageString.compare(STR_PACKAGETYPE_WINDOWS_ZIP) == 0)
    {
        platform = TargetPlatform::Windows;
        packageType = PackageType::ZIP;
    }
    else if (packageString.compare(STR_PACKAGETYPE_WINDOWS_MSI) == 0)
    {
        platform = TargetPlatform::Windows;
        packageType = PackageType::MSI;
    }
    else if (packageString.compare(STR_PACKAGETYPE_LINUX_TAR) == 0)
    {
        platform = TargetPlatform::Ubuntu;
        packageType = PackageType::TAR;
    }
    else if (packageString.compare(STR_PACKAGETYPE_LINUX_RPM) == 0)
    {
        platform = TargetPlatform::Ubuntu;
        packageType = PackageType::RPM;
    }
    else if (packageString.compare(STR_PACKAGETYPE_LINUX_DEBIAN) == 0)
    {
        platform = TargetPlatform::Ubuntu;
        packageType = PackageType::Debian;
    }
    else
    {
        platform = TargetPlatform::Unknown;
        packageType = PackageType::Unknown;
        isKnownType = false;
    }

    return isKnownType;
}

/// This structure represents an update package from Schema 1.5.
struct UpdatePackage_1_5
{
    /// The URL from which the archive/installer can be downloaded.
    std::string m_url;

    /// A value describing the kind of archive/installer that m_url points
    /// to. Products will filter irrelevant download links basaed on this value.
    PackageType m_packageType;

    /// The type of the release for the package that is referenced by this link.
    ReleaseType m_releaseType;

    /// The target platforms to which the packge referenced by this link is relevant.
    std::vector<TargetPlatform> m_targetPlatforms;
};

/// This structure contains the information received checking the
/// availability of product updates from Schema 1.5.
struct UpdateInfo_1_5
{
    /// True if an update to a newer version is available, false otherwise.
    bool m_isUpdateAvailable;

    /// The version of the available update.
    VersionInfo m_releaseVersion;

    /// The release date of the available update in the format YYYY-MM-DD.
    std::string m_releaseDate;

    /// Text describing the available update, may include release notes highlights for example.
    std::string m_releaseDescription;

    /// The available update packages: information and URL from which they can be downloaded.
    std::vector<UpdatePackage_1_5> m_availablePackages;

    /// Links to relevant pages, like product landing page on GPUOpen, or Releases page on Github.com.
    std::vector<InfoPageLink> m_infoLinks;
};

/// Parse JSON string for Schema 1.3 directly into the structures for schema 1.5.
static bool ParseJsonSchema_1_3(json& jsonDoc, UpdateInfo_1_5& updateInfo, std::string& errorMessage)
{
    bool isParsed = true;

    // Extract VersionString.
    if (jsonDoc.find(VERSIONSTRING) == jsonDoc.end())
    {
        isParsed = false;
        errorMessage.append(STR_ERROR_VERSION_FILE_IS_MISSING_THE_VERSIONSTRING_ENTRY);
    }
    else
    {
        // Attempt to parse the version string.
        if (!SplitVersionString_1_3(jsonDoc[VERSIONSTRING].get<std::string>(), updateInfo.m_releaseVersion, errorMessage))
        {
            isParsed = false;
        }
    }

    // Extract ReleaseDate.
    if (jsonDoc.find(RELEASEDATE) == jsonDoc.end())
    {
        isParsed = false;
        errorMessage.append(STR_ERROR_VERSION_FILE_IS_MISSING_THE_RELEASEDATE_ENTRY);
    }
    else
    {
        updateInfo.m_releaseDate = jsonDoc[RELEASEDATE].get<std::string>();
    }

    // Extract Description.
    if (jsonDoc.find(DESCRIPTION) == jsonDoc.end())
    {
        isParsed = false;
        errorMessage.append(STR_ERROR_VERSION_FILE_IS_MISSING_THE_DESCRIPTION_ENTRY);
    }
    else
    {
        updateInfo.m_releaseDescription = jsonDoc[DESCRIPTION].get<std::string>();
    }

    // Extract InfoPageLink.
    if (jsonDoc.find(INFOPAGEURL) == jsonDoc.end())
    {
        isParsed = false;
        errorMessage.append(STR_ERROR_VERSION_FILE_IS_MISSING_THE_INFOPAGEURL_ENTRY);
    }
    else
    {
        json jsonInfoPageObject = jsonDoc[INFOPAGEURL];
        if (jsonInfoPageObject.empty())
        {
            isParsed = false;
            errorMessage.append(STR_ERROR_VERSION_FILE_CONTAINS_AN_EMPTY_INFOPAGEURL_LIST);
        }
        else
        {
            for (json::iterator infoPageIter = jsonInfoPageObject.begin(); infoPageIter != jsonInfoPageObject.end(); ++infoPageIter)
            {
                if (infoPageIter->find(INFOPAGEURL_URL) != infoPageIter->end() &&
                    infoPageIter->find(INFOPAGEURL_DESCRIPTION) != infoPageIter->end())
                {
                    InfoPageLink infoPage;
                    infoPage.m_url = (*infoPageIter)[INFOPAGEURL_URL].get<std::string>();
                    infoPage.m_pageDescription = (*infoPageIter)[INFOPAGEURL_DESCRIPTION].get<std::string>();
                    updateInfo.m_infoLinks.push_back(infoPage);
                }
                else
                {
                    isParsed = false;
                    errorMessage.append(STR_ERROR_VERSION_FILE_CONTAINS_AN_INCOMPLETE_INFOPAGEURL_ENTRY);
                }
            }
        }
    }

    // Extract DownloadURL.
    if (jsonDoc.find(DOWNLOADURL) == jsonDoc.end())
    {
        isParsed = false;
        errorMessage.append(STR_ERROR_VERSION_FILE_IS_MISSING_THE_DOWNLOADURL_ENTRY);
    }
    else
    {
        json jsonDownloadUrlObject = jsonDoc[DOWNLOADURL];
        if (jsonDownloadUrlObject.empty())
        {
            isParsed = false;
            errorMessage.append(STR_ERROR_VERSION_FILE_CONTAINS_AN_EMPTY_DOWNLOADURL_LIST);
        }
        else
        {
            for (json::iterator downloadLinkIter = jsonDownloadUrlObject.begin(); downloadLinkIter != jsonDownloadUrlObject.end(); ++downloadLinkIter)
            {
                if (downloadLinkIter->find(DOWNLOADURL_URL) != downloadLinkIter->end() &&
                    downloadLinkIter->find(DOWNLOADURL_TARGETINFO) != downloadLinkIter->end())
                {
                    UpdatePackage_1_5 updatePackage;

                    std::string targetString = (*downloadLinkIter)[DOWNLOADURL_TARGETINFO].get<std::string>();

                    // Make sure the targetString is a valid target.
                    PackageType packageType = PackageType::Unknown;
                    TargetPlatform platform = TargetPlatform::Unknown;
                    if (!GetPackageType_1_3(targetString, platform, packageType))
                    {
                        isParsed = false;
                        errorMessage.append(STR_ERROR_VERSION_FILE_CONTAINS_AN_INVALID_DOWNLOADURL_TARGETINFO_VALUE);
                    }
                    else
                    {
                        updatePackage.m_url = (*downloadLinkIter)[DOWNLOADURL_URL].get<std::string>();
                        updatePackage.m_packageType = packageType;
                        updatePackage.m_targetPlatforms.push_back(platform);

                        // Schema 1.3 does not have a field to represent the release type, so default to GA
                        // since it can be assumed that everything at that point was a GA release.
                        updatePackage.m_releaseType = ReleaseType::GA;

                        updateInfo.m_availablePackages.push_back(updatePackage);
                    }
                }
                else
                {
                    isParsed = false;
                    errorMessage.append(STR_ERROR_VERSION_FILE_CONTAINS_AN_INCOMPLETE_DOWNLOADURL_ENTRY);
                }
            }
        }
    }

    return isParsed;
}

/// Translate the package string to the corresponding UpdatePackageType.
/// \return True if the packageString is recognized and a valid UpdatePackageType is set.
/// \return False if the packageString is not recognized.
static bool GetPackageType_1_5(const std::string& packageString, PackageType& packageType)
{
    bool isKnownType = true;

    if (packageString.compare(STR_PACKAGETYPE_ZIP) == 0)
    {
        packageType = PackageType::ZIP;
    }
    else if (packageString.compare(STR_PACKAGETYPE_MSI) == 0)
    {
        packageType = PackageType::MSI;
    }
    else if (packageString.compare(STR_PACKAGETYPE_TAR) == 0)
    {
        packageType = PackageType::TAR;
    }
    else if (packageString.compare(STR_PACKAGETYPE_RPM) == 0)
    {
        packageType = PackageType::RPM;
    }
    else if (packageString.compare(STR_PACKAGETYPE_DEBIAN) == 0)
    {
        packageType = PackageType::Debian;
    }
    else
    {
        packageType = PackageType::Unknown;
        isKnownType = false;
    }

    return isKnownType;
}

/// Translate the releaseTypeString to the corresponding ReleaseType enum value.
/// \return True if the releaseTypeString is recognized and a valid UpdatePackageType is set.
/// \return False if the releaseTypeString is not recognized.
static bool GetReleaseType_1_5(const std::string& releaseTypeString, ReleaseType& releaseType)
{
    bool isKnownType = true;

    if (releaseTypeString.compare(STR_RELEASETYPE_GA) == 0)
    {
        releaseType = ReleaseType::GA;
    }
    else if (releaseTypeString.compare(STR_RELEASETYPE_BETA) == 0)
    {
        releaseType = ReleaseType::Beta;
    }
    else if (releaseTypeString.compare(STR_RELEASETYPE_ALPHA) == 0)
    {
        releaseType = ReleaseType::Alpha;
    }
    else if (releaseTypeString.compare(STR_RELEASETYPE_PATCH) == 0)
    {
        releaseType = ReleaseType::Patch;
    }
    else if (releaseTypeString.compare(STR_RELEASETYPE_DEVELOPMENT) == 0)
    {
        releaseType = ReleaseType::Development;
    }
    else
    {
        releaseType = ReleaseType::Unknown;
        isKnownType = false;
    }

    return isKnownType;
}

/// Translate the target platforms JSON object to the corresponding TargetPlatforms list.
/// \return True if all the platforms are recognized.
/// \return False if any platform is not recognized.
static bool GetTargetPlatform_1_5(json& targetPlatformsJson, std::vector<TargetPlatform>& platforms, std::string& errorMessage)
{
    bool isKnownType = false;

    if (targetPlatformsJson.empty())
    {
        errorMessage.append(STR_ERROR_VERISON_FILE_CONTAINS_AN_EMPTY_DOWNLOADLINKS_TARGETPLATFORMS_LIST);
    }
    else
    {
        for (json::iterator platform = targetPlatformsJson.begin(); platform != targetPlatformsJson.end(); ++platform)
        {
            std::string platformString = platform->get<std::string>();

            if (platformString.compare(STR_PLATFORMTYPE_WINDOWS) == 0)
            {
                platforms.push_back(TargetPlatform::Windows);
                isKnownType = true;
            }
            else if (platformString.compare(STR_PLATFORMTYPE_UBUNTU) == 0)
            {
                platforms.push_back(TargetPlatform::Ubuntu);
                isKnownType = true;
            }
            else if (platformString.compare(STR_PLATFORMTYPE_RHEL) == 0)
            {
                platforms.push_back(TargetPlatform::RHEL);
                isKnownType = true;
            }
            else if (platformString.compare(STR_PLATFORMTYPE_DARWIN) == 0)
            {
                platforms.push_back(TargetPlatform::Darwin);
                isKnownType = true;
            }
            else
            {
                isKnownType = false;
                errorMessage.append(STR_ERROR_VERSION_FILE_CONTAINS_AN_INVALID_DOWNLOADLINKS_TARGETPLATFORM_VALUE);
                break;
            }
        }
    }

    return isKnownType;
}

/// Parse a JSON string that should should be formatted as Schema 1.6.
static bool ParseJsonSchema_1_5(json& jsonDoc, UpdateInfo_1_5& updateInfo, std::string& errorMessage)
{
    bool isParsed = true;

    // Extract ReleaseVersion.
    auto jsonVersionIter = jsonDoc.find(RELEASEVERSION);
    if (jsonVersionIter == jsonDoc.end())
    {
        isParsed = false;
        errorMessage.append(STR_ERROR_VERSION_FILE_IS_MISSING_THE_RELEASEVERSION_ENTRY);
    }
    else
    {
        if (!SplitVersionString_1_5(*jsonVersionIter, updateInfo.m_releaseVersion, errorMessage))
        {
            isParsed = false;
        }
    }

    // Extract ReleaseDate.
    auto jsonDateIter = jsonDoc.find(RELEASEDATE);
    if (jsonDateIter == jsonDoc.end())
    {
        isParsed = false;
        errorMessage.append(STR_ERROR_VERSION_FILE_IS_MISSING_THE_RELEASEDATE_ENTRY);
    }
    else
    {
        updateInfo.m_releaseDate = jsonDateIter->get<std::string>();
    }

    // Extract Description.
    auto jsonDescriptionIter = jsonDoc.find(RELEASEDESCRIPTION);
    if (jsonDescriptionIter == jsonDoc.end())
    {
        isParsed = false;
        errorMessage.append(STR_ERROR_VERSION_FILE_IS_MISSING_THE_RELEASEDESCRIPTION_ENTRY);
    }
    else
    {
        updateInfo.m_releaseDescription = jsonDescriptionIter->get<std::string>();
    }

    // Extract InfoPageLinks.
    auto jsonInfoIter = jsonDoc.find(INFOPAGELINKS);
    if (jsonInfoIter == jsonDoc.end())
    {
        isParsed = false;
        errorMessage.append(STR_ERROR_VERSION_FILE_IS_MISSING_THE_INFOPAGELINKS_ENTRY);
    }
    else
    {
        json& jsonInfoPageObject = *jsonInfoIter;

        if (jsonInfoPageObject.empty())
        {
            isParsed = false;
            errorMessage.append(STR_ERROR_VERSION_FILE_CONTAINS_AN_EMPTY_INFOPAGELINKS_LIST);
        }
        else
        {
            for (json::iterator infoPageIter = jsonInfoPageObject.begin(); infoPageIter != jsonInfoPageObject.end(); ++infoPageIter)
            {
                if (infoPageIter->find(INFOPAGELINKS_URL) != infoPageIter->end() &&
                    infoPageIter->find(INFOPAGELINKS_DESCRIPTION) != infoPageIter->end())
                {
                    InfoPageLink infoPage;
                    infoPage.m_url = (*infoPageIter)[INFOPAGELINKS_URL].get<std::string>();
                    infoPage.m_pageDescription = (*infoPageIter)[INFOPAGELINKS_DESCRIPTION].get<std::string>();
                    updateInfo.m_infoLinks.push_back(infoPage);
                }
                else
                {
                    isParsed = false;
                    errorMessage.append(STR_ERROR_VERSION_FILE_CONTAINS_AN_INCOMPLETE_INFOPAGELINKS_ENTRY);
                }
            }
        }
    }

    // Extract DownloadLinks.
    if (jsonDoc.find(DOWNLOADLINKS) == jsonDoc.end())
    {
        isParsed = false;
        errorMessage.append(STR_ERROR_VERSION_FILE_IS_MISSING_THE_DOWNLOADLINKS_ENTRY);
    }
    else
    {
        json jsonDownloadUrlObject = jsonDoc[DOWNLOADLINKS];

        if (jsonDownloadUrlObject.empty())
        {
            isParsed = false;
            errorMessage.append(STR_ERROR_VERSION_FILE_CONTAINS_AN_EMPTY_DOWNLOADLINKS_LIST);
        }
        else
        {
            for (json::iterator downloadLinkIter = jsonDownloadUrlObject.begin(); downloadLinkIter != jsonDownloadUrlObject.end(); ++downloadLinkIter)
            {
                auto urlIter = downloadLinkIter->find(DOWNLOADLINKS_URL);
                auto platformIter = downloadLinkIter->find(DOWNLOADLINKS_TARGETPLATFORMS);
                auto packageTypeIter = downloadLinkIter->find(DOWNLOADLINKS_PACKAGETYPE);
                auto releaseTypeIter = downloadLinkIter->find(DOWNLOADLINKS_RELEASETYPE);
                if (urlIter == downloadLinkIter->end())
                {
                    isParsed = false;
                    errorMessage.append(STR_ERROR_VERSION_FILE_IS_MISSING_THE_DOWNLOADLINKS_URL_ENTRY);
                }
                else if (platformIter == downloadLinkIter->end())
                {
                    isParsed = false;
                    errorMessage.append(STR_ERROR_VERSION_FILE_IS_MISSING_THE_DOWNLOADLINKS_TARGETPLATFORMS_ENTRY);
                }
                else if (packageTypeIter == downloadLinkIter->end())
                {
                    isParsed = false;
                    errorMessage.append(STR_ERROR_VERSION_FILE_IS_MISSING_THE_DOWNLOADLINKS_PACKAGETYPE_ENTRY);
                }
                else if (releaseTypeIter == downloadLinkIter->end())
                {
                    isParsed = false;
                    errorMessage.append(STR_ERROR_VERSION_FILE_IS_MISSING_THE_DOWNLOADLINKS_RELEASETYPE_ENTRY);
                }
                else
                {
                    UpdatePackage_1_5 updatePackage;

                    // Make sure the targetString is a valid target.
                    if (!GetReleaseType_1_5(releaseTypeIter->get<std::string>(), updatePackage.m_releaseType))
                    {
                        isParsed = false;
                        errorMessage.append(STR_ERROR_VERSION_FILE_CONTAINS_AN_INVALID_DOWNLOADLINKS_RELEASETYPE_VALUE);
                    }
                    else if (!GetPackageType_1_5(packageTypeIter->get<std::string>(), updatePackage.m_packageType))
                    {
                        isParsed = false;
                        errorMessage.append(STR_ERROR_VERSION_FILE_CONTAINS_AN_INVALID_DOWNLOADLINKS_PACKAGETYPE_VALUE);
                    }
                    else if (!GetTargetPlatform_1_5(*platformIter, updatePackage.m_targetPlatforms, errorMessage))
                    {
                        isParsed = false;
                    }
                    else
                    {
                        updatePackage.m_url = (*downloadLinkIter)[DOWNLOADLINKS_URL].get<std::string>();
                        updateInfo.m_availablePackages.push_back(updatePackage);
                    }
                }
            }
        }
    }

    return isParsed;
}

/// Convert from Schema 1.5 to 1.6.
static bool ConvertJsonSchema_1_5_To_1_6(const UpdateInfo_1_5& updateInfo_1_5, UpdateInfo& updateInfo)
{
    // The biggest difference between 1.5 and 1.6 is that 1.5 only had one release version,
    // which was composed of various downloads that could have been for various target platforms.
    // Contrast that with 1.6 which has multiple release versions (one per set of platforms).
    // Essentially, the target platforms from the bottom of the structure, to the top.
    //
    // The solution to fixing this is to find the first set of target platforms, then construct
    // a new ReleaseInfo struct to match this, and repeat for each set of target platforms.

    // Iterate through each UpdatePackage.
    for (auto packageIter = updateInfo_1_5.m_availablePackages.cbegin(); packageIter != updateInfo_1_5.m_availablePackages.cend(); ++packageIter)
    {
        ReleaseInfo* pReleaseInfo = nullptr;

        // Find an existing ReleaseInfo struct for the current set of target platforms and ReleaseType.
        for (auto releaseIter = updateInfo.m_releases.begin(); releaseIter != updateInfo.m_releases.end(); ++releaseIter)
        {
            if (releaseIter->m_targetPlatforms == packageIter->m_targetPlatforms &&
                releaseIter->m_type == packageIter->m_releaseType)
            {
                pReleaseInfo = &(*releaseIter);
            }
        }

        // If none of the existing releases match, then create a new one, and populate it
        // with the information that is easily transferable. Also, seed the new Tags field
        // with the target platforms and release type.
        if (pReleaseInfo == nullptr)
        {
            ReleaseInfo releaseInfo;

            releaseInfo.m_version = updateInfo_1_5.m_releaseVersion;
            releaseInfo.m_title = updateInfo_1_5.m_releaseDescription;
            releaseInfo.m_date = updateInfo_1_5.m_releaseDate;

            for (auto infoLinkIter = updateInfo_1_5.m_infoLinks.cbegin(); infoLinkIter != updateInfo_1_5.m_infoLinks.cend(); ++infoLinkIter)
            {
                releaseInfo.m_infoLinks.push_back(*infoLinkIter);
            }

            // Transfer over the two fields that make this a unique release (the set of target platforms, and release type).
            for (auto platformIter = packageIter->m_targetPlatforms.cbegin(); platformIter != packageIter->m_targetPlatforms.cend(); ++platformIter)
            {
                releaseInfo.m_targetPlatforms.push_back(*platformIter);
                releaseInfo.m_tags.push_back(TargetPlatformToString(*platformIter));
            }

            releaseInfo.m_type = packageIter->m_releaseType;
            releaseInfo.m_tags.push_back(ReleaseTypeToString(packageIter->m_releaseType));

            // Push this releaseInfo into the list, and set the release pointer.
            updateInfo.m_releases.push_back(releaseInfo);
            pReleaseInfo = &(updateInfo.m_releases[updateInfo.m_releases.size() - 1]);
        }

        assert(pReleaseInfo != nullptr);

        // Add the new DownloadLink from this package.
        if (pReleaseInfo != nullptr)
        {
            DownloadLink downloadLink;
            downloadLink.m_packageType = packageIter->m_packageType;
            downloadLink.m_url = packageIter->m_url;
            pReleaseInfo->m_downloadLinks.push_back(downloadLink);
        }
    }

    // At present, Schema 1.5 can always be converted to Schema 1.6.
    return true;
}

/// Translate the releaseTypeString to the corresponding ReleaseType enum value.
/// \return True if the releaseTypeString is recognized and a valid UpdatePackageType is set.
/// \return False if the releaseTypeString is not recognized.
static bool GetReleaseType_1_6(const std::string& releaseTypeString, ReleaseType& releaseType)
{
    // Remains unchanged from Schema 1.5.
    return GetReleaseType_1_5(releaseTypeString, releaseType);
}

/// Translate the target platforms JSON object to the corresponding TargetPlatforms list.
/// \return True if all the platforms are recognized.
/// \return False if any platform is not recognized.
static bool GetReleasePlatform_1_6(json& targetPlatformsJson, std::vector<TargetPlatform>& platforms, std::string& errorMessage)
{
    bool isKnownType = false;

    if (targetPlatformsJson.empty())
    {
        errorMessage.append(STR_ERROR_VERISON_FILE_CONTAINS_AN_EMPTY_RELEASEPLATFORMS_LIST);
    }
    else
    {
        for (json::iterator platform = targetPlatformsJson.begin(); platform != targetPlatformsJson.end(); ++platform)
        {
            std::string platformString = platform->get<std::string>();

            if (platformString.compare(STR_PLATFORMTYPE_WINDOWS) == 0)
            {
                platforms.push_back(TargetPlatform::Windows);
                isKnownType = true;
            }
            else if (platformString.compare(STR_PLATFORMTYPE_UBUNTU) == 0)
            {
                platforms.push_back(TargetPlatform::Ubuntu);
                isKnownType = true;
            }
            else if (platformString.compare(STR_PLATFORMTYPE_RHEL) == 0)
            {
                platforms.push_back(TargetPlatform::RHEL);
                isKnownType = true;
            }
            else if (platformString.compare(STR_PLATFORMTYPE_DARWIN) == 0)
            {
                platforms.push_back(TargetPlatform::Darwin);
                isKnownType = true;
            }
            else
            {
                isKnownType = false;
                errorMessage.append(STR_ERROR_VERSION_FILE_CONTAINS_AN_INVALID_RELEASEPLATFORMS_VALUE);
                break;
            }
        }
    }

    return isKnownType;
}

/// Translate the package string to the corresponding UpdatePackageType.
/// \return True if the packageString is recognized and a valid UpdatePackageType is set.
/// \return False if the packageString is not recognized.
static bool GetPackageType_1_6(const std::string& packageString, PackageType& packageType)
{
    // Remains unchanged from Schema 1.5.
    return GetPackageType_1_5(packageString, packageType);
}

/// Parse a JSON string that should should be formatted as Schema 1.6.
static bool ParseJsonSchema_1_6(json& jsonDoc, UpdateInfo& updateInfo, std::string& errorMessage)
{
    bool isParsed = true;

    // Extract Releases.
    auto jsonReleasesIter = jsonDoc.find(RELEASES);
    if (jsonReleasesIter == jsonDoc.end())
    {
        isParsed = false;
        errorMessage.append(STR_ERROR_VERSION_FILE_IS_MISSING_THE_RELEASES_ENTRY);
    }
    else if (jsonReleasesIter->empty())
    {
        isParsed = false;
        errorMessage.append(STR_ERROR_VERSION_FILE_CONTAINS_AN_EMPTY_RELEASES_LIST);
    }
    else
    {
        for (json::iterator releaseIter = jsonReleasesIter->begin(); releaseIter != jsonReleasesIter->end(); ++releaseIter)
        {
            ReleaseInfo releaseInfo;

            // Extract ReleaseVersion.
            auto jsonVersionIter = releaseIter->find(RELEASEVERSION);
            if (jsonVersionIter == releaseIter->end())
            {
                isParsed = false;
                errorMessage.append(STR_ERROR_VERSION_FILE_IS_MISSING_THE_RELEASEVERSION_ENTRY);
            }
            else
            {
                if (!SplitVersionString_1_5(*jsonVersionIter, releaseInfo.m_version, errorMessage))
                {
                    isParsed = false;
                }
            }

            // Extract ReleaseDate.
            auto jsonDateIter = releaseIter->find(RELEASEDATE);
            if (jsonDateIter == releaseIter->end())
            {
                isParsed = false;
                errorMessage.append(STR_ERROR_VERSION_FILE_IS_MISSING_THE_RELEASEDATE_ENTRY);
            }
            else
            {
                releaseInfo.m_date = jsonDateIter->get<std::string>();
            }

            // Extract ReleaseTitle.
            auto jsonTitleIter = releaseIter->find(RELEASETITLE);
            if (jsonTitleIter == releaseIter->end())
            {
                isParsed = false;
                errorMessage.append(STR_ERROR_VERSION_FILE_IS_MISSING_THE_RELEASETITLE_ENTRY);
            }
            else
            {
                releaseInfo.m_title = jsonTitleIter->get<std::string>();
            }

            // Extract ReleaseType.
            auto jsonTypeIter = releaseIter->find(RELEASETYPE);
            if (jsonTypeIter == releaseIter->end())
            {
                isParsed = false;
                errorMessage.append(STR_ERROR_VERSION_FILE_IS_MISSING_THE_RELEASETYPE_ENTRY);
            }
            else
            {
                if (!GetReleaseType_1_6(jsonTypeIter->get<std::string>(), releaseInfo.m_type))
                {
                    isParsed = false;
                    errorMessage.append(STR_ERROR_VERSION_FILE_CONTAINS_AN_INVALID_RELEASETYPE_VALUE);
                }
            }

            // Extract ReleasePlatforms.
            auto jsonPlatformsIter = releaseIter->find(RELEASEPLATFORMS);
            if (jsonPlatformsIter == releaseIter->end())
            {
                isParsed = false;
                errorMessage.append(STR_ERROR_VERSION_FILE_IS_MISSING_THE_RELEASEPLATFORMS_ENTRY);
            }
            else
            {
                if (!GetReleasePlatform_1_6(*jsonPlatformsIter, releaseInfo.m_targetPlatforms, errorMessage))
                {
                    isParsed = false;
                }
            }

            // Extract ReleaseTags.
            auto jsonTagsIter = releaseIter->find(RELEASETAGS);
            if (jsonTagsIter == releaseIter->end())
            {
                isParsed = false;
                errorMessage.append(STR_ERROR_VERSION_FILE_IS_MISSING_THE_RELEASETAGS_ENTRY);
            }
            else
            {
                // Extract each individual tag.
                for (json::iterator tagIter = jsonTagsIter->begin(); tagIter != jsonTagsIter->end(); ++tagIter)
                {
                    releaseInfo.m_tags.push_back(tagIter->get<std::string>());
                }
            }

            // Extract InfoPageLinks.
            auto jsonInfoLinksIter = releaseIter->find(INFOPAGELINKS);
            if (jsonInfoLinksIter == releaseIter->end())
            {
                isParsed = false;
                errorMessage.append(STR_ERROR_VERSION_FILE_IS_MISSING_THE_INFOPAGELINKS_ENTRY);
            }
            else
            {
                if (jsonInfoLinksIter->empty())
                {
                    isParsed = false;
                    errorMessage.append(STR_ERROR_VERSION_FILE_CONTAINS_AN_EMPTY_INFOPAGELINKS_LIST);
                }
                else
                {
                    for (json::iterator infoPageIter = jsonInfoLinksIter->begin(); infoPageIter != jsonInfoLinksIter->end(); ++infoPageIter)
                    {
                        if (infoPageIter->find(INFOPAGELINKS_URL) != infoPageIter->end() &&
                            infoPageIter->find(INFOPAGELINKS_DESCRIPTION) != infoPageIter->end())
                        {
                            InfoPageLink infoPage;
                            infoPage.m_url = (*infoPageIter)[INFOPAGELINKS_URL].get<std::string>();
                            infoPage.m_pageDescription = (*infoPageIter)[INFOPAGELINKS_DESCRIPTION].get<std::string>();
                            releaseInfo.m_infoLinks.push_back(infoPage);
                        }
                        else
                        {
                            isParsed = false;
                            errorMessage.append(STR_ERROR_VERSION_FILE_CONTAINS_AN_INCOMPLETE_INFOPAGELINKS_ENTRY);
                        }
                    }
                }
            }

            // Extract DownloadLinks.
            if (isParsed)
            {
                auto jsonDownloadLinksIter = releaseIter->find(DOWNLOADLINKS);
                if (jsonDownloadLinksIter == releaseIter->end())
                {
                    isParsed = false;
                    errorMessage.append(STR_ERROR_VERSION_FILE_IS_MISSING_THE_DOWNLOADLINKS_ENTRY);
                }
                else
                {
                    if (jsonDownloadLinksIter->empty())
                    {
                        isParsed = false;
                        errorMessage.append(STR_ERROR_VERSION_FILE_CONTAINS_AN_EMPTY_DOWNLOADLINKS_LIST);
                    }
                    else
                    {
                        for (json::iterator downloadLinkIter = jsonDownloadLinksIter->begin(); downloadLinkIter != jsonDownloadLinksIter->end(); ++downloadLinkIter)
                        {
                            auto urlIter = downloadLinkIter->find(DOWNLOADLINKS_URL);
                            auto packageTypeIter = downloadLinkIter->find(DOWNLOADLINKS_PACKAGETYPE);
                            if (urlIter == downloadLinkIter->end())
                            {
                                isParsed = false;
                                errorMessage.append(STR_ERROR_VERSION_FILE_IS_MISSING_THE_DOWNLOADLINKS_URL_ENTRY);
                            }
                            else if (packageTypeIter == downloadLinkIter->end())
                            {
                                isParsed = false;
                                errorMessage.append(STR_ERROR_VERSION_FILE_IS_MISSING_THE_DOWNLOADLINKS_PACKAGETYPE_ENTRY);
                            }
                            else
                            {
                                DownloadLink downloadLink;

                                // Make sure the targetString is a valid target.
                                if (!GetPackageType_1_6(packageTypeIter->get<std::string>(), downloadLink.m_packageType))
                                {
                                    isParsed = false;
                                    errorMessage.append(STR_ERROR_VERSION_FILE_CONTAINS_AN_INVALID_DOWNLOADLINKS_PACKAGETYPE_VALUE);
                                }
                                else
                                {
                                    downloadLink.m_url = (*downloadLinkIter)[DOWNLOADLINKS_URL].get<std::string>();
                                    releaseInfo.m_downloadLinks.push_back(downloadLink);
                                }
                            }
                        }
                    }
                }
            }

            // Now add this release to the list.
            updateInfo.m_releases.push_back(releaseInfo);
        }
    }

    return isParsed;
}

/// Updates all of the updateInfo except the bool to indicate whether it is a newer version.
static bool ParseJsonString(const std::string& jsonString, UpdateInfo& updateInfo, std::string& errorMessage)
{
    bool isParsed = true;

    try
    {
        json jsonDoc = json::parse(jsonString);

        if (jsonDoc.empty() ||
            jsonDoc.find(SCHEMAVERSION) == jsonDoc.end())
        {
            isParsed = false;
            errorMessage.append(STR_ERROR_VERSION_FILE_IS_MISSING_THE_SCHEMAVERSION_ENTRY);
        }
        else if (jsonDoc[SCHEMAVERSION].get<std::string>().compare(SCHEMA_VERSION_1_3) == 0)
        {
            UpdateInfo_1_5 updateInfo_1_5;
            if (!ParseJsonSchema_1_3(jsonDoc, updateInfo_1_5, errorMessage))
            {
                isParsed = false;
            }
            else
            {
                if (!ConvertJsonSchema_1_5_To_1_6(updateInfo_1_5, updateInfo))
                {
                    isParsed = false;
                    errorMessage.append(STR_ERROR_UNABLE_TO_CONVERT_1_3_TO_1_6);
                }
            }
        }
        else if (jsonDoc[SCHEMAVERSION].get<std::string>().compare(SCHEMA_VERSION_1_5) == 0)
        {
            UpdateInfo_1_5 updateInfo_1_5;
            if (!ParseJsonSchema_1_5(jsonDoc, updateInfo_1_5, errorMessage))
            {
                isParsed = false;
            }
            else
            {
                if (!ConvertJsonSchema_1_5_To_1_6(updateInfo_1_5, updateInfo))
                {
                    isParsed = false;
                    errorMessage.append(STR_ERROR_UNABLE_TO_CONVERT_1_5_TO_1_6);
                }
            }
        }
        else if (jsonDoc[SCHEMAVERSION].get<std::string>().compare(SCHEMA_VERSION_1_6) == 0)
        {
            if (!ParseJsonSchema_1_6(jsonDoc, updateInfo, errorMessage))
            {
                isParsed = false;
            }
        }
        else
        {
            isParsed = false;
            errorMessage.append(STR_ERROR_UNSUPPORTED_SCHEMAVERION);
        }
    }
    catch(std::exception& e)
    {
        isParsed = false;
        errorMessage.append(STR_ERROR_FAILED_TO_PARSE_VERSION_FILE);
        errorMessage.append(e.what());
    }

    return isParsed;
}

// Filter out (remove) the Releases that are not relevant to the current platform.
static bool FilterToCurrentPlatform(UpdateCheck::UpdateInfo& updateInfo)
{
    bool mayHaveUpdatesForCurrentPlatform = true;

#ifdef WIN32
    const UpdateCheck::TargetPlatform currentPlatform = UpdateCheck::TargetPlatform::Windows;
#elif __linux__
    const UpdateCheck::TargetPlatform currentPlatform = UpdateCheck::TargetPlatform::Ubuntu;
#elif __APPLE__
    const UpdateCheck::TargetPlatform currentPlatform = UpdateCheck::TargetPlatform::Darwin;
#else
    const UpdateCheck::TargetPlatform currentPlatform = UpdateCheck::TargetPlatform::Unknown;
#endif

    if (UpdateCheck::TargetPlatform::Unknown != currentPlatform)
    {
        // Sort the releases based on platform. Releases that are for the current platform
        // will be moved to the beginning; releases that don't match will be moved to the end.
        auto newEnd = std::remove_if(updateInfo.m_releases.begin(), updateInfo.m_releases.end(),
            [=](UpdateCheck::ReleaseInfo release) {
            return !std::any_of(release.m_targetPlatforms.begin(), release.m_targetPlatforms.end(),
                [=](UpdateCheck::TargetPlatform platform) {
                return platform == currentPlatform; }); });

        // Now actually remove the releases that are not for the current platform.
        updateInfo.m_releases.erase(newEnd, updateInfo.m_releases.end());

        // Updates may be available if there are still releases left.
        mayHaveUpdatesForCurrentPlatform = (updateInfo.m_releases.size() > 0);
    }

    return mayHaveUpdatesForCurrentPlatform;
}

bool UpdateCheck::CheckForUpdates(const UpdateCheck::VersionInfo& productVersion, const std::string& latestReleaseUrl, const std::string& jsonFileName, UpdateCheck::UpdateInfo& updateInfo, std::string& errorMessage)
{
    bool checkedForUpdate = false;
    updateInfo.m_isUpdateAvailable = false;
    std::string loadedJsonContents;

    try
    {
        // Confirm a path to a JSON file was provided.
        bool isJson = (jsonFileName.rfind(STR_JSON_FILE_EXTENSION) != std::string::npos);
        assert(isJson);

        if (!isJson)
        {
            // The provided URL doesn't point to a supported file type.
            checkedForUpdate = false;
            errorMessage.append(STR_ERROR_URL_MUST_POINT_TO_A_JSON_FILE);
        }
        else
        {
            if (latestReleaseUrl.find(STR_GITHUB_RELEASES_LATEST) != std::string::npos)
            {
                // Get JSON file from the latest release (using GitHub Release API).
                checkedForUpdate = LoadJsonFromLatestRelease(latestReleaseUrl, jsonFileName, loadedJsonContents, errorMessage);
            }
            else if (latestReleaseUrl.find(STR_HTTP_PREFIX) == 0)
            {
                // Attempt to download JSON file contents.
                std::string fullUrl;

                // If a filename was included in the parameters, append that to the URL.
                if (jsonFileName.empty())
                {
                    fullUrl = latestReleaseUrl;
                }
                else
                {
                    fullUrl = latestReleaseUrl + "/" + jsonFileName;
                }

                checkedForUpdate = DownloadJsonFile(fullUrl, loadedJsonContents, errorMessage);
            }
            else
            {
                // Attempt to load the JSON file from disk.
                std::string fullPath;
                if (latestReleaseUrl.empty())
                {
                    fullPath = jsonFileName;
                }
                else
                {
                    fullPath = latestReleaseUrl + "/" + jsonFileName;
                }

                checkedForUpdate = LoadJsonFile(fullPath, loadedJsonContents, errorMessage);
            }

            if (checkedForUpdate)
            {
                // Parse the JSON string to populate the updateInfo struct.
                checkedForUpdate = ParseJsonString(loadedJsonContents, updateInfo, errorMessage);

                if (checkedForUpdate)
                {
                    bool hasCompatibleUpdate = FilterToCurrentPlatform(updateInfo);

                    if (hasCompatibleUpdate)
                    {
                        // Check to see if a version from the update is newer than the current product version.
                        for (auto releaseIter = updateInfo.m_releases.begin(); releaseIter != updateInfo.m_releases.end(); ++releaseIter)
                        {
                            updateInfo.m_isUpdateAvailable = (releaseIter->m_version.Compare(productVersion) == NEWER);
                            if (updateInfo.m_isUpdateAvailable)
                            {
                                break;
                            }
                        }
                    }
                }
            }
        }
    }
    catch (std::exception& e)
    {
        checkedForUpdate = false;
        errorMessage.append(STR_ERROR_UNKNOWN_ERROR_OCCURRED);
        errorMessage.append(e.what());
    }

    return checkedForUpdate;
}

// Utility API to convert from a TargetPlatform enum to a string.
std::string UpdateCheck::TargetPlatformToString(const TargetPlatform targetPlatform)
{
    std::string text;

    switch (targetPlatform)
    {
    case TargetPlatform::Unknown:
        text = STR_PLATFORMTYPE_UNKNOWN;
        break;
    case TargetPlatform::Windows:
        text = STR_PLATFORMTYPE_WINDOWS;
        break;
    case TargetPlatform::Ubuntu:
        text = STR_PLATFORMTYPE_UBUNTU;
        break;
    case TargetPlatform::RHEL:
        text = STR_PLATFORMTYPE_RHEL;
        break;
    case TargetPlatform::Darwin:
        text = STR_PLATFORMTYPE_DARWIN;
        break;
    default:
        text = STR_UNDEFINED;
        break;
    }

    return text;
}

// Utility API to convert from a PackageType enum to a string.
std::string UpdateCheck::PackageTypeToString(const PackageType packageType)
{
    std::string text;

    switch (packageType)
    {
    case PackageType::Unknown:
        text = STR_PACKAGETYPE_UNKNOWN;
        break;
    case PackageType::ZIP:
        text = STR_PACKAGETYPE_ZIP;
        break;
    case PackageType::MSI:
        text = STR_PACKAGETYPE_MSI;
        break;
    case PackageType::TAR:
        text = STR_PACKAGETYPE_TAR;
        break;
    case PackageType::RPM:
        text = STR_PACKAGETYPE_RPM;
        break;
    case PackageType::Debian:
        text = STR_PACKAGETYPE_DEBIAN;
        break;
    default:
        text = STR_UNDEFINED;
        break;
    }

    return text;
}

// Utility API to convert from a ReleaseType enum to a string.
std::string UpdateCheck::ReleaseTypeToString(const ReleaseType releaseType)
{
    std::string text;

    switch (releaseType)
    {
    case ReleaseType::Unknown:
        text = STR_RELEASETYPE_UNKNOWN;
        break;
    case ReleaseType::GA:
        text = STR_RELEASETYPE_GA;
        break;
    case ReleaseType::Beta:
        text = STR_RELEASETYPE_BETA;
        break;
    case ReleaseType::Alpha:
        text = STR_RELEASETYPE_ALPHA;
        break;
    case ReleaseType::Patch:
        text = STR_RELEASETYPE_PATCH;
        break;
    case ReleaseType::Development:
        text = STR_RELEASETYPE_DEVELOPMENT;
        break;
    default:
        text = STR_UNDEFINED;
        break;
    }

    return text;
}
