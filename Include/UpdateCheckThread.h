//==============================================================================
// Copyright (c) 2019 Advanced Micro Devices, Inc. All rights reserved.
/// \author AMD Developer Tools Team
//
/// \file
//
/// \brief A Qt Thread to check for updates in the background.
//==============================================================================
#pragma once

#include <UpdateCheckApi.h>
#include <QThread>

namespace UpdateCheck
{
    // Contains the results of a check for updates and gets passed back to the
    // application through Qt slots & signals. If the check for updates gets
    // cancelled, a different signal is sent and these results are not
    // available. If the check fails, wasCheckSuccessful will be false and a
    // description of the error will be in errorMessage. If the check succeeeds
    // wasCheckSuccessful will be true, and the information about the available
    // versions will be in updateInfo. UpdateInfo also contains a boolean that
    // indicates whether or not the info is considered an update to the current
    // version of the application.
    struct Results
    {
        // Stores whether the CheckForUpdates was successful.
        bool wasCheckSuccessful;

        // If wasCheckSuccessful is false, this holds an error message
        // to understand what went wrong.
        QString errorMessage;

        // If wasCheckSuccessful is true, this will contain information
        // about the latest available version and whether it is considered
        // and update to the current version.
        UpdateCheck::UpdateInfo updateInfo;
    };

    // This is the worker object that gets executed in the background thread.
    // Only the ThreadController should ever interact with the Worker, not the
    // application.
    class Worker : public QObject
    {
        Q_OBJECT

    public:

        // Constructor which accepts the current application version number.
        // This allows the worker to do the comparison to determine if any new
        // versions are considered an update to the current version.
        Worker(uint32_t currentMajorVersion, uint32_t currentMinorVersion, uint32_t currentBuildVersion, uint32_t currentPatchVersion);

        // Virtual destructor.
        virtual ~Worker();

    public slots:
        // Activate this slot to perform the check for updates. If used
        // properly, this worker object will have already been moved to a
        // background thread, and therefore this function will also execute
        // in the background thread.
        void DoCheckForUpdates(const QString& latestReleasesURL, const QString& updatesAssetFilename);

    signals:
        // Signals that the check for updates has completed (either
        // successfully or not) and that a result is available.
        void ResultReady(const UpdateCheck::Results& updateCheckResults);

    private:
        // Hide the default constructor.
        Worker();

        // Stores the current RGA Version number while the CheckForUpdates is running.
        UpdateCheck::VersionInfo m_rgaVersionInfo;
    };

    // Controller object that creates the background thread and interacts with
    // the worker object to start the check for updates, and to receive the
    // results. This helps to simplify the integration of the check for updates
    // functionality into applications. Applications should interact with this
    // Object. When the ThreadController is created, a background thread is
    // also spawned, but not put to work immediately. In order to stop the
    // background thread, this object should be destroyed. Multiple of these
    // objects (and threads) can safely exist at the same time.
    class ThreadController : public QObject
    {
        Q_OBJECT

    public:
        // Constructor which takes in a parent object, and the current version
        // numbers. By supplying a parent object, this thread will
        // automatically get deleted when the parent is deleted, however it is
        // recommended to delete the ThreadController once it has notified the
        // application that it was cancelled or that it has completed. If the
        // check for updates has been cancelled, then the thread is no longer
        // useable and should be deleted. However, if the check for updates
        // completes normally (regardless of whether it was successful), the
        // ThreadController can be re-used to check for an update again.
        ThreadController(QObject* pParent, uint32_t currentMajorVersion, uint32_t currentMinorVersion, uint32_t currentBuildVersion, uint32_t currentPatchVersion);

        // Virtual destructor.
        virtual ~ThreadController();

    signals:

        // This should NOT be used by an application. See StartCheckForUpdates.
        // This signal is for communication between the ThreadController and
        // the worker object to signal the CheckForUpdates to start in the
        // background thread.
        void DoCheckForUpdates(const QString& latestReleasesURL, const QString& updatesAssetFilename);

        // Emitted by the background thread when the CheckForUpdates has completed.
        void CheckForUpdatesComplete(UpdateCheck::ThreadController* thread, const UpdateCheck::Results& updateCheckResults);

        // Emitted when the CheckForUpdates has completed due to being cancelled.
        // The resulting UpdateInfo will not be reliable if this signal is emitted.
        void CheckForUpdatesCancelled(UpdateCheck::ThreadController* thread);

    public slots:

        // For use by an application to start the check for updates. This slot
        // will make sure that certain flags are set to properly support the
        // ability to cancel a thread.
        void StartCheckForUpdates(const QString& latestReleasesURL, const QString& updatesAssetFilename);

        // For use by an application to request that the check for updates be
        // cancelled. This is an asynchronous request, so the application
        // should wait until it receives the CheckForUpdatesCancelled signal.
        void CancelCheckForUpdates();

    private slots:
        // This should NOT be used by an application. See CheckForUpdatesComplete.
        // This notifies the ThreadController that the worker object has
        // finished the check for Updates. The supplied results may not be
        // accurate if the thread was cancelled.
        void ThreadFinished(const UpdateCheck::Results& updateCheckResults);

    private:
        // Indicates that the thread controller received a request to cancel
        // the check for updates.
        bool m_wasCancelled;

        // The background thread that will execute the check for updates.
        QThread m_thread;
    };
}
