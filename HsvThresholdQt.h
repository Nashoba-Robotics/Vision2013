#ifndef __HSV_THRESHOLD_QT_H__
#define __HSV_THRESHOLD_QT_H__

#include "HsvThreshold.h"
#include <QObject>
#include <QSlider>
#include <QGridLayout>
#include <QLabel>

class HsvThresholdQt : public QObject, public HsvThreshold {
  Q_OBJECT
    public:
    QSlider *qSliderHueLow;
    QSlider *qSliderHueHigh;
    QSlider *qSliderHueLow2;
    QSlider *qSliderHueHigh2;
    QSlider *qSliderSatLow;
    QSlider *qSliderSatHigh;
    QSlider *qSliderValueLow;
    QSlider *qSliderValueHigh;

    QLabel *qLabelHueLow;
    QLabel *qLabelHueHigh;
    QLabel *qLabelHueLow2;
    QLabel *qLabelHueHigh2;
    QLabel *qLabelSatLow;
    QLabel *qLabelSatHigh;
    QLabel *qLabelValueLow;
    QLabel *qLabelValueHigh;
 public:
  HsvThresholdQt(bool twohuesIn, int huelowIn, int huehighIn, int huelowIn2, int huehighIn2,
		 int satlowIn, int sathighIn, int valuelowIn, int valuehighIn) :
    HsvThreshold(twohuesIn, huelowIn, huehighIn, huelowIn2, huehighIn2,
		 satlowIn, sathighIn, valuelowIn, valuehighIn),
    qSliderHueLow(0), qSliderHueHigh(0), qSliderHueLow2(0), qSliderHueHigh2(0),
    qSliderSatLow(0), qSliderSatHigh(0), qSliderValueLow(0), qSliderValueHigh(0) {
   qSliderHueLow = new QSlider(Qt::Horizontal);
   qSliderHueHigh = new QSlider(Qt::Horizontal);
   if (params.twohues) {
     qSliderHueLow2 = new QSlider(Qt::Horizontal);
     qSliderHueHigh2 = new QSlider(Qt::Horizontal);
   }
   qSliderSatLow = new QSlider(Qt::Horizontal);
   qSliderSatHigh = new QSlider(Qt::Horizontal);
   qSliderValueLow = new QSlider(Qt::Horizontal);
   qSliderValueHigh = new QSlider(Qt::Horizontal);

   qSliderHueLow->setRange(0,255);
   qSliderHueHigh->setRange(0,255);
   if (params.twohues) {
     qSliderHueLow2->setRange(0,255);
     qSliderHueHigh2->setRange(0,255);
   }
   qSliderSatLow->setRange(0,255);
   qSliderSatHigh->setRange(0,255);
   qSliderValueLow->setRange(0,255);
   qSliderValueHigh->setRange(0,255);

   qSliderHueLow->setValue(params.huelow);
   qSliderHueHigh->setValue(params.huehigh);
   if (params.twohues) {
     qSliderHueLow2->setValue(params.huelow2);
     qSliderHueHigh2->setValue(params.huehigh2);
   }
   qSliderSatLow->setValue(params.satlow);
   qSliderSatHigh->setValue(params.sathigh);
   qSliderValueLow->setValue(params.valuelow);
   qSliderValueHigh->setValue(params.valuehigh);
   
   
   qLabelHueLow = new QLabel("Hue Low");
   qLabelHueHigh = new QLabel("Hue High");
   if (params.twohues) {
     qLabelHueLow2 = new QLabel("Hue Low 2");
     qLabelHueHigh2 = new QLabel("Hue High 2");
   }
   qLabelSatLow = new QLabel("Saturation Low");
   qLabelSatHigh = new QLabel("Saturation High");
   qLabelValueLow = new QLabel("Value Low");
   qLabelValueHigh = new QLabel("Value High");
   
   QObject::connect(qSliderHueLow, SIGNAL(valueChanged(int)),
		    this, SLOT(hueLowSetValue(int)));
   QObject::connect(qSliderHueHigh, SIGNAL(valueChanged(int)),
		    this, SLOT(hueHighSetValue(int)));
   if (params.twohues) {
     QObject::connect(qSliderHueLow2, SIGNAL(valueChanged(int)),
		      this, SLOT(hueLowSetValue2(int)));
     QObject::connect(qSliderHueHigh2, SIGNAL(valueChanged(int)),
		      this, SLOT(hueHighSetValue2(int)));
   }
   QObject::connect(qSliderSatLow, SIGNAL(valueChanged(int)),
		    this, SLOT(satLowSetValue(int)));
   QObject::connect(qSliderSatHigh, SIGNAL(valueChanged(int)),
		    this, SLOT(satHighSetValue(int)));
   QObject::connect(qSliderValueLow, SIGNAL(valueChanged(int)),
		    this, SLOT(valueLowSetValue(int)));
   QObject::connect(qSliderValueHigh, SIGNAL(valueChanged(int)),
		    this, SLOT(valueHighSetValue(int)));
 }
  void showGui(QGridLayout *gridLayout) {
    int i = 0;
    gridLayout->addWidget(qLabelHueLow, i++, 0);
    gridLayout->addWidget(qLabelHueHigh, i++, 0);
    if (params.twohues) {
      gridLayout->addWidget(qLabelHueLow2, i++, 0);
      gridLayout->addWidget(qLabelHueHigh2, i++, 0);
    }
    gridLayout->addWidget(qLabelSatLow, i++, 0);
    gridLayout->addWidget(qLabelSatHigh, i++, 0);
    gridLayout->addWidget(qLabelValueLow, i++, 0);
    gridLayout->addWidget(qLabelValueHigh, i++, 0);

    i = 0;
    gridLayout->addWidget(qSliderHueLow, i++, 1);
    gridLayout->addWidget(qSliderHueHigh, i++, 1);
    if (params.twohues) {
      gridLayout->addWidget(qSliderHueLow2, i++, 1);
      gridLayout->addWidget(qSliderHueHigh2, i++, 1);
    }
    gridLayout->addWidget(qSliderSatLow, i++, 1);
    gridLayout->addWidget(qSliderSatHigh, i++, 1);
    gridLayout->addWidget(qSliderValueLow, i++, 1);
    gridLayout->addWidget(qSliderValueHigh, i++, 1);

    qLabelHueLow->setVisible(true);
    qLabelHueHigh->setVisible(true);
    if (params.twohues) {
      qLabelHueLow2->setVisible(true);
      qLabelHueHigh2->setVisible(true);
    }
    qLabelSatLow->setVisible(true);
    qLabelSatHigh->setVisible(true);
    qLabelValueLow->setVisible(true);
    qLabelValueHigh->setVisible(true);

    qSliderHueLow->setVisible(true);
    qSliderHueHigh->setVisible(true);
    if (params.twohues) {
      qSliderHueLow2->setVisible(true);
      qSliderHueHigh2->setVisible(true);
    }
    qSliderSatLow->setVisible(true);
    qSliderSatHigh->setVisible(true);
    qSliderValueLow->setVisible(true);
    qSliderValueHigh->setVisible(true);
  }
  void hideGui(QGridLayout *gridLayout) {
    gridLayout->removeWidget(qLabelHueLow);
    gridLayout->removeWidget(qLabelHueHigh);
    if (params.twohues) {
      gridLayout->removeWidget(qLabelHueLow2);
      gridLayout->removeWidget(qLabelHueHigh2);
    }
    gridLayout->removeWidget(qLabelSatLow);
    gridLayout->removeWidget(qLabelSatHigh);
    gridLayout->removeWidget(qLabelValueLow);
    gridLayout->removeWidget(qLabelValueHigh);

    gridLayout->removeWidget(qSliderHueLow);
    gridLayout->removeWidget(qSliderHueHigh);
    if (params.twohues) {
      gridLayout->removeWidget(qSliderHueLow2);
      gridLayout->removeWidget(qSliderHueHigh2);
    }
    gridLayout->removeWidget(qSliderSatLow);
    gridLayout->removeWidget(qSliderSatHigh);
    gridLayout->removeWidget(qSliderValueLow);
    gridLayout->removeWidget(qSliderValueHigh);

    qLabelHueLow->setVisible(false);
    qLabelHueHigh->setVisible(false);
    if (params.twohues) {
      qLabelHueLow2->setVisible(false);
      qLabelHueHigh2->setVisible(false);
    }
    qLabelSatLow->setVisible(false);
    qLabelSatHigh->setVisible(false);
    qLabelValueLow->setVisible(false);
    qLabelValueHigh->setVisible(false);

    qSliderHueLow->setVisible(false);
    qSliderHueHigh->setVisible(false);
    if (params.twohues) {
      qSliderHueLow2->setVisible(false);
      qSliderHueHigh2->setVisible(false);
    }
    qSliderSatLow->setVisible(false);
    qSliderSatHigh->setVisible(false);
    qSliderValueLow->setVisible(false);
    qSliderValueHigh->setVisible(false);

  }
 public slots:
    void hueLowSetValue(int value) {
      params.huelow = value;
    }
    void hueHighSetValue(int value) {
      params.huehigh = value;
    }
    void hueLowSetValue2(int value) {
      params.huelow2 = value;
    }
    void hueHighSetValue2(int value) {
      params.huehigh2 = value;
    }
    void satLowSetValue(int value) {
      params.satlow = value;
    }
    void satHighSetValue(int value) {
      params.sathigh = value;
    }
    void valueLowSetValue(int value) {
      params.valuelow = value;
    }
    void valueHighSetValue(int value) {
      params.valuehigh = value;
    }

};


#endif
