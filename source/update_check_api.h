//==============================================================================
/// Copyright (c) 2018-2020 Advanced Micro Devices, Inc. All rights reserved.
/// \author AMD Developer Tools Team
/// \file
/// \brief An interface to allow applications to check for updated versions.
//==============================================================================
#ifndef UPDATECHECKAPI_UPDATE_CHECK_API_H_
#define UPDATECHECKAPI_UPDATE_CHECK_API_H_

#include <string>
#include <vector>

// Versioning information of the UpdateCheckAPI.
#define UPDATECHECKAPI_MAJOR 2
#define UPDATECHECKAPI_MINOR 0
#define UPDATECHECKAPI_PATCH 0
#define UPDATECHECKAPI_BUILD 0

namespace UpdateCheck
{
    /// The platforms which an update package may target.
    enum class TargetPlatform
    {
        kUnknown = 0,
        kWindows,
        kUbuntu,
        kRhel,
        kDarwin
    };

    /// The types of an update package (archive, installer, etc.).
    enum class PackageType
    {
        kUnknown = 0,
        kZip,
        kMsi,
        kTar,
        kRpm,
        kDebian
    };

    /// The type of the release: General Availabilty, Beta, Alpha, a patch release or a development build.
    enum class ReleaseType
    {
        kUnknown = 0,
        kGeneralAvailability,
        kBeta,
        kAlpha,
        kPatch,

        // Development build (used for testing).
        kDevelopment
    };

    /// A structure representing a version of the format Major.Minor.Patch.Build.
    struct VersionInfo
    {
        /// Get the string representation of the version ("Major.Minor.Patch.Build").
        std::string ToString() const;

        /// \brief Compare this version to another version.
        ///
        /// \param other The other version info to compare against.
        /// \retval 1 if this version is newer.
        /// \retval -1 if this version is older.
        /// \retval 0 if this version is equal to the other version.
        int Compare(const VersionInfo& other) const;

        /// The Major component of the version.
        uint32_t major;

        /// The Minor component of the version.
        uint32_t minor;

        /// The Patch component of the version.
        uint32_t patch;

        /// The Build component of the version.
        uint32_t build;
    };

    /// This structure represents a link to a pages that may accompany the
    /// notification about an available prouct update, which is presented to the user.
    struct InfoPageLink
    {
        /// The URL of the relevant page.
        std::string url;

        /// A description of the page (for instance, "RGA Releases Page" or "RGP Product Page").
        std::string page_description;
    };

    /// A structure containing a url from which to download a release, along
    /// with the type of package pointed to by the URL.
    struct DownloadLink
    {
        /// The URL from which the archive/installer can be downloaded.
        std::string url;

        /// A value describing the kind of archive/installer that url points to.
        PackageType package_type;
    };

    /// A structure containing all the data pertaining to a specific release.
    struct ReleaseInfo
    {
        /// The version of the available update.
        VersionInfo version;

        /// The release date of the available update in the format YYYY-MM-DD.
        std::string date;

        /// Text describing the available update, may include release notes highlights for example.
        std::string title;

        /// The target platforms to which the packge referenced by this link is relevant.
        std::vector<TargetPlatform> target_platforms;

        /// The type of the release for the package that is referenced by this link.
        ReleaseType type;

        /// Arbitrary string tags that can help identify a particular release.
        std::vector<std::string> tags;

        /// The available update packages: package type and URL from which they can be downloaded.
        std::vector<DownloadLink> download_links;

        /// Links to relevant pages, like product landing page on GPUOpen, or Releases page on Github.com.
        std::vector<InfoPageLink> info_links;
    };

    /// A structure containing a collection of releases, and a flag to indicate
    /// if at least one of the releases is an update to the current version.
    struct UpdateInfo
    {
        /// True if an update to a newer version is available, false otherwise.
        bool is_update_available;

        /// List of releases available during this update.
        std::vector<ReleaseInfo> releases;
    };

    /// Get API Version information.
    /// \return Current version information.
    VersionInfo GetApiVersionInfo();

    /// API for checking the availability of product updates.
    bool CheckForUpdates(const VersionInfo& current_product_version,
                         const std::string& latest_release_url,
                         const std::string& json_filename,
                         UpdateInfo&        update_info,
                         std::string&       error_message);

    /// Utility API to convert from a TargetPlatform enum to a string.
    std::string TargetPlatformToString(const TargetPlatform target_platform);

    /// Utility API to convert from a PackageType enum to a string.
    std::string PackageTypeToString(const PackageType package_type);

    /// Utility API to convert from a ReleaseType enum to a string.
    std::string ReleaseTypeToString(const ReleaseType release_type);
}  // namespace UpdateCheck

#endif  // UPDATECHECKAPI_UPDATE_CHECK_API_H_
