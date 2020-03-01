/*
    Copyright 2017 - 2019 Benjamin Vedder	benjamin@vedder.se

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

#include "utility.h"
#include <cmath>
#include <QProgressDialog>
#include <QEventLoop>
#include <QNetworkAccessManager>
#include <QUrl>
#include <QNetworkRequest>
#include <QDebug>
#include <QNetworkReply>
#include <QMessageBox>
#include <QFile>
#include <QFileInfo>
#include <QtGlobal>
#include <QNetworkInterface>
#include <QDirIterator>

#ifdef Q_OS_ANDROID
#include <QtAndroid>
#include <QAndroidJniObject>
#include <QAndroidJniEnvironment>
#endif

Utility::Utility(QObject *parent) : QObject(parent)
{

}

double Utility::map(double x, double in_min, double in_max, double out_min, double out_max)
{
    return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
}

float Utility::throttle_curve(float val, float curve_acc, float curve_brake, int mode)
{
    float ret = 0.0;
    float val_a = fabsf(val);

    if (val < -1.0) {
        val = -1.0;
    }

    if (val > 1.0) {
        val = 1.0;
    }

    float curve;
    if (val >= 0.0) {
        curve = curve_acc;
    } else {
        curve = curve_brake;
    }

    // See
    // http://math.stackexchange.com/questions/297768/how-would-i-create-a-exponential-ramp-function-from-0-0-to-1-1-with-a-single-val
    if (mode == 0) { // Power
        if (curve >= 0.0) {
            ret = 1.0 - powf(1.0 - val_a, 1.0 + curve);
        } else {
            ret = powf(val_a, 1.0 - curve);
        }
    } else if (mode == 1) { // Exponential
        if (fabsf(curve) < 1e-10) {
            ret = val_a;
        } else {
            if (curve >= 0.0) {
                ret = 1.0 - ((expf(curve * (1.0 - val_a)) - 1.0) / (expf(curve) - 1.0));
            } else {
                ret = (expf(-curve * val_a) - 1.0) / (expf(-curve) - 1.0);
            }
        }
    } else if (mode == 2) { // Polynomial
        if (curve >= 0.0) {
            ret = 1.0 - ((1.0 - val_a) / (1.0 + curve * val_a));
        } else {
            ret = val_a / (1.0 - curve * (1.0 - val_a));
        }
    } else { // Linear
        ret = val_a;
    }

    if (val < 0.0) {
        ret = -ret;
    }

    return ret;
}

bool Utility::autoconnectBlockingWithProgress(VescInterface *openroad, QWidget *parent)
{
    if (!openroad) {
        return false;
    }

    QProgressDialog dialog("Autoconnecting...", QString(), 0, 0, parent);
    dialog.setWindowModality(Qt::WindowModal);
    dialog.show();

    bool res = openroad->autoconnect();

    if (!res) {
        openroad->emitMessageDialog(QObject::tr("Autoconnect"),
                                QObject::tr("Could not autoconnect. Make sure that the USB cable is plugged in "
                                            "and that the VESC is powered."),
                                false);
    }

    return res;
}

void Utility::checkVersion(VescInterface *openroad)
{
    QString version = QString::number(VT_VERSION);
    QUrl url("https://openroad-project.com/openroadtool-version.html");
    QNetworkAccessManager manager;
    QNetworkRequest request(url);
    QNetworkReply *reply = manager.get(request);
    QEventLoop loop;
    QObject::connect(reply, SIGNAL(finished()), &loop, SLOT(quit()));
    loop.exec();

    QString res = QString::fromUtf8(reply->readAll());

    if (res.startsWith("openroadtoolversion")) {
        res.remove(0, 15);
        res.remove(res.indexOf("openroadtoolversion"), res.size());

        if (res.toDouble() > version.toDouble()) {
            if (openroad) {
                openroad->emitStatusMessage("A new version of VESC Tool is available", true);
                openroad->emitMessageDialog(QObject::tr("New Software Available"),
                                        QObject::tr("A new version of VESC Tool is available. Go to "
                                                    "<a href=\"http://openroad-project.com/\">http://openroad-project.com/</a>"
                                                    " to download it and get all the latest features."),
                                        true);
            } else {
                qDebug() << "A new version of VESC Tool is available. Go to openroad-project.com to download it "
                            "and get all the latest features.";
            }
        }
    } else {
        qWarning() << res;
    }
}

QString Utility::fwChangeLog()
{
    QFile cl("://res/firmwares/CHANGELOG");
    if (cl.open(QIODevice::ReadOnly | QIODevice::Text)) {
        return QString::fromUtf8(cl.readAll());
    } else {
        return "";
    }
}

QString Utility::openroadToolChangeLog()
{
    QFile cl("://res/CHANGELOG");
    if (cl.open(QIODevice::ReadOnly | QIODevice::Text)) {
        return QString::fromUtf8(cl.readAll());
    } else {
        return "";
    }
}

QString Utility::aboutText()
{
    return tr("<b>VESC® Tool %1</b><br>"
          #if defined(VER_ORIGINAL)
              "Original Version<br>"
          #elif defined(VER_PLATINUM)
              "Platinum Version<br>"
          #elif defined(VER_GOLD)
              "Gold Version<br>"
          #elif defined(VER_SILVER)
              "Silver Version<br>"
          #elif defined(VER_BRONZE)
              "Bronze Version<br>"
          #elif defined(VER_FREE)
              "Free of Charge Version<br>"
          #endif
              "&copy; Benjamin Vedder 2016 - 2019<br>"
              "<a href=\"mailto:benjamin@vedder.se\">benjamin@vedder.se</a><br>"
              "<a href=\"https://openroad-project.com/\">https://openroad-project.com/</a>").
            arg(QString::number(VT_VERSION, 'f', 2));
}

QString Utility::uuid2Str(QByteArray uuid, bool space)
{
    QString strUuid;
    for (int i = 0;i < uuid.size();i++) {
        QString str = QString::number(uuid.at(i), 16).
                rightJustified(2, '0').toUpper();
        strUuid.append(((i > 0 && space) ? " " : "") + str);
    }

    return strUuid;
}

bool Utility::requestFilePermission()
{
#ifdef Q_OS_ANDROID
#if QT_VERSION >= QT_VERSION_CHECK(5, 10, 0)
    // https://codereview.qt-project.org/#/c/199162/
    QtAndroid::PermissionResult r = QtAndroid::checkPermission("android.permission.WRITE_EXTERNAL_STORAGE");
    if(r == QtAndroid::PermissionResult::Denied) {
        QtAndroid::requestPermissionsSync( QStringList() << "android.permission.WRITE_EXTERNAL_STORAGE", 5000);
        r = QtAndroid::checkPermission("android.permission.WRITE_EXTERNAL_STORAGE");
        if(r == QtAndroid::PermissionResult::Denied) {
            return false;
        }
    }

    return true;
#else
    return true;
#endif
#else
    return true;
#endif
}

void Utility::keepScreenOn(bool on)
{
#ifdef Q_OS_ANDROID
    QtAndroid::runOnAndroidThread([on]{
        QAndroidJniObject activity = QtAndroid::androidActivity();
        if (activity.isValid()) {
            QAndroidJniObject window =
                    activity.callObjectMethod("getWindow", "()Landroid/view/Window;");

            if (window.isValid()) {
                const int FLAG_KEEP_SCREEN_ON = 128;
                if (on) {
                    window.callMethod<void>("addFlags", "(I)V", FLAG_KEEP_SCREEN_ON);
                } else {
                    window.callMethod<void>("clearFlags", "(I)V", FLAG_KEEP_SCREEN_ON);
                }
            }
        }
        QAndroidJniEnvironment env;
        if (env->ExceptionCheck()) {
            env->ExceptionClear();
        }
    });
#else
    (void)on;
#endif
}

bool Utility::waitSignal(QObject *sender, QString signal, int timeoutMs)
{
    QEventLoop loop;
    QTimer timeoutTimer;
    timeoutTimer.setSingleShot(true);
    timeoutTimer.start(timeoutMs);
    auto conn1 = QObject::connect(sender, signal.toLocal8Bit().data(), &loop, SLOT(quit()));
    auto conn2 = QObject::connect(&timeoutTimer, SIGNAL(timeout()), &loop, SLOT(quit()));
    loop.exec();

    QObject::disconnect(conn1);
    QObject::disconnect(conn2);

    return timeoutTimer.isActive();
}

void Utility::sleepWithEventLoop(int timeMs)
{
    QEventLoop loop;
    QTimer timeoutTimer;
    timeoutTimer.setSingleShot(true);
    timeoutTimer.start(timeMs);
    auto conn1 = QObject::connect(&timeoutTimer, SIGNAL(timeout()), &loop, SLOT(quit()));
    loop.exec();
    QObject::disconnect(conn1);
}

QString Utility::detectAllFoc(VescInterface *openroad,
                              bool detect_can, double max_power_loss, double min_current_in,
                              double max_current_in, double openloop_rpm, double sl_erpm)
{
    QString res;
    bool detectOk = true;

    openroad->commands()->detectAllFoc(detect_can, max_power_loss, min_current_in,
                                   max_current_in, openloop_rpm, sl_erpm);

    QEventLoop loop;
    QTimer timeoutTimer;
    QTimer pollTimer;
    timeoutTimer.setSingleShot(true);
    timeoutTimer.start(180000);
    pollTimer.start(100);

    openroad->commands()->disableAppOutput(180000, true);

    int resDetect = 0;
    auto conn = connect(openroad->commands(), &Commands::detectAllFocReceived,
                        [&resDetect, &loop](int result) {
        resDetect = result;
        loop.quit();
    });

    QString pollRes;
    auto conn2 = connect(&pollTimer, &QTimer::timeout,
                        [&pollRes, &loop, &openroad]() {
        if (!openroad->isPortConnected()) {
            pollRes = "VESC disconnected during detection.";
            loop.quit();
        }
    });

    connect(&timeoutTimer, SIGNAL(timeout()), &loop, SLOT(quit()));
    loop.exec();

    disconnect(conn);
    disconnect(conn2);

    if (timeoutTimer.isActive() && pollRes.isEmpty()) {
        if (resDetect >= 0) {
            ConfigParams *p = openroad->mcConfig();
            ConfigParams *ap = openroad->appConfig();

            // MCConf should have been sent after the detection
            openroad->commands()->getAppConf();
            waitSignal(ap, SIGNAL(updated()), 1500);

            auto genRes = [&p, &ap]() {
                QString sensors;
                switch (p->getParamEnum("foc_sensor_mode")) {
                case 0: sensors = "Sensorless"; break;
                case 1: sensors = "Encoder"; break;
                case 2: sensors = "Hall Sensors"; break;
                default: break; }
                return QString("VESC ID            : %1\n"
                               "Motor current      : %2 A\n"
                               "Motor R            : %3 mΩ\n"
                               "Motor L            : %4 µH\n"
                               "Motor Flux Linkage : %5 mWb\n"
                               "Temp Comp          : %6\n"
                               "Sensors            : %7").
                        arg(ap->getParamInt("controller_id")).
                        arg(p->getParamDouble("l_current_max"), 0, 'f', 2).
                        arg(p->getParamDouble("foc_motor_r") * 1e3, 0, 'f', 2).
                        arg(p->getParamDouble("foc_motor_l") * 1e6, 0, 'f', 2).
                        arg(p->getParamDouble("foc_motor_flux_linkage") * 1e3, 0, 'f', 2).
                        arg(p->getParamBool("foc_temp_comp") ? "True" : "False").
                        arg(sensors);
            };

            QVector<int> canDevs = openroad->scanCan();
            res = genRes();

            int canLastFwd = openroad->commands()->getSendCan();
            int canLastId = openroad->commands()->getCanSendId();
            openroad->ignoreCanChange(true);

            if (!canDevs.empty()) {
                res += "\n\nVESCs on CAN-bus:";
            }

            for (int id: canDevs) {
                openroad->commands()->setSendCan(true, id);

                if (!checkFwCompatibility(openroad)) {
                    openroad->emitMessageDialog("FW Versions",
                                            "All VESCs must have the latest firmware to perform this operation.",
                                            false, false);
                    break;
                }

                openroad->commands()->getMcconf();
                waitSignal(p, SIGNAL(updated()), 1500);
                openroad->commands()->getAppConf();
                waitSignal(ap, SIGNAL(updated()), 1500);
                res += "\n\n" + genRes();
            }

            openroad->commands()->setSendCan(canLastFwd, canLastId);
            openroad->ignoreCanChange(false);
            openroad->commands()->getMcconf();
            waitSignal(p, SIGNAL(updated()), 1500);
            openroad->commands()->getAppConf();
            waitSignal(ap, SIGNAL(updated()), 1500);
        } else {
            QString reason;
            switch (resDetect) {
            case -1: reason = "Sensor detection failed"; break;
            case -10: reason = "Flux linkage detection failed"; break;
            case -50: reason = "CAN detection timeout"; break;
            case -51: reason = "CAN detection failed"; break;
            default: reason = QString::number(resDetect); break;
            }

            res = QString("Detection failed. Reason:\n%1").arg(reason);
            detectOk = false;
        }
    } else {
        if (!pollRes.isEmpty()) {
            res = QString("Detection failed. Reason:\n%1").arg(pollRes);
        } else {
            res = "Detection timed out.";
        }
        detectOk = false;
    }

    if (detectOk) {
        res.prepend("Success!\n\n");
    }

    openroad->commands()->disableAppOutput(0, true);

    return res;
}

bool Utility::resetInputCan(VescInterface *openroad, QVector<int> canIds)
{
    bool res = true;

    bool canLastFwd = openroad->commands()->getSendCan();
    int canLastId = openroad->commands()->getCanSendId();

    openroad->ignoreCanChange(true);

    // Local VESC first
    ConfigParams *ap = openroad->appConfig();
    openroad->commands()->setSendCan(false);

    if (!checkFwCompatibility(openroad)) {
        openroad->emitMessageDialog("FW Versions",
                                "All VESCs must have the latest firmware to perform this operation.",
                                false, false);
        res = false;
        qWarning() << "Incompatible firmware";
    }

    if (res) {
        openroad->commands()->getAppConf();
        res = waitSignal(ap, SIGNAL(updated()), 1500);

        if (!res) {
            qWarning() << "Appconf not received";
        }
    }

    if (res) {
        int canId = ap->getParamInt("controller_id");
        int canStatus = ap->getParamEnum("send_can_status");
        openroad->commands()->getAppConfDefault();
        res = waitSignal(ap, SIGNAL(updated()), 1500);

        if (!res) {
            qWarning() << "Default appconf not received";
        }

        if (res) {
            ap->updateParamInt("controller_id", canId);
            ap->updateParamEnum("send_can_status", canStatus);
            openroad->commands()->setAppConf();
            res = waitSignal(openroad->commands(), SIGNAL(ackReceived(QString)), 3000);

            if (!res) {
                qWarning() << "Appconf set no ack received";
            }
        }
    }

    // All VESCs on CAN-bus
    if (res) {
        for (int id: canIds) {
            openroad->commands()->setSendCan(true, id);

            if (!checkFwCompatibility(openroad)) {
                openroad->emitMessageDialog("FW Versions",
                                        "All VESCs must have the latest firmware to perform this operation.",
                                        false, false);
                res = false;
            }

            if (!res) {
                qWarning() << "Incompatible firmware";
                break;
            }

            openroad->commands()->getAppConf();
            res = waitSignal(ap, SIGNAL(updated()), 1500);

            if (!res) {
                qWarning() << "Appconf not received";
                break;
            }

            int canId = ap->getParamInt("controller_id");
            int canStatus = ap->getParamEnum("send_can_status");
            openroad->commands()->getAppConfDefault();
            res = waitSignal(ap, SIGNAL(updated()), 1500);

            if (!res) {
                qWarning() << "Default appconf not received";
                break;
            }

            ap->updateParamInt("controller_id", canId);
            ap->updateParamEnum("send_can_status", canStatus);
            openroad->commands()->setAppConf();
            res = waitSignal(openroad->commands(), SIGNAL(ackReceived(QString)), 3000);

            if (!res) {
                qWarning() << "Appconf set no ack received";
                break;
            }
        }
    }

    openroad->commands()->setSendCan(canLastFwd, canLastId);
    openroad->commands()->getAppConf();
    if (!waitSignal(ap, SIGNAL(updated()), 1500)) {
        qWarning() << "Appconf not received";
        res = false;
    }

    openroad->ignoreCanChange(false);

    return res;
}

bool Utility::setBatteryCutCan(VescInterface *openroad, QVector<int> canIds,
                               double cutStart, double cutEnd)
{
    bool res = true;

    bool canLastFwd = openroad->commands()->getSendCan();
    int canLastId = openroad->commands()->getCanSendId();

    openroad->ignoreCanChange(true);

    // Local VESC first
    ConfigParams *p = openroad->mcConfig();
    openroad->commands()->setSendCan(false);

    if (!checkFwCompatibility(openroad)) {
        openroad->emitMessageDialog("FW Versions",
                                "All VESCs must have the latest firmware to perform this operation.",
                                false, false);
        res = false;
    }

    if (res) {
        openroad->commands()->getMcconf();
        res = waitSignal(p, SIGNAL(updated()), 1500);
        if (!res) {
            openroad->emitMessageDialog("Read Motor Configuration",
                                    "Could not read motor configuration.",
                                    false, false);
        }
    }

    if (res) {
        p->updateParamDouble("l_battery_cut_start", cutStart);
        p->updateParamDouble("l_battery_cut_end", cutEnd);
        openroad->commands()->setMcconf(false);
        res = waitSignal(openroad->commands(), SIGNAL(ackReceived(QString)), 2000);

        if (!res) {
            openroad->emitMessageDialog("Write Motor Configuration",
                                    "Could not write motor configuration.",
                                    false, false);
        }
    }

    // All VESCs on CAN-bus
    if (res) {
        for (int id: canIds) {
            openroad->commands()->setSendCan(true, id);

            if (!checkFwCompatibility(openroad)) {
                openroad->emitMessageDialog("FW Versions",
                                        "All VESCs must have the latest firmware to perform this operation.",
                                        false, false);
                res = false;
                break;
            }

            openroad->commands()->getMcconf();
            res = waitSignal(p, SIGNAL(updated()), 1500);

            if (!res) {
                openroad->emitMessageDialog("Read Motor Configuration",
                                        "Could not read motor configuration.",
                                        false, false);

                break;
            }

            p->updateParamDouble("l_battery_cut_start", cutStart);
            p->updateParamDouble("l_battery_cut_end", cutEnd);
            openroad->commands()->setMcconf(false);
            res = waitSignal(openroad->commands(), SIGNAL(ackReceived(QString)), 2000);

            if (!res) {
                openroad->emitMessageDialog("Write Motor Configuration",
                                        "Could not write motor configuration.",
                                        false, false);

                break;
            }
        }
    }

    openroad->commands()->setSendCan(canLastFwd, canLastId);
    openroad->commands()->getMcconf();
    if (!waitSignal(p, SIGNAL(updated()), 1500)) {
        res = false;

        if (!res) {
            openroad->emitMessageDialog("Read Motor Configuration",
                                    "Could not read motor configuration.",
                                    false, false);
        }
    }

    openroad->ignoreCanChange(false);

    return res;
}

bool Utility::setBatteryCutCanFromCurrentConfig(VescInterface *openroad, QVector<int> canIds)
{
    ConfigParams *p = openroad->mcConfig();

    int battType = p->getParamEnum("si_battery_type");
    int cells = p->getParamInt("si_battery_cells");
    double start = -1.0;
    double end = -1.0;

    if (battType == 0) {
        start = 3.4;
        end = 3.0;
    } else if (battType == 1) {
        start = 2.9;
        end = 2.6;
    } else {
        return false;
    }

    start *= (double)cells;
    end *= (double)cells;

    return setBatteryCutCan(openroad, canIds, start, end);
}

bool Utility::setInvertDirection(VescInterface *openroad, int canId, bool inverted)
{
    bool res = true;

    bool canLastFwd = openroad->commands()->getSendCan();
    int canLastId = openroad->commands()->getCanSendId();

    openroad->ignoreCanChange(true);
    openroad->commands()->setSendCan(canId >= 0, canId);

    if (!checkFwCompatibility(openroad)) {
        openroad->emitMessageDialog("FW Versions",
                                "All VESCs must have the latest firmware to perform this operation.",
                                false, false);
        res = false;
    }

    ConfigParams *p = openroad->mcConfig();

    if (res) {
        openroad->commands()->getMcconf();
        res = waitSignal(p, SIGNAL(updated()), 1500);
    }

    if (res) {
        p->updateParamBool("m_invert_direction", inverted);
        openroad->commands()->setMcconf(false);
        res = waitSignal(openroad->commands(), SIGNAL(ackReceived(QString)), 2000);
    }

    openroad->commands()->setSendCan(canLastFwd, canLastId);
    openroad->commands()->getMcconf();
    if (!waitSignal(p, SIGNAL(updated()), 1500)) {
        res = false;
    }

    openroad->ignoreCanChange(false);

    return res;
}

bool Utility::getInvertDirection(VescInterface *openroad, int canId)
{
    bool res = false;

    bool canLastFwd = openroad->commands()->getSendCan();
    int canLastId = openroad->commands()->getCanSendId();

    openroad->ignoreCanChange(true);
    openroad->commands()->setSendCan(canId >= 0, canId);

    if (!checkFwCompatibility(openroad)) {
        openroad->emitMessageDialog("FW Versions",
                                "All VESCs must have the latest firmware to perform this operation.",
                                false, false);
        openroad->commands()->setSendCan(canLastFwd, canLastId);
        openroad->ignoreCanChange(false);
        return false;
    }

    ConfigParams *p = openroad->mcConfig();
    openroad->commands()->getMcconf();
    waitSignal(p, SIGNAL(updated()), 1500);
    res = p->getParamBool("m_invert_direction");

    openroad->commands()->setSendCan(canLastFwd, canLastId);
    openroad->commands()->getMcconf();
    waitSignal(p, SIGNAL(updated()), 1500);

    openroad->ignoreCanChange(false);

    return res;
}

QString Utility::testDirection(VescInterface *openroad, int canId, double duty, int ms)
{
    bool canLastFwd = openroad->commands()->getSendCan();
    int canLastId = openroad->commands()->getCanSendId();

    openroad->commands()->disableAppOutput(ms, true);

    openroad->ignoreCanChange(true);
    openroad->commands()->setSendCan(canId >= 0, canId);

    if (!checkFwCompatibility(openroad)) {
        openroad->emitMessageDialog("FW Versions",
                                "All VESCs must have the latest firmware to perform this operation.",
                                false, false);
        openroad->commands()->setSendCan(canLastFwd, canLastId);
        openroad->ignoreCanChange(false);
        return "FW not up to date";
    }

    QEventLoop loop;
    QTimer timeoutTimer;
    QTimer pollTimer;
    timeoutTimer.setSingleShot(true);
    timeoutTimer.start(ms);
    pollTimer.start(20);

    QString pollRes = "Ok";
    auto conn = connect(&pollTimer, &QTimer::timeout,
                        [&pollRes, &loop, &openroad, &duty, &ms, &timeoutTimer]() {
        if (!openroad->isPortConnected()) {
            pollRes = "VESC disconnected.";
            loop.quit();
        } else {
            double d = 2.0 * duty * (double)(ms - timeoutTimer.remainingTime()) / (double)ms;
            if (fabs(d) > fabs(duty)) {
                d = duty;
            }
            openroad->commands()->setDutyCycle(d);
        }
    });

    connect(&timeoutTimer, SIGNAL(timeout()), &loop, SLOT(quit()));
    loop.exec();

    disconnect(conn);

    openroad->commands()->setCurrent(0.0);

    openroad->commands()->setSendCan(canLastFwd, canLastId);
    openroad->ignoreCanChange(false);

    return pollRes;
}

/**
 * @brief Utility::restoreConfAll
 * Restore the VESC configuration to the default values.
 *
 * @param openroad
 * Pointer to a connected VescInterface instance.
 *
 * @param can
 * Scan CAN-bus and restore all devices.
 *
 * @param mc
 * Restore mc configuration.
 *
 * @param app
 * Restore app configuration.
 *
 * @return
 * true for success, false otherwise.
 */
