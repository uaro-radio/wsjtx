#include "SettingsTabs.hpp"

#include <QCheckBox>
#include <QComboBox>
#include <QFormLayout>
#include <QGroupBox>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QListWidget>
#include <QPushButton>
#include <QRadioButton>
#include <QSignalBlocker>
#include <QSpinBox>
#include <QVBoxLayout>

#include "moc_SettingsTabs.cpp"

namespace
{
  // The primary prefix travels with the row so that reading the ticks back
  // never depends on how the label was spelled.
  auto const entity_role = Qt::UserRole;
}

namespace UaHam
{
  FilterSettingsWidget::FilterSettingsWidget (QWidget * parent)
    : QWidget {parent}
    , off_ {new QRadioButton {tr ("Off — show everything"), this}}
    , block_ {new QRadioButton {tr ("Hide the countries ticked below"), this}}
    , only_ {new QRadioButton {tr ("Show only the countries ticked below"), this}}
    , search_ {new QLineEdit {this}}
    , countries_ {new QListWidget {this}}
    , summary_ {new QLabel {this}}
  {
    auto * mode_box = new QGroupBox {tr ("Country filter"), this};
    auto * mode_layout = new QVBoxLayout {mode_box};
    mode_layout->addWidget (off_);
    mode_layout->addWidget (block_);
    mode_layout->addWidget (only_);

    off_->setChecked (true);

    auto * explanation = new QLabel {
      tr ("Decides who reaches the decode windows by DXCC entity, not by matching text.\n"
          "A hidden station is hidden from Band Activity, from Rx Frequency and from Active "
          "Stations, and auto-sequencing will not answer it even if it calls you directly.\n"
          "ALL.TXT keeps every decode, and other programs reading WSJT-X over UDP still see "
          "them all.\n"
          "In \"show only\" mode a decode whose country cannot be established — free text, an "
          "unresolved <...> callsign — is hidden as well."), this};
    explanation->setWordWrap (true);

    search_->setPlaceholderText (tr ("Type to find a country…"));
    search_->setClearButtonEnabled (true);

    countries_->setSelectionMode (QAbstractItemView::NoSelection);
    countries_->setUniformItemSizes (true);

    auto * clear = new QPushButton {tr ("Untick all"), this};

    auto * tools = new QHBoxLayout;
    tools->addWidget (search_, 1);
    tools->addWidget (clear);

    auto * layout = new QVBoxLayout {this};
    layout->addWidget (mode_box);
    layout->addWidget (explanation);
    layout->addLayout (tools);
    layout->addWidget (countries_, 1);
    layout->addWidget (summary_);

    // The summary says what the current mode will do, so it has to follow the
    // mode as well as the ticks.
    for (auto * button : {off_, block_, only_})
      {
        connect (button, &QRadioButton::toggled, this, &FilterSettingsWidget::update_summary);
      }
    connect (search_, &QLineEdit::textChanged, this, &FilterSettingsWidget::apply_search);
    connect (countries_, &QListWidget::itemChanged, this, [this] (QListWidgetItem * item)
             {
               auto const prefix = item->data (entity_role).toString ();
               if (prefix.isEmpty ()) return;
               if (Qt::Checked == item->checkState ())
                 {
                   if (!selected_.contains (prefix)) selected_.append (prefix);
                 }
               else
                 {
                   selected_.removeAll (prefix);
                 }
               update_summary ();
             });
    connect (clear, &QPushButton::clicked, this, [this]
             {
               selected_.clear ();
               set_selected_entities (selected_);
             });

    update_summary ();
  }

  void FilterSettingsWidget::set_entities (QList<AD1CCty::Entity> const& entities)
  {
    // Rebuilding the rows fires itemChanged for each one, which would edit the
    // very list being restored.
    QSignalBlocker blocker {countries_};

    countries_->clear ();
    for (auto const& entity : entities)
      {
        auto * item = new QListWidgetItem {
          tr ("%1  (%2)").arg (entity.name).arg (entity.primary_prefix), countries_};
        item->setData (entity_role, entity.primary_prefix);
        item->setFlags (item->flags () | Qt::ItemIsUserCheckable);
        item->setCheckState (selected_.contains (entity.primary_prefix) ? Qt::Checked : Qt::Unchecked);
      }
    apply_search (search_->text ());
    update_summary ();
  }

  auto FilterSettingsWidget::mode () const -> CountryFilter::Mode
  {
    if (block_->isChecked ()) return CountryFilter::Block;
    if (only_->isChecked ()) return CountryFilter::Only;
    return CountryFilter::Off;
  }

  QStringList FilterSettingsWidget::selected_entities () const
  {
    return selected_;
  }

  void FilterSettingsWidget::set_mode (CountryFilter::Mode mode)
  {
    switch (mode)
      {
      case CountryFilter::Block: block_->setChecked (true); break;
      case CountryFilter::Only: only_->setChecked (true); break;
      default: off_->setChecked (true); break;
      }
  }

  void FilterSettingsWidget::set_selected_entities (QStringList const& entities)
  {
    selected_ = entities;
    selected_.removeAll (QString {});
    selected_.removeDuplicates ();

    QSignalBlocker blocker {countries_};
    for (int row = 0; row < countries_->count (); ++row)
      {
        auto * item = countries_->item (row);
        auto const prefix = item->data (entity_role).toString ();
        item->setCheckState (selected_.contains (prefix) ? Qt::Checked : Qt::Unchecked);
      }
    update_summary ();
  }

