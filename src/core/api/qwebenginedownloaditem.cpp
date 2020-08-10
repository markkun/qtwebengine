/****************************************************************************
**
** Copyright (C) 2019 The Qt Company Ltd.
** Contact: https://www.qt.io/licensing/
**
** This file is part of the QtWebEngine module of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:LGPL$
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and The Qt Company. For licensing terms
** and conditions see https://www.qt.io/terms-conditions. For further
** information use the contact form at https://www.qt.io/contact-us.
**
** GNU Lesser General Public License Usage
** Alternatively, this file may be used under the terms of the GNU Lesser
** General Public License version 3 as published by the Free Software
** Foundation and appearing in the file LICENSE.LGPL3 included in the
** packaging of this file. Please review the following information to
** ensure the GNU Lesser General Public License version 3 requirements
** will be met: https://www.gnu.org/licenses/lgpl-3.0.html.
**
** GNU General Public License Usage
** Alternatively, this file may be used under the terms of the GNU
** General Public License version 2.0 or (at your option) the GNU General
** Public license version 3 or any later version approved by the KDE Free
** Qt Foundation. The licenses are as published by the Free Software
** Foundation and appearing in the file LICENSE.GPL2 and LICENSE.GPL3
** included in the packaging of this file. Please review the following
** information to ensure the GNU General Public License requirements will
** be met: https://www.gnu.org/licenses/gpl-2.0.html and
** https://www.gnu.org/licenses/gpl-3.0.html.
**
** $QT_END_LICENSE$
**
****************************************************************************/

#include "qwebenginedownloaditem.h"
#include "qwebenginedownloaditem_p.h"

#include "profile_adapter.h"

#include <QDir>
#include "QFileInfo"

QT_BEGIN_NAMESPACE

using QtWebEngineCore::ProfileAdapterClient;

ASSERT_ENUMS_MATCH(ProfileAdapterClient::NoReason, QWebEngineDownloadItem::NoReason)
ASSERT_ENUMS_MATCH(ProfileAdapterClient::FileFailed, QWebEngineDownloadItem::FileFailed)
ASSERT_ENUMS_MATCH(ProfileAdapterClient::FileAccessDenied, QWebEngineDownloadItem::FileAccessDenied)
ASSERT_ENUMS_MATCH(ProfileAdapterClient::FileNoSpace, QWebEngineDownloadItem::FileNoSpace)
ASSERT_ENUMS_MATCH(ProfileAdapterClient::FileNameTooLong, QWebEngineDownloadItem::FileNameTooLong)
ASSERT_ENUMS_MATCH(ProfileAdapterClient::FileTooLarge, QWebEngineDownloadItem::FileTooLarge)
ASSERT_ENUMS_MATCH(ProfileAdapterClient::FileVirusInfected, QWebEngineDownloadItem::FileVirusInfected)
ASSERT_ENUMS_MATCH(ProfileAdapterClient::FileTransientError, QWebEngineDownloadItem::FileTransientError)
ASSERT_ENUMS_MATCH(ProfileAdapterClient::FileBlocked, QWebEngineDownloadItem::FileBlocked)
ASSERT_ENUMS_MATCH(ProfileAdapterClient::FileSecurityCheckFailed, QWebEngineDownloadItem::FileSecurityCheckFailed)
ASSERT_ENUMS_MATCH(ProfileAdapterClient::FileTooShort, QWebEngineDownloadItem::FileTooShort)
ASSERT_ENUMS_MATCH(ProfileAdapterClient::FileHashMismatch, QWebEngineDownloadItem::FileHashMismatch)
ASSERT_ENUMS_MATCH(ProfileAdapterClient::NetworkFailed, QWebEngineDownloadItem::NetworkFailed)
ASSERT_ENUMS_MATCH(ProfileAdapterClient::NetworkTimeout, QWebEngineDownloadItem::NetworkTimeout)
ASSERT_ENUMS_MATCH(ProfileAdapterClient::NetworkDisconnected, QWebEngineDownloadItem::NetworkDisconnected)
ASSERT_ENUMS_MATCH(ProfileAdapterClient::NetworkServerDown, QWebEngineDownloadItem::NetworkServerDown)
ASSERT_ENUMS_MATCH(ProfileAdapterClient::NetworkInvalidRequest, QWebEngineDownloadItem::NetworkInvalidRequest)
ASSERT_ENUMS_MATCH(ProfileAdapterClient::ServerFailed, QWebEngineDownloadItem::ServerFailed)
//ASSERT_ENUMS_MATCH(ProfileAdapterClient::ServerNoRange, QWebEngineDownloadItem::ServerNoRange)
ASSERT_ENUMS_MATCH(ProfileAdapterClient::ServerBadContent, QWebEngineDownloadItem::ServerBadContent)
ASSERT_ENUMS_MATCH(ProfileAdapterClient::ServerUnauthorized, QWebEngineDownloadItem::ServerUnauthorized)
ASSERT_ENUMS_MATCH(ProfileAdapterClient::ServerCertProblem, QWebEngineDownloadItem::ServerCertProblem)
ASSERT_ENUMS_MATCH(ProfileAdapterClient::ServerForbidden, QWebEngineDownloadItem::ServerForbidden)
ASSERT_ENUMS_MATCH(ProfileAdapterClient::ServerUnreachable, QWebEngineDownloadItem::ServerUnreachable)
ASSERT_ENUMS_MATCH(ProfileAdapterClient::UserCanceled, QWebEngineDownloadItem::UserCanceled)
//ASSERT_ENUMS_MATCH(ProfileAdapterClient::UserShutdown, QWebEngineDownloadItem::UserShutdown)
//ASSERT_ENUMS_MATCH(ProfileAdapterClient::Crash, QWebEngineDownloadItem::Crash)

