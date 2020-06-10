/**
 * @file eb.h
 * @copyright 2020 Aardsoft Oy
 * @author Bernd Wachter <bwachter@aardsoft.fi>
 * @date 2020
 */

#ifndef _EB_H
#define _EB_H

#include <QtCore>

struct QmlFileContainer{
    Q_GADGET
    Q_PROPERTY(QString title MEMBER title)
    Q_PROPERTY(QString fileName MEMBER fileName)
  public:
    QString title;
    QString fileName;
    bool inDrawer;
};


inline QDataStream& operator<<(QDataStream& out, const QmlFileContainer& c) {
  out << c.title << c.fileName << c.inDrawer;
  return out;
}

inline QDataStream& operator>>(QDataStream& in, QmlFileContainer& c) {
  in >> c.title >> c.fileName >> c.inDrawer;
  return in;
}

Q_DECLARE_METATYPE(QmlFileContainer)

struct JsonDataContainer{
    Q_GADGET
    Q_PROPERTY(QString requesterId MEMBER requesterId)
    Q_PROPERTY(QString jsonData MEMBER jsonData)
  public:
    QString requesterId;
    QString jsonData;
};

inline QDataStream& operator<<(QDataStream& out, const JsonDataContainer& c) {
  out << c.requesterId << c.jsonData;
  return out;
}

inline QDataStream& operator>>(QDataStream& in, JsonDataContainer& c) {
  in >> c.requesterId >> c.jsonData;
  return in;
}

Q_DECLARE_METATYPE(JsonDataContainer)

#endif
