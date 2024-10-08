/** \class EgammaHLTCaloTowerProducer
 *
 * Framework module that produces a collection
 * of calo towers in the region of interest for Egamma HLT reconnstruction,
 * \author M. Sani (UCSD)
 *
 */

#include "FWCore/Framework/interface/global/EDProducer.h"
#include "FWCore/Utilities/interface/InputTag.h"
#include "DataFormats/CaloTowers/interface/CaloTowerDefs.h"
#include "DataFormats/L1Trigger/interface/L1EmParticle.h"
#include "DataFormats/L1Trigger/interface/L1EmParticleFwd.h"
#include "DataFormats/RecoCandidate/interface/RecoCaloTowerCandidate.h"
#include "DataFormats/CaloTowers/interface/CaloTower.h"
#include "DataFormats/Common/interface/Handle.h"
#include "FWCore/Framework/interface/Event.h"
#include "FWCore/ParameterSet/interface/ParameterSet.h"

#include "DataFormats/L1Trigger/interface/L1EmParticle.h"
#include "DataFormats/L1Trigger/interface/L1EmParticleFwd.h"

#include "FWCore/ParameterSet/interface/ConfigurationDescriptions.h"
#include "FWCore/ParameterSet/interface/ParameterSetDescription.h"

#include "Math/GenVector/VectorUtil.h"

#include <cmath>
#include <string>

class EgammaHLTCaloTowerProducer : public edm::global::EDProducer<> {
public:
  EgammaHLTCaloTowerProducer(const edm::ParameterSet&);
  ~EgammaHLTCaloTowerProducer() override {}
  static void fillDescriptions(edm::ConfigurationDescriptions& descriptions);
  void produce(edm::StreamID, edm::Event&, edm::EventSetup const&) const final;

  const edm::EDGetTokenT<CaloTowerCollection> towers_;
  const double cone_;
  const edm::EDGetTokenT<edm::View<reco::Candidate>> l1isoseeds_;
  const edm::EDGetTokenT<edm::View<reco::Candidate>> l1nonisoseeds_;
  const double EtThreshold_;
  const double EThreshold_;
};

using namespace edm;
using namespace reco;
using namespace std;
using namespace l1extra;

EgammaHLTCaloTowerProducer::EgammaHLTCaloTowerProducer(const ParameterSet& p)
    : towers_(consumes<CaloTowerCollection>(p.getParameter<InputTag>("towerCollection"))),
      cone_(p.getParameter<double>("useTowersInCone")),
      l1isoseeds_(consumes<edm::View<reco::Candidate>>(p.getParameter<edm::InputTag>("L1IsoCand"))),
      l1nonisoseeds_(consumes<edm::View<reco::Candidate>>(p.getParameter<edm::InputTag>("L1NonIsoCand"))),
      EtThreshold_(p.getParameter<double>("EtMin")),
      EThreshold_(p.getParameter<double>("EMin")) {
  produces<CaloTowerCollection>();
}

void EgammaHLTCaloTowerProducer::fillDescriptions(edm::ConfigurationDescriptions& descriptions) {
  edm::ParameterSetDescription desc;

  desc.add<edm::InputTag>(("towerCollection"), edm::InputTag("hltRecoEcalCandidate"));
  desc.add<edm::InputTag>(("L1IsoCand"), edm::InputTag("hltTowerMakerForAll"));
  desc.add<edm::InputTag>(("L1NonIsoCand"), edm::InputTag("fixedGridRhoFastjetAllCalo"));
  desc.add<double>(("useTowersInCone"), 0.8);
  desc.add<double>(("EtMin"), 1.0);
  desc.add<double>(("EMin"), 1.0);
  descriptions.add(("hltCaloTowerForEgamma"), desc);
}

void EgammaHLTCaloTowerProducer::produce(edm::StreamID, edm::Event& evt, edm::EventSetup const&) const {
  edm::Handle<CaloTowerCollection> caloTowers;
  evt.getByToken(towers_, caloTowers);

  edm::Handle<edm::View<reco::Candidate>> emIsolColl;
  evt.getByToken(l1isoseeds_, emIsolColl);
  edm::Handle<edm::View<reco::Candidate>> emNonIsolColl;
  evt.getByToken(l1nonisoseeds_, emNonIsolColl);
  auto cands = std::make_unique<CaloTowerCollection>();
  cands->reserve(caloTowers->size());

  for (unsigned idx = 0; idx < caloTowers->size(); idx++) {
    const CaloTower* cal = &((*caloTowers)[idx]);
    if (cal->et() >= EtThreshold_ && cal->energy() >= EThreshold_) {
      bool fill = false;
      math::PtEtaPhiELorentzVector p(cal->et(), cal->eta(), cal->phi(), cal->energy());
      for (edm::View<reco::Candidate>::const_iterator emItr = emIsolColl->begin(); emItr != emIsolColl->end();
           ++emItr) {
        double delta = ROOT::Math::VectorUtil::DeltaR((*emItr).p4().Vect(), p);
        if (delta < cone_) {
          cands->push_back(*cal);
          fill = true;
          break;
        }
      }

      if (!fill) {
        for (edm::View<reco::Candidate>::const_iterator emItr = emNonIsolColl->begin(); emItr != emNonIsolColl->end();
             ++emItr) {
          double delta = ROOT::Math::VectorUtil::DeltaR((*emItr).p4().Vect(), p);
          if (delta < cone_) {
            cands->push_back(*cal);
            break;
          }
        }
      }
    }
  }

  evt.put(std::move(cands));
}

#include "FWCore/Framework/interface/MakerMacros.h"
DEFINE_FWK_MODULE(EgammaHLTCaloTowerProducer);
