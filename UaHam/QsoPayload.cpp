#include "QsoPayload.hpp"

#include <QByteArray>
#include <QDateTime>
#include <QRegularExpression>
#include <QString>
#include <QTimeZone>

namespace
{
  //
  // <TAG:LEN>VALUE, or <TAG:LEN:TYPE>VALUE. The declared length is honoured
  // rather than trusted: ADIF allows a value to be followed by anything at
  // all up to the next tag, and WSJT-X's own writer pads some fields.
  //
  QRegularExpression const field_re {R"(<([^:>]+):(\d+)(?::[^>]+)?>([^<]*))"
                                     , QRegularExpression::CaseInsensitiveOption};

  //
  // ADIF states frequency in megahertz; the platform logs kilohertz.
  //
  // Shifting the decimal point by three places, not multiplying: 14.074000
  // has to come out as 14074, and a contact 430 Hz from where it was worked
  // will not match the correspondent's own log. An unreadable value yields
  // nothing rather than a zero — the site then falls back to the band's
  // calling frequency, which is at least right.
  //
  QString mhz_to_khz (QString const& value)
  {
    bool ok {false};
    auto const mhz = value.trimmed ().toDouble (&ok);
    if (!ok || mhz <= 0.) return QString {};

    auto khz = QString::number (mhz * 1000., 'f', 3);
    while (khz.endsWith ('0')) khz.chop (1);
    if (khz.endsWith ('.')) khz.chop (1);
    return khz;
  }
}

namespace UaHam
{
  QJsonObject qso_payload_from_adif (QByteArray const& adif_record)
  {
    auto const record = QString::fromUtf8 (adif_record);

    QString call, band, frequency, mode, submode, rst_sent, rst_received;
    QString grid, station_callsign, qso_date, time_on;

    auto matches = field_re.globalMatch (record);
    while (matches.hasNext ())
      {
        auto const match = matches.next ();
        auto const tag = match.captured (1).toUpper ();
        auto const declared_length = match.captured (2).toInt ();
        auto value = match.captured (3);
        if (value.size () > declared_length) value = value.left (declared_length);
        value = value.trimmed ();

        if ("CALL" == tag) call = value;
        else if ("BAND" == tag) band = value.toLower ();
        else if ("FREQ" == tag) frequency = mhz_to_khz (value);
        else if ("MODE" == tag) mode = value.toUpper ();
        else if ("SUBMODE" == tag) submode = value.toUpper ();
        else if ("RST_SENT" == tag) rst_sent = value;
        else if ("RST_RCVD" == tag) rst_received = value;
        else if ("GRIDSQUARE" == tag) grid = value;
        else if ("STATION_CALLSIGN" == tag) station_callsign = value;
        else if ("QSO_DATE" == tag) qso_date = value;
        else if ("TIME_ON" == tag) time_on = value;
      }

    if (call.isEmpty ()) return QJsonObject {};

    // FT4, FT2, JS8 and Q65 are not ADIF modes of their own — they are MFSK
    // with a SUBMODE, and MODE alone reports all of them as the same thing.
    // Logging that literally files every FT4 contact as "MFSK": never
    // matching an award rule that asks for FT4. Only MFSK is unfolded; SSB
    // carries SUBMODE USB/LSB, and those are not modes anyone logs.
    if (!submode.isEmpty () && "MFSK" == mode) mode = submode;

    // QSO_DATE is YYYYMMDD and TIME_ON is HHMM or HHMMSS, both UTC. A record
    // missing either keeps the current time: the site needs some instant, and
    // "now" is wrong by seconds where an empty field is wrong by decades.
    auto worked_at = QDateTime::currentDateTimeUtc ();
    if (8 == qso_date.size () && time_on.size () >= 4)
      {
        auto const padded = 4 == time_on.size () ? time_on + "00" : time_on.left (6);
        auto const parsed = QDateTime::fromString (qso_date + padded, "yyyyMMddhhmmss");
        if (parsed.isValid ())
          {
            worked_at = parsed;
            worked_at.setTimeSpec (Qt::UTC);
          }
      }

    QJsonObject payload;
    payload["workedCallsign"] = call;
    payload["band"] = band;
    if (!frequency.isEmpty ()) payload["frequency"] = frequency;
    payload["mode"] = mode;
    payload["rstSent"] = rst_sent;
    payload["rstRecv"] = rst_received;
    payload["workedGrid"] = grid;
    payload["workedAt"] = worked_at.toString (Qt::ISODate);
    payload["activatorCallsign"] = station_callsign;
    payload["rawAdif"] = record;
    return payload;
  }
}
