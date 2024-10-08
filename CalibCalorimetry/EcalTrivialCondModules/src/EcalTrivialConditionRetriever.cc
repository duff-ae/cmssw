//
// Created: 2 Mar 2006
//          Shahram Rahatlou, University of Rome & INFN
//
#include <fstream>
#include <iostream>
#include <memory>

#include "TRandom3.h"

#include "CalibCalorimetry/EcalTrivialCondModules/interface/EcalTrivialConditionRetriever.h"
#include "SimG4CMS/Calo/interface/EnergyResolutionVsLumi.h"
#include "FWCore/ParameterSet/interface/ParameterSet.h"
#include "FWCore/Utilities/interface/Exception.h"

#include "DataFormats/EcalDetId/interface/EBDetId.h"
#include "DataFormats/EcalDetId/interface/EEDetId.h"
#include "DataFormats/EcalDetId/interface/ESDetId.h"
#include "DataFormats/EcalDetId/interface/EcalElectronicsId.h"
#include "DataFormats/EcalDetId/interface/EcalTriggerElectronicsId.h"
#include "DataFormats/EcalDetId/interface/EcalSubdetector.h"

//#include "DataFormats/Provenance/interface/Timestamp.h"

#include "FWCore/MessageLogger/interface/MessageLogger.h"
#include "CondTools/Ecal/interface/EcalIntercalibConstantsXMLTranslator.h"
#include "CondTools/Ecal/interface/EcalIntercalibConstantsMCXMLTranslator.h"

using namespace edm;

EcalTrivialConditionRetriever::EcalTrivialConditionRetriever(const edm::ParameterSet& ps) {
  std::string dataPath_ =
      ps.getUntrackedParameter<std::string>("dataPath", "CalibCalorimetry/EcalTrivialCondModules/data/");
  //  std::string dataPath_ = "CalibCalorimetry/EcalTrivialCondModules/data_test/";

  /*since CMSSW_10_6_0, this dataPath_ points to https://github.com/cms-data/CalibCalorimetry-EcalTrivialCondModules  (extra package). 
To modify the default values :
$ git clone https://github.com/cms-data/CalibCalorimetry-EcalTrivialCondModules.git
$ cd CalibCalorimetry-EcalTrivialCondModules
$ modify what you want
$ git commit -a
$ git remote add ModifyCalibCalorimetryExtraPackage  git@github.com:yourName/CalibCalorimetry-EcalTrivialCondModules
$ git push ModifyCalibCalorimetryExtraPackage master:building-calibCalorimetry-extra-package

other solution : change this dataPath name to work directly in afs, ex. :

  std::string dataPath_="CalibCalorimetry/EcalTrivialCondModules/data_test/";

 */

  // initialize parameters used to produce cond DB objects
  totLumi_ = ps.getUntrackedParameter<double>("TotLumi", 0.0);
  instLumi_ = ps.getUntrackedParameter<double>("InstLumi", 0.0);

  // initilize parameters used to produce cond DB objects
  adcToGeVEBConstant_ = ps.getUntrackedParameter<double>("adcToGeVEBConstant", 0.035);
  adcToGeVEEConstant_ = ps.getUntrackedParameter<double>("adcToGeVEEConstant", 0.060);

  intercalibConstantMeanMC_ = ps.getUntrackedParameter<double>("intercalibConstantMeanMC", 1.0);
  intercalibConstantSigmaMC_ = ps.getUntrackedParameter<double>("intercalibConstantSigmaMC", 0.0);

  linCorrMean_ = ps.getUntrackedParameter<double>("linCorrMean", 1.0);
  linCorrSigma_ = ps.getUntrackedParameter<double>("linCorrSigma", 0.0);

  linearTime1_ = (unsigned long)atoi(ps.getUntrackedParameter<std::string>("linearTime1", "1").c_str());
  linearTime2_ = (unsigned long)atoi(ps.getUntrackedParameter<std::string>("linearTime2", "0").c_str());
  linearTime3_ = (unsigned long)atoi(ps.getUntrackedParameter<std::string>("linearTime3", "0").c_str());

  intercalibConstantMean_ = ps.getUntrackedParameter<double>("intercalibConstantMean", 1.0);
  intercalibConstantSigma_ = ps.getUntrackedParameter<double>("intercalibConstantSigma", 0.0);
  intercalibErrorMean_ = ps.getUntrackedParameter<double>("IntercalibErrorMean", 0.0);

  timeCalibConstantMean_ = ps.getUntrackedParameter<double>("timeCalibConstantMean", 0.0);
  timeCalibConstantSigma_ = ps.getUntrackedParameter<double>("timeCalibConstantSigma", 0.0);
  timeCalibErrorMean_ = ps.getUntrackedParameter<double>("timeCalibErrorMean", 0.0);

  timeOffsetEBConstant_ = ps.getUntrackedParameter<double>("timeOffsetEBConstant", 0.0);
  timeOffsetEEConstant_ = ps.getUntrackedParameter<double>("timeOffsetEEConstant", 0.0);

  laserAlphaMean_ = ps.getUntrackedParameter<double>("laserAlphaMean", 1.55);
  laserAlphaSigma_ = ps.getUntrackedParameter<double>("laserAlphaSigma", 0);

  laserAPDPNTime1_ = (unsigned long)atoi(ps.getUntrackedParameter<std::string>("laserAPDPNTime1", "1").c_str());
  laserAPDPNTime2_ = (unsigned long)atoi(ps.getUntrackedParameter<std::string>("laserAPDPNTime2", "0").c_str());
  laserAPDPNTime3_ = (unsigned long)atoi(ps.getUntrackedParameter<std::string>("laserAPDPNTime3", "0").c_str());

  laserAPDPNRefMean_ = ps.getUntrackedParameter<double>("laserAPDPNRefMean", 1.0);
  laserAPDPNRefSigma_ = ps.getUntrackedParameter<double>("laserAPDPNRefSigma", 0.0);

  laserAPDPNMean_ = ps.getUntrackedParameter<double>("laserAPDPNMean", 1.0);
  laserAPDPNSigma_ = ps.getUntrackedParameter<double>("laserAPDPNSigma", 0.0);

  pfRecHitThresholdsNSigmas_ = ps.getUntrackedParameter<double>("EcalPFRecHitThresholdNSigmas", 1.0);
  pfRecHitThresholdsNSigmasHEta_ = ps.getUntrackedParameter<double>("EcalPFRecHitThresholdNSigmasHEta", 1.0);
  pfRecHitThresholdsEB_ = ps.getUntrackedParameter<double>("EcalPFRecHitThresholdEB", 0.0);
  pfRecHitThresholdsEE_ = ps.getUntrackedParameter<double>("EcalPFRecHitThresholdEE", 0.0);

  localContCorrParameters_ =
      ps.getUntrackedParameter<std::vector<double> >("localContCorrParameters", std::vector<double>(0));
  crackCorrParameters_ = ps.getUntrackedParameter<std::vector<double> >("crackCorrParameters", std::vector<double>(0));
  energyCorrectionParameters_ =
      ps.getUntrackedParameter<std::vector<double> >("energyCorrectionParameters", std::vector<double>(0));
  energyUncertaintyParameters_ =
      ps.getUntrackedParameter<std::vector<double> >("energyUncertaintyParameters", std::vector<double>(0));
  energyCorrectionObjectSpecificParameters_ = ps.getUntrackedParameter<std::vector<double> >(
      "energyCorrectionObjectSpecificParameters", std::vector<double>(0));

  EBpedMeanX12_ = ps.getUntrackedParameter<double>("EBpedMeanX12", 200.);
  EBpedRMSX12_ = ps.getUntrackedParameter<double>("EBpedRMSX12", 1.10);
  EBpedMeanX6_ = ps.getUntrackedParameter<double>("EBpedMeanX6", 200.);
  EBpedRMSX6_ = ps.getUntrackedParameter<double>("EBpedRMSX6", 0.90);
  EBpedMeanX1_ = ps.getUntrackedParameter<double>("EBpedMeanX1", 200.);
  EBpedRMSX1_ = ps.getUntrackedParameter<double>("EBpedRMSX1", 0.62);

  EEpedMeanX12_ = ps.getUntrackedParameter<double>("EEpedMeanX12", 200.);
  EEpedRMSX12_ = ps.getUntrackedParameter<double>("EEpedRMSX12", 2.50);
  EEpedMeanX6_ = ps.getUntrackedParameter<double>("EEpedMeanX6", 200.);
  EEpedRMSX6_ = ps.getUntrackedParameter<double>("EEpedRMSX6", 2.00);
  EEpedMeanX1_ = ps.getUntrackedParameter<double>("EEpedMeanX1", 200.);
  EEpedRMSX1_ = ps.getUntrackedParameter<double>("EEpedRMSX1", 1.40);

  gainRatio12over6_ = ps.getUntrackedParameter<double>("gainRatio12over6", 2.0);
  gainRatio6over1_ = ps.getUntrackedParameter<double>("gainRatio6over1", 6.0);

  getWeightsFromFile_ = ps.getUntrackedParameter<bool>("getWeightsFromFile", false);

  sampleMaskEB_ = ps.getUntrackedParameter<unsigned int>("sampleMaskEB", 1023);
  sampleMaskEE_ = ps.getUntrackedParameter<unsigned int>("sampleMaskEB", 1023);

  EBtimeCorrAmplitudeBins_ =
      ps.getUntrackedParameter<std::vector<double> >("EBtimeCorrAmplitudeBins", std::vector<double>());
  EBtimeCorrShiftBins_ = ps.getUntrackedParameter<std::vector<double> >("EBtimeCorrShiftBins", std::vector<double>());
  EEtimeCorrAmplitudeBins_ =
      ps.getUntrackedParameter<std::vector<double> >("EEtimeCorrAmplitudeBins", std::vector<double>());
  EEtimeCorrShiftBins_ = ps.getUntrackedParameter<std::vector<double> >("EEtimeCorrShiftBins", std::vector<double>());

  EBG12samplesCorrelation_ =
      ps.getUntrackedParameter<std::vector<double> >("EBG12samplesCorrelation", std::vector<double>());
  EBG6samplesCorrelation_ =
      ps.getUntrackedParameter<std::vector<double> >("EBG6samplesCorrelation", std::vector<double>());
  EBG1samplesCorrelation_ =
      ps.getUntrackedParameter<std::vector<double> >("EBG1samplesCorrelation", std::vector<double>());
  EEG12samplesCorrelation_ =
      ps.getUntrackedParameter<std::vector<double> >("EEG12samplesCorrelation", std::vector<double>());
  EEG6samplesCorrelation_ =
      ps.getUntrackedParameter<std::vector<double> >("EEG6samplesCorrelation", std::vector<double>());
  EEG1samplesCorrelation_ =
      ps.getUntrackedParameter<std::vector<double> >("EEG1samplesCorrelation", std::vector<double>());

  sim_pulse_shape_EB_thresh_ = ps.getParameter<double>("sim_pulse_shape_EB_thresh");
  sim_pulse_shape_EE_thresh_ = ps.getParameter<double>("sim_pulse_shape_EE_thresh");
  sim_pulse_shape_APD_thresh_ = ps.getParameter<double>("sim_pulse_shape_APD_thresh");

  sim_pulse_shape_TI_ = ps.getUntrackedParameter<double>("sim_pulse_shape_TI", 1.0);

  nTDCbins_ = 1;

  weightsForAsynchronousRunning_ = ps.getUntrackedParameter<bool>("weightsForTB", false);

  edm::LogInfo(" EcalTrivialConditionRetriever ");

  if (totLumi_ > 0) {
    edm::LogInfo(" EcalTrivialConditionRetriever going to create conditions based on the damage due to ")
        << totLumi_ << " fb-1 integrated luminosity";
  }

  if (weightsForAsynchronousRunning_) {
    getWeightsFromFile_ = true;  //override user request
    //nTDCbins_ = 25;
    nTDCbins_ = 50;  //modif Alex-21-07-2006
  }

  std::string weightType;
  std::ostringstream str;

  if (!weightsForAsynchronousRunning_)
    str << "_CMS.txt";
  else
    str << "_TB.txt";

  weightType = str.str();

  amplWeightsFile_ = ps.getUntrackedParameter<std::string>("amplWeightsFile", dataPath_ + "ampWeights" + weightType);
  amplWeightsAftFile_ =
      ps.getUntrackedParameter<std::string>("amplWeightsAftFile", dataPath_ + "ampWeightsAfterGainSwitch" + weightType);
  pedWeightsFile_ = ps.getUntrackedParameter<std::string>("pedWeightsFile", dataPath_ + "pedWeights" + weightType);
  pedWeightsAftFile_ =
      ps.getUntrackedParameter<std::string>("pedWeightsAftFile", dataPath_ + "pedWeightsAfterGainSwitch" + weightType);
  jittWeightsFile_ = ps.getUntrackedParameter<std::string>("jittWeightsFile", dataPath_ + "timeWeights" + weightType);
  jittWeightsAftFile_ = ps.getUntrackedParameter<std::string>("jittWeightsAftFile",
                                                              dataPath_ + "timeWeightsAfterGainSwitch" + weightType);
  chi2MatrixFile_ = ps.getUntrackedParameter<std::string>("chi2MatrixFile", dataPath_ + "chi2Matrix" + weightType);
  chi2MatrixAftFile_ =
      ps.getUntrackedParameter<std::string>("chi2MatrixAftFile", dataPath_ + "chi2MatrixAfterGainSwitch" + weightType);

  amplWeights_.resize(nTDCbins_);
  amplWeightsAft_.resize(nTDCbins_);
  pedWeights_.resize(nTDCbins_);
  pedWeightsAft_.resize(nTDCbins_);
  jittWeights_.resize(nTDCbins_);
  jittWeightsAft_.resize(nTDCbins_);
  chi2Matrix_.resize(nTDCbins_);
  chi2MatrixAft_.resize(nTDCbins_);

  // default weights for MGPA shape after pedestal subtraction
  getWeightsFromConfiguration(ps);

  producedEcalPedestals_ = ps.getUntrackedParameter<bool>("producedEcalPedestals", true);
  producedEcalWeights_ = ps.getUntrackedParameter<bool>("producedEcalWeights", true);

  producedEcalGainRatios_ = ps.getUntrackedParameter<bool>("producedEcalGainRatios", true);
  producedEcalADCToGeVConstant_ = ps.getUntrackedParameter<bool>("producedEcalADCToGeVConstant", true);

  producedEcalMappingElectronics_ = ps.getUntrackedParameter<bool>("producedEcalMappingElectronics", true);
  mappingFile_ = ps.getUntrackedParameter<std::string>("mappingFile", "");

  if (producedEcalMappingElectronics_) {
    if (!mappingFile_.empty()) {  // if file provided read channel map
      setWhatProduced(this, &EcalTrivialConditionRetriever::getMappingFromConfiguration);
    } else {
      setWhatProduced(this, &EcalTrivialConditionRetriever::produceEcalMappingElectronics);
    }
    findingRecord<EcalMappingElectronicsRcd>();
  }
  // Alignment from file
  getEBAlignmentFromFile_ = ps.getUntrackedParameter<bool>("getEBAlignmentFromFile", false);
  if (getEBAlignmentFromFile_) {
    EBAlignmentFile_ = ps.getUntrackedParameter<std::string>("EBAlignmentFile", dataPath_ + "EBAlignment.txt");
  }

  getEEAlignmentFromFile_ = ps.getUntrackedParameter<bool>("getEEAlignmentFromFile", false);
  if (getEEAlignmentFromFile_) {
    EEAlignmentFile_ = ps.getUntrackedParameter<std::string>("EEAlignmentFile", dataPath_ + "EEAlignment.txt");
  }

  getESAlignmentFromFile_ = ps.getUntrackedParameter<bool>("getESAlignmentFromFile", false);
  if (getESAlignmentFromFile_)
    ESAlignmentFile_ = ps.getUntrackedParameter<std::string>("ESAlignmentFile", dataPath_ + "ESAlignment.txt");

  verbose_ = ps.getUntrackedParameter<int>("verbose", 0);

  //Tell Producer what we produce
  //setWhatproduce(this);
  if (producedEcalPedestals_)
    setWhatProduced(this, &EcalTrivialConditionRetriever::produceEcalPedestals);

  if (producedEcalWeights_) {
    setWhatProduced(this, &EcalTrivialConditionRetriever::produceEcalWeightXtalGroups);
    setWhatProduced(this, &EcalTrivialConditionRetriever::produceEcalTBWeights);
  }

  if (producedEcalGainRatios_)
    setWhatProduced(this, &EcalTrivialConditionRetriever::produceEcalGainRatios);

  if (producedEcalADCToGeVConstant_)
    setWhatProduced(this, &EcalTrivialConditionRetriever::produceEcalADCToGeVConstant);

  // TimeOffsetConstant
  producedEcalTimeOffsetConstant_ = ps.getUntrackedParameter<bool>("producedEcalTimeOffsetConstant", true);
  //std::cout << " EcalTrivialConditionRetriever : producedEcalTimeOffsetConstant_" << producedEcalTimeOffsetConstant_ << std::endl;
  if (producedEcalTimeOffsetConstant_) {
    setWhatProduced(this, &EcalTrivialConditionRetriever::produceEcalTimeOffsetConstant);
    findingRecord<EcalTimeOffsetConstantRcd>();
  }

  // linear corrections
  producedEcalLinearCorrections_ = ps.getUntrackedParameter<bool>("producedEcalLinearCorrections", true);
  linearCorrectionsFile_ = ps.getUntrackedParameter<std::string>("linearCorrectionsFile", "");

  if (producedEcalLinearCorrections_) {     // user asks to produce constants
    if (!linearCorrectionsFile_.empty()) {  // if file provided read constants
      setWhatProduced(this, &EcalTrivialConditionRetriever::produceEcalLinearCorrections);
    } else {  // set all constants to 1. or smear as specified by user
      setWhatProduced(this, &EcalTrivialConditionRetriever::produceEcalLinearCorrections);
    }
    findingRecord<EcalLinearCorrectionsRcd>();
  }

  // intercalibration constants
  producedEcalIntercalibConstants_ = ps.getUntrackedParameter<bool>("producedEcalIntercalibConstants", true);
  intercalibConstantsFile_ = ps.getUntrackedParameter<std::string>("intercalibConstantsFile", "");

  intercalibConstantsMCFile_ = ps.getUntrackedParameter<std::string>("intercalibConstantsMCFile", "");

  if (producedEcalIntercalibConstants_) {     // user asks to produce constants
    if (!intercalibConstantsFile_.empty()) {  // if file provided read constants
      setWhatProduced(this, &EcalTrivialConditionRetriever::getIntercalibConstantsFromConfiguration);
    } else {  // set all constants to 1. or smear as specified by user
      setWhatProduced(this, &EcalTrivialConditionRetriever::produceEcalIntercalibConstants);
    }
    findingRecord<EcalIntercalibConstantsRcd>();
  }
  // MC intercalibrations
  producedEcalIntercalibConstantsMC_ = ps.getUntrackedParameter<bool>("producedEcalIntercalibConstantsMC", true);

  if (producedEcalIntercalibConstantsMC_) {     // user asks to produce constants
    if (!intercalibConstantsMCFile_.empty()) {  // if file provided read constants
      setWhatProduced(this, &EcalTrivialConditionRetriever::getIntercalibConstantsMCFromConfiguration);
    } else {  // set all constants to 1. or smear as specified by user
      setWhatProduced(this, &EcalTrivialConditionRetriever::produceEcalIntercalibConstantsMC);
    }
    findingRecord<EcalIntercalibConstantsMCRcd>();
  }

  // intercalibration constants
  producedEcalIntercalibErrors_ = ps.getUntrackedParameter<bool>("producedEcalIntercalibErrors", true);
  intercalibErrorsFile_ = ps.getUntrackedParameter<std::string>("intercalibErrorsFile", "");

  if (producedEcalIntercalibErrors_) {     // user asks to produce constants
    if (!intercalibErrorsFile_.empty()) {  // if file provided read constants
      setWhatProduced(this, &EcalTrivialConditionRetriever::getIntercalibErrorsFromConfiguration);
    } else {  // set all constants to 1. or smear as specified by user
      setWhatProduced(this, &EcalTrivialConditionRetriever::produceEcalIntercalibErrors);
    }
    findingRecord<EcalIntercalibErrorsRcd>();
  }

  // time calibration constants
  producedEcalTimeCalibConstants_ = ps.getUntrackedParameter<bool>("producedEcalTimeCalibConstants", true);
  timeCalibConstantsFile_ = ps.getUntrackedParameter<std::string>("timeCalibConstantsFile", "");

  if (producedEcalTimeCalibConstants_) {     // user asks to produce constants
    if (!timeCalibConstantsFile_.empty()) {  // if file provided read constants
      setWhatProduced(this, &EcalTrivialConditionRetriever::getTimeCalibConstantsFromConfiguration);
    } else {  // set all constants to 1. or smear as specified by user
      setWhatProduced(this, &EcalTrivialConditionRetriever::produceEcalTimeCalibConstants);
    }
    findingRecord<EcalTimeCalibConstantsRcd>();
  }

  // time calibration constants
  producedEcalTimeCalibErrors_ = ps.getUntrackedParameter<bool>("producedEcalTimeCalibErrors", true);
  timeCalibErrorsFile_ = ps.getUntrackedParameter<std::string>("timeCalibErrorsFile", "");

  if (producedEcalTimeCalibErrors_) {     // user asks to produce constants
    if (!timeCalibErrorsFile_.empty()) {  // if file provided read constants
      setWhatProduced(this, &EcalTrivialConditionRetriever::getTimeCalibErrorsFromConfiguration);
    } else {  // set all constants to 1. or smear as specified by user
      setWhatProduced(this, &EcalTrivialConditionRetriever::produceEcalTimeCalibErrors);
    }
    findingRecord<EcalTimeCalibErrorsRcd>();
  }

  // sim component shape
  getSimComponentShapeFromFile_ = ps.getUntrackedParameter<bool>("getSimComponentShapeFromFile", false);
  producedEcalSimComponentShape_ = ps.getUntrackedParameter<bool>("producedEcalSimComponentShape", true);
  std::vector<std::string> vComponentShapes;
  EBSimComponentShapeFiles_ =
      ps.getUntrackedParameter<std::vector<std::string> >("EBSimComponentShapeFiles", vComponentShapes);

  if (producedEcalSimComponentShape_) {  // user asks to produce constants
    setWhatProduced(this, &EcalTrivialConditionRetriever::getEcalSimComponentShapeFromConfiguration);
    findingRecord<EcalSimComponentShapeRcd>();
  }

  // sim pulse shape
  getSimPulseShapeFromFile_ = ps.getUntrackedParameter<bool>("getSimPulseShapeFromFile", false);
  producedEcalSimPulseShape_ = ps.getUntrackedParameter<bool>("producedEcalSimPulseShape", true);
  EBSimPulseShapeFile_ = ps.getUntrackedParameter<std::string>("EBSimPulseShapeFile", "");
  EESimPulseShapeFile_ = ps.getUntrackedParameter<std::string>("EESimPulseShapeFile", "");
  APDSimPulseShapeFile_ = ps.getUntrackedParameter<std::string>("APDSimPulseShapeFile", "");

  if (producedEcalSimPulseShape_) {  // user asks to produce constants
    setWhatProduced(this, &EcalTrivialConditionRetriever::getEcalSimPulseShapeFromConfiguration);
    findingRecord<EcalSimPulseShapeRcd>();
  }

  producedEcalPFRecHitThresholds_ = ps.getUntrackedParameter<bool>("producedEcalPFRecHitThresholds", false);

  // new for PFRecHit Thresholds
  pfRecHitFile_ = ps.getUntrackedParameter<std::string>("pFRecHitFile", dataPath_ + "EB_product_TL235.txt");
  pfRecHitFileEE_ = ps.getUntrackedParameter<std::string>("pFRecHitFileEE", dataPath_ + "EE_product_TL235.txt");

  if (producedEcalPFRecHitThresholds_) {  // user asks to produce constants
    if (!pfRecHitFile_.empty()) {         // if file provided read constants
      setWhatProduced(this, &EcalTrivialConditionRetriever::getPFRecHitThresholdsFromConfiguration);
    } else {  // set all constants to 0
      setWhatProduced(this, &EcalTrivialConditionRetriever::produceEcalPFRecHitThresholds);
    }
    findingRecord<EcalPFRecHitThresholdsRcd>();
  }

  // cluster corrections
  producedEcalClusterLocalContCorrParameters_ =
      ps.getUntrackedParameter<bool>("producedEcalClusterLocalContCorrParameters", false);
  producedEcalClusterCrackCorrParameters_ =
      ps.getUntrackedParameter<bool>("producedEcalClusterCrackCorrParameters", false);
  producedEcalClusterEnergyCorrectionParameters_ =
      ps.getUntrackedParameter<bool>("producedEcalClusterEnergyCorrectionParameters", false);
  producedEcalClusterEnergyUncertaintyParameters_ =
      ps.getUntrackedParameter<bool>("producedEcalClusterEnergyUncertaintyParameters", false);
  producedEcalClusterEnergyCorrectionObjectSpecificParameters_ =
      ps.getUntrackedParameter<bool>("producedEcalClusterEnergyCorrectionObjectSpecificParameters", false);
  if (producedEcalClusterLocalContCorrParameters_) {
    setWhatProduced(this, &EcalTrivialConditionRetriever::produceEcalClusterLocalContCorrParameters);
    findingRecord<EcalClusterLocalContCorrParametersRcd>();
  }
  if (producedEcalClusterCrackCorrParameters_) {
    setWhatProduced(this, &EcalTrivialConditionRetriever::produceEcalClusterCrackCorrParameters);
    findingRecord<EcalClusterCrackCorrParametersRcd>();
  }
  if (producedEcalClusterEnergyCorrectionParameters_) {
    setWhatProduced(this, &EcalTrivialConditionRetriever::produceEcalClusterEnergyCorrectionParameters);
    findingRecord<EcalClusterEnergyCorrectionParametersRcd>();
  }
  if (producedEcalClusterEnergyUncertaintyParameters_) {
    setWhatProduced(this, &EcalTrivialConditionRetriever::produceEcalClusterEnergyUncertaintyParameters);
    findingRecord<EcalClusterEnergyUncertaintyParametersRcd>();
  }
  if (producedEcalClusterEnergyCorrectionObjectSpecificParameters_) {
    setWhatProduced(this, &EcalTrivialConditionRetriever::produceEcalClusterEnergyCorrectionObjectSpecificParameters);
    findingRecord<EcalClusterEnergyCorrectionObjectSpecificParametersRcd>();
  }

  // laser correction
  producedEcalLaserCorrection_ = ps.getUntrackedParameter<bool>("producedEcalLaserCorrection", false);
  if (producedEcalLaserCorrection_) {  // user asks to produce constants
    // set all constants to 1. or smear as specified by user
    setWhatProduced(this, &EcalTrivialConditionRetriever::produceEcalLaserAlphas);
    findingRecord<EcalLaserAlphasRcd>();
    getLaserAlphaFromFileEB_ = ps.getUntrackedParameter<bool>("getLaserAlphaFromFileEB", false);
    getLaserAlphaFromFileEE_ = ps.getUntrackedParameter<bool>("getLaserAlphaFromFileEE", false);
    getLaserAlphaFromTypeEB_ = ps.getUntrackedParameter<bool>("getLaserAlphaFromTypeEB", false);
    getLaserAlphaFromTypeEE_ = ps.getUntrackedParameter<bool>("getLaserAlphaFromTypeEE", false);
    edm::LogInfo(" getLaserAlphaFromFileEB_ ") << getLaserAlphaFromFileEB_;
    edm::LogInfo(" getLaserAlphaFromFileEE_ ") << getLaserAlphaFromFileEE_;
    edm::LogInfo(" getLaserAlphaFromTypeEB_ ") << getLaserAlphaFromTypeEB_;
    edm::LogInfo(" getLaserAlphaFromTypeEE_ ") << getLaserAlphaFromTypeEE_;
    if (getLaserAlphaFromFileEB_) {
      EBLaserAlphaFile_ = ps.getUntrackedParameter<std::string>(
          "EBLaserAlphaFile", dataPath_ + "EBLaserAlpha.txt");  // file is used to read the alphas
    }
    if (getLaserAlphaFromFileEE_) {
      EELaserAlphaFile_ = ps.getUntrackedParameter<std::string>(
          "EELaserAlphaFile", dataPath_ + "EELaserAlpha.txt");  // file is used to read the alphas
    }
    if (getLaserAlphaFromTypeEB_) {
      laserAlphaMeanEBR_ = ps.getUntrackedParameter<double>("laserAlphaMeanEBR", 1.55);  // alpha russian crystals in EB
      laserAlphaMeanEBC_ = ps.getUntrackedParameter<double>("laserAlphaMeanEBC", 1.00);  // alpha chinese crystals in EB
      EBLaserAlphaFile_ = ps.getUntrackedParameter<std::string>(
          "EBLaserAlphaFile", dataPath_ + "EBLaserAlpha.txt");  // file to find out which one is russian/chinese
    }
    if (getLaserAlphaFromTypeEE_) {
      laserAlphaMeanEEC_higheta_ = ps.getUntrackedParameter<double>("laserAlphaMeanEEC_higheta",
                                                                    1.00);  // alpha chinese crystals in EE for eta>2.5
      laserAlphaMeanEER_higheta_ = ps.getUntrackedParameter<double>("laserAlphaMeanEER_higheta",
                                                                    1.16);  // alpha russian crystals in EE for eta>2
      laserAlphaMeanEER_ = ps.getUntrackedParameter<double>("laserAlphaMeanEER", 1.16);  // alpha russian crystals in EE
      laserAlphaMeanEEC_ = ps.getUntrackedParameter<double>("laserAlphaMeanEEC", 1.00);  // alpha chinese crystals in EE
      EELaserAlphaFile_ = ps.getUntrackedParameter<std::string>(
          "EELaserAlphaFile",
          dataPath_ + "EELaserAlpha.txt");  // file is used to find out which one is russian or chinese
      EELaserAlphaFile2_ = ps.getUntrackedParameter<std::string>(
          "EELaserAlphaFile2", dataPath_ + "EELaserAlpha2.txt");  // file is used to read the alphas
    }
    setWhatProduced(this, &EcalTrivialConditionRetriever::produceEcalLaserAPDPNRatiosRef);
    findingRecord<EcalLaserAPDPNRatiosRefRcd>();
    setWhatProduced(this, &EcalTrivialConditionRetriever::produceEcalLaserAPDPNRatios);
    findingRecord<EcalLaserAPDPNRatiosRcd>();
  }

  // channel status
  producedEcalChannelStatus_ = ps.getUntrackedParameter<bool>("producedEcalChannelStatus", true);
  channelStatusFile_ = ps.getUntrackedParameter<std::string>("channelStatusFile", "");

  if (producedEcalChannelStatus_) {
    if (!channelStatusFile_.empty()) {  // if file provided read channel map
      setWhatProduced(this, &EcalTrivialConditionRetriever::getChannelStatusFromConfiguration);
    } else {  // set all channels to working -- FIXME might be changed
      setWhatProduced(this, &EcalTrivialConditionRetriever::produceEcalChannelStatus);
    }
    findingRecord<EcalChannelStatusRcd>();
  }
  // DQM channel status
  producedEcalDQMChannelStatus_ = ps.getUntrackedParameter<bool>("producedEcalDQMChannelStatus", true);
  if (producedEcalDQMChannelStatus_) {
    setWhatProduced(this, &EcalTrivialConditionRetriever::produceEcalDQMChannelStatus);
    findingRecord<EcalDQMChannelStatusRcd>();
  }
  // DCS Tower status
  producedEcalDCSTowerStatus_ = ps.getUntrackedParameter<bool>("producedEcalDCSTowerStatus", true);
  if (producedEcalDCSTowerStatus_) {
    setWhatProduced(this, &EcalTrivialConditionRetriever::produceEcalDCSTowerStatus);
    findingRecord<EcalDCSTowerStatusRcd>();
  }
  // DAQ Tower status
  producedEcalDAQTowerStatus_ = ps.getUntrackedParameter<bool>("producedEcalDAQTowerStatus", true);
  if (producedEcalDAQTowerStatus_) {
    setWhatProduced(this, &EcalTrivialConditionRetriever::produceEcalDAQTowerStatus);
    findingRecord<EcalDAQTowerStatusRcd>();
  }
  // DQM Tower status
  producedEcalDQMTowerStatus_ = ps.getUntrackedParameter<bool>("producedEcalDQMTowerStatus", true);
  if (producedEcalDQMTowerStatus_) {
    setWhatProduced(this, &EcalTrivialConditionRetriever::produceEcalDQMTowerStatus);
    findingRecord<EcalDQMTowerStatusRcd>();
  }

  // trigger channel status
  producedEcalTrgChannelStatus_ = ps.getUntrackedParameter<bool>("producedEcalTrgChannelStatus", true);
  trgChannelStatusFile_ = ps.getUntrackedParameter<std::string>("trgChannelStatusFile", "");

  if (producedEcalTrgChannelStatus_) {
    if (!trgChannelStatusFile_.empty()) {  // if file provided read channel map
      setWhatProduced(this, &EcalTrivialConditionRetriever::getTrgChannelStatusFromConfiguration);
    } else {  // set all channels to working -- FIXME might be changed
      setWhatProduced(this, &EcalTrivialConditionRetriever::produceEcalTrgChannelStatus);
    }
    findingRecord<EcalTPGCrystalStatusRcd>();
  }

  // Alignment
  producedEcalAlignmentEB_ = ps.getUntrackedParameter<bool>("producedEcalAlignmentEB", true);
  if (producedEcalAlignmentEB_) {
    setWhatProduced(this, &EcalTrivialConditionRetriever::produceEcalAlignmentEB);
    findingRecord<EBAlignmentRcd>();
  }
  producedEcalAlignmentEE_ = ps.getUntrackedParameter<bool>("producedEcalAlignmentEE", true);
  if (producedEcalAlignmentEE_) {
    setWhatProduced(this, &EcalTrivialConditionRetriever::produceEcalAlignmentEE);
    findingRecord<EEAlignmentRcd>();
  }
  producedEcalAlignmentES_ = ps.getUntrackedParameter<bool>("producedEcalAlignmentES", true);
  if (producedEcalAlignmentES_) {
    setWhatProduced(this, &EcalTrivialConditionRetriever::produceEcalAlignmentES);
    findingRecord<ESAlignmentRcd>();
  }
  //Tell Finder what records we find
  if (producedEcalPedestals_)
    findingRecord<EcalPedestalsRcd>();

  if (producedEcalWeights_) {
    findingRecord<EcalWeightXtalGroupsRcd>();
    findingRecord<EcalTBWeightsRcd>();
  }

  if (producedEcalGainRatios_)
    findingRecord<EcalGainRatiosRcd>();

  if (producedEcalADCToGeVConstant_)
    findingRecord<EcalADCToGeVConstantRcd>();

  producedEcalSampleMask_ = ps.getUntrackedParameter<bool>("producedEcalSampleMask", true);
  if (producedEcalSampleMask_) {
    setWhatProduced(this, &EcalTrivialConditionRetriever::produceEcalSampleMask);
    findingRecord<EcalSampleMaskRcd>();
  }
  producedEcalTimeBiasCorrections_ = ps.getUntrackedParameter<bool>("producedEcalTimeBiasCorrections", false);
  if (producedEcalTimeBiasCorrections_) {
    setWhatProduced(this, &EcalTrivialConditionRetriever::produceEcalTimeBiasCorrections);
    findingRecord<EcalTimeBiasCorrectionsRcd>();
  }
  producedEcalSamplesCorrelation_ = ps.getUntrackedParameter<bool>("producedEcalSamplesCorrelation", false);
  if (producedEcalSamplesCorrelation_) {
    setWhatProduced(this, &EcalTrivialConditionRetriever::produceEcalSamplesCorrelation);
    findingRecord<EcalSamplesCorrelationRcd>();
    getSamplesCorrelationFromFile_ = ps.getUntrackedParameter<bool>("getSamplesCorrelationFromFile", false);
    if (getSamplesCorrelationFromFile_) {
      SamplesCorrelationFile_ =
          ps.getUntrackedParameter<std::string>("SamplesCorrelationFile", "EcalSamplesCorrelation.txt");
    }
  }
}

