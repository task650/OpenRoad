/*
    Copyright 2016 - 2017 Benjamin Vedder	benjamin@vedder.se

    This file is part of VESC Tool.

    VESC Tool is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    VESC Tool is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
    */

#include "pageconnection.h"
#include "ui_pageconnection.h"
#include "widgets/helpdialog.h"
#include "utility.h"
#include <QMessageBox>
#include <QListWidgetItem>
#include <QInputDialog>

PageConnection::PageConnection(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::PageConnection)
{
    ui->setupUi(this);
    layout()->setContentsMargins(0, 0, 0, 0);

    mOpenroad = nullptr;
    mTimer = new QTimer(this);

    connect(mTimer, SIGNAL(timeout()),
            this, SLOT(timerSlot()));

    mTimer->start(20);
}

PageConnection::~PageConnection()
{
    delete ui;
}

OpenroadInterface *PageConnection::openroad() const
{
    return mOpenroad;
}

void PageConnection::setOpenroad(OpenroadInterface *openroad)
{
    mOpenroad = openroad;

    ui->tcpServerEdit->setText(mOpenroad->getLastTcpServer());
    ui->tcpPortBox->setValue(mOpenroad->getLastTcpPort());

#ifdef HAS_BLUETOOTH
    connect(mOpenroad->bleDevice(), SIGNAL(scanDone(QVariantMap,bool)),
            this, SLOT(bleScanDone(QVariantMap,bool)));

    QString lastBleAddr = mOpenroad->getLastBleAddr();
    if (lastBleAddr != "") {
        QString setName = mOpenroad->getBleName(lastBleAddr);

        QString name;
        if (!setName.isEmpty()) {
            name += setName;
            name += " [";
            name += lastBleAddr;
            name += "]";
        } else {
            name = lastBleAddr;
        }
        ui->bleDevBox->insertItem(0, name, lastBleAddr);
    }
#endif

#ifdef HAS_SERIALPORT
    ui->serialBaudBox->setValue(mOpenroad->getLastSerialBaud());
#endif

#ifdef HAS_CANBUS
    ui->CANbusBitrateBox->setValue(mOpenroad->getLastCANbusBitrate());

    ui->CANbusInterfaceBox->clear();
    QList<QString> interfaces = mOpenroad->listCANbusInterfaces();

    for(int i = 0;i < interfaces.size();i++) {
        ui->CANbusInterfaceBox->addItem(interfaces.at(i), interfaces.at(i));
    }

    ui->CANbusInterfaceBox->setCurrentIndex(0);
#endif

    connect(mOpenroad->commands(), SIGNAL(pingCanRx(QVector<int>,bool)),
            this, SLOT(pingCanRx(QVector<int>,bool)));
    connect(mOpenroad, SIGNAL(CANbusNewNode(int)),
            this, SLOT(CANbusNewNode(int)));
    connect(mOpenroad, SIGNAL(CANbusInterfaceListUpdated()),
            this, SLOT(CANbusInterfaceListUpdated()));
    connect(mOpenroad, SIGNAL(pairingListUpdated()),
            this, SLOT(pairingListUpdated()));

    pairingListUpdated();
    on_serialRefreshButton_clicked();
}

void PageConnection::timerSlot()
{
    if (mOpenroad) {
        QString str = mOpenroad->getConnectedPortName();
        if (str != ui->statusLabel->text()) {
            ui->statusLabel->setText(mOpenroad->getConnectedPortName());
        }

        // CAN fwd
        if (ui->canFwdButton->isChecked() != mOpenroad->commands()->getSendCan()) {
            ui->canFwdButton->setChecked(mOpenroad->commands()->getSendCan());
        }

        if (ui->canFwdBox->count() > 0) {
            int canVal = ui->canFwdBox->currentData().toInt();
            if (canVal != mOpenroad->commands()->getCanSendId()) {
                for (int i = 0;i < ui->canFwdBox->count();i++) {
                    if (ui->canFwdBox->itemData(i).toInt() == canVal) {
                        ui->canFwdBox->setCurrentIndex(i);
                        break;
                    }
                }
            }
        }
    }

    QString ipTxt = "Server IPs\n";
    QString clientTxt = "Connected Clients\n";
    if (mOpenroad->tcpServerIsRunning()) {
        for (auto adr: Utility::getNetworkAddresses()) {
            ipTxt += adr.toString() + "\n";
        }

        if (mOpenroad->tcpServerIsClientConnected()) {
            clientTxt += mOpenroad->tcpServerClientIp();
        }
    } else {
        ui->tcpServerPortBox->setEnabled(true);
    }

    if (ui->tcpServerAddressesEdit->toPlainText() != ipTxt) {
        ui->tcpServerAddressesEdit->setPlainText(ipTxt);
    }

    if (ui->tcpServerClientsEdit->toPlainText() != clientTxt) {
        ui->tcpServerClientsEdit->setPlainText(clientTxt);
    }
}

