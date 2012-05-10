
#include <QtTest>

#include "program.h"

int program(int /*argc*/, char */*argv*/[])
{
  bool error = false;
  qDebug() << "Testing the program class...";

  MoleQueue::Program program;

//  program.setReplacement("input", "myInput.inp");
//  program.setReplacement("ncpus", "8");
  program.setRunTemplate("rungms $$input$$ 2010 $$ncpus$$");

  qDebug() << "Template: " << program.runTemplate();
//  qDebug() << "Expanded: " << program.expandedRunTemplate();

  MoleQueue::Program programCopy = program;
  qDebug() << "TemplateCopy: " << programCopy.runTemplate();
//  qDebug() << "ExpandedCopy: " << programCopy.expandedRunTemplate();
//  programCopy.setReplacement("input", "myOtherInput.inp");
  qDebug() << "Template: " << program.runTemplate();
//  qDebug() << "Expanded: " << program.expandedRunTemplate();
  qDebug() << "TemplateCopy: " << programCopy.runTemplate();
//  qDebug() << "ExpandedCopy: " << programCopy.expandedRunTemplate();

  return error ? 1 : 0;
}

// Work-around until this test is converted to a QtTest:
int main(int argc, char *argv[]) {return program(argc, argv);}
