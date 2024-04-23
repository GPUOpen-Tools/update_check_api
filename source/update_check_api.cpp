//==============================================================================
/// Copyright (c) 2018-2024 Advanced Micro Devices, Inc. All rights reserved.
/// @author AMD Developer Tools Team
/// @file
/// @brief An API for checking for updates to an application.
//==============================================================================
#include "update_check_api.h"
#include "update_check_api_strings.h"
#include "update_check_api_utils.h"

#ifdef _WIN32
#pragma warning(push)
#pragma warning(disable : 4127)  // warning C4127: conditional expression is constant
#endif

#include "third_party/json-3.9.1/json.hpp"

#ifdef _WIN32
#pragma warning(pop)
#endif

#include <assert.h>
#include <sstream>
#include <fstream>
#include <cstdlib>

#ifdef WIN32
#define updater_sscanf sscanf_s
#else  // Linux
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

const int kNewer = 1;
const int kOlder = -1;
const int kEqual = 0;

VersionInfo UpdateCheck::GetApiVersionInfo()
{
    VersionInfo version;
    version.major = UPDATECHECKAPI_MAJOR;
    version.minor = UPDATECHECKAPI_MINOR;
    version.patch = UPDATECHECKAPI_PATCH;
    version.build = UPDATECHECKAPI_BUILD;

    return version;
}

/// @brief Helper function to execute the Radeon Tools Download Assistant.
///
/// @param [in]  remote_url    The URL of the file to download.
/// @param [in]  local_file    The local file that it should be downloaded as.
/// @param [out] error_message Any error messages that occurred.
///
/// @return true if the file was successfully downloaded; false otherwise.
static bool ExecDownloader(const std::string& remote_url, const std::string& local_file, std::string& error_message)
{
    bool was_launched = false;

    try
    {
        bool        cancel_signal = false;
        std::string command_output;

        // Setup the cmd line.
        std::string cmd_line = kStringDownloaderApplication;
        cmd_line += " \"";
        cmd_line += remote_url;
        cmd_line += "\" ";
        cmd_line += local_file;

        // Download the file.
        was_launched = UpdateCheckApiUtils::ExecAndGrabOutput(cmd_line.c_str(), cancel_signal, command_output);

        if (!was_launched)
        {
            error_message.append(kStringErrorFailedToLaunchVersionFileDownloader);
        }
    }
    catch (std::exception& e)
    {
        was_launched = false;
        error_message.append(kStringErrorFailedToLaunchVersionFileDownloaderUnknownError);
        error_message.append(e.what());
    }

    return was_launched;
}

/// @brief Helper function to load a json file from disk.
///
/// @param [in]  json_file_path Path to the local JSON file.
/// @param [out] json_string    The contents of the JSON file.
/// @param [out] error_message  Any error messages that occurred.
///
/// @retval true on success; json_string will have the contents of the file at json_file_url.
/// @retval false on failure; json_string will be unchanged.
static bool LoadJsonFile(const std::string& json_file_path, std::string& json_string, std::string& error_message)
{
    bool is_loaded = true;
    json_string.clear();

    // Open the file.
    std::ifstream read_file(json_file_path.c_str());
    if (!read_file.good())
    {
        is_loaded = false;
        error_message.append(kStringErrorFailedToLoadVersionFile);
    }
    else
    {
        // Copy the file contents into the string.
        json_string.assign((std::istreambuf_iterator<char>(read_file)), std::istreambuf_iterator<char>());

        read_file.close();

        // Consider it loaded if the JSON string is not empty.
        if (json_string.empty())
        {
            is_loaded = false;
            error_message.append(kStringErrorDownloadedAnEmptyVersionFile);
        }
        else
        {
            is_loaded = true;
        }
    }

    return is_loaded;
}

/// @brief Helper function to download JSON file.
///
/// @param [in]  json_file_url  URL of the JSON file to download.
/// @param [out] json_string    The contents of the JSON file.
/// @param [out] error_message  Any error messages that occurred.
///
/// @retval true on success; json_string will have the contents of the file at json_file_url.
/// @retval false on failure; json_string will be unchanged.
static bool DownloadJsonFile(const std::string json_file_url, std::string& json_string, std::string& error_message)
{
    bool is_loaded = true;
    json_string.clear();

    std::string local_file;
    if (!UpdateCheckApiUtils::GetTempDirectory(local_file))
    {
        is_loaded = false;
    }
    else
    {
        assert(local_file.size() != 0);
        local_file += "/";

        size_t slash_position = json_file_url.find_last_of("/\\");
        size_t end_position   = json_file_url.find_last_of("?");
        if (end_position == std::string::npos)
        {
            end_position = json_file_url.size();
        }
        else
        {
            // Update the position to NOT include the character.
            end_position = end_position - 1;
        }

        if (slash_position != std::string::npos)
        {
            local_file += json_file_url.substr(slash_position + 1, end_position - slash_position);
        }
        else
        {
            local_file += json_file_url.substr(0, end_position);
        }

        // Attempt to delete the local file before downloading the new one.
        std::remove(local_file.c_str());

        // Download the JSON file.
        if (!ExecDownloader(json_file_url, local_file, error_message))
        {
            is_loaded = false;
            error_message.append(kStringErrorFailedToLaunchVersionFileDownloader);
        }
        else
        {
            is_loaded = LoadJsonFile(local_file, json_string, error_message);
        }
    }

    return is_loaded;
}

/// @brief Given a JSON list of release assets, finds the named asset and returns the associated JSON element.
///
/// @param [in]  asset_list    The "assets" element from a JSON file.
/// @param [in]  asset_name    A specific asset name to find.
/// @param [out] asset_element The associated asset element if the asset name is found.
///
/// @return true if the asset name was found in the asset list, false otherwise.
static bool FindAssetByName(json& asset_list, const std::string& asset_name, json& asset_element)
{
    bool was_asset_found = false;

    // Check each asset for the desired asset name.
    for (json::iterator asset_iter = asset_list.begin(); asset_iter != asset_list.end(); ++asset_iter)
    {
        auto asset_name_element = asset_iter->find(kStringTagAssetName);

        if (asset_name_element != asset_iter->end())
        {
            was_asset_found = (0 == asset_name_element->get<std::string>().compare(asset_name));
            if (was_asset_found)
            {
                asset_element = *asset_iter;
                break;
            }
        }
    }

    return was_asset_found;
}

/// @brief Finds the asset based on its filename and returns the URL from which to download the asset.
///
/// @param [in]  latest_release     The latest release JSON element.
/// @param [in]  asset_name         The asset name to find in the latest release element.
/// @param [out] asset_download_url The download URL for the specified asset.
/// @param [out] error_message      Any error messages that occurred.
///
/// @return true if the asset and download URL could be found; false otherwise.
static bool FindAssetDownloadUrl(json& latest_release, const std::string& asset_name, std::string& asset_download_url, std::string& error_message)
{
    bool has_asset_download_url = false;

    auto assets_element = latest_release.find(kStringTagAssets);

    if (assets_element == latest_release.end())
    {
        error_message.append(kStringErrorMissingAssetsTags);
    }
    else
    {
        json& assets_list = *assets_element;
        json  asset_element;
        bool  does_asset_exist = FindAssetByName(assets_list, asset_name, asset_element);

        if (!does_asset_exist)
        {
            error_message.append(kStringErrorAssetNotFound);
        }
        else
        {
            auto asset_url = asset_element.find(kStringTagAssetBrowserDownloadUrl);
            if (asset_url == asset_element.end())
            {
                error_message.append(kStringErrorDownloadUrlNotFoundInAsset);
            }
            else
            {
                asset_download_url     = asset_url->get<std::string>().c_str();
                has_asset_download_url = true;
            }
        }
    }

    return has_asset_download_url;
}