bool Utility::restoreConfAll(VescInterface *openroad, bool can, bool mc, bool app)
{
    bool res = true;

    bool canLastFwd = openroad->commands()->getSendCan();
    int canLastId = openroad->commands()->getCanSendId();

    if (can) {
        openroad->ignoreCanChange(true);
        openroad->commands()->setSendCan(false);
        if (!checkFwCompatibility(openroad)) {
            openroad->emitMessageDialog("FW Versions",
                                    "All VESCs must have the latest firmware to perform this operation.",
                                    false, false);
            openroad->commands()->setSendCan(canLastFwd, canLastId);
            openroad->ignoreCanChange(false);
            return false;
        }
    }

    if (mc) {
        ConfigParams *p = openroad->mcConfig();
        openroad->commands()->getMcconfDefault();
        res = waitSignal(p, SIGNAL(updated()), 1500);

        if (res) {
            openroad->commands()->setMcconf(false);
            res = waitSignal(openroad->commands(), SIGNAL(ackReceived(QString)), 2000);
        }
    }

    if (app) {
        ConfigParams *p = openroad->appConfig();
        openroad->commands()->getAppConfDefault();
        res = waitSignal(p, SIGNAL(updated()), 1500);

        if (res) {
            openroad->commands()->setAppConf();
            res = waitSignal(openroad->commands(), SIGNAL(ackReceived(QString)), 2000);
        }
    }

    if (res && can) {
        QVector<int> canDevs = openroad->scanCan();

        for (int d: canDevs) {
            openroad->commands()->setSendCan(true, d);

            if (!checkFwCompatibility(openroad)) {
                openroad->emitMessageDialog("FW Versions",
                                        "All VESCs must have the latest firmware to perform this operation.",
                                        false, false);
                openroad->commands()->setSendCan(canLastFwd, canLastId);
                openroad->ignoreCanChange(false);
                return false;
            }

            if (mc) {
                ConfigParams *p = openroad->mcConfig();
                openroad->commands()->getMcconfDefault();
                res = waitSignal(p, SIGNAL(updated()), 1500);

                if (!res) {
                    break;
                }

                openroad->commands()->setMcconf(false);
                res = waitSignal(openroad->commands(), SIGNAL(ackReceived(QString)), 2000);

                if (!res) {
                    break;
                }
            }

            if (app) {
                ConfigParams *p = openroad->appConfig();
                openroad->commands()->getAppConfDefault();
                res = waitSignal(p, SIGNAL(updated()), 1500);

                if (!res) {
                    break;
                }

                openroad->commands()->setAppConf();
                res = waitSignal(openroad->commands(), SIGNAL(ackReceived(QString)), 2000);

                if (!res) {
                    break;
                }
            }
        }
    }

    if (can) {
        openroad->commands()->setSendCan(canLastFwd, canLastId);

        if (mc) {
            ConfigParams *p = openroad->mcConfig();
            openroad->commands()->getMcconf();
            if (!waitSignal(p, SIGNAL(updated()), 1500)) {
                res = false;
                qWarning() << "Could not restore mc conf";
            }
        }

        if (app) {
            ConfigParams *p = openroad->appConfig();
            openroad->commands()->getAppConf();
            if (!waitSignal(p, SIGNAL(updated()), 1500)) {
                res = false;
                qWarning() << "Could not restore app conf";
            }
        }

        openroad->ignoreCanChange(false);
    }

    return res;
}