  void FilterSettingsWidget::apply_search (QString const& needle)
  {
    auto const trimmed = needle.trimmed ();
    for (int row = 0; row < countries_->count (); ++row)
      {
        auto * item = countries_->item (row);
        item->setHidden (!trimmed.isEmpty ()
                         && !item->text ().contains (trimmed, Qt::CaseInsensitive));
      }
  }

  void FilterSettingsWidget::update_summary ()
  {
    if (CountryFilter::Off == mode ())
      {
        summary_->setText (tr ("%n country(s) ticked. The filter is off, so nothing is hidden."
                               , "", selected_.size ()));
      }
    else if (selected_.isEmpty ())
      {
        // The alternative — an empty "show only" list — would hide every
        // decode on the band, which nobody has ever meant by it.
        summary_->setText (tr ("No countries ticked yet, so nothing is hidden."));
      }
    else
      {
        summary_->setText (CountryFilter::Block == mode ()
                           ? tr ("Hiding %n country(s).", "", selected_.size ())
                           : tr ("Showing %n country(s) and nothing else.", "", selected_.size ()));
      }
  }

  SiteSettingsWidget::SiteSettingsWidget (QWidget * parent)
    : QWidget {parent}
    , enabled_ {new QCheckBox {tr ("Send logged QSOs to uahamaward.com"), this}}
    , port_ {new QSpinBox {this}}
    , status_ {new QLabel {this}}
  {
    auto * explanation = new QLabel {
      tr ("Lets the award pages on uahamaward.com receive a contact the moment it is logged, "
          "whether you logged it yourself or auto-sequencing did.\n"
          "This replaces the separate bridge program: there is nothing else to download, and "
          "nothing else to keep up to date.\n"
          "With no log page open in a browser nothing is sent, and logging works exactly as it "
          "always does — the connection is a copy for somebody watching, never a step in "
          "logging a contact."), this};
    explanation->setWordWrap (true);

    // Below 1024 needs privileges on Unix, and the site only ever looks at
    // 8080 and the twenty ports above it.
    port_->setRange (1024, 65535);
    port_->setValue (8080);

    auto * form = new QFormLayout;
    form->addRow (tr ("WebSocket port:"), port_);

    auto * layout = new QVBoxLayout {this};
    layout->addWidget (enabled_);
    layout->addWidget (explanation);
    layout->addLayout (form);
    layout->addWidget (status_);
    layout->addStretch (1);

    connect (enabled_, &QCheckBox::toggled, port_, &QSpinBox::setEnabled);
    port_->setEnabled (enabled_->isChecked ());
  }

  bool SiteSettingsWidget::enabled () const {return enabled_->isChecked ();}
  quint16 SiteSettingsWidget::port () const {return static_cast<quint16> (port_->value ());}

  void SiteSettingsWidget::set_enabled (bool enabled)
  {
    enabled_->setChecked (enabled);
    port_->setEnabled (enabled);
  }

  void SiteSettingsWidget::set_port (quint16 port)
  {
    port_->setValue (port ? port : 8080);
  }

  void SiteSettingsWidget::show_status (QString const& text)
  {
    status_->setText (text);
  }

  namespace
  {
    //
    // The languages WSJT-X ships translations for, by the code L10nLoader
    // resolves. Each is named in its own language: somebody looking for their
    // own is not helped by finding it written in English.
    //
    // Kept beside the LANGUAGES list in CMakeLists.txt — a code here that is
    // not built there resolves to nothing and silently leaves the interface in
    // English.
    //
    struct Language
    {
      char const * code;
      char const * name;
    };

    Language const languages[] = {
      {"", QT_TRANSLATE_NOOP ("UaHam::LanguageSettingsWidget", "System default")},
      {"uk", "Українська"},
      {"en", "English"},
      {"ca", "Català"},
      {"da", "Dansk"},
      {"es", "Español"},
      {"hu", "Magyar"},
      {"it", "Italiano"},
      {"ja", "日本語"},
      {"zh", "中文"},
    };
  }

  LanguageSettingsWidget::LanguageSettingsWidget (QWidget * parent)
    : QWidget {parent}
    , language_ {new QComboBox {this}}
    , note_ {new QLabel {this}}
  {
    for (auto const& language : languages)
      {
        auto const label = *language.code
          ? QString::fromUtf8 (language.name)
          : tr ("System default");
        language_->addItem (label, QString::fromLatin1 (language.code));
      }

    note_->setWordWrap (true);
    note_->setText (tr (
        "Takes effect the next time WSJT-X starts: the interface language is "
        "chosen once, before any window exists.\n"
        "Anything not yet translated stays in English, and technical terms — "
        "mode names, units, the words on the air — are left alone on purpose."));

    auto * form = new QFormLayout;
    form->addRow (tr ("Interface language:"), language_);

    auto * layout = new QVBoxLayout {this};
    layout->addLayout (form);
    layout->addWidget (note_);
    layout->addStretch (1);
  }

  QString LanguageSettingsWidget::language () const
  {
    return language_->currentData ().toString ();
  }

  void LanguageSettingsWidget::set_language (QString const& language)
  {
    auto const at = language_->findData (language);
    // An unknown code — a hand-edited settings file, or a translation dropped
    // from a later build — falls back to the system default rather than
    // leaving the box showing a language that will not load.
    language_->setCurrentIndex (at < 0 ? 0 : at);
  }
}
