#include "QSYMessageCreator.h"
#include <QApplication>
#include <QSettings>
#include <QRadioButton>
#include <QButtonGroup>
#include <QObject>
#include <QMessageBox>
#include <QInputDialog>
#include <QCloseEvent>
#include <QTimer>
#include <QThread>
#include <QDialog>
#include <QPushButton>
#include <QLabel>
#include <QWidget>
#include <QLayout>
#include <QSpinBox>
#include "SettingsGroup.hpp"
#include "Configuration.hpp"
#include "ui_QSYMessageCreator.h"


QSYMessageCreator::QSYMessageCreator(QSettings * settings, Configuration const * configuration, QWidget *parent) :
  QWidget {parent},
  settings_ {settings},
  configuration_ {configuration},
  ui {new Ui::QSYMessageCreator}
{
  ui->setupUi(this);
  read_settings ();
  setWindowTitle ("QSYMessageCreator");
  QButtonGroup *modeButtonGroup = new QButtonGroup;
  modeButtonGroup->addButton(ui->radioButFM);
  modeButtonGroup->addButton(ui->radioButSSB);
  modeButtonGroup->addButton(ui->radioButCW);
  modeButtonGroup->addButton(ui->radioButFT8);
  modeButtonGroup->addButton(ui->radioButMSK);

  QButtonGroup *bandButtonGroup = new QButtonGroup;
  bandButtonGroup->addButton(ui->radioBut50);
  bandButtonGroup->addButton(ui->radioBut144);
  bandButtonGroup->addButton(ui->radioBut222);
  bandButtonGroup->addButton(ui->radioBut432);
  bandButtonGroup->addButton(ui->radioBut902);
  bandButtonGroup->addButton(ui->radioBut903);
  bandButtonGroup->addButton(ui->radioBut1296);
  bandButtonGroup->addButton(ui->radioBut2304);
  bandButtonGroup->addButton(ui->radioBut3400);
  bandButtonGroup->addButton(ui->radioBut5760);
  bandButtonGroup->addButton(ui->radioBut10368);
  bandButtonGroup->addButton(ui->radioBut24192);

#if (QT_VERSION < QT_VERSION_CHECK(5, 15, 0))
  connect(modeButtonGroup, QOverload<int, bool>::of(&QButtonGroup::buttonToggled), [=](int id, bool checked){
      qDebug() << "id" << id  << "toggled:" << checked;
      QString theBand = getBand();
      QString theMode = getMode(theBand, configuration_->region());
      WriteMessage(theBand, theMode);
  });

  connect(bandButtonGroup, QOverload<int, bool>::of(&QButtonGroup::buttonToggled), [=](int id, bool checked){
      qDebug() << "id" << id  << "toggled:" << checked;
      QString theBand = getBand();
      QString theMode = getMode(theBand, configuration_->region());
      WriteMessage(theBand, theMode);
  });
#else
  QObject::connect(modeButtonGroup, &QButtonGroup::idToggled, [&](int id, bool checked) {
      qDebug() << "Button" << id << "toggled:" << checked;
      QString theBand = getBand();
      QString theMode = getMode(theBand, configuration_->region());
      QSYMessageCreator::WriteMessage(theBand, theMode);
  });

  QObject::connect(bandButtonGroup, &QButtonGroup::idToggled, [&](int id, bool checked) {
      qDebug() << "Button" << id << "toggled:" << checked;
      QString theBand = getBand();
      QString theMode = getMode(theBand, configuration_->region());
      QSYMessageCreator::WriteMessage(theBand, theMode);
  });
#endif

  connect(ui->kHzBox, static_cast<void (QSpinBox::*)(int)>(&QSpinBox::valueChanged), this, &QSYMessageCreator::onkHzBoxValueChanged);

  setup(configuration_->region());
}

QSYMessageCreator::~QSYMessageCreator()
{
  delete ui;
}

