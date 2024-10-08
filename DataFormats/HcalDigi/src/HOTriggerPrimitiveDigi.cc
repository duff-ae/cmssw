#include "DataFormats/HcalDigi/interface/HOTriggerPrimitiveDigi.h"
#include "FWCore/MessageLogger/interface/MessageLogger.h"

HOTriggerPrimitiveDigi::HOTriggerPrimitiveDigi(
    int ieta, int iphi, int nsamples, int whichSampleTriggered, int databits) {
  if ((nsamples < 0) || (nsamples > HO_TP_SAMPLES_MAX)) {
    edm::LogWarning("HOTRiggerPrimitiveDigi") << "value " << nsamples << " of nsamples out of range.";
    if (nsamples < 0) {
      nsamples = 0;
    } else {
      nsamples = HO_TP_SAMPLES_MAX;
    }
  } else if ((whichSampleTriggered < 0) || (whichSampleTriggered > nsamples)) {
    edm::LogWarning("HOTriggerPrimitiveDigi")
        << "value " << whichSampleTriggered << " of specified Triggering Sample out of range.";
  } else if ((databits >> nsamples) != 0x0000) {
    edm::LogWarning("HOTRiggerPrimitiveDigi")
        << "nsamples " << nsamples << " and databits " << databits << "caused specified extra bits ("
        << (databits >> nsamples) << ") to be out of nsamples range.";
  }
  int samples = nsamples;

  theHO_TP = (abs(ieta) & 0xf) | ((ieta < 0) ? (0x10) : (0x00)) | ((iphi & 0x7F) << 5) | ((samples & 0xF) << 12) |
             (((whichSampleTriggered) & 0xF) << 16) | ((databits & 0x3FF) << 20);
}

//Request the value of a given HO TP bit in the HO TP Digi.
bool HOTriggerPrimitiveDigi::data(int whichbit) const {
  if ((whichbit < 0) || (whichbit > nsamples())) {
    edm::LogWarning("HOTriggerPrimitiveDigi") << "value " << whichbit << " of sample bit requested out of range.";
    return false;
  }
  return ((theHO_TP >> (20 + whichbit)) & 0x0001);
}

/* Stream the formatted contents of the HOTriggerPrimitiveDigi. */
std::ostream& operator<<(std::ostream& s, const HOTriggerPrimitiveDigi& HOtpd) {
  s << "(HO TP " << HOtpd.ieta() << ", " << HOtpd.iphi() << ",  ";
  //  s << HOtpd.whichSampleTriggered() << "_of_" << HOtpd.nsamples() << " [";
  for (int bit = 0; bit < HOtpd.nsamples(); bit++) {
    if (HOtpd.data(bit))
      s << "1";
    else
      s << "0";
    if (bit == HOtpd.whichSampleTriggered())
      s << "* ";
    else
      s << " ";
  }
  s << " )";
  return s;
}