void PageConnection::bleScanDone(QVariantMap devs, bool done)
{
#ifdef HAS_BLUETOOTH
    if (done) {
        ui->bleScanButton->setEnabled(true);
    }

    ui->bleDevBox->clear();
    for (auto d: devs.keys()) {
        QString devName = devs.value(d).toString();
        QString addr = d;
        QString setName = mOpenroad->getBleName(addr);

        if (!setName.isEmpty()) {
            QString name;
            name += setName;
            name += " [";
            name += addr;
            name += "]";
            ui->bleDevBox->insertItem(0, name, addr);
        } else if (devName.contains("VESC")) {
            QString name;
            name += devName;
            name += " [";
            name += addr;
            name += "]";
            ui->bleDevBox->insertItem(0, name, addr);
        } else {
            QString name;
            name += devName;
            name += " [";
            name += addr;
            name += "]";
            ui->bleDevBox->addItem(name, addr);
        }
    }
    ui->bleDevBox->setCurrentIndex(0);
#else
    (void)devs;
    (void)done;
#endif
}

void PageConnection::pingCanRx(QVector<int> devs, bool isTimeout)
{
    (void)isTimeout;
    ui->canRefreshButton->setEnabled(true);

    ui->canFwdBox->clear();
    for (int dev: devs) {
        ui->canFwdBox->addItem(QString("VESC %1").arg(dev), dev);
    }
}

void PageConnection::CANbusNewNode(int node)
{
    ui->CANbusTargetIdBox->addItem(QString::number(node), QString::number(node));
}

void PageConnection::CANbusInterfaceListUpdated()
{
    ui->CANbusInterfaceBox->clear();
    QList<QString> interfaces = mOpenroad->listCANbusInterfaces();

    for(int i = 0; i<interfaces.size(); i++) {
        ui->CANbusInterfaceBox->addItem(interfaces.at(i), interfaces.at(i));
    }

    ui->CANbusInterfaceBox->setCurrentIndex(0);
}

void PageConnection::pairingListUpdated()
{
    ui->pairedListWidget->clear();

    for (QString uuid: mOpenroad->getPairedUuids()) {
        QListWidgetItem *item = new QListWidgetItem;
        item->setText("UUID: " + uuid);
        item->setIcon(QIcon("://res/icon.png"));
        item->setData(Qt::UserRole, uuid);
        ui->pairedListWidget->addItem(item);
    }

    if (ui->pairedListWidget->count() > 0) {
        ui->pairedListWidget->setCurrentRow(0);
    }
}

void PageConnection::on_serialRefreshButton_clicked()
{
    if (mOpenroad) {
        ui->serialPortBox->clear();
        QList<VSerialInfo_t> ports = mOpenroad->listSerialPorts();
        foreach(const VSerialInfo_t &port, ports) {
            ui->serialPortBox->addItem(port.name, port.systemPath);
        }
        ui->serialPortBox->setCurrentIndex(0);
    }
}

void PageConnection::on_serialDisconnectButton_clicked()
{
    if (mOpenroad) {
        mOpenroad->disconnectPort();
    }
}

void PageConnection::on_serialConnectButton_clicked()
{
    if (mOpenroad) {
        mOpenroad->connectSerial(ui->serialPortBox->currentData().toString(),
                             ui->serialBaudBox->value());
    }
}

void PageConnection::on_CANbusScanButton_clicked()
{
    if (mOpenroad) {
        ui->CANbusScanButton->setEnabled(false);
        mOpenroad->connectCANbus("socketcan", ui->CANbusInterfaceBox->currentData().toString(),
                             ui->CANbusBitrateBox->value());

        ui->CANbusTargetIdBox->clear();
        mOpenroad->scanCANbus();
        ui->CANbusScanButton->setEnabled(true);
    }
}

void PageConnection::on_CANbusDisconnectButton_clicked()
{
    if (mOpenroad) {
        mOpenroad->disconnectPort();
    }
}

