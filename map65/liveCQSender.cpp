#include "liveCQSender.hpp"
#include <QSettings>
#include "SettingsGroup.hpp"
#include "mainwindow.h"
#include "qt_helpers.hpp"
#include "../revision_utils.hpp"

#include <fstream>
#include <iostream>
#include <cmath>
#include <QObject>
#include <QString>
#include <QDateTime>
#include <QSharedPointer>
#include <QSslSocket>
#include <QHostInfo>
#include <QQueue>
#include <QByteArray>
#include <QDataStream>
#include <QTimer>
#include <QDir>
#include <QSettings>
#include <QDebug>
#include <QMetaType>
#if QT_VERSION >= QT_VERSION_CHECK(5, 15, 0)
#include <QRandomGenerator>
#endif

#include "../Logger.hpp"
#include "devsetup.h"

# define DEBUGPSK 1;

namespace
{
  //quint16 SERVICE_PORT {80};  //this is http port
  quint16 SERVICE_PORT {443}; //this is https test port
}

liveCQSender::liveCQSender(QString const& myCall, QString const& myGrid, QString const& theUrl)  
      : logger_ {boost::log::keywords::channel = "PSKRPRT"}
      , m_myCall {myCall}
      , m_myGrid {myGrid}
      , m_theUrl {theUrl}
    {        
      qRegisterMetaType<QAbstractSocket::SocketError>();
      qDebug() << "Current thread for impl (liveCQSender):" << QThread::currentThread();
    } 
  
    
  void liveCQSender::init() {
     qDebug() << "liveCQSender::init running on thread: " << QThread::currentThread();      
    socket.reset(new QSslSocket(this));
    
    // Connect signals before connecting
    connect(socket.data(), &QAbstractSocket::connected, this, &liveCQSender::onConnected);
    connect(socket.data(), &QAbstractSocket::readyRead, this, &liveCQSender::onReadyRead); 
    connect(this, &liveCQSender::dataReady, this, &liveCQSender::sendData);   
#if QT_VERSION >= QT_VERSION_CHECK(5, 15, 0)
    connect(socket.data(), &QAbstractSocket::errorOccurred, this, &liveCQSender::handle_socket_error);  
#else   
    connect(socket.data(), static_cast<void(QAbstractSocket::*)(QAbstractSocket::SocketError)>(&QAbstractSocket::error),
        this, &liveCQSender::handle_socket_error);  
#endif

    // Start connection (non-blocking
    socket->connectToHostEncrypted("w3sz.com", SERVICE_PORT);
  }
  
  void liveCQSender::onConnected() {
    
    qDebug() << "LiveCQSender onConnected Connected securely!";
   
    for(const QByteArray &ba : onConnectedRequests) {
      qDebug() << "liveCQSender::onConnected request: " << ba;
      socket->write(ba); 
    }
    onConnectedRequests.clear();
  }
  
  
  void liveCQSender::onReadyRead() {
    QByteArray data = socket->readAll();
    // Parse headers, then handle chunks
    while (!data.isEmpty()) {
        int pos = data.indexOf("\r\n");
        bool ok;
        int size = data.left(pos).trimmed().toInt(&ok, 16);
        if (!ok || size == 0) break;
        data = data.mid(pos + 2);
        qDebug() << "Chunk:" << data.left(size);
        data = data.mid(size + 2); // skip \r\n
    }
      qDebug() << "liveCQSender Response:" <<   data;
}   
  
  
  void liveCQSender::handle_socket_error(QAbstractSocket::SocketError error)
  {
    qDebug() << "liveCQSender Socket error:" << error << socket->errorString();
  }   
  
  void liveCQSender::addRemoteStation (QByteArray const& postByteArray, QString const& theUrl)
  {
    qDebug() << theUrl;
    emit dataReady(postByteArray);
  }
  
  void liveCQSender::sendData(const QByteArray &payload1) {
    QByteArray request;
    QByteArray body;
    if(!payload1.isEmpty())
    {
      body.append(payload1); // Your data here
    }
    request.append("POST /livecq_update.php HTTP/1.1\r\n");
    request.append("Host: w3sz.com\r\n");
    request.append("Content-Type: application/x-www-form-urlencoded\r\n");
    request.append("Connection: keep-alive\r\n");
    request.append("Content-Length: " + QByteArray::number(body.size()) + "\r\n\r\n");
    if(!payload1.isEmpty())
    {
      request.append(body);
    }
    qDebug() << "LiveCQ sendData request: " << request;
    if (socket->state() == QAbstractSocket::ConnectedState) {
      socket->write(request);  
      qDebug() << "liveCQSender::sendData request2: " << request;
      }
    else {
      onConnectedRequests.append(request);
      request.clear();
      socket->connectToHostEncrypted("w3sz.com", SERVICE_PORT);
      // Data will be sent in onConnected()
    }
  }
  
  liveCQSender::~liveCQSender() = default; // Define it, even as default   
   
