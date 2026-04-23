#!/usr/bin/env bash
# Download all confirmed-component datasheets.
# v2 (2026-04-20): ALL URLs re-verified via web search. Multiple fallbacks per
# part. Uses browser-like User-Agent to reduce bot blocking.
#
# Run from: ~/stm_board/50 References/Datasheets/
#
# Usage:
#   bash download_all.sh             # downloads everything
#   bash download_all.sh --list      # just prints URLs without downloading
#
# Some vendor sites (Stereolabs ZED, NVIDIA Developer) require login/JS and
# will always fail — fetch manually via browser.

set -u

HERE="$(cd "$(dirname "$0")" && pwd)"
cd "$HERE" || exit 1

LIST_ONLY=0
[[ "${1:-}" == "--list" ]] && LIST_ONLY=1

FAILED=()
OK=0
SKIP=0

UA="Mozilla/5.0 (Macintosh; Intel Mac OS X 10_15_7) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/120.0.0.0 Safari/537.36"

# dl SUBDIR FILENAME URL [FALLBACK_URL...]
dl() {
  local subdir="$1"; shift
  local fname="$1"; shift
  local dest="$subdir/$fname"
  mkdir -p "$subdir"

  if [[ $LIST_ONLY -eq 1 ]]; then
    printf "%-55s %s\n" "$dest" "$1"
    shift
    for alt in "$@"; do
      printf "%-55s  (fallback) %s\n" "" "$alt"
    done
    return 0
  fi

  if [[ -s "$dest" ]]; then
    echo "  [skip] $dest (exists)"
    SKIP=$((SKIP+1))
    return 0
  fi

  local url
  for url in "$@"; do
    echo "  [get ] $dest  <- $url"
    if curl -fsSL -A "$UA" -H "Accept: application/pdf,*/*" \
         -o "$dest" --retry 2 --retry-delay 2 --max-time 90 "$url"; then
      # sanity check: file must be > 10 KB (error pages are usually tiny)
      local sz=$(stat -f%z "$dest" 2>/dev/null || stat -c%s "$dest" 2>/dev/null || echo 0)
      if [[ $sz -gt 10240 ]]; then
        OK=$((OK+1))
        return 0
      else
        echo "  [warn] got file < 10KB ($sz bytes), trying next URL"
        rm -f "$dest"
      fi
    fi
  done

  echo "  [FAIL] $dest  (all URLs failed)" >&2
  FAILED+=("$dest")
}

section() { echo; echo "### $1"; }

################################################################
section "01_MCU  (STM32H743, ST app notes, Nucleo-H743ZI)"
dl 01_MCU STM32H743xx.pdf \
  "https://www.st.com/resource/en/datasheet/stm32h743vi.pdf" \
  "https://www.st.com/resource/en/datasheet/stm32h743zi.pdf"

dl 01_MCU AN5436_H7_HW_Checklist.pdf \
  "https://www.st.com/resource/en/application_note/an5437-hardware-design-for-stm32h7-series-stmicroelectronics.pdf"

dl 01_MCU AN2867_Oscillator.pdf \
  "https://www.st.com/resource/en/application_note/an2867-guidelines-for-oscillator-design-on-stm8afals-and-stm32-mcusmpus-stmicroelectronics.pdf"

dl 01_MCU AN4879_USB.pdf \
  "https://www.st.com/resource/en/application_note/an4879-introduction-to-usb-hardware-and-pcb-guidelines-using-stm32-mcus-stmicroelectronics.pdf"

dl 01_MCU UM2407_Nucleo_H743ZI2.pdf \
  "https://www.st.com/resource/en/user_manual/um2407-stm32h7-nucleo144-boards-mb1364-stmicroelectronics.pdf" \
  "https://www.st.com/resource/en/user_manual/dm00499160-stm32h7-nucleo144-boards-mb1364-stmicroelectronics.pdf"

dl 01_MCU Nucleo-H743ZI_SCH.pdf \
  "https://www.st.com/resource/en/schematic_pack/mb1364-h743zi-c01_schematic.pdf"

section "02_ADC_Loadcell"
dl 02_ADC_Loadcell ADS131M04.pdf \
  "https://www.ti.com/lit/ds/symlink/ads131m04.pdf"

dl 02_ADC_Loadcell ADS131M04EVM_UG.pdf \
  "https://www.ti.com/lit/ug/sbau329/sbau329.pdf"