EcalTrivialConditionRetriever::~EcalTrivialConditionRetriever() {}

//
// member functions
//
void EcalTrivialConditionRetriever::setIntervalFor(const edm::eventsetup::EventSetupRecordKey& rk,
                                                   const edm::IOVSyncValue& iTime,
                                                   edm::ValidityInterval& oValidity) {
  if (verbose_ >= 1)
    edm::LogInfo("EcalTrivialConditionRetriever::setIntervalFor(): record key = ")
        << rk.name() << "\ttime: " << iTime.time().value();
  //For right now, we will just use an infinite interval of validity
  oValidity = edm::ValidityInterval(edm::IOVSyncValue::beginOfTime(), edm::IOVSyncValue::endOfTime());
}

//produce methods
std::unique_ptr<EcalPedestals> EcalTrivialConditionRetriever::produceEcalPedestals(const EcalPedestalsRcd&) {
  auto peds = std::make_unique<EcalPedestals>();
  EcalPedestals::Item EBitem;
  EcalPedestals::Item EEitem;

  EBitem.mean_x1 = EBpedMeanX1_;
  EBitem.rms_x1 = EBpedRMSX1_;
  EBitem.mean_x6 = EBpedMeanX6_;
  EBitem.rms_x6 = EBpedRMSX6_;
  EBitem.mean_x12 = EBpedMeanX12_;
  EBitem.rms_x12 = EBpedRMSX12_;

  EEitem.mean_x1 = EEpedMeanX1_;
  EEitem.rms_x1 = EEpedRMSX1_;
  EEitem.mean_x6 = EEpedMeanX6_;
  EEitem.rms_x6 = EEpedRMSX6_;
  EEitem.mean_x12 = EEpedMeanX12_;
  EEitem.rms_x12 = EEpedRMSX12_;

  for (int iEta = -EBDetId::MAX_IETA; iEta <= EBDetId::MAX_IETA; ++iEta) {
    if (iEta == 0)
      continue;

    if (totLumi_ > 0) {
      double eta = EBDetId::approxEta(EBDetId(iEta, 1));

      EnergyResolutionVsLumi ageing;
      ageing.setLumi(totLumi_);
      ageing.setInstLumi(instLumi_);
      eta = fabs(eta);
      double noisefactor = ageing.calcnoiseIncreaseADC(eta);

      EBitem.rms_x1 = EBpedRMSX1_ * noisefactor;
      EBitem.rms_x6 = EBpedRMSX6_ * noisefactor;
      EBitem.rms_x12 = EBpedRMSX12_ * noisefactor;
      edm::LogInfo("rms ped at eta:") << eta << " =" << EBitem.rms_x12;
    }

    for (int iPhi = EBDetId::MIN_IPHI; iPhi <= EBDetId::MAX_IPHI; ++iPhi) {
      // make an EBDetId since we need EBDetId::rawId() to be used as the key for the pedestals
      if (EBDetId::validDetId(iEta, iPhi)) {
        EBDetId ebdetid(iEta, iPhi);
        peds->insert(std::make_pair(ebdetid.rawId(), EBitem));
      }
    }
  }

  for (int iX = EEDetId::IX_MIN; iX <= EEDetId::IX_MAX; ++iX) {
    for (int iY = EEDetId::IY_MIN; iY <= EEDetId::IY_MAX; ++iY) {
      // make an EEDetId since we need EEDetId::rawId() to be used as the key for the pedestals
      if (EEDetId::validDetId(iX, iY, 1)) {
        EEDetId eedetidpos(iX, iY, 1);
        peds->insert(std::make_pair(eedetidpos.rawId(), EEitem));
      }
      if (EEDetId::validDetId(iX, iY, -1)) {
        EEDetId eedetidneg(iX, iY, -1);
        peds->insert(std::make_pair(eedetidneg.rawId(), EEitem));
      }
    }
  }

  //return std::unique_ptr<EcalPedestals>( peds );
  return peds;
}

std::unique_ptr<EcalWeightXtalGroups> EcalTrivialConditionRetriever::produceEcalWeightXtalGroups(
    const EcalWeightXtalGroupsRcd&) {
  auto xtalGroups = std::make_unique<EcalWeightXtalGroups>();
  EcalXtalGroupId defaultGroupId(1);
  for (int ieta = -EBDetId::MAX_IETA; ieta <= EBDetId::MAX_IETA; ++ieta) {
    if (ieta == 0)
      continue;
    for (int iphi = EBDetId::MIN_IPHI; iphi <= EBDetId::MAX_IPHI; ++iphi) {
      // make an EBDetId since we need EBDetId::rawId() to be used as the key for the pedestals
      if (EBDetId::validDetId(ieta, iphi)) {
        EBDetId ebid(ieta, iphi);
        //	  xtalGroups->setValue(ebid.rawId(), EcalXtalGroupId(ieta) ); // define rings in eta
        xtalGroups->setValue(ebid.rawId(), defaultGroupId);  // define rings in eta
      }
    }
  }

  for (int iX = EEDetId::IX_MIN; iX <= EEDetId::IX_MAX; ++iX) {
    for (int iY = EEDetId::IY_MIN; iY <= EEDetId::IY_MAX; ++iY) {
      // make an EEDetId since we need EEDetId::rawId() to be used as the key for the pedestals
      if (EEDetId::validDetId(iX, iY, 1)) {
        EEDetId eedetidpos(iX, iY, 1);
        xtalGroups->setValue(eedetidpos.rawId(), defaultGroupId);
      }
      if (EEDetId::validDetId(iX, iY, -1)) {
        EEDetId eedetidneg(iX, iY, -1);
        xtalGroups->setValue(eedetidneg.rawId(), defaultGroupId);
      }
    }
  }
  return xtalGroups;
}

