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

#include "pageappsettings.h"
#include "ui_pageappsettings.h"
#include "setupwizardapp.h"

PageAppSettings::PageAppSettings(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::PageAppSettings)
{
    ui->setupUi(this);
    layout()->setContentsMargins(0, 0, 0, 0);
    mOpenroad = nullptr;
}

PageAppSettings::~PageAppSettings()
{
    delete ui;
}

OpenroadInterface *PageAppSettings::openroad() const
{
    return mOpenroad;
}

void PageAppSettings::setOpenroad(OpenroadInterface *openroad)
{
    mOpenroad = openroad;

    if (mOpenroad) {
        reloadParams();
    }
}

void PageAppSettings::reloadParams()
{
    if (mOpenroad) {
        ConfigParam *p = mOpenroad->infoConfig()->getParam("app_setting_description");
        if (p != nullptr) {
            ui->textEdit->setHtml(p->description);
        } else {
            ui->textEdit->setText("App Setting Description not found.");
        }
    }
}

void PageAppSettings::on_appWizardButton_clicked()
{
    if (mOpenroad) {
        SetupWizardApp w(mOpenroad, this);
        w.exec();
    }
}
