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
        if(packet.charging())
          printf("Charging\n");
        else
          printf("Discharging\n");
        if(packet.cycling()) {
          printf("Cycling:\n");
          if(packet.cycle_mode())
            printf("  Charge > Discharge\n");
          else
            printf("  Discharge > Charge\n");
          printf("Cycle count: %d\n", packet.cycle_count());
        }
        printf("Battery:\n");
        printf("  Voltage: %.2lfV\n", packet.battery_voltage());
        printf("  Current: %.2lfA\n", packet.battery_current());
        printf("  Charge: %.0lfmAh\n", packet.battery_charge());
        printf("Time: %dmin\n", packet.minutes());
        printf("Mode: %s\n", packet.mode());
        if(!strcmp(packet.mode(), "LiPo / LiIo / LiFe")) {
          int cell, cells;
          if(packet.charging()) {
            printf("  Charge Cells: %d\n", packet.li_charge_cell_count());
            printf("  Charge Current: %.2lfA\n", packet.li_charge_current());
            cells = packet.li_charge_cell_count();
          } else {
            printf("  Discharge Cells: %d\n", packet.li_discharge_cell_count());
            printf("  Discharge Current: %.2lfA\n", packet.li_discharge_current());
            cells = packet.li_discharge_cell_count();
          }
          printf("  Cell voltages:");
          double * voltages = packet.li_cell_voltages();
          for(cell=0 ; cell<cells ; cell++){
            if(cell)
              printf(" ");
            printf("  %.2lfV", voltages[cell]);
          }
          printf("\n");
        }
      }catch(const char *err_msg){
        fprintf(stderr, "Invalid packet: %s\n", err_msg);
      }
      skipped = 0;
    }else{
//      fprintf(stderr, "Waiting for packet start (%ld)...\n", ++skipped);
    }
  }
}
