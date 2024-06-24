import FWCore.ParameterSet.Config as cms

from PhysicsTools.NanoAOD.common_cff import *

from DPGAnalysis.MuonTools.nano_mu_global_cff import *
from DPGAnalysis.MuonTools.nano_mu_digi_cff import *

muDPGNanoProducerBkg = cms.Sequence(globalTables
                                   + muDigiTablesBkg)

def muDPGNanoBkgCustomize(process) :

     for output in ["NANOEDMAODoutput", "NANOAODoutput", "NANOEDMAODSIMoutput", "NANOAODSIMoutput"]:
          if hasattr(process, output):
               getattr(process,output).outputCommands.append("keep nanoaodFlatTable_*Table*_*_*")
               getattr(process,output).outputCommands.append("drop edmTriggerResults_*_*_*")           
     
     return process
