/**
 * @file fc_game.c
 * @author Abdelhakim RAFIK
 * 
 * @version 1.0.1
 * @date 2021-06
 * 
 * @copyright Copyright (c) 2021
 * 
 */

#include "../include/fc_game_core.h"

/**
 * Create a new game instance
 * 
 * @param game 
 * @return fc_game_t* 
 */
fc_game_t* fc_create_game() {
	// create game instance
	return (fc_game_t*) malloc(sizeof(fc_game_t));
}

/**
 * Initialize a game
 * 
 * @param game 
 * @return uint8_t 
 */
uint8_t fc_game_init(fc_game_t *game) {
	// game not created
	if(!game)
		return 0;
	// allocate 2d table to store created cards
	uint8_t **saturation = (uint8_t**) malloc(sizeof(uint8_t*) * 13);
	for(uint8_t i=0; i<13; ++i)
		saturation[i] = (uint8_t*) malloc(sizeof(uint8_t) * 4);
	
	// initialisation of saturation table
	for(uint8_t i=0; i<13; ++i) {
		for(uint8_t j=0; j<4; ++j)
			saturation[i][j] = 0;
	}

	// initialise stack column
	for(uint8_t i=0; i<8; ++i) {
		game->listColumns[i].count = 0;
		game->listColumns[i].list = fc_dll_create();
	}

	// initialise freeCells and foundation columns
	for(uint8_t i=0; i<4; ++i) {
		game->freeCells[i] = NULL;
		game->foundations[i].count = 0;
		game->foundations[i].stack = NULL;
	}

	// fill stack columns with random cards
	uint8_t columnCardsNumber = 7;
	fc_card_t *card;
	for(uint8_t i=0; i<8; ++i) {
		// from the 5th column set the number of cards by column to 6
		if(i == 4)
			columnCardsNumber = 6;
		// fill cards
		for(uint8_t j=0; j<columnCardsNumber; ++j) {
			card = fc_create_random_card(saturation);
			// add card to end of list (zone1)
			if(card && fc_dll_add_to_tail(game->listColumns[i].list, card))
				// increment column card numbers
				game->listColumns[i].count++;
			else
				return 0;
		}
	}

	// free saturation table memory
	for(uint8_t i=0; i<13; ++i)
		free(saturation[i]);
	free(saturation);

	return 1;
}

/**
 * Move cards between zone 1 columns, from source index to destination index
 * 
 * @param game 
 * @param source 
 * @param destination 
 * @return uint8_t 
 */
uint8_t fc_move_cards_between_columns(fc_game_t *game, uint8_t cardIndex, uint8_t source, uint8_t destination) {
	// when empty game instance
	if(!game)
		return 2;
	
	// indexs validation
	if(!(destination >= 0 && destination <= 7 && source >= 0 && source <= 7))
		return 3; /* invalide indexs */

	// get cards
	fc_card_t *card = (fc_card_t*) fc_dll_peek(game->listColumns[source].list);
	fc_card_t *destTopCard = (fc_card_t*) fc_dll_peek(game->listColumns[destination].list);

	printf("M l%d -> l%d | %p | %p |\n", source+1, destination+1, card, destTopCard);

	// check rules and move card
	if(card && (destTopCard == NULL || (card->color != destTopCard->color && card->number + 1 == destTopCard->number))) {
		// move the source card to destination column
		fc_dll_add_to_head(game->listColumns[destination].list, card);
		game->listColumns[destination].count++;
		// remove card from source column
		fc_dll_pop(game->listColumns[source].list);
		game->listColumns[source].count--;
		// action success
		return 1;
	}
	// rules not validated
	return 4;
}

/**
 * Move card from free cell (zone2) to foundation (zone3)
 * 
 * @param game 
 * @param source 
 * @param destination 
 * @return uint8_t 
 */
