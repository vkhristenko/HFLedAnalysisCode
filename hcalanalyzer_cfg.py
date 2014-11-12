import FWCore.ParameterSet.Config as cms

process = cms.Process("Led")

process.load("FWCore.MessageService.MessageLogger_cfi")
process.MessageLogger.cerr.FwkReport.reportEvery = 100

process.maxEvents = cms.untracked.PSet( input = cms.untracked.int32(-1) )

#
#	take care of input 
#
import sys
if len(sys.argv)<5:
	print "### ERROR: Input Error!"
	print "### Exiting..."
	sys.exit(1)

runNumber = sys.argv[2]
comment = sys.argv[3]
upDown = sys.argv[4]

process.source = cms.Source("HcalTBSource",
    # replace 'myfile.root' with the source file you want to use
    fileNames = cms.untracked.vstring(
		'root://eoscms//eos/cms/store/group/comm_hcal/LS1/USC_%s.root' % (
			runNumber)
    )
)

process.options = cms.untracked.PSet(
		wantSummary = cms.untracked.bool(False)
		)

#process.tbunpack = cms.EDProducer("HcalTBObjectUnpacker",
#		HcalSlowDataFED = cms.untracked.int32(3),
#		HcalTriggerFED = cms.untracked.int32(1),	
#		)

process.hcalDigis = cms.EDProducer("HcalRawToDigi",
		UnpackHF = cms.untracked.bool(True),
		### Falg to enable unpacking of TTP channels(default = false)
		### UnpackTTP = cms.untracked.bool(True),
		FilterDataQuality = cms.bool(False),
		InputLabel = cms.InputTag('source'),
		ComplainEmptyData = cms.untracked.bool(False),
		UnpackCalib = cms.untracked.bool(True),
		firstSample = cms.int32(0),
		lastSample = cms.int32(9))

outFile = 'ntuples_MagnetRamp%s/HF_LED_%s_%s.root' % (upDown, runNumber, comment)
process.hcalAnalyzer = cms.EDAnalyzer('HcalAnalyzer',
		OutFileName = cms.untracked.string(outFile),
		Verbosity = cms.untracked.int32(0)
)


#process.load('Configuration.StandardSequences.Geometry_cff')
process.load('Configuration.Geometry.GeometryIdeal_cff')
process.load('RecoLocalCalo.Configuration.hcalLocalReco_cff')
process.load('RecoLocalCalo.HcalRecProducers.HcalSimpleReconstructor_hf_cfi')

process.load('Configuration.StandardSequences.FrontierConditions_GlobalTag_cff')
process.GlobalTag.globaltag = 'GR_R_60_V9::All'
process.es_prefer_GlobalTag = cms.ESPrefer('PoolDBESSource', 'GlobalTag')

process.p = cms.Path(process.hcalDigis * process.hfreco * process.hcalAnalyzer)
