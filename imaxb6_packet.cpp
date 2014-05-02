#include "imaxb6_packet.h"

#include <unistd.h>
#include <string.h>
#include <stdio.h>
//
// Packet addresses
//

// Open
#define IMAXB6_PACKET_POS_OPEN                    0
// Data segment
#define IMAXB6_PACKET_POS_DATA_START              1
#define IMAXB6_PACKET_POS_DATA_END                72
#define IMAXB6_PACKET_DATA_LENGTH                 (IMAXB6_PACKET_POS_DATA_END-IMAXB6_PACKET_POS_DATA_START+1)
// Checksum
#define IMAXB6_PACKET_POS_CHECKSUM_VALUE_HIGH     73
#define IMAXB6_PACKET_POS_CHECKSUM_VALUE_LOW      74
// Close
#define IMAXB6_PACKET_POS_CLOSE                   75

//
// Data segment addresses
//

// Model
#define IMAXB6_DATA_POS_MODEL_START             0
#define IMAXB6_DATA_POS_MODEL_END               6
#define IMAXB6_DATA_POS_MODEL_LENGTH            (IMAXB6_DATA_POS_MODEL_END-IMAXB6_DATA_POS_MODEL_START+1)
#define IMAXB6_DATA_MODEL_B6AC                  {0x1a, 0x04, 0x04, 0x2d, 0x0a, 0x00, 0x64}
#define IMAXB6_DATA_MODEL_QUATTRO_B6AC          {0x1e, 0x04, 0x04, 0x50, 0x05, 0x00, 0x6e}
// running state: bit 0×01 is set when running, cleared when standby
#define IMAXB6_DATA_POS_RUNNING_STATE           23
// Mode
#define IMAXB6_DATA_POS_MODE                    22
#define IMAXB6_DATA_MODE_CONFIG                 0x80
#define IMAXB6_DATA_MODE_LI                     0x81
#define IMAXB6_DATA_MODE_NIMH                   0x82
#define IMAXB6_DATA_MODE_NICD                   0x83
#define IMAXB6_DATA_MODE_PB                     0x84
#define IMAXB6_DATA_MODE_SAVE                   0x85
#define IMAXB6_DATA_MODE_LOAD                   0x86
// bit 0×01 is set when charging, clear when discharging
// bit 0×10 is set when cycling, clear when single charging or discharging
#define IMAXB6_DATA_POS_STATE                   7
// bit 0×01 contains the cycle mode, set for {Charge,Discharge}, clear for {Discharge,Charge}
#define IMAXB6_DATA_POS_CYCLE_MODE              14
// cycle count
#define IMAXB6_DATA_POS_CYCLE_COUNT             15
// current in A and cA
#define IMAXB6_DATA_POS_CURRENT_A               32
#define IMAXB6_DATA_POS_CURRENT_CA              33
// voltage in V and cV
#define IMAXB6_DATA_POS_VOLTAGE_V               34
#define IMAXB6_DATA_POS_VOLTAGE_CV              35
// input voltage in V and cV
#define IMAXB6_DATA_POS_INPUT_VOLTAGE_V         40
#define IMAXB6_DATA_POS_INPUT_VOLTAGE_CV        41
// charge in dAh and mAh
#define IMAXB6_DATA_POS_CHARGE_DAH              42
#define IMAXB6_DATA_POS_CHARGE_MAH              43
// time in minutes
#define IMAXB6_DATA_POS_TIME                    69

// Pb charge current in dA
#define IMAXB6_DATA_POS_PB_CHARGE_CURRENT       20
// Pb cell count
#define IMAXB6_DATA_POS_PB_CELL_COUNT           21

// NiCd current in dA
#define IMAXB6_DATA_POS_NICD_CHARGE_CURRENT     8
#define IMAXB6_DATA_POS_NICD_DISCHARGE_CURRENT  9
// NiCd discharge voltage in daV and cV
#define IMAXB6_DATA_POS_NICD_DISCHARGE_DAVOL    26
#define IMAXB6_DATA_POS_NICD_DISCHARGE_CVOL     27

// NiMH current in dA
#define IMAXB6_DATA_POS_NIMH_CHARGE_CURRENT     12
#define IMAXB6_DATA_POS_NIMH_DISCHARGE_CURRENT  13
// NiMH discharge voltage in daV and cV
#define IMAXB6_DATA_POS_NIMH_DISCHARGE_DAVOL    24
#define IMAXB6_DATA_POS_NIMH_DISCHARGE_CVOL     25

