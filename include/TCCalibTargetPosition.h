// SVN Info: $Id$

/*************************************************************************
 * Author: Dominik Werthmueller
 *************************************************************************/

//////////////////////////////////////////////////////////////////////////
//                                                                      //
// TCCalibTargetPosition                                                //
//                                                                      //
// Calibration module for the target position.                          //
//                                                                      //
//////////////////////////////////////////////////////////////////////////


#ifndef TCCALIBTARGETPOSITION_H
#define TCCALIBTARGETPOSITION_H

#include "TCanvas.h"
#include "TH2.h"
#include "TLine.h"

#include "TCCalib.h"
#include "TCUtils.h"
#include "TCFileManager.h"


class TCCalibTargetPosition : public TCCalib
{

private:
    Double_t fPeak;                     // pi0 peak position
    Double_t fSigma;                    // pi0 peak sigma
    Double_t fSigmaPrev;                // previous pi0 peak sigma
    TLine* fLine;                       // indicator line
    
    virtual void Init();
    virtual void Fit(Int_t elem);
    virtual void Calculate(Int_t elem);

public:
    TCCalibTargetPosition();
    virtual ~TCCalibTargetPosition();

    ClassDef(TCCalibTargetPosition, 0) // target position calibration
};

#endif

