// -*- Mode: C++ -*-
#ifndef UAHAM_STATUS_WIDGET_HPP_
#define UAHAM_STATUS_WIDGET_HPP_

#include <QWidget>

class QLabel;

namespace UaHam
{
  //
  // The «UaHam» tab beside «1» and «2» in the main window.
  //
  // Everything here is also in the status bar, in two words each. This is the
  // longer answer, for the moment when the band has gone quiet and the
  // operator wants to know whether that is the band or the filter — a
  // question the status bar can raise but not settle.
  //
  class StatusWidget final
    : public QWidget
  {
    Q_OBJECT

  public:
    explicit StatusWidget (QWidget * parent = nullptr);

    void show_filter (QString const& mode, int countries, unsigned hidden);
    void show_site (QString const& state, unsigned qsos_sent);

    // The operator pressed «Reset», so the counters start again from here.
    Q_SIGNAL void counters_reset ();

  private:
    QLabel * filter_mode_;
    QLabel * filter_hidden_;
    QLabel * site_state_;
    QLabel * site_sent_;
  };
}

#endif
