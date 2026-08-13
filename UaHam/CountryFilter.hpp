// -*- Mode: C++ -*-
#ifndef UAHAM_COUNTRY_FILTER_HPP_
#define UAHAM_COUNTRY_FILTER_HPP_

#include <QSet>
#include <QString>
#include <QStringList>

namespace UaHam
{
  //
  // Which stations are allowed to reach the operator's screen, decided by
  // DXCC entity rather than by matching text in the message.
  //
  // WSJT-X already ships a Filters tab, and this is deliberately not part of
  // it. That one holds twelve fixed text boxes per list and compares them with
  // startsWith() against a word of the decoded message, which answers a
  // different question: it asks what the message looks like, not who sent it.
  // A hunter who wants "everything from Japan, nothing else" cannot express
  // that as twelve prefixes — JA, JE…JS, 7J…7N, 8J…8N is already more than
  // twelve, and it still lets a JA station using a compound callsign through.
  // cty.dat knows the answer exactly, so the filter asks cty.dat.
  //
  // An entity is named by its primary prefix ("UR", "JA", "K"), which is
  // cty.dat's own key for it. The internal entity ids are handed out in file
  // order and are renumbered by every cty.dat update, so they must never
  // reach a settings file.
  //
  class CountryFilter final
  {
  public:
    enum Mode
    {
      // The order is stored in settings and read back by index, so these
      // values are part of the file format: append, never reorder.
      Off = 0,      // every decode through, whatever the list holds
      Block = 1,    // hide the listed entities
      Only = 2,     // hide everything except the listed entities
    };

    // Reading a mode back from settings, where the value may be anything at
    // all — an older release, a hand-edited file, a truncated write. Anything
    // unrecognised means Off, because a filter that cannot be understood must
    // not silently start hiding a band.
    static Mode mode_from_int (int value)
    {
      switch (value)
        {
        case Block: return Block;
        case Only: return Only;
        default: return Off;
        }
    }

    void configure (Mode mode, QStringList const& primary_prefixes)
    {
      mode_ = mode;
      entities_ = QSet<QString> {primary_prefixes.begin (), primary_prefixes.end ()};
      entities_.remove (QString {});
    }

    Mode mode () const {return mode_;}
    int size () const {return entities_.size ();}

    // Whether the filter can hide anything at all. An empty list in Only mode
    // would hide every decode on the band, which is never what the operator
    // meant by switching the mode on and not yet choosing a country — so an
    // empty list disables the filter instead.
    bool active () const {return Off != mode_ && !entities_.isEmpty ();}

    //
    // The decision, for a station whose entity has already been looked up.
    //
    // primary_prefix is empty when cty.dat could not place the callsign: free
    // text, an unresolved <...> hash, a decode that came apart. In Block mode
    // such a station is shown, because there is no evidence it is one of the
    // blocked ones. In Only mode it is hidden, because there is no evidence it
    // is one of the wanted ones — the operator asked for a specific list and
    // everything else is the noise they were escaping.
    //
    bool hides (QString const& primary_prefix) const
    {
      if (!active ()) return false;
      auto const listed = !primary_prefix.isEmpty () && entities_.contains (primary_prefix);
      return Block == mode_ ? listed : !listed;
    }

    // How many decodes this filter has hidden since the counter was last
    // cleared. Shown in the main window, so that a quiet band is
    // distinguishable from a filter that is quietly eating everything.
    unsigned hidden_count () const {return hidden_count_;}
    void note_hidden () {++hidden_count_;}
    void reset_hidden_count () {hidden_count_ = 0;}

  private:
    Mode mode_ {Off};
    QSet<QString> entities_;
    unsigned hidden_count_ {0};
  };
}

#endif