uint8_t fc_move_card_freecell_foundation(fc_game_t *game, uint8_t source, uint8_t destination) {
	// when empty game
	if(!game)
		return 2;
	// indexs validation
	if(!(destination >= 0 && destination <= 3 && source >= 0 && source <= 3))
		return 3; /* invalide indexs */
	// get card from source column
	fc_card_t *card = game->freeCells[source];
	fc_card_t *destTopCard = (fc_card_t*) fc_stack_peek(game->foundations[destination].stack);

	printf("M c%d -> f%d | %p | %p |\n", source+1, destination+1, card, destTopCard);

	// check rules and move card
	if(card && ((destTopCard == NULL && card->number == 1) || (destTopCard != NULL && card->type == destTopCard->type && card->number - 1 == destTopCard->number))) {
		// move the source card to destination column
		fc_stack_push(&(game->foundations[destination].stack), card);
		game->foundations[destination].count++;
		// remove card from source free cell
		game->freeCells[source] = NULL;
		// action success
		return 1;
	}
	// rules not validated
	return 4;
}

/**
 * Move card from free cell (zone2) to column (zone1)
 * 
 * @param game 
 * @param source 
 * @param destination 
 * @return uint8_t 
 */
uint8_t fc_move_card_freecell_column(fc_game_t *game, uint8_t source, uint8_t destination) {
	// when empty game
	if(!game)
		return 2;
	// indexs validation
	if(!(destination >= 0 && destination <= 7 && source >= 0 && source <= 3))
		return 3; /* invalide indexs */

	// get card from source column
	fc_card_t *card = game->freeCells[source];
	fc_card_t *destTopCard = (fc_card_t*) fc_dll_peek(game->listColumns[destination].list);

	printf("M c%d -> l%d | %p | %p |\n", source+1, destination+1, card, destTopCard);

	// check rules and move card
	if(card && (destTopCard == NULL || (card->color != destTopCard->color && card->number + 1 == destTopCard->number))) {
		// move the source card to destination column
		fc_dll_add_to_head(game->listColumns[destination].list, card);
		game->listColumns[destination].count++;
		// remove card from source free cell
		game->freeCells[source] = NULL;
		// action success
		return 1;
	}
	// rules not validated
	return 4;
}

/**
 * Move card from column (zone1) to free cell (zone2)
 * 
 * @param game 
 * @param source 
 * @param destination 
 * @return uint8_t 
 */
uint8_t fc_move_card_column_freecell(fc_game_t *game, uint8_t source, uint8_t destination) {
	// when empty game
	if(!game)
		return 2;
	// indexs validation
	if(!(destination >= 0 && destination <= 3 && source >= 0 && source <= 7))
		return 3; /* invalide indexs */
	// check destination freecell is empty
	if(game->freeCells[destination] != NULL)
		return 4; /* free cell not empty */

	// get card from source column
	fc_card_t *card = (fc_card_t*) fc_dll_pop(game->listColumns[source].list);
	game->listColumns[source].count--;
	// move card to free cell destination
	game->freeCells[destination] = card;
	// action success
	return 1;
}

/**
 * Move card from column (zone1) to foundation (zone3)
 * 
 * @param game 
 * @param source 
 * @param destination 
 * @return uint8_t 
 */
uint8_t fc_move_card_column_foundation(fc_game_t *game, uint8_t source, uint8_t destination) {
	// when empty game
	if(!game)
		return 2;
	// indexs validation
	if(!(destination >= 0 && destination <= 3 && source >= 0 && source <= 7))
		return 3; /* invalide indexs */
	// get card from source column
	fc_card_t *card = (fc_card_t*) fc_dll_peek(game->listColumns[source].list);
	fc_card_t *destTopCard = (fc_card_t*) fc_stack_peek(game->foundations[destination].stack);

	printf("M l%d -> f%d | %p | %p |\n", source+1, destination+1, card, destTopCard);

	// check rules and move card
	if(card && ((destTopCard == NULL && card->number == 1) || (destTopCard != NULL && card->type == destTopCard->type && card->number - 1 == destTopCard->number))) {
		// move the source card to destination column
		fc_stack_push(&(game->foundations[destination].stack), card);
		game->foundations[destination].count++;
		// remove card from source column
		fc_dll_pop(game->listColumns[source].list);
		game->listColumns[source].count--;
		// action success
		return 1;
	}
	// rules not validated
	return 4;
}