/// @brief Helper function to load JSON file from the latest release of a GitHub Repository.
///
/// @param [in]  json_file_url  URL of the JSON file to download.
/// @param [in]  json_file_name The local filename to save the downloaded JSON file.
/// @param [out] json_string    The contents of the downloaded JSON file.
/// @param [out] error_message  Any error messages that occurred.
///
/// @retval true on success; json_string will have the contents of the file at json_file_url.
/// @retval false on failure; json_string will be unchanged.
static bool LoadJsonFromLatestRelease(const std::string json_file_url, const std::string json_file_name, std::string& json_string, std::string& error_message)
{
    bool was_loaded = false;

    // Build a path to a temporary file.
    std::string latest_release_api_temp_file;
    if (!UpdateCheckApiUtils::GetTempDirectory(latest_release_api_temp_file))
    {
        error_message.append(kStringErrorUnableToFindTempDirectory);
    }
    else
    {
        latest_release_api_temp_file += "/";
        latest_release_api_temp_file += kStringLatestJsonFilename;

        if (!ExecDownloader(json_file_url, latest_release_api_temp_file, error_message))
        {
            error_message.append(kStringErrorFailedToLaunchVersionFileDownloader);
        }
        else
        {
            try
            {
                std::string latest_release_json;
                if (LoadJsonFile(latest_release_api_temp_file, latest_release_json, error_message))
                {
                    // Parsing the json can throw an exception if the string is
                    // not valid json. This can happen in networks that limit
                    // internet access, and result in downloading an html page.
                    json latest_release_json_doc = json::parse(latest_release_json);

                    std::string version_file_url;
                    if (FindAssetDownloadUrl(latest_release_json_doc, json_file_name, version_file_url, error_message))
                    {
                        was_loaded = DownloadJsonFile(version_file_url, json_string, error_message);
                    }
                    else
                    {
                        // Failed to find the Asset, so check for a "message" tag which may indicate an error from the GitHub Release API.
                        auto message_element = latest_release_json_doc.find(kStringTagMessage);

                        if (message_element != latest_release_json_doc.end())
                        {
                            error_message.append(message_element->get<std::string>().c_str());
                        }
                    }
                }
            }
            catch (std::exception& e)
            {
                was_loaded = false;
                error_message.append(kStringErrorFailedToLoadLatestReleaseInformation);
                error_message.append(e.what());
            }
        }
    }

    return was_loaded;
}

/// @brief Convert VersionInfo to a string.
///
/// @return The stringified version information.
std::string VersionInfo::ToString() const
{
    std::stringstream version_string;
    version_string << major << VERSION_DELIMITER << minor << VERSION_DELIMITER << patch << VERSION_DELIMITER << build;
    return version_string.str();
}

/// @brief Compare 'this' version to the supplied version.
///
/// @param [in] other_version The version to compare against.
///
/// @retval NEWER if 'this' version is newer than the other_version.
/// @retval OLDER if 'this' version is older than the other_version.
/// @retval EQUAL if 'this' version is equal to the other_version.
int VersionInfo::Compare(const VersionInfo& other_version) const
{
    int comparison = 0;

    // Compare Major version.
    if (major > other_version.major)
    {
        comparison = kNewer;
    }
    else if (major < other_version.major)
    {
        comparison = kOlder;
    }
    else
    {
        // Compare Minor version.
        if (minor > other_version.minor)
        {
            comparison = kNewer;
        }
        else if (minor < other_version.minor)
        {
            comparison = kOlder;
        }
        else
        {
            // Compare Patch version.
            if (patch > other_version.patch)
            {
                comparison = kNewer;
            }
            else if (patch < other_version.patch)
            {
                comparison = kOlder;
            }
            else
            {
                // Compare Build version.
                if (build > other_version.build)
                {
                    comparison = kNewer;
                }
                else if (build < other_version.build)
                {
                    comparison = kOlder;
                }
                else
                {
                    // Version numbers are the same.
                    comparison = kEqual;
                }
            }
        }
    }

    return comparison;
}

/// @brief Split the version string from Schema 1.3 into Major, Minor, Patch, and Build numbers.
///
/// @param [in]  version       The version string.
/// @param [out] version_info  The version information.
/// @param [out] error_message Any error messages that occurred.
///
/// @return true if version info is valid; false otherwise.
static bool SplitVersionString_1_3(const std::string& version, VersionInfo& version_info, std::string& error_message)
{
    bool is_valid = true;

    // If the version string is empty, it is invalid.
    if (version.empty())
    {
        is_valid = false;
    }
    else
    {
        // Version format: "Major.Minor.Patch.Build".
        version_info.major    = 0;
        version_info.minor    = 0;
        version_info.patch    = 0;
        version_info.build    = 0;
        int num_version_parts = updater_sscanf(version.c_str(),
                                               "%u" VERSION_DELIMITER "%u" VERSION_DELIMITER "%u" VERSION_DELIMITER "%u",
                                               &version_info.major,
                                               &version_info.minor,
                                               &version_info.patch,
                                               &version_info.build);

        if (num_version_parts == EOF || num_version_parts <= 0 || num_version_parts > 4)
        {
            is_valid = false;
            error_message.append(kStringErrorInvalidVersionNumberProvided);
        }
    }

    return is_valid;
}

/// @brief Split the version string from Schema 1.5 into Major, Minor, Patch, and Build numbers.
///
/// @param [in]  json_doc      The json document.
/// @param [out] version_info  The version information.
/// @param [out] error_message Any error messages that occurred.
///
/// @retval true on success
/// @retval false on error, and error_message will be appended to.
static bool SplitVersionString_1_5(json& json_doc, VersionInfo& version_info, std::string& error_message)
{
    bool is_valid = true;

    // Attempt to parse the version string.
    auto json_major_version = json_doc.find(RELEASEVERSION_MAJOR);
    auto json_minor_version = json_doc.find(RELEASEVERSION_MINOR);
    auto json_patch_version = json_doc.find(RELEASEVERSION_PATCH);
    auto json_build_version = json_doc.find(RELEASEVERSION_BUILD);

    if (json_major_version == json_doc.end() && json_minor_version == json_doc.end() && json_patch_version == json_doc.end() &&
        json_build_version == json_doc.end())
    {
        is_valid = false;
        error_message.append(kStringErrorInvalidVersionNumberProvided);
    }
    else
    {
        if (json_major_version != json_doc.end())
        {
            version_info.major = json_major_version->get<uint32_t>();
        }

        if (json_minor_version != json_doc.end())
        {
            version_info.minor = json_minor_version->get<uint32_t>();
        }

        if (json_patch_version != json_doc.end())
        {
            version_info.patch = json_patch_version->get<uint32_t>();
        }

        if (json_build_version != json_doc.end())
        {
            version_info.build = json_build_version->get<uint32_t>();
        }
    }

    return is_valid;
}