std::unique_ptr<EcalLinearCorrections> EcalTrivialConditionRetriever::produceEcalLinearCorrections(
    const EcalLinearCorrectionsRcd&) {
  auto ical = std::make_unique<EcalLinearCorrections>();

  for (int ieta = -EBDetId::MAX_IETA; ieta <= EBDetId::MAX_IETA; ++ieta) {
    if (ieta == 0)
      continue;
    for (int iphi = EBDetId::MIN_IPHI; iphi <= EBDetId::MAX_IPHI; ++iphi) {
      if (EBDetId::validDetId(ieta, iphi)) {
        EBDetId ebid(ieta, iphi);
        double r = (double)std::rand() / (double(RAND_MAX) + double(1));

        EcalLinearCorrections::Values pairAPDPN;
        pairAPDPN.p1 = linCorrMean_ + r * linCorrSigma_;
        pairAPDPN.p2 = linCorrMean_ + r * linCorrSigma_;
        pairAPDPN.p3 = linCorrMean_ + r * linCorrSigma_;
        ical->setValue(ebid, pairAPDPN);
      }
    }
  }

  for (int iX = EEDetId::IX_MIN; iX <= EEDetId::IX_MAX; ++iX) {
    for (int iY = EEDetId::IY_MIN; iY <= EEDetId::IY_MAX; ++iY) {
      // make an EEDetId since we need EEDetId::rawId() to be used as the key for the pedestals
      if (EEDetId::validDetId(iX, iY, 1)) {
        double r = (double)std::rand() / (double(RAND_MAX) + double(1));
        EEDetId eedetidpos(iX, iY, 1);

        EcalLinearCorrections::Values pairAPDPN;
        pairAPDPN.p1 = linCorrMean_ + r * linCorrSigma_;
        pairAPDPN.p2 = linCorrMean_ + r * linCorrSigma_;
        pairAPDPN.p3 = linCorrMean_ + r * linCorrSigma_;

        ical->setValue(eedetidpos, pairAPDPN);
      }

      if (EEDetId::validDetId(iX, iY, -1)) {
        double r1 = (double)std::rand() / (double(RAND_MAX) + double(1));
        EEDetId eedetidneg(iX, iY, -1);

        EcalLinearCorrections::Values pairAPDPN;
        pairAPDPN.p1 = linCorrMean_ + r1 * linCorrSigma_;
        pairAPDPN.p2 = linCorrMean_ + r1 * linCorrSigma_;
        pairAPDPN.p3 = linCorrMean_ + r1 * linCorrSigma_;

        ical->setValue(eedetidneg, pairAPDPN);
      }
    }
  }

  EcalLinearCorrections::Times TimeStamp;
  //  for(int i=1; i<=92; i++){
  for (int i = 0; i < 92; i++) {
    TimeStamp.t1 = Timestamp(linearTime1_);
    if (linearTime2_ == 0) {
      TimeStamp.t2 = Timestamp(edm::Timestamp::endOfTime().value());
    } else {
      TimeStamp.t2 = Timestamp(linearTime2_);
    }
    if (linearTime3_ == 0) {
      TimeStamp.t3 = Timestamp(edm::Timestamp::endOfTime().value());
    } else {
      TimeStamp.t3 = Timestamp(linearTime3_);
    }

    ical->setTime(i, TimeStamp);
  }

  return ical;
}

// new for the PF Rec Hit Thresholds

std::unique_ptr<EcalPFRecHitThresholds> EcalTrivialConditionRetriever::produceEcalPFRecHitThresholds(
    const EcalPFRecHitThresholdsRcd&) {
  auto ical = std::make_unique<EcalPFRecHitThresholds>();

  for (int ieta = -EBDetId::MAX_IETA; ieta <= EBDetId::MAX_IETA; ++ieta) {
    if (ieta == 0)
      continue;
    for (int iphi = EBDetId::MIN_IPHI; iphi <= EBDetId::MAX_IPHI; ++iphi) {
      // make an EBDetId since we need EBDetId::rawId() to be used as the key for the pedestals
      if (EBDetId::validDetId(ieta, iphi)) {
        EBDetId ebid(ieta, iphi);
        ical->setValue(ebid.rawId(), pfRecHitThresholdsEB_);
      }
    }
  }

  for (int iX = EEDetId::IX_MIN; iX <= EEDetId::IX_MAX; ++iX) {
    for (int iY = EEDetId::IY_MIN; iY <= EEDetId::IY_MAX; ++iY) {
      // make an EEDetId since we need EEDetId::rawId() to be used as the key for the pedestals
      if (EEDetId::validDetId(iX, iY, 1)) {
        EEDetId eedetidpos(iX, iY, 1);
        ical->setValue(eedetidpos.rawId(), pfRecHitThresholdsEE_);
      }
      if (EEDetId::validDetId(iX, iY, -1)) {
        EEDetId eedetidneg(iX, iY, -1);
        ical->setValue(eedetidneg.rawId(), pfRecHitThresholdsEE_);
      }
    }
  }

  return ical;
}

//------------------------------

std::unique_ptr<EcalIntercalibConstants> EcalTrivialConditionRetriever::produceEcalIntercalibConstants(
    const EcalIntercalibConstantsRcd&) {
  auto ical = std::make_unique<EcalIntercalibConstants>();

  for (int ieta = -EBDetId::MAX_IETA; ieta <= EBDetId::MAX_IETA; ++ieta) {
    if (ieta == 0)
      continue;
    for (int iphi = EBDetId::MIN_IPHI; iphi <= EBDetId::MAX_IPHI; ++iphi) {
      // make an EBDetId since we need EBDetId::rawId() to be used as the key for the pedestals
      if (EBDetId::validDetId(ieta, iphi)) {
        EBDetId ebid(ieta, iphi);
        double r = (double)std::rand() / (double(RAND_MAX) + double(1));
        ical->setValue(ebid.rawId(), intercalibConstantMean_ + r * intercalibConstantSigma_);
      }
    }
  }

  for (int iX = EEDetId::IX_MIN; iX <= EEDetId::IX_MAX; ++iX) {
    for (int iY = EEDetId::IY_MIN; iY <= EEDetId::IY_MAX; ++iY) {
      // make an EEDetId since we need EEDetId::rawId() to be used as the key for the pedestals
      if (EEDetId::validDetId(iX, iY, 1)) {
        double r = (double)std::rand() / (double(RAND_MAX) + double(1));
        EEDetId eedetidpos(iX, iY, 1);
        ical->setValue(eedetidpos.rawId(), intercalibConstantMean_ + r * intercalibConstantSigma_);
      }
      if (EEDetId::validDetId(iX, iY, -1)) {
        double r1 = (double)std::rand() / (double(RAND_MAX) + double(1));
        EEDetId eedetidneg(iX, iY, -1);
        ical->setValue(eedetidneg.rawId(), intercalibConstantMean_ + r1 * intercalibConstantSigma_);
      }
    }
  }

  return ical;
}

std::unique_ptr<EcalIntercalibConstantsMC> EcalTrivialConditionRetriever::produceEcalIntercalibConstantsMC(
    const EcalIntercalibConstantsMCRcd&) {
  auto ical = std::make_unique<EcalIntercalibConstantsMC>();

  for (int ieta = -EBDetId::MAX_IETA; ieta <= EBDetId::MAX_IETA; ++ieta) {
    if (ieta == 0)
      continue;
    for (int iphi = EBDetId::MIN_IPHI; iphi <= EBDetId::MAX_IPHI; ++iphi) {
      // make an EBDetId since we need EBDetId::rawId() to be used as the key for the pedestals
      if (EBDetId::validDetId(ieta, iphi)) {
        EBDetId ebid(ieta, iphi);
        double r = (double)std::rand() / (double(RAND_MAX) + double(1));
        ical->setValue(ebid.rawId(), intercalibConstantMeanMC_ + r * intercalibConstantSigmaMC_);
      }
    }
  }

  for (int iX = EEDetId::IX_MIN; iX <= EEDetId::IX_MAX; ++iX) {
    for (int iY = EEDetId::IY_MIN; iY <= EEDetId::IY_MAX; ++iY) {
      // make an EEDetId since we need EEDetId::rawId() to be used as the key for the pedestals
      if (EEDetId::validDetId(iX, iY, 1)) {
        double r = (double)std::rand() / (double(RAND_MAX) + double(1));
        EEDetId eedetidpos(iX, iY, 1);
        ical->setValue(eedetidpos.rawId(), intercalibConstantMeanMC_ + r * intercalibConstantSigmaMC_);
      }
      if (EEDetId::validDetId(iX, iY, -1)) {
        double r1 = (double)std::rand() / (double(RAND_MAX) + double(1));
        EEDetId eedetidneg(iX, iY, -1);
        ical->setValue(eedetidneg.rawId(), intercalibConstantMeanMC_ + r1 * intercalibConstantSigmaMC_);
      }
    }
  }

  return ical;
}

std::unique_ptr<EcalIntercalibErrors> EcalTrivialConditionRetriever::produceEcalIntercalibErrors(
    const EcalIntercalibErrorsRcd&) {
  auto ical = std::make_unique<EcalIntercalibErrors>();

  for (int ieta = -EBDetId::MAX_IETA; ieta <= EBDetId::MAX_IETA; ++ieta) {
    if (ieta == 0)
      continue;
    for (int iphi = EBDetId::MIN_IPHI; iphi <= EBDetId::MAX_IPHI; ++iphi) {
      // make an EBDetId since we need EBDetId::rawId() to be used as the key for the pedestals
      if (EBDetId::validDetId(ieta, iphi)) {
        EBDetId ebid(ieta, iphi);
        ical->setValue(ebid.rawId(), intercalibErrorMean_);
      }
    }
  }

  for (int iX = EEDetId::IX_MIN; iX <= EEDetId::IX_MAX; ++iX) {
    for (int iY = EEDetId::IY_MIN; iY <= EEDetId::IY_MAX; ++iY) {
      // make an EEDetId since we need EEDetId::rawId() to be used as the key for the pedestals
      if (EEDetId::validDetId(iX, iY, 1)) {
        EEDetId eedetidpos(iX, iY, 1);
        ical->setValue(eedetidpos.rawId(), intercalibErrorMean_);
      }
      if (EEDetId::validDetId(iX, iY, -1)) {
        EEDetId eedetidneg(iX, iY, -1);
        ical->setValue(eedetidneg.rawId(), intercalibErrorMean_);
      }
    }
  }

  return ical;
}

std::unique_ptr<EcalTimeCalibConstants> EcalTrivialConditionRetriever::produceEcalTimeCalibConstants(
    const EcalTimeCalibConstantsRcd&) {
  auto ical = std::make_unique<EcalTimeCalibConstants>();

  for (int ieta = -EBDetId::MAX_IETA; ieta <= EBDetId::MAX_IETA; ++ieta) {
    if (ieta == 0)
      continue;
    for (int iphi = EBDetId::MIN_IPHI; iphi <= EBDetId::MAX_IPHI; ++iphi) {
      // make an EBDetId since we need EBDetId::rawId() to be used as the key for the pedestals
      if (EBDetId::validDetId(ieta, iphi)) {
        EBDetId ebid(ieta, iphi);
        double r = (double)std::rand() / (double(RAND_MAX) + double(1));
        ical->setValue(ebid.rawId(), timeCalibConstantMean_ + r * timeCalibConstantSigma_);
      }
    }
  }

  for (int iX = EEDetId::IX_MIN; iX <= EEDetId::IX_MAX; ++iX) {
    for (int iY = EEDetId::IY_MIN; iY <= EEDetId::IY_MAX; ++iY) {
      // make an EEDetId since we need EEDetId::rawId() to be used as the key for the pedestals
      if (EEDetId::validDetId(iX, iY, 1)) {
        double r = (double)std::rand() / (double(RAND_MAX) + double(1));
        EEDetId eedetidpos(iX, iY, 1);
        ical->setValue(eedetidpos.rawId(), timeCalibConstantMean_ + r * timeCalibConstantSigma_);
      }
      if (EEDetId::validDetId(iX, iY, -1)) {
        double r1 = (double)std::rand() / (double(RAND_MAX) + double(1));
        EEDetId eedetidneg(iX, iY, -1);
        ical->setValue(eedetidneg.rawId(), timeCalibConstantMean_ + r1 * timeCalibConstantSigma_);
      }
    }
  }

  return ical;
}

std::unique_ptr<EcalTimeCalibErrors> EcalTrivialConditionRetriever::produceEcalTimeCalibErrors(
    const EcalTimeCalibErrorsRcd&) {
  auto ical = std::make_unique<EcalTimeCalibErrors>();

  for (int ieta = -EBDetId::MAX_IETA; ieta <= EBDetId::MAX_IETA; ++ieta) {
    if (ieta == 0)
      continue;
    for (int iphi = EBDetId::MIN_IPHI; iphi <= EBDetId::MAX_IPHI; ++iphi) {
      // make an EBDetId since we need EBDetId::rawId() to be used as the key for the pedestals
      if (EBDetId::validDetId(ieta, iphi)) {
        EBDetId ebid(ieta, iphi);
        ical->setValue(ebid.rawId(), timeCalibErrorMean_);
      }
    }
  }

  for (int iX = EEDetId::IX_MIN; iX <= EEDetId::IX_MAX; ++iX) {
    for (int iY = EEDetId::IY_MIN; iY <= EEDetId::IY_MAX; ++iY) {
      // make an EEDetId since we need EEDetId::rawId() to be used as the key for the pedestals
      if (EEDetId::validDetId(iX, iY, 1)) {
        EEDetId eedetidpos(iX, iY, 1);
        ical->setValue(eedetidpos.rawId(), timeCalibErrorMean_);
      }
      if (EEDetId::validDetId(iX, iY, -1)) {
        EEDetId eedetidneg(iX, iY, -1);
        ical->setValue(eedetidneg.rawId(), timeCalibErrorMean_);
      }
    }
  }

  return ical;
}

std::unique_ptr<EcalTimeOffsetConstant> EcalTrivialConditionRetriever::produceEcalTimeOffsetConstant(
    const EcalTimeOffsetConstantRcd&) {
  edm::LogInfo(" produceEcalTimeOffsetConstant: ");
  edm::LogInfo("  EB ") << timeOffsetEBConstant_ << " EE " << timeOffsetEEConstant_;
  return std::make_unique<EcalTimeOffsetConstant>(timeOffsetEBConstant_, timeOffsetEEConstant_);
}

std::unique_ptr<EcalGainRatios> EcalTrivialConditionRetriever::produceEcalGainRatios(const EcalGainRatiosRcd&) {
  auto gratio = std::make_unique<EcalGainRatios>();
  EcalMGPAGainRatio gr;
  gr.setGain12Over6(gainRatio12over6_);
  gr.setGain6Over1(gainRatio6over1_);

  for (int ieta = -EBDetId::MAX_IETA; ieta <= EBDetId::MAX_IETA; ++ieta) {
    if (ieta == 0)
      continue;
    for (int iphi = EBDetId::MIN_IPHI; iphi <= EBDetId::MAX_IPHI; ++iphi) {
      if (EBDetId::validDetId(ieta, iphi)) {
        EBDetId ebid(ieta, iphi);
        gratio->setValue(ebid.rawId(), gr);
      }
    }
  }

  for (int iX = EEDetId::IX_MIN; iX <= EEDetId::IX_MAX; ++iX) {
    for (int iY = EEDetId::IY_MIN; iY <= EEDetId::IY_MAX; ++iY) {
      // make an EEDetId since we need EEDetId::rawId() to be used as the key for the pedestals
      if (EEDetId::validDetId(iX, iY, 1)) {
        EEDetId eedetidpos(iX, iY, 1);
        gratio->setValue(eedetidpos.rawId(), gr);
      }
      if (EEDetId::validDetId(iX, iY, -1)) {
        EEDetId eedetidneg(iX, iY, -1);
        gratio->setValue(eedetidneg.rawId(), gr);
      }
    }
  }

  return gratio;
}

std::unique_ptr<EcalADCToGeVConstant> EcalTrivialConditionRetriever::produceEcalADCToGeVConstant(
    const EcalADCToGeVConstantRcd&) {
  return std::make_unique<EcalADCToGeVConstant>(adcToGeVEBConstant_, adcToGeVEEConstant_);
}

std::unique_ptr<EcalTBWeights> EcalTrivialConditionRetriever::produceEcalTBWeights(const EcalTBWeightsRcd&) {
  // create weights for the test-beam
  auto tbwgt = std::make_unique<EcalTBWeights>();

  // create weights for each distinct group ID
  //  int nMaxTDC = 10;
  //   for(int igrp=-EBDetId::MAX_IETA; igrp<=EBDetId::MAX_IETA; ++igrp) {
  //     if(igrp==0) continue;
  int igrp = 1;
  for (int itdc = 1; itdc <= nTDCbins_; ++itdc) {
    // generate random number
    //    double r = (double)std::rand()/( double(RAND_MAX)+double(1) );

    // make a new set of weights
    EcalWeightSet wgt;
    //typedef std::vector< std::vector<EcalWeight> > EcalWeightSet::EcalWeightMatrix;
    EcalWeightSet::EcalWeightMatrix& mat1 = wgt.getWeightsBeforeGainSwitch();
    EcalWeightSet::EcalWeightMatrix& mat2 = wgt.getWeightsAfterGainSwitch();

    //     if(verbose_>=1) {
    //       std::cout << "initial size of mat1: " << mat1.size() << std::endl;
    //       std::cout << "initial size of mat2: " << mat2.size() << std::endl;
    //     }

    // generate random numbers to use as weights
    /**
       for(size_t i=0; i<3; ++i) {
       std::vector<EcalWeight> tv1, tv2;
       for(size_t j=0; j<10; ++j) {
       double ww = igrp*itdc*r + i*10. + j;
       //std::cout << "row: " << i << " col: " << j << " -  val: " << ww  << std::endl;
       tv1.push_back( EcalWeight(ww) );
       tv2.push_back( EcalWeight(100+ww) );
       }
       mat1.push_back(tv1);
       mat2.push_back(tv2);
       }
    **/

    // use values provided by user
    mat1.Place_in_row(amplWeights_[itdc - 1], 0, 0);
    mat1.Place_in_row(pedWeights_[itdc - 1], 1, 0);
    mat1.Place_in_row(jittWeights_[itdc - 1], 2, 0);

    // wdights after gain switch
    mat2.Place_in_row(amplWeightsAft_[itdc - 1], 0, 0);
    mat2.Place_in_row(pedWeightsAft_[itdc - 1], 1, 0);
    mat2.Place_in_row(jittWeightsAft_[itdc - 1], 2, 0);

    // fill the chi2 matrcies with random numbers
    //    r = (double)std::rand()/( double(RAND_MAX)+double(1) );
    EcalWeightSet::EcalChi2WeightMatrix& mat3 = wgt.getChi2WeightsBeforeGainSwitch();
    EcalWeightSet::EcalChi2WeightMatrix& mat4 = wgt.getChi2WeightsAfterGainSwitch();
    mat3 = chi2Matrix_[itdc - 1];
    mat4 = chi2MatrixAft_[itdc - 1];

    //     for(size_t i=0; i<10; ++i)
    //       {
    // 	mat3.push_back(chi2Matrix_[itdc-1][i]);
    // 	mat4.push_back(chi2MatrixAft_[itdc-1][i]);
    //       }
    //       std::vector<EcalWeight> tv1, tv2;
    //       for(size_t j=0; j<10; ++j) {
    // 	double ww = igrp*itdc*r + i*10. + j;
    // 	tv1.push_back( EcalWeight(1000+ww) );
    // 	tv2.push_back( EcalWeight(1000+100+ww) );
    //       }

    //     if(verbose_>=1) {
    //       std::cout << "group: " << igrp << " TDC: " << itdc
    // 		<< " mat1: " << mat1.size() << " mat2: " << mat2.size()
    // 		<< " mat3: " << mat3.size() << " mat4: " << mat4.size()
    // 		<< std::endl;
    //     }

    // put the weight in the container
    tbwgt->setValue(std::make_pair(igrp, itdc), wgt);
  }
  //   }
  return tbwgt;
}

// cluster functions/corrections
std::unique_ptr<EcalClusterLocalContCorrParameters>
EcalTrivialConditionRetriever::produceEcalClusterLocalContCorrParameters(const EcalClusterLocalContCorrParametersRcd&) {
  auto ipar = std::make_unique<EcalClusterLocalContCorrParameters>();
  for (size_t i = 0; i < localContCorrParameters_.size(); ++i) {
    ipar->params().push_back(localContCorrParameters_[i]);
  }
  return ipar;
}
std::unique_ptr<EcalClusterCrackCorrParameters> EcalTrivialConditionRetriever::produceEcalClusterCrackCorrParameters(
    const EcalClusterCrackCorrParametersRcd&) {
  auto ipar = std::make_unique<EcalClusterCrackCorrParameters>();
  for (size_t i = 0; i < crackCorrParameters_.size(); ++i) {
    ipar->params().push_back(crackCorrParameters_[i]);
  }
  return ipar;
}
std::unique_ptr<EcalClusterEnergyCorrectionParameters>
EcalTrivialConditionRetriever::produceEcalClusterEnergyCorrectionParameters(
    const EcalClusterEnergyCorrectionParametersRcd&) {
  auto ipar = std::make_unique<EcalClusterEnergyCorrectionParameters>();
  for (size_t i = 0; i < energyCorrectionParameters_.size(); ++i) {
    ipar->params().push_back(energyCorrectionParameters_[i]);
  }
  return ipar;
}
std::unique_ptr<EcalClusterEnergyUncertaintyParameters>
EcalTrivialConditionRetriever::produceEcalClusterEnergyUncertaintyParameters(
    const EcalClusterEnergyUncertaintyParametersRcd&) {
  auto ipar = std::make_unique<EcalClusterEnergyUncertaintyParameters>();
  for (size_t i = 0; i < energyUncertaintyParameters_.size(); ++i) {
    ipar->params().push_back(energyUncertaintyParameters_[i]);
  }
  return ipar;
}
std::unique_ptr<EcalClusterEnergyCorrectionObjectSpecificParameters>
EcalTrivialConditionRetriever::produceEcalClusterEnergyCorrectionObjectSpecificParameters(
    const EcalClusterEnergyCorrectionObjectSpecificParametersRcd&) {
  auto ipar = std::make_unique<EcalClusterEnergyCorrectionObjectSpecificParameters>();
  for (size_t i = 0; i < energyCorrectionObjectSpecificParameters_.size(); ++i) {
    ipar->params().push_back(energyCorrectionObjectSpecificParameters_[i]);
  }
  return ipar;
}

