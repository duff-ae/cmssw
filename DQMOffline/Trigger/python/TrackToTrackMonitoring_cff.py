import FWCore.ParameterSet.Config as cms

from DQMServices.Core.DQM_cfg import *
DQMStore.collateHistograms =cms.untracked.bool(True)
from DQM.TrackingMonitorSource.TrackToTrackComparisonHists_cfi import TrackToTrackComparisonHists

trackSelector = cms.EDFilter('TrackSelector',
    src = cms.InputTag('generalTracks'),
    cut = cms.string("")
)
highPurityTracks = trackSelector.clone(
    cut = "quality('highPurity')"
)

hltMerged2highPurity = TrackToTrackComparisonHists.clone(
    monitoredTrack           = "hltMergedTracks",
    referenceTrack           = "highPurityTracks",
    monitoredBeamSpot        = "hltOnlineBeamSpot",
    referenceBeamSpot        = "offlineBeamSpot",
    topDirName               = "HLT/Tracking/ValidationWRTOffline/hltMergedWrtHighPurity",
    referencePrimaryVertices = "offlinePrimaryVertices",
    monitoredPrimaryVertices = "hltVerticesPFSelector"
)

from Configuration.Eras.Modifier_pp_on_PbPb_run3_cff import pp_on_PbPb_run3
pp_on_PbPb_run3.toModify(hltMerged2highPurity,
                         topDirName               = "HLT/Tracking/ValidationWRTOffline/hltMergedPPonAAWrtHighPurity",
                         monitoredTrack           = "hltMergedTracksPPOnAA",
                         monitoredPrimaryVertices = "hltVerticesPFFilterPPOnAA")                         

from Configuration.Eras.Modifier_phase2_tracker_cff import phase2_tracker
phase2_tracker.toModify(hltMerged2highPurity,
                        monitoredTrack           = cms.InputTag("hltGeneralTracks"),
                        monitoredPrimaryVertices = cms.InputTag("hltOfflinePrimaryVertices"))

hltMerged2highPurityPV = TrackToTrackComparisonHists.clone(
    dzWRTPvCut               = 0.1,
    monitoredTrack           = "hltMergedTracks",
    referenceTrack           = "highPurityTracks",
    monitoredBeamSpot        = "hltOnlineBeamSpot",
    referenceBeamSpot        = "offlineBeamSpot",
    topDirName               = "HLT/Tracking/ValidationWRTOffline/hltMergedWrtHighPurityPV",
    referencePrimaryVertices = "offlinePrimaryVertices",
    monitoredPrimaryVertices = "hltVerticesPFSelector"
)

pp_on_PbPb_run3.toModify(hltMerged2highPurityPV,
                         topDirName               = "HLT/Tracking/ValidationWRTOffline/hltMergedPPonAAWrtHighPurityPV",
                         monitoredTrack           = "hltMergedTracksPPOnAA",
                         monitoredPrimaryVertices = "hltVerticesPFFilterPPOnAA")                         

from Configuration.Eras.Modifier_phase2_tracker_cff import phase2_tracker
phase2_tracker.toModify(hltMerged2highPurityPV,
                        monitoredTrack           = cms.InputTag("hltGeneralTracks"),
                        monitoredPrimaryVertices = cms.InputTag("hltOfflinePrimaryVertices"))

#
# E/gamma monitoring
#

hltEgammaGsfTracksVsOffline = TrackToTrackComparisonHists.clone(
    monitoredTrack           = "hltEgammaGsfTracks",
    referenceTrack           = "electronGsfTracks",
    monitoredBeamSpot        = "hltOnlineBeamSpot",
    referenceBeamSpot        = "offlineBeamSpot",
    topDirName               = "HLT/EGM/Tracking/ValidationWRTOffline/hltEgammaGsfTracks",
    referencePrimaryVertices = "offlinePrimaryVertices",
    monitoredPrimaryVertices = "hltVerticesPFSelector"
)

hltEgammaGsfTracksVsOfflinePV = TrackToTrackComparisonHists.clone(
    dzWRTPvCut               = 0.1,
    monitoredTrack           = "hltEgammaGsfTracks",
    referenceTrack           = "electronGsfTracks",
    monitoredBeamSpot        = "hltOnlineBeamSpot",
    referenceBeamSpot        = "offlineBeamSpot",
    topDirName               = "HLT/EGM/Tracking/ValidationWRTOffline/hltEgammaGsfTracksPV",
    referencePrimaryVertices = "offlinePrimaryVertices",
    monitoredPrimaryVertices = "hltVerticesPFSelector"
)

from Configuration.Eras.Modifier_phase2_common_cff import phase2_common
phase2_common.toModify(hltEgammaGsfTracksVsOffline,
                       monitoredTrack           = cms.InputTag("hltEgammaGsfTracksL1Seeded"),
                       monitoredPrimaryVertices = cms.InputTag("hltOfflinePrimaryVertices"))

phase2_common.toModify(hltEgammaGsfTracksVsOfflinePV,
                       monitoredTrack           = cms.InputTag("hltEgammaGsfTracksL1Seeded"),
                       monitoredPrimaryVertices = cms.InputTag("hltOfflinePrimaryVertices"))

hltToOfflineTrackValidatorSequence = cms.Sequence(
    cms.ignore(highPurityTracks)
    + hltMerged2highPurity
    + hltMerged2highPurityPV
    + hltEgammaGsfTracksVsOffline
    + hltEgammaGsfTracksVsOfflinePV
)