/// @brief Translate the package string to the corresponding UpdatePackageType.
///
/// @param [in]  package_string The package string.
/// @param [out] platform       The platform.
/// @param [out] package_type   The package type.
///
/// @retval true if the package_string is recognized and a valid UpdatePackageType is set.
/// @retval false if the package_string is not recognized.
static bool GetPackageType_1_3(const std::string& package_string, TargetPlatform& platform, PackageType& package_type)
{
    bool is_known_type = true;

    if (package_string.compare(kStringPackageTypeWindowsZip) == 0)
    {
        platform     = TargetPlatform::kWindows;
        package_type = PackageType::kZip;
    }
    else if (package_string.compare(kStringPackageTypeWindowsMsi) == 0)
    {
        platform     = TargetPlatform::kWindows;
        package_type = PackageType::kMsi;
    }
    else if (package_string.compare(kStringPackageTypeLinuxTar) == 0)
    {
        platform     = TargetPlatform::kUbuntu;
        package_type = PackageType::kTar;
    }
    else if (package_string.compare(kStringPackageTypeLinuxRpm) == 0)
    {
        platform     = TargetPlatform::kUbuntu;
        package_type = PackageType::kRpm;
    }
    else if (package_string.compare(kStringPackageTypeLinuxDebian) == 0)
    {
        platform     = TargetPlatform::kUbuntu;
        package_type = PackageType::kDebian;
    }
    else
    {
        platform      = TargetPlatform::kUnknown;
        package_type  = PackageType::kUnknown;
        is_known_type = false;
    }

    return is_known_type;
}

/// This structure represents an update package from Schema 1.5.
struct UpdatePackage_1_5
{
    /// The URL from which the archive/installer can be downloaded.
    std::string url;

    /// A value describing the kind of archive/installer that url points
    /// to. Products will filter irrelevant download links basaed on this value.
    PackageType package_type;

    /// The type of the release for the package that is referenced by this link.
    ReleaseType release_type;

    /// The target platforms to which the packge referenced by this link is relevant.
    std::vector<TargetPlatform> target_platforms;
};

/// This structure contains the information received checking the
/// availability of product updates from Schema 1.5.
struct UpdateInfo_1_5
{
    /// True if an update to a newer version is available, false otherwise.
    bool is_update_available;

    /// The version of the available update.
    VersionInfo release_version;

    /// The release date of the available update in the format YYYY-MM-DD.
    std::string release_date;

    /// Text describing the available update, may include release notes highlights for example.
    std::string release_description;

    /// The available update packages: information and URL from which they can be downloaded.
    std::vector<UpdatePackage_1_5> available_packages;

    /// Links to relevant pages, like product landing page on GPUOpen, or Releases page on Github.com.
    std::vector<InfoPageLink> info_links;
};

/// @brief Parse JSON string for Schema 1.3 directly into the structures for schema 1.5.
///
/// @param [in]  json_document The json document.
/// @param [out] update_info   The update information structure for schema 1.5.
/// @param [out] error_message Any error messages that occurred.
///
/// @return true if json document is parsed successfully; false otherwise.
static bool ParseJsonSchema_1_3(json& json_document, UpdateInfo_1_5& update_info, std::string& error_message)
{
    bool is_parsed = true;

    // Extract VersionString.
    if (json_document.find(VERSIONSTRING) == json_document.end())
    {
        is_parsed = false;
        error_message.append(kStringErrorVersionFileIsMissingTheVersionStringEntry);
    }
    else
    {
        // Attempt to parse the version string.
        if (!SplitVersionString_1_3(json_document[VERSIONSTRING].get<std::string>(), update_info.release_version, error_message))
        {
            is_parsed = false;
        }
    }

    // Extract ReleaseDate.
    if (json_document.find(RELEASEDATE) == json_document.end())
    {
        is_parsed = false;
        error_message.append(kStringErrorVersionFileIsMissingTheReleaseDateEntry);
    }
    else
    {
        update_info.release_date = json_document[RELEASEDATE].get<std::string>();
    }

    // Extract Description.
    if (json_document.find(DESCRIPTION) == json_document.end())
    {
        is_parsed = false;
        error_message.append(kStringErrorVersionFileIsMissingTheDescriptionEntry);
    }
    else
    {
        update_info.release_description = json_document[DESCRIPTION].get<std::string>();
    }

    // Extract InfoPageLink.
    if (json_document.find(INFOPAGEURL) == json_document.end())
    {
        is_parsed = false;
        error_message.append(kStringErrorVersionFileIsMissingTheInfoPageUrlEntry);
    }
    else
    {
        json json_info_page_object = json_document[INFOPAGEURL];
        if (json_info_page_object.empty())
        {
            is_parsed = false;
            error_message.append(kStringErrorVersionFileContainsAnEmptyInfoPageUrlList);
        }
        else
        {
            for (json::iterator info_page_iter = json_info_page_object.begin(); info_page_iter != json_info_page_object.end(); ++info_page_iter)
            {
                if (info_page_iter->find(INFOPAGEURL_URL) != info_page_iter->end() && info_page_iter->find(INFOPAGEURL_DESCRIPTION) != info_page_iter->end())
                {
                    InfoPageLink info_page;
                    info_page.url              = (*info_page_iter)[INFOPAGEURL_URL].get<std::string>();
                    info_page.page_description = (*info_page_iter)[INFOPAGEURL_DESCRIPTION].get<std::string>();
                    update_info.info_links.push_back(info_page);
                }
                else
                {
                    is_parsed = false;
                    error_message.append(kStringErrorVersionFileContainsAnIncompleteInfoPageUrlEntry);
                }
            }
        }
    }

    // Extract DownloadURL.
    if (json_document.find(DOWNLOADURL) == json_document.end())
    {
        is_parsed = false;
        error_message.append(kStringErrorVersionFileIsMissingTheDownloadUrlEntry);
    }
    else
    {
        json json_download_url_object = json_document[DOWNLOADURL];
        if (json_download_url_object.empty())
        {
            is_parsed = false;
            error_message.append(kStringErrorVersionFileContainsAnEmptyDownloadUrlList);
        }
        else
        {
            for (json::iterator download_link_iter = json_download_url_object.begin(); download_link_iter != json_download_url_object.end();
                 ++download_link_iter)
            {
                if (download_link_iter->find(DOWNLOADURL_URL) != download_link_iter->end() &&
                    download_link_iter->find(DOWNLOADURL_TARGETINFO) != download_link_iter->end())
                {
                    UpdatePackage_1_5 updatePackage;

                    std::string targetString = (*download_link_iter)[DOWNLOADURL_TARGETINFO].get<std::string>();

                    // Make sure the targetString is a valid target.
                    PackageType    packageType = PackageType::kUnknown;
                    TargetPlatform platform    = TargetPlatform::kUnknown;
                    if (!GetPackageType_1_3(targetString, platform, packageType))
                    {
                        is_parsed = false;
                        error_message.append(kStringErrorVersionFileContainsAnInvalidDownloadUrlTargetInfoValue);
                    }
                    else
                    {
                        updatePackage.url          = (*download_link_iter)[DOWNLOADURL_URL].get<std::string>();
                        updatePackage.package_type = packageType;
                        updatePackage.target_platforms.push_back(platform);

                        // Schema 1.3 does not have a field to represent the release type, so default to GA
                        // since it can be assumed that everything at that point was a GA release.
                        updatePackage.release_type = ReleaseType::kGeneralAvailability;

                        update_info.available_packages.push_back(updatePackage);
                    }
                }
                else
                {
                    is_parsed = false;
                    error_message.append(kStringErrorVersionFileContainsAnIncompleteDownloadUrlEntry);
                }
            }
        }
    }

    return is_parsed;
}

