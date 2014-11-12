








import sys, subprocess

if len(sys.argv)<2:
	print "### ERROR: Incorrect input"
	print "### Exiting..."
	sys.exit(1)

#
#	Provide the input
#	And open up the input txt file
#
inFileName = sys.argv[1]
inFile =  open(inFileName)

#	Prep the command 
cmd_base = 'cmsRun hcalanalyzer_cfg.py'
head_dir = '/afs/cern.ch/work/v/vkhriste/CMSSW/CMSSW_5_3_21/src/UserCode/HcalAnalyzer'
scripts_dir = head_dir + '/scripts'
ntuples_dir = head_dir + '/ntuples_MagnetRampUp'
results_dir = head_dir + '/results_MagnetRampUp'

#	Read line by line and analyze
for line in inFile:
	v = line.split()
	if len(v)>0:
		runNumber = v[0]
		comment = v[1]
		field = v[2]
		cmd = 'cmsRun hcalanalyzer_cfg.py %s %s' % (
				runNumber, comment)
		print "### Creating Ntuple for Run %s" % runNumber
		subprocess.call(cmd, shell=True)

		cmd = '%s/analyze 0 %s/HF_LED_%s_%s.root %s/HF_LED_%s_%s.root %s %s' % (
				scripts_dir, ntuples_dir, runNumber, comment, 
				results_dir, runNumber, comment, comment, field)
		print "### Analyzing Ntuple for Run %s" % runNumber
		subprocess.call(cmd, shell=True)
