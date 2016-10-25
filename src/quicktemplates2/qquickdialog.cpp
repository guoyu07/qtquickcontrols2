/****************************************************************************
**
** Copyright (C) 2016 The Qt Company Ltd.
** Contact: http://www.qt.io/licensing/
**
** This file is part of the Qt Quick Templates 2 module of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:LGPL3$
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and The Qt Company. For licensing terms
** and conditions see http://www.qt.io/terms-conditions. For further
** information use the contact form at http://www.qt.io/contact-us.
**
** GNU Lesser General Public License Usage
** Alternatively, this file may be used under the terms of the GNU Lesser
** General Public License version 3 as published by the Free Software
** Foundation and appearing in the file LICENSE.LGPLv3 included in the
** packaging of this file. Please review the following information to
** ensure the GNU Lesser General Public License version 3 requirements
** will be met: https://www.gnu.org/licenses/lgpl.html.
**
** GNU General Public License Usage
** Alternatively, this file may be used under the terms of the GNU
** General Public License version 2.0 or later as published by the Free
** Software Foundation and appearing in the file LICENSE.GPL included in
** the packaging of this file. Please review the following information to
** ensure the GNU General Public License version 2.0 requirements will be
** met: http://www.gnu.org/licenses/gpl-2.0.html.
**
** $QT_END_LICENSE$
**
****************************************************************************/

#include "qquickdialog_p.h"
#include "qquickdialog_p_p.h"
#include "qquickdialogbuttonbox_p.h"
#include "qquickpopupitem_p_p.h"

#include <QtQml/qqmlinfo.h>
#include <QtQml/qqmlcontext.h>
#include <QtQml/qqmlcomponent.h>

QT_BEGIN_NAMESPACE

/*!
    \qmltype Dialog
    \inherits Popup
    \instantiates QQuickDialog
    \inqmlmodule QtQuick.Controls
    \ingroup qtquickcontrols2-dialogs
    \ingroup qtquickcontrols2-popups
    \brief A dialog control.
    \since 5.8

    A dialog is a popup mostly used for short-term tasks and brief communications
    with the user. Similarly to \l ApplicationWindow and \l Page, Dialog is organized
    into three sections: \l header, \l {Popup::}{contentItem}, and \l footer.

    \image qtquickcontrols2-page-wireframe.png

    \section1 Dialog Buttons

    Dialog's standard buttons are managed by \l DialogButtonBox. When a button box
    is assigned as a dialog \l footer or \l header, the dialog's \l standardButtons
    property is forwarded to the respective property of the button box. Furthermore,
    the \l {DialogButtonBox::}{accepted()} and \l {DialogButtonBox::}{rejected()}
    signals of the button box are connected to the respective signals in Dialog.

    \snippet qtquickcontrols2-dialog.qml 1

    \note If any standard buttons are specified for the dialog but no button box has
    been assigned as a footer or header, Dialog automatically creates an instance of
    \l buttonBox, and assigns it as a footer or header of the dialog depending on the
    value of the \l {DialogButtonBox::}{position} property. All built-in Dialog styles
    assign the button box as a footer.

    \section1 Modal Dialogs

    A \l {Popup::}{modal} dialog blocks input to other content beneath
    the dialog. When a modal dialog is opened, the user must finish
    interacting with the dialog and close it before they can access any
    other content in the same window.

    \snippet qtquickcontrols2-dialog-modal.qml 1

    \section1 Modeless Dialogs

    A modeless dialog is a dialog that operates independently of other
    content around the dialog. When a modeless dialog is opened, the user
    is allowed to interact with both the dialog and the other content in
    the same window.

    \snippet qtquickcontrols2-dialog-modeless.qml 1

    \sa DialogButtonBox, {Popup Controls}
*/

/*!
    \qmlsignal QtQuick.Controls::Dialog::accepted()

    This signal is emitted when the dialog has been accepted either
    interactively or by calling \l accept().

    \note This signal is \e not emitted when closing the dialog with
    \l {Popup::}{close()} or setting \l {Popup::}{visible} to \c false.

    \sa rejected()
*/

/*!
    \qmlsignal QtQuick.Controls::Dialog::rejected()

    This signal is emitted when the dialog has been rejected either
    interactively or by calling \l reject().

    \note This signal is \e not emitted when closing the dialog with
    \l {Popup::}{close()} or setting \l {Popup::}{visible} to \c false.

    \sa accepted()
*/

