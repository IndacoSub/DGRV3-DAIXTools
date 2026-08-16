// Team DAIX, 2026
// V3DAILYMANAGER — REMOTE OPERATIONS
//
// The majority of this code was written between 2020 and 2022
//
// This file contains all remote‑repository operations used by the DAILY
// automation pipeline. DAILY is completely separate from DAIXTOOLS and acts
// as a CI/CD system for Danganronpa V3 modding.
//
// These functions handle:
//   • Cloning repositories (main + beta branches)
//   • Updating .gitignore rules based on repository type
//   • Updating README files (IT + EN) with progress and timestamps
//   • Committing, tagging, and pushing changes to GitHub
//
// DAILY uses these helpers to publish:
//   • Patch repositories
//   • Text repositories
//   • Log repositories
//   • SPC repositories
//
// All Git operations are performed through VGit and VGitUtils.

#include "remote.h"

#include <iostream>
#include <fstream>

#include "files.h"
#include "common.h"

#include "vgit.h"
#include "vgit_utils.h"

namespace Remote {

	// Clone()
	// DAILY uses this to clone any repository required for the build:
	//   • DGRV3-Tools
	//   • DGRV3-Daily(-Private)
	//   • DGRV3-Daily-Text(-Private)
	//   • DGRV3-Daily-Log(-Private)
	//   • DGRV3-Daily-SPC(-Private)
	//
	// Steps:
	//   1. Check Git version
	//   2. Clone main branch
	//   3. Optionally clone beta branches (if beta == true)
	//   4. If beta cloning fails, write vgit_failed.txt so DAILY can abort early

	bool Clone(std::string const& repo, std::string const& repo_url, fsys::path const& current_dir, bool const& beta, std::vector<std::string> const& taken_branches) {

		// Get the current git version
		VGit::GetGitVersion();

		// Clones the main branch
		VGit::CloneRepositoryMain(repo_url);

		// If we want to use beta stuff
		if (beta) {
			// If beta failed
			if (!VGitUtils::CloneBeta(true, repo, repo_url, taken_branches)) {
				std::cout << std::endl;
				std::cout << "The repository couldn't be cloned successfully!" << std::endl;
				std::cout << std::endl;

				// Save to file that it failed, so that we may exit early
				VGitUtils::SaveToFile("vgit_failed.txt", ":(");

				return false;
			}
		}
		return true;
	}

	// UploadToCloud()
	// DAILY uses this to upload build results to GitHub.
	//
	// It performs:
	//   • Detect repository type (patch/text/log/SPC)
	//   • Update .gitignore rules based on repo type
	//   • Update README.md and README_EN.md with progress + timestamp
	//   • Stage all changes
	//   • Commit with timestamp + metadata
	//   • Create a tag (YYYY.MM.DD)
	//   • Push commit and tag to origin/main
	//
	// This is the final step of DAILY before reporting build completion.


	void UploadToCloud(fsys::path const& current_dir, std::string const& upload_repo_name, std::string const& info) {

		bool const is_private = Common::StringContains(upload_repo_name, "-Private");
		bool const is_text_repo = Common::StringContains(upload_repo_name, "-Text");
		bool const is_log_repo = Common::StringContains(upload_repo_name, "-Log");
		bool const is_spc_repo = Common::StringContains(upload_repo_name, "-SPC");

		std::string const repo_path = (current_dir / upload_repo_name).string();

		std::cout << "Upload repo name: " << upload_repo_name << " in " << current_dir << std::endl;

		std::cout << "Updating gitignore!" << std::endl;

		Remote::UpdateGitignore(repo_path, is_text_repo || is_log_repo, !is_text_repo && !is_log_repo && !is_spc_repo, is_spc_repo);

		std::cout << "gitignore updated!" << std::endl;

		std::cout << "Updating README!" << std::endl;

		std::pair<std::string, std::string> const update_time = Common::GetTime();

		Remote::UpdateReadme(repo_path, current_dir, update_time.first, is_private);

		std::cout << "README updated!" << std::endl;

		VGit::AddAll(repo_path);
		VGit::CommitWithMessage(repo_path, "Update: " + update_time.first + info);
		VGit::Tag(repo_path, update_time.second, update_time.first);
		VGit::Push(repo_path, "origin", "main");
		VGit::PushTag(repo_path, update_time.second);
	}

