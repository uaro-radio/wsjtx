#include "StatusWidget.hpp"

#include <QFont>
#include <QFormLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QVBoxLayout>

#include "moc_StatusWidget.cpp"

namespace UaHam
{
  StatusWidget::StatusWidget (QWidget * parent)
    : QWidget {parent}
    , filter_mode_ {new QLabel {this}}
    , filter_hidden_ {new QLabel {this}}
    , site_state_ {new QLabel {this}}
    , site_sent_ {new QLabel {this}}
  {
    auto * form = new QFormLayout;
    form->setLabelAlignment (Qt::AlignRight);
    form->addRow (tr ("Country filter:"), filter_mode_);
    form->addRow (tr ("Decodes hidden:"), filter_hidden_);
    form->addRow (tr ("uahamaward.com:"), site_state_);
    form->addRow (tr ("QSOs sent:"), site_sent_);

    // Both figures count from the start of the session, so the button clears
    // them together — asking "is it working now" is one question, not two.
    auto * reset = new QPushButton {tr ("Reset counters"), this};
    connect (reset, &QPushButton::clicked, this, &StatusWidget::counters_reset);

    auto * buttons = new QHBoxLayout;
    buttons->addWidget (reset);
    buttons->addStretch (1);

    auto * layout = new QVBoxLayout {this};
    layout->addLayout (form);
    layout->addLayout (buttons);
    layout->addStretch (1);

    show_filter (tr ("off"), 0, 0);
    show_site (tr ("off"), 0);
  }

  void StatusWidget::show_filter (QString const& mode, int countries, unsigned hidden)
  {
    filter_mode_->setText (countries
                           ? tr ("%1 — %n country(s)", "", countries).arg (mode)
                           : mode);
    filter_hidden_->setText (QString::number (hidden));
  }

  void StatusWidget::show_site (QString const& state, unsigned qsos_sent)
  {
    site_state_->setText (state);
    site_sent_->setText (QString::number (qsos_sent));
  }
}
