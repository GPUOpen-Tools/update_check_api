//==============================================================================
/// Copyright (c) 2018-2020 Advanced Micro Devices, Inc. All rights reserved.
/// \author AMD Developer Tools Team
/// \brief An interface to allow applications to check for updated versions.
//==============================================================================
#ifndef _UPDATECHECKAPI_H_
#define _UPDATECHECKAPI_H_

#include <string>
#include <vector>

// Versioning information of the UpdateCheckAPI.
#define UPDATECHECKAPI_MAJOR 1
#define UPDATECHECKAPI_MINOR 0
#define UPDATECHECKAPI_PATCH 0
#define UPDATECHECKAPI_BUILD 0

namespace UpdateCheck
{
    // The platforms which an update package may target.
    enum class TargetPlatform
    {
        Unknown = 0,
        Windows,
        Ubuntu,
        RHEL,
        Darwin
    };

    // The types of an update package (archive, installer, etc.).
    enum class PackageType
    {
        Unknown = 0,
        ZIP,
        MSI,
        TAR,
        RPM,
        Debian
    };

    // The type of the release: General Availabilty, Beta, Alpha, a patch release or a development build.
    enum class ReleaseType
    {
        Unknown = 0,
        GA,
        Beta,
        Alpha,
        Patch,

        // Development build (used for testing).
        Development
    };

    // A structure representing a version of the format Major.Minor.Patch.Build.
    struct VersionInfo
    {
        // Get the string representation of the version ("Major.Minor.Patch.Build").
        std::string ToString() const;

        // Compare this version to another version.
        // Returns 1 if this version is newer.
        // Returns -1 if this version is older.
        // Returns 0 if this version is equal to the other version.
        int Compare(const VersionInfo& other) const;

        // The Major component of the version.
        uint32_t m_major;

        // The Minor component of the version.
        uint32_t m_minor;

        // The Patch component of the version.
        uint32_t m_patch;

        // The Build component of the version.
        uint32_t m_build;
    };

    // This structure represents a link to a pages that may accompany the
    // notification about an available prouct update, which is presented to the user.
    struct InfoPageLink
    {
        // The URL of the relevant page.
        std::string m_url;

        // A description of the page (for instance, "RGA Releases Page" or "RGP Product Page").
        std::string m_pageDescription;
    };

    // A structure containing a url from which to download a release, along
    // with the type of package pointed to by the URL.
    struct DownloadLink
    {
        // The URL from which the archive/installer can be downloaded.
        std::string m_url;

        // A value describing the kind of archive/installer that m_url points to.
        PackageType m_packageType;
    };

    // A structure containing all the data pertaining to a specific release.
    struct ReleaseInfo
    {
        // The version of the available update.
        VersionInfo m_version;

        // The release date of the available update in the format YYYY-MM-DD.
        std::string m_date;

        // Text describing the available update, may include release notes highlights for example.
        std::string m_title;

        // The target platforms to which the packge referenced by this link is relevant.
        std::vector<TargetPlatform> m_targetPlatforms;

        // The type of the release for the package that is referenced by this link.
        ReleaseType m_type;

        // Arbitrary string tags that can help identify a particular release.
        std::vector<std::string> m_tags;

        // The available update packages: package type and URL from which they can be downloaded.
        std::vector<DownloadLink> m_downloadLinks;

        // Links to relevant pages, like product landing page on GPUOpen, or Releases page on Github.com.
        std::vector<InfoPageLink> m_infoLinks;
    };

    // A structure containing a collection of releases, and a flag to indicate
    // if at least one of the releases is an update to the current version.
    struct UpdateInfo
    {
        // True if an update to a newer version is available, false otherwise.
        bool m_isUpdateAvailable;

        // List of releases available during this update.
        std::vector<ReleaseInfo> m_releases;
    };

    // Get version info of the UpdateCheckAPI.
    VersionInfo GetApiVersionInfo();

    // API for checking the availability of product updates.
    bool CheckForUpdates(const VersionInfo& currentProductVersion, const std::string& latestReleaseUrl, const std::string& jsonFileName, UpdateInfo& updateInfo, std::string& errMsg);

    // Utility API to convert from a TargetPlatform enum to a string.
    std::string TargetPlatformToString(const TargetPlatform targetPlatform);

    // Utility API to convert from a PackageType enum to a string.
    std::string PackageTypeToString(const PackageType packageType);

    // Utility API to convert from a ReleaseType enum to a string.
    std::string ReleaseTypeToString(const ReleaseType releaseType);
}

#endif //_UPDATECHECKAPI_H_