void QQuickDialogPrivate::createButtonBox()
{
    Q_Q(QQuickDialog);
    QQmlContext *context = qmlContext(q);
    if (!context || !buttonBoxComponent)
        return;

    QObject *object = buttonBoxComponent->create(context);
    QQuickDialogButtonBox *buttonBox = qobject_cast<QQuickDialogButtonBox *>(object);
    if (!buttonBox) {
        if (object) {
            qmlInfo(q) << "buttonBox must be an instance of DialogButtonBox";
            delete object;
        }
        return;
    }

    if (buttonBox->position() == QQuickDialogButtonBox::Header) {
        if (layout->header()) {
            qmlInfo(q) << "Custom header detected. Cannot assign buttonBox as a header. No standard buttons will appear in the header.";
            delete buttonBox;
        } else {
            q->setHeader(buttonBox);
        }
    } else {
        if (layout->footer()) {
            qmlInfo(q) << "Custom footer detected. Cannot assign buttonBox as a footer. No standard buttons will appear in the footer.";
            delete buttonBox;
        } else {
            q->setFooter(buttonBox);
        }
    }
}

QQuickDialog::QQuickDialog(QObject *parent) :
    QQuickPopup(*(new QQuickDialogPrivate), parent)
{
    Q_D(QQuickDialog);
    d->layout.reset(new QQuickPageLayout(d->popupItem));
}

/*!
    \qmlproperty Item QtQuick.Controls::Dialog::header

    This property holds the dialog header item. The header item is positioned to
    the top, and resized to the width of the dialog. The default value is \c null.

    \note Assigning a \l DialogButtonBox as a dialog header automatically connects
    its \l {DialogButtonBox::}{accepted()} and \l {DialogButtonBox::}{rejected()}
    signals to the respective signals in Dialog.

    \note Assigning a \l DialogButtonBox, \l ToolBar, or \l TabBar as a dialog
    header automatically sets the respective \l DialogButtonBox::position,
    \l ToolBar::position, or \l TabBar::position property to \c Header.

    \sa footer, buttonBox
*/
QQuickItem *QQuickDialog::header() const
{
    Q_D(const QQuickDialog);
    return d->layout->header();
}

void QQuickDialog::setHeader(QQuickItem *header)
{
    Q_D(QQuickDialog);
    QQuickItem *oldHeader = d->layout->header();
    if (!d->layout->setHeader(header))
        return;

    if (QQuickDialogButtonBox *buttonBox = qobject_cast<QQuickDialogButtonBox *>(oldHeader)) {
        disconnect(buttonBox, &QQuickDialogButtonBox::accepted, this, &QQuickDialog::accept);
        disconnect(buttonBox, &QQuickDialogButtonBox::rejected, this, &QQuickDialog::reject);
        if (d->buttonBox == buttonBox)
            d->buttonBox = nullptr;
    }
    if (QQuickDialogButtonBox *buttonBox = qobject_cast<QQuickDialogButtonBox *>(header)) {
        connect(buttonBox, &QQuickDialogButtonBox::accepted, this, &QQuickDialog::accept);
        connect(buttonBox, &QQuickDialogButtonBox::rejected, this, &QQuickDialog::reject);
        d->buttonBox = buttonBox;
        buttonBox->setStandardButtons(d->standardButtons);
    }

    if (isComponentComplete())
        d->layout->update();
    emit headerChanged();
}

/*!
    \qmlproperty Item QtQuick.Controls::Dialog::footer

    This property holds the dialog footer item. The footer item is positioned to
    the bottom, and resized to the width of the dialog. The default value is \c null.

    \note Assigning a \l DialogButtonBox as a dialog footer automatically connects
    its \l {DialogButtonBox::}{accepted()} and \l {DialogButtonBox::}{rejected()}
    signals to the respective signals in Dialog.

    \note Assigning a \l DialogButtonBox, \l ToolBar, or \l TabBar as a dialog
    footer automatically sets the respective \l DialogButtonBox::position,
    \l ToolBar::position, or \l TabBar::position property to \c Footer.

    \sa header, buttonBox
*/
QQuickItem *QQuickDialog::footer() const
{
    Q_D(const QQuickDialog);
    return d->layout->footer();
}

void QQuickDialog::setFooter(QQuickItem *footer)
{
    Q_D(QQuickDialog);
    QQuickItem *oldFooter = d->layout->footer();
    if (!d->layout->setFooter(footer))
        return;

    if (QQuickDialogButtonBox *buttonBox = qobject_cast<QQuickDialogButtonBox *>(oldFooter)) {
        disconnect(buttonBox, &QQuickDialogButtonBox::accepted, this, &QQuickDialog::accept);
        disconnect(buttonBox, &QQuickDialogButtonBox::rejected, this, &QQuickDialog::reject);
        if (d->buttonBox == buttonBox)
            d->buttonBox = nullptr;
    }
    if (QQuickDialogButtonBox *buttonBox = qobject_cast<QQuickDialogButtonBox *>(footer)) {
        connect(buttonBox, &QQuickDialogButtonBox::accepted, this, &QQuickDialog::accept);
        connect(buttonBox, &QQuickDialogButtonBox::rejected, this, &QQuickDialog::reject);
        d->buttonBox = buttonBox;
        buttonBox->setStandardButtons(d->standardButtons);
    }

    if (isComponentComplete())
        d->layout->update();
    emit footerChanged();
}