// Li__ current in dA
#define IMAXB6_DATA_POS_LI_CHARGE_CURRENT       16
// Li__ charge cell count
#define IMAXB6_DATA_POS_LI_CHARGE_CELL_COUNT    17
// Li__ discharge current in dA
#define IMAXB6_DATA_POS_LI_DISCHARGE_CURRENT    18
// Li__ discharge cell count
#define IMAXB6_DATA_POS_LI_DISCHARGE_CELL_COUNT 19
// Li__ cell voltages
#define IMAXB6_DATA_POS_LI_CELL1_V              44
#define IMAXB6_DATA_POS_LI_CELL1_CV             45
#define IMAXB6_DATA_POS_LI_CELL2_V              46
#define IMAXB6_DATA_POS_LI_CELL2_CV             47
#define IMAXB6_DATA_POS_LI_CELL3_V              48
#define IMAXB6_DATA_POS_LI_CELL3_CV             49
#define IMAXB6_DATA_POS_LI_CELL4_V              50
#define IMAXB6_DATA_POS_LI_CELL4_CV             51
#define IMAXB6_DATA_POS_LI_CELL5_V              52
#define IMAXB6_DATA_POS_LI_CELL5_CV             53
#define IMAXB6_DATA_POS_LI_CELL6_V              54
#define IMAXB6_DATA_POS_LI_CELL6_CV             55

// Values that should be fixed
#define IMAXB6_DATA_POS_10 0x80
#define IMAXB6_DATA_POS_11 0x81
#define IMAXB6_DATA_POS_28 0x80
#define IMAXB6_DATA_POS_29 0x8c
#define IMAXB6_DATA_POS_30 0xB2
#define IMAXB6_DATA_POS_31 0x80
#define IMAXB6_DATA_POS_36 0x80
#define IMAXB6_DATA_POS_37 0x80
#define IMAXB6_DATA_POS_38 0x80
#define IMAXB6_DATA_POS_39 0x80
#define IMAXB6_DATA_POS_56 0x80
#define IMAXB6_DATA_POS_57 0x80
#define IMAXB6_DATA_POS_58 0x80
#define IMAXB6_DATA_POS_59 0x80
#define IMAXB6_DATA_POS_60 0x80
#define IMAXB6_DATA_POS_61 0x80
#define IMAXB6_DATA_POS_62 0x80
#define IMAXB6_DATA_POS_63 0x80
#define IMAXB6_DATA_POS_64 0x80
#define IMAXB6_DATA_POS_65 0x80
#define IMAXB6_DATA_POS_66 0x80
#define IMAXB6_DATA_POS_67 0x80
#define IMAXB6_DATA_POS_68 0x80
#define IMAXB6_DATA_POS_70 0x8F
#define IMAXB6_DATA_POS_71 0xA4

imaxb6_packet::imaxb6_packet(const uint8_t *new_raw_packet){
  // import data
  ssize_t pos;
  memcpy(raw_packet, new_raw_packet, IMAXB6_PACKET_LENGTH);
  raw_data = &raw_packet[IMAXB6_PACKET_POS_DATA_START];
  // validation
  basic_validation();
  // other validations
  model();
  mode();
  // Fixed byets
  assert_fixed_bytes();
}

void imaxb6_packet::basic_validation(){
  ssize_t pos;
  // open curly brace
  if(raw_packet[IMAXB6_PACKET_POS_OPEN] != '{')
    throw "Open curly brace not found.";
  // All data must have highest bit set to 1
  for(pos=0 ; pos<IMAXB6_PACKET_DATA_LENGTH ; pos++){
    if( (raw_data[pos] & 0x80) != 0x80 )
      throw "Byte in data segment without high bit set to 1.";
  }
  // Checksum
  if(( raw_packet[IMAXB6_PACKET_POS_CHECKSUM_VALUE_HIGH] & 0xF0) != 0x30)
   throw "Invalid checksum high byte.";
  if(( raw_packet[IMAXB6_PACKET_POS_CHECKSUM_VALUE_LOW] & 0xF0) != 0x30)
   throw "Invalid checksum low byte.";
  if(calculated_checksum() != packet_checksum())
    throw "Checksum mismatch.";
  // close curly brace
  if(raw_packet[IMAXB6_PACKET_POS_CLOSE] != '}')
    throw "Close curly brace not found.";
}

