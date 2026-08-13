// -*- Mode: C++ -*-
#ifndef UAHAM_CALL_INFO_WIDGET_HPP_
#define UAHAM_CALL_INFO_WIDGET_HPP_

#include <QWidget>

#include "UaHam/QrzLookup.hpp"

class QLabel;
class QPushButton;

namespace UaHam
{
  //
  // The «Call info» tab: who the station in the DX Call box is.
  //
  // WSJT-X shows the country and whether you have worked it before; this adds
  // the operator's name and where they are, which is what turns a callsign
  // into a person you are greeting by name.
  //
  // Everything here comes from QRZ.com and is shown to the operator working
  // that station. It is not stored, not logged, and not sent anywhere.
  //
  class CallInfoWidget final
    : public QWidget
  {
    Q_OBJECT

  public:
    explicit CallInfoWidget (QWidget * parent = nullptr);

    void show_record (QrzLookup::Record const&);
    void show_message (QString const& text);
    // Called when a lookup starts, so the panel does not go on showing the
    // previous station while the next one is being fetched.
    void awaiting (QString const& callsign);

    // «Open on qrz.com» — the page for whichever callsign is on display.
    Q_SIGNAL void open_requested (QString const& callsign);

  private:
    QLabel * callsign_;
    QLabel * name_;
    QLabel * location_;
    QLabel * country_;
    QLabel * grid_;
    QLabel * email_;
    QLabel * message_;
    QPushButton * open_;
    QString current_;
  };
}

#endif
