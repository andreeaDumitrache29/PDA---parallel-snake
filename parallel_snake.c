#include "main.h"
#include "omp.h"

/** determina coordonatele cozii pornind de la capul sarpelui si
mergand din vecin in vecin pana se ajunge la ultimul punct al sarpelui.
In cazuri extreme (ultima/prima linie/coloana) se verifica pozitia
opusa pe harta **/
void CreateComponentList(int num_lines, int num_cols, int **world, struct snake *snake){
	//pozitia curenta
	struct coord currentpos;
	//pozitia viitoare
	struct coord futurePos;
	//coordonatele capului
	int i = (*snake).head.line;
	int j = (*snake).head.col;
	//pozitia de unde am venit
	struct coord cameFrom = (*snake).head;
	//am ajuns la final
	int final = 0;
	while(final == 0){
		//verifica deasupra
		if(i >= 1 && i < num_lines && j < num_cols && world[i - 1][j] == (*snake).encoding 
			&& (i - 1) != cameFrom.line ){
			cameFrom.line = i;
			cameFrom.col = j;
			i = i - 1;
			futurePos.line = i; futurePos.col = j;
		}
		//prima linie
		else if(i == 0 && j < num_cols && world[num_lines - 1][j] == (*snake).encoding 
			&& (num_lines - 1) != cameFrom.line ){
			cameFrom.line = i;
			cameFrom.col = j;
			i = num_lines - 1;
			futurePos.line = i; futurePos.col = j;
		}
		//verifica jos
		else if(i >= 0 && i < num_lines - 1 && j < num_cols && world[i + 1][j] == (*snake).encoding 
			&& (i + 1) != cameFrom.line){
			cameFrom.line = i;
			cameFrom.col = j;
			i = i + 1;
			futurePos.line = i; futurePos.col = j;
		}
		//ultima linie
		else if(i == num_lines - 1 && j < num_cols && world[0][j] == (*snake).encoding 
			&& 0 != cameFrom.line ){
			cameFrom.line = i;
			cameFrom.col = j;
			i = 0;
			futurePos.line = i; futurePos.col = j;
		}
		//verifica stanga
		else if(i < num_lines && j >= 1 && j < num_cols && world[i][j - 1] == (*snake).encoding 
		 && (j - 1) != cameFrom.col){
			cameFrom.line = i;
			cameFrom.col = j;
			j = j - 1;
			futurePos.line = i; futurePos.col = j;
		}
		//prima coloana
		else if(i < num_lines && j == 0 && world[i][num_cols - 1] == (*snake).encoding 
			&& (num_cols - 1) != cameFrom.col){
			cameFrom.line = i;
			cameFrom.col = j;
			j = num_cols - 1;
			futurePos.line = i; futurePos.col = j;
		}
		//verifica dreapta
		else if(i < num_lines && j >= 0 && j < num_cols - 1 && world[i][j + 1] == (*snake).encoding  
			&& (j + 1) != cameFrom.col){
			cameFrom.line = i;
			cameFrom.col = j;
			j = j + 1;
			futurePos.line = i; futurePos.col = j;
		}
		//ultima coloana
		else if(i < num_lines && j == num_cols - 1 && world[i][0] == (*snake).encoding 
			&& (0) != cameFrom.col){
			cameFrom.line = i;
			cameFrom.col = j;
			j = 0;
			futurePos.line = i; futurePos.col = j;
		}else{
			//am ajuns la final, actualizeaza coada sarpelui curent
			(*snake).tail.line = i;
			(*snake).tail.col = j;
			final = 1;
		}
	}

}

/** determina noua coada a sarpelui in cazul unei mutari.
	presupun ca la un moment dar coada poate avea un singur
	vecin care face parte din snake-ul curent **/
struct coord findTail(int num_lines, int num_cols, int** world, struct snake snake){
	//coordonate coada curenta
	int i = snake.tail.line;
	int j = snake.tail.col;
	struct coord rez;
	//verifica linia de jos
	if(i >= 1 && world[i - 1][j] == snake.encoding){
		rez.line = i - 1;
		rez.col = j;
	}
	//curent pe prima linie -> verifica ultima linie
	else if(i == 0 && world[num_lines - 1][j] == snake.encoding){
		rez.line = num_lines - 1;
		rez.col = j;
	}
	//verifica linia de deasupra
	else if(i < num_lines - 1 && world[i + 1][j] == snake.encoding){
		rez.line = i + 1;
		rez.col = j;
	//curent pe ultima linie => verifica prima linie
	}else if(i == num_lines - 1 && world[0][j] == snake.encoding){
		rez.line = 0;
		rez.col = j;
	//verifica coloana din stanga
	}if(j >= 1 && world[i][j - 1] == snake.encoding){
		rez.line = i;
		rez.col = j - 1;
	}
	//curent prima coloana => verifica ultima coloana
	else if(j == 0 && world[i][num_cols - 1] == snake.encoding){
		rez.line = i;
		rez.col = num_cols - 1;
	}
	//verifica coloana din dreapta
	else if(j < num_cols - 1 && world[i][j + 1] == snake.encoding){
		rez.line = i;
		rez.col = j + 1;
	//ultima coloana curent => verifica prima coloana
	}else if(j == num_cols - 1 && world[i][0] == snake.encoding){
		rez.line = i;
		rez.col = 0;
	}
	return rez;
}