/* 
// laser records
std::unique_ptr<EcalLaserAlphas>
EcalTrivialConditionRetriever::produceEcalLaserAlphas( const EcalLaserAlphasRcd& )
{

  std::cout << " produceEcalLaserAlphas " << std::endl;
  auto ical = std::make_unique<EcalLaserAlphas>();

  // get Barrel alpha from type
  if(getLaserAlphaFromTypeEB_) {
    std::ifstream fEB(edm::FileInPath(EBLaserAlphaFile_).fullPath().c_str());
    int SMpos[36] = {-10, 4, -7, -16, 6, -9, 11, -17, 5, 18, 3, -8, 1, -3, -13, 14, -6, 2,
		     15, -18, 8, 17, -2, 9, -1, 10, -5, 7, -12, -11, 16, -4, -15, -14, 12, 13};
    // check!
    int SMCal[36] = {12,17,10, 1, 8, 4,27,20,23,25, 6,34,35,15,18,30,21, 9,
		     24,22,13,31,26,16, 2,11, 5, 0,29,28,14,33,32, 3, 7,19};

    for(int SMcons = 0; SMcons < 36; SMcons++) {
      int SM = SMpos[SMcons];
      if(SM < 0) SM = 17 + abs(SM);
      else SM--;
      if(SMCal[SM] != SMcons)
	 std::cout << " SM pb : read SM " <<  SMcons<< " SMpos " << SM
		   << " SMCal " << SMCal[SM] << std::endl;
    }

    std::string type, batch;
    int readSM, pos, bar, bar2;
    float alpha = 0;
    for(int SMcons = 0; SMcons < 36; SMcons++) {
      int SM = SMpos[SMcons];
      for(int ic = 0; ic < 1700; ic++) {
	fEB >> readSM >> pos >> bar >>  bar2 >> type >> batch;

	if(readSM != SMcons || pos != ic + 1) 
	  std::cout << " barrel read pb read SM " << readSM << " const SM " << SMcons
		    << " read pos " << pos << " ic " << ic << std::endl;
	if(SM < 0) SM = 18 + abs(SM);
	EBDetId ebdetid(SM, pos, EBDetId::SMCRYSTALMODE);
	if(bar == 33101 || bar == 30301 )
	  alpha = laserAlphaMeanEBR_;
	else if(bar == 33106) {
	  if(bar2 <= 2000)
	    alpha = laserAlphaMeanEBC_;
	  else {
	    std::cout << " problem with barcode first " << bar << " last " << bar2 
		      << " read SM " << readSM << " read pos " << pos << std::endl;
	    alpha = laserAlphaMeanEBR_;
	  }
	}
	ical->setValue( ebdetid, alpha );

	if( ic==1650  ){
	  std::cout << " ic/alpha "<<ic<<"/"<<alpha<<std::endl; 
	}

      }
    }  // loop over SMcons
    fEB.close(); 
    // end laserAlpha from type 
  }    else if(getLaserAlphaFromFileEB_) { 
    // laser alpha from file 
    std::cout <<"Laser alpha for EB will be taken from File"<<std::endl; 
    int ieta, iphi;
    float alpha;
    std::ifstream fEB(edm::FileInPath(EBLaserAlphaFile_).fullPath().c_str());
    //    std::ifstream fEB(EBLaserAlphaFile_.c_str());
    for(int ic = 0; ic < 61200; ic++) {
      fEB >> ieta>> iphi>>alpha;

      if (EBDetId::validDetId(ieta,iphi)) {
	EBDetId ebid(ieta,iphi);
	ical->setValue( ebid, alpha );
	std::cout << " ieta/iphi/alpha "<<ieta<<"/"<<iphi<<"/"<<alpha<<std::endl; 
      }
      if( ieta==10 ){
	std::cout << "I will print some alphas from the file... ieta/iphi/alpha "<<ieta<<"/"<<iphi<<"/"<<alpha<<std::endl; 
      }
    }
    fEB.close(); 

  } else {
    // laser alpha from mean and smearing 
    for(int ieta=-EBDetId::MAX_IETA; ieta<=EBDetId::MAX_IETA; ++ieta) {
      if(ieta==0) continue;
      for(int iphi=EBDetId::MIN_IPHI; iphi<=EBDetId::MAX_IPHI; ++iphi) {
	if (EBDetId::validDetId(ieta,iphi)) {
	  EBDetId ebid(ieta,iphi);
	  double r = (double)std::rand()/( double(RAND_MAX)+double(1) );
	  ical->setValue( ebid, laserAlphaMean_ + r*laserAlphaSigma_ );
	}
      }  // loop over iphi
    }  // loop over ieta
  }   // do not read a file

  std::cout << " produceEcalLaserAlphas EE" << std::endl;
  if(getLaserAlphaFromTypeEE_) {
    std::ifstream fEE(edm::FileInPath(EELaserAlphaFile_).fullPath().c_str());

    for(int crystal = 0; crystal < 14648; crystal++) {
      int x, y ,z, bid, bar, bar2;
      float LY, alpha = 0;
      fEE >> z >> x >> y >> LY >> bid >> bar >> bar2;
      if(x < 1 || x > 100 || y < 1 || y > 100)
	std::cout << " wrong coordinates for barcode " << bar 
		  << " x " << x << " y " << y << " z " << z << std::endl;
      else {
	if(bar == 33201 || (bar == 30399 && bar2 < 568))
	    alpha = laserAlphaMeanEER_;
	else if((bar == 33106 && bar2 > 2000 && bar2 < 4669) 
		|| (bar == 30399 && bar2 > 567))
	  alpha = laserAlphaMeanEEC_;
	else {
	  std::cout << " problem with barcode " << bar << " " << bar2 
		    << " x " << x << " y " << y << " z " << z << std::endl;
	  alpha = laserAlphaMeanEER_;
	}
      } 
      if (EEDetId::validDetId(x, y, z)) {
	EEDetId eedetidpos(x, y, z);
	ical->setValue( eedetidpos, alpha );
      }
      else // should not occur
	std::cout << " problem with EEDetId " << " x " << x << " y " << y << " z " << z << std::endl;
    }  
    fEE.close(); 

    // end laserAlpha from type EE

  } else if (getLaserAlphaFromFileEE_) {

    std::ifstream fEE(edm::FileInPath(EELaserAlphaFile_).fullPath().c_str());

    for(int crystal = 0; crystal < 14648; crystal++) {
      int x, y ,z;
      float alpha = 1;
      fEE >> z >> x >> y >> alpha;
      if(x < 1 || x > 100 || y < 1 || y > 100 || z==0 || z>1 || z<-1 ) {
	std::cout << "ERROR: wrong coordinates for crystal " 
		  << " x " << x << " y " << y << " z " << z << std::endl;
	std::cout << " the format of the file should be z x y alpha " << std::endl;
      } else {
	if (EEDetId::validDetId(x, y, z)) {
	  EEDetId eedetidpos(x, y, z);
	  ical->setValue( eedetidpos, alpha );
	}
	else // should not occur
	  std::cout << " problem with EEDetId " << " x " << x << " y " << y << " z " << z << std::endl;
      }  
    }
    fEE.close(); 

    // end laser alpha from file EE
  }  else {
    // alphas from python config file
    for(int iX=EEDetId::IX_MIN; iX<=EEDetId::IX_MAX ;++iX) {
      for(int iY=EEDetId::IY_MIN; iY<=EEDetId::IY_MAX; ++iY) {
	// make an EEDetId since we need EEDetId::rawId() to be used as the key for the pedestals
	if (EEDetId::validDetId(iX,iY,1)) {
	  double r = (double)std::rand()/( double(RAND_MAX)+double(1) );
	  EEDetId eedetidpos(iX,iY,1);
	  ical->setValue( eedetidpos, laserAlphaMean_ + r*laserAlphaSigma_ );
	}
	if (EEDetId::validDetId(iX,iY,-1)) {
	  double r1 = (double)std::rand()/( double(RAND_MAX)+double(1) );
	  EEDetId eedetidneg(iX,iY,-1);
	  ical->setValue( eedetidneg, laserAlphaMean_ + r1*laserAlphaSigma_ );
	}
      } // loop over iY
    } // loop over iX
  }  
  
  return ical;
}
*/

// laser alphas
std::unique_ptr<EcalLaserAlphas> EcalTrivialConditionRetriever::produceEcalLaserAlphas(const EcalLaserAlphasRcd&) {
  edm::LogInfo(" produceEcalLaserAlphas ");
  auto ical = std::make_unique<EcalLaserAlphas>();

  // get Barrel alpha from type
  if (getLaserAlphaFromTypeEB_) {
    std::ifstream fEB(edm::FileInPath(EBLaserAlphaFile_).fullPath().c_str());
    int SMpos[36] = {-10, 4,   -7, -16, 6,  -9, 11, -17, 5,  18, 3,   -8,  1,  -3, -13, 14,  -6, 2,
                     15,  -18, 8,  17,  -2, 9,  -1, 10,  -5, 7,  -12, -11, 16, -4, -15, -14, 12, 13};
    // check!
    int SMCal[36] = {12, 17, 10, 1,  8,  4,  27, 20, 23, 25, 6,  34, 35, 15, 18, 30, 21, 9,
                     24, 22, 13, 31, 26, 16, 2,  11, 5,  0,  29, 28, 14, 33, 32, 3,  7,  19};

    for (int SMcons = 0; SMcons < 36; SMcons++) {
      int SM = SMpos[SMcons];
      if (SM < 0)
        SM = 17 + abs(SM);
      else
        SM--;
      if (SMCal[SM] != SMcons)
        edm::LogInfo(" SM pb : read SM ") << SMcons << " SMpos " << SM << " SMCal " << SMCal[SM];
    }

    std::string type, batch;
    int readSM, pos, bar, bar2;
    float alpha = 0;
    for (int SMcons = 0; SMcons < 36; SMcons++) {
      int SM = SMpos[SMcons];
      for (int ic = 0; ic < 1700; ic++) {
        fEB >> readSM >> pos >> bar >> bar2 >> type >> batch;

        if (readSM != SMcons || pos != ic + 1)
          edm::LogInfo(" barrel read pb read SM ")
              << readSM << " const SM " << SMcons << " read pos " << pos << " ic " << ic;
        if (SM < 0)
          SM = 18 + abs(SM);
        EBDetId ebdetid(SM, pos, EBDetId::SMCRYSTALMODE);
        if (bar == 33101 || bar == 30301)
          alpha = laserAlphaMeanEBR_;
        else if (bar == 33106) {
          if (bar2 <= 2000)
            alpha = laserAlphaMeanEBC_;
          else {
            edm::LogInfo(" problem with barcode first ")
                << bar << " last " << bar2 << " read SM " << readSM << " read pos " << pos;
            alpha = laserAlphaMeanEBR_;
          }
        }
        ical->setValue(ebdetid, alpha);

        if (ic == 1650) {
          edm::LogInfo(" ic/alpha ") << ic << "/" << alpha;
        }
      }
    }  // loop over SMcons
    fEB.close();
    // end laserAlpha from type
  } else if (getLaserAlphaFromFileEB_) {
    // laser alpha from file
    edm::LogInfo("Laser alpha for EB will be taken from File");
    int ieta, iphi;
    float alpha;
    std::ifstream fEB(edm::FileInPath(EBLaserAlphaFile_).fullPath().c_str());
    //    std::ifstream fEB(EBLaserAlphaFile_.c_str());
    for (int ic = 0; ic < 61200; ic++) {
      fEB >> ieta >> iphi >> alpha;

      if (EBDetId::validDetId(ieta, iphi)) {
        EBDetId ebid(ieta, iphi);
        ical->setValue(ebid, alpha);
        //std::cout << " ieta/iphi/alpha "<<ieta<<"/"<<iphi<<"/"<<alpha<<std::endl;
      }
      if (ieta == 10) {
        edm::LogInfo("I will print some alphas from the file... ieta/iphi/alpha ")
            << ieta << "/" << iphi << "/" << alpha;
      }
    }
    fEB.close();

  } else {
    edm::LogInfo("laser alphas from default values");

    // laser alpha from mean and smearing
    for (int ieta = -EBDetId::MAX_IETA; ieta <= EBDetId::MAX_IETA; ++ieta) {
      if (ieta == 0)
        continue;
      for (int iphi = EBDetId::MIN_IPHI; iphi <= EBDetId::MAX_IPHI; ++iphi) {
        if (EBDetId::validDetId(ieta, iphi)) {
          EBDetId ebid(ieta, iphi);
          double r = (double)std::rand() / (double(RAND_MAX) + double(1));
          ical->setValue(ebid, laserAlphaMean_ + r * laserAlphaSigma_);
        }
      }  // loop over iphi
    }  // loop over ieta
  }  // do not read a file

  edm::LogInfo(" produceEcalLaserAlphas EE");
  // read mean laser per ring per year

  int itype, iring, iyear;
  float laser = 0;
  float las[2][140][6];

  if (getLaserAlphaFromTypeEE_) {
    edm::LogInfo(" EE laser alphas from type");

    std::ifstream fRing(edm::FileInPath(EELaserAlphaFile2_).fullPath().c_str());
    for (int i = 0; i < 1681; i++) {
      fRing >> itype >> iring >> iyear >> laser;
      edm::LogInfo(" ") << itype << " " << iring << " " << iyear << " " << laser << std::endl;
      las[itype][iring][iyear] = laser;
    }
    fRing.close();

    std::ifstream fEE(edm::FileInPath(EELaserAlphaFile_).fullPath().c_str());
    int nxt = 0;
    for (int crystal = 0; crystal < 14648; crystal++) {
      int x, y, z, bid, bar, bar2;
      float LY, alpha = 0;
      fEE >> z >> x >> y >> LY >> bid >> bar >> bar2;
      if (x < 1 || x > 100 || y < 1 || y > 100)
        edm::LogInfo(" wrong coordinates for barcode ") << bar << " x " << x << " y " << y << " z " << z;
      else {
        int iyear = 4;
        int iring = (int)(sqrt(((float)x - 50.5) * ((float)x - 50.5) + ((float)y - 50.5) * ((float)y - 50.5)) + 85);

        double eta = -log(tan(0.5 * atan(sqrt((x - 50.5) * (x - 50.5) + (y - 50.5) * (y - 50.5)) * 2.98 / 328.)));

        if (bar == 33201 || (bar == 30399 && bar2 < 568)) {
          // russian
          alpha = laserAlphaMeanEER_;

          double raggio = 50.5 - sqrt(((float)x - 50.5) * ((float)x - 50.5) + ((float)y - 50.5) * ((float)y - 50.5));

          if (raggio >= 25) {
            alpha = 1.07;
          }
          /*                                                                                                                                                                
            if(raggio>=34){                                                                                                                                                   
                                                                                                                                                                              
              if(eta>2.0) {                                                                                                                                                   
                itype=0;                                                                                                                                                      
                if(las[itype][iring][iyear]!=999){                                                                                                                            
                  alpha=0.8044+0.3555*las[itype][iring][iyear];                                                                                                               
                }                                                                                                                                                             
              }                                                                                                                                                               
                                                                                                                                                                              
            }                                                                                                                                                                 
            */

          if (x == 50)
            edm::LogInfo("R=") << raggio << " x " << x << " y " << y << " z " << z << "eta=" << eta
                               << " alpha=" << alpha << " R";

        } else if ((bar == 33106 && bar2 > 2000 && bar2 < 4669) || (bar == 30399 && bar2 > 567)) {
          // SIC
          itype = 1;
          alpha = laserAlphaMeanEEC_;

          double raggio = 50.5 - sqrt(((float)x - 50.5) * ((float)x - 50.5) + ((float)y - 50.5) * ((float)y - 50.5));

          if (raggio >= 25) {
            alpha = 0.80;
          }
          if (raggio >= 34) {
            float r = sqrt((x - 50.5) * (x - 50.5) + (y - 50.5) * (y - 50.5));
            if (r < 21) {
              nxt = nxt + 1;
              // inner SIC crystals

              if (las[itype][iring][iyear] != 999) {
                alpha = 0.7312 + 0.2688 * las[itype][iring][iyear];
              }
            }
          }
          if (x == 50)
            edm::LogInfo("R=") << raggio << " x " << x << " y " << y << " z " << z << "eta=" << eta
                               << " alpha=" << alpha << " C";

        } else {
          edm::LogInfo(" problem with barcode ") << bar << " " << bar2 << " x " << x << " y " << y << " z " << z;
          alpha = laserAlphaMeanEER_;
        }
      }

      if (EEDetId::validDetId(x, y, z)) {
        EEDetId eedetidpos(x, y, z);
        ical->setValue(eedetidpos, alpha);
        if (x == 50)
          edm::LogInfo(" x ") << x << " y " << y << " alpha " << alpha;
      } else  // should not occur
        edm::LogInfo(" problem with EEDetId ") << " x " << x << " y " << y << " z " << z;
    }
    fEE.close();
    edm::LogInfo("Number of inner SIC crystals with different alpha= ") << nxt;
    // end laserAlpha from type EE

  } else if (getLaserAlphaFromFileEE_) {
    edm::LogInfo(" EE laser alphas from file");

    std::ifstream fEE(edm::FileInPath(EELaserAlphaFile_).fullPath().c_str());

    for (int crystal = 0; crystal < 14648; crystal++) {
      int x, y, z;
      float alpha = 1;
      fEE >> z >> x >> y >> alpha;
      if (x < 1 || x > 100 || y < 1 || y > 100 || z == 0 || z > 1 || z < -1) {
        edm::LogInfo("ERROR: wrong coordinates for crystal ") << " x " << x << " y " << y << " z " << z;
        edm::LogInfo(" the format of the file should be z x y alpha ");
      } else {
        if (EEDetId::validDetId(x, y, z)) {
          EEDetId eedetidpos(x, y, z);
          ical->setValue(eedetidpos, alpha);
        } else  // should not occur
          edm::LogInfo(" problem with EEDetId ") << " x " << x << " y " << y << " z " << z;
      }
    }
    fEE.close();

    // end laser alpha from file EE
  } else {
    // alphas from python config file
    edm::LogInfo(" EE laser alphas from default values");
    for (int iX = EEDetId::IX_MIN; iX <= EEDetId::IX_MAX; ++iX) {
      for (int iY = EEDetId::IY_MIN; iY <= EEDetId::IY_MAX; ++iY) {
        // make an EEDetId since we need EEDetId::rawId() to be used as the key for the pedestals
        if (EEDetId::validDetId(iX, iY, 1)) {
          double r = (double)std::rand() / (double(RAND_MAX) + double(1));
          EEDetId eedetidpos(iX, iY, 1);
          ical->setValue(eedetidpos, laserAlphaMean_ + r * laserAlphaSigma_);
        }
        if (EEDetId::validDetId(iX, iY, -1)) {
          double r1 = (double)std::rand() / (double(RAND_MAX) + double(1));
          EEDetId eedetidneg(iX, iY, -1);
          ical->setValue(eedetidneg, laserAlphaMean_ + r1 * laserAlphaSigma_);
        }
      }  // loop over iY
    }  // loop over iX
  }

  return ical;
}

std::unique_ptr<EcalLaserAPDPNRatiosRef> EcalTrivialConditionRetriever::produceEcalLaserAPDPNRatiosRef(
    const EcalLaserAPDPNRatiosRefRcd&) {
  auto ical = std::make_unique<EcalLaserAPDPNRatiosRef>();
  for (int ieta = -EBDetId::MAX_IETA; ieta <= EBDetId::MAX_IETA; ++ieta) {
    if (ieta == 0)
      continue;
    for (int iphi = EBDetId::MIN_IPHI; iphi <= EBDetId::MAX_IPHI; ++iphi) {
      if (EBDetId::validDetId(ieta, iphi)) {
        EBDetId ebid(ieta, iphi);
        double r = (double)std::rand() / (double(RAND_MAX) + double(1));
        ical->setValue(ebid, laserAPDPNRefMean_ + r * laserAPDPNRefSigma_);
      }
    }
  }

  for (int iX = EEDetId::IX_MIN; iX <= EEDetId::IX_MAX; ++iX) {
    for (int iY = EEDetId::IY_MIN; iY <= EEDetId::IY_MAX; ++iY) {
      // make an EEDetId since we need EEDetId::rawId() to be used as the key for the pedestals
      if (EEDetId::validDetId(iX, iY, 1)) {
        double r = (double)std::rand() / (double(RAND_MAX) + double(1));
        EEDetId eedetidpos(iX, iY, 1);
        ical->setValue(eedetidpos, laserAPDPNRefMean_ + r * laserAPDPNRefSigma_);
      }

      if (EEDetId::validDetId(iX, iY, -1)) {
        double r1 = (double)std::rand() / (double(RAND_MAX) + double(1));
        EEDetId eedetidneg(iX, iY, -1);
        ical->setValue(eedetidneg, laserAPDPNRefMean_ + r1 * laserAPDPNRefSigma_);
      }
    }
  }

  return ical;
}

