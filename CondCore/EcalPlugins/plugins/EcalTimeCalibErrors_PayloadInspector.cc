#include "CondCore/Utilities/interface/PayloadInspectorModule.h"
#include "CondCore/Utilities/interface/PayloadInspector.h"
#include "CondCore/CondDB/interface/Time.h"
#include "DataFormats/EcalDetId/interface/EBDetId.h"
#include "DataFormats/EcalDetId/interface/EEDetId.h"
#include "CondCore/EcalPlugins/plugins/EcalDrawUtils.h"
#include "CondCore/EcalPlugins/plugins/EcalFloatCondObjectContainerUtils.h"
// the data format of the condition to be inspected
#include "CondFormats/EcalObjects/interface/EcalTimeCalibErrors.h"

#include "TH2F.h"
#include "TCanvas.h"
#include "TStyle.h"
#include "TLine.h"
#include "TLatex.h"

#include <memory>
#include <sstream>
#include <array>

namespace {

  enum { kEBChannels = 61200, kEEChannels = 14648 };
  enum {
    MIN_IETA = 1,
    MIN_IPHI = 1,
    MAX_IETA = 85,
    MAX_IPHI = 360,
    EBhistEtaMax = 171
  };  // barrel lower and upper bounds on eta and phi
  enum {
    IX_MIN = 1,
    IY_MIN = 1,
    IX_MAX = 100,
    IY_MAX = 100,
    EEhistXMax = 220
  };  // endcaps lower and upper bounds on x and y

  /*******************************************************
   
     2d histogram of ECAL barrel Time Calib Errors of 1 IOV 

  *******************************************************/

  // inherit from one of the predefined plot class: Histogram2D
  class EcalTimeCalibErrorsEBMap : public cond::payloadInspector::Histogram2D<EcalTimeCalibErrors> {
  public:
    EcalTimeCalibErrorsEBMap()
        : cond::payloadInspector::Histogram2D<EcalTimeCalibErrors>("ECAL Barrel Time Calib Errors- map ",
                                                                   "iphi",
                                                                   MAX_IPHI,
                                                                   MIN_IPHI,
                                                                   MAX_IPHI + 1,
                                                                   "ieta",
                                                                   EBhistEtaMax,
                                                                   -MAX_IETA,
                                                                   MAX_IETA + 1) {
      Base::setSingleIov(true);
    }

    // Histogram2D::fill (virtual) needs be overridden - the implementation should use fillWithValue
    bool fill() override {
      auto tag = PlotBase::getTag<0>();
      for (auto const& iov : tag.iovs) {
        std::shared_ptr<EcalTimeCalibErrors> payload = Base::fetchPayload(std::get<1>(iov));
        if (payload.get()) {
          // looping over the EB channels, via the dense-index, mapped into EBDetId's
          if (payload->barrelItems().empty())
            return false;
          // set to 0 for ieta 0 (no crystal)
          for (int iphi = MIN_IPHI; iphi < MAX_IPHI + 1; iphi++)
            fillWithValue(iphi, 0, 0);

          for (int cellid = EBDetId::MIN_HASH; cellid < EBDetId::kSizeForDenseIndexing; ++cellid) {
            uint32_t rawid = EBDetId::unhashIndex(cellid);

            // check the existence of ECAL Time Calib Errors, for a given ECAL barrel channel
            EcalFloatCondObjectContainer::const_iterator value_ptr = payload->find(rawid);
            if (value_ptr == payload->end())
              continue;  // cell absent from payload

            float weight = (float)(*value_ptr);

            // fill the Histogram2D here
            fillWithValue((EBDetId(rawid)).iphi(), (EBDetId(rawid)).ieta(), weight);
          }  // loop over cellid
        }  // if payload.get()
      }  // loop over IOV's (1 in this case)

      return true;

    }  //fill method
  };

  /*******************************************************
   
     2d histogram of ECAL EndCaps Time Calib Errors of 1 IOV 

  *******************************************************/

  class EcalTimeCalibErrorsEEMap : public cond::payloadInspector::Histogram2D<EcalTimeCalibErrors> {
  private:
    int EEhistSplit = 20;

  public:
    EcalTimeCalibErrorsEEMap()
        : cond::payloadInspector::Histogram2D<EcalTimeCalibErrors>("ECAL Endcap Time Calib Errors - map ",
                                                                   "ix",
                                                                   EEhistXMax,
                                                                   IX_MIN,
                                                                   EEhistXMax + 1,
                                                                   "iy",
                                                                   IY_MAX,
                                                                   IY_MIN,
                                                                   IY_MAX + 1) {
      Base::setSingleIov(true);
    }

