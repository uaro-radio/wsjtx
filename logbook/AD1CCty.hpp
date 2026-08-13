#ifndef AD1C_CTY_HPP_
#define AD1C_CTY_HPP_

#include <QObject>
#include <QDebug>
#include <QList>
#include <QString>
#include "pimpl_h.hpp"

class Configuration;

//
// AD1CCty  - Fast  access database  of Jim  Reisert, AD1C's,  cty.dat
// 						entity and entity override information file.
// 
class AD1CCty final
  : public QObject
{
  Q_OBJECT

public:
  //
  // Continent enumeration
  // 
  enum class Continent {UN, AF, AN, AS, EU, NA, OC, SA};
  static Continent continent (QString const& continent_id);
  static char const * continent (Continent);
  Q_ENUM (Continent)

  struct Record
  {
    explicit Record ();

    Continent continent;
    int CQ_zone;
    int ITU_zone;
    QString entity_name;
    bool WAE_only;
    float latitude;
    float longtitude;
    int UTC_offset;
    QString primary_prefix;
  };

  //
  // One DXCC entity, for callers that need the whole table rather than
  // the answer for a single call. The country filter's picker is the
  // only such caller today.
  //
  // The identity here is the primary prefix, not the internal id: ids are
  // handed out in the order cty.dat happens to list its entities, so every
  // update of that file renumbers them. A setting that remembered an id
  // would quietly start naming a different country.
  //
  struct Entity
  {
    QString primary_prefix;
    QString name;
    Continent continent;
    bool WAE_only;
  };

  explicit AD1CCty (Configuration const *);
  void reload(Configuration const * configuration);
  ~AD1CCty ();
  Record lookup (QString const& call) const;

  // Every entity cty.dat defines, ordered by name.
  QList<Entity> entities () const;

  QString version () const;
  Q_SIGNAL void cty_loaded (QString const& version) const;

  QString findState (QString const& grid) const;   //NJ0A

private:
  class impl;
  pimpl<impl> m_;
};

#if !defined (QT_NO_DEBUG_STREAM)
QDebug operator << (QDebug, AD1CCty::Record const&);
#endif

#endif