static inline QWebEngineDownloadItem::DownloadState toDownloadState(int state)
{
    switch (state) {
    case ProfileAdapterClient::DownloadInProgress:
        return QWebEngineDownloadItem::DownloadInProgress;
    case ProfileAdapterClient::DownloadCompleted:
        return QWebEngineDownloadItem::DownloadCompleted;
    case ProfileAdapterClient::DownloadCancelled:
        return QWebEngineDownloadItem::DownloadCancelled;
    case ProfileAdapterClient::DownloadInterrupted:
        return QWebEngineDownloadItem::DownloadInterrupted;
    default:
        Q_UNREACHABLE();
        return QWebEngineDownloadItem::DownloadCancelled;
    }
}

static inline QWebEngineDownloadItem::DownloadInterruptReason toDownloadInterruptReason(int reason)
{
    return static_cast<QWebEngineDownloadItem::DownloadInterruptReason>(reason);
}

/*!
    \class QWebEngineDownloadItem
    \brief The QWebEngineDownloadItem class provides information about a download.

    \inmodule QtWebEngineCore

    QWebEngineDownloadItem models a download throughout its life cycle, starting
    with a pending download request and finishing with a completed download. It
    can be used, for example, to get information about new downloads, to monitor
    progress, and to pause, resume, and cancel downloads.

    Downloads are usually triggered by user interaction on a web page. It is the
    QWebEngineProfile's responsibility to notify the application of new download
    requests, which it does by emitting the
    \l{QWebEngineProfile::downloadRequested}{downloadRequested} signal together
    with a newly created QWebEngineDownloadItem. The application can then
    examine this item and decide whether to accept it or not. A signal handler
    must explicitly call accept() on the item for \QWE to actually start
    downloading and writing data to disk. If no signal handler calls accept(),
    then the download request will be automatically rejected and nothing will be
    written to disk.

    \note Some properties, such as setting the path and file name where the file
    will be saved (see \l downloadDirectory() and \l downloadFileName()), can
    only be changed before calling accept().

    \section2 Object Life Cycle

    All items are guaranteed to be valid during the emission of the
    \l{QWebEngineProfile::downloadRequested}{downloadRequested} signal. If
    accept() is \e not called by any signal handler, then the item will be
    deleted \e immediately after signal emission. This means that the
    application \b{must not} keep references to rejected download items. It also
    means the application should not use a queued connection to this signal.

    If accept() \e is called by a signal handler, then the QWebEngineProfile
    will take ownership of the item. However, it is safe for the application to
    delete the item at any time, except during the handling of the
    \l{QWebEngineProfile::downloadRequested}{downloadRequested} signal. The
    QWebEngineProfile being a long-lived object, it is in fact recommended that
    the application delete any items it is no longer interested in.

    \note Deleting an item will also automatically cancel a download since 5.12.2,
    but it is recommended to cancel manually before deleting for portability.

    \section2 Web Page Downloads

    In addition to normal file downloads, which consist simply of retrieving
    some raw bytes from the network and writing them to disk, \QWE also
    supports saving complete web pages, which involves parsing the page's HTML,
    downloading any dependent resources, and potentially packaging everything
    into a special file format (\l savePageFormat). To check if a download is
    for a file or a web page, use \l isSavePageDownload.

    \sa QWebEngineProfile, QWebEngineProfile::downloadRequested,
    QWebEnginePage::download, QWebEnginePage::save
*/