void PageConnection::on_CANbusConnectButton_clicked()
{
    if (mOpenroad) {
        mOpenroad->setCANbusReceiverID(ui->CANbusTargetIdBox->currentData().toInt());
        mOpenroad->connectCANbus("socketcan", ui->CANbusInterfaceBox->currentData().toString(),
                             ui->CANbusBitrateBox->value());
    }
}

void PageConnection::on_tcpDisconnectButton_clicked()
{
    if (mOpenroad) {
        mOpenroad->disconnectPort();
    }
}

void PageConnection::on_tcpConnectButton_clicked()
{
    if (mOpenroad) {
        QString tcpServer = ui->tcpServerEdit->text();
        int tcpPort = ui->tcpPortBox->value();
        mOpenroad->connectTcp(tcpServer, tcpPort);
    }
}

void PageConnection::on_helpButton_clicked()
{
    if (mOpenroad) {
        HelpDialog::showHelp(this, mOpenroad->infoConfig(), "help_can_forward");
    }
}

void PageConnection::on_canFwdButton_toggled(bool checked)
{
    if (mOpenroad) {
        if (mOpenroad->commands()->getCanSendId() >= 0 || !checked) {
            mOpenroad->commands()->setSendCan(checked);
        } else {
            mOpenroad->emitMessageDialog("CAN Forward",
                                     "No CAN device is selected. Click on the refresh button "
                                     "if the selection box is empty.",
                                     false, false);
        }
    }
}

void PageConnection::on_autoConnectButton_clicked()
{
    Utility::autoconnectBlockingWithProgress(mOpenroad, this);
}

void PageConnection::on_bleScanButton_clicked()
{
#ifdef HAS_BLUETOOTH
    if (mOpenroad) {
        mOpenroad->bleDevice()->startScan();
        ui->bleScanButton->setEnabled(false);
    }
#endif
}

void PageConnection::on_bleDisconnectButton_clicked()
{
    if (mOpenroad) {
        mOpenroad->disconnectPort();
    }
}

void PageConnection::on_bleConnectButton_clicked()
{
    if (mOpenroad) {
        if (ui->bleDevBox->count() > 0) {
            mOpenroad->connectBle(ui->bleDevBox->currentData().toString());
        }
    }
}

void PageConnection::on_bleSetNameButton_clicked()
{
#ifdef HAS_BLUETOOTH
    if (mOpenroad) {
        QString name = ui->bleNameEdit->text();
        QString addr = ui->bleDevBox->currentData().toString();

        if (!name.isEmpty()) {
            mOpenroad->storeBleName(addr, name);
            name += " [";
            name += addr;
            name += "]";
            ui->bleDevBox->removeItem(0);
            ui->bleDevBox->insertItem(0, name, addr);
            ui->bleDevBox->setCurrentIndex(0);
        }
    }
#endif
}

void PageConnection::on_canFwdBox_currentIndexChanged(const QString &arg1)
{
    (void)arg1;
    if (mOpenroad && ui->canFwdBox->count() > 0) {
        mOpenroad->commands()->setCanSendId(quint32(ui->canFwdBox->currentData().toInt()));
    }
}

void PageConnection::on_canRefreshButton_clicked()
{
    if (mOpenroad) {
        ui->canRefreshButton->setEnabled(false);
        mOpenroad->commands()->pingCan();
    }
}

void PageConnection::on_canDefaultButton_clicked()
{
    ui->canFwdBox->clear();
    for (int dev = 0;dev < 255;dev++) {
        ui->canFwdBox->addItem(QString("VESC %1").arg(dev), dev);
    }
}

void PageConnection::on_pairConnectedButton_clicked()
{
    if (mOpenroad) {
        if (mOpenroad->isPortConnected()) {
            if (mOpenroad->commands()->isLimitedMode()) {
                mOpenroad->emitMessageDialog("Pair VESC",
                                         "The fiwmare must be updated to pair this VESC.",
                                         false, false);
            } else {
                QMessageBox::StandardButton reply;
                reply = QMessageBox::warning(this,
                                             tr("Pair connected VESC"),
                                             tr("This is going to pair the connected VESC with this instance of VESC Tool. VESC Tool instances "
                                                "that are not paired with this VESC will not be able to connect over bluetooth any more. Continue?"),
                                             QMessageBox::Ok | QMessageBox::Cancel);
                if (reply == QMessageBox::Ok) {
                    mOpenroad->addPairedUuid(mOpenroad->getConnectedUuid());
                    mOpenroad->storeSettings();
                    ConfigParams *ap = mOpenroad->appConfig();
                    mOpenroad->commands()->getAppConf();
                    bool res = Utility::waitSignal(ap, SIGNAL(updated()), 1500);

                    if (res) {
                        mOpenroad->appConfig()->updateParamBool("pairing_done", true, nullptr);
                        mOpenroad->commands()->setAppConf();
                    }
                }
            }
        } else {
            mOpenroad->emitMessageDialog("Pair VESC",
                                     "You are not connected to the VESC. Connect in order to pair it.",
                                     false, false);
        }
    }
}

