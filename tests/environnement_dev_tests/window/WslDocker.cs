// CI transport for a Windows runner whose Linux Docker daemon lives in WSL.
// Keep arguments separate, including PowerShell's multiline readiness probe.
using System.Diagnostics;
using System.Text.RegularExpressions;

var start = new ProcessStartInfo("wsl.exe") { UseShellExecute = false };
foreach (var arg in new[] { "-d", "Prysma", "-u", "prysma", "--exec", "docker" })
    start.ArgumentList.Add(arg);

for (int index = 0; index < args.Length; index++)
{
    string arg = args[index];
    // Only filesystem operands belong to WSL. The indexer's host path and
    // workspace ownership labels must retain their original Windows values.
    bool volume = index > 0 && args[index - 1] == "-v";
    bool copy = args[0] == "cp";
    if ((volume || copy) && Regex.IsMatch(arg, @"^[A-Za-z]:[\\/]"))
        arg = "/mnt/" + char.ToLowerInvariant(arg[0]) + arg.Substring(2).Replace('\\', '/');
    start.ArgumentList.Add(arg);
}
using var client = Process.Start(start)!;
client.WaitForExit();
return client.ExitCode;
