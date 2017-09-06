// Today we're trying to make a path finding algorithm in a 2d array.  We back in this bitch
#include <iostream> 
#include <math.h>
#include <cmath>
const int rows = 5;
const int columns = 5;
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
void path(int start_x, start_y, end_x, end_y)			// finds the shortest path from one point to another
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
		x_start = 1;									// these locations are accounting for the boarder of 0's, +1 for each value
		y_start = 1;
		x_end = 3;
		y_end = 3;
		x_bomb = 2;
		y_bomb = 2;
	}

	for (i = 0; i < rows; i++)							// set all the values in the array to a 0
	{
		for (j = 0; j < columns; j++)
		{
			field[i][j] = 0;
		}
	}

	field[x_start][y_start] = 1;						// starting location marked with a 1
	//std::cout << field[x_start][y_start] << std::endl;

	field[x_end][y_end] = 2;							// end location marked with a 2
	//std::cout << field[x_end][y_end] << std::endl;

	field[x_bomb][y_bomb] = 3;							// bomb location is marked as a 3
	//std::cout << field[x_bomb][y_bomb] << std::endl;

	for (i = 0; i < rows; i++) // print the grid
	{
		std::cout << "\n";
		//std::cout << i << std::endl;
		for (j = 0; j < columns; j++)
		{
			std::cout << field[i][j];
		}
	}
	/*
	std::cout << "number of bombs: ";
	std::cin >> n;
	for (i = 0; i < n; i++) //locations of additional bombs
	{
	std::cout << "x coordinate of bomb: ";
	std::cin >> x_bomb;
	std::cout << "y coordinate of bomb: ";
	std::cin >> y_bomb;
	field[x_bomb][y_bomb] = 3;
	}
	*/
	print_field(field);

	float distance;
	int x_step, y_step;
	distance = sqrt((x_end - x_start)*(x_end - x_start) + (y_end - y_start)*(y_end - y_start));

	bool arrived = false;								// 
	if (arrived == false)
	{
		for (i = x_start - 1; i < x_start + 2; i++)		// checks all possible steps to see if the move is legal and then compute the distance from the goal.  Whichever step comes the closest to the goal is the one we step to
		{
			for (j = y_start - 1; j < y_start + 2; j++)
			{
				if (field[i][j] == 0 && sqrt((x_end - i)*(x_end - i) + (y_end - j)*(y_end - j)) < distance)
				{
					distance = sqrt((x_end - i)*(x_end - i) + (y_end - j)*(y_end - j));
					x_step = i;
					y_step = j;
				}
			}
		}

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
	else if (arrived == true) // im having trouble with this else if statement.  It should excecute once the stepper finds the goal. the stepped does indeed find the goal because arrive = true, 
							  //	but the else statement doesn't excecute
	{
		std::cout << "WE MADE IT BOYS TO THE GOAL!";
	}
	if (arrived == true)
	{
		std::cout << "we made it";
	}

}





	// so right now the loop goes through each possible step, checks if the step is valid then computes the distance from the goal. if the step is closer, then it takes that step, good
	// i need to make the step checking part stop once the loop lands on the goal. I'm not sure if that is done with a if statement abo

}//end of int main so don't write anything below this
/*
ok we're looking pretty good but this algorithm has flaws. It will not work for all grids.  What needs to happen: draw a line from the start to the goal. Then check for bombs along the path.  
if the path hits a bomb, then there needs to be a check for a way around the bomb.the line will turn into a bent line and there needs to be a check that the new line doesn't cross over any bombs'
maybe this idea of lines wont work with arrays but for now I'm assuming that this will work :) there will need to be many computations of which point is closer to another point.  

ok so we already made the algorithm for the shortest path from a point to another.That function will be used with an if statement to determine if the path passes through a bomb
*/
