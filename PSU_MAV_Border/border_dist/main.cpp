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

/* Declare functions */
double get_length(double x1, double y1, double x2, double y2);
double get_y(double x);
double get_dist(double a, double b, double c);

/* Declare constants */
const double xMax = 640.0;      // Maximum x-axis grid location
const double yMax = 480.0;      // Maximum y-axis grid location
const double quad_coord[2] = {xMax/2.0, yMax/2.0};    // Grid location of quad
const double m = 10.0;          // Slope of border line
const double b = 1.0;           // Y intercept of border line
//------------------------------------------------------------------------------
int main(){
  double len_a, len_b, len_c, r;

  /* Compute leg of triangle */
  len_a = get_length(0.0, get_y(0.0), quad_coord[0], quad_coord[1]);
  cout << "Length a: " << len_a << endl;

  /* Compute base of triangle */
  len_b = get_length(quad_coord[0], get_y(quad_coord[0]), 0.0, get_y(0.0));
  cout << "Length b: " << len_b << endl;

  /* Compute leg of triangle */
  len_c = get_length(quad_coord[0], quad_coord[1], quad_coord[0], get_y(quad_coord[0]));
  cout << "Length c: " << len_c << endl;

  /* Compute shortest distance of quad from border */
  r = get_dist(len_a, len_b, len_c);
  cout << "Shortest distance to border: " << r << endl;
}
//------------------------------------------------------------------------------
/* Compute y distance on border cooresponding to an x location on the grid */
double get_y(double x){
  double y;

  y = m*x + b;

  return (y);
}
//------------------------------------------------------------------------------
/* Compute length of side of triangle given two grid points */
double get_length(double x1, double y1, double x2, double y2){

  return sqrt(pow((x1-x2),2.0) + pow((y1-y2),2.0));
}
//------------------------------------------------------------------------------
/* Compute shortest distance to border given lengths of the triangle */
double get_dist(double a, double b, double c){
  double area, s;

  s = 0.5*(a + b + c);
  area = sqrt(s*(s - a)*(s - b)*(s - c));
  return (2*area/b);
}
//------------------------------------------------------------------------------
