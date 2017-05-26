#include <iostream>
#include <math.h>
const int rows = 3;
const int columns = 3;
//---------------------------------------------------------
void print_field(int grid[][columns])
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
int maximum(int x_step[])
{
	int maxX = x_step[0], i;
	for (i = 0; i < 3; i++)
	{
		if (x_step[i] > maxX)
		{
			maxX = x_step[i];
		}
	}
	return(maxX);
}
//---------------------------------------------------------
int minimum(int step[])
{

	int min = step[0];

	return(min);
}
//---------------------------------------------------------
int main()
{
	int i, j;
	int x_start, y_start, x_end, y_end;
	int x_bomb, y_bomb;
	//int n;	//number of bombs
	int maxX, minX;
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
	else if (userInput == false)
	{
		x_start = 0;
		y_start = 0;
		x_end = 2;
		y_end = 2;
		x_bomb = 0;
		y_bomb = 2;
	}
	for (i = 0; i < rows; i++)
	{

		for (j = 0; j < columns; j++)
		{
			field[i][j] = 0;
		}
	}

	field[x_start][y_start] = 1;	//starting location marked with a 1
									//std::cout << field[x_start][y_start] << std::endl;

	field[x_end][y_end] = 2;	//end location marked with a 2
								//std::cout << field[x_end][y_end] << std::endl;

	field[x_bomb][y_bomb] = 3;	//bomb location is marked as a 3
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

	for (k = 0; k < n; k++) //locations of additional bombs
	{
		std::cout << "x coordinate of bomb: ";
		std::cin >> x_bomb;
		std::cout << "y coordinate of bomb: ";
		std::cin >> y_bomb;

		field[x_bomb][y_bomb] = 3;
	}

	*/
	print_field(field);


	int x_step[3], y_step[3]; //move all these to the top
	int x_min  = 0, x_max = 2, y_min = 0, y_max = 2;
	int countx = 0, county = 0;
	x_step[0] = x_start - 1;
	x_step[1] = x_start;
	x_step[2] = x_start + 1;
	y_step[0] = y_start - 1;
	y_step[1] = y_start;
	y_step[2] = y_start + 1;

	for (i = 0; i < 3; i++)
	{
		if (x_step[i] < x_min || x_step[i] > x_max)
		{
			x_step[i] = -3; // making any value that's out of bound equal to a negative number

			countx = countx + 1;//count the number of steps that are out of bounds
		}
	}

	for (i = 0; i < 3; i++)
	{
		if (y_step[i] < y_min || y_step[i] > y_max)
		{
			y_step[i] = -3; // making any value that's out of bound equal to a negative number

			county = county + 1;//count the number of steps that are out of bounds ie non negative
		}
	}


	minX = minimum(x_step);
	std::cout << minX;


	maxX = maximum(x_step);

	std::cout << maxX;

	bool illegal;
	illegal = false;

	for (i = 0; i < 3; i++)
	{
		if (x_step[i] < x_min || x_step[i] > x_max)
		{

		}

	}

}//end of int main so don't write anything below this
