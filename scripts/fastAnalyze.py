



import ROOT, sys, array

def analyzeLED():
	"""Analyzes LED Data w.r.t. Zero Field Run"""

	print "Analyzing LED Type..."
	fileName_LED_Zero = sys.argv[2]
	fileName_LED_Up = sys.argv[3]
	fileName_LED_Down = sys.argv[4]
	fileName_PED = sys.argv[5]

	fZero = open(fileName_LED_Zero)
	fUp = open(fileName_LED_Up)
	fDown = open(fileName_LED_Down)
	fPED = open(fileName_PED)

	vData_Up = []
	vData_Down = []
	vData_Zero = []
	vData_PED = []

	readData(vData_Up, fUp, "Up")
	readData(vData_Down, fDown, "Down")
	readData(vData_Zero, fZero, "Zero")
	readData(vData_PED, fPED, "PED Zero")

	rootOutFileTotal = ROOT.TFile("combined.root", "recreate")
	gSRvsField = ROOT.TGraph()
	gSRvsField.SetName("SRvsField")
	gSRvsField.SetTitle("Signal Ratio vs Field")
	vOut = []
	vOut[len(vOut):] = [gSRvsField]

	pathToFile = '/afs/cern.ch/work/v/vkhriste/CMSSW/CMSSW_5_3_21/src/UserCode/HcalAnalyzer/final_LED_MagnetRampUp/final_LED_Up2Zero_'
	print len(vData_Up)
	for x in vData_Up:
		compare('Up', x, vData_Zero[0], pathToFile, vData_PED[0], vOut)
		
	pathToFile = '/afs/cern.ch/work/v/vkhriste/CMSSW/CMSSW_5_3_21/src/UserCode/HcalAnalyzer/final_LED_MagnetRampDown/final_LED_Down2Zero_'
	print len(vData_Down)
	for x in vData_Down:
		compare('Down', x, vData_Zero[0], pathToFile, vData_PED[0], vOut)

	rootOutFileTotal.cd()
	gSRvsField.Write()
	rootOutFileTotal.Close()

def analyzePED():
	"""Checks Pedestal Validity at different filed values"""

	print 'Analyzing PED Type...'
	fileName_Zero = sys.argv[2]
	fileName_Up = sys.argv[3]
	fileName_Down = sys.argv[4]
	
	fZero = open(fileName_Zero)
	fUp = open(fileName_Up)
	fDown = open(fileName_Down)

	vData_Up = []
	vData_Down = []
	vData_Zero = []

	readData(vData_Up, fUp, "Up")
	readData(vData_Down, fDown, "Down")
	readData(vData_Zero, fZero, "Zero")

	#
	#	Analyze Ramping Up w.r.t. Zero
	#
	pathToFile = "/afs/cern.ch/work/v/vkhriste/CMSSW/CMSSW_5_3_21/src/UserCode/HcalAnalyzer/final_PED_MagnetRampUp/final_PED_Up2Zero_"
	print len(vData_Up)
	for x in vData_Up:
		compare('Up', x, vData_Zero[0], pathToFile)

	#
	#	Analyze Ramping Down w.r.t. Zero
	#
	pathToFile = "/afs/cern.ch/work/v/vkhriste/CMSSW/CMSSW_5_3_21/src/UserCode/HcalAnalyzer/final_PED_MagnetRampDown/final_PED_Down2Zero_"
	print len(vData_Down)
	for x in vData_Down:
		compare('Down',x, vData_Zero[0], pathToFile)

	print "Done Analyzing PED..."

def readData(allData, fileName, str):
	"""Reads Data from txt files"""

	for line in fileName:
		print 'Reading in %s-Field Data' % str
		v = line.split()
		fileName = v[0]
		cFile = open(fileName)
		data = ([[[[0 for idepth in range(2)] for iieta in range(13)] 
			for iiphi in range(36)] for idet in range(2)])
		for lline in cFile:
			vv = lline.split()
			idet = int(vv[0])
			iphi = vv[1]; iiphi = (int(iphi)-1)/2
			ieta = vv[2]; iieta = int(ieta)-29
			depth = vv[3]; idepth = int(depth)-1
			mean = float(vv[4])
			rms = float(vv[5])
			data[idet][iiphi][iieta][idepth] = [mean, rms]
		dataElement = [fileName, v[1], data]
		allData[len(allData):] = [dataElement]
	print "Done..."

def compare(upDown, dataEl, dataElZero, pathToFile, dataPED=None, vOut=None):
	"""Computes the Ratio of Signals and saves into Separate files"""

	rootFileName = pathToFile + dataEl[1] + '.root'
	outRootFile = ROOT.TFile(rootFileName, "recreate")
	hHFPRatio = ROOT.TH1D("HFP_Ratio_%s" % dataEl[1], "HFP Ratio for %s" % dataEl[1],
			100, 0, 2)
	hHFMRatio = ROOT.TH1D("HFM_Ratio_%s" % dataEl[1], "HFM Ratio for %s" % dataEl[1],
			100, 0, 2)

	print "Comparing for %sA" % dataEl[1]

	dataP = dataEl[2][0]
	dataM = dataEl[2][1]
	dataZP = dataElZero[2][0]
	dataZM = dataElZero[2][1]
	for iiphi in range(36):
		for iieta in range(13):
			for idepth in range(2):
				if dataZP[iiphi][iieta][idepth]==0:
					continue

#				meanP = dataP[iiphi][iieta][idepth][0]
#				meanM = dataM[iiphi][iieta][idepth][0]
#				meanZP = dataZP[iiphi][iieta][idepth][0]
#				meanZM = dataZM[iiphi][iieta][idepth][0]
#				print meanP, meanM, meanZP, meanZM

				pedP = 0
				pedM = 0
				if dataPED==None:
					pedP = 0
					pedM = 0
				else:
					pedP = dataPED[2][0][iiphi][iieta][idepth][0]
					pedM = dataPED[2][1][iiphi][iieta][idepth][0]

				ratioP = ((dataP[iiphi][iieta][idepth][0]-pedP)/
						(dataZP[iiphi][iieta][idepth][0]-pedP))
				ratioM = ((dataM[iiphi][iieta][idepth][0]-pedM)/
					(dataZM[iiphi][iieta][idepth][0]-pedM))
				hHFPRatio.Fill(ratioP)
				hHFMRatio.Fill(ratioM)

				if vOut!=None:
					nn = vOut[0].GetN()
					current = float(dataEl[1])
					field = current/1000.*0.209
					if upDown=='Down':
						field = -field
					vOut[0].SetPoint(nn+1, field, ratioP)
					vOut[0].SetPoint(nn+2, field, ratioM)

	outRootFile.Write()
	outRootFile.Close()

ledped = sys.argv[1]
if ledped=="LED":
	analyzeLED()
else:
	analyzePED()
