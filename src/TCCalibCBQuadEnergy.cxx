// SVN Info: $Id$

/*************************************************************************
 * Author: Dominik Werthmueller
 *************************************************************************/

//////////////////////////////////////////////////////////////////////////
//                                                                      //
// TCCalibCBQuadEnergy                                                  //
//                                                                      //
// Calibration module for the quadratic CB energy correction.           //
//                                                                      //
//////////////////////////////////////////////////////////////////////////


#include "TCCalibCBQuadEnergy.h"

ClassImp(TCCalibCBQuadEnergy)


//______________________________________________________________________________
TCCalibCBQuadEnergy::TCCalibCBQuadEnergy()
    : TCCalib("CB.QuadEnergy", "CB quadratic energy correction", kCALIB_CB_EQUAD0, TCConfig::kMaxCB)
{
    // Empty constructor.
    
    // init members
    fPar0 = 0;
    fPar1 = 0;
    fMainHisto2 = 0;
    fMainHisto3 = 0;
    fMainHisto2BG = 0;
    fMainHisto3BG = 0;
    fFitHisto1b = 0;
    fFitHisto2 = 0;
    fFitHisto3 = 0;
    fFitFunc1b = 0;
    fFitFuncBG = 0;
    fFitFunc1bBG = 0;
    fPi0Pos = 0;
    fEtaPos = 0;
    fPi0MeanE = 0;
    fEtaMeanE = 0;
    fLinePi0 = 0;
    fLineEta = 0;
    fLineMeanEPi0 = 0;
    fLineMeanEEta = 0;
    fPi0PosHisto = 0;
    fEtaPosHisto = 0;
    fIsFitPi0 = kTRUE;
    for (Int_t i = 0; i < 2; i++)
    {
        fPi0Prompt[i] = 0;
        fPi0BG1[i] = 0;
        fPi0BG2[i] = 0;
        fEtaPrompt[i] = 0;
        fEtaBG1[i] = 0;
        fEtaBG2[i] = 0;
    }
}

//______________________________________________________________________________
TCCalibCBQuadEnergy::~TCCalibCBQuadEnergy()
{
    // Destructor. 
    
    if (fPar0) delete [] fPar0;
    if (fPar1) delete [] fPar1;
    if (fMainHisto2) delete fMainHisto2;
    if (fMainHisto3) delete fMainHisto3;
    if (fMainHisto2BG) delete fMainHisto2BG;
    if (fMainHisto3BG) delete fMainHisto3BG;
    if (fFitHisto1b) delete fFitHisto1b;
    if (fFitHisto2) delete fFitHisto2;
    if (fFitHisto3) delete fFitHisto3;
    if (fFitFunc1b) delete fFitFunc1b;
    if (fFitFuncBG) delete fFitFuncBG;
    if (fFitFunc1bBG) delete fFitFunc1bBG;
    if (fLinePi0) delete fLinePi0;
    if (fLineEta) delete fLineEta;
    if (fLineMeanEPi0) delete fLineMeanEPi0;
    if (fLineMeanEEta) delete fLineMeanEEta;
    if (fPi0PosHisto) delete fPi0PosHisto;
    if (fEtaPosHisto) delete fEtaPosHisto;
}