std::unique_ptr<EcalLaserAPDPNRatios> EcalTrivialConditionRetriever::produceEcalLaserAPDPNRatios(
    const EcalLaserAPDPNRatiosRcd&) {
  EnergyResolutionVsLumi ageing;
  ageing.setLumi(totLumi_);
  ageing.setInstLumi(instLumi_);

  auto ical = std::make_unique<EcalLaserAPDPNRatios>();
  for (int ieta = -EBDetId::MAX_IETA; ieta <= EBDetId::MAX_IETA; ++ieta) {
    if (ieta == 0)
      continue;

    double eta = EBDetId::approxEta(EBDetId(ieta, 1));

    eta = fabs(eta);
    double drop = ageing.calcampDropTotal(eta);
    // edm::LogInfo("EB at eta=")<<eta<<" dropping by "<<drop;

    for (int iphi = EBDetId::MIN_IPHI; iphi <= EBDetId::MAX_IPHI; ++iphi) {
      if (EBDetId::validDetId(ieta, iphi)) {
        EBDetId ebid(ieta, iphi);
        double r = (double)std::rand() / (double(RAND_MAX) + double(1));

        EcalLaserAPDPNRatios::EcalLaserAPDPNpair pairAPDPN;
        pairAPDPN.p1 = laserAPDPNMean_ * drop + r * laserAPDPNSigma_;
        pairAPDPN.p2 = laserAPDPNMean_ * drop + r * laserAPDPNSigma_;
        pairAPDPN.p3 = laserAPDPNMean_ * drop + r * laserAPDPNSigma_;
        ical->setValue(ebid, pairAPDPN);
      }
    }
  }

  edm::LogInfo("----- EE -----");

  for (int iX = EEDetId::IX_MIN; iX <= EEDetId::IX_MAX; ++iX) {
    for (int iY = EEDetId::IY_MIN; iY <= EEDetId::IY_MAX; ++iY) {
      // make an EEDetId since we need EEDetId::rawId() to be used as the key for the pedestals

      if (EEDetId::validDetId(iX, iY, 1)) {
        double r = (double)std::rand() / (double(RAND_MAX) + double(1));
        EEDetId eedetidpos(iX, iY, 1);

        double eta = -log(tan(0.5 * atan(sqrt((iX - 50.0) * (iX - 50.0) + (iY - 50.0) * (iY - 50.0)) * 2.98 / 328.)));
        eta = fabs(eta);
        double drop = ageing.calcampDropTotal(eta);
        // if(iX==50) edm::LogInfo("EE at eta=")<<eta<<" dropping by "<<drop;

        EcalLaserAPDPNRatios::EcalLaserAPDPNpair pairAPDPN;
        pairAPDPN.p1 = laserAPDPNMean_ * drop + r * laserAPDPNSigma_;
        pairAPDPN.p2 = laserAPDPNMean_ * drop + r * laserAPDPNSigma_;
        pairAPDPN.p3 = laserAPDPNMean_ * drop + r * laserAPDPNSigma_;
        ical->setValue(eedetidpos, pairAPDPN);
      }

      if (EEDetId::validDetId(iX, iY, -1)) {
        double r1 = (double)std::rand() / (double(RAND_MAX) + double(1));
        EEDetId eedetidneg(iX, iY, -1);

        double eta = -log(tan(0.5 * atan(sqrt((iX - 50.0) * (iX - 50.0) + (iY - 50.0) * (iY - 50.0)) * 2.98 / 328.)));
        eta = fabs(eta);
        double drop = ageing.calcampDropTotal(eta);
        // if(iX==50) edm::LogInfo("EE at eta=")<<eta<<" dropping by "<<drop;

        EcalLaserAPDPNRatios::EcalLaserAPDPNpair pairAPDPN;
        pairAPDPN.p1 = laserAPDPNMean_ * drop + r1 * laserAPDPNSigma_;
        pairAPDPN.p2 = laserAPDPNMean_ * drop + r1 * laserAPDPNSigma_;
        pairAPDPN.p3 = laserAPDPNMean_ * drop + r1 * laserAPDPNSigma_;
        ical->setValue(eedetidneg, pairAPDPN);
      }
    }
  }

  EcalLaserAPDPNRatios::EcalLaserTimeStamp TimeStamp;
  //  for(int i=1; i<=92; i++){
  for (int i = 0; i < 92; i++) {
    TimeStamp.t1 = Timestamp(laserAPDPNTime1_);
    if (laserAPDPNTime2_ == 0) {
      TimeStamp.t2 = Timestamp(edm::Timestamp::endOfTime().value());
    } else {
      TimeStamp.t2 = Timestamp(laserAPDPNTime2_);
    }
    if (laserAPDPNTime3_ == 0) {
      TimeStamp.t3 = Timestamp(edm::Timestamp::endOfTime().value());
    } else {
      TimeStamp.t3 = Timestamp(laserAPDPNTime3_);
    }

    ical->setTime(i, TimeStamp);
  }

  return ical;
}

void EcalTrivialConditionRetriever::getWeightsFromConfiguration(const edm::ParameterSet& ps) {
  std::vector<std::vector<double> > amplwgtv(nTDCbins_);

  if (!getWeightsFromFile_ && nTDCbins_ == 1) {
    std::vector<double> vampl;
    //As default using simple 3+1 weights
    vampl.push_back(-0.33333);
    vampl.push_back(-0.33333);
    vampl.push_back(-0.33333);
    vampl.push_back(0.);
    vampl.push_back(0.);
    vampl.push_back(1.);
    vampl.push_back(0.);
    vampl.push_back(0.);
    vampl.push_back(0.);
    vampl.push_back(0.);
    amplwgtv[0] = ps.getUntrackedParameter<std::vector<double> >("amplWeights", vampl);
  } else if (getWeightsFromFile_) {
    edm::LogInfo("EcalTrivialConditionRetriever")
        << "Reading amplitude weights from file " << edm::FileInPath(amplWeightsFile_).fullPath().c_str();
    std::ifstream amplFile(edm::FileInPath(amplWeightsFile_).fullPath().c_str());
    int tdcBin = 0;
    while (!amplFile.eof() && tdcBin < nTDCbins_) {
      for (int j = 0; j < 10; ++j) {
        float ww;
        amplFile >> ww;
        amplwgtv[tdcBin].push_back(ww);
      }
      ++tdcBin;
    }
    assert(tdcBin == nTDCbins_);
    //Read from file
  } else {
    //Not supported
    edm::LogError("EcalTrivialConditionRetriever") << "Configuration not supported. Exception is raised ";
    throw cms::Exception("WrongConfig");
  }

  for (int i = 0; i < nTDCbins_; i++) {
    assert(amplwgtv[i].size() == 10);
    int j = 0;
    for (std::vector<double>::const_iterator it = amplwgtv[i].begin(); it != amplwgtv[i].end(); ++it) {
      (amplWeights_[i])[j] = *it;
      j++;
    }
  }

  std::vector<std::vector<double> > amplwgtvAftGain(nTDCbins_);

  if (!getWeightsFromFile_ && nTDCbins_ == 1) {
    std::vector<double> vamplAftGain;
    vamplAftGain.push_back(0.);
    vamplAftGain.push_back(0.);
    vamplAftGain.push_back(0.);
    vamplAftGain.push_back(0.);
    vamplAftGain.push_back(0.);
    vamplAftGain.push_back(1.);
    vamplAftGain.push_back(0.);
    vamplAftGain.push_back(0.);
    vamplAftGain.push_back(0.);
    vamplAftGain.push_back(0.);
    amplwgtvAftGain[0] = ps.getUntrackedParameter<std::vector<double> >("amplWeightsAftGain", vamplAftGain);
  } else if (getWeightsFromFile_) {
    //Read from file
    edm::LogInfo("EcalTrivialConditionRetriever") << "Reading amplitude weights aftre gain switch from file "
                                                  << edm::FileInPath(amplWeightsAftFile_).fullPath().c_str();
    std::ifstream amplFile(edm::FileInPath(amplWeightsAftFile_).fullPath().c_str());
    int tdcBin = 0;
    while (!amplFile.eof() && tdcBin < nTDCbins_) {
      for (int j = 0; j < 10; ++j) {
        float ww;
        amplFile >> ww;
        amplwgtvAftGain[tdcBin].push_back(ww);
      }
      ++tdcBin;
    }
    assert(tdcBin == nTDCbins_);
  } else {
    //Not supported
    edm::LogError("EcalTrivialConditionRetriever") << "Configuration not supported. Exception is raised ";
    throw cms::Exception("WrongConfig");
  }

  for (int i = 0; i < nTDCbins_; i++) {
    assert(amplwgtvAftGain[i].size() == 10);
    int j = 0;
    for (std::vector<double>::const_iterator it = amplwgtvAftGain[i].begin(); it != amplwgtvAftGain[i].end(); ++it) {
      (amplWeightsAft_[i])[j] = *it;
      j++;
    }
  }

  // default weights to reco amplitude w/o pedestal subtraction

  std::vector<std::vector<double> > pedwgtv(nTDCbins_);

  if (!getWeightsFromFile_ && nTDCbins_ == 1) {
    std::vector<double> vped;
    vped.push_back(0.33333);
    vped.push_back(0.33333);
    vped.push_back(0.33333);
    vped.push_back(0.);
    vped.push_back(0.);
    vped.push_back(0.);
    vped.push_back(0.);
    vped.push_back(0.);
    vped.push_back(0.);
    vped.push_back(0.);
    pedwgtv[0] = ps.getUntrackedParameter<std::vector<double> >("pedWeights", vped);
  } else if (getWeightsFromFile_) {
    //Read from file
    edm::LogInfo("EcalTrivialConditionRetriever")
        << "Reading pedestal weights from file " << edm::FileInPath(pedWeightsFile_).fullPath().c_str();
    std::ifstream pedFile(edm::FileInPath(pedWeightsFile_).fullPath().c_str());
    int tdcBin = 0;
    while (!pedFile.eof() && tdcBin < nTDCbins_) {
      for (int j = 0; j < 10; ++j) {
        float ww;
        pedFile >> ww;
        pedwgtv[tdcBin].push_back(ww);
      }
      ++tdcBin;
    }
    assert(tdcBin == nTDCbins_);
  } else {
    //Not supported
    edm::LogError("EcalTrivialConditionRetriever") << "Configuration not supported. Exception is raised ";
    throw cms::Exception("WrongConfig");
  }

  for (int i = 0; i < nTDCbins_; i++) {
    assert(pedwgtv[i].size() == 10);
    int j = 0;
    for (std::vector<double>::const_iterator it = pedwgtv[i].begin(); it != pedwgtv[i].end(); ++it) {
      (pedWeights_[i])[j] = *it;
      j++;
    }
  }

  std::vector<std::vector<double> > pedwgtvaft(nTDCbins_);

  if (!getWeightsFromFile_ && nTDCbins_ == 1) {
    std::vector<double> vped;
    vped.push_back(0.);
    vped.push_back(0.);
    vped.push_back(0.);
    vped.push_back(0.);
    vped.push_back(0.);
    vped.push_back(0.);
    vped.push_back(0.);
    vped.push_back(0.);
    vped.push_back(0.);
    vped.push_back(0.);
    pedwgtvaft[0] = ps.getUntrackedParameter<std::vector<double> >("pedWeightsAft", vped);
  } else if (getWeightsFromFile_) {
    //Read from file
    edm::LogInfo("EcalTrivialConditionRetriever") << "Reading pedestal after gain switch weights from file "
                                                  << edm::FileInPath(pedWeightsAftFile_).fullPath().c_str();
    std::ifstream pedFile(edm::FileInPath(pedWeightsAftFile_).fullPath().c_str());
    int tdcBin = 0;
    while (!pedFile.eof() && tdcBin < nTDCbins_) {
      for (int j = 0; j < 10; ++j) {
        float ww;
        pedFile >> ww;
        pedwgtvaft[tdcBin].push_back(ww);
      }
      ++tdcBin;
    }
    assert(tdcBin == nTDCbins_);
  } else {
    //Not supported
    edm::LogError("EcalTrivialConditionRetriever") << "Configuration not supported. Exception is raised ";
    throw cms::Exception("WrongConfig");
  }

  for (int i = 0; i < nTDCbins_; i++) {
    assert(pedwgtvaft[i].size() == 10);
    int j = 0;
    for (std::vector<double>::const_iterator it = pedwgtvaft[i].begin(); it != pedwgtvaft[i].end(); ++it) {
      (pedWeightsAft_[i])[j] = *it;
      j++;
    }
  }

  // default weights to reco jitter

  std::vector<std::vector<double> > jittwgtv(nTDCbins_);

  if (!getWeightsFromFile_ && nTDCbins_ == 1) {
    std::vector<double> vjitt;
    vjitt.push_back(0.04066309);
    vjitt.push_back(0.04066309);
    vjitt.push_back(0.04066309);
    vjitt.push_back(0.000);
    vjitt.push_back(1.325176);
    vjitt.push_back(-0.04997078);
    vjitt.push_back(-0.504338);
    vjitt.push_back(-0.5024844);
    vjitt.push_back(-0.3903718);
    vjitt.push_back(0.000);
    jittwgtv[0] = ps.getUntrackedParameter<std::vector<double> >("jittWeights", vjitt);
  } else if (getWeightsFromFile_) {
    //Read from file
    edm::LogInfo("EcalTrivialConditionRetriever")
        << "Reading jitter weights from file " << edm::FileInPath(jittWeightsFile_).fullPath().c_str();
    std::ifstream jittFile(edm::FileInPath(jittWeightsFile_).fullPath().c_str());
    int tdcBin = 0;
    while (!jittFile.eof() && tdcBin < nTDCbins_) {
      for (int j = 0; j < 10; ++j) {
        float ww;
        jittFile >> ww;
        jittwgtv[tdcBin].push_back(ww);
      }
      ++tdcBin;
    }
    assert(tdcBin == nTDCbins_);
  } else {
    //Not supported
    edm::LogError("EcalTrivialConditionRetriever") << "Configuration not supported. Exception is raised ";
    throw cms::Exception("WrongConfig");
  }

  for (int i = 0; i < nTDCbins_; i++) {
    assert(jittwgtv[i].size() == 10);
    int j = 0;
    for (std::vector<double>::const_iterator it = jittwgtv[i].begin(); it != jittwgtv[i].end(); ++it) {
      (jittWeights_[i])[j] = *it;
      j++;
    }
  }

  std::vector<std::vector<double> > jittwgtvaft(nTDCbins_);

  if (!getWeightsFromFile_ && nTDCbins_ == 1) {
    std::vector<double> vjitt;
    vjitt.push_back(0.);
    vjitt.push_back(0.);
    vjitt.push_back(0.);
    vjitt.push_back(0.);
    vjitt.push_back(1.097871);
    vjitt.push_back(-0.04551035);
    vjitt.push_back(-0.4159156);
    vjitt.push_back(-0.4185352);
    vjitt.push_back(-0.3367127);
    vjitt.push_back(0.);
    jittwgtvaft[0] = ps.getUntrackedParameter<std::vector<double> >("jittWeightsAft", vjitt);
  } else if (getWeightsFromFile_) {
    //Read from file
    edm::LogInfo("EcalTrivialConditionRetriever") << "Reading jitter after gain switch weights from file "
                                                  << edm::FileInPath(jittWeightsAftFile_).fullPath().c_str();
    std::ifstream jittFile(edm::FileInPath(jittWeightsAftFile_).fullPath().c_str());
    int tdcBin = 0;
    while (!jittFile.eof() && tdcBin < nTDCbins_) {
      for (int j = 0; j < 10; ++j) {
        float ww;
        jittFile >> ww;
        jittwgtvaft[tdcBin].push_back(ww);
      }
      ++tdcBin;
    }
    assert(tdcBin == nTDCbins_);
  } else {
    //Not supported
    edm::LogError("EcalTrivialConditionRetriever") << "Configuration not supported. Exception is raised ";
    throw cms::Exception("WrongConfig");
  }

  for (int i = 0; i < nTDCbins_; i++) {
    assert(jittwgtvaft[i].size() == 10);
    int j = 0;
    for (std::vector<double>::const_iterator it = jittwgtvaft[i].begin(); it != jittwgtvaft[i].end(); ++it) {
      (jittWeightsAft_[i])[j] = *it;
      j++;
    }
  }

  std::vector<EcalWeightSet::EcalChi2WeightMatrix> chi2Matrix(nTDCbins_);
  if (!getWeightsFromFile_ && nTDCbins_ == 1) {
    //        chi2Matrix[0].resize(10);
    //        for (int i=0;i<10;i++)
    //	 chi2Matrix[0][i].resize(10);

    chi2Matrix[0](0, 0) = 0.694371;
    chi2Matrix[0](0, 1) = -0.305629;
    chi2Matrix[0](0, 2) = -0.305629;
    chi2Matrix[0](0, 3) = 0.;
    chi2Matrix[0](0, 4) = 0.;
    chi2Matrix[0](0, 5) = 0.;
    chi2Matrix[0](0, 6) = 0.;
    chi2Matrix[0](0, 7) = 0.;
    chi2Matrix[0](0, 8) = 0.;
    chi2Matrix[0](0, 9) = 0.;
    chi2Matrix[0](1, 0) = -0.305629;
    chi2Matrix[0](1, 1) = 0.694371;
    chi2Matrix[0](1, 2) = -0.305629;
    chi2Matrix[0](1, 3) = 0.;
    chi2Matrix[0](1, 4) = 0.;
    chi2Matrix[0](1, 5) = 0.;
    chi2Matrix[0](1, 6) = 0.;
    chi2Matrix[0](1, 7) = 0.;
    chi2Matrix[0](1, 8) = 0.;
    chi2Matrix[0](1, 9) = 0.;
    chi2Matrix[0](2, 0) = -0.305629;
    chi2Matrix[0](2, 1) = -0.305629;
    chi2Matrix[0](2, 2) = 0.694371;
    chi2Matrix[0](2, 3) = 0.;
    chi2Matrix[0](2, 4) = 0.;
    chi2Matrix[0](2, 5) = 0.;
    chi2Matrix[0](2, 6) = 0.;
    chi2Matrix[0](2, 7) = 0.;
    chi2Matrix[0](2, 8) = 0.;
    chi2Matrix[0](2, 9) = 0.;
    chi2Matrix[0](3, 0) = 0.;
    chi2Matrix[0](3, 1) = 0.;
    chi2Matrix[0](3, 2) = 0.;
    chi2Matrix[0](3, 3) = 0.;
    chi2Matrix[0](3, 4) = 0.;
    chi2Matrix[0](3, 5) = 0.;
    chi2Matrix[0](3, 6) = 0.;
    chi2Matrix[0](3, 7) = 0.;
    chi2Matrix[0](3, 8) = 0.;
    chi2Matrix[0](3, 9) = 0.;
    chi2Matrix[0](4, 0) = 0.;
    chi2Matrix[0](4, 1) = 0.;
    chi2Matrix[0](4, 2) = 0.;
    chi2Matrix[0](4, 3) = 0.;
    chi2Matrix[0](4, 4) = 0.8027116;
    chi2Matrix[0](4, 5) = -0.2517103;
    chi2Matrix[0](4, 6) = -0.2232882;
    chi2Matrix[0](4, 7) = -0.1716192;
    chi2Matrix[0](4, 8) = -0.1239006;
    chi2Matrix[0](4, 9) = 0.;
    chi2Matrix[0](5, 0) = 0.;
    chi2Matrix[0](5, 1) = 0.;
    chi2Matrix[0](5, 2) = 0.;
    chi2Matrix[0](5, 3) = 0.;
    chi2Matrix[0](5, 4) = -0.2517103;
    chi2Matrix[0](5, 5) = 0.6528964;
    chi2Matrix[0](5, 6) = -0.2972839;
    chi2Matrix[0](5, 7) = -0.2067162;
    chi2Matrix[0](5, 8) = -0.1230729;
    chi2Matrix[0](5, 9) = 0.;
    chi2Matrix[0](6, 0) = 0.;
    chi2Matrix[0](6, 1) = 0.;
    chi2Matrix[0](6, 2) = 0.;
    chi2Matrix[0](6, 3) = 0.;
    chi2Matrix[0](6, 4) = -0.2232882;
    chi2Matrix[0](6, 5) = -0.2972839;
    chi2Matrix[0](6, 6) = 0.7413607;
    chi2Matrix[0](6, 7) = -0.1883866;
    chi2Matrix[0](6, 8) = -0.1235052;
    chi2Matrix[0](6, 9) = 0.;
    chi2Matrix[0](7, 0) = 0.;
    chi2Matrix[0](7, 1) = 0.;
    chi2Matrix[0](7, 2) = 0.;
    chi2Matrix[0](7, 3) = 0.;
    chi2Matrix[0](7, 4) = -0.1716192;
    chi2Matrix[0](7, 5) = -0.2067162;
    chi2Matrix[0](7, 6) = -0.1883866;
    chi2Matrix[0](7, 7) = 0.844935;
    chi2Matrix[0](7, 8) = -0.124291;
    chi2Matrix[0](7, 9) = 0.;
    chi2Matrix[0](8, 0) = 0.;
    chi2Matrix[0](8, 1) = 0.;
    chi2Matrix[0](8, 2) = 0.;
    chi2Matrix[0](8, 3) = 0.;
    chi2Matrix[0](8, 4) = -0.1239006;
    chi2Matrix[0](8, 5) = -0.1230729;
    chi2Matrix[0](8, 6) = -0.1235052;
    chi2Matrix[0](8, 7) = -0.124291;
    chi2Matrix[0](8, 8) = 0.8749833;
    chi2Matrix[0](8, 9) = 0.;
    chi2Matrix[0](9, 0) = 0.;
    chi2Matrix[0](9, 1) = 0.;
    chi2Matrix[0](9, 2) = 0.;
    chi2Matrix[0](9, 3) = 0.;
    chi2Matrix[0](9, 4) = 0.;
    chi2Matrix[0](9, 5) = 0.;
    chi2Matrix[0](9, 6) = 0.;
    chi2Matrix[0](9, 7) = 0.;
    chi2Matrix[0](9, 8) = 0.;
    chi2Matrix[0](9, 9) = 0.;
  } else if (getWeightsFromFile_) {
    //Read from file
    edm::LogInfo("EcalTrivialConditionRetriever")
        << "Reading chi2Matrix from file " << edm::FileInPath(chi2MatrixFile_).fullPath().c_str();
    std::ifstream chi2MatrixFile(edm::FileInPath(chi2MatrixFile_).fullPath().c_str());
    int tdcBin = 0;
    while (!chi2MatrixFile.eof() && tdcBin < nTDCbins_) {
      //	  chi2Matrix[tdcBin].resize(10);
      for (int j = 0; j < 10; ++j) {
        for (int l = 0; l < 10; ++l) {
          float ww;
          chi2MatrixFile >> ww;
          chi2Matrix[tdcBin](j, l) = ww;
        }
      }
      ++tdcBin;
    }
    assert(tdcBin == nTDCbins_);
  } else {
    //Not supported
    edm::LogError("EcalTrivialConditionRetriever") << "Configuration not supported. Exception is raised ";
    throw cms::Exception("WrongConfig");
  }

  //    for (int i=0;i<nTDCbins_;i++)
  //      {
  //       assert(chi2Matrix[i].size() == 10);
  chi2Matrix_ = chi2Matrix;
  //      }

  std::vector<EcalWeightSet::EcalChi2WeightMatrix> chi2MatrixAft(nTDCbins_);
  if (!getWeightsFromFile_ && nTDCbins_ == 1) {
    //       chi2MatrixAft[0].resize(10);
    //       for (int i=0;i<10;i++)
    //	 chi2MatrixAft[0][i].resize(10);

    chi2MatrixAft[0](0, 0) = 0.;
    chi2MatrixAft[0](0, 1) = 0.;
    chi2MatrixAft[0](0, 2) = 0.;
    chi2MatrixAft[0](0, 3) = 0.;
    chi2MatrixAft[0](0, 4) = 0.;
    chi2MatrixAft[0](0, 5) = 0.;
    chi2MatrixAft[0](0, 6) = 0.;
    chi2MatrixAft[0](0, 7) = 0.;
    chi2MatrixAft[0](0, 8) = 0.;
    chi2MatrixAft[0](0, 9) = 0.;
    chi2MatrixAft[0](1, 0) = 0.;
    chi2MatrixAft[0](1, 1) = 0.;
    chi2MatrixAft[0](1, 2) = 0.;
    chi2MatrixAft[0](1, 3) = 0.;
    chi2MatrixAft[0](1, 4) = 0.;
    chi2MatrixAft[0](1, 5) = 0.;
    chi2MatrixAft[0](1, 6) = 0.;
    chi2MatrixAft[0](1, 7) = 0.;
    chi2MatrixAft[0](1, 8) = 0.;
    chi2MatrixAft[0](1, 9) = 0.;
    chi2MatrixAft[0](2, 0) = 0.;
    chi2MatrixAft[0](2, 1) = 0.;
    chi2MatrixAft[0](2, 2) = 0.;
    chi2MatrixAft[0](2, 3) = 0.;
    chi2MatrixAft[0](2, 4) = 0.;
    chi2MatrixAft[0](2, 5) = 0.;
    chi2MatrixAft[0](2, 6) = 0.;
    chi2MatrixAft[0](2, 7) = 0.;
    chi2MatrixAft[0](2, 8) = 0.;
    chi2MatrixAft[0](2, 9) = 0.;
    chi2MatrixAft[0](3, 0) = 0.;
    chi2MatrixAft[0](3, 1) = 0.;
    chi2MatrixAft[0](3, 2) = 0.;
    chi2MatrixAft[0](3, 3) = 0.;
    chi2MatrixAft[0](3, 4) = 0.;
    chi2MatrixAft[0](3, 5) = 0.;
    chi2MatrixAft[0](3, 6) = 0.;
    chi2MatrixAft[0](3, 7) = 0.;
    chi2MatrixAft[0](3, 8) = 0.;
    chi2MatrixAft[0](3, 9) = 0.;
    chi2MatrixAft[0](4, 0) = 0.;
    chi2MatrixAft[0](4, 1) = 0.;
    chi2MatrixAft[0](4, 2) = 0.;
    chi2MatrixAft[0](4, 3) = 0.;
    chi2MatrixAft[0](4, 4) = 0.8030884;
    chi2MatrixAft[0](4, 5) = -0.2543541;
    chi2MatrixAft[0](4, 6) = -0.2243544;
    chi2MatrixAft[0](4, 7) = -0.1698177;
    chi2MatrixAft[0](4, 8) = -0.1194506;
    chi2MatrixAft[0](4, 9) = 0.;
    chi2MatrixAft[0](5, 0) = 0.;
    chi2MatrixAft[0](5, 1) = 0.;
    chi2MatrixAft[0](5, 2) = 0.;
    chi2MatrixAft[0](5, 3) = 0.;
    chi2MatrixAft[0](5, 4) = -0.2543541;
    chi2MatrixAft[0](5, 5) = 0.6714465;
    chi2MatrixAft[0](5, 6) = -0.2898025;
    chi2MatrixAft[0](5, 7) = -0.2193564;
    chi2MatrixAft[0](5, 8) = -0.1542964;
    chi2MatrixAft[0](5, 9) = 0.;
    chi2MatrixAft[0](6, 0) = 0.;
    chi2MatrixAft[0](6, 1) = 0.;
    chi2MatrixAft[0](6, 2) = 0.;
    chi2MatrixAft[0](6, 3) = 0.;
    chi2MatrixAft[0](6, 4) = -0.2243544;
    chi2MatrixAft[0](6, 5) = -0.2898025;
    chi2MatrixAft[0](6, 6) = 0.7443781;
    chi2MatrixAft[0](6, 7) = -0.1934846;
    chi2MatrixAft[0](6, 8) = -0.136098;
    chi2MatrixAft[0](6, 9) = 0.;
    chi2MatrixAft[0](7, 0) = 0.;
    chi2MatrixAft[0](7, 1) = 0.;
    chi2MatrixAft[0](7, 2) = 0.;
    chi2MatrixAft[0](7, 3) = 0.;
    chi2MatrixAft[0](7, 4) = -0.1698177;
    chi2MatrixAft[0](7, 5) = -0.2193564;
    chi2MatrixAft[0](7, 6) = -0.1934846;
    chi2MatrixAft[0](7, 7) = 0.8535482;
    chi2MatrixAft[0](7, 8) = -0.1030149;
    chi2MatrixAft[0](7, 9) = 0.;
    chi2MatrixAft[0](8, 0) = 0.;
    chi2MatrixAft[0](8, 1) = 0.;
    chi2MatrixAft[0](8, 2) = 0.;
    chi2MatrixAft[0](8, 3) = 0.;
    chi2MatrixAft[0](8, 4) = -0.1194506;
    chi2MatrixAft[0](8, 5) = -0.1542964;
    chi2MatrixAft[0](8, 6) = -0.136098;
    chi2MatrixAft[0](8, 7) = -0.1030149;
    chi2MatrixAft[0](8, 8) = 0.9275388;
    chi2MatrixAft[0](8, 9) = 0.;
    chi2MatrixAft[0](9, 0) = 0.;
    chi2MatrixAft[0](9, 1) = 0.;
    chi2MatrixAft[0](9, 2) = 0.;
    chi2MatrixAft[0](9, 3) = 0.;
    chi2MatrixAft[0](9, 4) = 0.;
    chi2MatrixAft[0](9, 5) = 0.;
    chi2MatrixAft[0](9, 6) = 0.;
    chi2MatrixAft[0](9, 7) = 0.;
    chi2MatrixAft[0](9, 8) = 0.;
    chi2MatrixAft[0](9, 9) = 0.;
  } else if (getWeightsFromFile_) {
    //Read from file
    edm::LogInfo("EcalTrivialConditionRetriever")
        << "Reading chi2MatrixAft from file " << edm::FileInPath(chi2MatrixAftFile_).fullPath().c_str();
    std::ifstream chi2MatrixAftFile(edm::FileInPath(chi2MatrixAftFile_).fullPath().c_str());
    int tdcBin = 0;
    while (!chi2MatrixAftFile.eof() && tdcBin < nTDCbins_) {
      //	  chi2MatrixAft[tdcBin].resize(10);
      for (int j = 0; j < 10; ++j) {
        for (int l = 0; l < 10; ++l) {
          float ww;
          chi2MatrixAftFile >> ww;
          chi2MatrixAft[tdcBin](j, l) = ww;
        }
      }
      ++tdcBin;
    }
    assert(tdcBin == nTDCbins_);
  } else {
    //Not supported
    edm::LogError("EcalTrivialConditionRetriever") << "Configuration not supported. Exception is raised ";
    throw cms::Exception("WrongConfig");
  }

  //    for (int i=0;i<nTDCbins_;i++)
  //      {
  //       assert(chi2MatrixAft[i].size() == 10);
  chi2MatrixAft_ = chi2MatrixAft;
  //      }
}

