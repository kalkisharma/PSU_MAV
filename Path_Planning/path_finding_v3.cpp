/*
Author:  Andrew Miller
Updated: 9/30/17
Summary: Prints the field and the robot's map.  Then the robot moves around randomly, mapping where it lands until it steps out of bounds.
The robot map is printed as well as how many steps the robot took.
Comments: 
Lines 99-110 is the code that makes the robot step randomly.  I'm not sure how to put that into a function so I'm leaving it in main for now.
I attempted to create some kind of loop that sends another robot once the current roboot stops but I haven't been successful.

2) Inside main should only be variable declarations (e.g. int x, y). Make everything else functions.

*/

#include <iostream>
#include <vector>
#include <cmath>
#include <ctime>
#include <random>
#include <cstdlib>

using namespace std;
const int rows = 9;
const int columns = 9;

void print_field(int grid[][columns]);								// prints the 2 by 2 array
void print_sequence(int grid[]);									// prints the 1d array
void path(int start_x, int start_y, int end_x, int end_y);			// finds the shortest path from one point to another

int main()
{
	int i, j;
	int x_start, y_start, x_end, y_end;
	int x_bomb, y_bomb;
	//int n;	//number of bombs
	bool userInput = false;

	cout << "number of rows: " << rows << std::endl;
	cout << "number of columns: " << columns << std::endl;

	int field[rows][columns];
	int robot_map[rows][columns];

	if (userInput == true)
	{
		cout << "\nx coordinate of starting location: ";
		cin >> x_start;
		cout << "y coordinate of starting location: ";
		cin >> y_start;
		cout << "x coordinate of ending location: ";
		cin >> x_end;
		cout << "y coordinate of ending location: ";
		cin >> y_end;
		cout << "x coordinate of bomb: ";
		cin >> x_bomb;
		cout << "y coordinate of bomb: ";
		cin >> y_bomb;
	}
	else if (userInput == false)						// for now the initial conditions are as follows
	{
		x_start = 4;									// these locations are accounting for the boarder of 0's, +1 for each value
		y_start = 1;
		x_end = 5;
		y_end = 7;
	}

	for (i = 0; i < rows; i++)							// set all the values in the field to a 0 and the boarders to a 4
	{
		for (j = 0; j < columns; j++)
		{
			field[i][j] = 0;
			if (i == 0 || i == 8 || j == 0 || j == 8)
			{
				field[i][j] = 4;
			}
		}
	}

	for (i = 0; i < rows; i++)							// set all the values in the array to a 0
	{
		for (j = 0; j < columns; j++)
		{
			robot_map[i][j] = 7;
		}
	}
	
	


	field[x_start][y_start] = 1;						// starting location marked with a 1
	field[x_end][y_end] = 2;							// end location marked with a 2

	print_field(field);									// calls the print grid function
	print_field(robot_map);								// prints the robots' map
	int x_step, y_step, dx, dy, steps = 0, iteration = 0;

	// Now lets make the robot move around randomly until it reaches the goal or it hits a wall
	srand((unsigned)time(0));
	int f;
	
	int x_curr = x_start, y_curr = y_start;
	for (i = 0; i < 1; i++)
	{
		x_curr = x_start;
		y_curr = y_start;
		iteration = 0;

		while (field[x_curr][y_curr] != 2)
		{
			x_curr = x_start;
			y_curr = y_start;
			while (field[x_curr][y_curr] != 2 && field[x_curr][y_curr] != 4)
			{
				do
				{
					do 
					{
						dx = rand() % 3 - 1;
						dy = rand() % 3 - 1;
					} while (dx == 0 && dy == 0);
					
					int x_check = x_curr + dx;
					int y_check = y_curr + dy;
				} while (robot_map[x_check][y_check] == 4);
				x_curr = x_check;
				y_curr = y_check;
				robot_map[x_curr][y_curr] = field[x_curr][y_curr];
				steps++;
				
			}

			print_field(robot_map);
			iteration++;
		}
		//cout << iteration << endl;
	}

}	// ________________________________END OF INT MAIN ________________________________

void print_field(int grid[][columns])					// prints the 2 by 2 array
{
	int i, j;
	for (i = 0; i < rows; i++) // print the grid
	{
		std::cout << "\n";
		//std::cout << i << std::endl;

		for (j = 0; j < columns; j++)
		{
			std::cout << grid[i][j];
		}
	}
	std::cout << std::endl;
}
//---------------------------------------------------------
void print_sequence(int grid[])							// prints the 1d array
{
	for (int i = 0; i < rows; i++) // print the grid
	{
		std::cout << grid[i];
	}
	std::cout << std::endl;
}
//---------------------------------------------------------
void path(int start_x, int start_y, int end_x, int end_y)			// finds the shortest path from one point to another
{
	float distance = 100000;

	for (int i = start_x - 1; i < start_x + 2; i++)
	{
		for (int j = start_y - 1; j < start_y + 2; j++)
		{
			if (sqrt(pow(end_x - i, 2) + (end_y - j, 2)) < distance)
			{
				distance = sqrt((end_x - i)*(end_x - i) + (end_y - j)*(end_y - j));
				start_x = i;
				start_y = j;
			}
		}
	}
}
//---------------------------------------------------------