void QSYMessageCreator::onkHzBoxValueChanged()  {
  QString theBand = getBand();
  QString theMode = getMode(theBand, configuration_->region());
  QString message = WriteMessage(theBand, theMode);
  WriteMessage(theBand, theMode);
}


void QSYMessageCreator::setup(int region) {
  if (region == 0) {
    QMessageBox *regionWarning = new QMessageBox(this);
    regionWarning->setModal(false);
    regionWarning->setIcon(QMessageBox::Warning);
    regionWarning->setText("You need to enter your IARU Region\nin Settings or FM frequencies\nwill not be shown!");
    regionWarning->setWindowFlags(regionWarning->windowFlags() | Qt::WindowStaysOnTopHint);
    regionWarning->show();
  }
}

void QSYMessageCreator::on_button1_clicked()
{
  QString theBand = getBand();
  QString theMode = getMode(theBand, configuration_->region());
  QString message = WriteMessage(theBand, theMode);
  Q_EMIT sendMessage(message);
}

void QSYMessageCreator::setQSYMessageCreatorStatusFalse()
{
  Q_EMIT sendQSYMessageCreatorStatus(false);
}

void QSYMessageCreator::closeEvent (QCloseEvent * e)
{
  write_settings ();
  setQSYMessageCreatorStatusFalse();
  e->accept();                 // was ignore
}


void QSYMessageCreator::read_settings ()
{
  SettingsGroup g (settings_, "QSYMessageCreator");
  move (settings_->value ("window/pos", pos ()).toPoint ());
  ui->kHzBox->setValue(settings_->value("kHz").toInt());
  setBand(settings_->value("band").toString());
  setMode(settings_->value("band").toString(), settings_->value("mode").toString(),configuration_->region());
  WriteMessage(settings_->value("band").toString(), settings_->value("mode").toString());
}

void QSYMessageCreator::write_settings ()
{
  SettingsGroup g (settings_, "QSYMessageCreator");
  settings_->setValue ("window/pos", pos ());
  settings_->setValue ("band", getBand());
  settings_->setValue ("mode", getMode(getBand(),configuration_->region()));
  settings_->setValue ("kHz", ui->kHzBox->value());
}

QString QSYMessageCreator::WriteMessage (QString band, QString mode)
{
  qint16 kHzFreq = ui->kHzBox->value();
  QString kHzStr = QStringLiteral("%1").arg(kHzFreq,3,10,QLatin1Char('0'));
  QString message = "$DX " + band + mode + kHzStr;
  ui->messageLabel->setText(message);
  return message;
}


QString QSYMessageCreator::getBand()
{
  QString band;
  if (ui->radioBut50->isChecked()) band = "A";
  else if (ui->radioBut144->isChecked()) band = "B";
  else if (ui->radioBut222->isChecked()) band = "C";
  else if (ui->radioBut432->isChecked()) band = "D";
  else if (ui->radioBut902->isChecked()) band = "92";
  else if (ui->radioBut903->isChecked()) band = "93";
  else if (ui->radioBut1296->isChecked()) band = "E";
  else if (ui->radioBut2304->isChecked()) band = "F";
  else if (ui->radioBut3400->isChecked()) band = "G";
  else if (ui->radioBut5760->isChecked()) band = "H";
  else if (ui->radioBut10368->isChecked()) band = "I";
  else if (ui->radioBut24192->isChecked()) band = "J";
  return band;
}


void QSYMessageCreator::setBand(QString band)
{
  if (band == "A") ui->radioBut50->setChecked(true);
  else if (band == "B") ui->radioBut144->setChecked(true);
  else if (band == "C") ui->radioBut222->setChecked(true);
  else if (band == "D") ui->radioBut432->setChecked(true);
  else if (band == "92") ui->radioBut902->setChecked(true);
  else if (band == "93") ui->radioBut903->setChecked(true);
  else if (band == "E") ui->radioBut1296->setChecked(true);
  else if (band == "F") ui->radioBut2304->setChecked(true);
  else if (band == "G") ui->radioBut3400->setChecked(true);
  else if (band == "H") ui->radioBut5760->setChecked(true);
  else if (band == "I") ui->radioBut10368->setChecked(true);
  else if (band == "J") ui->radioBut24192->setChecked(true);
}