dl 02_ADC_Loadcell REF5025.pdf \
  "https://www.ti.com/lit/ds/symlink/ref5025.pdf"

dl 02_ADC_Loadcell SBAA532_ADS131M_Design.pdf \
  "https://www.ti.com/lit/an/sbaa532/sbaa532.pdf"

section "03_Encoder"
dl 03_Encoder AS5048A.pdf \
  "https://media.digikey.com/pdf/Data%20Sheets/Austriamicrosystems%20PDFs/AS5048A,B.pdf" \
  "https://ams-osram.com/zh-cn/products/sensor-solutions/rotary-position-sensors/ams-as5048a-rotary-position-sensor"

section "04_CAN_Isolation"
dl 04_CAN_Isolation ISO1050.pdf \
  "https://www.ti.com/lit/ds/symlink/iso1050.pdf"

dl 04_CAN_Isolation MGJ2_Series.pdf \
  "https://www.murata.com/products/productdata/8807029997598/kdc-mgj2.pdf" \
  "https://www.ttieurope.com/content/dam/tti-europe/manufacturers/murata/doc/murata-ps-mgj2-isolated-2w-gate-drive-dc-dc-converters-datasheet-specifications.pdf"

dl 04_CAN_Isolation PRTR5V0U2X.pdf \
  "https://assets.nexperia.com/documents/data-sheet/PRTR5V0U2X.pdf"

dl 04_CAN_Isolation BZT52_series.pdf \
  "https://www.diodes.com/assets/Datasheets/ds18001.pdf"

dl 04_CAN_Isolation ACM2012_series.pdf \
  "https://product.tdk.com/info/en/catalog/datasheets/cmf_automotive_signal_acm2012_en.pdf"

dl 04_CAN_Isolation SM712.pdf \
  "https://www.mouser.com/datasheet/2/761/SEMT_S_A0003609968_1-2576005.pdf" \
  "https://datasheet.lcsc.com/lcsc/1808032011_SEMTECH-SM712-TCT_C12067.pdf"

section "05_USB_Debug"
dl 05_USB_Debug TPD4S012.pdf \
  "https://www.ti.com/lit/ds/symlink/tpd4s012.pdf"

dl 05_USB_Debug USB4105_USB-C.pdf \
  "https://gct.co/files/drawings/usb4105.pdf"

section "06_Battery_Monitor"
dl 06_Battery_Monitor INA228.pdf \
  "https://www.ti.com/lit/ds/symlink/ina228.pdf"

section "07_Logic_Power"
dl 07_Logic_Power TPS62933.pdf \
  "https://www.ti.com/lit/ds/symlink/tps62933.pdf"

dl 07_Logic_Power AP63205_family.pdf \
  "https://www.diodes.com/datasheet/download/AP63200-AP63201-AP63203-AP63205.pdf" \
  "https://www.diodes.com/assets/Datasheets/AP63200-AP63201-AP63203-AP63205.pdf" \
  "https://www.lcsc.com/datasheet/lcsc_datasheet_2412251026_Diodes-Incorporated-AP63205WU-7_C2071056.pdf"

dl 07_Logic_Power TLV75533.pdf \
  "https://www.ti.com/lit/ds/symlink/tlv75533.pdf"

dl 07_Logic_Power XAL4040.pdf \
  "https://www.coilcraft.com/getmedia/fb72e61d-5aee-47bb-b1ba-5f76a23b1d5d/xal4040.pdf"

section "08_Protection_Reverse"
dl 08_Protection_Reverse DMP2305U.pdf \
  "https://www.diodes.com/assets/Datasheets/DMP2305U.pdf" \
  "https://www.lcsc.com/datasheet/lcsc_datasheet_2304140030_Diodes-Incorporated-DMP2305U-7_C85097.pdf"

section "09_Protection_Inrush"
dl 09_Protection_Inrush SL08_series.pdf \
  "https://www.ametherm.com/hubfs/docs/data-sheets/sl08-datasheet.pdf"

dl 09_Protection_Inrush Omron_G5V-1.pdf \
  "https://omronfs.omron.com/en_US/ecb/products/pdf/en-g5v_1.pdf" \
  "https://docs.rs-online.com/63df/0900766b814b2cde.pdf"

section "10_Protection_Bus"
dl 10_Protection_Bus P6KE_series.pdf \
  "https://www.vishay.com/docs/88369/p6ke.pdf" \
  "https://www.littelfuse.com/assetdocs/tvs-diodes-p6ke-datasheet"

