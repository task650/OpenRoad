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

#include "pageappadc.h"
#include "ui_pageappadc.h"
#include "utility.h"

PageAppAdc::PageAppAdc(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::PageAppAdc)
{
    ui->setupUi(this);
    layout()->setContentsMargins(0, 0, 0, 0);
    mOpenroad = nullptr;

    ui->throttlePlot->addGraph();
    ui->throttlePlot->graph()->setName("Throttle Curve");
    ui->throttlePlot->xAxis->setLabel("Throttle Value");
    ui->throttlePlot->yAxis->setLabel("Output Value");
    ui->throttlePlot->legend->setVisible(true);
    ui->throttlePlot->axisRect()->insetLayout()->setInsetAlignment(0, Qt::AlignRight|Qt::AlignBottom);
}

PageAppAdc::~PageAppAdc()
{
    delete ui;
}

OpenroadInterface *PageAppAdc::openroad() const
{
    return mOpenroad;
}

void PageAppAdc::setOpenroad(OpenroadInterface *openroad)
{
    mOpenroad = openroad;

    if (mOpenroad) {
        reloadParams();

        ui->adcMap->setOpenroad(mOpenroad);

        connect(mOpenroad->appConfig(), SIGNAL(paramChangedDouble(QObject*,QString,double)),
                this, SLOT(paramChangedDouble(QObject*,QString,double)));
        connect(mOpenroad->appConfig(), SIGNAL(paramChangedEnum(QObject*,QString,int)),
                this, SLOT(paramChangedEnum(QObject*,QString,int)));

        paramChangedEnum(nullptr, "app_adc_conf.throttle_exp_mode", 0);
    }
}

void PageAppAdc::reloadParams()
{
    if (mOpenroad) {
        ui->generalTab->clearParams();
        ui->mappingTab->clearParams();
        ui->throttleCurveTab->clearParams();

        ui->generalTab->addParamSubgroup(mOpenroad->appConfig(), "adc", "general");
        ui->mappingTab->addParamSubgroup(mOpenroad->appConfig(), "adc", "mapping");
        ui->throttleCurveTab->addParamSubgroup(mOpenroad->appConfig(), "adc", "throttle curve");
    }
}

void PageAppAdc::paramChangedDouble(QObject *src, QString name, double newParam)
{
    (void)src;
    (void)newParam;

    if (name == "app_adc_conf.throttle_exp" || name == "app_adc_conf.throttle_exp_brake") {
        int mode = mOpenroad->appConfig()->getParamEnum("app_adc_conf.throttle_exp_mode");
        float val_acc = mOpenroad->appConfig()->getParamDouble("app_adc_conf.throttle_exp");
        float val_brake = mOpenroad->appConfig()->getParamDouble("app_adc_conf.throttle_exp_brake");

        QVector<double> x;
        QVector<double> y;
        for (float i = -1.0;i < 1.0001;i += 0.002) {
            x.append(i);
            double val = Utility::throttle_curve(i, val_acc, val_brake, mode);
            y.append(val);
        }
        ui->throttlePlot->graph()->setData(x, y);
        ui->throttlePlot->rescaleAxes();
        ui->throttlePlot->replot();
    }
}

void PageAppAdc::paramChangedEnum(QObject *src, QString name, int newParam)
{
    (void)src;
    (void)newParam;

    if (name == "app_adc_conf.throttle_exp_mode") {
        paramChangedDouble(0, "app_adc_conf.throttle_exp", 0.0);
    }
}
