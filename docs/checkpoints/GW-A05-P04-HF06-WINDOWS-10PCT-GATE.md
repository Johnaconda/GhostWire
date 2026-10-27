# GhostWire provisional 10% Windows native acceptance gate

This is an **owner-operated** check, not a previously observed Windows result.
The 10% boundary comes from the *provisional* 110-point scope model, not the
checkpoint number or unit-test pass fraction.

1. Extract the cumulative HF06 ZIP to a **new** folder; do not overlay an older
   checkout. Keep the archive quarantined if Microsoft Defender flags it. Inspect
   Protection History; source checksums alone are not malware clearance.
2. In Windows PowerShell with VS2026 x64 C++ tools available, change directory to
   the newly extracted `GhostWire` root (where CMakeLists.txt resides).
3. Run without bypassing security policy:

   ```powershell
   .\tools\windows\dev-windows.ps1 -Action all -Configuration Debug -OfflineTests -Fresh
   .\tools\windows\dev-windows.ps1 -Action all -Configuration Release -OfflineTests -Fresh
   ```

4. Record actual configure/build status, CTest totals, `protocol-unit`,
   `protocol-allocation-fault`, `development-module-registry`, and
   `development-effort-forecast` outcomes, plus Windows/Defender observations.
5. If PowerShell policy denies script execution, **do not bypass or whitelist**.
   Use native CMake/Ninja from the VS x64 developer environment instead:

   ```powershell
   cmake --preset portable-debug
   cmake --build --preset portable-debug
   ctest --test-dir out/build/portable-debug --output-on-failure
   cmake --preset portable-release
   cmake --build --preset portable-release
   ctest --test-dir out/build/portable-release --output-on-failure
   ```

6. The repo `main` and the extracted checkpoint are **different working trees**.
   Do not push or merge without owner authorization. No exact Windows test count
   is promised: conditional platform registrations differ from Linux.
7. If all checks pass, the owner may accept the 10% native gate without
   accepting the unfinished A05 upstream parser/network security gate.
