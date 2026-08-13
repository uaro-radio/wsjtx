#include "CallInfoWidget.hpp"

#include <QFont>
#include <QFormLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QVBoxLayout>

#include "moc_CallInfoWidget.cpp"

namespace UaHam
{
  CallInfoWidget::CallInfoWidget (QWidget * parent)
    : QWidget {parent}
    , callsign_ {new QLabel {this}}
    , name_ {new QLabel {this}}
    , location_ {new QLabel {this}}
    , country_ {new QLabel {this}}
    , grid_ {new QLabel {this}}
    , email_ {new QLabel {this}}
    , message_ {new QLabel {this}}
    , open_ {new QPushButton {tr ("Open on qrz.com"), this}}
  {
    auto bold = callsign_->font ();
    bold.setBold (true);
    callsign_->setFont (bold);

    // The address can be long and the window is not wide; letting these
    // labels choose their own width would push the whole tab sideways.
    for (auto * label : {callsign_, name_, location_, country_, grid_, email_, message_})
      {
        label->setTextInteractionFlags (Qt::TextSelectableByMouse);
        label->setWordWrap (true);
      }

    auto * form = new QFormLayout;
    form->setLabelAlignment (Qt::AlignRight);
    form->addRow (tr ("Callsign:"), callsign_);
    form->addRow (tr ("Name:"), name_);
    form->addRow (tr ("Location:"), location_);
    form->addRow (tr ("Country:"), country_);
    form->addRow (tr ("Grid:"), grid_);
    form->addRow (tr ("Email:"), email_);

    auto * buttons = new QHBoxLayout;
    buttons->addWidget (open_);
    buttons->addStretch (1);

    auto * layout = new QVBoxLayout {this};
    layout->addLayout (form);
    layout->addWidget (message_);
    layout->addLayout (buttons);
    layout->addStretch (1);

    connect (open_, &QPushButton::clicked, this, [this]
             {
               if (!current_.isEmpty ()) Q_EMIT open_requested (current_);
             });

    show_message (tr ("Enter a callsign in DX Call, or double-click a decode."));
  }

  void CallInfoWidget::awaiting (QString const& callsign)
  {
    current_ = callsign;
    callsign_->setText (callsign);
    for (auto * label : {name_, location_, country_, grid_, email_})
      {
        label->clear ();
      }
    show_message (tr ("Looking up %1…").arg (callsign));
  }

  void CallInfoWidget::show_record (QrzLookup::Record const& record)
  {
    current_ = record.callsign;
    callsign_->setText (record.callsign);
    name_->setText (record.name);

    QStringList where;
    if (!record.address.isEmpty ()) where << record.address;
    if (!record.town.isEmpty ()) where << record.town;
    if (!record.state.isEmpty ()) where << record.state;
    if (!record.postcode.isEmpty ()) where << record.postcode;
    location_->setText (where.join (", "));

    country_->setText (record.country);
    grid_->setText (record.grid);
    email_->setText (record.email);
    message_->clear ();
  }

  void CallInfoWidget::show_message (QString const& text)
  {
    message_->setText (text);
  }
}
