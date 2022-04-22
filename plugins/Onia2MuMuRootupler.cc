// system include files
#include <memory>

// user include files
#include "FWCore/Framework/interface/Frameworkfwd.h"
#include "FWCore/Framework/interface/one/EDAnalyzer.h"

#include "FWCore/Framework/interface/Event.h"
#include "FWCore/Framework/interface/MakerMacros.h"

#include "FWCore/ParameterSet/interface/ParameterSet.h"
#include "FWCore/Utilities/interface/InputTag.h"

#include "FWCore/Common/interface/TriggerNames.h"
#include "FWCore/ServiceRegistry/interface/Service.h"

#include "CommonTools/UtilAlgos/interface/TFileService.h"

#include "DataFormats/PatCandidates/interface/CompositeCandidate.h"
#include "DataFormats/HepMCCandidate/interface/GenParticle.h"
#include "DataFormats/HepMCCandidate/interface/GenParticleFwd.h"
#include "DataFormats/PatCandidates/interface/UserData.h"
#include "DataFormats/VertexReco/interface/Vertex.h"
#include "DataFormats/VertexReco/interface/VertexFwd.h"
#include "DataFormats/Common/interface/TriggerResults.h"

#include "DataFormats/PatCandidates/interface/TriggerObjectStandAlone.h"
#include "DataFormats/PatCandidates/interface/PackedTriggerPrescales.h"

#include "TLorentzVector.h"
#include "TTree.h"
#include <vector>
#include <sstream>

//
// class declaration
//

// If the analyzer does not use TFileService, please remove
// the template argument to the base class so the class inherits
// from  edm::one::EDAnalyzer<>
// This will improve performance in multithreaded jobs.

class Onia2MuMuRootupler : public edm::one::EDAnalyzer<edm::one::SharedResources> {
public:
  explicit Onia2MuMuRootupler(const edm::ParameterSet&);
  ~Onia2MuMuRootupler();

  static void fillDescriptions(edm::ConfigurationDescriptions& descriptions);

private:
  void beginJob() override;
  void analyze(const edm::Event&, const edm::EventSetup&) override;
  void endJob() override;

  std::string file_name;
  const edm::EDGetTokenT<pat::CompositeCandidateCollection> TheCandidateLabel;
  const edm::EDGetTokenT<reco::VertexCollection>            ThePrimaryVertexLabel;
  const edm::EDGetTokenT<edm::TriggerResults>               TheTriggerResultLabel;
  const edm::EDGetTokenT<reco::GenParticleCollection>       TheGenParticleLabel;
  const edm::EDGetTokenT<std::vector<pat::TriggerObjectStandAlone> >      TheTriggerObjectsLabel;
  const int  dimuon_pdgid_, dimuon_y1S_pdgid_, dimuon_y2S_pdgid_, dimuon_y3S_pdgid_;
  const bool isMC_,isMC_yNs_,ups_,jpsi_,psiPrime_,OnlyBest_;

  UInt_t run, event, nCandPerEvent, numPrimaryVertices, trigger, triggerL1;  

  TLorentzVector dimuon_p4;
  TLorentzVector muonp_p4;
  TLorentzVector muonn_p4;

  Double_t dimuon_vMass;

  Double_t dimuon_lxyPV, dimuon_lxyErrPV, dimuon_ctauBS, dimuon_ctauErrBS, dimuon_lxyBS, dimuon_lxyErrBS;
  Double_t mu1_pt, mu1_ptErr, mu1_d0, mu1_d0Err, mu1_dz, mu1_dzErr, mu1_dxy, mu1_dxyErr, mu1_charge, mu2_pt, mu2_ptErr, mu2_d0, mu2_d0Err, mu2_dz, mu2_dzErr, mu2_dxy, mu2_dxyErr, mu2_charge;
  Int_t mu1_nvsh, mu1_nvph, mu2_nvsh, mu2_nvph;
  Int_t dimuon_diMuIndx;

  Double_t dimuon_vProb, dimuon_vChi2, dimuon_DCA, dimuon_ctauPV, dimuon_ctauErrPV, dimuon_cosAlpha, dimuon_nSigma;
  Double_t track2_dRdimuon, track1_dRdimuon, ditrack_dRdimuon;

  Int_t iPVwithmuons;
  Double_t dimuon_vertexWeight;

