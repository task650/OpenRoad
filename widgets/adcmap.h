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

#ifndef ADCMAP_H
#define ADCMAP_H

#include <QWidget>
#include "openroadinterface.h"

namespace Ui {
class AdcMap;
}

class AdcMap : public QWidget
{
    Q_OBJECT

public:
    explicit AdcMap(QWidget *parent = 0);
    ~AdcMap();

    VescInterface *openroad() const;
    void setVesc(VescInterface *openroad);

private slots:
    void decodedAdcReceived(double value, double voltage, double value2, double voltage2);

    void on_controlTypeBox_currentIndexChanged(int index);
    void on_helpButton_clicked();
    void on_resetButton_clicked();
    void on_applyButton_clicked();

private:
    Ui::AdcMap *ui;
    VescInterface *mVesc;
    bool mResetDone;

};

#endif // ADCMAP_H
