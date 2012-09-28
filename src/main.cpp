#include "qjsondocument.h"
#include "qjsonobject.h"
#include "qjsonvalue.h"
#include <QDebug>

int main(int argc, char *argv[])
{
  QJsonObject json;

  json.insert("jsonrpc", QJsonValue(QString("2.0")));

  QJsonObject params;
  params.insert("method", QString("submitJob"));
  params.insert("program", QString("GAMESS"));

  json.insert("params", params);
  json.insert("id", QJsonValue(42));

  QJsonDocument doc(json);
  qDebug() << doc.toJson();

  return 0;
}
