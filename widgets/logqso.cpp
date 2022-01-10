#include "logqso.h"

#include <QLocale>
#include <QString>
#include <QSettings>
#include <QStandardPaths>
#include <QDir>

#include "logbook/logbook.h"
#include "MessageBox.hpp"
#include "Configuration.hpp"
#include "models/Bands.hpp"
#include "models/CabrilloLog.hpp"
#include "validators/MaidenheadLocatorValidator.hpp"

#include "ui_logqso.h"
#include "moc_logqso.cpp"

namespace
{
  struct PropMode
  {
    char const * id_;
    char const * name_;
  };
  constexpr PropMode prop_modes[] =
    {
     {"", ""}
     , {"AS", QT_TRANSLATE_NOOP ("LogQSO", "Aircraft scatter")}
     , {"AUE", QT_TRANSLATE_NOOP ("LogQSO", "Aurora-E")}
     , {"AUR", QT_TRANSLATE_NOOP ("LogQSO", "Aurora")}
     , {"BS", QT_TRANSLATE_NOOP ("LogQSO", "Back scatter")}
     , {"ECH", QT_TRANSLATE_NOOP ("LogQSO", "Echolink")}
     , {"EME", QT_TRANSLATE_NOOP ("LogQSO", "Earth-moon-earth")}
     , {"ES", QT_TRANSLATE_NOOP ("LogQSO", "Sporadic E")}
     , {"F2", QT_TRANSLATE_NOOP ("LogQSO", "F2 Reflection")}
     , {"FAI", QT_TRANSLATE_NOOP ("LogQSO", "Field aligned irregularities")}
     , {"INTERNET", QT_TRANSLATE_NOOP ("LogQSO", "Internet-assisted")}
     , {"ION", QT_TRANSLATE_NOOP ("LogQSO", "Ionoscatter")}
     , {"IRL", QT_TRANSLATE_NOOP ("LogQSO", "IRLP")}
     , {"MS", QT_TRANSLATE_NOOP ("LogQSO", "Meteor scatter")}
     , {"RPT", QT_TRANSLATE_NOOP ("LogQSO", "Non-satellite repeater or transponder")}
     , {"RS", QT_TRANSLATE_NOOP ("LogQSO", "Rain scatter")}
     , {"SAT", QT_TRANSLATE_NOOP ("LogQSO", "Satellite")}
     , {"TEP", QT_TRANSLATE_NOOP ("LogQSO", "Trans-equatorial")}
     , {"TR", QT_TRANSLATE_NOOP ("LogQSO", "Troposheric ducting")}
    };

