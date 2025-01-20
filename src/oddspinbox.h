#ifndef ODDSPINBOX_H
#define ODDSPINBOX_H

#include <QSpinBox>

class OddSpinBox : public QSpinBox {
    Q_OBJECT

public:
    explicit OddSpinBox(QWidget *parent = nullptr);

protected:
    int valueFromText(const QString &text) const override;
    QString textFromValue(int value) const override;
    void stepBy(int steps) override;
};


#endif // ODDSPINBOX_H
