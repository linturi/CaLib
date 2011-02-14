// SVN Info: $Id$

/*************************************************************************
 * Author: Dominik Werthmueller, Irakli Keshelashvili
 *************************************************************************/

//////////////////////////////////////////////////////////////////////////
//                                                                      //
// TCCalibTime                                                          //
//                                                                      //
// Base time calibration module class.                                  //
//                                                                      //
//////////////////////////////////////////////////////////////////////////


#include "TCCalibTime.h"

ClassImp(TCCalibTime)


//______________________________________________________________________________
TCCalibTime::TCCalibTime(const Char_t* name, const Char_t* title, CalibData_t data,
                         Int_t nElem)
    : TCCalib(name, title, data, nElem)
{
    // Constructor.

    // init members
    fTimeGain = new Double_t[fNelem];
    for (Int_t i = 0; i < fNelem; i++) fTimeGain[i] = 0;
    fMean = 0;
    fLine = 0;
}

//______________________________________________________________________________
TCCalibTime::~TCCalibTime()
{
    // Destructor. 
    
    if (fTimeGain) delete [] fTimeGain;
    if (fLine) delete fLine;
}

//______________________________________________________________________________
void TCCalibTime::Init()
{
    // Init the module.
    
    Char_t tmp[256];

    // init members
    fMean = 0;
    fLine = new TLine();
    
    // configure line
    fLine->SetLineColor(4);
    fLine->SetLineWidth(3);
 
    // get histogram name
    sprintf(tmp, "%s.Histo.Fit.Name", GetName());
    if (!TCReadConfig::GetReader()->GetConfig(tmp))
    {
        Error("Init", "Histogram name was not found in configuration!");
        return;
    }
    else fHistoName = *TCReadConfig::GetReader()->GetConfig(tmp);
    
    // get time gain for TDCs
    if (this->InheritsFrom("TCCalibTAPSTime"))
    {
        // get individual time gain for TAPS TDCs (only from first set)
        TCMySQLManager::GetManager()->ReadParameters(kCALIB_TAPS_T1, fCalibration.Data(), fSet[0], fTimeGain, fNelem);
    }
    else if (this->InheritsFrom("TCCalibVetoTime"))
    {
        // get individual time gain for Veto TDCs (only from first set)
        TCMySQLManager::GetManager()->ReadParameters(kCALIB_VETO_T1, fCalibration.Data(), fSet[0], fTimeGain, fNelem);
    }
    else
    {
        // no time gain for CB rise time
        if (!this->InheritsFrom("TCCalibCBRiseTime"))
        {
            sprintf(tmp, "%s.TDCGain", GetName());
            Double_t tdc_gain;
            if (!TCReadConfig::GetReader()->GetConfig(tmp)) tdc_gain = 0.11771;
            else tdc_gain = TCReadConfig::GetReader()->GetConfigDouble(tmp);
            for (Int_t i = 0; i < fNelem; i++) fTimeGain[i] = tdc_gain;
            Info("Init", "Using a TDC gain of %f ns/channel", tdc_gain);
        }
    }

    // read old parameters (only from first set)
    TCMySQLManager::GetManager()->ReadParameters(fData, fCalibration.Data(), fSet[0], fOldVal, fNelem);
    
    // copy to new parameters
    for (Int_t i = 0; i < fNelem; i++) fNewVal[i] = fOldVal[i];

    // sum up all files contained in this runset
    TCFileManager f(fData, fCalibration.Data(), fNset, fSet);
    
    // get the main calibration histogram
    fMainHisto = f.GetHistogram(fHistoName.Data());
    if (!fMainHisto)
    {
        Error("Init", "Main histogram does not exist!\n");
        return;
    }
    
    // create the overview histogram
    fOverviewHisto = new TH1F("Overview", ";Element;Time peak position [ns]", fNelem, 0, fNelem);
    fOverviewHisto->SetMarkerStyle(2);
    fOverviewHisto->SetMarkerColor(4);
    
    // draw main histogram
    fCanvasFit->Divide(1, 2, 0.001, 0.001);
    fCanvasFit->cd(1)->SetLogz();
    sprintf(tmp, "%s.Histo.Fit", GetName());
    TCUtils::FormatHistogram(fMainHisto, tmp);
    fMainHisto->Draw("colz");

    // draw the overview histogram
    fCanvasResult->cd();
    sprintf(tmp, "%s.Histo.Overview", GetName());
    TCUtils::FormatHistogram(fOverviewHisto, tmp);
    fOverviewHisto->Draw("P");
}