  struct Satellite
  {
    char const * id_;
    char const * name_;
  };
  constexpr Satellite satellite[] =
    {
     {"", ""}
      , {"AISAT1", QT_TRANSLATE_NOOP ("LogQSO", "AISAT-1 AMSAT India APRS DigipeaterA")}
      , {"AO-10", QT_TRANSLATE_NOOP ("LogQSO", "AMSAT-OSCAR 10")}
      , {"AO-109", QT_TRANSLATE_NOOP ("LogQSO", "AMSAT-OSCAR 109")}
      , {"AO-13", QT_TRANSLATE_NOOP ("LogQSO", "AMSAT-OSCAR 13")}
      , {"AO-16", QT_TRANSLATE_NOOP ("LogQSO", "AMSAT-OSCAR 16")}
      , {"AO-21", QT_TRANSLATE_NOOP ("LogQSO", "OSCAR 21/RS-14")}
      , {"AO-27", QT_TRANSLATE_NOOP ("LogQSO", "AMRAD-OSCAR 27")}
      , {"AO-3", QT_TRANSLATE_NOOP ("LogQSO", "AMSAT-OSCAR 3")}
      , {"AO-4", QT_TRANSLATE_NOOP ("LogQSO", "AMSAT-OSCAR 4")}
      , {"AO-40", QT_TRANSLATE_NOOP ("LogQSO", "AMSAT-OSCAR 40")}
      , {"AO-51", QT_TRANSLATE_NOOP ("LogQSO", "AMSAT-OSCAR 51")}
      , {"AO-6", QT_TRANSLATE_NOOP ("LogQSO", "AMSAT-OSCAR 6")}
      , {"AO-7", QT_TRANSLATE_NOOP ("LogQSO", "AMSAT-OSCAR 7")}
      , {"AO-73", QT_TRANSLATE_NOOP ("LogQSO", "AMSAT-OSCAR 73")}
      , {"AO-8", QT_TRANSLATE_NOOP ("LogQSO", "AMSAT-OSCAR 8")}
      , {"AO-85", QT_TRANSLATE_NOOP ("LogQSO", "AMSAT-OSCAR 85 (Fox-1A)")}
      , {"AO-91", QT_TRANSLATE_NOOP ("LogQSO", "AMSAT-OSCAR 91 (RadFxSat / Fox-1B)")}
      , {"AO-92", QT_TRANSLATE_NOOP ("LogQSO", "AMSAT-OSCAR 92 (Fox-1D)")}
      , {"ARISS", QT_TRANSLATE_NOOP ("LogQSO", "ARISS")}
      , {"Arsene", QT_TRANSLATE_NOOP ("LogQSO", "OSCAR 24")}
      , {"BO-102", QT_TRANSLATE_NOOP ("LogQSO", "BIT Progress-OSCAR 102 (CAS-7B)")}
      , {"BY70-1", QT_TRANSLATE_NOOP ("LogQSO", "Bayi Kepu Weixing 1")}
      , {"CAS-3H", QT_TRANSLATE_NOOP ("LogQSO", "LilacSat-2")}
      , {"CAS-4A", QT_TRANSLATE_NOOP ("LogQSO", "CAMSAT 4A (CAS-4A)")}
      , {"CAS-4B", QT_TRANSLATE_NOOP ("LogQSO", "CAMSAT 4B (CAS-4B)")}
      , {"DO-64", QT_TRANSLATE_NOOP ("LogQSO", "Delfi OSCAR-64")}
      , {"EO-79", QT_TRANSLATE_NOOP ("LogQSO", "FUNcube-3")}
      , {"EO-88", QT_TRANSLATE_NOOP ("LogQSO", "Emirates-OSCAR 88 (Nayif-1)")}
      , {"FO-12", QT_TRANSLATE_NOOP ("LogQSO", "Fuji-OSCAR 12")}
      , {"FO-20", QT_TRANSLATE_NOOP ("LogQSO", "Fuji-OSCAR 20")}
      , {"FO-29", QT_TRANSLATE_NOOP ("LogQSO", "Fuji-OSCAR 29")}
      , {"FO-99", QT_TRANSLATE_NOOP ("LogQSO", "Fuji-OSCAR 99 (NEXUS)")}
      , {"FS-3", QT_TRANSLATE_NOOP ("LogQSO", "FalconSAT 3")}
      , {"HO-107", QT_TRANSLATE_NOOP ("LogQSO", "HuskySat OSCAR 107")}
      , {"HO-113", QT_TRANSLATE_NOOP ("LogQSO", "HO-113")}
      , {"HO-68", QT_TRANSLATE_NOOP ("LogQSO", "Hope-Oscar 68")}
      , {"IO-86", QT_TRANSLATE_NOOP ("LogQSO", "Indonesia-OSCAR 86 (LAPAN-ORARI)")}
      , {"JO-97", QT_TRANSLATE_NOOP ("LogQSO", "Jordan-OSCAR 97(JY1Sat)")}
      , {"KEDR", QT_TRANSLATE_NOOP ("LogQSO", "ARISSat-1")}
      , {"LO-19", QT_TRANSLATE_NOOP ("LogQSO", "Lusat-OSCAR 19")}
      , {"LO-78", QT_TRANSLATE_NOOP ("LogQSO", "LituanicaSAT-1")}
      , {"LO-87", QT_TRANSLATE_NOOP ("LogQSO", "LUSEX-OSCAR 87")}
      , {"LO-90", QT_TRANSLATE_NOOP ("LogQSO", "LilacSat-OSCAR 90 (LilacSat-1)")}
      , {"MAYA-3", QT_TRANSLATE_NOOP ("LogQSO", "Cubesat")}
      , {"MAYA-4", QT_TRANSLATE_NOOP ("LogQSO", "Cubesat")}
      , {"MIREX", QT_TRANSLATE_NOOP ("LogQSO", "MIR Packet Digipeater")}
      , {"NO-103", QT_TRANSLATE_NOOP ("LogQSO", "Navy-OSCAR 103 (BRICSAT 2)")}
      , {"NO-104", QT_TRANSLATE_NOOP ("LogQSO", "Navy-OSCAR 104 (PSAT 2)")}
      , {"NO-44", QT_TRANSLATE_NOOP ("LogQSO", "Navy-OSCAR 44")}
      , {"NO-83", QT_TRANSLATE_NOOP ("LogQSO", "BRICsat")}
      , {"NO-84", QT_TRANSLATE_NOOP ("LogQSO", "PSAT")}
      , {"PO-101", QT_TRANSLATE_NOOP ("LogQSO", "Phillipines-OSCAR-101 (Diwata-2)")}
      , {"QO-100", QT_TRANSLATE_NOOP ("LogQSO", "Qatar-OSCAR 100 (Es'hail-2/P4A)")}
      , {"RS-1", QT_TRANSLATE_NOOP ("LogQSO", "Radio Sputnik 1")}
      , {"RS-10", QT_TRANSLATE_NOOP ("LogQSO", "Radio Sputnik 10")}
      , {"RS-11", QT_TRANSLATE_NOOP ("LogQSO", "Radio Sputnik 11")}
      , {"RS-12", QT_TRANSLATE_NOOP ("LogQSO", "Radio Sputnik 12")}
      , {"RS-13", QT_TRANSLATE_NOOP ("LogQSO", "Radio Sputnik 13")}
      , {"RS-15", QT_TRANSLATE_NOOP ("LogQSO", "Radio Sputnik 15")}
      , {"RS-2", QT_TRANSLATE_NOOP ("LogQSO", "Radio Sputnik 2")}
      , {"RS-44", QT_TRANSLATE_NOOP ("LogQSO", "Radio Sputnik 44 (DOSAAF-85)")}
      , {"RS-5", QT_TRANSLATE_NOOP ("LogQSO", "Radio Sputnik 5")}
      , {"RS-6", QT_TRANSLATE_NOOP ("LogQSO", "Radio Sputnik 6")}
      , {"RS-7", QT_TRANSLATE_NOOP ("LogQSO", "Radio Sputnik 7")}
      , {"RS-8", QT_TRANSLATE_NOOP ("LogQSO", "Radio Sputnik 8")}
      , {"SAREX", QT_TRANSLATE_NOOP ("LogQSO", "Shuttle Amateur Radio Experiment (SAREX) Digipeater")}
      , {"SO-35", QT_TRANSLATE_NOOP ("LogQSO", "Sunsat-OSCAR 35")}
      , {"SO-41", QT_TRANSLATE_NOOP ("LogQSO", "Saudi-OSCAR 41")}
      , {"SO-50", QT_TRANSLATE_NOOP ("LogQSO", "Saudi-OSCAR 50")}
      , {"SO-67", QT_TRANSLATE_NOOP ("LogQSO", "Sumbandila Oscar 67")}
      , {"TAURUS", QT_TRANSLATE_NOOP ("LogQSO", "Taurus-1 (Jinniuzuo-1)")}
      , {"TO-108", QT_TRANSLATE_NOOP ("LogQSO", "TQ-OSCAR 108 (CAS-6 / TQ-1)")}
      , {"UKUBE1", QT_TRANSLATE_NOOP ("LogQSO", "UKube-1 (FUNcube-2)")}
      , {"UO-14", QT_TRANSLATE_NOOP ("LogQSO", "UOSAT-OSCAR 14")}
      , {"UVSQ", QT_TRANSLATE_NOOP ("LogQSO", "CubeSat")}
      , {"VO-52", QT_TRANSLATE_NOOP ("LogQSO", "VUsat-OSCAR 52")}
      , {"XW-2A", QT_TRANSLATE_NOOP ("LogQSO", "Hope 2A (CAS-3A)")}
      , {"XW-2B", QT_TRANSLATE_NOOP ("LogQSO", "Hope 2B (CAS-3B)")}
      , {"XW-2C", QT_TRANSLATE_NOOP ("LogQSO", "Hope 2C (CAS-3C)")}
    };
}


