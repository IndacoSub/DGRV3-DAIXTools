// Team DAIX, 2026
// BUGVAC
//
// The majority of this code was written between 2020 and 2022
//
// This tool's purpose is to:
// 1. Load all character name pairs (first name + surname)
// 2. Detect lines in .txt script files containing those names
// 3. Swap name/surname positions depending on configuration
// 4. Handle dash-prefixed partial-name variants (e.g., "K-Kaede")
// 5. Rewrite all affected files with corrected name ordering
//
// Used in case people HATE characters being called by surname

using System.Collections.Generic;
using System.Runtime.ConstrainedExecution;
using System.Text;

namespace Bugvac
{
	public class Program
	{
        // Represents a name pair: first = given name, second = surname.
        // Example: { "Kaede", "Akamatsu" }

        public struct VarEntry
		{
			public string first;
			public string second;
		}

		public static Dictionary<string, bool> ConfigMap = new Dictionary<string, bool>();
        // Names: full name pairs used for normal swapping.
        // DashNames: dash-prefixed fragments used to swap partial matches.
        // Example DashName: { "K-Kaede", "A-Akamatsu" }

        public static List<VarEntry> Names = new List<VarEntry>();
		public static List<VarEntry> DashNames = new List<VarEntry>();

        // MV = Make Var
        // Adds a name pair. If either contains a dash, store it in DashNames.
        // Example: MV("K-Kaede", "A-Akamatsu") → DashNames
        //          MV("Kaede", "Akamatsu") → Names

        static void MV(string f, string s)
		{
			if(f.Contains("-") || s.Contains("-"))
			{
				DashNames.Add(new VarEntry() { first = f, second = s });
			} else
			{
				Names.Add(new VarEntry() { first = f, second= s });
			}
		}

        // MP = Make Pair
        // Adds a configuration flag to ConfigMap.
        // Example: MP("DoSwapNames", true)

        static void MP(string s, bool b)
		{
			ConfigMap.Add(s, b);
		}

        // Populates all name and dash-name pairs used for swapping.
        // These define which names Bugvac can detect and replace.

        public static void SetupNames()
		{

			Names.Clear();

			/* Names */

			MV("Kaede", "Akamatsu");
			MV("Shūichi", "Saihara");
			MV("Tsumugi", "Shirogane");
			MV("Kaito", "Momota");
			MV("Kokichi", "Ōma");
			MV("Rantarō", "Amami");
			MV("Tenko", "Chabashira");
			// MV("Angie", "Yonaga");
			MV("Ryōma", "Hoshi");
			MV("Korekiyo", "Shingūji");
			MV("Miu", "Iruma");
			// MV("Gonta", "Gokuhara");
			MV("Himiko", "Yumeno");
			MV("Maki", "Harukawa");
			MV("Kirumi", "Tōjō");
			// K1-B0 / Keebo (doesn't have a surname)

			DashNames.Clear();

			MV("K-Kaede", "A-Akamatsu");
			MV("Ka-Kaede", "A-Akamatsu");
			MV("Kae-Kaede", "A-Akamatsu");

			MV("S-Shūichi", "Sa-Saihara");
			MV("Sh-Shūichi", "Sa-Saihara");
			MV("Shū-Shūichi", "Sa-Saihara");
			MV("S-Shūichi", "S-Saihara");

			MV("T-Tsumugi", "Sh-Shirogane");
			MV("Ts-Tsumugi", "Sh-Shirogane");
			MV("Tsu-Tsumugi", "Sh-Shirogane");
			MV("T-Tsumugi", "S-Shirogane");

			MV("K-Kaito", "Mo-Momota");
			MV("Ka-Kaito", "Mo-Momota");
			MV("Kai-Kaito", "Mo-Momota");
			MV("K-Kaito", "M-Momota");

			MV("K-Kokichi", "Ō-Ōma");
			MV("Ko-Kokichi", "Ō-Ōma");
			MV("Kok-Kokichi", "Ō-Ōma");

			MV("R-Rantarō", "A-Amami");
			MV("Ra-Rantarō", "A-Amami");
			MV("Ran-Rantarō", "A-Amami");

			MV("T-Tenko", "Ch-Chabashira");
			MV("Te-Tenko", "Ch-Chabashira");
			MV("Ten-Tenko", "Ch-Chabashira");
			MV("T-Tenko", "C-Chabashira");

			// Angie = Angie

			MV("R-Ryōma", "H-Hoshi");
			MV("Ry-Ryōma", "H-Hoshi");
			MV("Ryō-Ryōma", "H-Hoshi");

			MV("K-Korekiyo", "S-Shingūji");
			MV("Ko-Korekiyo", "S-Shingūji");
			MV("Kor-Korekiyo", "S-Shingūji");

			MV("M-Miu", "I-Iruma");
			MV("Mi-Miu", "I-Iruma");
			MV("Miu--", "I-Iruma");

			// Gonta = Gonta

			MV("H-Himiko", "Y-Yumeno");
			MV("Hi-Himiko", "Y-Yumeno");
			MV("Him-Himiko", "Y-Yumeno");

			MV("M-Maki", "Ha-Harukawa");
			MV("Ma-Maki", "Ha-Harukawa");
			MV("Mak-Maki", "Ha-Harukawa");
			MV("M-Maki", "H-Harukawa");

			MV("K-Kirumi", "T-Tōjō");
			MV("Ki-Kirumi", "T-Tōjō");
			MV("Kir-Kirumi", "T-Tōjō");

			// Keebo = Keebo
		}

