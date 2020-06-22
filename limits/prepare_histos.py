import sys, pwd, commands, optparse
from ROOT import *

def parseOptions():

    usage = ('usage: %prog [options] datasetList\n'
             + '%prog -h for help')
    parser = optparse.OptionParser(usage)
    
    parser.add_option('-f', '--filename',   dest='filename',   type='string', default="",  help='input plots')
    parser.add_option('-o', '--outfile',   dest='outName',   type='string', default="w",  help='output suffix')
    parser.add_option('-c', '--channel',   dest='channel',   type='string', default="TauTau",  help='channel')

    # store options and arguments as global variables
    global opt, args
    (opt, args) = parser.parse_args()


listHistos = []
systNamesOUT=["CMS_scale_t_13TeV_DM0"] #,"CMS_scale_j_13TeV"]
systNames=["tesXXX_DM0"] #<--- to fix



parseOptions()
print "Running"
inFile = TFile.Open(opt.filename)
ih = 0
toth = len (inFile.GetListOfKeys())

histos_syst_up = []
histos_syst_down = []

yieldFolder = "scales2018/"

#procnames = [ "TT", "WJets", "TW","EWK", "WW", "WW", "WZ", "ZH","other", "ZZ", "DY0b","DY1b", "DY2b","GGHH_NLO_"]
procnames = ["DY","GGHH_NLO_"]

ih = 0



for key in inFile.GetListOfKeys() :
	ih = ih + 1
	if (ih%100 == 0) : print key,ih,"/",toth
	kname = key.GetName()
	template = inFile.Get(kname)

	if "GGHH_NLO" in kname: kname = kname.replace("GGHH_NLO","ggHH").replace("_xs","_kt_1_bbtt").replace("cHHH", "kl_")
        if "VBFHH"    in kname: kname = kname.replace("VBFHH","qqHH").replace("C3","kl") #write 1_5 as 1p5 from the beginning

        print kname
	#protection against empty bins
	changedInt = False
	for ibin in range(1,template.GetNbinsX()+1) :
		integral = template.Integral()
		if template.GetBinContent(ibin) <= 0 :
			changedInt = True
			template.SetBinContent(ibin,0.000001)
			template.SetBinError(ibin,0.000001)

                    
	if template.Integral()>0 and changedInt : template.Scale(integral/template.Integral())
	if "TH1" in template.ClassName(): 
		for isyst in range(len(systNames)):
			lineToCheckDown = systNames[isyst].replace('XXX',"down").replace("tes","tau")
			lineToCheckUp = systNames[isyst].replace('XXX',"up").replace("tes","tau")
			found = -1
                        print '---->', lineToCheckDown
			if lineToCheckDown in kname :
				appString = "Down"
				remString = "down"
				found = 1
			elif lineToCheckUp in kname:
				appString = "Up"
				remString = "up"
				found =0
			if found>=0 :
				names = kname.split("_")
                                for i in range(0,len(names)):
                                    print names[i]
                                    if (names[i].startswith('s') or names[i].startswith('VBFloose')):
					for j in range(1, i):
                                            names[0] += "_"+str(names[j])
                                        names[1] = names[i]
                                        break
				proc = names[0]
				#print names
				yieldName=yieldFolder+"/"+opt.channel+"_"+names[1]
				yieldName =yieldName+'_'+systNames[isyst].replace('XXX','')+".txt"
				infile = open(yieldName)
				scale = 1.000
				for line in infile :
					words = line.split()
					if words[0] == proc :
						#print "found ",words, proc,float(words[1+found])
						scale = float(words[1+found])
						break
				template.Scale(scale)
				#print proc,scale,yieldName
				#if abs(1-scale)>0.02 : print "correct",yieldName, proc,scale
				kname = kname.replace("_"+systNames[isyst].replace('XXX',remString).replace("tes","tau"),"")
				#print "toReplace:",systNames[isyst]+remString,"temporary:", newName
				kname = kname + "_"+ systNamesOUT[isyst] + appString
				print kname
		template.SetName(kname)
		template.SetTitle(kname)
		listHistos.append(template.Clone())

outFile = TFile.Open("analyzedOutPlotter_{0}_{1}.root".format(opt.channel, opt.outName),"RECREATE")
outFile.cd()

for h in listHistos :
	h.Write()
for h in histos_syst_up:
	h.Write()
for h in histos_syst_down:
	h.Write()

outFile.Close()