//______________________________________________________________________________
void TCCalibCBQuadEnergy::Init()
{
    // Init the module.
    
    Char_t tmp[256];

    // init members
    fPar0 = new Double_t[fNelem];
    fPar1 = new Double_t[fNelem];
    fFitHisto1b = 0;
    fFitHisto2 = 0;
    fFitHisto3 = 0;
    fFitFunc1b = 0;
    fFitFuncBG = 0;
    fFitFunc1bBG = 0;
    fPi0Pos = 0;
    fEtaPos = 0;
    fPi0MeanE = 0;
    fEtaMeanE = 0;
    fLinePi0 = new TLine();
    fLineEta = new TLine();
    fLineMeanEPi0 = new TLine();
    fLineMeanEEta = new TLine();
    fIsFitPi0 = kTRUE;
    for (Int_t i = 0; i < 2; i++)
    {
        fPi0Prompt[i] = 0;
        fPi0BG1[i] = 0;
        fPi0BG2[i] = 0;
        fEtaPrompt[i] = 0;
        fEtaBG1[i] = 0;
        fEtaBG2[i] = 0;
    }
    
    // configure lines
    fLinePi0->SetLineColor(4);
    fLinePi0->SetLineWidth(3);
    fLineEta->SetLineColor(4);
    fLineEta->SetLineWidth(3);
    fLineMeanEPi0->SetLineColor(4);
    fLineMeanEPi0->SetLineWidth(3);
    fLineMeanEEta->SetLineColor(4);
    fLineMeanEEta->SetLineWidth(3);
  
    // get main histogram name
    if (!TCReadConfig::GetReader()->GetConfig("CB.QuadEnergy.Histo.Fit.Name"))
    {
        Error("Init", "Histogram name was not found in configuration!");
        return;
    }
    else fHistoName = *TCReadConfig::GetReader()->GetConfig("CB.QuadEnergy.Histo.Fit.Name");
    
    // get mean pi0 energy histogram name
    TString hMeanPi0Name;
    if (!TCReadConfig::GetReader()->GetConfig("CB.QuadEnergy.Histo.MeanE.Pi0.Name"))
    {
        Error("Init", "Histogram name was not found in configuration!");
        return;
    }
    else hMeanPi0Name = *TCReadConfig::GetReader()->GetConfig("CB.QuadEnergy.Histo.MeanE.Pi0.Name");
    
    // get mean eta energy histogram name
    TString hMeanEtaName;
    if (!TCReadConfig::GetReader()->GetConfig("CB.QuadEnergy.Histo.MeanE.Eta.Name"))
    {
        Error("Init", "Histogram name was not found in configuration!");
        return;
    }
    else hMeanEtaName = *TCReadConfig::GetReader()->GetConfig("CB.QuadEnergy.Histo.MeanE.Eta.Name");
      
    // read old parameters (only from first set)
    TCMySQLManager::GetManager()->ReadParameters(kCALIB_CB_EQUAD0, fCalibration.Data(), fSet[0], fPar0, fNelem);
    TCMySQLManager::GetManager()->ReadParameters(kCALIB_CB_EQUAD1, fCalibration.Data(), fSet[0], fPar1, fNelem);

    // sum up all files contained in this runset
    TCFileManager f(fData, fCalibration.Data(), fNset, fSet);
    
    // get the main calibration histogram
    fMainHisto = f.GetHistogram(fHistoName.Data());
    if (!fMainHisto)
    {
        Error("Init", "Main histogram does not exist!\n");
        return;
    }
    
    // get the pi0 mean energy histogram
    sprintf(tmp, "%s_Prompt", hMeanPi0Name.Data());
    fMainHisto2 = (TH2*) f.GetHistogram(tmp);
    if (!fMainHisto2)
    {
        Error("Init", "Pi0 mean energy histogram does not exist!\n");
        return;
    }
    
    // get the eta mean energy histogram
    sprintf(tmp, "%s_Prompt", hMeanEtaName.Data());
    fMainHisto3 = (TH2*) f.GetHistogram(tmp);
    if (!fMainHisto3)
    {
        Error("Init", "Eta mean energy histogram does not exist!\n");
        return;
    }
    
    // get the pi0 mean energy histogram (background)
    sprintf(tmp, "%s_BG", hMeanPi0Name.Data());
    fMainHisto2BG = (TH2*) f.GetHistogram(tmp);
    if (!fMainHisto2BG)
    {
        Error("Init", "Pi0 mean energy (background) histogram does not exist!\n");
        return;
    }
    
    // get the eta mean energy histogram (background)
    sprintf(tmp, "%s_BG", hMeanEtaName.Data());
    fMainHisto3BG = (TH2*) f.GetHistogram(tmp);
    if (!fMainHisto3BG)
    {
        Error("Init", "Eta mean energy (background) histogram does not exist!\n");
        return;
    }
     
    // get pi0 invariant mass cuts
    TCReadConfig::GetReader()->GetConfigDoubleDouble("CB.QuadEnergy.Pi0.Prompt.Range", &fPi0Prompt[0], &fPi0Prompt[1]);
    TCReadConfig::GetReader()->GetConfigDoubleDouble("CB.QuadEnergy.Pi0.BG1.Range", &fPi0BG1[0], &fPi0BG1[1]);
    TCReadConfig::GetReader()->GetConfigDoubleDouble("CB.QuadEnergy.Pi0.BG2.Range", &fPi0BG2[0], &fPi0BG2[1]);
    
    // get eta invariant mass cuts
    TCReadConfig::GetReader()->GetConfigDoubleDouble("CB.QuadEnergy.Eta.Prompt.Range", &fEtaPrompt[0], &fEtaPrompt[1]);
    TCReadConfig::GetReader()->GetConfigDoubleDouble("CB.QuadEnergy.Eta.BG1.Range", &fEtaBG1[0], &fEtaBG1[1]);
    TCReadConfig::GetReader()->GetConfigDoubleDouble("CB.QuadEnergy.Eta.BG2.Range", &fEtaBG2[0], &fEtaBG2[1]);

    // create the pi0 overview histogram
    fPi0PosHisto = new TH1F("Pi0 position overview", ";Element;#pi^{0} peak position [MeV]", fNelem, 0, fNelem);
    fPi0PosHisto->SetMarkerStyle(2);
    fPi0PosHisto->SetMarkerColor(4);
    TCUtils::FormatHistogram(fPi0PosHisto, "CB.QuadEnergy.Histo.Overview.Pi0");
    
    // create the eta overview histogram
    fEtaPosHisto = new TH1F("Eta position overview", ";Element;#eta peak position [MeV]", fNelem, 0, fNelem);
    fEtaPosHisto->SetMarkerStyle(2);
    fEtaPosHisto->SetMarkerColor(4);
    TCUtils::FormatHistogram(fEtaPosHisto, "CB.QuadEnergy.Histo.Overview.Eta");
    
    // prepare fit histogram canvas
    fCanvasFit->Divide(1, 4, 0.001, 0.001);

    // draw the overview histograms
    fCanvasResult->Divide(1, 2, 0.001, 0.001);
    fCanvasResult->cd(1);
    fPi0PosHisto->Draw("P");
    fCanvasResult->cd(2);
    fEtaPosHisto->Draw("P");
}