// --------------------------------------------------------------------------------

std::unique_ptr<EcalChannelStatus> EcalTrivialConditionRetriever::getChannelStatusFromConfiguration(
    const EcalChannelStatusRcd&) {
  auto ecalStatus = std::make_unique<EcalChannelStatus>();

  // start by setting all statuses to 0

  // barrel
  for (int ieta = -EBDetId::MAX_IETA; ieta <= EBDetId::MAX_IETA; ++ieta) {
    if (ieta == 0)
      continue;
    for (int iphi = EBDetId::MIN_IPHI; iphi <= EBDetId::MAX_IPHI; ++iphi) {
      if (EBDetId::validDetId(ieta, iphi)) {
        EBDetId ebid(ieta, iphi);
        ecalStatus->setValue(ebid, 0);
      }
    }
  }
  // endcap
  for (int iX = EEDetId::IX_MIN; iX <= EEDetId::IX_MAX; ++iX) {
    for (int iY = EEDetId::IY_MIN; iY <= EEDetId::IY_MAX; ++iY) {
      // make an EEDetId since we need EEDetId::rawId() to be used as the key for the pedestals
      if (EEDetId::validDetId(iX, iY, 1)) {
        EEDetId eedetidpos(iX, iY, 1);
        ecalStatus->setValue(eedetidpos, 0);
      }
      if (EEDetId::validDetId(iX, iY, -1)) {
        EEDetId eedetidneg(iX, iY, -1);
        ecalStatus->setValue(eedetidneg, 0);
      }
    }
  }

  // overwrite the statuses which are in the file

  edm::LogInfo("EcalTrivialConditionRetriever")
      << "Reading channel statuses from file " << edm::FileInPath(channelStatusFile_).fullPath().c_str();
  std::ifstream statusFile(edm::FileInPath(channelStatusFile_).fullPath().c_str());
  if (!statusFile.good()) {
    edm::LogError("EcalTrivialConditionRetriever") << "*** Problems opening file: " << channelStatusFile_;
    throw cms::Exception("Cannot open ECAL channel status file");
  }

  std::string EcalSubDet;
  std::string str;
  int hashIndex(0);
  int status(0);

  while (!statusFile.eof()) {
    statusFile >> EcalSubDet;
    if (EcalSubDet != std::string("EB") && EcalSubDet != std::string("EE")) {
      std::getline(statusFile, str);
      continue;
    } else {
      statusFile >> hashIndex >> status;
    }
    // std::cout << EcalSubDet << " " << hashIndex << " " << status;

    if (EcalSubDet == std::string("EB")) {
      EBDetId ebid = EBDetId::unhashIndex(hashIndex);
      ecalStatus->setValue(ebid, status);
    } else if (EcalSubDet == std::string("EE")) {
      EEDetId eedetid = EEDetId::unhashIndex(hashIndex);
      ecalStatus->setValue(eedetid, status);
    } else {
      edm::LogError("EcalTrivialConditionRetriever") << " *** " << EcalSubDet << " is neither EB nor EE ";
    }
  }
  // the file is supposed to be in the form  -- FIXME

  statusFile.close();
  return ecalStatus;
}

std::unique_ptr<EcalChannelStatus> EcalTrivialConditionRetriever::produceEcalChannelStatus(const EcalChannelStatusRcd&) {
  auto ical = std::make_unique<EcalChannelStatus>();
  // barrel
  for (int ieta = -EBDetId::MAX_IETA; ieta <= EBDetId::MAX_IETA; ++ieta) {
    if (ieta == 0)
      continue;
    for (int iphi = EBDetId::MIN_IPHI; iphi <= EBDetId::MAX_IPHI; ++iphi) {
      if (EBDetId::validDetId(ieta, iphi)) {
        EBDetId ebid(ieta, iphi);
        ical->setValue(ebid, 0);
      }
    }
  }
  // endcap
  for (int iX = EEDetId::IX_MIN; iX <= EEDetId::IX_MAX; ++iX) {
    for (int iY = EEDetId::IY_MIN; iY <= EEDetId::IY_MAX; ++iY) {
      // make an EEDetId since we need EEDetId::rawId() to be used as the key for the pedestals
      if (EEDetId::validDetId(iX, iY, 1)) {
        EEDetId eedetidpos(iX, iY, 1);
        ical->setValue(eedetidpos, 0);
      }
      if (EEDetId::validDetId(iX, iY, -1)) {
        EEDetId eedetidneg(iX, iY, -1);
        ical->setValue(eedetidneg, 0);
      }
    }
  }
  return ical;
}

// --------------------------------------------------------------------------------
std::unique_ptr<EcalDQMChannelStatus> EcalTrivialConditionRetriever::produceEcalDQMChannelStatus(
    const EcalDQMChannelStatusRcd&) {
  uint32_t sta(0);

  auto ical = std::make_unique<EcalDQMChannelStatus>();
  // barrel
  for (int ieta = -EBDetId::MAX_IETA; ieta <= EBDetId::MAX_IETA; ++ieta) {
    if (ieta == 0)
      continue;
    for (int iphi = EBDetId::MIN_IPHI; iphi <= EBDetId::MAX_IPHI; ++iphi) {
      if (EBDetId::validDetId(ieta, iphi)) {
        EBDetId ebid(ieta, iphi);
        ical->setValue(ebid, sta);
      }
    }
  }
  // endcap
  for (int iX = EEDetId::IX_MIN; iX <= EEDetId::IX_MAX; ++iX) {
    for (int iY = EEDetId::IY_MIN; iY <= EEDetId::IY_MAX; ++iY) {
      // make an EEDetId since we need EEDetId::rawId() to be used as the key for the pedestals
      if (EEDetId::validDetId(iX, iY, 1)) {
        EEDetId eedetidpos(iX, iY, 1);
        ical->setValue(eedetidpos, sta);
      }
      if (EEDetId::validDetId(iX, iY, -1)) {
        EEDetId eedetidneg(iX, iY, -1);
        ical->setValue(eedetidneg, sta);
      }
    }
  }
  return ical;
}
// --------------------------------------------------------------------------------

std::unique_ptr<EcalDQMTowerStatus> EcalTrivialConditionRetriever::produceEcalDQMTowerStatus(
    const EcalDQMTowerStatusRcd&) {
  auto ical = std::make_unique<EcalDQMTowerStatus>();

  uint32_t sta(0);

  // barrel
  int iz = 0;
  for (int k = 0; k < 2; k++) {
    if (k == 0)
      iz = -1;
    if (k == 1)
      iz = +1;
    for (int i = 1; i < 73; i++) {
      for (int j = 1; j < 18; j++) {
        if (EcalTrigTowerDetId::validDetId(iz, EcalBarrel, j, i)) {
          EcalTrigTowerDetId ebid(iz, EcalBarrel, j, i);

          ical->setValue(ebid, sta);
        }
      }
    }
  }

  // endcap
  for (int k = 0; k < 2; k++) {
    if (k == 0)
      iz = -1;
    if (k == 1)
      iz = +1;
    for (int i = 1; i < 21; i++) {
      for (int j = 1; j < 21; j++) {
        if (EcalScDetId::validDetId(i, j, iz)) {
          EcalScDetId eeid(i, j, iz);
          ical->setValue(eeid, sta);
        }
      }
    }
  }

  return ical;
}

// --------------------------------------------------------------------------------
std::unique_ptr<EcalDCSTowerStatus> EcalTrivialConditionRetriever::produceEcalDCSTowerStatus(
    const EcalDCSTowerStatusRcd&) {
  auto ical = std::make_unique<EcalDCSTowerStatus>();

  int status(0);

  // barrel
  int iz = 0;
  for (int k = 0; k < 2; k++) {
    if (k == 0)
      iz = -1;
    if (k == 1)
      iz = +1;
    for (int i = 1; i < 73; i++) {
      for (int j = 1; j < 18; j++) {
        if (EcalTrigTowerDetId::validDetId(iz, EcalBarrel, j, i)) {
          EcalTrigTowerDetId ebid(iz, EcalBarrel, j, i);

          ical->setValue(ebid, status);
        }
      }
    }
  }

  // endcap
  for (int k = 0; k < 2; k++) {
    if (k == 0)
      iz = -1;
    if (k == 1)
      iz = +1;
    for (int i = 1; i < 21; i++) {
      for (int j = 1; j < 21; j++) {
        if (EcalScDetId::validDetId(i, j, iz)) {
          EcalScDetId eeid(i, j, iz);
          ical->setValue(eeid, status);
        }
      }
    }
  }

  return ical;
}
// --------------------------------------------------------------------------------

std::unique_ptr<EcalDAQTowerStatus> EcalTrivialConditionRetriever::produceEcalDAQTowerStatus(
    const EcalDAQTowerStatusRcd&) {
  auto ical = std::make_unique<EcalDAQTowerStatus>();

  int status(0);

  // barrel
  int iz = 0;
  for (int k = 0; k < 2; k++) {
    if (k == 0)
      iz = -1;
    if (k == 1)
      iz = +1;
    for (int i = 1; i < 73; i++) {
      for (int j = 1; j < 18; j++) {
        if (EcalTrigTowerDetId::validDetId(iz, EcalBarrel, j, i)) {
          EcalTrigTowerDetId ebid(iz, EcalBarrel, j, i);

          ical->setValue(ebid, status);
        }
      }
    }
  }

  // endcap
  for (int k = 0; k < 2; k++) {
    if (k == 0)
      iz = -1;
    if (k == 1)
      iz = +1;
    for (int i = 1; i < 21; i++) {
      for (int j = 1; j < 21; j++) {
        if (EcalScDetId::validDetId(i, j, iz)) {
          EcalScDetId eeid(i, j, iz);
          ical->setValue(eeid, status);
        }
      }
    }
  }

  return ical;
}
// --------------------------------------------------------------------------------

std::unique_ptr<EcalTPGCrystalStatus> EcalTrivialConditionRetriever::getTrgChannelStatusFromConfiguration(
    const EcalTPGCrystalStatusRcd&) {
  auto ecalStatus = std::make_unique<EcalTPGCrystalStatus>();

  // start by setting all statuses to 0

  // barrel
  for (int ieta = -EBDetId::MAX_IETA; ieta <= EBDetId::MAX_IETA; ++ieta) {
    if (ieta == 0)
      continue;
    for (int iphi = EBDetId::MIN_IPHI; iphi <= EBDetId::MAX_IPHI; ++iphi) {
      if (EBDetId::validDetId(ieta, iphi)) {
        EBDetId ebid(ieta, iphi);
        ecalStatus->setValue(ebid, 0);
      }
    }
  }
  // endcap
  for (int iX = EEDetId::IX_MIN; iX <= EEDetId::IX_MAX; ++iX) {
    for (int iY = EEDetId::IY_MIN; iY <= EEDetId::IY_MAX; ++iY) {
      // make an EEDetId since we need EEDetId::rawId() to be used as the key for the pedestals
      if (EEDetId::validDetId(iX, iY, 1)) {
        EEDetId eedetidpos(iX, iY, 1);
        ecalStatus->setValue(eedetidpos, 0);
      }
      if (EEDetId::validDetId(iX, iY, -1)) {
        EEDetId eedetidneg(iX, iY, -1);
        ecalStatus->setValue(eedetidneg, 0);
      }
    }
  }

  // overwrite the statuses which are in the file

  edm::LogInfo("EcalTrivialConditionRetriever")
      << "Reading channel statuses from file " << edm::FileInPath(channelStatusFile_).fullPath().c_str();
  std::ifstream statusFile(edm::FileInPath(channelStatusFile_).fullPath().c_str());
  if (!statusFile.good()) {
    edm::LogError("EcalTrivialConditionRetriever") << "*** Problems opening file: " << channelStatusFile_;
    throw cms::Exception("Cannot open ECAL channel status file");
  }

  std::string EcalSubDet;
  std::string str;
  int hashIndex(0);
  int status(0);

  while (!statusFile.eof()) {
    statusFile >> EcalSubDet;
    if (EcalSubDet != std::string("EB") && EcalSubDet != std::string("EE")) {
      std::getline(statusFile, str);
      continue;
    } else {
      statusFile >> hashIndex >> status;
    }
    // std::cout << EcalSubDet << " " << hashIndex << " " << status;

    if (EcalSubDet == std::string("EB")) {
      EBDetId ebid = EBDetId::unhashIndex(hashIndex);
      ecalStatus->setValue(ebid, status);
    } else if (EcalSubDet == std::string("EE")) {
      EEDetId eedetid = EEDetId::unhashIndex(hashIndex);
      ecalStatus->setValue(eedetid, status);
    } else {
      edm::LogError("EcalTrivialConditionRetriever") << " *** " << EcalSubDet << " is neither EB nor EE ";
    }
  }
  // the file is supposed to be in the form  -- FIXME

  statusFile.close();
  return ecalStatus;
}