/** muta toti serpii si actualizeaza matricea world **/
int moveSnake(int num_lines, int num_cols, int** world, struct snake **snake, int num_snakes){
	//vector pentru a retine coordonatele unde se muta capul fiecarui snake
	struct coord* newHead = (struct coord*)malloc(num_snakes* sizeof(struct coord));
	//linia si coloana curenta
	int line, col;
	//avem sau nu coliziune
	int collision = 0;
	int i, j;
	//vector de lockuri pentru a impiedica accesul concurent pe aceeasi celula de matrice
	omp_lock_t *locks = (omp_lock_t*)malloc((num_lines + num_cols) * sizeof(omp_lock_t));

	for(i = 0; i < num_lines + num_cols; i++){
			omp_init_lock(&locks[i]);
	}

	/** vom actualiza harta intai prin stergerea cozilor serpilor pentru a evita
	coliziunea acestora cu alti serpi **/
	for(i = 0; i < num_snakes; ++i)
	{
		world[(*snake)[i].tail.line][(*snake)[i].tail.col] = 0;
	}

	//in paralel pentru fiecare sarpe
	#pragma omp parallel shared(world, newHead, collision, snake)
	{
		#pragma omp for  private(i, line, col)
		for(i = 0; i < num_snakes; i++){
			//coordonatele curente ale capului
			line = (*snake)[i].head.line;
			col = (*snake)[i].head.col;
			//calculam noua pozitie in functie de directia de deplasare
			switch ((*snake)[i].direction){
				case 'N':
					newHead[i].col = col;
					if(line != 0){
						newHead[i].line = line - 1;
					}else {
						newHead[i].line = num_lines - 1;
					}
					break;
				case 'S' : 
					newHead[i].col = col;
					if(line != num_lines - 1){
						newHead[i].line = line + 1;
					}else if(line == num_lines - 1){
						newHead[i].line = 0;
					}
					break;
				case 'E' : 
					newHead[i].line = line;
					if(col != num_cols - 1){
						newHead[i].col = col + 1;
					}else{
						newHead[i].col = 0;
					}
					break;
				case 'V' : 
					newHead[i].line = line;
					if(col != 0){
						newHead[i].col = col - 1;
					}else{
						newHead[i].col = num_cols - 1;
					}
					break;
			}

			//vom modifica matricea, asa ca avem nevoie de lock pe celula curenta
			omp_set_lock(&locks[newHead[i].line + newHead[i].col]);
			//daca un sarpe se muta pe o pozitie care nu e libera avem coliziune
			if(world[newHead[i].line][newHead[i].col] != 0){
				#pragma omp critical
				{
					collision = 1;
				}
			}
			
			//cat timp nu e coliziune modifica harta prin mutarea capului sarpelui curent
			if(!collision){
				world[newHead[i].line][newHead[i].col] = (*snake)[i].encoding;
			}
			omp_unset_lock(&locks[newHead[i].line + newHead[i].col]);
		}

		//daca nu am avut coliziune
		if(collision == 0){
			#pragma omp for private(i)
			for(i = 0; i < num_snakes; i++){
				//determina coordonatele noii cozi si actualizeaza pozitia capului
				omp_set_lock(&locks[newHead[i].line + newHead[i].col]);
				(*snake)[i].tail = findTail(num_lines, num_cols, world, (*snake)[i]);
				(*snake)[i].head = newHead[i];	
				omp_unset_lock(&locks[newHead[i].line + newHead[i].col]);
			}
		}else{
			//daca am avut coliziune trebuie sa refacem configuratia anterioara
			#pragma omp for private(i)
			for(i = 0; i < num_snakes; i++){
				omp_set_lock(&locks[newHead[i].line + newHead[i].col]);
				//punem la loc pe harta encoding-ul sarpelui unde era coada
				world[(*snake)[i].tail.line][(*snake)[i].tail.col] = (*snake)[i].encoding;
				//daca am apucat sa mutam capul marcam celula respectiva ca fiind din nou libera
				if(world[newHead[i].line][newHead[i].col] == (*snake)[i].encoding){
					world[newHead[i].line][newHead[i].col] = 0;
				}
				omp_unset_lock(&locks[newHead[i].line + newHead[i].col]);
			}
		}
	}

	if(collision)
		return 1;
	return 0;
}

void run_simulation(int num_lines, int num_cols, int **world, int num_snakes,
	struct snake *snakes, int step_count, char *file_name) 
{
	// TODO: Implement Parallel Snake simulation using the default (env. OMP_NUM_THREADS) 
	// number of threads.
	//
	// DO NOT include any I/O stuff here, but make sure that world and snakes
	// parameters are updated as required for the final state.
	int i = 0;
	int j = 0;
	int col = 0;
	
	#pragma omp parallel
	{
		//calculeaza cozile pentru fiecare sarpe
		#pragma omp for private(i)
		for(i = 0; i < num_snakes; i++){
			CreateComponentList(num_lines, num_cols, world, &snakes[i]);
		}
	}

	//cat timp nu s-au scurs toate rundele
	while(step_count > 0){
		//muta serpii si daca a avut loc o coliziune opreste-te
		if(moveSnake(num_lines, num_cols, world, &snakes, num_snakes) != 0){
			col = 1;
			break;
		}
			
		step_count--;
	}
}


