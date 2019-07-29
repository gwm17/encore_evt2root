#include "ENCOREevt2root.h"
#include <TROOT.h>
#include <TApplication.h>
#include <string>
#include <iostream>
using namespace std;

int main(int argc, char* argv[]) {
  if(argc == 2) {
    TApplication app("app", &argc, argv);//if someone wants root graphics
    argv = app.Argv();
    evt2root converter;
    converter.run(argv[1]);
  } else {
    cout<<"Incorrect number of command line arguments!! Needs fullpath of rootfile"<<endl;
  }
}
