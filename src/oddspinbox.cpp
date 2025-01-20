/****************************************************************************
** File Name:   oddspinbox.cpp
**
** Description:
**     This file contains the implementation of the OddSpinBox class, which
**     is a custom spin box that only allows odd values.
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
#include "oddspinbox.h"

OddSpinBox::OddSpinBox(QWidget *parent) : QSpinBox(parent) {
    setSingleStep(2); // Increment by 2 to keep odd values.
}

/// Sets the value of the spin box
/// \param value The new value
/// \return The adjusted value
int OddSpinBox::valueFromText(const QString &text) const {
    int value = QSpinBox::valueFromText(text);
    return (value % 2 == 0) ? value + 1 : value; // Ensure odd.
}

/// Returns the text representation of the value
/// \param value The value to convert
/// \return The text representation of the value
QString OddSpinBox::textFromValue(int value) const {
    return QString::number(value);
}

/// Adjusts the value to be odd
/// \param steps The number of steps to adjust by
void OddSpinBox::stepBy(int steps) {
    int newValue = value() + steps * singleStep();
    setValue((newValue % 2 == 0) ? newValue + 1 : newValue); // Ensure odd.
}
