param([string]$esp = "10.0.0.6")
$ErrorActionPreference = "Stop"
try {
  $s = (Invoke-WebRequest -UseBasicParsing "http://$esp/state" -TimeoutSec 8).Content | ConvertFrom-Json
} catch { Write-Host "device $esp not responding" -ForegroundColor Red; exit 1 }
$W = 46
function Bar { param($c) Write-Host ("+" + ($c * ($W-2)) + "+") -ForegroundColor DarkCyan }
function Row { param($l,$col="White") $pad = $W - 4 - $l.Length; if($pad -lt 0){$pad=0}
  Write-Host "| " -NoNewline -ForegroundColor DarkCyan
  Write-Host ($l + (" " * $pad)) -NoNewline -ForegroundColor $col
  Write-Host " |" -ForegroundColor DarkCyan }
function Spark { param($a)
  if (-not $a) { return "" }
  $g = @(".",":","-","=","+","*","#","@")
  $lo = ($a | Measure-Object -Minimum).Minimum
  $hi = ($a | Measure-Object -Maximum).Maximum
  $r = $hi - $lo; if ($r -le 0) { $r = 1 }
  ($a | ForEach-Object { $g[[int](($_ - $lo) / $r * 7)] }) -join "" }

Write-Host ""
Bar "="
Row "(*) SLH  CRYPTO" "Cyan"
Bar "-"
foreach ($r in $s.rows) {
  $sym = $r.symbol.PadRight(4)
  if ($r.symbol -eq "SLH") {
    Row ("$sym  ILS " + [math]::Round($r.price,0) + "   [PRESALE]") "Yellow"
  } else {
    $p = "$" + ("{0:N2}" -f $r.price)
    $ch = "{0:+0.00;-0.00}" -f $r.change_24h
    $col = if ($r.change_24h -ge 0) { "Green" } else { "Red" }
    $ar = if ($r.change_24h -ge 0) { "^" } else { "v" }
    Row ("$sym " + $p.PadLeft(13) + "  $ar$ch%") $col
    $sp = Spark $r.spark
    if ($sp) { Row ("     " + $sp) "DarkGray" }
  }
}
Bar "-"
Row ("$($s.source)  |  $($s.age_s)s ago") "Gray"
Bar "="
Write-Host ""
Write-Host ("  " + $s.device.mac + "   " + $s.device.ip + "   " + $s.device.rssi + " dBm   up " + [math]::Round($s.device.uptime_s/60,1) + "m   heap " + [math]::Round($s.device.free_heap/1024) + "KB") -ForegroundColor DarkGray
Write-Host ("  touch: pressed=" + $s.touch.pressed + "  cal=[" + ($s.touch.cal -join ",") + "]") -ForegroundColor DarkGray
Write-Host ""
