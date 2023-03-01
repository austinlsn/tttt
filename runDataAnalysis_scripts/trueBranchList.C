#include <fstream>



int trueBranchList() {
  TFile *file = new TFile("/ceph/cms/store/group/tttt/Skims/230105/2018/DY_2l_M_50/DY_2l_M_50_1.root");
  TTree *tree = (TTree*)file->Get("Events");
  TObjArray *branches = tree->GetListOfBranches();

  ofstream myfile;
  myfile.open ("truBranch_names.txt");

  TIter next(branches);
  TBranch *branch;
  while ((branch = (TBranch*)next())) {
    myfile << branch->GetName() << endl;
  }

  myfile.close();

  return 0;
}
