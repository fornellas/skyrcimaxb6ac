#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <iostream>
#include <unistd.h>

#include "imaxb6_packet.h"

#define PACKET_LEN 76

int
main() {
  unsigned char buff[PACKET_LEN];
  ssize_t bytes_read;
  ssize_t skipped=0;

  while(1){
    errno = 0;
    bytes_read = read(0, buff, 1);
    if(bytes_read == 0) {
//      fprintf(stderr, "Waiting for packet start.\n");
      continue;
    }
    if(bytes_read < 0) {
      fprintf(stderr, "ERROR before packet start: %s.\n", strerror(errno));
      return 1;
    }

    if(buff[0]=='{'){
//      fprintf(stderr, "Packet start.\n");

      ssize_t bytes_to_read = PACKET_LEN-1;
      while(bytes_to_read) {
        bytes_read = read(0, &buff[PACKET_LEN - bytes_to_read], bytes_to_read);
        if(bytes_read < 0) {
          fprintf(stderr, "ERROR reading packet: %s.\n", strerror(errno));
          return 1;
        }
        bytes_to_read -= bytes_read;
//        fprintf(stderr, "Read %ld/%d of pakcet.\n", PACKET_LEN - bytes_to_read, PACKET_LEN);
      }

      int i;
/*
      for(i=0;i<PACKET_LEN;i++){
        if(i)
          printf(" ");
        printf("%.2d", i);
      }
      printf("\n");
*/
      for(i=0;i<PACKET_LEN;i++){
        if(i)
          printf(" ");
        printf("%.2X", buff[i]);
      }
      printf("\n");

      try{
        imaxb6_packet packet = imaxb6_packet(buff);
//        fprintf(stderr, "Packet OK.\n");
        printf("Model: %s\n", packet.model());
        printf("Input Voltage: %.2lfV\n", packet.input_voltage());
        if(packet.standby()) {
          printf("Standby.\n");
          continue;
        }
        if(packet.cycling()) {
          printf("Cycling:\n");
          if(packet.cycle_mode())
            printf("  Charge > Discharge\n");
          else
            printf("  Discharge > Charge\n");
          printf("Cycle count: %d\n", packet.cycle_count());
        }
        if(packet.charging())
          printf("Charging\n");
        else
          printf("Discharging\n");
        int cells;
        double voltage, current, power;
        printf("Mode: %s\n", packet.mode());
        if(!strcmp(packet.mode(), "LiPo / LiIo / LiFe")) {
          int cell;
          if(packet.charging()) {
            printf("Charge Current: %.2lfA\n", packet.li_charge_current());
            cells = packet.li_charge_cell_count();
          } else {
            printf("Discharge Current: %.2lfA\n", packet.li_discharge_current());
            cells = packet.li_discharge_cell_count();
          }
          printf("Cells: %d\n", cells);
        }
        if(!strcmp(packet.mode(), "Pb")) {
          if(packet.charging()) {
            printf("Pb Current: %.2lfA\n", packet.pb_charge_current());
          }
          printf("Cells: %d\n", packet.pb_cell_count());
        }
        if(!strcmp(packet.mode(), "NiMH")) {
          if(packet.charging()) {
            printf("Charge Current: %.2lfA\n", packet.nimh_charge_current());
          }else{
            printf("Discharge Current: %.2lfA\n", packet.nimh_discharge_current());
            voltage = packet.nimh_discharge_voltage();
            if(voltage != 0.0)
              printf("Discharge Voltage: %.2lfV\n", voltage);
            else
              printf("Discharge Voltage: Auto\n");
          }
        }
        if(!strcmp(packet.mode(), "NiCd")) {
          if(packet.charging()) {
            printf("Charge Current: %.2lfA\n", packet.nicd_charge_current());
          }else{
            printf("Discharge Current: %.2lfA\n", packet.nicd_discharge_current());
            voltage = packet.nicd_discharge_voltage();
            printf("Discharge Voltage: %.2lfV\n", voltage);
          }
        }
        printf("Readings:\n");

        voltage = packet.battery_voltage();
        current = packet.battery_current();
        power = voltage*current;
        printf("  Voltage: %.2lfV\n", voltage);
        printf("  Current: %.2lfA\n", current);
        printf("  Power: %.2lfW\n", voltage*current);
        printf("  Energy: %.0lfmAh\n", packet.battery_charge());
        if(!strcmp(packet.mode(), "LiPo / LiIo / LiFe")) {
          int cell;
          printf("  Cell voltages:");
          double * voltages = packet.li_cell_voltages();
          for(cell=0 ; cell<cells ; cell++){
            if(cell)
              printf(" ");
            printf("  %.2lfV", voltages[cell]);
          }
          printf("\n");
        }
        printf("Time: %dmin\n", packet.minutes());
      }catch(const char *err_msg){
        fprintf(stderr, "Invalid packet: %s\n", err_msg);
      }
      skipped = 0;
    }else{
//      fprintf(stderr, "Waiting for packet start (%ld)...\n", ++skipped);
    }
  }
}
