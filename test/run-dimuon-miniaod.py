import FWCore.ParameterSet.Config as cms
process = cms.Process('Quarkonia')

import FWCore.ParameterSet.VarParsing as VarParsing

options = VarParsing.VarParsing('analysis')
options.register('nThr',4, VarParsing.VarParsing.multiplicity.singleton,VarParsing.VarParsing.varType.int,"threads")
options.register('nEvt',10,VarParsing.VarParsing.multiplicity.singleton,VarParsing.VarParsing.varType.int,"events")
options.register('type','jpsi',VarParsing.VarParsing.multiplicity.singleton,VarParsing.VarParsing.varType.string,"quarkonia type")
options.register('MCTruth',False,VarParsing.VarParsing.multiplicity.singleton,VarParsing.VarParsing.varType.bool,"MC Truth")
options.parseArguments()

inputs = []
oniaType = options.type
dimuSelection = ""
pdgID = 0
pdgID_y1s = 0
pdgID_y2s = 0
pdgID_y3s = 0
bIsMC = False
bIsMC_yNs = False

if oniaType == 'jpsi':
    from Onia.dimuonAnalyzer.inputFiles import jpsiMC
    inputs = jpsiMC
    dimuSelection = "2.7 < mass && mass < 3.5 && charge==0 && abs(eta) < 2.4"
    pdgID = 443
    bIsMC = True
if oniaType == 'psiPrime':
    from Onia.dimuonAnalyzer.inputFiles import psiPrimeMC
    inputs = psiPrimeMC
    dimuSelection = "3.5 < mass && mass < 4.0 && charge==0 && abs(eta) < 2.4"
    pdgID = 100443
    bIsMC = True
if oniaType == 'ups':
    from Onia.dimuonAnalyzer.inputFiles import upsMC
    inputs = upsMC
    dimuSelection = "8.5 < mass && mass < 11.5 && charge==0 && abs(eta) < 2.4"
    pdgID_y1s = 553
    pdgID_y2s = 100553
    pdgID_y3s = 200553
    bIsMC_yNs = True

if options.MCTruth:
    output_filename = 'rootuple_'+oniaType+'_mcRun3_miniAOD_MCTruth.root'
else:
    output_filename = 'rootuple_'+oniaType+'_mcRun3_miniAOD.root'

thr = options.nThr
maxEvnt = options.nEvt

process.options.numberOfThreads=cms.untracked.uint32(thr)
process.options.numberOfStreams=cms.untracked.uint32(thr)

process.load('Configuration.StandardSequences.GeometryRecoDB_cff')
#process.load('Configuration.StandardSequences.MagneticField_38T_cff')
process.load('Configuration.StandardSequences.MagneticField_AutoFromDBCurrent_cff')
process.load('Configuration.StandardSequences.Reconstruction_cff')
process.load('Configuration.StandardSequences.FrontierConditions_GlobalTag_cff')

from Configuration.AlCa.GlobalTag import GlobalTag
process.GlobalTag = GlobalTag(process.GlobalTag, '123X_mcRun3_2021_realistic_v4', '')

process.load("FWCore.MessageLogger.MessageLogger_cfi")
process.MessageLogger.cerr.FwkReport.reportEvery = 100
process.maxEvents = cms.untracked.PSet(input = cms.untracked.int32(maxEvnt))
process.source = cms.Source("PoolSource", fileNames = cms.untracked.vstring(inputs))
process.source.duplicateCheckMode = cms.untracked.string('noDuplicateCheck')
process.TFileService = cms.Service("TFileService", fileName = cms.string(output_filename))
process.options   = cms.untracked.PSet( wantSummary = cms.untracked.bool(False))

process.oniaSelectedMuons = cms.EDFilter('PATMuonSelector',
   src = cms.InputTag('slimmedMuons'),
   cut = cms.string('muonID(\"TMOneStationTight\")'
                    ' && abs(innerTrack.dxy) < 0.3'
                    ' && abs(innerTrack.dz)  < 20.'
                    ' && innerTrack.hitPattern.trackerLayersWithMeasurement > 5'
                    ' && innerTrack.hitPattern.pixelLayersWithMeasurement > 0'
                    ' && innerTrack.quality(\"highPurity\")'
                    ' && (pt > 0.)'
   ),
   filter = cms.bool(True)
)

process.onia2MuMuPAT = cms.EDProducer('Onia2MuMuPAT',
  muons = cms.InputTag("oniaSelectedMuons"),
  beamSpotTag = cms.InputTag("offlineBeamSpot"),
  primaryVertexTag = cms.InputTag("offlineSlimmedPrimaryVertices"),
  higherPuritySelection = cms.string(""), ## At least one muon must pass this selection
  lowerPuritySelection  = cms.string(""), ## BOTH muons must pass this selection
  dimuonSelection  = cms.string(dimuSelection), ## The dimuon must pass this selection before vertexing
  addCommonVertex = cms.bool(True), ## Embed the full reco::Vertex out of the common vertex fit
  addMuonlessPrimaryVertex = cms.bool(False), ## Embed the primary vertex re-made from all the tracks except the two muons
  addMCTruth = cms.bool(options.MCTruth),      ## Add the common MC mother of the two muons, if any
  resolvePileUpAmbiguity = cms.bool(True)   ## Order PVs by their vicinity to the J/psi vertex, not by sumPt                            
)

process.onia2MuMuCounter = cms.EDFilter('CandViewCountFilter',
    src       = cms.InputTag("onia2MuMuPAT"),
    minNumber = cms.uint32(1),
)
'''
process.triggerSelection = cms.EDFilter("TriggerResultsFilter",
                                        triggerConditions = cms.vstring('HLT_Dimuon0_Jpsi_v*','HLT_Dimuon0_Jpsi_L1_4R_0er1p5R_v*','HLT_Dimuon0_Jpsi_L1_8_noCorr_v*',
                                                                         'HLT_Dimuon0_PsiPrime_L1_0er1p5_4_v*','HLT_Dimuon0_PsiPrime_L1_8_noCorr_v*',
                                                                         'HLT_Dimuon0_Upsilon_L1_4p5er2p0M_v*','HLT_Dimuon0_Upsilon_L1_noCorr_v*',
                                                                         'HLT_Dimuon0_PsiPrime_v*'),
                                        hltResults = cms.InputTag( "TriggerResults", "", "HLT" ),
                                        l1tResults = cms.InputTag( "" ),
                                        throw = cms.bool(False)
                                        )
'''
process.dimuonSequence = cms.Sequence(
                                   #process.triggerSelection *
                                   process.oniaSelectedMuons *
                                   process.onia2MuMuPAT*
                                   process.onia2MuMuCounter
                                   )

process.rootuple = cms.EDAnalyzer('Onia2MuMuRootupler',
                                TheCandidates = cms.InputTag("onia2MuMuPAT"),
                                PrimaryVertices = cms.InputTag("offlineSlimmedPrimaryVertices"),
                                TriggerResults = cms.InputTag("TriggerResults", "", "HLT"),
                                GenParticles = cms.InputTag("prunedGenParticles"),
                                dimuon_pdgid = cms.uint32(pdgID),
                                dimuon_y1S_pdgid = cms.uint32(pdgID_y1s),
                                dimuon_y2S_pdgid = cms.uint32(pdgID_y2s),
                                dimuon_y3S_pdgid = cms.uint32(pdgID_y3s),
                                isMC = cms.bool(bIsMC),
                                isMC_yNs = cms.bool(bIsMC_yNs),
                                OnlyBest = cms.bool(False)
)

process.p = cms.Path(process.dimuonSequence*process.rootuple)