        // Initializes all configuration flags with default values.
        // Mirrors the C++ ConfigMap used by other tools.

        public static void SetupConfigMap()
		{
			// ConfigMap.Add();

			ConfigMap.Clear();

			/* PLATFORM */

			MP("UseSwitchConfiguration", false);
			MP("UseXboxConfiguration", false);

			/* CLONING */

			MP("CloneBetas", false);

			/* CHARACTERS */

			// A.K.A. Compile for the Switch(/eventually others) version
			MP("ReplaceBlacklistedChars", false);

			/* VARIABLES */

			// Should we use our terminology or not?
			MP("UseIceConvention", true);
			MP("DoCheckVariables", true);
			MP("DoSwapNames", false);
			MP("DoRemoveSignals", false);
			MP("DoReplaceEmpty", true);

			/* FILES */

			// Only ones with different hashes, or all files?
			MP("CheckAllFiles", false);

			/* RANDOMIZATION */

			MP("DoRandomize", false);
			MP("SmartRandomization", false);

			/* UTILITY */

			MP("DoCountWords", false);

			/* BAKED */

			// ex. to use with RPG Maker(?)
			MP("NoCLTInBaked", false);
			// ex. if we want DGRV3-Daily to contain <PLATFORM=x>
			// to allow people to compile for their own preferred platform
			MP("NoPlatformInBaked", false);
		}

		static void ReadConfig(string file)
		{
			Console.WriteLine("Reading configuration file: " + file);

			if(!File.Exists(file))
			{
				Console.WriteLine("The configuration file was not found.");
				return;
			}

			using (StreamReader sr = new StreamReader(file))
			{
				long linecont = 0;
				string? temp = null;
				while ((temp = sr.ReadLine()) != null) {
					linecont++;
					if(temp.Length <= 0)
					{
						continue;
					}

					if(temp.StartsWith("{"))
					{
						continue;
					}

					if(temp.StartsWith("["))
					{
						continue;
					}

					if(temp.StartsWith("/"))
					{
						continue;
					}

					int find_equal = temp.IndexOf("=");
					if(find_equal == -1)
					{
						Console.WriteLine("No assignment found in : " + linecont);
						continue;
					}

					string parameter = temp.Substring(0, find_equal - 1);
					string allowed_characters = ("AaBbCcDdEeFfGgHhIiJjKkLlMmNnOoPpQqRrSsTtUuVvWwXxYyZz0123456789");
					int find_last_letter = -1;
					foreach(char cc in parameter)
					{
						if(!allowed_characters.Contains(cc))
						{
							break;
						}
						find_last_letter++;
					}
					if(find_last_letter == -1)
					{
						Console.WriteLine("Couldn't read last letter in line: " + linecont + " in parameter: \"" + parameter + "\"");
						foreach(char ch in parameter)
						{
							string allowed_str = allowed_characters;
							if(allowed_str == null)
							{
								Console.WriteLine("Invalid characters?");
								return;
							}
							if (allowed_str.IndexOf(ch) == -1)
							{
								Console.WriteLine("Unrecognized character: \"" + ch + "\"");
							}
						}
						continue;
					}

					parameter = parameter.Substring(0, find_last_letter + 1);

					if(ConfigMap.ContainsKey(parameter))
					{
						string value = temp.Substring(find_equal + 1 + 1);
						ConfigMap[parameter] = value != "FALSE";
						Console.WriteLine("Parameter " + parameter + " recognised as " + ConfigMap[parameter]);
					} else
					{
						Console.WriteLine("Setting not recognised in the ConfigMap (found in file but not in map): \"" + parameter + "\"");
					}
				}
				sr.Close();
				sr.Dispose();
			}

			Console.WriteLine("Configuration file red!");
		}

