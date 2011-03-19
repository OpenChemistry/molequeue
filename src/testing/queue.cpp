
#include <iostream>

#include "program.h"
#include "queue.h"

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

int queue(int argc, char *argv[])
{
  bool error = false;
  cout << "Testing the queue class..." << endl;

  MoleQueue::Program gamess;
  gamess.setName("GAMESS");
  gamess.setReplacement("input", "myInput.inp");
  gamess.setReplacement("ncpus", "8");
  gamess.setRunTemplate("rungms $$input$$ 2010 $$ncpus$$");

  MoleQueue::Program gaussian;
  gaussian.setName("Gaussian");
  gaussian.setReplacement("input", "input.com");
  gaussian.setRunTemplate("gaussian $$input$$");

  MoleQueue::Queue queue;

  return error ? 1 : 0;
}