std::unique_ptr<EcalTPGCrystalStatus> EcalTrivialConditionRetriever::produceEcalTrgChannelStatus(
    const EcalTPGCrystalStatusRcd&) {
  auto ical = std::make_unique<EcalTPGCrystalStatus>();
  // barrel
  for (int ieta = -EBDetId::MAX_IETA; ieta <= EBDetId::MAX_IETA; ++ieta) {
    if (ieta == 0)
      continue;
    for (int iphi = EBDetId::MIN_IPHI; iphi <= EBDetId::MAX_IPHI; ++iphi) {
      if (EBDetId::validDetId(ieta, iphi)) {
        EBDetId ebid(ieta, iphi);
        ical->setValue(ebid, 0);
      }
    }
  }
  // endcap
  for (int iX = EEDetId::IX_MIN; iX <= EEDetId::IX_MAX; ++iX) {
    for (int iY = EEDetId::IY_MIN; iY <= EEDetId::IY_MAX; ++iY) {
      // make an EEDetId since we need EEDetId::rawId() to be used as the key for the pedestals
      if (EEDetId::validDetId(iX, iY, 1)) {
        EEDetId eedetidpos(iX, iY, 1);
        ical->setValue(eedetidpos, 0);
      }
      if (EEDetId::validDetId(iX, iY, -1)) {
        EEDetId eedetidneg(iX, iY, -1);
        ical->setValue(eedetidneg, 0);
      }
    }
  }
  return ical;
}

std::unique_ptr<EcalIntercalibConstants> EcalTrivialConditionRetriever::getIntercalibConstantsFromConfiguration(
    const EcalIntercalibConstantsRcd&) {
  std::unique_ptr<EcalIntercalibConstants> ical;
  //        std::make_unique<EcalIntercalibConstants>();

  // Read the values from a txt file
  // -------------------------------

  edm::LogInfo("EcalTrivialConditionRetriever")
      << "Reading intercalibration constants from file " << intercalibConstantsFile_.c_str();

  if (intercalibConstantsFile_.find(".xml") != std::string::npos) {
    edm::LogInfo("generating Intercalib from xml file");

    EcalCondHeader h;
    EcalIntercalibConstants* rcd = new EcalIntercalibConstants;
    EcalIntercalibConstantsXMLTranslator::readXML(intercalibConstantsFile_, h, *rcd);

    if (totLumi_ != 0 || instLumi_ != 0) {
      edm::LogInfo("implementing ageing for intercalib");

      EcalIntercalibConstantsMC* rcdMC = new EcalIntercalibConstantsMC;

      if (intercalibConstantsMCFile_.find(".xml") != std::string::npos) {
        edm::LogInfo("generating IntercalibMC from xml file");

        EcalCondHeader h;
        EcalIntercalibConstantsMCXMLTranslator::readXML(intercalibConstantsMCFile_, h, *rcdMC);

      } else {
        edm::LogInfo("please provide the xml file of the EcalIntercalibConstantsMC");
      }

      TRandom3* gRandom = new TRandom3();

      EnergyResolutionVsLumi ageing;
      ageing.setLumi(totLumi_);
      ageing.setInstLumi(instLumi_);

      const EcalIntercalibConstantMap& mymap = rcd->getMap();
      const EcalIntercalibConstantMCMap& mymapMC = rcdMC->getMap();

      for (int ieta = -EBDetId::MAX_IETA; ieta <= EBDetId::MAX_IETA; ++ieta) {
        if (ieta == 0)
          continue;

        double eta = EBDetId::approxEta(EBDetId(ieta, 1));
        eta = fabs(eta);
        double constantTerm = ageing.calcresolutitonConstantTerm(eta);
        edm::LogInfo("EB at eta=") << eta << " constant term is " << constantTerm;

        for (int iphi = EBDetId::MIN_IPHI; iphi <= EBDetId::MAX_IPHI; ++iphi) {
          // make an EBDetId since we need EBDetId::rawId() to be used as the key for the pedestals
          if (EBDetId::validDetId(ieta, iphi)) {
            EBDetId ebid(ieta, iphi);
            EcalIntercalibConstants::const_iterator idref = mymap.find(ebid);
            EcalIntercalibConstant icalconstant = 1;
            if (idref != mymap.end())
              icalconstant = (*idref);

            EcalIntercalibConstantsMC::const_iterator idrefMC = mymapMC.find(ebid);
            EcalIntercalibConstantMC icalconstantMC = 1;
            if (idrefMC != mymapMC.end())
              icalconstantMC = (*idrefMC);

            double r = gRandom->Gaus(0, constantTerm);

            if (iphi == 10)
              edm::LogInfo("EB at eta=") << eta << " IC=" << icalconstant << " ICMC=" << icalconstantMC
                                         << " smear=" << r << " ";

            icalconstant = icalconstant + r * 1.29 * icalconstantMC;
            rcd->setValue(ebid.rawId(), icalconstant);

            if (iphi == 10)
              edm::LogInfo("newIC=") << icalconstant;

            EcalIntercalibConstant icalconstant2 = (*idref);
            if (icalconstant != icalconstant2)
              edm::LogInfo(">>>> error in smearing intercalib");
          }
        }
      }

      for (int iX = EEDetId::IX_MIN; iX <= EEDetId::IX_MAX; ++iX) {
        for (int iY = EEDetId::IY_MIN; iY <= EEDetId::IY_MAX; ++iY) {
          // make an EEDetId since we need EEDetId::rawId() to be used as the key for the pedestals
          if (EEDetId::validDetId(iX, iY, 1)) {
            EEDetId eedetidpos(iX, iY, 1);
            double eta =
                -log(tan(0.5 * atan(sqrt((iX - 50.0) * (iX - 50.0) + (iY - 50.0) * (iY - 50.0)) * 2.98 / 328.)));
            eta = fabs(eta);
            double constantTerm = ageing.calcresolutitonConstantTerm(eta);
            if (iX == 50)
              edm::LogInfo("EE at eta=") << eta << " constant term is " << constantTerm;

            EcalIntercalibConstants::const_iterator idref = mymap.find(eedetidpos);
            EcalIntercalibConstant icalconstant = 1;
            if (idref != mymap.end())
              icalconstant = (*idref);

            EcalIntercalibConstantsMC::const_iterator idrefMC = mymapMC.find(eedetidpos);
            EcalIntercalibConstantMC icalconstantMC = 1;
            if (idrefMC != mymapMC.end())
              icalconstantMC = (*idrefMC);

            double r = gRandom->Gaus(0, constantTerm);

            if (iX == 10)
              edm::LogInfo("EE at eta=") << eta << " IC=" << icalconstant << " ICMC=" << icalconstantMC
                                         << " smear=" << r << " ";
            icalconstant = icalconstant + r * 1.29 * icalconstantMC;
            rcd->setValue(eedetidpos.rawId(), icalconstant);
            if (iX == 10)
              edm::LogInfo("newIC=") << icalconstant;
          }
          if (EEDetId::validDetId(iX, iY, -1)) {
            EEDetId eedetidneg(iX, iY, -1);
            double eta =
                -log(tan(0.5 * atan(sqrt((iX - 50.0) * (iX - 50.0) + (iY - 50.0) * (iY - 50.0)) * 2.98 / 328.)));
            eta = fabs(eta);
            double constantTerm = ageing.calcresolutitonConstantTerm(eta);
            EcalIntercalibConstant icalconstant = 1;

            EcalIntercalibConstantsMC::const_iterator idrefMC = mymapMC.find(eedetidneg);
            EcalIntercalibConstantMC icalconstantMC = 1;
            if (idrefMC != mymapMC.end())
              icalconstantMC = (*idrefMC);

            double r = gRandom->Gaus(0, constantTerm);
            icalconstant = icalconstant + r * 1.29 * icalconstantMC;
            rcd->setValue(eedetidneg.rawId(), icalconstant);
          }
        }
      }

      ical = std::unique_ptr<EcalIntercalibConstants>(rcd);

      delete gRandom;
    }

  } else {
    ical = std::make_unique<EcalIntercalibConstants>();

    FILE* inpFile;
    inpFile = fopen(intercalibConstantsFile_.c_str(), "r");
    if (!inpFile) {
      edm::LogError("EcalTrivialConditionRetriever") << "*** Can not open file: " << intercalibConstantsFile_;
      throw cms::Exception("Cannot open inter-calibration coefficients txt file");
    }

    char line[256];
    std::ostringstream str;
    fgets(line, 255, inpFile);
    int sm_number = atoi(line);
    str << "sm: " << sm_number;

    fgets(line, 255, inpFile);
    //int nevents=atoi (line) ; // not necessary here just for online conddb

    fgets(line, 255, inpFile);
    std::string gen_tag = line;
    str << "gen tag: " << gen_tag;  // should I use this?

    fgets(line, 255, inpFile);
    std::string cali_method = line;
    str << "cali method: " << cali_method << std::endl;  // not important

    fgets(line, 255, inpFile);
    std::string cali_version = line;
    str << "cali version: " << cali_version << std::endl;  // not important

    fgets(line, 255, inpFile);
    std::string cali_type = line;
    str << "cali type: " << cali_type;  // not important

    edm::LogInfo("EcalTrivialConditionRetriever") << "[PIETRO] Intercalibration file - " << str.str();

    float calib[1700] = {1};
    int calib_status[1700] = {0};

    int ii = 0;

    while (fgets(line, 255, inpFile)) {
      ii++;
      int dmy_num = 0;
      float dmy_calib = 0.;
      float dmy_RMS = 0.;
      int dmy_events = 0;
      int dmy_status = 0;
      sscanf(line, "%d %f %f %d %d", &dmy_num, &dmy_calib, &dmy_RMS, &dmy_events, &dmy_status);
      assert(dmy_num >= 1);
      assert(dmy_num <= 1700);
      calib[dmy_num - 1] = dmy_calib;
      calib_status[dmy_num - 1] = dmy_status;

      //       edm::LogInfo ("EcalTrivialConditionRetriever")
      //                 << "[PIETRO] cry = " << dmy_num
      //                 << " calib = " << calib[dmy_num-1]
      //                 << " RMS = " << dmy_RMS
      //                 << " events = " << dmy_events
      //                 << " status = " << calib_status[dmy_num-1]
      //                 << std::endl ;
    }

    fclose(inpFile);  // close inp. file
    edm::LogInfo("EcalTrivialConditionRetriever") << "Read intercalibrations for " << ii << " xtals ";
    if (ii != 1700)
      edm::LogWarning("StoreEcalCondition") << "Some crystals missing, set to 1" << std::endl;

    // Transfer the data to the inter-calibration coefficients container
    // -----------------------------------------------------------------

    // DB supermodule always set to 1 for the TestBeam FIXME
    int sm_db = 1;
    // loop over channels
    for (int i = 0; i < 1700; i++) {
      //if (EBDetId::validDetId(iEta,iPhi)) {
      // CANNOT be used -- validDetId only checks ETA PHI method
      // doing a check by hand, here is the only place in CMSSW
      // outside TB code and EcalRawToDigi where needed
      // => no need for changing the DetId interface
      //
      // checking only sm_db -- guess can change with the above FIXME
      if (sm_db >= EBDetId::MIN_SM && sm_db <= EBDetId::MAX_SM) {
        EBDetId ebid(sm_db, i + 1, EBDetId::SMCRYSTALMODE);
        if (calib_status[i])
          ical->setValue(ebid.rawId(), calib[i]);
        else
          ical->setValue(ebid.rawId(), 1.);
      }
      //}
    }  // loop over channels
  }

  return ical;
}

// new for the PF rec hit thresholds

std::unique_ptr<EcalPFRecHitThresholds> EcalTrivialConditionRetriever::getPFRecHitThresholdsFromConfiguration(
    const EcalPFRecHitThresholdsRcd&) {
  std::unique_ptr<EcalPFRecHitThresholds> ical;

  // Reads the values from a txt file
  edm::LogInfo("EcalTrivialConditionRetriever")
      << "Reading PF RecHit Thresholds from file " << edm::FileInPath(pfRecHitFile_).fullPath().c_str();

  std::ifstream PFRecHitsFile(edm::FileInPath(pfRecHitFile_).fullPath().c_str());

  ical = std::make_unique<EcalPFRecHitThresholds>();

  // char line[50];

  int nxt = 0;

  edm::LogInfo("Going to multiply the sigmas by ") << pfRecHitThresholdsNSigmas_;
  edm::LogInfo("We will print some values ");

  while (!PFRecHitsFile.eof()) {
    //  while (fgets(line,50,inpFile)) {
    float thresh;
    int eta = 0;
    int phi = 0;
    int zeta = 0;
    PFRecHitsFile >> eta >> phi >> zeta >> thresh;
    //    sscanf(line, "%d %d %d %f", &eta, &phi, &zeta, &thresh);

    thresh = thresh * pfRecHitThresholdsNSigmas_;

    if (phi == 50)
      edm::LogInfo("EB ") << std::dec << eta << "/" << std::dec << phi << "/" << std::dec << zeta
                          << " thresh: " << thresh;
    nxt = nxt + 1;
    if (EBDetId::validDetId(eta, phi)) {
      EBDetId ebid(eta, phi);
      ical->setValue(ebid.rawId(), thresh);
    }
  }
  PFRecHitsFile.close();
  // fclose(inpFile);

  edm::LogInfo("Read number of EB crystals: ") << nxt;

  //******************************************************
  edm::LogInfo("Now reading the EE file ... ");
  edm::LogInfo("We will multiply the sigma in EE by ") << pfRecHitThresholdsNSigmas_;
  edm::LogInfo("We will multiply the sigma in EE at high eta by") << pfRecHitThresholdsNSigmasHEta_;
  edm::LogInfo("We will print some values ");

  edm::LogInfo("EcalTrivialConditionRetriever")
      << "Reading PF RecHit Thresholds EE from file " << edm::FileInPath(pfRecHitFileEE_).fullPath().c_str();
  std::ifstream PFRecHitsFileEE(edm::FileInPath(pfRecHitFileEE_).fullPath().c_str());

  nxt = 0;

  while (!PFRecHitsFileEE.eof()) {
    //  while (fgets(line,40,inpFileEE)) {
    float thresh;
    int ix = 0;
    int iy = 0;
    int iz = 0;
    PFRecHitsFileEE >> ix >> iy >> iz >> thresh;
    //    sscanf(line, "%d %d %d %f", &ix, &iy,&iz, &thresh);

    double eta =
        -log(tan(0.5 * atan(sqrt((ix - 50.5) * (ix - 50.5) + (iy - 50.5) * (iy - 50.5)) * 2.98 / 328.)));  // approx eta

    if (eta > 2.5) {
      thresh = thresh * pfRecHitThresholdsNSigmasHEta_;
    } else {
      thresh = thresh * pfRecHitThresholdsNSigmas_;
    }

    if (ix == 50)
      edm::LogInfo("EE ") << std::dec << ix << "/" << std::dec << iy << "/" << std::dec << iz << " thresh: " << thresh;
    if (EEDetId::validDetId(ix, iy, iz)) {
      EEDetId eeid(ix, iy, iz);
      ical->setValue(eeid.rawId(), thresh);
    }
    nxt = nxt + 1;
  }
  PFRecHitsFileEE.close();
  //  fclose(inpFileEE);
  edm::LogInfo("Read number of EE crystals: ") << nxt;
  edm::LogInfo("end PF Rec Hits ... ");

  return ical;
}

std::unique_ptr<EcalIntercalibConstantsMC> EcalTrivialConditionRetriever::getIntercalibConstantsMCFromConfiguration(
    const EcalIntercalibConstantsMCRcd&) {
  std::unique_ptr<EcalIntercalibConstantsMC> ical;
  //        std::make_unique<EcalIntercalibConstants>();

  // Read the values from a xml file
  // -------------------------------

  edm::LogInfo("EcalTrivialConditionRetriever")
      << "Reading intercalibration constants MC from file " << intercalibConstantsMCFile_.c_str();

  if (intercalibConstantsMCFile_.find(".xml") != std::string::npos) {
    edm::LogInfo("generating Intercalib MC from xml file");

    EcalCondHeader h;
    EcalIntercalibConstantsMC* rcd = new EcalIntercalibConstantsMC;
    EcalIntercalibConstantsMCXMLTranslator::readXML(intercalibConstantsMCFile_, h, *rcd);

    ical = std::unique_ptr<EcalIntercalibConstants>(rcd);

  } else {
    edm::LogInfo("ERROR>>> please provide a xml file");
  }

  return ical;
}

std::unique_ptr<EcalIntercalibErrors> EcalTrivialConditionRetriever::getIntercalibErrorsFromConfiguration(
    const EcalIntercalibErrorsRcd&) {
  auto ical = std::make_unique<EcalIntercalibErrors>();

  // Read the values from a txt file
  // -------------------------------

  edm::LogInfo("EcalTrivialConditionRetriever")
      << "Reading intercalibration constants from file " << intercalibErrorsFile_.c_str();

  FILE* inpFile;
  inpFile = fopen(intercalibErrorsFile_.c_str(), "r");
  if (!inpFile) {
    edm::LogError("EcalTrivialConditionRetriever") << "*** Can not open file: " << intercalibErrorsFile_;
    throw cms::Exception("Cannot open inter-calibration coefficients txt file");
  }

  char line[256];
  std::ostringstream str;
  fgets(line, 255, inpFile);
  int sm_number = atoi(line);
  str << "sm: " << sm_number;

  fgets(line, 255, inpFile);
  //int nevents=atoi (line) ; // not necessary here just for online conddb

  fgets(line, 255, inpFile);
  std::string gen_tag = line;
  str << "gen tag: " << gen_tag;  // should I use this?

  fgets(line, 255, inpFile);
  std::string cali_method = line;
  str << "cali method: " << cali_method << std::endl;  // not important

  fgets(line, 255, inpFile);
  std::string cali_version = line;
  str << "cali version: " << cali_version << std::endl;  // not important

  fgets(line, 255, inpFile);
  std::string cali_type = line;
  str << "cali type: " << cali_type;  // not important

  edm::LogInfo("EcalTrivialConditionRetriever") << "[PIETRO] Intercalibration file - " << str.str() << std::endl;

  float calib[1700] = {1};
  int calib_status[1700] = {0};

  int ii = 0;

  while (fgets(line, 255, inpFile)) {
    ii++;
    int dmy_num = 0;
    float dmy_calib = 0.;
    float dmy_RMS = 0.;
    int dmy_events = 0;
    int dmy_status = 0;
    sscanf(line, "%d %f %f %d %d", &dmy_num, &dmy_calib, &dmy_RMS, &dmy_events, &dmy_status);
    assert(dmy_num >= 1);
    assert(dmy_num <= 1700);
    calib[dmy_num - 1] = dmy_calib;
    calib_status[dmy_num - 1] = dmy_status;

    //       edm::LogInfo ("EcalTrivialConditionRetriever")
    //                 << "[PIETRO] cry = " << dmy_num
    //                 << " calib = " << calib[dmy_num-1]
    //                 << " RMS = " << dmy_RMS
    //                 << " events = " << dmy_events
    //                 << " status = " << calib_status[dmy_num-1]
    //                 << std::endl ;
  }

  fclose(inpFile);  // close inp. file
  edm::LogInfo("EcalTrivialConditionRetriever") << "Read intercalibrations for " << ii << " xtals ";
  if (ii != 1700)
    edm::LogWarning("StoreEcalCondition") << "Some crystals missing, set to 1" << std::endl;

  // Transfer the data to the inter-calibration coefficients container
  // -----------------------------------------------------------------

  // DB supermodule always set to 1 for the TestBeam FIXME
  int sm_db = 1;
  // loop over channels
  for (int i = 0; i < 1700; i++) {
    //if (EBDetId::validDetId(iEta,iPhi)) {
    // CANNOT be used -- validDetId only checks ETA PHI method
    // doing a check by hand, here is the only place in CMSSW
    // outside TB code and EcalRawToDigi where needed
    // => no need for changing the DetId interface
    //
    // checking only sm_db -- guess can change with the above FIXME
    if (sm_db >= EBDetId::MIN_SM && sm_db <= EBDetId::MAX_SM) {
      EBDetId ebid(sm_db, i + 1, EBDetId::SMCRYSTALMODE);
      if (calib_status[i])
        ical->setValue(ebid.rawId(), calib[i]);
      else
        ical->setValue(ebid.rawId(), 1.);
    }
    //}
  }  // loop over channels

  //  edm::LogInfo ("EcalTrivialConditionRetriever") << "INTERCALIBRATION DONE" ;
  return ical;
}

// --------------------------------------------------------------------------------

