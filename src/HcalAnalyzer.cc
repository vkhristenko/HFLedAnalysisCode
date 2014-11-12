// -*- C++ -*-
//
// Package:    HcalAnalyzer
// Class:      HcalAnalyzer
// 
/**\class HcalAnalyzer HcalAnalyzer.cc UserCode/HcalAnalyzer/src/HcalAnalyzer.cc

 Description: [one line class summary]

 Implementation:
     [Notes on implementation]
*/
//
// Original Author:  Viktor Khristenko,510 1-004,+41227672815,
//         Created:  Tue Sep 16 15:47:09 CEST 2014
// $Id$
//
//


// system include files
#include <memory>

// user include files
#include "FWCore/Framework/interface/Frameworkfwd.h"
#include "FWCore/Framework/interface/EDAnalyzer.h"
#include "FWCore/Framework/interface/Event.h"
#include "FWCore/Framework/interface/MakerMacros.h"
#include "FWCore/Framework/interface/ESHandle.h"

#include "FWCore/ParameterSet/interface/ParameterSet.h"
#include "FWCore/ServiceRegistry/interface/Service.h"

#include "EventFilter/HcalRawToDigi/interface/HcalHTRData.h"
#include "EventFilter/HcalRawToDigi/interface/HcalDCCHeader.h"
#include "EventFilter/HcalRawToDigi/interface/HcalUnpacker.h"
#include "DataFormats/HcalDetId/interface/HcalOtherDetId.h"
#include "DataFormats/HcalDigi/interface/HcalQIESample.h"
#include "DataFormats/HcalDetId/interface/HcalSubdetector.h"
#include "DataFormats/HcalDetId/interface/HcalCalibDetId.h"

#include "DataFormats/Common/interface/Handle.h"
#include "DataFormats/FEDRawData/interface/FEDRawDataCollection.h"
#include "DataFormats/FEDRawData/interface/FEDHeader.h"
#include "DataFormats/FEDRawData/interface/FEDTrailer.h"
#include "DataFormats/FEDRawData/interface/FEDNumbering.h"
#include "DataFormats/FEDRawData/interface/FEDRawData.h"

#include "CommonTools/UtilAlgos/interface/TFileService.h"
#include "DataFormats/HcalDigi/interface/HcalDigiCollections.h"
#include "CalibFormats/HcalObjects/interface/HcalDbService.h"
#include "CalibFormats/HcalObjects/interface/HcalDbRecord.h"
#include "CalibFormats/HcalObjects/interface/HcalCalibrations.h"
#include "CalibFormats/HcalObjects/interface/HcalCoderDb.h"

#include "TBDataFormats/HcalTBObjects/interface/HcalTBTriggerData.h"
#include "RecoTBCalo/HcalTBObjectUnpacker/interface/HcalTBTriggerDataUnpacker.h"
#include "RecoTBCalo/HcalTBObjectUnpacker/interface/HcalTBSlowDataUnpacker.h"

#include "TH1D.h"
#include "TH2D.h"
#include "TGraph.h"
#include "TCanvas.h"
#include "TTree.h"
#include "TProfile.h"
#include "TFile.h"
#include "TSystem.h"

#include <iostream>
#include <fstream>
#include <sstream>

using namespace std;

#define NUMCHS 2000
#define NUMADCS 128
double adc2fC[NUMADCS]={
	-0.5,0.5,1.5,2.5,3.5,4.5,5.5,6.5,7.5,8.5,9.5, 10.5,11.5,12.5,
	13.5,15.,17.,19.,21.,23.,25.,27.,29.5,32.5,35.5,38.5,42.,46.,50.,54.5,59.5,
	64.5,59.5,64.5,69.5,74.5,79.5,84.5,89.5,94.5,99.5,104.5,109.5,114.5,119.5,
	124.5,129.5,137.,147.,157.,167.,177.,187.,197.,209.5,224.5,239.5,254.5,272.,
	292.,312.,334.5,359.5,384.5,359.5,384.5,409.5,434.5,459.5,484.5,509.5,534.5,
	559.5,584.5,609.5,634.5,659.5,684.5,709.5,747.,797.,847.,897.,947.,997.,
	1047.,1109.5,1184.5,1259.5,1334.5,1422.,1522.,1622.,1734.5,1859.5,1984.5,
	1859.5,1984.5,2109.5,2234.5,2359.5,2484.5,2609.5,2734.5,2859.5,2984.5,
	3109.5,3234.5,3359.5,3484.5,3609.5,3797.,4047.,4297.,4547.,4797.,5047.,
	5297.,5609.5,5984.5,6359.5,6734.5,7172.,7672.,8172.,8734.5,9359.5,9984.5};

