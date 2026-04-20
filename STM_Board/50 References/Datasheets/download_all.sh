#!/usr/bin/env bash
# Download all confirmed-component datasheets into organized subfolders.
# Run from: ~/stm_board/50 References/Datasheets/
#
# Skipped: motor, motor driver, battery specific models (Phase B pending).
#
# Usage:
#   bash download_all.sh             # downloads everything
#   bash download_all.sh --list      # just prints the list of URLs
#
# Failed downloads will be reported at the end. Some vendors (Stereolabs,
# NVIDIA, Hirose) may require login/consent page — those will fail and
# must be fetched manually via browser.

set -u

HERE="$(cd "$(dirname "$0")" && pwd)"
cd "$HERE" || exit 1

LIST_ONLY=0
[[ "${1:-}" == "--list" ]] && LIST_ONLY=1

FAILED=()
OK=0
SKIP=0

dl() {
  local subdir="$1" fname="$2" url="$3"
  local dest="$subdir/$fname"
  mkdir -p "$subdir"
  if [[ $LIST_ONLY -eq 1 ]]; then
    printf "%-50s %s\n" "$dest" "$url"
    return 0
  fi
  if [[ -s "$dest" ]]; then
    echo "  [skip] $dest (exists)"
    SKIP=$((SKIP+1))
    return 0
  fi
  echo "  [get ] $dest"
  if curl -fsSL -A "Mozilla/5.0 (datasheet-fetcher)" -o "$dest" --retry 2 --retry-delay 2 --max-time 60 "$url"; then
    OK=$((OK+1))
  else
    echo "  [FAIL] $dest  ($url)" >&2
    rm -f "$dest"
    FAILED+=("$dest  <- $url")
  fi
}

section() { echo; echo "### $1"; }

################################################################
section "01_MCU"
dl 01_MCU STM32H743xx.pdf              "https://www.st.com/resource/en/datasheet/stm32h743vi.pdf"
dl 01_MCU AN5436_H7_HW_Checklist.pdf   "https://www.st.com/resource/en/application_note/an5437-hardware-design-for-stm32h7-series-stmicroelectronics.pdf"
dl 01_MCU AN2867_Oscillator.pdf        "https://www.st.com/resource/en/application_note/an2867-guidelines-for-oscillator-design-on-stm8afals-and-stm32-mcusmpus-stmicroelectronics.pdf"
dl 01_MCU AN4879_USB_HS.pdf            "https://www.st.com/resource/en/application_note/an4879-introduction-to-usb-hardware-and-pcb-guidelines-using-stm32-mcus-stmicroelectronics.pdf"
dl 01_MCU UM1974_Nucleo_H743ZI.pdf     "https://www.st.com/resource/en/user_manual/um1974-stm32-nucleo144-boards-mb1137-stmicroelectronics.pdf"

section "02_ADC_Loadcell"
dl 02_ADC_Loadcell ADS131M04.pdf           "https://www.ti.com/lit/ds/symlink/ads131m04.pdf"
dl 02_ADC_Loadcell ADS131M04EVM_UG.pdf     "https://www.ti.com/lit/ug/sbau329/sbau329.pdf"
dl 02_ADC_Loadcell REF5025.pdf             "https://www.ti.com/lit/ds/symlink/ref5025.pdf"
dl 02_ADC_Loadcell SBAA532_ADS131M_Design.pdf "https://www.ti.com/lit/an/sbaa532/sbaa532.pdf"

section "03_Encoder"
# AS5048A: ams sometimes redirects to HTML landing; manual fetch may be needed.
dl 03_Encoder AS5048A.pdf "https://look.ams-osram.com/m/7059eac7531a86fd/original/AS5048-Data-Sheet.pdf"

section "04_CAN_Isolation"
dl 04_CAN_Isolation ISO1050.pdf         "https://www.ti.com/lit/ds/symlink/iso1050.pdf"
dl 04_CAN_Isolation MGJ2D05050SC.pdf    "https://power.murata.com/datasheet?/data/power/ncl/kdc_mgj2.pdf"
dl 04_CAN_Isolation PRTR5V0U2X.pdf      "https://assets.nexperia.com/documents/data-sheet/PRTR5V0U2X.pdf"
dl 04_CAN_Isolation BZT52_series.pdf    "https://www.diodes.com/assets/Datasheets/ds18001.pdf"
# ACM2012-900 and SM712 often behind product pages — may need manual
dl 04_CAN_Isolation SM712.pdf           "https://www.semtech.com/uploads/documents/sm712.pdf"

section "05_USB_Debug"
dl 05_USB_Debug TPD4S012.pdf "https://www.ti.com/lit/ds/symlink/tpd4s012.pdf"
dl 05_USB_Debug USB4105.pdf  "https://gct.co/files/drawings/usb4105.pdf"

section "06_Battery_Monitor"
dl 06_Battery_Monitor INA228.pdf "https://www.ti.com/lit/ds/symlink/ina228.pdf"

