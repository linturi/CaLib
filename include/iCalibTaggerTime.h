/*************************************************************************
 * Author: Irakli Keshelashvili, Dominik Werthmueller
 *************************************************************************/

//////////////////////////////////////////////////////////////////////////
//                                                                      //
// iCalibTaggerTime                                                     //
//                                                                      //
// Calibration module for the Tagger time.                              //
//                                                                      //
//////////////////////////////////////////////////////////////////////////


#ifndef ICALIBTAGGERTIME_H
#define ICALIBTAGGERTIME_H

#include "TCanvas.h"
#include "TH2.h"
#include "TLine.h"

#include "iCalib.h"
#include "iFileManager.h"


class iCalibTaggerTime : public iCalib
{

private:
    Double_t fTimeGain;                 // Tagger TDC gain
    Double_t fMean;                     // mean time position
    TLine* fLine;                       // indicator line
    
    virtual void Init();
    virtual void Fit(Int_t elem);
    virtual void Calculate(Int_t elem);

public:
    iCalibTaggerTime();
    virtual ~iCalibTaggerTime();

    ClassDef(iCalibTaggerTime, 0)   // Tagger time calibration
};

#endif
