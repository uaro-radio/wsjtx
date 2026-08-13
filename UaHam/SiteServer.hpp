// -*- Mode: C++ -*-
#ifndef UAHAM_SITE_SERVER_HPP_
#define UAHAM_SITE_SERVER_HPP_

#include <QObject>
#include <QJsonObject>
#include <QList>
#include <QString>

class QWebSocket;
class QWebSocketServer;

namespace UaHam
{
  //
  // The WebSocket endpoint uahamaward.com's log pages connect to.
  //
  // It exists to delete a moving part. Until now an operator had to download,
  // configure and keep updated a separate program (uaham-bridge) whose whole
  // job was to receive WSJT-X's UDP packets and repeat them to the browser.
  // WSJT-X can hold that socket itself, so this speaks the bridge's protocol
  // exactly — same default port, same greeting, same message names — and the
  // site cannot tell which of the two it is talking to.
  //
  // Nothing is ever pushed at a browser that is not there: with no client
  // connected a logged QSO is simply not published, and WSJT-X's own logging
  // is untouched either way. The socket is not a step in logging a contact,
  // it is a copy sent to somebody watching.
  //
  // Bound to the loopback address on purpose. The browser reaches it at
  // 127.0.0.1 and nothing else has any business reaching it at all — and on
  // Windows, binding a wildcard address is what raises the firewall prompt
  // that an operator has no way to judge.
  //
  class SiteServer final
    : public QObject
  {
    Q_OBJECT

  public:
    // How far past the requested port to keep looking for a free one. The
    // same window the site's browser-side probe walks, so a server that had
    // to move is always inside the range the site searches.
    static constexpr quint16 PORT_SCAN_SPAN = 20;

    explicit SiteServer (QString const& version, QObject * parent = nullptr);
    ~SiteServer ();

    // Listen on the first free port at or after the one asked for. False
    // means the whole window was taken; error_string () says what the last
    // attempt reported.
    bool start (quint16 preferred_port);
    void stop ();

    bool listening () const;
    quint16 port () const;
    int client_count () const;
    QString error_string () const {return error_;}

    // Send a logged contact to every connected browser. Returns the number of
    // clients it reached, which is zero when nobody is listening — not a
    // failure, just nobody watching.
    int publish_qso (QJsonObject const& payload);

    // Connections came or went; the main window's indicator follows this.
    Q_SIGNAL void clients_changed (int count);

  private:
    Q_SLOT void on_new_connection ();
    Q_SLOT void on_client_disconnected ();

    void send (QWebSocket *, QJsonObject const& message);
    QJsonObject greeting () const;

    QString version_;
    QString error_;
    QWebSocketServer * server_ {nullptr};
    QList<QWebSocket *> clients_;
  };
}

#endif