  TLorentzVector gen_dimuon_p4;
  TLorentzVector gen_y1s_p4, gen_y2s_p4, gen_y3s_p4;
  Int_t          gen_dimuon_pdgId, gen_dimuon_charge;
  TLorentzVector gen_muonp_p4;
  TLorentzVector gen_muonn_p4;

  TTree* TheTree;

  // ----------member data ---------------------------
  //edm::EDGetTokenT<TrackCollection> tracksToken_;  //used to select what tracks to read from configuration file
#ifdef THIS_IS_AN_EVENTSETUP_EXAMPLE
  edm::ESGetToken<SetupData, SetupRecord> setupToken_;
#endif
};

//
// constants, enums and typedefs
//

//
// static data member definitions
//

//
// constructors and destructor
//
Onia2MuMuRootupler::Onia2MuMuRootupler(const edm::ParameterSet& iConfig)
    :
TheCandidateLabel(consumes<pat::CompositeCandidateCollection>(iConfig.getParameter < edm::InputTag > ("TheCandidates"))),
ThePrimaryVertexLabel(consumes<reco::VertexCollection>(iConfig.getParameter < edm::InputTag > ("PrimaryVertices"))),
TheTriggerResultLabel(consumes<edm::TriggerResults>(iConfig.getParameter < edm::InputTag > ("TriggerResults"))),
TheGenParticleLabel(consumes<reco::GenParticleCollection>(iConfig.getParameter < edm::InputTag > ("GenParticles"))),
TheTriggerObjectsLabel(consumes<std::vector<pat::TriggerObjectStandAlone> >(iConfig.getParameter < edm::InputTag > ("triggerObjects"))),
dimuon_pdgid_(iConfig.getParameter<uint32_t>("dimuon_pdgid")),
dimuon_y1S_pdgid_(iConfig.getParameter<uint32_t>("dimuon_y1S_pdgid")),
dimuon_y2S_pdgid_(iConfig.getParameter<uint32_t>("dimuon_y2S_pdgid")),
dimuon_y3S_pdgid_(iConfig.getParameter<uint32_t>("dimuon_y3S_pdgid")),
isMC_(iConfig.getParameter<bool>("isMC")),
isMC_yNs_(iConfig.getParameter<bool>("isMC_yNs")),
ups_(iConfig.getParameter<bool>("ups")),
jpsi_(iConfig.getParameter<bool>("jpsi")),
psiPrime_(iConfig.getParameter<bool>("psiPrime")),
OnlyBest_(iConfig.getParameter<bool>("OnlyBest"))
{
#ifdef THIS_IS_AN_EVENTSETUP_EXAMPLE
  setupDataToken_ = esConsumes<SetupData, SetupRecord>();
#endif
  //now do what ever initialization is needed

  edm::Service<TFileService> fs;
  TheTree = fs->make<TTree>("DiMuonTree","DiMuonTree");

  TheTree->Branch("run",                &run,                "run/I");
  TheTree->Branch("event",              &event,              "event/I");
  TheTree->Branch("nCandPerEvent", &nCandPerEvent, "nCandPerEvent/I");
  TheTree->Branch("numPrimaryVertices", &numPrimaryVertices, "numPrimaryVertices/I");
  TheTree->Branch("trigger",            &trigger,            "trigger/I");
  TheTree->Branch("triggerL1",            &triggerL1,            "triggerL1/I");
  TheTree->Branch("dimuon_p4",   "TLorentzVector", &dimuon_p4);
  TheTree->Branch("muonp_p4",    "TLorentzVector", &muonp_p4);
  TheTree->Branch("muonn_p4",    "TLorentzVector", &muonn_p4);

  TheTree->Branch("iPVwithmuons",        &iPVwithmuons,        "iPVwithmuons/I");

  TheTree->Branch("dimuon_diMuIndx",       &dimuon_diMuIndx,        "dimuon_diMuIndx/I");
  TheTree->Branch("dimuon_vertexWeight",        &dimuon_vertexWeight,        "dimuon_vertexWeight/D");
  TheTree->Branch("dimuon_vProb",        &dimuon_vProb,        "dimuon_vProb/D");
  TheTree->Branch("dimuon_vMass",        &dimuon_vMass,        "dimuon_vMass/D");
  TheTree->Branch("dimuon_vNChi2",       &dimuon_vChi2,        "dimuon_vNChi2/D");
  TheTree->Branch("dimuon_DCA",          &dimuon_DCA,          "dimuon_DCA/D");
  TheTree->Branch("dimuon_ctauPV",       &dimuon_ctauPV,       "dimuon_ctauPV/D");
  TheTree->Branch("dimuon_ctauErrPV",    &dimuon_ctauErrPV,    "dimuon_ctauErrPV/D");
  TheTree->Branch("dimuon_lxyPV",        &dimuon_lxyPV,          "dimuon_lxyPV/D");
  TheTree->Branch("dimuon_lxyErrPV",     &dimuon_lxyErrPV,       "dimuon_lxyErrPV/D");
  TheTree->Branch("dimuon_cosAlpha",     &dimuon_cosAlpha,     "dimuon_cosAlpha/D");
  TheTree->Branch("dimuon_ctauBS",       &dimuon_ctauBS,       "dimuon_ctauBS/D");
  TheTree->Branch("dimuon_ctauErrBS",    &dimuon_ctauErrBS,    "dimuon_ctauErrBS/D");
  TheTree->Branch("dimuon_lxyBS",        &dimuon_lxyBS,          "dimuon_lxyBS/D");
  TheTree->Branch("dimuon_lxyErrBS",     &dimuon_lxyErrBS,       "dimuon_lxyErrBS/D");

  TheTree->Branch("mu1_pt",    &mu1_pt,    "mu1_pt/D");
  TheTree->Branch("mu1_ptErr",    &mu1_ptErr,    "mu1_ptErr/D");
  TheTree->Branch("mu1_d0",    &mu1_d0,    "mu1_d0/D");
  TheTree->Branch("mu1_d0Err", &mu1_d0Err, "mu1_d0Err/D");
  TheTree->Branch("mu1_dz",    &mu1_dz,    "mu1_dz/D");
  TheTree->Branch("mu1_dzErr",    &mu1_dzErr,    "mu1_dzErr/D");
  TheTree->Branch("mu1_dxy",   &mu1_dxy,   "mu1_dxy/D");
  TheTree->Branch("mu1_dxyErr",   &mu1_dxyErr,   "mu1_dxyErr/D");
  TheTree->Branch("mu1_nvsh",  &mu1_nvsh,  "mu1_nvsh/I");
  TheTree->Branch("mu1_nvph",  &mu1_nvph,  "mu1_nvph/I");
  TheTree->Branch("mu1_charge",  &mu1_charge,  "mu1_charge/I");

  TheTree->Branch("mu2_pt",    &mu2_pt,    "mu2_pt/D");
  TheTree->Branch("mu2_ptErr",    &mu2_ptErr,    "mu2_ptErr/D");
  TheTree->Branch("mu2_d0",    &mu2_d0,    "mu2_d0/D");
  TheTree->Branch("mu2_d0Err", &mu2_d0Err, "mu2_d0Err/D");
  TheTree->Branch("mu2_dz",    &mu2_dz,    "mu2_dz/D");
  TheTree->Branch("mu2_dzErr",    &mu2_dzErr,    "mu2_dzErr/D");
  TheTree->Branch("mu2_dxy",   &mu2_dxy,   "mu2_dxy/D");
  TheTree->Branch("mu2_dxyErr",   &mu2_dxyErr,   "mu2_dxyErr/D");
  TheTree->Branch("mu2_nvsh",  &mu2_nvsh,  "mu2_nvsh/I");
  TheTree->Branch("mu2_nvph",  &mu2_nvph,  "mu2_nvph/I");
  TheTree->Branch("mu2_charge",  &mu2_charge,  "mu2_charge/I");

  if(isMC_ || isMC_yNs_) {
    TheTree->Branch("gen_dimuon_pdgId",      &gen_dimuon_pdgId,      "gen_dimuon_pdgId/I");
    TheTree->Branch("gen_dimuon_p4",   "TLorentzVector", &gen_dimuon_p4);
    TheTree->Branch("gen_y1s_p4",   "TLorentzVector", &gen_y1s_p4);
    TheTree->Branch("gen_y2s_p4",   "TLorentzVector", &gen_y2s_p4);
    TheTree->Branch("gen_y3s_p4",   "TLorentzVector", &gen_y3s_p4);
    TheTree->Branch("gen_muonp_p4",    "TLorentzVector", &gen_muonp_p4);
    TheTree->Branch("gen_muonn_p4",    "TLorentzVector", &gen_muonn_p4);
    TheTree->Branch("gen_dimuon_charge",     &gen_dimuon_charge,       "gen_dimuon_charge/I");
  }

}

