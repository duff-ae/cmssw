/****************************************************************************
*
* This is a part of TOTEM ofFileline software.
*
* Based on TMultiDimFet.h (from ROOT) by Christian Holm Christense.
*
* Authors:
*   Hubert Niewiadomski
*   Jan Kašpar (jan.kaspar@gmail.com)
*
****************************************************************************/

#ifndef SimG4Core_TotemRPProtonTransportParametrization_TMultiDimFet_H
#define SimG4Core_TotemRPProtonTransportParametrization_TMultiDimFet_H

#ifndef ROOT_TNamed
#include "TNamed.h"
#endif
#ifndef ROOT_TVectorD
#include "TVectorD.h"
#endif
#ifndef ROOT_TMatrixD
#include "TMatrixD.h"
#endif
#ifndef ROOT_TList
#include "TList.h"
#endif
#ifndef ROOT_TVirtualFitter
#include "TVirtualFitter.h"
#endif

#include <vector>

//class TBrowser;

class TMultiDimFet : public TNamed {
public:
  enum EMDFPolyType { kMonomials, kChebyshev, kLegendre };

protected:
  TVectorD fQuantity;          //! Training sample, dependent quantity
  TVectorD fSqError;           //! Training sample, error in quantity
  Double_t fMeanQuantity;      // Mean of dependent quantity
  Double_t fMaxQuantity;       //! Max value of dependent quantity
  Double_t fMinQuantity;       //! Min value of dependent quantity
  Double_t fSumSqQuantity;     //! SumSquare of dependent quantity
  Double_t fSumSqAvgQuantity;  //! Sum of squares away from mean

  TVectorD fVariables;      //! Training sample, independent variables
  Int_t fNVariables;        // Number of independent variables
  TVectorD fMeanVariables;  //! mean value of independent variables
  TVectorD fMaxVariables;   // max value of independent variables
  TVectorD fMinVariables;   // min value of independent variables

  Int_t fSampleSize;  //! Size of training sample

  TVectorD fTestQuantity;   //! Test sample, dependent quantity
  TVectorD fTestSqError;    //! Test sample, Error in quantity
  TVectorD fTestVariables;  //! Test sample, independent variables

  Int_t fTestSampleSize;  //! Size of test sample

  Double_t fMinAngle;             //! Min angle for acepting new function
  Double_t fMaxAngle;             //! Max angle for acepting new function
  Int_t fMaxTerms;                // Max terms expected in final expr.
  Double_t fMinRelativeError;     //! Min relative error accepted
  std::vector<Int_t> fMaxPowers;  //! maximum powers, ex-array
  Double_t fPowerLimit;           //! Control parameter

  TMatrixD fFunctions;                //! Functions evaluated over sample
  Int_t fMaxFunctions;                // max number of functions
  std::vector<Int_t> fFunctionCodes;  //! acceptance code, ex-array
  Int_t fMaxStudy;                    //! max functions to study

  TMatrixD fOrthFunctions;      //! As above, but orthogonalised
  TVectorD fOrthFunctionNorms;  //! Norm of the evaluated functions

  std::vector<Int_t> fMaxPowersFinal;  //! maximum powers from fit, ex-array
  Int_t fMaxFunctionsTimesNVariables;  // fMaxFunctionsTimesNVariables
  std::vector<Int_t> fPowers;          // ex-array
  std::vector<Int_t> fPowerIndex;      // Index of accepted powers, ex-array

  TVectorD fResiduals;      //! Vector of the final residuals
  Double_t fMaxResidual;    //! Max redsidual value
  Double_t fMinResidual;    //! Min redsidual value
  Int_t fMaxResidualRow;    //! Row giving max residual
  Int_t fMinResidualRow;    //! Row giving min residual
  Double_t fSumSqResidual;  //! Sum of Square residuals

  Int_t fNCoefficients;           // Dimension of model coefficients
  TVectorD fOrthCoefficients;     //! The model coefficients
  TMatrixD fOrthCurvatureMatrix;  //! Model matrix
  TVectorD fCoefficients;         // Vector of the final coefficients
  TVectorD fCoefficientsRMS;      //! Vector of RMS of coefficients
  Double_t fRMS;                  //! Root mean square of fit
  Double_t fChi2;                 //! Chi square of fit
  Int_t fParameterisationCode;    //! Exit code of parameterisation

  Double_t fError;                 //! Error from parameterization
  Double_t fTestError;             //! Error from test
  Double_t fPrecision;             //! Relative precision of param
  Double_t fTestPrecision;         //! Relative precision of test
  Double_t fCorrelationCoeff;      //! Multi Correlation coefficient
  TMatrixD fCorrelationMatrix;     //! Correlation matrix
  Double_t fTestCorrelationCoeff;  //! Multi Correlation coefficient

  TList *fHistograms;     //! List of histograms
  Byte_t fHistogramMask;  //! Bit pattern of hisograms used

  //TVirtualFitter* fFitter;            //! Fit object (MINUIT)

  EMDFPolyType fPolyType;   // Type of polynomials to use
  Bool_t fShowCorrelation;  // print correlation matrix
  Bool_t fIsUserFunction;   // Flag for user defined function
  Bool_t fIsVerbose;        //

  virtual Double_t EvalFactor(Int_t p, Double_t x) const;
  virtual Double_t EvalControl(const Int_t *powers);
  virtual void MakeCoefficientErrors();
  virtual void MakeCorrelation();
  virtual Double_t MakeGramSchmidt(Int_t function);
  virtual void MakeCoefficients();
  virtual void MakeCandidates();
  virtual void MakeNormalized();
  virtual void MakeParameterization();
  virtual void MakeRealCode(const char *filename, const char *classname, Option_t *option = "");
  virtual Bool_t Select(const Int_t *iv);
  virtual Bool_t TestFunction(Double_t squareResidual, Double_t dResidur);

public:
  TMultiDimFet();
  TMultiDimFet(const TMultiDimFet &in) = default;
  const TMultiDimFet &operator=(const TMultiDimFet &in);

