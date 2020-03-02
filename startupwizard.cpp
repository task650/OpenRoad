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

#include "startupwizard.h"
#include <QVBoxLayout>
#include <QMessageBox>
#include <QScrollBar>
#include <QDesktopServices>

StartupWizard::StartupWizard(OpenroadInterface *openroad, QWidget *parent)
    : QWizard(parent)
{
    QSettings().setValue("intro_done", false);

    setPage(Page_Intro, new StartupIntroPage(openroad));
    setPage(Page_Usage, new StartupUsagePage(openroad));
    setPage(Page_Warranty, new StartupWarrantyPage(openroad));
    setPage(Page_Conclusion, new StartupConclusionPage(openroad));

    setStartId(Page_Intro);
    setWizardStyle(ModernStyle);
    setPixmap(QWizard::LogoPixmap, QPixmap("://res/icon.png").
              scaled(40, 40,
                     Qt::KeepAspectRatio,
                     Qt::SmoothTransformation));
    resize(800, 450);

    setWindowTitle(tr("VESC Tool Introduction"));

    mSideLabel = new AspectImgLabel(Qt::Vertical);
    mSideLabel->setPixmap(QPixmap("://res/logo_intro_wizard.png"));
    mSideLabel->setScaledContents(true);
    setSideWidget(mSideLabel);

    connect(this, SIGNAL(currentIdChanged(int)),
            this, SLOT(idChanged(int)));
}

void StartupWizard::idChanged(int id)
{
    if (id == Page_Intro || id == Page_Conclusion) {
        setSideWidget(mSideLabel);
        mSideLabel->setVisible(true);
    } else {
        setSideWidget(0);
    }
}

StartupIntroPage::StartupIntroPage(OpenroadInterface *openroad, QWidget *parent)
    : QWizardPage(parent)
{
    mBrowser = new VTextBrowser;
    mBrowser->setFrameStyle(QFrame::NoFrame);
    mBrowser->viewport()->setAutoFillBackground(false);

    ConfigParam *p = openroad->infoConfig()->getParam("wizard_startup_intro");
    if (p != 0) {
        setTitle(p->longName);
        mBrowser->setHtml(p->description);
    } else {
        setTitle("Text not found.");
        mBrowser->setText("Text not found.");
    }

    QVBoxLayout *layout = new QVBoxLayout;
    layout->addWidget(mBrowser);
    setLayout(layout);
}

int StartupIntroPage::nextId() const
{
    return StartupWizard::Page_Usage;
}

StartupUsagePage::StartupUsagePage(OpenroadInterface *openroad, QWidget *parent)
    : QWizardPage(parent)
{
    mBrowser = new VTextBrowser;
    mAcceptBox = new QCheckBox("Yes, I understand and accept");

    ConfigParam *p = openroad->infoConfig()->getParam("wizard_startup_usage");
    if (p != 0) {
        setTitle(p->longName);
        setSubTitle(p->valString);
        mBrowser->setHtml(p->description);
    } else {
        setTitle("Text not found.");
        setSubTitle("Text not found");
        mBrowser->setText("Text not found.");
    }

    registerField("usageAccept*", mAcceptBox);

    connect(mBrowser->verticalScrollBar(), SIGNAL(valueChanged(int)),
            this, SLOT(scrollValueChanged(int)));
    connect(mBrowser->verticalScrollBar(), SIGNAL(rangeChanged(int,int)),
            this, SLOT(scrollRangeChanged()));

    QVBoxLayout *layout = new QVBoxLayout;
    layout->addWidget(mBrowser);
    layout->addWidget(mAcceptBox);
    setLayout(layout);
}

int StartupUsagePage::nextId() const
{
    return StartupWizard::Page_Warranty;
}

void StartupUsagePage::scrollValueChanged(int value)
{
    if (value == mBrowser->verticalScrollBar()->maximum()) {
        mAcceptBox->setEnabled(true);
    } else {
        mAcceptBox->setEnabled(false);
        mAcceptBox->setChecked(false);
    }
}

void StartupUsagePage::scrollRangeChanged()
{
    scrollValueChanged(mBrowser->verticalScrollBar()->value());
}

StartupWarrantyPage::StartupWarrantyPage(OpenroadInterface *openroad, QWidget *parent)
    : QWizardPage(parent)
{
    mBrowser = new VTextBrowser;
    mAcceptBox = new QCheckBox("Yes, I understand and accept");

    ConfigParam *p = openroad->infoConfig()->getParam("wizard_startup_warranty");
    if (p != 0) {
        setTitle(p->longName);
        setSubTitle(p->valString);
        mBrowser->setHtml(p->description);
    } else {
        setTitle("Text not found.");
        setSubTitle("Text not found");
        mBrowser->setText("Text not found.");
    }

    registerField("warrantyAccept*", mAcceptBox);

    connect(mBrowser->verticalScrollBar(), SIGNAL(valueChanged(int)),
            this, SLOT(scrollValueChanged(int)));
    connect(mBrowser->verticalScrollBar(), SIGNAL(rangeChanged(int,int)),
            this, SLOT(scrollRangeChanged()));

    QVBoxLayout *layout = new QVBoxLayout;
    layout->addWidget(mBrowser);
    layout->addWidget(mAcceptBox);
    setLayout(layout);
}

int StartupWarrantyPage::nextId() const
{
    return StartupWizard::Page_Conclusion;
}

void StartupWarrantyPage::scrollValueChanged(int value)
{
    if (value == mBrowser->verticalScrollBar()->maximum()) {
        mAcceptBox->setEnabled(true);
    } else {
        mAcceptBox->setEnabled(false);
        mAcceptBox->setChecked(false);
    }
}

void StartupWarrantyPage::scrollRangeChanged()
{
    scrollValueChanged(mBrowser->verticalScrollBar()->value());
}

StartupConclusionPage::StartupConclusionPage(OpenroadInterface *openroad, QWidget *parent)
    : QWizardPage(parent)
{
    mBrowser = new VTextBrowser;
    mBrowser->setFrameStyle(QFrame::NoFrame);
    mBrowser->viewport()->setAutoFillBackground(false);

    ConfigParam *p = openroad->infoConfig()->getParam("wizard_startup_conclusion");
    if (p != 0) {
        setTitle(p->longName);
        mBrowser->setHtml(p->description);
    } else {
        setTitle("Text not found.");
        mBrowser->setText("Text not found.");
    }

    QVBoxLayout *layout = new QVBoxLayout;
    layout->addWidget(mBrowser);
    setLayout(layout);
}

int StartupConclusionPage::nextId() const
{
    return -1;
}

bool StartupConclusionPage::validatePage()
{
    QSettings().setValue("intro_done", true);
    return true;
}