	// UpdateGitignore()
	// DAILY regenerates .gitignore for each repository before upload.
	//
	// Rules depend on repository type:
	//   • Text repo: allow .txt, block patches
	//   • Patch repo: allow .ups, block .txt
	//   • SPC repo: allow .spc, block .ups and .txt
	//
	// All repos block heavy binary formats (.7z, .cpk, .assets, .arc, etc.).
	//
	// The old .gitignore is deleted and replaced with a freshly generated one.


	void UpdateGitignore(fsys::path const& file_location, bool const& allow_txt, bool const& allow_patches, bool const& allow_spc) {

		auto const gitignore_file = file_location / ".gitignore";

		std::cout << "Current gitignore: " << gitignore_file << std::endl;

		if (!fsys::remove(gitignore_file) && fsys::exists(gitignore_file)) {
			std::cout << "Couldn't remove old gitignore file: " << gitignore_file << "!" << std::endl;
			return;
		}

		std::stringstream ss{};
		
		ss << "" << std::endl;
		ss << "*.7z" << std::endl;
		ss << "*.cpk" << std::endl;
		ss << "*.srd" << std::endl;
		ss << "*.sfl" << std::endl;
		ss << "*.stx" << std::endl;
		ss << "*.ab" << std::endl;
		ss << "*.pb" << std::endl;
		ss << "*.assets" << std::endl;
		ss << "*.arc" << std::endl;
		ss << "*_url.txt" << std::endl;
		ss << "*license.txt" << std::endl;
		ss << "autov3.txt" << std::endl;
		//ss << "../../../V3DailyManager/*" << std::endl;

		if (!allow_spc) {
			ss << "*.spc" << std::endl;
		}

		if (!allow_txt) {
			// Add .txt?
			ss << "*.txt" << std::endl;
		}

		if (!allow_patches) {
			// Add .ups patches?
			ss << "*.ups" << std::endl;
		}

		ss << "" << std::endl;

		std::ofstream ofsout(gitignore_file, std::ios::out);
		ofsout << ss.str();
		ofsout.close();
	}

	// UpdateReadme()
	// DAILY updates both README.md (Italian) and README_EN.md (English).
	//
	// Steps:
	//   • Check if the files exist
	//   • Retrieve translation progress percentages via Files::GetPercentages()
	//   • Call UpdateReadmeIT() and UpdateReadmeEN() to regenerate content
	//
	// These READMEs are displayed publicly on the Daily repositories.


	void UpdateReadme(fsys::path const& readme_location, fsys::path const& current_dir, std::string const& time, bool is_private) {

		auto const readme_file_it = readme_location / "README.md";
		if (!fsys::exists(readme_file_it)) {
			std::cout << "Readme file not found!" << std::endl;
		}

		auto const readme_file_en = readme_location / "README_EN.md";
		if (!fsys::exists(readme_file_en)) {
			std::cout << "English readme file not found!" << std::endl;
		}

		std::vector<std::string> const percentages = Files::GetPercentages(current_dir);

		UpdateReadmeIT(percentages, readme_file_it, time, is_private);
		UpdateReadmeEN(percentages, readme_file_en, time, is_private);
	}