/// @brief Translate the package string to the corresponding UpdatePackageType.
///
/// @param [in]  package_string The package string.
/// @param [out] package_type   The package type.
///
/// @retval true if the package_string is recognized and a valid UpdatePackageType is set.
/// @retval false if the package_string is not recognized.
static bool GetPackageType_1_5(const std::string& package_string, PackageType& package_type)
{
    bool is_known_type = true;

    if (package_string.compare(kStringPackageTypeZip) == 0)
    {
        package_type = PackageType::kZip;
    }
    else if (package_string.compare(kStringPackageTypeMsi) == 0)
    {
        package_type = PackageType::kMsi;
    }
    else if (package_string.compare(kStringPackageTypeTar) == 0)
    {
        package_type = PackageType::kTar;
    }
    else if (package_string.compare(kStringPackageTypeRpm) == 0)
    {
        package_type = PackageType::kRpm;
    }
    else if (package_string.compare(kStringPackageTypeDebian) == 0)
    {
        package_type = PackageType::kDebian;
    }
    else
    {
        package_type  = PackageType::kUnknown;
        is_known_type = false;
    }

    return is_known_type;
}

/// @brief Translate the release_type_string to the corresponding ReleaseType enum value.
///
/// @param [in]  release_type_string The release string.
/// @param [out] release_type        The release type.
///
/// @return true if the release_type_string is recognized and a valid UpdatePackageType is set.
/// @return false if the release_type_string is not recognized.
static bool GetReleaseType_1_5(const std::string& release_type_string, ReleaseType& release_type)
{
    bool is_known_type = true;

    if (release_type_string.compare(kStringReleaseTypeGeneralAvailability) == 0)
    {
        release_type = ReleaseType::kGeneralAvailability;
    }
    else if (release_type_string.compare(kStringReleaseTypeBeta) == 0)
    {
        release_type = ReleaseType::kBeta;
    }
    else if (release_type_string.compare(kStringReleaseTypeAlpha) == 0)
    {
        release_type = ReleaseType::kAlpha;
    }
    else if (release_type_string.compare(kStringReleaseTypePatch) == 0)
    {
        release_type = ReleaseType::kPatch;
    }
    else if (release_type_string.compare(kStringReleaseTypeDevelopment) == 0)
    {
        release_type = ReleaseType::kDevelopment;
    }
    else
    {
        release_type  = ReleaseType::kUnknown;
        is_known_type = false;
    }

    return is_known_type;
}

/// @brief Translate the target platforms JSON object to the corresponding TargetPlatforms list.
///
/// @param [in]  target_platforms_json The target platform json document.
/// @param [out] platforms             The target platform.
/// @param [out] error_message         Any error messsages that occurred.
///
/// @retval true if all the platforms are recognized.
/// @retval false if any platform is not recognized.
static bool GetTargetPlatform_1_5(json& target_platforms_json, std::vector<TargetPlatform>& platforms, std::string& error_message)
{
    bool is_known_type = false;

    if (target_platforms_json.empty())
    {
        error_message.append(kStringErrorVersionFileContainsAnEmptyDownloadLinksTargetPlatformsList);
    }
    else
    {
        for (json::iterator platform = target_platforms_json.begin(); platform != target_platforms_json.end(); ++platform)
        {
            std::string platform_string = platform->get<std::string>();

            if (platform_string.compare(kStringPlatformTypeWindows) == 0)
            {
                platforms.push_back(TargetPlatform::kWindows);
                is_known_type = true;
            }
            else if (platform_string.compare(kStringPlatformTypeUbuntu) == 0)
            {
                platforms.push_back(TargetPlatform::kUbuntu);
                is_known_type = true;
            }
            else if (platform_string.compare(kStringPlatformTypeRhel) == 0)
            {
                platforms.push_back(TargetPlatform::kRhel);
                is_known_type = true;
            }
            else if (platform_string.compare(kStringPlatformTypeDarwin) == 0)
            {
                platforms.push_back(TargetPlatform::kDarwin);
                is_known_type = true;
            }
            else
            {
                is_known_type = false;
                error_message.append(kStringErrorVersionFileContainsAnInvalidDownloadLinksTargetPlatformValue);
                break;
            }
        }
    }

    return is_known_type;
}