  TMultiDimFet(Int_t dimension, EMDFPolyType type = kMonomials, Option_t *option = "");
  ~TMultiDimFet() override;

  virtual void AddRow(const Double_t *x, Double_t D, Double_t E = 0);
  virtual void AddTestRow(const Double_t *x, Double_t D, Double_t E = 0);
  //void     Browse(TBrowser* b) override;
  void Clear(Option_t *option = "") override;  // *MENU*
  //void     Draw(Option_t * ="d") override { }
  virtual Double_t Eval(const Double_t *x, const Double_t *coeff = nullptr) const;
  virtual void FindParameterization(double precision);  // *MENU*
  //virtual void     Fit(Option_t *option=""); // *MENU*

  Double_t GetChi2() const { return fChi2; }
  const TMatrixD *GetCorrelationMatrix() const { return &fCorrelationMatrix; }
  const TVectorD *GetCoefficients() const { return &fCoefficients; }
  Double_t GetError() const { return fError; }
  std::vector<Int_t> GetFunctionCodes() const { return fFunctionCodes; }
  const TMatrixD *GetFunctions() const { return &fFunctions; }
  virtual TList *GetHistograms() const { return fHistograms; }
  Double_t GetMaxAngle() const { return fMaxAngle; }
  Int_t GetMaxFunctions() const { return fMaxFunctions; }
  std::vector<Int_t> GetMaxPowers() const { return fMaxPowers; }
  Double_t GetMaxQuantity() const { return fMaxQuantity; }
  Int_t GetMaxStudy() const { return fMaxStudy; }
  Int_t GetMaxTerms() const { return fMaxTerms; }
  const TVectorD *GetMaxVariables() const { return &fMaxVariables; }
  Double_t GetMeanQuantity() const { return fMeanQuantity; }
  const TVectorD *GetMeanVariables() const { return &fMeanVariables; }
  Double_t GetMinAngle() const { return fMinAngle; }
  Double_t GetMinQuantity() const { return fMinQuantity; }
  Double_t GetMinRelativeError() const { return fMinRelativeError; }
  const TVectorD *GetMinVariables() const { return &fMinVariables; }
  Int_t GetNVariables() const { return fNVariables; }
  Int_t GetNCoefficients() const { return fNCoefficients; }
  Int_t GetPolyType() const { return fPolyType; }
  std::vector<Int_t> GetPowerIndex() const { return fPowerIndex; }
  Double_t GetPowerLimit() const { return fPowerLimit; }
  std::vector<Int_t> GetPowers() const { return fPowers; }
  Double_t GetPrecision() const { return fPrecision; }
  const TVectorD *GetQuantity() const { return &fQuantity; }
  Double_t GetResidualMax() const { return fMaxResidual; }
  Double_t GetResidualMin() const { return fMinResidual; }
  Int_t GetResidualMaxRow() const { return fMaxResidualRow; }
  Int_t GetResidualMinRow() const { return fMinResidualRow; }
  Double_t GetResidualSumSq() const { return fSumSqResidual; }
  Double_t GetRMS() const { return fRMS; }
  Int_t GetSampleSize() const { return fSampleSize; }
  const TVectorD *GetSqError() const { return &fSqError; }
  Double_t GetSumSqAvgQuantity() const { return fSumSqAvgQuantity; }
  Double_t GetSumSqQuantity() const { return fSumSqQuantity; }
  Double_t GetTestError() const { return fTestError; }
  Double_t GetTestPrecision() const { return fTestPrecision; }
  const TVectorD *GetTestQuantity() const { return &fTestQuantity; }
  Int_t GetTestSampleSize() const { return fTestSampleSize; }
  const TVectorD *GetTestSqError() const { return &fTestSqError; }
  const TVectorD *GetTestVariables() const { return &fTestVariables; }
  const TVectorD *GetVariables() const { return &fVariables; }

  //static TMultiDimFet* Instance()               { return fgInstance; }
  Bool_t IsFolder() const override { return kTRUE; }
  virtual Double_t MakeChi2(const Double_t *coeff = nullptr);
  virtual void MakeCode(const char *functionName = "MDF", Option_t *option = "");   // *MENU*
  virtual void MakeHistograms(Option_t *option = "A");                              // *MENU*
  virtual void MakeMethod(const Char_t *className = "MDF", Option_t *option = "");  // *MENU*
  void Print(Option_t *option = "ps") const override;                               // *MENU*
  virtual void PrintPolynomialsSpecial(Option_t *option = "m") const;               // *MENU*

  void SetMaxAngle(Double_t angle = 0);
  void SetMaxFunctions(Int_t n) { fMaxFunctions = n; }
  void SetMaxPowers(const Int_t *powers);
  void SetMaxStudy(Int_t n) { fMaxStudy = n; }
  void SetMaxTerms(Int_t terms) { fMaxTerms = terms; }
  void SetMinRelativeError(Double_t error);
  void SetMinAngle(Double_t angle = 1);
  void SetPowerLimit(Double_t limit = 1e-3);
  virtual void SetPowers(const Int_t *powers, Int_t terms);

  void ReducePolynomial(double error);
  void ZeroDoubiousCoefficients(double error);

  ClassDefOverride(TMultiDimFet, 1)  // Multi dimensional fit class
};
#endif  //SimG4Core_TotemRPProtonTransportParametrization_TMultiDimFet_H
