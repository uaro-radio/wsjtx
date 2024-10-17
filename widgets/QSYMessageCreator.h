// -*- Mode: C++ -*-
#ifndef QSYMESSAGECREATOR_H
#define QSYMESSAGECREATOR_H
#include <QWidget>
#include <QObject>
#include <QCloseEvent> 

namespace Ui {
  class QSYMessageCreator;
}

class QSYMessageCreator
  : public QWidget
{
  Q_OBJECT

public:
  explicit QSYMessageCreator(QWidget * parent = 0);  
  ~QSYMessageCreator();
  
protected:
    void showEvent(QShowEvent *event) override {
        QWidget::showEvent(event); // Call the base class implementation        
        setup();
    }  void closeEvent(QCloseEvent *event) override {
        // Custom close handling
	    setQSYMessageCreatorStatusFalse();     
		event->accept();
    }
  
signals:
  void sendMessage(const QString &value); 
  void sendChkBoxChange(const bool &value);  
  void sendQSYMessageCreatorStatus(const bool &value);
  
private:
  Ui::QSYMessageCreator *ui;
  QString getBand();
  QString getMode(QString band);
  
private slots:
  void on_button1_clicked();
  void on_showMessagesChkBox_stateChanged();
  void setup();
  void setQSYMessageCreatorStatusFalse();
};

#endif //QSYMESSAGECREATOR_H