void PageConnection::on_addConnectedButton_clicked()
{
    if (mOpenroad) {
        if (mOpenroad->isPortConnected()) {
            mOpenroad->addPairedUuid(mOpenroad->getConnectedUuid());
            mOpenroad->storeSettings();
        } else {
            mOpenroad->emitMessageDialog("Add UUID",
                                     "You are not connected to the VESC. Connect in order to add it.",
                                     false, false);
        }
    }
}

void PageConnection::on_deletePairedButton_clicked()
{
    if (mOpenroad) {
        QListWidgetItem *item = ui->pairedListWidget->currentItem();

        if (item) {
            QMessageBox::StandardButton reply;
            reply = QMessageBox::warning(this,
                                         tr("Delete paired VESC"),
                                         tr("This is going to delete this VESC from the paired list. If that VESC "
                                            "has the pairing flag set you won't be able to connect to it over BLE "
                                            "any more. Are you sure?"),
                                         QMessageBox::Ok | QMessageBox::Cancel);
            if (reply == QMessageBox::Ok) {
                mOpenroad->deletePairedUuid(item->data(Qt::UserRole).toString());
                mOpenroad->storeSettings();
            }
        }
    }
}

void PageConnection::on_clearPairedButton_clicked()
{
    if (mOpenroad) {
        QListWidgetItem *item = ui->pairedListWidget->currentItem();

        if (item) {
            QMessageBox::StandardButton reply;
            reply = QMessageBox::warning(this,
                                         tr("Clear paired VESCs"),
                                         tr("This is going to clear the pairing list of this instance of VESC Tool. Are you sure?"),
                                         QMessageBox::Ok | QMessageBox::Cancel);
            if (reply == QMessageBox::Ok) {
                mOpenroad->clearPairedUuids();
                mOpenroad->storeSettings();
            }
        }
    }
}

void PageConnection::on_addUuidButton_clicked()
{
    if (mOpenroad) {
        bool ok;
        QString text = QInputDialog::getText(this, "Add UUID",
                                             "UUID:", QLineEdit::Normal,
                                             "", &ok);
        if (ok) {
            mOpenroad->addPairedUuid(text);
            mOpenroad->storeSettings();
        }
    }
}

void PageConnection::on_unpairButton_clicked()
{
    if (mOpenroad) {
        if (mOpenroad->isPortConnected()) {
            if (mOpenroad->commands()->isLimitedMode()) {
                mOpenroad->emitMessageDialog("Unpair VESC",
                                         "The fiwmare must be updated on this VESC first.",
                                         false, false);
            } else {
                QListWidgetItem *item = ui->pairedListWidget->currentItem();

                if (item) {
                    QMessageBox::StandardButton reply;
                    reply = QMessageBox::warning(this,
                                                 tr("Unpair connected VESC"),
                                                 tr("This is going to unpair the connected VESC. Continue?"),
                                                 QMessageBox::Ok | QMessageBox::Cancel);
                    if (reply == QMessageBox::Ok) {
                        ConfigParams *ap = mOpenroad->appConfig();
                        mOpenroad->commands()->getAppConf();
                        bool res = Utility::waitSignal(ap, SIGNAL(updated()), 1500);

                        if (res) {
                            mOpenroad->appConfig()->updateParamBool("pairing_done", false, nullptr);
                            mOpenroad->commands()->setAppConf();
                            mOpenroad->deletePairedUuid(mOpenroad->getConnectedUuid());
                            mOpenroad->storeSettings();
                        }
                    }
                }
            }
        } else {
            mOpenroad->emitMessageDialog("Unpair VESC",
                                     "You are not connected to the VESC. Connect in order to unpair it.",
                                     false, false);
        }
    }
}

void PageConnection::on_tcpServerEnableBox_toggled(bool arg1)
{
    if (mOpenroad) {
        if (arg1) {
            mOpenroad->tcpServerStart(ui->tcpServerPortBox->value());
            ui->tcpServerPortBox->setEnabled(false);
        } else {
            mOpenroad->tcpServerStop();
        }
    }
}
