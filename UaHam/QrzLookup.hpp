// -*- Mode: C++ -*-
#ifndef UAHAM_QRZ_LOOKUP_HPP_
#define UAHAM_QRZ_LOOKUP_HPP_

#include <QObject>
#include <QString>

class QNetworkAccessManager;
class QNetworkReply;

namespace UaHam
{
  //
  // Who is this station? — asked of QRZ.com's XML callbook service.
  //
  // Ported from WSJT-Z, which does the same thing inline in its main window.
  // Kept as a class of its own here: the network conversation is two requests
  // with a session key between them, and that is easier to follow — and to
  // fix — when it is not interleaved with a decoder.
  //
  // This is the callbook service (xmldata.qrz.com), not the Logbook API. It
  // needs the operator's own QRZ.com account with an XML subscription; without
  // one, every request answers with an error, which is reported as-is rather
  // than guessed at.
  //
  // The result is shown to the operator working that station and nowhere else.
  // QRZ's member agreement forbids collecting these details for other uses,
  // and an address that arrives here must not travel any further.
  //
  class QrzLookup final
    : public QObject
  {
    Q_OBJECT

  public:
    struct Record
    {
      QString callsign;
      QString name;
      QString address;
      QString town;
      QString state;
      QString postcode;
      QString country;
      QString grid;
      QString email;
    };

    explicit QrzLookup (QObject * parent = nullptr);
    ~QrzLookup ();

    // Credentials from the settings dialog. Passing empty ones forgets the
    // session, which is what "the operator cleared their password" should do.
    void configure (QString const& username, QString const& password);
    bool configured () const;

    // Ask about a callsign. A request made before the session exists is
    // remembered and repeated once it does, so the first lookup after startup
    // is not silently lost.
    void lookup (QString const& callsign);

    Q_SIGNAL void found (Record const&);
    Q_SIGNAL void failed (QString const& reason);

  private:
    void on_session_reply (QNetworkReply *);
    void open_session ();

    QNetworkAccessManager * network_;
    QString username_;
    QString password_;
    QString session_key_;
    QString pending_;
  };
}

#endif
