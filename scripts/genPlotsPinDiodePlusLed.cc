




#define NUMPHIS 36
#define NUMETAS 13
#define NUMDEPTHS 2







struct TCalibLedInfo
{
	int numChs;
	int iphi[50];
	int ieta[50];
	int cBoxChannel[50];
	vector<string> *cBoxString;
	int nTS[50];
	double pulse[50][10];
};

struct THFInfo
{
	int numChs;
	int iphi[2000];
	int ieta[2000];
	int depth[2000];
	double pulse[2000][10];
};

void genPlotsPinDiodePlusLed(int verb)
{
	TCalibLedInfo calibInfo;
	THFInfo hfInfo;

	TFile *in = new TFile("../ntuples_MagnetRampUp/HF_LED_229255.root");

	in->cd("DiodeInfo");
	TTree *tree = (TTree*)gDirectory->Get("Events");
	tree->SetBranchAddress("numChs", &calibInfo.numChs);
	tree->SetBranchAddress("iphi", &calibInfo.iphi);
	tree->SetBranchAddress("ieta", &calibInfo.ieta);
	tree->SetBranchAddress("cBoxChannel", &calibInfo.cBoxChannel);
	tree->SetBranchAddress("cBoxString", &calibInfo.cBoxString);
	tree->SetBranchAddress("nTS", &calibInfo.nTS);
	tree->SetBranchAddress("pulse", calibInfo.pulse);

	in->cd("HFPMTInfo");
	TTree *treeHF = (TTree*)gDirectory->Get("Events");
	treeHF->SetBranchAddress("numChs", &hfInfo.numChs);
	treeHF->SetBranchAddress("iphi", &hfInfo.iphi);
	treeHF->SetBranchAddress("ieta", &hfInfo.ieta);
	treeHF->SetBranchAddress("depth", &hfInfo.depth);
	treeHF->SetBranchAddress("pulse", hfInfo.pulse);

	//
	//	Set up output
	//
	TFile *out  = new TFile("out_test.root", 
			"recreate");
	out->mkdir("PinDiode");
	out->mkdir("LED");

	out->cd("PinDiode");
	gDirectory->mkdir("Histos");
	gDirectory->mkdir("AvgPulses");
	char histName[200];
	TH1D *hDiodeSigHist[24];
	TProfile *pDiodeAvgSigShapes[24];

	out->cd("LED");
	gDirectory->mkdir("HFM");
	gDirectory->mkdir("HFP");

	gDirectory->cd("HFM");
	gDirectory->mkdir("Histos");
	gDirectory->mkdir("AvgPulses");

	out->cd("LED/HFP");
	gDirectory->mkdir("Histos");
	gDirectory->mkdir("AvgPulses");
	TH1D *hLEDSigHist_M[NUMPHIS][NUMETAS][NUMDEPTHS];
	TProfile *pLEDAvgSigShapes_M[NUMPHIS][NUMETAS][NUMDEPTHS];
	TH1D *hLEDSigHist_P[NUMPHIS][NUMETAS][NUMDEPTHS];
	TProfile *pLEDAvgSigShapes_P[NUMPHIS][NUMETAS][NUMDEPTHS];
	for (int iiphi=0; iiphi<NUMPHIS; iiphi++)
	{
		for (int iieta=0; iieta<NUMETAS; iieta++)
		{
			for (int idepth=0; idepth<NUMDEPTHS; idepth++)
			{
				out->cd("LED/HFM/Histos");
				sprintf(histName, "LED_Histo_IPHI%d_IETA%d_D%d",
						2*iiphi+1, iieta+29, idepth+1);
				hLEDSigHist_M[iiphi][iieta][idepth] = new TH1D(
						histName, histName, 100000, 0, 10000);

				out->cd("LED/HFM/AvgPulses");
				sprintf(histName, "LED_AvgShape_IPHI%d_IETA%d_D%d",
						2*iiphi+1, iieta+29, idepth+1);
				pLEDAvgSigShapes_M[iiphi][iieta][idepth] = new TProfile(
						histName, histName, 10, 0, 10);
			}
		}
	}
	for (int iiphi=0; iiphi<NUMPHIS; iiphi++)
	{
		for (int iieta=0; iieta<NUMETAS; iieta++)
		{
			for (int idepth=0; idepth<NUMDEPTHS; idepth++)
			{
				out->cd("LED/HFP/Histos");
				sprintf(histName, "LED_Histo_IPHI%d_IETA%d_D%d",
						2*iiphi+1, iieta+29, idepth+1);
				hLEDSigHist_P[iiphi][iieta][idepth] = new TH1D(
						histName, histName, 100000, 0, 10000);

				out->cd("LED/HFP/AvgPulses");
				sprintf(histName, "LED_AvgShape_IPHI%d_IETA%d_D%d",
						2*iiphi+1, iieta+29, idepth+1);
				pLEDAvgSigShapes_P[iiphi][iieta][idepth] = new TProfile(
						histName, histName, 10, 0, 10);
			}
		}
	}

	int numEvents = tree->GetEntries();
	cout << "### Total PinDiode Events=" << numEvents << endl;
	for (int iEvent=0; iEvent<numEvents; iEvent++)
	{
		tree->GetEntry(iEvent);
		treeHF->GetEntry(iEvent);

		if (iEvent%100==0)
			cout << "### iEvent=" << iEvent << endl;

		//
		//	Go over each of 24 channels
		//	for HF Pin Diodes
		//
		for (int iCh=0; iCh<calibInfo.numChs; iCh++)
		{
			if (iEvent==0)
			{
				out->cd("PinDiode/Histos");
				sprintf(histName, "%s_IPHI%d_IETA%d_CHTYPE%d",
						calibInfo.cBoxString->at(iCh).c_str(), 
						calibInfo.iphi[iCh],
						calibInfo.ieta[iCh], calibInfo.cBoxChannel[iCh]);
				hDiodeSigHist[iCh] = new TH1D(histName, histName,
						10000, 0, 1000);

				out->cd("PinDiode/AvgPulses");
				pDiodeAvgSigShapes[iCh] = new TProfile(histName, histName,
						10, 0, 10);
			}

			if (verb>0)
			{
				cout << "iCh=" << iCh << "  iphi=" << calibInfo.iphi[iCh]
					<< "  ieta=" << calibInfo.ieta[iCh] 
					<< "  cBoxChannel=" << calibInfo.cBoxChannel[iCh]
					<< "  cBoxString=" << (*calibInfo.cBoxString)[iCh] << endl;
			
				for (int iTS=0; iTS<calibInfo.nTS[iCh]; iTS++)
					cout << calibInfo.pulse[iCh][iTS] << "  ";
				cout << endl;
			}

			Double_t totSigSum = 0;
			for (int iTS=0; iTS<calibInfo.nTS[iCh]; iTS++)
			{
				totSigSum += calibInfo.pulse[iCh][iTS];
				pDiodeAvgSigShapes[iCh]->Fill(iTS, calibInfo.pulse[iCh][iTS]);
			}
			hDiodeSigHist[iCh]->Fill(totSigSum);

		}

		//
		//	Go over HF PMT channels
		//
		for (int iCh=0; iCh<hfInfo.numChs; iCh++)
		{
			int iphi = hfInfo.iphi[iCh];
			int ieta = hfInfo.ieta[iCh];
			int depth = hfInfo.depth[iCh];
			int iiphi = (iphi-1)/2;
			int iieta = abs(ieta) - 29;
			int idepth = depth-1;

			Double_t totSigSum = 0;
			for (int iTS=0; iTS<10; iTS++)
			{
				totSigSum += hfInfo.pulse[iCh][iTS];
				if (ieta<0)
					pLEDAvgSigShapes_M[iiphi][iieta][idepth]->Fill(iTS,
							hfInfo.pulse[iCh][iTS]);
				else if (ieta > 0)
					pLEDAvgSigShapes_P[iiphi][iieta][idepth]->Fill(iTS,
							hfInfo.pulse[iCh][iTS]);
			}

			if (ieta<0)
				hLEDSigHist_M[iiphi][iieta][idepth]->Fill(totSigSum);
			else if (ieta>0)
				hLEDSigHist_P[iiphi][iieta][idepth]->Fill(totSigSum);
		}
	}



	out->Write();

	return;
}
