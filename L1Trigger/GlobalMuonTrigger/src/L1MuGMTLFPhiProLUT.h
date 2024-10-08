//-------------------------------------------------
//
/** \class L1MuGMTLFPhiProLUT
 *
 *   LFPhiPro look-up table
 *          
 *   this class was automatically generated by 
 *     L1MuGMTLUT::MakeSubClass()  
*/
//
//   Author :
//   H. Sakulin            HEPHY Vienna
//
//   Migrated to CMSSW:
//   I. Mikulec
//
//--------------------------------------------------
#ifndef L1TriggerGlobalMuonTrigger_L1MuGMTLFPhiProLUT_h
#define L1TriggerGlobalMuonTrigger_L1MuGMTLFPhiProLUT_h

//---------------
// C++ Headers --
//---------------

//----------------------
// Base Class Headers --
//----------------------
#include "L1Trigger/GlobalMuonTrigger/src/L1MuGMTLUT.h"

//------------------------------------
// Collaborating Class Declarations --
//------------------------------------

//              ---------------------
//              -- Class Interface --
//              ---------------------

class L1MuGMTLFPhiProLUT : public L1MuGMTLUT {
public:
  enum { DT, BRPC, CSC, FRPC };

  /// constuctor using function-lookup
  L1MuGMTLFPhiProLUT() : L1MuGMTLUT("LFPhiPro", "DT BRPC CSC FRPC", "eta(4) pt(5) charge(1)", "dphi(9)", 10, false) {
    InitParameters();
  };

  /// destructor
  ~L1MuGMTLFPhiProLUT() override {}

  /// specific lookup function for dphi
  unsigned SpecificLookup_dphi(int idx, unsigned eta, unsigned pt, unsigned charge) const {
    std::vector<unsigned> addr(3);
    addr[0] = eta;
    addr[1] = pt;
    addr[2] = charge;
    return Lookup(idx, addr)[0];
  };

  /// specific lookup function for entire output field
  unsigned SpecificLookup(int idx, unsigned eta, unsigned pt, unsigned charge) const {
    std::vector<unsigned> addr(3);
    addr[0] = eta;
    addr[1] = pt;
    addr[2] = charge;
    return LookupPacked(idx, addr);
  };

  /// access to lookup function with packed input and output

  unsigned LookupFunctionPacked(int idx, unsigned address) const override {
    std::vector<unsigned> addr = u2vec(address, m_Inputs);
    return TheLookupFunction(idx, addr[0], addr[1], addr[2]);
  };

private:
  /// Initialize scales, configuration parameters, alignment constants, ...
  void InitParameters();

  /// The lookup function - here the functionality of the LUT is implemented
  unsigned TheLookupFunction(int idx, unsigned eta, unsigned pt, unsigned charge) const;

  /// Private data members (LUT parameters);
};
#endif
