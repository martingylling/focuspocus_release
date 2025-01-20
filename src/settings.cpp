/****************************************************************************
** File Name:   settings.cpp
**
** Description:
**     This file contains the implementation of the Settings class, which
**     provides functionality for saving and loading application parameters.
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

#include "Settings.h"
#include <QFile>
#include <QDataStream>
#include <QDebug>

/// Saves parameters to a file
/// \param filePath The file path
/// \param parameters The parameters to save
void Settings::save(const QString &filePath, const QMap<QString, QVariant> &parameters) {
    QFile file(filePath);
    if (!file.open(QIODevice::WriteOnly)) {
        qDebug() << "Error: Could not open file for writing -" << filePath;
        return;
    }

    QDataStream out(&file);

    // Write header
    out.writeRawData("PARAMS", 8);

    // Write parameter count
    out << quint16(parameters.size());

    // Write parameters
    for (auto it = parameters.cbegin(); it != parameters.cend(); ++it) {
        QByteArray name = it.key().toUtf8();
        out << quint8(name.size());
        out.writeRawData(name.constData(), name.size());

        const QVariant &value = it.value();
        if (value.type() == QVariant::Int) {
            out << quint8(0x01); // Type: Int
            out << value.toInt();
        } else if (value.type() == QVariant::Bool) {
            out << quint8(0x02); // Type: Bool
            out << value.toBool();
        } else if (value.type() == QVariant::Double) {
            out << quint8(0x03); // Type: Double
            out << value.toDouble();
        } else {
            // Handle other types as needed
            qDebug() << "Warning: Unsupported value type for parameter -" << it.key();
        }
    }

    file.close();
}

/// Loads parameters from a file
/// \param filePath The file path
/// \param ok True if the load was successful
/// \return The loaded parameters
QMap<QString, QVariant> Settings::load(const QString &filePath, bool &ok) {
    QMap<QString, QVariant> parameters;
    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly)) {
        qDebug() << "Error: Could not open file for reading -" << filePath;
        ok = false;
        return parameters;
    }

    QDataStream in(&file);

    // Verify header
    char header[8];
    in.readRawData(header, 8);
    QString headerStr = QString::fromUtf8(header, 6).trimmed();
    if (headerStr != "PARAMS") {
        qDebug() << "Error: Invalid file format -" << filePath;
        ok = false;
        return parameters;
    }

    // Read parameter count
    quint16 paramCount;
    in >> paramCount;

    // Read parameters
    for (quint16 i = 0; i < paramCount; ++i) {
        quint8 nameLen;
        in >> nameLen;

        QByteArray nameData(nameLen, Qt::Uninitialized);
        in.readRawData(nameData.data(), nameLen);
        QString name = QString::fromUtf8(nameData);

        quint8 type;
        in >> type;

        QVariant value;
        if (type == 0x01) {
            int intValue;
            in >> intValue;
            value = intValue;
        } else if (type == 0x02) {
            bool boolValue;
            in >> boolValue;
            value = boolValue;
        } else if (type == 0x03) {
            double doubleValue;
            in >> doubleValue;
            value = doubleValue;
        } else {
            // Handle unknown types
            qDebug() << "Warning: Unsupported value type in file for parameter -" << name;
        }

        parameters[name] = value;
    }

    file.close();
    return parameters;
}