struct TCalibLedInfo
{
	int numChs;
	int iphi[50];
	int ieta[50];
	int cBoxChannel[50];
	vector<string> cBoxString;
	int nTS[50];
	double pulse[50][10];
};

struct THFInfo
{
	int numChs;
	int iphi[NUMCHS];
	int ieta[NUMCHS];
	int depth[NUMCHS];
	double pulse[NUMCHS][10];
};

//
// class declaration
//

class HcalAnalyzer : public edm::EDAnalyzer {
   public:
      explicit HcalAnalyzer(const edm::ParameterSet&);
      ~HcalAnalyzer();

      static void fillDescriptions(edm::ConfigurationDescriptions& descriptions);

   private:
      virtual void beginJob() ;
      virtual void analyze(const edm::Event&, const edm::EventSetup&);
	  void getLedInfo(const edm::Event&, const edm::EventSetup&);
      virtual void endJob() ;

		TFile *_file;
		TTree *_tree;
		TTree *_treePMTs;
		string _outFileName;
		int _verbosity;
		TCalibLedInfo _calibInfo;
		THFInfo _hfInfo;

      virtual void beginRun(edm::Run const&, edm::EventSetup const&);
      virtual void endRun(edm::Run const&, edm::EventSetup const&);
      virtual void beginLuminosityBlock(edm::LuminosityBlock const&, edm::EventSetup const&);
      virtual void endLuminosityBlock(edm::LuminosityBlock const&, edm::EventSetup const&);

      // ----------member data ---------------------------
};

//
// constants, enums and typedefs
//

//
// static data member definitions
//

//
// constructors and destructor
//
HcalAnalyzer::HcalAnalyzer(const edm::ParameterSet& iConfig) :
	_outFileName(iConfig.getUntrackedParameter<string>("OutFileName")),
	_verbosity(iConfig.getUntrackedParameter<int>("Verbosity"))
{
   //now do what ever initialization is needed

	_file = new TFile(_outFileName.c_str(), "recreate");
	_file->mkdir("DiodeInfo");
	_file->mkdir("HFPMTInfo");

	_file->cd("DiodeInfo");
	_tree = new TTree("Events", "Events");
	_tree->Branch("numChs", &_calibInfo.numChs, "numChs/I");
	_tree->Branch("iphi", _calibInfo.iphi, "iphi[numChs]/I");
	_tree->Branch("ieta", _calibInfo.ieta, "ieta[numChs]/I");
	_tree->Branch("cBoxChannel", _calibInfo.cBoxChannel, "cBoxChannel[numChs]/I");
	_tree->Branch("cBoxString", &_calibInfo.cBoxString);
	_tree->Branch("nTS", _calibInfo.nTS, "nTS[numChs]/I");
	_tree->Branch("pulse", _calibInfo.pulse, "pulse[numChs][10]/D");

	_file->cd("HFPMTInfo");
	_treePMTs = new TTree("Events", "Events");
	_treePMTs->Branch("numChs", &_hfInfo.numChs, "numChs/I");
	_treePMTs->Branch("iphi", _hfInfo.iphi, "iphi[numChs]/I");
	_treePMTs->Branch("ieta", _hfInfo.ieta, "ieta[numChs]/I");
	_treePMTs->Branch("depth", _hfInfo.depth, "depth[numChs]/I");
	_treePMTs->Branch("pulse", _hfInfo.pulse, "pulse[numChs][10]/D");
}


HcalAnalyzer::~HcalAnalyzer()
{
 
   // do anything here that needs to be done at desctruction time
   // (e.g. close files, deallocate resources etc.)

}

