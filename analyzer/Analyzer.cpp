#include "Analyzer.hpp"

#include <iostream>

Analyzer::Analyzer(TString const& tree_name, std::vector<TString> const& input_files)
: m_chain(std::make_unique<TChain>(tree_name)),
  m_data(),
  m_input_files(input_files)
{
    for (auto const& input_file: m_input_files)
    {
        m_chain->Add(input_file);
    }
    m_data = std::make_unique<EventData>(*m_chain); 
}

void Analyzer::Analyze() const
{
    std::cout << "Files in TChain:\n";
    for (auto const& input_file: m_input_files)
    {
        std::cout << "\t" << input_file << "\n";
    }
    std::cout << "nEntries = " << m_chain->GetEntries() << "\n";
}