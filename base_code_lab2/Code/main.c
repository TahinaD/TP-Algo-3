// Need this to use the getline C function on Linux. Works without this on MacOs. Not tested on Windows.
#define _GNU_SOURCE
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <assert.h>
#include <math.h>

#include "token.h"
#include "queue.h"
#include "stack.h"


/** 
 * Utilities function to print the token queues
 */
void print_token(const void* e, void* user_param);
void print_queue(FILE* f, Queue* q);

Queue* stringToTokenQueue(const char* expression);
Queue* shuntingYard(Queue* infix);
float evaluateExpression(Queue* postfix);

/** 
 * Functions to be written by students
 */
ptrToken convert_queue_top_to_token(ptrQueue q) {
	ptrToken t = (Token *)queue_top(q);
	return t;
}

ptrToken convert_stack_top_to_token(ptrStack s) {
	ptrToken t = (Token *)stack_top(s);
	return t;
}


void computeExpressions(FILE* input) {
	(void)input;
	char * line = NULL;
    size_t len = 0;
	
	while ((getline(&line, &len, input)) != -1) {
		char * expr = line;
		while (*expr == ' ')
			expr++;
		if (*expr != '\n') {
			printf("Input : %s", expr);
			
			ptrQueue infix = stringToTokenQueue(expr);
			
			if (token_is_parenthesis(queue_top(infix)) || token_is_number(queue_top(infix))) {
				printf("Infix : ");
				print_queue(stdout, infix);
				printf("\n");
			
				printf("Postfix : ");
				ptrQueue postfix = shuntingYard(infix);
				print_queue(stdout, postfix);
				printf("\n");

				printf("Evaluate : %f", evaluateExpression(postfix));
			}
			else {
				Token* infix_top = convert_queue_top_to_token(infix);
				queue_pop(infix);
				delete_token(&infix_top);
				delete_queue(&infix);
			}
			printf("\n\n");
		}
	}

	free(line);
}

bool isSymbol(char c) {
	return c == '+' || c == '-' || c == '*' || c == '/' || c == '^' || c == '(' || c == ')';
}

bool isDigit(char c) {
	return (c - '0') >= 0 && (c - '0') <= 9;
}

Queue* stringToTokenQueue(const char* expression) {
	Queue* result = create_queue();
	const char* curpos = expression;
	int nb = 0;

	while (*curpos != '\0') {
		while (*curpos == ' ' || *curpos == '\n')
			curpos++;

		if (isSymbol(*curpos))
			nb++;

		else if (isDigit(*curpos)) {
			const char * fin = curpos;
			while (isDigit(*fin)) {
				fin++;
				nb++;
			}
		}

		else if (!(*curpos == '\0' || isSymbol(*curpos) || isDigit(*curpos))) {
			fprintf(stderr, "Caractère incorrect dans l'expression. \n");
			while (!queue_empty(result)) {
				Token * token = convert_queue_top_to_token(result);
				queue_pop(result);
				delete_token(&token);
			}
			queue_push(result, create_token_from_string("erreur", 6));
			return result;
		}

		if (*curpos != '\0')
			queue_push(result, create_token_from_string(curpos, nb));
		curpos += nb;
		nb = 0;
	}

	return result;
}