Onia2MuMuRootupler::~Onia2MuMuRootupler() {
  // do anything here that needs to be done at desctruction time
  // (e.g. close files, deallocate resources etc.)
  //
  // please remove this method altogether if it would be left empty
}

//
// member functions
//

// ------------ method called for each event  ------------
void Onia2MuMuRootupler::analyze(const edm::Event& iEvent, const edm::EventSetup& iSetup) {
  using namespace edm;

  edm::Handle < pat::CompositeCandidateCollection >TheCandidates;
  iEvent.getByToken(TheCandidateLabel, TheCandidates);

  edm::Handle < reco::VertexCollection  >ThePrimaryVertices;
  iEvent.getByToken(ThePrimaryVertexLabel, ThePrimaryVertices);

  edm::Handle < edm::TriggerResults > triggerResults_handle;
  iEvent.getByToken(TheTriggerResultLabel, triggerResults_handle);

  edm::Handle <std::vector<pat::TriggerObjectStandAlone> > triggerObjects;
  iEvent.getByToken(TheTriggerObjectsLabel, triggerObjects);

  numPrimaryVertices = ThePrimaryVertices->size();
  run = iEvent.id().run();
  event = iEvent.id().event();

  if ( isMC_ || isMC_yNs_) {
    edm::Handle<reco::GenParticleCollection> GenParticles;
    iEvent.getByToken(TheGenParticleLabel, GenParticles);
    int foundit = 0;
    //int gen_track1_pdgid_ = 0;
    gen_dimuon_pdgId = 0;
    if (GenParticles.isValid() ) {
      for ( reco::GenParticleCollection::const_iterator itParticle = GenParticles->begin(); itParticle != GenParticles->end(); ++itParticle ) {
         int pdgId = itParticle->pdgId();
         if ((isMC_ && abs(pdgId) == dimuon_pdgid_) 
            || (isMC_yNs_ && (abs(pdgId) == dimuon_y1S_pdgid_ || abs(pdgId) == dimuon_y2S_pdgid_ || abs(pdgId) == dimuon_y3S_pdgid_))) {
            gen_dimuon_charge = itParticle->charge();
            gen_dimuon_p4.SetPtEtaPhiM(itParticle->pt(),itParticle->eta(),itParticle->phi(),itParticle->mass());
            if (isMC_yNs_ && abs(pdgId) == dimuon_y1S_pdgid_) gen_y1s_p4.SetPtEtaPhiM(itParticle->pt(),itParticle->eta(),itParticle->phi(),itParticle->mass());
            if (isMC_yNs_ && abs(pdgId) == dimuon_y2S_pdgid_) gen_y2s_p4.SetPtEtaPhiM(itParticle->pt(),itParticle->eta(),itParticle->phi(),itParticle->mass());
            if (isMC_yNs_ && abs(pdgId) == dimuon_y3S_pdgid_) gen_y3s_p4.SetPtEtaPhiM(itParticle->pt(),itParticle->eta(),itParticle->phi(),itParticle->mass());
            gen_dimuon_pdgId = pdgId;
            foundit++;
            for (uint i = 0; i < itParticle->numberOfDaughters(); ++i) {
               const reco::Candidate* p = itParticle->daughter(i);
               if ( p->pdgId() == -13 && p->status() == 1 ) {
                  gen_muonp_p4.SetPtEtaPhiM(p->pt(),p->eta(),p->phi(),p->mass());
                  foundit++;
               }
               if ( p->pdgId() == 13 && p->status() == 1 ) {
                  gen_muonn_p4.SetPtEtaPhiM(p->pt(),p->eta(),p->phi(),p->mass());
                  foundit++;
               }
            }
            if ( foundit == 3 ) break; else { foundit = 0; gen_dimuon_pdgId = 0;}
         }
      }
    }
    if (!gen_dimuon_pdgId) std::cout << "OniaRecoTrackRootupler: didn't find the given decay " << run << "," << event << std::endl;
  } // end if isMC


  trigger = 0;
  if (triggerResults_handle.isValid()) {
    const edm::TriggerNames & TheTriggerNames = iEvent.triggerNames(*triggerResults_handle);
    unsigned int NTRIGGERS = 17;
    std::string TriggersToTest[NTRIGGERS] = {
       "HLT_Dimuon0_Upsilon_L1_4p5er2p0M","HLT_Dimuon12_Upsilon_y1p4",
       "HLT_Dimuon8_Upsilon_y1p4","HLT_Dimuon9_Upsilon_y1p4",
       "HLT_Dimuon10_Upsilon_y1p4","HLT_Dimuon11_Upsilon_y1p4",
       "HLT_Dimuon13_Upsilon_y1p4","HLT_Dimuon14_Upsilon_y1p4",
       "HLT_Dimuon12_Upsilon","HLT_Dimuon16_PsiPrime",
       "HLT_Dimuon18_PsiPrime","HLT_Dimuon20_PsiPrime",
       "HLT_Dimuon14_PsiPrime","HLT_Dimuon16_PsiPrime_noCorrL1",
       "HLT_Dimuon14_PsiPrime_noCorrL1","HLT_Dimuon18_PsiPrime_noCorrL1",
       "HLT_Dimuon20_PsiPrime_noCorrL1"
    };

    for (unsigned int i = 0; i < NTRIGGERS; i++) {
       for (int version = 1; version < 19; version++) {
          std::stringstream ss;
          ss << TriggersToTest[i] << "_v" << version;
          unsigned int bit = TheTriggerNames.triggerIndex(edm::InputTag(ss.str()).label());
          if (bit < triggerResults_handle->size() && triggerResults_handle->accept(bit) && !triggerResults_handle->error(bit)) {
            trigger += (1<<i);
            break;
          }
       }
    }
  } else std::cout << "*** NO triggerResults found " << iEvent.id().run() << "," << iEvent.id().event() << std::endl;

  triggerL1 = 0;
  if (triggerObjects.isValid()) {
    for (pat::TriggerObjectStandAlone obj : *triggerObjects) {
      //obj.unpackPathNames(TheTriggerNames);
      obj.unpackNamesAndLabels(iEvent,*triggerResults_handle);
      for (unsigned h = 0; h < obj.filterLabels().size(); ++h) {
        if (ups_ && obj.filterLabels()[h] == "hltL1s12DoubleMu4p5er2p0SQOSMass7to18") {
          triggerL1 = 1;
          //std::cout << std::endl;
          //std::cout << "\t   Filters:    ";
          //std::cout << " " << obj.filterLabels()[h];
          //std::cout << "\t   Conditions:    ";
          //std::cout << " " << obj.conditionNames()[h];
          //std::cout << std::endl;
        }
        if (psiPrime_ && obj.filterLabels()[h] == "hltL1sDoubleMu4SQOSdRMax1p2DoubleMu0er1p5SQOSdRMax1p4") {triggerL1 = 1;}
        if (jpsi_ && obj.filterLabels()[h] == "hltL1sDoubleMu4SQOSdRMax1p2DoubleMu0er1p5SQOSdRMax1p4") {triggerL1 = 1;}
      }
    }
  } else std::cout << "*** No triggerObjects found " << iEvent.id().run() << "," << iEvent.id().event() << std::endl;

  if (TheCandidates.isValid() && !TheCandidates->empty()) {
    pat::CompositeCandidate TheDimuon_;
    nCandPerEvent = TheCandidates->size();
    for (unsigned int i=0; i< TheCandidates->size(); i++){
      TheDimuon_       = TheCandidates->at(i);

      dimuon_p4.SetPtEtaPhiM(TheDimuon_.pt(),TheDimuon_.eta(),TheDimuon_.phi(),TheDimuon_.mass());
      //std::cout<<" dimuon pT = "<<TheDimuon_.mass()<<" vMass = "<<TheDimuon_.userFloat("vMass")<<std::endl;

      dimuon_diMuIndx     = TheDimuon_.userInt("diMuIndex");
      dimuon_vertexWeight = TheDimuon_.userFloat("vertexWeight");
      dimuon_vProb        = TheDimuon_.userFloat("vProb");
      dimuon_vMass        = TheDimuon_.userFloat("vMass");
      dimuon_vChi2        = TheDimuon_.userFloat("vNChi2");
      dimuon_DCA          = TheDimuon_.userFloat("DCA");
      dimuon_ctauPV       = TheDimuon_.userFloat("ppdlPV");
      dimuon_ctauErrPV    = TheDimuon_.userFloat("ppdlErrPV");
      dimuon_cosAlpha     = TheDimuon_.userFloat("cosAlpha");

      dimuon_lxyPV        = TheDimuon_.userFloat("lxyPV");
      dimuon_lxyErrPV     = TheDimuon_.userFloat("lxyErrPV");
      dimuon_ctauBS       = TheDimuon_.userFloat("ppdlBS");
      dimuon_ctauErrBS    = TheDimuon_.userFloat("ppdlErrBS");
      dimuon_lxyBS        = TheDimuon_.userFloat("lxyBS");
      dimuon_lxyErrBS     = TheDimuon_.userFloat("lxyErrBS");

      iPVwithmuons = TheDimuon_.userInt("iPV");

      const reco::Candidate::LorentzVector vP = TheDimuon_.daughter("muon1")->p4();
      const reco::Candidate::LorentzVector vM = TheDimuon_.daughter("muon2")->p4();
      if (TheDimuon_.daughter("muon1")->charge() > 0) {
         muonp_p4.SetPtEtaPhiM(vP.pt(), vP.eta(), vP.phi(), vP.mass());
         muonn_p4.SetPtEtaPhiM(vM.pt(), vM.eta(), vM.phi(), vM.mass());
      } else {
         muonn_p4.SetPtEtaPhiM(vP.pt(), vP.eta(), vP.phi(), vP.mass());
         muonp_p4.SetPtEtaPhiM(vM.pt(), vM.eta(), vM.phi(), vM.mass());
      }

      mu1_pt      = TheDimuon_.userFloat("mu1_pt");
      mu1_ptErr   = TheDimuon_.userFloat("mu1_ptErr");
      mu1_d0      = TheDimuon_.userFloat("mu1_d0");
      mu1_d0Err   = TheDimuon_.userFloat("mu1_d0Err");
      mu1_dz      = TheDimuon_.userFloat("mu1_dz");
      mu1_dzErr   = TheDimuon_.userFloat("mu1_dzErr");
      mu1_dxy     = TheDimuon_.userFloat("mu1_dxy");
      mu1_dxyErr  = TheDimuon_.userFloat("mu1_dxyErr");
      mu1_nvsh    = TheDimuon_.userInt("mu1_nvsh");
      mu1_nvph    = TheDimuon_.userInt("mu1_nvph");
      mu1_charge  = TheDimuon_.userInt("mu1_charge");

      mu2_pt      = TheDimuon_.userFloat("mu2_pt");
      mu2_ptErr   = TheDimuon_.userFloat("mu2_ptErr");
      mu2_d0      = TheDimuon_.userFloat("mu2_d0");
      mu2_d0Err   = TheDimuon_.userFloat("mu2_d0Err");
      mu2_dz      = TheDimuon_.userFloat("mu2_dz");
      mu2_dzErr   = TheDimuon_.userFloat("mu2_dzErr");
      mu2_dxy     = TheDimuon_.userFloat("mu2_dxy");
      mu2_dxyErr  = TheDimuon_.userFloat("mu2_dxyErr");
      mu2_nvsh    = TheDimuon_.userInt("mu2_nvsh");
      mu2_nvph    = TheDimuon_.userInt("mu2_nvph");
      mu2_charge  = TheDimuon_.userInt("mu2_charge");

      //std::cout<<" ===> test dz = "<<TheDimuon_.daughter("muon1")->dz()<<std::endl

      TheTree->Fill();
      if (OnlyBest_) break; 
    }
  } else std::cout<< "No candidate information " << run << "," << event <<std::endl;

#ifdef THIS_IS_AN_EVENTSETUP_EXAMPLE
  // if the SetupData is always needed
  auto setup = iSetup.getData(setupToken_);
  // if need the ESHandle to check if the SetupData was there or not
  auto pSetup = iSetup.getHandle(setupToken_);
#endif
}

// ------------ method called once each job just before starting event loop  ------------
void Onia2MuMuRootupler::beginJob() {
  // please remove this method if not needed
}

// ------------ method called once each job just after ending the event loop  ------------
void Onia2MuMuRootupler::endJob() {
  // please remove this method if not needed
}

// ------------ method fills 'descriptions' with the allowed parameters for the module  ------------
void Onia2MuMuRootupler::fillDescriptions(edm::ConfigurationDescriptions& descriptions) {
  //The following says we do not know what parameters are allowed so do no validation
  // Please change this to state exactly what you do use, even if it is no parameters
  edm::ParameterSetDescription desc;
  desc.setUnknown();
  descriptions.addDefault(desc);

  //Specify that only 'tracks' is allowed
  //To use, remove the default given above and uncomment below
  //ParameterSetDescription desc;
  //desc.addUntracked<edm::InputTag>("tracks","ctfWithMaterialTracks");
  //descriptions.addWithDefaultLabel(desc);
}

//define this as a plug-in
DEFINE_FWK_MODULE(Onia2MuMuRootupler);
