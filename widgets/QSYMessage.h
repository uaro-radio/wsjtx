// -*- Mode: C++ -*-
#ifndef QSYMESSAGE_H
#define QSYMESSAGE_H

#include <QWidget>

namespace Ui {
  class QSYMessage;
}

class QSYMessage
    : public QWidget
{
  Q_OBJECT


public:
  explicit QSYMessage(const QString& message, const QString& theCall, QWidget * parent = 0);
  ~QSYMessage();
  void getBandModeFreq();

signals:
  void sendReply(const QString &value);

private:
  Ui::QSYMessage *ui;
  QString receivedMessage;
  QString receivedCall;

private slots:
  void on_yesButton_clicked();
  void on_noButton_clicked();

};

#endif //QSYMESSAGE_H
