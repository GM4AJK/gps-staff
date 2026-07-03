#!/usr/bin/env bash
# Grant all permissions to the GPS Staff app on a connected Android device.
# Run once after each fresh install: bash scripts/grant-android-permissions.sh
# Requires adb in PATH. On this machine:
#   export PATH="$PATH:/mnt/c/Users/kirkh/Local Settings/Android/Sdk/platform-tools"

PKG="com.example.gps_staff_app"

DANGEROUS=(
    android.permission.ACCESS_FINE_LOCATION
    android.permission.ACCESS_COARSE_LOCATION
    android.permission.ACCESS_BACKGROUND_LOCATION
    android.permission.READ_EXTERNAL_STORAGE
    android.permission.WRITE_EXTERNAL_STORAGE
    android.permission.BLUETOOTH_SCAN
    android.permission.BLUETOOTH_CONNECT
    android.permission.BLUETOOTH_ADVERTISE
    android.permission.POST_NOTIFICATIONS
)

echo "Granting permissions to $PKG..."
for perm in "${DANGEROUS[@]}"; do
    adb shell pm grant "$PKG" "$perm" && echo "  OK  $perm" || echo "  --  $perm (not applicable on this API level)"
done

# MANAGE_EXTERNAL_STORAGE cannot be granted via pm grant — needs appops
echo "Granting MANAGE_EXTERNAL_STORAGE via appops..."
adb shell appops set "$PKG" MANAGE_EXTERNAL_STORAGE allow && echo "  OK  MANAGE_EXTERNAL_STORAGE"

echo "Done."
