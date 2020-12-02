//==============================================================================
/// Copyright (c) 2019-2020 Advanced Micro Devices, Inc. All rights reserved.
/// \author AMD Developer Tools Team
/// \file
/// \brief A Qt Dialog to display the results of the UpdateCheckApi.
//==============================================================================
#include "update_check_results_dialog.h"
#include "ui_update_check_results_dialog.h"

// HTML Formatting strings.
static const char* kStringHtmlNewline                    = "<br/>";
static const char* kStringhtmlStartUnorderedList         = "<ul>";
static const char* kStringHtmlEndUnorderedList           = "</ul>";
static const char* kStringHtmlStartListItem              = "<li>";
static const char* kStringHtmlEndListItem                = "</li>";
static const char* kStringHtmlAHrefStart                 = "<a href=\"";
static const char* kStringHtmlAHrefEndTitleStart         = "\" title=\"";
static const char* kStringHtmlAHrefEndTitleEnd           = "\">";
static const char* kStringHtmlAClose                     = "</a>";
static const char* kStringHtmlStrongOpen                 = "<strong>";
static const char* kStringHtmlStrongClose                = "</strong>";
static const char* kStringHtmlDivIndent40Open            = "<div style=\"text-indent: 40px;\">";
static const char* kStringHtmlDivClose                   = "</div>";
static const char* kStringDialogTitle                    = "Available Updates";
static const char* kStringUnableToCheckForUpdates        = "Unable to check for updates.";
static const char* kStringUpdatesNoUpdateAvailable       = "No updates available.";
static const char* kStringUpdatesNewUpdateAvailable      = "New updates available: ";
static const char* kStringUpdatesDownloadThisReleaseFrom = "Download available in these formats:";
static const char* kStringUpdatesForMoreInformationVisit = "For more information, visit:";
static const char* kStringNewVersion                     = "New version: ";
static const char* kStringReleaseDate                    = "Release date: ";
static const char* kStringTags                           = "Tags: ";
static const char* kStringTagsSeparator                  = ", ";

UpdateCheckResultsDialog::UpdateCheckResultsDialog(QWidget* parent)
    : QDialog(parent)
    , ui_(new Ui::UpdateCheckResultsDialog)
{
    ui_->setupUi(this);

    // Remove the frame of the textbrowser.
    ui_->text_browser_->setFrameStyle(QFrame::NoFrame);

    // Make the background of the textbrowser transparent.
    ui_->text_browser_->setAttribute(Qt::WA_TranslucentBackground);
    ui_->text_browser_->setStyleSheet("background: rgba(255,255,255,0%)");

    setWindowTitle(kStringDialogTitle);
}

UpdateCheckResultsDialog::~UpdateCheckResultsDialog()
{
    delete ui_;
}

