#ifndef SETTINGS_H
#define SETTINGS_H

#include <QDialog>
#include <QDebug>

namespace Ui {
class settings;
}

class settings : public QDialog
{
    Q_OBJECT

public:
    explicit settings(QWidget *parent = 0);
    ~settings();
    void InterfaceSettings();
    void CreateConnections();

private slots:
    void ok_clicked();
    void cancel_clicked();

signals:
    void sendThreshold(quint16 threshold, quint16 xUpperBound, quint16 yUpperBound);

private:
    Ui::settings *ui;
};

#endif // SETTINGS_H
