# To make this work we first have to:
# 	1. crate the following folders: 
#			- MY_ttCRlimits_${Y}inclusive
#			- MY_ttCRlimits_${Y}inclusive/ETau
#			- MY_ttCRlimits_${Y}inclusive/MuTau
#			- MY_ttCRlimits_${Y}inclusive/TauTau
# 	2. launch the submitHistoFiller.py to produce the outplotters inside the folders just created (cfr. paths in the mainCfg)
#	3. launch this bash script
#
# This will create the .root files that then have to be passed to the write_cards script 



export Y="2016"
export CHANNELS="TauTau MuTau ETau"

for ch in $CHANNELS
do 
	cd ../MY_ttCRlimits_${Y}inclusive/${ch}/
	hadd outPlotter${ch}.root outPlotter_*.root 
	#rm outPlotter_*.root
	cd -
done 

cd ../MY_ttCRlimits_${Y}inclusive/
hadd outPlotter.root ETau/outPlotterETau.root MuTau/outPlotterMuTau.root TauTau/outPlotterTauTau.root 
cp TauTau/main*.cfg ./mainCfg_ttCR_Legacy${Y}.cfg

cd ..
python scripts/combineFillerOutputs.py --dir MY_ttCRlimits_${Y}inclusive/

cd limits
python prepare_histos.py -f ../MY_ttCRlimits_${Y}inclusive/analyzedOutPlotter.root -o CR -c 'inclusive' -y ${Y}
mv -v analyzedOutPlotter_${Y}_inclusive_CR.root ../MY_ttCRlimits_${Y}inclusive/


