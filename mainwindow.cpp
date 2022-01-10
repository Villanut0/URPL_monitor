#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent),
      ui(new Ui::MainWindow),
      _status(new QLabel),
      _serial(new QSerialPort(this)),
      _settings(new SettingsDialog)
{
    ui->setupUi(this);

    QTimer::singleShot(0, this, SLOT(showMaximized()));

    ui->statusbar->addWidget(_status);

    ui->plotLoad->addSerie(LOAD_CELL_NAME);
    ui->plotLoad->setXText(X_AXIS_NAME);
    ui->plotLoad->setYText(LOAD_Y_AXIS_NAME);
    ui->plotLoad->setYAxisRange(LOAD_CELL_MIN, LOAD_CELL_MAX);

    ui->plotPressure->addSerie(P0_NAME);
    ui->plotPressure->addSerie(P1_NAME);
    ui->plotPressure->setXText(X_AXIS_NAME);
    ui->plotPressure->setYText(PRESSURE_Y_AXIS_NAME);
    ui->plotPressure->setYAxisRange(PRESSURE_MIN, PRESSURE_MAX);

    ui->plotTemp0->addSerie(T0_NAME);
    ui->plotTemp0->addSerie(T1_NAME);
    ui->plotTemp0->setXText(X_AXIS_NAME);
    ui->plotTemp0->setYText(TEMP_Y_AXIS_NAME);
    ui->plotTemp0->setYAxisRange(T_J_MIN, T_J_MAX);

    ui->plotTemp1->addSerie(T2_NAME);
    ui->plotTemp1->addSerie(T3_NAME);
    ui->plotTemp1->setXText(X_AXIS_NAME);
    ui->plotTemp1->setYText(TEMP_Y_AXIS_NAME);
    ui->plotTemp1->setYAxisRange(T_N_MIN, T_N_MAX);

    connect(_serial, &QSerialPort::readyRead, this, &MainWindow::readData);

    initActionsConnections();

    QTimer *timer = new QTimer(this);
    connect(timer, SIGNAL(timeout()), this, SLOT(launch()));
    timer->start(1000);
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::openSerialPort()
{
    const SettingsDialog::Settings p = _settings->settings();
    _serial->setPortName(p.name);
    _serial->setBaudRate(p.baudRate);
    _serial->setDataBits(p.dataBits);
    _serial->setParity(p.parity);
    _serial->setStopBits(p.stopBits);
    _serial->setFlowControl(p.flowControl);
    if (_serial->open(QIODevice::ReadWrite)) {
        ui->actionConnect->setEnabled(false);
        ui->actionDisconnect->setEnabled(true);
        ui->actionSettings->setEnabled(false);
        ui->activateCommandsCheck->setEnabled(true);
        _status->setText(tr("Connected to %1 : %2, %3, %4, %5, %6")
                          .arg(p.name).arg(p.stringBaudRate).arg(p.stringDataBits)
                          .arg(p.stringParity).arg(p.stringStopBits).arg(p.stringFlowControl));
    } else {
        QMessageBox::critical(this, tr("Error"), _serial->errorString());

        _status->setText(tr("Open error"));
    }

    if (p.save)
    {
        _file.setFileName(p.saveFilePath);

        if(_file.open(QIODevice::WriteOnly | QIODevice::Text))
        {
            QTextStream txtStream(&_file);
            txtStream << "Time (ms)" << ","
                      << LOAD_CELL_NAME << ","
                      << T0_NAME << ","
                      << T1_NAME << ","
                      << T2_NAME << ","
                      << T3_NAME << ","
                      << P0_NAME << ","
                      << P1_NAME << ","
                      << '\n';

            _file.close();
        }
    }
}


void MainWindow::closeSerialPort()
{
    if (_serial->isOpen())
    {
        on_abortButton_released();
        _serial->close();
    }
    ui->actionConnect->setEnabled(true);
    ui->actionDisconnect->setEnabled(false);
    ui->actionSettings->setEnabled(true);
    ui->activateCommandsCheck->setEnabled(false);
    ui->commandsBox->setEnabled(false);
    ui->controlBox->setEnabled(false);
    ui->activateCommandsCheck->setCheckState(Qt::Unchecked);
    _status->setText(tr("Disconnected"));
}

void MainWindow::readData()
{
    _serial->flush();
    while (_serial->canReadLine())
    {
        QByteArray raw_data = _serial->readLine().trimmed();
        if (raw_data == "ACK_LAUNCH")
        {
            ui->controlConsole->setTextColor(Qt::darkGreen);
            ui->controlConsole->append("LAUNCH RECEIVED");
        }
        else if (raw_data == "ACK_ABORT")
        {
            ui->controlConsole->setTextColor(Qt::darkRed);
            ui->controlConsole->append("ABORT RECEIVED");
        }
        else
        {
            QList<QByteArray> raw_data_list = raw_data.split(',');
            if (raw_data_list.count() == 8)
            {
                double timeSec = raw_data_list[0].toDouble()/1000;
                ui->plotLoad->appendData(0, QPointF(timeSec, raw_data_list[1].toDouble()), X_AXIS_BUFFER_SIZE);

                ui->plotTemp0->appendData(0, QPointF(timeSec, raw_data_list[2].toDouble()), X_AXIS_BUFFER_SIZE);
                ui->plotTemp0->appendData(1, QPointF(timeSec, raw_data_list[3].toDouble()), X_AXIS_BUFFER_SIZE);

                ui->plotTemp1->appendData(0, QPointF(timeSec, raw_data_list[4].toDouble()), X_AXIS_BUFFER_SIZE);
                ui->plotTemp1->appendData(1, QPointF(timeSec, raw_data_list[5].toDouble()), X_AXIS_BUFFER_SIZE);

                ui->plotPressure->appendData(0, QPointF(timeSec, raw_data_list[6].toDouble()), X_AXIS_BUFFER_SIZE);
                ui->plotPressure->appendData(1, QPointF(timeSec, raw_data_list[7].toDouble()), X_AXIS_BUFFER_SIZE);

                writeDataCSV(raw_data_list);
            }
        }
    }
}

void MainWindow::writeDataCSV(QList<QByteArray> data)
{
    const SettingsDialog::Settings p = _settings->settings();

    if (p.save)
    {
        if(_file.open(QIODevice::Append | QIODevice::Text))
        {
            QTextStream txtStream(&_file);

            for (const auto &array : data)
            {
                txtStream << array.toDouble() << ",";
            }
            txtStream << '\n';
            _file.close();
        }
    }
}

void MainWindow::launch()
{
    if (_launchOk)
    {
        if (_launchTimer > 0)
        {
            ui->controlConsole->setTextColor(Qt::green);
            ui->timerLcd->display(_launchTimer);
            _launchTimer--;
        }
        else if (_launchTimer == 0)
        {
            ui->controlConsole->setTextColor(Qt::green);
            ui->controlConsole->append("LAUNCH COMMAND SENT");

            ui->timerLcd->display(0);

            _launchOk = false;

            ui->activateCommandsCheck->setEnabled(true);
            ui->timerLabel->setEnabled(true);
            ui->secondsLabel->setEnabled(true);
            ui->timerSpinBox->setEnabled(true);
            ui->launchButton->setEnabled(true);
            ui->abortButton->setEnabled(false);
            ui->timerLcd->setEnabled(false);

            _serial->write("LAUNCH\n");
        }
    }
}

void MainWindow::initActionsConnections()
{
    connect(ui->actionConnect, &QAction::triggered, this, &MainWindow::openSerialPort);
    connect(ui->actionDisconnect, &QAction::triggered, this, &MainWindow::closeSerialPort);
    connect(ui->actionSettings, &QAction::triggered, _settings, &SettingsDialog::show);
}


void MainWindow::on_activateCommandsCheck_stateChanged(int arg1)
{
    if (arg1 == Qt::Unchecked)
    {
        ui->controlBox->setEnabled(false);
        ui->commandsBox->setEnabled(false);
    }
    else if (arg1 == Qt::Checked)
    {
        QMessageBox::StandardButton resultWarning = QMessageBox::warning(this, "URPL Monitor",
                                                                        tr("Are you sure?\n"),
                                                                        QMessageBox::No | QMessageBox::Yes,
                                                                        QMessageBox::No);
        if (resultWarning == QMessageBox::No)
        {
            ui->activateCommandsCheck->setCheckState(Qt::Unchecked);
            return;
        }

        ui->controlBox->setEnabled(true);
        ui->commandsBox->setEnabled(true);
        ui->abortButton->setEnabled(false);
        ui->timerLcd->setEnabled(false);
        ui->controlConsole->clear();
    }
}

void MainWindow::on_launchButton_released()
{
    QMessageBox::StandardButton resultWarning = QMessageBox::warning(this, "URPL Monitor",
                                                                    tr("Are you sure?\n"),
                                                                    QMessageBox::No | QMessageBox::Yes,
                                                                    QMessageBox::No);
    if (resultWarning == QMessageBox::No)
    {
        return;
    }

    ui->activateCommandsCheck->setEnabled(false);
    ui->timerLabel->setEnabled(false);
    ui->secondsLabel->setEnabled(false);
    ui->timerSpinBox->setEnabled(false);
    ui->launchButton->setEnabled(false);
    ui->abortButton->setEnabled(true);
    ui->timerLcd->setEnabled(true);

    ui->controlConsole->clear();

    ui->controlConsole->setTextColor(Qt::green);
    ui->controlConsole->append("LAUNCH INITIATED");

    _launchOk = true;
    _launchTimer = ui->timerSpinBox->value();
}


void MainWindow::on_abortButton_released()
{
    if (_launchOk == true)
    {
        ui->activateCommandsCheck->setEnabled(true);
        ui->timerLabel->setEnabled(true);
        ui->secondsLabel->setEnabled(true);
        ui->timerSpinBox->setEnabled(true);
        ui->launchButton->setEnabled(true);
        ui->abortButton->setEnabled(false);
        ui->timerLcd->setEnabled(false);

        ui->controlConsole->setTextColor(Qt::red);
        ui->controlConsole->append("LAUNCH ABORTED");

        _launchOk = false;
        _launchTimer = 0;

        _serial->write("ABORT\n");
    }
}