bool Utility::almostEqual(double A, double B, double eps)
{
    return fabs(A - B) <= eps * fmax(1.0f, fmax(fabs(A), fabs(B)));
}

bool Utility::createParamParserC(VescInterface *openroad, QString filename)
{
    if (filename.toLower().endsWith(".c") || filename.toLower().endsWith(".h")) {
        filename.chop(2);
    }

    QString sourceFileName = filename + ".c";
    QString headerFileName = filename + ".h";

    QFile sourceFile(sourceFileName);
    if (!sourceFile.open(QIODevice::WriteOnly)) {
        qWarning() << tr("Could not open %1 for writing").arg(sourceFileName);
        return false;
    }

    QFile headerFile(headerFileName);
    if (!headerFile.open(QIODevice::WriteOnly)) {
        qWarning() << tr("Could not open %1 for writing").arg(headerFileName);
        return false;
    }

    QTextStream outSource(&sourceFile);
    QTextStream outHeader(&headerFile);
    QFileInfo headerInfo(headerFile);
    QString headerNameStr = headerInfo.fileName().toUpper().replace(".", "_") + "_";
    QString prefix = headerInfo.fileName();
    prefix.chop(2);

    outHeader << "// This file is autogenerated by VESC Tool\n\n";

    outHeader << "#ifndef " + headerNameStr + "\n";
    outHeader << "#define " + headerNameStr + "\n\n";

    outHeader << "#include \"datatypes.h\"\n";
    outHeader << "#include <stdint.h>\n";
    outHeader << "#include <stdbool.h>\n\n";

    outHeader << "// Constants\n";
    outHeader << "#define MCCONF_SIGNATURE\t\t" << openroad->mcConfig()->getSignature() << "\n";
    outHeader << "#define APPCONF_SIGNATURE\t\t" << openroad->appConfig()->getSignature() << "\n\n";

    outHeader << "// Functions\n";
    outHeader << "int32_t " << prefix << "_serialize_mcconf(uint8_t *buffer, const mc_configuration *conf);\n";
    outHeader << "int32_t " << prefix << "_serialize_appconf(uint8_t *buffer, const app_configuration *conf);\n\n";

    outHeader << "bool " << prefix << "_deserialize_mcconf(const uint8_t *buffer, mc_configuration *conf);\n";
    outHeader << "bool " << prefix << "_deserialize_appconf(const uint8_t *buffer, app_configuration *conf);\n\n";

    outHeader << "void " << prefix << "_set_defaults_mcconf(mc_configuration *conf);\n";
    outHeader << "void " << prefix << "_set_defaults_appconf(app_configuration *conf);\n\n";

    outHeader << "// " + headerNameStr + "\n";
    outHeader << "#endif\n";

    auto serialFunc = [](ConfigParams *params, QTextStream &s) {
        QStringList serialOrder = params->getSerializeOrder();

        for (int i = 0;i < serialOrder.size();i++) {
            QString name = serialOrder.at(i);

            ConfigParam *p = params->getParam(name);

            int last__ = name.lastIndexOf("__");
            if (last__ > 0) {
                QString end = name.mid(last__ + 2);
                bool ok;
                int endNum = end.toInt(&ok);
                if (ok) {
                    name = name.left(last__) + QString("[%1]").arg(endNum);
                }
            }

            if (p) {
                switch (p->type) {
                case CFG_T_BOOL:
                case CFG_T_ENUM:
                    s << "\t" << "buffer[ind++] = conf->" << name << ";\n";
                    break;

                case CFG_T_INT:
                    switch (p->vTx) {
                    case VESC_TX_UINT8:
                    case VESC_TX_INT8:
                        s << "\t" << "buffer[ind++] = (uint8_t)conf->" << name << ";\n";
                        break;

                    case VESC_TX_UINT16:
                        s << "\t" << "buffer_append_uint16(buffer, conf->" << name << ", &ind);\n";
                        break;

                    case VESC_TX_INT16:
                        s << "\t" << "buffer_append_int16(buffer, conf->" << name << ", &ind);\n";
                        break;

                    case VESC_TX_UINT32:
                        s << "\t" << "buffer_append_uint32(buffer, conf->" << name << ", &ind);\n";
                        break;

                    case VESC_TX_INT32:
                        s << "\t" << "buffer_append_int32(buffer, conf->" << name << ", &ind);\n";
                        break;

                    default:
                        qWarning() << "Serialization type not supporter";
                        break;
                    }
                    break;

                case CFG_T_DOUBLE:
                    switch (p->vTx) {
                    case VESC_TX_DOUBLE16:
                        s << "\t" << "buffer_append_float16(buffer, conf->" << name << ", " << p->vTxDoubleScale << ", &ind);\n";
                        break;

                    case VESC_TX_DOUBLE32:
                        s << "\t" << "buffer_append_float32(buffer, conf->" << name << ", " << p->vTxDoubleScale << ", &ind);\n";
                        break;

                    case VESC_TX_DOUBLE32_AUTO:
                        s << "\t" << "buffer_append_float32_auto(buffer, conf->" << name << ", &ind);\n";
                        break;

                    default:
                        qWarning() << "Serialization type not supporter";
                        break;
                    }
                    break;

                default:
                    qWarning() << name << ": type not supported.";
                    break;
                }
            } else {
                qWarning() << name << "not found.";
            }
        }
    };

    auto deserialFunc = [](ConfigParams *params, QTextStream &s) {
        QStringList serialOrder = params->getSerializeOrder();

        for (int i = 0;i < serialOrder.size();i++) {
            QString name = serialOrder.at(i);

            ConfigParam *p = params->getParam(name);

            int last__ = name.lastIndexOf("__");
            if (last__ > 0) {
                QString end = name.mid(last__ + 2);
                bool ok;
                int endNum = end.toInt(&ok);
                if (ok) {
                    name = name.left(last__) + QString("[%1]").arg(endNum);
                }
            }

            if (p) {
                switch (p->type) {
                case CFG_T_BOOL:
                case CFG_T_ENUM:
                    s << "\tconf->" << name << " = buffer[ind++];\n";
                    break;

                case CFG_T_INT:
                    switch (p->vTx) {
                    case VESC_TX_UINT8:
                        s << "\tconf->" << name << " = buffer[ind++];\n";
                        break;

                    case VESC_TX_INT8:
                        s << "\tconf->" << name << " = (int8_t)buffer[ind++];\n";
                        break;

                    case VESC_TX_UINT16:
                        s << "\tconf->" << name << " = buffer_get_uint16(buffer, &ind);\n";
                        break;

                    case VESC_TX_INT16:
                        s << "\tconf->" << name << " = buffer_get_int16(buffer, &ind);\n";
                        break;

                    case VESC_TX_UINT32:
                        s << "\tconf->" << name << " = buffer_get_uint32(buffer, &ind);\n";
                        break;

                    case VESC_TX_INT32:
                        s << "\tconf->" << name << " = buffer_get_int32(buffer, &ind);\n";
                        break;

                    default:
                        qWarning() << "Serialization type not supporter";
                        break;
                    }
                    break;
                    break;

                case CFG_T_DOUBLE:
                    switch (p->vTx) {
                    case VESC_TX_DOUBLE16:
                        s << "\tconf->" << name << " = buffer_get_float16(buffer, " << p->vTxDoubleScale << ", &ind);\n";
                        break;

                    case VESC_TX_DOUBLE32:
                        s << "\tconf->" << name << " = buffer_get_float32(buffer, " << p->vTxDoubleScale << ", &ind);\n";
                        break;

                    case VESC_TX_DOUBLE32_AUTO:
                        s << "\tconf->" << name << " = buffer_get_float32_auto(buffer, &ind);\n";
                        break;

                    default:
                        qWarning() << "Serialization type not supporter";
                        break;
                    }
                    break;

                default:
                    qWarning() << name << ": type not supported.";
                    break;
                }
            } else {
                qWarning() << name << "not found.";
            }
        }
    };

    auto defaultFunc = [](ConfigParams *params, QTextStream &s) {
        QStringList serialOrder = params->getSerializeOrder();

        for (int i = 0;i < serialOrder.size();i++) {
            QString name = serialOrder.at(i);

            ConfigParam *p = params->getParam(name);

            int last__ = name.lastIndexOf("__");
            if (last__ > 0) {
                QString end = name.mid(last__ + 2);
                bool ok;
                int endNum = end.toInt(&ok);
                if (ok) {
                    name = name.left(last__) + QString("[%1]").arg(endNum);
                }
            }

            if (p) {
                QString def = p->cDefine;
                // Kind of a hack...
                if (name == "controller_id") {
                    def = "HW_DEFAULT_ID";
                }
                s << "\tconf->" << name << " = " << def << ";\n";
            } else {
                qWarning() << name << "not found.";
            }
        }
    };

    outSource << "// This file is autogenerated by VESC Tool\n\n";
    outSource << "#include \"buffer.h\"\n";
    outSource << "#include \"conf_general.h\"\n";
    outSource << "#include \"" << headerInfo.fileName() << "\"\n\n";

    outSource << "int32_t " << prefix << "_serialize_mcconf(uint8_t *buffer, const mc_configuration *conf) {\n";
    outSource << "\t" << "int32_t ind = 0;\n\n";
    outSource << "\t" << "buffer_append_uint32(buffer, MCCONF_SIGNATURE, &ind);\n\n";
    serialFunc(openroad->mcConfig(), outSource);
    outSource << "\n\t" << "return ind;\n";
    outSource << "}\n\n";

    outSource << "int32_t " << prefix << "_serialize_appconf(uint8_t *buffer, const app_configuration *conf) {\n";
    outSource << "\t" << "int32_t ind = 0;\n\n";
    outSource << "\t" << "buffer_append_uint32(buffer, APPCONF_SIGNATURE, &ind);\n\n";
    serialFunc(openroad->appConfig(), outSource);
    outSource << "\n\t" << "return ind;\n";
    outSource << "}\n\n";

    outSource << "bool " << prefix << "_deserialize_mcconf(const uint8_t *buffer, mc_configuration *conf) {\n";
    outSource << "\t" << "int32_t ind = 0;\n\n";
    outSource << "\t" << "uint32_t signature = buffer_get_uint32(buffer, &ind);\n";
    outSource << "\t" << "if (signature != MCCONF_SIGNATURE) {\n";
    outSource << "\t\t" << "return false;\n";
    outSource << "\t" << "}\n\n";
    deserialFunc(openroad->mcConfig(), outSource);
    outSource << "\n\t" << "return true;\n";
    outSource << "}\n\n";

    outSource << "bool " << prefix << "_deserialize_appconf(const uint8_t *buffer, app_configuration *conf) {\n";
    outSource << "\t" << "int32_t ind = 0;\n\n";
    outSource << "\t" << "uint32_t signature = buffer_get_uint32(buffer, &ind);\n";
    outSource << "\t" << "if (signature != APPCONF_SIGNATURE) {\n";
    outSource << "\t\t" << "return false;\n";
    outSource << "\t" << "}\n\n";
    deserialFunc(openroad->appConfig(), outSource);
    outSource << "\n\t" << "return true;\n";
    outSource << "}\n\n";

    outSource << "void " << prefix << "_set_defaults_mcconf(mc_configuration *conf) {\n";
    defaultFunc(openroad->mcConfig(), outSource);
    outSource << "}\n\n";

    outSource << "void " << prefix << "_set_defaults_appconf(app_configuration *conf) {\n";
    defaultFunc(openroad->appConfig(), outSource);
    outSource << "}\n";

    outHeader.flush();
    outSource.flush();
    headerFile.close();
    sourceFile.close();

    return true;
}