/**
 * Check if the user has won the game
 * 
 * @param game 
 * @return uint8_t 
 */
uint8_t fc_game_is_won(fc_game_t *game) {
	// check foundation columns
	for(uint8_t i=0; i<4; i++) {
		if(game->foundations[i].count != 13)
			return 0;
	}
	// if all foundation columns count is 13 then the use has won the game
	return 1;
}

/**
 * Check if the game is over, no more moves possible
 * 
 * @param game 
 * @return uint8_t 
 */
uint8_t fc_game_is_over(fc_game_t *game) {
	// check if one of freecells is empty
	for(uint8_t i=0; i<4; i++)
		if(game->freeCells[i] == NULL){
			printf("Empty Freecell");
			return 0;
		}

	fc_card_t *currentCard;
	fc_card_t *card;
	
	// check freecell move possibilities
	for(uint8_t i=0; i<4; i++) {
		// get card from freecell 
		currentCard = game->freeCells[i];
		// check possible move from freecells to foundation column
		for(uint8_t j=0; j<4; j++) {
			card = (fc_card_t*) fc_stack_peek(game->foundations[j].stack);
			if(card != NULL && currentCard->number - 1 == card->number && currentCard->type == card->type) {
				printf("Freecell to foundation");
				return 0;
			}
		}
		// check possible move from freecells to list columns
		for(uint8_t j=0; j<8; j++) {
			card = (fc_card_t*) fc_dll_peek(game->listColumns[j].list);
			if(card != NULL && currentCard->number + 1 == card->number && currentCard->color != card->color) {
				printf("Freecell to list");
				return 0;
			}
		}
	}

	// check column move possibilities
	for(uint8_t i=0; i<8; i++) {
		currentCard = (fc_card_t*) fc_dll_peek(game->listColumns[i].list);
		if(currentCard == NULL) {
			printf("Empty list column");
			return 0; /* Empty column */
		}
		// check possible move from column list to foundation
		for(uint8_t j=0; j<4; j++) {
			card = (fc_card_t*) fc_stack_peek(game->foundations[j].stack);
			if((card == NULL && currentCard->number == 1) || (card != NULL && currentCard->number -1 == card->number && currentCard->type == card->type)) {
				printf("List to foundation");
				return 0;
			}
		}

		// check move possiblities between columns
		for(uint8_t j=i+1; j<8; j++) {
			card = (fc_card_t*) fc_dll_peek(game->listColumns[j].list);
			// check move posibilitie between i column and j column
			if(card != NULL && ((currentCard->number + 1 == card->number && currentCard->color != card->color) || (card->number + 1 == currentCard->number && card->color != currentCard->color))) {
				printf("Between columns");
				return 0;
			}
		}
	}
	// game over
	return 1;
}

/**
 * Free game content
 * 
 * @param game 
 */
void fc_free_game_content(fc_game_t *game) {

	// free stack columns content
	fc_card_t *card;
	for(uint8_t i=0; i<8; ++i) {
		while(card = (fc_card_t*) fc_dll_pop(game->listColumns[i].list))
			fc_free_card(card);
		fc_dll_free(game->listColumns[i].list);
		game->listColumns[i].count = 0;
	}

	// free foundation columns and freecells
	for(uint8_t i=0; i<4; ++i) {
		// free foundation columns
		while (card = (fc_card_t*) fc_stack_pop(&(game->foundations[i].stack)))
			fc_free_card(card);
		game->foundations[i].count = 0;

		// free freecells
		if(game->freeCells[i])
			fc_free_card(game->freeCells[i]);
	}
}

/**
 * Destroy a game instance
 * 
 * @param game 
 */
void fc_free_game(fc_game_t *game) {
	// free game content
	fc_free_game_content(game);
	// free game memory
	free(game);
}