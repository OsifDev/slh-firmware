param([string]$esp = "10.0.0.6")
try { $s = (Invoke-WebRequest -UseBasicParsing "http://$esp/state" -TimeoutSec 10).Content | ConvertFrom-Json }
catch { Write-Host "device not responding" -ForegroundColor Red; exit 1 }
Write-Host ""
Write-Host "  SLH MARKET CONTEXT    $(Get-Date -f 'HH:mm:ss')" -ForegroundColor Cyan
Write-Host "  ------------------------------------------------------------" -ForegroundColor DarkCyan
foreach ($r in $s.rows) {
  if ($r.symbol -eq "SLH") { continue }
  $col = if ($r.change_24h -ge 0) { "Green" } else { "Red" }
  Write-Host ""
  Write-Host ("  {0}   `${1:N2}   {2:+0.00;-0.00}%" -f $r.symbol, $r.price, $r.change_24h) -ForegroundColor $col
  $pos = [int]$r.pos_in_range
  $bar = ""
  for ($i=0; $i -lt 40; $i++) { if ($i -eq [int]($pos*0.4)) { $bar += "|" } else { $bar += "-" } }
  Write-Host ("     24h range  [" + $bar + "]  " + $pos + "%") -ForegroundColor DarkGray
  Write-Host ("     low `${0:N2}   high `${1:N2}   spread {2:N2}%" -f $r.low24, $r.high24, $r.range_pct) -ForegroundColor DarkGray
  $vt = if ($r.range_pct -gt 4) { "HIGH" } elseif ($r.range_pct -gt 2) { "NORMAL" } else { "LOW" }
  $vc = if ($r.range_pct -gt 4) { "Yellow" } else { "DarkGray" }
  Write-Host ("     volatility {0}   volume {1:N0}   trades {2:N0}" -f $vt, $r.volume, $r.trades) -ForegroundColor $vc
}
Write-Host ""
Write-Host "  ------------------------------------------------------------" -ForegroundColor DarkCyan
Write-Host "  source $($s.source)   data age $($s.age_s)s   $($s.device.ip)" -ForegroundColor DarkGray
Write-Host "  This is market context, not a recommendation." -ForegroundColor DarkGray
Write-Host ""