		static void ViewDebugCurrentConfig()
		{
			Console.WriteLine();
			Console.WriteLine("------------ CURRENT CONFIGURATION ------------");
			Console.WriteLine();
			foreach(var config in ConfigMap)
			{
				Console.WriteLine(config.Key + ": " + (config.Value ? "true" : "false"));
			}
			Console.WriteLine();
			Console.WriteLine("-----------------------------------------------");
			Console.WriteLine();
		}

        // Filters out non-script files (README, LICENSE, .git, backups, SHA files).
        // Only .txt files containing actual script lines should be processed.

        static bool IsFileGood(string file)
		{
			if(file.Contains("README"))
			{
				return false;
			}

			if(file.Contains("LICENSE"))
			{
				return false;
			}

			if(file.Contains(".git"))
			{
				return false;
			}

			if (file.Contains("_sha"))
			{
				return false;
			}

			if(file.Contains("_lines"))
			{
				return false;
			}

			if(file.Contains("Baked"))
			{
				return false;
			}

			if(file.Length <= 0)
			{
				return false;
			}

			if(file.Contains("vars_bak"))
			{
				return false;
			}

			if(!file.Contains(".txt"))
			{
				return false;
			}

			return true;
		}

        // Loads the list of files with mismatched SHA hashes (generated by Necronomicon).
        // Used when CheckAllFiles = false.

        static List<string> ReadDifferentHashesFile(string filename)
		{
			if(!File.Exists(filename))
			{
				return new List<string>();
			}

			List<string> ret = new List<string>();
			string cur = Directory.GetCurrentDirectory();

			using (StreamReader sr = new StreamReader(filename))
			{
				string? temp = null;
				while((temp = sr.ReadLine()) != null)
				{
					string newfile = Path.Combine(cur, temp);
					ret.Add(newfile);
				}
				sr.Close();
				sr.Dispose();
			}

			return ret;
		}

        // Determines which files Bugvac should process:
        // - If CheckAllFiles = true → scan entire repo
        // - Otherwise → only files listed in different_hashes.txt

        static List<string> GetFilesToSearch(string search_path)
		{
			List<string> file_to_search = new List<string>();

			if (ConfigMap["CheckAllFiles"])
			{
				var files = Directory.GetFiles(search_path, "*", SearchOption.AllDirectories);
				foreach(string file in files)
				{
					if(!IsFileGood(file))
					{
						continue;
					}

					file_to_search.Add(file);
				}
			} else
			{
				string current_dir = Directory.GetCurrentDirectory();
				string hashesfile = Path.Combine(current_dir, "different_hashes.txt");
				List<string> vec = ReadDifferentHashesFile(hashesfile);
				foreach(string file in vec)
				{
					if(!IsFileGood(file))
					{
						continue;
					}

					file_to_search.Add(file);
				}
			}

			return file_to_search;
		}

        // Returns true if the line contains any known name or surname.
        // Used to decide whether swapping is needed.

        static bool AnyNameInLine(string full_string)
		{
			foreach(var n in Names)
			{
				if(full_string.Contains(n.first))
				{
					return true;
				}

				if(full_string.Contains(n.second)) {
					return true;
				}
			}

			return false;
		}

