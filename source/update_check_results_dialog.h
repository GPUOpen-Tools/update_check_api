//==============================================================================
/// Copyright (c) 2019-2020 Advanced Micro Devices, Inc. All rights reserved.
/// \author AMD Developer Tools Team
/// \file
/// \brief A Qt Dialog to display the results of the UpdateCheckApi.
//==============================================================================
#ifndef UPDATECHECKAPI_UPDATE_CHECK_RESULTS_DIALOG_H_
#define UPDATECHECKAPI_UPDATE_CHECK_RESULTS_DIALOG_H_

#include <QDialog>

#include "update_check_thread.h"

namespace Ui {
class UpdateCheckResultsDialog;
}

class UpdateCheckResultsDialog : public QDialog
{
    Q_OBJECT

public:
    /// \brief Constructor
    /// \param parent The parent widget.
    explicit UpdateCheckResultsDialog(QWidget* parent = 0);

    /// \brief Virtual destructor
    virtual ~UpdateCheckResultsDialog();

    /// \brief Sets the results to display in the dialog.
    /// \param results The results to display in the dialog.
    void SetResults(const UpdateCheck::Results& results);

    /// \brief Sets the results to display in the dialog.
    /// \param update_info The update information to display.
    void SetResults(const UpdateCheck::UpdateInfo& update_info);

    /// \brief Controls whether or not to show the tags on the results dialog.
    /// \param show_tags Indicates whether or not to show the tags on the results dialog.
    void SetShowTags(bool show_tags);

private:

    /// The user interface.
    Ui::UpdateCheckResultsDialog *ui_;

    /// Indicates whether or not to show the tags on the results dialog.
    bool show_tags_ = true;
};
#endif // UPDATECHECKAPI_UPDATE_CHECK_RESULTS_DIALOG_H_
