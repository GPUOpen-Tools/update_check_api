//==============================================================================
// Copyright (c) 2019 Advanced Micro Devices, Inc. All rights reserved.
//
/// \author AMD Developer Tools Team
/// \brief A Qt Dialog to display the results of the UpdateCheckApi.
//==============================================================================
#include "UpdateCheckResultsDialog.h"
#include "ui_UpdateCheckResultsDialog.h"

// HTML Formatting strings.
static const char* STR_HTML_NEWLINE = "<br/>";
static const char* STR_HTML_START_UNORDERED_LIST = "<ul>";
static const char* STR_HTML_END_UNORDERED_LIST = "</ul>";
static const char* STR_HTML_START_LIST_ITEM = "<li>";
static const char* STR_HTML_END_LIST_ITEM = "</li>";
static const char* STR_HTML_A_HREF_START = "<a href=\"";
static const char* STR_HTML_A_HREF_END_TITLE_START = "\" title=\"";
static const char* STR_HTML_A_HREF_END_TITLE_END = "\">";
static const char* STR_HTML_A_CLOSE = "</a>";
static const char* STR_HTML_STRONG_OPEN = "<strong>";
static const char* STR_HTML_STRONG_CLOSE = "</strong>";
static const char* STR_HTML_DIV_INDENT_40_OPEN = "<div style=\"text-indent: 40px;\">";
static const char* STR_HTML_DIV_CLOSE = "</div>";
static const char* STR_DIALOG_TITLE = "Available Updates";
static const char* STR_UNABLE_TO_CHECK_FOR_UPDATES = "Unable to check for updates.";
static const char* STR_UPDATES_NO_UPDATE_AVAILABLE = "No updates available.";
static const char* STR_UPDATES_NEW_UPDATE_AVAILABLE = "New updates available: ";
static const char* STR_UPDATES_DOWNLOAD_THIS_RELEASE_FROM = "Download available in these formats:";
static const char* STR_UPDATES_FOR_MORE_INFORMATION_VISIT = "For more information, visit:";
static const char* STR_NEW_VERSION = "New version: ";
static const char* STR_RELEASE_DATE = "Release date: ";
static const char* STR_TAGS = "Tags: ";
static const char* STR_TAGS_SEPARATOR = ", ";

UpdateCheckResultsDialog::UpdateCheckResultsDialog(QWidget* pParent) :
    QDialog(pParent),
    ui(new Ui::UpdateCheckResultsDialog)
{
    ui->setupUi(this);

    // Remove the frame of the textbrowser.
    ui->textBrowser->setFrameStyle(QFrame::NoFrame);

    // Make the background of the textbrowser transparent.
    ui->textBrowser->setAttribute(Qt::WA_TranslucentBackground);
    ui->textBrowser->setStyleSheet("background: rgba(255,255,255,0%)");

    setWindowTitle(STR_DIALOG_TITLE);
}

UpdateCheckResultsDialog::~UpdateCheckResultsDialog()
{
    delete ui;
}

