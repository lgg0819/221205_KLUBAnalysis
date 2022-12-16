#!/bin/bash
export X509_USER_PROXY=~/Condor/x509up_u149480
source /cvmfs/cms.cern.ch/cmsset_default.sh
cd /afs/cern.ch/user/g/gylee/bb_tautau/CMSSW_11_1_9/src/KLUBAnalysis || exit 1
export SCRAM_ARCH=slc6_amd64_gcc491
eval `scram r -sh`
source /afs/cern.ch/user/g/gylee/bb_tautau/CMSSW_11_1_9/src/KLUBAnalysis/scripts/setup.sh
cd /afs/cern.ch/user/g/gylee/bb_tautau/CMSSW_11_1_9/src/KLUBAnalysis/221214_out || exit 1
hadd outPlotter.root *.root
cd - || exit 1
python /afs/cern.ch/user/g/gylee/bb_tautau/CMSSW_11_1_9/src/KLUBAnalysis/scripts/combineFillerOutputs.py --dir /afs/cern.ch/user/g/gylee/bb_tautau/CMSSW_11_1_9/src/KLUBAnalysis/221214_out
