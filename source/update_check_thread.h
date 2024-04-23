//==============================================================================
/// Copyright (c) 2019-2024 Advanced Micro Devices, Inc. All rights reserved.
/// @author AMD Developer Tools Team
/// @file
/// @brief A Qt Thread to check for updates in the background.
//==============================================================================
#ifndef UPDATECHECKAPI_UPDATE_CHECK_THREAD_H_
#define UPDATECHECKAPI_UPDATE_CHECK_THREAD_H_

#include "update_check_api.h"
#include <QThread>

namespace UpdateCheck
{
    /// @brief Contains the results of a check for updates and gets passed back to the application through Qt slots & signals.
    ///
    /// If the check for updates gets
    /// cancelled, a different signal is sent and these results are not
    /// available. If the check fails, wasCheckSuccessful will be false and a
    /// description of the error will be in error_message. If the check succeeeds
    /// wasCheckSuccessful will be true, and the information about the available
    /// versions will be in update_info. UpdateInfo also contains a boolean that
    /// indicates whether or not the info is considered an update to the current
    /// version of the application.
    struct Results
    {
        /// Stores whether the CheckForUpdates was successful.
        bool was_check_successful;

        /// If wasCheckSuccessful is false, this holds an error message
        /// to understand what went wrong.
        QString error_message;

        /// If wasCheckSuccessful is true, this will contain information
        /// about the latest available version and whether it is considered
        /// and update to the current version.
        UpdateCheck::UpdateInfo update_info;
    };

    /// @brief This is the worker object that gets executed in the background thread.
    ///
    /// Only the ThreadController should ever interact with the Worker, not the
    /// application.
    class Worker : public QObject
    {
        Q_OBJECT

    public:
        /// @brief Constructor which accepts the current application version number.
        ///
        /// This allows the worker to do the comparison to determine if any new
        /// versions are considered an update to the current version.
        ///
        /// @param [in] current_major_version The current major version.
        /// @param [in] current_minor_version The current minor version.
        /// @param [in] current_build_version The current build version.
        /// @param [in] current_patch_version The current patch version.
        Worker(uint32_t current_major_version, uint32_t current_minor_version, uint32_t current_build_version, uint32_t current_patch_version);

        /// @brief Virtual destructor.
        virtual ~Worker();

    public slots:
        /// @brief Activate this slot to perform the check for updates.
        ///
        /// If used properly, this worker object will have already been moved to a
        /// background thread, and therefore this function will also execute
        /// in the background thread.
        ///
        /// @param [in] latest_releases_url    The latest releases url.
        /// @param [in] updates_asset_filename The updates asset filename.
        void DoCheckForUpdates(const QString& latest_releases_url, const QString& updates_asset_filename);

    signals:
        /// @brief Signals that the check for updates has completed (either successfully or not) and that a result is available.
        ///
        /// @param [in] update_check_results The results of the check for updates.
        void ResultReady(const UpdateCheck::Results& update_check_results);

    private:
        /// @brief Hide the default constructor.
        Worker();

        /// Stores the current version number while the CheckForUpdates is running.
        UpdateCheck::VersionInfo version_info_;
    };

    /// @brief Controller object that creates the background thread and interacts with the worker object to start the check for updates, and to receive the results.
    ///
    /// This helps to simplify the integration of the check for updates
    /// functionality into applications. Applications should interact with this
    /// Object. When the ThreadController is created, a background thread is
    /// also spawned, but not put to work immediately. In order to stop the
    /// background thread, this object should be destroyed. Multiple of these
    /// objects (and threads) can safely exist at the same time.
    class ThreadController : public QObject
    {
        Q_OBJECT

    public:
        /// @brief Constructor which takes in a parent object, and the current version numbers.
        ///
        /// By supplying a parent object, this thread will
        /// automatically get deleted when the parent is deleted, however it is
        /// recommended to delete the ThreadController once it has notified the
        /// application that it was cancelled or that it has completed. If the
        /// check for updates has been cancelled, then the thread is no longer
        /// useable and should be deleted. However, if the check for updates
        /// completes normally (regardless of whether it was successful), the
        /// ThreadController can be re-used to check for an update again.
        ///
        /// @param [in] parent                The parent QObject.
        /// @param [in] current_major_version The current major version.
        /// @param [in] current_minor_version The current minor version.
        /// @param [in] current_build_version The current build version.
        /// @param [in] current_patch_version The current patch version.
        ThreadController(QObject* parent,
                         uint32_t current_major_version,
                         uint32_t current_minor_version,
                         uint32_t current_build_version,
                         uint32_t current_patch_version);

        /// @brief Virtual destructor.
        virtual ~ThreadController();

    signals:

        /// @brief This should NOT be used by an application. See StartCheckForUpdates.
        ///
        /// This signal is for communication between the ThreadController and
        /// the worker object to signal the CheckForUpdates to start in the
        /// background thread.
        ///
        /// @param [in] latest_releases_url    The latest releases url.
        /// @param [in] updates_asset_filename The updates asset filename.
        void DoCheckForUpdates(const QString& latest_releases_url, const QString& updates_asset_filename);

        /// @brief Emitted by the background thread when the CheckForUpdates has completed.
        ///
        /// @param [in] thread               The thread controller used to check for updates.
        /// @param [in] update_check_results The results of the check for updates.
        void CheckForUpdatesComplete(UpdateCheck::ThreadController* thread, const UpdateCheck::Results& update_check_results);

        /// @brief Emitted when the CheckForUpdates has completed due to being cancelled.
        ///
        /// The resulting UpdateInfo will not be reliable if this signal is emitted.
        ///
        /// @param [in] thread The thread controller used to check for updates.
        void CheckForUpdatesCancelled(UpdateCheck::ThreadController* thread);

    public slots:

        /// @brief For use by an application to start the check for updates.
        ///
        /// This slot will make sure that certain flags are set to properly support the
        /// ability to cancel a thread.
        ///
        /// @param [in]  latest_releases_url    The latest releases url.
        /// @param [in]  updates_asset_filename The updates asset filename.
        void StartCheckForUpdates(const QString& latest_releases_url, const QString& updates_asset_filename);

        /// @brief For use by an application to request that the check for updates be cancelled.
        ///
        /// This is an asynchronous request, so the application
        /// should wait until it receives the CheckForUpdatesCancelled signal.
        void CancelCheckForUpdates();

    private slots:
        /// @brief This should NOT be used by an application. See CheckForUpdatesComplete.
        ///
        /// This notifies the ThreadController that the worker object has
        /// finished the check for Updates. The supplied results may not be
        /// accurate if the thread was cancelled.
        ///
        /// @param [in] update_check_results The results of the check for updates.
        void ThreadFinished(const UpdateCheck::Results& update_check_results);

    private:
        /// Indicates that the thread controller received a request to cancel
        /// the check for updates.
        bool was_cancelled_;

        /// The background thread that will execute the check for updates.
        QThread thread_;
    };
}  // namespace UpdateCheck
#endif  // UPDATECHECKAPI_UPDATE_CHECK_THREAD_H_
