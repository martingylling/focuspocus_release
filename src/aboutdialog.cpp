
/****************************************************************************
** File Name:   aboutdialog.cpp
**
** Description:
**     This file contains the implementation of the AboutDialog class, which
**     is a simple dialog that displays information about the application.
**
** Author:      Martin Gylling
** Created On:  2024-12-16
**
** Last Modified By: Martin Gylling
** Last Modified On: 2025-01-20
**
** License: LGPL (Lesser General Public License)
**
****************************************************************************/

#include "aboutdialog.h"
#include "ui_aboutdialog.h"

AboutDialog::AboutDialog(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::AboutDialog)
{
    ui->setupUi(this);
    connect(ui->OkButton, &QPushButton::clicked, this, &QDialog::accept);
}

AboutDialog::~AboutDialog()
{
    delete ui;
}

/// Opens the link in the user's default web browser
/// \param link The URL to open
void AboutDialog::on_label_2_linkActivated(const QString &link)
{
    qDebug() << link;
    QDesktopServices::openUrl(QUrl(link));
}