QWebEngineDownloadItemPrivate::QWebEngineDownloadItemPrivate(QtWebEngineCore::ProfileAdapter *adapter, const QUrl &url)
    : m_profileAdapter(adapter)
    , downloadFinished(false)
    , downloadId(-1)
    , downloadState(QWebEngineDownloadItem::DownloadCancelled)
    , savePageFormat(QWebEngineDownloadItem::MimeHtmlSaveFormat)
    , interruptReason(QWebEngineDownloadItem::NoReason)
    , downloadUrl(url)
    , downloadPaused(false)
    , isCustomFileName(false)
    , totalBytes(-1)
    , receivedBytes(0)
    , isSavePageDownload(false)
    , page(nullptr)
{
}

QWebEngineDownloadItemPrivate::~QWebEngineDownloadItemPrivate()
{
}

void QWebEngineDownloadItemPrivate::update(const ProfileAdapterClient::DownloadItemInfo &info)
{
    Q_Q(QWebEngineDownloadItem);

    Q_ASSERT(downloadState != QWebEngineDownloadItem::DownloadRequested);

    if (toDownloadInterruptReason(info.downloadInterruptReason) != interruptReason) {
        interruptReason = toDownloadInterruptReason(info.downloadInterruptReason);
        Q_EMIT q->interruptReasonChanged();
    }
    if (toDownloadState(info.state) != downloadState) {
        downloadState = toDownloadState(info.state);
        Q_EMIT q->stateChanged(downloadState);
    }

    if (info.receivedBytes != receivedBytes || info.totalBytes != totalBytes) {

      if (info.receivedBytes != receivedBytes) {
          receivedBytes = info.receivedBytes;
          Q_EMIT q->receivedBytesChanged();
      }
      if (info.totalBytes != totalBytes) {
          totalBytes = info.totalBytes;
          Q_EMIT q->totalBytesChanged();
      }
      Q_EMIT q->downloadProgress(receivedBytes, totalBytes);
    }

    if (info.done)
        setFinished();

    if (downloadPaused != info.paused) {
        downloadPaused = info.paused;
        Q_EMIT q->isPausedChanged();
    }
}

void QWebEngineDownloadItemPrivate::setFinished()
{
    if (downloadFinished)
        return;

    downloadFinished = true;
    Q_EMIT q_ptr->isFinishedChanged();
}

/*!
    Accepts the current download request, which will start the download.

    If the item is in the \l DownloadRequested state, then it will transition
    into the \l DownloadInProgress state and the downloading will begin. If the
    item is in any other state, then nothing will happen.

    \sa finished(), stateChanged()
*/

void QWebEngineDownloadItem::accept()
{
    Q_D(QWebEngineDownloadItem);

    if (d->downloadState != QWebEngineDownloadItem::DownloadRequested)
        return;

    d->downloadState = QWebEngineDownloadItem::DownloadInProgress;
    Q_EMIT stateChanged(d->downloadState);
}

/*!
    Cancels the current download.

    If the item is in the \l DownloadInProgress state, then it will transition
    into the \l DownloadCancelled state, the downloading will stop, and partially
    downloaded files will be deleted from disk.

    If the item is in the \l DownloadCompleted state, then nothing will happen.
    If the item is in any other state, then it will transition into the \l
    DownloadCancelled state without further effect.

    \sa finished(), stateChanged()
*/

