#include <iostream>
#include <vector>
#include <thread>
#include <chrono>
#include <random>
#include <cstdlib>
#include <ctime>
#include <Windows.h>


std::mt19937 mt_rand(time(0));				//provide seed to random function

//mapping of chars to the gameobjects
enum {
	FENCE = '#',
	SNAKE_BODY_BLOCK = 'o',
	FOOD = '*',
	EMPTY_CELL = ' '
};

//determines snake body unit (position)
struct SnakeBodyBlock{
	int x_pos;
	int y_pos;

	SnakeBodyBlock(int x, int y){
		x_pos = x;
		y_pos = y;
	}
};


//snake gameobject class
class Snake{
	std::vector<SnakeBodyBlock*> snake;
public:
	inline std::vector<SnakeBodyBlock*>& retrunSnake(){ return snake; }
};


//food gameobject class
class Food{
	int x_food_pos;
	int y_food_pos;

public:
	Food(int,int);

	inline int returnXpos(){ return x_food_pos; }
	inline int returnYpos(){ return y_food_pos; }
	
};

Food::Food(int x,int y){
	x_food_pos = x;
	y_food_pos = y;
}

//manaages all the gameobjects
class GameManager{
	
	int playable_area_width;					//width of the game
	int playable_area_height;					//height of the game

	char** game_map;							//character map

	bool isGameOver;							//to determine wether game is over

	int snake_direction;						//holds value for snakes current direction

	Snake* s;
	Food* f;

	int score;									//score

public:
	

	GameManager(int,int);
	void initializeEnvironment();				//initializes the game for the first time
	void displayEnvironment();					//displays char map
	void gameLoop();							// main game loop
	void snakePositionUpdateOnMap();			//updates snake_body_blocks' position in char map
	void snakePositionRemoved();				//removes old snake_body_blocks' position from char map
	void snakeblockPositionReassign();			//assigns new position to the snake_body_block apart from head
	void foodPositionUpdate(Food*);				//updates position of the food in the char map
	void displayGameOver();						//game over msg
	void resetGame();							//resets score to zero
};

GameManager::GameManager(int width,int height){
	playable_area_width = width;
	playable_area_height = height;

	isGameOver = false;

	game_map = new char*[height];
	for (int i = 0; i < height; i++){
		game_map[i] = new char[width];
	}

	snake_direction = 8;
	s = new Snake();
	s->retrunSnake().push_back(new SnakeBodyBlock(playable_area_width/2, playable_area_height/2));


	f = new Food(mt_rand() % (playable_area_width-2) + 1,mt_rand() % (playable_area_height-2)+1);
}

void GameManager::initializeEnvironment(){
	for (int i = 0; i < playable_area_height; i++){
		for (int j = 0; j < playable_area_width; j++){
			if (i == 0 || i == playable_area_height - 1){
				game_map[i][j] = FENCE;
			}
			else if (j == 0 || j == playable_area_width - 1){
				game_map[i][j] = FENCE;
			}
			else{
				game_map[i][j] = EMPTY_CELL;
			}
		}
	}

	game_map[playable_area_height / 2][playable_area_width / 2] = SNAKE_BODY_BLOCK;
	game_map[f->returnYpos()][f->returnXpos()] = FOOD;
}

void GameManager::displayEnvironment(){
	std::cout << "Score: " << score<<"\n";
	for (int i = 0 ; i < playable_area_height ; i++){
		for (int j = 0 ; j < playable_area_width ; j++){
			std::cout << game_map[i][j];
		}
		std::cout << "\n";
	}
}

