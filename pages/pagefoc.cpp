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

#include "pagefoc.h"
#include "ui_pagefoc.h"

PageFoc::PageFoc(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::PageFoc)
{
    ui->setupUi(this);
    layout()->setContentsMargins(0, 0, 0, 0);
    mOpenroad = 0;
}

PageFoc::~PageFoc()
{
    delete ui;
}

OpenroadInterface *PageFoc::openroad() const
{
    return mOpenroad;
}

void PageFoc::setOpenroad(OpenroadInterface *openroad)
{
    mOpenroad = openroad;

    if (mOpenroad) {
        ui->detectFoc->setOpenroad(mOpenroad);
        ui->detectFocHall->setOpenroad(mOpenroad);
        ui->detectFocEncoder->setOpenroad(mOpenroad);

        reloadParams();
    }
}

void PageFoc::reloadParams()
{
    if (mOpenroad) {
        ui->generalTab->clearParams();
        ui->sensorlessTab->clearParams();
        ui->hallTab->clearParams();
        ui->encoderTab->clearParams();
        ui->hfiTab->clearParams();
        ui->advancedTab->clearParams();

        ui->generalTab->addParamSubgroup(mOpenroad->mcConfig(), "foc", "general");
        ui->sensorlessTab->addParamSubgroup(mOpenroad->mcConfig(), "foc", "sensorless");
        ui->hallTab->addParamSubgroup(mOpenroad->mcConfig(), "foc", "hall sensors");
        ui->encoderTab->addParamSubgroup(mOpenroad->mcConfig(), "foc", "encoder");
        ui->hfiTab->addParamSubgroup(mOpenroad->mcConfig(), "foc", "hfi");
        ui->advancedTab->addParamSubgroup(mOpenroad->mcConfig(), "foc", "advanced");
    }
}