void QWebEngineDownloadItem::cancel()
{
    Q_D(QWebEngineDownloadItem);

    QWebEngineDownloadItem::DownloadState state = d->downloadState;

    if (state == QWebEngineDownloadItem::DownloadCompleted
            || state == QWebEngineDownloadItem::DownloadCancelled)
        return;

    // We directly cancel the download request if the user cancels
    // before it even started, so no need to notify the profile here.
    if (state == QWebEngineDownloadItem::DownloadInProgress) {
        if (d->m_profileAdapter)
            d->m_profileAdapter->cancelDownload(d->downloadId);
    } else {
        d->downloadState = QWebEngineDownloadItem::DownloadCancelled;
        Q_EMIT stateChanged(d->downloadState);
        d->setFinished();
    }
}

/*!
    Pauses the download.

    Has no effect if the state is not \l DownloadInProgress. Does not change the
    state.

    \sa resume(), isPaused()
*/

void QWebEngineDownloadItem::pause()
{
    Q_D(QWebEngineDownloadItem);

    QWebEngineDownloadItem::DownloadState state = d->downloadState;

    if (state != QWebEngineDownloadItem::DownloadInProgress)
        return;

    if (d->m_profileAdapter)
        d->m_profileAdapter->pauseDownload(d->downloadId);
}

/*!
    Resumes the current download if it was paused or interrupted.

    Has no effect if the state is not \l DownloadInProgress or \l
    DownloadInterrupted. Does not change the state.

    \sa pause(), isPaused(), state()
*/
void QWebEngineDownloadItem::resume()
{
    Q_D(QWebEngineDownloadItem);

    QWebEngineDownloadItem::DownloadState state = d->downloadState;

    if (d->downloadFinished || (state != QWebEngineDownloadItem::DownloadInProgress && state != QWebEngineDownloadItem::DownloadInterrupted))
        return;
    if (d->m_profileAdapter)
        d->m_profileAdapter->resumeDownload(d->downloadId);
}

/*!
    Returns the download item's ID.
*/

quint32 QWebEngineDownloadItem::id() const
{
    Q_D(const QWebEngineDownloadItem);
    return d->downloadId;
}

/*!
    \fn void QWebEngineDownloadItem::finished()

    This signal is emitted when the download finishes.

    \sa state(), isFinished()
*/

/*!
    \fn void QWebEngineDownloadItem::isPausedChanged(bool isPaused)

    This signal is emitted whenever \a isPaused changes.

    \sa pause(), isPaused()
*/

/*!
    \fn void QWebEngineDownloadItem::stateChanged(DownloadState state)

    This signal is emitted whenever the download's \a state changes.

    \sa state(), DownloadState
*/

/*!
    \fn void QWebEngineDownloadItem::downloadProgress(qint64 bytesReceived, qint64 bytesTotal)

    This signal is emitted to indicate the progress of the download request.

    The \a bytesReceived parameter indicates the number of bytes received, while
    \a bytesTotal indicates the total number of bytes expected to be downloaded.
    If the size of the file to be downloaded is not known, \c bytesTotal will be
    0.

    \sa totalBytes(), receivedBytes()
*/

/*!
    \enum QWebEngineDownloadItem::DownloadState

    This enum describes the state of the download:

    \value DownloadRequested Download has been requested, but has not been accepted yet.
    \value DownloadInProgress Download is in progress.
    \value DownloadCompleted Download completed successfully.
    \value DownloadCancelled Download has been cancelled.
    \value DownloadInterrupted Download has been interrupted (by the server or because of lost
            connectivity).
*/

/*!
    \enum QWebEngineDownloadItem::SavePageFormat

    This enum describes the format that is used to save a web page.

    \value UnknownSaveFormat This is not a request for downloading a complete web page.
    \value SingleHtmlSaveFormat The page is saved as a single HTML page. Resources such as images
           are not saved.
    \value CompleteHtmlSaveFormat The page is saved as a complete HTML page, for example a directory
            containing the single HTML page and the resources.
    \value MimeHtmlSaveFormat The page is saved as a complete web page in the MIME HTML format.
*/

