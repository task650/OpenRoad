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

#include "pagewelcome.h"
#include "ui_pagewelcome.h"
#include "setupwizardmotor.h"
#include "setupwizardapp.h"
#include "utility.h"
#include "widgets/detectallfocdialog.h"
#include <QMessageBox>

PageWelcome::PageWelcome(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::PageWelcome)
{
    ui->setupUi(this);
    layout()->setContentsMargins(0, 0, 0, 0);
    mOpenroad = 0;
    ui->bgWidget->setPixmap(QPixmap("://res/bg.png"));

    connect(ui->wizardFocSimpleButton, SIGNAL(clicked(bool)),
            this, SLOT(startSetupWizardFocSimple()));
    connect(ui->wizardAppButton, SIGNAL(clicked(bool)),
            this, SLOT(startSetupWizardApp()));
}

PageWelcome::~PageWelcome()
{
    delete ui;
}

void PageWelcome::startSetupWizardFocSimple()
{
    if (mOpenroad) {
        DetectAllFocDialog::showDialog(mOpenroad, this);
    }
}

void PageWelcome::startSetupWizardMotor()
{
    if (mOpenroad) {
        SetupWizardMotor w(mOpenroad, this);
        w.exec();
    }
}

void PageWelcome::startSetupWizardApp()
{
    if (mOpenroad) {
        SetupWizardApp w(mOpenroad, this);
        w.exec();
    }
}

OpenroadInterface *PageWelcome::openroad() const
{
    return mOpenroad;
}

void PageWelcome::setOpenroad(OpenroadInterface *openroad)
{
    mOpenroad = openroad;
}

void PageWelcome::on_autoConnectButton_clicked()
{
    Utility::autoconnectBlockingWithProgress(mOpenroad, this);
}