//______________________________________________________________________________
Double_t TCCalibCBQuadEnergy::BGFunc(Double_t* x, Double_t* par)
{
    // Fitting function that excludes the peak region.

    if (fIsFitPi0 && x[0] > fPi0Prompt[0] && x[0] < fPi0Prompt[1])
    {
        TF1::RejectPoint();
        return 0;
    }
    if (!fIsFitPi0 && x[0] > fEtaPrompt[0] && x[0] < fEtaPrompt[1])
    {
        TF1::RejectPoint();
        return 0;
    }
    else 
    {
        return par[0] + par[1]*x[0] + par[2]*x[0]*x[0] + par[3]*x[0]*x[0]*x[0];
    }
}

//______________________________________________________________________________
void TCCalibCBQuadEnergy::Fit(Int_t elem)
{
    // Perform the fit of the element 'elem'.
    
    Char_t tmp[256];
    
    // get the 2g invariant mass histograms
    sprintf(tmp, "ProjHisto_%d", elem);
    TH2* h2 = (TH2*) fMainHisto;
    if (fFitHisto) delete fFitHisto;
    if (fFitHisto1b) delete fFitHisto1b;
    fFitHisto = (TH1*) h2->ProjectionX(tmp, elem+1, elem+1, "e");
    sprintf(tmp, "ProjHisto_%db", elem);
    fFitHisto1b = (TH1*) fFitHisto->Clone(tmp);
    TCUtils::FormatHistogram(fFitHisto, "CB.QuadEnergy.Histo.Fit.Pi0.IM");
    TCUtils::FormatHistogram(fFitHisto1b, "CB.QuadEnergy.Histo.Fit.Eta.IM");
 
    // get pi0 mean energy projection
    sprintf(tmp, "ProjHistoMeanPi0_%d", elem);
    h2 = (TH2*) fMainHisto2;
    if (fFitHisto2) delete fFitHisto2;
    fFitHisto2 = h2->ProjectionX(tmp, elem+1, elem+1, "e");
    TCUtils::FormatHistogram(fFitHisto2, "CB.QuadEnergy.Histo.Fit.Pi0.MeanE");

    // get eta mean energy projection
    sprintf(tmp, "ProjHistoMeanEta_%d", elem);
    h2 = (TH2*) fMainHisto3;
    if (fFitHisto3) delete fFitHisto3;
    fFitHisto3 = h2->ProjectionX(tmp, elem+1, elem+1, "e");
    TCUtils::FormatHistogram(fFitHisto3, "CB.QuadEnergy.Histo.Fit.Eta.MeanE");
    
    // get pi0 mean energy projection (background)
    sprintf(tmp, "ProjHistoMeanPi0BG_%d", elem);
    h2 = (TH2*) fMainHisto2BG;
    TH1* hMeanEPi0BG = h2->ProjectionX(tmp, elem+1, elem+1, "e");
    TCUtils::FormatHistogram(hMeanEPi0BG, "CB.QuadEnergy.Histo.Fit.Pi0.MeanE");

    // get eta mean energy projection (background)
    sprintf(tmp, "ProjHistoMeanEtaBG_%d", elem);
    h2 = (TH2*) fMainHisto3BG;
    TH1* hMeanEEtaBG = h2->ProjectionX(tmp, elem+1, elem+1, "e");
    TCUtils::FormatHistogram(hMeanEEtaBG, "CB.QuadEnergy.Histo.Fit.Eta.MeanE");
    
    // get x-axis range
    Double_t xmin = fFitHisto1b->GetXaxis()->GetBinCenter(fFitHisto1b->GetXaxis()->GetFirst());
    Double_t xmax = fFitHisto1b->GetXaxis()->GetBinCenter(fFitHisto1b->GetXaxis()->GetLast());

    // set new range & get the peak position of eta
    fFitHisto1b->GetXaxis()->SetRangeUser(500, 600);
    Double_t fMaxEta = fFitHisto1b->GetBinCenter(fFitHisto1b->GetMaximumBin());
    fFitHisto1b->GetXaxis()->SetRangeUser(xmin, xmax);
     
    // draw pi0 stuff
    fCanvasFit->cd(1); 
    fFitHisto->SetFillColor(35);   
    fFitHisto->Draw("hist");
    
    // draw eta stuff
    fCanvasFit->cd(2); 
    fFitHisto1b->SetFillColor(35);   
    fFitHisto1b->Draw("hist");
    
    fCanvasFit->cd(3); 
    fFitHisto2->SetFillColor(35);
    fFitHisto2->Draw("hist");
    
    fCanvasFit->cd(4); 
    fFitHisto3->SetFillColor(35);
    fFitHisto3->Draw("hist");
 
    // check for sufficient statistics
    if (fFitHisto->GetEntries())
    {
        // delete old functions
        if (fFitFunc) delete fFitFunc;
        if (fFitFunc1b) delete fFitFunc1b;
        if (fFitFuncBG) delete fFitFuncBG;
        if (fFitFunc1bBG) delete fFitFunc1bBG;
	
       
        // fitting range
        Double_t pi0_fit_min = fPi0Prompt[0] - 10;
        Double_t pi0_fit_max = fPi0Prompt[1] + 20;
        Double_t eta_fit_min = fEtaBG1[0];
        Double_t eta_fit_max = fEtaBG2[1] > 690 ? fEtaBG2[1] : 690;

        // create pi0 background function (excludes peak)
        sprintf(tmp, "fPi0_BG_%i", elem);
        fFitFuncBG = new TF1(tmp, this, &TCCalibCBQuadEnergy::BGFunc, pi0_fit_min, pi0_fit_max, 2);
        fIsFitPi0 = kTRUE;
        
        // fit pi0 background
        fFitHisto->Fit(fFitFuncBG, "RBQ0");
 
        // create eta background function (excludes peak)
        sprintf(tmp, "fEta_BG_%i", elem);
        fFitFunc1bBG = new TF1(tmp, this, &TCCalibCBQuadEnergy::BGFunc, eta_fit_min, eta_fit_max, 3);
        fIsFitPi0 = kFALSE;
        
        // fit eta background
        fFitHisto1b->Fit(fFitFunc1bBG, "RBQ0");
       
        // create full pi0 background function
        sprintf(tmp, "fPi0_BG_full_%i", elem);
        TF1* func = new TF1(tmp, "pol1", pi0_fit_min, pi0_fit_max);
        func->SetParameters(fFitFuncBG->GetParameter(0), fFitFuncBG->GetParameter(1));
        func->SetLineColor(3);
        delete fFitFuncBG;
        fFitFuncBG = func;
        
        // create full eta background function
        sprintf(tmp, "fEta_BG_full_%i", elem);
        func = new TF1(tmp, "pol2", eta_fit_min, eta_fit_max);
        func->SetParameters(fFitFunc1bBG->GetParameter(0), fFitFunc1bBG->GetParameter(1), fFitFunc1bBG->GetParameter(2));
        func->SetLineColor(3);
        delete fFitFunc1bBG;
        fFitFunc1bBG = func;
         
        // create pi0 fitting function
        sprintf(tmp, "fPi0_%i", elem);
        fFitFunc = new TF1(tmp, "gaus(0)+pol1(3)", pi0_fit_min, pi0_fit_max);
        fFitFunc->SetLineColor(2);
        
        // create eta fitting function
        sprintf(tmp, "fEta_%i", elem);
        fFitFunc1b = new TF1(tmp, "gaus(0)+pol2(3)", eta_fit_min, eta_fit_max);
        fFitFunc1b->SetLineColor(2);
  
        // configure pi0 fitting function
        fFitFunc->SetParameters(fFitHisto->GetMaximum(), 135, 10, 1, 1, 1);
        fFitFunc->SetParLimits(0, 0, 1e6);
        fFitFunc->SetParLimits(1, 120, 140);
        fFitFunc->SetParLimits(2, 0, 40);
        fFitFunc->FixParameter(3, fFitFuncBG->GetParameter(0));
        fFitFunc->FixParameter(4, fFitFuncBG->GetParameter(1));

        // configure eta fitting function
        fFitFunc1b->SetParameters(fFitHisto1b->GetMaximum(), fMaxEta, 15, 1, 1, 1, 0.1);
        fFitFunc1b->SetParLimits(0, 1, fFitHisto1b->GetMaximum()+1);
        fFitFunc1b->SetParLimits(1, 520, 580);
        fFitFunc1b->SetParLimits(2, 1, 50);
        fFitFunc1b->FixParameter(3, fFitFunc1bBG->GetParameter(0));
        fFitFunc1b->FixParameter(4, fFitFunc1bBG->GetParameter(1));
        fFitFunc1b->FixParameter(5, fFitFunc1bBG->GetParameter(2));
	
        // fit histograms
        for (Int_t i = 0; i < 10; i++)
            if (!fFitHisto->Fit(fFitFunc, "RBQ0")) break;
        for (Int_t i = 0; i < 10; i++)
            if (!fFitHisto1b->Fit(fFitFunc1b, "RBQ0")) break;
        
        // calculate pi0 background subtraction factor
        Double_t int_sig_pi0 = fFitFuncBG->Integral(fPi0Prompt[0], fPi0Prompt[1]);
        Double_t int_bg_pi0 = fFitHisto->Integral(fFitHisto->FindBin(fPi0BG1[0]), fFitHisto->FindBin(fPi0BG1[1]), "width") +
                              fFitHisto->Integral(fFitHisto->FindBin(fPi0BG2[0]), fFitHisto->FindBin(fPi0BG2[1]), "width");
        Double_t bgfac_pi0 = - int_sig_pi0 / int_bg_pi0;

        // calculate eta background subtraction factor
        Double_t int_sig_eta = fFitFunc1bBG->Integral(fEtaPrompt[0], fEtaPrompt[1]);
        Double_t int_bg_eta = fFitHisto1b->Integral(fFitHisto1b->FindBin(fEtaBG1[0]), fFitHisto1b->FindBin(fEtaBG1[1]), "width") +
                              fFitHisto1b->Integral(fFitHisto1b->FindBin(fEtaBG2[0]), fFitHisto1b->FindBin(fEtaBG2[1]), "width");
        Double_t bgfac_eta = - int_sig_eta / int_bg_eta;
        
        // calculate eta background subtraction factor
        //Double_t int_sig_eta = fFitFunc1bBG->Integral(fEtaPrompt[0], fEtaPrompt[1]);
        //Double_t int_bg_eta = fFitFunc1bBG->Integral(fEtaBG1[0], fEtaBG1[1]) + fFitFunc1bBG->Integral(fEtaBG2[0], fEtaBG2[1]);
        //Double_t bgfac_eta = - int_sig_eta / int_bg_eta;
 
        // subtract background from pi0 mean energy
        fFitHisto2->Add(hMeanEPi0BG, bgfac_pi0);
        fFitHisto3->Add(hMeanEEtaBG, bgfac_eta);

        for (Int_t i = 1; i <= fFitHisto2->GetNbinsX(); i++)
            if (fFitHisto2->GetBinContent(i) < 0) fFitHisto2->SetBinContent(i, 0);

        for (Int_t i = 1; i <= fFitHisto3->GetNbinsX(); i++)
            if (fFitHisto3->GetBinContent(i) < 0) fFitHisto3->SetBinContent(i, 0);

        // get results
        fPi0Pos = fFitFunc->GetParameter(1);
        fEtaPos = fFitFunc1b->GetParameter(1);
        fPi0MeanE = fFitHisto2->GetMean();
        fEtaMeanE = fFitHisto3->GetMean();

        // draw pi0 position indicator line
        fLinePi0->SetY1(0);
        fLinePi0->SetY2(fFitHisto->GetMaximum() + 20);
        
        // draw eta position indicator line
        fLineEta->SetY1(0);
        fLineEta->SetY2(fFitHisto->GetMaximum() + 20);
        
        // check if mass is in normal range
        if (fPi0Pos < 80 || fPi0Pos > 200) fPi0Pos = 135;
        if (fEtaPos < 450 || fEtaPos > 650) fEtaPos = 547;
        
        // set indicator lines
        fLinePi0->SetX1(fPi0Pos);
        fLinePi0->SetX2(fPi0Pos);
        fLineEta->SetX1(fEtaPos);
        fLineEta->SetX2(fEtaPos);

        // set lines
        fLineMeanEPi0->SetX1(fPi0MeanE);
        fLineMeanEPi0->SetX2(fPi0MeanE);
        fLineMeanEPi0->SetY1(0);
        fLineMeanEPi0->SetY2(fFitHisto2->GetMaximum());
        fLineMeanEEta->SetX1(fEtaMeanE);
        fLineMeanEEta->SetX2(fEtaMeanE);
        fLineMeanEEta->SetY1(0);
        fLineMeanEEta->SetY2(fFitHisto3->GetMaximum());

        // draw pi0 stuff
        fCanvasFit->cd(1); 
        if (fFitFunc) fFitFunc->Draw("same");
        if (fFitFuncBG) fFitFuncBG->Draw("same");
        fLinePi0->Draw();
        
        // draw eta stuff
        fCanvasFit->cd(2); 
        if (fFitFunc1b) fFitFunc1b->Draw("same");
        if (fFitFunc1bBG) fFitFunc1bBG->Draw("same");
        fLineEta->Draw();
        
        fCanvasFit->cd(3); 
        fLineMeanEPi0->Draw();
        fFitHisto3->Rebin(2);
        fFitHisto3->Draw("hist");

        fCanvasFit->cd(4); 
        fLineMeanEEta->Draw();
        hMeanEEtaBG->Rebin(2);
        hMeanEEtaBG->Draw("hist");

        // clean-up
        //delete hMeanEPi0BG;
        //delete hMeanEEtaBG;
    }
    
    // update canvas
    fCanvasFit->Update();
 
    // update overview
    if (elem % 20 == 0)
    {
        fCanvasResult->cd(1);
        fPi0PosHisto->Draw("E1");
        fCanvasResult->cd(2);
        fEtaPosHisto->Draw("E1");
        fCanvasResult->Update();
    }   
}