/*!
    \enum QWebEngineDownloadItem::DownloadInterruptReason

    Describes the reason why a download was interrupted:

    \value NoReason Unknown reason or not interrupted.
    \value FileFailed General file operation failure.
    \value FileAccessDenied The file cannot be written locally, due to access restrictions.
    \value FileNoSpace Insufficient space on the target drive.
    \value FileNameTooLong The directory or file name is too long.
    \value FileTooLarge The file size exceeds the file system limitation.
    \value FileVirusInfected The file is infected with a virus.
    \value FileTransientError Temporary problem (for example the file is in use,
           out of memory, or too many files are opened at once).
    \value FileBlocked The file was blocked due to local policy.
    \value FileSecurityCheckFailed An attempt to check the safety of the download
           failed due to unexpected reasons.
    \value FileTooShort An attempt was made to seek past the end of a file when
           opening a file (as part of resuming a previously interrupted download).
    \value FileHashMismatch The partial file did not match the expected hash.

    \value NetworkFailed General network failure.
    \value NetworkTimeout The network operation has timed out.
    \value NetworkDisconnected The network connection has been terminated.
    \value NetworkServerDown The server has gone down.
    \value NetworkInvalidRequest The network request was invalid (for example, the
           original or redirected URL is invalid, has an unsupported scheme, or is disallowed by policy).

    \value ServerFailed General server failure.
    \value ServerBadContent The server does not have the requested data.
    \value ServerUnauthorized The server did not authorize access to the resource.
    \value ServerCertProblem A problem with the server certificate occurred.
    \value ServerForbidden Access forbidden by the server.
    \value ServerUnreachable Unexpected server response (might indicate that
           the responding server may not be the intended server).
    \value UserCanceled The user canceled the download.
*/

/*!
    Returns the download item's current state.

    \sa DownloadState
*/

QWebEngineDownloadItem::DownloadState QWebEngineDownloadItem::state() const
{
    Q_D(const QWebEngineDownloadItem);
    return d->downloadState;
}

/*!
    Returns the the total amount of data to download in bytes.

    \c -1 means the size is unknown.
*/

qint64 QWebEngineDownloadItem::totalBytes() const
{
    Q_D(const QWebEngineDownloadItem);
    return d->totalBytes;
}

/*!
    Returns the amount of data in bytes that has been downloaded so far.

    \c -1 means the size is unknown.
*/

qint64 QWebEngineDownloadItem::receivedBytes() const
{
    Q_D(const QWebEngineDownloadItem);
    return d->receivedBytes;
}

/*!
    Returns the download's origin URL.
*/

QUrl QWebEngineDownloadItem::url() const
{
    Q_D(const QWebEngineDownloadItem);
    return d->downloadUrl;
}

/*!
    Returns the MIME type of the download.
*/

QString QWebEngineDownloadItem::mimeType() const
{
    Q_D(const QWebEngineDownloadItem);
    return d->mimeType;
}

/*!
    Returns the download directory path.
*/

QString QWebEngineDownloadItem::downloadDirectory() const
{
    Q_D(const QWebEngineDownloadItem);
    return d->downloadDirectory;
}

/*!
    Sets \a directory as the directory path to download the file to.

    The download directory path can only be set in response to the QWebEngineProfile::downloadRequested()
    signal before the download is accepted. Past that point, this function has no effect on the
    download item's state.
*/

void QWebEngineDownloadItem::setDownloadDirectory(const QString &directory)
{
    Q_D(QWebEngineDownloadItem);
    if (d->downloadState != QWebEngineDownloadItem::DownloadRequested) {
        qWarning("Setting the download directory is not allowed after the download has been accepted.");
        return;
    }

    if (!directory.isEmpty() && d->downloadDirectory != directory)
        d->downloadDirectory = directory;

    if (!d->isCustomFileName && d->m_profileAdapter)
        d->downloadFileName = QFileInfo(d->m_profileAdapter->determineDownloadPath(d->downloadDirectory,
                                                                                            d->suggestedFileName,
                                                                                            d->startTime)).fileName();
}