void imaxb6_packet::assert_fixed_bytes() {
  // Fixed bytes
  if(raw_data[10] != IMAXB6_DATA_POS_10)
    throw "Unknown data at 10.";
  if(raw_data[11] != IMAXB6_DATA_POS_11)
    throw "Unknown data at 11.";
  if(raw_data[28] != IMAXB6_DATA_POS_28)
    throw "Unknown data at 28.";
  if(raw_data[29] != IMAXB6_DATA_POS_29)
    throw "Unknown data at 29.";
  if(raw_data[30] != IMAXB6_DATA_POS_30)
    throw "Unknown data at 30.";
  if(raw_data[31] != IMAXB6_DATA_POS_31)
    throw "Unknown data at 31.";
  if(raw_data[36] != IMAXB6_DATA_POS_36)
    throw "Unknown data at 36.";
  if(raw_data[37] != IMAXB6_DATA_POS_37)
    throw "Unknown data at 37.";
  if(raw_data[38] != IMAXB6_DATA_POS_38)
    throw "Unknown data at 38.";
  if(raw_data[39] != IMAXB6_DATA_POS_39)
    throw "Unknown data at 39.";
  if(raw_data[56] != IMAXB6_DATA_POS_56)
    throw "Unknown data at 56.";
  if(raw_data[57] != IMAXB6_DATA_POS_57)
    throw "Unknown data at 57.";
  if(raw_data[58] != IMAXB6_DATA_POS_58)
    throw "Unknown data at 58.";
  if(raw_data[59] != IMAXB6_DATA_POS_59)
    throw "Unknown data at 59.";
  if(raw_data[60] != IMAXB6_DATA_POS_60)
    throw "Unknown data at 60.";
  if(raw_data[61] != IMAXB6_DATA_POS_61)
    throw "Unknown data at 61.";
  if(raw_data[62] != IMAXB6_DATA_POS_62)
    throw "Unknown data at 62.";
  if(raw_data[63] != IMAXB6_DATA_POS_63)
    throw "Unknown data at 63.";
  if(raw_data[64] != IMAXB6_DATA_POS_64)
    throw "Unknown data at 64.";
  if(raw_data[65] != IMAXB6_DATA_POS_65)
    throw "Unknown data at 65.";
  if(raw_data[66] != IMAXB6_DATA_POS_66)
    throw "Unknown data at 66.";
  if(raw_data[67] != IMAXB6_DATA_POS_67)
    throw "Unknown data at 67.";
  if(raw_data[68] != IMAXB6_DATA_POS_68)
    throw "Unknown data at 68.";
  if(raw_data[70] != IMAXB6_DATA_POS_70)
    throw "Unknown data at 70.";
  if(raw_data[71] != IMAXB6_DATA_POS_71)
    throw "Unknown data at 71.";
}

const char *imaxb6_packet::model() {
  const void *packet_model = (const void *) &raw_data[IMAXB6_DATA_POS_MODEL_START];
  const uint8_t model_b6ac[] = IMAXB6_DATA_MODEL_B6AC;
  const uint8_t model_quattrob6ac[] = IMAXB6_DATA_MODEL_QUATTRO_B6AC;
  if(memcmp(packet_model, model_b6ac, IMAXB6_DATA_POS_MODEL_LENGTH))
    return "B6AC";
  if(memcmp(packet_model, model_quattrob6ac, IMAXB6_DATA_POS_MODEL_LENGTH))
    return "Quattro B6AC";
  throw "Unknown model.";
}

uint8_t imaxb6_packet::calculated_checksum() {
  uint8_t checksum=0;
  ssize_t pos;
  for(pos=IMAXB6_PACKET_POS_DATA_START ; pos<=IMAXB6_PACKET_POS_DATA_END ; pos++){
    checksum += raw_packet[pos];
  }
  return checksum;
}

uint8_t imaxb6_packet::packet_checksum() {
  uint8_t checksum_high;
  uint8_t checksum_low;
  checksum_high = ( raw_packet[IMAXB6_PACKET_POS_CHECKSUM_VALUE_HIGH] & 0x0F) << 4;
  checksum_low = raw_packet[IMAXB6_PACKET_POS_CHECKSUM_VALUE_LOW] & 0x0F;
  return checksum_high | checksum_low;
}

uint8_t imaxb6_packet::standby() {
  return !(raw_data[IMAXB6_DATA_POS_RUNNING_STATE] & 0x01);
}

const char * imaxb6_packet::mode() {
  switch(raw_data[IMAXB6_DATA_POS_MODE]){
    case IMAXB6_DATA_MODE_CONFIG:
      return "Config";
    case IMAXB6_DATA_MODE_LI:
      return "LiPo / LiIo / LiFe";
    case IMAXB6_DATA_MODE_NIMH:
      return "NiMH";
    case IMAXB6_DATA_MODE_NICD:
      return "NiCd";
    case IMAXB6_DATA_MODE_PB:
      return "Pb";
    case IMAXB6_DATA_MODE_SAVE:
      return "Save";
    case IMAXB6_DATA_MODE_LOAD:
      return "Load";
    default:
      throw "Unknown Mode";
  }
}