void HcalAnalyzer::getLedInfo(const edm::Event &iEvent, 
		const edm::EventSetup &iSetup)
{
	using namespace edm;

	//
	//	Extracting Pin Diode Calibration Data
	//
	edm::Handle<HcalCalibDigiCollection> hCalibCollection;
	iEvent.getByType(hCalibCollection);
	int numCalibHFChs = 0;
	for (HcalCalibDigiCollection::const_iterator digi=hCalibCollection->begin();
			digi!=hCalibCollection->end(); ++digi)
	{
		int iphi = digi->id().iphi();
		int ieta = digi->id().ieta();
		int cBoxChannel = digi->id().cboxChannel();
		string cBoxString = digi->id().cboxChannelString();
		HcalSubdetector hcalSub = digi->id().hcalSubdet();

		//
		//	If this isn't HF data, skip it...
		//
		if (hcalSub!=4)
			continue;

		int nTS = digi->size();
		double pulse[100];
		for (int i=0; i<nTS; i++)
			pulse[i] = adc2fC[digi->sample(i).adc()&0xff];

		//
		//	Redundant to some extent
		//	Set the Branched arrays...
		//
		_calibInfo.iphi[numCalibHFChs] = iphi;
		_calibInfo.ieta[numCalibHFChs] = ieta;
		_calibInfo.cBoxChannel[numCalibHFChs] = cBoxChannel;
		_calibInfo.cBoxString.push_back(cBoxString);
		_calibInfo.nTS[numCalibHFChs] = nTS;
		for (int i=0; i<nTS; i++)
			_calibInfo.pulse[numCalibHFChs][i] = pulse[i];

		if (_verbosity>0)
		{
			cout << "### hcalSub=" << hcalSub << "  cBoxChannel="
				<< cBoxChannel << "  iphi=" << iphi << "  ieta=" << ieta
				<< "  cBoxString=" << cBoxString << endl;
			cout << "### ";
			for (int i=0; i<nTS; i++)
				cout << pulse[i] << "  ";
			cout << endl;
		}

		numCalibHFChs++;
	}
	_calibInfo.numChs = numCalibHFChs;
	_tree->Fill();

	//
	//	Extracting PMTs Data
	//
	edm::Handle<HFDigiCollection> hfDigiCollection;
	iEvent.getByType(hfDigiCollection);	
	int numChs = 0;
	for (HFDigiCollection::const_iterator digi=hfDigiCollection->begin();
			digi!=hfDigiCollection->end(); ++digi)
	{

		int iphi = digi->id().iphi();
		int ieta = digi->id().ieta();
		int depth = digi->id().depth();
		int nTS = digi->size();

		//
		//	Set the Branched arrays
		//
		_hfInfo.iphi[numChs] = iphi;
		_hfInfo.ieta[numChs] = ieta;
		_hfInfo.depth[numChs] = depth;

	
		for (int iTS=0; iTS<nTS; iTS++)
			_hfInfo.pulse[numChs][iTS] = adc2fC[digi->sample(iTS).adc()&0xff];

		numChs++;
	}
	_hfInfo.numChs = numChs;
	_treePMTs->Fill();

	return;
}


//
// member functions
//

// ------------ method called for each event  ------------
void
HcalAnalyzer::analyze(const edm::Event& iEvent, const edm::EventSetup& iSetup)
{
   using namespace edm;

	getLedInfo(iEvent, iSetup);

#ifdef THIS_IS_AN_EVENT_EXAMPLE
   Handle<ExampleData> pIn;
   iEvent.getByLabel("example",pIn);
#endif
   
#ifdef THIS_IS_AN_EVENTSETUP_EXAMPLE
   ESHandle<SetupData> pSetup;
   iSetup.get<SetupRecord>().get(pSetup);
#endif
}


// ------------ method called once each job just before starting event loop  ------------
void 
HcalAnalyzer::beginJob()
{
}

// ------------ method called once each job just after ending the event loop  ------------
void 
HcalAnalyzer::endJob() 
{
	_file->Write();
	_file->Close();
}

// ------------ method called when starting to processes a run  ------------
void 
HcalAnalyzer::beginRun(edm::Run const&, edm::EventSetup const&)
{
}

// ------------ method called when ending the processing of a run  ------------
void 
HcalAnalyzer::endRun(edm::Run const&, edm::EventSetup const&)
{
}

// ------------ method called when starting to processes a luminosity block  ------------
void 
HcalAnalyzer::beginLuminosityBlock(edm::LuminosityBlock const&, edm::EventSetup const&)
{
}

// ------------ method called when ending the processing of a luminosity block  ------------
void 
HcalAnalyzer::endLuminosityBlock(edm::LuminosityBlock const&, edm::EventSetup const&)
{
}

// ------------ method fills 'descriptions' with the allowed parameters for the module  ------------
void
HcalAnalyzer::fillDescriptions(edm::ConfigurationDescriptions& descriptions) {
  //The following says we do not know what parameters are allowed so do no validation
  // Please change this to state exactly what you do use, even if it is no parameters
  edm::ParameterSetDescription desc;
  desc.setUnknown();
  descriptions.addDefault(desc);
}

//define this as a plug-in
DEFINE_FWK_MODULE(HcalAnalyzer);