LogQSO::LogQSO(QString const& programTitle, QSettings * settings
               , Configuration const * config, LogBook * log, QWidget *parent)
  : QDialog {parent, Qt::WindowStaysOnTopHint | Qt::WindowTitleHint | Qt::WindowSystemMenuHint}
  , ui(new Ui::LogQSO)
  , m_settings (settings)
  , m_config {config}
  , m_log {log}
{
  ui->setupUi(this);
  setWindowTitle(programTitle + " - Log QSO");
  for (auto const& prop_mode : prop_modes)
    {
      ui->comboBoxPropMode->addItem (prop_mode.name_, prop_mode.id_);
    }
  for (auto const& satellite : satellite)
    {
      ui->comboBoxSatellite->addItem (satellite.name_, satellite.id_);
    }
  loadSettings ();
  connect (ui->comboBoxPropMode, &QComboBox::currentTextChanged, this, &LogQSO::propModeChanged);
  auto date_time_format = QLocale {}.dateFormat (QLocale::ShortFormat) + " hh:mm:ss";
  ui->start_date_time->setDisplayFormat (date_time_format);
  ui->end_date_time->setDisplayFormat (date_time_format);
  ui->grid->setValidator (new MaidenheadLocatorValidator {this});
}

LogQSO::~LogQSO ()
{
}

