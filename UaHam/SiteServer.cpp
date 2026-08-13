#include "SiteServer.hpp"

#include <QDateTime>
#include <QHostAddress>
#include <QJsonDocument>
#include <QWebSocket>
#include <QWebSocketServer>

#include "Logger.hpp"
// Boost.Log has no idea what a QString is; this is where WSJT-X teaches it.
#include "qt_helpers.hpp"

#include "moc_SiteServer.cpp"

namespace
{
  auto const server_name = "UaHamAward";

  // The bridge published this alongside its own version so the site could
  // show the operator which ports were in use. WSJT-X receives no UDP of its
  // own here, so the field is kept for shape and reported as unused rather
  // than dropped: an older page reads it without checking whether it exists.
  auto const no_udp_port = 0;
}

namespace UaHam
{
  SiteServer::SiteServer (QString const& version, QObject * parent)
    : QObject {parent}
    , version_ {version}
  {
  }

  SiteServer::~SiteServer ()
  {
    stop ();
  }

  bool SiteServer::start (quint16 preferred_port)
  {
    stop ();
    error_.clear ();

    server_ = new QWebSocketServer {server_name, QWebSocketServer::NonSecureMode, this};
    connect (server_, &QWebSocketServer::newConnection, this, &SiteServer::on_new_connection);

    // Port 0 asks the system for whatever is spare: one attempt, since it
    // cannot come back busy. Counting in int rather than quint16 so that a
    // port near the top of the range cannot wrap the end of the window round
    // to somewhere near zero.
    int const first = preferred_port;
    int const last = preferred_port ? qMin (first + PORT_SCAN_SPAN - 1, 65535) : 0;
    for (int p = first; p <= last; ++p)
      {
        if (server_->listen (QHostAddress::LocalHost, static_cast<quint16> (p)))
          {
            LOG_INFO (QString {"UaHamAward site server listening on 127.0.0.1:%1"}
                      .arg (server_->serverPort ()));
            return true;
          }
        error_ = server_->errorString ();
      }

    LOG_INFO (QString {"UaHamAward site server could not listen on %1: %2"}
              .arg (preferred_port).arg (error_));
    delete server_;
    server_ = nullptr;
    return false;
  }

  void SiteServer::stop ()
  {
    if (!server_) return;

    // Closing the server first stops anything new arriving while the existing
    // sockets are being torn down.
    server_->close ();
    auto const had_clients = !clients_.isEmpty ();
    for (auto * client : clients_)
      {
        // Deliberately not through on_client_disconnected: that one edits the
        // list this loop is walking.
        client->disconnect (this);
        client->close ();
        client->deleteLater ();
      }
    clients_.clear ();
    server_->deleteLater ();
    server_ = nullptr;
    if (had_clients) Q_EMIT clients_changed (0);
  }

  bool SiteServer::listening () const
  {
    return server_ && server_->isListening ();
  }

  quint16 SiteServer::port () const
  {
    return server_ ? server_->serverPort () : 0;
  }

  int SiteServer::client_count () const
  {
    return clients_.size ();
  }

  void SiteServer::on_new_connection ()
  {
    while (server_ && server_->hasPendingConnections ())
      {
        auto * client = server_->nextPendingConnection ();
        if (!client) break;

        connect (client, &QWebSocket::disconnected, this, &SiteServer::on_client_disconnected);

        // The greeting goes out before the socket joins the broadcast list,
        // so this write can never interleave with a QSO going to everyone.
        send (client, greeting ());
        clients_.append (client);
        LOG_INFO (QString {"UaHamAward site connected (%1 client(s))"}.arg (clients_.size ()));
        Q_EMIT clients_changed (clients_.size ());
      }
  }

  void SiteServer::on_client_disconnected ()
  {
    auto * client = qobject_cast<QWebSocket *> (sender ());
    if (!client) return;

    if (clients_.removeAll (client))
      {
        LOG_INFO (QString {"UaHamAward site disconnected (%1 client(s))"}.arg (clients_.size ()));
        Q_EMIT clients_changed (clients_.size ());
      }
    client->deleteLater ();
  }

  int SiteServer::publish_qso (QJsonObject const& payload)
  {
    if (payload.isEmpty () || clients_.isEmpty ()) return 0;

    QJsonObject message;
    message["type"] = "qso_logged";
    message["payload"] = payload;
    message["timestamp"] = QDateTime::currentDateTimeUtc ().toString (Qt::ISODate);

    for (auto * client : clients_)
      {
        send (client, message);
      }
    return clients_.size ();
  }

  QJsonObject SiteServer::greeting () const
  {
    QJsonObject info;
    info["version"] = version_;
    info["udpPort"] = no_udp_port;
    info["wsPort"] = port ();
    // Not part of the bridge's greeting, and ignored by every page that reads
    // it today. It is here so a future version of the site can say "WSJT-X"
    // where it currently says "bridge" without having to guess from a version
    // string.
    info["source"] = "wsjtx";

    QJsonObject message;
    message["type"] = "hello";
    message["info"] = info;
    message["timestamp"] = QDateTime::currentDateTimeUtc ().toString (Qt::ISODate);
    return message;
  }

  void SiteServer::send (QWebSocket * client, QJsonObject const& message)
  {
    client->sendTextMessage (QString::fromUtf8 (QJsonDocument {message}.toJson (QJsonDocument::Compact)));
  }
}
