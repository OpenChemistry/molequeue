#include "qjsondocument.h"
#include "qjsonobject.h"
#include "qjsonvalue.h"
#include "qjsonarray.h"
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

  json["myKey"] = 5;
  json["myString"] = QLatin1String("Test");

  QJsonArray array;
  array.append(2);
  array.append(3);
  array.append(5);
  array.append(7);
  array.append(11);
  array.append(QString("Aaaarrrrgghhhhh"));
  json.insert("primes", array);

  QJsonDocument doc(json);
  qDebug() << doc.toJson();

  QJsonDocument doc2;
  doc2 = QJsonDocument::fromJson(doc.toJson());

  qDebug() << "Other JSON:\n" << doc2.toJson();

  return 0;
}