/*!
    \qmlproperty Component QtQuick.Controls::Dialog::buttonBox

    This property holds a delegate for creating a button box.

    A button box is only created if any standard buttons are set.
    The \l {DialogButtonBox::}{position} property determines whether
    the button box is assigned as a \l header or \l footer.

    \sa standardButtons, header, footer, DialogButtonBox
*/
QQmlComponent *QQuickDialog::buttonBox() const
{
    Q_D(const QQuickDialog);
    return d->buttonBoxComponent;
}

void QQuickDialog::setButtonBox(QQmlComponent *box)
{
    Q_D(QQuickDialog);
    if (d->buttonBoxComponent == box)
        return;

    d->buttonBoxComponent = box;
    emit buttonBoxChanged();
}

/*!
    \qmlproperty enumeration QtQuick.Controls::Dialog::standardButtons

    This property holds a combination of standard buttons that are used by the dialog.

    \snippet qtquickcontrols2-dialog.qml 1

    Possible flags:
    \value Dialog.Ok An "OK" button defined with the \c AcceptRole.
    \value Dialog.Open An "Open" button defined with the \c AcceptRole.
    \value Dialog.Save A "Save" button defined with the \c AcceptRole.
    \value Dialog.Cancel A "Cancel" button defined with the \c RejectRole.
    \value Dialog.Close A "Close" button defined with the \c RejectRole.
    \value Dialog.Discard A "Discard" or "Don't Save" button, depending on the platform, defined with the \c DestructiveRole.
    \value Dialog.Apply An "Apply" button defined with the \c ApplyRole.
    \value Dialog.Reset A "Reset" button defined with the \c ResetRole.
    \value Dialog.RestoreDefaults A "Restore Defaults" button defined with the \c ResetRole.
    \value Dialog.Help A "Help" button defined with the \c HelpRole.
    \value Dialog.SaveAll A "Save All" button defined with the \c AcceptRole.
    \value Dialog.Yes A "Yes" button defined with the \c YesRole.
    \value Dialog.YesToAll A "Yes to All" button defined with the \c YesRole.
    \value Dialog.No A "No" button defined with the \c NoRole.
    \value Dialog.NoToAll A "No to All" button defined with the \c NoRole.
    \value Dialog.Abort An "Abort" button defined with the \c RejectRole.
    \value Dialog.Retry A "Retry" button defined with the \c AcceptRole.
    \value Dialog.Ignore An "Ignore" button defined with the \c AcceptRole.
    \value Dialog.NoButton An invalid button.

    \sa buttonBox, DialogButtonBox
*/
QPlatformDialogHelper::StandardButtons QQuickDialog::standardButtons() const
{
    Q_D(const QQuickDialog);
    return d->standardButtons;
}

void QQuickDialog::setStandardButtons(QPlatformDialogHelper::StandardButtons buttons)
{
    Q_D(QQuickDialog);
    if (d->standardButtons == buttons)
        return;

    d->standardButtons = buttons;
    if (isComponentComplete()) {
        if (d->buttonBox)
            d->buttonBox->setStandardButtons(buttons);
        else if (buttons)
            d->createButtonBox();
    }
    emit standardButtonsChanged();
}

/*!
    \qmlmethod void QtQuick.Controls::Dialog::accept()

    Closes the dialog and emits the \l accepted() signal.

    \sa reject()
*/
void QQuickDialog::accept()
{
    close();
    emit accepted();
}

/*!
    \qmlmethod void QtQuick.Controls::Dialog::reject()

    Closes the dialog and emits the \l rejected() signal.

    \sa accept()
*/
void QQuickDialog::reject()
{
    close();
    emit rejected();
}

void QQuickDialog::componentComplete()
{
    Q_D(QQuickDialog);
    QQuickPopup::componentComplete();
    if (!d->buttonBox && d->standardButtons)
        d->createButtonBox();
}

void QQuickDialog::geometryChanged(const QRectF &newGeometry, const QRectF &oldGeometry)
{
    Q_D(QQuickDialog);
    QQuickPopup::geometryChanged(newGeometry, oldGeometry);
    d->layout->update();
}

void QQuickDialog::paddingChange(const QMarginsF &newPadding, const QMarginsF &oldPadding)
{
    Q_D(QQuickDialog);
    QQuickPopup::paddingChange(newPadding, oldPadding);
    d->layout->update();
}

QT_END_NAMESPACE
