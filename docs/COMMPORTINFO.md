# COM Port / USB Serial Access from WSL2

Windows COM ports are not automatically available in WSL2 (which runs as a VM). The supported solution is **usbipd-win**, which attaches the USB device directly into WSL so it appears as `/dev/ttyACM*` or `/dev/ttyUSB*`.

---

## Installation

### Windows (PowerShell as Admin)

```powershell
winget install usbipd
```

### WSL (Ubuntu)

```bash
sudo apt install linux-tools-generic hwdata
sudo update-alternatives --install /usr/local/bin/usbip usbip $(find /usr/lib/linux-tools/*/usbip | tail -1) 20
```

### dialout group (one-time)

```bash
sudo usermod -aG dialout $USER
# then: wsl --shutdown from Windows, reopen WSL
```

---

## Daily Use

Run these from PowerShell as Admin each time you plug in a device:

```powershell
usbipd list                          # find BUSID
usbipd bind --busid <BUSID>          # first time only - makes device shareable
usbipd attach --wsl --busid <BUSID>  # attach to WSL
```

Use `--auto-attach` to re-attach automatically after USB replug while the command is running:

```powershell
usbipd attach --wsl --busid <BUSID> --auto-attach
```

Then in WSL:

```bash
ls /dev/ttyACM*
```

---

## usbipd list with COM Port Numbers

Standard `usbipd list` does not show Windows COM port numbers. Save this script as e.g. `C:\Users\<username>\scripts\usbipd-list.ps1` and run it instead:

```powershell
$comPorts = Get-CimInstance Win32_PnPEntity |
    Where-Object { $_.Name -match '\(COM\d+\)' } |
    ForEach-Object {
        $hwId = $_.PNPDeviceID
        $vid  = if ($hwId -match 'VID_([0-9A-Fa-f]{4})') { $Matches[1].ToLower() } else { $null }
        $pid_ = if ($hwId -match 'PID_([0-9A-Fa-f]{4})') { $Matches[1].ToLower() } else { $null }
        [PSCustomObject]@{
            COM    = if ($_.Name -match '\((COM\d+)\)') { $Matches[1] } else { $null }
            VidPid = if ($vid -and $pid_) { "${vid}:${pid_}" } else { $null }
        }
    }

usbipd list | ForEach-Object {
    $line  = $_
    $match = $comPorts | Where-Object { $_.VidPid -and $line -match $_.VidPid } | Select-Object -First 1
    if ($match) { "$line  [$($match.COM)]" } else { $line }
}
```

Add a WSL alias for convenience in `~/.bashrc`:

```bash
alias usbipd-list='powershell.exe -File "/mnt/c/Users/<username>/scripts/usbipd-list.ps1"'
```

---

## Identifying Devices (ESP32-S3)

Each ESP32-S3 has a unique USB serial number derived from its MAC address. Query it with:

```bash
udevadm info /dev/ttyACM0 | grep -E 'SERIAL|VENDOR|MODEL|PRODUCT'
```

### Known Devices

| Symlink              | Serial Number      | Notes                 | COM Port | WSL dev    |
|----------------------|--------------------|-----------------------|----------|------------|
| `/dev/esp32_base`    | 3C:0F:02:E5:46:F4  | BLE peripheral (base) | COM6     | ttyACM1    |
| `/dev/esp32_rover`   | 3C:0F:02:E5:50:FC  | BLE central (rover)   | COM12    | ttyACM0    |
| TBD                  |                    |                       |          |            |
| TBD                  |                    |                       |          |            |

---

## Persistent Symlinks via udev

Symlinks are configured in `/etc/udev/rules.d/99-esp32.rules`. Current content:

```
SUBSYSTEM=="tty", ATTRS{idVendor}=="303a", ATTRS{idProduct}=="1001", ATTRS{serial}=="Espressif_USB_JTAG_serial_debug_unit_3C:0F:02:E5:46:F4", SYMLINK+="esp32_base"
SUBSYSTEM=="tty", ATTRS{idVendor}=="303a", ATTRS{idProduct}=="1001", ATTRS{serial}=="Espressif_USB_JTAG_serial_debug_unit_3C:0F:02:E5:50:FC", SYMLINK+="esp32_rover"
```

To add a new device: enumerate it with `udevadm info /dev/ttyACMx`, then append a line and reload:

```bash
sudo udevadm control --reload-rules && sudo udevadm trigger
```