/// @brief Parse a JSON string that should should be formatted as Schema 1.6.
///
/// @param [in]  json_doc      The json document.
/// @param [out] update_info   The update information structure for schema 1.5.
/// @param [out] error_message Any error messsages that occurred.
///
/// @return true if json string is parsed successfully; false otherwise.
static bool ParseJsonSchema_1_5(json& json_doc, UpdateInfo_1_5& update_info, std::string& error_message)
{
    bool is_parsed = true;

    // Extract ReleaseVersion.
    auto json_verson_iter = json_doc.find(RELEASEVERSION);
    if (json_verson_iter == json_doc.end())
    {
        is_parsed = false;
        error_message.append(kStringErrorVersionFileIsMissingTheReleaseVersionEntry);
    }
    else
    {
        if (!SplitVersionString_1_5(*json_verson_iter, update_info.release_version, error_message))
        {
            is_parsed = false;
        }
    }

    // Extract ReleaseDate.
    auto json_date_iter = json_doc.find(RELEASEDATE);
    if (json_date_iter == json_doc.end())
    {
        is_parsed = false;
        error_message.append(kStringErrorVersionFileIsMissingTheReleaseDateEntry);
    }
    else
    {
        update_info.release_date = json_date_iter->get<std::string>();
    }

    // Extract Description.
    auto json_description_iter = json_doc.find(RELEASEDESCRIPTION);
    if (json_description_iter == json_doc.end())
    {
        is_parsed = false;
        error_message.append(kStringErrorVersionFileIsMissingTheReleaseDescriptionEntry);
    }
    else
    {
        update_info.release_description = json_description_iter->get<std::string>();
    }

    // Extract InfoPageLinks.
    auto json_info_iter = json_doc.find(INFOPAGELINKS);
    if (json_info_iter == json_doc.end())
    {
        is_parsed = false;
        error_message.append(kStringErrorVersionFileIsMissingTheInfoPageLinksEntry);
    }
    else
    {
        json& json_info_page_object = *json_info_iter;

        if (json_info_page_object.empty())
        {
            is_parsed = false;
            error_message.append(kStringErrorVersionFileContainsAnEmptyInfoPageLinksList);
        }
        else
        {
            for (json::iterator info_page_iter = json_info_page_object.begin(); info_page_iter != json_info_page_object.end(); ++info_page_iter)
            {
                if (info_page_iter->find(INFOPAGELINKS_URL) != info_page_iter->end() &&
                    info_page_iter->find(INFOPAGELINKS_DESCRIPTION) != info_page_iter->end())
                {
                    InfoPageLink info_page;
                    info_page.url              = (*info_page_iter)[INFOPAGELINKS_URL].get<std::string>();
                    info_page.page_description = (*info_page_iter)[INFOPAGELINKS_DESCRIPTION].get<std::string>();
                    update_info.info_links.push_back(info_page);
                }
                else
                {
                    is_parsed = false;
                    error_message.append(kStringErrorVersionFileContainsAnIncompleteInfoPageLinksEntry);
                }
            }
        }
    }

    // Extract DownloadLinks.
    if (json_doc.find(DOWNLOADLINKS) == json_doc.end())
    {
        is_parsed = false;
        error_message.append(kStringErrorVersionFileIsMissingTheDownloadLinksEntry);
    }
    else
    {
        json json_download_url_object = json_doc[DOWNLOADLINKS];

        if (json_download_url_object.empty())
        {
            is_parsed = false;
            error_message.append(kStringErrorVersionFileContainsAnEmptyDownloadLinksList);
        }
        else
        {
            for (json::iterator download_link_iter = json_download_url_object.begin(); download_link_iter != json_download_url_object.end();
                 ++download_link_iter)
            {
                auto url_iter          = download_link_iter->find(DOWNLOADLINKS_URL);
                auto platform_iter     = download_link_iter->find(DOWNLOADLINKS_TARGETPLATFORMS);
                auto package_type_iter = download_link_iter->find(DOWNLOADLINKS_PACKAGETYPE);
                auto release_type_iter = download_link_iter->find(DOWNLOADLINKS_RELEASETYPE);
                if (url_iter == download_link_iter->end())
                {
                    is_parsed = false;
                    error_message.append(kStringErrorVersionFileIsMissingTheDownloadLinksUrlEntry);
                }
                else if (platform_iter == download_link_iter->end())
                {
                    is_parsed = false;
                    error_message.append(kStringErrorVersionFileIsMissingTheDownloadLinksTargetPlatformsEntry);
                }
                else if (package_type_iter == download_link_iter->end())
                {
                    is_parsed = false;
                    error_message.append(kStringErrorVersionFileIsMissingTheDownloadLinksPackageTypeEntry);
                }
                else if (release_type_iter == download_link_iter->end())
                {
                    is_parsed = false;
                    error_message.append(kStringErrorVersionFileIsMissingTheDownloadLinksReleaseTypeEntry);
                }
                else
                {
                    UpdatePackage_1_5 update_package;

                    // Make sure the targetString is a valid target.
                    if (!GetReleaseType_1_5(release_type_iter->get<std::string>(), update_package.release_type))
                    {
                        is_parsed = false;
                        error_message.append(kStringErrorVersionFileContainsAnInvalidDownloadLinksReleaseTypeValue);
                    }
                    else if (!GetPackageType_1_5(package_type_iter->get<std::string>(), update_package.package_type))
                    {
                        is_parsed = false;
                        error_message.append(kStringErrorVersionFileContainsAnInvalidDownloadLinksPackageTypeValue);
                    }
                    else if (!GetTargetPlatform_1_5(*platform_iter, update_package.target_platforms, error_message))
                    {
                        is_parsed = false;
                    }
                    else
                    {
                        update_package.url = (*download_link_iter)[DOWNLOADLINKS_URL].get<std::string>();
                        update_info.available_packages.push_back(update_package);
                    }
                }
            }
        }
    }

    return is_parsed;
}

/// @brief Convert from Schema 1.5 to 1.6.
///
/// @param [in]  update_info_1_5 The update information structure for schema 1.5.
/// @param [out] update_info     The update information structure for schema 1.6.
///
/// @retval true always.
static bool ConvertJsonSchema_1_5_To_1_6(const UpdateInfo_1_5& update_info_1_5, UpdateInfo& update_info)
{
    // The biggest difference between 1.5 and 1.6 is that 1.5 only had one release version,
    // which was composed of various downloads that could have been for various target platforms.
    // Contrast that with 1.6 which has multiple release versions (one per set of platforms).
    // Essentially, the target platforms from the bottom of the structure, to the top.
    //
    // The solution to fixing this is to find the first set of target platforms, then construct
    // a new ReleaseInfo struct to match this, and repeat for each set of target platforms.

    // Iterate through each UpdatePackage.
    for (auto package_iter = update_info_1_5.available_packages.cbegin(); package_iter != update_info_1_5.available_packages.cend(); ++package_iter)
    {
        ReleaseInfo* existing_release_info = nullptr;

        // Find an existing ReleaseInfo struct for the current set of target platforms and ReleaseType.
        for (auto release_iter = update_info.releases.begin(); release_iter != update_info.releases.end(); ++release_iter)
        {
            if (release_iter->target_platforms == package_iter->target_platforms && release_iter->type == package_iter->release_type)
            {
                existing_release_info = &(*release_iter);
            }
        }

        // If none of the existing releases match, then create a new one, and populate it
        // with the information that is easily transferable. Also, seed the new Tags field
        // with the target platforms and release type.
        if (existing_release_info == nullptr)
        {
            ReleaseInfo transfer_release_info;

            transfer_release_info.version = update_info_1_5.release_version;
            transfer_release_info.title   = update_info_1_5.release_description;
            transfer_release_info.date    = update_info_1_5.release_date;

            for (auto info_link_iter = update_info_1_5.info_links.cbegin(); info_link_iter != update_info_1_5.info_links.cend(); ++info_link_iter)
            {
                transfer_release_info.info_links.push_back(*info_link_iter);
            }

            // Transfer over the two fields that make this a unique release (the set of target platforms, and release type).
            for (auto platform_iter = package_iter->target_platforms.cbegin(); platform_iter != package_iter->target_platforms.cend(); ++platform_iter)
            {
                transfer_release_info.target_platforms.push_back(*platform_iter);
                transfer_release_info.tags.push_back(TargetPlatformToString(*platform_iter));
            }

            transfer_release_info.type = package_iter->release_type;
            transfer_release_info.tags.push_back(ReleaseTypeToString(package_iter->release_type));

            // Push this releaseInfo into the list, and set the release pointer.
            update_info.releases.push_back(transfer_release_info);
            existing_release_info = &(update_info.releases[update_info.releases.size() - 1]);
        }

        assert(existing_release_info != nullptr);

        // Add the new DownloadLink from this package.
        if (existing_release_info != nullptr)
        {
            DownloadLink download_link;
            download_link.package_type = package_iter->package_type;
            download_link.url          = package_iter->url;
            existing_release_info->download_links.push_back(download_link);
        }
    }

    // At present, Schema 1.5 can always be converted to Schema 1.6.
    return true;
}

/// @brief Translate the release_type_string to the corresponding ReleaseType enum value.
///
/// @param [in]  release_type_string The release type string.
/// @param [out] release_type        The release type.
///
/// @retval true if the release_type_string is recognized and a valid UpdatePackageType is set.
/// @retval false if the release_type_string is not recognized.
static bool GetReleaseType_1_6(const std::string& release_type_string, ReleaseType& release_type)
{
    // Remains unchanged from Schema 1.5.
    return GetReleaseType_1_5(release_type_string, release_type);
}