void QSYMessageCreator::setMode(QString band, QString mode, int region)
{
  QString MHz;
  if (mode=="V") {
    ui->radioButSSB->setChecked(true);
    ui->messageLabel_FM->setHidden(true);
    ui->label_2_FM->setHidden(true);
  }
  else if (mode=="W") {
    ui->radioButCW->setChecked(true);
    ui->messageLabel_FM->setHidden(true);
    ui->label_2_FM->setHidden(true);
  }
  else if (mode=="4") {
    ui->radioButMSK->setChecked(true);
    ui->messageLabel_FM->setHidden(true);
    ui->label_2_FM->setHidden(true);
  }
  else if (mode=="8") {
    ui->radioButFT8->setChecked(true);
    ui->messageLabel_FM->setHidden(true);
    ui->label_2_FM->setHidden(true);
  }
  else if (mode=="M") {
    ui->radioButFM->setChecked(true);
    if (band == "A") {
      if (region==2) {
        MHz = "52";
      } else {
        MHz = "51";
      }
    }
    else if (band == "B") {
      if (region==2) {
        MHz = "146";
      } else {
        MHz = "145";
      }
    }
    else if (band == "C") {
      if (region==2) {
        MHz = "223";
      } else {
        MHz = "";
      }
    }
    else if (band == "D") {
      if (region==2) {
        MHz = "446";
      } else {
        MHz = "433";
      }
    }
    if ((band =="A" || band == "B" || band == "C" || band == "D") && configuration_->region () > 0) {
      ui->label_2_FM->setText("Region " + QString::number(configuration_->region()) + " MHz");
      ui->messageLabel_FM->setHidden(false);
      ui->label_2_FM->setHidden(false);
      ui->messageLabel_FM->setText(MHz);
      if (region !=2 &&(band=="C")) {
        ui->messageLabel_FM->setHidden(true);
        ui->label_2_FM->setHidden(true);
      }
    } else {
      ui->messageLabel_FM->setHidden(true);
      ui->label_2_FM->setHidden(true);
    }
  }
}

QString QSYMessageCreator::getMode(QString band,int region)
{
  QString mode;
  QString  MHz;
  if (ui->radioButSSB->isChecked()) {
    mode = "V";
  }
  else if (ui->radioButFM->isChecked()) {
    mode = "M";
    if(band == "A") {
      if (region==2) {
        MHz = "52";
      } else {
        MHz = "51";
      }
    }
    else if (band == "B") {
      if(region==2) {
        MHz = "146";
      } else {
        MHz = "145";
      }
    }
    else if (band == "C") {
      if (region==2) {
        MHz = "223";
      } else {
        MHz = "";
      }
    }
    else if (band == "D") {
      if (region==2) {
        MHz = "446";
      } else {
        MHz = "433";
      }
    }
  }
  else if (ui->radioButCW->isChecked()) {
    mode = "W";
  }
  else if (ui->radioButFT8->isChecked()) {
    mode = "8";
  }
  else if (ui->radioButMSK->isChecked()) {
    mode = "4";
  }
  if ((band =="A" || band == "B" || band == "C" || band == "D") && (mode =="M") && configuration_->region () > 0) {
    ui->label_2_FM->setText("Region " + QString::number(configuration_->region()) + " MHz");
    ui->messageLabel_FM->setHidden(false);
    ui->label_2_FM->setHidden(false);
    ui->messageLabel_FM->setText(MHz);
    if(region !=2 &&(band=="C"))  {
      ui->messageLabel_FM->setHidden(true);
      ui->label_2_FM->setHidden(true);
    }
  } else {
    ui->messageLabel_FM->setHidden(true);
    ui->label_2_FM->setHidden(true);
  }
  return mode;
}
