#!/usr/bin/env python
import os, sys
#import pandas as pd

names = [
['myPUHisto_0j0b','PU_Legacy2017_SF_DY_LO_0j0b'],
['myPUHisto_1j0b','PU_Legacy2017_SF_DY_LO_1j0b'],
['myPUHisto_1j1b','PU_Legacy2017_SF_DY_LO_1j1b'],
['myPUHisto_2j0b','PU_Legacy2017_SF_DY_LO_2j0b'],
['myPUHisto_2j1b','PU_Legacy2017_SF_DY_LO_2j1b'],
['myPUHisto_2j2b','PU_Legacy2017_SF_DY_LO_2j2b'],
['myPUHisto_3j0b','PU_Legacy2017_SF_DY_LO_3j0b'],
['myPUHisto_3j1b','PU_Legacy2017_SF_DY_LO_3j1b'],
['myPUHisto_3j2b','PU_Legacy2017_SF_DY_LO_3j2b'],
['myPUHisto_3j3b','PU_Legacy2017_SF_DY_LO_3j3b'],
['myPUHisto_4j0b','PU_Legacy2017_SF_DY_LO_4j0b'],
['myPUHisto_4j1b','PU_Legacy2017_SF_DY_LO_4j1b'],
['myPUHisto_4j2b','PU_Legacy2017_SF_DY_LO_4j2b'],
['myPUHisto_4j3b','PU_Legacy2017_SF_DY_LO_4j3b'],
['myPUHisto_4j4b','PU_Legacy2017_SF_DY_LO_4j4b']
]

#filepath = 'outputs_January2019/DY_special/DYJets_special_MyMCPileupHistogram.root'
filepath = 'MyMCPileupHistogram_DYspecial2017.root'

for name in names:

    #command = "root -l \"makeRatio_DY_LO_bins.C(\\\""+filepath+"\\\", \\\""+name[0]+"\\\")\" > outputs_January2019/DY_special/"+name[1]+".txt -q"
    command = "root -l \"makeRatio_DY_LO_bins.C(\\\""+filepath+"\\\", \\\""+name[0]+"\\\")\" > Legacy_Run2_PU_SF/2017/"+name[1]+".txt -q"

    print command

    os.system(command)

    #with  open("outputs_January2019/DY_special/"+name[1]+".txt", 'r') as fin:
    with  open("Legacy_Run2_PU_SF/2017/"+name[1]+".txt", 'r') as fin:
        data = fin.read().splitlines(True)
        for k in range(len(data)):
            weight= []
            if k < 3: continue;
            weight= data[k].split("=")
            data[k] = weight[1].replace(";","")
            data[k] = data[k].replace(" ","")

    #with open("outputs_January2019/DY_special/"+name[1]+".txt", 'w') as fout:
    with open("Legacy_Run2_PU_SF/2017/"+name[1]+".txt", 'w') as fout:
        fout.writelines(data[3:])