/// @brief Translate the target platforms JSON object to the corresponding TargetPlatforms list.
///
/// @param [in]  target_platforms_json The target platforms json.
/// @param [out] platforms             The target platforms list.
/// @param [out] error_message         Any error messsages that occurred.
///
/// @retval true if all the platforms are recognized.
/// @retval false if any platform is not recognized.
static bool GetReleasePlatform_1_6(json& target_platforms_json, std::vector<TargetPlatform>& platforms, std::string& error_message)
{
    bool is_known_type = false;

    if (target_platforms_json.empty())
    {
        error_message.append(kStringErrorVersionFileContainsAnEmptyReleasePlatformsList);
    }
    else
    {
        for (json::iterator platform = target_platforms_json.begin(); platform != target_platforms_json.end(); ++platform)
        {
            std::string platform_string = platform->get<std::string>();

            if (platform_string.compare(kStringPlatformTypeWindows) == 0)
            {
                platforms.push_back(TargetPlatform::kWindows);
                is_known_type = true;
            }
            else if (platform_string.compare(kStringPlatformTypeUbuntu) == 0)
            {
                platforms.push_back(TargetPlatform::kUbuntu);
                is_known_type = true;
            }
            else if (platform_string.compare(kStringPlatformTypeRhel) == 0)
            {
                platforms.push_back(TargetPlatform::kRhel);
                is_known_type = true;
            }
            else if (platform_string.compare(kStringPlatformTypeDarwin) == 0)
            {
                platforms.push_back(TargetPlatform::kDarwin);
                is_known_type = true;
            }
            else
            {
                is_known_type = false;
                error_message.append(kStringErrorVersionFileContainsAnInvalidReleasePlatformsValue);
                break;
            }
        }
    }

    return is_known_type;
}

/// @brief Translate the package string to the corresponding UpdatePackageType.
///
/// @param [in]  package_string The package string.
/// @param [out] package_type   The update package type.
///
/// @return true if the package_string is recognized and a valid UpdatePackageType is set.
/// @return false if the package_string is not recognized.
static bool GetPackageType_1_6(const std::string& package_string, PackageType& package_type)
{
    // Remains unchanged from Schema 1.5.
    return GetPackageType_1_5(package_string, package_type);
}

/// @brief Parse a JSON string that should should be formatted as Schema 1.6.
///
/// @param [in]  json_doc      The json document.
/// @param [out] update_info   The update information structure.
/// @param [out] error_message Any error messsages that occurred.
///
/// @return true if json string is parsed successfully; false otherwise.
static bool ParseJsonSchema_1_6(json& json_doc, UpdateInfo& update_info, std::string& error_message)
{
    bool is_parsed = true;

    // Extract Releases.
    auto json_releases_iter = json_doc.find(RELEASES);
    if (json_releases_iter == json_doc.end())
    {
        is_parsed = false;
        error_message.append(kStringErrorVersionFileIsMissingTheReleasesEntry);
    }
    else if (json_releases_iter->empty())
    {
        is_parsed = false;
        error_message.append(kStringErrorVersionFileContainsAnEmptyReleasesList);
    }
    else
    {
        for (json::iterator release_iter = json_releases_iter->begin(); release_iter != json_releases_iter->end(); ++release_iter)
        {
            ReleaseInfo release_info;

            // Extract ReleaseVersion.
            auto json_version_iter = release_iter->find(RELEASEVERSION);
            if (json_version_iter == release_iter->end())
            {
                is_parsed = false;
                error_message.append(kStringErrorVersionFileIsMissingTheReleaseVersionEntry);
            }
            else
            {
                if (!SplitVersionString_1_5(*json_version_iter, release_info.version, error_message))
                {
                    is_parsed = false;
                }
            }

            // Extract ReleaseDate.
            auto json_date_iter = release_iter->find(RELEASEDATE);
            if (json_date_iter == release_iter->end())
            {
                is_parsed = false;
                error_message.append(kStringErrorVersionFileIsMissingTheReleaseDateEntry);
            }
            else
            {
                release_info.date = json_date_iter->get<std::string>();
            }

            // Extract ReleaseTitle.
            auto json_title_iter = release_iter->find(RELEASETITLE);
            if (json_title_iter == release_iter->end())
            {
                is_parsed = false;
                error_message.append(kStringErrorVersionFileIsMissingTheReleaseTitleEntry);
            }
            else
            {
                release_info.title = json_title_iter->get<std::string>();
            }

            // Extract ReleaseType.
            auto json_type_iter = release_iter->find(RELEASETYPE);
            if (json_type_iter == release_iter->end())
            {
                is_parsed = false;
                error_message.append(kStringErrorVersionFileIsMissingTheReleaseTypeEntry);
            }
            else
            {
                if (!GetReleaseType_1_6(json_type_iter->get<std::string>(), release_info.type))
                {
                    is_parsed = false;
                    error_message.append(kStringErrorVersionFileContainsAnInvalidReleaseTypeValue);
                }
            }

            // Extract ReleasePlatforms.
            auto json_platforms_iter = release_iter->find(RELEASEPLATFORMS);
            if (json_platforms_iter == release_iter->end())
            {
                is_parsed = false;
                error_message.append(kStringErrorVersionFileIsMissingTheReleasePlatformsEntry);
            }
            else
            {
                if (!GetReleasePlatform_1_6(*json_platforms_iter, release_info.target_platforms, error_message))
                {
                    is_parsed = false;
                }
            }

            // Extract ReleaseTags.
            auto json_tags_iter = release_iter->find(RELEASETAGS);
            if (json_tags_iter == release_iter->end())
            {
                is_parsed = false;
                error_message.append(kStringErrorVersionFileIsMissingTheReleaseTagsEntry);
            }
            else
            {
                // Extract each individual tag.
                for (json::iterator tagIter = json_tags_iter->begin(); tagIter != json_tags_iter->end(); ++tagIter)
                {
                    release_info.tags.push_back(tagIter->get<std::string>());
                }
            }

            // Extract InfoPageLinks.
            auto json_info_links_iter = release_iter->find(INFOPAGELINKS);
            if (json_info_links_iter == release_iter->end())
            {
                is_parsed = false;
                error_message.append(kStringErrorVersionFileIsMissingTheInfoPageLinksEntry);
            }
            else
            {
                if (json_info_links_iter->empty())
                {
                    is_parsed = false;
                    error_message.append(kStringErrorVersionFileContainsAnEmptyInfoPageLinksList);
                }
                else
                {
                    for (json::iterator info_page_iter = json_info_links_iter->begin(); info_page_iter != json_info_links_iter->end(); ++info_page_iter)
                    {
                        if (info_page_iter->find(INFOPAGELINKS_URL) != info_page_iter->end() &&
                            info_page_iter->find(INFOPAGELINKS_DESCRIPTION) != info_page_iter->end())
                        {
                            InfoPageLink info_page;
                            info_page.url              = (*info_page_iter)[INFOPAGELINKS_URL].get<std::string>();
                            info_page.page_description = (*info_page_iter)[INFOPAGELINKS_DESCRIPTION].get<std::string>();
                            release_info.info_links.push_back(info_page);
                        }
                        else
                        {
                            is_parsed = false;
                            error_message.append(kStringErrorVersionFileContainsAnIncompleteInfoPageLinksEntry);
                        }
                    }
                }
            }

            // Extract DownloadLinks.
            if (is_parsed)
            {
                auto json_download_links_iter = release_iter->find(DOWNLOADLINKS);
                if (json_download_links_iter == release_iter->end())
                {
                    is_parsed = false;
                    error_message.append(kStringErrorVersionFileIsMissingTheDownloadLinksEntry);
                }
                else
                {
                    if (json_download_links_iter->empty())
                    {
                        is_parsed = false;
                        error_message.append(kStringErrorVersionFileContainsAnEmptyDownloadLinksList);
                    }
                    else
                    {
                        for (json::iterator download_link_iter = json_download_links_iter->begin(); download_link_iter != json_download_links_iter->end();
                             ++download_link_iter)
                        {
                            auto url_iter          = download_link_iter->find(DOWNLOADLINKS_URL);
                            auto package_type_iter = download_link_iter->find(DOWNLOADLINKS_PACKAGETYPE);
                            auto package_name_iter = download_link_iter->find(DOWNLOADLINKS_PACKAGENAME);
                            if (url_iter == download_link_iter->end())
                            {
                                is_parsed = false;
                                error_message.append(kStringErrorVersionFileIsMissingTheDownloadLinksUrlEntry);
                            }
                            else if (package_type_iter == download_link_iter->end())
                            {
                                is_parsed = false;
                                error_message.append(kStringErrorVersionFileIsMissingTheDownloadLinksPackageTypeEntry);
                            }
                            else
                            {
                                DownloadLink download_link;

                                // Make sure the targetString is a valid target.
                                if (!GetPackageType_1_6(package_type_iter->get<std::string>(), download_link.package_type))
                                {
                                    is_parsed = false;
                                    error_message.append(kStringErrorVersionFileContainsAnInvalidDownloadLinksPackageTypeValue);
                                }
                                else
                                {
                                    // Extract the url.
                                    download_link.url = (*download_link_iter)[DOWNLOADLINKS_URL].get<std::string>();

                                    // Extract the package name.
                                    if (package_name_iter != download_link_iter->end())
                                    {
                                        download_link.package_name = package_name_iter->get<std::string>();
                                    }

                                    release_info.download_links.push_back(download_link);
                                }
                            }
                        }
                    }
                }
            }

            // Now add this release to the list.
            update_info.releases.push_back(release_info);
        }
    }

    return is_parsed;
}

