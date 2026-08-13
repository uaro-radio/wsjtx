// -*- Mode: C++ -*-
#ifndef UAHAM_SETTINGS_TABS_HPP_
#define UAHAM_SETTINGS_TABS_HPP_

#include <QWidget>
#include <QStringList>

#include "logbook/AD1CCty.hpp"
#include "UaHam/CountryFilter.hpp"

class QCheckBox;
class QComboBox;
class QLabel;
class QLineEdit;
class QListWidget;
class QRadioButton;
class QSpinBox;

namespace UaHam
{
  //
  // Settings → «UaHam Filter»: which countries reach the screen.
  //
  // Built in code rather than added to Configuration.ui. That file is over
  // five thousand lines of generated XML which upstream edits in most
  // releases, and every tab added to it is a merge conflict in every future
  // version of WSJT-X. A widget of our own in a directory of our own is one
  // line of contact instead.
  //
  class FilterSettingsWidget final
    : public QWidget
  {
    Q_OBJECT

  public:
    explicit FilterSettingsWidget (QWidget * parent = nullptr);

    // The country list comes from the log book's cty.dat, which is loaded
    // once and lives in the main window. Called before the dialog is shown,
    // so a cty.dat updated while WSJT-X runs is picked up without a restart.
    void set_entities (QList<AD1CCty::Entity> const&);

    CountryFilter::Mode mode () const;
    QStringList selected_entities () const;

    void set_mode (CountryFilter::Mode);
    void set_selected_entities (QStringList const&);

  private:
    Q_SLOT void apply_search (QString const& needle);
    Q_SLOT void update_summary ();

    QRadioButton * off_;
    QRadioButton * block_;
    QRadioButton * only_;
    QLineEdit * search_;
    QListWidget * countries_;
    QLabel * summary_;

    // Ticks survive a cty.dat that no longer lists the entity, and survive
    // the search box hiding a row. Neither the widget's own check states nor
    // the visible rows can be the record of what the operator chose.
    QStringList selected_;
  };

  //
  // Settings → «UaHam Site»: the WebSocket the browser connects to.
  //
  class SiteSettingsWidget final
    : public QWidget
  {
    Q_OBJECT

  public:
    explicit SiteSettingsWidget (QWidget * parent = nullptr);

    bool enabled () const;
    quint16 port () const;

    void set_enabled (bool);
    void set_port (quint16);

    // What the server is actually doing, so the dialog does not have to claim
    // that a port it cannot bind is in use.
    void show_status (QString const& text);

  private:
    QCheckBox * enabled_;
    QSpinBox * port_;
    QLabel * status_;
  };

  //
  // Settings → «QRZ»: the account used for callsign lookups.
  //
  // QRZ.com's XML callbook service needs the operator's own account with an
  // XML subscription. The password is stored in the settings file as typed —
  // WSJT-X keeps every other password the same way, and pretending otherwise
  // by obfuscating it would suggest a protection that is not there.
  //
  class QrzSettingsWidget final
    : public QWidget
  {
    Q_OBJECT

  public:
    explicit QrzSettingsWidget (QWidget * parent = nullptr);

    QString username () const;
    QString password () const;
    void set_credentials (QString const& username, QString const& password);

    // What the last lookup did, so a wrong password is visible here rather
    // than only in the main window.
    void show_status (QString const&);

  private:
    QLineEdit * username_;
    QLineEdit * password_;
    QLabel * status_;
  };

  //
  // Settings → «Language»: which language the interface is in.
  //
  // WSJT-X picks its language from the system locale, and could otherwise only
  // be overridden with a --language argument on the command line — which an
  // operator who double-clicks an icon has no way to pass. This is that
  // argument, in a place they can reach.
  //
  // Qt installs translators once, at startup, before any of this dialog
  // exists, so a change here takes effect on the next run. Saying so on the
  // widget is better than pretending otherwise and leaving a half-translated
  // window behind.
  //
  class LanguageSettingsWidget final
    : public QWidget
  {
    Q_OBJECT

  public:
    explicit LanguageSettingsWidget (QWidget * parent = nullptr);

    // Empty means "follow the system", which is what WSJT-X always did.
    QString language () const;
    void set_language (QString const&);

  private:
    QComboBox * language_;
    QLabel * note_;
  };
}

#endif