uint32_t Utility::crc32c(uint8_t *data, uint32_t len)
{
    uint32_t crc = 0xFFFFFFFF;

    for (uint32_t i = 0; i < len;i++) {
        uint32_t byte = data[i];
        crc = crc ^ byte;

        for (int j = 7;j >= 0;j--) {
            uint32_t mask = -(crc & 1);
            crc = (crc >> 1) ^ (0x82F63B78 & mask);
        }
    }

    return ~crc;
}

bool Utility::checkFwCompatibility(VescInterface *openroad)
{
    bool res = false;

    auto conn = connect(openroad->commands(), &Commands::fwVersionReceived,
            [&res, openroad](int major, int minor, QString hw, QByteArray uuid, bool isPaired) {
        (void)hw;(void)uuid;(void)isPaired;
        if (openroad->getSupportedFirmwarePairs().contains(qMakePair(major, minor))) {
            res = true;
        }
    });

    disconnect(openroad->commands(), SIGNAL(fwVersionReceived(int,int,QString,QByteArray,bool)),
               openroad, SLOT(fwVersionReceived(int,int,QString,QByteArray,bool)));

    openroad->commands()->getFwVersion();
    waitSignal(openroad->commands(), SIGNAL(fwVersionReceived(int,int,QString,QByteArray,bool)), 1500);

    disconnect(conn);

    connect(openroad->commands(), SIGNAL(fwVersionReceived(int,int,QString,QByteArray,bool)),
            openroad, SLOT(fwVersionReceived(int,int,QString,QByteArray,bool)));

    return res;
}

