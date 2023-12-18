#include "Clustering.hpp"

#include "TLorentzVector.h"

std::vector<Int_t> GetFinalParticles(Int_t const* motherIdxs, Int_t const* StatusArr, Int_t nGenPart)
{
    std::vector<Int_t> indices(nGenPart);
    std::vector<Bool_t> mask(nGenPart, false);
    for (Int_t pos = 0; pos < static_cast<Int_t>(nGenPart); ++pos)
    {
        Int_t motherIdx = motherIdxs[pos];
        if (motherIdx == -1) continue;
        mask[motherIdx] = true;
    }
    std::generate_n(indices.begin(), nGenPart, [count = 0]() mutable { return count++; });

    auto IsMother = [&mask](Int_t pos) { return mask[pos]; };
    indices.erase(std::remove_if(indices.begin(), indices.end(), IsMother), indices.end());

    return indices;
}

std::vector<Bool_t> PossibleJetConstituents(Int_t const* motherIdxs, Int_t const* StatusArr, Int_t nGenPart, GenPartIndex const& idx)
{
    std::vector<Bool_t> mask(nGenPart, true);
    Int_t lep_pos = idx.l;
    Int_t nu_pos = idx.nu;
    for (Int_t pos = 0; pos < static_cast<Int_t>(nGenPart); ++pos)
    {
        Int_t motherIdx = motherIdxs[pos];
        Int_t status = StatusArr[pos];
        if (IsIncoming(status) || pos == lep_pos || pos == nu_pos) mask[pos] = false;
        if (motherIdx == -1) continue;
        mask[motherIdx] = false;
    }
    return mask;
}

void AnalyzeJets(std::unique_ptr<TH1I> const& unused_cand, std::unique_ptr<TH1I> const& bad_jets, PtEtaPhiMArray const& genPart, PtEtaPhiMArray const& genJet, std::vector<Bool_t> candidates, Int_t n_unused)
{
    auto& [jet_pt, jet_eta, jet_phi, jet_m, nJets] = genJet;
    auto& [part_pt, part_eta, part_phi, part_m, nParts] = genPart;

    Int_t n_bad_jets = 0;

    for (Int_t jetIdx = 0; jetIdx < static_cast<Int_t>(nJets); ++jetIdx) // loop over all jets in event
    {
        TLorentzVector jet, jet_cand;
        jet.SetPtEtaPhiM(jet_pt[jetIdx], jet_eta[jetIdx], jet_phi[jetIdx], jet_m[jetIdx]);

        for (Int_t partIdx = 0; partIdx < static_cast<Int_t>(nParts); ++partIdx)
        {
            assert(nParts == candidates.size());
            if (!candidates[partIdx]) continue; // skip all particles that can't potentially constitue jet

            TLorentzVector part;
            part.SetPtEtaPhiM(part_pt[partIdx], part_eta[partIdx], part_phi[partIdx], part_m[partIdx]);

            float dR = part.DeltaR(jet);
            if (dR < 0.4)
            {   
                jet_cand += part;
                --n_unused;
            }
        }
        
        Double_t pt_ratio = jet_cand.Pt()/jet.Pt();
        if (pt_ratio > 1.1 || pt_ratio < 0.9) ++n_bad_jets;
    }

    unused_cand->Fill(n_unused);
    bad_jets->Fill(n_bad_jets);
}

std::vector<Overlap> FindOverlaps(PtEtaPhiMArray const& gen_jets, PtEtaPhiMArray const& gen_parts, std::vector<Bool_t> const& finals)
{
    std::vector<Overlap> res;

    auto const& [jet_pt, jet_eta, jet_phi, jet_m, n_jets] = gen_jets;
    auto const& [part_pt, part_eta, part_phi, part_m, n_parts] = gen_parts;

    for (Int_t partIdx = 0; partIdx < static_cast<Int_t>(n_parts); ++ partIdx) // loop over particles and compute dR with each jet
    {
        if (!finals[partIdx]) continue; // skip all parts that cannot be jet constituents

        Overlap ov;
        auto& [idx, jets] = ov;
        idx = partIdx;

        TLorentzVector part;
        part.SetPtEtaPhiM(part_pt[partIdx], part_eta[partIdx], part_phi[partIdx], part_m[partIdx]);

        for (Int_t jetIdx = 0; jetIdx < static_cast<Int_t>(n_jets); ++jetIdx)
        {
            TLorentzVector jet;
            jet.SetPtEtaPhiM(jet_pt[jetIdx], jet_eta[jetIdx], jet_phi[jetIdx], jet_m[jetIdx]);

            Float_t dR = part.DeltaR(jet);
            if (dR < 0.4) jets.push_back(jetIdx);
        }

        if (jets.size() > 1) res.push_back(std::move(ov));
    }

    return res;
}

std::pair<Int_t, Int_t> FindBestMatch(Overlap const& ov, PtEtaPhiMArray const& gen_jets, PtEtaPhiMArray const& gen_parts)
{
    auto const& [partIdx, jets] = ov;
    auto const& [jet_pt, jet_eta, jet_phi, jet_m, n_jets] = gen_jets;
    auto const& [part_pt, part_eta, part_phi, part_m, n_parts] = gen_parts;

    TLorentzVector part;
    part.SetPtEtaPhiM(part_pt[partIdx], part_eta[partIdx], part_phi[partIdx], part_m[partIdx]);

    Int_t best_jet = 0;
    Float_t best_dr = 1; // overlaps are found by FindOverlaps which guarantees that dR between part and all jets and overlaps will be < 0.4
    for (Int_t jetIdx = 0; jetIdx < static_cast<Int_t>(jets.size()); ++jetIdx)
    {
        TLorentzVector jet;
        jet.SetPtEtaPhiM(jet_pt[jetIdx], jet_eta[jetIdx], jet_phi[jetIdx], jet_m[jetIdx]);

        Float_t dR = part.DeltaR(jet);
        if (dR < best_dr) best_jet = jetIdx;
    }

    return std::make_pair(partIdx, best_jet);
}

// std::vector<Int_t> GetDaughtersOf(Int_t mother_pdgId, Int_t mother_idx) {}

Bool_t IsFinal(Int_t pos, Int_t const* motherIdxs, Int_t nGenPart)
{
    for (Int_t i = 0; i < nGenPart; ++i)
    {
        Int_t mother_idx = motherIdxs[i]; // mother of current particle
        if (mother_idx == pos) return false;
    }
    return true;
}

Bool_t DecayProductOf(Int_t this_part, Int_t pos, Int_t const* motherIdxs)
{
    Int_t current_mother = motherIdxs[this_part]; 
    while (current_mother != -1)
    {
        if (current_mother == pos) return true;
        this_part = current_mother;
        current_mother = motherIdxs[this_part];
    }
    return false;
}