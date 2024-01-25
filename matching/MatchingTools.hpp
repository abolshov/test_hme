#ifndef M_TOOLS_HPP
#define M_TOOLS_HPP

#include <vector>
#include <iostream>

static const std::vector<int> LIGHT_QUARKS = {1, 2, 3, 4};
static const std::vector<int> LEPTONS = {11, 13};
static const std::vector<int> NEUTRINOS = {12, 14};

static constexpr int N_SIG_PART = 8;

static constexpr int RADION_ID = 35;
static constexpr int HIGGS_ID = 25;
static constexpr int W_ID = 24;
// static constexpr int WPLUS_ID = 24;
// static constexpr int WMINUS_ID = -24;
static constexpr int B_ID = 5;
// static constexpr int BBAR_ID = -5;

// specifies order of signal (hh->bbWW->bbqqlv) particles
enum SIG { h1, h2, b1, b2, q1, q2, l, nu };

// finds closest daughters of particle at location part_idx in the event;
// returns indices of what found
std::vector<int> GetNextGeneration(int part_idx, int const* mothers, int n_gen_part);

// finds all descendants of particle at location part_idx in the event;
// descendants are grouped into generations (inner vectors): generation 0 is particle itself, generation 1 are its immediate daughters, etc ...
std::vector<std::vector<int>> GetDescendants(int part_idx, int const* mothers, int n_gen_part);

// finds all particles by index that do not have daughters
std::vector<int> GetFinalParticles(int const* mothers, int n_gen_part);

// by default onlyh prints pdg_id(mother_idx) (in that format)
// if print_idx is true, prints pdg_id[idx](mother_idx)
void PrintDecay(std::vector<std::vector<int>> const& decay, int const* pdg_ids, int const* mothers, bool print_idx = false, std::ostream& stream = std::cout);

// returns index of the last occurence of particle with pdgId target_id
int FindLast(int target_id, int const* pdg_ids, int n_gen_part);

// returns indices of signal (see enum SIG) particles
// access specific particles via SIG::_
// head is position of the last heavy higgs, i.e. index where the decay starts 
// if any signal particle was not found returns empty vector
std::vector<int> GetSignal(int const* pdg_ids, int const* mothers, int n_gen_part);

// finds daughters of mother at mother_idx with pdg ids in range desc_range
std::vector<int> FindSpecificDescendants(std::vector<int> const& desc_range, int mother_idx, int const* mothers, int const* pdg_ids, int n_gen_part);
#endif