
#include <iostream>

#include "program.h"

using std::cout;
using std::cerr;
using std::endl;

namespace {

template<typename A, typename B>
void checkResult(const A& result, const B& expected, bool &error)
{
  if (result != expected) {
    cerr << "Error, expected result " << expected << ", got " << result << endl;
    error = true;
  }
}

}

int program(int argc, char *argv[])
{
  bool error = false;
  cout << "Testing the program class..." << endl;

  MoleQueue::Program program;

  program.setReplacement("input", "myInput.inp");
  program.setReplacement("ncpus", "8");
  program.setRunTemplate("rungms $$input$$ 2010 $$ncpus$$");

  cout << "Template: " << program.runTemplate().toStdString() << endl;
  cout << "Expanded: " << program.expandedRunTemplate().toStdString() << endl;

  MoleQueue::Program programCopy = program;
  cout << "TemplateCopy: " << programCopy.runTemplate().toStdString() << endl;
  cout << "ExpandedCopy: " << programCopy.expandedRunTemplate().toStdString() << endl;
  programCopy.setReplacement("input", "myOtherInput.inp");
  cout << "Template: " << program.runTemplate().toStdString() << endl;
  cout << "Expanded: " << program.expandedRunTemplate().toStdString() << endl;
  cout << "TemplateCopy: " << programCopy.runTemplate().toStdString() << endl;
  cout << "ExpandedCopy: " << programCopy.expandedRunTemplate().toStdString() << endl;

  return error ? 1 : 0;
}
