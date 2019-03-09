//==============================================================================
// Copyright (c) 2019 Advanced Micro Devices, Inc. All rights reserved.
/// \author AMD Developer Tools Team
//
/// \file
//
/// \brief A Qt Dialog to display the results of the UpdateCheckApi.
//==============================================================================
#pragma once

#include <QDialog>

#include "UpdateCheckThread.h"

namespace Ui {
class UpdateCheckResultsDialog;
}

class UpdateCheckResultsDialog : public QDialog
{
    Q_OBJECT

public:
    explicit UpdateCheckResultsDialog(QWidget* pParent = 0);
    ~UpdateCheckResultsDialog();

    void SetResults(const UpdateCheck::Results& results);
    void SetResults(const UpdateCheck::UpdateInfo& updateInfo);

private:
    Ui::UpdateCheckResultsDialog *ui;
};