//______________________________________________________________________________
void TCCalibCBQuadEnergy::Calculate(Int_t elem)
{
    // Calculate the new value of the element 'elem'.
    
    Bool_t no_corr = kFALSE;

    // check if fit was performed
    if (fFitHisto->GetEntries())
    {
        // check if pi0 line position was modified by hand
        if (fLinePi0->GetX1() != fPi0Pos) fPi0Pos = fLinePi0->GetX1();
        
        // check if etaline position was modified by hand
        if (fLineEta->GetX1() != fEtaPos) fEtaPos = fLineEta->GetX1();
        
        // calculate quadratic correction factors
        Double_t mean_E_ratio = fEtaMeanE / fPi0MeanE;
        Double_t pion_im_ratio = TCConfig::kPi0Mass / fPi0Pos;
        Double_t eta_im_ratio = TCConfig::kEtaMass / fEtaPos;
        fPar0[elem] = (eta_im_ratio - mean_E_ratio*pion_im_ratio) / (1. - mean_E_ratio);
        fPar1[elem] = (pion_im_ratio - fPar0[elem]) / fPi0MeanE;
        
        // check values
        if (TMath::IsNaN(fPar0[elem]) || TMath::IsNaN(fPar1[elem]))
        {
            fPar0[elem] = 1;
            fPar1[elem] = 0;
            no_corr = kTRUE;
        }

        // update overview histograms
        fPi0PosHisto->SetBinContent(elem+1, fPi0Pos);
        fPi0PosHisto->SetBinError(elem+1, 0.0000001);
        fEtaPosHisto->SetBinContent(elem+1, fEtaPos);
        fEtaPosHisto->SetBinError(elem+1, 0.0000001);
    }
    else
    {   
        fPar0[elem] = 1;
        fPar1[elem] = 0;
        no_corr = kTRUE;
    }

    // user information
    printf("Element: %03d    Pi0 Pos.: %6.2f    Pi0 ME: %6.2f    "
           "Eta Pos.: %6.2f    Eta ME: %6.2f    Par0: %12.8f    Par1: %e",
           elem, fPi0Pos, fPi0MeanE, fEtaPos, fEtaMeanE, fPar0[elem], fPar1[elem]);
    if (no_corr) printf("    -> no correction");
    if (TCUtils::IsCBHole(elem)) printf(" (hole)");
    printf("\n");
}   

//______________________________________________________________________________
void TCCalibCBQuadEnergy::PrintValues()
{
    // Print out the old and new values for all elements.

    // loop over elements
    for (Int_t i = 0; i < fNelem; i++)
    {
        printf("Element: %03d    Par0: %12.8f    "
               "Par1: %12.8f\n",
               i, fPar0[i], fPar1[i]);
    }
}

//______________________________________________________________________________
void TCCalibCBQuadEnergy::Write()
{
    // Write the obtained calibration values to the database.
    
    // write values to database
    for (Int_t i = 0; i < fNset; i++)
    {
        TCMySQLManager::GetManager()->WriteParameters(kCALIB_CB_EQUAD0, fCalibration.Data(), fSet[i], fPar0, fNelem);
        TCMySQLManager::GetManager()->WriteParameters(kCALIB_CB_EQUAD1, fCalibration.Data(), fSet[i], fPar1, fNelem);
    }
}