void LogQSO::loadSettings ()
{
  m_settings->beginGroup ("LogQSO");
  restoreGeometry (m_settings->value ("geometry", saveGeometry ()).toByteArray ());
  ui->cbTxPower->setChecked (m_settings->value ("SaveTxPower", false).toBool ());
  ui->cbComments->setChecked (m_settings->value ("SaveComments", false).toBool ());
  ui->cbPropMode->setChecked (m_settings->value ("SavePropMode", false).toBool ());
  ui->cbSatellite->setChecked (m_settings->value ("SaveSatellite", false).toBool ());
  m_txPower = m_settings->value ("TxPower", "").toString ();
  m_comments = m_settings->value ("LogComments", "").toString();
  int prop_index {0};
  if (ui->cbPropMode->isChecked ())
    {
      prop_index = ui->comboBoxPropMode->findData (m_settings->value ("PropMode", "").toString());
    }
  ui->comboBoxPropMode->setCurrentIndex (prop_index);
  int satellite {0};
  if (ui->cbSatellite->isChecked ())
    {
      satellite = ui->comboBoxSatellite->findData (m_settings->value ("Satellite", "").toString());
    }
  ui->comboBoxSatellite->setCurrentIndex (satellite);
  if (m_settings->value ("PropMode", "") != "SAT")
  {
      ui->cbSatellite->setDisabled(true);
      ui->comboBoxSatellite->setDisabled(true);
  }
  m_freqRx = m_settings->value ("FreqRx", "").toString ();
  ui->cbFreqRx->setChecked (m_settings->value ("SaveFreqRx", false).toBool ());
  m_settings->endGroup ();
}

void LogQSO::storeSettings () const
{
  m_settings->beginGroup ("LogQSO");
  m_settings->setValue ("geometry", saveGeometry ());
  m_settings->setValue ("SaveTxPower", ui->cbTxPower->isChecked ());
  m_settings->setValue ("SaveComments", ui->cbComments->isChecked ());
  m_settings->setValue ("SavePropMode", ui->cbPropMode->isChecked ());
  m_settings->setValue ("SaveSatellite", ui->cbSatellite->isChecked ());
  m_settings->setValue ("SaveFreqRx", ui->cbFreqRx->isChecked ());
  m_settings->setValue ("TxPower", m_txPower);
  m_settings->setValue ("LogComments", m_comments);
  m_settings->setValue ("PropMode", ui->comboBoxPropMode->currentData ());
  m_settings->setValue ("Satellite", ui->comboBoxSatellite->currentData ());
  m_settings->setValue ("FreqRx", m_freqRx);
  m_settings->endGroup ();
}