dl 10_Protection_Bus Rubycon_ZLH.pdf \
  "https://www.rubycon.co.jp/wp-content/uploads/catalog-aluminum/ZLH.pdf"

section "11_Protection_Local"
dl 11_Protection_Local SMAJ_series.pdf \
  "https://www.vishay.com/docs/88428/smaj.pdf" \
  "https://www.bourns.com/docs/Product-Datasheets/SMAJ.pdf"

dl 11_Protection_Local OS-CON_SVPD.pdf \
  "https://media.digikey.com/pdf/Data%20Sheets/Panasonic%20Capacitors%20PDFs/SVPD_Series.pdf" \
  "https://industrial.panasonic.com/cdbs/www-data/pdf/AAB8000/AAB8000C181.pdf"

section "12_Protection_Brake"
dl 12_Protection_Brake TLV3201.pdf \
  "https://www.ti.com/lit/ds/symlink/tlv3201.pdf"

dl 12_Protection_Brake IPB025N10N3G.pdf \
  "https://www.infineon.com/dgdl/Infineon-IPB025N10N3G-DS-v02_03-en.pdf?fileId=db3a30431ce5fb52011d1ab1d9d51349"

section "13_Safety_UI"
dl 13_Safety_UI 74LVC1G08.pdf \
  "https://assets.nexperia.com/documents/data-sheet/74LVC1G08.pdf"

dl 13_Safety_UI 74LVC1G373.pdf \
  "https://assets.nexperia.com/documents/data-sheet/74LVC1G373.pdf"

section "14_Storage"
dl 14_Storage DM3AT_microSD.pdf \
  "https://www.lcsc.com/datasheet/lcsc_datasheet_1810301220_HRS-Hirose-DM3AT-SF-PEJM5_C114218.pdf"

dl 14_Storage AN5200_SDMMC.pdf \
  "https://www.st.com/resource/en/application_note/an5200-getting-started-with-the-stm32h7-mcu-sdmmc-host-controller-stmicroelectronics.pdf" \
  "https://www.st.com/resource/en/application_note/dm00525510-getting-started-with-stm32h7-series-sdmmc-host-controller-stmicroelectronics.pdf"

section "15_Wireless"
dl 15_Wireless ESP32-C3-MINI-1U.pdf \
  "https://www.mouser.com/datasheet/2/891/esp32_c3_mini_1_datasheet_en-2578671.pdf" \
  "https://documentation.espressif.com/esp32-c3-mini-1_datasheet_en.pdf" \
  "https://www.espressif.com/sites/default/files/documentation/esp32-c3-mini-1_datasheet_en.pdf"

dl 15_Wireless ESP32-C3_HW_Design.pdf \
  "https://www.espressif.com/sites/default/files/documentation/esp32-c3_hardware_design_guidelines_en.pdf"

section "16_Passives_Grounding"
dl 16_Passives_Grounding Murata_BLM_ferrites.pdf \
  "https://www.murata.com/products/productdata/8796738650142/ENFA0003.pdf"

section "17_System_Modules"
dl 17_System_Modules Jetson_Orin_NX.pdf \
  "https://www.mouser.com/datasheet/2/744/Seeed_Studio_06132023_102110781-3216398.pdf"

dl 17_System_Modules Jetson_Orin_NX_DesignGuide.pdf \
  "https://www.mouser.com/pdfDocs/Jetson_Orin_NX_Series_and_Orin_Nano_Series_Design_Guide_DG-10931-001_v11.pdf"

# Manual-fetch items (login/consent walls, skip):
echo
echo "### Manual fetch required (login/consent walls)"
echo "  Stereolabs ZED X Mini:  https://www.stereolabs.com/zed-x-mini"
echo "  EBIMU EBMotion V5:      https://www.e2box.co.kr/product/EBIMU-9DOFV5.asp"
echo "  (Save manually to 17_System_Modules/)"

################################################################
echo
echo "==== Summary ===="
echo "  OK:    $OK"
echo "  Skip:  $SKIP"
echo "  Fail:  ${#FAILED[@]}"
if [[ ${#FAILED[@]} -gt 0 ]]; then
  echo
  echo "==== Failed files (fetch manually) ===="
  printf '  %s\n' "${FAILED[@]}"
fi

if [[ $LIST_ONLY -eq 0 ]]; then
  echo
  echo "PDFs saved under: $HERE/"
fi
