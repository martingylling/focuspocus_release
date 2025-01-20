#ifndef ODDSLIDER_H
#define ODDSLIDER_H

#include <QSlider>

class OddSlider : public QSlider {
    Q_OBJECT

public:
    explicit OddSlider(QWidget *parent = nullptr);

public slots:
    void setValue(int value);

private:
    int adjustToOdd(int value) const;

};

#endif // ODDSLIDER_H