void LogQSO::initLogQSO(QString const& hisCall, QString const& hisGrid, QString mode,
                        QString const& rptSent, QString const& rptRcvd,
                        QDateTime const& dateTimeOn, QDateTime const& dateTimeOff,
                        Radio::Frequency dialFreq, bool noSuffix, QString xSent, QString xRcvd)
{
  if(!isHidden()) return;
  ui->call->setText (hisCall);
  ui->grid->setText (hisGrid);
  ui->name->clear ();
  if (ui->cbTxPower->isChecked ())
    {
      ui->txPower->setText (m_txPower);
    }
  else
    {
      ui->txPower->clear ();
    }
  if (ui->cbFreqRx->isChecked ())
    {
      ui->freqRx->setText (m_freqRx);
    }
  else
    {
      ui->freqRx->clear ();
    }
  if (ui->cbComments->isChecked ())
    {
      ui->comments->setText (m_comments);
    }
  else
    {
      ui->comments->clear ();
    }
  if (m_config->report_in_comments()) {
    auto t=mode;
    if(rptSent!="") t+="  Sent: " + rptSent;
    if(rptRcvd!="") t+="  Rcvd: " + rptRcvd;
    ui->comments->setText(t);
  }
  if(noSuffix and mode.mid(0,3)=="JT9") mode="JT9";
  if(m_config->log_as_RTTY() and mode.mid(0,3)=="JT9") mode="RTTY";
  ui->mode->setText(mode);
  ui->sent->setText(rptSent);
  ui->rcvd->setText(rptRcvd);
  ui->start_date_time->setDateTime (dateTimeOn);
  ui->end_date_time->setDateTime (dateTimeOff);
  m_dialFreq=dialFreq;
  m_myCall=m_config->my_callsign();
  m_myGrid=m_config->my_grid();
  ui->band->setText (m_config->bands ()->find (dialFreq));
  ui->loggedOperator->setText(m_config->opCall());
  ui->exchSent->setText (xSent);
  ui->exchRcvd->setText (xRcvd);
  if (!ui->cbPropMode->isChecked ())
    {
      ui->comboBoxPropMode->setCurrentIndex (-1);
    }
  if (!ui->cbSatellite->isChecked ())
    {
      ui->comboBoxSatellite->setCurrentIndex (-1);
    }

  using SpOp = Configuration::SpecialOperatingActivity;
  auto special_op = m_config->special_op_id ();
  if (SpOp::FOX == special_op
      || (m_config->autoLog ()
          && SpOp::NONE < special_op && special_op < SpOp::FOX))
    {
      // allow auto logging in Fox mode and contests
      accept();
    }
  else
    {
      show();
    }
}

