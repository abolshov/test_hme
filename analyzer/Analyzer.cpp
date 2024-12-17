#include "Analyzer.hpp"
#include "Definitions.hpp"
#include "EstimatorUtils.hpp"

#include <iostream>

#include "TH1.h"

Analyzer::Analyzer(TString const& tree_name, std::map<TString, Channel> const& input_file_map, TString const& pdf_file_name, Mode mode)
:   m_file_map(input_file_map)
,   m_tree_name(tree_name)  
,   m_estimator(pdf_file_name)   
{
    TH1::AddDirectory(false);
}

void Analyzer::ProcessFile(TString const& name, Channel ch)
{
    TFile* file = TFile::Open(name);
    TTree* tree = static_cast<TTree*>(file->Get<TTree>(m_tree_name));

    m_storage.ConnectTree(tree, ch);
    ULong64_t n_events = tree->GetEntries();
    for (ULong64_t evt = 0; evt < n_events; ++evt)
    {
        ProcessEvent(evt, tree, ch);
    }

    file->Close();
}

void Analyzer::ProcessEvent(ULong64_t evt, TTree* tree, Channel ch)
{
    tree->GetEntry(evt);

    VecLVF_t jets = GetRecoJetP4(m_storage);
    VecLVF_t leptons = GetRecoLepP4(m_storage, ch);
    LorentzVectorF_t met = GetRecoMET(m_storage);
    std::vector<Float_t> jet_resolutions = GetPNetRes(m_storage);
}