// -*- Mode: C++ -*-
#ifndef QSYMESSAGECREATOR_H
#define QSYMESSAGECREATOR_H
#include <QWidget>
#include <QObject>
#include <QCloseEvent>

class QSettings;
class Configuration;
namespace Ui {
  class QSYMessageCreator;
}

class QSYMessageCreator
  : public QWidget
{
  Q_OBJECT

public:
  explicit QSYMessageCreator(QSettings * settings, Configuration const *, QWidget * parent = 0);
  ~QSYMessageCreator();
  QString WriteMessage(QString band, QString mode);

protected:
  void showEvent(QShowEvent *event) override {
    QWidget::showEvent(event); // Call the base class implementation    
  }
  void closeEvent(QCloseEvent *event) override;

signals:
  void sendMessage(const QString &value);
  void sendChkBoxChange(const bool &value);
  void sendQSYMessageCreatorStatus(const bool &value);

private:
  QSettings * settings_;
  Configuration const * configuration_;

  Ui::QSYMessageCreator *ui;
  QString getBand();
  void setBand(QString band);
  void setMode(QString band, QString mode, int region);
  QString getMode(QString band, int region);
  void setup(int region);

private slots:
  void on_button1_clicked();
  void setQSYMessageCreatorStatusFalse();
  void read_settings ();
  void write_settings ();
};

#endif //QSYMESSAGECREATOR_H
