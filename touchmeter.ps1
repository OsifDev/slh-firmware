param([string]$esp = "10.0.0.4", [int]$secs = 20)
$end = (Get-Date).AddSeconds($secs)
$hits = 0; $miss = 0
Write-Host ""
Write-Host "  TOUCH METER - press and drag on the screen" -ForegroundColor Yellow
Write-Host "  --------------------------------------------------" -ForegroundColor DarkCyan
while ((Get-Date) -lt $end) {
  try {
    $t = Invoke-RestMethod "http://$esp/touch" -TimeoutSec 3
    if ($t.pressed) {
      $hits++
      $zone = if ($t.screen.y -ge 196) { "NAVBAR" } elseif ($t.screen.y -lt 34) { "header" } else { "content" }
      $bar = "#" * [math]::Min(40, [int]($t.raw.z / 40))
      Write-Host ("  x={0,3} y={1,3}  z={2,4}  {3,-7} {4}" -f $t.screen.x, $t.screen.y, $t.raw.z, $zone, $bar) -ForegroundColor Green
    } else { $miss++ }
  } catch { }
  Start-Sleep -Milliseconds 120
}
Write-Host "  --------------------------------------------------" -ForegroundColor DarkCyan
$total = $hits + $miss
$pct = if ($total) { [math]::Round($hits * 100 / $total) } else { 0 }
Write-Host ("  detected {0} of {1} samples  ({2}%)" -f $hits, $total, $pct) -ForegroundColor Cyan
Write-Host ""