    bool fill() override {
      auto tag = PlotBase::getTag<0>();
      for (auto const& iov : tag.iovs) {
        std::shared_ptr<EcalTimeCalibErrors> payload = Base::fetchPayload(std::get<1>(iov));
        if (payload.get()) {
          if (payload->endcapItems().empty())
            return false;

          // set to 0 everywhwere
          for (int ix = IX_MIN; ix < EEhistXMax + 1; ix++)
            for (int iy = IY_MIN; iy < IY_MAX + 1; iy++)
              fillWithValue(ix, iy, 0);

          for (int cellid = 0; cellid < EEDetId::kSizeForDenseIndexing; ++cellid) {  // loop on EE cells
            if (EEDetId::validHashIndex(cellid)) {
              uint32_t rawid = EEDetId::unhashIndex(cellid);
              EcalFloatCondObjectContainer::const_iterator value_ptr = payload->find(rawid);
              if (value_ptr == payload->end())
                continue;  // cell absent from payload

              float weight = (float)(*value_ptr);
              EEDetId myEEId(rawid);
              if (myEEId.zside() == -1)
                fillWithValue(myEEId.ix(), myEEId.iy(), weight);
              else
                fillWithValue(myEEId.ix() + IX_MAX + EEhistSplit, myEEId.iy(), weight);
            }  // validDetId
          }  // loop over cellid

        }  // payload
      }  // loop over IOV's (1 in this case)
      return true;
    }  // fill method
  };

  /*************************************************
     2d plot of Ecal Time Calib Errors of 1 IOV
  *************************************************/
  class EcalTimeCalibErrorsPlot : public cond::payloadInspector::PlotImage<EcalTimeCalibErrors> {
  public:
    EcalTimeCalibErrorsPlot()
        : cond::payloadInspector::PlotImage<EcalTimeCalibErrors>("Ecal Time Calib Errors - map ") {
      setSingleIov(true);
    }

    bool fill(const std::vector<std::tuple<cond::Time_t, cond::Hash> >& iovs) override {
      TH2F* barrel = new TH2F("EB", "mean EB", MAX_IPHI, 0, MAX_IPHI, 2 * MAX_IETA, -MAX_IETA, MAX_IETA);
      TH2F* endc_p = new TH2F("EE+", "mean EE+", IX_MAX, IX_MIN, IX_MAX + 1, IY_MAX, IY_MIN, IY_MAX + 1);
      TH2F* endc_m = new TH2F("EE-", "mean EE-", IX_MAX, IX_MIN, IX_MAX + 1, IY_MAX, IY_MIN, IY_MAX + 1);

      auto iov = iovs.front();
      std::shared_ptr<EcalTimeCalibErrors> payload = fetchPayload(std::get<1>(iov));
      unsigned int run = std::get<0>(iov);

      if (payload.get()) {
        if (payload->barrelItems().empty())
          return false;

        fillEBMap_SingleIOV<EcalTimeCalibErrors>(payload, barrel);

        if (payload->endcapItems().empty())
          return false;

        fillEEMap_SingleIOV<EcalTimeCalibErrors>(payload, endc_m, endc_p);

      }  // payload

      gStyle->SetPalette(1);
      gStyle->SetOptStat(0);
      TCanvas canvas("CC map", "CC map", 1600, 450);
      TLatex t1;
      t1.SetNDC();
      t1.SetTextAlign(26);
      t1.SetTextSize(0.05);
      t1.DrawLatex(0.5, 0.96, Form("Ecal Time Calib Errors, IOV %i", run));

      float xmi[3] = {0.0, 0.24, 0.76};
      float xma[3] = {0.24, 0.76, 1.00};
      std::array<std::unique_ptr<TPad>, 3> pad;
      for (int obj = 0; obj < 3; obj++) {
        pad[obj] = std::make_unique<TPad>(Form("p_%i", obj), Form("p_%i", obj), xmi[obj], 0.0, xma[obj], 0.94);
        pad[obj]->Draw();
      }
      //      EcalDrawMaps ICMap;
      pad[0]->cd();
      //      ICMap.DrawEE(endc_m, 0., 2.);
      DrawEE(endc_m, 0., 2.5);
      pad[1]->cd();
      //      ICMap.DrawEB(barrel, 0., 2.);
      DrawEB(barrel, 0., 2.5);
      pad[2]->cd();
      //      ICMap.DrawEE(endc_p, 0., 2.);
      DrawEE(endc_p, 0., 2.5);

      std::string ImageName(m_imageFileName);
      canvas.SaveAs(ImageName.c_str());
      return true;
    }  // fill method
  };

