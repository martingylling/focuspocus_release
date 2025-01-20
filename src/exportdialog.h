#ifndef EXPORTDIALOG_H
#define EXPORTDIALOG_H

#include <QDialog>

namespace Ui {
class ExportDialog;
}

class ExportDialog : public QDialog
{
    Q_OBJECT

public:
    explicit ExportDialog(QWidget *parent = nullptr);
    ~ExportDialog();
    int getImageQuality();


private slots:
    void valueChanged(int value);

private:
    Ui::ExportDialog *ui;
};

#endif // EXPORTDIALOG_H
