param([string]$esp = "10.0.0.4")
try { $v = (Invoke-WebRequest -UseBasicParsing "http://$esp/view" -TimeoutSec 8).Content | ConvertFrom-Json }
catch { Write-Host "device not responding" -ForegroundColor Red; exit 1 }
Write-Host ""
Write-Host "  +----------------------------------------+" -ForegroundColor DarkCyan
$hdr = "  | SLH  " + $v.screen.ToUpper()
Write-Host ($hdr.PadRight(43) + "|") -ForegroundColor Cyan
Write-Host "  +----------------------------------------+" -ForegroundColor DarkCyan
foreach ($l in $v.lines) { Write-Host ("  | " + $l.PadRight(38) + " |") -ForegroundColor White }
Write-Host "  +----------------------------------------+" -ForegroundColor DarkCyan
$ftr = "  | " + $v.source + "  " + $v.age_s + "s ago"
Write-Host ($ftr.PadRight(43) + "|") -ForegroundColor DarkGray
Write-Host "  +----------------------------------------+" -ForegroundColor DarkCyan
Write-Host ""
