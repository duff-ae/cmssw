// Author: Marco Rovere - marco.rovere@cern.ch
// Date: 11/2018

#ifndef RecoHGCal_TICL_ClusterFilterBySize_H__
#define RecoHGCal_TICL_ClusterFilterBySize_H__

#include "DataFormats/CaloRecHit/interface/CaloCluster.h"
#include "ClusterFilterBase.h"

#include <memory>
#include <utility>

// Filter clusters that belong to a specific algorithm
namespace ticl {
  class ClusterFilterBySize final : public ClusterFilterBase {
  public:
    ClusterFilterBySize(const edm::ParameterSet& ps)
        : ClusterFilterBase(ps), max_cluster_size_(ps.getParameter<int>("max_cluster_size")) {}
    ~ClusterFilterBySize() override {}

    void filter(const std::vector<reco::CaloCluster>& layerClusters,
                std::vector<float>& layerClustersMask,
                hgcal::RecHitTools& rhtools) const override {
      for (size_t i = 0; i < layerClusters.size(); i++) {
        if (layerClusters[i].hitsAndFractions().size() > max_cluster_size_) {
          layerClustersMask[i] = 0.;
        }
      }
    }

  private:
    unsigned int max_cluster_size_;
  };
}  // namespace ticl

#endif
