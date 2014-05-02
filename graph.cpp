#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <inttypes.h>

class graph {
  private:
    double *new_values, max_values;
    uint8_t received_values;
    double interpolate(double x, double x0, double y0, double x1, double y1);
  public:
    graph(uint8_t new_width);
    void push(double new_value);
    void print();
    double *values;
};

graph::graph(uint8_t new_max_values){
  max_values = (double) new_max_values;
  if(NULL==(values=(double *)calloc((ssize_t)max_values, sizeof(double))))
    throw "Out of memory.";
  if(NULL==(new_values=(double *)calloc((ssize_t)max_values, sizeof(double))))
    throw "Out of memory.";
  received_values = 0;
}

double graph::interpolate(double x, double x0, double y0, double x1, double y1){
  return y0 + (y1 - y0) * ( (x - x0) / (x1 - x0) );
}

void graph::push(double new_value){
  if(received_values<max_values){
    values[received_values] = new_value;
    received_values++;
  } else {
    uint8_t pos;
    new_values[0] = values[0];
    for(pos=1;pos<(uint8_t)max_values-1;pos++){
      double x, x0, y0, x1, y1;
      x = (double)pos/(max_values-1);
      x0 = (double)pos/max_values;
      y0 = (double)values[pos];
      if(pos+1 == (uint8_t)max_values-1){
        x1 = 1.0;
        y1 = new_value;
      }else{
        x1 = (double)(pos+1)/max_values;
        y1 = (double)values[pos+1];
      }
      new_values[pos] = interpolate(x, x0, y0, x1, y1);
    }
    new_values[(ssize_t)max_values-1] = new_value;
    double *tmp;
    tmp = values;
    values = new_values;
    new_values = tmp;
  }

}

void graph::print() {
int c;
for(c=0;c<(uint8_t)max_values&&c<received_values;c++) {
  printf("%.2lf ", values[c]);
}
printf("\n");
}

int
main() {
  double x;
  graph g(20);
  for(x=0;x<=20;x+=0.6){
//puts("BEGIN==============================");
//g.print();
    g.push(sin(x));
g.print();
//puts("END==============================");
  }
}
