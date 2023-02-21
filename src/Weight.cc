#include "Weight.h"

using namespace std;

void Weight::addSysts(vector<pair<string, string> > vSysts)
{
  cerr<<"### Weight.cc is opended ###"<<endl;

  for (pair<string, string> el : vSysts)
    addSyst (el.first, el.second);
}