/// @brief Updates all of the update_info except the bool to indicate whether it is a newer version.
///
/// @param [in]  json_string   The json string.
/// @param [out] update_info   The update information structure.
/// @param [out] error_message Any error messsages that occurred.
///
/// @return true if json string is parsed successfully; false otherwise.
static bool ParseJsonString(const std::string& json_string, UpdateInfo& update_info, std::string& error_message)
{
    bool is_parsed = true;

    try
    {
        json json_doc = json::parse(json_string);

        if (json_doc.empty() || json_doc.find(SCHEMAVERSION) == json_doc.end())
        {
            is_parsed = false;
            error_message.append(kStringErrorVersionFileIsMissingTheSchemaVersionEntry);
        }
        else if (json_doc[SCHEMAVERSION].get<std::string>().compare(SCHEMA_VERSION_1_3) == 0)
        {
            UpdateInfo_1_5 update_info_1_5;
            if (!ParseJsonSchema_1_3(json_doc, update_info_1_5, error_message))
            {
                is_parsed = false;
            }
            else
            {
                if (!ConvertJsonSchema_1_5_To_1_6(update_info_1_5, update_info))
                {
                    is_parsed = false;
                    error_message.append(kStringErrorUnableToConvert1_3To1_6);
                }
            }
        }
        else if (json_doc[SCHEMAVERSION].get<std::string>().compare(SCHEMA_VERSION_1_5) == 0)
        {
            UpdateInfo_1_5 update_info_1_5;
            if (!ParseJsonSchema_1_5(json_doc, update_info_1_5, error_message))
            {
                is_parsed = false;
            }
            else
            {
                if (!ConvertJsonSchema_1_5_To_1_6(update_info_1_5, update_info))
                {
                    is_parsed = false;
                    error_message.append(kStringErrorUnableToConvert1_5To1_6);
                }
            }
        }
        else if (json_doc[SCHEMAVERSION].get<std::string>().compare(SCHEMA_VERSION_1_6) == 0)
        {
            if (!ParseJsonSchema_1_6(json_doc, update_info, error_message))
            {
                is_parsed = false;
            }
        }
        else
        {
            is_parsed = false;
            error_message.append(kStringErrorUnsupportedSchemaVersion);
        }
    }
    catch (std::exception& e)
    {
        is_parsed = false;
        error_message.append(kStringFailedToParseVersionFile);
        error_message.append(e.what());
    }

    return is_parsed;
}

/// @brief Filter out (remove) the Releases that are not relevant to the current platform.
///
/// @param [in] update_info The update info struct.
///
/// @return true if there may be updates availble for the current platform; false otherwise.
static bool FilterToCurrentPlatform(UpdateCheck::UpdateInfo& update_info)
{
    bool may_have_updates_for_current_platform = true;

#ifdef WIN32
    const UpdateCheck::TargetPlatform current_platform = UpdateCheck::TargetPlatform::kWindows;
#elif __linux__
    const UpdateCheck::TargetPlatform current_platform = UpdateCheck::TargetPlatform::kUbuntu;
#elif __APPLE__
    const UpdateCheck::TargetPlatform current_platform = UpdateCheck::TargetPlatform::kDarwin;
#else
    const UpdateCheck::TargetPlatform current_platform = UpdateCheck::TargetPlatform::kUnknown;
#endif

    if (UpdateCheck::TargetPlatform::kUnknown != current_platform)
    {
        // Sort the releases based on platform. Releases that are for the current platform
        // will be moved to the beginning; releases that don't match will be moved to the end.
        auto new_end = std::remove_if(update_info.releases.begin(), update_info.releases.end(), [=](UpdateCheck::ReleaseInfo release) {
            return !std::any_of(release.target_platforms.begin(), release.target_platforms.end(), [=](UpdateCheck::TargetPlatform platform) {
                return platform == current_platform;
            });
        });

        // Now actually remove the releases that are not for the current platform.
        update_info.releases.erase(new_end, update_info.releases.end());

        // Updates may be available if there are still releases left.
        may_have_updates_for_current_platform = (update_info.releases.size() > 0);
    }

    return may_have_updates_for_current_platform;
}

/// @brief Parse a version string and populate the UpdateCheck::VersionInfo struct.
///
/// This function takes a version string in the format "major.minor.patch.build" and
/// extracts the individual components (major, minor, patch, and build) from the string.
/// It then stores these components in the provided UpdateCheck::VersionInfo struct.
///
/// @param [in]  version_str The input version string to be parsed.
/// @param [out] version     A reference to the UpdateCheck::VersionInfo struct to store the parsed version components.
///
/// @retval true if the version string is successfully parsed and all components are valid.
/// @retval false if the parsing fails or the version string format is incorrect.
static bool ParseVersionString(const std::string& version_str, UpdateCheck::VersionInfo& version)
{
    bool              ret = false;
    std::stringstream ss(version_str);
    char              dot;

    // Check if the version string can be parsed successfully.
    if (ss >> version.major && ss >> dot && dot == '.' && ss >> version.minor && ss >> dot && dot == '.' && ss >> version.patch && ss >> dot && dot == '.' &&
        ss >> version.build)
    {
        ret = true;
    }

    return ret;
}