QVariantList Utility::getNetworkAddresses()
{
    QVariantList res;

    for(QHostAddress a: QNetworkInterface::allAddresses()) {
        if(!a.isLoopback()) {
            if (a.protocol() == QAbstractSocket::IPv4Protocol) {
                res << a.toString();
            }
        }
    }

    return res;
}

void Utility::startGnssForegroundService()
{
#ifdef Q_OS_ANDROID
    QAndroidJniObject::callStaticMethod<void>("com/vedder/openroad/Utils",
                                              "startVForegroundService",
                                              "(Landroid/content/Context;)V",
                                              QtAndroid::androidActivity().object());
#endif
}

void Utility::stopGnssForegroundService()
{
#ifdef Q_OS_ANDROID
    QAndroidJniObject::callStaticMethod<void>("com/vedder/openroad/Utils",
                                              "stopVForegroundService",
                                              "(Landroid/content/Context;)V",
                                              QtAndroid::androidActivity().object());
#endif
}

void Utility::llhToXyz(double lat, double lon, double height, double *x, double *y, double *z)
{
    double sinp = sin(lat * M_PI / 180.0);
    double cosp = cos(lat * M_PI / 180.0);
    double sinl = sin(lon * M_PI / 180.0);
    double cosl = cos(lon * M_PI / 180.0);
    double e2 = FE_WGS84 * (2.0 - FE_WGS84);
    double v = RE_WGS84 / sqrt(1.0 - e2 * sinp * sinp);

    *x = (v + height) * cosp * cosl;
    *y = (v + height) * cosp * sinl;
    *z = (v * (1.0 - e2) + height) * sinp;
}

