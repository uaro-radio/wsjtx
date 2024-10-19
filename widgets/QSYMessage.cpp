#include <QApplication>
#include <QDateTime>
#include "QSYMessage.h"
#include "ui_QSYMessage.h"

QSYMessage::QSYMessage(const QString& message,const QString& theCall, QWidget *parent) :
    QWidget(parent),
    ui(new Ui::QSYMessage),
    receivedMessage(message),receivedCall(theCall)
{
  ui->setupUi(this);
  setWindowTitle ("BAND CHANGE NOW" + QTime::currentTime().toString("[hh:mm:ss]"));
  ui->label->setStyleSheet("font: bold; font-size: 36pt");
  ui->label_2->setStyleSheet("font: bold; font-size: 36pt");
  ui->label_3->setStyleSheet("font: bold; font-size: 36pt");
  getBandModeFreq();
}

QSYMessage::~QSYMessage()
{
  delete ui;
}

void QSYMessage::on_yesButton_clicked()
{
  QString message = QString(receivedCall) + QString(" OKQSY");
  Q_EMIT sendReply(message);
  ui->yesButton->setStyleSheet("background-color:#00ff00");
  ui->noButton->setStyleSheet("background-color:palette(button).color()");
}

void QSYMessage::on_noButton_clicked()
{
  QString message = QString(receivedCall) + QString(" NOQSY");
  Q_EMIT sendReply(message);
  ui->noButton->setStyleSheet("background-color:red; color:white");
  ui->yesButton->setStyleSheet("background-color:palette(button).color()");
}

void QSYMessage::getBandModeFreq()
{
  if(receivedMessage.at(0) =='$' && receivedMessage.size() >= 5)
  {
#if QT_VERSION >= QT_VERSION_CHECK (5, 15, 0)
    QStringList bhList = receivedMessage.split(" ",Qt::SkipEmptyParts);
#else
    QStringList bhList = receivedMessage.split(" ",QString::SkipEmptyParts);
#endif
    ui->label->setText(bhList[1]);
    ui->label_2->setText("replied");
    ui->label_3->setText(bhList[2].mid(0,2));
    ui->yesButton->hide();
    ui->noButton->hide();
  }
  else if(receivedMessage.at(0).isLetter() || receivedMessage.at(0) == '9')
  {
    QString bandParam = "";
    QChar modeParam = '\0';
    QString freqParam = "";

    if (receivedMessage.at(0).isLetter())
    {
      bandParam = receivedMessage.mid(0,1);
      modeParam = receivedMessage.at(1);
      freqParam = receivedMessage.mid(2, 3);
    }
    else if (receivedMessage.at(0) == '9')
    {
      bandParam = receivedMessage.mid(0,2);
      modeParam = receivedMessage.at(2);
      freqParam = receivedMessage.mid(3,3);
    }
    QString band = "";
    QString mode = "";
    QString freq = "";

    if (bandParam ==  "A"){
      if(modeParam != 'M') {
        freq = "50.";
      }
      else
      {freq = "52.";}
    }
    else if (bandParam ==  "B"){
      if(modeParam != 'M') {
        freq = "144.";
      }
      else
      {freq = "146.";}
    }
    else if (bandParam ==  "C"){
      if(modeParam != 'M') {
        freq = "222.";
      }
      else
      {freq = "223.";}
    }
    else if (bandParam ==  "D"){
      if(modeParam != 'M') {
        freq = "432.";
      }
      else
      {freq = "446.";}
    }
    else if (bandParam ==  "E"){
      freq = "1296.";
    }
    else if (bandParam ==  "F"){
      freq = "2304.";
    }
    else if (bandParam ==  "G"){
      freq = "3400.";
    }
    else if (bandParam ==  "H"){
      freq = "5760.";
    }
    else if (bandParam ==  "I"){
      freq = "10368.";
    }
    else if (bandParam ==  "J"){
      freq = "24192.";
    }
    else if (bandParam ==  "92"){
      freq = "902.";
    }
    else if (bandParam ==  "93"){
      freq = "903.";
    }
    else {
      freq = "";
    }

    freq = freq + freqParam;

    if (modeParam ==  'V'){
      mode = "SSB";
    }
    else if (modeParam == '4'){
      mode = "MSK";
    }
    else if (modeParam ==  '8'){
      mode = "FT8";
    }
    else if (modeParam ==  'M'){
      mode = "FM";
    }
    else if (modeParam ==  'W'){
      mode = "CW";
    }
    else {
      mode = "";
    }
    ui->label->setText("QSY to");
    ui->label_2->setText(freq + " MHz");
    ui->label_3->setText("mode " + mode);
  }
}