/// @brief Get the tool version from the environment variable "RDTS_UPDATER_ASSUME_VERSION".
///
/// This function retrieves the tool version from the environment variable "RDTS_UPDATER_ASSUME_VERSION"
/// and parses it to extract the major, minor, patch, and build components.
///
/// @param [out] version A reference to the UpdateCheck::VersionInfo struct to store the retrieved or default version components.
///
/// @retval true if the tool version is successfully retrieved and parsed from the environment variable.
/// @retval false if the tool version retrieval fails or the version string format is incorrect.
static bool GetToolVersion(UpdateCheck::VersionInfo& version)
{
    bool        ret               = false;
    const char* env_variable_name = "RDTS_UPDATER_ASSUME_VERSION";
    char*       tool_version      = nullptr;

#ifdef _WIN32
    size_t required_size;
    getenv_s(&required_size, nullptr, 0, env_variable_name);

    if (required_size != 0)
    {
        tool_version = new (std::nothrow) char[required_size];
        if (tool_version != nullptr)
        {
            getenv_s(&required_size, tool_version, required_size, env_variable_name);
        }
    }

#else

    tool_version = std::getenv(env_variable_name);
#endif

    if (tool_version != nullptr)
    {
        ret = true;
        if (!ParseVersionString(tool_version, version))
        {
            version.major = 1;
            version.minor = 0;
            version.patch = 0;
            version.build = 0;
        }
#ifdef _WIN32
        delete[] tool_version;
#endif
    }
    return ret;
}

/// @brief API for checking the availability of product updates.
///
/// @param [in]  product_version     The current product version.
/// @param [in]  latest_releases_url The latest releases url.
/// @param [in]  json_filename       The json file name.
/// @param [in]  update_info         The update info struct.
/// @param [out] error_message       Any error messsages that occurred.
///
/// @return true on success.
/// @return true if checking for updates is successful; false otherwise.
bool UpdateCheck::CheckForUpdates(const UpdateCheck::VersionInfo& product_version,
                                  const std::string&              latest_releases_url,
                                  const std::string&              json_filename,
                                  UpdateCheck::UpdateInfo&        update_info,
                                  std::string&                    error_message)
{
    bool checked_for_update         = false;
    update_info.is_update_available = false;
    std::string loaded_json_contents;

    try
    {
        // Confirm a path to a JSON file was provided.
        bool is_json = (json_filename.rfind(kStringJsonFileExtension) != std::string::npos);
        assert(is_json);

        if (!is_json)
        {
            // The provided URL doesn't point to a supported file type.
            checked_for_update = false;
            error_message.append(kStringErrorUrlMustPointToAJsonFile);
        }
        else
        {
            if (latest_releases_url.find(kStringGithubReleasesLatest) != std::string::npos)
            {
                // Get JSON file from the latest release (using GitHub Release API).
                checked_for_update = LoadJsonFromLatestRelease(latest_releases_url, json_filename, loaded_json_contents, error_message);
            }
            else if (latest_releases_url.find(kStringHttpPrefix) == 0)
            {
                // Attempt to download JSON file contents.
                std::string full_url;

                // If a filename was included in the parameters, append that to the URL.
                if (json_filename.empty())
                {
                    full_url = latest_releases_url;
                }
                else
                {
                    full_url = latest_releases_url + "/" + json_filename;
                }

                checked_for_update = DownloadJsonFile(full_url, loaded_json_contents, error_message);
            }
            else
            {
                // Attempt to load the JSON file from disk.
                std::string full_path;
                if (latest_releases_url.empty())
                {
                    full_path = json_filename;
                }
                else
                {
                    full_path = latest_releases_url + "/" + json_filename;
                }

                checked_for_update = LoadJsonFile(full_path, loaded_json_contents, error_message);
            }

            if (checked_for_update)
            {
                // Parse the JSON string to populate the update_info struct.
                checked_for_update = ParseJsonString(loaded_json_contents, update_info, error_message);

                if (checked_for_update)
                {
                    bool has_compatible_update = FilterToCurrentPlatform(update_info);

                    if (has_compatible_update)
                    {
                        // Check to see if a version from the update is newer than the current product version.
                        UpdateCheck::VersionInfo version_to_compare;
                        if (!GetToolVersion(version_to_compare))
                        {
                            version_to_compare = product_version;
                        }

                        for (auto release_iter = update_info.releases.begin(); release_iter != update_info.releases.end(); ++release_iter)
                        {
                            update_info.is_update_available = (release_iter->version.Compare(version_to_compare) == kNewer);
                            if (update_info.is_update_available)
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
        checked_for_update = false;
        error_message.append(kStringErrorUnknownErrorOccurred);
        error_message.append(e.what());
    }

    return checked_for_update;
}

/// @brief Utility API to convert from a TargetPlatform enum to a string.
///
/// @param [in] target_platform The target platform value to convert to the string equivalent.
///
/// @return The string representation of the supplied target platform.
std::string UpdateCheck::TargetPlatformToString(const TargetPlatform target_platform)
{
    std::string text;

    switch (target_platform)
    {
    case TargetPlatform::kUnknown:
        text = kStringPlatformTypeUnknown;
        break;
    case TargetPlatform::kWindows:
        text = kStringPlatformTypeWindows;
        break;
    case TargetPlatform::kUbuntu:
        text = kStringPlatformTypeUbuntu;
        break;
    case TargetPlatform::kRhel:
        text = kStringPlatformTypeRhel;
        break;
    case TargetPlatform::kDarwin:
        text = kStringPlatformTypeDarwin;
        break;
    default:
        text = kStringUndefined;
        break;
    }

    return text;
}

/// @brief Utility API to convert from a PackageType enum to a string.
///
/// @param [in] package_type The package type value to convert to the string equivalent.
///
/// @return The string representation of the supplied package type.
std::string UpdateCheck::PackageTypeToString(const PackageType package_type)
{
    std::string text;

    switch (package_type)
    {
    case PackageType::kUnknown:
        text = kStringPackageTypeUnknown;
        break;
    case PackageType::kZip:
        text = kStringPackageTypeZip;
        break;
    case PackageType::kMsi:
        text = kStringPackageTypeMsi;
        break;
    case PackageType::kTar:
        text = kStringPackageTypeTar;
        break;
    case PackageType::kRpm:
        text = kStringPackageTypeRpm;
        break;
    case PackageType::kDebian:
        text = kStringPackageTypeDebian;
        break;
    default:
        text = kStringUndefined;
        break;
    }

    return text;
}

/// @brief Utility API to convert from a ReleaseType enum to a string.
///
/// @param [in] release_type The release type value to convert to the string equivalent.
///
/// @return The string representation of the supplied release type.
std::string UpdateCheck::ReleaseTypeToString(const ReleaseType release_type)
{
    std::string text;

    switch (release_type)
    {
    case ReleaseType::kUnknown:
        text = kStringReleaseTypeUnknown;
        break;
    case ReleaseType::kGeneralAvailability:
        text = kStringReleaseTypeGeneralAvailability;
        break;
    case ReleaseType::kBeta:
        text = kStringReleaseTypeBeta;
        break;
    case ReleaseType::kAlpha:
        text = kStringReleaseTypeAlpha;
        break;
    case ReleaseType::kPatch:
        text = kStringReleaseTypePatch;
        break;
    case ReleaseType::kDevelopment:
        text = kStringReleaseTypeDevelopment;
        break;
    default:
        text = kStringUndefined;
        break;
    }

    return text;
}