void UpdateCheckResultsDialog::SetResults(const UpdateCheck::UpdateInfo& updateInfo)
{
    QString updateResultHtml;

    if (!updateInfo.m_isUpdateAvailable)
    {
        updateResultHtml.append(STR_UPDATES_NO_UPDATE_AVAILABLE);
        updateResultHtml.append(STR_HTML_NEWLINE);
    }
    else
    {
        updateResultHtml.append(STR_UPDATES_NEW_UPDATE_AVAILABLE);
        updateResultHtml.append(STR_HTML_NEWLINE);
        updateResultHtml.append(STR_HTML_NEWLINE);

        for (auto releaseIter = updateInfo.m_releases.cbegin(); releaseIter != updateInfo.m_releases.cend(); ++releaseIter)
        {
            updateResultHtml.append(STR_HTML_STRONG_OPEN);
            updateResultHtml.append(releaseIter->m_title.c_str());
            updateResultHtml.append(STR_HTML_STRONG_CLOSE);
            updateResultHtml.append(STR_HTML_NEWLINE);
            updateResultHtml.append(STR_HTML_NEWLINE);
            updateResultHtml.append(STR_NEW_VERSION);
            updateResultHtml.append(releaseIter->m_version.ToString().c_str());
            updateResultHtml.append(" (");
            updateResultHtml.append(UpdateCheck::ReleaseTypeToString(releaseIter->m_type).c_str());
            updateResultHtml.append(")");
            updateResultHtml.append(STR_HTML_NEWLINE);
            updateResultHtml.append(STR_RELEASE_DATE);
            updateResultHtml.append(releaseIter->m_date.c_str());
            updateResultHtml.append(STR_HTML_NEWLINE);

            if (!releaseIter->m_tags.empty())
            {
                updateResultHtml.append(STR_TAGS);
                updateResultHtml.append(releaseIter->m_tags[0].c_str());

                for (size_t i = 1; i != releaseIter->m_tags.size(); ++i)
                {
                    updateResultHtml.append(STR_TAGS_SEPARATOR);
                    updateResultHtml.append(releaseIter->m_tags[i].c_str());
                }
            }

            updateResultHtml.append(STR_HTML_NEWLINE);
            updateResultHtml.append(STR_HTML_NEWLINE);

            // Display download links.
            if (!releaseIter->m_downloadLinks.empty())
            {
                updateResultHtml.append(STR_UPDATES_DOWNLOAD_THIS_RELEASE_FROM);
                updateResultHtml.append(STR_HTML_NEWLINE);

                // Sample output: "Windows: [MSI] [ZIP] [ZIP]" with the full hyperlink as a tooltip over the MSI, ZIP, ZIP to allow
                // users to distinquish between the two ZIP files.
                for (auto platformIter = releaseIter->m_targetPlatforms.cbegin(); platformIter != releaseIter->m_targetPlatforms.cend(); ++platformIter)
                {
                    updateResultHtml.append(STR_HTML_DIV_INDENT_40_OPEN);
                    updateResultHtml.append(UpdateCheck::TargetPlatformToString(*platformIter).c_str());
                    updateResultHtml.append(":");

                    for (auto iter = releaseIter->m_downloadLinks.cbegin(); iter != releaseIter->m_downloadLinks.cend(); ++iter)
                    {
                        //updateResultHtml.append(STR_HTML_START_LIST_ITEM);
                        updateResultHtml.append(" [");
                        updateResultHtml.append(STR_HTML_A_HREF_START);
                        updateResultHtml.append(iter->m_url.c_str());
                        updateResultHtml.append(STR_HTML_A_HREF_END_TITLE_START);
                        updateResultHtml.append(iter->m_url.c_str());
                        updateResultHtml.append(STR_HTML_A_HREF_END_TITLE_END);
                        updateResultHtml.append(UpdateCheck::PackageTypeToString(iter->m_packageType).c_str());
                        updateResultHtml.append(STR_HTML_A_CLOSE);
                        updateResultHtml.append("]");
                    }

                    updateResultHtml.append(STR_HTML_DIV_CLOSE);
                }

                updateResultHtml.append(STR_HTML_NEWLINE);
            }

            // Diplay additional info links.
            if (!releaseIter->m_infoLinks.empty())
            {
                updateResultHtml.append(STR_UPDATES_FOR_MORE_INFORMATION_VISIT);
                updateResultHtml.append(STR_HTML_START_UNORDERED_LIST);

                for (auto iter = releaseIter->m_infoLinks.cbegin(); iter != releaseIter->m_infoLinks.cend(); ++iter)
                {
                    updateResultHtml.append(STR_HTML_START_LIST_ITEM);
                    updateResultHtml.append(STR_HTML_A_HREF_START);
                    updateResultHtml.append(iter->m_url.c_str());
                    updateResultHtml.append(STR_HTML_A_HREF_END_TITLE_START);
                    updateResultHtml.append(iter->m_url.c_str());
                    updateResultHtml.append(STR_HTML_A_HREF_END_TITLE_END);
                    updateResultHtml.append(iter->m_pageDescription.c_str());
                    updateResultHtml.append(STR_HTML_A_CLOSE);
                    updateResultHtml.append(STR_HTML_END_LIST_ITEM);
                }

                updateResultHtml.append(STR_HTML_END_UNORDERED_LIST);
            }
        }
    }

    // Update the QTextBrowser with the HTML that was built above.
    ui->textBrowser->setHtml(updateResultHtml);
}

void UpdateCheckResultsDialog::SetResults(const UpdateCheck::Results& results)
{
    if (!results.wasCheckSuccessful)
    {
        QString updateResultHtml;
        updateResultHtml.append(STR_UNABLE_TO_CHECK_FOR_UPDATES);
        updateResultHtml.append(STR_HTML_NEWLINE);
        updateResultHtml.append(results.errorMessage);

        // Update the QTextBrowser with the HTML that was built above.
        ui->textBrowser->setHtml(updateResultHtml);
    }
    else
    {
        SetResults(results.updateInfo);
    }
}