//______________________________________________________________________________
void TCCalibTime::Fit(Int_t elem)
{
    // Perform the fit of the element 'elem'.
    
    Char_t tmp[256];
    
    // create histogram projection for this element
    sprintf(tmp, "ProjHisto_%i", elem);
    TH2* h2 = (TH2*) fMainHisto;
    if (fFitHisto) delete fFitHisto;
    fFitHisto = (TH1D*) h2->ProjectionX(tmp, elem+1, elem+1, "e");
    
    // init variables
    Double_t factor = 2.5;
    Double_t peakval = 0;
    Double_t range = 3.8;

    // check for sufficient statistics
    if (fFitHisto->GetEntries())
    {
        // delete old function
        if (fFitFunc) delete fFitFunc;
        sprintf(tmp, "fTime_%i", elem);

	// the fit function
	fFitFunc = new TF1("fFitFunc", "pol1(0)+gaus(2)");
	fFitFunc->SetLineColor(2);
	
	// get important parameter positions
	Double_t maxPos = fFitHisto->GetXaxis()->GetBinCenter(fFitHisto->GetMaximumBin());
	Double_t max = fFitHisto->GetBinContent(fFitHisto->GetMaximumBin());

	// configure fitting function
	fFitFunc->SetParameters(1, 0.1, max, maxPos, 8);
	fFitFunc->SetParLimits(3, maxPos - 2, maxPos + 2);
	fFitFunc->SetParLimits(4, 0, 20);                  
    
        // special configuration for certain classes
         if (!this->InheritsFrom("TCCalibTaggerTime") && 
             !this->InheritsFrom("TCCalibTAPSTime")   && 
             !this->InheritsFrom("TCCalibVetoTime"))
        {   
            // only gaussian
            fFitFunc->FixParameter(0, 0);
            fFitFunc->FixParameter(1, 0);
        }
        if (this->InheritsFrom("TCCalibTAPSTime"))
        {
	    fFitFunc->SetParLimits(4, 0.1, 2);                  
        }

        // estimate peak position
        peakval = fFitHisto->GetBinCenter(fFitHisto->GetMaximumBin());

        // temporary
        fMean = peakval;

        // first iteration
	fFitFunc->SetRange(peakval - range, peakval + range);
	//fFitFunc->SetParameters(fFitHisto->GetMaximum(), peakval, 7);
	fFitHisto->Fit(fFitFunc, "RBQ0");

        // second iteration
	peakval = fFitFunc->GetParameter(3);
        Double_t sigma = fFitFunc->GetParameter(4);
        fFitFunc->SetRange(peakval -factor*sigma, peakval +factor*sigma);
        fFitHisto->Fit(fFitFunc, "RBQ0");

        // final results
        fMean = fFitFunc->GetParameter(3); 

        // draw mean indicator line
        fLine->SetY1(0);
        fLine->SetY2(fFitHisto->GetMaximum() + 20);
        fLine->SetX1(fMean);
        fLine->SetX2(fMean);
    }

    // draw histogram
    fFitHisto->SetFillColor(35);
    fCanvasFit->cd(2);
    sprintf(tmp, "%s.Histo.Fit", GetName());
    TCUtils::FormatHistogram(fFitHisto, tmp);
    fFitHisto->Draw("hist");
    
    // draw fitting function
    if (fFitFunc) fFitFunc->Draw("same");
    
    // draw indicator line
    fLine->Draw();
    
    // update canvas
    fCanvasFit->Update();
    
    // update overview
    if (elem % 20 == 0)
    {
        fCanvasResult->cd();
        fOverviewHisto->Draw("E1");
        fCanvasResult->Update();
    }   
}

//______________________________________________________________________________
void TCCalibTime::Calculate(Int_t elem)
{
    // Calculate the new value of the element 'elem'.
    
    Char_t tmp[256];
    Bool_t unchanged = kFALSE;

    // check if fit was performed
    if (fFitHisto->GetEntries())
    {
        // check if line position was modified by hand
        if (fLine->GetX1() != fMean) fMean = fLine->GetX1();

        // calculate the new offset
        if (this->InheritsFrom("TCCalibCBRiseTime"))
        {
            fNewVal[elem] = fOldVal[elem] + fMean;
        }
        else
        {
            fNewVal[elem] = fOldVal[elem] + fMean / fTimeGain[elem];
        }

        // update overview histogram
        fOverviewHisto->SetBinContent(elem + 1, fMean);
        fOverviewHisto->SetBinError(elem + 1, 0.0000001);
    }
    else
    {   
        // do not change old value
        fNewVal[elem] = fOldVal[elem];
        unchanged = kTRUE;
    }

    // user information
    if (this->InheritsFrom("TCCalibCBRiseTime")) strcpy(tmp, "rise time");
    else strcpy(tmp, "offset");

    printf("Element: %03d    Peak: %12.8f    "
           "old %s: %12.8f    new %s: %12.8f    diff: %6.2f %%",
           elem, fMean, tmp, fOldVal[elem], tmp, fNewVal[elem],
           TCUtils::GetDiffPercent(fOldVal[elem], fNewVal[elem]));
    if (unchanged) printf("    -> unchanged");
    
    if (this->InheritsFrom("TCCalibCBTime") ||
        this->InheritsFrom("TCCalibCBRiseTime")) 
    {
        if (TCUtils::IsCBHole(elem)) printf(" (hole)");
    }
    printf("\n");
}   

