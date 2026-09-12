param([string]$esp = "10.0.0.6")
Write-Host ""
Write-Host "   ####  #     #   #   " -ForegroundColor Cyan
Write-Host "  #      #     #   #   " -ForegroundColor Cyan
Write-Host "   ###   #     #####   FIRMWARE FLASH" -ForegroundColor Cyan
Write-Host "      #  #     #   #   " -ForegroundColor Cyan
Write-Host "  ####   ##### #   #   target: " -NoNewline -ForegroundColor Cyan
Write-Host $esp -ForegroundColor Yellow
Write-Host ""
$t0 = Get-Date
$bin = Join-Path $PSScriptRoot ".pio\build\esp32-2432s028\firmware.bin"
pio run
if ($LASTEXITCODE -ne 0) { Write-Host "BUILD FAILED" -ForegroundColor Red; exit 1 }
$size = (Get-Item $bin).Length
$b = "----slhboundary" + [guid]::NewGuid().ToString("N")
$enc = [System.Text.Encoding]::GetEncoding("iso-8859-1")
$head = "--$b`r`nContent-Disposition: form-data; name=`"f`"; filename=`"firmware.bin`"`r`nContent-Type: application/octet-stream`r`n`r`n"
$tail = "`r`n--$b--`r`n"
$body = New-Object System.IO.MemoryStream
$hb = $enc.GetBytes($head); $body.Write($hb, 0, $hb.Length)
$fb = [System.IO.File]::ReadAllBytes($bin); $body.Write($fb, 0, $fb.Length)
$tb = $enc.GetBytes($tail); $body.Write($tb, 0, $tb.Length)
$bytes = $body.ToArray(); $body.Close()
try {
  $req = [System.Net.HttpWebRequest]::Create("http://$esp/update")
  $req.Method = "POST"
  $req.ContentType = "multipart/form-data; boundary=$b"
  $req.ContentLength = $bytes.Length
  $req.Timeout = 180000
  $req.ReadWriteTimeout = 180000
  $st = $req.GetRequestStream()
  $st.Write($bytes, 0, $bytes.Length)
  $st.Close()
  $resp = $req.GetResponse()
  $sr = New-Object System.IO.StreamReader($resp.GetResponseStream())
  Write-Host ("device said: " + $sr.ReadToEnd()) -ForegroundColor Green
  $sr.Close(); $resp.Close()
} catch { Write-Host "OTA FAILED: $($_.Exception.Message)" -ForegroundColor Red; exit 1 }
Start-Sleep 14
try {
  $s = (Invoke-WebRequest -UseBasicParsing "http://$esp/state" -TimeoutSec 10).Content | ConvertFrom-Json
  Write-Host "BACK ONLINE - uptime $($s.device.uptime_s)s" -ForegroundColor Green
} catch { Write-Host "not up yet" -ForegroundColor Yellow }
