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

#ifndef SETUPWIZARDMOTOR_H
#define SETUPWIZARDMOTOR_H

#include <QWizard>
#include <QLabel>
#include <QComboBox>

#include "openroadinterface.h"
#include "widgets/paramtable.h"
#include "widgets/batterycalculator.h"
#include "widgets/detectbldc.h"
#include "widgets/detectfoc.h"
#include "widgets/detectfocencoder.h"
#include "widgets/detectfochall.h"
#include "pages/pageconnection.h"
#include "pages/pagefirmware.h"
#include "widgets/aspectimglabel.h"

class SetupWizardMotor : public QWizard
{
    Q_OBJECT

public:
    enum {
        Page_Intro = 0,
        Page_Connection,
        Page_Firmware,
        Page_MotorType,
        Page_Currents,
        Page_Voltages,
        Page_Sensors,
        Page_Bldc,
        Page_Foc,
        Page_FocEncoder,
        Page_FocHall,
        Page_Conclusion
    };

    enum {
        Sensor_Sensorless = 0,
        Sensor_Hall,
        Sensor_EncoderAbi,
        Sensor_EncoderAs,
        Sensor_Resolver_AD2S1205,
        Sensor_Encoder_SinCos,
        Sensor_EncoderBiSS
    };

    SetupWizardMotor(OpenroadInterface *openroad, QWidget *parent = 0);

private slots:
    void idChanged(int id);

private:
    AspectImgLabel *mSideLabel;

};

class IntroPage : public QWizardPage
{
    Q_OBJECT

public:
    IntroPage(OpenroadInterface *openroad, QWidget *parent = 0);
    int nextId() const Q_DECL_OVERRIDE;
    bool validatePage() Q_DECL_OVERRIDE;

private:
    OpenroadInterface *mOpenroad;
    QLabel *mLabel;

};

class ConnectionPage : public QWizardPage
{
    Q_OBJECT

public:
    ConnectionPage(OpenroadInterface *openroad, QWidget *parent = 0);
    int nextId() const Q_DECL_OVERRIDE;
    bool isComplete() const Q_DECL_OVERRIDE;

private:
    OpenroadInterface *mOpenroad;
    PageConnection *mPageConnection;

};

class FirmwarePage : public QWizardPage
{
    Q_OBJECT

public:
    FirmwarePage(OpenroadInterface *openroad, QWidget *parent = 0);
    int nextId() const Q_DECL_OVERRIDE;
    bool isComplete() const Q_DECL_OVERRIDE;
    bool validatePage() Q_DECL_OVERRIDE;
    void initializePage() Q_DECL_OVERRIDE;

private:
    OpenroadInterface *mOpenroad;
    PageFirmware *mPageFirmware;

};

class MotorTypePage : public QWizardPage
{
    Q_OBJECT

public:
    MotorTypePage(OpenroadInterface *openroad, QWidget *parent = 0);
    int nextId() const Q_DECL_OVERRIDE;
    bool validatePage() Q_DECL_OVERRIDE;
    void showEvent(QShowEvent *event) Q_DECL_OVERRIDE;

private:
    OpenroadInterface *mOpenroad;
    ParamTable *mParamTab;
    bool mLoadDefaultAsked;

};

class CurrentsPage : public QWizardPage
{
    Q_OBJECT

public:
    CurrentsPage(OpenroadInterface *openroad, QWidget *parent = 0);
    int nextId() const Q_DECL_OVERRIDE;
    bool validatePage() Q_DECL_OVERRIDE;
    void showEvent(QShowEvent *event) Q_DECL_OVERRIDE;

private:
    OpenroadInterface *mOpenroad;
    ParamTable *mParamTab;
    QLabel *mLabel;
    bool mWarningShown;
    bool mConfigureBatteryCutoff;

};

class VoltagesPage : public QWizardPage
{
    Q_OBJECT

public:
    VoltagesPage(OpenroadInterface *openroad, QWidget *parent = 0);
    int nextId() const Q_DECL_OVERRIDE;
    bool validatePage() Q_DECL_OVERRIDE;

private:
    OpenroadInterface *mOpenroad;
    ParamTable *mParamTab;
    BatteryCalculator *mCalc;

};

class SensorsPage : public QWizardPage
{
    Q_OBJECT

public:
    SensorsPage(OpenroadInterface *openroad, QWidget *parent = 0);
    int nextId() const Q_DECL_OVERRIDE;
    bool validatePage() Q_DECL_OVERRIDE;
    void initializePage() Q_DECL_OVERRIDE;
    void cleanupPage() Q_DECL_OVERRIDE;

private slots:
    void indexChanged(int ind);

private:
    OpenroadInterface *mOpenroad;
    QComboBox *mSensorMode;
    ParamTable *mParamTab;
    int mTypeBefore;

};

class BldcPage : public QWizardPage
{
    Q_OBJECT

public:
    BldcPage(OpenroadInterface *openroad, QWidget *parent = 0);
    int nextId() const Q_DECL_OVERRIDE;
    bool validatePage() Q_DECL_OVERRIDE;
    void initializePage() Q_DECL_OVERRIDE;

private:
    OpenroadInterface *mOpenroad;
    ParamTable *mParamTab;
    DetectBldc *mDetect;

};

class FocPage : public QWizardPage
{
    Q_OBJECT

public:
    FocPage(OpenroadInterface *openroad, QWidget *parent = 0);
    int nextId() const Q_DECL_OVERRIDE;
    bool validatePage() Q_DECL_OVERRIDE;

private:
    OpenroadInterface *mOpenroad;
    ParamTable *mParamTab;
    DetectFoc *mDetect;

};

class FocEncoderPage : public QWizardPage
{
    Q_OBJECT

public:
    FocEncoderPage(OpenroadInterface *openroad, QWidget *parent = 0);
    int nextId() const Q_DECL_OVERRIDE;
    bool validatePage() Q_DECL_OVERRIDE;

private:
    OpenroadInterface *mOpenroad;
    ParamTable *mParamTab;
    DetectFocEncoder *mDetect;

};

class FocHallPage : public QWizardPage
{
    Q_OBJECT

public:
    FocHallPage(OpenroadInterface *openroad, QWidget *parent = 0);
    int nextId() const Q_DECL_OVERRIDE;
    bool validatePage() Q_DECL_OVERRIDE;

private:
    OpenroadInterface *mOpenroad;
    ParamTable *mParamTab;
    DetectFocHall *mDetect;

};

class ConclusionPage : public QWizardPage
{
    Q_OBJECT

public:
    ConclusionPage(OpenroadInterface *openroad, QWidget *parent = 0);
    int nextId() const Q_DECL_OVERRIDE;

private:
    OpenroadInterface *mOpenroad;
    QLabel *mLabel;

};

#endif // SETUPWIZARDMOTOR_H
