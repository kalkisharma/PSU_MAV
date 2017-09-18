#include <iostream> 
#include <vector>
#include <cmath>
const int rows = 9;
const int columns = 9;
//-----------------------------------------------
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

	for (i = 0; i < rows; i++)							// set all the values in the array to a 0
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

	field[x_start][y_start] = 1;						// starting location marked with a 1
	field[x_end][y_end] = 2;							// end location marked with a 2
	field[x_bomb][y_bomb] = 3;							// bomb location is marked as a 3

	for (i = 3; i < 8; i++)								// makes a wall of bombs
	{
		field[i][4] = 3;
	}

	print_field(field);									// calls the print grid function

	int x_step, y_step;
	float distance = sqrt((x_end - x_start)*(x_end - x_start) + (y_end - y_start)*(y_end - y_start));
	int diagonal[4][3];
	bool arrived = false;								
	if (arrived == false)
	{
		for (i = x_start - 1; i < x_start + 2; i++)		// checks all possible steps 
		{
			for (j = y_start - 1; j < y_start + 2; j++)
			{
				robot_map[i][j] = field[i][j];			// the robot creates its own map of its surroundings for later use.

				if (i == x_start - 1 && j == y_start - 1)		// checks the diagonals and stores the value and the coordinates
				{
					diagonal[1][0] = field[i][j];
					diagonal[1][1] = i;
					diagonal[1][2] = j;
				}
				else if (i == x_start - 1 && j == y_start + 1)
				{
					diagonal[0][0] = field[i][j];
					diagonal[0][1] = i;
					diagonal[0][2] = j;
				}
				else if (i == x_start + 1 && j == y_start - 1)
				{
					diagonal[2][0] = field[i][j];
					diagonal[2][1] = i;
					diagonal[2][2] = j;
				}
				else if (i == x_start + 1 && j == y_start + 1)
				{
					diagonal[3][0] = field[i][j];
					diagonal[3][1] = i;
					diagonal[3][2] = j;
				}
			}
		}

		for (i = 0; i < 4; i++)
		{
			if (diagonal[i][0] == 0)
			{

			}
		}
		/*

		for (i = x_step - 1; i < x_step + 2; i++)	// this is a similar loop to the one above but starts on a step rather than the starting point. Also this loop will stop once the stepper lands on the goal
		{
			for (j = y_step - 1; j < y_step + 2; j++)
			{
				if (field[i][j] == 0 && sqrt((x_end - i)*(x_end - i) + (y_end - j)*(y_end - j)) < distance)
				{
					distance = sqrt((x_end - i)*(x_end - i) + (y_end - j)*(y_end - j));
					x_step = i;
					y_step = j;
				}
				if (field[i][j] == 2)
				{
					arrived = true;
				}
			}
		}
		std::cout << x_step << y_step;
	}

	if (arrived == true)
	{
		std::cout << "we made it";
	}
	*/
	}
}	// ________________________________END OF INT MAIN ________________________________