  /*****************************************************************
     2d plot of Ecal Time Calib Errors difference between 2 IOVs
  *****************************************************************/
  template <cond::payloadInspector::IOVMultiplicity nIOVs, int ntags, int method>
  class EcalTimeCalibErrorsBase : public cond::payloadInspector::PlotImage<EcalTimeCalibErrors, nIOVs, ntags> {
  public:
    EcalTimeCalibErrorsBase()
        : cond::payloadInspector::PlotImage<EcalTimeCalibErrors, nIOVs, ntags>("Ecal Time Calib Errors comparison") {}

    bool fill() override {
      TH2F* barrel = new TH2F("EB", "mean EB", MAX_IPHI, 0, MAX_IPHI, 2 * MAX_IETA, -MAX_IETA, MAX_IETA);
      TH2F* endc_p = new TH2F("EE+", "mean EE+", IX_MAX, IX_MIN, IX_MAX + 1, IY_MAX, IY_MIN, IY_MAX + 1);
      TH2F* endc_m = new TH2F("EE-", "mean EE-", IX_MAX, IX_MIN, IX_MAX + 1, IY_MAX, IY_MIN, IY_MAX + 1);
      float pEBmin, pEEmin, pEBmax, pEEmax;
      pEBmin = 10.;
      pEEmin = 10.;
      pEBmax = -10.;
      pEEmax = -10.;

      unsigned int run[2];
      float pEB[kEBChannels], pEE[kEEChannels];
      std::string l_tagname[2];
      auto iovs = cond::payloadInspector::PlotBase::getTag<0>().iovs;
      l_tagname[0] = cond::payloadInspector::PlotBase::getTag<0>().name;
      auto firstiov = iovs.front();
      run[0] = std::get<0>(firstiov);
      std::tuple<cond::Time_t, cond::Hash> lastiov;
      if (ntags == 2) {
        auto tag2iovs = cond::payloadInspector::PlotBase::getTag<1>().iovs;
        l_tagname[1] = cond::payloadInspector::PlotBase::getTag<1>().name;
        lastiov = tag2iovs.front();
      } else {
        lastiov = iovs.back();
        l_tagname[1] = l_tagname[0];
      }
      run[1] = std::get<0>(lastiov);
      for (int irun = 0; irun < nIOVs; irun++) {
        std::shared_ptr<EcalTimeCalibErrors> payload;
        if (irun == 0) {
          payload = this->fetchPayload(std::get<1>(firstiov));
        } else {
          payload = this->fetchPayload(std::get<1>(lastiov));
        }
        if (payload.get()) {
          if (payload->barrelItems().empty())
            return false;

          fillEBMap_TwoIOVs<EcalTimeCalibErrors>(payload, barrel, irun, pEB, pEBmin, pEBmax, method);

          if (payload->endcapItems().empty())
            return false;

          fillEEMap_TwoIOVs<EcalTimeCalibErrors>(payload, endc_m, endc_p, irun, pEE, pEEmin, pEEmax, method);

        }  // payload
      }  // loop over IOVs

      gStyle->SetPalette(1);
      gStyle->SetOptStat(0);
      TCanvas canvas("CC map", "CC map", 1600, 450);
      TLatex t1;
      t1.SetNDC();
      t1.SetTextAlign(26);
      int len = l_tagname[0].length() + l_tagname[1].length();
      std::string dr[2] = {"-", "/"};
      if (ntags == 2) {
        if (len < 170) {
          t1.SetTextSize(0.05);
          t1.DrawLatex(0.5,
                       0.96,
                       Form("%s IOV %i %s %s  IOV %i",
                            l_tagname[1].c_str(),
                            run[1],
                            dr[method].c_str(),
                            l_tagname[0].c_str(),
                            run[0]));
        } else {
          t1.SetTextSize(0.05);
          t1.DrawLatex(0.5, 0.96, Form("Ecal Time Calib Errors, IOV %i%s  %i", run[1], dr[method].c_str(), run[0]));
        }
      } else {
        t1.SetTextSize(0.05);
        t1.DrawLatex(0.5, 0.96, Form("%s, IOV %i %s %i", l_tagname[0].c_str(), run[1], dr[method].c_str(), run[0]));
      }
      float xmi[3] = {0.0, 0.24, 0.76};
      float xma[3] = {0.24, 0.76, 1.00};
      std::array<std::unique_ptr<TPad>, 3> pad;

      for (int obj = 0; obj < 3; obj++) {
        pad[obj] = std::make_unique<TPad>(Form("p_%i", obj), Form("p_%i", obj), xmi[obj], 0.0, xma[obj], 0.94);
        pad[obj]->Draw();
      }

      pad[0]->cd();
      DrawEE(endc_m, pEEmin, pEEmax);
      pad[1]->cd();
      DrawEB(barrel, pEBmin, pEBmax);
      pad[2]->cd();
      DrawEE(endc_p, pEEmin, pEEmax);

      std::string ImageName(this->m_imageFileName);
      canvas.SaveAs(ImageName.c_str());
      return true;
    }  // fill method
  };  // class EcalTimeCalibErrorsDiffBase
  using EcalTimeCalibErrorsDiffOneTag = EcalTimeCalibErrorsBase<cond::payloadInspector::SINGLE_IOV, 1, 0>;
  using EcalTimeCalibErrorsDiffTwoTags = EcalTimeCalibErrorsBase<cond::payloadInspector::SINGLE_IOV, 2, 0>;
  using EcalTimeCalibErrorsRatioOneTag = EcalTimeCalibErrorsBase<cond::payloadInspector::SINGLE_IOV, 1, 1>;
  using EcalTimeCalibErrorsRatioTwoTags = EcalTimeCalibErrorsBase<cond::payloadInspector::SINGLE_IOV, 2, 1>;

