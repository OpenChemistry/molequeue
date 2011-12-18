
#include <QtTest>

#include <MoleQueue/program.h>
#include <MoleQueue/queue.h>

int queue(int argc, char *argv[])
{
  bool error = false;
  qDebug() << "Testing the queue class...";

  MoleQueue::Program *gamess = new MoleQueue::Program;
  gamess->setName("GAMESS");
//  gamess.setReplacement("input", "myInput.inp");
//  gamess.setReplacement("ncpus", "8");
  gamess->setRunTemplate("rungms $$input$$ 2010 $$ncpus$$");

  MoleQueue::Program *gaussian = new MoleQueue::Program;
  gaussian->setName("Gaussian");
//  gaussian.setReplacement("input", "input.com");
  gaussian->setRunTemplate("gaussian $$input$$");

  MoleQueue::Queue queue;
  if (!queue.addProgram(gamess)) {
    error = true;
    qDebug() << "Error adding the gamess program to the queue.";
  }
  if (!queue.addProgram(gaussian)) {
    error = true;
    qDebug() << "Error adding the gaussian program to the queue.";
  }
  QStringList programs = queue.programs();
  qDebug() << "Programs in queue: " << programs.join(" ");

  if (!queue.removeProgram("GAMESS")) {
    error = true;
    qDebug() << "Error removing the GAMESS program from the queue.";
  }

  foreach (const QString &name, programs)
    qDebug() << name;

  programs = queue.programs();
  qDebug() << "Programs in queue: " << programs.join(" ");

  return error ? 1 : 0;
}
