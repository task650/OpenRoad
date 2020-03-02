/*
    Copyright 2017 Benjamin Vedder	benjamin@vedder.se

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

#ifndef UTILITY_H
#define UTILITY_H

#include <QObject>
#include <QMetaEnum>
#include <cstdint>
#include "openroadinterface.h"

#define FE_WGS84        (1.0/298.257223563) // earth flattening (WGS84)
#define RE_WGS84        6378137.0           // earth semimajor axis (WGS84) (m)

class Utility : public QObject
{
    Q_OBJECT

public:
    explicit Utility(QObject *parent = nullptr);

    static double map(double x, double in_min, double in_max, double out_min, double out_max);
    static float throttle_curve(float val, float curve_acc, float curve_brake, int mode);
    static bool autoconnectBlockingWithProgress(OpenroadInterface *openroad, QWidget *parent = nullptr);
    Q_INVOKABLE static void checkVersion(OpenroadInterface *openroad = nullptr);
    Q_INVOKABLE static QString fwChangeLog();
    Q_INVOKABLE static QString openroadToolChangeLog();
    Q_INVOKABLE static QString aboutText();
    Q_INVOKABLE static QString uuid2Str(QByteArray uuid, bool space);
    Q_INVOKABLE static bool requestFilePermission();
    Q_INVOKABLE static void keepScreenOn(bool on);
    Q_INVOKABLE static bool waitSignal(QObject *sender, QString signal, int timeoutMs);
    Q_INVOKABLE static void sleepWithEventLoop(int timeMs);
    Q_INVOKABLE static QString detectAllFoc(OpenroadInterface *openroad,
                                            bool detect_can, double max_power_loss, double min_current_in,
                                            double max_current_in, double openloop_rpm, double sl_erpm);
    Q_INVOKABLE static bool resetInputCan(OpenroadInterface *openroad, QVector<int> canIds);
    Q_INVOKABLE static bool setBatteryCutCan(OpenroadInterface *openroad, QVector<int> canIds,
                                             double cutStart, double cutEnd);
    Q_INVOKABLE static bool setBatteryCutCanFromCurrentConfig(OpenroadInterface *openroad, QVector<int> canIds);
    Q_INVOKABLE static bool setInvertDirection(OpenroadInterface *openroad, int canId, bool inverted);
    Q_INVOKABLE static bool getInvertDirection(OpenroadInterface *openroad, int canId);
    Q_INVOKABLE static QString testDirection(OpenroadInterface *openroad, int canId, double duty, int ms);
    Q_INVOKABLE static bool restoreConfAll(OpenroadInterface *openroad, bool can, bool mc, bool app);
    Q_INVOKABLE static bool almostEqual(double A, double B, double eps);
    static bool createParamParserC(OpenroadInterface *openroad, QString filename);
    static uint32_t crc32c(uint8_t *data, uint32_t len);
    static bool checkFwCompatibility(OpenroadInterface *openroad);
    Q_INVOKABLE static QVariantList getNetworkAddresses();
    Q_INVOKABLE static void startGnssForegroundService();
    Q_INVOKABLE static void stopGnssForegroundService();

    static void llhToXyz(double lat, double lon, double height, double *x, double *y, double *z);
    static void xyzToLlh(double x, double y, double z, double *lat, double *lon, double *height);
    static void createEnuMatrix(double lat, double lon, double *enuMat);
    static void llhToEnu(const double *iLlh, const double *llh, double *xyz);
    static void enuToLlh(const double *iLlh, const double *xyz, double *llh);

    static bool configCheckCompatibility(int fwMajor, int fwMinor);
    static bool configLoad(OpenroadInterface *openroad, int fwMajor, int fwMinor);
    static QPair<int, int> configLatestSupported();
    static bool configLoadLatest(OpenroadInterface *openroad);
    static QVector<QPair<int, int>> configSupportedFws();
    static bool configLoadCompatible(OpenroadInterface *openroad, QString &uuidRx);

    template<typename QEnum>
    static QString QEnumToQString (const QEnum value) {
        return QString(QMetaEnum::fromType<QEnum>().valueToKey(value));
    }

signals:

public slots:
};

#endif // UTILITY_H
