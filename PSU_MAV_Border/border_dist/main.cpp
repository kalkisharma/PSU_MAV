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

//
class Distance{
private:
  double theta = 53;        // Camera half-angle in x-direction (640 pixels)
  double phi = 40;          // Camera half-angle in y-direction (480 pixels)
  double xMax = 640.0;      // Maximum x-axis grid location
  double yMax = 480.0;      // Maximum y-axis grid location
public:
  //Distance() : {}
  /* Compute shortest distance to border given lengths of the triangle */
  double get_dist(double m, double b, double h){
    double x1, x2, x3, y1, y2, y3, len_a, len_b, len_c, area, s, r_pix, r_m;

    x1 = 0.0;
    x2 = xMax/2.0;
    x3 = x2;
    y2 = yMax/2.0;

    y1 = get_y(x1, m, b);
    y3 = get_y(x3, m, b);

    len_a = get_length(x1, y1, x2, y2);
    len_b = get_length(x1, y1, x3, y3);
    len_c = get_length(x2, y2, x3, y3);

    s = 0.5*(len_a + len_b + len_c);
    area = sqrt(s*(s - len_a)*(s - len_b)*(s - len_c));
    r_pix = (2*area/len_b);
    r_m = r_pix * 2 * h / (sin(theta*3.14/180.0) * 640.0);
    return r_m;
  }
  /* Compute y distance on border corresponding to an x location on the grid */
  double get_y(double x, double m, double b){
    double y;

    y = m*x + b;

    return (y);
  }
  /* Compute length of side of triangle given two grid points */
  double get_length(double x1, double y1, double x2, double y2){

    return sqrt(pow((x1-x2),2.0) + pow((y1-y2),2.0));
  }
};
//------------------------------------------------------------------------------
int main(){
  double slope;           // Slope of border line
  double y_intercept;      // Y intercept of border line
  double alt;             // MAV altitude
  double r;

  slope = 15.0;
  y_intercept = 220.0;
  alt = 1.0;

  Distance dist;

  /* Compute shortest distance of quad from border */
  r = dist.get_dist(slope, y_intercept, alt);
  cout << "Shortest distance to border: " << r << "m" << endl;
}
//------------------------------------------------------------------------------
