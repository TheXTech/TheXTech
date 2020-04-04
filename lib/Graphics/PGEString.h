/*
 * Used for compatibiltiy of common code with both QString and std::string
 */
#ifndef PGEString_HHH
#define PGEString_HHH
#pragma once

#ifdef PGE_EDITOR
#include <QString>
#include <QStringList>
#include <QList>
typedef QString     PGEString;
typedef QStringList PGEStringList;
#define PGEList     QList
#define StdToPGEString(str)     QString::fromStdString(str)
#define PGEStringToStd(str)     (str).toStdString()
#define PGEStringLit(str)       QStringLiteral(str)
typedef int pge_size_t;
#else
#include <string>
#include <vector>
typedef std::string                 PGEString;
typedef std::vector<std::string>    PGEStringList;
#define PGEList                     std::vector
#define StdToPGEString(str)         (str)
#define PGEStringToStd(str)         (str)
#define PGEStringLit(str)
typedef size_t pge_size_t;
#endif

template <class T>
inline const T &pgeConstReference(const T &t)
{
    return t;
}

#endif /*PGEString_HHH*/