void Utility::xyzToLlh(double x, double y, double z, double *lat, double *lon, double *height)
{
    double e2 = FE_WGS84 * (2.0 - FE_WGS84);
    double r2 = x * x + y * y;
    double za = z;
    double zk = 0.0;
    double sinp = 0.0;
    double v = RE_WGS84;

    while (fabs(za - zk) >= 1E-4) {
        zk = za;
        sinp = za / sqrt(r2 + za * za);
        v = RE_WGS84 / sqrt(1.0 - e2 * sinp * sinp);
        za = z + v * e2 * sinp;
    }

    *lat = (r2 > 1E-12 ? atan(za / sqrt(r2)) : (z > 0.0 ? M_PI / 2.0 : -M_PI / 2.0)) * 180.0 / M_PI;
    *lon = (r2 > 1E-12 ? atan2(y, x) : 0.0) * 180.0 / M_PI;
    *height = sqrt(r2 + za * za) - v;
}

void Utility::createEnuMatrix(double lat, double lon, double *enuMat)
{
    double so = sin(lon * M_PI / 180.0);
    double co = cos(lon * M_PI / 180.0);
    double sa = sin(lat * M_PI / 180.0);
    double ca = cos(lat * M_PI / 180.0);

    // ENU
    enuMat[0] = -so;
    enuMat[1] = co;
    enuMat[2] = 0.0;

    enuMat[3] = -sa * co;
    enuMat[4] = -sa * so;
    enuMat[5] = ca;

    enuMat[6] = ca * co;
    enuMat[7] = ca * so;
    enuMat[8] = sa;
}