void GameManager::gameLoop(){
	resetGame();
	while (!isGameOver){
		
		system("cls");
		displayEnvironment();
		std::vector<SnakeBodyBlock*> &tempSnake = s->retrunSnake();
		snakePositionRemoved();
		std::vector<SnakeBodyBlock*> cache;						//cache will store old snake position whichis to be removed
		cache = s->retrunSnake();

		//checking input and assigning direction
		if (GetAsyncKeyState(VK_UP)){
			if (snake_direction!=2)
				snake_direction = 8;
		}
		else if (GetAsyncKeyState(VK_DOWN)){
			if (snake_direction!=8)
				snake_direction = 2;
		}
		else if (GetAsyncKeyState(VK_LEFT)){
			if (snake_direction!=6)
				snake_direction = 4;
		}
		else if (GetAsyncKeyState(VK_RIGHT)){
			if (snake_direction!=4)
				snake_direction = 6;
		}
		
		snakeblockPositionReassign();

		//moving head based on direction
		switch (snake_direction)
		{
		case 8:
			tempSnake[0]->y_pos -= 1;
			break;
		case 2:
			tempSnake[0]->y_pos += 1;
			break;
		case 4:
			tempSnake[0]->x_pos -= 1;
			break;
		case 6:
			tempSnake[0]->x_pos += 1;
			break;
		default:
			break;
		}

		switch (game_map[tempSnake[0]->y_pos][tempSnake[0]->x_pos])
		{
		case FOOD:
			score += 1;
			tempSnake.push_back(new SnakeBodyBlock(tempSnake[0]->x_pos, tempSnake[0]->y_pos));
			
			//spawning food at raandom places
			int random_food_xPos;
			int random_food_yPos;
			do{
				random_food_xPos = (mt_rand() % (playable_area_width - 1)) + 1;
				random_food_yPos = (mt_rand() % (playable_area_height - 1)) + 1;
			} while (game_map[random_food_yPos][random_food_xPos] != EMPTY_CELL);

			Food *food;
			food = new Food(random_food_xPos, random_food_yPos);
			foodPositionUpdate(food);
			break;
		case FENCE:
			isGameOver = true;
			displayGameOver();
			break;
		default:
			break;
		}
		
		//check if the new head position coinsides with old snake body_block_positions (a game over condition)
		for (int k = 4; k < cache.size(); k++){
			if (cache[k]->x_pos == tempSnake[0]->x_pos && cache[k]->y_pos == tempSnake[0]->y_pos){
				isGameOver = true;
				displayGameOver();
			}
		}


		snakePositionUpdateOnMap();
	

		std::this_thread::sleep_for(std::chrono::milliseconds(200));
	}
}


void GameManager::snakePositionUpdateOnMap(){
	std::vector<SnakeBodyBlock*> &tempSnake = s->retrunSnake();
	for (unsigned int i = 0; i < tempSnake.size(); i++){
		game_map[tempSnake[i]->y_pos][tempSnake[i]->x_pos] = SNAKE_BODY_BLOCK;
	}
}

void GameManager::snakePositionRemoved(){
	std::vector<SnakeBodyBlock*> &tempSnake = s->retrunSnake();
	for (unsigned int i = 0; i < tempSnake.size(); i++){
		game_map[tempSnake[i]->y_pos][tempSnake[i]->x_pos] = EMPTY_CELL;
	}
}

void GameManager::snakeblockPositionReassign(){
	std::vector<SnakeBodyBlock*> &tempSnake = s->retrunSnake();
	for (int i = tempSnake.size()-1; i>0; i--){
		tempSnake[i]->x_pos = tempSnake[i - 1]->x_pos;
		tempSnake[i]->y_pos = tempSnake[i - 1]->y_pos;
	}
}

void GameManager::foodPositionUpdate(Food *food){
	game_map[food->returnYpos()][food->returnXpos()] = FOOD;
}

void GameManager::displayGameOver(){
	system("cls");
	std::cout << "\n\n\n\n\n\n\n\t\tG A M E   O V E R\n\n\t\t    Continue? \n\t\t      Y  N\n\n\n\n\n\n\n\n\n";
}

void GameManager::resetGame(){
	score = 0;
}

int main(){
	char ch;
	do{
		GameManager G(40, 20);
		G.initializeEnvironment();
		G.gameLoop();
		std::cin >> ch;
	} while (ch == 'y');
	return 0;
}