void LogQSO::accept()
{
  auto hisCall = ui->call->text ();
  auto hisGrid = ui->grid->text ();
  auto mode = ui->mode->text ();
  auto rptSent = ui->sent->text ();
  auto rptRcvd = ui->rcvd->text ();
  auto dateTimeOn = ui->start_date_time->dateTime ();
  auto dateTimeOff = ui->end_date_time->dateTime ();
  auto band = ui->band->text ();
  auto name = ui->name->text ();
  m_txPower = ui->txPower->text ();
  m_comments = ui->comments->text ();
  auto strDialFreq = QString::number (m_dialFreq / 1.e6,'f',6);
  auto operator_call = ui->loggedOperator->text ();
  auto xsent = ui->exchSent->text ();
  auto xrcvd = ui->exchRcvd->text ();

  using SpOp = Configuration::SpecialOperatingActivity;
  auto special_op = m_config->special_op_id ();

  if (special_op == SpOp::NA_VHF or special_op == SpOp::WW_DIGI) {
    if(xrcvd!="" and hisGrid!=xrcvd) hisGrid=xrcvd;
  }

  if ((special_op == SpOp::RTTY and xsent!="" and xrcvd!="")) {
    if(rptSent=="" or !xsent.contains(rptSent+" ")) rptSent=xsent.split(" "
#if QT_VERSION < QT_VERSION_CHECK(5, 15, 0)
                                                                        , QString::SkipEmptyParts
#else
                                                                        , Qt::SkipEmptyParts
#endif
                                                                        ).at(0);
    if(rptRcvd=="" or !xrcvd.contains(rptRcvd+" ")) rptRcvd=xrcvd.split(" "
#if QT_VERSION < QT_VERSION_CHECK(5, 15, 0)
                                                                        , QString::SkipEmptyParts
#else
                                                                        , Qt::SkipEmptyParts
#endif
                                                                        ).at(0);
  }

  // validate
  if (SpOp::NONE < special_op && special_op < SpOp::FOX)
    {
      if (xsent.isEmpty () || xrcvd.isEmpty ())
        {
          show ();
          MessageBox::warning_message (this, tr ("Invalid QSO Data"),
                                       tr ("Check exchange sent and received"));
          return;               // without accepting
        }

      if (!m_log->contest_log ()->add_QSO (m_dialFreq, mode, dateTimeOff, hisCall, xsent, xrcvd))
        {
          show ();
          MessageBox::warning_message (this, tr ("Invalid QSO Data"),
                                       tr ("Check all fields"));
          return;               // without accepting
        }
    }

  auto const& prop_mode = ui->comboBoxPropMode->currentData ().toString ();
  auto const& satellite = ui->comboBoxSatellite->currentData ().toString ();
  m_freqRx = ui->freqRx->text ();
  //Log this QSO to file "wsjtx.log"
  static QFile f {QDir {QStandardPaths::writableLocation (QStandardPaths::DataLocation)}.absoluteFilePath ("wsjtx.log")};
  if(!f.open(QIODevice::Text | QIODevice::Append)) {
    MessageBox::warning_message (this, tr ("Log file error"),
                                 tr ("Cannot open \"%1\" for append").arg (f.fileName ()),
                                 tr ("Error: %1").arg (f.errorString ()));
  } else {
    QString logEntry=dateTimeOn.date().toString("yyyy-MM-dd,") +
      dateTimeOn.time().toString("hh:mm:ss,") +
      dateTimeOff.date().toString("yyyy-MM-dd,") +
      dateTimeOff.time().toString("hh:mm:ss,") + hisCall + "," +
      hisGrid + "," + strDialFreq + "," + mode +
      "," + rptSent + "," + rptRcvd + "," + m_txPower +
      "," + m_comments + "," + name + "," + prop_mode +
      "," + satellite + "," + m_freqRx;
    QTextStream out(&f);
    out << logEntry <<
#if QT_VERSION < QT_VERSION_CHECK(5, 15, 0)
                 endl
#else
                 Qt::endl
#endif
                 ;
    f.close();
  }

  //Clean up and finish logging
  Q_EMIT acceptQSO (dateTimeOff
                    , hisCall
                    , hisGrid
                    , m_dialFreq
                    , mode
                    , rptSent
                    , rptRcvd
                    , m_txPower
                    , m_comments
                    , name
                    , dateTimeOn
                    , operator_call
                    , m_myCall
                    , m_myGrid
                    , xsent
                    , xrcvd
                    , prop_mode
                    , satellite
                    , m_freqRx
                    , m_log->QSOToADIF (hisCall
                                        , hisGrid
                                        , mode
                                        , rptSent
                                        , rptRcvd
                                        , dateTimeOn
                                        , dateTimeOff
                                        , band
                                        , m_comments
                                        , name
                                        , strDialFreq
                                        , m_myCall
                                        , m_myGrid
                                        , m_txPower
                                        , operator_call
                                        , xsent
                                        , xrcvd
                                        , prop_mode
                                        , satellite
                                        , m_freqRx));
  QDialog::accept();
}

void LogQSO::propModeChanged()
{
    if (ui->comboBoxPropMode->currentData() != "SAT") {
        ui->comboBoxSatellite->setDisabled(true);
        ui->cbSatellite->setDisabled(true);
    } else {
        ui->comboBoxSatellite->setDisabled(false);
        ui->cbSatellite->setDisabled(false);
    }

}

// closeEvent is only called from the system menu close widget for a
// modeless dialog so we use the hideEvent override to store the
// window settings
void LogQSO::hideEvent (QHideEvent * e)
{
  storeSettings ();
  QDialog::hideEvent (e);
}
