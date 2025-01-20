#ifndef ABOUTDIALOG_H
#define ABOUTDIALOG_H

#include <QDialog>
#include <QDesktopServices>

namespace Ui {
class AboutDialog;
}

class AboutDialog : public QDialog
{
    Q_OBJECT

public:
    explicit AboutDialog(QWidget *parent = nullptr);
    ~AboutDialog();

private slots:
    void on_label_2_linkActivated(const QString &link);

private:
    Ui::AboutDialog *ui;
};

#endif // ABOUTDIALOG_H