	void UpdateReadmeIT(std::vector<std::string> const& percentages, fsys::path const& readme_file_it, std::string const& update_time, bool is_private) {

		std::stringstream ss{};
		ss << "## Info ##" << std::endl;
		ss << std::endl;
		ss << "Traduzione italiana del gioco \"Danganronpa V3: Killing Harmony\" (repository pubblica del testo)." << std::endl;
		ss << std::endl;
		ss << "Se la repository non è stata aggiornata per settimane, per favore apri una \"issue\" su questa repository" << std::endl;
		ss << "o entra nel nostro server Discord per segnalare direttamente il problema (https://discord.gg/EZ4X3hxKYh)." << std::endl;
		ss << std::endl;
		ss << "Se non ti piace/convince come abbiamo tradotto qualcosa, facci sapere." << std::endl;
		ss << std::endl;
		ss << "I nostri canali Telegram sono @IndacoSub e @V3TraduzioneIta, unisciti se vuoi!" << std::endl;
		ss << std::endl;
		ss << "## Progressi ##" << std::endl;
		ss << std::endl;
		ss << "Ultimo aggiornamento (approssimativamente): " << update_time << std::endl;
		ss << std::endl;
		if (!percentages.empty()) {
			for (auto const& percent : percentages) {
				ss << percent << std::flush << std::endl << std::endl;
			}
		}
		else {
			ss << "Dati non disponibili. Per favore contattaci per segnalare il problema." << std::endl;
		}
		ss << std::endl;
		ss << "## Altri link ##" << std::endl;
		ss << std::endl;
		if (is_private) {
			ss << EncryptString("[Repo patch](https://github.com/IndacoSub/DGRV3-Daily-Private)") << std::endl;
			ss << std::endl;
			ss << EncryptString("[Repo testo](https://github.com/IndacoSub/DGRV3-Daily-Private-Text)") << std::endl;
			ss << std::endl;
			ss << EncryptString("[Repo logs](https://github.com/IndacoSub/DGRV3-Daily-Private-Log)") << std::endl;
			ss << std::endl;
		}
		else {
			ss << EncryptString("[Repo patch](https://github.com/IndacoSub/DGRV3-Daily)") << std::endl;
			ss << std::endl;
			ss << EncryptString("[Repo testo](https://github.com/IndacoSub/DGRV3-Daily-Text)") << std::endl;
			ss << std::endl;
			ss << EncryptString("[Repo logs](https://github.com/IndacoSub/DGRV3-Daily-Log)") << std::endl;
			ss << std::endl;
		}

		if (!fsys::remove(readme_file_it)) {
			std::cout << "Couldn't remove old README file!" << std::endl;
			//return;
		}

		std::ofstream ofsout(readme_file_it, std::ios::out);
		ofsout << ss.str();
		ofsout.close();
	}

	void UpdateReadmeEN(std::vector<std::string> const& percentages, fsys::path const& readme_file_en, std::string const& update_time, bool is_private) {

		std::stringstream ss{};
		ss << "## Info ##" << std::endl;
		ss << std::endl;
		ss << "Italian translation for the game \"Danganronpa V3: Killing Harmony\" (public text repository)." << std::endl;
		ss << std::endl;
		ss << "If the repository hasn't been updated in some weeks, please open an issue on this repository" << std::endl;
		ss << "or join our Discord server to report the issue directly (https://discord.gg/EZ4X3hxKYh)." << std::endl;
		ss << std::endl;
		ss << "If you don't like how we translated something, please let us know." << std::endl;
		ss << std::endl;
		ss << "Our Telegram channels are @IndacoSub and @V3TraduzioneIta, join them if you want!" << std::endl;
		ss << std::endl;
		ss << "## Progress ##" << std::endl;
		ss << std::endl;
		ss << "Updated (approximatively): " << update_time << std::endl;
		ss << std::endl;
		if (!percentages.empty()) {
			for (auto const& percent : percentages) {
				ss << percent << std::flush << std::endl << std::endl;
			}
		}
		else {
			ss << "Unavailable data. Please contact us and report this problem." << std::endl;
		}
		ss << std::endl;
		ss << "## More links ##" << std::endl;
		ss << std::endl;
		if (is_private) {
			ss << EncryptString("[Patch repo](https://github.com/IndacoSub/DGRV3-Daily-Private)") << std::endl;
			ss << std::endl;
			ss << EncryptString("[Text repo](https://github.com/IndacoSub/DGRV3-Daily-Private-Text)") << std::endl;
			ss << std::endl;
			ss << EncryptString("[Logs repo](https://github.com/IndacoSub/DGRV3-Daily-Private-Log)") << std::endl;
			ss << std::endl;
		}
		else {
			ss << EncryptString("[Patch repo](https://github.com/IndacoSub/DGRV3-Daily)") << std::endl;
			ss << std::endl;
			ss << EncryptString("[Text repo](https://github.com/IndacoSub/DGRV3-Daily-Text)") << std::endl;
			ss << std::endl;
			ss << EncryptString("[Logs repo](https://github.com/IndacoSub/DGRV3-Daily-Log)") << std::endl;
			ss << std::endl;
		}

		if (!fsys::remove(readme_file_en)) {
			std::cout << "Couldn't remove old README file!" << std::endl;
			//return;
		}

		std::ofstream ofsout(readme_file_en, std::ios::out);
		ofsout << ss.str();
		ofsout.close();
	}
}