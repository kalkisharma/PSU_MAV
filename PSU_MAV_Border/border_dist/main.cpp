/*
 * border_dist
 * Inputs:
 *  m - slope of line
 *  b - y intercept of line
 *  xMax - grid points along x-axis
 *  yMax - grid points along y-axis
 */

#include <iostream>
#include <math.h>
using std::endl;
using std::cout;

double get_length(double x1, double y1, double x2, double y2);
double get_y(double x);
double get_dist(double a, double b, double c);

const double xMax = 640.0;
const double yMax = 480.0;
const double quad_coord[2] = {xMax/2.0, yMax/2.0};
const double m = 10.0;
const double b = 1.0;
//------------------------------------------------------------------------------
int main(){
  double len_a, len_b, len_c, r;

  len_a = get_length(0.0, get_y(0.0), quad_coord[0], quad_coord[1]);
  cout << len_a << endl;

  len_b = get_length(quad_coord[0], get_y(quad_coord[0]), 0.0, get_y(0.0));
  cout << len_b << endl;

  len_c = get_length(quad_coord[0], quad_coord[1], quad_coord[0], get_y(quad_coord[0]));
  cout << len_c << endl;

  r = get_dist(len_a, len_b, len_c);
  cout << r << endl;
}
//------------------------------------------------------------------------------
double get_y(double x){
  double y;

  y = m*x + b;

  return (y);
}
//------------------------------------------------------------------------------
double get_length(double x1, double y1, double x2, double y2){

  return sqrt(pow((x1-x2),2.0) + pow((y1-y2),2.0));
}
//------------------------------------------------------------------------------
double get_dist(double a, double b, double c){
  double area, s;

  s = 0.5*(a + b + c);
  area = sqrt(s*(s - a)*(s - b)*(s - c));
  return (2*area/b);
}
//------------------------------------------------------------------------------