void Utility::llhToEnu(const double *iLlh, const double *llh, double *xyz)
{
    double ix, iy, iz;
    llhToXyz(iLlh[0], iLlh[1], iLlh[2], &ix, &iy, &iz);

    double x, y, z;
    llhToXyz(llh[0], llh[1], llh[2], &x, &y, &z);

    double enuMat[9];
    createEnuMatrix(iLlh[0], iLlh[1], enuMat);

    double dx = x - ix;
    double dy = y - iy;
    double dz = z - iz;

    xyz[0] = enuMat[0] * dx + enuMat[1] * dy + enuMat[2] * dz;
    xyz[1] = enuMat[3] * dx + enuMat[4] * dy + enuMat[5] * dz;
    xyz[2] = enuMat[6] * dx + enuMat[7] * dy + enuMat[8] * dz;
}

void Utility::enuToLlh(const double *iLlh, const double *xyz, double *llh)
{
    double ix, iy, iz;
    llhToXyz(iLlh[0], iLlh[1], iLlh[2], &ix, &iy, &iz);

    double enuMat[9];
    createEnuMatrix(iLlh[0], iLlh[1], enuMat);

    double x = enuMat[0] * xyz[0] + enuMat[3] * xyz[1] + enuMat[6] * xyz[2] + ix;
    double y = enuMat[1] * xyz[0] + enuMat[4] * xyz[1] + enuMat[7] * xyz[2] + iy;
    double z = enuMat[2] * xyz[0] + enuMat[5] * xyz[1] + enuMat[8] * xyz[2] + iz;

    xyzToLlh(x, y, z, &llh[0], &llh[1], &llh[2]);
}

