//==============================================================================
/// Copyright (c) 2019-2020 Advanced Micro Devices, Inc. All rights reserved.
/// \author AMD Developer Tools Team
/// \file
/// \brief A Qt Thread to check for updates in the background.
//==============================================================================

#include "update_check_thread.h"

namespace UpdateCheck
{
    Worker::Worker(uint32_t current_major_version, uint32_t current_minor_version, uint32_t current_build_version, uint32_t current_patch_version)
        : QObject(nullptr)
    {
        qRegisterMetaType<UpdateCheck::Results>("UpdateCheck::Results");
        version_info_ = {current_major_version, current_minor_version, current_patch_version, current_build_version};
    }

    // Hidden default constructor.
    Worker::Worker()
    {
    }

    Worker::~Worker()
    {
    }

    void Worker::DoCheckForUpdates(const QString& latest_releases_url, const QString& updates_asset_filename)
    {
        UpdateCheck::Results update_check_results;

        // Check for updates.
        std::string temp_error_message;
        update_check_results.was_check_successful = UpdateCheck::CheckForUpdates(
            version_info_, latest_releases_url.toStdString(), updates_asset_filename.toStdString(), update_check_results.update_info, temp_error_message);

        // Store the error message.
        update_check_results.error_message = temp_error_message.c_str();

        emit ResultReady(update_check_results);
    }

    ThreadController::ThreadController(QObject* parent,
                                       uint32_t current_major_version,
                                       uint32_t current_minor_version,
                                       uint32_t current_build_version,
                                       uint32_t current_patch_version)
        : QObject(parent)
        , was_cancelled_(false)
    {
        thread_.setObjectName("CheckForUpdatesThread");

        // Create the worker object, and move it to a thread which will be run in the background.
        Worker* worker_object = new Worker(current_major_version, current_minor_version, current_build_version, current_patch_version);
        worker_object->moveToThread(&thread_);

        // When the thread is finished, make sure the worker object gets deleted.
        connect(&thread_, &QThread::finished, worker_object, &QObject::deleteLater);

        // When a caller wants to start the check for updates, do it.
        connect(this, &ThreadController::DoCheckForUpdates, worker_object, &Worker::DoCheckForUpdates);

        // When the results are ready from the background thread, notify any listeners.
        connect(worker_object, &Worker::ResultReady, this, &ThreadController::ThreadFinished);

        // Start the background thread. This just creates it and lets it sit in the background
        // but does not actually start the check for updates.
        thread_.start();
    }

    ThreadController::~ThreadController()
    {
        // When the controlling thread object is being destroyed,
        // tell the background thread to quit (in case it's still running)
        // and then wait for it to actually end.
        thread_.quit();
        thread_.wait();
    }

    void ThreadController::StartCheckForUpdates(const QString& latest_releases_url, const QString& updates_asset_filename)
    {
        was_cancelled_ = false;
        emit DoCheckForUpdates(latest_releases_url, updates_asset_filename);
    }

    void ThreadController::CancelCheckForUpdates()
    {
        if (thread_.isRunning())
        {
            was_cancelled_ = true;
            thread_.quit();
        }
        else
        {
            // Do we need to emit a CheckForUpdatesCancelled() here?
        }
    }

    void ThreadController::ThreadFinished(const UpdateCheck::Results& update_check_results)
    {
        if (was_cancelled_)
        {
            emit CheckForUpdatesCancelled(this);
        }
        else
        {
            emit CheckForUpdatesComplete(this, update_check_results);
        }
    }
}  // namespace UpdateCheck
