#ifndef SETTINGS_H
#define SETTINGS_H

#include <QString>
#include <QMap>
#include <QVariant>

class Settings {
public:
    // Static method to save parameters to a file
    static void save(const QString &filePath, const QMap<QString, QVariant> &parameters);

    // Static method to load parameters from a file
    static QMap<QString, QVariant> load(const QString &filePath, bool &ok);
};

#endif // SETTINGS_H