std::unique_ptr<EcalTimeCalibConstants> EcalTrivialConditionRetriever::getTimeCalibConstantsFromConfiguration(
    const EcalTimeCalibConstantsRcd&) {
  auto ical = std::make_unique<EcalTimeCalibConstants>();

  // Read the values from a txt file
  // -------------------------------

  edm::LogInfo("EcalTrivialConditionRetriever")
      << "Reading time calibration constants from file " << timeCalibConstantsFile_.c_str();

  FILE* inpFile;
  inpFile = fopen(timeCalibConstantsFile_.c_str(), "r");
  if (!inpFile) {
    edm::LogError("EcalTrivialConditionRetriever") << "*** Can not open file: " << timeCalibConstantsFile_;
    throw cms::Exception("Cannot open inter-calibration coefficients txt file");
  }

  char line[256];
  std::ostringstream str;
  fgets(line, 255, inpFile);
  int sm_number = atoi(line);
  str << "sm: " << sm_number;

  fgets(line, 255, inpFile);
  //int nevents=atoi (line) ; // not necessary here just for online conddb

  fgets(line, 255, inpFile);
  std::string gen_tag = line;
  str << "gen tag: " << gen_tag;  // should I use this?

  fgets(line, 255, inpFile);
  std::string cali_method = line;
  str << "cali method: " << cali_method << std::endl;  // not important

  fgets(line, 255, inpFile);
  std::string cali_version = line;
  str << "cali version: " << cali_version << std::endl;  // not important

  fgets(line, 255, inpFile);
  std::string cali_type = line;
  str << "cali type: " << cali_type;  // not important

  edm::LogInfo("EcalTrivialConditionRetriever") << "TimeCalibration file - " << str.str() << std::endl;

  float calib[1700] = {1};
  int calib_status[1700] = {0};

  int ii = 0;

  while (fgets(line, 255, inpFile)) {
    ii++;
    int dmy_num = 0;
    float dmy_calib = 0.;
    float dmy_RMS = 0.;
    int dmy_events = 0;
    int dmy_status = 0;
    sscanf(line, "%d %f %f %d %d", &dmy_num, &dmy_calib, &dmy_RMS, &dmy_events, &dmy_status);
    assert(dmy_num >= 1);
    assert(dmy_num <= 1700);
    calib[dmy_num - 1] = dmy_calib;
    calib_status[dmy_num - 1] = dmy_status;

    //       edm::LogInfo ("EcalTrivialConditionRetriever")
    //                 << "[PIETRO] cry = " << dmy_num
    //                 << " calib = " << calib[dmy_num-1]
    //                 << " RMS = " << dmy_RMS
    //                 << " events = " << dmy_events
    //                 << " status = " << calib_status[dmy_num-1]
    //                 << std::endl ;
  }

  fclose(inpFile);  // close inp. file
  edm::LogInfo("EcalTrivialConditionRetriever") << "Read timeCalibrations for " << ii << " xtals ";
  if (ii != 1700)
    edm::LogWarning("StoreEcalCondition") << "Some crystals missing, set to 1" << std::endl;

  // Transfer the data to the inter-calibration coefficients container
  // -----------------------------------------------------------------

  // DB supermodule always set to 1 for the TestBeam FIXME
  int sm_db = 1;
  // loop over channels
  for (int i = 0; i < 1700; i++) {
    //if (EBDetId::validDetId(iEta,iPhi)) {
    // CANNOT be used -- validDetId only checks ETA PHI method
    // doing a check by hand, here is the only place in CMSSW
    // outside TB code and EcalRawToDigi where needed
    // => no need for changing the DetId interface
    //
    // checking only sm_db -- guess can change with the above FIXME
    if (sm_db >= EBDetId::MIN_SM && sm_db <= EBDetId::MAX_SM) {
      EBDetId ebid(sm_db, i + 1, EBDetId::SMCRYSTALMODE);
      if (calib_status[i])
        ical->setValue(ebid.rawId(), calib[i]);
      else
        ical->setValue(ebid.rawId(), 1.);
    }
    //}
  }  // loop over channels

  //  edm::LogInfo ("EcalTrivialConditionRetriever") << "INTERCALIBRATION DONE" ;
  return ical;
}

std::unique_ptr<EcalTimeCalibErrors> EcalTrivialConditionRetriever::getTimeCalibErrorsFromConfiguration(
    const EcalTimeCalibErrorsRcd&) {
  auto ical = std::make_unique<EcalTimeCalibErrors>();

  // Read the values from a txt file
  // -------------------------------

  edm::LogInfo("EcalTrivialConditionRetriever")
      << "Reading timeCalibration constants from file " << timeCalibErrorsFile_.c_str();

  FILE* inpFile;
  inpFile = fopen(timeCalibErrorsFile_.c_str(), "r");
  if (!inpFile) {
    edm::LogError("EcalTrivialConditionRetriever") << "*** Can not open file: " << timeCalibErrorsFile_;
    throw cms::Exception("Cannot open inter-calibration coefficients txt file");
  }

  char line[256];
  std::ostringstream str;
  fgets(line, 255, inpFile);
  int sm_number = atoi(line);
  str << "sm: " << sm_number;

  fgets(line, 255, inpFile);
  //int nevents=atoi (line) ; // not necessary here just for online conddb

  fgets(line, 255, inpFile);
  std::string gen_tag = line;
  str << "gen tag: " << gen_tag;  // should I use this?

  fgets(line, 255, inpFile);
  std::string cali_method = line;
  str << "cali method: " << cali_method << std::endl;  // not important

  fgets(line, 255, inpFile);
  std::string cali_version = line;
  str << "cali version: " << cali_version << std::endl;  // not important

  fgets(line, 255, inpFile);
  std::string cali_type = line;
  str << "cali type: " << cali_type;  // not important

  edm::LogInfo("EcalTrivialConditionRetriever") << "TimeCalibration file - " << str.str() << std::endl;

  float calib[1700] = {1};
  int calib_status[1700] = {0};

  int ii = 0;

  while (fgets(line, 255, inpFile)) {
    ii++;
    int dmy_num = 0;
    float dmy_calib = 0.;
    float dmy_RMS = 0.;
    int dmy_events = 0;
    int dmy_status = 0;
    sscanf(line, "%d %f %f %d %d", &dmy_num, &dmy_calib, &dmy_RMS, &dmy_events, &dmy_status);
    assert(dmy_num >= 1);
    assert(dmy_num <= 1700);
    calib[dmy_num - 1] = dmy_calib;
    calib_status[dmy_num - 1] = dmy_status;

    //       edm::LogInfo ("EcalTrivialConditionRetriever")
    //                 << "[PIETRO] cry = " << dmy_num
    //                 << " calib = " << calib[dmy_num-1]
    //                 << " RMS = " << dmy_RMS
    //                 << " events = " << dmy_events
    //                 << " status = " << calib_status[dmy_num-1]
    //                 << std::endl ;
  }

  fclose(inpFile);  // close inp. file
  edm::LogInfo("EcalTrivialConditionRetriever") << "Read time calibrations for " << ii << " xtals ";
  if (ii != 1700)
    edm::LogWarning("StoreEcalCondition") << "Some crystals missing, set to 1" << std::endl;

  // Transfer the data to the inter-calibration coefficients container
  // -----------------------------------------------------------------

  // DB supermodule always set to 1 for the TestBeam FIXME
  int sm_db = 1;
  // loop over channels
  for (int i = 0; i < 1700; i++) {
    //if (EBDetId::validDetId(iEta,iPhi)) {
    // CANNOT be used -- validDetId only checks ETA PHI method
    // doing a check by hand, here is the only place in CMSSW
    // outside TB code and EcalRawToDigi where needed
    // => no need for changing the DetId interface
    //
    // checking only sm_db -- guess can change with the above FIXME
    if (sm_db >= EBDetId::MIN_SM && sm_db <= EBDetId::MAX_SM) {
      EBDetId ebid(sm_db, i + 1, EBDetId::SMCRYSTALMODE);
      if (calib_status[i])
        ical->setValue(ebid.rawId(), calib[i]);
      else
        ical->setValue(ebid.rawId(), 1.);
    }
    //}
  }  // loop over channels

  //  edm::LogInfo ("EcalTrivialConditionRetriever") << "INTERCALIBRATION DONE" ;
  return ical;
}

// --------------------------------------------------------------------------------

std::unique_ptr<EcalMappingElectronics> EcalTrivialConditionRetriever::getMappingFromConfiguration(
    const EcalMappingElectronicsRcd&) {
  auto mapping = std::make_unique<EcalMappingElectronics>();
  edm::LogInfo("EcalTrivialConditionRetriever")
      << "Reading mapping from file " << edm::FileInPath(mappingFile_).fullPath().c_str();

  std::ifstream f(edm::FileInPath(mappingFile_).fullPath().c_str());
  if (!f.good()) {
    edm::LogError("EcalTrivialConditionRetriever") << "File not found";
    throw cms::Exception("FileNotFound");
  }

  // uint32_t detid, elecid, triggerid;

  int ix, iy, iz, CL;
  // int dccid, towerid, stripid, xtalid;
  // int tccid, tower, ipseudostrip, xtalinps;
  int dccid, towerid, pseudostrip_in_SC, xtal_in_pseudostrip;
  int tccid, tower, pseudostrip_in_TCC, pseudostrip_in_TT;

  while (!f.eof()) {
    // f >> detid >> elecid >> triggerid;
    f >> ix >> iy >> iz >> CL >> dccid >> towerid >> pseudostrip_in_SC >> xtal_in_pseudostrip >> tccid >> tower >>
        pseudostrip_in_TCC >> pseudostrip_in_TT;

    //       if (!EEDetId::validDetId(ix,iy,iz))
    // 	  continue;

    EEDetId detid(ix, iy, iz, EEDetId::XYMODE);
    // std::cout << " dcc tower ps_in_SC xtal_in_ps " << dccid << " " << towerid << " " << pseudostrip_in_SC << " " << xtal_in_pseudostrip << std::endl;
    EcalElectronicsId elecid(dccid, towerid, pseudostrip_in_SC, xtal_in_pseudostrip);
    // std::cout << " tcc tt ps_in_TT xtal_in_ps " << tccid << " " << tower << " " << pseudostrip_in_TT << " " << xtal_in_pseudostrip << std::endl;
    EcalTriggerElectronicsId triggerid(tccid, tower, pseudostrip_in_TT, xtal_in_pseudostrip);
    EcalMappingElement aElement;
    aElement.electronicsid = elecid.rawId();
    aElement.triggerid = triggerid.rawId();
    (*mapping).setValue(detid, aElement);
  }

  f.close();
  return mapping;
}

std::unique_ptr<EcalMappingElectronics> EcalTrivialConditionRetriever::produceEcalMappingElectronics(
    const EcalMappingElectronicsRcd&) {
  auto ical = std::make_unique<EcalMappingElectronics>();
  return ical;
}

// --------------------------------------------------------------------------------

std::unique_ptr<Alignments> EcalTrivialConditionRetriever::produceEcalAlignmentEB(const EBAlignmentRcd&) {
  double mytrans[3] = {0., 0., 0.};
  double myeuler[3] = {0., 0., 0.};
  std::ifstream f;
  if (getEBAlignmentFromFile_)
    f.open(edm::FileInPath(EBAlignmentFile_).fullPath().c_str());
  std::vector<AlignTransform> my_align;
  int ieta = 1;
  int iphi = 0;
  for (int SM = 1; SM < 37; SM++) {
    // make an EBDetId since we need EBDetId::rawId()
    if (SM > 18) {
      iphi = 1 + (SM - 19) * 20;
      ieta = -1;
    } else
      iphi = SM * 20;
    EBDetId ebdetId(ieta, iphi);
    if (getEBAlignmentFromFile_) {
      f >> myeuler[0] >> myeuler[1] >> myeuler[2] >> mytrans[0] >> mytrans[1] >> mytrans[2];
      edm::LogInfo(" translation ") << mytrans[0] << " " << mytrans[1] << " " << mytrans[2] << "\n"
                                    << " euler " << myeuler[0] << " " << myeuler[1] << " " << myeuler[2];
    }
    CLHEP::Hep3Vector translation(mytrans[0], mytrans[1], mytrans[2]);
    CLHEP::HepEulerAngles euler(myeuler[0], myeuler[1], myeuler[2]);
    AlignTransform transform(translation, euler, ebdetId);
    my_align.push_back(transform);
  }
  /*
  // check
  uint32_t m_rawid;
  double m_x, m_y, m_z;
  double m_phi, m_theta, m_psi;
  int SM = 0;
  for (std::vector<AlignTransform>::const_iterator i = my_align.begin(); i != my_align.end(); ++i){	
    m_rawid = i->rawId();
    CLHEP::Hep3Vector translation = i->translation();
    m_x = translation.x();
    m_y = translation.y();
    m_z = translation.z();
    CLHEP::HepRotation rotation = i->rotation();
    m_phi = rotation.getPhi();
    m_theta = rotation.getTheta();
    m_psi = rotation.getPsi();
    SM++;
    std::cout << " SM " << SM << " Id " << m_rawid << " x " << m_x << " y " << m_y << " z " << m_z
	      << " phi " << m_phi << " theta " << m_theta << " psi " << m_psi << std::endl;
  }
  */
  Alignments a;
  a.m_align = my_align;

  return std::make_unique<Alignments>(a);
}

std::unique_ptr<Alignments> EcalTrivialConditionRetriever::produceEcalAlignmentEE(const EEAlignmentRcd&) {
  double mytrans[3] = {0., 0., 0.};
  double myeuler[3] = {0., 0., 0.};
  std::ifstream f;
  if (getEEAlignmentFromFile_)
    f.open(edm::FileInPath(EEAlignmentFile_).fullPath().c_str());
  std::vector<AlignTransform> my_align;
  int ix = 20;
  int iy = 50;
  int side = -1;
  for (int Dee = 0; Dee < 4; Dee++) {
    // make an EEDetId since we need EEDetId::rawId()
    if (Dee == 1 || Dee == 3)
      ix = 70;
    else
      ix = 20;
    if (Dee == 2)
      side = 1;
    EEDetId eedetId(ix, iy, side);
    if (getEEAlignmentFromFile_) {
      f >> myeuler[0] >> myeuler[1] >> myeuler[2] >> mytrans[0] >> mytrans[1] >> mytrans[2];
      edm::LogInfo(" translation ") << mytrans[0] << " " << mytrans[1] << " " << mytrans[2] << "\n"
                                    << " euler " << myeuler[0] << " " << myeuler[1] << " " << myeuler[2];
    }
    CLHEP::Hep3Vector translation(mytrans[0], mytrans[1], mytrans[2]);
    CLHEP::HepEulerAngles euler(myeuler[0], myeuler[1], myeuler[2]);
    AlignTransform transform(translation, euler, eedetId);
    my_align.push_back(transform);
  }
  Alignments a;
  a.m_align = my_align;
  return std::make_unique<Alignments>(a);
}

std::unique_ptr<Alignments> EcalTrivialConditionRetriever::produceEcalAlignmentES(const ESAlignmentRcd&) {
  double mytrans[3] = {0., 0., 0.};
  double myeuler[3] = {0., 0., 0.};
  std::ifstream f;
  if (getESAlignmentFromFile_)
    f.open(edm::FileInPath(ESAlignmentFile_).fullPath().c_str());
  std::vector<AlignTransform> my_align;
  //  int ix_vect[10] = {10, 30, 30, 50, 10, 30, 10, 30};
  int pl_vect[10] = {2, 2, 1, 1, 1, 1, 2, 2};
  int iy = 10;
  int side = -1;
  int strip = 1;
  for (int layer = 0; layer < 8; layer++) {
    // make an ESDetId since we need ESDetId::rawId()
    int ix = 10 + (layer % 2) * 20;
    int plane = pl_vect[layer];
    if (layer > 3)
      side = 1;
    ESDetId esdetId(strip, ix, iy, plane, side);
    if (getESAlignmentFromFile_) {
      f >> myeuler[0] >> myeuler[1] >> myeuler[2] >> mytrans[0] >> mytrans[1] >> mytrans[2];
      edm::LogInfo(" translation ") << mytrans[0] << " " << mytrans[1] << " " << mytrans[2] << "\n"
                                    << " euler " << myeuler[0] << " " << myeuler[1] << " " << myeuler[2];
    }
    CLHEP::Hep3Vector translation(mytrans[0], mytrans[1], mytrans[2]);
    CLHEP::HepEulerAngles euler(myeuler[0], myeuler[1], myeuler[2]);
    AlignTransform transform(translation, euler, esdetId);
    my_align.push_back(transform);
  }
  Alignments a;
  a.m_align = my_align;
  return std::make_unique<Alignments>(a);
}

std::unique_ptr<EcalSampleMask> EcalTrivialConditionRetriever::produceEcalSampleMask(const EcalSampleMaskRcd&) {
  return std::make_unique<EcalSampleMask>(sampleMaskEB_, sampleMaskEE_);
}

std::unique_ptr<EcalTimeBiasCorrections> EcalTrivialConditionRetriever::produceEcalTimeBiasCorrections(
    const EcalTimeBiasCorrectionsRcd&) {
  auto ipar = std::make_unique<EcalTimeBiasCorrections>();
  copy(EBtimeCorrAmplitudeBins_.begin(), EBtimeCorrAmplitudeBins_.end(), back_inserter(ipar->EBTimeCorrAmplitudeBins));
  copy(EBtimeCorrShiftBins_.begin(), EBtimeCorrShiftBins_.end(), back_inserter(ipar->EBTimeCorrShiftBins));
  copy(EEtimeCorrAmplitudeBins_.begin(), EEtimeCorrAmplitudeBins_.end(), back_inserter(ipar->EETimeCorrAmplitudeBins));
  copy(EEtimeCorrShiftBins_.begin(), EEtimeCorrShiftBins_.end(), back_inserter(ipar->EETimeCorrShiftBins));
  return ipar;
}

std::unique_ptr<EcalSamplesCorrelation> EcalTrivialConditionRetriever::produceEcalSamplesCorrelation(
    const EcalSamplesCorrelationRcd&) {
  if (getSamplesCorrelationFromFile_) {
    std::ifstream f;
    f.open(edm::FileInPath(SamplesCorrelationFile_).fullPath().c_str());
    float ww;
    for (int j = 0; j < 10; ++j) {
      f >> ww;
      EBG12samplesCorrelation_.push_back(ww);
    }
    for (int j = 0; j < 10; ++j) {
      f >> ww;
      EBG6samplesCorrelation_.push_back(ww);
    }
    for (int j = 0; j < 10; ++j) {
      f >> ww;
      EBG1samplesCorrelation_.push_back(ww);
    }
    for (int j = 0; j < 10; ++j) {
      f >> ww;
      EEG12samplesCorrelation_.push_back(ww);
    }
    for (int j = 0; j < 10; ++j) {
      f >> ww;
      EEG6samplesCorrelation_.push_back(ww);
    }
    for (int j = 0; j < 10; ++j) {
      f >> ww;
      EEG1samplesCorrelation_.push_back(ww);
    }
    f.close();
  }
  auto ipar = std::make_unique<EcalSamplesCorrelation>();
  copy(EBG12samplesCorrelation_.begin(), EBG12samplesCorrelation_.end(), back_inserter(ipar->EBG12SamplesCorrelation));
  copy(EBG6samplesCorrelation_.begin(), EBG6samplesCorrelation_.end(), back_inserter(ipar->EBG6SamplesCorrelation));
  copy(EBG1samplesCorrelation_.begin(), EBG1samplesCorrelation_.end(), back_inserter(ipar->EBG1SamplesCorrelation));
  copy(EEG12samplesCorrelation_.begin(), EEG12samplesCorrelation_.end(), back_inserter(ipar->EEG12SamplesCorrelation));
  copy(EEG6samplesCorrelation_.begin(), EEG6samplesCorrelation_.end(), back_inserter(ipar->EEG6SamplesCorrelation));
  copy(EEG1samplesCorrelation_.begin(), EEG1samplesCorrelation_.end(), back_inserter(ipar->EEG1SamplesCorrelation));
  return ipar;
}

std::unique_ptr<EcalSimPulseShape> EcalTrivialConditionRetriever::getEcalSimPulseShapeFromConfiguration(
    const EcalSimPulseShapeRcd&) {
  auto result = std::make_unique<EcalSimPulseShape>();

  // save time interval to be used for the pulse shape
  result->time_interval = sim_pulse_shape_TI_;

  // containers to store the shape info
  std::vector<double> EBshape;
  std::vector<double> EEshape;
  std::vector<double> APDshape;

  // --- get the 3 shapes from the user provided txt files
  if (!EBSimPulseShapeFile_.empty()) {
    std::ifstream shapeEBFile;
    shapeEBFile.open(EBSimPulseShapeFile_.c_str());
    double ww;
    while (shapeEBFile >> ww)
      EBshape.push_back(ww);
    shapeEBFile.close();
  }
  if (!EESimPulseShapeFile_.empty()) {
    std::ifstream shapeEEFile;
    shapeEEFile.open(EESimPulseShapeFile_.c_str());
    double ww;
    while (shapeEEFile >> ww)
      EEshape.push_back(ww);
    shapeEEFile.close();
  }
  if (!APDSimPulseShapeFile_.empty()) {
    std::ifstream shapeAPDFile;
    shapeAPDFile.open(APDSimPulseShapeFile_.c_str());
    double ww;
    while (shapeAPDFile >> ww)
      APDshape.push_back(ww);
    shapeAPDFile.close();
  }

  // --- save threshold
  result->barrel_thresh = sim_pulse_shape_EB_thresh_;
  result->endcap_thresh = sim_pulse_shape_EE_thresh_;
  result->apd_thresh = sim_pulse_shape_APD_thresh_;

  // --- copy
  copy(EBshape.begin(), EBshape.end(), back_inserter(result->barrel_shape));
  copy(EEshape.begin(), EEshape.end(), back_inserter(result->endcap_shape));
  copy(APDshape.begin(), APDshape.end(), back_inserter(result->apd_shape));

  return result;
}

std::unique_ptr<EcalSimComponentShape> EcalTrivialConditionRetriever::getEcalSimComponentShapeFromConfiguration(
    const EcalSimComponentShapeRcd&) {
  auto result = std::make_unique<EcalSimComponentShape>();

  // save time interval to be used for the pulse shape
  result->time_interval = sim_pulse_shape_TI_;

  // containers to store the shape info
  std::vector<std::vector<float> > EBshapes;

  // --- get the shapes from the user provided txt files
  if (!EBSimComponentShapeFiles_.empty()) {
    int iShape(0);
    for (auto filename : EBSimComponentShapeFiles_) {
      std::ifstream shapeEBFile;
      EBshapes.emplace_back();
      shapeEBFile.open(EBSimComponentShapeFiles_[iShape].c_str());
      float ww;
      while (shapeEBFile >> ww)
        EBshapes[iShape].push_back(ww);
      shapeEBFile.close();
      ++iShape;
    }
  }

  // --- save threshold
  result->barrel_thresh = sim_pulse_shape_EB_thresh_;

  // --- copy
  copy(EBshapes.begin(), EBshapes.end(), back_inserter(result->barrel_shapes));

  return result;
}
