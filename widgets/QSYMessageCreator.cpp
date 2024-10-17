#include <QApplication>
#include <QButtonGroup>
#include <QMessageBox>
#include <QInputDialog>
#include <QTimer>
#include "QSYMessageCreator.h"
#include "ui_QSYMessageCreator.h"


  QSYMessageCreator::QSYMessageCreator(QWidget *parent) :
  QWidget(parent),
  ui(new Ui::QSYMessageCreator)
{
  ui->setupUi(this);
  setWindowTitle ("QSYMessageCreator");
}

QSYMessageCreator::~QSYMessageCreator()
{
	delete ui;
}

void QSYMessageCreator::setup()
{		
	if (ui->radioButFM->isChecked())
	{
		ui->label_2_FM->show();
		ui->messageLabel_FM->show();
	}
	else
	{	
		ui->label_2_FM->hide();
		ui->messageLabel_FM->hide();
	}
}

void QSYMessageCreator::on_button1_clicked()
{
	QString band = getBand();
	QString mode = getMode(band);
	qint16 kHz = ui->kHzBox->value();
	QString kHzStr = QString::number(kHz);
	QString message = "$DX " + band + mode + kHzStr;
	ui->messageLabel->setText(message);
	Q_EMIT sendMessage(message);
}

void QSYMessageCreator::on_showMessagesChkBox_stateChanged()
{
	bool chkBoxValue = ui->showMessagesChkBox->isChecked();	
	Q_EMIT sendChkBoxChange(chkBoxValue);
}

void QSYMessageCreator::setQSYMessageCreatorStatusFalse()
{
	Q_EMIT sendQSYMessageCreatorStatus(false);
}

QString QSYMessageCreator::getBand()
{
	QString band;
	if (ui->radioBut50->isChecked())
	{
		band = "A";
	}
	else if (ui->radioBut144->isChecked())
	{
		band = "B";
	}
	else if (ui->radioBut222->isChecked())
	{
		band = "C";
	}
	else if (ui->radioBut432->isChecked())
	{
		band = "D";
	}
	else if (ui->radioBut902->isChecked())
	{
		band = "92";
	}
	else if (ui->radioBut903->isChecked())
	{
		band = "93";
	}
	else if (ui->radioBut1296->isChecked())
	{
		band = "E";
	}
	else if (ui->radioBut2304->isChecked())
	{
		band = "F";
	}
	else if (ui->radioBut3400->isChecked())
	{
		band = "G";
	}
	else if (ui->radioBut5760->isChecked())
	{
		band = "H";
	}
	else if (ui->radioBut10368->isChecked())
	{
		band = "I";
	}
	else if (ui->radioBut24192->isChecked())
	{
		band = "J";
	}
	return band;
	
}

QString QSYMessageCreator::getMode(QString band)
{
	QString mode;
	QString  MHz;
	if (ui->radioButSSB->isChecked())
	{
		mode = "V";
	}
	else if (ui->radioButFM->isChecked())
	{
		mode = "M";
		if(band == "A")
		{
			MHz = "52";
		}
		else if (band == "B")
		{
			MHz = "146";
		}
		else if (band == "C")
		{
			MHz = "223";
		}
		else if (band == "D")
		{
			MHz = "446";
		}
	}
	else if (ui->radioButCW->isChecked())
	{
		mode = "W";
	}
	else if (ui->radioButFT8->isChecked())
	{
		mode = "8";
	}
	else if (ui->radioButMSK->isChecked())
	{
		mode = "4";
	}
	if((band =="A" || band == "B" || band == "C" || band == "D") && (mode =="M"))
	{			
		ui->messageLabel_FM->setHidden(false);
		ui->label_2_FM->setHidden(false);	
		ui->messageLabel_FM->setText(MHz);
	}
	else
	{		
		ui->messageLabel_FM->setHidden(true);
		ui->label_2_FM->setHidden(true);		
	}
	return mode;
}