/*!
    Returns the file name to download the file to.
*/

QString QWebEngineDownloadItem::downloadFileName() const
{
    Q_D(const QWebEngineDownloadItem);
    return d->downloadFileName;
}

/*!
    Sets \a fileName as the file name to download the file to.

    The download file name can only be set in response to the QWebEngineProfile::downloadRequested()
    signal before the download is accepted. Past that point, this function has no effect on the
    download item's state.
*/

void QWebEngineDownloadItem::setDownloadFileName(const QString &fileName)
{
    Q_D(QWebEngineDownloadItem);
    if (d->downloadState != QWebEngineDownloadItem::DownloadRequested) {
        qWarning("Setting the download file name is not allowed after the download has been accepted.");
        return;
    }

    if (!fileName.isEmpty()) {
        d->downloadFileName = fileName;
        d->isCustomFileName = true;
    }
}

/*!
    Returns the suggested file name.
*/

QString QWebEngineDownloadItem::suggestedFileName() const
{
    Q_D(const QWebEngineDownloadItem);
    return d->suggestedFileName;
}

/*!
    Returns whether this download is finished (completed, cancelled, or non-resumable interrupted state).

    \sa finished(), state(),
*/

bool QWebEngineDownloadItem::isFinished() const
{
    Q_D(const QWebEngineDownloadItem);
    return d->downloadFinished;
}

/*!
    Returns whether this download is paused.

    \sa pause(), resume()
*/

bool QWebEngineDownloadItem::isPaused() const
{
    Q_D(const QWebEngineDownloadItem);
    return d->downloadPaused;
}

/*!
    Returns the format the web page will be saved in if this is a download request for a web page.
    \sa setSavePageFormat(), isSavePageDownload()
*/
QWebEngineDownloadItem::SavePageFormat QWebEngineDownloadItem::savePageFormat() const
{
    Q_D(const QWebEngineDownloadItem);
    return d->savePageFormat;
}

/*!
    Sets the \a format the web page will be saved in if this is a download request for a web page.

    \sa savePageFormat(), isSavePageDownload()
*/
void QWebEngineDownloadItem::setSavePageFormat(QWebEngineDownloadItem::SavePageFormat format)
{
  Q_D(QWebEngineDownloadItem);
  if (d->savePageFormat != format) {
      d->savePageFormat = format;
      Q_EMIT savePageFormatChanged();
  }
}

/*!
    Returns \c true if this is a download request for saving a web page.

    \sa savePageFormat(), setSavePageFormat()
 */
bool QWebEngineDownloadItem::isSavePageDownload() const
{
    Q_D(const QWebEngineDownloadItem);
    return d->isSavePageDownload;
}

/*!
    Returns the reason why the download was interrupted.

    \sa interruptReasonString()
*/

QWebEngineDownloadItem::DownloadInterruptReason QWebEngineDownloadItem::interruptReason() const
{
    Q_D(const QWebEngineDownloadItem);
    return d->interruptReason;
}

/*!
    Returns a human-readable description of the reason for interrupting the download.

    \sa interruptReason()
*/

QString QWebEngineDownloadItem::interruptReasonString() const
{
    return ProfileAdapterClient::downloadInterruptReasonToString(
              static_cast<ProfileAdapterClient::DownloadInterruptReason>(interruptReason()));
}

/*!
    Returns the page the download was requested on. If the download was not triggered by content in a page,
    \c nullptr is returned.
*/
QObject *QWebEngineDownloadItem::page() const
{
    Q_D(const QWebEngineDownloadItem);
    return d->page;
}

QWebEngineDownloadItem::QWebEngineDownloadItem(QWebEngineDownloadItemPrivate *p, QObject *parent)
    : QObject(parent)
    , d_ptr(p)
{
    p->q_ptr = this;
}

/*! \internal
*/
QWebEngineDownloadItem::~QWebEngineDownloadItem()
{
    // MEMO Items are owned by profile by default and will be destroyed on profile's destruction
    //      It's not safe to access profile in that case, so we rely on profile to clean up items
    if (!isFinished())
        cancel();
}

QT_END_NAMESPACE
