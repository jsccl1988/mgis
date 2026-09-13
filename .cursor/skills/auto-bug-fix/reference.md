# mgis launch, WER, and symbols

Read this when running the auto-bug-fix loop on the WinMain desktop binary.

## Launch (real binary)

`out/smartgis.exe` is a WinMain app. **cwd must be `out/`** so DLL/data loads match a normal desktop start. Do not treat a console harness or Explorer double-click of a `*_test.exe` as the product.

From repo root `c:/Dev/src/gis/mgis`:

```powershell
$exe = (Resolve-Path "out/smartgis.exe").Path
$p = Start-Process -FilePath $exe -WorkingDirectory (Resolve-Path "out") -PassThru
# Poll: window title SmartGIS, then $p.HasExited / $p.ExitCode / lifetime
```

If `out/smartgis.exe` is missing, rebuild with `.\build.bat` first (or **auto-build-fix** on compile failure).

Watch:

- Window created? Title **SmartGIS**?
- Process still alive after the user’s scenario (not just “it started”)?
- Immediate exit with no window → load failure / missing DLL (not necessarily AV)

## WER / Application Error

```powershell
Get-WinEvent -FilterHashtable @{
  LogName = 'Application'
  ProviderName = 'Application Error'
} -MaxEvents 8 | ForEach-Object { $_.TimeCreated; $_.Message }
```

Record **faulting application**, **faulting module**, **exception code**, **fault offset**, and time vs process lifetime.

Also check `C:/ProgramData/Microsoft/Windows/WER/ReportArchive` when Event Log is thin.

Common codes:

| Code | Meaning |
| --- | --- |
| `0xc0000005` | Access violation |
| `0xc000041d` | Unhandled exception after a prior fault (often a follow-up, not a second root cause) |
| `0xc0000135` / “dll was not found” | Missing DLL / load failure |

## Map module + offset

Do not pick a `.cpp` from the DLL name.

1. Confirm the on-disk module under `out/` (e.g. `out/content.dll`) matches the WER module.
2. `dumpbin /headers out/content.dll` — ImageBase, section layout.
3. Resolve fault offset against **PDB** next to the DLL, a **`.map`**, or a debugger (`devenv`, WinDbg, `llvm-symbolizer` if present).
4. Only then open that function / line.

If symbols cannot resolve the offset, gather more evidence (debug build, PDB present, exact module version). Do not guess.

## Example classification (not a live task)

`out/smartgis.exe` launched, window **SmartGIS** appeared, then AV in `content.dll` exception `0xc0000005` offset `0x476ec`, follow-up `0xc000041d`, process gone ~30s.

That is **crash-after-window**, not a missing DLL, and not a console `return 0` 秒退. Map `0x476ec` in that `content.dll` before editing.

## Rebuild

Same entry as **auto-build-fix**: from `c:/Dev/src/gis/mgis` run `.\build.bat`. Canonical compile transcript: `out/build.log`. After a green build, resume the reproduce steps in this file — compile green is not the done bar.
