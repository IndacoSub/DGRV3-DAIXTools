// Team DAIX, 2026
// LOOSEFLOORBOARD — ASCII Folding / Accent Removal Tool
//
// The majority of this code was written between 2020 and 2022
//
// This program was only possible thanks to a few users on StackOverflow and GitHub.
//
// This tool's purpose is to:
// 1. Convert accented and non‑ASCII Unicode characters into ASCII equivalents
// 2. Work around platform/font limitations (Switch, older consoles, etc.)
// 3. Optionally replace “Z/z” with “2” for specific blacklisted filenames
// 4. Recursively process all .txt files in the working directory
// 5. Skip vars_bak.txt and anything inside base_spc/
// 6. Provide pre‑processing for HydraulicPress when ReplaceBlacklistedChars = TRUE
//
// LooseFloorboard is an optional TextTools helper used only when certain
// platforms cannot display accented characters. It ensures text compatibility
// before HydraulicPress performs variable replacement and baking.

using System;
using System.Collections.Generic;
using System.IO;
using System.Threading;

namespace LooseFloorboard
{
    class Program
    {
        static void Main(string[] args)
        {
            Console.WriteLine("Current folder: " + Directory.GetCurrentDirectory());

            Thread.Sleep(5000); // sleep for 5 seconds... Why though?

            // Array which contains file where the letter "Z"/"z" should be replaced
            List<string> blacklisted_z = new List<string>()
            {
                "A-MapListNameAscii.txt", // Map names (contains some Zs)
            };

            string str = "";
            foreach (var file in
                 Directory.EnumerateFiles(Directory.GetCurrentDirectory(), "*.txt", SearchOption.AllDirectories))
            {
                string fn = Path.GetFileName(file);
                str = file.ToString();
                if(str.Contains("vars_bak"))
                {
                    continue;
                }
                if(str.Contains("base_spc"))
                {
                    continue;
                }
                var lines = File.ReadAllLines(str);

                List<string> outstr = new List<string>();
                foreach (string line in lines)
                {
                    outstr.Add(StringExtensions.FoldToASCII(line, blacklisted_z.Contains(fn)));
                }
                File.Delete(str);
                File.WriteAllLines(str, outstr);
            }
        }
    }
}