        // Swaps name ↔ surname depending on mode.
        // Also handles dash-prefix variants (e.g., "K-Kaede").
        // Example: "Kaede Akamatsu" → "Akamatsu Kaede"

        static string SwapNames(string str, string name, string surname)
		{
			bool surname_mode = true;

			string mixed = name + " " + surname;
			string mixed_inv = surname + " " + name;

			bool name_found = str.Contains(name);
			bool surname_found = str.Contains(surname);

			bool single_found = name_found || surname_found;
			if (!single_found)
			{
				return str;
			}

			bool both_found = str.Contains(mixed) || str.Contains(mixed_inv);
			if (both_found)
			{
				Console.WriteLine("Found possible missed <SIGNAL_NOSWAP>? in " + str);
				return str;
			}

			string ret = str;

			foreach(VarEntry dashes in DashNames)
			{
				if(surname_mode)
				{
					if (dashes.second.Contains(surname))
					{
						//Console.WriteLine("(Surname) Replacing " + surname + "'s dash: from " + dashes.second + " to " + dashes.first); 
						ret = ret.Replace(dashes.second, dashes.first);
					}
				} else
				{
					if(dashes.first.Contains(name))
					{
						//Console.WriteLine("(Name) Replacing " + name + "'s dash: from " + dashes.first + " to " + dashes.second);
						ret = ret.Replace(dashes.first, dashes.second);
					}
				}
			}

			if (surname_mode)
			{
				ret = ret.Replace(surname, name);
			} else
			{
				ret = ret.Replace(name, surname);
			}
			return ret;
		}

        // Reads each file, swaps names where needed, and writes the modified content back.
        // Skips lines containing <SIGNAL_NOSWAP>.

        static void ReplaceNames(List<string> files_to_search)
		{
			bool testing = false;
			if(files_to_search.Count <= 0 && !testing)
			{
				Console.WriteLine("The list of files to search was found empty!");
				return;
			}

			Console.WriteLine("Replacing names...");

			if (!testing)
			{
				for (int ftp = 0; ftp < files_to_search.Count; ftp++)
				{
					StringBuilder sb = new StringBuilder();

					Console.WriteLine(ftp + " / " + files_to_search.Count);

					using (StreamReader sr = new StreamReader(files_to_search[ftp]))
					{
						string? line = null;
						while ((line = sr.ReadLine()) != null)
						{
							bool doswapname = AnyNameInLine(line) && !line.Contains("<SIGNAL_NOSWAP>");

							if (doswapname)
							{
								foreach (var i in Names)
								{
									string name = i.first;
									string surname = i.second;

									line = SwapNames(line, name, surname);
								}
							}

							sb.AppendLine(line);
						}
						sr.Close();
						sr.Dispose();
					}

					using (StreamWriter sw = new StreamWriter(files_to_search[ftp]))
					{
						sw.Write(sb.ToString());

						sw.Close();
						sw.Dispose();
					}
				}
			} else
			{
				string line = "test";
				bool doswapnames = !line.Contains("<SIGNAL_NOSWAP>");
				if (doswapnames)
				{
					Console.WriteLine("Original: \"" + line + "\"");
					foreach (var i in Names)
					{
						string name = i.first;
						string surname = i.second;

						line = SwapNames(line, name, surname);
					}
				}
				Console.WriteLine("Final: \"" + line + "\"");
			}
		}

        // Entry point:
        // 1. Load names and config
        // 2. Read TextConfig.config
        // 3. If DoSwapNames = true → perform name swapping

        static void Main(string[] args)
		{
			Console.WriteLine("Bugvac ON! (v1.1)");
			string current_dir = Directory.GetCurrentDirectory();
			string dgrv3path = Path.Combine(current_dir, "DGRV3");
			string configfile = Path.Combine(current_dir, "TextConfig.config");
			SetupNames();
			SetupConfigMap();
			ReadConfig(configfile);
			ViewDebugCurrentConfig();
			if (!ConfigMap["DoSwapNames"])
			{
				Console.WriteLine("Nothing to do, DoSwapNames is FALSE...");
				return;
			}
			List<string> fts = GetFilesToSearch(dgrv3path);
			ReplaceNames(fts);
			Console.WriteLine("Names replaced!");
		}
	}
}