void UpdateCheckResultsDialog::SetResults(const UpdateCheck::UpdateInfo& update_info)
{
    QString update_result_html;

    if (!update_info.is_update_available)
    {
        update_result_html.append(kStringUpdatesNoUpdateAvailable);
        update_result_html.append(kStringHtmlNewline);
    }
    else
    {
        update_result_html.append(kStringUpdatesNewUpdateAvailable);
        update_result_html.append(kStringHtmlNewline);
        update_result_html.append(kStringHtmlNewline);

        for (auto release_iter = update_info.releases.cbegin(); release_iter != update_info.releases.cend(); ++release_iter)
        {
            update_result_html.append(kStringHtmlStrongOpen);
            update_result_html.append(release_iter->title.c_str());
            update_result_html.append(kStringHtmlStrongClose);
            update_result_html.append(kStringHtmlNewline);
            update_result_html.append(kStringHtmlNewline);
            update_result_html.append(kStringNewVersion);
            update_result_html.append(release_iter->version.ToString().c_str());
            update_result_html.append(" (");
            update_result_html.append(UpdateCheck::ReleaseTypeToString(release_iter->type).c_str());
            update_result_html.append(")");
            update_result_html.append(kStringHtmlNewline);
            update_result_html.append(kStringReleaseDate);
            update_result_html.append(release_iter->date.c_str());
            update_result_html.append(kStringHtmlNewline);

            if (show_tags_)
            {
                if (!release_iter->tags.empty())
                {
                    update_result_html.append(kStringTags);
                    update_result_html.append(release_iter->tags[0].c_str());

                    for (size_t i = 1; i != release_iter->tags.size(); ++i)
                    {
                        update_result_html.append(kStringTagsSeparator);
                        update_result_html.append(release_iter->tags[i].c_str());
                    }
                }

                update_result_html.append(kStringHtmlNewline);
            }

            update_result_html.append(kStringHtmlNewline);

            // Display download links.
            if (!release_iter->download_links.empty())
            {
                update_result_html.append(kStringUpdatesDownloadThisReleaseFrom);
                update_result_html.append(kStringHtmlNewline);

                // Sample output: "Windows: [MSI] [ZIP] [ZIP]" with the full hyper link as a tooltip over the MSI, ZIP, ZIP to allow
                // users to distinguish between the two ZIP files.
                for (auto platformIter = release_iter->target_platforms.cbegin(); platformIter != release_iter->target_platforms.cend(); ++platformIter)
                {
                    update_result_html.append(kStringHtmlDivIndent40Open);
                    update_result_html.append(UpdateCheck::TargetPlatformToString(*platformIter).c_str());
                    update_result_html.append(":");

                    for (auto iter = release_iter->download_links.cbegin(); iter != release_iter->download_links.cend(); ++iter)
                    {
                        //update_result_html.append(kStringHtmlStartListItem);
                        update_result_html.append(" [");
                        update_result_html.append(kStringHtmlAHrefStart);
                        update_result_html.append(iter->url.c_str());
                        update_result_html.append(kStringHtmlAHrefEndTitleStart);
                        update_result_html.append(iter->url.c_str());
                        update_result_html.append(kStringHtmlAHrefEndTitleEnd);
                        update_result_html.append(UpdateCheck::PackageTypeToString(iter->package_type).c_str());
                        update_result_html.append(kStringHtmlAClose);
                        update_result_html.append("]");
                    }

                    update_result_html.append(kStringHtmlDivClose);
                }

                update_result_html.append(kStringHtmlNewline);
            }

            // Display additional info links.
            if (!release_iter->info_links.empty())
            {
                update_result_html.append(kStringUpdatesForMoreInformationVisit);
                update_result_html.append(kStringhtmlStartUnorderedList);

                for (auto iter = release_iter->info_links.cbegin(); iter != release_iter->info_links.cend(); ++iter)
                {
                    update_result_html.append(kStringHtmlStartListItem);
                    update_result_html.append(kStringHtmlAHrefStart);
                    update_result_html.append(iter->url.c_str());
                    update_result_html.append(kStringHtmlAHrefEndTitleStart);
                    update_result_html.append(iter->url.c_str());
                    update_result_html.append(kStringHtmlAHrefEndTitleEnd);
                    update_result_html.append(iter->page_description.c_str());
                    update_result_html.append(kStringHtmlAClose);
                    update_result_html.append(kStringHtmlEndListItem);
                }

                update_result_html.append(kStringHtmlEndUnorderedList);
            }
        }
    }

    // Update the QTextBrowser with the HTML that was built above.
    ui_->text_browser_->setHtml(update_result_html);
}

void UpdateCheckResultsDialog::SetResults(const UpdateCheck::Results& results)
{
    if (!results.was_check_successful)
    {
        QString update_result_html;
        update_result_html.append(kStringUnableToCheckForUpdates);
        update_result_html.append(kStringHtmlNewline);
        update_result_html.append(results.error_message);

        // Update the QTextBrowser with the HTML that was built above.
        ui_->text_browser_->setHtml(update_result_html);
    }
    else
    {
        SetResults(results.update_info);
    }
}

void UpdateCheckResultsDialog::SetShowTags(bool show_tags)
{
    show_tags_ = show_tags;
}
