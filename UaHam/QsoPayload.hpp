// -*- Mode: C++ -*-
#ifndef UAHAM_QSO_PAYLOAD_HPP_
#define UAHAM_QSO_PAYLOAD_HPP_

#include <QJsonObject>

class QByteArray;

namespace UaHam
{
  //
  // The ADIF record of a logged QSO, in the shape uahamaward.com expects.
  //
  // This is a deliberate re-implementation of what the Go bridge
  // (uaham-bridge, wsjtx-bridge/main.go) did with the ADIF that arrived over
  // UDP. The point of the whole exercise is that the site cannot tell the
  // difference: the same field names, the same units, the same handling of
  // MFSK submodes. An operator who stops running the bridge should see their
  // contacts arrive exactly as before, and the site should need no change at
  // all to accept them.
  //
  // Returns an empty object when the record carries no callsign, which is the
  // one field the site cannot do without.
  //
  QJsonObject qso_payload_from_adif (QByteArray const& adif_record);
}

#endif