uint8_t imaxb6_packet::charging() {
  return raw_data[IMAXB6_DATA_POS_STATE] & 0x01;
}

uint8_t imaxb6_packet::cycling() {
  return raw_data[IMAXB6_DATA_POS_STATE] & 0x10;
}

uint8_t imaxb6_packet::cycle_mode() {
  return raw_data[IMAXB6_DATA_POS_CYCLE_MODE] & 0x01;
}

uint8_t imaxb6_packet::cycle_count() {
  return raw_data[IMAXB6_DATA_POS_CYCLE_COUNT] & 0x7f;
}

double imaxb6_packet::read_double(ssize_t pos) {
  return (double)(raw_data[pos] & 0x7f) + (double)(raw_data[pos+1] & 0x7f)/100.0;
}

double imaxb6_packet::input_voltage() {
  return read_double(IMAXB6_DATA_POS_INPUT_VOLTAGE_V);
}

double imaxb6_packet::battery_voltage() {
  return read_double(IMAXB6_DATA_POS_VOLTAGE_V); 
}

double imaxb6_packet::battery_current() {
  return read_double(IMAXB6_DATA_POS_CURRENT_A); 
}

double imaxb6_packet::battery_charge() {
  return read_double(IMAXB6_DATA_POS_CHARGE_DAH)*100.0;
}

uint8_t imaxb6_packet::minutes() {
  return raw_data[IMAXB6_DATA_POS_TIME] & 0x7F;
}

// Li

uint8_t imaxb6_packet::li_charge_cell_count() {
  return raw_data[IMAXB6_DATA_POS_LI_CHARGE_CELL_COUNT] & 0x7F;
}

double imaxb6_packet::li_charge_current() {
  return (double)(raw_data[IMAXB6_DATA_POS_LI_CHARGE_CURRENT] & 0x7F)/10.0;
}

double imaxb6_packet::li_discharge_current() {
  return (double)(raw_data[IMAXB6_DATA_POS_LI_DISCHARGE_CURRENT] & 0x7F)/10.0;
}

uint8_t imaxb6_packet::li_discharge_cell_count() {
  return raw_data[IMAXB6_DATA_POS_LI_DISCHARGE_CELL_COUNT] & 0x7F;
}

double * imaxb6_packet::li_cell_voltages() {
  uint8_t cell;
  for(cell=0 ; cell < IMAXB6_LI_MAX_CELLS ; cell++)
    voltages[cell] = read_double(IMAXB6_DATA_POS_LI_CELL1_V+cell*2);
  return voltages;
}

// Pb

double imaxb6_packet::pb_charge_current(){
  return (double)(raw_data[IMAXB6_DATA_POS_PB_CHARGE_CURRENT] & 0x7F)/10.0;
}

uint8_t imaxb6_packet::pb_cell_count(){
  return raw_data[IMAXB6_DATA_POS_PB_CELL_COUNT] & 0x7F;
}

// NiCd

double imaxb6_packet::nicd_charge_current(){
  return (double)( raw_data[IMAXB6_DATA_POS_NICD_CHARGE_CURRENT] & 0x7F ) /10.0;
}

double imaxb6_packet::nicd_discharge_current(){
  return (double)( raw_data[IMAXB6_DATA_POS_NICD_DISCHARGE_CURRENT] & 0x7F ) /10.0;
}

double imaxb6_packet::nicd_discharge_voltage(){
  double v0, v1;
  v0 = (double)(raw_data[IMAXB6_DATA_POS_NICD_DISCHARGE_DAVOL] & 0x7F);
  v1 = (double)(raw_data[IMAXB6_DATA_POS_NICD_DISCHARGE_CVOL] & 0x7F);
  return v0*10.0 + v1/10.0;
}

// NiMH

double imaxb6_packet::nimh_charge_current(){
  return (double)( raw_data[IMAXB6_DATA_POS_NIMH_CHARGE_CURRENT] & 0x7F ) /10.0;
}

double imaxb6_packet::nimh_discharge_current(){
  return (double)( raw_data[IMAXB6_DATA_POS_NIMH_DISCHARGE_CURRENT] & 0x7F ) /10.0;
}

double imaxb6_packet::nimh_discharge_voltage(){
  double v0, v1;
  v0 = (double)(raw_data[IMAXB6_DATA_POS_NIMH_DISCHARGE_DAVOL] & 0x7F);
  v1 = (double)(raw_data[IMAXB6_DATA_POS_NIMH_DISCHARGE_CVOL] & 0x7F);
  return v0*10.0 + v1/10.0;
}
