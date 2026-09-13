param([string]$cmd = "status", [string]$arg = "")
$VIP = @{ "osif" = "10.0.0.2"; "zvika" = "10.0.0.3" }
function Hit($ip, $path) { try { return Invoke-RestMethod "http://$ip$path" -TimeoutSec 6 } catch { return $null } }
switch ($cmd) {
  "status" {
    Write-Host ""
    foreach ($k in $VIP.Keys) {
      $ip = $VIP[$k]
      $s = Hit $ip "/state"
      if (-not $s) { Write-Host ("  {0,-8} {1,-10} OFFLINE" -f $k.ToUpper(), $ip) -ForegroundColor Red; continue }
      $v = Hit $ip "/view"
      $age = if ($v) { $v.age_s } else { 0 }
      $col = if ($age -gt 180) { "Yellow" } else { "Green" }
      Write-Host ("  {0,-8} {1,-10} {2,-8} up {3,5}m  heap {4}KB  data {5}s" -f $k.ToUpper(), $ip, $v.screen, [math]::Round($s.device.uptime_s/60), [math]::Round($s.device.free_heap/1024), $age) -ForegroundColor $col
    }
    Write-Host ""
  }
  "screen" {
    foreach ($k in $VIP.Keys) {
      $r = Hit $VIP[$k] "/screen?to=$arg"
      $ok = if ($r) { "OK" } else { "FAIL" }
      Write-Host ("  {0,-8} -> {1}  {2}" -f $k.ToUpper(), $arg, $ok) -ForegroundColor Cyan
    }
  }
  "view" {
    foreach ($k in $VIP.Keys) {
      $v = Hit $VIP[$k] "/view"
      if (-not $v) { continue }
      Write-Host ""
      Write-Host ("  === " + $k.ToUpper() + " / " + $v.screen.ToUpper() + " ===") -ForegroundColor Cyan
      foreach ($l in $v.lines) { Write-Host ("    " + $l) }
    }
    Write-Host ""
  }
  "flash" {
    foreach ($k in $VIP.Keys) { & .\flash-ota.ps1 -esp $VIP[$k] }
  }
  default { Write-Host "  usage: slh.ps1 status | view | screen <name> | flash" -ForegroundColor Yellow }
}
