
/****************************************************************************
** File Name:   exportdialog.cpp
**
** Description:
**      This file contains the implementation of the ExportDialog class, which
**      is a simple dialog that allows the user to select the quality of an
**      exported image and the file format.
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

#include "exportdialog.h"
#include "ui_exportdialog.h"

ExportDialog::ExportDialog(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::ExportDialog)
{
    ui->setupUi(this);
    connect(ui->QualitySlider, &QSlider::valueChanged, this, &ExportDialog::valueChanged);
    connect(ui->OkButton, &QPushButton::clicked, this, &QDialog::accept);
    connect(ui->CancelButton, &QPushButton::clicked, this, &QDialog::reject);
}

ExportDialog::~ExportDialog()
{
    delete ui;
}

/// Updates the quality label when the slider value changes
/// \param value The new slider value
void ExportDialog::valueChanged(int value)
{
    ui->QualityLabel->setText(QString::number(value));
}

/// Returns the selected image quality
/// \return The selected image quality
int ExportDialog::getImageQuality(){
    return ui->QualitySlider->value();
}
