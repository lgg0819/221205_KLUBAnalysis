// ntuple skimmer for analysis and synchronization

#include <iostream>
#include <string>
#include <fstream>
#include <sstream>
#include <bitset>
#include <map>
#include "TTree.h"
#include "TH1F.h"
#include "TH2F.h"
#include "TH3F.h"
#include "TFile.h"
#include "TBranch.h"
#include "TString.h"
#include "TLorentzVector.h"

// bigTree is produced on an existing ntuple as follows (see at the end of the file) 
#include "bigTree.h" 
#include "smallTree.h"
#include "OfflineProducerHelper.h"
#include "PUReweight.h"
#include "triggerReader_cross.h"
#include "bJetRegrVars.h"
#include "bTagSF.h"
#include "HHReweight5D.h"
#include "../../HHKinFit2/include/HHKinFitMasterHeavyHiggs.h"
#include "SFProvider.h"
#include "BDTfunctionsUtils.h"
#include "TauIDSFTool.h"

#include "lester_mt2_bisect.h"

#include "ScaleFactor.h"
#include "ConfigParser.h"
#include "EffCounter.h"
#include "exceptions/HHInvMConstraintException.h"
#include "exceptions/HHEnergyRangeException.h"
#include "exceptions/HHEnergyConstraintException.h"

#include "TMVA/MsgLogger.h"
#include "TMVA/Config.h"
#include "TMVA/Tools.h"
#include "TMVA/Factory.h"
#include "TMVA/Reader.h"

#include <boost/algorithm/string/replace.hpp>
#include <boost/format.hpp>
#include <Math/VectorUtil.h>

using namespace std ;

const double aTopRW = 0.0615;
const double bTopRW = -0.0005;
// const float DYscale_LL[3] = {8.72847e-01, 1.69905e+00, 1.63717e+00} ; // computed from fit for LL and MM b tag
// const float DYscale_MM[3] = {9.44841e-01, 1.29404e+00, 1.28542e+00} ;
//const float DYscale_LL[3] = {1.13604, 0.784789, 1.06947} ; // computed from fit for LL and MM b tag - to be updated for DY LO once the disagreement is fixed
//const float DYscale_MM[3] = {1.11219, 1.11436, 0.743777} ; // for now we use the same numbers computed with DY NLO sample
//const float DYscale_LL_NLO[3] = {1.13604, 0.784789, 1.06947} ; // computed from fit for LL and MM b tag for the DYNLO sample
//const float DYscale_MM_NLO[3] = {1.11219, 1.11436, 0.743777} ;
//const float DYscale_MM_NLO[3] = {1.03277, 1.03968, 0.742346} ;

// 2018 deepCSV
//const float DYscale_LL[3] = {0.760857,2.20159,0.886533} ; // computed from fit for LL and MM b tag - to be updated for DY LO once the disagreement is fixed
//const float DYscale_MM[3] = {0.860531,1.10466,1.49516} ; // for now we use the same numbers computed with DY NLO sample

// 2018 deepFlavor
const float DYscale_LL[3] = {0.748154,2.15445,1.63619} ; // for now we use the same numbers computed with DY NLO sample
const float DYscale_MM[3] = {0.862686,1.08509,1.10947} ; // for now we use the same numbers computed with DY NLO sample

// Computed from PI group for DY NLO binned
// - number of b-jets [0b, 1b, 2b]
// - pT(MuMu):
//   - < 20 GeV
//   - between 20 and 40 GeV
//   - between 40 and 100 GeV
//   - > 100 GeV
//loose PUjetID mtt
//const float DYscale_NLO_VLowPt[3] = {1.31303,  0.953595, 0.80987 };
//const float DYscale_NLO_LowPt [3] = {1.14463,  1.03926,  0.702278};
//const float DYscale_NLO_MedPt [3] = {1.02131,  1.23579,  0.554874};
//const float DYscale_NLO_HighPt[3] = {0.927168, 1.29899,  0.804798};
//no PUjetID mtt
//const float DYscale_NLO_VLowPt[3] = {1.46867,  0.5192, 0.978186 };
//const float DYscale_NLO_LowPt [3] = {1.21142,  0.986553,  0.68062};
//const float DYscale_NLO_MedPt [3] = {1.0527,  1.21448,  0.553397};
//const float DYscale_NLO_HighPt[3] = {0.939918, 1.29117,  0.799882};

//loose PUjetID mh
const float DYscale_NLO_VLowPt[3] = {1.13233,  1.01982,  0.711067};
const float DYscale_NLO_LowPt [3] = {1.03589,  0.978028, 0.845717};
const float DYscale_NLO_MedPt [3] = {0.972632,  1.17491, 0.554073};
const float DYscale_NLO_HighPt[3] = {0.904923, 1.31081,  0.686584};


// Computed from PI group for DY LO binned
// - number of b-jets [0b, 1b, 2b]
// - pT(MuMu):
//   - <= 10 GeV
//   - >10 and <=30
//   - >30 and <=50
//   - >50 and <=100
//   - >100 and <=200
//   - >200
//const float DYscale_LO_VLowPt [3] = {1.163, 0.719, 0.471};
//const float DYscale_LO_LowPt  [3] = {1.342,  1.13, 0.832};
//const float DYscale_LO_MedPt1 [3] = {1.244, 1.233, 0.908};
//const float DYscale_LO_MedPt2 [3] = {1.176, 1.251, 0.939};
//const float DYscale_LO_HighPt [3] = {1.053, 1.433, 0.838};
//const float DYscale_LO_VHighPt[3] = { 0.81, 1.723, 0.852};

//loose PUjetID, mtt
const float DYscale_LO_VLowPt [3] = {1.14855, 0.822656, 0.765073};
const float DYscale_LO_LowPt  [3] = {1.35142, 1.23179,  0.817443};
const float DYscale_LO_MedPt1 [3] = {1.25914, 1.25708,  0.933082};
const float DYscale_LO_MedPt2 [3] = {1.1834,  1.3345,   0.988823};
const float DYscale_LO_HighPt [3] = {1.06258, 1.43929,  0.974521};
const float DYscale_LO_VHighPt[3] = {0.786693,1.59505,  0.917797};


//loose PUjetID, mh
//const float DYscale_LO_VLowPt [3] = {1.03497,  1.2492,  0.512155};
//const float DYscale_LO_LowPt  [3] = {1.24255,  1.24875, 0.834341};
//const float DYscale_LO_MedPt1 [3] = {1.19378,  1.24953, 0.92162};
//const float DYscale_LO_MedPt2 [3] = {1.13619,  1.28016, 0.939999};
//const float DYscale_LO_HighPt [3] = {1.02876,  1.45217, 0.853559};
//const float DYscale_LO_VHighPt[3] = {0.801474, 1.68679, 0.878232};


/* NOTE ON THE COMPUTATION OF STITCH WEIGHTS:
** - to be updated at each production, using the number of processed events N_inclusive and N_njets for each sample
** - say f_i is the fraction of inclusive events in the i bin on njets (can be 2D nb-njet as well)
** = then sigma_i = f_i * sigmal_LO
** - stitchWeight (0jet) = f_0 / (f_0 * N_inclusive)
** - stitchWeight (njet) = f_n / (f_n * N_inclusive + N_njets)
*/ 

// const float stitchWeights [5] = {1.11179e-7, 3.04659e-9, 3.28633e-9, 3.48951e-9, 2.5776e-9} ; // weights DY stitch in njets, to be updated at each production (depend on n evtsn processed)
// const float stitchWeights [5] = {11.55916, 0.316751, 0.341677, 0.362801, 0.267991} ; // weights DY stitch in njets, to be updated at each production (depend on n evts processed)

// const float stitchWeights [5] = {2.01536E-08, 2.71202E-09, 2.92616E-09, 3.0373E-09, 2.38728E-09} ; // jet binned only, 27 giu 2016
// const float stitchWeights [][5] = {
//     {2.98077961089 , 0.0 , 0.0 , 0.0 , 0.0},
//     {0.400849633946 , 0.313302746388 , 0.0 , 0.0 , 0.0},
//     {0.434801486598 , 0.334010654578 , 0.102986214642 , 0.0 , 0.0},
//     {0.449060210108 , 0.342010066467 , 0.101739957862 , 0.100837020714 , 0.0},
//     {0.354615200387 , 0.285223034235 , 0.0977183487048 , 0.098552902997 , 0.0936281612454}
// }; // jet binned and b binned, 8 jul 2016

// const float stitchWeights [][5] = {
//     {2.97927051428 , 0.0 , 0.0 , 0.0 , 0.0},
//     {0.401700471936 , 0.313567146487 , 0.0 , 0.0 , 0.0},
//     {0.43376913912 , 0.33311444536 , 0.101563317164 , 0.0 , 0.0},
//     {0.44761355606 , 0.340870252304 , 0.10029914665 , 0.0994092045617 , 0.0},
//     {0.353436942964 , 0.284254291888 , 0.0963966329522 , 0.0972197079415 , 0.092337393936}
// };  // jet binned and b binned, 28 nov 2016

//const float stitchWeights [][5] = {
//  {2.96970591027 , 0.0 , 0.0 , 0.0 , 0.0},
//  {0.40848145146 , 0.33006603191 , 0.0 , 0.0 , 0.0},
//  {0.420075226373 , 0.337888056259 , 0.096536134169 , 0.0 , 0.0},
//  {0.431426212048 , 0.344817310665 , 0.0952166256522 , 0.094350931903 , 0.0},
//  {0.339954183508 , 0.284560899763 , 0.0915028373483 , 0.0922864405088 , 0.0874799674999}
//}; // jet binned and b binned, 07 Feb 2017

//const float stitchWeights [][5] = {
//  {1.51432918823 , 0.0 , 0.0 , 0.0 , 0.0},
//  {0.537021861953 , 0.537498571149 , 0.0 , 0.0 , 0.0},
//  {0.598363343318 , 0.598946235287 , 0.10509584187 , 0.0 , 0.0},
//  {0.947765704522 , 0.942420974822 , 0.109982447103 , 0.108336573578 , 0.0},
//  {1.51470061251 , 1.51281269465 , 0.117228562794 , 0.118913419351 , 0.109809154254}
//};// jet binned and b binned, 15 May 2018 

//const float stitchWeights [][5] = {
//  {1.50437157089 , 0.0 , 0.0 , 0.0 , 0.0},
//  {0.549757427781 , 0.550260336008 , 0.0 , 0.0 , 0.0},
//  {0.612466927142 , 0.613081681207 , 0.105177388196 , 0.0 , 0.0},
//  {0.951588541673 , 0.946165272042 , 0.109649575683 , 0.108005302201 , 0.0},
//  {0.707904938882 , 0.707489572546 , 0.107365495398 , 0.108784308592 , 0.101084909493}
//};// jet binned and b binned, 11 Jul 2018

const float stitchWeights [][5] = {
    {1.9437352898 , 0.0 , 0.0 , 0.0 , 0.0},
    {0.338279616574 , 0.338237399666 , 0.0 , 0.0 , 0.0},
    {0.703224977784 , 0.703022547668 , 0.0887784920115 , 0.0 , 0.0},
    {1.23750214201 , 1.23826181452 , 0.0929601982056 , 0.0921407650285 , 0.0},
    {1.91397145914 , 1.99919641957 , 0.1170483389 , 0.0973090477015 , 0.104743459131}
};// jet binned and b binned, 5 Mar 2019


// DY binned in 0j0b, 1j0b...
// each with it's PU weight and XS:
// total XS= 5765.4 pb
// myPUHisto_0j0b : xsFrac: 0.697167 - xs: 4019.4 pb
// myPUHisto_1j0b : xsFrac: 0.196537 - xs: 1133.1 pb
// myPUHisto_1j1b : xsFrac: 0.007292 - xs: 42.04  pb
// myPUHisto_2j0b : xsFrac: 0.063167 - xs: 364.18 pb
// myPUHisto_2j1b : xsFrac: 0.003200 - xs: 18.5   pb
// myPUHisto_2j2b : xsFrac: 0.001070 - xs: 6.17   pb
// myPUHisto_3j0b : xsFrac: 0.018860 - xs: 108.73 pb
// myPUHisto_3j1b : xsFrac: 0.000887 - xs: 5.11   pb
// myPUHisto_3j2b : xsFrac: 0.000704 - xs: 4.06   pb
// myPUHisto_3j3b : xsFrac: 9.78e-06 - xs: 0.056  pb
// myPUHisto_4j0b : xsFrac: 0.010130 - xs: 58.40  pb
// myPUHisto_4j1b : xsFrac: 0.000425 - xs: 2.45   pb
// myPUHisto_4j2b : xsFrac: 0.000535 - xs: 3.08   pb
// myPUHisto_4j3b : xsFrac: 1.12e-05 - xs: 0.064  pb
// myPUHisto_4j4b : xsFrac: 1.98e-06 - xs: 0.011  pb



// ---- ---- ---- ---- ---- ---- ---- ---- ---- ---- ---- ---- ---- ---- ---- ---- -
// open input txt file and append all the files it contains to TChain
void appendFromFileList (TChain* chain, TString filename)
{
  //cout << "=== inizio parser ===" << endl;
  std::ifstream infile(filename.Data());
  std::string line;
  while (std::getline(infile, line))
    {
      line = line.substr(0, line.find("#", 0)); // remove comments introduced by #
      while (line.find(" ") != std::string::npos) line = line.erase(line.find(" "), 1); // remove white spaces
      while (line.find("\n") != std::string::npos) line = line.erase(line.find("\n"), 1); // remove new line characters
      while (line.find("\r") != std::string::npos) line = line.erase(line.find("\r"), 1); // remove carriage return characters
      if (!line.empty()) // skip empty lines
	chain->Add(line.c_str());
    }
  return;
}

// ---- ---- ---- ---- ---- ---- ---- ---- ---- ---- ---- ---- ---- ---- ---- ---- -
// open the first file in the input list, retrieve the histogram "Counters" for the trigger names and return a copy of it
TH1F* getFirstFileHisto (TString filename, bool isForTriggers=true)
{
  std::ifstream infile(filename.Data());
  std::string line;
  while (std::getline(infile, line))
    {
      line = line.substr(0, line.find("#", 0)); // remove comments introduced by #
      while (line.find(" ") != std::string::npos) line = line.erase(line.find(" "), 1); // remove white spaces
      while (line.find("\n") != std::string::npos) line = line.erase(line.find("\n"), 1); // remove new line characters
      while (line.find("\r") != std::string::npos) line = line.erase(line.find("\r"), 1); // remove carriage return characters
      if (!line.empty()) // skip empty lines
	break;
    }
    
  TFile* fIn = TFile::Open (line.c_str());
  TH1F* dummy = (TH1F*) fIn->Get ("HTauTauTree/Counters");
  TString name = "Counters_perTrigger";
  if(!isForTriggers) {
    dummy = (TH1F*) fIn->Get ("HTauTauTree/TauIDs");
    name = "Counters_pertauID";
  }
  TH1F* histo = new TH1F (*dummy);
  histo-> SetDirectory(0);
  histo->SetName (name.Data());
  fIn->Close();
  return histo;
}

// --- --- --- --- --- --- --- --- --- --- --- --- --- --- --- --- --- --- --- ---
/**
   muons = 0
   electrons = 1
   taus = 2
*/

float getIso (unsigned int iDau, float pt, bigTree & theBigTree)
{
  int type = theBigTree.particleType->at (iDau) ;
  // is tauH
  if (type == 2)
    // return theBigTree.daughters_byCombinedIsolationDeltaBetaCorrRaw3Hits->at(iDau) ;
    //return theBigTree.daughters_byIsolationMVArun2v1DBoldDMwLTraw->at(iDau) ;
    //return theBigTree.daughters_byIsolationMVArun2017v2DBoldDMwLTraw2017->at(iDau) ; //FRA: update for 2017 data (94X)
    return theBigTree.daughters_byDeepTau2017v2p1VSjetraw->at(iDau) ; //Davide: update for 2018 data (102X)
  // muon
  if (type == 1 || type == 0)
    return theBigTree.combreliso->at(iDau);

  return -1 ;
}

// convert the 6 tau iso discriminators into an integer, from 0 == VLoose to 6 == VVTight
// example for MVA id (same for cut based):
// == 0 : NotIso
// >= 1 : pass VLoose
// >= 2 : pass Loose
// >= 3 : pass Medium
// >= 4 : pass Tigth
// >= 5 : pass VTight
// each number denotes the most stringent discriminator passed, so that selection candidates as:
// MVAiso >= 3 --> all candidates that are *at least* medium iso (or more isolated)
// MVAiso <= 3 --> sideband : all anti-isolated candidates, *not more* isolated than medium WP
int makeIsoDiscr (unsigned int iDau, vector<int>& nameToIdxMap, bigTree & theBigTree)
{
  int isoInt = 0;
  Long64_t tauID = theBigTree.tauID->at(iDau);
  while (isoInt < (int) nameToIdxMap.size())
    {
      int bit = nameToIdxMap.at(isoInt) ;
      bool pass = (((tauID >> bit) & 1 ) > 0);
      if (!pass) break; // will freeze isoInt to the previous value
      isoInt += 1;
    } 
  return isoInt;
}

int getTauIDIdx (TH1F* h_tauID, string tauIDName)
{
  int ibin = -1;
  for (int i = 1; i <= h_tauID->GetNbinsX(); ++i)
    {
      string binlabel = h_tauID->GetXaxis()->GetBinLabel(i);
      if (tauIDName == binlabel)
      {
        ibin = i;
        break;
      }
    }
  return ibin-1;
}

// ---- ---- ---- ---- ---- ---- ---- ---- ---- ---- ---- ----

bool CheckBit (int number, int bitpos)
{
  bool res = number & (1 << bitpos);
  return res;
}

// ---- ---- ---- ---- ---- ---- ---- ---- ---- ---- ---- ----
// check that the VBF jets are correctly matched to different TrigerObjects
bool checkVBFjetMatch(bool DEBUG, int iJet, int kJet, bigTree & theBigTree)
{
  bool goodmatch= true;

  Long64_t jet1lead = theBigTree.jets_VBFleadFilterMatch   ->at(iJet);
  Long64_t jet1subl = theBigTree.jets_VBFsubleadFilterMatch->at(iJet);
  Long64_t jet2lead = theBigTree.jets_VBFleadFilterMatch   ->at(kJet);
  Long64_t jet2subl = theBigTree.jets_VBFsubleadFilterMatch->at(kJet);

  if (DEBUG)
  {
    cout << "---   checking VBF jet legs   --- " << endl;
    cout << "jet "<< iJet << " - leading    : " << std::bitset<64>(jet1lead) << endl;
    cout << "jet "<< iJet << " - subleading : " << std::bitset<64>(jet1subl) << endl;
    cout << "jet "<< kJet << " - leading    : " << std::bitset<64>(jet2lead) << endl;
    cout << "jet "<< kJet << " - subleading : " << std::bitset<64>(jet2subl) << endl;
    cout << "XOR leading       : " << (jet1lead^jet2lead) << endl;
    cout << "XOR subleading    : " << (jet1subl^jet2subl) << endl;
    if ( (jet1subl^jet2subl) == 0 ) cout << " -- same TO for the subleading jets" << endl;
    if ( (jet1lead^jet2lead) == 0 ) cout << " -- same TO for the leading jets" << endl;
    cout << "--- end checking VBF jet legs --- " << endl;
  }

  bool firstJetZero  = (jet1lead==0 && jet1subl==0);
  bool secondJetZero = (jet2lead==0 && jet2subl==0);
  if (firstJetZero || secondJetZero) goodmatch = false; //one of the two jets is not matched to any filter
  if ( (jet1subl^jet2subl) == 0 )    goodmatch = false; //two jets are matched to the same TrigObj (subleading filter)
  if ( (jet1lead^jet2lead) == 0 )    goodmatch = false; //two jets are matched to the same TrigObj (subleading filter)

  return goodmatch;
}

// ---- ---- ---- ---- ---- ---- ---- ---- ---- ---- ---- ----

// implement operator < for b tag . first : CSV score ;  second : index
bool bJetSort (const pair<float, int>& i, const pair<float, int>& j) {
  if (i.first != j.first) return (i.first < j.first) ; // lowest CVS
  return i.second > j.second ; // highest index == lowest pt
}

// ---- ---- ---- ---- ---- ---- ---- ---- ---- ---- ---- ---- ---- ---- ---- ---- -


float deltaPhi (float phi1, float phi2)
{
  float deltaphi = fabs (phi1 - phi2) ;
  if (deltaphi > 6.283185308) deltaphi -= 6.283185308 ;
  if (deltaphi > 3.141592654) deltaphi = 6.283185308 - deltaphi ;
  return deltaphi ;
}

// ---- ---- ---- ---- ---- ---- ---- ---- ---- ---- ---- ----
float getZ (float eta, float eta1, float eta2)
{
  float Z  =  (eta - 0.5*(eta1 + eta2)) / fabs (eta1 - eta2) ;
  return Z;
}

// ---- ---- ---- ---- ---- ---- ---- ---- ---- ---- ---- ----

vector<int> findSubjetIdxs (unsigned int iFatJet, bigTree & theBigTree)
{
  vector<int> indexes;
  int idxFatJet = (int) iFatJet;
  for (unsigned int isj = 0; isj < theBigTree.subjets_ak8MotherIdx->size(); isj++)
    {
      if (theBigTree.subjets_ak8MotherIdx->at(isj) == idxFatJet)
	indexes.push_back(isj);
    }
  return indexes;
}


// ---- ---- ---- ---- ---- ---- ---- ---- ---- ---- ---- ---- ---- ---- ---- ---- -
// get shifted MET
TVector2 getShiftedMET (float shift, TVector2 MET, bigTree & theBigTree, bool DEBUG=false)
{
  double corrMETx = MET.Px();
  double corrMETy = MET.Py();

  if (DEBUG) cout << "*********** DEBUGGING JETS *********** "<< endl;

  for (unsigned int iJet = 0 ; iJet < theBigTree.jets_px->size () ; ++iJet)
  {
    // if (theBigTree.PFjetID->at (iJet) < 1) continue;
    // build original jet
    TLorentzVector tlv_jet (theBigTree.jets_px->at(iJet), theBigTree.jets_py->at(iJet), theBigTree.jets_pz->at(iJet), theBigTree.jets_e->at(iJet));

    // get uncertainty
    double unc = theBigTree.jets_jecUnc->at(iJet);

    // build shifted jet
    TLorentzVector tlv_jet_shifted = tlv_jet;
    tlv_jet_shifted.SetPtEtaPhiE(
        (1.+shift*unc) * tlv_jet.Pt(),
        tlv_jet.Eta(),
        tlv_jet.Phi(),
        (1.+shift*unc) * tlv_jet.E()
        );

    // shift MET - first the original jet
    corrMETx += tlv_jet.Px();
    corrMETy += tlv_jet.Py();

    // shift MET - then the shifted jet
    corrMETx -= tlv_jet_shifted.Px();
    corrMETy -= tlv_jet_shifted.Py();

    // Debug printing
    if (DEBUG)
    {
      cout << " Jet "   << setw(1) << left << iJet
      << " - pt: "      << setw(7) << left << tlv_jet.Pt()
      << " - eta: "     << setw(9) << left << tlv_jet.Eta()
      << " - sf: "      << setw(8) << left << (1.+shift*unc)
      << " - unc: "     << setw(10) << left << unc
      << " - pfjetID: " << setw(2) <<left << theBigTree.PFjetID->at(iJet) << endl;
    }
  }

  TVector2 shiftedMET(corrMETx, corrMETy);

  if (DEBUG) cout << "*********** ************** *********** "<< endl;

  return shiftedMET;
}

// ---- ---- ---- ---- ---- ---- ---- ---- ---- ---- ---- ---- ---- ---- ---- ---- -
// turn on function for trigger reweight

float turnOnCB(float x, float m0, float sigma, float alpha, float n, float norm)
{
  float sqrtPiOver2 = TMath::Sqrt(TMath::PiOver2());
  float sqrt2 = TMath::Sqrt(2.);
  float sig = abs(sigma);
  float t = (x - m0)/sig * alpha / abs(alpha);
  float absAlpha = abs(alpha/sig);
  float a = TMath::Power(n/absAlpha, n) * TMath::Exp(-0.5 * absAlpha * absAlpha);
  float b = absAlpha - n/absAlpha;
  float arg = absAlpha / sqrt2;
  float ApproxErf = TMath::Erf(arg);
  if (arg > 5.) ApproxErf = 1.;
  else if(arg < -5.) ApproxErf = -1.;
  float leftArea = (1. + ApproxErf) * sqrtPiOver2;
  float rightArea = ( a * 1./TMath::Power(absAlpha-b, n-1) ) / (n-1);
  float area = leftArea + rightArea;
  if (t <= absAlpha){
    arg = t / sqrt2;
    if (arg > 5.) ApproxErf = 1.;
    else {
      if(arg < -5.) ApproxErf = -1.;
      else ApproxErf = TMath::Erf(arg);
    }
    return norm * (1 + ApproxErf) * sqrtPiOver2 / area;
  }
  return norm * (leftArea + a * (1/TMath::Power(t-b,n-1) - 1/TMath::Power(absAlpha - b,n-1)) / (1-n)) / area;
}

// WP is 0 : Noiso , 1 : VLoose, 2: Loose, 3: Medium , 4: Tight 5: Vtight 6: VVtight
float turnOnSF(float pt, int WP, bool realTau)
{
  //return 1.0/turnOnCB(pt,3.60274e+01,5.89434e+00,5.82870e+00,1.83737e+00,9.58000e-01)*turnOnCB(pt,3.45412e+01,5.63353e+00,2.49242e+00,3.35896e+00,1);
  //return turnOnCB(pt,3.45412e+01,5.63353e+00,2.49242e+00,3.35896e+00,1);
  // return turnOnCB(pt,3.60274e+01,5.89434e+00,5.82870e+00,1.83737e+00,9.58000e-01);

  float real_m0    [7] = {3.86506E+01 , 3.86057E+01 , 3.85953E+01 , 3.81821E+01 , 3.81919E+01 , 3.77850E+01 , 3.76157E+01} ;
  float real_sigma [7] = {5.81155E+00 , 5.77127E+00 , 5.74632E+00 , 5.33452E+00 , 5.38746E+00 , 4.93611E+00 , 4.76127E+00} ;
  float real_alpha [7] = {5.82783E+00 , 5.61388E+00 , 5.08553E+00 , 4.42570E+00 , 4.44730E+00 , 4.22634E+00 , 3.62497E+00} ;
  float real_n     [7] = {3.38903E+00 , 3.77719E+00 , 5.45593E+00 , 4.70512E+00 , 7.39646E+00 , 2.85533E+00 , 3.51839E+00} ;
  float real_norm  [7] = {9.33449E+00 , 9.30159E-01 , 9.42168E-01 , 9.45637E-01 , 9.33402E-01 , 9.92196E-01 , 9.83701E-01} ;

  float fake_m0    [7] = {4.03919E+01 , 3.99115E+01 , 3.94747E+01 , 3.92674E+01 , 3.90677E+01 , 3.92867E+01 , 3.89518E+01} ;
  float fake_sigma [7] = {7.55333E+00 , 7.32760E+00 , 7.23546E+00 , 7.17092E+00 , 7.03152E+00 , 7.22249E+00 , 6.69525E+00} ;
  float fake_alpha [7] = {1.20102E+01 , 1.17174E+01 , 1.08089E+01 , 1.10546E+01 , 1.11690E+01 , 1.14726E+01 , 9.86033E+00} ;
  float fake_n     [7] = {1.26661E+00 , 1.26857E+00 , 1.33930E+00 , 1.31852E+00 , 1.29314E+00 , 1.32792E+00 , 1.39875E+00} ;
  float fake_norm  [7] = {1.00000E+00 , 1.00000E+00 , 1.00000E+00 , 1.00000E+00 , 9.99999E-01 , 1.00000E+00 , 1.00000E+00} ;

  if (realTau) return turnOnCB (pt, real_m0[WP], real_sigma[WP], real_alpha[WP], real_n[WP], real_norm[WP] );  
  else         return turnOnCB (pt, fake_m0[WP], fake_sigma[WP], fake_alpha[WP], fake_n[WP], fake_norm[WP] );  
}

// generic function to read content of 1D / 2D histos, taking care of x axis limit (no under/over flow)
double getContentHisto1D(TH1* histo, double x)
{
  int nbinsx = histo->GetNbinsX();
  int ibin = histo->FindBin(x);
  if (ibin == 0) ibin = 1;
  if (ibin > nbinsx) ibin = nbinsx;
  return histo->GetBinContent(ibin);
}

double getContentHisto2D(TH2* histo, double x, double y)
{
  int nbinsx = histo->GetNbinsX();
  int nbinsy = histo->GetNbinsY();
  int ibinx = histo->GetXaxis()->FindBin(x);
  int ibiny = histo->GetYaxis()->FindBin(y);

  if (ibinx == 0)     ibinx = 1;
  if (ibinx > nbinsx) ibinx = nbinsx;

  if (ibiny == 0)     ibiny = 1;
  if (ibiny > nbinsy) ibiny = nbinsy;
  
  return histo->GetBinContent(ibinx, ibiny);
}

double getContentHisto3D(TH3* histo, double x, double y, double z)
{
  int nbinsx = histo->GetNbinsX();
  int nbinsy = histo->GetNbinsY();
  int nbinsz = histo->GetNbinsZ();

  int ibinx = histo->GetXaxis()->FindBin(x);
  int ibiny = histo->GetYaxis()->FindBin(y);
  int ibinz = histo->GetZaxis()->FindBin(z);

  if (ibinx == 0)     ibinx = 1;
  if (ibinx > nbinsx) ibinx = nbinsx;

  if (ibiny == 0)     ibiny = 1;
  if (ibiny > nbinsy) ibiny = nbinsy;

  if (ibinz == 0)     ibinz = 1;
  if (ibinz > nbinsz) ibinz = nbinsz;

  return histo->GetBinContent(ibinx, ibiny, ibinz);
}

// ---- ---- ---- ---- ---- ---- ---- ---- ---- ---- ---- ---- ---- ---- ---- ---- -
// ---- ---- ---- ---- ---- ---- ---- ---- ---- ---- ---- ---- ---- ---- ---- ---- -
// ---- ---- ---- ---- ---- ---- ---- ---- ---- ---- ---- ---- ---- ---- ---- ---- -


int main (int argc, char** argv)
{
  // read input file and cfg
  // ---- ---- ---- ---- ---- ---- ---- ---- ---- ---- ---- ---- ---- ----

  if (argc < 22)
    { 
      cerr << "missing input parameters : argc is: " << argc << endl ;
      cerr << "usage: " << argv[0]
           << " inputFileNameList outputFileName crossSection isData configFile runHHKinFit"
           << " xsecScale(stitch) HTMax(stitch) HTMin(stitch) isTTBar DY_Nbs HHreweightFile TT_stitchType"
           << " runMT2 isHHsignal NjetRequired(stitch) kl_rew kt_rew c2_rew cg_rew c2g_rew susyModel" << endl ; 
      return 1;
    }

  TString inputFile = argv[1] ;
  TString outputFile = argv[2] ;
  cout << "** INFO: inputFile  : " << inputFile << endl;
  cout << "** INFO: outputFile : " << outputFile << endl;
  
  float XS = atof (argv[3]) ;  
  bool isMC = true;
  int isDatabuf = atoi (argv[4]);
  if (isDatabuf == 1)
    {
      isMC = false; 
      XS = 1.;
    }
  cout << "** INFO: x-section: " << XS << endl;
  cout << "** INFO: is it MC?  " << isMC << endl;

  if (gConfigParser) return 1 ;
  gConfigParser = new ConfigParser () ;  
  TString config ; 
  config.Form ("%s",argv[5]) ;
  cout << "** INFO: reading config : " << config << endl;

  bool runHHKinFit = false;
  string opt7 (argv[6]);
  if (opt7 == "1") runHHKinFit = true;
  cout << "** INFO: running HHKinFit: " << runHHKinFit << endl;

  float xsecScale = 1.0;
  xsecScale = atof (argv[7]);
  cout << "** INFO: xsec scaled by: " << xsecScale << endl;
  XS = XS*xsecScale;

  float HTMax = -999.0;
  HTMax = atof(argv[8]);
  cout << "** INFO: removing HT < " << HTMax << " [-999 means no removal]" << endl;

  float HTMin = -999.0;
  HTMin = atof(argv[9]);
  cout << "** INFO: removing HT > " << HTMin << " [-999 means no removal]" << endl;

  int isTTBarI = atoi(argv[10]);
  bool isTTBar = (isTTBarI == 1) ? true : false;
  if (!isMC) isTTBar = false; // force it, you never know...
  cout << "** INFO: is this a TTbar sample? : " << isTTBar << endl;

  bool DY_Nbs = false; // run on genjets to count in DY samples the number of b jets
  bool DY_tostitch = false;
  int I_DY_Nbs = atoi(argv[11]);
  if (I_DY_Nbs == 1)
    {
      DY_Nbs = true; 
      DY_tostitch = true; // FIXME!! this is ok only if we use jet binned samples
    }
  cout << "** INFO: loop on gen jet to do a b-based DY split? " << DY_Nbs << " " << DY_tostitch << endl;

  TFile* HHreweightFile = 0;
  TString doreweight = argv[12];
  cout << "** INFO: reweightin file for non-resonant hh is: " << doreweight << " [ 0 for no reweghting done ]" << endl;
  if (doreweight != TString("0"))
    HHreweightFile = new TFile (doreweight);

  int TT_stitchType = atoi(argv[13]);
  if (!isTTBar) TT_stitchType = 0; // just force if not TT...
  cout << "** INFO: TT stitch type: " << TT_stitchType << " [0: no stitch , 1: fully had, 2: semilept t, 3: semilept tbar, 4: fully lept, 5: semilept all]" << endl;

  bool runMT2 = false;
  string opt14 (argv[14]);
  if (opt14 == "1") runMT2 = true;
  cout << "** INFO: running MT2: " << runMT2 << endl;

  bool isHHsignal = false;
  string opt15 (argv[15]);
  if (opt15 == "1") isHHsignal = true;
  cout << "** INFO: is HH signal: " << isHHsignal << endl;

  int NjetRequired = atoi(argv[16]);
  cout << "** INFO: requiring exactly " << NjetRequired << " outgoing partons [<0 for no cut on this]" << endl;

  float kl_rew = atof(argv[17]);
  float kt_rew = atof(argv[18]);
  float c2_rew = atof(argv[19]);
  float cg_rew = atof(argv[20]);
  float c2g_rew = atof(argv[21]);
  cout << "** INFO: kl, kt reweight " << kl_rew << " " << kt_rew << " [kt < -990 || kl < -990 : no HH reweight]" << endl;
  cout << "**       c2, cg, c2g reweight " << c2_rew << " " << cg_rew << " " << c2g_rew << " [if any is < -990: will do only a klambda / kt reweight if requested]" << endl;

  string susyModel = argv[22];
  cout << "** INFO: requesting SUSY model to be: -" << susyModel << "- [NOTSUSY: no request on this parameter]" << endl;

  // external weight file for PUreweight - sample per sample
  TString PUreweightFile = argv[23];
  cout << "** INFO: PU reweight external file: " << PUreweightFile << endl;
  
  int DY_nJets  = atoi(argv[24]);
  int DY_nBJets = atoi(argv[25]);
  cout << "** INFO: nJets/nBjets for DY bin weights: " << DY_nJets << " / " << DY_nBJets << endl;
  int isDYI = atoi(argv[26]);
  bool isDY = (isDYI == 1) ? true : false;
  // ------------------  decide what to do for the reweight of HH samples
  enum HHrewTypeList {
    kNone      = 0,
    kFromHisto = 1,
    kDynamic   = 2
  };
  int HHrewType = kNone; // default is no reweight
  if (HHreweightFile && kl_rew >= -990 && kt_rew >= -990) {
    cout << "** WARNING: you required both histo based and dynamic reweight, cannot do both at the same time. Will set histo" << endl;
    HHrewType = kFromHisto;
  }
  else if (HHreweightFile)
    HHrewType = kFromHisto;
  else if (kl_rew >= -990 && kt_rew >= -990)
    HHrewType = kDynamic;
  cout << "** INFO: HH reweight type is " << HHrewType << " [ 0: no reweight, 1: from histo, 2: dynamic ]" << endl;



  // prepare variables needed throughout the code
  // ---- ---- ---- ---- ---- ---- ---- ---- ---- ---- ---- ---- ---- ----    
  if (! (gConfigParser->init (config)))
    {
      cout << ">>> parseConfigFile::Could not open configuration file " << config << endl ;
      return -1 ;
    }

  bool   beInclusive         = gConfigParser->readBoolOption   ("selections::beInclusive") ;
  bool   useDeepFlavor       = gConfigParser->readBoolOption   ("selections::useDeepFlavor") ;
  float  PUjetID_WP          = gConfigParser->readFloatOption  ("parameters::PUjetIDWP") ;
  float  PFjetID_WP          = gConfigParser->readIntOption    ("parameters::PFjetIDWP") ;
  // int    saveOS              = gConfigParser->readIntOption    ("parameters::saveOS") ;
  float  lepCleaningCone     = gConfigParser->readFloatOption  ("parameters::lepCleaningCone") ;
  int    bChoiceFlag         = gConfigParser->readFloatOption  ("parameters::bChoiceFlag") ;
  int    PUReweight_MC       = gConfigParser->readFloatOption  ("parameters::PUReweightMC") ; 
  int    PUReweight_target   = gConfigParser->readFloatOption  ("parameters::PUReweighttarget") ; 
  string leptonSelectionFlag = gConfigParser->readStringOption ("parameters::lepSelections") ;
  int maxNjetsSaved          = gConfigParser->readIntOption    ("parameters::maxNjetsSaved") ;
  
  enum sortingStrategy {
    kLLRFramDefault = 0, // taking order from LLR framework <--> ordered by MVA ID
    kHTauTau = 1,        // using HTauTau of lowest iso on 1st candidate, including (A,B) and (B,A)
    kPtAndRawIso = 2     // order each pair leg by pt (ptA > ptB), then compare *raw* iso of first leg
  };

  int sortStrategyThTh = 0;
  if (gConfigParser->isDefined("parameters::pairStrategy"))
    {
      sortStrategyThTh = gConfigParser->readIntOption("parameters::pairStrategy");
    }
  cout << "** INFO: thth sorting strategy? [0: kLLRFramDefault, 1: kHTauTau, 2: kPtAndRawIso]" << sortStrategyThTh << endl;

  ULong64_t debugEvent = -1; // will be converted to numerical max, and never reached
  if (gConfigParser->isDefined("parameters::debugEvent"))
    debugEvent = (ULong64_t) gConfigParser->readIntOption("parameters::debugEvent");


  vector<string> trigMuTau   =  (isMC ? gConfigParser->readStringListOption ("triggersMC::MuTau")  : gConfigParser->readStringListOption ("triggersData::MuTau")) ;
  vector<string> trigTauTau  =  (isMC ? gConfigParser->readStringListOption ("triggersMC::TauTau") : gConfigParser->readStringListOption ("triggersData::TauTau")) ;
  vector<string> trigEleTau  =  (isMC ? gConfigParser->readStringListOption ("triggersMC::EleTau") : gConfigParser->readStringListOption ("triggersData::EleTau")) ;
  // vector<string> trigEleMu   =  (isMC ? gConfigParser->readStringListOption ("triggersMC::EleMu")  : gConfigParser->readStringListOption ("triggersData::EleMu")) ;
  //I didn't store MuMu and I don't care for eleele
  vector<string> trigEleEle  =  (isMC ? gConfigParser->readStringListOption ("triggersMC::EleEle")  : gConfigParser->readStringListOption ("triggersData::EleEle")) ;
  vector<string> trigMuMu    =  (isMC ? gConfigParser->readStringListOption ("triggersMC::MuMu")  : gConfigParser->readStringListOption ("triggersData::MuMu")) ;
  // cross triggers for muTau and eleTau
  vector<string> crossTrigTauTau = (isMC ? gConfigParser->readStringListOption ("triggersMC::crossTauTau") : gConfigParser->readStringListOption ("triggersData::crossTauTau")) ;
  vector<string> crossTrigMuTau  = (isMC ? gConfigParser->readStringListOption ("triggersMC::crossMuTau")  : gConfigParser->readStringListOption ("triggersData::crossMuTau") ) ;
  vector<string> crossTrigEleTau = (isMC ? gConfigParser->readStringListOption ("triggersMC::crossEleTau") : gConfigParser->readStringListOption ("triggersData::crossEleTau")) ;
  vector<string> vbfTriggers     = (isMC ? gConfigParser->readStringListOption ("triggersMC::vbfTriggers") : gConfigParser->readStringListOption ("triggersData::vbfTriggers")) ;

  // bool applyTriggers = isMC ? false : true; // true if ask triggerbit + matching, false if doing reweight
  //bool applyTriggers = isMC ? gConfigParser->readBoolOption ("parameters::applyTriggersMC") : true; // true if ask triggerbit + matching, false if doing reweight

  bool applyTriggers = gConfigParser->readBoolOption ("parameters::applyTriggersMC") ;

  // applyTriggers = true;
  cout << "** INFO: apply triggers? " << applyTriggers << " [ 0: reweight , 1: triggerbit+matching ]" << endl;
  if (applyTriggers)
    {
      cout << "** INFO: triggers used in the skim : " << endl;
    
      cout << "  @ MuTau" << endl; cout << "   --> ";
      for (unsigned int i = 0 ; i < trigMuTau.size(); i++) cout << "  " << trigMuTau.at(i);
      cout << endl;

      cout << "  @ EleTau" << endl; cout << "   --> ";
      for (unsigned int i = 0 ; i < trigEleTau.size(); i++) cout << "  " << trigEleTau.at(i);
      cout << endl;

      cout << "  @ TauTau" << endl; cout << "   --> ";
      for (unsigned int i = 0 ; i < trigTauTau.size(); i++) cout << "  " << trigTauTau.at(i);
      cout << endl;

      cout << "  @ crossMuTau" << endl; cout << "   --> ";
      for (unsigned int i = 0 ; i < crossTrigMuTau.size(); i++) cout << "  " << crossTrigMuTau.at(i);
      cout << endl;

      cout << "  @ crossEleTau" << endl; cout << "   --> ";
      for (unsigned int i = 0 ; i < crossTrigEleTau.size(); i++) cout << "  " << crossTrigEleTau.at(i);
      cout << endl;
      
      cout << "  @ crossTauTau" << endl; cout << "   --> ";
      for (unsigned int i = 0 ; i < crossTrigTauTau.size(); i++) cout << "  " << crossTrigTauTau.at(i);
      cout << endl;

      cout << "  @ vbfTriggers" << endl; cout << "   --> ";
      for (unsigned int i = 0 ; i < vbfTriggers.size(); i++) cout << "  " << vbfTriggers.at(i);
      cout << endl;
    }

  //string bRegrWeights("");
  //bool computeBregr = gConfigParser->readBoolOption ("bRegression::computeBregr");
  //if (computeBregr) bRegrWeights = gConfigParser->readStringOption("bRegression::weights");

  //cout << "** INFO: computing b jet regression? " << computeBregr << " with weights " << bRegrWeights << endl;

  // DY new reweight (LO to NLO) map - Fractional Weight, Pt Weight and SF Weight maps
  bool doDYLOtoNLOreweight = (gConfigParser->isDefined("DYLOtoNLOreweight::doReweight") ? gConfigParser->readBoolOption ("DYLOtoNLOreweight::doReweight") : false);
  if (!isDY) doDYLOtoNLOreweight = false; //skip DYLOtoNLOreweight if the sample is not DY
  cout << "** INFO: apply DY LO to NLO reweight? " << doDYLOtoNLOreweight << endl;
  map <string, double> fractional_weight_map;
  std::map<std::string, TH1D*> pt_weight_histo_map;
  std::map<std::string,double> scale_factor_map;
  if (doDYLOtoNLOreweight)
  {
    // Fractional weight map
    fractional_weight_map["0Jet"]       = 0.93;
    fractional_weight_map["1Jet_0bJet"] = 1.02;
    fractional_weight_map["1Jet_1bJet"] = 1.38;
    fractional_weight_map["2Jet_0bJet"] = 0.99;
    fractional_weight_map["2Jet_1bJet"] = 1.15;
    fractional_weight_map["2Jet_2bJet"] = 1.39;

    // LO to NLO pt file and map
    TString NLO_weight_file_name = gConfigParser->readStringOption("DYLOtoNLOreweight::inputFile");
    TFile* NLO_weight_file = new TFile (NLO_weight_file_name);
    cout << "** INFO: apply DY LO to NLO reweight with NLOfile: " << NLO_weight_file_name << endl;

    pt_weight_histo_map["0Jet"]       = (TH1D*) NLO_weight_file->Get("h_ratio_pt0Jet");
    pt_weight_histo_map["1Jet_0bJet"] = (TH1D*) NLO_weight_file->Get("h_ratio_pt1Jet_0bJet");
    pt_weight_histo_map["1Jet_1bJet"] = (TH1D*) NLO_weight_file->Get("h_ratio_pt1Jet_0bJet");
    pt_weight_histo_map["2Jet_0bJet"] = (TH1D*) NLO_weight_file->Get("h_ratio_pt2Jet_0bJet");
    pt_weight_histo_map["2Jet_1bJet"] = (TH1D*) NLO_weight_file->Get("h_ratio_pt2Jet_1bJet");
    pt_weight_histo_map["2Jet_2bJet"] = (TH1D*) NLO_weight_file->Get("h_ratio_pt2Jet_2bJet");

    // SF file and map
    TString sf_weight_file_name = gConfigParser->readStringOption("DYLOtoNLOreweight::sfFile");
    TFile* sf_weight_file = new TFile (sf_weight_file_name);
    cout << "** INFO: apply DY LO to NLO reweight with SFfile: " << sf_weight_file_name << endl;

    TH1D* DY_sf_histo = (TH1D*) sf_weight_file->Get("NbjetBins_NjetBins/scale_factors");
    int nbins = DY_sf_histo->GetNbinsX();
    for(int i=1; i<=nbins;i++)
    {
        std::string scale_factor_name = DY_sf_histo->GetXaxis()->GetBinLabel(i);
        double value = DY_sf_histo->GetBinContent(i);
        scale_factor_map[scale_factor_name] = value;
    }
  }

  // input and output setup
  // ---- ---- ---- ---- ---- ---- ---- ---- ---- ---- ---- ---- ---- ----
  TChain * bigChain = new TChain ("HTauTauTree/HTauTauTree") ;

  appendFromFileList (bigChain, inputFile);
  bigChain->SetCacheSize(0);
  bigTree theBigTree (bigChain) ;
  //Create a new file + a clone of old tree header. Do not copy events
  TFile * smallFile = new TFile (outputFile, "recreate") ;
  smallFile->cd () ;
  smallTree theSmallTree ("HTauTauTree") ;

  // for HHKinFit
  int hypo_mh1 = 125;
  int hypo_mh2 = 125;

  // for efficiencies
  float totalEvents = 0. ;
  float selectedEvents = 0. ;
  int totalNoWeightsEventsNum = 0 ;
  int selectedNoWeightsEventsNum = 0 ;
  
  //int DY_tot  = 0;
  //int DY_pass = 0;
  //int DY_fail = 0;

  // for VBF trigger matching
  bool isVBFfired = false;

  // ------------------------------
  
  TH1F* hTriggers = getFirstFileHisto (inputFile);
  TH1F* hTauIDS = getFirstFileHisto (inputFile,false);
  
  //FRA new triggerReader_cross to take into account the usage of crossTriggers
  triggerReader_cross trigReader (hTriggers);
  trigReader.addTauTauTrigs (trigTauTau);
  trigReader.addMuTauTrigs  (trigMuTau);
  trigReader.addEleTauTrigs (trigEleTau);
  trigReader.addMuMuTrigs   (trigMuMu);
  trigReader.addEleEleTrigs (trigEleEle);

  // add crossTriggers
  trigReader.addTauTauCrossTrigs (crossTrigTauTau);
  trigReader.addMuTauCrossTrigs  (crossTrigMuTau);
  trigReader.addEleTauCrossTrigs (crossTrigEleTau);

  // add VBF triggers for jet matching
  trigReader.addVBFTrigs (vbfTriggers);

  // print full list (this is needed to identify the the triggers that fired in the bitwise variable)
  trigReader.printTriggerList();
  
  // ------------------------------

  OfflineProducerHelper oph (hTriggers, hTauIDS) ;

  // ------------------------------

  //bJetRegrVars bjrv;
  //TMVA::Reader *bRreader = new TMVA::Reader();
  //bjrv.setReader (bRreader);
  //string bRegrMethodName = "BDTG method";
  //if (computeBregr)
  //  bRreader->BookMVA( bRegrMethodName.c_str(), bRegrWeights.c_str() );
  
  // ------------------------------

  //PUReweight reweight (PUReweight::RUN2ANALYSIS); // NONE : no PU reweight (always returns 1) - RUN2ANALYSIS: get weights according to MC and data targets
  PUReweight reweight (PUReweight::RUN2ANALYSIS, PUreweightFile);

  // ------------------------------

  cout << "** INFO: useDepFlavor? " << useDeepFlavor << endl;

  string bTag_SFFile;
  string bTag_effFile;
  
  if(useDeepFlavor) 
  {
  	bTag_SFFile = gConfigParser->readStringOption("bTagScaleFactors::SFFileDeepFlavor");
  	bTag_effFile = gConfigParser->readStringOption("bTagScaleFactors::effFileDeepFlavor");
  }
  else
  {
  	bTag_SFFile = gConfigParser->readStringOption("bTagScaleFactors::SFFileDeepCSV");
  	bTag_effFile = gConfigParser->readStringOption("bTagScaleFactors::effFileDeepCSV");
  }	
  	
  cout << "B Tag SF file: " << bTag_SFFile << endl;
  bTagSF bTagSFHelper (bTag_SFFile, bTag_effFile, "", "94X_DeepCSV_V1");
  if(useDeepFlavor)
    bTagSFHelper.SetWPset("94X_DeepFlavor_V1");

  // ------------------------------
  
  //tau legs trigger SF for data and mc
  //from: https://github.com/cms-tau-pog/TauTriggerSFs/tree/run2_SFs
  tau_trigger::SFProvider * tauTrgSF_ditau = new tau_trigger::SFProvider("weights/trigger_SF_Legacy/2017/2017_tauTriggerEff_DeepTau2017v2p1.root", "ditau", "Medium");
  tau_trigger::SFProvider * tauTrgSF_mutau = new tau_trigger::SFProvider("weights/trigger_SF_Legacy/2017/2017_tauTriggerEff_DeepTau2017v2p1.root", "mutau", "Medium");
  tau_trigger::SFProvider * tauTrgSF_etau  = new tau_trigger::SFProvider("weights/trigger_SF_Legacy/2017/2017_tauTriggerEff_DeepTau2017v2p1.root", "etau" , "Medium");

  // electron/muon leg trigger SF for data and mc
  ScaleFactor * muTauTrgSF = new ScaleFactor();
  ScaleFactor * eTauTrgSF  = new ScaleFactor();
  ScaleFactor * muTrgSF    = new ScaleFactor();
  ScaleFactor * eTrgSF     = new ScaleFactor();

  muTauTrgSF->init_ScaleFactor("weights/trigger_SF_Legacy/2017/Muon_MuTau_IsoMu20.root");
  muTrgSF   ->init_ScaleFactor("weights/trigger_SF_Legacy/2017/Muon_IsoMu24orIsoMu27.root");
  eTauTrgSF ->init_ScaleFactor("weights/trigger_SF_Legacy/2017/Electron_EleTau_Ele24_fix.root");
  eTrgSF    ->init_ScaleFactor("weights/trigger_SF_Legacy/2017/Electron_Ele32orEle35_fix.root");

  //VBF trigger weights -- jet legs
  TFile* VBFjets_file = new TFile ("weights/trigger_SF_Legacy/2017/VBFHTauTauHLT_jetlegs_3Dcoarse.root"); // !! FIXME !! to be updated for DeepTauV2p1
  TH3D*  VBFjets_SF   = (TH3D*) VBFjets_file->Get("SF_mjj_pT1_pT2");

  // ------------------------------

  // electron/muon IdAndIso SF
  ScaleFactor * myIDandISOScaleFactor[2]; // [0: mu, 1: ele]
  for (int i = 0 ; i < 2; i++)
        myIDandISOScaleFactor[i] = new ScaleFactor();

  myIDandISOScaleFactor[0] -> init_ScaleFactor("weights/HTT_IdAndIso_SF_Legacy/2017/Muon_IdIso_IsoLt0.15_eff_RerecoFall17.root"); //FRA: is this ok?
  myIDandISOScaleFactor[1] -> init_ScaleFactor("weights/HTT_IdAndIso_SF_Legacy/2017/Electron_Run2017_IdIso.root");

  // tau IdAndIso SF
  TauIDSFTool * tauSFTool_MVA        = new TauIDSFTool("2017ReReco","MVAoldDM2017v2","Medium",1);       // for MVA2017v2 vs jets
  TauIDSFTool * antiEleSFTool_vloose = new TauIDSFTool("2017ReReco","antiEleMVA6"   ,"VLoose");         // for MVA2017v2 vs ele VLoose
  TauIDSFTool * antiEleSFTool_tight  = new TauIDSFTool("2017ReReco","antiEleMVA6"   ,"Tight");          // for MVA2017v2 vs ele Tight
  TauIDSFTool * antiMuSFTool_loose   = new TauIDSFTool("2017ReReco","antiMu3"       ,"Loose");          // for MVA2017v2 vs mu Loose
  TauIDSFTool * antiMuSFTool_tight   = new TauIDSFTool("2017ReReco","antiMu3"       ,"Tight");          // for MVA2017v2 vs mu Tight
  TauIDSFTool * tauSFTool_deep       = new TauIDSFTool("2017ReReco","DeepTau2017v2p1VSjet","Medium",1); // for DeepTauv2p1 vs jets

  // ------------------------------

  // reweighting file for HH non resonant
  TH1F* hreweightHH   = 0;
  TH2F* hreweightHH2D = 0;
  // if (HHreweightFile)
  if (HHrewType == kFromHisto)
    {
      cout << "** INFO: doing reweight for HH samples" << endl;
      if (HHreweightFile->GetListOfKeys()->Contains("hratio") )
	{  
	  hreweightHH = (TH1F*) HHreweightFile->Get("hratio");
	  cout << "** INFO: 1D reweight using hratio" << endl;
	}
      else if (HHreweightFile->GetListOfKeys()->Contains("hratio2D") )
	{
	  hreweightHH2D = (TH2F*) HHreweightFile->Get("hratio2D");            
	  cout << "** INFO: 2D reweight using hratio2D" << endl;
	}
      else
	{
	  cout << "** ERROR: reweight histo not found in file provided, stopping execuction" << endl;
	  return 1;
	}
    }

  // ------------------------------
  // reweight file in case of "dynamic" reweight
  // there is a unique input map, read it from the cfg file
  // HHReweight* hhreweighter = nullptr;
  HHReweight5D* hhreweighter = nullptr;
  TH2* hhreweighterInputMap = nullptr;
  if (HHrewType == kDynamic)
    {
      string inMapFile   = gConfigParser->readStringOption("HHReweight::inputFile");
      string inHistoName = gConfigParser->readStringOption("HHReweight::histoName");
      string coeffFile    = gConfigParser->readStringOption("HHReweight::coeffFile");
      cout << "** INFO: reading histo named: " << inHistoName << " from file: " << inMapFile << endl;
      cout << "** INFO: HH reweight coefficient file is: " << coeffFile << endl;
      TFile* fHHDynamicRew = new TFile(inMapFile.c_str());
      hhreweighterInputMap = (TH2*) fHHDynamicRew->Get(inHistoName.c_str());
      // hhreweighter = new HHReweight(coeffFile, hhreweighterInputMap);
      hhreweighter = new HHReweight5D(coeffFile, hhreweighterInputMap);
    }


  // ------------------------------
  // indexes of tau ID bits

  map <string, int> tauIDsMap;
  for (int ibin = 0; ibin < hTauIDS->GetNbinsX(); ++ibin)
    {
      if (string(hTauIDS->GetXaxis()->GetBinLabel(ibin+1)) == string("byLooseCombinedIsolationDeltaBetaCorr3Hits") )
	tauIDsMap ["byLooseCombinedIsolationDeltaBetaCorr3Hits"] = ibin ;

      if (string(hTauIDS->GetXaxis()->GetBinLabel(ibin+1)) == string("byMediumCombinedIsolationDeltaBetaCorr3Hits") )
	tauIDsMap ["byLooseCombinedIsolationDeltaBetaCorr3Hits"] = ibin ;

      if (string(hTauIDS->GetXaxis()->GetBinLabel(ibin+1)) == string("byTightCombinedIsolationDeltaBetaCorr3Hits") )
	tauIDsMap ["byLooseCombinedIsolationDeltaBetaCorr3Hits"] = ibin ;
    }

  // MVA tau ID
  vector<int> tauMVAIDIdx;
  //tauMVAIDIdx.push_back(getTauIDIdx(hTauIDS, "byVVLooseIsolationMVArun2v1DBoldDMwLT"));
  tauMVAIDIdx.push_back(getTauIDIdx(hTauIDS, "byVLooseIsolationMVArun2v1DBoldDMwLT"));
  tauMVAIDIdx.push_back(getTauIDIdx(hTauIDS, "byLooseIsolationMVArun2v1DBoldDMwLT"));
  tauMVAIDIdx.push_back(getTauIDIdx(hTauIDS, "byMediumIsolationMVArun2v1DBoldDMwLT"));
  tauMVAIDIdx.push_back(getTauIDIdx(hTauIDS, "byTightIsolationMVArun2v1DBoldDMwLT"));
  tauMVAIDIdx.push_back(getTauIDIdx(hTauIDS, "byVTightIsolationMVArun2v1DBoldDMwLT"));
  if (find(tauMVAIDIdx.begin(), tauMVAIDIdx.end(), -1) != tauMVAIDIdx.end())
    {
      cout << "** WARNING!! did not found some MVA tau IDs" << endl;
      for (unsigned int i = 0; i < tauMVAIDIdx.size(); ++i)
	cout << tauMVAIDIdx.at(i) << " " ;
      cout << endl;
    }
    
  // new MVA tau ID // FRA syncFeb2018
  vector<int> tauMVAIDIdxNew;
  //tauMVAIDIdxNew.push_back(getTauIDIdx(hTauIDS, "byVVLooseIsolationMVArun2017v2DBoldDMwLT2017")); //SYNCH HTT
  tauMVAIDIdxNew.push_back(getTauIDIdx(hTauIDS, "byVLooseIsolationMVArun2017v2DBoldDMwLT2017"));
  tauMVAIDIdxNew.push_back(getTauIDIdx(hTauIDS, "byLooseIsolationMVArun2017v2DBoldDMwLT2017"));
  tauMVAIDIdxNew.push_back(getTauIDIdx(hTauIDS, "byMediumIsolationMVArun2017v2DBoldDMwLT2017"));
  tauMVAIDIdxNew.push_back(getTauIDIdx(hTauIDS, "byTightIsolationMVArun2017v2DBoldDMwLT2017"));
  tauMVAIDIdxNew.push_back(getTauIDIdx(hTauIDS, "byVTightIsolationMVArun2017v2DBoldDMwLT2017"));
  if (find(tauMVAIDIdxNew.begin(), tauMVAIDIdxNew.end(), -1) != tauMVAIDIdxNew.end())
    {
      cout << "** WARNING!! did not found some MVA tau IDs New" << endl;
      for (unsigned int i = 0; i < tauMVAIDIdxNew.size(); ++i)
	cout << tauMVAIDIdxNew.at(i) << " " ;
      cout << endl;
    }

  // new MVA tau ID // FRA syncFeb2018
  vector<int> tauMVAIDIdxNewdR0p3;
  tauMVAIDIdxNewdR0p3.push_back(getTauIDIdx(hTauIDS, "byVLooseIsolationMVArun2017v2DBoldDMdR0p3wLT2017"));
  tauMVAIDIdxNewdR0p3.push_back(getTauIDIdx(hTauIDS, "byLooseIsolationMVArun2017v2DBoldDMdR0p3wLT2017"));
  tauMVAIDIdxNewdR0p3.push_back(getTauIDIdx(hTauIDS, "byMediumIsolationMVArun2017v2DBoldDMdR0p3wLT2017"));
  tauMVAIDIdxNewdR0p3.push_back(getTauIDIdx(hTauIDS, "byTightIsolationMVArun2017v2DBoldDMdR0p3wLT2017"));
  tauMVAIDIdxNewdR0p3.push_back(getTauIDIdx(hTauIDS, "byVTightIsolationMVArun2017v2DBoldDMdR0p3wLT2017"));
  if (find(tauMVAIDIdxNewdR0p3.begin(), tauMVAIDIdxNewdR0p3.end(), -1) != tauMVAIDIdxNewdR0p3.end())
    {
      cout << "** WARNING!! did not found some MVA tau IDs New" << endl;
      for (unsigned int i = 0; i < tauMVAIDIdxNewdR0p3.size(); ++i)
	cout << tauMVAIDIdxNewdR0p3.at(i) << " " ;
      cout << endl;
    }

  // cut based tau ID
  vector<int> tauCUTIDIdx;
  tauCUTIDIdx.push_back(getTauIDIdx(hTauIDS, "byLooseCombinedIsolationDeltaBetaCorr3Hits"));
  tauCUTIDIdx.push_back(getTauIDIdx(hTauIDS, "byMediumCombinedIsolationDeltaBetaCorr3Hits"));
  tauCUTIDIdx.push_back(getTauIDIdx(hTauIDS, "byTightCombinedIsolationDeltaBetaCorr3Hits"));
  if (find(tauCUTIDIdx.begin(), tauCUTIDIdx.end(), -1) != tauCUTIDIdx.end())
    {
      cout << "** WARNING!! did not found some cut-based tau IDs" << endl;
      for (unsigned int i = 0; i < tauCUTIDIdx.size(); ++i)
	cout << tauCUTIDIdx.at(i) << " " ;
      cout << endl;
    }


  // anti-ele discr
  vector<int> tauAntiEleIdx;
  tauAntiEleIdx.push_back(getTauIDIdx(hTauIDS, "againstElectronVLooseMVA6"));
  tauAntiEleIdx.push_back(getTauIDIdx(hTauIDS, "againstElectronLooseMVA6"));
  tauAntiEleIdx.push_back(getTauIDIdx(hTauIDS, "againstElectronMediumMVA6"));
  tauAntiEleIdx.push_back(getTauIDIdx(hTauIDS, "againstElectronTightMVA6"));
  tauAntiEleIdx.push_back(getTauIDIdx(hTauIDS, "againstElectronVTightMVA6"));
  if (find(tauAntiEleIdx.begin(), tauAntiEleIdx.end(), -1) != tauAntiEleIdx.end())
    {
      cout << "** WARNING!! did not found some cut-based tau IDs" << endl;
      for (unsigned int i = 0; i < tauAntiEleIdx.size(); ++i)
	cout << tauAntiEleIdx.at(i) << " " ;
      cout << endl;
    }

  // anti-mu discr
  vector<int> tauAntiMuIdx;
  tauAntiMuIdx.push_back(getTauIDIdx(hTauIDS, "againstMuonLoose3"));
  tauAntiMuIdx.push_back(getTauIDIdx(hTauIDS, "againstMuonTight3"));
  if (find(tauAntiMuIdx.begin(), tauAntiMuIdx.end(), -1) != tauAntiMuIdx.end())
    {
      cout << "** WARNING!! did not found some cut-based tau IDs" << endl;
      for (unsigned int i = 0; i < tauAntiMuIdx.size(); ++i)
	cout << tauAntiMuIdx.at(i) << " " ;
      cout << endl;
    }

  // DNN Tau ID vs jet
  vector<int> deepTauVsJetIdx;
  deepTauVsJetIdx.push_back(getTauIDIdx(hTauIDS, "byVVVLooseDeepTau2017v2p1VSjet"));
  deepTauVsJetIdx.push_back(getTauIDIdx(hTauIDS, "byVVLooseDeepTau2017v2p1VSjet")); 
  deepTauVsJetIdx.push_back(getTauIDIdx(hTauIDS, "byVLooseDeepTau2017v2p1VSjet"));  
  deepTauVsJetIdx.push_back(getTauIDIdx(hTauIDS, "byLooseDeepTau2017v2p1VSjet"));   
  deepTauVsJetIdx.push_back(getTauIDIdx(hTauIDS, "byMediumDeepTau2017v2p1VSjet"));  
  deepTauVsJetIdx.push_back(getTauIDIdx(hTauIDS, "byTightDeepTau2017v2p1VSjet"));   
  deepTauVsJetIdx.push_back(getTauIDIdx(hTauIDS, "byVTightDeepTau2017v2p1VSjet"));  
  deepTauVsJetIdx.push_back(getTauIDIdx(hTauIDS, "byVVTightDeepTau2017v2p1VSjet")); 
  if (find(deepTauVsJetIdx.begin(), deepTauVsJetIdx.end(), -1) != deepTauVsJetIdx.end())
    {
      cout << "** WARNING!! did not found some cut-based tau IDs" << endl;
      for (unsigned int i = 0; i < deepTauVsJetIdx.size(); ++i)
	cout << deepTauVsJetIdx.at(i) << " " ;
      cout << endl;
    }

  // DNN Tau ID vs ele
  vector<int> deepTauVsEleIdx;  
  deepTauVsEleIdx.push_back(getTauIDIdx(hTauIDS, "byVVVLooseDeepTau2017v2p1VSe"));  
  deepTauVsEleIdx.push_back(getTauIDIdx(hTauIDS, "byVVLooseDeepTau2017v2p1VSe")); 
  deepTauVsEleIdx.push_back(getTauIDIdx(hTauIDS, "byVLooseDeepTau2017v2p1VSe"));   
  deepTauVsEleIdx.push_back(getTauIDIdx(hTauIDS, "byLooseDeepTau2017v2p1VSe"));  
  deepTauVsEleIdx.push_back(getTauIDIdx(hTauIDS, "byMediumDeepTau2017v2p1VSe"));   
  deepTauVsEleIdx.push_back(getTauIDIdx(hTauIDS, "byTightDeepTau2017v2p1VSe"));  
  deepTauVsEleIdx.push_back(getTauIDIdx(hTauIDS, "byVTightDeepTau2017v2p1VSe"));   
  deepTauVsEleIdx.push_back(getTauIDIdx(hTauIDS, "byVVTightDeepTau2017v2p1VSe"));   
  if (find(deepTauVsEleIdx.begin(), deepTauVsEleIdx.end(), -1) != deepTauVsEleIdx.end())
    {
      cout << "** WARNING!! did not found some cut-based tau IDs" << endl;
      for (unsigned int i = 0; i < deepTauVsEleIdx.size(); ++i)
	cout << deepTauVsEleIdx.at(i) << " " ;
      cout << endl;
    }

  // DNN Tau ID vs mu
  vector<int> deepTauVsMuIdx;
  deepTauVsMuIdx.push_back(getTauIDIdx(hTauIDS, "byVLooseDeepTau2017v2p1VSmu")); 
  deepTauVsMuIdx.push_back(getTauIDIdx(hTauIDS, "byLooseDeepTau2017v2p1VSmu")); 
  deepTauVsMuIdx.push_back(getTauIDIdx(hTauIDS, "byMediumDeepTau2017v2p1VSmu")); 
  deepTauVsMuIdx.push_back(getTauIDIdx(hTauIDS, "byTightDeepTau2017v2p1VSmu")); 
  if (find(deepTauVsMuIdx.begin(), deepTauVsMuIdx.end(), -1) != deepTauVsMuIdx.end())
    {
      cout << "** WARNING!! did not found some cut-based tau IDs" << endl;
      for (unsigned int i = 0; i < deepTauVsMuIdx.size(); ++i)
	cout << deepTauVsMuIdx.at(i) << " " ;
      cout << endl;
    }

  // -----------------------------------
  // event counters for efficiency study
  EffCounter ec;
  ec.AddMarker ("all");
  ec.AddMarker ("METfilter");
  ec.AddMarker ("NoBadMuons");
  ec.AddMarker ("PairExists");
  ec.AddMarker ("PairFoundBaseline");
  ec.AddMarker ("Trigger");
  ec.AddMarker ("TwoJets");

  // for hh signal only -- split by gen decay
  EffCounter* ecHHsig;
  if (isHHsignal)
    {
      ecHHsig = new EffCounter[6];
      for (int ic = 0; ic < 6; ++ic)
	{
	  ecHHsig[ic].AddMarker ("all");
	  ecHHsig[ic].AddMarker ("METfilter");
	  ecHHsig[ic].AddMarker ("NoBadMuons");
	  ecHHsig[ic].AddMarker ("PairExists");
	  ecHHsig[ic].AddMarker ("PairFoundBaseline");
	  ecHHsig[ic].AddMarker ("PairMatchesGen");
	  ecHHsig[ic].AddMarker ("Trigger");
	  ecHHsig[ic].AddMarker ("TwoJets");      
	}
    }

  //FRA debug
  //int debugEvents[2] = {
  //                      85365648,
  //                      62741920};

  // loop over events
  // ---- ---- ---- ---- ---- ---- ---- ---- ---- ---- ---- ---- ---- ----
  for (Long64_t iEvent = 0 ; true ; ++iEvent) 
    {
      if (iEvent % 10000 == 0)  cout << "- reading event " << iEvent << endl ;
      //if (iEvent == 20000)  break ;
      //cout << "-------- reading event " << iEvent << endl ;
      theSmallTree.clearVars () ;
      
      int got = theBigTree.fChain->GetEntry(iEvent);

      if (got == 0) break;
      bool DEBUG = false;
      //if (theBigTree.EventNumber != debugEvent) continue; //FRA debug
      //bool goodDebugEvent = false;
      //for (unsigned int i=0; i<sizeof(debugEvents)/sizeof(*debugEvents); i++) //FRA debug
      //{
      //  if (theBigTree.EventNumber == debugEvents[i]) goodDebugEvent = true;
      //  if (goodDebugEvent)
      //  {
      //      DEBUG = true;
      //      cout << "****** DEBUG: event=" << theBigTree.EventNumber << " run=" << theBigTree.RunNumber << " lumi=" << theBigTree.lumi << " (entry number=" << iEvent << ")" << endl;
      //      break;
      //  }
      //}
      //if (!goodDebugEvent) continue;

      if (theBigTree.EventNumber == debugEvent )
	{
	  cout << "****** DEBUG : debugging event=" << theBigTree.EventNumber << " run=" << theBigTree.RunNumber << " lumi=" << theBigTree.lumi << " (entry number=" << iEvent << ")" << endl;
	  DEBUG = true;
	}
      //if (debugEvent > 0 && DEBUG == false) continue;

      // remove a lumisection that was present in 16 Giu JSON and removed in 22 and subsequent JSON
      // 25 Nov 2016 : edit : removed line because of new reprocessing and json
      // if (!isMC && theBigTree.RunNumber == 274094 && theBigTree.lumi >= 105 && theBigTree.lumi <= 107) continue;

      // directly reject events outside HT range in case of stitching of inclusive sample-- they should not count in weights
      //cout << " ********** HTMAX - MIN - LHE: " << HTMax << " - " << HTMin << " - " << theBigTree.lheHt << endl;
      if (HTMax > 0)
	{
	  if (theBigTree.lheHt > HTMax) continue;
	}

      if (HTMin > 0)
	{
	  if (theBigTree.lheHt < HTMin) continue;
	}

      // directly reject events I want to remove in W+Jets stitching on njets
      if (NjetRequired >= 0)
	{
	  if (theBigTree.lheNOutPartons != NjetRequired) continue;
	}

      // skip event if I want a specific SUSY point from the fastsim
      //if (susyModel != string("NOTSUSY"))
      //{
      //  if (string(theBigTree.susyModel.Data()) != susyModel) continue;
      //}

      float stitchWeight = 1.0;
      if (DY_tostitch)
	{
	  int njets = theBigTree.lheNOutPartons;
	  int nb    = theBigTree.lheNOutB;
	  // these protections should be useless
	  if (njets < 0) njets = 0;
	  if (njets > 4) njets = 4;
	  if (nb < 0)    nb = 0;
	  if (nb > 4)    nb = 4;

	  stitchWeight = stitchWeights[njets][nb];
	}

    if (!DY_tostitch && DY_nJets >= 0) //FRA debug
    {
        int njets = theBigTree.lheNOutPartons;
        int nb    = theBigTree.lheNOutB;
        //cout << "- njets: " << njets << " - nb: " << nb << endl; //FRA debug
        if (njets != DY_nJets || nb != DY_nBJets) continue;
    }

      if (DEBUG && isMC)
	{
	  cout << "** DEBUG : gen particle list" << endl;
	  for (unsigned int igen = 0; igen < theBigTree.genpart_pdg->size(); igen++)
	    {
	      int pdg = theBigTree.genpart_pdg->at(igen);
	      if (abs(pdg) == 66615 || abs(pdg) == 11 || abs(pdg) == 13)
		{
		  TLorentzVector vGPDebug;
		  vGPDebug.SetPxPyPzE (theBigTree.genpart_px->at(igen), theBigTree.genpart_py->at(igen), theBigTree.genpart_pz->at(igen), theBigTree.genpart_e->at(igen) ) ;
		  cout << igen << " pdg=" << pdg << " pt=" << vGPDebug.Pt() << " eta=" << vGPDebug.Eta() << " phi=" << vGPDebug.Phi() << endl;          
		}
	      if (abs(pdg) == 25)
		{
		  cout << igen << " pdg=" << pdg << " decay=" << theBigTree.genpart_HZDecayMode->at(igen) << endl; 
		}
	    }
	}

      // gen info -- fetch tt pair and compute top PT reweight
      float topPtReweight = 1.0; // 1 for all the other samples      
      theSmallTree.m_TTtopPtreweight =  1.0 ;
      theSmallTree.m_TTtopPtreweight_up =  1.0 ;
      theSmallTree.m_TTtopPtreweight_down =  1.0 ;
      if (isTTBar)
	{
	  float ptTop1 = -1.0;
	  float ptTop2 = -1.0;
	  int decayTop1 = -999;
	  int decayTop2 = -999;
	  int pdgIdTop1 = -999;
	  int pdgIdTop2 = -999;

	  for (unsigned int igen = 0; igen < theBigTree.genpart_pdg->size(); igen++)
	    {
	      int pdg = theBigTree.genpart_pdg->at(igen);
	      int genflags = theBigTree.genpart_flags->at(igen);
	      bool isFirst = CheckBit (genflags, 12);  //if (fl.isFirstCopy()) flags |= (1 << 12);
	      //int topDM = theBigTree.genpart_TopDecayMode->at(igen);

	      if (abs(pdg) == 6 && isFirst) // top -- pt reweight wants to have ME tops
		{
		  TLorentzVector TopV;
		  TopV.SetPxPyPzE (theBigTree.genpart_px->at(igen), theBigTree.genpart_py->at(igen), theBigTree.genpart_pz->at(igen), theBigTree.genpart_e->at(igen) ) ;
		  if (ptTop1 < 0)
		    {
		      ptTop1 = TopV.Pt();
		      decayTop1 = theBigTree.genpart_TopDecayMode->at(igen);
		      pdgIdTop1 = theBigTree.genpart_pdg->at(igen);
		    }
		  else if (ptTop2 < 0)
		    { 
		      ptTop2 = TopV.Pt(); 
		      decayTop2 = theBigTree.genpart_TopDecayMode->at(igen);
		      pdgIdTop2 = theBigTree.genpart_pdg->at(igen);
		      break;
		    } // check done in paralles shows that I never have > 2 top .  break is safe .
		  // else cout << " !! skim warning: sample is declared as as ttbar, but I have > 2 gen top in the event! " << endl;
		}
	    }
	  if (ptTop1 < 0 || ptTop2 < 0)
	    {
	      cout << "** WARNING: sample is declared as TTbar but in the event I didn't find 2 tops (1,2) :" << ptTop1 << " " << ptTop2 << endl;
	    }
	  else
	    {
	      // filter by decay mode if needed for stitching
	      // [0: no stitch , 1: fully had, 2: semilept t, 3: semilept tbar, 4: fully lept, 5: semilept all]
	      // TopDecayMode: 0: Had, 1-5: leptonic, 6: other -- consider "other" as a possible hadronic decay (includes rare W->bc)
          
	      bool isT1Lept = (decayTop1 >= 1 && decayTop1 <= 5) ;
	      bool isT2Lept = (decayTop2 >= 1 && decayTop2 <= 5) ;
	      if (pdgIdTop1 / pdgIdTop2 > 0) cout << "** WARNING: I found two tops with the same sign " << pdgIdTop1 << " " << pdgIdTop2 << endl;
          
	      // cout << "DEBUG: event with " << pdgIdTop1 << "-->  " << decayTop1 << " | " << pdgIdTop2 << "--> " << decayTop2 << " SKIMTYPE=" << TT_stitchType << endl;
	      switch (TT_stitchType)
		{
		case 0:
		  break; // no stitching
            
		case 1:
		  if (isT1Lept || isT2Lept) continue; // fully had
		  break;
            
		case 2: // top leptonic, antitop hadronic
		  if (pdgIdTop1 > 0) // 1 is top
		    {
		      if (! (isT1Lept && !isT2Lept) ) continue;
		    }
		  else // 1 is antitop
		    {
		      if (! (isT2Lept && !isT1Lept) ) continue;
		    }
		  break;
            
		case 3:
		  if (pdgIdTop1 > 0) // 1 is top
		    {
		      if (! (!isT1Lept && isT2Lept) ) continue;
		    }
		  else // 1 is antitop
		    {
		      if (! (!isT2Lept && isT1Lept) ) continue;
		    }
		  break;
            
		case 4:
		  if (!isT1Lept || !isT2Lept) continue;
		  break;

		case 5:
		  if (isT1Lept == isT2Lept) continue; // must be one had and the other lep, not equal
		  break;
            
		default:
		  cout << "** WARNING: unknown TT stytch type " << TT_stitchType << endl;
		}

	      // cout << "WAS ACCEPTED" << endl;

	      if(DEBUG)
		{
		  cout << "@ TOP pt reweight: " << endl;
		  cout << "  top1 pt=" << ptTop1 << endl;
		  cout << "  top2 pt=" << ptTop2 << endl;
		}

	      float SFTop1 = TMath::Exp(aTopRW+bTopRW*ptTop1);
	      float SFTop2 = TMath::Exp(aTopRW+bTopRW*ptTop2);
	      topPtReweight = TMath::Sqrt (SFTop1*SFTop2);
          
	      // old recipe
	      // theSmallTree.m_TTtopPtreweight      = topPtReweight ;
	      // theSmallTree.m_TTtopPtreweight_up   = topPtReweight*topPtReweight ;
	      // theSmallTree.m_TTtopPtreweight_down = 1.0 ;

	      // new recipe
	      theSmallTree.m_TTtopPtreweight      = 1.0; // nominal has no weight
	      theSmallTree.m_TTtopPtreweight_up   = topPtReweight; // pt rew to be used as a syst
	      theSmallTree.m_TTtopPtreweight_down = 1.0 ; // and down unused

	      theSmallTree.m_genDecMode1 = decayTop1;
	      theSmallTree.m_genDecMode2 = decayTop2;
	    }
	}

      // For Drell-Yan only: loop on genjets and count how many are there with 0,1,2 b
      // 0: 0bjet, 1: 1 b jet, 2: >= 2 b jet
      theSmallTree.m_DYscale_LL = 1.0; // all the other MC samples + data have no weight
      theSmallTree.m_DYscale_MM = 1.0;        
      theSmallTree.m_DYscale_LL_NLO = 1.0; // all the other MC samples + data have no weight
      theSmallTree.m_DYscale_MM_NLO = 1.0;

      if (isMC && isDY) //to be done both for DY NLO and DY in jet bins
      {
        TLorentzVector vgj;
        int nbs = 0;
        for (unsigned int igj = 0; igj < theBigTree.genjet_px->size(); igj++)
        {
          vgj.SetPxPyPzE(theBigTree.genjet_px->at(igj), theBigTree.genjet_py->at(igj), theBigTree.genjet_pz->at(igj), theBigTree.genjet_e->at(igj));
          if (vgj.Pt() > 20 && TMath::Abs(vgj.Eta()) < 2.5)
          {
            int theFlav = theBigTree.genjet_hadronFlavour->at(igj);
            if (abs(theFlav) == 5) nbs++;
          }

          if(DEBUG)
          {
            cout << " -- gen jet : " << igj << " pt=" << vgj.Pt() << " eta=" << vgj.Eta() <<  " hadFlav=" << theBigTree.genjet_hadronFlavour->at(igj) << endl;
          }
        }
        if (nbs > 2) nbs = 2;

        theSmallTree.m_nBhadrons = nbs;
        theSmallTree.m_DYscale_LL = DYscale_LL[nbs];
        theSmallTree.m_DYscale_MM = DYscale_MM[nbs];

        // Get LHE nBPartons
        int n_bJets = theBigTree.lheNOutB ;
        if (n_bJets>=2) n_bJets=2; // Make sure that events with n_bJets == 3 are included in the n_bJets == 2 case

        // loop through gen parts ot identify Z boson
        int idx1 = -1;
        for (unsigned int igen = 0; igen < theBigTree.genpart_px->size(); igen++)
	    {
          bool isLast   = CheckBit(theBigTree.genpart_flags->at(igen), 13) ; // 13 = isLastCopy
          bool isPrompt = CheckBit(theBigTree.genpart_flags->at(igen),  0) ; //  0 = isPrompt
          if (theBigTree.genpart_pdg->at(igen) == 23 && isLast && isPrompt) // Z0 + isLast + isPrompt
          {
            idx1 = igen;
          }
	    }

	    // if found, Build the genZ TLorentzVector
	    float genZ_pt = -999.;
        //DY_tot += 1;
	    if (idx1 >= 0)
	    {
          //DY_pass +=1;
	      // store gen decay mode of the Z identified
	      theSmallTree.m_genDecMode1 = theBigTree.genpart_HZDecayMode->at(idx1);

	      // build the genZ TLorentzVector
	      TLorentzVector genZ;
	      genZ.SetPxPyPzE(theBigTree.genpart_px->at(idx1), theBigTree.genpart_py->at(idx1), theBigTree.genpart_pz->at(idx1), theBigTree.genpart_e->at(idx1));
	      genZ_pt = genZ.Pt();

	      // Fill DY NLO weights according to nbs and pT(Z)
          if (genZ_pt < 20.)
          {
            theSmallTree.m_DYscale_LL_NLO = DYscale_NLO_VLowPt[n_bJets];
            theSmallTree.m_DYscale_MM_NLO = DYscale_NLO_VLowPt[n_bJets];
          }
          else if (genZ_pt >= 20. && genZ_pt < 40.)
          {
            theSmallTree.m_DYscale_LL_NLO = DYscale_NLO_LowPt[n_bJets];
            theSmallTree.m_DYscale_MM_NLO = DYscale_NLO_LowPt[n_bJets];
          }
          else if (genZ_pt >= 40. && genZ_pt < 100.)
          {
            theSmallTree.m_DYscale_LL_NLO = DYscale_NLO_MedPt[n_bJets];
            theSmallTree.m_DYscale_MM_NLO = DYscale_NLO_MedPt[n_bJets];
          }
          else /* pT(Z)>=100. */
          {
            theSmallTree.m_DYscale_LL_NLO = DYscale_NLO_HighPt[n_bJets];
            theSmallTree.m_DYscale_MM_NLO = DYscale_NLO_HighPt[n_bJets];
          }

          // Fill the DY LO weights
          if      (genZ_pt<=10.)                  theSmallTree.m_DYLOtoNLOreweight = DYscale_LO_VLowPt [n_bJets];
          else if (genZ_pt>10.  && genZ_pt<=30. ) theSmallTree.m_DYLOtoNLOreweight = DYscale_LO_LowPt  [n_bJets];
          else if (genZ_pt>30.  && genZ_pt<=50. ) theSmallTree.m_DYLOtoNLOreweight = DYscale_LO_MedPt1 [n_bJets];
          else if (genZ_pt>50.  && genZ_pt<=100.) theSmallTree.m_DYLOtoNLOreweight = DYscale_LO_MedPt2 [n_bJets];
          else if (genZ_pt>100. && genZ_pt<=200.) theSmallTree.m_DYLOtoNLOreweight = DYscale_LO_HighPt [n_bJets];
          else /*genZ_pt>200*/                    theSmallTree.m_DYLOtoNLOreweight = DYscale_LO_VHighPt[n_bJets];
	    }
        else // genZ not found: use the lowest pT bin
        {
            //DY_fail+=1;
            theSmallTree.m_DYscale_LL_NLO    = DYscale_NLO_VLowPt[n_bJets];
            theSmallTree.m_DYscale_MM_NLO    = DYscale_NLO_VLowPt[n_bJets];
            theSmallTree.m_DYLOtoNLOreweight = DYscale_LO_VLowPt [n_bJets];
        }

        // Debug printout
        if(DEBUG)
        {
            cout << "------- DY reweight ------" << endl;
            cout << " - nbs  : " << nbs << endl;
            cout << " - pT(Z): " << genZ_pt << endl;
            cout << " - DYscale_MM        : " << theSmallTree.m_DYscale_MM << endl;
            cout << " - DYscale_MM_NLO    : " << theSmallTree.m_DYscale_MM_NLO << endl;
            cout << " - DYLOtoNLOreweight : " << theSmallTree.m_DYLOtoNLOreweight << endl;
            cout << "--------------------------" << endl;
        }
      }

      // New DY reweight
      if (isMC && false) /* (isMC && doDYLOtoNLOreweight) - filled in the previous if */
      {
        float fractional_weight = 0;
        float pt_weight         = 0;
        float sf_weight         = 1;

        // Get LHE nPartons and nBPartons
        float n_selected_gen_jets = theBigTree.lheNOutPartons ;
        float n_bJets = theBigTree.lheNOutB ;

        // fraction map categories
        std::string lhe_category = "";
        if(n_selected_gen_jets==0) lhe_category="0Jet";
        else if (n_selected_gen_jets == 1)
        {
            if      (n_bJets==0) lhe_category="1Jet_0bJet";
            else if (n_bJets==1) lhe_category="1Jet_1bJet";
        }
        else if (n_selected_gen_jets==2)
        {
            if      (n_bJets==0) lhe_category="2Jet_0bJet";
            else if (n_bJets==1) lhe_category="2Jet_1bJet";
            else if (n_bJets==2) lhe_category="2Jet_2bJet";
        }


        // SF map categories
        std::string sf_category  = "";
        if (n_bJets==0)
        {
            if      (n_selected_gen_jets <= 2) sf_category = "DY_MC_0b_0Jet";
            else if (n_selected_gen_jets >  2) sf_category = "DY_MC_0b_2Jet";
        }
        else if (n_bJets==1)
        {
            if      (n_selected_gen_jets <= 2) sf_category = "DY_MC_1b_0Jet";
            else if (n_selected_gen_jets >  2) sf_category = "DY_MC_1b_2Jet";
        }
        else if (n_bJets==2)
        {
            if      (n_selected_gen_jets <= 2) sf_category = "DY_MC_2b_0Jet";
            else if (n_selected_gen_jets == 2) sf_category = "DY_MC_2b_2Jet";
        }

        // DY NLO samples have only <= 2 gen jets, so use the reweight only if n_selected_gen_jets <= 2
        if (n_selected_gen_jets <= 2)
        {
            // Fractional weight
            fractional_weight = fractional_weight_map[lhe_category];

            // Pt weight
            // loop through gen parts ot identify Z boson
            int idx1 = -1;
            for (unsigned int igen = 0; igen < theBigTree.genpart_px->size(); igen++)
            {
                bool isLast   = CheckBit(theBigTree.genpart_flags->at(igen), 13) ; // 13 = isLastCopy
                bool isPrompt = CheckBit(theBigTree.genpart_flags->at(igen),  0) ; //  0 = isPrompt
                if (theBigTree.genpart_pdg->at(igen) == 23 && isLast && isPrompt) // Z0 + isLast + isPrompt
                {
                    idx1 = igen;
                }
            }

            // if found, Build the genZ TLorentzVector
            float genZ_pt = -999.;
            if (idx1 >= 0)
            {
                TLorentzVector genZ;
                genZ.SetPxPyPzE(theBigTree.genpart_px->at(idx1), theBigTree.genpart_py->at(idx1), theBigTree.genpart_pz->at(idx1), theBigTree.genpart_e->at(idx1));

                genZ_pt = genZ.Pt();
                pt_weight = pt_weight_histo_map[lhe_category]->GetBinContent(pt_weight_histo_map[lhe_category]->FindBin(genZ_pt));
            }
        }

        // SF weight
        sf_weight = scale_factor_map[sf_category];

        // Debug printout
        if(DEBUG)
        {
            cout << "------- DY reweight ------" << endl;
            cout << " - LHE Npartons / Bpartons: " << n_selected_gen_jets << " / " << n_bJets << endl;
            cout << " - lhe_category: " << lhe_category << endl;
            cout << " - sf_category : " << sf_category << endl;
            cout << " - weights (fractional/pt/sf/ total): "<<fractional_weight << " / " << pt_weight << " / " << sf_weight <<" / " << fractional_weight * pt_weight * sf_weight <<endl;
            cout << "--------------------------" << endl;
        }

        theSmallTree.m_DYLOtoNLOreweight = fractional_weight * pt_weight * sf_weight;

      } // end new DY reweight

      // HH reweight for non resonant
      float HHweight = 1.0;
      TLorentzVector vHardScatter1;
      TLorentzVector vHardScatter2;
      int idx1hs = -1; // hard scatted product
      int idx2hs = -1;
      int pdg1hs = -999;
      int pdg2hs = -999;    
      int t1hs = -1;
      int t2hs = -1;

      int idx1hs_b = -1;     // bjet-1 index     // FRA DEBUG  
      int idx2hs_b = -1;     // bjet-2 index                   
      TLorentzVector vGenB1; // bjet-1 tlv                     
      TLorentzVector vGenB2; // bjet-2 tlv                     

      // if (hreweightHH || hreweightHH2D || isHHsignal) // isHHsignal: only to do loop on genparts, but no rew
      if (isHHsignal || HHrewType == kFromHisto || HHrewType == kDynamic) // isHHsignal: only to do loop on genparts, but no rew
	{
	  // cout << "DEBUG: reweight!!!" << endl;
	  TLorentzVector vH1, vH2, vBoost, vSum;
	  float mHH = -1;
	  float ct1 = -999;
	  // loop on gen to find Higgs
	  int idx1 = -1;
	  int idx2 = -1;
	  int idx1last = -1;
	  int idx2last = -1;
	  // cout << " ------------------------ " << endl;
	  for (unsigned int igen = 0; igen < theBigTree.genpart_px->size(); igen++)
	    {
	      bool isFirst     = CheckBit (theBigTree.genpart_flags->at(igen), 12) ; // 12 = isFirstCopy
	      bool isLast      = CheckBit (theBigTree.genpart_flags->at(igen), 13) ; // 13 = isLastCopy
	      bool isHardScatt = CheckBit (theBigTree.genpart_flags->at(igen), 5) ; //   3 = isPromptTauDecayProduct
	      bool isHardProcess = CheckBit (theBigTree.genpart_flags->at(igen), 7) ; //  7 = isHardProcess, for b coming from H  
	      // bool isDirectPromptTauDecayProduct = CheckBit (theBigTree.genpart_flags->at(igen), 5) ; // 5 = isDirectPromptTauDecayProduct
	      int pdg = theBigTree.genpart_pdg->at(igen);
	      int mothIdx = theBigTree.genpart_TauMothInd->at(igen);
        
	      bool mothIsHardScatt = false;
	      if (mothIdx > -1)
		{
		  bool mothIsLast =  CheckBit(theBigTree.genpart_flags->at(mothIdx), 13);
		  // NB: I need t ask that the mother is last idx, otherwise I get a nonphysics "tauh" by the tauh builder function from the tau->tau "decay" in pythia
		  mothIsHardScatt = (mothIsLast && CheckBit (theBigTree.genpart_flags->at(mothIdx), 8)); // 0 = isPrompt(), 7: hardProcess , 8: fromHardProcess
		}

	      // if (abs(pdg) == 11 || abs(pdg) == 13 || abs(pdg) == 15 || abs(pdg) == 66615)
	      // {
	      //   bitset<32> bs (theBigTree.genpart_flags->at(igen)) ; 
	      //   cout << "/// igen = " << igen << " pdgId " << pdg << " flag=" << bs << " mothidx=" <<  theBigTree.genpart_TauMothInd->at(igen) << " px=" << theBigTree.genpart_px->at(igen) << endl;
	      //   // cout << "/// igen = " << igen << " pdgId " << pdg << " isFirst=" << isFirst << " isLast=" << isLast << " isHardScatt=" << isHardScatt << " mothIsHardScatt=" << mothIsHardScatt << " isDirectPromptTauDecayProduct=" << isDirectPromptTauDecayProduct << " mothIdx=" << theBigTree.genpart_TauMothInd->at(igen) << endl;
	      // }


	      if (abs(pdg) == 25)
		{
		  // cout << igen << " H boson: Px " << theBigTree.genpart_px->at(igen) << " first? " << isFirst << " decMode : " << theBigTree.genpart_HZDecayMode->at(igen) << endl;
		  if (isFirst)
		    {
		      if (idx1 >= 0 && idx2 >= 0)
			{
			  cout << "** ERROR: more than 2 H identified (first)" << endl;
			  continue;
			}
		      (idx1 == -1) ? (idx1 = igen) : (idx2 = igen) ;
		    }
		  if (isLast)
		    {
		      if (idx1last >= 0 && idx2last >= 0)
			{
			  cout << "** ERROR: more than 2 H identified (last)" << endl;
			  // continue; // no need to skip the event in this case -- dec mode just for studies
			}
		      (idx1last == -1) ? (idx1last = igen) : (idx2last = igen) ;
		    }
		}
        
	      if ( (abs(pdg) == 11 || abs(pdg) == 13 ) && isHardScatt && isLast && mothIsHardScatt)
		{
		  if (idx1hs == -1) idx1hs = igen;
		  else if (idx2hs == -1) idx2hs = igen;
		  else
		    {
		      cout << "** ERROR: there are more than 2 hard scatter tau dec prod: evt = " << theBigTree.EventNumber << endl;
		      // cout << "idx1: " << idx1hs << " --> pdg = " << theBigTree.genpart_pdg->at(idx1hs) << " px = " << theBigTree.genpart_px->at(idx1hs) << endl;
		      // cout << "idx2: " << idx2hs << " --> pdg = " << theBigTree.genpart_pdg->at(idx2hs) << " px = " << theBigTree.genpart_px->at(idx2hs) << endl;
		      // cout << "THIS: " << pdg << " px=" << theBigTree.genpart_px->at(igen) << endl;
		    }
		}
        
	      if ( abs(pdg) == 66615 && mothIsHardScatt)
		{
		  // cout << "  <<< preso" << endl;
		  if (idx1hs == -1) idx1hs = igen;
		  else if (idx2hs == -1) idx2hs = igen;
		  else
		    {
		      cout << "** ERROR: there are more than 2 hard scatter tau dec prod: evt = " << theBigTree.EventNumber << endl;
		      // cout << "idx1: " << idx1hs << " --> pdg = " << theBigTree.genpart_pdg->at(idx1hs) << " px = " << theBigTree.genpart_px->at(idx1hs) << endl;
		      // cout << "idx2: " << idx2hs << " --> pdg = " << theBigTree.genpart_pdg->at(idx2hs) << " px = " << theBigTree.genpart_px->at(idx2hs) << endl;
		      // cout << "THIS: " << pdg << " px=" << theBigTree.genpart_px->at(igen) << endl;
		    }
		}
		
	       // FRA DEBUG - find the bjets from the Higgs decay     
              if ( abs(pdg) == 5 && isHardProcess)
              {
        	if (idx1hs_b == -1) idx1hs_b = igen;
        	else if (idx2hs_b == -1) idx2hs_b = igen;
        	else
        	{
        	  cout << "** ERROR: there are more than 2 hard scatter b quarks: evt = " << theBigTree.EventNumber << endl;
        	}
              }

	    }
	    

	  if (idx1 == -1 || idx2 == -1)
	    {
	      cout << "** ERROR: couldn't find 2 H (first)" << endl;
	      continue;
	    }

	  if (idx1last != -1 && idx2last != -1) // this is not critical if not found
	    {
	      // store gen decay mode of the two H identified
	      theSmallTree.m_genDecMode1 = theBigTree.genpart_HZDecayMode->at(idx1last);
	      theSmallTree.m_genDecMode2 = theBigTree.genpart_HZDecayMode->at(idx2last);
        
	      // cout << "THIS H decay mode: " << theSmallTree.m_genDecMode1 << " " << theSmallTree.m_genDecMode2 << endl;

	      // // get tau decaying one
	      // int idxTauDecayed = (theBigTree.genpart_HZDecayMode->at(idx1last) != 8 ? idx1last : idx2last);

	      // // find hard scatter daughters and check if they match this decay type
	      // pair<int, int> hsProds = oph.getHardScatterSons()
	      // int hsIdx1 = hsProds.first;
	      // int hsIdx2 = hsProds.second;
	    }
	  else
	    cout << "** ERROR: couldn't find 2 H (last)" << endl;

	  if (idx1hs != -1 && idx2hs != -1)
	    {
	      pdg1hs = theBigTree.genpart_pdg->at(idx1hs);
	      pdg2hs = theBigTree.genpart_pdg->at(idx2hs);

	      if      (abs(pdg1hs) == 11) t1hs = 1;
	      else if (abs(pdg1hs) == 13) t1hs = 0;
	      else                        t1hs = 2;

	      if      (abs(pdg2hs) == 11) t2hs = 1;
	      else if (abs(pdg2hs) == 13) t2hs = 0;
	      else                        t2hs = 2;

	      if (oph.getPairType(t1hs, t2hs) != (theSmallTree.m_genDecMode1 + theSmallTree.m_genDecMode2 - 8))
		{
		  cout << "** ERROR: decay modes do not match! " << theBigTree.genpart_pdg->at(idx1hs) << " " << theBigTree.genpart_pdg->at(idx2hs) << " != "
		       << ( theSmallTree.m_genDecMode1 + theSmallTree.m_genDecMode2 - 8) << endl;
		}
	      vHardScatter1.SetPxPyPzE (theBigTree.genpart_px->at(idx1hs), theBigTree.genpart_py->at(idx1hs), theBigTree.genpart_pz->at(idx1hs), theBigTree.genpart_e->at(idx1hs));
	      vHardScatter2.SetPxPyPzE (theBigTree.genpart_px->at(idx2hs), theBigTree.genpart_py->at(idx2hs), theBigTree.genpart_pz->at(idx2hs), theBigTree.genpart_e->at(idx2hs));
	    }
	  else
	    cout << "** ERROR: couldn't find 2 H->tautau gen dec prod " << idx1hs << " " << idx2hs << endl;


	  vH1.SetPxPyPzE (theBigTree.genpart_px->at(idx1), theBigTree.genpart_py->at(idx1), theBigTree.genpart_pz->at(idx1), theBigTree.genpart_e->at(idx1) );
	  vH2.SetPxPyPzE (theBigTree.genpart_px->at(idx2), theBigTree.genpart_py->at(idx2), theBigTree.genpart_pz->at(idx2), theBigTree.genpart_e->at(idx2) );
	  vSum = vH1+vH2;
	  mHH = vSum.M();
	  vH1.Boost(-vSum.BoostVector());                     
	  ct1 = vH1.CosTheta();
	  
	  
	  // FRA DEBUG - build gen b jets       
          if (idx1hs_b != -1 && idx2hs_b != -1)
          {
              vGenB1.SetPxPyPzE (theBigTree.genpart_px->at(idx1hs_b), theBigTree.genpart_py->at(idx1hs_b), theBigTree.genpart_pz->at(idx1hs_b), theBigTree.genpart_e->at(idx1hs_b) );
              vGenB2.SetPxPyPzE (theBigTree.genpart_px->at(idx2hs_b), theBigTree.genpart_py->at(idx2hs_b), theBigTree.genpart_pz->at(idx2hs_b), theBigTree.genpart_e->at(idx2hs_b) );
          }
          else
	    cout << "** ERROR: couldn't find 2 H->bb gen dec prod " << idx1hs_b << " " << idx2hs_b << endl;


	  // assign a weight depending on the reweight type 

	  if (hreweightHH && HHrewType == kFromHisto) // 1D
	    {
	      int ibin = hreweightHH->FindBin(mHH);
	      HHweight = hreweightHH->GetBinContent(ibin);
	    }
	  else if (hreweightHH2D && HHrewType == kFromHisto) // 2D
	    {
	      int ibin = hreweightHH2D->FindBin(mHH, ct1);
	      HHweight = hreweightHH2D->GetBinContent(ibin);        
	    }
	  else if (HHrewType == kDynamic)
	    {
	      // HHweight = hhreweighter->getWeight(kl_rew, kt_rew, mHH, ct1);
	      if (c2_rew < -990 || cg_rew < -990 || c2g_rew < -990) // no valid BSM coefficients -- just kl/kt reweight (for backwards compatibility)
		HHweight = hhreweighter->getWeight(kl_rew, kt_rew, mHH, ct1);
	      else // full 5D reweight
		HHweight = hhreweighter->getWeight(kl_rew, kt_rew, c2_rew, cg_rew, c2g_rew, mHH, ct1);
	    }

	  theSmallTree.m_genMHH = mHH;
	  theSmallTree.m_genCosth = ct1;

	  // cout << " ........... GEN FINISHED ........... " << " evt=" << theBigTree.EventNumber << " run=" << theBigTree.RunNumber << " lumi=" << theBigTree.lumi << endl;

	}

      ///////////////////////////////////////////////////////////
      // END of gen related stuff -- compute tot number of events

      int genHHDecMode = (isHHsignal ? theSmallTree.m_genDecMode1 + theSmallTree.m_genDecMode2 - 8 : 0);
      if (genHHDecMode < 0)
	{
	  genHHDecMode = 0; // dummy protection if couldn't find initial H
	  cout << "** ERROR: negative dec mode, for safety set it ot 0" << endl;
	}
      double EvtW;
      if (theBigTree.npu >= 0 && theBigTree.npu <= 99) // good PU weights
        EvtW = isMC ? (theBigTree.aMCatNLOweight * reweight.weight(PUReweight_MC,PUReweight_target,theBigTree.npu,PUreweightFile) * topPtReweight * HHweight) : 1.0;
      else if (theBigTree.npu >= 100)                  // use the last available bin for the PU weight
        EvtW = isMC ? (theBigTree.aMCatNLOweight * reweight.weight(PUReweight_MC,PUReweight_target,99, PUreweightFile) * topPtReweight * HHweight) : 1.0;
      else                                             // if npu<0 --> bug in MC --> weight=0
        EvtW = isMC ? 0.0 : 1.0;

      if (isMC)
	{
	  totalEvents += EvtW;
	}
      else
	{
	  totalEvents += 1 ;
	}
      ec.Increment ("all", EvtW);
      if (isHHsignal) ecHHsig[genHHDecMode].Increment ("all", EvtW);

      ++totalNoWeightsEventsNum ;
    
    
      // ----------------------------------------------------------
      //  apply MET filters

      int metbit = theBigTree.metfilterbit;
     
      int metpass = metbit & (1 << 0); //"Flag_goodVertices"
      metpass += metbit & (1 << 2);    //"Flag_HBHENoiseIsoFilter"
      metpass += metbit & (1 << 5);    //"Flag_BadPFMuonFilter"
      // metpass += metbit & (1 << 6);    //"Flag_BadChargedCandidateFilter" update March2019: removed 
      // Update Fall17 94X
      metpass += metbit & (1 << 1);    //"Flag_HBHENoiseFilter"
      metpass += metbit & (1 << 3);    //"Flag_EcalDeadCellTriggerPrimitiveFilter"
      metpass += metbit & (1 << 4);    //"Flag_globalSuperTightHalo2016Filter"
      if(!isMC) metpass += metbit & (1 << 7); // "Flag_eeBadScFilter" not suggested on twiki; EDIT: now suggested for data (Moriond2018)
      if (theBigTree.passecalBadCalibFilterUpdate) metpass += 1;
      //(metpass <= 0) cout << " - failed metbit(9): " << std::bitset<9>(metbit) << endl; //FRA

      //update March2019: "Flag_BadChargedCandidateFilter" removed
      if(isMC && metpass < 7) continue ;
      if(!isMC && metpass < 8) continue ;   
     
      ec.Increment ("METfilter", EvtW);
      if (isHHsignal) ecHHsig[genHHDecMode].Increment ("METfilter", EvtW);

      // ----------------------------------------------------------
      // require that the event is not affected by the Bad/Clone Muon problem -- for 2016 data
      //if (theBigTree.NBadMu > 0) continue ; //FRA: Sync Feb2018
      ec.Increment ("NoBadMuons", EvtW);
      if (isHHsignal) ecHHsig[genHHDecMode].Increment ("NoBadMuons", EvtW);

      // ----------------------------------------------------------
      // require at least 1 pair
      if (theBigTree.indexDau1->size () == 0) continue ;
      ec.Increment ("PairExists", EvtW);
      if (isHHsignal) ecHHsig[genHHDecMode].Increment ("PairExists", EvtW);

      // ----------------------------------------------------------
      // assess the pair type 
      // loop over the daughters to select pair type: mu > e > tau
      // apply tight baseline (with iso to check)
    
      int nmu = 0;
      int nmu10 = 0; // low pt muons for DY sideband, not entering in nmu
      int nele = 0;
      int nele10 = 0;
    
      if(DEBUG)
      {
        cout << "***** DEBUG: reco particles (remember: check if baseline sels are aligned to OfflineProducerHelper)" << endl;
      }

      for (unsigned int idau = 0; idau < theBigTree.daughters_px->size(); ++idau)
      {
        int dauType = theBigTree.particleType->at(idau);
        if (oph.isMuon(dauType))
        {
          bool passMu   = oph.muBaseline (&theBigTree, idau, 21., 2.1, 0.15, OfflineProducerHelper::MuTight, string("All") , (DEBUG ? true : false));
          bool passMu10 = oph.muBaseline (&theBigTree, idau, 10., 2.4, 0.15, OfflineProducerHelper::MuTight, string("All") , (DEBUG ? true : false));

          if (passMu) ++nmu;
          else if (passMu10) ++nmu10;
        }
        else if (oph.isElectron(dauType))
        {
          bool passEle   = oph.eleBaseline (&theBigTree, idau, 25., 2.1, 0.1, OfflineProducerHelper::EMVATight, string("All") , (DEBUG ? true : false));
          bool passEle10 = oph.eleBaseline (&theBigTree, idau, 10., 2.1, 0.1, OfflineProducerHelper::EMVATight, string("All") , (DEBUG ? true : false));

          if (passEle) ++nele;
          else if (passEle10) ++nele10;
        }

        if(DEBUG)
        {
          TLorentzVector dauTlvDebug (
                  theBigTree.daughters_px->at (idau),
                  theBigTree.daughters_py->at (idau),
                  theBigTree.daughters_pz->at (idau),
                  theBigTree.daughters_e ->at (idau));

          // NB: remember to align this debug to the content of OfflineProducerHelper
          cout << ".... reco part "
          << " idx dau="   << setw(3)  << left << idau
          << " type="      << setw(3)  << left << dauType
          << " pt="        << setw(10) << left << dauTlvDebug.Pt()
          << " eta="       << setw(10) << left << dauTlvDebug.Eta()
          << " phi="       << setw(10) << left << dauTlvDebug.Phi()
          << " iso="       << setw(10) << left << getIso (idau, dauTlvDebug.Pt (), theBigTree)
          << " dxy="       << setw(15) << left << theBigTree.dxy->at(idau)
          << " dz="        << setw(15) << left << theBigTree.dz->at(idau)
          << " mutightID=" << setw(3)  << left << CheckBit(theBigTree.daughters_muonID->at(idau),3)
          << " mubase="    << setw(3)  << left << oph.muBaseline (&theBigTree, idau, 10., 2.4, 0.15, OfflineProducerHelper::MuTight, string("All"))
          << " ebase="     << setw(3)  << left << oph.eleBaseline(&theBigTree, idau, 10., 2.1, 0.1, OfflineProducerHelper::EMVATight, string("All"))
          << endl;
        }
      } // end loop on daughters

      int pairType = 2; // tau tau
      if (nmu > 0)
      {
        if (nmu == 1 && nmu10 == 0)
          pairType = 0 ; // mu tau
        else
          pairType = 3 ; // mu mu
      }
      else if (nele > 0)
      {
      if (nele == 1 && nele10 == 0)
        pairType = 1;  // ele tau
      else
        pairType = 4 ; // ele ele
      }

      // ----------------------------------------------------------
      // choose the first pair passing baseline and being of the right pair type

      int chosenTauPair = -1;

      if (pairType == 2 && sortStrategyThTh == kHTauTau)
      {
        chosenTauPair = oph.getBestPairHTauTau(&theBigTree, leptonSelectionFlag, (DEBUG ? true : false));
      }

      else if (pairType == 2 && sortStrategyThTh == kPtAndRawIso)
      {
        chosenTauPair = oph.getBestPairPtAndRawIsoOrd(&theBigTree, leptonSelectionFlag, (DEBUG ? true : false));
      }

      // (mu tauh), (e tauh), (tauhtauh && kLLRFramDefault)
      else
      {
        if(DEBUG)
        {
          for (unsigned int iPair = 0 ; iPair < theBigTree.indexDau1->size () ; ++iPair)
          {
            int t_firstDaughterIndex  = theBigTree.indexDau1->at (iPair) ;
            int t_secondDaughterIndex = theBigTree.indexDau2->at (iPair) ;
            int t_type1 = theBigTree.particleType->at (t_firstDaughterIndex) ;
            int t_type2 = theBigTree.particleType->at (t_secondDaughterIndex) ;
            cout << " **## Pair: " << iPair << " indexes(" <<t_firstDaughterIndex << "," << t_secondDaughterIndex << ") pairType: "<< pairType << " getPairType: "<< oph.getPairType (t_type1, t_type2) << endl;
          }

          for (unsigned int iLep = 0 ; (iLep < theBigTree.daughters_px->size ()) ; ++iLep)
          {
            TLorentzVector tlv_dummyLepton(
                theBigTree.daughters_px->at (iLep),
                theBigTree.daughters_py->at (iLep),
                theBigTree.daughters_pz->at (iLep),
                theBigTree.daughters_e ->at (iLep)) ;

            cout << " idx="  << iLep
               << " type=" << theBigTree.particleType->at(iLep)
               << " DM="   << theBigTree.decayMode->at(iLep)
               << " DMold="<< theBigTree.daughters_decayModeFindingOldDMs->at(iLep)
               << " pt="   << tlv_dummyLepton.Pt()
               << " eta="  << tlv_dummyLepton.Eta()
               << " phi="  << tlv_dummyLepton.Phi()
               << " iso="  << getIso (iLep, tlv_dummyLepton.Pt (), theBigTree)
               << " dxy="  << theBigTree.dxy->at(iLep)
               << " dz="   << theBigTree.dz->at(iLep)
               << endl;
          }
        } // end DEBUG

        for (unsigned int iPair = 0 ; iPair < theBigTree.indexDau1->size () ; ++iPair)
        {
          int t_firstDaughterIndex  = theBigTree.indexDau1->at (iPair) ;
          int t_secondDaughterIndex = theBigTree.indexDau2->at (iPair) ;
          int t_type1 = theBigTree.particleType->at (t_firstDaughterIndex) ;
          int t_type2 = theBigTree.particleType->at (t_secondDaughterIndex) ;
          if ( oph.getPairType (t_type1, t_type2) != pairType ) continue ;
          string baselineSels = ( (pairType <= 2) ? leptonSelectionFlag : (leptonSelectionFlag + "-Iso")) ; // for ee, mumu, emu, ask isolation in baseline

          if ( oph.pairPassBaseline (&theBigTree, iPair, baselineSels, (DEBUG ? true : false) ) ) // rlx izo to limit to tau iso < 7 -- good for sideband
          {
            chosenTauPair = iPair;
            break;
          }
        }
      } // end else (mu tauh), (e tauh), (tauhtauh && kLLRFramDefault)

      if(DEBUG)
      {
        cout << "**** DEBUG : chosen pair : " << chosenTauPair << " str=" << leptonSelectionFlag << " pairType==" << pairType << endl;
        cout << "     ... going to list all pairs of same pairType as the one assessed with reco leptons" << endl;
        for (unsigned int iPair = 0 ; iPair < theBigTree.indexDau1->size () ; ++iPair)
        {
          int t_firstDaughterIndex  = theBigTree.indexDau1->at (iPair) ;
          int t_secondDaughterIndex = theBigTree.indexDau2->at (iPair) ;
          int t_type1 = theBigTree.particleType->at (t_firstDaughterIndex) ;
          int t_type2 = theBigTree.particleType->at (t_secondDaughterIndex) ;
          if ( oph.getPairType (t_type1, t_type2) != pairType ) continue ;
          TLorentzVector tttt (
            theBigTree.daughters_px->at (t_secondDaughterIndex),
            theBigTree.daughters_py->at (t_secondDaughterIndex),
            theBigTree.daughters_pz->at (t_secondDaughterIndex),
            theBigTree.daughters_e ->at (t_secondDaughterIndex));

          cout << "- " << iPair << " idx1=" << t_firstDaughterIndex << " idx2=" << t_secondDaughterIndex << " isoTau=" <<  getIso (t_secondDaughterIndex, tttt.Pt (), theBigTree) << " tauPt=" << tttt.Pt() << " type2=" << t_type2 << " eta=" << tttt.Eta() << " phi=" << tttt.Phi() << endl;
          cout << "   >>> DM=" << theBigTree.daughters_decayModeFindingOldDMs->at(t_secondDaughterIndex) << " dxy=" << theBigTree.dxy->at(t_secondDaughterIndex) << " dz=" << theBigTree.dz->at(t_secondDaughterIndex) << endl;
        }
      }

      if (chosenTauPair < 0) continue; // no pair found over baseline

      ec.Increment ("PairFoundBaseline", EvtW);
      if (isHHsignal)
      {
        ecHHsig[genHHDecMode].Increment ("PairFoundBaseline", EvtW);
        if (pairType == genHHDecMode)
          ecHHsig[genHHDecMode].Increment ("PairMatchesGen", EvtW);
      }

      // ----------------------------------------------------------
      // pair has been assessed , check trigger information

      // in TauTau channel make sure the first tau is the most isolated one
      int tmp_firstDaughterIndex  = theBigTree.indexDau1->at (chosenTauPair) ;
      int tmp_secondDaughterIndex = theBigTree.indexDau2->at (chosenTauPair) ;

      if (pairType == 2 && (theBigTree.daughters_byDeepTau2017v2p1VSjetraw->at(theBigTree.indexDau1->at (chosenTauPair)) < theBigTree.daughters_byDeepTau2017v2p1VSjetraw->at(theBigTree.indexDau2->at (chosenTauPair))) )
      {
          tmp_firstDaughterIndex  = theBigTree.indexDau2->at (chosenTauPair) ;
          tmp_secondDaughterIndex = theBigTree.indexDau1->at (chosenTauPair) ;
      }

      const int firstDaughterIndex  = tmp_firstDaughterIndex ;
      const int secondDaughterIndex = tmp_secondDaughterIndex ;
      const int type1 = theBigTree.particleType->at (firstDaughterIndex) ;
      const int type2 = theBigTree.particleType->at (secondDaughterIndex) ;        
      const int pType = pairType ;

      const int isOS  = theBigTree.isOSCand->at (chosenTauPair) ;
      bool lep1HasTES = false;
      bool lep2HasTES = false;
      bool lep1HasEES = false;
      bool lep2HasEES = false;

      int DM1 = theBigTree.decayMode->at(firstDaughterIndex);
      int DM2 = theBigTree.decayMode->at(secondDaughterIndex);
      if (DM1 == 2) DM1 = 1;
      if (DM2 == 2) DM2 = 1;

      if (DM1 == 11) DM1 = 10; //Davide: DM 11 should be used in combination with deepTau: temporarily forced to 10 because SF ar not available
      if (DM2 == 11) DM2 = 10;

      //save decaymode
      if(pairType == 0){ //mutauh
        theSmallTree.m_dau1_decayMode = -1;
        theSmallTree.m_dau2_decayMode = DM2;
      }

      if(pairType == 1){ //etauh
        theSmallTree.m_dau1_decayMode = -1;
        theSmallTree.m_dau2_decayMode = DM2;
      }

      if(pairType == 2){ //tauhtauh
        theSmallTree.m_dau1_decayMode = DM1;
        theSmallTree.m_dau2_decayMode = DM2;
      }

      if(pairType > 2){ //pairs without tauh
        theSmallTree.m_dau1_decayMode = -1;
        theSmallTree.m_dau2_decayMode = -1;
      }

      if (isMC)
      {
        int nRealTaus= 0;
        lep1HasTES = (theBigTree.genmatch->at(firstDaughterIndex)  == 5 ? true : false);
        lep2HasTES = (theBigTree.genmatch->at(secondDaughterIndex) == 5 ? true : false);
        lep1HasEES = ( (theBigTree.genmatch->at(firstDaughterIndex)==1 || theBigTree.genmatch->at(firstDaughterIndex)==3) ? true : false);
        lep2HasEES = ( (theBigTree.genmatch->at(secondDaughterIndex)==1 || theBigTree.genmatch->at(secondDaughterIndex)==3) ? true : false);
        theSmallTree.m_isTau1real = (lep1HasTES == true ? 1 : 0); // -1: data; 0: fake tau, 1: real tau
        theSmallTree.m_isTau2real = (lep2HasTES == true ? 1 : 0);
        if (lep1HasTES) nRealTaus +=1;
        if (lep2HasTES) nRealTaus +=1;
        theSmallTree.m_nRealTaus = nRealTaus;                     // -1: data; > 0: # real taus in MC
      }


      const TLorentzVector tlv_firstLepton (
					    theBigTree.daughters_px->at (firstDaughterIndex),
					    theBigTree.daughters_py->at (firstDaughterIndex),
					    theBigTree.daughters_pz->at (firstDaughterIndex),
					    theBigTree.daughters_e->at (firstDaughterIndex)
					    );

      const TLorentzVector tlv_secondLepton (
					     theBigTree.daughters_px->at (secondDaughterIndex),
					     theBigTree.daughters_py->at (secondDaughterIndex),
					     theBigTree.daughters_pz->at (secondDaughterIndex),
					     theBigTree.daughters_e->at (secondDaughterIndex)
					     );

      // scale up: only applies to tau
      // TES up/down
      TLorentzVector tlv_firstLepton_tauup (tlv_firstLepton);
      TLorentzVector tlv_firstLepton_taudown (tlv_firstLepton);
      if (lep1HasTES)
      {
        tlv_firstLepton_tauup.SetPxPyPzE (
                        theBigTree.daughters_px_TauUp->at (firstDaughterIndex),
                        theBigTree.daughters_py_TauUp->at (firstDaughterIndex),
                        theBigTree.daughters_pz_TauUp->at (firstDaughterIndex),
                        theBigTree.daughters_e_TauUp->at (firstDaughterIndex)
                        );
        tlv_firstLepton_taudown.SetPxPyPzE (
                          theBigTree.daughters_px_TauDown->at (firstDaughterIndex),
                          theBigTree.daughters_py_TauDown->at (firstDaughterIndex),
                          theBigTree.daughters_pz_TauDown->at (firstDaughterIndex),
                          theBigTree.daughters_e_TauDown->at (firstDaughterIndex)
                          );
      }

      TLorentzVector tlv_secondLepton_tauup (tlv_secondLepton);
      TLorentzVector tlv_secondLepton_taudown (tlv_secondLepton);
      if (lep2HasTES)
      {
        tlv_secondLepton_tauup.SetPxPyPzE (
                         theBigTree.daughters_px_TauUp->at (secondDaughterIndex),
                         theBigTree.daughters_py_TauUp->at (secondDaughterIndex),
                         theBigTree.daughters_pz_TauUp->at (secondDaughterIndex),
                         theBigTree.daughters_e_TauUp->at (secondDaughterIndex)
                         );
        tlv_secondLepton_taudown.SetPxPyPzE (
                           theBigTree.daughters_px_TauDown->at (secondDaughterIndex),
                           theBigTree.daughters_py_TauDown->at (secondDaughterIndex),
                           theBigTree.daughters_pz_TauDown->at (secondDaughterIndex),
                           theBigTree.daughters_e_TauDown->at (secondDaughterIndex)
                           );
      }

      // EES up/down
      TLorentzVector tlv_firstLepton_eleup (tlv_firstLepton);
      TLorentzVector tlv_firstLepton_eledown (tlv_firstLepton);
      if (lep1HasEES)
      {
        tlv_firstLepton_eleup.SetPxPyPzE (
                        theBigTree.daughters_px_EleUp->at (firstDaughterIndex),
                        theBigTree.daughters_py_EleUp->at (firstDaughterIndex),
                        theBigTree.daughters_pz_EleUp->at (firstDaughterIndex),
                        theBigTree.daughters_e_EleUp->at (firstDaughterIndex)
                        );
        tlv_firstLepton_eledown.SetPxPyPzE (
                          theBigTree.daughters_px_EleDown->at (firstDaughterIndex),
                          theBigTree.daughters_py_EleDown->at (firstDaughterIndex),
                          theBigTree.daughters_pz_EleDown->at (firstDaughterIndex),
                          theBigTree.daughters_e_EleDown->at (firstDaughterIndex)
                          );
      }

      TLorentzVector tlv_secondLepton_eleup (tlv_secondLepton);
      TLorentzVector tlv_secondLepton_eledown (tlv_secondLepton);
      if (lep2HasEES)
      {
        tlv_secondLepton_eleup.SetPxPyPzE (
                         theBigTree.daughters_px_EleUp->at (secondDaughterIndex),
                         theBigTree.daughters_py_EleUp->at (secondDaughterIndex),
                         theBigTree.daughters_pz_EleUp->at (secondDaughterIndex),
                         theBigTree.daughters_e_EleUp->at (secondDaughterIndex)
                         );
        tlv_secondLepton_eledown.SetPxPyPzE (
                           theBigTree.daughters_px_TauDown->at (secondDaughterIndex),
                           theBigTree.daughters_py_TauDown->at (secondDaughterIndex),
                           theBigTree.daughters_pz_TauDown->at (secondDaughterIndex),
                           theBigTree.daughters_e_TauDown->at (secondDaughterIndex)
                           );
      }

      if (DEBUG)
      {
        cout << "------- CHOSEN PAIR -------" << endl;
        cout << " dau_1 (pt,eta,phi,iso): "<<tlv_firstLepton.Pt()<<" "<<tlv_firstLepton.Eta()<<" "<<tlv_firstLepton.Phi()<<" "<<getIso(firstDaughterIndex, tlv_firstLepton.Pt (), theBigTree)<<endl;
        cout << " dau_2 (pt,eta,phi,iso): "<<tlv_secondLepton.Pt()<<" "<<tlv_secondLepton.Eta()<<" "<<tlv_secondLepton.Phi()<<" "<<getIso(secondDaughterIndex, tlv_secondLepton.Pt (), theBigTree)<<endl;
        cout << "---------------------"<< endl;
      }

      if (DEBUG)
      {
        cout << "------- TAU TES DEBUG -------" << endl;
        cout << " tau1 centr: " << tlv_firstLepton.Pt() << " / " << tlv_firstLepton.Eta() << endl;
        cout << " tau1 up: " << tlv_firstLepton_tauup.Pt() << " / " << tlv_firstLepton_tauup.Eta() << endl;
        cout << " tau1 dw: " << tlv_firstLepton_taudown.Pt() << " / " << tlv_firstLepton_taudown.Eta() << endl;

        cout << " tau2 centr: " << tlv_secondLepton.Pt() << " / " << tlv_secondLepton.Eta() << endl;
        cout << " tau2 up: " << tlv_secondLepton_tauup.Pt() << " / " << tlv_secondLepton_tauup.Eta() << endl;
        cout << " tau2 dw: " << tlv_secondLepton_taudown.Pt() << " / " << tlv_secondLepton_taudown.Eta() << endl;
        cout << "---------------------"<< endl;
      }

      if (DEBUG)
      {
        cout << "------- TAU EES DEBUG -------" << endl;
        cout << " tau1 centr: " << tlv_firstLepton.Pt() << " / " << tlv_firstLepton.Eta() << endl;
        cout << " tau1 up: " << tlv_firstLepton_eleup.Pt() << " / " << tlv_firstLepton_eleup.Eta() << endl;
        cout << " tau1 dw: " << tlv_firstLepton_eledown.Pt() << " / " << tlv_firstLepton_eledown.Eta() << endl;

        cout << " tau2 centr: " << tlv_secondLepton.Pt() << " / " << tlv_secondLepton.Eta() << endl;
        cout << " tau2 up: " << tlv_secondLepton_eleup.Pt() << " / " << tlv_secondLepton_eleup.Eta() << endl;
        cout << " tau2 dw: " << tlv_secondLepton_eledown.Pt() << " / " << tlv_secondLepton_eledown.Eta() << endl;
        cout << "---------------------"<< endl;
      }


      // DATA strategy
      int pass_triggerbit = 0;
      if (applyTriggers)
      {
        Long64_t triggerbit = theBigTree.triggerbit;

        Long64_t matchFlag1 = (Long64_t) theBigTree.daughters_trgMatched->at(firstDaughterIndex);
        Long64_t matchFlag2 = (Long64_t) theBigTree.daughters_trgMatched->at(secondDaughterIndex);
        Long64_t goodTriggerType1 = (Long64_t) theBigTree.daughters_isGoodTriggerType->at(firstDaughterIndex);
        Long64_t goodTriggerType2 = (Long64_t) theBigTree.daughters_isGoodTriggerType->at(secondDaughterIndex);

        Long64_t trgNotOverlapFlag = (Long64_t) theBigTree.mothers_trgSeparateMatch->at(chosenTauPair);
        bool passTrg = trigReader.checkOR (pairType,triggerbit, &pass_triggerbit, matchFlag1, matchFlag2, trgNotOverlapFlag, goodTriggerType1, goodTriggerType2, tlv_firstLepton.Pt(), tlv_secondLepton.Pt(), tlv_secondLepton.Eta()) ;

        // L1 match for 2017 data
        if (isMC)
        {
          if (pairType == 2)
          {
            bool passL1IsoTau32 = false;
            if (theBigTree.daughters_highestEt_L1IsoTauMatched->at(firstDaughterIndex) > 32 && theBigTree.daughters_highestEt_L1IsoTauMatched->at(secondDaughterIndex) > 32)
            {
              passL1IsoTau32 = true;
            }
            if (!passL1IsoTau32) passTrg = false;
          }
        }

        // !! FIXME !! --> update the trigger bits to the right paths
        // Weight to be applied for IsoMu24, prescaled for ~3fb-1 in 2017
        // MuTau: if it didn't also pass IsoMu27 or MuTau trigger
        if (pairType == 0 && isMC){
            if (CheckBit(pass_triggerbit,0) && !CheckBit(pass_triggerbit,1) && !CheckBit(pass_triggerbit,7)) theSmallTree.m_prescaleWeight =  (41557. - 3625.) / 41557.;
        }
        // MuMu: if it didn't also pass IsoMu27
        if (pairType == 3 && isMC){
            if (CheckBit(pass_triggerbit,0) && !CheckBit(pass_triggerbit,1)) theSmallTree.m_prescaleWeight =  (41557. - 3625.) / 41557.;
        }

        if(DEBUG)
        {
          if (isMC && (pairType == 2 || pairType == 1 || pairType == 0))
          {
            cout << "passTrg? "<< trigReader.checkOR (pairType,triggerbit, &pass_triggerbit, matchFlag1, matchFlag2, trgNotOverlapFlag, goodTriggerType1, goodTriggerType2, tlv_firstLepton.Pt(), tlv_secondLepton.Pt(), tlv_secondLepton.Eta())<<endl;
            cout << "L1 pt1 "<<theBigTree.daughters_highestEt_L1IsoTauMatched->at(firstDaughterIndex) << "L1 pt2 "<<theBigTree.daughters_highestEt_L1IsoTauMatched->at(secondDaughterIndex)<<endl;
            cout << "---> passTrg? "<<passTrg<<endl;
          }
        }

        isVBFfired = trigReader.isVBFfired(triggerbit, matchFlag1, matchFlag2, trgNotOverlapFlag, goodTriggerType1, goodTriggerType2, tlv_firstLepton.Pt(), tlv_secondLepton.Pt());

        bool triggerAccept = false;
        triggerAccept = passTrg;

        if(DEBUG)
        {
          if(pairType == 0)//MuTau
          {
            trigReader.listMuTau(triggerbit, matchFlag1, matchFlag2, trgNotOverlapFlag, goodTriggerType1, goodTriggerType2);
          }
          if(pairType == 1)//ETau
          {
             trigReader.listETau(triggerbit, matchFlag1, matchFlag2, trgNotOverlapFlag, goodTriggerType1, goodTriggerType2);
          }
          if(pairType == 2)//TauTau
          {
            trigReader.listTauTau(triggerbit, matchFlag1, matchFlag2, trgNotOverlapFlag, goodTriggerType1, goodTriggerType2);
          }
        }

        if (DEBUG) std::cout << "----> Final Trigger passed? " << triggerAccept << std::endl;
        if (!triggerAccept) continue;

        theSmallTree.m_pass_triggerbit = pass_triggerbit;
        ec.Increment ("Trigger", EvtW); // for data, EvtW is 1.0
        if (isHHsignal && pairType == genHHDecMode) ecHHsig[genHHDecMode].Increment ("Trigger", EvtW);
      }


      // ----------------------------------------------------------
      // pair selection is now complete, compute oher quantitites
    
      TLorentzVector tlv_tauH = tlv_firstLepton + tlv_secondLepton ;
      TLorentzVector tlv_tauH_SVFIT ;

      theSmallTree.m_tauH_SVFIT_mass = theBigTree.SVfitMass->at (chosenTauPair) ;
      theSmallTree.m_tauH_SVFIT_mass_up   = (isMC ? theBigTree.SVfitMassTauUp->at (chosenTauPair) : theSmallTree.m_tauH_SVFIT_mass);
      theSmallTree.m_tauH_SVFIT_mass_down = (isMC ? theBigTree.SVfitMassTauDown->at (chosenTauPair) : theSmallTree.m_tauH_SVFIT_mass);
      theSmallTree.m_tauH_SVFIT_mass_METup   = (isMC ? theBigTree.SVfitMassMETUp->at (chosenTauPair) : theSmallTree.m_tauH_SVFIT_mass);
      theSmallTree.m_tauH_SVFIT_mass_METdown = (isMC ? theBigTree.SVfitMassMETDown->at (chosenTauPair) : theSmallTree.m_tauH_SVFIT_mass);
      // in case the SVFIT mass is calculated
      if (theBigTree.SVfitMass->at (chosenTauPair) > -900.)
	{
	  theSmallTree.m_tauH_SVFIT_pt     = theBigTree.SVfit_pt->at  (chosenTauPair) ;
	  theSmallTree.m_tauH_SVFIT_eta    = theBigTree.SVfit_eta->at (chosenTauPair) ;
	  theSmallTree.m_tauH_SVFIT_phi    = theBigTree.SVfit_phi->at (chosenTauPair) ;
	  theSmallTree.m_tauH_SVFIT_METphi = theBigTree.SVfit_fitMETPhi->at (chosenTauPair) ;
	  theSmallTree.m_tauH_SVFIT_METrho = theBigTree.SVfit_fitMETRho->at (chosenTauPair) ;      

	  tlv_tauH_SVFIT.SetPtEtaPhiM (
				       theBigTree.SVfit_pt->at (chosenTauPair),
				       theBigTree.SVfit_eta->at (chosenTauPair),
				       theBigTree.SVfit_phi->at (chosenTauPair),
				       theBigTree.SVfitMass->at (chosenTauPair)
				       ) ;

	  theSmallTree.m_tauHsvfitMet_deltaPhi = deltaPhi (theBigTree.metphi, tlv_tauH_SVFIT.Phi ()) ;
	  theSmallTree.m_ditau_deltaR_per_tauHsvfitpt = tlv_firstLepton.DeltaR(tlv_secondLepton) * tlv_tauH_SVFIT.Pt();
	}

    if (DEBUG)
    {
      cout << "------- SVFIT ------" << endl;
      cout << " is calculated ? " << theBigTree.SVfitMass->at(chosenTauPair) << endl;
      cout << " pt/eta/phi: " << tlv_tauH_SVFIT.Pt() << " / " << tlv_tauH_SVFIT.Eta() << " / " << tlv_tauH_SVFIT.Phi() << endl;
      cout << " is calculated UP ? " << theSmallTree.m_tauH_SVFIT_mass_up << endl;
      cout << " is calculated DOWN ? " << theSmallTree.m_tauH_SVFIT_mass_down << endl;
      cout << "--------------------" << endl;
    }

      // check if the selected leptons A,B match the gen hard scatter products 1,2
      if (isHHsignal)
	{
	  bool type1A = (abs(t1hs) == abs(type1));
	  bool type1B = (abs(t1hs) == abs(type2));
	  bool type2A = (abs(t2hs) == abs(type1));
	  bool type2B = (abs(t2hs) == abs(type2));
    
	  bool dR1A = (vHardScatter1.DeltaR(tlv_firstLepton) < 0.5);
	  bool dR1B = (vHardScatter1.DeltaR(tlv_secondLepton) < 0.5);
	  bool dR2A = (vHardScatter2.DeltaR(tlv_firstLepton) < 0.5);
	  bool dR2B = (vHardScatter2.DeltaR(tlv_secondLepton) < 0.5);

	  // FIXME: fill gen matched info pt/eta/phi/e
	  if ( (type1A && dR1A) || (type2A && dR2A))
	    theSmallTree.m_hasgenmatch1 = true;
	  if ( (type1B && dR1B) || (type2B && dR2B))
	    theSmallTree.m_hasgenmatch2 = true;
	}

      theSmallTree.m_pairType    = pType ;
      // Need to change the channel: LLR-> 0:muTau - 1:eTau  /  PI-> 0:eTau - 1:muTau
      if      (theSmallTree.m_pairType == 0) theSmallTree.m_BDT_channel = 1.;
      else if (theSmallTree.m_pairType == 1) theSmallTree.m_BDT_channel = 0.;
      else if (theSmallTree.m_pairType == 2) theSmallTree.m_BDT_channel = 2.;

      //cout << " ------------------> CHECK CHANNEL pairType/BDT_chan: " << theSmallTree.m_pairType << "/" << theSmallTree.m_BDT_channel << endl;

      if (theBigTree.npu >= 0 && theBigTree.npu <= 99) // good PU weights
        theSmallTree.m_PUReweight  = (isMC ? reweight.weight(PUReweight_MC,PUReweight_target,theBigTree.npu,PUreweightFile) : 1) ;
      else if (theBigTree.npu >= 100)                  // use the last available bin for the PU weight
        theSmallTree.m_PUReweight  = (isMC ? reweight.weight(PUReweight_MC,PUReweight_target,99,PUreweightFile) : 1) ;
      else                                             // if npu<0 --> bug in MC --> weight=0
        theSmallTree.m_PUReweight  = (isMC ? 0 : 1) ;
      theSmallTree.m_MC_weight   = (isMC ? theBigTree.aMCatNLOweight * XS * stitchWeight * HHweight : 1) ;
      theSmallTree.m_lheht       = (isMC ? theBigTree.lheHt : 0) ;
      theSmallTree.m_EventNumber = theBigTree.EventNumber ;
      theSmallTree.m_RunNumber   = theBigTree.RunNumber ;
      theSmallTree.m_npv  = theBigTree.npv ;
      theSmallTree.m_npu  = theBigTree.npu ;
      theSmallTree.m_lumi = theBigTree.lumi ;
      theSmallTree.m_triggerbit = theBigTree.triggerbit ;
      theSmallTree.m_rho  = theBigTree.rho ;
      theSmallTree.m_isMC = isMC ;
      theSmallTree.m_isOS = theBigTree.isOSCand->at (chosenTauPair) ;
      theSmallTree.m_lheNOutPartons = theBigTree.lheNOutPartons ;
      theSmallTree.m_lheNOutB = theBigTree.lheNOutB ;
      // theSmallTree.m_met_phi = theBigTree.metphi ;
      // theSmallTree.m_met_et = theBigTree.met ;
      theSmallTree.m_met_er_phi = theBigTree.met_er_phi ;
      theSmallTree.m_met_er_et = theBigTree.met_er ;
      TVector2 vMET (theBigTree.METx->at(chosenTauPair) , theBigTree.METy->at(chosenTauPair));
      theSmallTree.m_met_phi   = vMET.Phi();
      theSmallTree.m_met_et    = vMET.Mod();
      theSmallTree.m_met_cov00 = theBigTree.MET_cov00->at (chosenTauPair);
      theSmallTree.m_met_cov01 = theBigTree.MET_cov01->at (chosenTauPair);
      theSmallTree.m_met_cov10 = theBigTree.MET_cov10->at (chosenTauPair);
      theSmallTree.m_met_cov11 = theBigTree.MET_cov11->at (chosenTauPair);

      // L1ECALPrefiringWeight - https://twiki.cern.ch/twiki/bin/viewauth/CMS/L1ECALPrefiringWeightRecipe
      theSmallTree.m_L1pref_weight = theBigTree.prefiringweight;

      // Shifted MET for JES
      // This will be useful when splitting JECs
      //TVector2 vMET_jetup   = getShiftedMET(+1., vMET, theBigTree, DEBUG);
      //TVector2 vMET_jetdown = getShiftedMET(-1., vMET, theBigTree, DEBUG);
      // Now we use the total shift already stored in LLR ntuples
      TVector2 vMET_jetup   (theBigTree.METx_UP_JES->at(chosenTauPair) , theBigTree.METy_UP_JES->at(chosenTauPair));
      TVector2 vMET_jetdown (theBigTree.METx_DOWN_JES->at(chosenTauPair) , theBigTree.METy_DOWN_JES->at(chosenTauPair));
      theSmallTree.m_met_phi_jetup   = vMET_jetup.Phi();
      theSmallTree.m_met_et_jetup    = vMET_jetup.Mod();
      theSmallTree.m_met_phi_jetdown = vMET_jetdown.Phi();
      theSmallTree.m_met_et_jetdown  = vMET_jetdown.Mod();

      // Shifted MET for TES
      TVector2 vMET_tauup   (theBigTree.METx_UP_TES->at(chosenTauPair)  , theBigTree.METy_UP_TES->at(chosenTauPair));
      TVector2 vMET_taudown (theBigTree.METx_DOWN_TES->at(chosenTauPair), theBigTree.METy_DOWN_TES->at(chosenTauPair));
      theSmallTree.m_met_phi_tauup   = vMET_tauup.Phi();
      theSmallTree.m_met_et_tauup    = vMET_tauup.Mod();
      theSmallTree.m_met_phi_taudown = vMET_taudown.Phi();
      theSmallTree.m_met_et_taudown  = vMET_taudown.Mod();

      // Shifted MET for EES
      TVector2 vMET_eleup   (theBigTree.METx_UP_EES->at(chosenTauPair)  , theBigTree.METy_UP_EES->at(chosenTauPair));
      TVector2 vMET_eledown (theBigTree.METx_DOWN_EES->at(chosenTauPair), theBigTree.METy_DOWN_EES->at(chosenTauPair));
      theSmallTree.m_met_phi_eleup   = vMET_eleup.Phi();
      theSmallTree.m_met_et_eleup    = vMET_eleup.Mod();
      theSmallTree.m_met_phi_eledown = vMET_eledown.Phi();
      theSmallTree.m_met_et_eledown  = vMET_eledown.Mod();

      if (DEBUG)
      {
        cout << "------- MET DEBUG -------" << endl;
        cout << " met centr : " << theSmallTree.m_met_et << " / " << theSmallTree.m_met_phi << endl;
        cout << " met jet up: " << theSmallTree.m_met_et_jetup << " / " << theSmallTree.m_met_phi_jetup << endl;
        cout << " met jet dw: " << theSmallTree.m_met_et_jetdown << " / " << theSmallTree.m_met_phi_jetdown << endl;
        cout << " met tau up: " << theSmallTree.m_met_et_tauup << " / " << theSmallTree.m_met_phi_tauup << endl;
        cout << " met tau dw: " << theSmallTree.m_met_et_taudown << " / " << theSmallTree.m_met_phi_taudown << endl;
        cout << " met ele up: " << theSmallTree.m_met_et_eleup << " / " << theSmallTree.m_met_phi_eleup << endl;
        cout << " met ele dw: " << theSmallTree.m_met_et_eledown << " / " << theSmallTree.m_met_phi_eledown << endl;
        cout << "-------------------------" << endl;
      }

      // // in TauTau channel make sure the first tau is the most isolated one
      if (pairType == 2 && (theBigTree.daughters_byDeepTau2017v2p1VSjetraw->at(theBigTree.indexDau1->at (chosenTauPair)) < theBigTree.daughters_byDeepTau2017v2p1VSjetraw->at(theBigTree.indexDau2->at (chosenTauPair))) )
      {
        theSmallTree.m_mT1       = theBigTree.mT_Dau2->at (chosenTauPair) ;
        theSmallTree.m_mT2       = theBigTree.mT_Dau1->at (chosenTauPair) ;
      }
      else
      {
        theSmallTree.m_mT1       = theBigTree.mT_Dau1->at (chosenTauPair) ;
        theSmallTree.m_mT2       = theBigTree.mT_Dau2->at (chosenTauPair) ;
      }

      theSmallTree.m_tauH_pt   = tlv_tauH.Pt () ;
      theSmallTree.m_tauH_eta  = tlv_tauH.Eta () ;
      theSmallTree.m_tauH_phi  = tlv_tauH.Phi () ;
      theSmallTree.m_tauH_e    = tlv_tauH.E () ;
      theSmallTree.m_tauH_mass = tlv_tauH.M () ;

      theSmallTree.m_tauHMet_deltaPhi = deltaPhi (theBigTree.metphi, tlv_tauH.Phi ()) ;
      theSmallTree.m_ditau_deltaPhi = deltaPhi (tlv_firstLepton.Phi (), tlv_secondLepton.Phi ()) ;
      theSmallTree.m_ditau_deltaEta = fabs(tlv_firstLepton.Eta ()- tlv_secondLepton.Eta ()) ;
      theSmallTree.m_ditau_deltaR = tlv_firstLepton.DeltaR(tlv_secondLepton) ;
      theSmallTree.m_dau1MET_deltaphi = deltaPhi (theBigTree.metphi , tlv_firstLepton.Phi ()) ;
      theSmallTree.m_dau2MET_deltaphi = deltaPhi (theBigTree.metphi , tlv_secondLepton.Phi ()) ;

      // Create the MET TLorentzVector for BDT variables, since it's MET --> (px,py,0,0)
      TLorentzVector tlv_MET;
      tlv_MET.SetPxPyPzE( theBigTree.METx->at(chosenTauPair), theBigTree.METy->at(chosenTauPair), 0, std::hypot(theBigTree.METx->at(chosenTauPair), theBigTree.METy->at(chosenTauPair)) );

      theSmallTree.m_tauH_MET_pt                  = (tlv_tauH + tlv_MET).Pt();
      theSmallTree.m_dau2_MET_deltaEta            = fabs(tlv_secondLepton.Eta()); // since MET.Eta()==0 by definition, dEta(tau2,MET)=|tau2.Eta()|
      theSmallTree.m_ditau_deltaR_per_tauH_MET_pt = theSmallTree.m_ditau_deltaR * theSmallTree.m_tauH_MET_pt;

      theSmallTree.m_p_zeta         = Calculate_Pzeta(tlv_firstLepton, tlv_secondLepton, tlv_MET);
      theSmallTree.m_p_zeta_visible = Calculate_visiblePzeta(tlv_firstLepton, tlv_secondLepton);

      theSmallTree.m_mT_tauH_MET       = Calculate_MT(tlv_tauH + tlv_MET, tlv_MET);
      theSmallTree.m_mT_total          = Calculate_TotalMT(tlv_firstLepton, tlv_secondLepton, tlv_MET);
      if (theBigTree.SVfitMass->at (chosenTauPair) > -900.) // in case SVfit is calculated
      {
        theSmallTree.m_mT_tauH_SVFIT_MET             = Calculate_MT(tlv_tauH_SVFIT, tlv_MET);
        theSmallTree.m_BDT_tauHsvfitMet_abs_deltaPhi = fabs(ROOT::Math::VectorUtil::DeltaPhi(tlv_tauH_SVFIT, tlv_MET));
        theSmallTree.m_BDT_tauHsvfitMet_deltaPhi     = ROOT::Math::VectorUtil::DeltaPhi(tlv_tauH_SVFIT, tlv_MET);
      }

      theSmallTree.m_BDT_ditau_deltaPhi = ROOT::Math::VectorUtil::DeltaPhi(tlv_firstLepton, tlv_secondLepton);
      theSmallTree.m_BDT_dau1MET_deltaPhi = ROOT::Math::VectorUtil::DeltaPhi(tlv_firstLepton, tlv_MET);

      theSmallTree.m_dau1_iso = getIso (firstDaughterIndex, tlv_firstLepton.Pt (), theBigTree) ;
      theSmallTree.m_dau1_MVAiso = makeIsoDiscr (firstDaughterIndex, tauMVAIDIdx, theBigTree) ;
      theSmallTree.m_dau1_MVAisoNew = makeIsoDiscr (firstDaughterIndex, tauMVAIDIdxNew, theBigTree) ; //FRA syncFeb2018
      theSmallTree.m_dau1_MVAisoNewdR0p3 = makeIsoDiscr (firstDaughterIndex, tauMVAIDIdxNewdR0p3, theBigTree) ; // FRA syncApr2018
      theSmallTree.m_dau1_CUTiso = makeIsoDiscr (firstDaughterIndex, tauCUTIDIdx, theBigTree) ;
      theSmallTree.m_dau1_antiele = makeIsoDiscr (firstDaughterIndex, tauAntiEleIdx, theBigTree) ;
      theSmallTree.m_dau1_antimu  = makeIsoDiscr (firstDaughterIndex, tauAntiMuIdx, theBigTree) ;
      theSmallTree.m_dau1_deepTauVsJet = makeIsoDiscr (firstDaughterIndex, deepTauVsJetIdx , theBigTree) ;
      theSmallTree.m_dau1_deepTauVsEle = makeIsoDiscr (firstDaughterIndex, deepTauVsEleIdx , theBigTree) ;
      theSmallTree.m_dau1_deepTauVsMu = makeIsoDiscr (firstDaughterIndex, deepTauVsMuIdx , theBigTree) ;

      theSmallTree.m_dau1_photonPtSumOutsideSignalCone = theBigTree.photonPtSumOutsideSignalCone->at (firstDaughterIndex) ;

      int ibit = tauIDsMap["byLooseCombinedIsolationDeltaBetaCorr3Hits"] ;
      theSmallTree.m_dau1_byLooseCombinedIsolationDeltaBetaCorr3Hits = ( theBigTree.tauID->at (firstDaughterIndex)  & (1 << ibit) ) ? true : false ;
      theSmallTree.m_dau2_byLooseCombinedIsolationDeltaBetaCorr3Hits = ( theBigTree.tauID->at (secondDaughterIndex) & (1 << ibit) ) ? true : false ;

      ibit = tauIDsMap["byMediumCombinedIsolationDeltaBetaCorr3Hits"] ;
      theSmallTree.m_dau1_byMediumCombinedIsolationDeltaBetaCorr3Hits = ( theBigTree.tauID->at (firstDaughterIndex)  & (1 << ibit) ) ? true : false ;
      theSmallTree.m_dau2_byMediumCombinedIsolationDeltaBetaCorr3Hits = ( theBigTree.tauID->at (secondDaughterIndex) & (1 << ibit) ) ? true : false ;

      ibit = tauIDsMap["byTightCombinedIsolationDeltaBetaCorr3Hits"] ;
      theSmallTree.m_dau1_byTightCombinedIsolationDeltaBetaCorr3Hits = ( theBigTree.tauID->at (firstDaughterIndex)  & (1 << ibit) ) ? true : false ;
      theSmallTree.m_dau2_byTightCombinedIsolationDeltaBetaCorr3Hits = ( theBigTree.tauID->at (secondDaughterIndex) & (1 << ibit) ) ? true : false ;

      theSmallTree.m_dau1_pt = tlv_firstLepton.Pt () ;
      theSmallTree.m_dau1_pt_tauup = tlv_firstLepton_tauup.Pt () ;
      theSmallTree.m_dau1_pt_taudown = tlv_firstLepton_taudown.Pt () ;
      theSmallTree.m_dau1_pt_eleup = tlv_firstLepton_eleup.Pt () ;
      theSmallTree.m_dau1_pt_eledown = tlv_firstLepton_eledown.Pt () ;
      theSmallTree.m_dau1_eta = tlv_firstLepton.Eta () ;
      theSmallTree.m_dau1_phi = tlv_firstLepton.Phi () ;
      theSmallTree.m_dau1_e = theBigTree.daughters_e->at (firstDaughterIndex) ;
      theSmallTree.m_dau1_dxy = theBigTree.dxy->at(firstDaughterIndex) ;
      theSmallTree.m_dau1_dz  = theBigTree.dz->at(firstDaughterIndex) ;
      theSmallTree.m_dau1_flav = theBigTree.daughters_charge->at (firstDaughterIndex) * (theBigTree.particleType->at (firstDaughterIndex) + 1) ;
      // 1 = from muons collection
      // 2 = from electrons collection
      // 3 = from tauH collection
      theSmallTree.m_dau2_iso = getIso (secondDaughterIndex, tlv_secondLepton.Pt (), theBigTree) ;
      theSmallTree.m_dau2_MVAiso = makeIsoDiscr (secondDaughterIndex, tauMVAIDIdx, theBigTree) ;
      theSmallTree.m_dau2_MVAisoNew = makeIsoDiscr (secondDaughterIndex, tauMVAIDIdxNew, theBigTree) ; //FRA syncFeb2018
      theSmallTree.m_dau2_MVAisoNewdR0p3 = makeIsoDiscr (secondDaughterIndex, tauMVAIDIdxNewdR0p3, theBigTree) ; // FRA syncApr2018
      theSmallTree.m_dau2_CUTiso = makeIsoDiscr (secondDaughterIndex, tauCUTIDIdx, theBigTree) ;
      theSmallTree.m_dau2_antiele = makeIsoDiscr (secondDaughterIndex, tauAntiEleIdx, theBigTree) ;
      theSmallTree.m_dau2_antimu  = makeIsoDiscr (secondDaughterIndex, tauAntiMuIdx, theBigTree) ;
      theSmallTree.m_dau2_deepTauVsJet = makeIsoDiscr (secondDaughterIndex, deepTauVsJetIdx , theBigTree) ;
      theSmallTree.m_dau2_deepTauVsEle = makeIsoDiscr (secondDaughterIndex, deepTauVsEleIdx , theBigTree) ;
      theSmallTree.m_dau2_deepTauVsMu = makeIsoDiscr (secondDaughterIndex, deepTauVsMuIdx , theBigTree) ;
      theSmallTree.m_dau2_photonPtSumOutsideSignalCone = theBigTree.photonPtSumOutsideSignalCone->at (secondDaughterIndex) ;      
      theSmallTree.m_dau2_pt = tlv_secondLepton.Pt () ;
      theSmallTree.m_dau2_pt_tauup = tlv_secondLepton_tauup.Pt () ;
      theSmallTree.m_dau2_pt_taudown = tlv_secondLepton_taudown.Pt () ;
      theSmallTree.m_dau2_pt_eleup = tlv_secondLepton_eleup.Pt () ;
      theSmallTree.m_dau2_pt_eledown = tlv_secondLepton_eledown.Pt () ;
      theSmallTree.m_dau2_eta = tlv_secondLepton.Eta () ;
      theSmallTree.m_dau2_phi = tlv_secondLepton.Phi () ;
      theSmallTree.m_dau2_e = theBigTree.daughters_e->at (secondDaughterIndex) ;
      theSmallTree.m_dau2_dxy = theBigTree.dxy->at(secondDaughterIndex) ;
      theSmallTree.m_dau2_dz  = theBigTree.dz->at(secondDaughterIndex) ;
      theSmallTree.m_dau2_flav = theBigTree.daughters_charge->at (secondDaughterIndex) * (theBigTree.particleType->at (secondDaughterIndex) + 1) ;



      // DATA/MC ID and ISO ScaleFactors
      // Mu and Ele ID and ISO: https://github.com/CMS-HTT/LeptonEfficiencies
      // Tau ID and ISO (MVA and DeepTau): https://twiki.cern.ch/twiki/bin/view/CMS/TauIDRecommendationForRun2
      // Still preliminary:
      // DeepTauVSele: https://indico.cern.ch/event/865792/contributions/3659828/attachments/1954858/3246751/ETauFR-update2Dec.pdf
      // DeepTauVSmu : https://indico.cern.ch/event/866243/contributions/3650016/attachments/1950974/3238736/mutauFRRun2_Yiwen_20191121.pdf
      bool isFakeJet1 = true;
      bool isFakeJet2 = true;

      float idAndIsoSF_MVA  = 1.0;       // use this for MVA2017v2
      float idAndIsoSF_deep = 1.0;       // use this for DeepTauv2p1
      float idAndIsoAndFakeSF_MVA  = 1.0; // use this for MVA2017v2 + e/mu->tauh fake SF
      float idAndIsoAndFakeSF_deep = 1.0; // use this for DeepTauv2p1 + e/mu->tauh fake SF

      // MuTau Channel // anti-ele VLoose / anti-mu Tight / anti-jet Medium
      if (pType == 0 && isMC)
      {
        float mu1pt  = tlv_firstLepton.Pt();
        float mu1eta = TMath::Abs(tlv_firstLepton.Eta());

        float tau2pt  = tlv_secondLepton.Pt();
        float tau2eta = TMath::Abs(tlv_secondLepton.Eta());
        float tau2DM  = theSmallTree.m_dau2_decayMode;
        float tau2Genmatch = theBigTree.genmatch->at(secondDaughterIndex);

        float idAndIsoSF_leg1      = 1.;
        float idAndIsoSF_leg2_MVA_vsJet = 1.;
        float idAndIsoSF_leg2_MVA_vsEle = 1.;
        float idAndIsoSF_leg2_MVA_vsMu  = 1.;
        float idAndIsoSF_leg2_deep_vsJet = 1.;
        float idAndIsoSF_leg2_deep_vsEle = 1.;
        float idAndIsoSF_leg2_deep_vsMu  = 1.;

        if (mu1eta < 2.4)
        {
          idAndIsoSF_leg1 = myIDandISOScaleFactor[0]->get_ScaleFactor(mu1pt, mu1eta);
        }

        idAndIsoSF_leg2_MVA_vsJet = tauSFTool_MVA        ->getSFvsDM (tau2pt , tau2DM, tau2Genmatch);
        idAndIsoSF_leg2_MVA_vsEle = antiEleSFTool_vloose ->getSFvsEta(tau2eta, tau2Genmatch);
        idAndIsoSF_leg2_MVA_vsMu  = antiMuSFTool_tight   ->getSFvsEta(tau2eta, tau2Genmatch);

        idAndIsoSF_leg2_deep_vsJet = tauSFTool_deep->getSFvsDM(tau2pt, tau2DM, tau2Genmatch);

        //idAndIsoSF_leg2_deep_vsEle --> FIXME: not included in TauIDSFTool for now, preliminary version hardcoded here:
        if (tau2Genmatch == 1 || tau2Genmatch == 3 ) // e->tauh fake
        {
          if (tau2eta < 1.448 ) idAndIsoSF_leg2_deep_vsEle = 1.11; // anti-ele VVloose
          if (tau2eta > 1.558 ) idAndIsoSF_leg2_deep_vsEle = 1.03; // anti-ele VVloose
        }

        //idAndIsoSF_leg2_deep_vsMu  --> FIXME: not included in TauIDSFTool for now, preliminary version hardcoded here:
        if (tau2Genmatch == 2 || tau2Genmatch == 4 ) // mu->tauh fake
        {
          if      (tau2eta < 0.4) idAndIsoSF_leg2_deep_vsMu = 0.917; // anti-mu Tight
          else if (tau2eta < 0.8) idAndIsoSF_leg2_deep_vsMu = 0.791; // anti-mu Tight
          else if (tau2eta < 1.2) idAndIsoSF_leg2_deep_vsMu = 0.668; // anti-mu Tight
          else if (tau2eta < 1.7) idAndIsoSF_leg2_deep_vsMu = 1.068; // anti-mu Tight
          else if (tau2eta < 2.3) idAndIsoSF_leg2_deep_vsMu = 4.074; // anti-mu Tight
        }

        if (tau2Genmatch==1 || tau2Genmatch==2 || tau2Genmatch==3 || tau2Genmatch==4 || tau2Genmatch==5)
        {
          isFakeJet2 = false;
        }

        idAndIsoSF_MVA  = idAndIsoSF_leg1 * idAndIsoSF_leg2_MVA_vsJet;
        idAndIsoSF_deep = idAndIsoSF_leg1 * idAndIsoSF_leg2_deep_vsJet;
        idAndIsoAndFakeSF_MVA  = idAndIsoSF_leg1 * idAndIsoSF_leg2_MVA_vsJet  * idAndIsoSF_leg2_MVA_vsEle  * idAndIsoSF_leg2_MVA_vsMu;
        idAndIsoAndFakeSF_deep = idAndIsoSF_leg1 * idAndIsoSF_leg2_deep_vsJet * idAndIsoSF_leg2_deep_vsEle * idAndIsoSF_leg2_deep_vsMu;

        if (DEBUG)
        {
          cout << "--- DEBUG idAndIsoSF ---" << endl;
          cout << "pairType  : " << pType << endl;
          cout << "totSF MVA : " << idAndIsoSF_MVA << endl;
          cout << "totSF deep: " << idAndIsoSF_deep << endl;
        }
      }

      // EleTau Channel // anti-ele Tight / anti-mu Loose / anti-jet Medium
      else if (pType == 1 && isMC)
      {
        float ele1pt  = tlv_firstLepton.Pt();
        float ele1eta = TMath::Abs(tlv_firstLepton.Eta());

        float tau2pt  = tlv_secondLepton.Pt();
        float tau2eta = TMath::Abs(tlv_secondLepton.Eta());
        float tau2DM  = theSmallTree.m_dau2_decayMode;
        float tau2Genmatch = theBigTree.genmatch->at(secondDaughterIndex);

        float idAndIsoSF_leg1      = 1.;
        float idAndIsoSF_leg2_MVA_vsJet = 1.;
        float idAndIsoSF_leg2_MVA_vsEle = 1.;
        float idAndIsoSF_leg2_MVA_vsMu  = 1.;
        float idAndIsoSF_leg2_deep_vsJet = 1.;
        float idAndIsoSF_leg2_deep_vsEle = 1.;
        float idAndIsoSF_leg2_deep_vsMu  = 1.;

        if (ele1eta < 2.4)
        {
          idAndIsoSF_leg1 = myIDandISOScaleFactor[1]->get_ScaleFactor(ele1pt, ele1eta);
        }

        idAndIsoSF_leg2_MVA_vsJet = tauSFTool_MVA       ->getSFvsDM (tau2pt , tau2DM, tau2Genmatch);
        idAndIsoSF_leg2_MVA_vsEle = antiEleSFTool_tight ->getSFvsEta(tau2eta, tau2Genmatch);
        idAndIsoSF_leg2_MVA_vsMu  = antiMuSFTool_loose  ->getSFvsEta(tau2eta, tau2Genmatch);

        idAndIsoSF_leg2_deep_vsJet = tauSFTool_deep->getSFvsDM(tau2pt, tau2DM, tau2Genmatch);

        //idAndIsoSF_leg2_deep_vsEle --> FIXME: not included in TauIDSFTool for now, preliminary version hardcoded here:
        if (tau2Genmatch == 1 || tau2Genmatch == 3 ) // e->tauh fake
        {
          if (tau2eta < 1.448 ) idAndIsoSF_leg2_deep_vsEle = 1.22; // anti-ele Tight
          if (tau2eta > 1.558 ) idAndIsoSF_leg2_deep_vsEle = 0.93; // anti-ele Tight
        }

        //idAndIsoSF_leg2_deep_vsMu  --> FIXME: not included in TauIDSFTool for now, preliminary version hardcoded here:
        if (tau2Genmatch == 2 || tau2Genmatch == 4 ) // mu->tauh fake
        {
          if      (tau2eta < 0.4) idAndIsoSF_leg2_deep_vsMu = 1.024; // anti-mu Loose
          else if (tau2eta < 0.8) idAndIsoSF_leg2_deep_vsMu = 0.695; // anti-mu Loose
          else if (tau2eta < 1.2) idAndIsoSF_leg2_deep_vsMu = 0.987; // anti-mu Loose
          else if (tau2eta < 1.7) idAndIsoSF_leg2_deep_vsMu = 0.906; // anti-mu Loose
          else if (tau2eta < 2.3) idAndIsoSF_leg2_deep_vsMu = 5.447; // anti-mu Loose
        }

        if (tau2Genmatch==1 || tau2Genmatch==2 || tau2Genmatch==3 || tau2Genmatch==4 || tau2Genmatch==5)
        {
          isFakeJet2 = false;
        }

        idAndIsoSF_MVA  = idAndIsoSF_leg1 * idAndIsoSF_leg2_MVA_vsJet;
        idAndIsoSF_deep = idAndIsoSF_leg1 * idAndIsoSF_leg2_deep_vsJet;
        idAndIsoAndFakeSF_MVA  = idAndIsoSF_leg1 * idAndIsoSF_leg2_MVA_vsJet  * idAndIsoSF_leg2_MVA_vsEle  * idAndIsoSF_leg2_MVA_vsMu;
        idAndIsoAndFakeSF_deep = idAndIsoSF_leg1 * idAndIsoSF_leg2_deep_vsJet * idAndIsoSF_leg2_deep_vsEle * idAndIsoSF_leg2_deep_vsMu;

        if (DEBUG)
        {
          cout << "--- DEBUG idAndIsoSF ---" << endl;
          cout << "pairType  : " << pType << endl;
          cout << "totSF MVA : " << idAndIsoSF_MVA << endl;
          cout << "totSF deep: " << idAndIsoSF_deep << endl;
        }
      }

      // TauTau Channel // anti-ele VLoose / anti-mu Loose / anti-jet Medium
      else if (pType == 2 && isMC)
      {

        float tau1pt  = tlv_firstLepton.Pt();
        float tau1eta = TMath::Abs(tlv_firstLepton.Eta());
        float tau1DM  = theSmallTree.m_dau1_decayMode;
        float tau1Genmatch = theBigTree.genmatch->at(firstDaughterIndex);

        float tau2pt  = tlv_secondLepton.Pt();
        float tau2eta = TMath::Abs(tlv_secondLepton.Eta());
        float tau2DM  = theSmallTree.m_dau2_decayMode;
        float tau2Genmatch = theBigTree.genmatch->at(secondDaughterIndex);

        float idAndIsoSF_leg1_MVA_vsJet = 1.;
        float idAndIsoSF_leg1_MVA_vsEle = 1.;
        float idAndIsoSF_leg1_MVA_vsMu  = 1.;
        float idAndIsoSF_leg1_deep_vsJet = 1.;
        float idAndIsoSF_leg1_deep_vsEle = 1.;
        float idAndIsoSF_leg1_deep_vsMu  = 1.;

        float idAndIsoSF_leg2_MVA_vsJet = 1.;
        float idAndIsoSF_leg2_MVA_vsEle = 1.;
        float idAndIsoSF_leg2_MVA_vsMu  = 1.;
        float idAndIsoSF_leg2_deep_vsJet = 1.;
        float idAndIsoSF_leg2_deep_vsEle = 1.;
        float idAndIsoSF_leg2_deep_vsMu  = 1.;

        idAndIsoSF_leg1_MVA_vsJet = tauSFTool_MVA        ->getSFvsDM (tau1pt , tau1DM, tau1Genmatch);
        idAndIsoSF_leg1_MVA_vsEle = antiEleSFTool_vloose ->getSFvsEta(tau1eta, tau1Genmatch);
        idAndIsoSF_leg1_MVA_vsMu  = antiMuSFTool_loose   ->getSFvsEta(tau1eta, tau1Genmatch);

        idAndIsoSF_leg1_deep_vsJet = tauSFTool_deep->getSFvsDM(tau1pt, tau1DM, tau1Genmatch);

        //idAndIsoSF_leg1_deep_vsEle --> FIXME: not included in TauIDSFTool for now, preliminary version hardcoded here:
        if (tau1Genmatch == 1 || tau1Genmatch == 3 ) // e->tauh fake
        {
          if (tau1eta < 1.448 ) idAndIsoSF_leg1_deep_vsEle = 1.11; // anti-ele VVLoose
          if (tau1eta > 1.558 ) idAndIsoSF_leg1_deep_vsEle = 1.03; // anti-ele VVLoose
        }

        //idAndIsoSF_leg1_deep_vsMu  --> FIXME: not included in TauIDSFTool for now, preliminary version hardcoded here:
        if (tau1Genmatch == 2 || tau1Genmatch == 4 ) // mu->tauh fake
        {
          if      (tau2eta < 0.4) idAndIsoSF_leg2_deep_vsMu = 1.024; // anti-mu Loose
          else if (tau2eta < 0.8) idAndIsoSF_leg2_deep_vsMu = 0.695; // anti-mu Loose
          else if (tau2eta < 1.2) idAndIsoSF_leg2_deep_vsMu = 0.987; // anti-mu Loose
          else if (tau2eta < 1.7) idAndIsoSF_leg2_deep_vsMu = 0.906; // anti-mu Loose
          else if (tau2eta < 2.3) idAndIsoSF_leg2_deep_vsMu = 5.447; // anti-mu Loose
        }

        idAndIsoSF_leg2_MVA_vsJet = tauSFTool_MVA        ->getSFvsDM (tau2pt , tau2DM, tau2Genmatch);
        idAndIsoSF_leg2_MVA_vsEle = antiEleSFTool_vloose ->getSFvsEta(tau2eta, tau2Genmatch);
        idAndIsoSF_leg2_MVA_vsMu  = antiMuSFTool_loose   ->getSFvsEta(tau2eta, tau2Genmatch);

        idAndIsoSF_leg2_deep_vsJet = tauSFTool_deep->getSFvsDM(tau2pt, tau2DM, tau2Genmatch);

        //idAndIsoSF_leg2_deep_vsEle --> FIXME: not included in TauIDSFTool for now, preliminary version hardcoded here:
        if (tau2Genmatch == 1 || tau2Genmatch == 3 ) // e->tauh fake
        {
          if (tau2eta < 1.448 ) idAndIsoSF_leg2_deep_vsEle = 1.11; // anti-ele VVLoose
          if (tau2eta > 1.558 ) idAndIsoSF_leg2_deep_vsEle = 1.03; // anti-ele VVLoose
        }

        //idAndIsoSF_leg2_deep_vsMu  --> FIXME: not included in TauIDSFTool for now, preliminary version hardcoded here:
        if (tau2Genmatch == 2 || tau2Genmatch == 4 ) // mu->tauh fake
        {
          if      (tau2eta < 0.4) idAndIsoSF_leg2_deep_vsMu = 1.024; // anti-mu Loose
          else if (tau2eta < 0.8) idAndIsoSF_leg2_deep_vsMu = 0.695; // anti-mu Loose
          else if (tau2eta < 1.2) idAndIsoSF_leg2_deep_vsMu = 0.987; // anti-mu Loose
          else if (tau2eta < 1.7) idAndIsoSF_leg2_deep_vsMu = 0.906; // anti-mu Loose
          else if (tau2eta < 2.3) idAndIsoSF_leg2_deep_vsMu = 5.447; // anti-mu Loose
        }

        if (tau1Genmatch==1 || tau1Genmatch==2 || tau1Genmatch==3 || tau1Genmatch==4 || tau1Genmatch==5)
        {
          isFakeJet1 = false;
        }

        if (tau2Genmatch==1 || tau2Genmatch==2 || tau2Genmatch==3 || tau2Genmatch==4 || tau2Genmatch==5)
        {
          isFakeJet2 = false;
        }

        idAndIsoSF_MVA  = idAndIsoSF_leg1_MVA_vsJet * idAndIsoSF_leg2_MVA_vsJet;
        idAndIsoSF_deep = idAndIsoSF_leg1_deep_vsJet * idAndIsoSF_leg2_deep_vsJet;
        idAndIsoAndFakeSF_MVA  = idAndIsoSF_leg1_MVA_vsJet * idAndIsoSF_leg1_MVA_vsEle * idAndIsoSF_leg1_MVA_vsMu * idAndIsoSF_leg2_MVA_vsJet  * idAndIsoSF_leg2_MVA_vsEle  * idAndIsoSF_leg2_MVA_vsMu;
        idAndIsoAndFakeSF_deep = idAndIsoSF_leg1_deep_vsJet * idAndIsoSF_leg1_deep_vsEle * idAndIsoSF_leg1_deep_vsMu * idAndIsoSF_leg2_deep_vsJet * idAndIsoSF_leg2_deep_vsEle * idAndIsoSF_leg2_deep_vsMu;

        if (DEBUG)
        {
          cout << "--- DEBUG idAndIsoSF ---" << endl;
          cout << "pairType  : " << pType << endl;
          cout << "totSF MVA : " << idAndIsoSF_MVA << endl;
          cout << "totSF deep: " << idAndIsoSF_deep << endl;
        }
      }

      // MuMu Channel
      else if (pType == 3 && isMC)
      {
        float mu1pt  = tlv_firstLepton.Pt();
        float mu1eta = TMath::Abs(tlv_firstLepton.Eta());
        float mu2pt  = tlv_secondLepton.Pt();
        float mu2eta = TMath::Abs(tlv_secondLepton.Eta());

        float idAndIsoSF_leg1 = 1.;
        float idAndIsoSF_leg2 = 1.;

        if (mu1eta < 2.4)
        {
          idAndIsoSF_leg1 = myIDandISOScaleFactor[0]->get_ScaleFactor(mu1pt, mu1eta);
        }
        if (mu2eta < 2.4)
        {
          idAndIsoSF_leg2 = myIDandISOScaleFactor[0]->get_ScaleFactor(mu2pt, mu2eta);
        }

        idAndIsoSF_MVA = idAndIsoSF_deep = idAndIsoAndFakeSF_MVA = idAndIsoAndFakeSF_deep = idAndIsoSF_leg1 * idAndIsoSF_leg2;

        if (DEBUG)
        {
          cout << "--- DEBUG idAndIsoSF ---" << endl;
          cout << "pairType  : " << pType << endl;
          cout << "totSF MVA : " << idAndIsoSF_MVA << endl;
          cout << "totSF deep: " << idAndIsoSF_deep << endl;
        }
      }
      
      // EleEle Channel
      else if (pType == 4 && isMC)
      {
        float ele1pt  = tlv_firstLepton.Pt();
        float ele1eta = TMath::Abs(tlv_firstLepton.Eta());
        float ele2pt  = tlv_secondLepton.Pt();
        float ele2eta = TMath::Abs(tlv_secondLepton.Eta());

        float idAndIsoSF_leg1 = 1.;
        float idAndIsoSF_leg2 = 1.;

        if (ele1eta < 2.4)
        {
          idAndIsoSF_leg1 = myIDandISOScaleFactor[1]->get_ScaleFactor(ele1pt, ele1eta);
        }
        if (ele2eta < 2.4)
        {
          idAndIsoSF_leg2 = myIDandISOScaleFactor[1]->get_ScaleFactor(ele2pt, ele2eta);
        }

        idAndIsoSF_MVA = idAndIsoSF_deep = idAndIsoAndFakeSF_MVA = idAndIsoAndFakeSF_deep = idAndIsoSF_leg1 * idAndIsoSF_leg2;

        if (DEBUG)
        {
          cout << "--- DEBUG idAndIsoSF ---" << endl;
          cout << "pairType  : " << pType << endl;
          cout << "totSF MVA : " << idAndIsoSF_MVA << endl;
          cout << "totSF deep: " << idAndIsoSF_deep << endl;
        }
      }

      // Save the IDandISO SF (event per event)
      theSmallTree.m_IdAndIsoSF_MVA  = (isMC ? idAndIsoSF_MVA  : 1.0);
      theSmallTree.m_IdAndIsoSF_deep = (isMC ? idAndIsoSF_deep : 1.0);
      theSmallTree.m_IdAndIsoAndFakeSF_MVA  = (isMC ? idAndIsoAndFakeSF_MVA  : 1.0);
      theSmallTree.m_IdAndIsoAndFakeSF_deep = (isMC ? idAndIsoAndFakeSF_deep : 1.0);

      //Jet faking Tau SF
      //derived from WJet sideband: http://camendol.web.cern.ch/camendol/HH2017/plotsHH2017MuTau/31Oct2018_DYNLO_ctrlWJets_SS/antiB_jets30_tau30_SStight/
      double jetFakeSF1 = 1.;
      double jetFakeSF2 = 1.;
      if (pType == 0 || pType == 1 || pType == 2) // 2nd tau
	{
	  if (isFakeJet2){
	    if(fabs(tlv_secondLepton.Eta()) < 1.46){
	      jetFakeSF2 = 1.38469;
	    }else{
	      jetFakeSF2 = 1.69035;
	    }     
	  }
	}
      
      if(pType == 2)                //1st tau
	{
	  if (isFakeJet1){
	    if(fabs(tlv_firstLepton.Eta()) < 1.46){
	      jetFakeSF1 = 1.38469;
	    }else{
	      jetFakeSF1 = 1.69035;
	    }     
	  }
	}
      theSmallTree.m_jetFakeSF = (isMC ? jetFakeSF1*jetFakeSF2 : 1.0);
      
      // DATA/MC Trigger ScaleFactors
      // https://github.com/CMS-HTT/LeptonEfficiencies
      // https://github.com/truggles/TauTriggerSFs2017

      // recommendations for cross triggers:  https://twiki.cern.ch/twiki/bin/view/CMS/HiggsToTauTauWorking2017#Trigger_Information

      
      float trigSF = 1.0;
      float trigSF_single = 1.0;
      float trigSF_cross = 1.0;

      if(applyTriggers)
	{
	  // MuTau Channel
	  if (pType == 0 && isMC)
	    {
	      if(fabs(tlv_secondLepton.Eta()) < 2.1){ //eta region covered both by cross-trigger and single lepton trigger
		
		int passCross = 1;
		int passSingle = 1;
		
		if (tlv_firstLepton.Pt() < 26.) passSingle = 0;  
		if (tlv_secondLepton.Pt() < 32.) passCross = 0;  

		//lepton trigger
		double SFL_Data = muTrgSF->get_EfficiencyData(tlv_firstLepton.Pt(), tlv_firstLepton.Eta());
		double SFL_MC = muTrgSF->get_EfficiencyMC(tlv_firstLepton.Pt(), tlv_firstLepton.Eta());

		//cross-trigger
		//mu leg
		double SFl_Data = muTauTrgSF->get_EfficiencyData(tlv_firstLepton.Pt(), tlv_firstLepton.Eta());
		double SFl_MC = muTauTrgSF->get_EfficiencyMC(tlv_firstLepton.Pt(), tlv_firstLepton.Eta());

		//tau leg
		double SFtau_Data = tauTrgSF_mutau->getEfficiencyData(tlv_secondLepton.Pt(), DM2, 0); // last entry is uncertainty: 0 central, +1 up, -1 down
		double SFtau_MC   = tauTrgSF_mutau->getEfficiencyMC  (tlv_secondLepton.Pt(), DM2, 0); // last entry is uncertainty: 0 central, +1 up, -1 down
		
		double Eff_Data =  passSingle * SFL_Data * (1 - passCross * SFtau_Data) + passCross * SFl_Data * SFtau_Data;
		double Eff_MC   =  passSingle * SFL_MC * (1 - passCross * SFtau_MC) + passCross * SFl_MC * SFtau_MC;

		trigSF = Eff_Data / Eff_MC;

		//trig SF for analysis only with cross-trigger
		double SFl = muTauTrgSF->get_ScaleFactor(tlv_firstLepton.Pt(), tlv_firstLepton.Eta());
		double SFtau = tauTrgSF_mutau->getSF(tlv_secondLepton.Pt(), DM2, 0); // last entry is uncertainty: 0 central, +1 up, -1 down
		trigSF_cross = SFl*SFtau;
	      }else{ //eta region covered only by single lepton trigger
		double SF = muTrgSF->get_ScaleFactor(tlv_firstLepton.Pt(), tlv_firstLepton.Eta());
		trigSF = SF;
	      }
	      //trig SF for analysis only with single-mu trigger
	      trigSF_single =  muTrgSF->get_ScaleFactor(tlv_firstLepton.Pt(), tlv_firstLepton.Eta());
	    }
	  
	  // EleTau Channel
	  else if (pType == 1 && isMC)
	    {
	      if(fabs(tlv_secondLepton.Eta()) < 2.1){ //eta region covered both by cross-trigger and single lepton trigger
		int passCross = 1;
		int passSingle = 1;
		
		if (tlv_firstLepton.Pt() < 35.) passSingle = 0;  
		if (tlv_secondLepton.Pt() < 35.) passCross = 0;  

		//lepton trigger
		double SFL_Data = eTrgSF->get_EfficiencyData(tlv_firstLepton.Pt(), tlv_firstLepton.Eta());
		double SFL_MC = eTrgSF->get_EfficiencyMC(tlv_firstLepton.Pt(), tlv_firstLepton.Eta());
		
		//cross-trigger
		//e leg
		double SFl_Data = eTauTrgSF->get_EfficiencyData(tlv_firstLepton.Pt(), tlv_firstLepton.Eta());
		double SFl_MC = eTauTrgSF->get_EfficiencyMC(tlv_firstLepton.Pt(), tlv_firstLepton.Eta());
		
		//tau leg
		double SFtau_Data = tauTrgSF_etau->getEfficiencyData(tlv_secondLepton.Pt(), DM2, 0); // last entry is uncertainty: 0 central, +1 up, -1 down
		double SFtau_MC   = tauTrgSF_etau->getEfficiencyMC  (tlv_secondLepton.Pt(), DM2, 0); // last entry is uncertainty: 0 central, +1 up, -1 down
		
		double Eff_Data =  passSingle * SFL_Data * (1 - passCross * SFtau_Data) + passCross * SFl_Data * SFtau_Data;
		double Eff_MC   =  passSingle * SFL_MC * (1 - passCross * SFtau_MC) + passCross * SFl_MC * SFtau_MC;
		
		trigSF = Eff_Data / Eff_MC;

		//trig SF for analysis only with cross-trigger
		double SFl = eTauTrgSF->get_ScaleFactor(tlv_firstLepton.Pt(), tlv_firstLepton.Eta());
		double SFtau = tauTrgSF_etau->getSF(tlv_secondLepton.Pt(), DM2, 0); // last entry is uncertainty: 0 central, +1 up, -1 down
		trigSF_cross = SFl*SFtau;
	      }else{ //eta region covered only by single lepton trigger
		double SF = eTrgSF->get_ScaleFactor(tlv_firstLepton.Pt(), tlv_firstLepton.Eta());
		trigSF = SF;
	      }
	      //trig SF for analysis only with single-e trigger
	      trigSF_single =  eTrgSF->get_ScaleFactor(tlv_firstLepton.Pt(), tlv_firstLepton.Eta());
	    }
	  // TauTau Channel
	  else if (pType == 2 && isMC)
	    {
	      double SF1 = tauTrgSF_ditau->getSF(tlv_firstLepton.Pt() , DM1, 0); // last entry is uncertainty: 0 central, +1 up, -1 down
	      double SF2 = tauTrgSF_ditau->getSF(tlv_secondLepton.Pt(), DM2, 0); // last entry is uncertainty: 0 central, +1 up, -1 down
	      trigSF = SF1 * SF2;
	    }
	  
	  // MuMu Channel
	  else if (pType == 3 && isMC)
	    {
	      double SF = muTrgSF->get_ScaleFactor(tlv_firstLepton.Pt(), tlv_firstLepton.Eta());
	      trigSF = SF;
	
	    }
	  
	  // EleEle Channel
	  else if (pType == 4 && isMC)
	    {
	      double SF = eTrgSF->get_ScaleFactor(tlv_firstLepton.Pt(), tlv_firstLepton.Eta());
	      trigSF = SF;
	      
	    }
	}
      theSmallTree.m_trigSF     = (isMC ? trigSF : 1.0);
      theSmallTree.m_trigSF_single     = (isMC ? trigSF_single : 1.0);
      theSmallTree.m_trigSF_cross     = (isMC ? trigSF_cross : 1.0);

      theSmallTree.m_totalWeight = (isMC? (41557./7.20811e+10) * theSmallTree.m_MC_weight* theSmallTree.m_PUReweight* theSmallTree.m_DYscale_MM_NLO* trigSF* theSmallTree.m_IdAndIsoAndFakeSF_deep: 1.0);
      //this is just a residual of some synch

      // Third lepton veto
      // loop over leptons
      vector<pair<float, int> > thirdLeptons ; // pt, idx
      for (unsigned int iLep = 0 ; (iLep < theBigTree.daughters_px->size ()) ; ++iLep)
      {
        // skip the H decay candiates
        if (int (iLep) == firstDaughterIndex || int (iLep) == secondDaughterIndex) continue;

        // remove taus
        if (theBigTree.particleType->at (iLep) == 2)
        {
          continue ;
        }
        else if (theBigTree.particleType->at (iLep) == 0) // muons
        {
          if (!oph.muBaseline (&theBigTree, iLep, 10., 2.4, 0.3, OfflineProducerHelper::MuMedium)) continue;
        }
        else if (theBigTree.particleType->at (iLep) == 1) // electrons
        {
          if (!oph.eleBaseline (&theBigTree, iLep, 10., 2.5, 0.3, OfflineProducerHelper::EMVAMedium)) continue;  //FRA: syncFeb2018
        }

        TLorentzVector tlv_dummyLepton
        (
          theBigTree.daughters_px->at (iLep),
          theBigTree.daughters_py->at (iLep),
          theBigTree.daughters_pz->at (iLep),
          theBigTree.daughters_e->at (iLep)
        );
        thirdLeptons.push_back (make_pair(tlv_dummyLepton.Pt(), iLep)) ;

        if(DEBUG)
        {
          cout << "** 3rd lep veto passed"
          << " idx="  << iLep
          << " type=" << theBigTree.particleType->at(iLep)
          << " pt="   << tlv_dummyLepton.Pt()
          << " eta="  << tlv_dummyLepton.Eta()
          << " phi="  << tlv_dummyLepton.Phi()
          << " iso="  << getIso (iLep, tlv_dummyLepton.Pt (), theBigTree)
          << " dxy="  << theBigTree.dxy->at(iLep)
          << " dz="   << theBigTree.dz->at(iLep)
          << endl;
        }
      } // loop over leptons

      sort (thirdLeptons.begin(), thirdLeptons.end()) ;
      // reverse loop to start from last one == highest pT
      for (int iLep = thirdLeptons.size() -1; (iLep >=0) && (theSmallTree.m_nleps < 2) ; iLep--)
      {
        TLorentzVector tlv_dummyLepton
        (
          theBigTree.daughters_px->at (iLep),
          theBigTree.daughters_py->at (iLep),
          theBigTree.daughters_pz->at (iLep),
          theBigTree.daughters_e->at (iLep)
        );

        theSmallTree.m_leps_pt.push_back   (tlv_dummyLepton.Pt ()) ;
        theSmallTree.m_leps_eta.push_back  (tlv_dummyLepton.Eta ()) ;
        theSmallTree.m_leps_phi.push_back  (tlv_dummyLepton.Phi ()) ;
        theSmallTree.m_leps_e.push_back    (tlv_dummyLepton.E ()) ;
        theSmallTree.m_leps_flav.push_back (theBigTree.particleType->at (iLep)) ;
        ++theSmallTree.m_nleps ;
      }

      if(DEBUG)
      {
        cout << "***** DEBUG: nleps="<< theSmallTree.m_nleps<< endl;
      }

      // ----------------------------------------------------------
      // select jets 
      // ----------------------------------------------------------

      vector <pair <float, int> > jets_and_sortPar ;
      // loop over jets
      TLorentzVector jetVecSum (0,0,0,0);
      for (unsigned int iJet = 0 ; iJet < theBigTree.jets_px->size () ; ++iJet)
      {
        // JET PU ID cut
        if (theBigTree.PFjetID->at (iJet) < PFjetID_WP) continue; // 0 ; don't pass PF Jet ID; 1: tight, 2: tightLepVeto

        TLorentzVector tlv_jet
        (
         theBigTree.jets_px->at (iJet),
         theBigTree.jets_py->at (iJet),
         theBigTree.jets_pz->at (iJet),
         theBigTree.jets_e->at (iJet)
        ) ;

        if (DEBUG)
        {
          cout << "------- Jet PU ID  DEBUG---------" << endl;
          cout << "iJet: " << iJet << "  -- pT/Eta/Phi: " << tlv_jet.Pt() << "/" << tlv_jet.Eta() << "/" << tlv_jet.Phi() << endl;
          cout << "discr: " << theBigTree.jets_PUJetID->at(iJet) << " --  discrUpdated: " << theBigTree.jets_PUJetIDupdated->at(iJet) << endl;
          cout << "discrWP: " << theBigTree.jets_PUJetIDupdated_WP->at(iJet) << " -- bitwise: " << std::bitset<5>(theBigTree.jets_PUJetIDupdated_WP->at(iJet)) << endl;
          cout << "Pass Loose : " << CheckBit(theBigTree.jets_PUJetIDupdated_WP->at(iJet), 2) << endl;
          cout << "Pass Medium: " << CheckBit(theBigTree.jets_PUJetIDupdated_WP->at(iJet), 1) << endl;
          cout << "Pass Tight : " << CheckBit(theBigTree.jets_PUJetIDupdated_WP->at(iJet), 0) << endl;
          cout << "---------------------------------" << endl;
        }

        if (tlv_jet.Pt () < 20.) continue ;
        if (tlv_jet.DeltaR (tlv_firstLepton) < lepCleaningCone) continue ;
        if (tlv_jet.DeltaR (tlv_secondLepton) < lepCleaningCone) continue ;

        // Apply PUjetID only to jets with pt < 50 GeV ( https://twiki.cern.ch/twiki/bin/view/CMS/HiggsToTauTauWorkingLegacyRun2#Jets )
        // PU jet ID WP = 2: loose
        if (PUjetID_WP > -1)
        {
          if ( !(CheckBit(theBigTree.jets_PUJetIDupdated_WP->at(iJet), PUjetID_WP)) && tlv_jet.Pt()<50.) continue;
        }

        // all jets selected as btag cands apart from eta cut
        int ajetHadFlav = abs(theBigTree.jets_HadronFlavour->at(iJet));
        if (ajetHadFlav == 5) ++theSmallTree.m_njetsBHadFlav;
        if (ajetHadFlav == 4) ++theSmallTree.m_njetsCHadFlav;

        // 2.4 for b-tag
        if (TMath::Abs(tlv_jet.Eta()) > 2.4) continue;

        // n bjets candidates
        if (tlv_jet.Pt () > 20)  ++theSmallTree.m_nbjets20 ;
        if (tlv_jet.Pt () > 50)  ++theSmallTree.m_nbjets50 ;
       
        //SortParameter = (bChoiceFlag == 1 ) ? bTag : Pt ;
        float sortPar;
        if(useDeepFlavor)
          sortPar = (bChoiceFlag == 1 ) ? theBigTree.bDeepFlavor_probb->at(iJet) + theBigTree.bDeepFlavor_probbb->at(iJet) + theBigTree.bDeepFlavor_problepb->at(iJet) : tlv_jet.Pt() ;
        else
          sortPar = (bChoiceFlag == 1 ) ? theBigTree.bDeepCSV_probb->at(iJet) + theBigTree.bDeepCSV_probbb->at(iJet) : tlv_jet.Pt() ;

        if (bChoiceFlag != 1 && bChoiceFlag != 2) cout << "** WARNING : bChoiceFlag not known :" << bChoiceFlag << endl;
        jets_and_sortPar.push_back (make_pair (sortPar, iJet) );

      } // loop over jets

      theSmallTree.m_nbjetscand = jets_and_sortPar.size();
      theSmallTree.m_nfatjets = theBigTree.ak8jets_px->size();

      if (!beInclusive && jets_and_sortPar.size () < 2) continue ;
      ec.Increment("TwoJets", EvtW);
      if (isHHsignal && pairType == genHHDecMode) ecHHsig[genHHDecMode].Increment ("TwoJets", EvtW);

      // sort jet collection by deepCSV
      sort (jets_and_sortPar.begin(), jets_and_sortPar.end(), bJetSort); //sort by first parameter, then pt (dummy if pt order chosen)
      if (jets_and_sortPar.size () >= 2)
      {
        bool isVBF = false;
        vector<pair <int, float> > jets_and_BTag;
        for (auto pair : jets_and_sortPar) jets_and_BTag.push_back (make_pair(pair.second, pair.first)); // just for compatibility...

        // NB !!! the following function only works if jets_and_sortPar contains <CVSscore, idx>
        vector<float> bTagWeight = bTagSFHelper.getEvtWeight (jets_and_BTag, theBigTree.jets_px, theBigTree.jets_py, theBigTree.jets_pz, theBigTree.jets_e, theBigTree.jets_HadronFlavour, pType) ;
        theSmallTree.m_bTagweightL = (isMC ? bTagWeight.at(0) : 1.0) ;
        theSmallTree.m_bTagweightM = (isMC ? bTagWeight.at(1) : 1.0) ;
        theSmallTree.m_bTagweightT = (isMC ? bTagWeight.at(2) : 1.0) ;

        bool bPairFound = false;
        int njets = jets_and_sortPar.size();

        // medium WP for 2017 is: 0.4941 for DeepCSV, 0.3033 for DeepFlavor
        if(useDeepFlavor)
        {
            if (jets_and_sortPar.at(njets-2).first>0.3033) bPairFound = true;
        }
        else
        {
            if (jets_and_sortPar.at(njets-2).first>0.4941) bPairFound = true;
        }

        const int bjet1idx = jets_and_sortPar.at(njets-1).second ;
        int bjet2idx_temp  = jets_and_sortPar.at(njets-2).second ;

        //VBF tag
        int VBFidx1 = -1;
        int VBFidx2 = -1;
        theSmallTree.m_isVBF = 0;

        // build all the possible VBF-jet pairs
        std::vector< tuple<float, int, int> > VBFcand_Mjj;
        if (theBigTree.jets_px->size ()>1)
        {
          for (unsigned int iJet = 0; (iJet < theBigTree.jets_px->size ()) && (theSmallTree.m_njets < maxNjetsSaved); ++iJet)
          {
            // JET PU ID cut
            if (theBigTree.PFjetID->at (iJet) < PFjetID_WP) continue; // 0 ; don't pass PF Jet ID; 1: tight, 2: tightLepVeto

            // Skip the already selected b-jets
            if (int (iJet) == bjet1idx) continue;
            if(bPairFound && int (iJet) == bjet2idx_temp) continue;

            TLorentzVector ijet;
            ijet.SetPxPyPzE(
                           theBigTree.jets_px->at (iJet),
                           theBigTree.jets_py->at (iJet),
                           theBigTree.jets_pz->at (iJet),
                           theBigTree.jets_e ->at (iJet)
                           );

            if (ijet.DeltaR (tlv_firstLepton) < lepCleaningCone) continue ;
            if (ijet.DeltaR (tlv_secondLepton) < lepCleaningCone) continue ;
            if(ijet.Pt() < 30.) continue;
            if(fabs(ijet.Eta()) > 4.7) continue; // max eta allowed: https://twiki.cern.ch/twiki/bin/view/CMS/HiggsToTauTauWorkingLegacyRun2#Jets

            // Apply PUjetID only to jets with pt < 50 GeV ( https://twiki.cern.ch/twiki/bin/view/CMS/HiggsToTauTauWorkingLegacyRun2#Jets )
            // PU jet ID WP = 2: loose
            if (PUjetID_WP > -1)
            {
              if ( !(CheckBit(theBigTree.jets_PUJetIDupdated_WP->at(iJet), PUjetID_WP)) && ijet.Pt()<50.) continue;
            }

            // Apply further cleaning for 2017 noisy jets, as suggested by HTT group: https://twiki.cern.ch/twiki/bin/view/CMS/HiggsToTauTauWorkingLegacyRun2#Jets
            // The noisy jets to be removed are defined as: 20 < pt < 50 && abs(eta) > 2.65 && abs(eta) < 3.139
            if ( ijet.Pt()<50. && fabs(ijet.Eta())>2.65 && fabs(ijet.Eta()<3.139) ) continue;

            for (unsigned int kJet = iJet+1 ;   (kJet < theBigTree.jets_px->size ()) && (theSmallTree.m_njets < maxNjetsSaved) ;  ++kJet)
            {
              // JET PU ID cut
              if (theBigTree.PFjetID->at (kJet) < PFjetID_WP) continue; // 0 ; don't pass PF Jet ID; 1: tight, 2: tightLepVeto

              // Skip the already selected b-jets
              if (int (kJet) == bjet1idx) continue;
              if (bPairFound && int (kJet) == bjet2idx_temp) continue;

              TLorentzVector kjet;
              kjet.SetPxPyPzE(
                             theBigTree.jets_px->at (kJet),
                             theBigTree.jets_py->at (kJet),
                             theBigTree.jets_pz->at (kJet),
                             theBigTree.jets_e ->at (kJet)
                             );

              if (kjet.DeltaR (tlv_firstLepton) < lepCleaningCone) continue ;
              if (kjet.DeltaR (tlv_secondLepton) < lepCleaningCone) continue ;
              if(kjet.Pt() < 30.) continue;
              if(fabs(kjet.Eta()) > 4.7) continue; // max eta allowed: https://twiki.cern.ch/twiki/bin/view/CMS/HiggsToTauTauWorkingLegacyRun2#Jets

              // Apply PUjetID only to jets with pt < 50 GeV ( https://twiki.cern.ch/twiki/bin/view/CMS/HiggsToTauTauWorkingLegacyRun2#Jets )
              // PU jet ID WP = 2: loose
              if (PUjetID_WP > -1)
              {
                if ( !(CheckBit(theBigTree.jets_PUJetIDupdated_WP->at(kJet), PUjetID_WP)) && kjet.Pt()<50.) continue;
              }

              // Apply further cleaning for 2017 noisy jets, as suggested by HTT group: https://twiki.cern.ch/twiki/bin/view/CMS/HiggsToTauTauWorkingLegacyRun2#Jets
              // The noisy jets to be removed are defined as: 20 < pt < 50 && abs(eta) > 2.65 && abs(eta) < 3.139
              if ( kjet.Pt()<50. && fabs(kjet.Eta())>2.65 && fabs(kjet.Eta()<3.139) ) continue;

              TLorentzVector jetPair = ijet+kjet;

              bool VBFjetLegsMatched = true;
              if (isVBFfired) VBFjetLegsMatched = checkVBFjetMatch(DEBUG, iJet, kJet, theBigTree);
              if (isVBFfired && !VBFjetLegsMatched) continue;
              VBFcand_Mjj.push_back(make_tuple(jetPair.M(),iJet,kJet));
            }
          }

          // if is a VBF event (in the tautau channel) but no good candidate is found --> throw away the event
          if (isVBFfired && VBFcand_Mjj.size()<=0) continue;

          if (VBFcand_Mjj.size()>0)
          {
            std::sort(VBFcand_Mjj.begin(),VBFcand_Mjj.end());
            isVBF = true;
            VBFidx1 = std::get<1>(*(VBFcand_Mjj.rbegin()));
            VBFidx2 = std::get<2>(*(VBFcand_Mjj.rbegin()));
          }

        } // all possible VBF jets pairs built

        int bjet2idx_notVBF = bjet2idx_temp;  // assign 2nd jet by CSV in any case, then:
        int bjet2idx_isVBF  = bjet2idx_temp;

        if (!bPairFound)  // if the bjet2 was not definitive yet
        {
          if(isVBF)
          {
            if ((bjet2idx_temp == VBFidx1) || (bjet2idx_temp == VBFidx2))   // and the 2nd jet by CSV was already picked as VBF jet
            {
              for (int bidx = 2; bidx<=int(jets_and_sortPar.size()); bidx++)  // look for the next jet by CSV
              {
                int idxbyCSV = jets_and_sortPar.at(njets-bidx).second;
                if ((idxbyCSV== VBFidx1)||(idxbyCSV== VBFidx2)) continue;
                bPairFound = true;
                bjet2idx_isVBF = idxbyCSV;
                break;
              }
              if(bPairFound == false)  // if there were not enough jets
              {
                isVBF = false;         // discard the jets as VBF jets
                VBFcand_Mjj.clear();
              }
            }
          }
        }

        if (isVBF)
            theSmallTree.m_isVBF = 1;
        else
            theSmallTree.m_isVBF = 0;

        // Use this for VBF trigger SF
        if (isVBF && isVBFfired) theSmallTree.m_isVBFtrigger = 1;

        const int bjet2idx = isVBF? bjet2idx_isVBF : bjet2idx_notVBF;

        // Now that I've selected the bjets build the TLorentzVectors
        TLorentzVector tlv_firstBjet (theBigTree.jets_px->at(bjet1idx), theBigTree.jets_py->at(bjet1idx), theBigTree.jets_pz->at(bjet1idx), theBigTree.jets_e->at(bjet1idx));
        TLorentzVector tlv_secondBjet(theBigTree.jets_px->at(bjet2idx), theBigTree.jets_py->at(bjet2idx), theBigTree.jets_pz->at(bjet2idx), theBigTree.jets_e->at(bjet2idx));
	
        bool bjets_gen_matched = ((tlv_firstBjet .DeltaR(vGenB1)<0.5 && tlv_secondBjet.DeltaR(vGenB2)<0.5) || (tlv_firstBjet.DeltaR(vGenB2)<0.5 && tlv_secondBjet.DeltaR(vGenB1)<0.5) );
        bool bjet1_gen_matched = ( tlv_firstBjet .DeltaR(vGenB1)<0.5 || tlv_firstBjet .DeltaR(vGenB2)<0.5 );
        bool bjet2_gen_matched = ( tlv_secondBjet.DeltaR(vGenB1)<0.5 || tlv_secondBjet.DeltaR(vGenB2)<0.5 );

        theSmallTree.m_bjets_gen_matched = bjets_gen_matched ? 1 : 0;
        theSmallTree.m_bjet1_gen_matched = bjet1_gen_matched ? 1 : 0;
        theSmallTree.m_bjet2_gen_matched = bjet2_gen_matched ? 1 : 0;

        double ptRegr[2] = {tlv_firstBjet.Pt(), tlv_secondBjet.Pt()};
        //if (computeBregr)
        //{
        //  for (int iBJet = 0; iBJet <=1; iBJet++)
        //  {
        //    int bidx = (iBJet == 0 ? bjet1idx : bjet2idx);
        //    bjrv.Jet_pt     = (iBJet == 0 ? tlv_firstBjet.Pt()  : tlv_secondBjet.Pt());
        //    bjrv.Jet_eta    = (iBJet == 0 ? tlv_firstBjet.Eta() : tlv_secondBjet.Eta());
        //    //bjrv.Jet_corr         = theBigTree.jets_rawPt->at(bidx);
        //    bjrv.Jet_corr         = theBigTree.jetRawf->at(bidx); // should be 1./jetrawf ??
        //    bjrv.rho              = theBigTree.rho;
        //    bjrv.Jet_mt           = theBigTree.jets_mT->at(bidx);
        //    bjrv.Jet_leadTrackPt  = theBigTree.jets_leadTrackPt->at(bidx);
        //    bjrv.Jet_leptonPtRel  = theBigTree.jets_leptonPtRel->at(bidx);
        //    bjrv.Jet_leptonPt     = theBigTree.jets_leptonPt->at(bidx);
        //    bjrv.Jet_leptonDeltaR = theBigTree.jets_leptonDeltaR->at(bidx);
        //    bjrv.Jet_neHEF   = theBigTree.jets_nHEF->at(bidx);
        //    bjrv.Jet_neEmEF  = theBigTree.jets_nEmEF->at(bidx);
        //    bjrv.Jet_chMult  = theBigTree.jets_chMult->at(bidx);
        //    bjrv.Jet_vtxPt   = theBigTree.jets_vtxPt->at(bidx);
        //    bjrv.Jet_vtxMass = theBigTree.jets_vtxMass->at(bidx);
        //    bjrv.Jet_vtx3dL  = theBigTree.jets_vtx3dL->at(bidx);
        //    bjrv.Jet_vtxNtrk = theBigTree.jets_vtxNtrk->at(bidx);
        //    bjrv.Jet_vtx3deL = theBigTree.jets_vtx3deL->at(bidx);

        //    ptRegr[iBJet] = (bRreader->EvaluateRegression (bRegrMethodName.c_str()))[0];
        //  }
        //}

        // save the b-jets
        TLorentzVector tlv_firstBjet_raw = tlv_firstBjet;
        TLorentzVector tlv_secondBjet_raw = tlv_secondBjet;

        // ----- up/down variation using JEC
        double unc_first = theBigTree.jets_jecUnc->at (bjet1idx);
        TLorentzVector tlv_firstBjet_raw_jetup = tlv_firstBjet_raw;
        TLorentzVector tlv_firstBjet_raw_jetdown = tlv_firstBjet_raw;

        tlv_firstBjet_raw_jetup.SetPtEtaPhiE(
                           (1.+unc_first) *tlv_firstBjet_raw_jetup.Pt(),
                           tlv_firstBjet_raw_jetup.Eta(),
                           tlv_firstBjet_raw_jetup.Phi(),
                           (1.+unc_first) *tlv_firstBjet_raw_jetup.E()
                           );
        tlv_firstBjet_raw_jetdown.SetPtEtaPhiE(
                         (1.-unc_first) *tlv_firstBjet_raw_jetdown.Pt(),
                         tlv_firstBjet_raw_jetdown.Eta(),
                         tlv_firstBjet_raw_jetdown.Phi(),
                         (1.-unc_first) *tlv_firstBjet_raw_jetdown.E()
                         );

        double unc_second = theBigTree.jets_jecUnc->at (bjet2idx);
        TLorentzVector tlv_secondBjet_raw_jetup = tlv_secondBjet_raw;
        TLorentzVector tlv_secondBjet_raw_jetdown = tlv_secondBjet_raw;

        tlv_secondBjet_raw_jetup.SetPtEtaPhiE(
                        (1.+unc_second) *tlv_secondBjet_raw_jetup.Pt(),
                        tlv_secondBjet_raw_jetup.Eta(),
                        tlv_secondBjet_raw_jetup.Phi(),
                        (1.+unc_second) *tlv_secondBjet_raw_jetup.E()
                        );
        tlv_secondBjet_raw_jetdown.SetPtEtaPhiE(
                          (1.-unc_second) *tlv_secondBjet_raw_jetdown.Pt(),
                          tlv_secondBjet_raw_jetdown.Eta(),
                          tlv_secondBjet_raw_jetdown.Phi(),
                          (1.-unc_second) *tlv_secondBjet_raw_jetdown.E()
                          );

        if (DEBUG)
        {
            cout << "-------- JET JEC -------" << endl;
            cout << "jet1 UP: " << tlv_firstBjet_raw_jetup.Pt() <<endl;
            cout << "jet1 DW: " << tlv_firstBjet_raw_jetdown.Pt()<<endl;
            cout << "jet2 UP: " << tlv_secondBjet_raw_jetup.Pt()<<endl;
            cout << "jet2 DW: " << tlv_secondBjet_raw_jetdown.Pt()<<endl;
        }

        theSmallTree.m_bjet1_pt_raw = tlv_firstBjet_raw.Pt();
        theSmallTree.m_bjet2_pt_raw = tlv_secondBjet_raw.Pt();
        theSmallTree.m_bjet1_pt_raw_jetup = tlv_firstBjet_raw_jetup.Pt();
        theSmallTree.m_bjet2_pt_raw_jetup = tlv_secondBjet_raw_jetup.Pt();
        theSmallTree.m_bjet1_pt_raw_jetdown = tlv_firstBjet_raw_jetdown.Pt();
        theSmallTree.m_bjet2_pt_raw_jetdown = tlv_secondBjet_raw_jetdown.Pt();

        TLorentzVector tlv_bH_raw = tlv_firstBjet + tlv_secondBjet ;
        TLorentzVector tlv_bH_raw_jetup   = tlv_firstBjet_raw_jetup + tlv_secondBjet_raw_jetup ;
        TLorentzVector tlv_bH_raw_jetdown =  tlv_firstBjet_raw_jetdown + tlv_secondBjet_raw_jetdown ;

        theSmallTree.m_bH_mass_raw = tlv_bH_raw.M();
        theSmallTree.m_bH_mass_raw_jetup   = tlv_bH_raw_jetup.M();
        theSmallTree.m_bH_mass_raw_jetdown = tlv_bH_raw_jetdown.M();

        // FIXME : here mass is manually set to 0, should we change it?
        float ptScale1 = ptRegr[0] / tlv_firstBjet.Pt() ;
        float ptScale2 = ptRegr[1] / tlv_secondBjet.Pt() ;
        tlv_firstBjet.SetPtEtaPhiE (ptRegr[0], tlv_firstBjet.Eta(), tlv_firstBjet.Phi(), ptScale1*tlv_firstBjet.Energy());
        tlv_secondBjet.SetPtEtaPhiE (ptRegr[1], tlv_secondBjet.Eta(), tlv_secondBjet.Phi(), ptScale2*tlv_secondBjet.Energy());

        theSmallTree.m_bjet1_pt   = tlv_firstBjet.Pt () ;
        theSmallTree.m_bjet1_eta  = tlv_firstBjet.Eta () ;
        theSmallTree.m_bjet1_phi  = tlv_firstBjet.Phi () ;
        theSmallTree.m_bjet1_e    = theBigTree.jets_e->at (bjet1idx) ;
        theSmallTree.m_bjet1_bID  = theBigTree.bCSVscore->at (bjet1idx) ;
        theSmallTree.m_bjet1_bID_deepCSV  = theBigTree.bDeepCSV_probb->at(bjet1idx) + theBigTree.bDeepCSV_probbb->at(bjet1idx) ;
        theSmallTree.m_bjet1_bID_deepFlavor  = theBigTree.bDeepFlavor_probb->at(bjet1idx) + theBigTree.bDeepFlavor_probbb->at(bjet1idx) + theBigTree.bDeepFlavor_problepb->at(bjet1idx);
        theSmallTree.m_bjet1_bMVAID  = theBigTree.pfCombinedMVAV2BJetTags->at (bjet1idx) ;
        theSmallTree.m_bjet1_flav = theBigTree.jets_HadronFlavour->at (bjet1idx) ;
        double bjet1_JER = theBigTree.jets_JER->at(bjet1idx);
        theSmallTree.m_bjet1_JER = bjet1_JER ;

        theSmallTree.m_bjet2_pt   = tlv_secondBjet.Pt () ;
        theSmallTree.m_bjet2_eta  = tlv_secondBjet.Eta () ;
        theSmallTree.m_bjet2_phi  = tlv_secondBjet.Phi () ;
        theSmallTree.m_bjet2_e    = theBigTree.jets_e->at (bjet2idx) ;
        theSmallTree.m_bjet2_bID  = theBigTree.bCSVscore->at (bjet2idx) ;
        theSmallTree.m_bjet2_bID_deepCSV  = theBigTree.bDeepCSV_probb->at(bjet2idx) + theBigTree.bDeepCSV_probbb->at(bjet2idx) ;
        theSmallTree.m_bjet2_bID_deepFlavor  = theBigTree.bDeepFlavor_probb->at(bjet2idx) + theBigTree.bDeepFlavor_probbb->at(bjet2idx) + theBigTree.bDeepFlavor_problepb->at(bjet2idx);
        theSmallTree.m_bjet2_bMVAID  = theBigTree.pfCombinedMVAV2BJetTags->at (bjet2idx) ;
        theSmallTree.m_bjet2_flav = theBigTree.jets_HadronFlavour->at (bjet2idx) ;
        double bjet2_JER = theBigTree.jets_JER->at(bjet2idx);
        theSmallTree.m_bjet2_JER = bjet2_JER ;

        theSmallTree.m_bjets_bID  = theBigTree.bCSVscore->at (bjet1idx) +theBigTree.bCSVscore->at (bjet2idx) ;
        theSmallTree.m_bjets_bID_deepCSV  = theBigTree.bDeepCSV_probb->at(bjet1idx) + theBigTree.bDeepCSV_probbb->at(bjet1idx) + theBigTree.bDeepCSV_probb->at(bjet2idx) + theBigTree.bDeepCSV_probbb->at(bjet2idx);
        theSmallTree.m_bjets_bID_deepFlavor  = theBigTree.bDeepFlavor_probb->at(bjet1idx) + theBigTree.bDeepFlavor_probbb->at(bjet1idx) + theBigTree.bDeepFlavor_problepb->at(bjet1idx) + theBigTree.bDeepFlavor_probb->at(bjet2idx) + theBigTree.bDeepFlavor_probbb->at(bjet2idx) + theBigTree.bDeepFlavor_problepb->at(bjet2idx);
        
        // Save gen info for b-jets
        bool hasgj1 = false;
        bool hasgj2 = false;

        if (isMC)
        {
          int mcind = theBigTree.jets_genjetIndex->at(bjet1idx);
          if (mcind>=0)
          {
            TLorentzVector gen(theBigTree.genjet_px->at(mcind),theBigTree.genjet_py->at(mcind),theBigTree.genjet_pz->at(mcind),theBigTree.genjet_e->at(mcind));
            theSmallTree.m_genjet1_pt = gen.Pt();
            theSmallTree.m_genjet1_eta = gen.Eta();
            theSmallTree.m_genjet1_phi = gen.Phi();
            theSmallTree.m_genjet1_e = gen.E();
            if (gen.Pt() > 8) hasgj1 = true;
          }

          mcind = theBigTree.jets_genjetIndex->at(bjet2idx);
          if (mcind>=0)
          {
            TLorentzVector gen(theBigTree.genjet_px->at(mcind),theBigTree.genjet_py->at(mcind),theBigTree.genjet_pz->at(mcind),theBigTree.genjet_e->at(mcind));
            theSmallTree.m_genjet2_pt = gen.Pt();
            theSmallTree.m_genjet2_eta = gen.Eta();
            theSmallTree.m_genjet2_phi = gen.Phi();
            theSmallTree.m_genjet2_e = gen.E();
            if (gen.Pt() > 8) hasgj2 = true;
          }
        }
        theSmallTree.m_bjet1_hasgenjet = hasgj1 ;
        theSmallTree.m_bjet2_hasgenjet = hasgj2 ;

        // Save HT_20, HT_50 and HT_20_BDT(with cut on |eta|<4.7)
        TLorentzVector jetVecSum (0,0,0,0);
        if (DEBUG) cout << "----- BDT HT debug ------" << endl;
        for (unsigned int iJet = 0 ; iJet < theBigTree.jets_px->size () ; ++iJet)
        {
          // JET PU ID cut
          if (theBigTree.PFjetID->at (iJet) < PFjetID_WP) continue; // 0 ; don't pass PF Jet ID; 1: tight, 2: tightLepVeto

          // Build the jet TLorentzVector
          TLorentzVector tlv_jet(theBigTree.jets_px->at(iJet), theBigTree.jets_py->at(iJet), theBigTree.jets_pz->at(iJet), theBigTree.jets_e->at(iJet)) ;

          // Pt cut for jets
          if (tlv_jet.Pt () < 20.) continue ;

          // Lepton and b-jet cleaning
          if (tlv_jet.DeltaR (tlv_firstLepton) < lepCleaningCone) continue ;
          if (tlv_jet.DeltaR (tlv_secondLepton) < lepCleaningCone) continue ;
          if ( (int) iJet == bjet1idx || (int) iJet == bjet2idx ) continue ;

          // Apply PUjetID only to jets with pt < 50 GeV ( https://twiki.cern.ch/twiki/bin/view/CMS/HiggsToTauTauWorkingLegacyRun2#Jets )
          // PU jet ID WP = 2: loose
          if (PUjetID_WP > -1)
          {
            if ( !(CheckBit(theBigTree.jets_PUJetIDupdated_WP->at(iJet), PUjetID_WP)) && tlv_jet.Pt()<50.) continue;
          }

          // Apply further cleaning for 2017 noisy jets, as suggested by HTT group: https://twiki.cern.ch/twiki/bin/view/CMS/HiggsToTauTauWorkingLegacyRun2#Jets
          // The noisy jets to be removed are defined as: 20 < pt < 50 && abs(eta) > 2.65 && abs(eta) < 3.139
          if ( tlv_jet.Pt()<50. && fabs(tlv_jet.Eta())>2.65 && fabs(tlv_jet.Eta()<3.139) ) continue;

          // use these jets for HT
          if (tlv_jet.Pt () > 20)
          {
            ++theSmallTree.m_njets20 ;
            theSmallTree.m_HT20 += tlv_jet.Pt() ;
            jetVecSum += tlv_jet ;

            if (TMath::Abs(tlv_jet.Eta()) < 4.7)
            {
              theSmallTree.m_BDT_HT20 += tlv_jet.Pt() ;
              if (DEBUG) cout << " ---> Jet " << iJet << " - pt: " << tlv_jet.Pt() << " - HT: " << theSmallTree.m_BDT_HT20 << endl;
            }
          }
          if (tlv_jet.Pt () > 50)
          {
            ++theSmallTree.m_njets50 ;
            theSmallTree.m_HT50 += tlv_jet.Pt() ;
          }
        }
        theSmallTree.m_HT20Full = theSmallTree.m_HT20 + tlv_firstLepton.Pt() + tlv_secondLepton.Pt() ;
        theSmallTree.m_jet20centrality = jetVecSum.Pt() / theSmallTree.m_HT20Full ;

        if (DEBUG) cout << "  HT = " << theSmallTree.m_BDT_HT20 << endl;
        if (DEBUG) cout << "---------------------" << endl;

        float METx = theBigTree.METx->at (chosenTauPair) ;
        float METy = theBigTree.METy->at (chosenTauPair) ;
        //float METpt = 0;//TMath::Sqrt (METx*METx + METy*METy) ;

        TLorentzVector tlv_bH = tlv_firstBjet + tlv_secondBjet ;
        TLorentzVector tlv_neutrinos =  tlv_bH - tlv_bH_raw;
        theSmallTree.m_met_et_corr = theBigTree.met - tlv_neutrinos.Et() ;

        const TVector2 ptmiss = TVector2(METx, METy) ;
        TMatrixD metcov (2, 2) ;
        metcov (0,0) = theBigTree.MET_cov00->at (chosenTauPair) ;
        metcov (1,0) = theBigTree.MET_cov10->at (chosenTauPair) ;
        metcov (0,1) = theBigTree.MET_cov01->at (chosenTauPair) ;
        metcov (1,1) = theBigTree.MET_cov11->at (chosenTauPair) ;
        const TMatrixD stableMetCov = metcov;

        // MET shifted for JES
        // This will be useful when splitting JECs
        //const TVector2 ptmiss_jetup   = getShiftedMET(+1., ptmiss, theBigTree);
        //const TVector2 ptmiss_jetdown = getShiftedMET(-1., ptmiss, theBigTree);
        // For now we use the total shift already stored in LLR ntuples
        const TVector2 ptmiss_jetup   (theBigTree.METx_UP_JES->at(chosenTauPair) , theBigTree.METy_UP_JES->at(chosenTauPair));
        const TVector2 ptmiss_jetdown (theBigTree.METx_DOWN_JES->at(chosenTauPair) , theBigTree.METy_DOWN_JES->at(chosenTauPair));

        // MET shifted for TES
        const TVector2 ptmiss_tauup   (theBigTree.METx_UP_TES->at(chosenTauPair) , theBigTree.METy_UP_TES->at(chosenTauPair));
        const TVector2 ptmiss_taudown (theBigTree.METx_DOWN_TES->at(chosenTauPair) , theBigTree.METy_DOWN_TES->at(chosenTauPair));

        // MET shifted for EES
        const TVector2 ptmiss_eleup   (theBigTree.METx_UP_EES->at(chosenTauPair) , theBigTree.METy_UP_EES->at(chosenTauPair));
        const TVector2 ptmiss_eledown (theBigTree.METx_DOWN_EES->at(chosenTauPair) , theBigTree.METy_DOWN_EES->at(chosenTauPair));

        theSmallTree.m_bH_pt = tlv_bH.Pt () ;
        theSmallTree.m_bH_eta = tlv_bH.Eta () ;
        theSmallTree.m_bH_phi = tlv_bH.Phi () ;
        theSmallTree.m_bH_e = tlv_bH.E () ;
        theSmallTree.m_bH_mass = tlv_bH.M () ;

        theSmallTree.m_bH_MET_deltaEta    = std::abs(tlv_bH.Eta()); // since MET.Eta()==0 by definition, dEta(bH,MET)=|bH.Eta()|
        theSmallTree.m_bH_MET_deltaR      = tlv_bH.DeltaR(tlv_MET);
        theSmallTree.m_bH_tauH_MET_deltaR = tlv_bH.DeltaR(tlv_tauH + tlv_MET);
        theSmallTree.m_BDT_bHMet_deltaPhi = ROOT::Math::VectorUtil::DeltaPhi(tlv_bH, tlv_MET);
        theSmallTree.m_BDT_topPairMasses  = Calculate_topPairMasses(getLVfromTLV(tlv_firstLepton), getLVfromTLV(tlv_secondLepton), getLVfromTLV(tlv_firstBjet), getLVfromTLV(tlv_secondBjet), getLVfromTLV(tlv_MET)).first;
        theSmallTree.m_BDT_MX             = Calculate_MX(tlv_firstLepton, tlv_secondLepton, tlv_firstBjet, tlv_secondBjet, tlv_MET);
        theSmallTree.m_BDT_bH_tauH_MET_InvMass = ROOT::Math::VectorUtil::InvariantMass(tlv_bH, tlv_tauH + tlv_MET);
        theSmallTree.m_BDT_bH_tauH_InvMass     = ROOT::Math::VectorUtil::InvariantMass(tlv_bH, tlv_tauH);
        theSmallTree.m_BDT_MET_bH_cosTheta = Calculate_cosTheta_2bodies(getLVfromTLV(tlv_MET), getLVfromTLV(tlv_bH));
        theSmallTree.m_BDT_b1_bH_cosTheta  = Calculate_cosTheta_2bodies(getLVfromTLV(tlv_firstBjet), getLVfromTLV(tlv_bH));

        TLorentzVector tlv_HH = tlv_bH + tlv_tauH ;
        TLorentzVector tlv_HH_raw = tlv_bH_raw + tlv_tauH ;
        theSmallTree.m_HH_pt = tlv_HH.Pt () ;
        theSmallTree.m_HH_eta = tlv_HH.Eta () ;
        theSmallTree.m_HH_phi = tlv_HH.Phi () ;
        theSmallTree.m_HH_e = tlv_HH.E () ;
        theSmallTree.m_HH_mass = tlv_HH.M () ;
        theSmallTree.m_HH_mass_raw = tlv_HH_raw.M () ;
        theSmallTree.m_HH_deltaR = tlv_bH.DeltaR(tlv_tauH);

        TLorentzVector tlv_HH_raw_tauup = tlv_bH_raw + tlv_firstLepton_tauup + tlv_secondLepton_tauup ;
        TLorentzVector tlv_HH_raw_taudown = tlv_bH_raw + tlv_firstLepton_taudown + tlv_secondLepton_taudown ;

        theSmallTree.m_HH_mass_raw_tauup = tlv_HH_raw_tauup.M();
        theSmallTree.m_HH_mass_raw_taudown = tlv_HH_raw_taudown.M();

        TLorentzVector tlv_HH_raw_eleup = tlv_bH_raw + tlv_firstLepton_eleup + tlv_secondLepton_eleup ;
        TLorentzVector tlv_HH_raw_eledown = tlv_bH_raw + tlv_firstLepton_eledown + tlv_secondLepton_eledown ;

        theSmallTree.m_HH_mass_raw_eleup = tlv_HH_raw_eleup.M();
        theSmallTree.m_HH_mass_raw_eledown = tlv_HH_raw_eledown.M();

        // in case the SVFIT mass is calculated
        if (theBigTree.SVfitMass->at (chosenTauPair) > -900.)
        {
          TLorentzVector tlv_HHsvfit  = tlv_bH + tlv_tauH_SVFIT ;
          theSmallTree.m_HHsvfit_pt   = tlv_HHsvfit.Pt () ;
          theSmallTree.m_HHsvfit_eta  = tlv_HHsvfit.Eta () ;
          theSmallTree.m_HHsvfit_phi  = tlv_HHsvfit.Phi () ;
          theSmallTree.m_HHsvfit_e    = tlv_HHsvfit.E () ;
          theSmallTree.m_HHsvfit_mass = tlv_HHsvfit.M () ;

          theSmallTree.m_BDT_HHsvfit_abs_deltaPhi  = fabs(ROOT::Math::VectorUtil::DeltaPhi(tlv_bH, tlv_tauH_SVFIT));
          theSmallTree.m_BDT_bH_tauH_SVFIT_InvMass = ROOT::Math::VectorUtil::InvariantMass(tlv_bH, tlv_tauH_SVFIT);
          theSmallTree.m_BDT_total_CalcPhi = Calculate_phi (getLVfromTLV(tlv_firstLepton), getLVfromTLV(tlv_secondLepton), getLVfromTLV(tlv_firstBjet), getLVfromTLV(tlv_secondBjet), getLVfromTLV(tlv_tauH_SVFIT), getLVfromTLV(tlv_bH));
          theSmallTree.m_BDT_ditau_CalcPhi = Calculate_phi1(getLVfromTLV(tlv_firstLepton), getLVfromTLV(tlv_secondLepton), getLVfromTLV(tlv_tauH_SVFIT), getLVfromTLV(tlv_bH));
          theSmallTree.m_BDT_dib_CalcPhi   = Calculate_phi1(getLVfromTLV(tlv_firstBjet), getLVfromTLV(tlv_secondBjet), getLVfromTLV(tlv_tauH_SVFIT), getLVfromTLV(tlv_bH));
          theSmallTree.m_BDT_MET_tauH_SVFIT_cosTheta   = Calculate_cosTheta_2bodies(getLVfromTLV(tlv_MET), getLVfromTLV(tlv_tauH_SVFIT));
          theSmallTree.m_BDT_tauH_SVFIT_reson_cosTheta = Calculate_cosTheta_2bodies(getLVfromTLV(tlv_tauH_SVFIT), getLVfromTLV((tlv_firstLepton+tlv_secondLepton+tlv_firstBjet+tlv_secondBjet+tlv_MET)) );

        } // in case the SVFIT mass is calculated

        // compute HHKinFit -- ask a reasonable mass window to suppress most error messages
        bool wrongHHK=false;
        float HHKmass = -999;
        float HHKChi2 = -999;
        // if (runHHKinFit && tlv_HH_raw.M() > 20 && tlv_HH_raw.M() < 200)
        //if (runHHKinFit && pairType <= 2 && tlv_bH_raw.M() > 50 && tlv_bH_raw.M() < 200 && theBigTree.SVfitMass->at (chosenTauPair) > 50 && theBigTree.SVfitMass->at (chosenTauPair) < 200) // no kinfit for ee / mumu + very loose mass window
        if (runHHKinFit && pairType <= 3) // FIXME: temporary
        {
          HHKinFit2::HHKinFitMasterHeavyHiggs kinFits = HHKinFit2::HHKinFitMasterHeavyHiggs(tlv_firstBjet, tlv_secondBjet, tlv_firstLepton, tlv_secondLepton, ptmiss, stableMetCov, bjet1_JER, bjet2_JER) ;
          HHKinFit2::HHKinFitMasterHeavyHiggs kinFitsraw = HHKinFit2::HHKinFitMasterHeavyHiggs(tlv_firstBjet_raw, tlv_secondBjet_raw, tlv_firstLepton, tlv_secondLepton,  ptmiss, stableMetCov, bjet1_JER, bjet2_JER) ;
          HHKinFit2::HHKinFitMasterHeavyHiggs kinFitsraw_tauup = HHKinFit2::HHKinFitMasterHeavyHiggs(tlv_firstBjet_raw, tlv_secondBjet_raw, tlv_firstLepton_tauup, tlv_secondLepton_tauup,  ptmiss_tauup, stableMetCov, bjet1_JER, bjet2_JER) ;
          HHKinFit2::HHKinFitMasterHeavyHiggs kinFitsraw_taudown = HHKinFit2::HHKinFitMasterHeavyHiggs(tlv_firstBjet_raw, tlv_secondBjet_raw, tlv_firstLepton_taudown, tlv_secondLepton_taudown,  ptmiss_taudown, stableMetCov, bjet1_JER, bjet2_JER) ;
          HHKinFit2::HHKinFitMasterHeavyHiggs kinFitsraw_jetup = HHKinFit2::HHKinFitMasterHeavyHiggs(tlv_firstBjet_raw_jetup, tlv_secondBjet_raw_jetup, tlv_firstLepton, tlv_secondLepton,  ptmiss_jetup, stableMetCov, bjet1_JER, bjet2_JER) ;
          HHKinFit2::HHKinFitMasterHeavyHiggs kinFitsraw_jetdown = HHKinFit2::HHKinFitMasterHeavyHiggs(tlv_firstBjet_raw_jetdown, tlv_secondBjet_raw_jetdown, tlv_firstLepton, tlv_secondLepton,  ptmiss_jetdown, stableMetCov, bjet1_JER, bjet2_JER) ;
          HHKinFit2::HHKinFitMasterHeavyHiggs kinFitsraw_eleup = HHKinFit2::HHKinFitMasterHeavyHiggs(tlv_firstBjet_raw, tlv_secondBjet_raw, tlv_firstLepton_eleup, tlv_secondLepton_eleup,  ptmiss_eleup, stableMetCov, bjet1_JER, bjet2_JER) ;
          HHKinFit2::HHKinFitMasterHeavyHiggs kinFitsraw_eledown = HHKinFit2::HHKinFitMasterHeavyHiggs(tlv_firstBjet_raw, tlv_secondBjet_raw, tlv_firstLepton_eledown, tlv_secondLepton_eledown,  ptmiss_eledown, stableMetCov, bjet1_JER, bjet2_JER) ;

          //           kinFits.setAdvancedBalance (&ptmiss, metcov) ;
          //           kinFits.setSimpleBalance (ptmiss.Pt (),10) ; //alternative which uses only the absolute value of ptmiss in the fit
          //
          //           kinFits.addMh1Hypothesis (hypo_mh1) ;
          //           kinFits.addMh2Hypothesis (hypo_mh2) ;
          kinFits.   addHypo(hypo_mh1,hypo_mh2);
          kinFitsraw.addHypo(hypo_mh1,hypo_mh2);
          kinFitsraw_tauup.addHypo(hypo_mh1,hypo_mh2);
          kinFitsraw_taudown.addHypo(hypo_mh1,hypo_mh2);
          kinFitsraw_jetup.addHypo(hypo_mh1,hypo_mh2);
          kinFitsraw_jetdown.addHypo(hypo_mh1,hypo_mh2);
          kinFitsraw_eleup.addHypo(hypo_mh1,hypo_mh2);
          kinFitsraw_eledown.addHypo(hypo_mh1,hypo_mh2);

          try{ kinFits.fit();}
          catch(HHKinFit2::HHInvMConstraintException e)
          {
            cout<<"INVME THIS EVENT WAS WRONG, INV MASS CONSTRAIN EXCEPTION"<<endl;
            cout<<"INVME masshypo1 = 125,    masshypo2 = 125"<<endl;
            cout<<"INVME Tau1"<<endl;
            cout<<"INVME (E,Px,Py,Pz,M) "<<tlv_firstLepton.E()<<","<<tlv_firstLepton.Px()<<","<<tlv_firstLepton.Py()<<","<<tlv_firstLepton.Pz()<<","<<tlv_firstLepton.M()<<endl;
            cout<<"INVME Tau2"<<endl;
            cout<<"INVME (E,Px,Py,Pz,M) "<<tlv_secondLepton.E()<<","<<tlv_secondLepton.Px()<<","<<tlv_secondLepton.Py()<<","<<tlv_secondLepton.Pz()<<","<<tlv_secondLepton.M()<<endl;
            cout<<"INVME B1"<<endl;
            cout<<"INVME (E,Px,Py,Pz,M) "<<tlv_firstBjet.E()<<","<<tlv_firstBjet.Px()<<","<<tlv_firstBjet.Py()<<","<<tlv_firstBjet.Pz()<<","<<tlv_firstBjet.M()<<endl;
            cout<<"INVME B2"<<endl;
            cout<<"INVME (E,Px,Py,Pz,M) "<<tlv_secondBjet.E()<<","<<tlv_secondBjet.Px()<<","<<tlv_secondBjet.Py()<<","<<tlv_secondBjet.Pz()<<","<<tlv_secondBjet.M()<<endl;
            cout<<"INVME MET"<<endl;
            cout<<"INVME (E,Px,Py,Pz,M) "<<","<<ptmiss.Px()<<","<<ptmiss.Py()<<endl;
            cout<<"INVME METCOV "<<endl;
            cout<<"INVME "<<metcov (0,0)<<"  "<<metcov (0,1)<<endl;// = theBigTree.MET_cov00->at (chosenTauPair) ;
            cout<<"INVME "<<metcov (1,0)<<"  "<<metcov (1,1)<<endl;// = theBigTree.MET_cov10->at (chosenTauPair) ;
            cout<<"INVME tau1, tau2, b1, b2"<<endl;
            cout<<"INVME ";
            tlv_firstLepton.Print();
            cout<<"INVME ";
            tlv_secondLepton.Print();
            cout<<"INVME ";
            tlv_firstBjet.Print();
            cout<<"INVME ";
            tlv_secondBjet.Print();
            wrongHHK=true;
          }
          catch (HHKinFit2::HHEnergyRangeException e)
          {
            cout<<"ERANGE THIS EVENT WAS WRONG, ENERGY RANGE EXCEPTION"<<endl;
            cout<<"ERANGE masshypo1 = 125,    masshypo2 = 125"<<endl;
            cout<<"ERANGE Tau1"<<endl;
            cout<<"ERANGE (E,Px,Py,Pz,M) "<<tlv_firstLepton.E()<<","<<tlv_firstLepton.Px()<<","<<tlv_firstLepton.Py()<<","<<tlv_firstLepton.Pz()<<","<<tlv_firstLepton.M()<<endl;
            cout<<"ERANGE Tau2"<<endl;
            cout<<"ERANGE (E,Px,Py,Pz,M) "<<tlv_secondLepton.E()<<","<<tlv_secondLepton.Px()<<","<<tlv_secondLepton.Py()<<","<<tlv_secondLepton.Pz()<<","<<tlv_secondLepton.M()<<endl;
            cout<<"ERANGE B1"<<endl;
            cout<<"ERANGE (E,Px,Py,Pz,M) "<<tlv_firstBjet.E()<<","<<tlv_firstBjet.Px()<<","<<tlv_firstBjet.Py()<<","<<tlv_firstBjet.Pz()<<","<<tlv_firstBjet.M()<<endl;
            cout<<"ERANGE B2"<<endl;
            cout<<"ERANGE (E,Px,Py,Pz,M) "<<tlv_secondBjet.E()<<","<<tlv_secondBjet.Px()<<","<<tlv_secondBjet.Py()<<","<<tlv_secondBjet.Pz()<<","<<tlv_secondBjet.M()<<endl;
            cout<<"ERANGE MET"<<endl;
            cout<<"ERANGE (E,Px,Py,Pz,M) "<<","<<ptmiss.Px()<<","<<ptmiss.Py()<<endl;
            cout<<"ERANGE METCOV "<<endl;
            cout<<"ERANGE "<<metcov (0,0)<<"  "<<metcov (0,1)<<endl;// = theBigTree.MET_cov00->at (chosenTauPair) ;
            cout<<"ERANGE "<<metcov (1,0)<<"  "<<metcov (1,1)<<endl;// = theBigTree.MET_cov10->at (chosenTauPair) ;
            cout<<"ERANGE tau1, tau2, b1, b2"<<endl;
            cout<<"ERANGE ";
            tlv_firstLepton.Print();
            cout<<"ERANGE ";
            tlv_secondLepton.Print();
            cout<<"ERANGE ";
            tlv_firstBjet.Print();
            cout<<"ERANGE ";
            tlv_secondBjet.Print();
            wrongHHK=true;
          }
          catch(HHKinFit2::HHEnergyConstraintException e)
          {
            cout<<"ECON THIS EVENT WAS WRONG, ENERGY CONSTRAIN EXCEPTION"<<endl;
            cout<<"ECON masshypo1 = 125,    masshypo2 = 125"<<endl;
            cout<<"ECON Tau1"<<endl;
            cout<<"ECON (E,Px,Py,Pz,M) "<<tlv_firstLepton.E()<<","<<tlv_firstLepton.Px()<<","<<tlv_firstLepton.Py()<<","<<tlv_firstLepton.Pz()<<","<<tlv_firstLepton.M()<<endl;
            cout<<"ECON Tau2"<<endl;
            cout<<"ECON (E,Px,Py,Pz,M) "<<tlv_secondLepton.E()<<","<<tlv_secondLepton.Px()<<","<<tlv_secondLepton.Py()<<","<<tlv_secondLepton.Pz()<<","<<tlv_secondLepton.M()<<endl;
            cout<<"ECON B1"<<endl;
            cout<<"ECON (E,Px,Py,Pz,M) "<<tlv_firstBjet.E()<<","<<tlv_firstBjet.Px()<<","<<tlv_firstBjet.Py()<<","<<tlv_firstBjet.Pz()<<","<<tlv_firstBjet.M()<<endl;
            cout<<"ECON B2"<<endl;
            cout<<"ECON (E,Px,Py,Pz,M) "<<tlv_secondBjet.E()<<","<<tlv_secondBjet.Px()<<","<<tlv_secondBjet.Py()<<","<<tlv_secondBjet.Pz()<<","<<tlv_secondBjet.M()<<endl;
            cout<<"ECON MET"<<endl;
            cout<<"ECON (E,Px,Py,Pz,M) "<<","<<ptmiss.Px()<<","<<ptmiss.Py()<<endl;
            cout<<"ECON METCOV "<<endl;
            cout<<"ECON "<<metcov (0,0)<<"  "<<metcov (0,1)<<endl;// = theBigTree.MET_cov00->at (chosenTauPair) ;
            cout<<"ECON "<<metcov (1,0)<<"  "<<metcov (1,1)<<endl;// = theBigTree.MET_cov10->at (chosenTauPair) ;
            cout<<"ECON tau1, tau2, b1, b2"<<endl;
            cout<<"ECON ";
            tlv_firstLepton.Print();
            cout<<"ECON ";
            tlv_secondLepton.Print();
            cout<<"ECON ";
            tlv_firstBjet.Print();
            cout<<"ECON ";
            tlv_secondBjet.Print();
            wrongHHK=true;
          }
          if(!wrongHHK)
          {
            HHKmass = kinFits.getMH () ;
            HHKChi2 = kinFits.getChi2 () ;
          }
          else
          {
            if(isOS)HHKmass = -333;
          }

          // nominal kinfit raw
          bool wrongHHKraw =false;
          try {kinFitsraw.fit();}
          catch(HHKinFit2::HHInvMConstraintException e){wrongHHKraw=true;}
          catch(HHKinFit2::HHEnergyConstraintException e){wrongHHKraw=true;}
          catch (HHKinFit2::HHEnergyRangeException e){wrongHHKraw=true;}
          if(!wrongHHKraw)
          {
            theSmallTree.m_HHKin_mass_raw = kinFitsraw.getMH();
            theSmallTree.m_HHKin_mass_raw_chi2        = kinFitsraw.getChi2();
            theSmallTree.m_HHKin_mass_raw_convergence = kinFitsraw.getConvergence();
            theSmallTree.m_HHKin_mass_raw_prob        = kinFitsraw.getFitProb();
          }
          else theSmallTree.m_HHKin_mass_raw = -100 ;
          if (theBigTree.SVfitMass->at (chosenTauPair) > -900. && !wrongHHK)
          {
            TLorentzVector b1 = kinFits.getFittedBJet1();
            TLorentzVector b2 = kinFits.getFittedBJet2();
            TLorentzVector bH_HKin = b1 + b2;
            TLorentzVector tlv_HHsvfit = bH_HKin + tlv_tauH_SVFIT ;

            theSmallTree.m_HHkinsvfit_bHmass = bH_HKin.M();
            theSmallTree.m_HHkinsvfit_pt  = tlv_HHsvfit.Pt () ;
            theSmallTree.m_HHkinsvfit_eta = tlv_HHsvfit.Eta () ;
            theSmallTree.m_HHkinsvfit_phi = tlv_HHsvfit.Phi () ;
            theSmallTree.m_HHkinsvfit_e   = tlv_HHsvfit.E () ;
            theSmallTree.m_HHkinsvfit_m   = tlv_HHsvfit.M () ;
          } // in case the SVFIT mass is calculated

          // raw kinfit TES up
          bool wrongHHKraw_tauup =false;
          try {kinFitsraw_tauup.fit();}
          catch(HHKinFit2::HHInvMConstraintException e){wrongHHKraw_tauup=true;}
          catch(HHKinFit2::HHEnergyConstraintException e){wrongHHKraw_tauup=true;}
          catch (HHKinFit2::HHEnergyRangeException e){wrongHHKraw_tauup=true;}
          if(!wrongHHKraw_tauup){theSmallTree.m_HHKin_mass_raw_tauup = kinFitsraw_tauup.getMH();}
          else theSmallTree.m_HHKin_mass_raw_tauup = -100 ;

          // raw kinfit TES down
          bool wrongHHKraw_taudown =false;
          try {kinFitsraw_taudown.fit();}
          catch(HHKinFit2::HHInvMConstraintException e){wrongHHKraw_taudown=true;}
          catch(HHKinFit2::HHEnergyConstraintException e){wrongHHKraw_taudown=true;}
          catch (HHKinFit2::HHEnergyRangeException e){wrongHHKraw_taudown=true;}
          if(!wrongHHKraw_taudown){theSmallTree.m_HHKin_mass_raw_taudown = kinFitsraw_taudown.getMH();}
          else theSmallTree.m_HHKin_mass_raw_taudown = -100 ;

          // raw kinfit EES up
          bool wrongHHKraw_eleup =false;
          try {kinFitsraw_eleup.fit();}
          catch(HHKinFit2::HHInvMConstraintException e){wrongHHKraw_eleup=true;}
          catch(HHKinFit2::HHEnergyConstraintException e){wrongHHKraw_eleup=true;}
          catch (HHKinFit2::HHEnergyRangeException e){wrongHHKraw_eleup=true;}
          if(!wrongHHKraw_eleup){theSmallTree.m_HHKin_mass_raw_eleup = kinFitsraw_eleup.getMH();}
          else theSmallTree.m_HHKin_mass_raw_eleup = -100 ;

          // raw kinfit EES down
          bool wrongHHKraw_eledown =false;
          try {kinFitsraw_eledown.fit();}
          catch(HHKinFit2::HHInvMConstraintException e){wrongHHKraw_eledown=true;}
          catch(HHKinFit2::HHEnergyConstraintException e){wrongHHKraw_eledown=true;}
          catch (HHKinFit2::HHEnergyRangeException e){wrongHHKraw_eledown=true;}
          if(!wrongHHKraw_eledown){theSmallTree.m_HHKin_mass_raw_eledown = kinFitsraw_eledown.getMH();}
          else theSmallTree.m_HHKin_mass_raw_eledown = -100 ;

          // raw kinfit JES up
          bool wrongHHKraw_jetup =false;
          try {kinFitsraw_jetup.fit();}
          catch(HHKinFit2::HHInvMConstraintException e){wrongHHKraw_jetup=true;}
          catch(HHKinFit2::HHEnergyConstraintException e){wrongHHKraw_jetup=true;}
          catch (HHKinFit2::HHEnergyRangeException e){wrongHHKraw_jetup=true;}
          if(!wrongHHKraw_jetup){theSmallTree.m_HHKin_mass_raw_jetup = kinFitsraw_jetup.getMH();}
          else theSmallTree.m_HHKin_mass_raw_jetup = -100 ;

          // raw kinfit JES down
          bool wrongHHKraw_jetdown =false;
          try {kinFitsraw_jetdown.fit();}
          catch(HHKinFit2::HHInvMConstraintException e){wrongHHKraw_jetdown=true;}
          catch(HHKinFit2::HHEnergyConstraintException e){wrongHHKraw_jetdown=true;}
          catch (HHKinFit2::HHEnergyRangeException e){wrongHHKraw_jetdown=true;}
          if(!wrongHHKraw_jetdown){theSmallTree.m_HHKin_mass_raw_jetdown = kinFitsraw_jetdown.getMH();}
          else theSmallTree.m_HHKin_mass_raw_jetdown = -100 ;

        } // end if doing HHKinFit

        theSmallTree.m_HHKin_mass_raw_copy = theSmallTree.m_HHKin_mass_raw ; // store twice if different binning needed

        theSmallTree.m_HHKin_mass = HHKmass;//kinFits.getMH () ;
        theSmallTree.m_HHKin_chi2 = HHKChi2;//kinFits.getChi2 () ;

        // Stransverse mass
        if (runMT2)
        {
          double mVisA = tlv_firstBjet_raw.M(); // mass of visible object on side A.  Must be >=0.
          double pxA = tlv_firstBjet_raw.Px();  // x momentum of visible object on side A.
          double pyA = tlv_firstBjet_raw.Py();  // y momentum of visible object on side A.

          double mVisB = tlv_secondBjet_raw.M(); // mass of visible object on side B.  Must be >=0.
          double pxB = tlv_secondBjet_raw.Px();  // x momentum of visible object on side B.
          double pyB = tlv_secondBjet_raw.Py();  // y momentum of visible object on side B.

          double pxMiss = tlv_firstLepton.Px() + tlv_secondLepton.Px() + theBigTree.METx->at(chosenTauPair); // x component of missing transverse momentum.
          double pyMiss = tlv_firstLepton.Py() + tlv_secondLepton.Py() + theBigTree.METy->at(chosenTauPair); // y component of missing transverse momentum.

          double chiA = tlv_firstLepton.M();  // hypothesised mass of invisible on side A.  Must be >=0.
          double chiB = tlv_secondLepton.M(); // hypothesised mass of invisible on side B.  Must be >=0.

          // TES variations
          double pxMiss_tauup = tlv_firstLepton_tauup.Px() + tlv_secondLepton_tauup.Px() + theBigTree.METx_UP_TES->at(chosenTauPair); // shiftedMET for TES
          double pyMiss_tauup = tlv_firstLepton_tauup.Py() + tlv_secondLepton_tauup.Py() + theBigTree.METy_UP_TES->at(chosenTauPair); // shiftedMET for TES
          double chiA_tauup = tlv_firstLepton_tauup.M();  // hypothesised mass of invisible on side A.  Must be >=0.
          double chiB_tauup = tlv_secondLepton_tauup.M(); // hypothesised mass of invisible on side B.  Must be >=0.

          double pxMiss_taudown = tlv_firstLepton_taudown.Px() + tlv_secondLepton_taudown.Px() + theBigTree.METx_DOWN_TES->at(chosenTauPair); // shiftedMET for TES
          double pyMiss_taudown = tlv_firstLepton_taudown.Py() + tlv_secondLepton_taudown.Py() + theBigTree.METy_DOWN_TES->at(chosenTauPair); // shiftedMET for TES
          double chiA_taudown = tlv_firstLepton_taudown.M();  // hypothesised mass of invisible on side A.  Must be >=0.
          double chiB_taudown = tlv_secondLepton_taudown.M(); // hypothesised mass of invisible on side B.  Must be >=0.

          // JES variations
          double mVisA_jetup = tlv_firstBjet_raw_jetup.M();  // mass of visible object on side A.
          double pxA_jetup = tlv_firstBjet_raw_jetup.Px();   // x momentum of visible object on side A.
          double pyA_jetup = tlv_firstBjet_raw_jetup.Py();   // y momentum of visible object on side A.
          double mVisB_jetup = tlv_secondBjet_raw_jetup.M(); // mass of visible object on side B.
          double pxB_jetup = tlv_secondBjet_raw_jetup.Px();  // x momentum of visible object on side B.
          double pyB_jetup = tlv_secondBjet_raw_jetup.Py();  // y momentum of visible object on side B.
          double pxMiss_jetup = tlv_firstLepton.Px() + tlv_secondLepton.Px() + ptmiss_jetup.Px(); // shiftedMET for JES
          double pyMiss_jetup = tlv_firstLepton.Py() + tlv_secondLepton.Py() + ptmiss_jetup.Py(); // shiftedMET for JES

          double mVisA_jetdown = tlv_firstBjet_raw_jetdown.M();  // mass of visible object on side A.
          double pxA_jetdown = tlv_firstBjet_raw_jetdown.Px();   // x momentum of visible object on side A.
          double pyA_jetdown = tlv_firstBjet_raw_jetdown.Py();   // y momentum of visible object on side A.
          double mVisB_jetdown = tlv_secondBjet_raw_jetdown.M(); // mass of visible object on side B.
          double pxB_jetdown = tlv_secondBjet_raw_jetdown.Px();  // x momentum of visible object on side B.
          double pyB_jetdown = tlv_secondBjet_raw_jetdown.Py();  // y momentum of visible object on side B.
          double pxMiss_jetdown = tlv_firstLepton.Px() + tlv_secondLepton.Px() + ptmiss_jetdown.Px(); // shiftedMET for JES
          double pyMiss_jetdown = tlv_firstLepton.Py() + tlv_secondLepton.Py() + ptmiss_jetdown.Py(); // shiftedMET for JES

          // EES variations
          double pxMiss_eleup = tlv_firstLepton_eleup.Px() + tlv_secondLepton_eleup.Px() + theBigTree.METx_UP_EES->at(chosenTauPair); // shiftedMET for TES
          double pyMiss_eleup = tlv_firstLepton_eleup.Py() + tlv_secondLepton_eleup.Py() + theBigTree.METy_UP_EES->at(chosenTauPair); // shiftedMET for TES
          double chiA_eleup = tlv_firstLepton_eleup.M();  // hypothesised mass of invisible on side A.  Must be >=0.
          double chiB_eleup = tlv_secondLepton_eleup.M(); // hypothesised mass of invisible on side B.  Must be >=0.

          double pxMiss_eledown = tlv_firstLepton_eledown.Px() + tlv_secondLepton_eledown.Px() + theBigTree.METx_DOWN_EES->at(chosenTauPair); // shiftedMET for TES
          double pyMiss_eledown = tlv_firstLepton_eledown.Py() + tlv_secondLepton_eledown.Py() + theBigTree.METy_DOWN_EES->at(chosenTauPair); // shiftedMET for TES
          double chiA_eledown = tlv_firstLepton_eledown.M();  // hypothesised mass of invisible on side A.  Must be >=0.
          double chiB_eledown = tlv_secondLepton_eledown.M(); // hypothesised mass of invisible on side B.  Must be >=0.

          double desiredPrecisionOnMt2 = 0; // Must be >=0.  If 0 alg aims for machine precision.  if >0, MT2 computed to supplied absolute precision.

          asymm_mt2_lester_bisect::disableCopyrightMessage();
        
          double MT2 = asymm_mt2_lester_bisect::get_mT2(
                       mVisA, pxA, pyA,
                       mVisB, pxB, pyB,
                       pxMiss, pyMiss,
                       chiA, chiB,
                       desiredPrecisionOnMt2);

          double MT2_tauup = asymm_mt2_lester_bisect::get_mT2(
                             mVisA, pxA, pyA,
                             mVisB, pxB, pyB,
                             pxMiss_tauup, pyMiss_tauup,
                             chiA_tauup, chiB_tauup,
                             desiredPrecisionOnMt2);

          double MT2_taudown = asymm_mt2_lester_bisect::get_mT2(
                               mVisA, pxA, pyA,
                               mVisB, pxB, pyB,
                               pxMiss_taudown, pyMiss_taudown,
                               chiA_taudown, chiB_taudown,
                               desiredPrecisionOnMt2);

          double MT2_jetup = asymm_mt2_lester_bisect::get_mT2(
                             mVisA_jetup, pxA_jetup, pyA_jetup,
                             mVisB_jetup, pxB_jetup, pyB_jetup,
                             pxMiss_jetup, pyMiss_jetup, // shiftedMET
                             chiA, chiB,
                             desiredPrecisionOnMt2);

          double MT2_jetdown = asymm_mt2_lester_bisect::get_mT2(
                               mVisA_jetdown, pxA_jetdown, pyA_jetdown,
                               mVisB_jetdown, pxB_jetdown, pyB_jetdown,
                               pxMiss_jetdown, pyMiss_jetdown, // shiftedMET
                               chiA, chiB,
                               desiredPrecisionOnMt2);

          double MT2_eleup = asymm_mt2_lester_bisect::get_mT2(
                             mVisA, pxA, pyA,
                             mVisB, pxB, pyB,
                             pxMiss_eleup, pyMiss_eleup,
                             chiA_eleup, chiB_eleup,
                             desiredPrecisionOnMt2);

          double MT2_eledown = asymm_mt2_lester_bisect::get_mT2(
                               mVisA, pxA, pyA,
                               mVisB, pxB, pyB,
                               pxMiss_eledown, pyMiss_eledown,
                               chiA_eledown, chiB_eledown,
                               desiredPrecisionOnMt2);

          theSmallTree.m_MT2 = MT2;
          theSmallTree.m_MT2_tauup = MT2_tauup;
          theSmallTree.m_MT2_taudown = MT2_taudown;
          theSmallTree.m_MT2_jetup = MT2_jetup;
          theSmallTree.m_MT2_jetdown = MT2_jetdown;
          theSmallTree.m_MT2_eleup = MT2_eleup;
          theSmallTree.m_MT2_eledown = MT2_eledown;

        } // end calcultion of MT2

        if (DEBUG)  cout << "---------- MT2 DEBUG: " << theSmallTree.m_MT2 << endl;

        theSmallTree.m_HH_deltaPhi = deltaPhi (tlv_bH.Phi (), tlv_tauH.Phi ()) ;
        theSmallTree.m_HH_deltaEta = fabs(tlv_bH.Eta()- tlv_tauH.Eta ()) ;
        theSmallTree.m_HHsvfit_deltaPhi = deltaPhi (tlv_bH.Phi (), tlv_tauH_SVFIT.Phi ()) ;
        theSmallTree.m_bHMet_deltaPhi = deltaPhi (theBigTree.metphi, tlv_bH.Phi ()) ;
        theSmallTree.m_dib_deltaPhi = deltaPhi (tlv_firstBjet.Phi (), tlv_secondBjet.Phi ()) ;
        theSmallTree.m_dib_deltaEta = fabs(tlv_firstBjet.Eta()-tlv_secondBjet.Eta ()) ;
        theSmallTree.m_dib_deltaR = tlv_firstBjet.DeltaR(tlv_secondBjet) ;
        theSmallTree.m_dib_deltaR_per_bHpt = theSmallTree.m_dib_deltaR * tlv_bH_raw.Pt();
        theSmallTree.m_dib_dEtaSign = tlv_firstBjet.Eta() * tlv_secondBjet.Eta(); // VBF BDT

        theSmallTree.m_BDT_dib_deltaPhi     = ROOT::Math::VectorUtil::DeltaPhi(tlv_firstBjet, tlv_secondBjet);
        theSmallTree.m_BDT_dib_abs_deltaPhi = fabs(ROOT::Math::VectorUtil::DeltaPhi(tlv_firstBjet, tlv_secondBjet));

        vector <float> dRBTau;
        dRBTau.push_back (tlv_firstLepton.DeltaR(tlv_firstBjet));
        dRBTau.push_back (tlv_firstLepton.DeltaR(tlv_secondBjet));
        dRBTau.push_back (tlv_secondLepton.DeltaR(tlv_firstBjet));
        dRBTau.push_back (tlv_secondLepton.DeltaR(tlv_secondBjet));
        theSmallTree.m_btau_deltaRmin = *std::min_element(dRBTau.begin(), dRBTau.end());
        theSmallTree.m_btau_deltaRmax = *std::max_element(dRBTau.begin(), dRBTau.end());

        // Save other VBF related quantities
        if ( theBigTree.jets_px->size()> 1 && VBFcand_Mjj.size()>0 )
        {
          // save the VBF-jets
          TLorentzVector VBFjet1;
          VBFjet1.SetPxPyPzE( theBigTree.jets_px->at(VBFidx1), theBigTree.jets_py->at(VBFidx1), theBigTree.jets_pz->at(VBFidx1), theBigTree.jets_e->at(VBFidx1) );
          TLorentzVector VBFjet2;
          VBFjet2.SetPxPyPzE( theBigTree.jets_px->at(VBFidx2), theBigTree.jets_py->at(VBFidx2), theBigTree.jets_pz->at(VBFidx2), theBigTree.jets_e->at(VBFidx2) );

          bool hasgj1_VBF = false;
          bool hasgj2_VBF = false;

          if (isMC && theSmallTree.m_isVBFtrigger == 1)
          {
            double jetSF = getContentHisto3D(VBFjets_SF, std::get<0>(*(VBFcand_Mjj.rbegin())), VBFjet1.Pt(), VBFjet2.Pt());

            double SFTau1 = 1.;
            // !! FIXME !! to be updated for DeepTauV2p1
            if      (tlv_firstLepton.Pt() <  25) SFTau1 = 0.97;
            else if (tlv_firstLepton.Pt() <  30) SFTau1 = 0.755;
            else if (tlv_firstLepton.Pt() <  35) SFTau1 = 0.715;
            else if (tlv_firstLepton.Pt() <  40) SFTau1 = 0.75;
            else if (tlv_firstLepton.Pt() <  45) SFTau1 = 0.78;
            else if (tlv_firstLepton.Pt() <  50) SFTau1 = 0.835;
            else if (tlv_firstLepton.Pt() <  80) SFTau1 = 0.942;
            else if (tlv_firstLepton.Pt() < 200) SFTau1 = 0.91;
            else                                 SFTau1 = 0.97;

            double SFTau2 = 1.;
            // !! FIXME !! to be updated for DeepTauV2p1
            if      (tlv_secondLepton.Pt() <  25) SFTau2 = 0.97;
            else if (tlv_secondLepton.Pt() <  30) SFTau2 = 0.755;
            else if (tlv_secondLepton.Pt() <  35) SFTau2 = 0.715;
            else if (tlv_secondLepton.Pt() <  40) SFTau2 = 0.75;
            else if (tlv_secondLepton.Pt() <  45) SFTau2 = 0.78;
            else if (tlv_secondLepton.Pt() <  50) SFTau2 = 0.835;
            else if (tlv_secondLepton.Pt() <  80) SFTau2 = 0.942;
            else if (tlv_secondLepton.Pt() < 200) SFTau2 = 0.91;
            else                                  SFTau2 = 0.97;

            float VBFtrigNorm = 27.07/41.557;   // norm to RunD-F lumi: in 2017 VBF triggers online from RunD

            theSmallTree.m_VBFtrigSF = jetSF * SFTau1 * SFTau2 * VBFtrigNorm;
          }

          // Save gen info for VBF jets
          if (isMC)
          {
            int mcind = theBigTree.jets_genjetIndex->at(VBFidx1);
            if (mcind>=0)
            {
              TLorentzVector thisGenJet(theBigTree.genjet_px->at(mcind),theBigTree.genjet_py->at(mcind),theBigTree.genjet_pz->at(mcind),theBigTree.genjet_e->at(mcind));
              if (thisGenJet.Pt() > 8)
              {
                hasgj1_VBF = true;
                theSmallTree.m_VBFgenjet1_pt =  thisGenJet.Pt() ;
                theSmallTree.m_VBFgenjet1_eta = thisGenJet.Eta();
                theSmallTree.m_VBFgenjet1_phi = thisGenJet.Phi();
                theSmallTree.m_VBFgenjet1_e =   thisGenJet.E();
              }
            }
            mcind = theBigTree.jets_genjetIndex->at(VBFidx2);
            if (mcind>=0)
            {
              TLorentzVector thisGenJet(theBigTree.genjet_px->at(mcind),theBigTree.genjet_py->at(mcind),theBigTree.genjet_pz->at(mcind),theBigTree.genjet_e->at(mcind));
              if (thisGenJet.Pt() > 8)
              {
                hasgj2_VBF = true;
                theSmallTree.m_VBFgenjet2_pt =  thisGenJet.Pt() ;
                theSmallTree.m_VBFgenjet2_eta = thisGenJet.Eta();
                theSmallTree.m_VBFgenjet2_phi = thisGenJet.Phi();
                theSmallTree.m_VBFgenjet2_e =   thisGenJet.E();
              }
            }
          }

          // Save VBF variables
          theSmallTree.m_VBFjj_mass         = std::get<0>(*(VBFcand_Mjj.rbegin()));
          theSmallTree.m_VBFjj_mass_log     = log(std::get<0>(*(VBFcand_Mjj.rbegin())));
          theSmallTree.m_VBFjj_deltaEta     = fabs(VBFjet1.Eta()-VBFjet2.Eta());
          theSmallTree.m_VBFjj_HT           = VBFjet1.Pt()+VBFjet2.Pt();
          theSmallTree.m_VBFjj_dEtaSign = VBFjet1.Eta() * VBFjet2.Eta(); // VBF BDT
          theSmallTree.m_VBFjet1_pt         = VBFjet1.Pt() ;
          theSmallTree.m_VBFjet1_eta        = VBFjet1.Eta();
          theSmallTree.m_VBFjet1_phi        = VBFjet1.Phi();
          theSmallTree.m_VBFjet1_e          = VBFjet1.E();
          theSmallTree.m_VBFjet1_btag       = (theBigTree.bCSVscore->at (VBFidx1)) ;
          theSmallTree.m_VBFjet1_btag_deepCSV = theBigTree.bDeepCSV_probb->at(VBFidx1) + theBigTree.bDeepCSV_probbb->at(VBFidx1) ;
          theSmallTree.m_VBFjet1_btag_deepFlavor = theBigTree.bDeepFlavor_probb->at(VBFidx1) + theBigTree.bDeepFlavor_probbb->at(VBFidx1) + theBigTree.bDeepFlavor_problepb->at(VBFidx1);
          theSmallTree.m_VBFjet1_flav       = (theBigTree.jets_HadronFlavour->at (VBFidx1)) ;
          theSmallTree.m_VBFjet1_hasgenjet  = hasgj1_VBF ;
          theSmallTree.m_VBFjet2_pt         = VBFjet2.Pt() ;
          theSmallTree.m_VBFjet2_eta        = VBFjet2.Eta();
          theSmallTree.m_VBFjet2_phi        = VBFjet2.Phi();
          theSmallTree.m_VBFjet2_e          = VBFjet2.E();
          theSmallTree.m_VBFjet2_btag       = (theBigTree.bCSVscore->at (VBFidx2)) ;
          theSmallTree.m_VBFjet2_btag_deepCSV = theBigTree.bDeepCSV_probb->at(VBFidx2) + theBigTree.bDeepCSV_probbb->at(VBFidx2) ;
          theSmallTree.m_VBFjet2_btag_deepFlavor = theBigTree.bDeepFlavor_probb->at(VBFidx2) + theBigTree.bDeepFlavor_probbb->at(VBFidx2) + theBigTree.bDeepFlavor_problepb->at(VBFidx2);
          theSmallTree.m_VBFjet2_flav       = (theBigTree.jets_HadronFlavour->at (VBFidx2)) ;
          theSmallTree.m_VBFjet2_hasgenjet  = hasgj2_VBF ;
          theSmallTree.m_VBFjet2_PUjetID    = (theBigTree.jets_PUJetID->at (VBFidx2)); // VBF BDT
          theSmallTree.m_bH_VBF1_deltaEta   = fabs(tlv_bH.Eta() - VBFjet1.Eta()); // VBF BDT

          theSmallTree.m_dau1_z = getZ(tlv_firstLepton.Eta(),VBFjet1.Eta(),VBFjet2.Eta());
          theSmallTree.m_dau2_z = getZ(tlv_secondLepton.Eta(),VBFjet1.Eta(),VBFjet2.Eta());
          theSmallTree.m_bjet1_z = getZ(tlv_firstBjet.Eta(),VBFjet1.Eta(),VBFjet2.Eta());
          theSmallTree.m_bjet2_z = getZ(tlv_secondBjet.Eta(),VBFjet1.Eta(),VBFjet2.Eta());
          theSmallTree.m_tauH_z = getZ(tlv_tauH.Eta(),VBFjet1.Eta(),VBFjet2.Eta());
          theSmallTree.m_bH_z = getZ(tlv_bH.Eta(),VBFjet1.Eta(),VBFjet2.Eta());
          theSmallTree.m_HH_z = getZ(tlv_HH.Eta(),VBFjet1.Eta(),VBFjet2.Eta());

          //top mass calculation
          TLorentzVector VBFcentral = VBFjet1;
          TLorentzVector VBFforward = VBFjet2;

          if (fabs(VBFjet1.Eta())>fabs(VBFjet2.Eta())) std::swap(VBFcentral,VBFforward);
          TLorentzVector fakeTau = tlv_secondLepton;
          if (pairType == 2 && theSmallTree.m_dau1_iso > theSmallTree.m_dau2_iso)
          {
            fakeTau = tlv_firstLepton;
          }

          TLorentzVector Wc = VBFcentral + fakeTau;
          TLorentzVector Wf = VBFforward + fakeTau;
          TLorentzVector bclose = tlv_firstBjet;
          if(tlv_secondBjet.DeltaR(Wc)<tlv_firstBjet.DeltaR(Wc)) bclose = tlv_secondBjet;
          TLorentzVector top_Wc_bclose = Wc + bclose;
          bclose = tlv_firstBjet;
          if(tlv_secondBjet.DeltaR(Wf)<tlv_firstBjet.DeltaR(Wf)) bclose = tlv_secondBjet;
          TLorentzVector top_Wf_bclose = Wf + bclose;
          TLorentzVector bcentral = tlv_firstBjet;
          TLorentzVector bforward = tlv_secondBjet;
          if (fabs(tlv_firstBjet.Eta())>fabs(tlv_secondBjet.Eta())) std::swap(bcentral,bforward);
          TLorentzVector top_Wc_bcentral = Wc + bcentral;
          TLorentzVector top_Wf_bcentral = Wf + bcentral;
          TLorentzVector top_Wc_bforward = Wc + bforward;
          TLorentzVector top_Wf_bforward = Wf + bforward;

          float Wmass = 80.4;
          TLorentzVector W1 = VBFjet1 + fakeTau;
          TLorentzVector W2 = VBFjet2 + fakeTau;

          theSmallTree.m_top_Wc_bclose_mass = top_Wc_bclose.M();
          theSmallTree.m_top_Wc_bcentral_mass = top_Wc_bcentral.M();
          theSmallTree.m_top_Wc_bforward_mass = top_Wc_bforward.M();
          theSmallTree.m_top_Wf_bclose_mass = top_Wf_bclose.M();
          theSmallTree.m_top_Wf_bcentral_mass = top_Wf_bcentral.M();
          theSmallTree.m_top_Wf_bforward_mass = top_Wf_bforward.M();

          if (fabs(Wmass-W1.M()) > fabs(Wmass-W2.M())) std::swap(W1,W2);
          bclose = tlv_firstBjet;
          if (tlv_firstBjet.DeltaR(W1) > tlv_secondBjet.DeltaR(W1)) bclose = tlv_secondBjet;
          TLorentzVector top_Wmass_bclose = W1+ bclose;
          theSmallTree.m_top_Wmass_bclose_mass = top_Wmass_bclose.M();

          //boson centrality
          float DeltaEta_minus = std::min(tlv_tauH.Eta(), tlv_bH.Eta())- std::min(VBFjet1.Eta(), VBFjet2.Eta());
          float DeltaEta_plus =std::max(VBFjet1.Eta(), VBFjet2.Eta()) - std::max(tlv_tauH.Eta(), tlv_bH.Eta());
          float zV = std::min(DeltaEta_minus, DeltaEta_plus);
          theSmallTree.m_HH_zV = zV;

          //pT balance
          TVector3 v_tauH = tlv_tauH.Vect();
          TVector3 v_bH = tlv_bH.Vect();
          float HH_A = (v_tauH + v_bH).Mag()/(tlv_tauH.Pt() + tlv_bH.Pt());
          theSmallTree.m_HH_A = HH_A;
        }

        // loop over jets
        int genjets = 0;
        int jets  = 0;
        for (unsigned int iJet = 0; (iJet < theBigTree.jets_px->size ()) && (theSmallTree.m_njets < maxNjetsSaved); ++iJet)
        {
          // PG filter jets at will
          if (theBigTree.PFjetID->at (iJet) < PFjetID_WP) continue; // 0 ; don't pass PF Jet ID; 1: tight, 2: tightLepVeto

          // skip the H decay candiates
          if (int (iJet) == bjet1idx ){
            theSmallTree.m_bjet1_jecUnc = theBigTree.jets_jecUnc->at(iJet);
            continue;
          }else if(int (iJet) == bjet2idx){
            theSmallTree.m_bjet2_jecUnc = theBigTree.jets_jecUnc->at(iJet);
            continue ;
          }
          TLorentzVector tlv_dummyJet(
                                      theBigTree.jets_px->at (iJet),
                                      theBigTree.jets_py->at (iJet),
                                      theBigTree.jets_pz->at (iJet),
                                      theBigTree.jets_e->at (iJet)
                                     );

          // Apply PUjetID only to jets with pt < 50 GeV ( https://twiki.cern.ch/twiki/bin/view/CMS/HiggsToTauTauWorkingLegacyRun2#Jets )
          // PU jet ID WP = 2: loose
          if (PUjetID_WP > -1)
          {
            if ( !(CheckBit(theBigTree.jets_PUJetIDupdated_WP->at(iJet), PUjetID_WP)) && tlv_dummyJet.Pt()<50.) continue;
          }

          // Apply further cleaning for 2017 noisy jets, as suggested by HTT group: https://twiki.cern.ch/twiki/bin/view/CMS/HiggsToTauTauWorkingLegacyRun2#Jets
          // The noisy jets to be removed are defined as: 20 < pt < 50 && abs(eta) > 2.65 && abs(eta) < 3.139
          if ( tlv_dummyJet.Pt()<50. && fabs(tlv_dummyJet.Eta())>2.65 && fabs(tlv_dummyJet.Eta()<3.139) ) continue;

          // remove jets that overlap with the tau selected in the leg 1 and 2
          if (tlv_firstLepton.DeltaR(tlv_dummyJet) < lepCleaningCone){
            theSmallTree.m_dau1_jecUnc = theBigTree.jets_jecUnc->at(iJet);
            continue;
          }
          if (tlv_secondLepton.DeltaR(tlv_dummyJet) < lepCleaningCone){
            theSmallTree.m_dau2_jecUnc = theBigTree.jets_jecUnc->at(iJet);
            continue;
          }

          // find matching gen jet
          bool hasgj = false;
          if (isMC)
          {
            int mcind = theBigTree.jets_genjetIndex->at(iJet);
            if (mcind>=0)
            {
              TLorentzVector thisGenJet(theBigTree.genjet_px->at(mcind),theBigTree.genjet_py->at(mcind),theBigTree.genjet_pz->at(mcind),theBigTree.genjet_e->at(mcind));
              if (thisGenJet.Pt() > 8)
              {
                hasgj = true;
                if(genjets == 0)
                {
                  theSmallTree.m_genjet3_pt =  thisGenJet.Pt() ;
                  theSmallTree.m_genjet3_eta = thisGenJet.Eta();
                  theSmallTree.m_genjet3_phi = thisGenJet.Phi();
                  theSmallTree.m_genjet3_e =   thisGenJet.E();
                }else if(genjets == 1){
                  theSmallTree.m_genjet4_pt =  thisGenJet.Pt() ;
                  theSmallTree.m_genjet4_eta = thisGenJet.Eta();
                  theSmallTree.m_genjet4_phi = thisGenJet.Phi();
                  theSmallTree.m_genjet4_e =   thisGenJet.E();
                }
                genjets ++;
              }
            }
          }

          //if VBF, skip VBF jets candidates and save 5th jet
          if (VBFcand_Mjj.size()>0)
          {
            if(int (iJet) != VBFidx1 && int (iJet) != VBFidx2 and jets == 0)
            {
              theSmallTree.m_jet5_VBF_pt   = tlv_dummyJet.Pt() ;
              theSmallTree.m_jet5_VBF_eta  = tlv_dummyJet.Eta();
              theSmallTree.m_jet5_VBF_phi  = tlv_dummyJet.Phi();
              theSmallTree.m_jet5_VBF_e    = tlv_dummyJet.E();
              theSmallTree.m_jet5_VBF_btag = (theBigTree.bCSVscore->at (iJet)) ;
              theSmallTree.m_jet5_VBF_btag_deepCSV = (theBigTree.bDeepCSV_probb->at(iJet) + theBigTree.bDeepCSV_probbb->at(iJet)) ;
              theSmallTree.m_jet5_VBF_btag_deepFlavor = (theBigTree.bDeepFlavor_probb->at(iJet) + theBigTree.bDeepFlavor_probbb->at(iJet) + theBigTree.bDeepFlavor_problepb->at(iJet)) ;
              theSmallTree.m_jet5_VBF_flav = (theBigTree.jets_HadronFlavour->at (iJet)) ;
              theSmallTree.m_jet5_VBF_hasgenjet = hasgj ;
              theSmallTree.m_jet5_VBF_z = getZ(tlv_dummyJet.Eta(),theSmallTree.m_VBFjet1_eta,theSmallTree.m_VBFjet2_eta);
              if (hasgj)
              {
                int mcind = theBigTree.jets_genjetIndex->at(iJet);
                TLorentzVector thisGenJet(theBigTree.genjet_px->at(mcind),theBigTree.genjet_py->at(mcind),theBigTree.genjet_pz->at(mcind),theBigTree.genjet_e->at(mcind));
                theSmallTree.m_genjet5_VBF_pt =  thisGenJet.Pt() ;
                theSmallTree.m_genjet5_VBF_eta = thisGenJet.Eta();
                theSmallTree.m_genjet5_VBF_phi = thisGenJet.Phi();
                theSmallTree.m_genjet5_VBF_e =   thisGenJet.E();
              }
		      jets++;
            }
            if(int (iJet) != VBFidx1 && int (iJet) != VBFidx2) ++ theSmallTree.m_addjets;
	      }

          theSmallTree.m_jets_pt.push_back (tlv_dummyJet.Pt ()) ;
          theSmallTree.m_jets_eta.push_back (tlv_dummyJet.Eta ()) ;
          theSmallTree.m_jets_phi.push_back (tlv_dummyJet.Phi ()) ;
          theSmallTree.m_jets_e.push_back (theBigTree.jets_e->at (iJet)) ;
          theSmallTree.m_jets_btag.push_back (theBigTree.bCSVscore->at (iJet)) ;
          theSmallTree.m_jets_btag_deepCSV.push_back (theBigTree.bDeepCSV_probb->at(iJet) + theBigTree.bDeepCSV_probbb->at(iJet)) ;
          theSmallTree.m_jets_btag_deepFlavor.push_back (theBigTree.bDeepFlavor_probb->at(iJet) + theBigTree.bDeepFlavor_probbb->at(iJet) + theBigTree.bDeepFlavor_problepb->at(iJet)) ;
          theSmallTree.m_jets_flav.push_back (theBigTree.jets_HadronFlavour->at (iJet)) ;
          theSmallTree.m_jets_jecUnc.push_back (theBigTree.jets_jecUnc->at (iJet)) ;
          theSmallTree.m_jets_hasgenjet.push_back (hasgj) ;
          ++theSmallTree.m_njets ;
        } // loop over jets

        if (DEBUG)
        {
            cout << "-------- JETS DEBUG -------" << endl;
            cout << "b1 (pt,eta,flav, deepCSV): " << theSmallTree.m_bjet1_pt << "  " << theSmallTree.m_bjet1_eta << "  " << theSmallTree.m_bjet1_flav << "  " << theSmallTree.m_bjet1_bID_deepCSV << endl;
            cout << "b2 (pt,eta,flav, deepCSV): " << theSmallTree.m_bjet2_pt << "  " << theSmallTree.m_bjet2_eta << "  " << theSmallTree.m_bjet2_flav << "  " << theSmallTree.m_bjet2_bID_deepCSV << endl;
            cout << "Other jets:" << endl;
            for (unsigned int i=0; i<theSmallTree.m_jets_pt.size(); i++)
            {
                cout << "Jet " << i << " (pt,eta,flav, deepCSV): " << theSmallTree.m_jets_pt.at(i) << "  " << theSmallTree.m_jets_eta.at(i) << "  " << theSmallTree.m_jets_flav.at(i) << "  " << theSmallTree.m_jets_btag_deepCSV.at(i) << endl;
            }
            cout << "------------------------" << endl;
        }

        if (theSmallTree.m_jets_pt.size()>0)
        {
          theSmallTree.m_jet3_pt =theSmallTree.m_jets_pt.at(0);
          theSmallTree.m_jet3_eta =theSmallTree.m_jets_eta.at(0);
          theSmallTree.m_jet3_phi =theSmallTree.m_jets_phi.at(0);
          theSmallTree.m_jet3_e =theSmallTree.m_jets_e.at(0);
          theSmallTree.m_jet3_btag= theSmallTree.m_jets_btag.at (0);
          theSmallTree.m_jet3_btag_deepCSV= theSmallTree.m_jets_btag_deepCSV.at (0);
          theSmallTree.m_jet3_btag_deepFlavor= theSmallTree.m_jets_btag_deepFlavor.at (0);
          theSmallTree.m_jet3_flav= theSmallTree.m_jets_flav.at (0);
          theSmallTree.m_jet3_hasgenjet= theSmallTree.m_jets_hasgenjet.at (0);
        }
        if (theSmallTree.m_jets_pt.size()>1)
        {
          theSmallTree.m_jet4_pt =theSmallTree.m_jets_pt.at(1);
          theSmallTree.m_jet4_eta =theSmallTree.m_jets_eta.at(1);
          theSmallTree.m_jet4_phi =theSmallTree.m_jets_phi.at(1);
          theSmallTree.m_jet4_e =theSmallTree.m_jets_e.at(1);
          theSmallTree.m_jet4_btag= theSmallTree.m_jets_btag.at (1);
          theSmallTree.m_jet4_btag_deepCSV= theSmallTree.m_jets_btag_deepCSV.at (1);
          theSmallTree.m_jet4_btag_deepFlavor= theSmallTree.m_jets_btag_deepFlavor.at (1);
          theSmallTree.m_jet4_flav= theSmallTree.m_jets_flav.at (1);
          theSmallTree.m_jet4_hasgenjet= theSmallTree.m_jets_hasgenjet.at (1);
	  
          //invariant mass of first 2 additional jets (skipping H decay) ordered by Pt
          TLorentzVector tlv_jet1;
          TLorentzVector tlv_jet2;
          tlv_jet1.SetPtEtaPhiE(
                  theSmallTree.m_jets_pt.at(0),
                  theSmallTree.m_jets_eta.at(0),
                  theSmallTree.m_jets_phi.at(0),
                  theSmallTree.m_jets_e.at(0)
                  );
          tlv_jet2.SetPtEtaPhiE(
                  theSmallTree.m_jets_pt.at(1),
                  theSmallTree.m_jets_eta.at(1),
                  theSmallTree.m_jets_phi.at(1),
                  theSmallTree.m_jets_e.at(1)
                  );
          TLorentzVector tlv_jetPair = tlv_jet1 + tlv_jet2;

          theSmallTree.m_jj_mass = tlv_jetPair.M();
          theSmallTree.m_jj_mass_log = log(tlv_jetPair.M());
          theSmallTree.m_jj_deltaEta = fabs(tlv_jet1.Eta() - tlv_jet2.Eta());
          theSmallTree.m_jj_HT = tlv_jet1.Pt()+tlv_jet2.Pt();

          TLorentzVector b = tlv_firstBjet;
          if(tlv_secondBjet.Pt() > tlv_firstBjet.Pt()) b = tlv_secondBjet;
          TLorentzVector top_Wjj_b = b + tlv_jetPair; //2 highest pt additional jets + highest pt bjet
          theSmallTree.m_top_Wjj_b_mass = top_Wjj_b.M();

          TLorentzVector bclose = tlv_firstBjet;
          if(tlv_secondBjet.DeltaR(tlv_jetPair) < tlv_firstBjet.DeltaR(tlv_jetPair)) bclose =tlv_secondBjet;
          TLorentzVector top_Wjj_bclose = b + tlv_jetPair; //2 highest pt additional jets + closest bjet
          theSmallTree.m_top_Wjj_bclose_mass = top_Wjj_bclose.M();
        }
        if (theSmallTree.m_jets_pt.size()>2)
        {
          theSmallTree.m_jet5_pt =theSmallTree.m_jets_pt.at(2);
          theSmallTree.m_jet5_eta =theSmallTree.m_jets_eta.at(2);
          theSmallTree.m_jet5_phi =theSmallTree.m_jets_phi.at(2);
          theSmallTree.m_jet5_e =theSmallTree.m_jets_e.at(2);
          theSmallTree.m_jet5_btag= theSmallTree.m_jets_btag.at (2);
          theSmallTree.m_jet5_btag_deepCSV= theSmallTree.m_jets_btag_deepCSV.at (2);
          theSmallTree.m_jet5_btag_deepFlavor= theSmallTree.m_jets_btag_deepFlavor.at (2);
          theSmallTree.m_jet5_flav= theSmallTree.m_jets_flav.at (2);
          theSmallTree.m_jet5_hasgenjet= theSmallTree.m_jets_hasgenjet.at (2);
        }

      if (DEBUG)
      {
        cout << "--- VBF jets ---" << endl;
        cout << "isVBF: " << theSmallTree.m_isVBF << endl;
        cout << "VBF1(pt,eta,phi): " << theSmallTree.m_VBFjet1_pt << " / " << theSmallTree.m_VBFjet1_eta << " / " << theSmallTree.m_VBFjet1_phi << endl;
        cout << "VBF2(pt,eta,phi): " << theSmallTree.m_VBFjet2_pt << " / " << theSmallTree.m_VBFjet2_eta << " / " << theSmallTree.m_VBFjet2_phi << endl;
        cout << "----------------" << endl;
      }

      // Boosted section
	  theSmallTree.m_isBoosted = 0;
	  if (theBigTree.ak8jets_px->size() > 0)
	    {
	      // int idxSub1 = -1;
	      // int idxSub2 = -1;
	      // int idxFatj = -1;

	      vector<pair<float, int>> fatjets_bTag;
	      for (unsigned int ifj = 0; ifj < theBigTree.ak8jets_px->size(); ++ifj)
		{
		  TLorentzVector tlv_fj (theBigTree.ak8jets_px->at(ifj) , theBigTree.ak8jets_py->at(ifj) , theBigTree.ak8jets_pz->at(ifj) , theBigTree.ak8jets_e->at(ifj));
		  if (theBigTree.ak8jets_SoftDropMass -> at(ifj) < 30) continue;
		  if ( theBigTree.ak8jets_nsubjets->at(ifj) < 2 ) continue;
              
		  TLorentzVector tlv_subj1;
		  TLorentzVector tlv_subj2;
		  vector<int> sjIdxs = findSubjetIdxs(ifj, theBigTree);

		  int nSJ = 0;
		  for (int isj : sjIdxs)
		    {
		      ++nSJ;
		      if (nSJ > 2) break; // FIXME: storing first two <--> highest pt, order subjets for b tag?
		      if (nSJ == 1)
			{
			  tlv_subj1.SetPxPyPzE (theBigTree.subjets_px->at(isj), theBigTree.subjets_py->at(isj), theBigTree.subjets_pz->at(isj), theBigTree.subjets_e->at(isj));
			}
		      if (nSJ == 2)
			{
			  tlv_subj2.SetPxPyPzE (theBigTree.subjets_px->at(isj), theBigTree.subjets_py->at(isj), theBigTree.subjets_pz->at(isj), theBigTree.subjets_e->at(isj));
			}

		      if(DEBUG)
			{
			  cout << "- nSJ=" << nSJ << " px=" << theBigTree.subjets_px->at(isj) << endl;
			}

		    }

		  bool A1B2 = (tlv_subj1.DeltaR(tlv_firstBjet) < 0.4)   && (tlv_subj2.DeltaR(tlv_secondBjet) < 0.4 );
		  bool A2B1 = (tlv_subj1.DeltaR(tlv_secondBjet) < 0.4)  && (tlv_subj2.DeltaR(tlv_firstBjet) < 0.4 );

		  if(DEBUG)
		    {
		      cout << " fatjet: idx " << ifj << " nsj=" << sjIdxs.size() 
			   << " sj1pt=" << tlv_subj1.Pt() << " sj1eta=" << tlv_subj1.Eta() << " sj1phi=" << tlv_subj1.Phi()
			   << " sj2pt=" << tlv_subj2.Pt() << " sj2eta=" << tlv_subj2.Eta() << " sj2phi=" << tlv_subj2.Phi()
			   << " !passMatch=" << (!A1B2 && !A2B1) << endl;
		    }

		  if (!A1B2 && !A2B1) continue; // is not matched to resolved jets

		  //fatjets_bTag.push_back(make_pair(theBigTree.ak8jets_CSV->size(), ifj));
		  if(useDeepFlavor)
		  	fatjets_bTag.push_back(make_pair(theBigTree.ak8jets_deepFlavor_probb->at(ifj)+theBigTree.ak8jets_deepFlavor_probbb->at(ifj)+theBigTree.ak8jets_deepFlavor_problepb->at(ifj), ifj));
		  else
		  	fatjets_bTag.push_back(make_pair(theBigTree.ak8jets_deepCSV_probb->at(ifj)+theBigTree.ak8jets_deepCSV_probbb->at(ifj), ifj));
		}

	      if(DEBUG)
		{
		  cout << " N selected fatjets : " << fatjets_bTag.size() << endl;
		}

	      if (fatjets_bTag.size() != 0) 
		{
		  theSmallTree.m_isBoosted = 1;
		  sort (fatjets_bTag.begin(), fatjets_bTag.end());
		  int fjIdx = fatjets_bTag.back().second;
		  TLorentzVector tlv_fj (theBigTree.ak8jets_px->at(fjIdx) , theBigTree.ak8jets_py->at(fjIdx) , theBigTree.ak8jets_pz->at(fjIdx) , theBigTree.ak8jets_e->at(fjIdx));
		  theSmallTree.m_fatjet_pt   = tlv_fj.Pt();
		  theSmallTree.m_fatjet_eta  = tlv_fj.Eta();
		  theSmallTree.m_fatjet_phi  = tlv_fj.Phi();
		  theSmallTree.m_fatjet_e    = tlv_fj.E();
		  theSmallTree.m_fatjet_bID  = theBigTree.ak8jets_CSV->at(fjIdx);
		  theSmallTree.m_fatjet_bID_deepCSV  = theBigTree.ak8jets_deepCSV_probb->at(fjIdx) + theBigTree.ak8jets_deepCSV_probbb->at(fjIdx);
		  theSmallTree.m_fatjet_bID_deepFlavor  = theBigTree.ak8jets_deepFlavor_probb->at(fjIdx) + theBigTree.ak8jets_deepFlavor_probbb->at(fjIdx) + theBigTree.ak8jets_deepFlavor_problepb->at(fjIdx);
		  theSmallTree.m_fatjet_filteredMass = theBigTree.ak8jets_FilteredMass -> at(fjIdx) ;
		  theSmallTree.m_fatjet_prunedMass   = theBigTree.ak8jets_PrunedMass   -> at(fjIdx) ;
		  theSmallTree.m_fatjet_trimmedMass  = theBigTree.ak8jets_TrimmedMass  -> at(fjIdx) ;
		  theSmallTree.m_fatjet_softdropMass = theBigTree.ak8jets_SoftDropMass -> at(fjIdx) ;
		  theSmallTree.m_fatjet_tau1 = theBigTree.ak8jets_tau1->at(fjIdx);
		  theSmallTree.m_fatjet_tau2 = theBigTree.ak8jets_tau2->at(fjIdx);
		  theSmallTree.m_fatjet_tau3 = theBigTree.ak8jets_tau3->at(fjIdx);
		  theSmallTree.m_fatjet_nsubjets = theBigTree.ak8jets_nsubjets->at(fjIdx);
		  // FIXME: redoing this a second time, can be optimized
		  if ( theBigTree.ak8jets_nsubjets->at(fjIdx) < 2) cout << "ERROR: there are not 2 subjets. Should not happen!!" << endl;
		  TLorentzVector tlv_subj1;
		  TLorentzVector tlv_subj2;
		  vector<int> sjIdxs = findSubjetIdxs(fjIdx, theBigTree);
		  int nSJ = 0;
		  for (int isj : sjIdxs)
		    {
		      ++nSJ;
		      if (nSJ > 2) break; // FIXME: storing first two <--> highest pt, order subjets for b tag?
		      if (nSJ == 1)
			{
			  tlv_subj1.SetPxPyPzE (theBigTree.subjets_px->at(isj), theBigTree.subjets_py->at(isj), theBigTree.subjets_pz->at(isj), theBigTree.subjets_e->at(isj));
			  theSmallTree.m_subjetjet1_pt   = tlv_subj1.Pt();
			  theSmallTree.m_subjetjet1_eta  = tlv_subj1.Eta();
			  theSmallTree.m_subjetjet1_phi  = tlv_subj1.Phi();
			  theSmallTree.m_subjetjet1_e    = tlv_subj1.E();
			  theSmallTree.m_subjetjet1_bID  = theBigTree.subjets_CSV->at(isj) ;
			  theSmallTree.m_subjetjet1_bID_deepCSV  = theBigTree.subjets_deepCSV_probb->at(isj) + theBigTree.subjets_deepCSV_probbb->at(isj) ;
			  theSmallTree.m_subjetjet1_bID_deepFlavor  = theBigTree.subjets_deepFlavor_probb->at(isj) + theBigTree.subjets_deepFlavor_probbb->at(isj) + theBigTree.subjets_deepFlavor_problepb->at(isj) ;
			}
		      if (nSJ == 2)
			{
			  tlv_subj2.SetPxPyPzE (theBigTree.subjets_px->at(isj), theBigTree.subjets_py->at(isj), theBigTree.subjets_pz->at(isj), theBigTree.subjets_e->at(isj));
			  theSmallTree.m_subjetjet2_pt   = tlv_subj2.Pt();
			  theSmallTree.m_subjetjet2_eta  = tlv_subj2.Eta();
			  theSmallTree.m_subjetjet2_phi  = tlv_subj2.Phi();
			  theSmallTree.m_subjetjet2_e    = tlv_subj2.E();
			  theSmallTree.m_subjetjet2_bID  = theBigTree.subjets_CSV->at(isj) ;
			  theSmallTree.m_subjetjet2_bID_deepCSV  = theBigTree.subjets_deepCSV_probb->at(isj) + theBigTree.subjets_deepCSV_probbb->at(isj) ;
			  theSmallTree.m_subjetjet2_bID_deepFlavor  = theBigTree.subjets_deepFlavor_probb->at(isj) + theBigTree.subjets_deepFlavor_probbb->at(isj) + theBigTree.subjets_deepFlavor_problepb->at(isj);
			}
		      theSmallTree.m_dR_subj1_subj2 = tlv_subj1.DeltaR(tlv_subj2);
		    } 
		}
	    }
	}// if there's two jets in the event, at least

      if (isMC) selectedEvents += theBigTree.aMCatNLOweight ;  //FIXME: probably wrong, but unused up to now
      else selectedEvents += 1 ;
      ++selectedNoWeightsEventsNum ;

      if (DEBUG) //FRA DEBUG
      {
        cout << "--------------" << endl;
        cout << " - Debug SFs -" << endl;
        cout << "  PU           : " << theSmallTree.m_PUReweight << endl;
        cout << "  IDandISO MVA : " << theSmallTree.m_IdAndIsoSF_MVA << endl;
        cout << "    w/ FakeRate: " << theSmallTree.m_IdAndIsoAndFakeSF_MVA << endl;
        cout << "  IDandISO deep: " << theSmallTree.m_IdAndIsoSF_deep << endl;
        cout << "    w/ FakeRate: " << theSmallTree.m_IdAndIsoAndFakeSF_deep << endl;
        cout << "  trig         : " << theSmallTree.m_trigSF << endl;
        cout << "  bTag         : " << theSmallTree.m_bTagweightM << endl;
        cout << "--------------" << endl;
      }

      if (DEBUG) //FRA DEBUG
      {
          cout << "--- DEBUG MC weights ---" << endl;
          cout << "aMCatNLOweight: " << theBigTree.aMCatNLOweight << endl;
          cout << "XS            : " << XS << endl;
          cout << "stitchWeight  : " << stitchWeight << endl;
          cout << "HHweight      : " << HHweight << endl;
          cout << "MC_weight     : " << theSmallTree.m_MC_weight << endl;
          cout << "Yield weight MVA : " << theSmallTree.m_MC_weight * theSmallTree.m_PUReweight * theSmallTree.m_IdAndIsoAndFakeSF_MVA * theSmallTree.m_trigSF << endl;
          cout << "Yield weight deep: " << theSmallTree.m_MC_weight * theSmallTree.m_PUReweight * theSmallTree.m_IdAndIsoAndFakeSF_deep * theSmallTree.m_trigSF << endl;
          cout << "------------------------" << endl;
      }

      theSmallTree.Fill () ;
    }

  cout << "1: " << totalEvents << endl ;
  cout << "2: " << selectedEvents << endl ;
  cout << "3: " << totalNoWeightsEventsNum << endl ;
  cout << "4: " << selectedNoWeightsEventsNum << endl ;

  if (totalEvents != 0) cout << "efficiency = " << selectedEvents / totalEvents << endl ;
  else                  cout << "NO events found\n" ;
  TH1F h_eff ("h_eff", "h_eff", 4 , 0, 4) ;
  h_eff.SetBinContent (1, totalEvents) ;
  h_eff.SetBinContent (2, selectedEvents) ;
  h_eff.SetBinContent (3, totalNoWeightsEventsNum) ;
  h_eff.SetBinContent (4, selectedNoWeightsEventsNum) ;
  
  //cout << " DY_tot : " << DY_tot << endl;
  //cout << " DY_pass: " << DY_pass << endl;
  //cout << " DY fail: " << DY_fail << endl;
  //cout << " % pass : " << DY_pass*100./DY_tot << endl;
  //cout << " % fail : " << DY_fail*100./DY_tot << endl;

  // store more detailed eff counter in output
  vector<pair<string, double> > vEffSumm = ec.GetSummary();
  TH1F* h_effSummary = new TH1F ("h_effSummary", "h_effSummary", vEffSumm.size(), 0, vEffSumm.size());
  for (uint isumm = 0; isumm < vEffSumm.size(); ++isumm)
    {
      h_effSummary->SetBinContent(isumm+1, vEffSumm.at(isumm).second);
      h_effSummary->GetXaxis()->SetBinLabel(isumm+1, vEffSumm.at(isumm).first.c_str());
    }

  TH1F* hEffHHSigsSummary [6];
  if (isHHsignal)
    {
      std::vector<string> vNames = {
	"MuTau",
	"ETau",
	"TauTau",
	"MuMu",
	"EE",
	"EMu"
      };
    
    
      for (uint ich = 0; ich < 6; ++ich)
	{
	  string hname = string("h_effSummary_") + vNames.at(ich);
	  vector<pair<string, double> > vEffSummHH = ecHHsig[ich].GetSummary();
	  hEffHHSigsSummary[ich] = new TH1F (hname.c_str(), hname.c_str(), vEffSummHH.size(), 0, vEffSummHH.size());
	  for (uint isumm = 0; isumm < vEffSummHH.size(); ++isumm)
	    {
	      hEffHHSigsSummary[ich]->SetBinContent(isumm+1, vEffSummHH.at(isumm).second);
	      hEffHHSigsSummary[ich]->GetXaxis()->SetBinLabel(isumm+1, vEffSummHH.at(isumm).first.c_str());
	    }
	}

    }

  // for (unsigned int i = 0 ; i < counter.size () ; ++i)
  //   h_eff.SetBinContent (5 + i, counter.at (i)) ;

  smallFile->cd() ;
  h_eff.Write () ;
  h_effSummary->Write() ;
  if (isHHsignal)
    {
      for (uint ich = 0; ich < 6; ++ich)
	hEffHHSigsSummary[ich]->Write();
    }

  smallFile->Write () ;
  smallFile->Close () ;

  // free memory used by histos for eff
  delete h_effSummary;
  
  if (isHHsignal)
    {
      for (uint ich = 0; ich < 6; ++ich)
        delete hEffHHSigsSummary[ich];
    }



  bool computeMVA    = (gConfigParser->isDefined("TMVA::computeMVA")        ? gConfigParser->readBoolOption ("TMVA::computeMVA")        : false);
  bool computeMVARes = (gConfigParser->isDefined("BDTResonant::computeMVA") ? gConfigParser->readBoolOption ("BDTResonant::computeMVA") : false);
  bool computeMVAResHM = (gConfigParser->isDefined("BDTResonantHM::computeMVA") ? gConfigParser->readBoolOption ("BDTResonantHM::computeMVA") : false);
  bool computeMVAResLM = (gConfigParser->isDefined("BDTResonantLM::computeMVA") ? gConfigParser->readBoolOption ("BDTResonantLM::computeMVA") : false);
  bool computeMVANonRes = (gConfigParser->isDefined("BDTNonResonant::computeMVA") ? gConfigParser->readBoolOption ("BDTNonResonant::computeMVA") : false);

  if (computeMVA || computeMVARes || computeMVAResHM || computeMVAResLM)
    {  
      bool doMuTau  = gConfigParser->isDefined("TMVA::weightsMuTau");
      bool doETau   = gConfigParser->isDefined("TMVA::weightsETau");
      bool doTauTau = gConfigParser->isDefined("TMVA::weightsTauTau");
      bool doLepTau = gConfigParser->isDefined("TMVA::weightsLepTau");
      bool doResonant = computeMVARes;
      bool doResonantHM = computeMVAResHM;
      bool doResonantLM = computeMVAResLM;
      bool doNonResonant = computeMVANonRes;

      string TMVAweightsTauTau   = "";
      string TMVAweightsMuTau    = "";
      string TMVAweightsETau     = "";
      string TMVAweightsLepTau   = "";
      string TMVAweightsResonant = "";
      string TMVAweightsResonantHM = "";
      string TMVAweightsResonantLM = "";
      string TMVAweightsNonResonant = "";
    
      if (doMuTau)    TMVAweightsMuTau  = gConfigParser->readStringOption ("TMVA::weightsMuTau");
      if (doETau)     TMVAweightsETau   = gConfigParser->readStringOption ("TMVA::weightsETau");
      if (doTauTau)   TMVAweightsTauTau = gConfigParser->readStringOption ("TMVA::weightsTauTau");
      if (doLepTau)   TMVAweightsLepTau = gConfigParser->readStringOption ("TMVA::weightsLepTau");
      if (doResonant) TMVAweightsResonant = gConfigParser->readStringOption ("BDTResonant::weights");
      if (doResonantHM) TMVAweightsResonantHM = gConfigParser->readStringOption ("BDTResonantHM::weights");
      if (doResonantLM) TMVAweightsResonantLM = gConfigParser->readStringOption ("BDTResonantLM::weights");
      if (doNonResonant) TMVAweightsNonResonant = gConfigParser->readStringOption ("BDTNonResonant::weights");

      // bool TMVAspectatorsIn      = gConfigParser->readBoolOption   ("TMVA::spectatorsPresent");
      vector<string> TMVAspectators = ( computeMVA ? gConfigParser->readStringListOption   ("TMVA::spectators") : vector<string>(0) );
      vector<string> TMVAvariables  = ( computeMVA ? gConfigParser->readStringListOption   ("TMVA::variables") : vector<string>(0) );
      vector<string> TMVAvariablesResonant   = ( doResonant ? gConfigParser->readStringListOption   ("BDTResonant::variables") : vector<string>(0) );
      vector<string> TMVAvariablesResonantHM = ( doResonantHM ? gConfigParser->readStringListOption   ("BDTResonantHM::variables") : vector<string>(0) );
      vector<string> TMVAvariablesResonantLM = ( doResonantLM ? gConfigParser->readStringListOption   ("BDTResonantLM::variables") : vector<string>(0) );
      vector<string> TMVAvariablesNonResonant = ( doNonResonant ? gConfigParser->readStringListOption   ("BDTNonResonant::variables") : vector<string>(0) );

      // split the resonant name in two strings
      vector<pair<string, string>> splitTMVAvariablesResonant;
      for (unsigned int iv = 0 ; iv < TMVAvariablesResonant.size () ; ++iv)
	{
	  // split my_name:BDT_name in two strings
	  std::stringstream packedName(TMVAvariablesResonant.at(iv));
	  std::string segment;
	  std::vector<std::string> unpackedNames;
	  while(std::getline(packedName, segment, ':'))
	    unpackedNames.push_back(segment);

	  splitTMVAvariablesResonant.push_back(make_pair(unpackedNames.at(0), unpackedNames.at(1))); 
	} 

      // split the resonant name in two strings
      cout << "BDT resonant HIGH MASS vars:" << endl;
      vector<pair<string, string>> splitTMVAvariablesResonantHM;
      for (unsigned int iv = 0 ; iv < TMVAvariablesResonantHM.size () ; ++iv)
	{
	  // split my_name:BDT_name in two strings
	  std::stringstream packedName(TMVAvariablesResonantHM.at(iv));
	  std::string segment;
	  std::vector<std::string> unpackedNames;
	  while(std::getline(packedName, segment, ':'))
	    unpackedNames.push_back(segment);

	  // replace "internal" names for graphics names -- shitty parser!!
	  boost::replace_all(unpackedNames.at(1), "_T_", "*");
	  boost::replace_all(unpackedNames.at(1), "__", "()");

	  splitTMVAvariablesResonantHM.push_back(make_pair(unpackedNames.at(0), unpackedNames.at(1))); 
	  cout << " ... " << iv << " " << unpackedNames.at(0) << " --> " << unpackedNames.at(1) << endl;
	} 
      cout << endl;

      // split the resonant name in two strings
      vector<pair<string, string>> splitTMVAvariablesResonantLM;
      cout << "BDT resonant LOW MASS vars:" << endl;
      for (unsigned int iv = 0 ; iv < TMVAvariablesResonantLM.size () ; ++iv)
	{
	  // split my_name:BDT_name in two strings
	  std::stringstream packedName(TMVAvariablesResonantLM.at(iv));
	  std::string segment;
	  std::vector<std::string> unpackedNames;
	  while(std::getline(packedName, segment, ':'))
	    unpackedNames.push_back(segment);

	  // replace "internal" names for graphics names -- shitty parser!!
	  boost::replace_all(unpackedNames.at(1), "_T_", "*");
	  boost::replace_all(unpackedNames.at(1), "__", "()");

	  splitTMVAvariablesResonantLM.push_back(make_pair(unpackedNames.at(0), unpackedNames.at(1))); 
	  cout << " ... " << iv << " " << unpackedNames.at(0) << " --> " << unpackedNames.at(1) << endl;
	} 

      // split the non resonant name in two strings
      vector<pair<string, string>> splitTMVAvariablesNonResonant;
      cout << "BDT non resonant vars:" << endl;
      for (unsigned int iv = 0 ; iv < TMVAvariablesNonResonant.size () ; ++iv)
	{
	  // split my_name:BDT_name in two strings
	  std::stringstream packedName(TMVAvariablesNonResonant.at(iv));
	  std::string segment;
	  std::vector<std::string> unpackedNames;
	  while(std::getline(packedName, segment, ':'))
	    unpackedNames.push_back(segment);

	  // replace "internal" names for graphics names -- shitty parser!!
	  boost::replace_all(unpackedNames.at(1), "_T_", "*");
	  boost::replace_all(unpackedNames.at(1), "__", "()");

	  splitTMVAvariablesNonResonant.push_back(make_pair(unpackedNames.at(0), unpackedNames.at(1))); 
	  cout << " ... " << iv << " " << unpackedNames.at(0) << " --> " << unpackedNames.at(1) << endl;
	}


      // now merge all names into a vector to get a list of uniquely needed elements
      std::vector<string> allVars;
      allVars.insert(allVars.end(), TMVAspectators.begin(), TMVAspectators.end());
      allVars.insert(allVars.end(), TMVAvariables.begin(), TMVAvariables.end());
      for (unsigned int iv = 0; iv < splitTMVAvariablesResonant.size(); ++iv)
        allVars.push_back(splitTMVAvariablesResonant.at(iv).first);
      for (unsigned int iv = 0; iv < splitTMVAvariablesResonantHM.size(); ++iv)
        allVars.push_back(splitTMVAvariablesResonantHM.at(iv).first);
      for (unsigned int iv = 0; iv < splitTMVAvariablesResonantLM.size(); ++iv)
        allVars.push_back(splitTMVAvariablesResonantLM.at(iv).first);
      for (unsigned int iv = 0; iv < splitTMVAvariablesNonResonant.size(); ++iv)
        allVars.push_back(splitTMVAvariablesNonResonant.at(iv).first);

      sort(allVars.begin(), allVars.end());
      allVars.erase( unique( allVars.begin(), allVars.end() ), allVars.end() );
      std::map<string, float> allVarsMap;
      for (string var : allVars)
	allVarsMap[var] = 0.0;

      TFile *outFile = TFile::Open(outputFile,"UPDATE");
      TTree *treenew = (TTree*)outFile->Get("HTauTauTree");

      TMVA::Reader * reader = new TMVA::Reader () ;
      TMVA::Reader * readerResonant = new TMVA::Reader () ;
      TMVA::Reader * readerResonantHM = new TMVA::Reader () ;
      TMVA::Reader * readerResonantLM = new TMVA::Reader () ;
      TMVA::Reader * readerNonResonant = new TMVA::Reader () ;
      Float_t mvatautau,mvamutau, mvaetau, mvaleptau, mvaresonant, mvaresonantHM, mvaresonantLM, mvanonresonant;
      TBranch *mvaBranchmutau;
      TBranch *mvaBranchtautau;
      TBranch *mvaBranchetau;
      TBranch *mvaBranchleptau;
      TBranch *mvaBranchResonant;
      TBranch *mvaBranchResonantHM;
      TBranch *mvaBranchResonantLM;
      TBranch *mvaBranchNonResonant;

      for (string var : TMVAvariables)
	{
	  treenew->SetBranchAddress (var.c_str (), &(allVarsMap.at (var))) ;
	  reader->AddVariable (var, &(allVarsMap.at (var))) ;
	}  

      for (string var : TMVAspectators)
	{
	  treenew->SetBranchAddress (var.c_str (), &(allVarsMap.at (var))) ;
	  reader->AddSpectator (var, &(allVarsMap.at (var))) ;
	}  

      for (pair<string, string> vpair : splitTMVAvariablesResonant)
	{
	  treenew->SetBranchAddress (vpair.first.c_str (), &(allVarsMap.at (vpair.first))) ;
	  readerResonant->AddVariable (vpair.second.c_str (), &(allVarsMap.at (vpair.first))) ;      
	}

      for (pair<string, string> vpair : splitTMVAvariablesResonantHM)
	{
	  treenew->SetBranchAddress (vpair.first.c_str (), &(allVarsMap.at (vpair.first))) ;
	  readerResonantHM->AddVariable (vpair.second.c_str (), &(allVarsMap.at (vpair.first))) ;      
	  // cout << "DEBUG HM: " << vpair.second.c_str () <<  " <-- " << vpair.first.c_str () << endl;
	}

      for (pair<string, string> vpair : splitTMVAvariablesResonantLM)
	{
	  treenew->SetBranchAddress (vpair.first.c_str (), &(allVarsMap.at (vpair.first))) ;
	  readerResonantLM->AddVariable (vpair.second.c_str (), &(allVarsMap.at (vpair.first))) ;      
	}

      for (pair<string, string> vpair : splitTMVAvariablesNonResonant)
	{
	  treenew->SetBranchAddress (vpair.first.c_str (), &(allVarsMap.at (vpair.first))) ;
	  readerNonResonant->AddVariable (vpair.second.c_str (), &(allVarsMap.at (vpair.first))) ;      
	}

      if (doMuTau)  mvaBranchmutau = treenew->Branch ("MuTauKine", &mvamutau, "MuTauKine/F") ;
      if (doETau)   mvaBranchetau = treenew->Branch ("ETauKine", &mvaetau, "ETauKine/F") ;
      if (doTauTau) mvaBranchtautau = treenew->Branch ("TauTauKine", &mvatautau, "TauTauKine/F") ;
      if (doLepTau) mvaBranchleptau = treenew->Branch ("LepTauKine", &mvaleptau, "LepTauKine/F") ;
      if (doResonant) mvaBranchResonant = treenew->Branch ("BDTResonant", &mvaresonant, "BDTResonant/F") ;
      if (doResonantHM) mvaBranchResonantHM = treenew->Branch ("BDTResonantHM", &mvaresonantHM, "BDTResonantHM/F") ;
      if (doResonantLM) mvaBranchResonantLM = treenew->Branch ("BDTResonantLM", &mvaresonantLM, "BDTResonantLM/F") ;
      if (doNonResonant) mvaBranchNonResonant = treenew->Branch ("BDTNonResonant", &mvanonresonant, "BDTNonResonant/F") ;
      //}
      if (doMuTau)   reader->BookMVA ("MuTauKine",  TMVAweightsMuTau.c_str ()) ;
      if (doETau)    reader->BookMVA ("ETauKine",  TMVAweightsETau.c_str ()) ;
      if (doTauTau)  reader->BookMVA ("TauTauKine",  TMVAweightsTauTau.c_str ()) ;
      if (doLepTau)  reader->BookMVA ("LepTauKine",  TMVAweightsLepTau.c_str ()) ;
      if (doResonant)  readerResonant->BookMVA ("BDT_full_mass_iso_nodrbbsv",  TMVAweightsResonant.c_str ()) ;
      if (doResonantHM)  readerResonantHM->BookMVA ("500t_PU_mass_newvars_HIGH_oldvars",  TMVAweightsResonantHM.c_str ()) ;
      if (doResonantLM)  readerResonantLM->BookMVA ("500t_PU_mass_newvars_LOW",  TMVAweightsResonantLM.c_str ()) ;
      if (doNonResonant)  readerNonResonant->BookMVA ("BDT_nonres_SM",  TMVAweightsNonResonant.c_str ()) ;

      int nentries = treenew->GetEntries();
      for(int i=0;i<nentries;i++){
	treenew->GetEntry(i);

	if (doMuTau)   mvamutau= reader->EvaluateMVA ("MuTauKine") ;  
	if (doETau)    mvaetau= reader->EvaluateMVA ("ETauKine") ;  
	if (doTauTau)  mvatautau= reader->EvaluateMVA ("TauTauKine") ;  
	if (doLepTau)  mvaleptau= reader->EvaluateMVA ("LepTauKine") ;  
	if (doResonant)  mvaresonant= readerResonant->EvaluateMVA ("BDT_full_mass_iso_nodrbbsv") ;  
	if (doResonantHM)  mvaresonantHM= readerResonantHM->EvaluateMVA ("500t_PU_mass_newvars_HIGH_oldvars") ;  
	if (doResonantLM)  mvaresonantLM= readerResonantLM->EvaluateMVA ("500t_PU_mass_newvars_LOW") ;  
	if (doNonResonant)  mvanonresonant= readerNonResonant->EvaluateMVA ("BDT_nonres_SM") ;  

	if (doMuTau)    mvaBranchmutau->Fill();
	if (doETau)     mvaBranchetau->Fill();
	if (doTauTau)   mvaBranchtautau->Fill();
	if (doLepTau)   mvaBranchleptau->Fill();
	if (doResonant)  mvaBranchResonant->Fill();
	if (doResonantHM)  mvaBranchResonantHM->Fill();
	if (doResonantLM)  mvaBranchResonantLM->Fill();
	if (doNonResonant)  mvaBranchNonResonant->Fill();
      }

      outFile->cd () ;
      h_eff.Write () ;
      treenew->Write ("", TObject::kOverwrite) ;

      delete reader;
      delete readerResonant;
      delete readerResonantHM;
      delete readerResonantLM;
      delete readerNonResonant;
    }


  // NEW BDT
  bool computeBDTsm = (gConfigParser->isDefined("BDTsm::computeMVA") ? gConfigParser->readBoolOption ("BDTsm::computeMVA") : false);
  bool computeBDTlm = (gConfigParser->isDefined("BDTlm::computeMVA") ? gConfigParser->readBoolOption ("BDTlm::computeMVA") : false);
  bool computeBDTmm = (gConfigParser->isDefined("BDTmm::computeMVA") ? gConfigParser->readBoolOption ("BDTmm::computeMVA") : false);
  bool computeBDThm = (gConfigParser->isDefined("BDThm::computeMVA") ? gConfigParser->readBoolOption ("BDThm::computeMVA") : false);
  bool computeVBFBDT = (gConfigParser->isDefined("BDTVBF::computeMVA") ? gConfigParser->readBoolOption ("BDTVBF::computeMVA") : false);

  if (computeBDTsm || computeBDTlm || computeBDTmm || computeBDThm || computeVBFBDT)
  {
    cout << " ------------ ############### ----- NEW BDT ----- ############### ------------ " <<endl;

    bool doSM = computeBDTsm;
    bool doLM = computeBDTlm;
    bool doMM = computeBDTmm;
    bool doHM = computeBDThm;
    bool doVBF = computeVBFBDT;

    // weights file
    string TMVAweightsSM = "";
    string TMVAweightsLM = "";
    string TMVAweightsMM = "";
    string TMVAweightsHM = "";
    string TMVAweightsVBF = "";
    vector<float> SM_kl;
    vector<float> LM_mass;
    vector<float> MM_mass;
    vector<float> HM_mass;
    vector<int> LM_spin;
    vector<int> MM_spin;
    vector<int> HM_spin;

    if (doSM)
    {
      TMVAweightsSM = gConfigParser->readStringOption ("BDTsm::weights");
      SM_kl         = gConfigParser->readFloatListOption("BDTsm::kl");
    }
    if (doLM)
    {
      TMVAweightsLM = gConfigParser->readStringOption ("BDTlm::weights");
      LM_mass       = gConfigParser->readFloatListOption ("BDTlm::mass");
      LM_spin       = gConfigParser->readIntListOption ("BDTlm::spin");
    }
    if (doMM)
    {
      TMVAweightsMM = gConfigParser->readStringOption ("BDTmm::weights");
      MM_mass       = gConfigParser->readFloatListOption ("BDTmm::mass");
      MM_spin       = gConfigParser->readIntListOption ("BDTmm::spin");
    }
    if (doHM)
    {
      TMVAweightsHM = gConfigParser->readStringOption ("BDThm::weights");
      HM_mass       = gConfigParser->readFloatListOption ("BDThm::mass");
      HM_spin       = gConfigParser->readIntListOption ("BDThm::spin");
    }
    if (doVBF)
    {
      TMVAweightsVBF = gConfigParser->readStringOption ("BDTVBF::weights");
    }

    // Input variables
    vector<string> TMVAvariablesSM = ( doSM ? gConfigParser->readStringListOption ("BDTsm::variables") : vector<string>(0) );
    vector<string> TMVAvariablesLM = ( doLM ? gConfigParser->readStringListOption ("BDTlm::variables") : vector<string>(0) );
    vector<string> TMVAvariablesMM = ( doMM ? gConfigParser->readStringListOption ("BDTmm::variables") : vector<string>(0) );
    vector<string> TMVAvariablesHM = ( doHM ? gConfigParser->readStringListOption ("BDThm::variables") : vector<string>(0) );
    vector<string> TMVAvariablesVBF = ( doVBF ? gConfigParser->readStringListOption ("BDTVBF::variables") : vector<string>(0) );

    // Split the resonant name in two strings
    vector<pair<string, string>> splitTMVAvariablesSM;
    for (unsigned int iv = 0 ; iv < TMVAvariablesSM.size () ; ++iv)
    {
      // Split my_name:BDT_name in two strings
      std::stringstream packedName(TMVAvariablesSM.at(iv));
      std::string segment;
      std::vector<std::string> unpackedNames;
      while(std::getline(packedName, segment, ':'))
        unpackedNames.push_back(segment);

      splitTMVAvariablesSM.push_back(make_pair(unpackedNames.at(0), unpackedNames.at(1)));
    }
    
    vector<pair<string, string>> splitTMVAvariablesLM;
    for (unsigned int iv = 0 ; iv < TMVAvariablesLM.size () ; ++iv)
    {
      // Split my_name:BDT_name in two strings
      std::stringstream packedName(TMVAvariablesLM.at(iv));
      std::string segment;
      std::vector<std::string> unpackedNames;
      while(std::getline(packedName, segment, ':'))
        unpackedNames.push_back(segment);

      splitTMVAvariablesLM.push_back(make_pair(unpackedNames.at(0), unpackedNames.at(1)));
    }

    vector<pair<string, string>> splitTMVAvariablesMM;
    for (unsigned int iv = 0 ; iv < TMVAvariablesMM.size () ; ++iv)
    {
      // Split my_name:BDT_name in two strings
      std::stringstream packedName(TMVAvariablesMM.at(iv));
      std::string segment;
      std::vector<std::string> unpackedNames;
      while(std::getline(packedName, segment, ':'))
        unpackedNames.push_back(segment);

      splitTMVAvariablesMM.push_back(make_pair(unpackedNames.at(0), unpackedNames.at(1)));
    }

    vector<pair<string, string>> splitTMVAvariablesHM;
    for (unsigned int iv = 0 ; iv < TMVAvariablesHM.size () ; ++iv)
    {
      // Split my_name:BDT_name in two strings
      std::stringstream packedName(TMVAvariablesHM.at(iv));
      std::string segment;
      std::vector<std::string> unpackedNames;
      while(std::getline(packedName, segment, ':'))
        unpackedNames.push_back(segment);

      splitTMVAvariablesHM.push_back(make_pair(unpackedNames.at(0), unpackedNames.at(1)));
    }

    vector<pair<string, string>> splitTMVAvariablesVBF;
    for (unsigned int iv = 0 ; iv < TMVAvariablesVBF.size () ; ++iv)
    {
      // Split my_name:BDT_name in two strings
      std::stringstream packedName(TMVAvariablesVBF.at(iv));
      std::string segment;
      std::vector<std::string> unpackedNames;
      while(std::getline(packedName, segment, ':'))
        unpackedNames.push_back(segment);

      splitTMVAvariablesVBF.push_back(make_pair(unpackedNames.at(0), unpackedNames.at(1)));
    }

    // Now merge all names into a vector to get a list of uniquely needed elements
    std::vector<string> allVars;
    for (unsigned int iv = 0; iv < splitTMVAvariablesSM.size(); ++iv)
      allVars.push_back(splitTMVAvariablesSM.at(iv).first);
    for (unsigned int iv = 0; iv < splitTMVAvariablesLM.size(); ++iv)
      allVars.push_back(splitTMVAvariablesLM.at(iv).first);
    for (unsigned int iv = 0; iv < splitTMVAvariablesMM.size(); ++iv)
      allVars.push_back(splitTMVAvariablesMM.at(iv).first);
    for (unsigned int iv = 0; iv < splitTMVAvariablesHM.size(); ++iv)
      allVars.push_back(splitTMVAvariablesHM.at(iv).first);
    for (unsigned int iv = 0; iv < splitTMVAvariablesVBF.size(); ++iv)
      allVars.push_back(splitTMVAvariablesVBF.at(iv).first);

    sort(allVars.begin(), allVars.end());
    allVars.erase( unique( allVars.begin(), allVars.end() ), allVars.end() );
    
    // Create map to contain values of variables
    std::map<string, float> allVarsMap;
    for (string var : allVars)
    allVarsMap[var] = 0.0;

    // Open tree to be updated
    TFile *outFile = TFile::Open(outputFile,"UPDATE");
    TTree *treenew = (TTree*)outFile->Get("HTauTauTree");
    int nentries = treenew->GetEntries();

    // Create vectors to store all the BDT outputs and relative vectors of TBranches
    std::vector<float> outSM (SM_kl.size());
    std::vector<float> outLM (LM_spin.size()*LM_mass.size());
    std::vector<float> outMM (MM_spin.size()*MM_mass.size());
    std::vector<float> outHM (HM_spin.size()*HM_mass.size());
    float mvaVBF;

    std::vector<TBranch*> branchSM (SM_kl.size());
    std::vector<TBranch*> branchLM (LM_spin.size()*LM_mass.size());
    std::vector<TBranch*> branchMM (MM_spin.size()*MM_mass.size());
    std::vector<TBranch*> branchHM (HM_spin.size()*HM_mass.size());
    TBranch *mvaBranchVBF;

    // Declare the TMVA readers
    TMVA::Reader * readerSM = new TMVA::Reader () ;
    TMVA::Reader * readerLM = new TMVA::Reader () ;
    TMVA::Reader * readerMM = new TMVA::Reader () ;
    TMVA::Reader * readerHM = new TMVA::Reader () ;
    TMVA::Reader * readerVBF = new TMVA::Reader () ;

    // Use a different variable for channel, otherwise it does not work, I don't know why
    float channel_BDT;
    treenew ->SetBranchAddress ("BDT_channel", &channel_BDT) ;

    // Assign variables to SM reader
    for (pair<string, string> vpair : splitTMVAvariablesSM)
	{
	  treenew ->SetBranchAddress (vpair.first.c_str (), &(allVarsMap.at (vpair.first))) ;
	  readerSM->AddVariable (vpair.second.c_str (), &(allVarsMap.at (vpair.first))) ;
	}
    // Add the channel and kl variable to the SM reader
    readerSM->AddVariable("channel", &channel_BDT);
    float kl_var;
    readerSM->AddVariable("kl", &kl_var);


    // Assign variables to LM reader
    for (pair<string, string> vpair : splitTMVAvariablesLM)
	{
	  treenew ->SetBranchAddress (vpair.first.c_str (), &(allVarsMap.at (vpair.first))) ;
	  readerLM->AddVariable (vpair.second.c_str (), &(allVarsMap.at (vpair.first))) ;
	}
    // Add mass, channel and spin to the LM reader
    float mass_LM;
    float spin_LM;
    readerLM->AddVariable("mass", &mass_LM);
    readerLM->AddVariable("channel", &channel_BDT);
    readerLM->AddVariable("spin", &spin_LM);


    // Assign variables to MM reader
    for (pair<string, string> vpair : splitTMVAvariablesMM)
	{
	  treenew ->SetBranchAddress (vpair.first.c_str (), &(allVarsMap.at (vpair.first))) ;
	  readerMM->AddVariable (vpair.second.c_str (), &(allVarsMap.at (vpair.first))) ;
	}
    // Add mass, channel and spin to the LM reader
    float mass_MM;
    float spin_MM;
    readerMM->AddVariable("mass", &mass_MM);
    readerMM->AddVariable("channel", &channel_BDT);
    readerMM->AddVariable("spin", &spin_MM);


    // Assign variables to HM reader
    for (pair<string, string> vpair : splitTMVAvariablesHM)
	{
	  treenew ->SetBranchAddress (vpair.first.c_str (), &(allVarsMap.at (vpair.first))) ;
	  readerHM->AddVariable (vpair.second.c_str (), &(allVarsMap.at (vpair.first))) ;
	}
    // Add mass, channel and spin to the LM reader
    float mass_HM;
    float spin_HM;
    readerHM->AddVariable("mass", &mass_HM);
    readerHM->AddVariable("channel", &channel_BDT);
    readerHM->AddVariable("spin", &spin_HM);


    // Assign variables to VBF reader
    for (pair<string, string> vpair : splitTMVAvariablesVBF)
	{
	  treenew ->SetBranchAddress (vpair.first.c_str (), &(allVarsMap.at (vpair.first))) ;
	  readerVBF->AddVariable (vpair.second.c_str (), &(allVarsMap.at (vpair.first))) ;
	}

    // Book the MVA methods
    if(doSM) readerSM->BookMVA("Grad_1", TMVAweightsSM.c_str() );
    if(doLM) readerLM->BookMVA("Grad_2", TMVAweightsLM.c_str() );
    if(doMM) readerMM->BookMVA("Grad_3", TMVAweightsMM.c_str() );
    if(doHM) readerHM->BookMVA("Grad_4", TMVAweightsHM.c_str() );
    if(doVBF) readerVBF->BookMVA("VBF", TMVAweightsVBF.c_str() );

    // Calculate BDT output for SM
    if (doSM)
    {
        int idxSM = 0;
        for (unsigned int ikl = 0; ikl < SM_kl.size(); ++ikl)
        {
            // Declare the BDT output branch
            std::string branch_name = boost::str(boost::format("BDToutSM_kl_%d") % SM_kl.at(ikl));
            branchSM.at(idxSM) = treenew->Branch(branch_name.c_str(), &outSM.at(idxSM));

            // Assign value to parametrization variables
            kl_var = SM_kl.at(ikl);

            // Calculate BDT output
            for(int i=0;i<nentries;i++)
            {
                treenew->GetEntry(i);
                outSM.at(idxSM) = readerSM->EvaluateMVA("Grad_1");
                branchSM.at(idxSM)->Fill();
            }
            ++idxSM;
        }
    }


    // Calculate BDT output for LM
    if (doLM)
    {
        int idxLM = 0;
        for (unsigned int ispin = 0; ispin < LM_spin.size(); ++ispin)
        {
            for (unsigned int imass = 0; imass < LM_mass.size(); ++imass)
            {
                // Declare the BDT output branch
                std::string branch_name = boost::str(boost::format("BDToutLM_spin_%d_mass_%d") % LM_spin.at(ispin) % LM_mass.at(imass));
                branchLM.at(idxLM) = treenew->Branch(branch_name.c_str(), &outLM.at(idxLM));

                // Assign value to parametrization variables
                mass_LM = LM_mass.at(imass);
                spin_LM = LM_spin.at(ispin);

                // Calculate BDT output
                for(int i=0;i<nentries;i++)
                {
                    treenew->GetEntry(i);
                    outLM.at(idxLM) = readerLM->EvaluateMVA("Grad_2");
                    branchLM.at(idxLM)->Fill();
                }
                ++idxLM;
            }
        }
    }


    // Calculate BDT output for MM
    if (doMM)
    {
        int idxMM = 0;
        for (unsigned int ispin = 0; ispin < MM_spin.size(); ++ispin)
        {
            for (unsigned int imass = 0; imass < MM_mass.size(); ++imass)
            {
                // Declare the BDT output branch
                std::string branch_name = boost::str(boost::format("BDToutMM_spin_%d_mass_%d") % MM_spin.at(ispin) % MM_mass.at(imass));
                branchMM.at(idxMM) = treenew->Branch(branch_name.c_str(), &outMM.at(idxMM));

                // Assign value to parametrization variables
                mass_MM = MM_mass.at(imass);
                spin_MM = MM_spin.at(ispin);

                // Calculate BDT output
                for(int i=0;i<nentries;i++)
                {
                    treenew->GetEntry(i);
                    outMM.at(idxMM) = readerMM->EvaluateMVA("Grad_3");
                    branchMM.at(idxMM)->Fill();
                }
                ++idxMM;
            }
        }
    }


    // Calculate BDT output for HM
    if (doHM)
    {
        int idxHM = 0;
        for (unsigned int ispin = 0; ispin < HM_spin.size(); ++ispin)
        {
            for (unsigned int imass = 0; imass < HM_mass.size(); ++imass)
            {
                // Declare the BDT output branch
                std::string branch_name = boost::str(boost::format("BDToutHM_spin_%d_mass_%d") % HM_spin.at(ispin) % HM_mass.at(imass));
                branchHM.at(idxHM) = treenew->Branch(branch_name.c_str(), &outHM.at(idxHM));

                // Assign value to parametrization variables
                mass_HM = HM_mass.at(imass);
                spin_HM = HM_spin.at(ispin);

                // Calculate BDT output
                for(int i=0;i<nentries;i++)
                {
                    treenew->GetEntry(i);
                    outHM.at(idxHM) = readerHM->EvaluateMVA("Grad_4");
                    branchHM.at(idxHM)->Fill();
                }
                ++idxHM;
            }
        }
    }


    // Calculate BDT output for VBF
    if (doVBF)
    {
        mvaBranchVBF = treenew->Branch("BDToutVBF", &mvaVBF, "BDToutVBF/F") ;
        for(int i=0;i<nentries;i++)
        {
          treenew->GetEntry(i);
          mvaVBF= readerVBF->EvaluateMVA("VBF") ;
          mvaBranchVBF->Fill();
        }
    }

    // Update tree and delete readers
    outFile->cd();
    treenew->Write ("", TObject::kOverwrite) ;
    delete readerSM;
    delete readerLM;
    delete readerMM;
    delete readerHM;
    delete readerVBF;

  } // End new BDT

  cout << "... SKIM finished, exiting." << endl;
  return 0 ;
}