  /*******************************************************
     2d plot of Ecal Time Calib Errors Summary of 1 IOV
   *******************************************************/
  class EcalTimeCalibErrorsSummaryPlot : public cond::payloadInspector::PlotImage<EcalTimeCalibErrors> {
  public:
    EcalTimeCalibErrorsSummaryPlot()
        : cond::payloadInspector::PlotImage<EcalTimeCalibErrors>("Ecal Time Calib Errors Summary- map ") {
      setSingleIov(true);
    }

    bool fill(const std::vector<std::tuple<cond::Time_t, cond::Hash> >& iovs) override {
      auto iov = iovs.front();
      std::shared_ptr<EcalTimeCalibErrors> payload = fetchPayload(std::get<1>(iov));
      unsigned int run = std::get<0>(iov);
      TH2F* align;
      int NbRows;

      if (payload.get()) {
        NbRows = 2;
        align = new TH2F("", "", 0, 0, 0, 0, 0, 0);

        float mean_x_EB = 0.0f;
        float mean_x_EE = 0.0f;

        float rms_EB = 0.0f;
        float rms_EE = 0.0f;

        int num_x_EB = 0;
        int num_x_EE = 0;

        payload->summary(mean_x_EB, rms_EB, num_x_EB, mean_x_EE, rms_EE, num_x_EE);
        fillTableWithSummary(align, "Ecal Time Calib Errors", mean_x_EB, rms_EB, num_x_EB, mean_x_EE, rms_EE, num_x_EE);

      } else
        return false;

      gStyle->SetPalette(1);
      gStyle->SetOptStat(0);
      TCanvas canvas("CC map", "CC map", 1000, 1000);
      TLatex t1;
      t1.SetNDC();
      t1.SetTextAlign(26);
      t1.SetTextSize(0.04);
      t1.SetTextColor(2);
      t1.DrawLatex(0.5, 0.96, Form("Ecal Time Calib Errors Summary, IOV %i", run));

      TPad pad("pad", "pad", 0.0, 0.0, 1.0, 0.94);
      pad.Draw();
      pad.cd();
      align->Draw("TEXT");

      drawTable(NbRows, 4);

      std::string ImageName(m_imageFileName);
      canvas.SaveAs(ImageName.c_str());

      return true;
    }
  };

}  // namespace

// Register the classes as boost python plugin
PAYLOAD_INSPECTOR_MODULE(EcalTimeCalibErrors) {
  PAYLOAD_INSPECTOR_CLASS(EcalTimeCalibErrorsEBMap);
  PAYLOAD_INSPECTOR_CLASS(EcalTimeCalibErrorsEEMap);
  PAYLOAD_INSPECTOR_CLASS(EcalTimeCalibErrorsPlot);
  PAYLOAD_INSPECTOR_CLASS(EcalTimeCalibErrorsDiffOneTag);
  PAYLOAD_INSPECTOR_CLASS(EcalTimeCalibErrorsDiffTwoTags);
  PAYLOAD_INSPECTOR_CLASS(EcalTimeCalibErrorsRatioOneTag);
  PAYLOAD_INSPECTOR_CLASS(EcalTimeCalibErrorsRatioTwoTags);
  PAYLOAD_INSPECTOR_CLASS(EcalTimeCalibErrorsSummaryPlot);
}
