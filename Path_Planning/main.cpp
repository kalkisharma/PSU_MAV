/*
Author:  Andrew Miller
Updated: 9/20/17
Summary: Prints the field and the robot's map.  Later the robot will go scouting for the goal
Comments: 1) Change your functions like I modified the function print_field(). Instead of writing the entire
              function, just write the function declaration and then write the actual function after
              main()
          2) Inside main should only be variable declarations (e.g. int x, y). Make everything else functions.
          3) Replace (x_end - x_start)*(x_end - x_start) with pow(x_end - x_start, 2)
          4) From lines 114-126 the code is incomplete. Correct the code and after changes 1-3 have been made
              upload onto Git again.
*/

#include <iostream>
#include <vector>
#include <cmath>

const int rows = 9;
const int columns = 9;
//-----------------------------------------------
void print_field(int grid[][columns]);					// prints the 2 by 2 array

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
			if (sqrt((end_x - i)*(end_x - i) + (end_y - j)*(end_y - j)) < distance)
			{
				distance = sqrt((end_x - i)*(end_x - i) + (end_y - j)*(end_y - j));
				start_x = i;
				start_y = j;
			}
		}
	}
}
//---------------------------------------------------------
int main()
{
	int i, j;
	int x_start, y_start, x_end, y_end;
	int x_bomb, y_bomb;
	//int n;	//number of bombs
	bool userInput = false;

	std::cout << "number of rows: " << rows << std::endl;
	std::cout << "number of columns: " << columns << std::endl;

	int field[rows][columns];
	int robot_map[rows][columns];

	if (userInput == true)
	{
		std::cout << "\nx coordinate of starting location: ";
		std::cin >> x_start;
		std::cout << "y coordinate of starting location: ";
		std::cin >> y_start;
		std::cout << "x coordinate of ending location: ";
		std::cin >> x_end;
		std::cout << "y coordinate of ending location: ";
		std::cin >> y_end;
		std::cout << "x coordinate of bomb: ";
		std::cin >> x_bomb;
		std::cout << "y coordinate of bomb: ";
		std::cin >> y_bomb;
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
			robot_map[i][j] = 0;
		}
	}


	field[x_start][y_start] = 1;						// starting location marked with a 1
	field[x_end][y_end] = 2;							// end location marked with a 2

	print_field(field);									// calls the print grid function
	print_field(robot_map);								// prints the robots' map
	int x_step, y_step;
	float distance = sqrt((x_end - x_start)*(x_end - x_start) + (y_end - y_start)*(y_end - y_start));
	for (int n = 0; n < 12; n++)
	{
		for (i = x_start - 1; i < x_start + 2, i++)
		{
			for (j = y_start - 1; j < y_start + 2, j++)
			{
				if (robot_map[i][j] = 0)
				{
					int
				}
			}
		}
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
