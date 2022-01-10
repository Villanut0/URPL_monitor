#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QSerialPort>
#include <QMessageBox>
#include <QLabel>
#include <QTimer>
#include "Plot.h"
#include "settingsdialog.h"
#include "definitions.h"

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void openSerialPort();
    void closeSerialPort();
    void readData();
    void writeDataCSV(QList<QByteArray> data);

    void launch();

    void on_activateCommandsCheck_stateChanged(int arg1);

    void on_launchButton_released();

    void on_abortButton_released();

private:
    Ui::MainWindow *ui;
    QLabel *_status;
    QSerialPort *_serial = nullptr;
    SettingsDialog *_settings = nullptr;
    QFile _file;
    bool _launchOk = false;
    int _launchTimer = 0;

    void initActionsConnections();
};
#endif // MAINWINDOW_H
