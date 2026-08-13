#include "QrzLookup.hpp"

#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QUrl>
#include <QUrlQuery>
#include <QXmlStreamReader>

#include "moc_QrzLookup.cpp"

namespace
{
  auto const service_url = "https://xmldata.qrz.com/xml/";

  // QRZ answers a session request and a callsign request with the same
  // document shape, so one reader serves both. Only the fields WSJT-Z shows
  // are taken; the rest of the record is none of this program's business.
  struct Parsed
  {
    QString key;
    QString error;
    UaHam::QrzLookup::Record record;
  };

  Parsed read (QByteArray const& data)
  {
    Parsed out;
    QXmlStreamReader reader {data};
    while (!reader.atEnd () && !reader.hasError ())
      {
        if (QXmlStreamReader::StartElement != reader.readNext ()) continue;
        auto const name = reader.name ().toString ();
        if ("Key" == name) out.key = reader.readElementText ();
        else if ("Error" == name) out.error = reader.readElementText ();
        else if ("call" == name) out.record.callsign = reader.readElementText ();
        else if ("fname" == name) out.record.name = reader.readElementText ();
        else if ("name" == name)
          {
            // QRZ splits the name in two, first name then surname.
            auto const surname = reader.readElementText ();
            out.record.name = out.record.name.isEmpty ()
              ? surname : out.record.name + ' ' + surname;
          }
        else if ("addr1" == name) out.record.address = reader.readElementText ();
        else if ("addr2" == name) out.record.town = reader.readElementText ();
        else if ("state" == name) out.record.state = reader.readElementText ();
        else if ("zip" == name) out.record.postcode = reader.readElementText ();
        else if ("country" == name) out.record.country = reader.readElementText ();
        else if ("grid" == name) out.record.grid = reader.readElementText ();
        else if ("email" == name) out.record.email = reader.readElementText ();
      }
    if (out.error.isEmpty () && reader.hasError ())
      {
        out.error = reader.errorString ();
      }
    return out;
  }
}

namespace UaHam
{
  QrzLookup::QrzLookup (QObject * parent)
    : QObject {parent}
    , network_ {new QNetworkAccessManager {this}}
  {
  }

  QrzLookup::~QrzLookup ()
  {
  }

  void QrzLookup::configure (QString const& username, QString const& password)
  {
    if (username == username_ && password == password_) return;
    username_ = username;
    password_ = password;
    // Credentials changed, so the session bought with the old ones is no
    // longer ours to use.
    session_key_.clear ();
    if (configured ()) open_session ();
  }

  bool QrzLookup::configured () const
  {
    return !username_.isEmpty () && !password_.isEmpty ();
  }

  void QrzLookup::open_session ()
  {
    QUrlQuery query;
    query.addQueryItem ("username", username_);
    query.addQueryItem ("password", password_);
    QUrl url {service_url};
    url.setQuery (query);

    auto * reply = network_->get (QNetworkRequest {url});
    connect (reply, &QNetworkReply::finished, this, [this, reply] {on_session_reply (reply);});
  }

  void QrzLookup::on_session_reply (QNetworkReply * reply)
  {
    reply->deleteLater ();
    if (QNetworkReply::NoError != reply->error ())
      {
        Q_EMIT failed (reply->errorString ());
        return;
      }

    auto const parsed = read (reply->readAll ());
    if (!parsed.error.isEmpty () || parsed.key.isEmpty ())
      {
        // QRZ says "username/password incorrect" or "not subscribed" in this
        // field; passing it through verbatim is more use than any wording of
        // ours would be.
        Q_EMIT failed (parsed.error.isEmpty () ? tr ("no session key returned") : parsed.error);
        return;
      }

    session_key_ = parsed.key;
    if (!pending_.isEmpty ())
      {
        auto const call = pending_;
        pending_.clear ();
        lookup (call);
      }
  }

  void QrzLookup::lookup (QString const& callsign)
  {
    if (callsign.isEmpty () || !configured ()) return;

    if (session_key_.isEmpty ())
      {
        // Remember it and ask again once logged in — the first callsign an
        // operator clicks after starting up would otherwise be the one lookup
        // that silently does nothing.
        pending_ = callsign;
        open_session ();
        return;
      }

    QUrlQuery query;
    query.addQueryItem ("s", session_key_);
    query.addQueryItem ("callsign", callsign);
    QUrl url {service_url};
    url.setQuery (query);

    auto * reply = network_->get (QNetworkRequest {url});
    connect (reply, &QNetworkReply::finished, this, [this, reply, callsign]
             {
               reply->deleteLater ();
               if (QNetworkReply::NoError != reply->error ())
                 {
                   Q_EMIT failed (reply->errorString ());
                   return;
                 }
               auto parsed = read (reply->readAll ());
               if (!parsed.error.isEmpty ())
                 {
                   // A session times out after a while of not being used, and
                   // QRZ reports that as an ordinary error. One silent retry
                   // costs a round trip and saves the operator from a lookup
                   // that fails for reasons they cannot see.
                   if (parsed.error.contains ("session", Qt::CaseInsensitive)
                       && !session_key_.isEmpty ())
                     {
                       session_key_.clear ();
                       pending_ = callsign;
                       open_session ();
                       return;
                     }
                   Q_EMIT failed (parsed.error);
                   return;
                 }
               if (parsed.record.callsign.isEmpty ()) parsed.record.callsign = callsign;
               Q_EMIT found (parsed.record);
             });
  }
}
