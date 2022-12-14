#! /usr/bin/env python
import sys, pwd, commands, optparse
import os
import re

#define function for parsing options
def parseOptions():
    usage = ('usage: %prog [options] datasetList\n'
             + '%prog -h for help')
    parser = optparse.OptionParser(usage)
    
    parser.add_option('-n'    , '--name' , dest='n'    , type='string', default=''   , help='name'        )
    parser.add_option('-b'    , '--blind', dest='blind',                default=False, action='store_true')
    parser.add_option('--ggF' ,            dest='ggF'  ,                default=False, action='store_true')
    parser.add_option('--VBF' ,            dest='VBF'  ,                default=False, action='store_true')
    parser.add_option('--fast',            dest='fast' ,                default=False, action='store_true')
    parser.add_option('--frTH',            dest='frTH' ,                default=False, action='store_true')
    parser.add_option('--kl'  ,            dest='kl'   , type='string', default='1'                       )
    parser.add_option('--kt'  ,            dest='kt'   , type='string', default='1'                       )
    parser.add_option('--CV'  ,            dest='CV'   , type='string', default='1'                       )
    parser.add_option('--C2V' ,            dest='C2V'  , type='string', default='1'                       )
 
    # store options and arguments as global variables
    global opt, args
    (opt, args) = parser.parse_args()



if __name__ == "__main__":
    parseOptions()
    global opt, args

    cmsswBase=os.environ['CMSSW_BASE']
    jobsDir = os.getcwd()
    #create a wrapper for standalone cmssw job
    scriptFile = open('%s/runJob_Asym_%s.sh'%(jobsDir,opt.n), 'w')
    scriptFile.write('#!/bin/bash\n')
    scriptFile.write('export X509_USER_PROXY=~/.t3/proxy.cert\n')
    scriptFile.write('source /cvmfs/cms.cern.ch/cmsset_default.sh\n')
    scriptFile.write('cd {}\n'.format(cmsswBase + '/src/'))
    scriptFile.write('eval `scram r -sh`\n')
    scriptFile.write('cd %s\n'%jobsDir)
    scriptFile.write('ulimit -s unlimited \n')
    command = "combine -M AsymptoticLimits"
    if opt.blind:
        command = command + " --run blind "
        
    currFolder = os.getcwd ()
    command = command + " %s/comb.root" % jobsDir
    set_parameters = []
    red_parameters = []
    fre_parameters = []
    freeze = True

    # limits only on r_gghh
    if opt.ggF and not opt.VBF:
        red_parameters.append("r_gghh")
        fre_parameters.append("r")
        set_parameters.append("r_qqhh=1")

    # limits only on r_qqhh
    if opt.VBF and not opt.ggF:
        red_parameters.append("r_qqhh")
        fre_parameters.append("r")
        set_parameters.append("r_gghh=1")

    # limits on r
    if opt.ggF and opt.VBF:
        red_parameters.append("r")
        freeze = False # all parameters except r already frozen by default
        set_parameters.append("r_qqhh=1,r_gghh=1")

    command     = command + " --redefineSignalPOIs " + ",".join(red_parameters)
    if freeze:
        command = command + " --freezeParameters "   + ",".join(fre_parameters)
    if len(set_parameters)>0:
        command = command + " --setParameters "
        command = command + ",".join(set_parameters) + ","

    couplings = []
    if opt.kt:  couplings.append("kt="+opt.kt)
    if opt.kl:  couplings.append("kl="+opt.kl)
    if opt.CV:  couplings.append("CV="+opt.CV)
    if opt.C2V: couplings.append("C2V="+opt.C2V)
    command = command + ",".join(couplings)

    # add options to make combine faster
    if opt.fast:
        command = command + " --cminDefaultMinimizerType Minuit2 --cminDefaultMinimizerStrategy 0 --cminFallbackAlgo Minuit2,0:1.0 "

    # Freeze theory uncretainties group
    theoryName = ""
    if opt.frTH:
        command = command + " --freezeNuisanceGroups theory "
        theoryName = "_noTH"

    #scriptFile.write('%s -n %s_forLim &> out_Asym_%s.log \n' % (command, opt.n,opt.n))
    #scriptFile.write('%s -n %s_forLim_noTH --freezeNuisanceGroups theory &> out_Asym_%s_noTH.log \n' % (command,opt.n,opt.n))
    scriptFile.write('%s -m 125 -n %s_forLim%s &> out_Asym_%s%s.log \n' % (command,opt.n,theoryName,opt.n,theoryName))
    scriptFile.write('echo "All done for job %s" \n'%opt.n)
    scriptFile.close()
    os.system('chmod u+rwx %s/runJob_Asym_%s.sh'%(jobsDir,opt.n))
    os.system("/opt/exp_soft/cms/t3/t3submit -long \'%s/runJob_Asym_%s.sh\'"%(jobsDir,opt.n))

