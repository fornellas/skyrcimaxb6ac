#ifndef imaxb6_packet_h
#define imaxb6_packet_h

#include <inttypes.h>
#include <unistd.h>

#define IMAXB6_PACKET_LENGTH 76
#define IMAXB6_LI_MAX_CELLS 6

class imaxb6_packet {
  private:
    // whole packet
    uint8_t raw_packet[IMAXB6_PACKET_LENGTH];
    // data segment
    uint8_t *raw_data;
    // validate integrity
    void basic_validation();
    void assert_fixed_bytes();
    // checksums
    uint8_t calculated_checksum();
    uint8_t packet_checksum();
    // parse double from data
    double read_double(ssize_t pos);
    // li cell voltages
    double voltages[IMAXB6_LI_MAX_CELLS];
  public:
    imaxb6_packet(const uint8_t *new_raw_packet);
    const char *model();
    uint8_t standby();
    const char *mode();
    uint8_t charging(); // true for charging, false for discharging
    uint8_t cycling();
    uint8_t cycle_mode(); // true for C>D, false for D>C
    uint8_t cycle_count();
    double input_voltage();
    double battery_voltage();
    double battery_current();
    double battery_charge();
    uint16_t minutes();
    // LiPo / LiIo / LiFe
    uint8_t li_charge_cell_count();
    double li_charge_current();
    double li_discharge_current();
    uint8_t li_discharge_cell_count();
    double * li_cell_voltages();
    // Pb
    double pb_charge_current();
    uint8_t pb_cell_count();
    // NiCd
    double nicd_charge_current();
    double nicd_discharge_current();
    double nicd_discharge_voltage();
    // NiMH
    double nimh_charge_current();
    double nimh_discharge_current();
    double nimh_discharge_voltage();
};

#endif
