#include "Selector.hpp"

#include <algorithm>
#include <iostream>

std::vector<int> Selector::GetNextGen(int part_idx, int const* mothers, int n_gen_part) const 
{
    std::vector<int> gen;
    for (int i = 0; i < n_gen_part; ++i)
    {
        if (mothers[i] == part_idx)
        {
            gen.push_back(i);
        }
    }
    return gen;
}

void Selector::BuildTree(std::unique_ptr<EventData> const& data)
{
    int first_X_idx = -1;
    int generation_counter = 0;
    auto ptr = std::find(data->GenPart_pdgId, data->GenPart_pdgId + data->nGenPart, RADION_ID);
    first_X_idx = ptr - data->GenPart_pdgId;

    // TODO: consider case when after loop first_X_idx = -1
    int const* mothers = data->GenPart_genPartIdxMother;
    int n_gen_part = data->nGenPart;

    m_generations[generation_counter] = {first_X_idx};
    std::vector<int> gen = GetNextGen(first_X_idx, mothers, n_gen_part); // compute first-gen daughters of X
    m_stable[generation_counter] = gen.empty() ? std::vector<bool>(1, true) : std::vector<bool>(1, false);
    ++generation_counter;

    while (!gen.empty())
    {
        m_generations[generation_counter] = gen;

        // compute next generation daughters for each particle in gen
        // fill mask of stable particles for particles in gen
        std::vector<int> next_gen;
        std::vector<bool> stable_mask;
        for (auto part: gen)
        {
            std::vector<int> tmp = GetNextGen(part, mothers, n_gen_part);
            stable_mask.push_back(tmp.empty() ? true : false);
            next_gen.insert(next_gen.end(), tmp.begin(), tmp.end());
        }

        m_stable[generation_counter] = stable_mask;
        gen = next_gen;
        ++generation_counter;
    }

    auto last_gen_it = --m_generations.end();
    auto const& [last_gen_number, last_gen] = *last_gen_it;
    int last_gen_sz = last_gen.size();
    m_stable[last_gen_number] = std::vector<bool>(last_gen_sz, true);
}

void Selector::PrintTree(std::unique_ptr<EventData> const& data) const 
{
    int const* pdg_id = data->GenPart_pdgId;
    for (auto const& [gen_num, gen_vec]: m_generations)
    {
        std::cout << "generation " << gen_num << ":\n\t";
        for (auto p: gen_vec)
        {
            std::cout << pdg_id[p] << " ";
        }
        std::cout << "\n";
    }
}

bool Selector::IsDescOf(int cand_idx, int parent_idx, int const* mothers) const
{
    int mother_idx = mothers[cand_idx];
    while(mother_idx != -1)
    {
        if (mother_idx == parent_idx) return true;
        int new_mother_idx = mothers[mother_idx];
        mother_idx = new_mother_idx;
    }
    return false;
}

std::pair<std::vector<int>, std::vector<int>> Selector::FindFirstLast(int target_id, int mother_idx, int const* pdg_ids, int const* mothers) const
{
    std::map<int, std::vector<int>> occurences;
    for (auto const& [gen_num, gen_vec]: m_generations)
    {
        for (auto const& index: gen_vec)
        {
            // if particle at index descends from mother_idx (i.e. across generations), also count it
            if (pdg_ids[index] == target_id && IsDescOf(index, mother_idx, mothers))
            {
                occurences[gen_num].push_back(index);
            }
        }
    }

    if (occurences.empty())
    {
        return std::make_pair<std::vector<int>, std::vector<int>>({}, {});
    }

    auto first = occurences.begin();
    auto last = --occurences.end();

    if (first == last)
    {
        std::vector<int> v = first->second;
        return {v, v};
    }

    return std::make_pair<std::vector<int>, std::vector<int>>(std::move(first->second), std::move(last->second));
}

std::optional<SignalData> Selector::Select(std::unique_ptr<EventData> const& data) const
{
    return std::nullopt;
}