section "07_Logic_Power"
dl 07_Logic_Power TPS62933.pdf   "https://www.ti.com/lit/ds/symlink/tps62933.pdf"
dl 07_Logic_Power AP63205WU.pdf  "https://www.diodes.com/assets/Datasheets/AP63200-AP63201-AP63203-AP63205.pdf"
dl 07_Logic_Power TLV75533.pdf   "https://www.ti.com/lit/ds/symlink/tlv75533.pdf"
dl 07_Logic_Power XAL4040.pdf    "https://www.coilcraft.com/getmedia/fb72e61d-5aee-47bb-b1ba-5f76a23b1d5d/xal4040.pdf"

section "08_Protection_Reverse"
dl 08_Protection_Reverse DMP2305U.pdf "https://www.diodes.com/assets/Datasheets/DMP2305U.pdf"
# BZT52C12 shares same family datasheet as above (BZT52_series.pdf)

section "09_Protection_Inrush"
dl 09_Protection_Inrush SL08_NTC.pdf      "https://www.ametherm.com/hubfs/docs/data-sheets/sl08-datasheet.pdf"
dl 09_Protection_Inrush Omron_G5V-1.pdf   "https://omronfs.omron.com/en_US/ecb/products/pdf/en-g5v_1.pdf"

section "10_Protection_Bus"
dl 10_Protection_Bus P6KE33CA.pdf  "https://www.vishay.com/docs/88471/p6kea.pdf"
dl 10_Protection_Bus Rubycon_ZLH.pdf "https://www.rubycon.co.jp/wp-content/uploads/catalog/e_pdfs/aluminum/e_ZLH.pdf"

section "11_Protection_Local"
dl 11_Protection_Local SMAJ_series.pdf  "https://www.vishay.com/docs/88428/smaj.pdf"
# OS-CON SVPD often behind Panasonic portal
dl 11_Protection_Local OS-CON_SVPD.pdf  "https://industrial.panasonic.com/cdbs/www-data/pdf/RDE0000/ABA0000C1063.pdf"

section "12_Protection_Brake"
dl 12_Protection_Brake TLV3201.pdf     "https://www.ti.com/lit/ds/symlink/tlv3201.pdf"
dl 12_Protection_Brake IPB025N10N3.pdf "https://www.infineon.com/dgdl/Infineon-IPB025N10N3G-DataSheet-v02_01-EN.pdf?fileId=db3a30432239cccd01226ef8a974070a"

section "13_Safety_UI"
dl 13_Safety_UI 74LVC1G08.pdf   "https://assets.nexperia.com/documents/data-sheet/74LVC1G08.pdf"
dl 13_Safety_UI 74LVC1G373.pdf  "https://assets.nexperia.com/documents/data-sheet/74LVC1G373.pdf"

section "14_Storage"
dl 14_Storage AN5200_SDMMC.pdf "https://www.st.com/resource/en/application_note/an5200-how-to-use-sdmmc-in-stm32-microcontrollers-stmicroelectronics.pdf"
# Hirose DM3AT usually requires portal — may fail, fetch manually

section "15_Wireless"
dl 15_Wireless ESP32-C3-MINI-1U.pdf  "https://www.espressif.com/sites/default/files/documentation/esp32-c3-mini-1_datasheet_en.pdf"
dl 15_Wireless ESP32-C3_HW_Design.pdf "https://www.espressif.com/sites/default/files/documentation/esp32-c3_hardware_design_guidelines_en.pdf"

section "16_Passives_Grounding"
# Murata ferrites share a consolidated PDF
dl 16_Passives_Grounding Murata_ferrite_beads.pdf "https://www.murata.com/products/productdata/8796738650142/ENFA0003.pdf"

section "17_System_Modules (Jetson, ZED, EBIMU — vendor pages, may need manual)"
dl 17_System_Modules Jetson_Orin_NX.pdf "https://developer.nvidia.com/downloads/embedded/l4t/r35_release_v1.0/datasheets/JETSON_ORIN_NX_MODULE_DS_10929_002.pdf"
# ZED X Mini spec page is HTML — fetch manually from https://store.stereolabs.com/
# EBIMU V5: e2box.co.kr — Korean site, fetch manually

################################################################
# Reference boards (GitHub repos / schematics)
echo
echo "### Reference Boards (manual clone / download recommended)"
echo "  Nucleo-H743ZI schematic: https://www.st.com/en/evaluation-tools/nucleo-h743zi.html"
echo "  MJBots Moteus:           git clone https://github.com/mjbots/moteus"
echo "  ODrive v3.6:             git clone https://github.com/odriverobotics/ODrive"
echo "  OpenExo (local):         ../../../Walker_Main/Hardware/PCB/CAN_PCB/"

################################################################
echo
echo "==== Summary ===="
echo "  OK:     $OK"
echo "  Skip:   $SKIP"
echo "  Fail:   ${#FAILED[@]}"
if [[ ${#FAILED[@]} -gt 0 ]]; then
  echo
  echo "==== Failed (fetch manually via browser) ===="
  printf '  %s\n' "${FAILED[@]}"
fi

if [[ $LIST_ONLY -eq 0 ]]; then
  echo
  echo "PDFs saved under: $HERE/"
fi
