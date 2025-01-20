/****************************************************************************
** File Name:   oddslider.cpp
**
** Description:
**     This file contains the implementation of the OddSlider class, which
**     is a custom slider that only allows odd values.
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
#include "oddslider.h"

OddSlider::OddSlider(QWidget *parent)
    : QSlider(parent) {}

/// Sets the value of the slider
/// \param value The new value
void OddSlider::setValue(int value) {
    int adjusted = adjustToOdd(value); //Ensure odd value.
    QSlider::setValue(adjusted); // Ensure odd value.
}

/// Adjusts the value to be odd
/// \param value The value to adjust
/// \return The adjusted value
int OddSlider::adjustToOdd(int value) const {
    return (value % 2 == 0) ? value + 1 : value;
}
