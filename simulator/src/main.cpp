#include "headers.h"
#include "cmdline.h"
#include "simulatorPro.h"
#include <fstream>
#include <iostream>
using namespace std;
using namespace cmdline;


parser Cmdline_Parser(int argc, char * argv[])
{
  parser option;
  option.add <string> ("input1", 'a', "Original Circuit file", false, "circuit/i2c_exact.blif");
  option.add <string> ("input2", 'b', "Approximate Circuit file", false, "circuit/i2c_approx.blif");
  option.add <string> ("file", 'f', "Error report file", false, "test.txt");
  option.add <string> ("mode", 'm', "Error Metric", false, "er");
  option.add <int> ("num", 'n', "num. of testing patterns", false, 18);
  option.add <int> ("batch", 'B', "num. of batches", false, 1);
  option.add <int> ("full", 'F', "Full simulation", false, 0);
  option.parse_check(argc, argv);
  return option;
}


int main(int argc, char * argv[])
{
  // command line parser
  parser option = Cmdline_Parser(argc, argv);
  string input1 = option.get <string> ("input1");
  string input2 = option.get <string> ("input2");
  string error_file = option.get <string> ("file");
  string mode_str = option.get<string>("mode");
  int nFrame = option.get <int> ("num");
  int nBatch = option.get <int> ("batch");
  int fFullSim = option.get <int> ("full");
  // initialize
  Abc_Start();
  Abc_Frame_t * pAbc1 = Abc_FrameGetGlobalFrame();
  ostringstream command1("");
  command1 << "read_blif " << input1;
  DASSERT(!Cmd_CommandExecute(pAbc1, command1.str().c_str()));
  Abc_Ntk_t* pNtk1 = Abc_NtkDup(Abc_FrameReadNtk(pAbc1));

  Abc_Frame_t* pAbc2 = Abc_FrameGetGlobalFrame();
  ostringstream command2("");
  command2 << "read_blif " << input2;
  DASSERT(!Cmd_CommandExecute(pAbc2, command2.str().c_str()));
  Abc_Ntk_t * pNtk2 = Abc_NtkDup(Abc_FrameReadNtk(pAbc2));

  random_device rd;
  clock_t st = clock();
  unsigned mode = 0;
  if (mode_str == "er")
    mode = 0;
  else if (mode_str == "med")
    mode = 1;
  else if (mode_str == "nmed")
    mode = 2;
  else if (mode_str == "rmed")
    mode = 3;
  else
    DASSERT(0);
  double* ErrorMetrics = MeasureBatchErrorMetrics(pNtk1, pNtk2, nBatch, 1 << nFrame, 0, fFullSim, mode);
  ofstream f_error;
  f_error.open(error_file.c_str());
  f_error << *ErrorMetrics << endl;
  f_error.close();

  // recycle memory
  free(ErrorMetrics);
  Abc_NtkDelete(pNtk1);
  Abc_NtkDelete(pNtk2);
  Abc_Stop();

  return 0;
}
