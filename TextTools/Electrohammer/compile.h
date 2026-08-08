#pragma once

#include <vector>
#include <atomic>

#include "Electrohammer.h"
#include "entrymanagement.h"

namespace Compiler {
	std::string CalculateStx(std::string const& txt, std::filesystem::path const& where);
	void CalculateSpc(std::string const& stx, std::filesystem::path const& program, std::filesystem::path const& where, std::string const& file_to_insert);
	void Compile(std::filesystem::path const& where, std::vector<EntryMG::Entry>& entries);
	void CalculateEntrySTX(EntryMG::Entry const& entry);
	void CalculateAllEntriesSPC(std::vector<EntryMG::Entry> const& entries);
	std::string MoveTXTForSTXTool(std::string const& filename, std::filesystem::path const& whereto, bool const& actually_copy = true);

	inline std::atomic<std::uint64_t> STXCompiled = 0;
	inline std::atomic<std::uint64_t> SPCCompiled = 0;
}