bool Utility::configCheckCompatibility(int fwMajor, int fwMinor)
{
    QDirIterator it("://res/config");

    while (it.hasNext()) {
        QFileInfo fi(it.next());
        QStringList names = fi.fileName().split("_o_");

        if (fi.isDir()) {
            for(auto name: names) {
                auto parts = name.split(".");
                if (parts.size() == 2) {
                    int major = parts.at(0).toInt();
                    int minor = parts.at(1).toInt();
                    if (major == fwMajor && minor == fwMinor) {
                        return true;
                    }
                }
            }
        }
    }

    return false;
}

bool Utility::configLoad(VescInterface *openroad, int fwMajor, int fwMinor)
{
    QDirIterator it("://res/config");

    while (it.hasNext()) {
        QFileInfo fi(it.next());
        QStringList names = fi.fileName().split("_o_");

        if (fi.isDir()) {
            for(auto name: names) {
                auto parts = name.split(".");
                if (parts.size() == 2) {
                    int major = parts.at(0).toInt();
                    int minor = parts.at(1).toInt();
                    if (major == fwMajor && minor == fwMinor) {
                        QFileInfo fMc(it.filePath() + "/parameters_mcconf.xml");
                        QFileInfo fApp(it.filePath() + "/parameters_appconf.xml");
                        QFileInfo fInfo(it.filePath() + "/info.xml");

                        if (fMc.exists() && fApp.exists() && fInfo.exists()) {
                            openroad->mcConfig()->loadParamsXml(fMc.absoluteFilePath());
                            openroad->appConfig()->loadParamsXml(fApp.absoluteFilePath());
                            openroad->infoConfig()->loadParamsXml(fInfo.absoluteFilePath());
                            openroad->emitConfigurationChanged();
                            return true;
                        } else {
                            qWarning() << "Configurations not found in firmware directory" << it.path();
                            return false;
                        }
                    }
                }
            }
        }
    }

    return false;
}

QPair<int, int> Utility::configLatestSupported()
{
    QPair<int, int> res = qMakePair(-1, -1);
    QDirIterator it("://res/config");

    while (it.hasNext()) {
        QFileInfo fi(it.next());
        QStringList names = fi.fileName().split("_o_");

        if (fi.isDir()) {
            for(auto name: names) {
                auto parts = name.split(".");
                if (parts.size() == 2) {
                    QPair<int, int> ver = qMakePair(parts.at(0).toInt(), parts.at(1).toInt());
                    if (ver > res) {
                        res = ver;
                    }
                }
            }
        }
    }

    return res;
}

bool Utility::configLoadLatest(VescInterface *openroad)
{
    auto latestSupported = configLatestSupported();

    if (latestSupported.first >= 0) {
        configLoad(openroad, latestSupported.first, latestSupported.second);
        return true;
    } else {
        return false;
    }
}

QVector<QPair<int, int> > Utility::configSupportedFws()
{
    QVector<QPair<int, int>> res;
    QDirIterator it("://res/config");

    while (it.hasNext()) {
        QFileInfo fi(it.next());
        QStringList names = fi.fileName().split("_o_");

        if (fi.isDir()) {
            for(auto name: names) {
                auto parts = name.split(".");
                if (parts.size() == 2) {
                    int major = parts.at(0).toInt();
                    int minor = parts.at(1).toInt();
                    res.append(qMakePair(major, minor));
                }
            }
        }
    }

    return res;
}

bool Utility::configLoadCompatible(VescInterface *openroad, QString &uuidRx)
{
    bool res = false;

    auto conn = connect(openroad->commands(), &Commands::fwVersionReceived,
            [&res, &uuidRx, openroad](int major, int minor, QString hw, QByteArray uuid, bool isPaired) {
        (void)hw;(void)uuid;(void)isPaired;

        if (openroad->getSupportedFirmwarePairs().contains(qMakePair(major, minor))) {
            res = true;
        } else {
            res = configLoad(openroad, major, minor);
        }

        QString uuidStr = uuid2Str(uuid, true);
        uuidRx = uuidStr.toUpper();
        uuidRx.replace(" ", "");

        if (!res) {
            openroad->emitMessageDialog("Load Config", "Could not load configuration parser.", false, false);
        }
    });

    disconnect(openroad->commands(), SIGNAL(fwVersionReceived(int,int,QString,QByteArray,bool)),
               openroad, SLOT(fwVersionReceived(int,int,QString,QByteArray,bool)));

    openroad->commands()->getFwVersion();

    if (!waitSignal(openroad->commands(), SIGNAL(fwVersionReceived(int,int,QString,QByteArray,bool)), 1500)) {
        openroad->emitMessageDialog("Load Config", "No response when reading firmware version.", false, false);
    }

    disconnect(conn);

    connect(openroad->commands(), SIGNAL(fwVersionReceived(int,int,QString,QByteArray,bool)),
            openroad, SLOT(fwVersionReceived(int,int,QString,QByteArray,bool)));

    return res;
}
