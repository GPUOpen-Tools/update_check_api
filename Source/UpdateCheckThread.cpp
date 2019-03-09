//==============================================================================
// Copyright (c) 2019 Advanced Micro Devices, Inc. All rights reserved.
/// \author AMD Developer Tools Team
//
/// \file
//
/// \brief A Qt Thread to check for updates in the background.
//==============================================================================

#include "UpdateCheckThread.h"

namespace UpdateCheck
{
    Worker::Worker(uint32_t currentMajorVersion, uint32_t currentMinorVersion, uint32_t currentBuildVersion, uint32_t currentPatchVersion)
        : QObject(nullptr)
    {
        qRegisterMetaType<UpdateCheck::Results>("UpdateCheck::Results");
        m_rgaVersionInfo = { currentMajorVersion, currentMinorVersion, currentBuildVersion, currentPatchVersion };
    }

    // Hidden default constructor.
    Worker::Worker()
    {
    }

    Worker::~Worker()
    {
    }

    void Worker::DoCheckForUpdates(const QString& latestReleasesURL, const QString& updatesAssetFilename)
    {
        UpdateCheck::Results updateCheckResults;

        // Check for updates.
        std::string tmpErrorMessage;
        updateCheckResults.wasCheckSuccessful = UpdateCheck::CheckForUpdates(m_rgaVersionInfo, latestReleasesURL.toStdString(), updatesAssetFilename.toStdString(), updateCheckResults.updateInfo, tmpErrorMessage);

        // Store the error message.
        updateCheckResults.errorMessage = tmpErrorMessage.c_str();

        emit ResultReady(updateCheckResults);
    }

    ThreadController::ThreadController(QObject* pParent, uint32_t currentMajorVersion, uint32_t currentMinorVersion, uint32_t currentBuildVersion, uint32_t currentPatchVersion)
        : QObject(pParent),
        m_wasCancelled(false)
    {
        m_thread.setObjectName("CheckForUpdatesThread");

        // Create the worker object, and move it to a thread which will be run in the background.
        Worker* pWorker = new Worker(currentMajorVersion, currentMinorVersion, currentBuildVersion, currentPatchVersion);
        pWorker->moveToThread(&m_thread);

        // When the thread is finished, make sure the worker object gets deleted.
        connect(&m_thread, &QThread::finished, pWorker, &QObject::deleteLater);

        // When a caller wants to start the check for updates, do it.
        connect(this, &ThreadController::DoCheckForUpdates, pWorker, &Worker::DoCheckForUpdates);

        // When the results are ready from the background thread, notify any listeners.
        connect(pWorker, &Worker::ResultReady, this, &ThreadController::ThreadFinished);

        // Start the background thread. This just creates it and lets it sit in the background
        // but does not actually start the check for updates.
        m_thread.start();
    }

    ThreadController::~ThreadController()
    {
        // When the controlling thread object is being destroyed,
        // tell the background thread to quit (in case it's still running)
        // and then wait for it to actually end.
        m_thread.quit();
        m_thread.wait();
    }

    void ThreadController::StartCheckForUpdates(const QString& latestReleasesURL, const QString& updatesAssetFilename)
    {
        m_wasCancelled = false;
        emit DoCheckForUpdates(latestReleasesURL, updatesAssetFilename);
    }

    void ThreadController::CancelCheckForUpdates()
    {
        if (m_thread.isRunning())
        {
            m_wasCancelled = true;
            m_thread.quit();
        }
        else
        {
            // Do we need to emit a CheckForUpdatesCancelled() here?
        }
    }

    void ThreadController::ThreadFinished(const UpdateCheck::Results& updateCheckResults)
    {
        if (m_wasCancelled)
        {
            emit CheckForUpdatesCancelled(this);
        }
        else
        {
            emit CheckForUpdatesComplete(this, updateCheckResults);
        }
    }
}