Queue* shuntingYard(Queue* infix) {
	Queue* postfix = create_queue();
	Stack* oper = create_stack(queue_size(infix));
	Token* read;

	while (!queue_empty(infix)) {
		read = convert_queue_top_to_token(infix);
		queue_pop(infix);

		if (token_is_number(read))
			queue_push(postfix, read);

		else if (token_is_operator(read)) {
			
			while (!stack_empty(oper) && (token_operator_priority(stack_top(oper)) > token_operator_priority(read) 
				|| (token_operator_priority(stack_top(oper)) == token_operator_priority(read) 
					&& token_operator_leftAssociative(stack_top(oper)))) 
					&& (!token_is_parenthesis(stack_top(oper)) || token_parenthesis(stack_top(oper)) == ')')) {
				
				Token* oper_top = convert_stack_top_to_token(oper);
				stack_pop(oper);
				queue_push(postfix, oper_top);
			}
			stack_push(oper, read);
		}

		else if (token_parenthesis(read) == '(') {
			stack_push(oper, read);
		}

		else if (token_parenthesis(read) == ')') {
			while (!stack_empty(oper) && token_parenthesis(stack_top(oper)) != '(') {
				Token* oper_top = convert_stack_top_to_token(oper);
				stack_pop(oper);
				queue_push(postfix, oper_top);
			}

			if (!stack_empty(oper)) {
				Token* oper_top = convert_stack_top_to_token(oper);
				stack_pop(oper);
				delete_token(&oper_top);
			}
			else
				fprintf(stderr, "Parenthèse ouvrante manquante. Expression évaluée avec parenthèse ouvrante sous-entendue au début de l'expression. \n");
			delete_token(&read);
		}

	}

	delete_queue(&infix);

	while (!stack_empty(oper)) {
		Token* oper_top = convert_stack_top_to_token(oper);
		stack_pop(oper);

		if (token_is_parenthesis(oper_top)) {
			fprintf(stderr, "Parenthèse fermante manquante. Expression évaluée avec parenthèse fermante sous-entendue à la fin de l'expression. \n");
			delete_token(&oper_top);
		}
		else
			queue_push(postfix, oper_top);
	}

	delete_stack(&oper);
	
	return postfix;
}


Token* evaluateOperator(Token* arg1, Token* op, Token* arg2) {
	float res;
	if (token_operator(op) == '+')
		res = token_value(arg1) + token_value(arg2);
	else if (token_operator(op) == '-')
		res = token_value(arg1) - token_value(arg2);
	else if (token_operator(op) == '*')
		res = token_value(arg1) * token_value(arg2);
	else if (token_operator(op) == '/' && token_value(arg2) != 0)
		res = token_value(arg1) / token_value(arg2);
	else if (token_operator(op) == '/' && token_value(arg2) == 0)
		return create_token_from_string("non defini", 10);
	else
		res = powf(token_value(arg1), token_value(arg2));
	return create_token_from_value(res);
}

float evaluateExpression(Queue* postfix) {
	Token* token;
	float resultat, div_0 = false;
	Stack * postfix_bis = create_stack(queue_size(postfix));
	
	while (!queue_empty(postfix)) {
		token = convert_queue_top_to_token(postfix);
		queue_pop(postfix);

		if (token_is_operator(token)) {
			Token* val1 = convert_stack_top_to_token(postfix_bis);
			stack_pop(postfix_bis);
			
			Token* val2 = convert_stack_top_to_token(postfix_bis);
			stack_pop(postfix_bis);
		
			Token* res_op = evaluateOperator(val2, token, val1);
			if (!token_is_number(res_op)) {
				fprintf(stderr, "Division par 0. Expression non évaluée. Retourne 0. \n");
				div_0 = true;
			}
		
			stack_push(postfix_bis, res_op);

			delete_token(&token);
			delete_token(&val1);
			delete_token(&val2);
		}

		else if (token_is_number(token)) 
			stack_push(postfix_bis, token);
	}
	
	token = convert_stack_top_to_token(postfix_bis);
	resultat = (div_0 ? 0 : token_value(token));
	stack_pop(postfix_bis);

	delete_token(&token);
	delete_stack(&postfix_bis);
	delete_queue(&postfix);
	return resultat;
}

/** Main function for testing.
 * The main function expects one parameter that is the file where expressions to translate are
 * to be read.
 *
 * This file must contain a valid expression on each line
 *
 */
int main(int argc, char** argv){
	if (argc<2) {
		fprintf(stderr,"usage : %s filename\n", argv[0]);
		return 1;
	}
	
	FILE* input = fopen(argv[1], "r");

	if ( !input ) {
		perror(argv[1]);
		return 1;
	}

	computeExpressions(input);

	fclose(input);
	return 0;
}
 
void print_token(const void* e, void* user_param) {
	FILE* f = (FILE*)user_param;
	Token* t = (Token*)e;
	token_dump(f, t);
}

void print_queue(FILE* f, Queue* q) {
	fprintf(f, "(%d) --  ", queue_size(q));
	queue_map(q, print_token, f);
}